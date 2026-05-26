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

STRUCTLIBDEF int slib_arena_init(slib_arena* const arena) {
    arena->head = malloc(sizeof(slib_arena_page));
    if (!arena->head) {
        fprintf(stderr, __FILE__": failed to init arena\n");
        return 0;
    }
    arena->head->cursor = arena->head->data;
    arena->head->next   = NULL;
    arena->current      = arena->head;
    return 1;
}

static inline slib_arena_page* _internal_prepare_new_page() {
    slib_arena_page* const page = malloc(sizeof(slib_arena_page));
    if (!page) {
        fprintf(stderr, __FILE__": failed to grow arena\n");
        return NULL;
    }
    page->cursor = page->data;
    page->next   = NULL;
    return page;
}

static inline void* _internal_page_bump(slib_arena_page* page, size_t count) {
    void* const block = page->cursor;
    page->cursor += count;
    return block;
}

static inline void* _internal_alloc_tail(slib_arena* arena, size_t count) {
    const size_t size = arena->current->cursor - arena->current->data;
    if (size + count > SLIB_ARENA_PAGE_CAP) {
        if (!arena->current->next) {
            slib_arena_page* const page = _internal_prepare_new_page();
            if (!page) return NULL;
            arena->current->next = page;
        }
        // lazy reset current+1 page
        arena->current = arena->current->next;
        arena->current->cursor = arena->current->data;
    }
    return _internal_page_bump(arena->current, count);    
}

STRUCTLIBDEF void* slib_arena_alloc(slib_arena* const arena, size_t count) {
    assert(SLIB_ARENA_PAGE_CAP >= count && "page size is to small");
    slib_arena_page* page = arena->head;
    while (page != arena->current) {
        const size_t size = page->cursor - page->data;
        if (size + count <= SLIB_ARENA_PAGE_CAP) {
            return _internal_page_bump(page, count);
        }
        page = page->next;
    }
    return _internal_alloc_tail(arena, count);
}

STRUCTLIBDEF void* slib_arena_alloc_tail(slib_arena* const arena, size_t count) {
    assert(SLIB_ARENA_PAGE_CAP >= count && "page size is to small");
    return _internal_alloc_tail(arena, count);
}

STRUCTLIBDEF void slib_arena_reset(slib_arena* const arena) {
    arena->current         = arena->head;
    arena->current->cursor = arena->head->data;
}

STRUCTLIBDEF void slib_arena_deinit(slib_arena* const arena) {
    slib_arena_page* page = arena->head;
    while (page) {
        slib_arena_page* next = page->next;
        free(page);
        page = next;
    }
    arena->head    = NULL;
    arena->current = NULL;
}

STRUCTLIBDEF size_t slib_arena_page_count(const slib_arena* const arena) {
    size_t pages = 0;
    const slib_arena_page* p;
    for (p = arena->head; p; p = p->next) {
        pages++;
    }
    return pages;
}
