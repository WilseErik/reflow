
// =============================================================================
// Include statements
// =============================================================================
#include "timers.h"

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "status.h"
#include "buttons.h"
#include "uart.h"
#include "gpio.h"

// =============================================================================
// Private type definitions
// =============================================================================

// =============================================================================
// Global variables
// =============================================================================

// =============================================================================
// Private constants
// =============================================================================



//
// HW constants
//

static uint32_t TIMER_CLOCK_FREQ_HZ = 16000000;
static uint32_t MSEC_TIMER_FREQ_HZ = 1000;


#define TCKPS_ONE_TO_ONE_PRESCALE                       0
#define TCKPS_ONE_TO_EIGHT_PRESCALE                     1
#define TCKPS_ONE_TO_SIXTYFOUR_PRESCALE                 2
#define TCKPS_ONE_TO_TWOHUNDRED_AND_FIFTYSIX_PRESCALE   3

#define TCS_INTERNAL_CLOCK                              0

//
// Heater PWM
//
static uint16_t HEATER_PWM_INTERVAL_20MS     = 50;

// =============================================================================
// Private variables
// =============================================================================
static volatile uint16_t prescaler_20ms = 0;
static volatile uint16_t prescaler_100ms = 0;
static volatile uint16_t prescaler_250ms = 0;
static volatile uint16_t prescaler_1000ms = 0;

static volatile uint32_t current_time = 0;

static volatile bool heater_control_on = false;
static volatile uint16_t heater_timer_20ms = 0;
static volatile uint16_t heater_duty = 0;

static volatile uint16_t reflow_time = 0;

// =============================================================================
// Private function declarations
// =============================================================================

// =============================================================================
// Public function definitions
// =============================================================================

/* From PIC24FJ128GA010 FAMILY datasheet, doc nbr DS39747D - page 103
 *
 * To configure Timer1 for operation:
 * 1. Set the TON bit (= 1).
 * 2. Select the timer prescaler ratio using the
 * TCKPS1:TCKPS0 bits.
 * 3. Set the Clock and Gating modes using the TCS
 * and TGATE bits.
 * 4. Set or clear the TSYNC bit to configure
 * synchronous or asynchronous operation.
 * 5. Load the timer period value into the PR1
 * register.
 * 6. If interrupts are required, set the interrupt enable
 * bit, T1IE. Use the priority bits, T1IP2:T1IP0, to
 * set the interrupt priority.
 *
 */
void timers_start_msec_timer(void)
{
    uint32_t counts;

    T1CON = 0x0000;
    T1CONbits.TON = 1;

    counts = TIMER_CLOCK_FREQ_HZ / MSEC_TIMER_FREQ_HZ;

    if ((counts / 1) < UINT16_MAX)
    {
        T1CONbits.TCKPS = TCKPS_ONE_TO_ONE_PRESCALE;
        PR1 = counts / 1;
    }
    else if ((counts / 8) < UINT16_MAX)
    {
        T1CONbits.TCKPS = TCKPS_ONE_TO_EIGHT_PRESCALE;
        PR1 = counts / 8;
    }
    else if ((counts / 64) < UINT16_MAX)
    {
        T1CONbits.TCKPS = TCKPS_ONE_TO_SIXTYFOUR_PRESCALE;
        PR1 = counts / 64;
    }
    else
    {
        T1CONbits.TCKPS = TCKPS_ONE_TO_TWOHUNDRED_AND_FIFTYSIX_PRESCALE;
        PR1 = counts / 256;
    }

    T1CONbits.TCS = TCS_INTERNAL_CLOCK;

    IEC0bits.T1IE = 1;  // Enable interrupt generation of timer 1
    IPC0bits.T1IP = 4;  // Set isr priority
}

uint32_t timers_get_millis(void)
{
    volatile uint32_t t1;
    volatile uint32_t t2;

    // Avoid incorrect readings of current_time due to preemption by T1 isr.
    do
    {
        t1 = current_time;
        t2 = current_time;
    }
    while (t1 != t2);

    return t1;
}

void timers_activate_heater_control(void)
{
    heater_control_on = true;
}

void timers_deactivate_heater_control(void)
{
    heater_control_on = false;
}

void timers_set_heater_duty(uint8_t duty)
{
    if (duty <= 50)
    {
        heater_duty = duty;
    }
    else
    {
        heater_duty = 50;
    }
}

uint16_t timers_get_reflow_time(void)
{
    volatile uint16_t t1;
    volatile uint16_t t2;

    // Prevent incorrect reads due to timer isr
    do
    {
        t1 = reflow_time;
        t2 = reflow_time;
    } while (t1 != t2);

    return t1;
}

void timers_reset_reflow_time(void)
{
    // Prevent incorrect writes due to timer isr
    do
    {
        reflow_time = 0;
    } while (reflow_time != 0);
}

// =============================================================================
// Private function definitions
// =============================================================================

void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void)
{
    IFS0bits.T1IF = 0;

    ++current_time;

    buttons_run_debounce_logic();
    
    if (20 == ++prescaler_20ms)
    {
        prescaler_20ms = 0;
        
        if (HEATER_PWM_INTERVAL_20MS == heater_timer_20ms)
        {
            heater_timer_20ms = 0;

            if (heater_control_on)
            {
                HEATER_ON;
            }
        }

        if (heater_timer_20ms == heater_duty)
        {
            if (heater_control_on)
            {
                HEATER_OFF;
            }
        }

        ++heater_timer_20ms;
    }

    if (100 == ++prescaler_100ms)
    {
        prescaler_100ms = 0;

        status_set(STATUS_RUN_PID_FLAG, true);
    }

    if (250 == ++prescaler_250ms)
    {
        static uint16_t start_delay;

        prescaler_250ms = 0;

        if (4 == start_delay)
        {
            status_set(STATUS_START_TEMP_READING_FLAG, true);
        }
        else
        {
            ++start_delay;
        }

        status_set(STATUS_LCD_REFRESH_FLAG, true);
    }

    if (1000 == ++prescaler_1000ms)
    {
        prescaler_1000ms = 0;

        ++reflow_time;
        status_set(STATUS_UART_LOG_TEMP_FLAG, true);
        status_set(STATUS_UPDATE_TARGET_TEMP_FLAG, true);
    }
}