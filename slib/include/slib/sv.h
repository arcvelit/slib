#ifndef SLIB_SV_H
#define SLIB_SV_H

// sv.h (static)

#include <stddef.h>

#ifndef SLIB_API
# define SLIB_API
#endif

typedef struct {
    char*  data;
    size_t size;
} slib_sv;

// Create a string view
SLIB_API slib_sv slib_sv_make(char* data, size_t size);

#ifdef SLIB_STRIP_PREFIXES
typedef  slib_sv sv;
# define sv_make slib_sv_make
#endif


#endif // SLIB_SV_H