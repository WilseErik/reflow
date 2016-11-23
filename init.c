// =============================================================================
// Include statements
// =============================================================================

#include "init.h"

#include "gpio.h"
#include "uart.h"
#include "timers.h"
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

void init(void)
{
    gpio_init();
    uart_init();

    uart_write_string("Reflow oven controller v1.0\n\r");
    uart_write_string("Last compiled nov 2016\n\r");
    uart_write_string("Type 'help' for help.\n\r");

    buttons_init();
    max6675_init();
    timers_start_msec_timer();
}

// =============================================================================
// Private function definitions
// =============================================================================



