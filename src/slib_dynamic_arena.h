#ifndef _SLIB_DYNAMIC_ARENA_C
#define _SLIB_DYNAMIC_ARENA_C

/**
 * slib_dynamic_arena.h provides a dynamic arena
 * 
 * The DynamicArena object is basically a dynamic array that grows
 * when more space is required. Allocations return an index
 * in the DynamicArena to avoid use-after-free situations.
 * 
 * If the DynamicArena fails to grow, it remains as is and the
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

#ifndef DYNAMIC_ARENA_INITIAL_CAP
# define DYNAMIC_ARENA_INITIAL_CAP 1024
#endif // DYNAMIC_ARENA_INITIAL_CAP

#ifndef ARENA_ALLOC 
# define ARENA_ALLOC malloc
#endif // ARENA_ALLOC

#ifndef ARENA_DEALLOC 
# define ARENA_DEALLOC free
#endif // ARENA_ALLOC

#ifndef ARENA_REALLOC
# define ARENA_REALLOC realloc
#endif // ARENA_REALLOC

typedef struct {
    uint8_t* data;
    size_t capacity;
    size_t size;
} DynamicArena;

// Index arithmetics for arena
#define arena_incr(_T, _index, _incr) ((_index)+(_incr)*sizeof(_T))

// Translates index to pointer
#define arena_get(_T, _a, _index) ((_T*)((_a).data+(_index)))

// Translates index to pointer with offset index (i.e. for arrays)
#define arena_offset(_T, _a, _index, _off) arena_get(_T, _a, _index + (_off)*sizeof(_T))

// Initialize arena object with initial capacity
STRUCTLIBDEF int dynamic_arena_init(DynamicArena* const arena);

// Allocate a block and get its index (fail: SIZE_MAX)
STRUCTLIBDEF size_t dynamic_arena_alloc(DynamicArena* const arena, size_t count);

// Reset the size to zero (lazy)
STRUCTLIBDEF void dynamic_arena_reset(DynamicArena* const arena);

// Free the whole arena
STRUCTLIBDEF void dynamic_arena_deinit(DynamicArena* const arena);

#ifdef SLIB_DYNAMIC_ARENA_IMPLEMENTATION

STRUCTLIBDEF int dynamic_arena_init(DynamicArena* const arena) {
    arena->data = ARENA_ALLOC(DYNAMIC_ARENA_INITIAL_CAP);
    if (!arena->data) {
        fprintf(stderr, __FILE__": failed to init arena");
        return 0;
    }
    arena->capacity = DYNAMIC_ARENA_INITIAL_CAP;
    arena->size = 0;
    return 1;
}

STRUCTLIBDEF size_t dynamic_arena_alloc(DynamicArena* const arena, size_t count) {
    const size_t required = arena->size + count; 
    if (required > arena->capacity) {
        size_t cap = arena->capacity;
        do cap *= 2; while (required > cap);
        uint8_t* const reallocated = ARENA_REALLOC(arena->data, cap);
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

STRUCTLIBDEF void dynamic_arena_reset(DynamicArena* const arena) {
    arena->size = 0; // super easy!
}

STRUCTLIBDEF void dynamic_arena_deinit(DynamicArena* const arena) {
    ARENA_DEALLOC(arena->data);
    arena->data     = NULL;
    arena->capacity = 0;
    arena->size     = 0;
}

#endif // SLIB_DYNAMIC_ARENA_IMPLEMENTATION

#endif // _SLIB_DYNAMIC_ARENA_C
