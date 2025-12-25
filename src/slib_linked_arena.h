#ifndef _SLIB_SLIB_LINKED_ARENA_C
#define _SLIB_SLIB_LINKED_ARENA_C

/**
 * slib_linked_arena.h provides a general purpose pool allocator
 * 
 * Start with
 * SLIBLinkedArena a = {0};
 * slib_linked_arena_init(&a);
 * 
 * Allocate with 
 * malloc       -> implements first-fit
 * malloc_tail  -> implements next-fit
 * 
 * Free memory or reset with
 * slib_linked_arena_deinit
 * slib_linked_arena_reset
 * 
 * */

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#ifndef STRUCTLIBDEF
# define STRUCTLIBDEF
#endif // STRUCTLIBDEF

#ifndef SLIB_LINKED_ARENA_PAGE_CAP
# define SLIB_LINKED_ARENA_PAGE_CAP 1024
#endif // SLIB_LINKED_ARENA_PAGE_CAP

typedef struct SLIBLinkedArenaPage SLIBLinkedArenaPage;

typedef struct {
    SLIBLinkedArenaPage* head;
    SLIBLinkedArenaPage* current;
} SLIBLinkedArena;

// Initialize arena object with a page
STRUCTLIBDEF int slib_linked_arena_init(SLIBLinkedArena* const arena);

// Allocate memory block with first-fit strategy
STRUCTLIBDEF void* slib_linked_arena_alloc(SLIBLinkedArena* const arena, size_t count);

// Allocate memory block with next-fit strategy
STRUCTLIBDEF void* slib_linked_arena_alloc_tail(SLIBLinkedArena* const arena, size_t count);

// Reset cursor to head (lazy)
STRUCTLIBDEF void slib_linked_arena_reset(SLIBLinkedArena* const arena);

// Free the whole arena
STRUCTLIBDEF void slib_linked_arena_deinit(SLIBLinkedArena* const arena);

// Count the number of allocate pages
STRUCTLIBDEF size_t slib_linked_arena_page_count(const SLIBLinkedArena* const arena);

#ifdef SLIB_STRIP_PREFIXES
typedef  SLIBLinkedArena         LinkedArena;
# define linked_arena_init       slib_linked_arena_init
# define linked_arena_alloc      slib_linked_arena_alloc
# define linked_arena_alloc_tail slib_linked_arena_alloc_tail
# define linked_arena_reset      slib_linked_arena_reset
# define linked_arena_deinit     slib_linked_arena_deinit
# define linked_arena_page_count slib_linked_arena_page_count
#endif // SLIB_STRIP_PREFIXES

#ifdef SLIB_LINKED_ARENA_IMPLEMENTATION

#ifndef SLIB_IGNORE_ALIGNMENT_WARNINGS
static_assert(SLIB_LINKED_ARENA_PAGE_CAP % 8 == 0, "pages should be 8 byte alligned");
#endif // SLIB_IGNORE_ALIGNMENT_WARNINGS

struct SLIBLinkedArenaPage {
    uint8_t data[SLIB_LINKED_ARENA_PAGE_CAP];
    SLIBLinkedArenaPage* next;
    uint8_t* cursor;
};

STRUCTLIBDEF int slib_linked_arena_init(SLIBLinkedArena* const arena) {
    arena->head = malloc(sizeof(SLIBLinkedArenaPage));
    if (!arena->head) {
        fprintf(stderr, __FILE__": failed to init arena\n");
        return 0;
    }
    arena->head->cursor = arena->head->data;
    arena->head->next   = NULL;
    arena->current      = arena->head;
    return 1;
}

static inline SLIBLinkedArenaPage* _internal_prepare_new_page() {
    SLIBLinkedArenaPage* const page = malloc(sizeof(SLIBLinkedArenaPage));
    if (!page) {
        fprintf(stderr, __FILE__": failed to grow arena\n");
        return NULL;
    }
    page->cursor = page->data;
    page->next   = NULL;
    return page;
}

static inline void* _internal_page_bump(SLIBLinkedArenaPage* page, size_t count) {
    void* const block = page->cursor;
    page->cursor += count;
    return block;
}

static inline void* _internal_alloc_tail(SLIBLinkedArena* arena, size_t count) {
    const size_t size = arena->current->cursor - arena->current->data;
    if (size + count > SLIB_LINKED_ARENA_PAGE_CAP) {
        if (!arena->current->next) {
            SLIBLinkedArenaPage* const page = _internal_prepare_new_page();
            if (!page) return NULL;
            arena->current->next = page;
        }
        // lazy reset current+1 page
        arena->current = arena->current->next;
        arena->current->cursor = arena->current->data;
    }
    return _internal_page_bump(arena->current, count);    
}

STRUCTLIBDEF void* slib_linked_arena_alloc(SLIBLinkedArena* const arena, size_t count) {
    assert(SLIB_LINKED_ARENA_PAGE_CAP >= count && "page size is to small");
    SLIBLinkedArenaPage* page = arena->head;
    while (page != arena->current) {
        const size_t size = page->cursor - page->data;
        if (size + count <= SLIB_LINKED_ARENA_PAGE_CAP) {
            return _internal_page_bump(page, count);
        }
        page = page->next;
    }
    return _internal_alloc_tail(arena, count);
}

STRUCTLIBDEF void* slib_linked_arena_alloc_tail(SLIBLinkedArena* const arena, size_t count) {
    assert(SLIB_LINKED_ARENA_PAGE_CAP >= count && "page size is to small");
    return _internal_alloc_tail(arena, count);
}

STRUCTLIBDEF void slib_linked_arena_reset(SLIBLinkedArena* const arena) {
    arena->current         = arena->head;
    arena->current->cursor = arena->head->data;
}

STRUCTLIBDEF void slib_linked_arena_deinit(SLIBLinkedArena* const arena) {
    SLIBLinkedArenaPage* page = arena->head;
    while (page) {
        SLIBLinkedArenaPage* next = page->next;
        free(page);
        page = next;
    }
    arena->head    = NULL;
    arena->current = NULL;
}

STRUCTLIBDEF size_t slib_linked_arena_page_count(const SLIBLinkedArena* const arena) {
    size_t pages = 0;
    const SLIBLinkedArenaPage* p;
    for (p = arena->head; p; p = p->next) {
        pages++;
    }
    return pages;
}

#endif // SLIB_LINKED_ARENA_IMPLEMENTATION

#endif // _SLIB_SLIB_LINKED_ARENA_C
