// =============================================================================
// Include statements
// =============================================================================
#include <xc.h>

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "init.h"
#include "status.h"
#include "gpio.h"
#include "buttons.h"
#include "max6675.h"
#include "timers.h"
#include "lcd.h"
#include "uart.h"
#include "terminal.h"
#include "control.h"
#include "temp_curve.h"
#include "led.h"
#include "flash.h"
#include "fixed_point.h"
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

static const uint32_t MAX_TIME_BETWEEN_TEMP_READINGS_MS = 600;

// =============================================================================
// Private variables
// =============================================================================

// =============================================================================
// Private function declarations
// =============================================================================

/**
 * @brief Handles the stop button pushed event.
 */
static inline void handle_stop_button_event(void);

/**
 * @brief Handles the critical error event.
 */
static inline void handle_critical_error_event(void);

/**
 * @brief Handles the run PID event.
 */
static inline void handle_pid_event(void);

/**
 * @brief Handles the start temperature reading event.
 */
static inline void handle_start_temp_reading_event(void);

/**
 * @brief Handles the update reflow time event.
 */
static inline void handle_reflow_time_update_event(void);

/**
 * @brief Handles the refresh LCD display event.
 */
static inline void handle_lcd_refresh_event(void);

/**
 * @brief Handles the UART log temperature event.
 */
static inline void handle_uart_log_temp_event(void);

/**
 * @brief Handles the UART character received event.
 */
static inline void handle_uart_receive_event(void);

/**
 * @brief Handles the start button pushed event.
 */
static inline void handle_start_button_event(void);

/**
 * @brief Handles the switch to lead profile event.
 */
static inline void handle_switch_to_lead_profile(void);

/**
 * @brief Handles the switch to lead free profile event.
 */
static inline void handle_switch_to_lead_free_profile(void);


// =============================================================================
// Public function definitions
// =============================================================================

int main(void)
{
    init();

    while (1)
    {
        ClrWdt();

        //
        // Stop button
        //
        if (status_check(STATUS_STOP_BUTTON_PUSHED_FLAG))
            handle_stop_button_event();
        //
        // Error detect
        //
        else if (status_check(STATUS_CRITICAL_ERROR_FLAG))
            handle_critical_error_event();
        //
        // Run control loop
        //
        else if (status_check(STATUS_RUN_PID_FLAG))
            handle_pid_event();
        //
        // Start temp reading
        //
        else if (status_check(STATUS_START_TEMP_READING_FLAG))
            handle_start_temp_reading_event();
        //
        // Update reflow time
        //
        else if (status_check(STATUS_REFLOW_TIME_UPDATED_FLAG))
            handle_reflow_time_update_event();
        //
        // Refresh LCD screen
        //
        else if (status_check(STATUS_LCD_REFRESH_FLAG))
            handle_lcd_refresh_event();
        //
        // Log temp over UART
        //
        else if (status_check(STATUS_UART_LOG_TEMP_FLAG))
            handle_uart_log_temp_event();
        //
        // Handle UART receive buffer
        //
        else if (status_check(STATUS_UART_RECEIVE_FLAG))
            handle_uart_receive_event();
        //
        // Handle switch to lead reflow profile
        //
        else if (status_check(STATUS_SWITCH_TO_LEAD_FLAG))
            handle_switch_to_lead_profile();
        //
        // Handle switch to lead free reflow profile
        //
        else if (status_check(STATUS_SWITCH_TO_LEAD_FREE_FLAG))
            handle_switch_to_lead_free_profile();
        //
        // Handle start buitton pushed
        //
        else if (status_check(STATUS_START_BUTTON_PUSHED_FLAG))
            handle_start_button_event();
        //
        // Detect stalls
        //
        else if (max6675_first_reading_done() &&
                (timers_get_millis() - max6675_get_last_reading_time() >
                 MAX_TIME_BETWEEN_TEMP_READINGS_MS))
            status_set(STATUS_CRITICAL_ERROR_FLAG, CRIT_ERR_READ_TIMEOUT);
    }

    return EXIT_SUCCESS;
}

// =============================================================================
// Private function definitions
// =============================================================================

static inline void handle_stop_button_event(void)
{
    status_clear(STATUS_STOP_BUTTON_PUSHED_FLAG);

    status_set(STATUS_REFLOW_PROGRAM_ACTIVE, false);
    timers_deactivate_heater_control();
    HEATER_OFF;

    uart_write_string("Stop button pushed\r\n");
}

