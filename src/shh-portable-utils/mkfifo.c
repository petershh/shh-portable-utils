#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <skalibs/genalloc.h>
#include <skalibs/strerr2.h>
#include <skalibs/sgetopt.h>

#include "shhfuncs.h"

#define USAGE "mkfifo [-m mode] file ..."

int main(int argc, char const *const *argv)
{
    mode_t mask;
    mode_t mode = 0666;
    int failure = 0;
    genalloc directives = GENALLOC_ZERO;
    subgetopt l = SUBGETOPT_ZERO;
    PROG = "mkfifo";

    mask = umask(0);
    umask(mask);

    for (;;) {
        int opt = subgetopt_r(argc, argv, "m:", &l);
        if (opt == -1)
            break;
        switch (opt) {
            case 'm':
                if ('0' <= l.arg[0] && l.arg[0] <= '7') {
                    if (parse_mode_octal(l.arg, &mode))
                        strerr_dief2x(100, "invalid mode: ", l.arg);
                } else if (parse_mode_symbolic(l.arg, &directives)) {
                    if (errno == EINVAL)
                        strerr_dief2x(100, "invalid mode: ", l.arg);
                    else
                        strerr_diefu1sys(111, "parse mode");
                } else
                    mode = change_mode(0666, genalloc_s(chmod_directive,
                                                        &directives),
                                       genalloc_len(chmod_directive,
                                                    &directives),
                                       mask);
                break;
            default:
                strerr_dieusage(100, USAGE);
        }
    }
    argc -= l.ind;
    argv += l.ind;

    if (!argc)
        strerr_dieusage(100, USAGE);

    for (char const *const *name = argv; *name; name++)
        if (mkfifo(*name, mode)) {
            strerr_warnwu2sys("create fifo ", *name);
            failure = 1;
        }

    return failure ? 111 : 0;
}
