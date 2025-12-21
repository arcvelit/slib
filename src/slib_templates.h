#ifndef _SLIB_TEMPLATES_C
#define _SLIB_TEMPLATES_C

// Welcome to my unreadable header...

#include <stdlib.h>
#include <assert.h>

// Neat trick #1: starting a comment at the end of a directive 
// allows to add semicolons without the compiler complaining!

#define SLIB_PAIR(_StructName, _First, _Fname, _Second, _Sname) \
typedef struct {\
    _First  _Fname;\
    _Second _Sname;\
} _StructName;//

#define SLIB_TRIPLET(_StructName, _First, _Fname, _Second, _Sname, _Third, _Tname) \
typedef struct {\
    _First  _Fname;\
    _Second _Sname;\
    _Third  _Tname;\
} _StructName;//

/**
 * Template for dynamic arrays
 * 
 * declare it global scope like this
 * SLIB_DA(Integers, int);
 * provides: .data, .size, .cap
 * ___
 * 
 * reserve method -> like in C++
 * 
 * grow    method -> returns pointer to appended block
 * # allows emplace for big structs #
 * 
 * append  method -> add elements
 * 
 * free    method -> free memory and sanitize 
 */

#define SLIB_DA_INITIAL_CAP 4
#define SLIB_DA(_StructName, _T)\
typedef struct {\
    _T*     data;\
    size_t  size;\
    size_t  cap;\
} _StructName;\
static inline int _StructName##_reserve(_StructName* const __struct, size_t __amount) {\
    if (__struct->cap < __amount) {\
        _T* const new_data = realloc(__struct->data, __amount * sizeof(_T));\
        if (!new_data) return 0;\
        __struct->data = new_data;\
        __struct->cap  = __amount;\
    }\
    return 1;\
}\
static inline _T* _StructName##_grow(_StructName* const __struct, size_t __amount) {\
    const size_t required = __struct->size + __amount;\
    if (required > __struct->cap) {\
        size_t new_cap = __struct->cap;\
        if (new_cap) {\
            do new_cap *= 2; while (new_cap < required);\
        } else { new_cap = required; }\
        _T* new_data = realloc(__struct->data, new_cap * sizeof(_T));\
        if (!new_data) return 0;\
        __struct->data = new_data;\
        __struct->cap  = new_cap;\
    }\
    _T* const ret = __struct->data + __struct->size;\
    __struct->size += __amount;\
    return ret;\
}\
static inline int _StructName##_append(_StructName* const __struct, const _T __e) {\
    if (__struct->size == __struct->cap) {\
        const size_t new_cap = __struct->cap ? __struct->cap * 2 : SLIB_DA_INITIAL_CAP;\
        _T* new_data = realloc(__struct->data, new_cap * sizeof(_T));\
        if (!new_data) return 0;\
        __struct->data = new_data;\
        __struct->cap  = new_cap;\
    }\
    __struct->data[__struct->size++] = __e;\
    return 1;\
}\
static inline void _StructName##_free(_StructName* const __struct) {\
    free(__struct->data);\
    __struct->data = 0;\
    __struct->size = 0;\
    __struct->cap  = 0;\
}//



#endif // _SLIB_TEMPLATES_C