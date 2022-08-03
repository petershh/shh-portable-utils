#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <skalibs/strerr2.h>
#include <skalibs/genalloc.h>
#include <skalibs/sgetopt.h>

#include "shhfuncs.h"

#define USAGE "mkdir [-p] [-m mode] dir..."

int main(int argc, char **argv)
{
    mode_t mask;
    mode_t mode = 0777;
    int failure = 0;
    int mkparent = 0;
    genalloc directives = GENALLOC_ZERO;
    subgetopt l = SUBGETOPT_ZERO;
    PROG = "mkdir";

    mask = umask(0);
    umask(mask);

    for (;;) {
        int opt = subgetopt_r(argc, (char const *const *)argv, "pm:", &l);
        if (opt == -1)
            break;
        switch (opt) {
            case 'p':
                mkparent = 1;
                break;
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
                    mode = change_mode(0777, genalloc_s(chmod_directive,
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

    for (char **path = argv; *path; path++) {
        if (mkparent) {
            char *afterslash = *path + 1;
            char save;
            for (;;) {
                afterslash = strchr(afterslash, '/');
                if (!afterslash) {
                    if (mkdir(*path, mode)) {
                        if (errno != EEXIST) {
                            strerr_warnwu2sys("mkdir ", *path);
                            failure = 1;
                        }
                    }
                    break;
                }
                afterslash++;
                save = *afterslash;
                *afterslash = '\0';
                if (mkdir(*path, 0)) {
                    if (errno != EEXIST) {
                        strerr_warnwu2sys("mkdir ", *path);
                        failure = 1;
                        break;
                    }
                    *afterslash = save;
                    continue;
                }
                if (chmod(*path, (S_IWUSR | S_IXUSR | ~mask) & 0777)) {
                    strerr_warnwu2sys("chmod ", *path);
                    failure = 1;
                    break;
                }
                *afterslash = save;
            }
        } else {
            if (mkdir(*path, mode)) {
                strerr_warnwu2sys("mkdir ", *path);
                failure = 1;
            }
        }
    }
    return failure ? 111 : 0;
}
