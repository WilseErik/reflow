#ifndef SERVO_H
#define	SERVO_H

#ifdef	__cplusplus
extern "C" {
#endif

// =============================================================================
// Include statements
// =============================================================================
#include <stdbool.h>
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

#define SERVO_MAX_POS 1200
#define SERVO_MIN_POS 0

// =============================================================================
// Public function declarations
// =============================================================================

/**
 * @brief Initializes the servo driver.
 */
void servo_init(void);

/**
 * @brief Sets the target position of the servo.
 * @param position      target position to set in [0, 1200].
 */
void servo_set_pos(uint16_t position);

/**
 * @brief Gets the current servo position.
 * @return Current servo position.
 */
uint16_t servo_get_pos(void);

#ifdef	__cplusplus
}
#endif

#endif	/* SERVO_H */

