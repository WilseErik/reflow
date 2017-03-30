/*
 * This regulator is based on Model Predictive Control (MPC),
 * using the Interior Point Method for finding the optimal control values.
 *
 * The algorithm forms a cost function for a set of future regulator outputs.
 * This cost is based on the deviation from the desired temperature curve which
 * occours when these regulator outputs are used.
 * The cost function is then minimized in order to find the optimal temperature
 * trajectory, by the use of the Interior Point Method. The Interior Point
 * Method is adjusted to use a quasi-newton method which estimates the inverse
 * of the hessian of the cost function in order to decrease computational
 * time.
 */



// =============================================================================
// Include statements
// =============================================================================
#include "predictive_control.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "fixed_point.h"
#include "matrix.h"

// =============================================================================
// Private type definitions
// =============================================================================

// =============================================================================
// Global variables
// =============================================================================

// =============================================================================
// Private constants
// =============================================================================

// =============================================================================
// Private variables
// =============================================================================

/*
 *       Mathematical model of oven:
 *
 *
 *       XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 *       XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 *       XXXX                                               XXXX
 *       XXXX===============================================XXXX
 *       XXXX    ^              + Th                        XXXX
 *       XXXX    |           Temperature of                 XXXX
 *       XXXX  Heater        heater                    Tw + XXXX
 *       XXXX                                   Temperature XXXX
 *       XXXX                    + Ta           of wall     XXXX
 *       XXXX                Temperature of                 XXXX
 *       XXXX                thermocouple                   XXXX
 *       XXXX                                               XXXX
 *       XXXX                                               XXXX
 *       XXXX                                               XXXX
 *       XXXX===============================================XXXX
 *       XXXX                                               XXXX
 *       XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 *       XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 *
 *      Let Ta, Th and Tw be relative to the temperature outside of the oven.
 */

////////////////////////////////////////////////////////////
//      System matricies
////////////////////////////////////////////////////////////
#define NBR_OF_STATES 3

// System can be written on the form using state space notation:
//      x_k+1 = A*x_k + B*u
//      y = C*x
//
MATRIX_DECLARE_STATIC(A, NBR_OF_STATES, NBR_OF_STATES);
MATRIX_DECLARE_STATIC(B, NBR_OF_STATES, 1);
MATRIX_DECLARE_STATIC(C, 1, NBR_OF_STATES);

////////////////////////////////////////////////////////////
//      Observer matricies
////////////////////////////////////////////////////////////

// Use an observer to estimate the state:
//
// x_est_k+1 = A*x_est_k + B*u + K(y - y_est)
// y_est = C*x_est
//
MATRIX_DECLARE_STATIC(K, NBR_OF_STATES, 1);
MATRIX_DECLARE_STATIC(x_est, NBR_OF_STATES, 1);

// Precalculate K*C and A - KC since the observer can be written as:
//
// x_est_k+1 = (A - KC)x_est_k + B*u + Ky
// y_est = C*x_est
//
MATRIX_DECLARE_STATIC(KC, NBR_OF_STATES, NBR_OF_STATES);
MATRIX_DECLARE_STATIC(A_minus_KC, NBR_OF_STATES, NBR_OF_STATES);

////////////////////////////////////////////////////////////
//      MPC parameters
////////////////////////////////////////////////////////////

// Constraints
#define U_MAX INT_TO_Q16_16(50)
#define U_MIN INT_TO_Q16_16(0)
#define U_MID INT_TO_Q16_16(25)

MATRIX_DECLARE_STATIC(u_optimal, PREDICTION_HORIZON, 1);

// =============================================================================
// Private function declarations
// =============================================================================

//
// Set up matricies
//
static void construct_a_matrix(void);
static void construct_b_matrix(void);
static void construct_c_matrix(void);
static void construct_k_matrix(void);
static void construct_x_est_matrix(void);

static void calc_next_state_estimate(q16_16_t current_temp, q16_16_t heater);

