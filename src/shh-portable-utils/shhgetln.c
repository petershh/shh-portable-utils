#include <skalibs/buffer.h>
#include <skalibs/skamisc.h>
#include <skalibs/stralloc.h>

#include "shhfuncs.h"

int shhgetln(buffer *b, stralloc *sa, char sep)
{
    size_t start = sa->len;
    for (;;) {
        ssize_t r = skagetln_nofill(b, sa, sep);
        if (r)
            return r;
        r = buffer_fill(b);
        if (r < 0)
            return r;
        if (!r) {
            if (sa->s && (sa->len > start)) {
                if (!stralloc_append(sa, sep))
                    return -1;
                return 1;
            } else
                return 0;
        }
    }
}
