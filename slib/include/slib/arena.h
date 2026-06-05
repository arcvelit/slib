#ifndef SLIB_ARENA_H
#define SLIB_ARENA_H

// arena.h (static)

#include <stddef.h>

#ifndef SLIB_API
# define SLIB_API
#endif

#ifndef SLIB_ARENA_PAGE_CAP
# define SLIB_ARENA_PAGE_CAP 1024
#endif

#if SLIB_ARENA_PAGE_CAP % 8 != 0
# warning "arena pages should be 8-byte aligned"
#endif

typedef struct slib_arena_page slib_arena_page;

typedef struct {
    slib_arena_page* head;
    slib_arena_page* current;
} slib_arena;

// Initialize arena object with a page
SLIB_API int slib_arena_init(slib_arena* arena);

// Allocate memory block with first-fit strategy
SLIB_API void* slib_arena_alloc(slib_arena* arena, size_t count);

// Allocate memory block with next-fit strategy (quick)
SLIB_API void* slib_arena_alloc_tail(slib_arena* arena, size_t count);

// Reset cursor to head (lazy)
SLIB_API void slib_arena_reset(slib_arena* arena);

// Free the area and return to pre init state
SLIB_API void slib_arena_cleanup(slib_arena* arena);

// Count the number of allocate pages
SLIB_API size_t slib_arena_page_count(const slib_arena* arena);

#ifdef SLIB_STRIP_PREFIXES
typedef  slib_arena       arena;
# define arena_init       slib_arena_init
# define arena_alloc      slib_arena_alloc
# define arena_alloc_tail slib_arena_alloc_tail
# define arena_reset      slib_arena_reset
# define arena_cleanup     slib_arena_cleanup
# define arena_page_count slib_arena_page_count
#endif

#endif // SLIB_ARENA_H
