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
#include "flash.h"
#include "control.h"
#include "timers.h"

// =============================================================================
// Private type definitions
// =============================================================================

// =============================================================================
// Global variables
// =============================================================================

// =============================================================================
// Private constants
// =============================================================================

#define HEX_BYTE_STR_LEN  (2 + 1)
#define HEX_WORD_STR_LEN  (4 + 1)
#define HEX_DWORD_STR_LEN (8 + 1)

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
 Gets one byte from the flash data memory.
 Parameter: <index in hex format>
 Returns: <hex value of byte at specified index>
 */
static const char GET_FLASH[]       = "get flash";

/*§
 Gets the Kp constant in the PID regulator.
 Returns: <Kp value as  a decimal value>
 */
static const char GET_PID_KP[]      = "get kp";

/*§
 Gets the Ip constant in the PID regulator.
 Returns: <Ip value as  a decimal value>
 */
static const char GET_PID_KI[]      = "get ki";

/*§
 Gets the Dp constant in the PID regulator.
 Returns: <Dp value as  a decimal value>
 */
static const char GET_PID_KD[]      = "get kd";

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

/*§
 Sets one byte in the flash data memory.
 Paramter: <index in hex format> <one byte value in hex format>
 */
static const char SET_FLASH[]       = "set flash";

/*§
 Sets the Kp constant in the PID regulator.
 Parameter: <Value in the range [-65535.0, 65535.0]>
 */
static const char SET_PID_KP[]      = "set kp";

/*§
 Sets the Ki constant in the PID regulator.
 Parameter: <Value in the range [-65535.0, 65535.0]>
 */
static const char SET_PID_KI[]      = "set ki";

/*§
 Sets the Kd constant in the PID regulator.
 Parameter: <Value in the range [-65535.0, 65535.0]>
 */
static const char SET_PID_KD[]      = "set kd";

/*§
 Activates the heater PWM and sets the pwm value.
 Parameter: <Duty in range [0, 50]>
 */
static const char SET_HEAT_PWM[]  = "set heat pwm";

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

static void get_flash(void);

static void get_pid_kp(void);
static void get_pid_ki(void);
static void get_pid_kd(void);

static void set_heater(void);
static void set_servo_pos(void);
static void set_flash(void);

static void set_pid_kp(void);
static void set_pid_ki(void);
static void set_pid_kd(void);

