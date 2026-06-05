#include <slib/arena.h>

#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

struct slib_arena_page {
    uint8_t data[SLIB_ARENA_PAGE_CAP];
    slib_arena_page* next;
    uint8_t* cursor;
};

SLIB_API int slib_arena_init(slib_arena* arena) {
    arena->head = malloc(sizeof(slib_arena_page));
    assert(arena->head && "error: failed to init slib_arena");

    arena->head->cursor = arena->head->data;
    arena->head->next   = NULL;
    arena->current      = arena->head;
    return 1;
}

static inline slib_arena_page* slib__internal_prepare_new_page() {
    slib_arena_page* const page = malloc(sizeof(slib_arena_page));
    assert(page && "error: failed to grow slib_arena");
    
    page->cursor = page->data;
    page->next   = NULL;
    return page;
}

static inline void* slib__internal_page_bump(slib_arena_page* page, size_t count) {
    void* const block = page->cursor;
    page->cursor += count;
    return block;
}

static inline void* slib__internal_alloc_tail(slib_arena* arena, size_t count) {
    const size_t size = arena->current->cursor - arena->current->data;
    if (size + count > SLIB_ARENA_PAGE_CAP) {
        if (!arena->current->next) {
            slib_arena_page* const page = slib__internal_prepare_new_page();
            if (!page) return NULL;
            arena->current->next = page;
        }
        // lazy reset current+1 page
        arena->current = arena->current->next;
        arena->current->cursor = arena->current->data;
    }
    return slib__internal_page_bump(arena->current, count);    
}

SLIB_API void* slib_arena_alloc(slib_arena* arena, size_t count) {
    assert(SLIB_ARENA_PAGE_CAP >= count && "page size is to small");
    slib_arena_page* page = arena->head;
    while (page != arena->current) {
        const size_t size = page->cursor - page->data;
        if (size + count <= SLIB_ARENA_PAGE_CAP) {
            return slib__internal_page_bump(page, count);
        }
        page = page->next;
    }
    return slib__internal_alloc_tail(arena, count);
}

SLIB_API void* slib_arena_alloc_tail(slib_arena* arena, size_t count) {
    assert(SLIB_ARENA_PAGE_CAP >= count && "page size is to small");
    return slib__internal_alloc_tail(arena, count);
}

SLIB_API void slib_arena_reset(slib_arena* arena) {
    arena->current         = arena->head;
    arena->current->cursor = arena->head->data;
}

SLIB_API void slib_arena_cleanup(slib_arena* arena) {
    slib_arena_page* page = arena->head;
    while (page) {
        slib_arena_page* next = page->next;
        free(page);
        page = next;
    }
    arena->head    = NULL;
    arena->current = NULL;
}

SLIB_API size_t slib_arena_page_count(const slib_arena* arena) {
    size_t pages = 0;
    const slib_arena_page* p;
    for (p = arena->head; p; p = p->next) {
        pages++;
    }
    return pages;
}
