#ifndef _SLIB_ARENA_C
#define _SLIB_ARENA_C

/**
 * Arena.h provides a general purpose pool allocator
 * 
 * Upon initializing, the arena allocates a page
 * 
 * Arena a = {0};
 * arena_init(&a, 1024);
 * 
 * Arena.current: [ Page 1 ] -> null
 * |____.head:      null
 * 
 * When a page reaches full capacity or is too full to allocate
 * it allocates a new page and adds it to the linked list
 * 
 * Arena.current: [ Page 2 ] -> null
 * |___ .head:    [ Page 1 ] -> [ Page 2 ] -> null
 * 
 * Calling reset puts the cursor back to the head thus 'clearing' the Arena
 * 
 * Arena.current: [ Page 1 ] -> ...
 * |___ .head:    [ Page 1 ] -> [ Page 2 ] -> null
 *
 * */

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef STRUCTLIBDEF
    #define STRUCTLIBDEF
#endif

#ifndef GLOBAL_ARENA_PAGE_CAP
    #define GLOBAL_ARENA_PAGE_CAP 1024
#endif // GLOBAL_ARENA_PAGE_CAP

#ifndef ARENA_ALLOCATOR 
    #define ARENA_ALLOCATOR malloc
#endif // ARENA_ALLOCATOR

typedef struct Arena_Page Arena_Page;
struct Arena_Page {
    uint8_t data[GLOBAL_ARENA_PAGE_CAP];
    Arena_Page*  next;
};

typedef struct {
    Arena_Page* head;
    Arena_Page* current;
    uint8_t* cursor;
    size_t page_cap;
} Arena;

STRUCTLIBDEF void arena_init(Arena* const arena, const size_t page_cap);
STRUCTLIBDEF uint8_t* arena_alloc(Arena* const arena, size_t count);
STRUCTLIBDEF void arena_reset(Arena* const arena);

#ifdef SLIB_ARENA_IMPLEMENTATION

STRUCTLIBDEF void arena_init(Arena* const arena, const size_t page_cap) {
    arena->head       = (Arena_Page*)ARENA_ALLOCATOR(sizeof(Arena_Page));
    arena->head->next = 0;
    arena->current    = arena->head;
    arena->cursor     = &arena->head->data[0];
    arena->page_cap   = page_cap;
}

STRUCTLIBDEF uint8_t* arena_alloc(Arena* const arena, size_t count) {
    const size_t page_size = arena->cursor - arena->current->data;
    if (page_size + count > arena->page_cap) {
        if (count > arena->page_cap) {
            fprintf(stderr, "error: arena page capacity is too small\n");
            return 0;
        }
        Arena_Page* next_page = arena->current->next;
        if (!next_page) {
            next_page = (Arena_Page*)ARENA_ALLOCATOR(sizeof(Arena_Page));
            if (!next_page) {
                fprintf(stderr, "error: failed to allocate arena page\n");
                return 0;
            }
            next_page->next = 0;
            arena->current->next = next_page;
        }
        arena->current       = next_page;
        arena->cursor        = &next_page->data[0];
    }
    uint8_t* const alloc_cursor = arena->cursor;
    arena->cursor = alloc_cursor + count;
    return alloc_cursor;
}

STRUCTLIBDEF void arena_reset(Arena* const arena) {
    arena->current =  arena->head;
    arena->cursor  = &arena->head->data[0];
}

#endif // SLIB_ARENA_IMPLEMENTATION

#endif // _SLIB_ARENA_C