static inline void handle_critical_error_event(void)
{
    char msg[17] = {0};
    
    timers_deactivate_heater_control();
    HEATER_OFF;

    sprintf(msg, "Crit Error %d", status_check(STATUS_CRITICAL_ERROR_FLAG));
    uart_write_string(msg);


    DEBUG_1_LED_ON;
    DEBUG_2_LED_ON;
    DEBUG_3_LED_ON;
    DEBUG_4_LED_ON;

    sprintf(msg, "Error code: %03u ", status_check(STATUS_CRITICAL_ERROR_FLAG));
    
    while (lcd_is_busy())
    {
        HEATER_OFF;
    }

    lcd_set_text("Critical error  ", msg);

    while (1)
    {
        HEATER_OFF;
    }
}

static inline void handle_pid_event(void)
{
    status_clear(STATUS_RUN_PID_FLAG);

    if (status_check(STATUS_REFLOW_PROGRAM_ACTIVE))
    {
        q16_16_t temp = int_to_q16_16(max6675_get_current_temp());
        q16_16_t y = control_update_pid(temp >> 2);

        if (y >= 0)
        {
            uint16_t duty;
            duty = (uint16_t)q16_16_to_int(y);

            if (duty > TIMERS_HEATER_MAX_DUTY)
            {
                duty = TIMERS_HEATER_MAX_DUTY;
            }

            timers_set_heater_duty((uint8_t)duty);
            servo_set_pos(0);
        }
        else
        {
            uint16_t start_of_cool;

            if (buttons_is_profile_switch_lead())
            {
                start_of_cool =
                        flash_read_word(FLASH_INDEX_LEAD_COOL_START_SEC);
            }
            else
            {
                start_of_cool =
                        flash_read_word(FLASH_INDEX_LEAD_FREE_COOL_START_SEC);
            }

            if (timers_get_reflow_time() < start_of_cool)
            {
                timers_set_heater_duty(0);
                servo_set_pos(0);
            }
            else
            {
                q16_16_t servo_factor =
                        (q16_16_t)flash_read_dword(FLASH_INDEX_SERVO_FACTOR);
                int16_t servo_pos;

                servo_pos = q16_16_to_int(q16_16_multiply(y, servo_factor));

                if (servo_pos > SERVO_MAX_POS)
                {
                    servo_set_pos(SERVO_MAX_POS);
                }
                else
                {
                    servo_set_pos(servo_pos);
                }
            }
        }
    }
}

static inline void handle_start_temp_reading_event(void)
{
    status_clear(STATUS_START_TEMP_READING_FLAG);
    max6675_start_temp_reading();
}

static inline void handle_reflow_time_update_event(void)
{
    uint16_t time;
    bool prog_active;

    status_clear(STATUS_REFLOW_TIME_UPDATED_FLAG);

    time = timers_get_reflow_time();
    prog_active = status_check(STATUS_REFLOW_PROGRAM_ACTIVE);

    if (prog_active && (time > temp_curve_get_time_of_last_val()))
    {
        status_clear(STATUS_REFLOW_PROGRAM_ACTIVE);
        prog_active = false;

        timers_deactivate_heater_control();
        HEATER_OFF;
    }

    
    led_update(time, prog_active);

    if (prog_active)
    {
        control_set_target_value(temp_curve_eval(time));

        if (buttons_is_profile_switch_lead())
        {
            if (time < flash_read_word(FLASH_INDEX_LEAD_SOAK_START_SEC))
            {
                status_set(STATUS_REFLOW_STATE, STATUS_REFLOW_STATE_PREHEAT);
            }
            else if (time < flash_read_word(FLASH_INDEX_LEAD_REFLOW_START_SEC))
            {
                status_set(STATUS_REFLOW_STATE, STATUS_REFLOW_STATE_SOAK);
            }
            else if (time < flash_read_word(FLASH_INDEX_LEAD_COOL_START_SEC))
            {
                status_set(STATUS_REFLOW_STATE, STATUS_REFLOW_STATE_REFLOW);
            }
            else
            {
                status_set(STATUS_REFLOW_STATE, STATUS_REFLOW_STATE_COOL);
            }
        }
        else
        {
            if (time < flash_read_word(FLASH_INDEX_LEAD_FREE_SOAK_START_SEC))
            {
                status_set(STATUS_REFLOW_STATE, STATUS_REFLOW_STATE_PREHEAT);
            }
            else if (time < flash_read_word(FLASH_INDEX_LEAD_FREE_REFLOW_START_SEC))
            {
                status_set(STATUS_REFLOW_STATE, STATUS_REFLOW_STATE_SOAK);
            }
            else if (time < flash_read_word(FLASH_INDEX_LEAD_FREE_COOL_START_SEC))
            {
                status_set(STATUS_REFLOW_STATE, STATUS_REFLOW_STATE_REFLOW);
            }
            else
            {
                status_set(STATUS_REFLOW_STATE, STATUS_REFLOW_STATE_COOL);
            }
        }
    }
}

