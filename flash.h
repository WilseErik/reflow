/*
 * This file allows one to use the flash memory as non volatile data memory.
 *
 * The data memory is 1536 bytes long and resides in the instruction
 * memory space. In order to change one byte, the whole 1536 bytes sector has
 * to be erased and reprogramed. Therefore writes should be done together as
 * much as possible in order to minimize wear.
 *
 * Write operations are slow and will stall be whole processor.
 */


#ifndef FLASH_H
#define	FLASH_H

#ifdef	__cplusplus
extern "C" {
#endif

// =============================================================================
// Include statements
// =============================================================================

#include <stdbool.h>
#include <stdint.h>

#include "temp_curve.h"
    
// =============================================================================
// Public type definitions
// =============================================================================

typedef enum
{
    FLASH_INDEX_WDT_RESETS  = 0x00,

    //
    // PID Control parameters
    //
    FLASH_INDEX_K           = 0x02, // PID gain
    FLASH_INDEX_TI          = 0x06, // PID integral time contant
    FLASH_INDEX_TD          = 0x0A, // PID prediction horizon
    FLASH_INDEX_TTR         = 0x0E, // PID Integral tracking time constant
    FLASH_INDEX_D_MAX_GAIN  = 0x12, // PID N
    FLASH_INDEX_SERVO_FACTOR= 0x16, // Scaling between heater and servo output
    FLASH_INDEX_FILTER_LEN  = 0x1A, // Length of temp. moving average filter.

    //
    // Temperature curve
    //
    FLASH_INDEX_LEAD_FREE_TEMP_CURVE_SIZE   = 0x100,
    FLASH_INDEX_LEAD_FREE_SOAK_START_SEC    = 0x102,
    FLASH_INDEX_LEAD_FREE_REFLOW_START_SEC  = 0x104,
    FLASH_INDEX_LEAD_FREE_COOL_START_SEC    = 0x106,
    FLASH_INDEX_LEAD_FREE_TEMP_CURVE_START  = 0x108,
            // index 0x108 - 0x1FF reserved for lead free temp. curve data.
    FLASH_INDEX_LEAD_FREE_TEMP_CURVE_END    = 0x1FF,

    FLASH_INDEX_LEAD_TEMP_CURVE_SIZE        = 0x200,
    FLASH_INDEX_LEAD_SOAK_START_SEC         = 0x202,
    FLASH_INDEX_LEAD_REFLOW_START_SEC       = 0x204,
    FLASH_INDEX_LEAD_COOL_START_SEC         = 0x206,
    FLASH_INDEX_LEAD_TEMP_CURVE_START       = 0x208,
            // index 0x208 - 0x2FF reserved for lead free temp. curve data.
    FLASH_INDEX_LEAD_TEMP_CURVE_END         = 0x2FF,
} flash_index_t;

// =============================================================================
// Global variable declarations
// =============================================================================

// =============================================================================
// Global constatants
// =============================================================================


// Number of bytes the can be stored in the data memory.
#define FLASH_MEM_SIZE  1024

// =============================================================================
// Public function declarations
// =============================================================================

/**
 * @brief Initializes the flash by setting any missing values to their default.
 */
void flash_init(void);

/**
 * @brief Reads one byte from the flash memory.
 * @param index     Index to read from.
 * @return the read value.
 */
uint8_t flash_read_byte(flash_index_t index);

/**
 * @brief Reads one word from the flash memory.
 * @param index     Index to read from.
 * @return the read value.
 */
uint16_t flash_read_word(flash_index_t index);

/**
 * @brief Reads one double word from the flash memory.
 * @param index     Index to read from.
 * @return the read value.
 */
uint32_t flash_read_dword(flash_index_t index);

/**
 * @brief Makes a RAM copy of the flash data memory.
 */
void flash_init_write_buffer(void);

/**
 * @brief Updates one byte in the data memory buffer.
 * @param index     Index to the byte to modify.
 * @param data      The byte to write.
 */
void flash_write_byte_to_buffer(flash_index_t index, uint8_t data);

/**
 * @brief Updates one word in the data memory buffer.
 * @param index     Index to the word to modify.
 * @param data      The word to write.
 */
void flash_write_word_to_buffer(flash_index_t index, uint16_t data);

/**
 * @brief Updates one double word in the data memory buffer.
 * @param index     Index to the dword to modify.
 * @param data      The dword to write.
 */
void flash_write_dword_to_buffer(flash_index_t index, uint32_t data);

/**
 * @brief Writes the data memory buffer to the flash memory.
 * @details This function is blocking and will stall the entire cpu
 * for up to a few milliseconds.
 */
void flash_write_buffer_to_flash(void);

#ifdef	__cplusplus
}
#endif

#endif	/* FLASH_H */

