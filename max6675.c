
// =============================================================================
// Include statements
// =============================================================================
#include "max6675.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <xc.h>

#include "gpio.h"
#include "status.h"
#include "timers.h"

// =============================================================================
// Private type definitions
// =============================================================================

typedef enum
{
    READ_STATE_IDLE,
    READ_STATE_READING_IC_1,
    READ_STATE_READING_IC_2,
} max6675_read_state_t;

#define FILTER_BUFFER_SIZE 20
typedef struct
{
    uint16_t latest;
    uint16_t oldest;
    uint16_t size;
    uint32_t sum;
    uint16_t mean;
    uint16_t buffer[FILTER_BUFFER_SIZE];
} filter_buffer_t;

// =============================================================================
// Global variables
// =============================================================================

// =============================================================================
// Private constants
// =============================================================================

static const uint16_t MAX6675_ERROR_MASK = 0x00060;
static const uint16_t MAX_TEMP_DIFF_BETWEEN_SENSORS = 100;
static const uint16_t MAX_TEMPERATURE = 300;


// =============================================================================
// Private variables
// =============================================================================

static volatile max6675_read_state_t read_state;
static volatile uint16_t ic1_reading;
static volatile uint16_t ic2_reading;

static volatile filter_buffer_t filter_buffer;

static volatile uint32_t last_reading_timestamp;
static volatile bool first_reading_complete = false;

// =============================================================================
// Private function declarations
// =============================================================================

/**
 * @Brief Adds one temperature reading to the filter buffer.
 * @param temp - reading to add.
 */
static void add_reading(uint16_t temp);

// =============================================================================
// Public function definitions
// =============================================================================

/*
 * From document DS39747D - page 119
 *
 * To set up the SPI module for the Standard Master mode
 * of operation:
 * 1. If using interrupts:
 * a) Clear the SPIxIF bit in the respective IFSx
 * register.
 * b) Set the SPIxIE bit in the respective IECx
 * register.
 * c) Write the SPIxIP bits in the respective IPCx
 * register to set the interrupt priority.
 * 2. Write the desired settings to the SPIxCON
 * register with MSTEN (SPIxCON1<5>) = 1.
 * 3. Clear the SPIROV bit (SPIxSTAT<6>).
 * 4. Enable SPI operation by setting the SPIEN bit
 * (SPIxSTAT<15>).
 * 5. Write the data to be transmitted to the SPIxBUF
 * register. Transmission (and reception) will start
 * as soon as data is written to the SPIxBUF
 * register.
 *
 */
void max6675_init(void)
{
    read_state = READ_STATE_IDLE;

    memset((filter_buffer_t*)&filter_buffer, 0, sizeof(filter_buffer));

    first_reading_complete = false;
    last_reading_timestamp = 0;

    IFS0bits.SPI1IF = 0;
    IEC0bits.SPI1IE = 1;
    IPC2bits.SPI1IP = 2;

    // Interrupt when the last bit is shifted out of SPIxSR,
    // now the transmit is complete
    SPI1STATbits.SISEL = 5;

    SPI1CON1 = 0x0000;

    SPI1CON1bits.DISSDO = 1;    // SDO as gpio
    SPI1CON1bits.MODE16 = 1;    // 16 bit mode
    SPI1CON1bits.MSTEN  = 1;    // Master mode

    // To get a spi baud of 1MHz use a 1:16 prescaler.
    // This assumes that the spi module is driven by a 16Mhz clock.

    // Secondary prescaler 1:1
    SPI1CON1bits.SPRE0 = 1;
    SPI1CON1bits.SPRE1 = 1;
    SPI1CON1bits.SPRE2 = 1;

    // Primary prescaler 1:16
    SPI1CON1bits.PPRE = 1;

    // Not in framed mode, Enhanced Buffer disabled
    SPI1CON2 = 0x0000;

    SPI1STATbits.SPIROV = 0;

    // Enable the spi module
    SPI1STATbits.SPIEN = 1;

}

void max6675_start_temp_reading(void)
{
    read_state = READ_STATE_READING_IC_1;

    MAX6675_1_CS_ON;
    SPI1BUF = 0x0000;   // load with dummy value to start reading
}

uint16_t max6675_get_current_temp(void)
{
    return filter_buffer.mean;
}

