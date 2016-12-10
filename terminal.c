// =============================================================================
// Include statements
// =============================================================================
#include "terminal.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "uart.h"
#include "terminal_help.h"
#include "max6675.h"
#include "gpio.h"
#include "servo.h"

// =============================================================================
// Private type definitions
// =============================================================================

// =============================================================================
// Global variables
// =============================================================================

// =============================================================================
// Private constants
// =============================================================================

static const char NEWLINE[]         = "\r\n";

static const char CMD_TYPE_HELP[]   = "help";
static const char CMD_TYPE_SET[]    = "set ";
static const char CMD_TYPE_GET[]    = "get ";

static const char SYNTAX_ERROR[]    = "[Syntax error]";
static const char ARGUMENT_ERROR[]  = "[Invalid argument]";

#define CMD_BUFFER_SIZE 1025

//
// Commands
//

/*§
 Say hi!
 */
static const char CMD_HELLO[]       = "hello";

/*§
 Reads one value from the first MAX6675 ic.
 Returns: <read value as 16 bits in hex format>
 */
static const char CMD_TEST_TEMP[]   = "test temp";

/*§
 Sets the heater on or off.
 Parameter: <'on' or 'off'>
 */
static const char SET_HEATER[]      = "set heater";

/*§
 Sets the servo position.
 Parameter: <position as integer in [0, 1200]>
 */
static const char SET_SERVO_POS[]   = "set servo pos";

// =============================================================================
// Private variables
// =============================================================================

static char cmd_buffer[CMD_BUFFER_SIZE] = {0};
static bool arg_error = false;

// =============================================================================
// Private function declarations
// =============================================================================

/**
 * @brief Copies the uart rx buffer to the cmd_buffer.
 */
static void copy_to_cmd_buffer(void);

/**
 * @brief Parses and executes the command in the command buffer.
 */
static void execute_command(void);

//
// Command help functions
//
static void cmd_hello(void);
static void cmd_test_temp(void);

static void set_heater(void);
static void set_servo_pos(void);

// =============================================================================
// Public function definitions
// =============================================================================

void terminal_handle_uart_event(void)
{
    copy_to_cmd_buffer();
    execute_command();
}

// =============================================================================
// Private function definitions
// =============================================================================

static void copy_to_cmd_buffer(void)
{
    uint16_t nbr_of_bytes;
    uint16_t current_index = 0;

    nbr_of_bytes = uart_get_receive_buffer_size();

    for (current_index = 0; current_index != nbr_of_bytes; ++current_index)
    {
        cmd_buffer[current_index] = (char)uart_get(current_index);
    }

    cmd_buffer[current_index] = NULL;

    uart_clear_receive_buffer();
}

static void execute_command(void)
{
    bool syntax_error = false;
    arg_error = false;

    if (NULL != strstr(cmd_buffer, CMD_TYPE_HELP))
    {
        terminal_help(cmd_buffer);
    }
    else if (NULL != strstr(cmd_buffer, CMD_TYPE_GET))
    {
        syntax_error = true;
    }
    else if (NULL != strstr(cmd_buffer, CMD_TYPE_SET))
    {
        if (NULL != strstr(cmd_buffer, SET_HEATER))
        {
            set_heater();
        }
        else if (NULL != strstr(cmd_buffer, SET_SERVO_POS))
        {
            set_servo_pos();
        }
        else
        {
            syntax_error = true;
        }
    }
    else
    {
        if (NULL != strstr(cmd_buffer, CMD_HELLO))
        {
            cmd_hello();
        }
        else if (NULL != strstr(cmd_buffer, CMD_TEST_TEMP))
        {
            cmd_test_temp();
        }
        else
        {
            syntax_error = true;
        }
    }

    if (syntax_error)
    {
        uart_write_string(SYNTAX_ERROR);
        uart_write_string(NEWLINE);
    }
    else if (arg_error)
    {
        uart_write_string(ARGUMENT_ERROR);
        uart_write_string(NEWLINE);
    }
}

static void cmd_hello(void)
{
    uart_write_string("Hello!");
    uart_write_string(NEWLINE);
}

static void cmd_test_temp(void)
{
    char ans[32] = {0};
    uint16_t reading = max6675_read_blocking();
    sprintf(ans, "%04X%s", reading, NEWLINE);
    uart_write_string(ans);
}

static void set_heater(void)
{
    uint8_t * p;

    p = (uint8_t*)strstr(cmd_buffer, SET_HEATER);
    p += strlen(SET_HEATER);
    p += 1;     // +1 for space

    if (('o' == *p) && ('n' == *(p + 1)))
    {
        HEATER_ON;
    } else if (('o' == *p) && ('f' == *(p + 1)) && ('f' == *(p + 2)))
    {
        HEATER_OFF;
    }
    else
    {
        arg_error = true;
    }
}

static void set_servo_pos(void)
{
    uint8_t * p;
    char arg[5] = {0};

    p = (uint8_t*)strstr(cmd_buffer, SET_SERVO_POS);
    p += strlen(SET_SERVO_POS);
    p += 1;     // +1 for space

    if (!isdigit(*p))
    {
        arg_error = true;
    }
    else
    {
        uint8_t i = 0;
        uint16_t pos;

        while ((i != 5) && isdigit(*p))
        {
            arg[i++] = *(p++);
        }

        arg[i] = NULL;

        pos = (uint16_t)atoi(arg);

        if ((pos > 0) && (pos <= 1200))
        {
            servo_set_pos(pos);
        }
        else
        {
            arg_error = true;
        }
    }


}