/**
 * @brief Calculates the cost of a given set of future regulator outputs.
 * @param x - Starting state.
 * @param u_future - List of future inputs
 * @param r_future - List of reference values
 * @return
 */
static q16_16_t cost_function(const matrix_t * x,
                              const matrix_t * u_future,
                              const matrix_t * r_future);

/**
 * @brief Calculates the gradient of the cost function.
 * @param gradient - Matrix to store the gradient in.
 * @param x - Current system state.
 * @param u - Future system inputs/regulator outputs.
 * @param r - Future reference values.
 */
static void find_gradient(matrix_t * gradient,
                          const matrix_t * x,
                          const matrix_t * u,
                          const matrix_t * r);

/**
 * @brief Modifies the gradient to take system constraints into account.
 * @details Adds barrier terms to the cost function on the form log(u[i]-umax).
 * @param gradient - Gradient matrix to modify.
 * @param u - System inputs.
 * @param t - How much the barrier term should be scaled.
 */
static void add_constraint_barrier(matrix_t * gradient,
                                   const matrix_t * u,
                                   const q16_16_t t);

/**
 * @brief Finds the u that minimizes the cost function.
 * @details This function uses a quasi-Newton method to find where the gradient
 * of the cost function is zero. By using the Davidon?Fletcher?Powell formula,
 * the inverse of the hessian can be approximated, so that one does not have
 * to calculate the hessian and then find the left inverse of it.
 * https://en.wikipedia.org/wiki/Quasi-Newton_method
 * https://en.wikipedia.org/wiki/Davidon%E2%80%93Fletcher%E2%80%93Powell_formula
 *
 * @param u_optimal - Matrix to store the u that minimizes the cost function.
 * @param t - Barrier function scaling.
 * @param x - Start system state.
 * @param u - Starting point for finding u_optimal from.
 * @param r - Desired system outputs.
 * @param hessian_inv - Approximation of the hessian inverse of the cost
 * function.
 */
static void minimize_cost(matrix_t * u_optimal,
                          const q16_16_t t,
                          const matrix_t * x,
                          const matrix_t * u,
                          const matrix_t * r,
                          matrix_t * hessian_inv);

/**
 * @brief Finds the optimal set of future regulator outputs for minimizing the
 * regulation error.
 * @details This function uses the barrier variant of the
 * interior point method to minimize the cost function given constraints on u.
 * In this way the constrained quadratic minimization problem of minimizing the
 * cost function can be turned into a set of non-constrained non-linear
 * minimization problems, which can be solved using a quasi-newton method.
 * https://en.wikipedia.org/wiki/Interior_point_method
 *
 * @param u_optimal - Matrix to store the optimal regulator outputs in.
 * @param x - System state.
 * @param r - Desired system outputs.
 */
static void find_optimal_u(matrix_t * u_optimal,
                           const matrix_t * x,
                           const matrix_t * r);

/**
 * @brief Checks if all elements e, in u fullfill: u_min <= e <= u_max
 * @param u Matrix to check
 * @param u_min Lower bound
 * @param u_max Upper bound
 * @return True if all elements in u are within bounds.
 */
static bool is_u_within_constraints(const matrix_t * u,
                                    const q16_16_t u_min,
                                    const q16_16_t u_max);

// =============================================================================
// Public function definitions
// =============================================================================

void predictive_control_init(void)
{
    construct_a_matrix();
    construct_b_matrix();
    construct_c_matrix();
    construct_k_matrix();
    construct_x_est_matrix();

    MATRIX_CREATE(KC, NBR_OF_STATES, NBR_OF_STATES);
    matrix_mult(&K, &C, &KC);

    MATRIX_CREATE(A_minus_KC, NBR_OF_STATES, NBR_OF_STATES);
    matrix_diff(&A, &KC, &A_minus_KC);

    MATRIX_CREATE(u_optimal, PREDICTION_HORIZON, 1);
    matrix_zero(&u_optimal);
}

void predictive_control_update_state(q16_16_t new_reading, q16_16_t last_u)
{
    calc_next_state_estimate(new_reading, last_u);
}

