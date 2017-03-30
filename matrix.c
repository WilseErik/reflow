// =============================================================================
// Include statements
// =============================================================================

#include "matrix.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "uart.h"

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

// =============================================================================
// Private function declarations
// =============================================================================

/**
 * @brief Reports a matrix operation error.
 * @param func - function name of calling function.
 */
static void matrix_op_err(const char * func);

// =============================================================================
// Public function definitions
// =============================================================================

void matrix_create(matrix_t * m,
                   uint16_t rows,
                   uint16_t cols,
                   q16_16_t * array)
{
    m->rows = rows;
    m->cols = cols;
    m->m = array;
}

void matrix_zero(matrix_t * m)
{
    const uint16_t r_max = m->rows;
    const uint16_t c_max = m->cols;
    uint16_t r;
    uint16_t c;
    q16_16_t * mat = m->m;

    q16_16_t * p = mat;

    for (r = 0; r != r_max; ++r)
    {
        for (c = 0; c != c_max; ++c)
        {
            *p++ = 0;
        }
    }
}

void matrix_eye(matrix_t * m)
{
    const uint16_t r_max = m->rows;
    const uint16_t c_max = m->cols;
    uint16_t r;
    q16_16_t * p = m->m;

    if (m->rows != m->cols)
    {
        matrix_op_err(__func__);
        return;
    }

    matrix_zero(m);

    for (r = 0; r != r_max; ++r)
    {
        *(p + r * c_max + r) = Q16_16_T_ONE;
    }
}

matrix_t * matrix_add(const matrix_t * a, const matrix_t * b, matrix_t * sum)
{
    const uint16_t r_max = a->rows;
    const uint16_t c_max = a->cols;
    uint16_t r;
    uint16_t c;

    q16_16_t * p_sum = sum->m;
    q16_16_t * p_a = a->m;
    q16_16_t * p_b = b->m;
    
    if (!matrix_is_same_dimension(a, b))
    {
        matrix_op_err(__func__);
        return NULL;
    }

    for (r = 0; r != r_max; ++r)
    {
        for (c = 0; c != c_max; ++c)
        {
            *p_sum++ = *p_a++ + *p_b++;
        }
    }

    return sum;
}

matrix_t * matrix_diff(const matrix_t * a, const matrix_t * b, matrix_t * diff)
{
    const uint16_t r_max = a->rows;
    const uint16_t c_max = a->cols;
    uint16_t r;
    uint16_t c;

    q16_16_t * p_diff = diff->m;
    q16_16_t * p_a = a->m;
    q16_16_t * p_b = b->m;

    if (!matrix_is_same_dimension(a, b))
    {
        matrix_op_err(__func__);
        return NULL;
    }

    for (r = 0; r != r_max; ++r)
    {
        for (c = 0; c != c_max; ++c)
        {
            *p_diff++ = *p_a++ - *p_b++;
        }
    }

    return diff;
}

matrix_t * matrix_mult(const matrix_t * a, const matrix_t * b, matrix_t * prod)
{
    const uint16_t r_max = prod->rows;
    const uint16_t c_max = prod->cols;
    uint16_t r;
    uint16_t c;
    uint16_t e;
    
    q16_16_t * a_mat = a->m;
    q16_16_t * b_mat = b->m;
    q16_16_t * prod_mat = prod->m;

    if ((a->cols != b->rows) ||
        (a->rows != prod->rows) || (b->cols != prod->cols))
    {
        matrix_op_err(__func__);
        return  NULL;
    }

    matrix_zero(prod);

    for (r = 0; r != r_max; ++r)
    {
        for (c = 0; c != c_max; ++c)
        {
            uint16_t row_offset = r * c_max;

            for (e = 0; e != r_max; ++e)
            {
                q16_16_t item_prod = q16_16_multiply(*(a_mat + row_offset + e),
                                                     *(b_mat + e * c_max + c));
                *(prod_mat + row_offset + c) += item_prod;
            }
        }
    }

    return prod;
}

