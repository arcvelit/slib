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

#ifndef STRUCTLIBDEF
# define STRUCTLIBDEF
#endif

#if SLIB_ASSERT_MATRIX_MULT 
# include <assert.h> // hidden directive for speed
#endif // SLIB_ASSERT_MATRIX_MULT

#define MATRIX_INDEX(mat, i, j) ((mat)->data[(i)*(mat)->cols+(j)])

typedef struct {
    float*   data;
    uint32_t rows;
    uint32_t cols;
} SLIBMatrix;

STRUCTLIBDEF SLIBMatrix slib_matrix_make(float* const data, const uint32_t rows, const uint32_t cols);
STRUCTLIBDEF void slib_matrix_copy(SLIBMatrix* dst, const SLIBMatrix* src);
STRUCTLIBDEF void slib_matrix_mult(SLIBMatrix* src, const SLIBMatrix* mat1, const SLIBMatrix* mat2);

#ifdef SLIB_STRIP_PREFIXES
typedef  SLIBMatrix  Matrix;
# define matrix_make slib_matrix_make
# define matrix_copy slib_matrix_copy
# define matrix_mult slib_matrix_mult
#endif // SLIB_STRIP_PREFIXES

#ifdef SLIB_MATRIX_IMPLEMENTATION

STRUCTLIBDEF SLIBMatrix slib_matrix_make(float* const data, const uint32_t rows, const uint32_t cols) {
    return (SLIBMatrix) {
        .data = data,
        .rows = rows,
        .cols = cols
    };
}

STRUCTLIBDEF void slib_matrix_copy(SLIBMatrix* const dst, const SLIBMatrix* const src) {
    memcpy(dst->data, src->data, src->rows * src->cols * sizeof(float));
}

STRUCTLIBDEF void slib_matrix_mult(SLIBMatrix* const src, const SLIBMatrix* const mat1, const SLIBMatrix* const mat2) {
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