#ifndef SLIB_VECTOR_TYPE
#error "SLIB_VECTOR_TYPE must be defined"
#endif

// vector.h (generic)

#include <stdlib.h>
#include <string.h>

#define SLIB_CONCAT(a,b)  a##b
#define SLIB_CONCAT2(a,b) SLIB_CONCAT(a,b)

#ifndef SLIB_API
# define SLIB_API
#endif

#ifndef SLIB_VECTOR_ERROR
#define SLIB_VECTOR_ERROR
static int slib_vecerr = 0;
#endif

#ifdef SLIB_STRIP_PREFIXES
# define SLIB_VECTOR SLIB_CONCAT2(vec_, SLIB_VECTOR_TYPE)
# define vecerr slib_vecerr
#else
# define SLIB_VECTOR SLIB_CONCAT2(slib_vec_, SLIB_VECTOR_TYPE)
#endif

#ifndef SLIB_VECTOR_INITIAL_CAP
#define SLIB_VECTOR_INITIAL_CAP 4
#endif

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

#define SLIB_VECTOR_COMPARATOR SLIB_CONCAT2(SLIB_VECTOR, _comparator)

// Comparing function pointer
typedef int (*SLIB_VECTOR_COMPARATOR)(SLIB_VECTOR_TYPE*);

#define SLIB_VECTOR_ITERATOR SLIB_CONCAT2(SLIB_VECTOR, _iterator)

typedef struct {
    const SLIB_VECTOR* const vec;
    SLIB_VECTOR_TYPE* next;
} SLIB_VECTOR_ITERATOR;

#define SLIB_VECTOR_ITERATOR_MAKE_M SLIB_CONCAT2(SLIB_VECTOR, _iterator_make)
#define SLIB_VECTOR_FIRST_M         SLIB_CONCAT2(SLIB_VECTOR, _first)
#define SLIB_VECTOR_NEXT_M          SLIB_CONCAT2(SLIB_VECTOR, _next)

// Ensure the vector has a capacity of this amount
SLIB_API void SLIB_VECTOR_RESERVE_M(SLIB_VECTOR* const vec, size_t amount);

// Bumps the size of the vector and returns a pointer to the first slot (allows to emplace without copying)
SLIB_API SLIB_VECTOR_TYPE* SLIB_VECTOR_GROW_M(SLIB_VECTOR* const vec, size_t amount);

// Increases the size and copies the object to the vector
SLIB_API void SLIB_VECTOR_APPEND_M(SLIB_VECTOR* const vec, const SLIB_VECTOR_TYPE elem);

// Appends the content of another vector
SLIB_API void SLIB_VECTOR_APPEND_MANY_M(SLIB_VECTOR* const dst, const SLIB_VECTOR* const src);

// Appends from an array of elements (uses memcpy)
SLIB_API void SLIB_VECTOR_APPEND_BUFFER_M(SLIB_VECTOR* const dst, const SLIB_VECTOR_TYPE* const src, const size_t size);

// Frees the allocated memory
SLIB_API void SLIB_VECTOR_FREE_M(SLIB_VECTOR* const vec);

// Creates a basic iterator state
SLIB_API SLIB_VECTOR_ITERATOR SLIB_VECTOR_ITERATOR_MAKE_M(const SLIB_VECTOR* vec);

// Returns pointer of first occurence or null
SLIB_API SLIB_VECTOR_TYPE* SLIB_VECTOR_FIRST_M(SLIB_VECTOR* const vec, SLIB_VECTOR_COMPARATOR comparator);

// Returns pointer of next occurence or null
SLIB_API SLIB_VECTOR_TYPE* SLIB_VECTOR_NEXT_M(SLIB_VECTOR_ITERATOR* vec_iter, SLIB_VECTOR_COMPARATOR comparator);

#ifdef SLIB_IMPLEMENTATION

// Internals
#define SLIB__VECTOR_PREPARE_CAPACITY_INTERNAL SLIB_CONCAT2(SLIB_VECTOR, __increase_capacity_internal)

static int SLIB__VECTOR_PREPARE_CAPACITY_INTERNAL(SLIB_VECTOR* const vec, const size_t required) {
    slib_vecerr = 0;
    if (vec->cap < required) {
        size_t new_cap = vec->cap ? vec->cap : SLIB_VECTOR_INITIAL_CAP;
        do new_cap *= 2; while (new_cap < required);
        SLIB_VECTOR_TYPE* const new_data = realloc(vec->data, new_cap * sizeof(SLIB_VECTOR_TYPE));
        if (!new_data) {
            slib_vecerr = 1;
            return 0;
        }
        vec->data = new_data;
        vec->cap  = new_cap;

    }
    return 1;
}