matrix_t * matrix_transpose(const matrix_t * m, matrix_t * result)
{
    const uint16_t r_max = result->rows;
    const uint16_t c_max = result->cols;
    uint16_t r;
    uint16_t c;
    q16_16_t * m_mat = m->m;
    q16_16_t * result_mat = result->m;

    if ((m->cols != result->rows) || (m->rows != result->cols))
    {
        matrix_op_err(__func__);
        return NULL;
    }

    for (r = 0; r != r_max; ++r)
    {
        uint16_t row_offset_result = r * c_max;
        uint16_t row_offset_m = c * c_max;

        for (c = 0; c != c_max; ++c)
        {
            *(result_mat + row_offset_result + c) = *(m_mat + row_offset_m + r);
        }
    }

    return result;
}

matrix_t * matrix_mult_l_transpose(const matrix_t * a, const matrix_t * b,
        matrix_t * result)
{
    const uint16_t r_max = result->rows;
    const uint16_t c_max = result->cols;
    uint16_t r;
    uint16_t c;
    uint16_t e;

    q16_16_t * a_mat = a->m;
    q16_16_t * b_mat = b->m;
    q16_16_t * prod_mat = result->m;

    if ((a->rows != b->rows) ||
        (a->cols != result->rows) || (b->cols != result->cols))
    {
        matrix_op_err(__func__);
        return  NULL;
    }

    matrix_zero(result);

    for (r = 0; r != r_max; ++r)
    {
        for (c = 0; c != c_max; ++c)
        {
            uint16_t row_offset = r * c_max;

            for (e = 0; e != r_max; ++e)
            {
                q16_16_t item_prod = q16_16_multiply(*(a_mat + e * c_max + r),
                                                     *(b_mat + e * c_max + c));
                *(prod_mat + row_offset + c) += item_prod;
            }
        }
    }

    return result;
}

matrix_t * matrix_mult_r_transpose(const matrix_t * a, const matrix_t * b,
        matrix_t * result)
{
    const uint16_t r_max = result->rows;
    const uint16_t c_max = result->cols;
    uint16_t r;
    uint16_t c;
    uint16_t e;

    q16_16_t * a_mat = a->m;
    q16_16_t * b_mat = b->m;
    q16_16_t * prod_mat = result->m;

    if ((a->cols != b->cols) ||
        (a->rows != result->rows) || (b->rows != result->cols))
    {
        matrix_op_err(__func__);
        return  NULL;
    }

    matrix_zero(result);

    for (r = 0; r != r_max; ++r)
    {
        for (c = 0; c != c_max; ++c)
        {
            uint16_t row_offset = r * c_max;

            for (e = 0; e != r_max; ++e)
            {
                q16_16_t item_prod = q16_16_multiply(*(a_mat + row_offset + e),
                                                     *(b_mat + c * c_max + e));
                *(prod_mat + row_offset + c) += item_prod;
            }
        }
    }

    return result;
}

matrix_t * matrix_mult_elements(const matrix_t * m, q16_16_t factor,
        matrix_t * result)
{
    const uint16_t r_max = m->rows;
    const uint16_t c_max = m->cols;
    uint16_t r;
    uint16_t c;
    q16_16_t * p_m = m->m;
    q16_16_t * p_result = result->m;

    if ((m->rows != result->rows) || (m->cols != result->cols))
    {
        matrix_op_err(__func__);
        return NULL;
    }

    for (r = 0; r != r_max; ++r)
    {
        for (c = 0; c != c_max; ++c)
        {
            *p_result++ = q16_16_multiply(*p_m++, factor);
        }
    }

    return result;
}

matrix_t * matrix_copy(const matrix_t * src, matrix_t * dst)
{
    const uint16_t r_max = src->rows;
    const uint16_t c_max = src->cols;
    uint16_t r;
    uint16_t c;
    q16_16_t * p_src = src->m;
    q16_16_t * p_dst = dst->m;

    if (!matrix_is_same_dimension(src, dst))
    {
        matrix_op_err(__func__);
        return NULL;
    }

    for (r = 0; r != r_max; ++r)
    {
        for (c = 0; c != c_max; ++c)
        {
            *p_dst++ = *p_src++;
        }
    }

    return dst;
}

bool matrix_is_same_dimension(const matrix_t * a, const matrix_t * b)
{
    return ((a->cols == b->cols) && (a->rows == b->rows));
}

// =============================================================================
// Private function definitions
// =============================================================================

static void matrix_op_err(const char * func)
{
    uart_write_string("Invalid matrix operation in ");
    uart_write_string(func);
    uart_write_string("\r\n");
}