static void set_heat_pwm(void);

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
        if (NULL != strstr(cmd_buffer, GET_FLASH))
        {
            get_flash();
        }
        else if (NULL != strstr(cmd_buffer, GET_PID_KP))
        {
            get_pid_kp();
        }
        else if (NULL != strstr(cmd_buffer, GET_PID_KI))
        {
            get_pid_ki();
        }
        else if (NULL != strstr(cmd_buffer, GET_PID_KD))
        {
            get_pid_kd();
        }
        else
        {
            syntax_error = true;
        }
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
        else if (NULL != strstr(cmd_buffer, SET_FLASH))
        {
            set_flash();
        }
        else if (NULL != strstr(cmd_buffer, SET_PID_KP))
        {
            set_pid_kp();
        }
        else if (NULL != strstr(cmd_buffer, SET_PID_KI))
        {
            set_pid_ki();
        }
        else if (NULL != strstr(cmd_buffer, SET_PID_KD))
        {
            set_pid_kd();
        }
        else if (NULL != strstr(cmd_buffer, SET_HEAT_PWM))
        {
            set_heat_pwm();
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

static void get_flash(void)
{
    uint8_t * p;
    char address_arg[HEX_WORD_STR_LEN] = {0};

    p = (uint8_t*)strstr(cmd_buffer, GET_FLASH);
    p += strlen(GET_FLASH);
    p += 1;     // +1 for space

    if (!isxdigit(*p))
    {
        arg_error = true;
    }
    else
    {
        uint8_t i = 0;
        uint16_t address;

        //
        // Parse address argument
        //
        while ((i != HEX_WORD_STR_LEN) && isxdigit(*p))
        {
            address_arg[i++] = *(p++);
        }

        address_arg[i] = NULL;
        address = (uint16_t)strtol(address_arg, NULL, 16);

        //
        // Perform flash read
        //
        if (address < FLASH_MEM_SIZE)
        {
            uint8_t value;
            char ans[32];
            
            value = flash_read_byte((flash_index_t)address);
            sprintf(ans, "%02X%s", value, NEWLINE);
            uart_write_string(ans);
        }
        else
        {
            arg_error = true;
        }
    }
}

static void get_pid_kp(void)
{
    char ans[32];

    sprintf(ans, "%lf%s", q16_16_to_double(control_get_kp()), NEWLINE);
    uart_write_string(ans);
}

static void get_pid_ki(void)
{
    char ans[32];

    sprintf(ans, "%lf%s", q16_16_to_double(control_get_ki()), NEWLINE);
    uart_write_string(ans);
}
static void get_pid_kd(void)
{
    char ans[32];

    sprintf(ans, "%lf%s", q16_16_to_double(control_get_kd()), NEWLINE);
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

static void set_flash(void)
{
    uint8_t * p;
    char address_arg[HEX_WORD_STR_LEN] = {0};
    char value_arg[HEX_BYTE_STR_LEN] = {0};

    p = (uint8_t*)strstr(cmd_buffer, SET_FLASH);
    p += strlen(SET_FLASH);
    p += 1;     // +1 for space

    if (!isxdigit(*p))
    {
        arg_error = true;
    }
    else
    {
        uint8_t i = 0;
        uint16_t address;
        uint8_t value;

        //
        // Parse address argument
        //
        while ((i != HEX_WORD_STR_LEN) && isxdigit(*p))
        {
            address_arg[i++] = *(p++);
        }

        address_arg[i] = NULL;
        arg_error = arg_error || (i == 0);
        address = (uint16_t)strtol(address_arg, NULL, 16);

        //
        // Parse value argument
        //
        p += 1; // +1 for space
        i = 0;
        
        while ((i != HEX_BYTE_STR_LEN) && isxdigit(*p))
        {
            value_arg[i++] = *(p++);
        }

        value_arg[i] = NULL;
        arg_error = arg_error || (i == 0);
        value = (uint8_t)strtol(value_arg, NULL, 16);
        
        //
        // Perform flash write
        //
        if (!arg_error && (address < FLASH_MEM_SIZE))
        {
            flash_init_write_buffer();
            flash_write_byte_to_buffer((flash_index_t)address, value);
            flash_write_buffer_to_flash();
        }
        else
        {
            arg_error = true;
        }
    }
}

static void set_pid_kp(void)
{
    uint8_t * p;
    char arg[16] = {0};
    uint8_t i = 0;

    p = (uint8_t*)strstr(cmd_buffer, SET_PID_KP);
    p += strlen(SET_PID_KP);
    p += 1;     // +1 for space

    while (isdigit(*p) || ('-' == *p))
    {
        arg[i++] = *(p++);
    }

    arg[i] = NULL;

    arg_error = (0 == i);

    if (!arg_error)
    {
        double kp_as_double = atoi(arg);
        q16_16_t kp = double_to_q16_16(kp_as_double);

        control_set_kp(kp);
        
        flash_init_write_buffer();
        flash_write_word_to_buffer(FLASH_INDEX_KP, (uint16_t)kp);
        flash_write_buffer_to_flash();
    }
}

static void set_pid_ki(void)
{
    uint8_t * p;
    char arg[16] = {0};
    uint8_t i = 0;

    p = (uint8_t*)strstr(cmd_buffer, SET_PID_KI);
    p += strlen(SET_PID_KI);
    p += 1;     // +1 for space

    while (isdigit(*p) || ('-' == *p))
    {
        arg[i++] = *(p++);
    }

    arg[i] = NULL;

    arg_error = (0 == i);

    if (!arg_error)
    {
        double ki_as_double = atoi(arg);
        q16_16_t ki = double_to_q16_16(ki_as_double);

        control_set_ki(ki);

        flash_init_write_buffer();
        flash_write_word_to_buffer(FLASH_INDEX_KI, (uint16_t)ki);
        flash_write_buffer_to_flash();
    }
}

static void set_pid_kd(void)
{
    uint8_t * p;
    char arg[16] = {0};
    uint8_t i = 0;

    p = (uint8_t*)strstr(cmd_buffer, SET_PID_KD);
    p += strlen(SET_PID_KD);
    p += 1;     // +1 for space

    while (isdigit(*p) || ('-' == *p))
    {
        arg[i++] = *(p++);
    }

    arg[i] = NULL;

    arg_error = (0 == i);

    if (!arg_error)
    {
        double kd_as_double = atoi(arg);
        q16_16_t kd = double_to_q16_16(kd_as_double);

        control_set_kd(kd);

        flash_init_write_buffer();
        flash_write_word_to_buffer(FLASH_INDEX_KD, (uint16_t)kd);
        flash_write_buffer_to_flash();
    }
}

static void set_heat_pwm(void)
{
    uint8_t * p;
    char arg[5] = {0};

    p = (uint8_t*)strstr(cmd_buffer, SET_HEAT_PWM);
    p += strlen(SET_HEAT_PWM);
    p += 1;     // +1 for space

    if (!isdigit(*p))
    {
        arg_error = true;
    }
    else
    {
        uint8_t i = 0;
        uint16_t pwm;

        while ((i != 5) && isdigit(*p))
        {
            arg[i++] = *(p++);
        }

        arg[i] = NULL;

        pwm = (uint16_t)atoi(arg);

        if ((pwm >= 0) && (pwm <= 50))
        {
            timers_activate_heater_control();
            timers_set_heater_duty(pwm);
        }
        else
        {
            arg_error = true;
        }
    }
}