// API
SLIB_API void SLIB_VECTOR_RESERVE_M(SLIB_VECTOR* const vec, size_t amount) {
    slib_vecerr = 0;
    if (vec->cap < amount) {
        SLIB_VECTOR_TYPE* const new_data = realloc(vec->data, amount * sizeof(SLIB_VECTOR_TYPE));
        if (!new_data) {
            slib_vecerr = 1;
            return;
        };
        vec->data = new_data;
        vec->cap  = amount;
    }
}

SLIB_API SLIB_VECTOR_TYPE* SLIB_VECTOR_GROW_M(SLIB_VECTOR* const vec, size_t amount) {
    const size_t required = vec->size + amount;
    if (SLIB__VECTOR_PREPARE_CAPACITY_INTERNAL(vec, required)) {
        SLIB_VECTOR_TYPE* const ret = vec->data + vec->size;
        vec->size += amount;
        return ret;
    }
    return NULL;
}

SLIB_API void SLIB_VECTOR_APPEND_M(SLIB_VECTOR* const vec, const SLIB_VECTOR_TYPE elem) {
    if (SLIB__VECTOR_PREPARE_CAPACITY_INTERNAL(vec, vec->size + 1)) {
        vec->data[vec->size++] = elem;
    }
}

SLIB_API void SLIB_VECTOR_APPEND_MANY_M(SLIB_VECTOR* const dst, const SLIB_VECTOR* const src) {
    const size_t new_size = dst->size + src->size;
    if (SLIB__VECTOR_PREPARE_CAPACITY_INTERNAL(dst, new_size)) {
        memcpy(dst->data + dst->size, src->data, src->size * sizeof(SLIB_VECTOR_TYPE));
        dst->size = new_size;
    }
}

SLIB_API void SLIB_VECTOR_APPEND_BUFFER_M(SLIB_VECTOR* const dst, const SLIB_VECTOR_TYPE* const src, const size_t size) {
    const size_t new_size = dst->size + size;
    if (SLIB__VECTOR_PREPARE_CAPACITY_INTERNAL(dst, new_size)) {
        memcpy(dst->data + dst->size, src, size * sizeof(SLIB_VECTOR_TYPE));
        dst->size = new_size;
    }
}

SLIB_API void SLIB_VECTOR_FREE_M(SLIB_VECTOR* const vec) {
    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->cap  = 0;
}

SLIB_API SLIB_VECTOR_TYPE* SLIB_VECTOR_FIRST_M(SLIB_VECTOR* const vec, SLIB_VECTOR_COMPARATOR comparator) {
    const SLIB_VECTOR_TYPE* const end = vec->data + vec->size;
    for (SLIB_VECTOR_TYPE* it = vec->data; it < end; it++) {
        if (comparator(it)) {
            return it;
        }
    }
    return NULL;
}

SLIB_API SLIB_VECTOR_ITERATOR SLIB_VECTOR_ITERATOR_MAKE_M(const SLIB_VECTOR* vec) {
    return (SLIB_VECTOR_ITERATOR) {
        .vec = vec,
        .next = vec->data
    };
}

SLIB_API SLIB_VECTOR_TYPE* SLIB_VECTOR_NEXT_M(SLIB_VECTOR_ITERATOR* vec_iter, SLIB_VECTOR_COMPARATOR comparator) {
    const SLIB_VECTOR_TYPE* const end = vec_iter->vec->data + vec_iter->vec->size;
    while (vec_iter->next < end) {
        if (comparator(vec_iter->next)) {
            return vec_iter->next++;
        }
        vec_iter->next++;
    }
    return NULL;
}

#undef SLIB__VECTOR_PREPARE_CAPACITY_INTERNAL

#endif // SLIB_IMPLEMENTATION

#undef SLIB_VECTOR_TYPE

#undef SLIB_VECTOR

#undef SLIB_VECTOR_RESERVE_M
#undef SLIB_VECTOR_GROW_M
#undef SLIB_VECTOR_APPEND_M
#undef SLIB_VECTOR_APPEND_MANY_M
#undef SLIB_VECTOR_APPEND_BUFFER_M
#undef SLIB_VECTOR_FREE_M