q16_16_t predictive_control_calc_output(matrix_t * r)
{
    find_optimal_u(&u_optimal, &x_est, r);

    return *matrix_at(&u_optimal, 1, 1);
}

// =============================================================================
// Private function definitions
// =============================================================================

static void construct_a_matrix(void)
{
    MATRIX_CREATE(A, NBR_OF_STATES, NBR_OF_STATES);
    
    *matrix_at(&A, 0, 0) = DOUBLE_TO_Q16_16(1.935992437676738);
    *matrix_at(&A, 1, 0) = DOUBLE_TO_Q16_16(1.000000000000000);
    *matrix_at(&A, 2, 0) = DOUBLE_TO_Q16_16(0);

    *matrix_at(&A, 0, 1) = DOUBLE_TO_Q16_16(-0.940877922422651);
    *matrix_at(&A, 1, 1) = DOUBLE_TO_Q16_16(0);
    *matrix_at(&A, 2, 1) = DOUBLE_TO_Q16_16(1.000000000000000);

    *matrix_at(&A, 0, 2) = DOUBLE_TO_Q16_16(0);
    *matrix_at(&A, 1, 2) = DOUBLE_TO_Q16_16(0);
    *matrix_at(&A, 2, 2) = DOUBLE_TO_Q16_16(0);
}

static void construct_b_matrix(void)
{
    MATRIX_CREATE(B, NBR_OF_STATES, 1);

    *matrix_at(&B, 0, 0) = DOUBLE_TO_Q16_16(0.125000000000000);
    *matrix_at(&B, 1, 0) = DOUBLE_TO_Q16_16(0);
    *matrix_at(&B, 2, 0) = DOUBLE_TO_Q16_16(0);
}

static void construct_c_matrix(void)
{
    MATRIX_CREATE(C, 1, NBR_OF_STATES);
    
    *matrix_at(&C, 0, 0) = DOUBLE_TO_Q16_16(0.028071256717781);
    *matrix_at(&C, 0, 1) = DOUBLE_TO_Q16_16(0.182757700635478);
    *matrix_at(&C, 0, 2) = DOUBLE_TO_Q16_16(-0.036746593971083);
}

static void construct_k_matrix(void)
{
    MATRIX_CREATE(K, NBR_OF_STATES, 1);
    
    *matrix_at(&K, 0, 0) = 1;
    *matrix_at(&K, 1, 0) = 1;
    *matrix_at(&K, 2, 0) = 1;
}

static void construct_x_est_matrix(void)
{
    MATRIX_CREATE(x_est, NBR_OF_STATES, 1);
    
    *matrix_at(&x_est, 0, 0) = 0;
    *matrix_at(&x_est, 1, 0) = 0;
    *matrix_at(&x_est, 2, 0) = 0;
}

static void calc_next_state_estimate(q16_16_t current_temp, q16_16_t heater)
{
    MATRIX_DECLARE_AND_CREATE(m1, NBR_OF_STATES, NBR_OF_STATES);
    MATRIX_DECLARE_AND_CREATE(next_x_est, NBR_OF_STATES, 1);
    MATRIX_DECLARE_AND_CREATE(m2, NBR_OF_STATES, 1);

    matrix_mult(&A_minus_KC, &x_est, &m1);
    matrix_mult(&m1, &x_est, &next_x_est);

    matrix_mult_elements(&B, heater, &m2);
    matrix_add(&next_x_est, &m2, &next_x_est);

    matrix_mult_elements(&K, current_temp, &m2);
    matrix_add(&next_x_est, &m2, &next_x_est);

    matrix_copy(&next_x_est, &x_est);
}

