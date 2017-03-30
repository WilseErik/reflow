#ifndef MATRIX_H
#define	MATRIX_H

#ifdef	__cplusplus
extern "C" {
#endif

// =============================================================================
// Include statements
// =============================================================================

#include "fixed_point.h"


#include <stdint.h>
#include <stdbool.h>
    
// =============================================================================
// Public type definitions
// =============================================================================

typedef struct matrix_t
{
    uint16_t rows;
    uint16_t cols;
    q16_16_t * m;
} matrix_t;

// =============================================================================
// Global variable declarations
// =============================================================================

// =============================================================================
// Global constatants
// =============================================================================

// =============================================================================
// Public function declarations
// =============================================================================

#define MATRIX_ELEMENTS(r, c) ((r)*(c))

/*
 * Creates a matrix declaration.
 * For example MATRIX_DECLARE(a, 2, 3) expands to:
 * matrix_t a;
 * q16_16_t a_mat[2*3];
 */
#define MATRIX_DECLARE(name, r, c) matrix_t name; \
    q16_16_t name ## _mat[(r)*(c)];

/*
 * Creates a static matrix declaration.
 * For example MATRIX_DECLARE_STATIC(a, 2, 3) expands to:
 * static matrix_t a;
 * static q16_16_t a_mat[2*3];
 */
#define MATRIX_DECLARE_STATIC(name, r, c) static matrix_t name; \
    static q16_16_t name ## _mat[(r)*(c)];

/*
 * Creates a previously declared matrix.
 * For example MATRIX_CREATE(a, 2, 3) expands to:
 * matrix_create(&a, 2, 3, (q16_16_t*)a_mat);
 */
#define MATRIX_CREATE(name, r, c) matrix_create(& name, (r), (c), \
    (q16_16_t*)name ## _mat);

/*
 * Creates  and declares a matrix.
 * For example MATRIX_DECLARE_AND_CREATE(a, 2, 3) expands to:
 * matrix_t a;
 * q16_16_t a_mat[2*3];
 * matrix_create(&a, 2, 3, (q16_16_t*)a_mat);
 */
#define MATRIX_DECLARE_AND_CREATE(name, r, c) matrix_t name; \
    q16_16_t name ## _mat[(r)*(c)]; \
    matrix_create(& name, (r), (c), (q16_16_t*)name ## _mat);

/**
 * @brief Gets a pointer to the element at row r, column c of matrix m.
 * @param m - Matrix to find element in.
 * @param r - Row of desired element.
 * @param c - Column of desired element.
 * @return
 */
static inline q16_16_t * matrix_at(const matrix_t * m, uint16_t r, uint16_t c)
{
    return (m->m + r * m->cols + c);
}

/**
 * @brief Fills in the matrix_t struct.
 * @param m - Struct to fill.
 * @param rows - Number of rows.
 * @param cols - Number of columns.
 * @param array - Array to store matrix in.
 */
void matrix_create(matrix_t * m,
                   uint16_t rows,
                   uint16_t cols,
                   q16_16_t * array);

/**
 * @brief Sets all elements of the matrix to zero.
 * @param m - matrix to clear.
 */
void matrix_zero(matrix_t * m);

/**
 * @brief Sets the matrix to the identity matrix.
 * @details Matix must be have same amount of rows and columns.
 * @param m - Matrix to modify.
 */
void matrix_eye(matrix_t * m);

/**
 * @brief Calculates the sum of two matrices a + b.
 * @param a - one term.
 * @param b - second term.
 * @param sum - Resulting sum.
 * @return Pointer to the result.
 */
matrix_t * matrix_add(const matrix_t * a, const matrix_t * b, matrix_t * sum);

/**
 * @brief Calculates the difference between two matrices, diff = a - b.
 * @param a - one term.
 * @param b - second term.
 * @param diff - Resulting difference.
 * @return Pointer to the result.
 */
matrix_t * matrix_diff(const matrix_t * a, const matrix_t * b, matrix_t * diff);

/**
 * @brief Multiplies two matricies, prod = a * b.
 * @param a - One factor.
 * @param b - Second factor.
 * @param prod - Product of a and b.
 * @return Pointer to the result.
 */
matrix_t * matrix_mult(const matrix_t * a, const matrix_t * b, matrix_t * prod);

/**
 * @brief Calculates the transpose of a matrix.
 * @param m - Matrix to transpose.
 * @param result - Transposed matrix.
 * @return Pointer to the result.
 */
matrix_t * matrix_transpose(const matrix_t * m, matrix_t * result);

/**
 * @brief Calulates result = a^T * b
 * @param a Matrix to transpose before multiplying.
 * @param b Right multiplication factor.
 * @param result result = a^T * b
 * @return pointer to the result matrix
 */
matrix_t * matrix_mult_l_transpose(const matrix_t * a, const matrix_t * b,
        matrix_t * result);

/**
 * @brief Calulates result = a * b^T
 * @param a Left multiplication factor.
 * @param b Matrix to transpose before multiplying.
 * @param result result = a * b^T
 * @return pointer to the result matrix
 */
matrix_t * matrix_mult_r_transpose(const matrix_t * a, const matrix_t * b,
        matrix_t * result);

/**
 * @brief Multiplies all elements in the matrix by a scalar factor.
 * @param m - Matrix factor.
 * @param factor - Scalar factor.
 * @param result - result of the multiplication.
 * @return Pointer to the result.
 */
matrix_t * matrix_mult_elements(const matrix_t * m, q16_16_t factor,
        matrix_t * result);

/**
 * @breif Copies the values of one matrix to another.
 * @param src - Matrix to copy from.
 * @param dst - Matrix to copy to.
 * @return Pointer to the destination.
 */
matrix_t * matrix_copy(const matrix_t * src, matrix_t * dst);

/**
 * @brief Checks is two matricies have the same dimension.
 * @param a
 * @param b
 * @return True if both matricies have the same dimensions.
 */
bool matrix_is_same_dimension(const matrix_t * a, const matrix_t * b);

#ifdef	__cplusplus
}
#endif

#endif	/* HEADER_TEMPLATE_H */

