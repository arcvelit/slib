#ifndef _SLIB_DYNAMIC_ARENA_C
#define _SLIB_DYNAMIC_ARENA_C

/**
 * dynamic_arena.h provides a dynamic arena
 * 
 * The slib_dynamic_arena object is basically a dynamic array that grows
 * when more space is required. Allocations return an index
 * in the slib_dynamic_arena to avoid use-after-free situations.
 * 
 * If the slib_dynamic_arena fails to grow, it remains as is and the
 * returned index is SIZE_MAX.
 * 
 * This is probably super useless.
 * 
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef STRUCTLIBDEF
# define STRUCTLIBDEF
#endif

#ifndef SLIB_DYNAMIC_ARENA_INITIAL_CAP
# define SLIB_DYNAMIC_ARENA_INITIAL_CAP 1024
#endif // SLIB_DYNAMIC_ARENA_INITIAL_CAP

typedef struct {
    uint8_t* data;
    size_t capacity;
    size_t size;
} slib_dynamic_arena;

// Index arithmetics for arena
#define arena_incr(_T, _index, _incr) ((_index)+(_incr)*sizeof(_T))

// Translates index to pointer
#define arena_get(_T, _a, _index) ((_T*)((_a).data+(_index)))

// Translates index to pointer with offset index (i.e. for arrays)
#define arena_offset(_T, _a, _index, _off) arena_get(_T, _a, _index + (_off)*sizeof(_T))

// Initialize arena object with initial capacity
STRUCTLIBDEF int slib_dynamic_arena_init(slib_dynamic_arena* const arena);

// Allocate a block and get its index (fail: SIZE_MAX)
STRUCTLIBDEF size_t slib_dynamic_arena_alloc(slib_dynamic_arena* const arena, size_t count);

// Reset the size to zero (lazy)
STRUCTLIBDEF void slib_dynamic_arena_reset(slib_dynamic_arena* const arena);

// Free the whole arena
STRUCTLIBDEF void slib_dynamic_arena_deinit(slib_dynamic_arena* const arena);

#ifdef SLIB_STRIP_PREFIXES
typedef  slib_dynamic_arena   dynamic_arena;
# define dynamic_arena_init   slib_dynamic_arena_init
# define dynamic_arena_alloc  slib_dynamic_arena_alloc
# define dynamic_arena_reset  slib_dynamic_arena_reset
# define dynamic_arena_deinit slib_dynamic_arena_deinit
#endif // SLIB_STRIP_PREFIXES

#ifdef SLIB_IMPLEMENTATION

STRUCTLIBDEF int slib_dynamic_arena_init(slib_dynamic_arena* const arena) {
    arena->data = malloc(SLIB_DYNAMIC_ARENA_INITIAL_CAP);
    if (!arena->data) {
        fprintf(stderr, __FILE__": failed to init arena");
        return 0;
    }
    arena->capacity = SLIB_DYNAMIC_ARENA_INITIAL_CAP;
    arena->size = 0;
    return 1;
}

STRUCTLIBDEF size_t slib_dynamic_arena_alloc(slib_dynamic_arena* const arena, size_t count) {
    const size_t required = arena->size + count; 
    if (required > arena->capacity) {
        size_t cap = arena->capacity;
        do cap *= 2; while (required > cap);
        uint8_t* const reallocated = realloc(arena->data, cap);
        if (!reallocated) {
            fprintf(stderr, __FILE__": failed to grow arena");
            return SIZE_MAX;
        }
        arena->data = reallocated;
        arena->capacity = cap;
    }
    size_t block = arena->size;
    arena->size += count;
    return block;
}

STRUCTLIBDEF void slib_dynamic_arena_reset(slib_dynamic_arena* const arena) {
    arena->size = 0; // super easy!
}

STRUCTLIBDEF void slib_dynamic_arena_deinit(slib_dynamic_arena* const arena) {
    free(arena->data);
    arena->data     = NULL;
    arena->capacity = 0;
    arena->size     = 0;
}

#endif // SLIB_IMPLEMENTATION

#endif // _SLIB_DYNAMIC_ARENA_C
