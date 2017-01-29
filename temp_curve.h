#ifndef TEMP_CURVE_H
#define	TEMP_CURVE_H

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

typedef enum
{
    TEMP_CURVE_LEAD,
    TEMP_CURVE_LEAD_FREE
} temp_curve_variant_t;

typedef struct temp_curve_calib_point_t
{
    q16_16_t temp;
    uint16_t time;
} temp_curve_calib_point_t;

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
 * @brief Loads the temperature curve data from flash memory.
 * @param variant - temperature curve to load from flash memory.
 */
void temp_curve_init(temp_curve_variant_t variant);

/**
 * @brief Evaluates the temperature value at a specified time.
 * @param time - Time at to get the temperature for in seconds.
 * @return Calculated temperature.
 */
q16_16_t temp_curve_eval(uint16_t time);

/**
 * @brief Gets the time of the last data point in the temperature curve.
 * @return Time of last data point in seconds.
 */
uint16_t temp_curve_get_time_of_last_val(void);

#ifdef	__cplusplus
}
#endif

#endif	/* TEMP_CURVE_H */

