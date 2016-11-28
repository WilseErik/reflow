// =============================================================================
// Include statements
// =============================================================================
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
        //
        // Stop button
        //
        if (status_check(STATUS_STOP_BUTTON_PUSHED_FLAG))
        {
            status_clear(STATUS_STOP_BUTTON_PUSHED_FLAG);
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
        else if (status_check(STATUS_START_TEMP_READING_FLAG))
        {
            status_clear(STATUS_START_TEMP_READING_FLAG);
            max6675_start_temp_reading();
        }
        //
        // Refresh LCD screen
        //
        else if (status_check(STATUS_LCD_REFRESH_FLAG))
        {
            if (!lcd_is_busy())
            {
                status_clear(STATUS_LCD_REFRESH_FLAG);
                lcd_refresh();
            }
        }
        //
        // Log temp over UART
        //
        else if (status_check(STATUS_UART_LOG_TEMP_FLAG))
        {
            status_clear(STATUS_UART_LOG_TEMP_FLAG);
            // Write current temp over UART
        }
        //
        // Handle UART receive buffer
        //
        else if (status_check(STATUS_UART_RECEIVE_FLAG))
        {
            status_clear(STATUS_UART_RECEIVE_FLAG);
            DEBUG_1_LED_PIN = !DEBUG_1_LED_PIN;
        }
        //
        // Start button
        //
        else if (status_check(STATUS_START_BUTTON_PUSHED_FLAG))
        {
            status_clear(STATUS_START_BUTTON_PUSHED_FLAG);
            buttons_start_pushed();
        }
        //
        // Detect stalls
        //
        else if (max6675_first_reading_done() &&
                (timers_get_millis() - max6675_get_last_reading_time() >
                 MAX_TIME_BETWEEN_TEMP_READINGS_MS))
        {
            status_set(STATUS_CRITICAL_ERROR_FLAG, CRIT_ERR_READ_TIMEOUT);
        }
    }

    return EXIT_SUCCESS;
}

// =============================================================================
// Private function definitions
// =============================================================================


