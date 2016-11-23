#ifndef MAX6675_H
#define	MAX6675_H

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
 * @brief Initializes the max6675 interfaces.
 */
void max6675_init(void);

/**
 * @brief Starts a temperature reading using the two max6675 ICs.
 * @details The reading is done asynchronously.
 */
void max6675_start_temp_reading(void);

/**
 * @brief  The current filtered temperature.
 * @return current filtered temperature in degrees celcius * 4.
 */
uint16_t max6675_get_current_temp(void);

#ifdef	__cplusplus
}
#endif

#endif	/* MAX6675_H */

