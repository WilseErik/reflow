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

#ifdef	__cplusplus
}
#endif

#endif	/* TIMERS_H */

