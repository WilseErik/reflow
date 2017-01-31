// =============================================================================
// Include statements
// =============================================================================
#include "buttons.h"

#include <stdbool.h>
#include <stdint.h>

#include "gpio.h"
#include "status.h"

// =============================================================================
// Private type definitions
// =============================================================================

typedef enum
{
    BUTTON_IS_RELEASED = 0,
    BUTTON_IS_PRESSED  = 1,
} button_state_t;

// =============================================================================
// Global variables
// =============================================================================

// =============================================================================
// Private constants
// =============================================================================

static const uint16_t PRESSED_FILTER_VAL   = 0x7FFF;
static const uint16_t RELEASED_FILTER_VAL = 0x8000;

// =============================================================================
// Private variables
// =============================================================================

static volatile uint16_t start_button_filter = 0x0000;
static volatile uint16_t stop_button_filter = 0x0000;
static volatile uint16_t lead_switch_filter = 0x0000;

static volatile button_state_t start_button_state;
static volatile button_state_t stop_button_state;
static volatile button_state_t lead_switch_state;

// =============================================================================
// Private function declarations
// =============================================================================

// =============================================================================
// Public function definitions
// =============================================================================

void buttons_init(void)
{
    start_button_state = START_BUTTON_PIN;
    stop_button_state  = STOP_BUTTON_PIN;
    lead_switch_state  = LEAD_SWITCH_PIN;

    if (start_button_state)
    {
        start_button_filter = 0xFFFF;
    }
    else
    {
        start_button_filter = 0x0000;
    }

    if (stop_button_state)
    {
        stop_button_filter = 0xFFFF;
    }
    else
    {
        stop_button_filter = 0x0000;
    }

    if (lead_switch_state)
    {
        lead_switch_filter = 0xFFFF;
    }
    else
    {
        lead_switch_filter = 0x0000;
    }
}

void buttons_run_debounce_logic(void)
{
    //
    // Start button
    //
    start_button_filter = start_button_filter << 1;

    if (START_BUTTON_PIN)
    {
        start_button_filter |= 0x0001;
    }
    else
    {
        start_button_filter &= ~0x0001;   
    }

    switch (start_button_state)
    {
        case BUTTON_IS_RELEASED:
            if (RELEASED_FILTER_VAL == start_button_filter)
            {
                status_set(STATUS_START_BUTTON_PUSHED_FLAG, true);
                start_button_state = BUTTON_IS_PRESSED;
            }
            break;

        case BUTTON_IS_PRESSED:
            if (PRESSED_FILTER_VAL == start_button_filter)
            {
                start_button_state = BUTTON_IS_RELEASED;
            }
            break;
    }

    //
    // Stop button
    //
    stop_button_filter = stop_button_filter << 1;

    if (STOP_BUTTON_PIN)
    {
        stop_button_filter |= 0x0001;
    }
    else
    {
        stop_button_filter &= ~0x0001;
    }

    switch (stop_button_state)
    {
        case BUTTON_IS_RELEASED:
            if (RELEASED_FILTER_VAL == stop_button_filter)
            {
                status_set(STATUS_STOP_BUTTON_PUSHED_FLAG, true);
                stop_button_state = BUTTON_IS_PRESSED;
            }
            break;

        case BUTTON_IS_PRESSED:
            if (PRESSED_FILTER_VAL == stop_button_filter)
            {
                stop_button_state = BUTTON_IS_RELEASED;
            }
            break;
    }

    //
    // Lead switch
    //
    lead_switch_filter = lead_switch_filter << 1;

    if (LEAD_SWITCH_PIN)
    {
        lead_switch_filter |= 0x0001;
    }
    else
    {
        lead_switch_filter &= ~0x0001;
    }

    switch (lead_switch_state)
    {
        case BUTTON_IS_RELEASED:
            if (RELEASED_FILTER_VAL == lead_switch_filter)
            {
                status_set(STATUS_SWITCH_TO_LEAD_FLAG, true);
                status_clear(STATUS_SWITCH_TO_LEAD_FREE_FLAG);
                lead_switch_state = BUTTON_IS_PRESSED;
            }
            break;

        case BUTTON_IS_PRESSED:
            if (PRESSED_FILTER_VAL == lead_switch_filter)
            {
                status_set(STATUS_SWITCH_TO_LEAD_FREE_FLAG, true);
                status_clear(STATUS_SWITCH_TO_LEAD_FLAG);
                lead_switch_state = BUTTON_IS_RELEASED;
            }
            break;
    }
}

bool buttons_is_profile_switch_lead(void)
{
    return lead_switch_state == BUTTON_IS_PRESSED;
}

// =============================================================================
// Private function definitions
// =============================================================================



