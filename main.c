// =============================================================================
// Include statements
// =============================================================================
#include <stdlib.h>

#include "init.h"
#include "status.h"
#include "gpio.h"
#include "buttons.h"
#include "max6675.h"

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
        if (status_check(STATUS_STOP_BUTTON_PUSHED_FLAG))
        {
            status_clear(STATUS_STOP_BUTTON_PUSHED_FLAG);
            buttons_stop_pushed();
        }
        else if (status_check(STATUS_CRITICAL_ERROR_FLAG))
        {
            status_clear(STATUS_CRITICAL_ERROR_FLAG);

            while (1)
            {
                HEATER_OFF;
            }
        }
        else if (status_check(STATUS_RUN_PID_FLAG))
        {
            status_clear(STATUS_RUN_PID_FLAG);
            // Run pid
        }
        else if (status_check(STATUS_START_TEMP_READING_FLAG))
        {
            status_clear(STATUS_START_TEMP_READING_FLAG);
            max6675_start_temp_reading();
        }
        else if (status_check(STATUS_UPDATE_LCD_FLAG))
        {
            status_clear(STATUS_UPDATE_LCD_FLAG);
            // Start update of display
        }
        else if (status_check(STATUS_UART_LOG_TEMP_FLAG))
        {
            status_clear(STATUS_UART_LOG_TEMP_FLAG);
            // Write current temp over UART
        }
        else if (status_check(STATUS_UART_RECEIVE_FLAG))
        {
            status_clear(STATUS_UART_RECEIVE_FLAG);
            DEBUG_1_LED_PIN = !DEBUG_1_LED_PIN;
        }
        else if (status_check(STATUS_START_BUTTON_PUSHED_FLAG))
        {
            status_clear(STATUS_START_BUTTON_PUSHED_FLAG);
            buttons_start_pushed();
        }
    }

    return EXIT_SUCCESS;
}

// =============================================================================
// Private function definitions
// =============================================================================


