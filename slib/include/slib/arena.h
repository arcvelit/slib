#ifndef _SLIB_SLIB_ARENA_C
#define _SLIB_SLIB_ARENA_C

// arena.h (static)

#include <stddef.h>

#ifndef STRUCTLIBDEF
# define STRUCTLIBDEF
#endif // STRUCTLIBDEF

#ifndef SLIB_ARENA_PAGE_CAP
# define SLIB_ARENA_PAGE_CAP 1024
#endif // SLIB_ARENA_PAGE_CAP

#if SLIB_ARENA_PAGE_CAP % 8 != 0
# warning "arena pages should be 8-byte aligned"
#endif

typedef struct slib_arena_page slib_arena_page;

typedef struct {
    slib_arena_page* head;
    slib_arena_page* current;
} slib_arena;

// Initialize arena object with a page
STRUCTLIBDEF int slib_arena_init(slib_arena* const arena);

// Allocate memory block with first-fit strategy
STRUCTLIBDEF void* slib_arena_alloc(slib_arena* const arena, size_t count);

// Allocate memory block with next-fit strategy (quick)
STRUCTLIBDEF void* slib_arena_alloc_tail(slib_arena* const arena, size_t count);

// Reset cursor to head (lazy)
STRUCTLIBDEF void slib_arena_reset(slib_arena* const arena);

// Free the whole arena
STRUCTLIBDEF void slib_arena_deinit(slib_arena* const arena);

// Count the number of allocate pages
STRUCTLIBDEF size_t slib_arena_page_count(const slib_arena* const arena);

#ifdef SLIB_STRIP_PREFIXES
typedef  slib_arena       arena;
# define arena_init       slib_arena_init
# define arena_alloc      slib_arena_alloc
# define arena_alloc_tail slib_arena_alloc_tail
# define arena_reset      slib_arena_reset
# define arena_deinit     slib_arena_deinit
# define arena_page_count slib_arena_page_count
#endif // SLIB_STRIP_PREFIXES

#endif // _SLIB_SLIB_ARENA_C
