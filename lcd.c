// =============================================================================
// Include statements
// =============================================================================
#include "lcd.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <xc.h>

#include "gpio.h"
#include "status.h"

// =============================================================================
// Private type definitions
// =============================================================================

typedef struct lcd_instr_t
{
    bool rs;
    bool rw;
    uint8_t db;
    uint16_t wait_us;
    bool valid;
} lcd_instr_t;

#define TASK_LIST_SIZE 100

typedef struct lcd_task_list_t
{
    lcd_instr_t tasks[TASK_LIST_SIZE];
    uint16_t first;
    uint16_t last;
} task_list_t;

// =============================================================================
// Global variables
// =============================================================================

// =============================================================================
// Private constants
// =============================================================================

#define WAIT_1_USEC do {                                                       \
                __builtin_nop;__builtin_nop;__builtin_nop;__builtin_nop;       \
                __builtin_nop;__builtin_nop;__builtin_nop;__builtin_nop;       \
                __builtin_nop;__builtin_nop;__builtin_nop;__builtin_nop;       \
                __builtin_nop;__builtin_nop;__builtin_nop;__builtin_nop;       \
                     } while (0)

static const uint16_t DDRAM_FIRST_LINE_START = 0x00;
static const uint16_t DDRAM_SECOND_LINE_START = 0x40;

// =============================================================================
// Private variables
// =============================================================================

static volatile bool initialized = false;
static volatile bool queue_is_executing = false;

static volatile task_list_t task_list;

// =============================================================================
// Private function declarations
// =============================================================================

/**
 * @brief Starts to run the instruction queue.
 * @details The queue is run asyncronously.
 */
static void run_queue(void);

//
// Add instruction functions
//
static void queue_instr_clear_display(void);

static void queue_instr_return_home(void);

static void queue_instr_entry_mode_set(bool i_d, bool s);

static void queue_instr_display_on_off(bool disp_on,
                                        bool cursor_on,
                                        bool cursor_pos_on);

static void queue_instr_disp_shift(bool s_c, bool r_l);

static void queue_instr_function_set(bool use_8_bit_interface,
                                      bool use_2_lines,
                                      bool font_size_5_11);

static void queue_instr_set_cgram_addr(uint8_t addr);

static void queue_instr_set_ddram_addr(uint8_t addr);

static void queue_instr_read_busy_flag(void);

static void queue_instr_write_data_to_ram(uint8_t data);

static void queue_instr_read_data_from_ram(void);

//
// Time
//
/**
 * @brief Initializes the timer.
 */
static void lcd_timer_init(void);

/**
 * @brief Sets a time to wait.
 * @details When the wait time expires, wait_time_complete will be called.
 * @param microseconds_to_wait - Time to wait in us.
 */
static void start_wait(uint16_t microseconds_to_wait);

/**
 * @brief Called every time a wait time completes.
 */
static void wait_time_complete(void);

// =============================================================================
// Public function definitions
// =============================================================================

