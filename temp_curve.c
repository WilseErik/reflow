/*
 * This file calculates the target temperature for a specific point in time
 * into the reflow process.
 *
 * The temperature curve is stored as a lookup table made of calibration points.
 * Linear interpolation is done between calibration points.
 *
 * Temp
 * ^
 * |
 * |                                xx
 * |                               x   x
 * |                             x       x
 * |                           x            x
 * |                        x                   x
 * |                     x
 * |           x                                        x
 * | x
 *  --------------------------------------------------------------> time
 *
 *
 */



// =============================================================================
// Include statements
// =============================================================================

#include "temp_curve.h"

#include <stdbool.h>
#include <stdint.h>

#include "fixed_point.h"
#include "flash.h"

// =============================================================================
// Private type definitions
// =============================================================================

// =============================================================================
// Global variables
// =============================================================================

// =============================================================================
// Private constants
// =============================================================================

#define LOOKUP_TABLE_LENGHT ((FLASH_INDEX_LEAD_FREE_TEMP_CURVE_END -           \
                              FLASH_INDEX_LEAD_FREE_TEMP_CURVE_START)          \
                             / sizeof(temp_curve_calib_point_t))
// =============================================================================
// Private variables
// =============================================================================

static temp_curve_calib_point_t lookup_table[LOOKUP_TABLE_LENGHT];
static uint8_t nbr_of_calib_points = 0;

// =============================================================================
// Private function declarations
// =============================================================================

// =============================================================================
// Public function definitions
// =============================================================================

void temp_curve_init(temp_curve_variant_t variant)
{
    uint16_t flash_index;
    uint8_t i;

    switch (variant)
    {
        case TEMP_CURVE_LEAD_FREE:
            nbr_of_calib_points =
                    flash_read_byte(FLASH_INDEX_LEAD_FREE_TEMP_CURVE_SIZE);
            flash_index = FLASH_INDEX_LEAD_FREE_TEMP_CURVE_START;
            break;

        case TEMP_CURVE_LEAD:
            nbr_of_calib_points =
                    flash_read_byte(FLASH_INDEX_LEAD_TEMP_CURVE_SIZE);
            flash_index = FLASH_INDEX_LEAD_TEMP_CURVE_START;
            break;

        default:
            break;
    }

    for (i = 0; i != nbr_of_calib_points; ++i)
    {
        lookup_table[i].temp = (q16_16_t)flash_read_word(flash_index);
        lookup_table[i].time = flash_read_dword(flash_index + sizeof(q16_16_t));

        flash_index += sizeof(temp_curve_calib_point_t);
    }
}

q16_16_t temp_curve_eval(uint16_t time)
{
    q16_16_t ret_val;
    uint8_t i;

    i = 0;

    while ((i != nbr_of_calib_points) && (lookup_table[i].time < time))
    {
        ++i;
    }

    if (0 == i)
    {
        ret_val = lookup_table[i].temp;
    }
    else if ((nbr_of_calib_points == i) && lookup_table[i - 1].time < time)
    {
        ret_val = lookup_table[nbr_of_calib_points - 1].temp;
    }
    else
    {
        /* Let's interpolate between lookup_table[i-1] and lookup_table[i]
         *
         *  y axis - temp
         *      ^
         *      |
         *  y2  |                 x
         *      |
         *  y1  |    x
         *      |
         *      ------------------------> x axis - time
         *           x1    /\     x2
         *                 ||
         *             find temp
         *             at this point
         */
        q16_16_t x1;
        q16_16_t x2;
        q16_16_t y1;
        q16_16_t y2;

        q16_16_t derivate;

        x1 = int_to_q16_16(lookup_table[i - 1].time);
        x2 = int_to_q16_16(lookup_table[i    ].time);
        y1 = lookup_table[i - 1].temp;
        y2 = lookup_table[i    ].temp;

        derivate = q16_16_divide((y2 - y1), (x2 - x1));

        ret_val = y1 + q16_16_multiply(derivate, time - x1);
    }

    return ret_val;
}

// =============================================================================
// Private function definitions
// =============================================================================



