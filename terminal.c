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
#include "buttons.h"

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
 Evalues the temperature curve at one point in time.
 Parameters: <seconds in to reflow program>
 Returns: <target temperature>
 */
static const char CMD_TEMP_CURVE_EVAL[] = "temp curve eval";

/*§
 Initiates the flash write buffer with the contents of theflash data memory.
 */
static const char CMD_INIT_WRITE_BUFFER[] = "init flash bufffer";

/*§
 Writes one byte to the flash buffer.
 Paramters: <index in hex format> <one byte value in hex format>
 */
static const char CMD_BUFFERED_WRITE[] = "buffered write";

/*§
 Write the contents of the flash buffer to the flash memory.
 */
static const char CMD_FLUSH_BUFFER[]    = "flush flash buffer";

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
 Gets the PID integral tracking time constant.
 Returns: <PID integral tracking time constant>
 */
static const char GET_PID_TTR[]     = "get ttr";

/*§
 Gets the max gain of the PID derivate term.
 Returns: <Max PID derivate part gain>
 */
static const char GET_PID_D_MAX_GAIN[] = "get d max gain";

/*§
 Gets the servo output scaling factor.
 Return: <scaling factor as positive float value>
 */
static const char GET_PID_SERVO_FACTOR[] = "get pid servo factor";

/*§
 Gets the timestamp in [s] for when the soak period starts.
 This is done for the profile selected by the reflow profile switch.
 */
static const char GET_START_OF_SOAK[] = "get start of soak";

/*§
 Gets the timestamp in [s] for when the reflow period starts.
 This is done for the profile selected by the reflow profile switch.
 */
static const char GET_START_OF_REFLOW[] = "get start of reflow";

/*§
 Gets the timestamp in [s] for when the cooling period starts.
 This is done for the profile selected by the reflow profile switch.
 */
static const char GET_START_OF_COOL[] = "get start of cool";

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
 Sets the integrator tracking time constant in the PID regulator.
 Parameter: <Value in the range [0.0, 65535.0]>
 */
static const char SET_PID_TTR[]     = "set ttr";

/*§
 Sets the max gain of the PID derivative term.
Parameter: <Value in the range [0.0, 65535.0]>
 */
static const char SET_PID_D_MAX_GAIN[] = "set d max gain";

/*§
 Sets how much the servo should extend for negative control values.
 Paramter: <positive float scaling value>
 */
static const char SET_PID_SERVO_FACTOR[] = "set pid servo factor";

/*§
 Activates the heater PWM and sets the pwm value.
 Parameter: <Duty in range [0, 50]>
 */
static const char SET_HEAT_PWM[]  = "set heat pwm";

/*§
 Sets the timestamp in [s] for when the soak period starts.
 This is done for the profile selected by the reflow profile switch.
 Parameter: <timestamp in seconds>
 */
static const char SET_START_OF_SOAK[] = "set start of soak";

/*§
 Sets the timestamp in [s] for when the reflow period starts.
 This is done for the profile selected by the reflow profile switch.
 Parameter: <timestamp in seconds>
 */
static const char SET_START_OF_REFLOW[] = "set start of reflow";

/*§
 Sets the timestamp in [s] for when the cool period starts.
 This is done for the profile selected by the reflow profile switch.
 Parameter: <timestamp in seconds>
 */
static const char SET_START_OF_COOL[] = "set start of cool";

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
static void cmd_temp_curve_eval(void);

static void cmd_init_flash_buffer(void);
static void cmd_buffered_write(void);
static void cmd_flush_buffer(void);

static void get_flash(void);

static void get_pid_kp(void);
static void get_pid_ki(void);
static void get_pid_kd(void);
static void get_pid_ttr(void);
static void get_pid_d_max_gain(void);
static void get_pid_servo_factor(void);

static void get_start_of_soak(void);
static void get_start_of_reflow(void);
static void get_start_of_cool(void);

static void set_heater(void);
static void set_servo_pos(void);
static void set_flash(void);

static void set_pid_kp(void);
static void set_pid_ki(void);
static void set_pid_kd(void);
static void set_pid_ttr(void);
static void set_pid_max_d_gain(void);
static void set_pid_servo_factor(void);

