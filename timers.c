
// =============================================================================
// Include statements
// =============================================================================
#include "timers.h"

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

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

void timers_start_msec_timer(void)
{
    ;
}

// =============================================================================
// Private function definitions
// =============================================================================


/*
 * Implement ISR
 *
 * _____10 Hz_________________
 * - Heater PWM period
 * - Read sensors
 * - Update LCD
 *
 * _____100 Hz________________
 * - Uart log
 * 
 */