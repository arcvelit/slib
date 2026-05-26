#ifndef SLIB_MATRIX_TYPE
#error "SLIB_MATRIX_TYPE must be defined"
#endif // SLIB_MATRIX_TYPE

// matrix.h (generic)

#include <stdint.h>
#include <string.h>

#ifdef SLIB_ASSERT_MATRIX_MULT 
# include <assert.h> // hidden directive for speed
#endif // SLIB_ASSERT_MATRIX_MULT

#define SLIB_CONCAT(a,b)  a##b
#define SLIB_CONCAT2(a,b) SLIB_CONCAT(a,b)

#ifndef STRUCTLIBDEF
# define STRUCTLIBDEF
#endif

#ifdef SLIB_STRIP_PREFIXES
# define SLIB_MATRIX SLIB_CONCAT2(mat_, SLIB_MATRIX_TYPE)
#else
# define SLIB_MATRIX SLIB_CONCAT2(slib_mat_, SLIB_MATRIX_TYPE)
#endif // SLIB_STRIP_PREFIXES

typedef struct {
    SLIB_MATRIX_TYPE* data;
    uint32_t rows;
    uint32_t cols;
} SLIB_MATRIX;

#define SLIB_MATRIX_MAKE_M SLIB_CONCAT2(SLIB_MATRIX, _make)
#define SLIB_MATRIX_COPY_M SLIB_CONCAT2(SLIB_MATRIX, _copy)
#define SLIB_MATRIX_MULT_M SLIB_CONCAT2(SLIB_MATRIX, _mult)
#define SLIB_MATRIX_SCALE_M SLIB_CONCAT2(SLIB_MATRIX, _scale)

STRUCTLIBDEF SLIB_MATRIX SLIB_MATRIX_MAKE_M(SLIB_MATRIX_TYPE* const data, const uint32_t rows, const uint32_t cols);
STRUCTLIBDEF void SLIB_MATRIX_COPY_M(SLIB_MATRIX* dst, const SLIB_MATRIX* src);
STRUCTLIBDEF void SLIB_MATRIX_MULT_M(SLIB_MATRIX* src, const SLIB_MATRIX mat1, const SLIB_MATRIX mat2);
STRUCTLIBDEF void SLIB_MATRIX_SCALE_M(SLIB_MATRIX* src, const SLIB_MATRIX_TYPE scalar);

#ifdef SLIB_IMPLEMENTATION

#define SLIB_MAT_INDEX(mat, i, j) ((mat).data[(i)*(mat).cols+(j)])

STRUCTLIBDEF SLIB_MATRIX SLIB_MATRIX_MAKE_M(SLIB_MATRIX_TYPE* const data, const uint32_t rows, const uint32_t cols) {
    return (SLIB_MATRIX) {
        .data = data,
        .rows = rows,
        .cols = cols
    };
}

STRUCTLIBDEF void SLIB_MATRIX_COPY_M(SLIB_MATRIX* const dst, const SLIB_MATRIX* const src) {
    memcpy(dst->data, src->data, src->rows * src->cols * sizeof(SLIB_MATRIX_TYPE));
}

STRUCTLIBDEF void SLIB_MATRIX_MULT_M(SLIB_MATRIX* const src, const SLIB_MATRIX mat1, const SLIB_MATRIX mat2) {
    #ifdef SLIB_ASSERT_MATRIX_MULT
    assert(mat1.cols == mat2.rows && "matrix size mismatch");
    #endif
    for (uint32_t i = 0; i < mat1.rows; i++) {
        for (uint32_t j = 0; j < mat2.cols; j++) {
            SLIB_MATRIX_TYPE acc = 0;
            for (uint32_t k = 0; k < mat2.rows; k++) {
                acc += SLIB_MAT_INDEX(mat1, i, k) * SLIB_MAT_INDEX(mat2, k, j);
            }
            SLIB_MAT_INDEX(*src, i, j) = acc;
        }
    }
}

STRUCTLIBDEF void SLIB_MATRIX_SCALE_M(SLIB_MATRIX* src, const SLIB_MATRIX_TYPE scalar) {
    for (size_t i = 0; i < src->rows * src->cols; i++) {
        src->data[i] *= scalar;
    }
}

#endif // SLIB_IMPLEMENTATION

#undef SLIB_MATRIX_TYPE

#undef SLIB_MATRIX

#undef SLIB_MATRIX_MAKE_M
#undef SLIB_MATRIX_COPY_M
#undef SLIB_MATRIX_MULT_M
