#ifndef BUTTONS_H
#define	BUTTONS_H

#ifdef	__cplusplus
extern "C" {
#endif

// =============================================================================
// Include statements
// =============================================================================

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
 * @brief The code to run whenever the start button has been pushed.
 */
void buttons_start_pushed(void);

/**
 * @brief The code to run whenever the stop button has been pushed.
 */
void buttons_stop_pushed(void);

#ifdef	__cplusplus
}
#endif

#endif	/* BUTTONS_H */

