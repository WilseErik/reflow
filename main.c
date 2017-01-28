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

// =============================================================================
// Private type definitions
// =============================================================================

// =============================================================================
// Global variables
// =============================================================================

// =============================================================================
// Private constants
// =============================================================================

static const uint32_t MAX_TIME_BETWEEN_TEMP_READINGS_MS = 100;

// =============================================================================
// Private variables
// =============================================================================

// =============================================================================
// Private function declarations
// =============================================================================

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
        {
            status_clear(STATUS_STOP_BUTTON_PUSHED_FLAG);

            status_set(STATUS_REFLOW_PROGRAM_ACTIVE, false);
            timers_deactivate_heater_control();
            HEATER_OFF;
            
            buttons_stop_pushed();
        }
        //
        // Error detect
        //
        else if (status_check(STATUS_CRITICAL_ERROR_FLAG))
        {
            char msg[16] = {0};
            HEATER_OFF;

            sprintf(msg, "Crit Err %d", status_check(STATUS_CRITICAL_ERROR_FLAG));
            uart_write_string(msg);

            DEBUG_1_LED_ON;
            DEBUG_2_LED_ON;
            DEBUG_3_LED_ON;
            DEBUG_4_LED_ON;

            while (1)
            {
                HEATER_OFF;
            }
        }
#if 0
        //
        // Run control loop
        //
        else if (status_check(STATUS_RUN_PID_FLAG))
        {
            status_clear(STATUS_RUN_PID_FLAG);
            // Run pid
        }
        //
        // Start temp reading
        //
#endif
        else if (status_check(STATUS_START_TEMP_READING_FLAG))
        {
            status_clear(STATUS_START_TEMP_READING_FLAG);
            max6675_start_temp_reading();
        }
        else if (status_check(STATUS_REFLOW_TIME_UPDATED_FLAG))
        {
            uint16_t time;
            bool prog_active;

            status_clear(STATUS_REFLOW_TIME_UPDATED_FLAG);

            time = timers_get_reflow_time();
            
            prog_active = status_check(STATUS_REFLOW_PROGRAM_ACTIVE);
            led_update(time, prog_active);

            if (prog_active)
            {
                control_set_target_value(temp_curve_eval(time));
            }
        }
#if 1
        //
        // Refresh LCD screen
        //
        else if (status_check(STATUS_LCD_REFRESH_FLAG))
        {
            if (!lcd_is_busy())
            {
                static char upper_line[LCD_LINE_LEN];
                static char lower_line[LCD_LINE_LEN];
                uint16_t temp;
                uint8_t temp_decimals;
                uint16_t char_nbr;

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
                sprintf(upper_line, "T = %03u.%02u C ", temp, temp_decimals);
                upper_line[13] = ' ';

                for (char_nbr = 0; char_nbr != LCD_LINE_LEN; ++char_nbr)
                {
                    lower_line[char_nbr] = ' ';
                }

                lcd_set_text(upper_line, lower_line);
            }
        }
#endif
        //
        // Log temp over UART
        //
        else if (status_check(STATUS_UART_LOG_TEMP_FLAG))
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

                sprintf(print, "temp = %03u.%02u; time = %u\r\n",
                        temp, temp_decimals, timers_get_reflow_time());
                uart_write_string(print);
            }
        }
        //
        // Handle UART receive buffer
        //
        else if (status_check(STATUS_UART_RECEIVE_FLAG))
        {
            status_clear(STATUS_UART_RECEIVE_FLAG);
            terminal_handle_uart_event();
            DEBUG_1_LED_PIN = !DEBUG_1_LED_PIN;
        }
        //
        // Start button
        //
        else if (status_check(STATUS_START_BUTTON_PUSHED_FLAG))
        {
            status_clear(STATUS_START_BUTTON_PUSHED_FLAG);
            timers_reset_reflow_time();
            timers_activate_heater_control();
            status_set(STATUS_REFLOW_PROGRAM_ACTIVE, true);
            buttons_start_pushed();
        }
#if 0
        //
        // Detect stalls
        //
        else if (max6675_first_reading_done() &&
                (timers_get_millis() - max6675_get_last_reading_time() >
                 MAX_TIME_BETWEEN_TEMP_READINGS_MS))
        {
            status_set(STATUS_CRITICAL_ERROR_FLAG, CRIT_ERR_READ_TIMEOUT);
        }
#endif
    }

    return EXIT_SUCCESS;
}

// =============================================================================
// Private function definitions
// =============================================================================


