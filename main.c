// =============================================================================
// Include statements
// =============================================================================
#include <stdlib.h>

#include "init.h"
#include "status.h"
#include "gpio.h"

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
        if (status_check(STATUS_RUN_PID_FLAG))
        {
            status_clear(STATUS_RUN_PID_FLAG);
            // Run pid
        }
        else if (status_check(STATUS_READ_FIRST_TEMP_FLAG))
        {
            status_clear(STATUS_READ_FIRST_TEMP_FLAG);
            // Start sensor reading
        }
        else if (status_check(STATUS_READ_SECOND_TEMP_FLAG))
        {
            status_clear(STATUS_READ_SECOND_TEMP_FLAG);
            // Start sensor reading
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
    }

    return EXIT_SUCCESS;
}

// =============================================================================
// Private function definitions
// =============================================================================


