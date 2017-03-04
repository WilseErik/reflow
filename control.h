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
void control_set_k(q16_16_t k);

/**
 * @brief Sets the integral time constant of the PID regulator.
 * @param ki - integral time constant to set.
 */
void control_set_ti(q16_16_t ti);

/**
 * @brief Sets the derivate time constant of the PID regulator.
 * @param kd - derivate time constant to set.
 */
void control_set_td(q16_16_t td);

/**
 * @brief Sets the integral tracking time constant of the PID regulator.
 * @param kd - time constant to set.
 */
void control_set_ttr(q16_16_t ttr);

/**
 * @brief Gets the P part of the PID regulator.
 * @return P contant in the PID regulator.
 */
q16_16_t control_get_k(void);


/**
 * @brief Gets the I part of the PID regulator.
 * @return I contant in the PID regulator.
 */
q16_16_t control_get_ti(void);


/**
 * @brief Gets the D part of the PID regulator.
 * @return D contant in the PID regulator.
 */
q16_16_t control_get_td(void);


/**
 * @brief Inititalizes the temperature regulator.
 * @details Kp, Ki and Kd values should be set before calling this function.
 */
void control_init(void);


/**
 * @brief Updates the temperature PID controller.
 * @param current_temperature - The current temperature in the oven.
 * @return Calculated heater power to use.
 */
void control_update_pid(q16_16_t current_reading);


/**
 * @brief Sets the target value of the regulator.
 * @param target_value - new target value to regulate towards.
 */
void control_set_target_value(q16_16_t target_value);

/**
 * @breif Enables or disables the controler to use the servo for cooling.
 * @param enable - true = enabled, false = disabled.
 */
void control_enable_servo(bool enable);


#ifdef	__cplusplus
}
#endif

#endif	/* CONTROL_H */

