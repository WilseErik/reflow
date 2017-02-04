// =============================================================================
// Include statements
// =============================================================================

#include "control.h"

#include <stdbool.h>
#include <stdint.h>

#include "fixed_point.h"

#include "timers.h"
#include "servo.h"
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

static const q16_16_t SAMPING_INTEVAL_SEC = DOUBLE_TO_Q16_16(0.100);

static const q16_16_t HEATER_MAX = INT_TO_Q16_16(50);
static const q16_16_t SERVO_MIN  = INT_TO_Q16_16(-50);

// =============================================================================
// Private variables
// =============================================================================

static q16_16_t K = 0;             // PID gain
static q16_16_t t_i = 0;           // Integral time constant
static q16_16_t t_d = 0;           // Dervivative time constant
static q16_16_t t_tr;              // Integral tracking time constant
static q16_16_t d_max_gain = 0;    // Max dervivative part gain

static q16_16_t reference_val = 0;
static q16_16_t integral = 0;
static q16_16_t integral_factor = 0;
static q16_16_t tracking_factor = 0;

static q16_16_t derivative = 0;
static q16_16_t last_reading = 0;
static q16_16_t ad = 0;
static q16_16_t bd = 0;

static bool servo_enabled = false;

static volatile bool initialized = false;

// =============================================================================
// Private function declarations
// =============================================================================

// =============================================================================
// Public function definitions
// =============================================================================


void control_set_k(q16_16_t kp)
{
    K = kp;
}

void control_set_ti(q16_16_t ti)
{
    t_i = ti;

    integral_factor = q16_16_multiply(K, SAMPING_INTEVAL_SEC);
    integral_factor = q16_16_divide(integral_factor, t_i);
}

void control_set_td(q16_16_t td)
{
    t_d = td;

    ad = q16_16_divide(t_d, q16_16_multiply(t_d, SAMPING_INTEVAL_SEC));
    bd = q16_16_multiply(q16_16_multiply(K, ad), d_max_gain);
}

q16_16_t control_get_k(void)
{
    return K;
}

q16_16_t control_get_ti(void)
{
    return t_i;
}

q16_16_t control_get_td(void)
{
    return t_d;
}

void control_init(void)
{
    integral = 0;
    derivative = 0;

    t_tr = (q16_16_t)flash_read_dword(FLASH_INDEX_TTR);
    tracking_factor = q16_16_divide(SAMPING_INTEVAL_SEC, t_tr);
    d_max_gain = (q16_16_t)flash_read_dword(FLASH_INDEX_D_MAX_GAIN);

    control_set_k((q16_16_t)flash_read_dword(FLASH_INDEX_K));
    control_set_ti((q16_16_t)flash_read_dword(FLASH_INDEX_TI));
    control_set_td((q16_16_t)flash_read_dword(FLASH_INDEX_TD));

    initialized = true;
}

q16_16_t control_update_pid(q16_16_t current_reading)
{
    q16_16_t ret_val = 0;

    if (initialized)
    {
        q16_16_t error;
        q16_16_t pid_result;
        q16_16_t pid_restricted_result;

        error = reference_val - current_reading;

        //
        // Calculate and low pass derivative
        //
        derivative =
            q16_16_multiply(ad, derivative) +
            q16_16_multiply(bd, current_reading - last_reading);

        //
        // Calculate PID output
        //
        pid_result = K * error + integral + derivative;

        //
        // Calculate restricted output value
        //
        if (pid_result > HEATER_MAX)
        {
            pid_restricted_result = HEATER_MAX;
        }
        else if (servo_enabled && (pid_result < SERVO_MIN))
        {
            pid_restricted_result = SERVO_MIN;
        }
        else if (!servo_enabled && (pid_result < 0))
        {
            pid_restricted_result = 0;
        }
        else
        {
            pid_restricted_result = pid_result;
        }
        
        //
        // Set heater duty and servo position
        //
        if (pid_restricted_result >= 0)
        {
            timers_set_heater_duty(q16_16_to_int(pid_restricted_result));
            servo_set_pos(0);
        }
        else
        {
            const q16_16_t SERVO_FACTOR = INT_TO_Q16_16(-24);

            timers_set_heater_duty(0);

            if (servo_enabled)
            {

                servo_set_pos(q16_16_to_int(
                    q16_16_multiply(SERVO_FACTOR, pid_restricted_result)));
            }
            else
            {
                servo_set_pos(0);
            }
        }

        //
        // Precalculate I part for next time
        //
        integral += q16_16_multiply(error, integral_factor);
        integral += q16_16_multiply(
            tracking_factor,
            pid_restricted_result - pid_result);

        //
        // Save this reading
        //
        last_reading = current_reading;
    }
    else
    {
        ret_val = 0;
    }

    return ret_val;
}

void control_set_target_value(q16_16_t target_value)
{
    reference_val = target_value;
}

void control_enable_servo(bool enable)
{
    servo_enabled = enable;
}


// =============================================================================
// Private function definitions
// =============================================================================