static void set_start_of_soak(void);
static void set_start_of_reflow(void);
static void set_start_of_cool(void);

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
        else if (NULL != strstr(cmd_buffer, GET_PID_TTR))
        {
            get_pid_ttr();
        }
        else if (NULL != strstr(cmd_buffer, GET_PID_D_MAX_GAIN))
        {
            get_pid_d_max_gain();
        }
        else if (NULL != strstr(cmd_buffer, GET_PID_SERVO_FACTOR))
        {
            get_pid_servo_factor();
        }
        else if (NULL != strstr(cmd_buffer, GET_START_OF_SOAK))
        {
            get_start_of_soak();
        }
        else if (NULL != strstr(cmd_buffer, GET_START_OF_REFLOW))
        {
            get_start_of_reflow();
        }
        else if (NULL != strstr(cmd_buffer, GET_START_OF_COOL))
        {
            get_start_of_cool();
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
        else if (NULL != strstr(cmd_buffer, SET_PID_TTR))
        {
            set_pid_ttr();
        }
        else if (NULL != strstr(cmd_buffer, SET_PID_D_MAX_GAIN))
        {
            set_pid_max_d_gain();
        }
        else if (NULL != strstr(cmd_buffer, SET_PID_SERVO_FACTOR))
        {
            set_pid_servo_factor();
        }
        else if (NULL != strstr(cmd_buffer, SET_HEAT_PWM))
        {
            set_heat_pwm();
        }
        else if (NULL != strstr(cmd_buffer, SET_START_OF_SOAK))
        {
            set_start_of_soak();
        }
        else if (NULL != strstr(cmd_buffer, SET_START_OF_REFLOW))
        {
            set_start_of_reflow();
        }
        else if (NULL != strstr(cmd_buffer, SET_START_OF_COOL))
        {
            set_start_of_cool();
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
        else if (NULL != strstr(cmd_buffer, CMD_TEMP_CURVE_EVAL))
        {
            cmd_temp_curve_eval();
        }
        else if (NULL != strstr(cmd_buffer, CMD_INIT_WRITE_BUFFER))
        {
            cmd_init_flash_buffer();
        }
        else if (NULL != strstr(cmd_buffer, CMD_BUFFERED_WRITE))
        {
            cmd_buffered_write();
        }
        else if (NULL != strstr(cmd_buffer, CMD_FLUSH_BUFFER))
        {
            cmd_flush_buffer();
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

static void cmd_temp_curve_eval(void)
{
    char ans[32] = {0};
    uint8_t * p;
    char arg[16] = {0};

    p = (uint8_t*)strstr(cmd_buffer, CMD_TEMP_CURVE_EVAL);
    p += strlen(CMD_TEMP_CURVE_EVAL);
    p += 1;     // +1 for space

    if (!isdigit(*p))
    {
        arg_error = true;
    }
    else
    {
        uint8_t i = 0;
        uint16_t time;
        q16_16_t temp;

        while ((i != 15) && isdigit(*p))
        {
            arg[i++] = *(p++);
        }

        arg[i] = NULL;

        time = (uint16_t)atoi(arg);

        temp = temp_curve_eval(time);

        sprintf(ans, "%lf%s", q16_16_to_double(temp), NEWLINE);
        uart_write_string(ans);
    }
}

static void cmd_init_flash_buffer(void)
{
    flash_init_write_buffer();
}

static void cmd_buffered_write(void)
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
            flash_write_byte_to_buffer((flash_index_t)address, value);
        }
        else
        {
            arg_error = true;
        }
    }
}

static void cmd_flush_buffer(void)
{
    flash_write_buffer_to_flash();
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

    sprintf(ans, "%lf%s", q16_16_to_double(control_get_k()), NEWLINE);
    uart_write_string(ans);
}

static void get_pid_ki(void)
{
    char ans[32];

    sprintf(ans, "%lf%s", q16_16_to_double(control_get_ti()), NEWLINE);
    uart_write_string(ans);
}
static void get_pid_kd(void)
{
    char ans[32];

    sprintf(ans, "%lf%s", q16_16_to_double(control_get_td()), NEWLINE);
    uart_write_string(ans);
}

