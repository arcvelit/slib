#ifndef _SLIB_LINKED_ARENA_C
#define _SLIB_LINKED_ARENA_C

/**
 * slib_linked_arena.h provides a general purpose pool allocator
 * 
 * Start with
 * LinkedArena a = {0};
 * linked_arena_init(&a);
 * 
 * Allocate with 
 * linked_arena_alloc       -> implements first-fit
 * linked_arena_alloc_tail  -> implements next-fit
 * 
 * Free memory or reset with
 * linked_arena_deinit
 * linked_arena_reset
 * 
 * */

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#ifndef STRUCTLIBDEF
# define STRUCTLIBDEF
#endif // STRUCTLIBDEF

#ifndef LINKED_ARENA_PAGE_CAP
# define LINKED_ARENA_PAGE_CAP 1024
#endif // LINKED_ARENA_PAGE_CAP

#ifndef LINKED_ARENA_ALLOC 
# define LINKED_ARENA_ALLOC malloc
#endif // LINKED_ARENA_ALLOC

#ifndef LINKED_ARENA_DEALLOC 
# define LINKED_ARENA_DEALLOC free
#endif // LINKED_ARENA_ALLOC

typedef struct LinkedArenaPage LinkedArenaPage;

typedef struct {
    LinkedArenaPage* head;
    LinkedArenaPage* current;
} LinkedArena;

// Initialize arena object with a page
STRUCTLIBDEF int linked_arena_init(LinkedArena* const arena);

// Allocate memory block with first-fit strategy
STRUCTLIBDEF void* linked_arena_alloc(LinkedArena* const arena, size_t count);

// Allocate memory block with next-fit strategy
STRUCTLIBDEF void* linked_arena_alloc_tail(LinkedArena* const arena, size_t count);

// Reset cursor to head (lazy)
STRUCTLIBDEF void linked_arena_reset(LinkedArena* const arena);

// Free the whole arena
STRUCTLIBDEF void linked_arena_deinit(LinkedArena* const arena);

// Count the number of allocate pages
STRUCTLIBDEF size_t linked_arena_page_count(const LinkedArena* const arena);

#ifdef SLIB_LINKED_ARENA_IMPLEMENTATION

#ifndef SLIB_IGNORE_ALIGNMENT_WARNINGS
static_assert(LINKED_ARENA_PAGE_CAP % 8 == 0, "pages should be 8 byte alligned");
#endif // SLIB_IGNORE_ALIGNMENT_WARNINGS

struct LinkedArenaPage {
    uint8_t data[LINKED_ARENA_PAGE_CAP];
    LinkedArenaPage* next;
    uint8_t* cursor;
};

STRUCTLIBDEF int linked_arena_init(LinkedArena* const arena) {
    arena->head = LINKED_ARENA_ALLOC(sizeof(LinkedArenaPage));
    if (!arena->head) {
        fprintf(stderr, __FILE__": failed to init arena\n");
        return 0;
    }
    arena->head->cursor = arena->head->data;
    arena->head->next   = NULL;
    arena->current      = arena->head;
    return 1;
}

static inline LinkedArenaPage* _internal_prepare_new_page() {
    LinkedArenaPage* const page = LINKED_ARENA_ALLOC(sizeof(LinkedArenaPage));
    if (!page) {
        fprintf(stderr, __FILE__": failed to grow arena\n");
        return NULL;
    }
    page->cursor = page->data;
    page->next   = NULL;
    return page;
}

static inline void* _internal_page_bump(LinkedArenaPage* page, size_t count) {
    void* const block = page->cursor;
    page->cursor += count;
    return block;
}

static inline void* _internal_alloc_tail(LinkedArena* arena, size_t count) {
    const size_t size = arena->current->cursor - arena->current->data;
    if (size + count > LINKED_ARENA_PAGE_CAP) {
        if (!arena->current->next) {
            LinkedArenaPage* const page = _internal_prepare_new_page();
            if (!page) return NULL;
            arena->current->next = page;
        }
        // lazy reset current+1 page
        arena->current = arena->current->next;
        arena->current->cursor = arena->current->data;
    }
    return _internal_page_bump(arena->current, count);    
}

STRUCTLIBDEF void* linked_arena_alloc(LinkedArena* const arena, size_t count) {
    assert(LINKED_ARENA_PAGE_CAP >= count && "page size is to small");
    LinkedArenaPage* page = arena->head;
    while (page != arena->current) {
        const size_t size = page->cursor - page->data;
        if (size + count <= LINKED_ARENA_PAGE_CAP) {
            return _internal_page_bump(page, count);
        }
        page = page->next;
    }
    return _internal_alloc_tail(arena, count);
}

STRUCTLIBDEF void* linked_arena_alloc_tail(LinkedArena* const arena, size_t count) {
    assert(LINKED_ARENA_PAGE_CAP >= count && "page size is to small");
    return _internal_alloc_tail(arena, count);
}

STRUCTLIBDEF void linked_arena_reset(LinkedArena* const arena) {
    arena->current         = arena->head;
    arena->current->cursor = arena->head->data;
}

STRUCTLIBDEF void linked_arena_deinit(LinkedArena* const arena) {
    LinkedArenaPage* page = arena->head;
    while (page) {
        LinkedArenaPage* next = page->next;
        LINKED_ARENA_DEALLOC(page);
        page = next;
    }
    arena->head    = NULL;
    arena->current = NULL;
}

STRUCTLIBDEF size_t linked_arena_page_count(const LinkedArena* const arena) {
    size_t pages = 0;
    const LinkedArenaPage* p;
    for (p = arena->head; p; p = p->next) {
        pages++;
    }
    return pages;
}

#endif // SLIB_LINKED_ARENA_IMPLEMENTATION

#endif // _SLIB_LINKED_ARENA_C
