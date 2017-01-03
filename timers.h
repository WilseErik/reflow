#ifndef TIMERS_H
#define	TIMERS_H

#ifdef	__cplusplus
extern "C" {
#endif

// =============================================================================
// Include statements
// =============================================================================

#include <stdint.h>

// =============================================================================
// Public type definitions
// =============================================================================

// =============================================================================
// Global variable declarations
// =============================================================================

// =============================================================================
// Global constatants
// =============================================================================

// =============================================================================
// Public function declarations
// =============================================================================

/**
 * @brief Starts the 1000Hz timer.
 * @details This timer is responsible for task scheduling and general
 *          time keeping.
 */
void timers_start_msec_timer(void);

/**
 * @brief Gets the current timestamp in ms.
 */
uint32_t timers_get_millis(void);

/**
 * @brief Activates the heater pwm control.
 */
void timers_activate_heater_control(void);

/**
 * @breif Deactivates the heater element pwm control and turns off the heater.
 */
void timers_deactivate_heater_control(void);

/**
 * @bruef Sets the duty cycle of the heater element.
 * @param duty - Duty cycle in range [0, 50].
 */
void timers_set_heater_duty(uint8_t duty);

#ifdef	__cplusplus
}
#endif

#endif	/* TIMERS_H */