void lcd_init(void)
{
    if (!initialized)
    {
        memset((task_list_t*)&task_list, 0, sizeof (task_list_t));
        lcd_timer_init();

        queue_instr_function_set(true,  // 8 bit mode
                                 true,  // use 2 lines, not 1
                                 true); // dont care when using 2 lines
        queue_instr_function_set(true, true, true);
        
        // Disp on, no cursor
        queue_instr_display_on_off(true, false, false);

        queue_instr_clear_display();

        queue_instr_entry_mode_set(true, false);    // do not shift display.

        run_queue();
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

bool lcd_is_busy(void)
{
    return queue_is_executing;
}

void lcd_set_text(char first_line[LCD_LINE_LEN],
                  char second_line[LCD_LINE_LEN])
{
    // The first line starts at ddram address 0x00
    // The second line starts at ddram address 0x40
    uint16_t i;

    if (!initialized || lcd_is_busy())
    {
        return;
    }

    queue_instr_set_ddram_addr(DDRAM_FIRST_LINE_START);

    for (i = 0; i != LCD_LINE_LEN; ++i)
    {
        queue_instr_write_data_to_ram(first_line[i]);
    }

    queue_instr_set_ddram_addr(DDRAM_SECOND_LINE_START);

    for (i = 0; i != LCD_LINE_LEN; ++i)
    {
        queue_instr_write_data_to_ram(second_line[i]);
    }
    

    
}


// =============================================================================
// Private function definitions
// =============================================================================

static void run_queue(void)
{
    uint16_t i = task_list.first;

    queue_is_executing = task_list.tasks[i].valid;

    if (task_list.tasks[i].valid)
    {
        if (task_list.tasks[i].rs)
        {
            LCD_RS_PIN = 1;
        }
        else
        {
            LCD_RS_PIN = 0;
        }


        if (task_list.tasks[i].rw)
        {
            LCD_RW_PIN = 1;

            LCD_SET_DATA_DIR_IN;
            LCD_E_PIN = 1;

            // Wait at least 460ns
            WAIT_1_USEC;

            LCD_E_PIN = 0;

            start_wait(task_list.tasks[i].wait_us);
        }
        else
        {
            // Write mode
            LCD_RW_PIN = 0;

            LCD_SET_DATA_DIR_OUT;
            LCD_SET_DATA(task_list.tasks[i].db);
            LCD_E_PIN = 1;

            // Wait at least 460ns
            WAIT_1_USEC;

            LCD_E_PIN = 0;

            start_wait(task_list.tasks[i].wait_us);
        }
    }
    else if (!initialized)
    {
        // End of queue reached, if not initialized this queue must have been
        // the initialization sequence.
        initialized = true;
    }
}

static void queue_instr_clear_display(void)
{
    uint16_t next = task_list.last + 1;

    if (TASK_LIST_SIZE == next)
    {
        next = 0;
    }

    task_list.tasks[next].rs = false;
    task_list.tasks[next].rw = false;
    task_list.tasks[next].db = 0x01;
    task_list.tasks[next].wait_us = 2000;
    task_list.tasks[next].valid = true;

    task_list.last = next;
}

static void queue_instr_return_home(void)
{
    uint16_t next = task_list.last + 1;

    if (TASK_LIST_SIZE == next)
    {
        next = 0;
    }

    task_list.tasks[next].rs = false;
    task_list.tasks[next].rw = false;
    task_list.tasks[next].db = 0x02;
    task_list.tasks[next].wait_us = 2000;
    task_list.tasks[next].valid = true;

    task_list.last = next;
}

static void queue_instr_entry_mode_set(bool i_d, bool s)
{
    uint16_t next = task_list.last + 1;

    if (TASK_LIST_SIZE == next)
    {
        next = 0;
    }

    task_list.tasks[next].rs = false;
    task_list.tasks[next].rw = false;
    task_list.tasks[next].db = 0x04;

    if (i_d)
        task_list.tasks[next].db |= 0x02;
    if (s)
        task_list.tasks[next].db |= 0x01;

    task_list.tasks[next].wait_us = 50;
    task_list.tasks[next].valid = true;

    task_list.last = next;
}

static void queue_instr_display_on_off(bool disp_on,
                                        bool cursor_on,
                                        bool cursor_pos_on)
{
    uint16_t next = task_list.last + 1;

    if (TASK_LIST_SIZE == next)
    {
        next = 0;
    }

    task_list.tasks[next].rs = false;
    task_list.tasks[next].rw = false;
    task_list.tasks[next].db = 0x08;

    if (disp_on)
        task_list.tasks[next].db |= 0x04;
    if (cursor_on)
        task_list.tasks[next].db |= 0x02;
    if (cursor_pos_on)
        task_list.tasks[next].db |= 0x01;

    task_list.tasks[next].wait_us = 50;
    task_list.tasks[next].valid = true;

    task_list.last = next;
}

static void queue_instr_disp_shift(bool s_c, bool r_l)
{
    uint16_t next = task_list.last + 1;

    if (TASK_LIST_SIZE == next)
    {
        next = 0;
    }

    task_list.tasks[next].rs = false;
    task_list.tasks[next].rw = false;
    task_list.tasks[next].db = 0x10;

    if (s_c)
        task_list.tasks[next].db |= 0x08;
    if (r_l)
        task_list.tasks[next].db |= 0x04;

    task_list.tasks[next].wait_us = 50;
    task_list.tasks[next].valid = true;

    task_list.last = next;
}

static void queue_instr_function_set(bool use_8_bit_interface,
                                      bool use_2_lines,
                                      bool font_size_5_11)
{
    uint16_t next = task_list.last + 1;

    if (TASK_LIST_SIZE == next)
    {
        next = 0;
    }

    task_list.tasks[next].rs = false;
    task_list.tasks[next].rw = false;
    task_list.tasks[next].db = 0x20;

    if (use_8_bit_interface)
        task_list.tasks[next].db |= 0x10;
    if (use_2_lines)
        task_list.tasks[next].db |= 0x08;
    if (font_size_5_11)
        task_list.tasks[next].db |= 0x04;

    task_list.tasks[next].wait_us = 50;
    task_list.tasks[next].valid = true;

    task_list.last = next;
}

static void queue_instr_set_cgram_addr(uint8_t addr)
{
    uint16_t next = task_list.last + 1;

    if (TASK_LIST_SIZE == next)
    {
        next = 0;
    }

    task_list.tasks[next].rs = false;
    task_list.tasks[next].rw = false;
    task_list.tasks[next].db = 0x40 | (addr & 0x3F);
    task_list.tasks[next].wait_us = 50;
    task_list.tasks[next].valid = true;

    task_list.last = next;
}

static void queue_instr_set_ddram_addr(uint8_t addr)
{
    uint16_t next = task_list.last + 1;

    if (TASK_LIST_SIZE == next)
    {
        next = 0;
    }

    task_list.tasks[next].rs = false;
    task_list.tasks[next].rw = false;
    task_list.tasks[next].db = 0x80 | (addr & 0x7F);
    task_list.tasks[next].wait_us = 50;
    task_list.tasks[next].valid = true;

    task_list.last = next;
}

static void queue_instr_read_busy_flag(void)
{
    uint16_t next = task_list.last + 1;

    if (TASK_LIST_SIZE == next)
    {
        next = 0;
    }

    task_list.tasks[next].rs = false;
    task_list.tasks[next].rw = true;
    task_list.tasks[next].db = 0;
    task_list.tasks[next].wait_us = 0;
    task_list.tasks[next].valid = true;

    task_list.last = next;

    lcd_instr_t i;
    i.rs = false;
    i.rw = true;
    i.db = 0x00;
}

static void queue_instr_write_data_to_ram(uint8_t data)
{
    uint16_t next = task_list.last + 1;

    if (TASK_LIST_SIZE == next)
    {
        next = 0;
    }

    task_list.tasks[next].rs = true;
    task_list.tasks[next].rw = false;
    task_list.tasks[next].db = 0xC0 | (data & 0x3F);
    task_list.tasks[next].wait_us = 50;
    task_list.tasks[next].valid = true;

    task_list.last = next;
}

static void queue_instr_read_data_from_ram(void)
{
    uint16_t next = task_list.last + 1;

    if (TASK_LIST_SIZE == next)
    {
        next = 0;
    }

    task_list.tasks[next].rs = true;
    task_list.tasks[next].rw = true;
    task_list.tasks[next].db = 0x00;
    task_list.tasks[next].wait_us = 50;
    task_list.tasks[next].valid = true;

    task_list.last = next;
}



//
// Time
//

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
    if (0 == microseconds_to_wait)
    {
        wait_time_complete();
    }
    else
    {
        TMR4 = 0x0000;
        // There is 16 timer4 ticks per microsecond
        PR4 = microseconds_to_wait << 4;

        T4CONbits.TON = 1;
    }
}

static void wait_time_complete(void)
{
    uint16_t next_first = task_list.first + 1;

    if (TASK_LIST_SIZE == next_first)
    {
        next_first = 0;
    }

    task_list.tasks[task_list.first].valid = false;
    task_list.first = next_first;

    run_queue();

}

void __attribute__((interrupt, no_auto_psv)) _T4Interrupt(void)
{
    T4CONbits.TON = 0;
    IFS1bits.T4IF = 0;

    wait_time_complete();
}