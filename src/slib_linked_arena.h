#ifndef _SLIB_LINKED_ARENA_C
#define _SLIB_LINKED_ARENA_C

/**
 * slib_linked_arena.h provides a general purpose pool allocator
 * 
 * Upon initializing, the arena allocates a page
 * 
 * LinkedArena a = {0};
 * linked_arena_init(&a, 1024);
 * 
 * LinkedArena.current: [ Page 1 ] -> null
 * |____.head:      null
 * 
 * When a page reaches full capacity or is too full to allocate
 * it allocates a new page and adds it to the linked list
 * 
 * LinkedArena.current: [ Page 2 ] -> null
 * |___ .head:    [ Page 1 ] -> [ Page 2 ] -> null
 * 
 * Calling reset puts the cursor back to the head thus 'clearing' the LinkedArena
 * 
 * LinkedArena.current: [ Page 1 ] -> ...
 * |___ .head:    [ Page 1 ] -> [ Page 2 ] -> null
 * 
 * 
 * Tradeoff: if the required memory block is too large, it will jump to the next 
 * page, essential wasting space in the original page (not really "full").
 *
 * */

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#ifndef STRUCTLIBDEF
    #define STRUCTLIBDEF
#endif

#ifndef LINKED_ARENA_PAGE_CAP
    #define LINKED_ARENA_PAGE_CAP 1024
#endif // LINKED_ARENA_PAGE_CAP

#ifndef LINKED_ARENA_ALLOC 
    #define LINKED_ARENA_ALLOC malloc
#endif // LINKED_ARENA_ALLOC

#ifndef LINKED_ARENA_DEALLOC 
    #define LINKED_ARENA_DEALLOC free
#endif // LINKED_ARENA_ALLOC

typedef struct LinkedArenaPage LinkedArenaPage;
struct LinkedArenaPage {
    uint8_t data[LINKED_ARENA_PAGE_CAP];
    LinkedArenaPage* next;
};

typedef struct {
    LinkedArenaPage* head;
    LinkedArenaPage* current;
    uint8_t* cursor;
} LinkedArena;

STRUCTLIBDEF void linked_arena_init(LinkedArena* const arena);
STRUCTLIBDEF void* linked_arena_alloc(LinkedArena* const arena, size_t count);
STRUCTLIBDEF void linked_arena_reset(LinkedArena* const arena);
STRUCTLIBDEF void linked_arena_deinit(LinkedArena* const arena);
STRUCTLIBDEF size_t linked_arena_page_count(const LinkedArena* const arena);

#ifdef SLIB_LINKED_ARENA_IMPL

STRUCTLIBDEF void linked_arena_init(LinkedArena* const arena) {
    arena->head       = LINKED_ARENA_ALLOC(sizeof(LinkedArenaPage));
    arena->head->next = 0;
    arena->current    = arena->head;
    arena->cursor     = &arena->head->data[0];
}

STRUCTLIBDEF void* linked_arena_alloc(LinkedArena* const arena, size_t count) {
    assert(LINKED_ARENA_PAGE_CAP >= count && "page size is to small");
    const size_t page_size = arena->cursor - arena->current->data;
    if (page_size + count > arena->LINKED_ARENA_PAGE_CAP) {
        if (!arena->current->next) {
            LinkedArenaPage* const page =
            LINKED_ARENA_ALLOC(sizeof(LinkedArenaPage));
            if (!page) {
                fprintf(stderr, __FILE__": failed to grow arena\n");
                return 0;
            }
            page->next = 0;
            arena->current->next = page;
        }
        arena->current =  arena->current->next;
        arena->cursor  = &arena->current->data[0];
    }
    uint8_t* const ret_cursor = arena->cursor;
    arena->cursor = ret_cursor + count;
    return ret_cursor;
}

STRUCTLIBDEF void linked_arena_reset(LinkedArena* const arena) {
    arena->current =  arena->head;
    arena->cursor  = &arena->head->data[0];
}

STRUCTLIBDEF void linked_arena_deinit(LinkedArena* const arena) {
    LinkedArenaPage* page = arena->head;
    while (page) {
        LinkedArenaPage* next = page->next;
        LINKED_ARENA_DEALLOC(page);
        page = next;
    }
    arena->head    = 0;
    arena->cursor  = 0;
    arena->current = 0;
}

STRUCTLIBDEF size_t linked_arena_page_count(const LinkedArena* const arena) {
    size_t pages = 0;
    const LinkedArenaPage* p;
    for (p = arena->head; p; p = p->next) {
        pages++;
    }
    return pages;
}

#endif // SLIB_LINKED_ARENA_IMPL

#endif // _SLIB_LINKED_ARENA_C