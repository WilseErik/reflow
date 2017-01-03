// =============================================================================
// Include statements
// =============================================================================

#include "control.h"

#include <stdbool.h>
#include <stdint.h>

#include "fixed_point.h"

// =============================================================================
// Private type definitions
// =============================================================================

// =============================================================================
// Global variables
// =============================================================================

// =============================================================================
// Private constants
// =============================================================================

#define INTEGRAL_SCALE_FACTOR           0.01

#define DERIVATE_FILTER_OLD_VAL_FACTOR  0.5
#define DERIVATE_FILTER_NEW_VAL_FACTOR  (1.0 - DERIVATE_FILTER_OLD_VAL_FACTOR)

static const q16_16_t integral_scale_factor =
    DOUBLE_TO_Q16_16(INTEGRAL_SCALE_FACTOR);

static const q16_16_t derivate_factor_old_val_filter =
    DOUBLE_TO_Q16_16(DERIVATE_FILTER_OLD_VAL_FACTOR);
static const q16_16_t derivate_factor_new_val_filter =
    DOUBLE_TO_Q16_16(DERIVATE_FILTER_NEW_VAL_FACTOR);

// =============================================================================
// Private variables
// =============================================================================

static volatile q16_16_t p = 0;
static volatile q16_16_t i = 0;
static volatile q16_16_t d = 0;

static volatile q16_16_t reference_val = 0;
static volatile q16_16_t integral = 0;

static volatile q16_16_t derivate = 0;
static volatile q16_16_t last_reading = 0;

static volatile bool initialized = false;

// =============================================================================
// Private function declarations
// =============================================================================

// =============================================================================
// Public function definitions
// =============================================================================


void control_set_kp(q16_16_t kp)
{
    p = kp;
}

void control_set_ki(q16_16_t ki)
{
    i = ki;
}

void control_set_kd(q16_16_t kd)
{
    d = kd;
}

q16_16_t control_get_kp(void)
{
    return p;
}

q16_16_t control_get_ki(void)
{
    return i;
}

q16_16_t control_get_kd(void)
{
    return d;
}

void control_start(void)
{
    integral = 0;
    derivate = 0;
    
    initialized = true;
}

q16_16_t control_update_pid(q16_16_t current_reading)
{
    q16_16_t ret_val = 0;

    if (initialized)
    {
        q16_16_t p_part;
        q16_16_t i_part;
        q16_16_t d_part;

        q16_16_t current_derivate;
        q16_16_t error = reference_val - current_reading;

        integral += q16_16_multiply(integral_scale_factor, error);
        current_derivate = current_reading - last_reading;
        derivate =
            q16_16_multiply(derivate_factor_old_val_filter, derivate) +
            q16_16_multiply(derivate_factor_new_val_filter, current_reading);

        p_part = q16_16_multiply(p, error);
        i_part = q16_16_multiply(i, integral);
        d_part = q16_16_multiply(d, derivate);

        ret_val = p_part + i_part - d_part;
    }
    else
    {
        ret_val = 0;
    }

    last_reading = current_reading;

    return ret_val;
}

void control_set_target_temperature(q16_16_t target_value)
{
    reference_val = target_value;
}


// =============================================================================
// Private function definitions
// =============================================================================