static void get_pid_ttr(void)
{
    char ans[32];

    q16_16_t t_tr = (q16_16_t)flash_read_dword(FLASH_INDEX_TTR);

    sprintf(ans, "%lf%s", q16_16_to_double(t_tr), NEWLINE);
    uart_write_string(ans);
}

static void get_pid_d_max_gain(void)
{
    char ans[32];

    q16_16_t d_max_gain = (q16_16_t)flash_read_dword(FLASH_INDEX_D_MAX_GAIN);

    sprintf(ans, "%lf%s", q16_16_to_double(d_max_gain), NEWLINE);
    uart_write_string(ans);
}

static void get_pid_servo_factor(void)
{
    char ans[32];

    sprintf(ans, "%lf%s", q16_16_to_double(
            (q16_16_t)flash_read_dword(FLASH_INDEX_SERVO_FACTOR)),
            NEWLINE);
    uart_write_string(ans);
}

static void get_start_of_soak(void)
{
    char ans[32];

    if (buttons_is_profile_switch_lead())
    {
        sprintf(ans, "%us%s",
            flash_read_word(FLASH_INDEX_LEAD_SOAK_START_SEC),
            NEWLINE);
    }
    else
    {
        sprintf(ans, "%us%s",
            flash_read_word(FLASH_INDEX_LEAD_FREE_SOAK_START_SEC),
            NEWLINE);
    }

    uart_write_string(ans);
}

static void get_start_of_reflow(void)
{
    char ans[32];

    if (buttons_is_profile_switch_lead())
    {
        sprintf(ans, "%us%s",
            flash_read_word(FLASH_INDEX_LEAD_REFLOW_START_SEC),
            NEWLINE);
    }
    else
    {
        sprintf(ans, "%us%s",
            flash_read_word(FLASH_INDEX_LEAD_FREE_REFLOW_START_SEC),
            NEWLINE);
    }

    uart_write_string(ans);
}

static void get_start_of_cool(void)
{
    char ans[32];

    if (buttons_is_profile_switch_lead())
    {
        sprintf(ans, "%us%s",
            flash_read_word(FLASH_INDEX_LEAD_COOL_START_SEC),
            NEWLINE);
    }
    else
    {
        sprintf(ans, "%us%s",
            flash_read_word(FLASH_INDEX_LEAD_FREE_COOL_START_SEC),
            NEWLINE);
    }

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

        control_set_k(kp);
        
        flash_init_write_buffer();
        flash_write_dword_to_buffer(FLASH_INDEX_K, (uint32_t)kp);
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

        control_set_ti(ki);

        flash_init_write_buffer();
        flash_write_dword_to_buffer(FLASH_INDEX_TI, (uint32_t)ki);
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

        control_set_td(kd);

        flash_init_write_buffer();
        flash_write_dword_to_buffer(FLASH_INDEX_TD , (uint32_t)kd);
        flash_write_buffer_to_flash();
    }
}

static void set_pid_ttr(void)
{
    uint8_t * p;
    char arg[16] = {0};
    uint8_t i = 0;

    p = (uint8_t*)strstr(cmd_buffer, SET_PID_TTR);
    p += strlen(SET_PID_TTR);
    p += 1;     // +1 for space

    while (isdigit(*p) || ('.' == *p))
    {
        arg[i++] = *(p++);
    }

    arg[i] = NULL;

    arg_error = (0 == i);

    if (!arg_error)
    {
        double ttr_as_double = atoi(arg);
        q16_16_t factor = double_to_q16_16(ttr_as_double);

        flash_init_write_buffer();
        flash_write_dword_to_buffer(FLASH_INDEX_TTR, (uint32_t)factor);
        flash_write_buffer_to_flash();
    }
}

static void set_pid_max_d_gain(void)
{
    uint8_t * p;
    char arg[16] = {0};
    uint8_t i = 0;

    p = (uint8_t*)strstr(cmd_buffer, SET_PID_D_MAX_GAIN);
    p += strlen(SET_PID_D_MAX_GAIN);
    p += 1;     // +1 for space

    while (isdigit(*p) || ('.' == *p))
    {
        arg[i++] = *(p++);
    }

    arg[i] = NULL;

    arg_error = (0 == i);

    if (!arg_error)
    {
        double factor_as_double = atoi(arg);
        q16_16_t factor = double_to_q16_16(factor_as_double);

        flash_init_write_buffer();
        flash_write_dword_to_buffer(FLASH_INDEX_D_MAX_GAIN, (uint32_t)factor);
        flash_write_buffer_to_flash();
    }
}