static inline void handle_lcd_refresh_event(void)
{
    if (!lcd_is_busy())
    {
        char upper_line[LCD_LINE_LEN + 1];
        char lower_line[LCD_LINE_LEN + 1];
        uint16_t temp;
        uint8_t temp_decimals;
        uint8_t minutes = 0;
        uint8_t seconds = 0;
        char reflow_profile[12];

        temp = max6675_get_current_temp();

        switch (temp & 0x0003)
        {
            case 0:
                temp_decimals = 0;
                break;
            case 1:
                temp_decimals = 25;
                break;
            case 2:
                temp_decimals = 50;
                break;
            case 3:
                temp_decimals = 75;
                break;
        }

        temp = temp >> 2;

        status_clear(STATUS_LCD_REFRESH_FLAG);
        
        if (status_check(STATUS_REFLOW_PROGRAM_ACTIVE))
        {
            uint16_t time_left = temp_curve_get_time_of_last_val();
            time_left -= timers_get_reflow_time();

            seconds = time_left % 60;
            minutes = time_left / 60;
        }
        else
        {
            minutes = 0;
            seconds = 0;
        }

        sprintf(upper_line, "%03u.%02u C   %02u:%02u ",
                temp, temp_decimals, minutes, seconds);

        if (buttons_is_profile_switch_lead())
        {
            sprintf(reflow_profile, "    Pb-free");
        }
        else
        {
            sprintf(reflow_profile, "60/40 Sn-Pb");
        }

        if (status_check(STATUS_REFLOW_PROGRAM_ACTIVE))
        {
            switch (status_check(STATUS_REFLOW_STATE))
            {
            case STATUS_REFLOW_STATE_PREHEAT:
                sprintf(lower_line, "Preheating      ");
                break;

            case STATUS_REFLOW_STATE_SOAK:
                sprintf(lower_line, "Soaking         ");
                break;

            case STATUS_REFLOW_STATE_REFLOW:
                sprintf(lower_line, "Reflowing       ");
                break;

            case STATUS_REFLOW_STATE_COOL:
                sprintf(lower_line, "Cooling         ");
                break;
            }
        }
        else
        {
            sprintf(lower_line, "Idle %s", reflow_profile);
        }

        lcd_set_text(upper_line, lower_line);
    }
}

static inline void handle_uart_log_temp_event(void)
{
    char print[32];
    uint16_t temp;
    uint8_t temp_decimals;
    static bool not_first_time;

    status_clear(STATUS_UART_LOG_TEMP_FLAG);

    if (status_check(STATUS_REFLOW_PROGRAM_ACTIVE))
    {
        temp = max6675_get_current_temp();

        switch (temp & 0x0003)
        {
            case 0:
                temp_decimals = 0;
                break;
            case 1:
                temp_decimals = 25;
                break;
            case 2:
                temp_decimals = 50;
                break;
            case 3:
                temp_decimals = 75;
                break;
        }

        temp = temp >> 2;

        if (!not_first_time)
        {
            not_first_time = true;
            uart_write_string("\n\rtemperature;time\r\n");
        }

        sprintf(print, "%03u.%02u;%u\r\n",
                temp, temp_decimals, timers_get_reflow_time());
        uart_write_string(print);
    }
}

static inline void handle_uart_receive_event(void)
{
    status_clear(STATUS_UART_RECEIVE_FLAG);
    terminal_handle_uart_event();
    DEBUG_1_LED_PIN = !DEBUG_1_LED_PIN;
}

static inline void handle_start_button_event(void)
{
    status_clear(STATUS_START_BUTTON_PUSHED_FLAG);
    timers_reset_reflow_time();
    timers_activate_heater_control();
    status_set(STATUS_REFLOW_PROGRAM_ACTIVE, true);

    uart_write_string("Start button pushed\r\n");
}

static inline void handle_switch_to_lead_profile(void)
{
    status_clear(STATUS_SWITCH_TO_LEAD_FLAG);

    led_init(flash_read_word(FLASH_INDEX_LEAD_SOAK_START_SEC),
             flash_read_word(FLASH_INDEX_LEAD_REFLOW_START_SEC),
             flash_read_word(FLASH_INDEX_LEAD_COOL_START_SEC));

    temp_curve_init(TEMP_CURVE_LEAD);

    uart_write_string("Switch to lead profile\r\n");
}

static inline void handle_switch_to_lead_free_profile(void)
{
    status_clear(STATUS_SWITCH_TO_LEAD_FREE_FLAG);

    led_init(flash_read_word(FLASH_INDEX_LEAD_FREE_SOAK_START_SEC),
             flash_read_word(FLASH_INDEX_LEAD_FREE_REFLOW_START_SEC),
             flash_read_word(FLASH_INDEX_LEAD_FREE_COOL_START_SEC));

    temp_curve_init(TEMP_CURVE_LEAD_FREE);

    uart_write_string("Switch to lead free profile\r\n");
}