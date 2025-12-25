#ifndef _SLIB_SLIB_LINKED_ARENA_C
#define _SLIB_SLIB_LINKED_ARENA_C

/**
 * linked_arena.h provides a general purpose pool allocator
 * 
 * Start with
 * slib_linked_arena a = {0};
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

typedef struct slib_linked_arena_page slib_linked_arena_page;

typedef struct {
    slib_linked_arena_page* head;
    slib_linked_arena_page* current;
} slib_linked_arena;

// Initialize arena object with a page
STRUCTLIBDEF int slib_linked_arena_init(slib_linked_arena* const arena);

// Allocate memory block with first-fit strategy
STRUCTLIBDEF void* slib_linked_arena_alloc(slib_linked_arena* const arena, size_t count);

// Allocate memory block with next-fit strategy
STRUCTLIBDEF void* slib_linked_arena_alloc_tail(slib_linked_arena* const arena, size_t count);

// Reset cursor to head (lazy)
STRUCTLIBDEF void slib_linked_arena_reset(slib_linked_arena* const arena);

// Free the whole arena
STRUCTLIBDEF void slib_linked_arena_deinit(slib_linked_arena* const arena);

// Count the number of allocate pages
STRUCTLIBDEF size_t slib_linked_arena_page_count(const slib_linked_arena* const arena);

#ifdef SLIB_STRIP_PREFIXES
typedef  slib_linked_arena       linked_arena;
# define linked_arena_init       slib_linked_arena_init
# define linked_arena_alloc      slib_linked_arena_alloc
# define linked_arena_alloc_tail slib_linked_arena_alloc_tail
# define linked_arena_reset      slib_linked_arena_reset
# define linked_arena_deinit     slib_linked_arena_deinit
# define linked_arena_page_count slib_linked_arena_page_count
#endif // SLIB_STRIP_PREFIXES

#ifdef SLIB_IMPLEMENTATION

#if SLIB_LINKED_ARENA_PAGE_CAP % 8 != 0
# warning "arena pages should be 8-byte aligned"
#endif 

struct slib_linked_arena_page {
    uint8_t data[SLIB_LINKED_ARENA_PAGE_CAP];
    slib_linked_arena_page* next;
    uint8_t* cursor;
};

STRUCTLIBDEF int slib_linked_arena_init(slib_linked_arena* const arena) {
    arena->head = malloc(sizeof(slib_linked_arena_page));
    if (!arena->head) {
        fprintf(stderr, __FILE__": failed to init arena\n");
        return 0;
    }
    arena->head->cursor = arena->head->data;
    arena->head->next   = NULL;
    arena->current      = arena->head;
    return 1;
}

static inline slib_linked_arena_page* _internal_prepare_new_page() {
    slib_linked_arena_page* const page = malloc(sizeof(slib_linked_arena_page));
    if (!page) {
        fprintf(stderr, __FILE__": failed to grow arena\n");
        return NULL;
    }
    page->cursor = page->data;
    page->next   = NULL;
    return page;
}

static inline void* _internal_page_bump(slib_linked_arena_page* page, size_t count) {
    void* const block = page->cursor;
    page->cursor += count;
    return block;
}

static inline void* _internal_alloc_tail(slib_linked_arena* arena, size_t count) {
    const size_t size = arena->current->cursor - arena->current->data;
    if (size + count > SLIB_LINKED_ARENA_PAGE_CAP) {
        if (!arena->current->next) {
            slib_linked_arena_page* const page = _internal_prepare_new_page();
            if (!page) return NULL;
            arena->current->next = page;
        }
        // lazy reset current+1 page
        arena->current = arena->current->next;
        arena->current->cursor = arena->current->data;
    }
    return _internal_page_bump(arena->current, count);    
}

STRUCTLIBDEF void* slib_linked_arena_alloc(slib_linked_arena* const arena, size_t count) {
    assert(SLIB_LINKED_ARENA_PAGE_CAP >= count && "page size is to small");
    slib_linked_arena_page* page = arena->head;
    while (page != arena->current) {
        const size_t size = page->cursor - page->data;
        if (size + count <= SLIB_LINKED_ARENA_PAGE_CAP) {
            return _internal_page_bump(page, count);
        }
        page = page->next;
    }
    return _internal_alloc_tail(arena, count);
}

STRUCTLIBDEF void* slib_linked_arena_alloc_tail(slib_linked_arena* const arena, size_t count) {
    assert(SLIB_LINKED_ARENA_PAGE_CAP >= count && "page size is to small");
    return _internal_alloc_tail(arena, count);
}

STRUCTLIBDEF void slib_linked_arena_reset(slib_linked_arena* const arena) {
    arena->current         = arena->head;
    arena->current->cursor = arena->head->data;
}

STRUCTLIBDEF void slib_linked_arena_deinit(slib_linked_arena* const arena) {
    slib_linked_arena_page* page = arena->head;
    while (page) {
        slib_linked_arena_page* next = page->next;
        free(page);
        page = next;
    }
    arena->head    = NULL;
    arena->current = NULL;
}

STRUCTLIBDEF size_t slib_linked_arena_page_count(const slib_linked_arena* const arena) {
    size_t pages = 0;
    const slib_linked_arena_page* p;
    for (p = arena->head; p; p = p->next) {
        pages++;
    }
    return pages;
}

#endif // SLIB_IMPLEMENTATION

#endif // _SLIB_SLIB_LINKED_ARENA_C
