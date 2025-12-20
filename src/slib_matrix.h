#ifndef _SLIB_MATRIX_C
#define _SLIB_MATRIX_C

/**
 * slib_matrix.h provides general purpose linear algebra tools on the stack
 * 
 * Create a matrix this way:
 * 
 *  float _a[2][2] = {0};
 *  Mat A = matrix_make((float*)&_a, 2, 2);
 * 
 * */

#include <stdint.h>
#include <string.h>
#if SLIB_MATRIX_INDEX 
    #include <assert.h>
#endif // SLIB_ASSERT_MATRIX_MULT

#ifndef STRUCTLIBDEF
    #define STRUCTLIBDEF
#endif

#define MATRIX_INDEX(mat, i, j) ((mat)->data[(i)*(mat)->cols+(j)])

typedef struct {
    float*   data;
    uint32_t rows;
    uint32_t cols;
} Matrix;

STRUCTLIBDEF Matrix matrix_make(float* const data, const uint32_t rows, const uint32_t cols);
STRUCTLIBDEF void matrix_copy(Matrix* dst, const Matrix* src);
STRUCTLIBDEF void matrix_mult(Matrix* src, const Matrix* mat1, const Matrix* mat2);

#ifdef SLIB_MATRIX_IMPLEMENTATION

STRUCTLIBDEF Matrix matrix_make(float* const data, const uint32_t rows, const uint32_t cols) {
    return (Matrix){
        .data = data,
        .rows = rows,
        .cols = cols
    };
}

STRUCTLIBDEF void matrix_copy(Matrix* const dst, const Matrix* const src) {
    memcpy(dst->data, src->data, src->rows * src->cols * sizeof(float));
}

STRUCTLIBDEF void matrix_mult(Matrix* const src, const Matrix* const mat1, const Matrix* const mat2) {
    #ifdef SLIB_ASSERT_MATRIX_MULT
    assert(mat1->cols == mat2->rows && "matrix size mismatch");
    #endif
    for (uint32_t i = 0; i < mat1->rows; i++) {
        for (uint32_t j = 0; j < mat2->cols; j++) {
            float acc = 0;
            for (uint32_t k = 0; k < mat2->rows; k++) {
                acc += MATRIX_INDEX (mat1, i, k) * MATRIX_INDEX (mat2, k, j);
            }
            MATRIX_INDEX (src, i, j) = acc;
        }
    }
}

#endif // SLIB_MATRIX_IMPLEMENTATION

#endif // _SLIB_MATRIX_C