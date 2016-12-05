// =============================================================================
// Include statements
// =============================================================================
#include "terminal.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "uart.h"
#include "terminal_help.h"

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

#define CMD_BUFFER_SIZE 1025

//
// Commands
//

/*§
 Say hi!
 */
static const char CMD_HELLO[]    = "hello";

// =============================================================================
// Private variables
// =============================================================================

static char cmd_buffer[CMD_BUFFER_SIZE] = {0};

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

    if (NULL != strstr(cmd_buffer, CMD_TYPE_HELP))
    {
        terminal_help(cmd_buffer);
    }
    else if (NULL != strstr(cmd_buffer, CMD_TYPE_GET))
    {
        ;
    }
    else if (NULL != strstr(cmd_buffer, CMD_TYPE_SET))
    {
        ;
    }
    else
    {
        if (NULL != strstr(cmd_buffer, CMD_HELLO))
        {
            cmd_hello();
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
    else
    {

    }
}

static void cmd_hello(void)
{
    uart_write_string("Hello!");
    uart_write_string(NEWLINE);
}
