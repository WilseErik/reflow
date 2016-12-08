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

/**
 * @brief Gets the timestamp of when the last reading was completed.
 * @return timestamp of last reading completion timestamp.
 */
uint32_t max6675_get_last_reading_time(void);

/**
 * @brief Checks if the first reading since startup has been made.
 * @details If a reading has been made there is a valid last reading time.
 * @return true if at least one reading as been done since startup.
 */
bool max6675_first_reading_done(void);

/**
 * @brief Performs a blocking read of one of the MAX6675 ICs.
 * @return Raw read 16bit value.
 */
uint16_t max6675_read_blocking(void);

#ifdef	__cplusplus
}
#endif

#endif	/* MAX6675_H */

