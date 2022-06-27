#include <string.h>
#include "shhfuncs.h"

size_t byte_notin(char const *s, size_t n, char const *sep, size_t len)
{
    char const *t = s;
    while (n) {
        n--;
        if (!memchr(sep, *t, len))
            break;
        t++;
    }
    return t - s;
}
