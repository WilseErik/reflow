
// =============================================================================
// Include statements
// =============================================================================
#include <stdbool.h>
#include <stdint.h>

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

static uint16_t end_of_preheat;
static uint16_t end_of_soak;
static uint16_t end_of_reflow;

// =============================================================================
// Private function declarations
// =============================================================================

// =============================================================================
// Public function definitions
// =============================================================================

void led_init(uint16_t preheat, uint16_t soak, uint16_t reflow)
{
    end_of_preheat = preheat;
    end_of_soak    = soak;
    end_of_reflow  = reflow;

    PREHEAT_LED_OFF;
    SOAK_LED_OFF;
    REFLOW_LED_OFF;
    COOL_LED_OFF;
}

void led_update(uint16_t reflow_time_sec, bool reflow_program_active)
{
    if (reflow_program_active)
    {
        if (reflow_time_sec > end_of_reflow)
        {
            COOL_LED_ON;
            
            PREHEAT_LED_OFF;
            SOAK_LED_OFF;
            REFLOW_LED_OFF;
        }
        else if (reflow_time_sec > end_of_soak)
        {
            REFLOW_LED_ON;

            PREHEAT_LED_OFF;
            SOAK_LED_OFF;
            COOL_LED_OFF;
        }
        else if (reflow_time_sec > end_of_preheat)
        {
            SOAK_LED_ON;
            
            PREHEAT_LED_OFF;
            REFLOW_LED_OFF;
            COOL_LED_OFF;
        }
        else
        {
            PREHEAT_LED_ON;
            
            SOAK_LED_OFF;
            REFLOW_LED_OFF;
            COOL_LED_OFF;
        }
    }
    else
    {
        PREHEAT_LED_OFF;
        SOAK_LED_OFF;
        REFLOW_LED_OFF;
        COOL_LED_OFF;
    }
}
// =============================================================================
// Private function definitions
// =============================================================================



