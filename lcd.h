/**
 * This file handles the LCD character display.
 * In order to keep track of time, this file uses the timer 4 module.
 */

#ifndef LCD_H
#define	LCD_H

#ifdef	__cplusplus
extern "C" {
#endif

// =============================================================================
// Include statements
// =============================================================================

// =============================================================================
// Public type definitions
// =============================================================================

typedef enum
{
    LCD_STATE_IDLE,
    LCD_STATE_OVEN_PREHEAT,
    LCD_STATE_OVEN_SOAK,
    LCD_STATE_OVEN_REFLOW,
    LCD_STATE_OVEN_COOL,
    LCD_STATE_POST_COOL,
    LCD_STATE_SHOW_ERROR
} lcd_state_t;

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
 * @brief Initializes the LCD display.
 */
void lcd_init(void);

/**
 * @brief Refreshes the LCD display.
 */
void lcd_refresh(void);

/**
 * @brief Changes the state of the LCD display.
 * @param new_state - state to set.
 */
void lcd_set_state(lcd_state_t new_state);

#ifdef	__cplusplus
}
#endif

#endif	/* LCD_H */