static q16_16_t cost_function(const matrix_t * x,
                              const matrix_t * u_future,
                              const matrix_t * r_future)
{
    q16_16_t cost = 0;

    uint8_t i;

    MATRIX_DECLARE(x_sim, NBR_OF_STATES, 1);
    MATRIX_DECLARE(x_sim_next, NBR_OF_STATES, 1);
    MATRIX_DECLARE(temp, NBR_OF_STATES, 1);
    MATRIX_DECLARE(y_sim, 1, 1);

    MATRIX_CREATE(x_sim, NBR_OF_STATES, 1);
    MATRIX_CREATE(x_sim_next, NBR_OF_STATES, 1);
    MATRIX_CREATE(temp, NBR_OF_STATES, 1);
    MATRIX_CREATE(y_sim, 1, 1);

    matrix_copy(x, &x_sim);

    for (i = 0; i != PREDICTION_HORIZON; ++i)
    {
        q16_16_t error_term;
        q16_16_t u_value;

        // x = Ax
        matrix_mult(&A, &x_sim, &x_sim_next);

        // x += Bu
        u_value = *matrix_at(u_future, i, 0);
        matrix_mult_elements(&B, u_value, &temp);
        matrix_add(&x_sim_next, &temp, &x_sim_next);

        // y = Cx
        matrix_mult(&C, &x_sim_next, &y_sim);

        // error = (r - y)^2
        error_term = *matrix_at(r_future, 0, i) - *matrix_at(&y_sim, 0, 0);
        error_term = error_term * error_term;

        // cost += error
        cost += error_term;
        
        matrix_copy(&x_sim_next, &x_sim);
    }

    return cost;
}

static void find_gradient(matrix_t * gradient,
                          const matrix_t * x,
                          const matrix_t * u,
                          const matrix_t * r)
{
    q16_16_t step_size = DOUBLE_TO_Q16_16(0.01);
    uint16_t row = 0;
    MATRIX_DECLARE(step, PREDICTION_HORIZON, 1);
    MATRIX_DECLARE(u_plus_step, PREDICTION_HORIZON, 1);

    MATRIX_CREATE(step, PREDICTION_HORIZON, 1);
    matrix_zero(&step);

    MATRIX_CREATE(u_plus_step, PREDICTION_HORIZON, 1);
    matrix_copy(u, &u_plus_step);

    for (row = 0; row != PREDICTION_HORIZON; ++row)
    {
        *matrix_at(&step, row, 0) = step_size;

        matrix_add(u, &step, &u_plus_step);
        *matrix_at(gradient, row, 0) =
                q16_16_divide(
                (cost_function(x, &u_plus_step, r) - cost_function(x, u, r)),
                step_size);

        *matrix_at(&step, row, 0) = 0;
    }
}

static void add_constraint_barrier(matrix_t * gradient,
                                   const matrix_t * u,
                                   const q16_16_t t)
{
    uint16_t row;

    for (row = 0; row != PREDICTION_HORIZON; ++row)
    {
        *matrix_at(gradient, row, 0) += 
                q16_16_divide(-t, *matrix_at(u, row, 0) - U_MIN);
        *matrix_at(gradient, row, 0) +=
                q16_16_divide(t, U_MAX - *matrix_at(u, row, 0));
    }
}