uint32_t max6675_get_last_reading_time(void)
{
    uint32_t t1;
    uint32_t t2;

    // Avoid that _SPI1Interrupt preempts and changes last_reading_timestamp
    // during an assignment.
    do
    {
        t1 = last_reading_timestamp;
        t2 = last_reading_timestamp;
    } while (t1 != t2);

    return t1;
}

bool max6675_first_reading_done(void)
{
    return first_reading_complete;
}

uint16_t max6675_read_blocking(void)
{
    uint16_t ret_val = 0x0000;

    IEC0bits.SPI1IE = 0;

    MAX6675_1_CS_ON;
    SPI1BUF = 0x0000;   // load with dummy value to start reading

    while (!SPI1STATbits.SPIRBF)
    {
        DEBUG_2_LED_ON;
    }
    DEBUG_2_LED_OFF;

    ret_val = SPI1BUF;
    MAX6675_1_CS_OFF;
    
    IEC0bits.SPI1IE = 1;

    return ret_val;
}

// =============================================================================
// Private function definitions
// =============================================================================

static void add_reading(uint16_t temp)
{
    if (filter_buffer.size == FILTER_BUFFER_SIZE)
    {
        filter_buffer.sum -= filter_buffer.buffer[filter_buffer.oldest];
        
        if (FILTER_BUFFER_SIZE == ++(filter_buffer.oldest))
        {
            filter_buffer.oldest = 0;
        }

        if (FILTER_BUFFER_SIZE == ++(filter_buffer.latest))
        {
            filter_buffer.latest = 0;
        }

        filter_buffer.buffer[filter_buffer.latest] = temp;
        filter_buffer.sum += temp;

        filter_buffer.mean = filter_buffer.sum / FILTER_BUFFER_SIZE;
    }
    else if (filter_buffer.size == 0)
    {
        filter_buffer.buffer[0] = temp;
        filter_buffer.sum = temp;
        filter_buffer.mean = temp;
        filter_buffer.size = 1;
    }
    else
    {
        filter_buffer.buffer[++(filter_buffer.latest)] = temp;
        filter_buffer.size += 1;
        filter_buffer.sum += temp;
        filter_buffer.mean = filter_buffer.sum / filter_buffer.size;
    }

    last_reading_timestamp = timers_get_millis();
    first_reading_complete = true;

    if (filter_buffer.mean > MAX_TEMPERATURE * 4)
    {
        status_set(CRIT_ERR_OVERTEMP, true);
    }
}

void __attribute__((interrupt, no_auto_psv)) _SPI1Interrupt(void)
{
    bool parse_read_values = false;
    DEBUG_1_LED_TOGGLE;

    switch (read_state)
    {
        case READ_STATE_READING_IC_1:
            MAX6675_1_CS_OFF;
            MAX6675_2_CS_ON;
            ic1_reading = SPI1BUF;
            read_state = READ_STATE_READING_IC_2;
            SPI1BUF = 0x0000;   // load with dummy value to start reading
            break;

        case READ_STATE_READING_IC_2:
            MAX6675_2_CS_OFF;
            ic2_reading = SPI1BUF;
            read_state = READ_STATE_IDLE;
            parse_read_values = true;
            break;

        case READ_STATE_IDLE:
            break;
    }

    if (parse_read_values)
    {
        uint16_t first_temp;
        uint16_t second_temp;

        first_temp = ic1_reading >> 3;
        first_temp &= 0x0FFF;

        second_temp = ic2_reading >> 3;
        second_temp &= 0x0FFF;

        if (ic1_reading & MAX6675_ERROR_MASK)
        {
            status_set(CRIT_ERR_LOOSE_THERMOCOUPLE, true);
            return;
        }

        if (ic2_reading & MAX6675_ERROR_MASK)
        {
            status_set(CRIT_ERR_LOOSE_THERMOCOUPLE, true);
            return;
        }

        if (first_temp > second_temp)
        {
            if (first_temp - second_temp > MAX_TEMP_DIFF_BETWEEN_SENSORS)
            {
                status_set(CRIT_ERR_INVALID_TEMP_READING, true);
                return;
            }
        }
        else
        {
            if (second_temp - first_temp > MAX_TEMP_DIFF_BETWEEN_SENSORS)
            {
                status_set(CRIT_ERR_INVALID_TEMP_READING, true);
                return;
            }
        }

        add_reading((first_temp + second_temp) / 2);
    }
}
