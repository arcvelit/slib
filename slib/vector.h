#ifndef SLIB_VECTOR_TYPE
#error "SLIB_VECTOR_TYPE must be defined"
#endif // SLIB_VECTOR_TYPE

#include <stdlib.h>
#include <string.h>

/**
 * vector.h provides a template vector
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

#define JOIN(a,b) a##b
#define JOIN2(a,b) JOIN(a,b)

#ifndef STRUCTLIBDEF
# define STRUCTLIBDEF
#endif // STRUCTLIBDEF

#ifdef SLIB_STRIP_PREFIXES
# define SLIB_VECTOR_PREFIX JOIN2(vec_, SLIB_VECTOR_TYPE)
#else
# define SLIB_VECTOR_PREFIX JOIN2(slib_vec_, SLIB_VECTOR_TYPE)
#endif // SLIB_STRIP_PREFIXES

#ifndef VECTOR_INITIAL_CAP
#define VECTOR_INITIAL_CAP 4
#endif // VECTOR_INITIAL_CAP

typedef struct {
    SLIB_VECTOR_TYPE* data;
    size_t size;
    size_t cap;
} SLIB_VECTOR_PREFIX;

#define VECTOR_RESERVE_M     JOIN2(SLIB_VECTOR_PREFIX, _reserve)
#define VECTOR_GROW_M        JOIN2(SLIB_VECTOR_PREFIX, _grow)
#define VECTOR_APPEND_M      JOIN2(SLIB_VECTOR_PREFIX, _append)
#define VECTOR_APPEND_MANY_M JOIN2(SLIB_VECTOR_PREFIX, _append_many)
#define VECTOR_FREE_M        JOIN2(SLIB_VECTOR_PREFIX, _free)

STRUCTLIBDEF int VECTOR_RESERVE_M(SLIB_VECTOR_PREFIX* const vec, size_t amount);
STRUCTLIBDEF SLIB_VECTOR_TYPE* VECTOR_GROW_M(SLIB_VECTOR_PREFIX* const vec, size_t amount);
STRUCTLIBDEF int VECTOR_APPEND_M(SLIB_VECTOR_PREFIX* const vec, const SLIB_VECTOR_TYPE elem);
STRUCTLIBDEF int VECTOR_APPEND_MANY_M(SLIB_VECTOR_PREFIX* const dst, const SLIB_VECTOR_PREFIX* const src);
STRUCTLIBDEF void VECTOR_FREE_M(SLIB_VECTOR_PREFIX* const vec);

#ifdef SLIB_IMPLEMENTATION

STRUCTLIBDEF int VECTOR_RESERVE_M(SLIB_VECTOR_PREFIX* const vec, size_t amount) {
    if (vec->cap < amount) {
        SLIB_VECTOR_TYPE* const new_data = realloc(vec->data, amount * sizeof(SLIB_VECTOR_TYPE));
        if (!new_data) return 0;
        vec->data = new_data;
        vec->cap  = amount;
    }
    return 1;
}

STRUCTLIBDEF SLIB_VECTOR_TYPE* VECTOR_GROW_M(SLIB_VECTOR_PREFIX* const vec, size_t amount) {
const size_t required = vec->size + amount;
    if (required > vec->cap) {
        size_t new_cap = vec->cap;
        if (new_cap) {
            do new_cap *= 2; while (new_cap < required);
        } else { new_cap = required; }
        SLIB_VECTOR_TYPE* const new_data = realloc(vec->data, new_cap * sizeof(SLIB_VECTOR_TYPE));
        if (!new_data) return 0;
        vec->data = new_data;
        vec->cap  = new_cap;
    }
    SLIB_VECTOR_TYPE* const ret = vec->data + vec->size;
    vec->size += amount;
    return ret;
}

STRUCTLIBDEF int VECTOR_APPEND_M(SLIB_VECTOR_PREFIX* const vec, const SLIB_VECTOR_TYPE elem) {
    if (vec->size == vec->cap) {
        const size_t new_cap = vec->cap ? vec->cap * 2 : VECTOR_INITIAL_CAP;
        SLIB_VECTOR_TYPE* const new_data = realloc(vec->data, new_cap * sizeof(SLIB_VECTOR_TYPE));
        if (!new_data) return 0;
        vec->data = new_data;
        vec->cap  = new_cap;
    }
    vec->data[vec->size++] = elem;
    return 1;
}

STRUCTLIBDEF int VECTOR_APPEND_MANY_M(SLIB_VECTOR_PREFIX* const dst, const SLIB_VECTOR_PREFIX* const src) {
    const size_t new_size = dst->size + src->size;
    if (VECTOR_RESERVE_M(dst, new_size)) {
        memcpy(dst->data + dst->size, src->data, src->size * sizeof(SLIB_VECTOR_TYPE));
        dst->size = new_size;
        return 1;
    }
    return 0;
}

STRUCTLIBDEF void VECTOR_FREE_M(SLIB_VECTOR_PREFIX* const vec) {
    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->cap  = 0;
}

#endif // VECTOR_IMPLEMENTATION

#undef SLIB_VECTOR_TYPE

#undef SLIB_VECTOR_PREFIX

#undef VECTOR_RESERVE_M
#undef VECTOR_GROW_M
#undef VECTOR_APPEND_M
#undef VECTOR_APPEND_MANY_M
#undef VECTOR_FREE_M
