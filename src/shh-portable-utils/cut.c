#include <string.h>

#include <skalibs/buffer.h>
#include <skalibs/skamisc.h>
#include <skalibs/disize.h>
#include <skalibs/sgetopt.h>
#include <skalibs/stralloc.h>
#include <skalibs/genalloc.h>
#include <skalibs/djbunix.h>
#include <skalibs/strerr2.h>

#define USAGE "cut -b list [-n] [file...] | -c list [file...] | -f list [-d delim] [-s] [file..]"

typedef void (*cut_func_t)(buffer*, genalloc*, char, int);

void byte_cut(buffer*, genalloc*, char, int);

void field_cut(buffer*, genalloc*, char, int);

void parse(genalloc*, char*);

int main (int argc, char const *const *argv)
{
    cut_func_t cut;
    char buf[BUFFER_INSIZE];
    char delim = '\t';
    int mode_was_chosen = 0;
    int supress_nodelim = 0;
    genalloc what = GENALLOC_ZERO;
    buffer buffer_in_;
    buffer *buffer_in;
    subgetopt l = SUBGETOPT_ZERO;
    PROG = "cut";

    for (;;) {
        int opt = subgetopt_r(argc, argv, "b:c:f:nd:s", &l);
        if (opt == -1)
            break;
        switch (opt) {
            case 'c':
                /* Fallthrough. This is a big TODO: supporting characters
                 * as per POSIX requires locale support, which I plan to
                 * add later. Locale support will be configurable at build time:
                 * embedded systems do not need this feature, but locales
                 * apparently will impose additional code and complexity.
                 */
            case 'b':
                if (mode_was_chosen)
                    strerr_dieusage(100, USAGE);
                mode_was_chosen = 1;
                cut = byte_cut;
                parse(&what, l.arg);
                break;
            case 'f': 
                if (mode_was_chosen)
                    strerr_dieusage(100, USAGE);
                mode_was_chosen = 1;
                parse(&what, l.arg);
                cut = field_cut;
                break;
            case 'n':
                break; /* See comment on -c */
            case 'd':
                delim = *l.arg;
                break;
            case 's':
                supress_nodelim = 1;
                break;
            default:
                strerr_dieusage(100, USAGE);
        }
    }
    argc -= l.ind;
    argc += l.ind;

    if (!genalloc_len(disize, &what))
        strerr_dieusage(100, USAGE);
    return 0;
}
