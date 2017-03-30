#ifndef PREDICTIVE_CONTROL_H
#define	PREDICTIVE_CONTROL_H

#ifdef	__cplusplus
extern "C" {
#endif

// =============================================================================
// Include statements
// =============================================================================

#include <stdbool.h>
#include <stdint.h>

#include "fixed_point.h"
#include "matrix.h"

// =============================================================================
// Public type definitions
// =============================================================================

// =============================================================================
// Global variable declarations
// =============================================================================

// =============================================================================
// Global constatants
// =============================================================================
#define PREDICTION_HORIZON (10)

// =============================================================================
// Public function declarations
// =============================================================================

/**
 * @brief Initializes the predictive control module.
 */
void predictive_control_init(void);

/**
 * @brief Updates the state of the internal model of the system.
 * @param new_reading - Sampled output of the system.
 * @param last_u - control signal from last sample.
 */
void predictive_control_update_state(q16_16_t new_reading, q16_16_t last_u);

/**
 * @brief Calculates the next output.
 */
q16_16_t predictive_control_calc_output(matrix_t * r);

#ifdef	__cplusplus
}
#endif

#endif	/* PREDICTIVE_CONTROL_H */

