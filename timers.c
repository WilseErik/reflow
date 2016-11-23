
// =============================================================================
// Include statements
// =============================================================================
#include "timers.h"

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "status.h"

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

// =============================================================================
// Private variables
// =============================================================================

static volatile uint16_t timer1_reload_value = 0;

static volatile uint16_t prescaler_10ms = 0;
static volatile uint16_t prescaler_250ms = 0;

static volatile uint32_t current_time = 0;

// =============================================================================
// Private function declarations
// =============================================================================

/**
 * @brief Calculates which value the timer should be reloaded with at overflow.
 */
static uint16_t calc_timer1_reload_val(uint32_t prescaler);

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
        timer1_reload_value = calc_timer1_reload_val(1);
    }
    else if ((counts / 8) < UINT16_MAX)
    {
        T1CONbits.TCKPS = TCKPS_ONE_TO_EIGHT_PRESCALE;
        timer1_reload_value = calc_timer1_reload_val(8);
    }
    else if ((counts / 64) < UINT16_MAX)
    {
        T1CONbits.TCKPS = TCKPS_ONE_TO_SIXTYFOUR_PRESCALE;
        timer1_reload_value = calc_timer1_reload_val(64);
    }
    else
    {
        T1CONbits.TCKPS = TCKPS_ONE_TO_TWOHUNDRED_AND_FIFTYSIX_PRESCALE;
        timer1_reload_value = calc_timer1_reload_val(256);
    }

    T1CONbits.TCS = TCS_INTERNAL_CLOCK;

    IEC0bits.T1IE = 1;  // Enable interrupt generation of timer 1
    IPC0bits.T1IP = 4;  // Set isr priority
}

uint32_t timers_get_millis(void)
{
    uint32_t t1;
    uint32_t t2;

    // Avoid incorrect readings of current_time due to preemption by T1 isr.
    do
    {
        t1 = current_time;
        t2 = current_time;
    }
    while (t1 != t2);

    return t1;
}

// =============================================================================
// Private function definitions
// =============================================================================

static uint16_t calc_timer1_reload_val(uint32_t prescaler)
{
    uint32_t counts = (TIMER_CLOCK_FREQ_HZ / prescaler) / MSEC_TIMER_FREQ_HZ;
    uint16_t reload = UINT16_MAX - counts;
    
    return reload;
}

void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void)
{
    TMR1 = timer1_reload_value;
    IFS0bits.T1IF = 0;

    ++current_time;
    
    if (10 == ++prescaler_10ms)
    {
        prescaler_10ms = 0;

        status_set(STATUS_READ_FIRST_TEMP_FLAG, true);
        status_set(STATUS_RUN_PID_FLAG, true);
    }

    if (250 == ++prescaler_250ms)
    {
        prescaler_250ms = 0;

        status_set(STATUS_UPDATE_LCD_FLAG, true);
        status_set(STATUS_UART_LOG_TEMP_FLAG, true);
    }

}