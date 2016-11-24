// =============================================================================
// Include statements
// =============================================================================
#include "lcd.h"

#include <stdbool.h>
#include <stdint.h>

#include <xc.h>

#include "gpio.h"
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

// =============================================================================
// Private variables
// =============================================================================

static volatile bool initialized = false;

// =============================================================================
// Private function declarations
// =============================================================================

//
// Get instruction code functions
//
static uint8_t get_instr_clear_display(void);

static uint8_t get_instr_return_home(void);

static uint8_t get_instr_entry_mode_set(bool i_d, bool s);

static uint8_t get_instr_display_on_off(bool disp_on,
                                        bool cursor_on,
                                        bool cursor_pos_on);

static uint8_t get_instr_function_set(bool use_8_bit_interface,
                                      bool use_2_lines,
                                      bool font_size_5_11);

static uint8_t get_instr_set_cgram_addr(uint8_t addr);

static uint8_t get_instr_set_ddram_addr(uint8_t addr);

static uint8_t get_instr_read_busy_flag(void);

static uint8_t get_instr_write_data_to_ram(uint8_t data);

static uint8_t get_instr_read_data_from_ram(void);


// =============================================================================
// Public function definitions
// =============================================================================

void lcd_init(void)
{
    if (!initialized)
    {
        
    }
}

void lcd_refresh(void)
{
    ;
}

void lcd_set_state(lcd_state_t new_state)
{
    ;
}


// =============================================================================
// Private function definitions
// =============================================================================

/*
 * From document DS39747D - page 105
 *
 * To configure any of the timers for individual 16-bit
 * operation:
 * 1. Clear the T32 bit corresponding to that timer
 * (T2CON<3> for Timer2 and Timer3 or
 * T4CON<3> for Timer4 and Timer5).
 * 2. Select the timer prescaler ratio using the
 * TCKPS1:TCKPS0 bits.
 * 3. Set the Clock and Gating modes using the TCS
 * and TGATE bits.
 * 4. Load the timer period value into the PRx register.
 * 5. If interrupts are required, set the interrupt enable
 * bit, TxIE; use the priority bits, TxIP2:TxIP0, to
 * set the interrupt priority.
 * 6. Set the TON bit (TxCON<15> = 1).
 */
static void lcd_timer_init(void)
{
    /*
     TON   = 0          Timer off
     TSIDL = 0          Continue module operation in Idle mode
     TGATE = 0          Gated time accumulation disabled
     TCKPS1:TCKPS0 = 0  Timery Input Clock Prescale Select bits
     T32   = 0          Timer4 and Timer5 act as two 16-bit timers
     TCS   = 0          Internal clock (FOSC/2)
    */
    T4CON = 0x00000;
    
    // Enable interrupt, low priority
    IFS1bits.T4IF = 0;
    IEC1bits.T4IE = 1;
    IPC6bits.T4IP = 1;
}

static void start_wait(uint16_t microseconds_to_wait)
{
    const uint16_t TIMER4_TICK_PER_USEC = 16;

    TMR4 = 0x0000;
    PR4 = microseconds_to_wait * TIMER4_TICK_PER_USEC;

    T4CONbits.TON = 0;
}

void __attribute__((interrupt, no_auto_psv)) _T4Interrupt(void)
{
    T4CONbits.TON = 0;
    IFS1bits.T4IF = 0;

    status_set(STATUS_LCD_WAIT_TIME_PASSED_FLAG, true);
}