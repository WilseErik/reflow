#ifndef LED_H
#define	LED_H

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

// =============================================================================
// Public function declarations
// =============================================================================

/**
 * @brief Initializes the control panel LEDs.
 * @param end_of_preheat - When the preheat period ends, in seconds since start
 *        of reflow program
 * @param end_of_soak - When the soak period ends, in seconds since start
 *        of reflow program
 * @param end_of_reflow - When the refow period ends, in seconds since start
 *        of reflow program
 */
void led_init(uint16_t end_of_preheat, uint16_t end_of_soak,
              uint16_t end_of_reflow);

/**
 * @brief Updates the LEDs according to the current time since the reflow
 *        program was started.
 * @param refow_time_sec - Time [s] since the reflow program was started.
 * @param reflow_program_active - If the reflow program is currently active.
 */
void led_update(uint16_t reflow_time_sec, bool reflow_program_active);

#ifdef	__cplusplus
}
#endif

#endif	/* LED_H */

