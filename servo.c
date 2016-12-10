/*
 * This file acts as a driver for the servo motor.
 *
 * The servo has the following spec.:
 *  Pulse width:  900 - 2100us
 *  Pulse period: Let's go with 20ms
 *
 *
 * The servo is controlled by sending pulses of fixed lengths like:
 *
 * ^               period = 20ms
 * |<------------------------------------------->
 * |
 * |xxxxxxxxx                                    xxxx
 * |x       x                                    x
 * |x       x                                    x
 * |x       x                                    x
 * |x       x                                    x
 *  --------xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx--->
 *
 *  <------->
 * {pulse width | 900 - 2100us}
 *
 *
 * The period is measured by timer 2, while the pulse width is measured
 * by timer 3.
 *
 */

// =============================================================================
// Include statements
// =============================================================================

#include "servo.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <xc.h>

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

#define TIMER_CLOCK_FREQ_HZ 16000000
#define SERVO_INTERVAL_HZ 50
#define PULSE_WIDTH_MAX_US 2100
#define PULSE_WIDTH_MIN_US 900

#define TCKPS_ONE_TO_ONE_PRESCALE                       0
#define TCKPS_ONE_TO_EIGHT_PRESCALE                     1
#define TCKPS_ONE_TO_SIXTYFOUR_PRESCALE                 2
#define TCKPS_ONE_TO_TWOHUNDRED_AND_FIFTYSIX_PRESCALE   3

// =============================================================================
// Private variables
// =============================================================================

// =============================================================================
// Private function declarations
// =============================================================================

/**
 * @brief Calculates the timer prescale setting.
 * @param clock_counts  The amounts of clock cycles per timer period.
 * @return TxCON TCKPS value
 */
uint8_t calc_timer_prescaler(uint32_t clock_counts);

/**
 * @brief Calculates the value of the PRx register.
 * @param timer_con_reg     Address of the TxCON register.
 * @param counts            Number of clock cycles per period.
 * @return Calulated PRx register value.
 */
uint16_t calc_pr_reg(volatile uint16_t * timer_con_reg, uint32_t counts);

// =============================================================================
// Public function definitions
// =============================================================================

void servo_init(void)
{
    uint32_t counts;

    SERVO_CONTROL_DIR = DIR_OUT;
    SERVO_CONTROL_PIN = 0;

    //
    // Set up timer 2
    //
    /*
     TON   = 0          Timer off
     TSIDL = 0          Continue module operation in Idle mode
     TGATE = 0          Gated time accumulation disabled
     TCKPS1:TCKPS0 = 0  Timer2 Input Clock Prescale Select bits
     T32   = 0          Timer2 and Timer3 act as two 16-bit timers
     TCS   = 0          Internal clock (FOSC/2)
    */
    T2CON = 0x00000;

    // Enable interrupt, low priority
    IFS0bits.T2IF = 0;
    IEC0bits.T2IE = 1;
    IPC1bits.T2IP = 1;
    
    counts = TIMER_CLOCK_FREQ_HZ / SERVO_INTERVAL_HZ;
    
    T2CONbits.TCKPS = calc_timer_prescaler(counts);
    PR2 = calc_pr_reg(&T2CON, counts);

    //
    // Set up timer 3
    //
    /*
     TON   = 0          Timer off
     TSIDL = 0          Continue module operation in Idle mode
     TGATE = 0          Gated time accumulation disabled
     TCKPS1:TCKPS0 = 0  Timer2 Input Clock Prescale Select bits
     T32   = 0          Timer2 and Timer3 act as two 16-bit timers
     TCS   = 0          Internal clock (FOSC/2)
    */
    T3CON = 0x00000;

    // Enable interrupt, low priority
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1;
    IPC2bits.T3IP = 2;

    counts = (TIMER_CLOCK_FREQ_HZ / 1000000) * PULSE_WIDTH_MAX_US;

    T3CONbits.TCKPS = calc_timer_prescaler(counts);
    PR3 = calc_pr_reg(&T3CON, counts);

    //
    // Start timer 2
    //
    T2CONbits.TON = 1;
}

void servo_set_pos(uint16_t position)
{
    uint16_t pulse_width_us = PULSE_WIDTH_MIN_US + position;
    uint32_t clock_counts = (TIMER_CLOCK_FREQ_HZ / 1000000) * pulse_width_us;

    T3CONbits.TON = 0;
    SERVO_CONTROL_PIN = 0;
    TMR3 = 0x00000000;
    PR3 = calc_pr_reg(&T3CON, clock_counts);
}

// =============================================================================
// Private function definitions
// =============================================================================

void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void)
{
    IFS0bits.T2IF = 0;

    T3CONbits.TON = 1;

    SERVO_CONTROL_PIN = 1;
}

void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void)
{
    IFS0bits.T3IF = 0;

    T3CONbits.TON = 0;
    TMR3 = 0x00000000;

    SERVO_CONTROL_PIN = 0;
}

uint8_t calc_timer_prescaler(uint32_t counts)
{
    uint8_t ret_val;

    if ((counts / 1) < UINT16_MAX)
    {
        ret_val = TCKPS_ONE_TO_ONE_PRESCALE;
    }
    else if ((counts / 8) < UINT16_MAX)
    {
        ret_val = TCKPS_ONE_TO_EIGHT_PRESCALE;
    }
    else if ((counts / 64) < UINT16_MAX)
    {
        ret_val = TCKPS_ONE_TO_SIXTYFOUR_PRESCALE;
    }
    else
    {
        ret_val = TCKPS_ONE_TO_TWOHUNDRED_AND_FIFTYSIX_PRESCALE;
    }
    
    return ret_val;
}

uint16_t calc_pr_reg(volatile uint16_t * timer_con_reg, uint32_t counts)
{
    // T1CONBITS - T5CONBITS are the same in regards to prescaler bits,
    // let's use T2CONBITS
    T2CONBITS * reg = (T2CONBITS*)timer_con_reg;

    if (TCKPS_ONE_TO_ONE_PRESCALE == reg->TCKPS)
    {
        counts = counts;
    }
    else if (TCKPS_ONE_TO_EIGHT_PRESCALE == reg->TCKPS)
    {
        counts /= 8;
    }
    else if (TCKPS_ONE_TO_SIXTYFOUR_PRESCALE == reg->TCKPS)
    {
        counts /= 64;
    }
    else if (TCKPS_ONE_TO_TWOHUNDRED_AND_FIFTYSIX_PRESCALE == reg->TCKPS)
    {
        counts /= 256;
    }

    return counts;
}