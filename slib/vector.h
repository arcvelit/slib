#ifndef SLIB_VECTOR_TYPE
#error "SLIB_VECTOR_TYPE must be defined"
#endif // SLIB_VECTOR_TYPE

#include <stdlib.h>
#include <string.h>

/**
 * vector.h provides a template vector definition and implementation
 * 
 * Declare a vector with
 * 
 * #define SLIB_VECTOR_TYPE int
 * #include "vector.h"
 * 
 * Prefixes will be
 * slib_vec_int_...
 * 
 * unless SLIB_STRIP_PREFIXES is defined, otherwise
 * vec_int_....
 * 
 */

#define SLIB_CONCAT(a,b)  a##b
#define SLIB_CONCAT2(a,b) SLIB_CONCAT(a,b)

#ifndef STRUCTLIBDEF
# define STRUCTLIBDEF
#endif // STRUCTLIBDEF

#ifdef SLIB_STRIP_PREFIXES
# define SLIB_VECTOR SLIB_CONCAT2(vec_, SLIB_VECTOR_TYPE)
#else
# define SLIB_VECTOR SLIB_CONCAT2(slib_vec_, SLIB_VECTOR_TYPE)
#endif // SLIB_STRIP_PREFIXES

#ifndef SLIB_VECTOR_INITIAL_CAP
#define SLIB_VECTOR_INITIAL_CAP 4
#endif // SLIB_VECTOR_INITIAL_CAP

typedef struct {
    SLIB_VECTOR_TYPE* data;
    size_t size;
    size_t cap;
} SLIB_VECTOR;

#define SLIB_VECTOR_RESERVE_M       SLIB_CONCAT2(SLIB_VECTOR, _reserve)
#define SLIB_VECTOR_GROW_M          SLIB_CONCAT2(SLIB_VECTOR, _grow)
#define SLIB_VECTOR_APPEND_M        SLIB_CONCAT2(SLIB_VECTOR, _append)
#define SLIB_VECTOR_APPEND_MANY_M   SLIB_CONCAT2(SLIB_VECTOR, _append_many)
#define SLIB_VECTOR_APPEND_BUFFER_M SLIB_CONCAT2(SLIB_VECTOR, _append_buffer)
#define SLIB_VECTOR_FREE_M          SLIB_CONCAT2(SLIB_VECTOR, _free)

STRUCTLIBDEF int SLIB_VECTOR_RESERVE_M(SLIB_VECTOR* const vec, size_t amount);
STRUCTLIBDEF SLIB_VECTOR_TYPE* SLIB_VECTOR_GROW_M(SLIB_VECTOR* const vec, size_t amount);
STRUCTLIBDEF int SLIB_VECTOR_APPEND_M(SLIB_VECTOR* const vec, const SLIB_VECTOR_TYPE elem);
STRUCTLIBDEF int SLIB_VECTOR_APPEND_MANY_M(SLIB_VECTOR* const dst, const SLIB_VECTOR* const src);
STRUCTLIBDEF int SLIB_VECTOR_APPEND_BUFFER_M(SLIB_VECTOR* const dst, const SLIB_VECTOR_TYPE* const src, const size_t size);
STRUCTLIBDEF void SLIB_VECTOR_FREE_M(SLIB_VECTOR* const vec);

#ifdef SLIB_IMPLEMENTATION

// Internals
#define SLIB_VECTOR_PREPARE_CAPACITY_INTERNAL SLIB_CONCAT2(SLIB_VECTOR, _increase_capacity_internal)

static inline int SLIB_VECTOR_PREPARE_CAPACITY_INTERNAL(SLIB_VECTOR* const vec, const size_t required) {
    if (vec->cap < required) {
        size_t new_cap = vec->cap ? vec->cap : SLIB_VECTOR_INITIAL_CAP;
        do new_cap *= 2; while (new_cap < required);
        fflush(stdout);
        SLIB_VECTOR_TYPE* const new_data = realloc(vec->data, new_cap * sizeof(SLIB_VECTOR_TYPE));
        if (!new_data) return 0;
        vec->data = new_data;
        vec->cap  = new_cap;

    }
    return 1;
}

// API
STRUCTLIBDEF int SLIB_VECTOR_RESERVE_M(SLIB_VECTOR* const vec, size_t amount) {
    if (vec->cap < amount) {
        SLIB_VECTOR_TYPE* const new_data = realloc(vec->data, amount * sizeof(SLIB_VECTOR_TYPE));
        if (!new_data) return 0;
        vec->data = new_data;
        vec->cap  = amount;
    }
    return 1;
}

STRUCTLIBDEF SLIB_VECTOR_TYPE* SLIB_VECTOR_GROW_M(SLIB_VECTOR* const vec, size_t amount) {
    const size_t required = vec->size + amount;
    if (SLIB_VECTOR_PREPARE_CAPACITY_INTERNAL(vec, required)) {
        SLIB_VECTOR_TYPE* const ret = vec->data + vec->size;
        vec->size += amount;
        return ret;
    }
    return NULL;
}

STRUCTLIBDEF int SLIB_VECTOR_APPEND_M(SLIB_VECTOR* const vec, const SLIB_VECTOR_TYPE elem) {
    if (SLIB_VECTOR_PREPARE_CAPACITY_INTERNAL(vec, vec->size + 1)) {
        vec->data[vec->size++] = elem;
        return 1;
    }
    return 0;
}

STRUCTLIBDEF int SLIB_VECTOR_APPEND_MANY_M(SLIB_VECTOR* const dst, const SLIB_VECTOR* const src) {
    const size_t new_size = dst->size + src->size;
    if (SLIB_VECTOR_PREPARE_CAPACITY_INTERNAL(dst, new_size)) {
        memcpy(dst->data + dst->size, src->data, src->size * sizeof(SLIB_VECTOR_TYPE));
        dst->size = new_size;
        return 1;
    }
    return 0;
}

STRUCTLIBDEF int SLIB_VECTOR_APPEND_BUFFER_M(SLIB_VECTOR* const dst, const SLIB_VECTOR_TYPE* const src, const size_t size) {
    const size_t new_size = dst->size + size;
    if (SLIB_VECTOR_PREPARE_CAPACITY_INTERNAL(dst, new_size)) {
        memcpy(dst->data + dst->size, src, size * sizeof(SLIB_VECTOR_TYPE));
        dst->size = new_size;
        return 1;
    }
    return 0;
}

STRUCTLIBDEF void SLIB_VECTOR_FREE_M(SLIB_VECTOR* const vec) {
    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->cap  = 0;
}

#undef SLIB_VECTOR_PREPARE_CAPACITY_INTERNAL

#endif // SLIB_IMPLEMENTATION

#undef SLIB_VECTOR_TYPE

#undef SLIB_VECTOR

#undef SLIB_VECTOR_RESERVE_M
#undef SLIB_VECTOR_GROW_M
#undef SLIB_VECTOR_APPEND_M
#undef SLIB_VECTOR_APPEND_MANY_M
#undef SLIB_VECTOR_APPEND_BUFFER_M
#undef SLIB_VECTOR_FREE_M
