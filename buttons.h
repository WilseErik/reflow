#ifndef BUTTONS_H
#define	BUTTONS_H

#ifdef	__cplusplus
extern "C" {
#endif

// =============================================================================
// Include statements
// =============================================================================
#include <stdbool.h>
    
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
 * @brief Initializes the button logic.
 * @details Requires that the gpios are initialized.
 */
void buttons_init(void);

/**
 * @brief Runs the debunce logic.
 */
void buttons_run_debounce_logic(void);

/**
 * @brief Checks if the reflow profile switch is in the 'Lead' position, not in
 *        'Lead free'.
 * @return true if in 'lead' position, false if in 'lead free' position.
 */
bool buttons_is_profile_switch_lead(void);

#ifdef	__cplusplus
}
#endif

#endif	/* BUTTONS_H */

