#ifndef _SLIB_ARENA_C
#define _SLIB_ARENA_C

/**
 * slib_arena.h provides a dynamic arena
 * 
 * The Arena object is basically a dynamic array that grows
 * when more space is required. Allocations return an index
 * in the Arena to avoid use-after-free situations.
 * 
 * If the Arena fails to grow, it remains as is and the
 * returned index is SIZE_MAX.
 * 
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef STRUCTLIBDEF
    #define STRUCTLIBDEF
#endif

#ifndef ARENA_ALLOC 
    #define ARENA_ALLOC malloc
#endif // ARENA_ALLOC

#ifndef ARENA_DEALLOC 
    #define ARENA_DEALLOC free
#endif // ARENA_ALLOC

#ifndef ARENA_REALLOC
    #define ARENA_REALLOC realloc
#endif // ARENA_REALLOC

typedef struct {
    uint8_t* data;
    size_t capacity;
    size_t size;
} Arena;

// Index arithmetics for arena
#define arena_incr(_T, _index, _incr) ((_index)+(_incr)*sizeof(_T))

// Translates index to pointer
#define arena_get(_T, _a, _index) ((_T*)((_a).data+(_index)))

// Translates index to pointer with offset index (i.e. for arrays)
#define arena_offset(_T, _a, _index, _off) arena_get(_T, _a, _index + (_off)*sizeof(_T))

STRUCTLIBDEF void arena_init(Arena* const arena, const size_t capacity);
STRUCTLIBDEF size_t arena_alloc(Arena* const arena, size_t count);
STRUCTLIBDEF void arena_reset(Arena* const arena);
STRUCTLIBDEF void arena_deinit(Arena* const arena);

#ifdef SLIB_ARENA_IMPL

STRUCTLIBDEF void arena_init(Arena* const arena, const size_t capacity) {
    arena->data     = (uint8_t*)ARENA_ALLOC(capacity); // gcc complaining
    arena->capacity = capacity;
    arena->size     = 0;
}

STRUCTLIBDEF size_t arena_alloc(Arena* const arena, size_t count) {
    const size_t required = arena->size + count; 
    if (required > arena->capacity) {
        size_t cap = arena->capacity;
        do cap *= 2; while (required > cap);
        uint8_t* const reallocated = (uint8_t*)ARENA_REALLOC(arena->data, cap);
        if (!reallocated) {
            fprintf(stderr, __FILE__": failed to grow arena");
            return SIZE_MAX;
        }
        arena->data = reallocated;
        arena->capacity = cap;
    }
    size_t ret_index = arena->size;
    arena->size += count;
    return ret_index;
}

STRUCTLIBDEF void arena_reset(Arena* const arena) {
    arena->size = 0; // super easy!
}

STRUCTLIBDEF void arena_deinit(Arena* const arena) {
    ARENA_DEALLOC(arena->data);
    arena->data     = 0;
    arena->capacity = 0;
    arena->size     = 0;
}

#endif // SLIB_ARENA_IMPL

#endif // _SLIB_ARENA_C