static void set_pid_servo_factor(void)
{
    uint8_t * p;
    char arg[16] = {0};
    uint8_t i = 0;

    p = (uint8_t*)strstr(cmd_buffer, SET_PID_SERVO_FACTOR);
    p += strlen(SET_PID_SERVO_FACTOR);
    p += 1;     // +1 for space

    while (isdigit(*p) || ('.' == *p))
    {
        arg[i++] = *(p++);
    }

    arg[i] = NULL;

    arg_error = (0 == i);

    if (!arg_error)
    {
        double factor_as_double = -1.0 * atoi(arg);
        q16_16_t factor = double_to_q16_16(factor_as_double);

        flash_init_write_buffer();
        flash_write_dword_to_buffer(FLASH_INDEX_SERVO_FACTOR, (uint32_t)factor);
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

static void set_start_of_soak(void)
{
    uint8_t * p;
    char arg[8] = {0};

    p = (uint8_t*)strstr(cmd_buffer, SET_START_OF_SOAK);
    p += strlen(SET_START_OF_SOAK);
    p += 1;     // +1 for space

    if (!isdigit(*p))
    {
        arg_error = true;
    }
    else
    {
        uint8_t i = 0;
        uint16_t time;

        while ((i != 7) && isdigit(*p))
        {
            arg[i++] = *(p++);
        }

        arg[i] = NULL;

        time = (uint16_t)atoi(arg);

        flash_init_write_buffer();
        
        if (buttons_is_profile_switch_lead())
        {
            flash_write_word_to_buffer(FLASH_INDEX_LEAD_SOAK_START_SEC, time);
        }
        else
        {
            flash_write_word_to_buffer(
                    FLASH_INDEX_LEAD_FREE_SOAK_START_SEC, time);
        }

        flash_write_buffer_to_flash();
    }
}

static void set_start_of_reflow(void)
{
    uint8_t * p;
    char arg[8] = {0};

    p = (uint8_t*)strstr(cmd_buffer, SET_START_OF_REFLOW);
    p += strlen(SET_START_OF_REFLOW);
    p += 1;     // +1 for space

    if (!isdigit(*p))
    {
        arg_error = true;
    }
    else
    {
        uint8_t i = 0;
        uint16_t time;

        while ((i != 7) && isdigit(*p))
        {
            arg[i++] = *(p++);
        }

        arg[i] = NULL;

        time = (uint16_t)atoi(arg);

        flash_init_write_buffer();

        if (buttons_is_profile_switch_lead())
        {
            flash_write_word_to_buffer(FLASH_INDEX_LEAD_REFLOW_START_SEC, time);
        }
        else
        {
            flash_write_word_to_buffer(
                    FLASH_INDEX_LEAD_FREE_REFLOW_START_SEC, time);
        }

        flash_write_buffer_to_flash();
    }
}

static void set_start_of_cool(void)
{
    uint8_t * p;
    char arg[8] = {0};

    p = (uint8_t*)strstr(cmd_buffer, SET_START_OF_COOL);
    p += strlen(SET_START_OF_COOL);
    p += 1;     // +1 for space

    if (!isdigit(*p))
    {
        arg_error = true;
    }
    else
    {
        uint8_t i = 0;
        uint16_t time;

        while ((i != 7) && isdigit(*p))
        {
            arg[i++] = *(p++);
        }

        arg[i] = NULL;

        time = (uint16_t)atoi(arg);

        flash_init_write_buffer();

        if (buttons_is_profile_switch_lead())
        {
            flash_write_word_to_buffer(FLASH_INDEX_LEAD_COOL_START_SEC, time);
        }
        else
        {
            flash_write_word_to_buffer(
                    FLASH_INDEX_LEAD_FREE_COOL_START_SEC, time);
        }

        flash_write_buffer_to_flash();
    }
}