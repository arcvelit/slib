#include <slib/sv.h>

SLIB_API slib_sv slib_sv_make(char* data, size_t size) {
    return (slib_sv) {
        .data = data,
        .size = size
    };
}
