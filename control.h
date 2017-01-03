/**
 * This unit contains the PID regulator.
 */

#ifndef CONTROL_H
#define	CONTROL_H

#ifdef	__cplusplus
extern "C" {
#endif

// =============================================================================
// Include statements
// =============================================================================

#include <stdbool.h>
#include <stdint.h>

#include "fixed_point.h"

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
 * @brief Sets the proportional part of the PID regulator.
 * @param kp - proportional part to set.
 */
void control_set_kp(q16_16_t kp);

/**
 * @brief Sets the integral part of the PID regulator.
 * @param ki - integral part to set.
 */
void control_set_ki(q16_16_t ki);

/**
 * @brief Sets the derivate part of the PID regulator.
 * @param kd - derivate part to set.
 */
void control_set_kd(q16_16_t kd);

/**
 * @brief Gets the P part of the PID regulator.
 * @return P contant in the PID regulator.
 */
q16_16_t control_get_kp(void);


/**
 * @brief Gets the I part of the PID regulator.
 * @return I contant in the PID regulator.
 */
q16_16_t control_get_ki(void);


/**
 * @brief Gets the D part of the PID regulator.
 * @return D contant in the PID regulator.
 */
q16_16_t control_get_kd(void);


/**
 * @brief Inititalizes the temperature regulator.
 * @details Kp, Ki and Kd values should be set before calling this function.
 */
void control_start(void);


/**
 * @brief Updates the temperature PID controller.
 * @param current_temperature - The current temperature in the oven.
 * @return Calculated heater power to use.
 */
q16_16_t control_update_pid(q16_16_t current_reading);


/**
 * @brief Sets the target value of the regulator.
 * @param target_value - new target value to regulate towards.
 */
void control_set_target_value(q16_16_t target_value);


#ifdef	__cplusplus
}
#endif

#endif	/* CONTROL_H */