static void minimize_cost(matrix_t * u_optimal,
                          const q16_16_t t,
                          const matrix_t * x,
                          const matrix_t * u,
                          const matrix_t * r,
                          matrix_t * hessian_inv)
{
    const uint16_t max_iterations = 500;
    uint16_t current_iteration = 0;
    const q16_16_t allowed_error = DOUBLE_TO_Q16_16(0.1);
    const q16_16_t target_step = DOUBLE_TO_Q16_16(0.1);
    q16_16_t current_error;
    q16_16_t last_error;
    q16_16_t i;

    MATRIX_DECLARE_STATIC(next_u, PREDICTION_HORIZON, 1);
    MATRIX_DECLARE_STATIC(gradient, PREDICTION_HORIZON, 1);
    MATRIX_DECLARE_STATIC(next_gradient, PREDICTION_HORIZON, 1);
    MATRIX_DECLARE_STATIC(p, PREDICTION_HORIZON, 1);
    MATRIX_DECLARE_STATIC(du, PREDICTION_HORIZON, 1);
    MATRIX_DECLARE_STATIC(y, PREDICTION_HORIZON, 1);

    //
    // Help matricies for updating the hessian
    //
    MATRIX_DECLARE_STATIC(hessian_inv_tmp, PREDICTION_HORIZON, PREDICTION_HORIZON);
    MATRIX_DECLARE_STATIC(hessian_inv_tmp2, PREDICTION_HORIZON, PREDICTION_HORIZON);
    MATRIX_DECLARE_STATIC(tmp_1x1, 1, 1);
    MATRIX_DECLARE_STATIC(tmp_row_vector, PREDICTION_HORIZON, 1);
    MATRIX_DECLARE_STATIC(tmp_col_vector, PREDICTION_HORIZON, 1);

    MATRIX_CREATE(next_u, PREDICTION_HORIZON, 1);
    MATRIX_CREATE(gradient, PREDICTION_HORIZON, 1);
    MATRIX_CREATE(next_gradient, PREDICTION_HORIZON, 1);
    MATRIX_CREATE(p, PREDICTION_HORIZON, 1);
    MATRIX_CREATE(du, PREDICTION_HORIZON, 1);
    MATRIX_CREATE(y, PREDICTION_HORIZON, 1);

    MATRIX_CREATE(hessian_inv_tmp, PREDICTION_HORIZON, PREDICTION_HORIZON);
    MATRIX_CREATE(hessian_inv_tmp2, PREDICTION_HORIZON, PREDICTION_HORIZON);
    MATRIX_CREATE(tmp_1x1, 1, 1);
    MATRIX_CREATE(tmp_row_vector, PREDICTION_HORIZON, 1);
    MATRIX_CREATE(tmp_col_vector, PREDICTION_HORIZON, 1);

    find_gradient(&next_gradient, x, u, r);
    add_constraint_barrier(&next_gradient, u, t);

    current_error = 0;

    for (i = 0; i != PREDICTION_HORIZON; ++i)
    {
        q16_16_t grad_element;
        grad_element = *matrix_at(&gradient, i, 0);
        current_error += q16_16_multiply(grad_element, grad_element);
    }

    last_error = current_error + 1;

    while ((current_iteration != max_iterations) &&
            (current_error > allowed_error) &&
            (current_error < last_error))
    {
        q16_16_t max_value;
        q16_16_t alpha;

        matrix_copy(&next_gradient, &gradient);
        matrix_copy(&next_u, u_optimal);
        last_error = current_error;
        
        //
        // Find next step
        //
        matrix_mult(hessian_inv, &gradient, &p);

        max_value = 0;
        
        for (i = 0; i != PREDICTION_HORIZON; ++i)
        {
            q16_16_t gradient_element = *matrix_at(&gradient, i, 0);
            
            if ( gradient_element > 0 && gradient_element > max_value)
            {
                max_value = gradient_element;
            }
            else if (gradient_element < 0 && -gradient_element > max_value)
            {
                max_value = -gradient_element;
            }
        }

        alpha = q16_16_divide(target_step, max_value);

        matrix_mult_elements(&p, alpha, &du);
        matrix_diff(u_optimal, &du, &next_u);

        //
        // Update hessian inverse approximation
        //
        find_gradient(&next_gradient, x, &next_u, r);
        add_constraint_barrier(&next_gradient, &next_u, t);

        matrix_diff(&next_gradient, &gradient, &y);

        /*
         To get the new inverse hessian approximation, we need to calculate:

         hessian_inv = hessian_inv + (du * du') / (du' * y) - ...
            (hessian_inv * y * y' * hessian_inv) / (y' * hessian_inv * y);
         
         */

        // hessian_inv_tmp = (du * du') / (du' * y)
        matrix_mult_r_transpose(&du, &du, &hessian_inv_tmp);
        matrix_mult_l_transpose(&du, &y, &tmp_1x1);
        matrix_mult_elements(&hessian_inv_tmp, *matrix_at(&tmp_1x1, 1, 1), &hessian_inv_tmp);

        // tmp_1x1 = (y' * hessian_inv * y)
        matrix_mult_l_transpose(&y, hessian_inv, &tmp_col_vector);
        matrix_mult(&tmp_col_vector, &y, &tmp_1x1);

        // hessian_inv_tmp2 = 
        //      (hessian_inv * y * y' * hessian_inv) / (y' * hessian_inv * y)
        matrix_mult(hessian_inv, &y, &tmp_row_vector);
        matrix_mult_l_transpose(&y, hessian_inv, &tmp_col_vector);
        matrix_mult(&tmp_row_vector, &tmp_col_vector, &hessian_inv_tmp2);
        matrix_mult_elements(&hessian_inv_tmp2, *matrix_at(&tmp_1x1, 1, 1),
                             &hessian_inv_tmp2);

        // hessian_inv += hessian_inv_tmp - hessian_inv_tmp2
        matrix_add(hessian_inv, &hessian_inv_tmp, hessian_inv);
        matrix_diff(hessian_inv, &hessian_inv_tmp2, hessian_inv);


        //
        // Move on to next iteration (if needed)
        //
        current_iteration += 1;
        
        current_error = 0;

        for (i = 0; i != PREDICTION_HORIZON; ++i)
        {
            q16_16_t grad_element;
            grad_element = *matrix_at(&gradient, i, 0);
            current_error += q16_16_multiply(grad_element, grad_element);
        }
    }
}

