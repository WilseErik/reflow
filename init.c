// =============================================================================
// Include statements
// =============================================================================
#include "init.h"

#include <xc.h>

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

/**
 * @brief Prints basic information at startup over the UART interface.
 * @param reset_reason      The RCON register value at startup.
 */
static void print_start_message(uint16_t reset_reason);

// =============================================================================
// Public function definitions
// =============================================================================

void init(void)
{
    uint16_t reset_reason;

    CLKDIVbits.RCDIV = 0;
    reset_reason = RCON;

    gpio_init();

    uart_init();
    print_start_message(reset_reason);

    buttons_init();
    max6675_init();
    timers_start_msec_timer();
}

// =============================================================================
// Private function definitions
// =============================================================================

static void print_start_message(uint16_t reset_reason)
{
    uart_write_string("\n\r\n\r");
    uart_write_string("Reflow oven controller v1.0\n\r");
    uart_write_string("Last compiled ");
    uart_write_string(__DATE__);
    uart_write_string(", ");
    uart_write_string(__TIME__);
    uart_write_string("\n\rReset reason was:");

    if (reset_reason & _RCON_TRAPR_MASK)
    {
        RCONbits.TRAPR = 0;
        uart_write_string("\n\r\tTrap reset");
    }

    if (reset_reason & _RCON_IOPUWR_MASK)
    {
        RCONbits.IOPUWR = 0;
        uart_write_string("\n\r\tIllegal op. code");
    }

    if (reset_reason & _RCON_EXTR_MASK)
    {
        RCONbits.EXTR = 0;
        uart_write_string("\n\r\tExternal reset");
    }

    if (reset_reason & _RCON_SWR_MASK)
    {
        RCONbits.SWR = 0;
        uart_write_string("\n\r\tSoftware reset");
    }

    if (reset_reason & _RCON_WDTO_MASK)
    {
        RCONbits.WDTO = 0;
        uart_write_string("\n\r\tWatchdog timeout reset");
    }

    if (reset_reason & _RCON_BOR_MASK)
    {
        RCONbits.BOR = 0;
        uart_write_string("\n\r\tBrown out reset");
    }

    if (reset_reason & _RCON_POR_MASK)
    {
        RCONbits.POR = 0;
        uart_write_string("\n\r\tPower on reset");
    }

    uart_write_string("\n\rType 'help' for help.\n\r");
}