static void find_optimal_u(matrix_t * u_optimal,
                           const matrix_t * x,
                           const matrix_t * r)
{
    uint16_t row;
    MATRIX_DECLARE_STATIC(hessian_inv, PREDICTION_HORIZON, PREDICTION_HORIZON);
    MATRIX_DECLARE_STATIC(u_start, PREDICTION_HORIZON, 1);

    q16_16_t t = INT_TO_Q16_16(1000);
    const uint16_t MAX_ITER = 20;
    const q16_16_t H_INV_DIAG_VAL = DOUBLE_TO_Q16_16(0.04);
    const q16_16_t T_FACTOR = DOUBLE_TO_Q16_16(0.1);
    q16_16_t iter = 1;
    bool done = false;
    
    MATRIX_CREATE(hessian_inv, PREDICTION_HORIZON, PREDICTION_HORIZON);
    MATRIX_CREATE(u_start, PREDICTION_HORIZON, 1);

    matrix_zero(&hessian_inv);

    for (row = 0; row != PREDICTION_HORIZON; ++row)
    {
        *matrix_at(&hessian_inv, row, row) = H_INV_DIAG_VAL;
    }
    
    for (row = 0; row != PREDICTION_HORIZON; ++row)
    {
        *matrix_at(u_optimal, row, 0) = U_MID;
    }

    while (iter != MAX_ITER && !done)
    {
        matrix_copy(u_optimal, &u_start);

        minimize_cost(u_optimal,
                      t,
                      x,
                      &u_start,
                      r,
                      &hessian_inv);

        if (!is_u_within_constraints(u_optimal, U_MIN, U_MAX))
        {
            done = true;
            matrix_copy(&u_start, u_optimal);
        }
        else
        {
            t = q16_16_multiply(t, T_FACTOR);
            iter += 1;
        }
    }
}

static bool is_u_within_constraints(const matrix_t * u,
                                    const q16_16_t u_min,
                                    const q16_16_t u_max)
{
    bool retVal = true;
    q16_16_t * mat = u->m;
    uint16_t rows = u->rows;
    uint16_t cols = u->cols;
    uint16_t r;
    uint16_t c;
    
    for (r = 0; r != rows; ++r)
    {
        uint16_t row_offset = r * cols;
        
        for (c = 0; c != cols; ++c)
        {
            q16_16_t element = *(mat + row_offset + c);

            if ((u_min < element) || (element > u_max))
            {
                retVal = false;
            }
        }
    }
    
    return retVal;
}