#include <errno.h>
#include <string.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <skalibs/sgetopt.h>
#include <skalibs/strerr2.h>
#include <skalibs/types.h>
#include <skalibs/skamisc.h>
#include <skalibs/stralloc.h>
#include <skalibs/genalloc.h>

#include "shhfuncs.h"

#define USAGE "chmod [-R] mode file..."

int traverse_dir(stralloc*, mode_t, chmod_directive*, size_t, mode_t);

int doit(char const*, mode_t, mode_t, chmod_directive*, size_t, mode_t);

int main(int argc, char const *const *argv)
{
    struct stat st;
    mode_t mask;
    mode_t mode = 0;
    int recurse = 0;
    int failure = 0;
    chmod_directive *d;
    size_t len;
    stralloc s = STRALLOC_ZERO;
    genalloc directives = GENALLOC_ZERO;
    subgetopt l = SUBGETOPT_ZERO;
    PROG = "chmod";
    for (;;) {
        int opt = subgetopt_r(argc, argv, "R", &l);
        if (opt == -1)
            break;
        switch (opt) {
            case 'R':
                recurse = 1;
                break;
            default:
                strerr_dieusage(100, USAGE);
        }
    }
    argc -= l.ind;
    argv += l.ind;

    if (argc < 2)
        strerr_dieusage(100, USAGE);

    mask = umask(0);
    umask(mask);

    if ('0' <= argv[0][0] && argv[0][0] <= '7') {
        if (parse_mode_octal(argv[0], &mode))
            strerr_dief2sys(100, "invalid mode: ", argv[0]);
    } else {
        if (parse_mode_symbolic(argv[0], &directives) == -1) {
            if (errno == EINVAL)
                strerr_diefu2x(100, "invalid mode: ", argv[0]);
            else
                strerr_diefu1sys(111, "parse mode");
        }
        if (!genalloc_len(chmod_directive, &directives))
            strerr_dieusage(100, USAGE);
    }

    d = genalloc_s(chmod_directive, &directives);
    len = genalloc_len(chmod_directive, &directives);

    for (char const *const *filename = argv + 1; *filename; filename++) {
        if (stat(*filename, &st)) {
            strerr_warnwu2sys("stat ", *filename);
            failure = 1;
            continue;
        }
        if (S_ISDIR(st.st_mode) && recurse) {
            satmp.len = 0;
            if (!stralloc_catb(&s, *filename, strlen(*filename) + 1)) {
                strerr_warnwu2sys("copy file name ", *filename);
                failure = 1;
                continue;
            }
            /* POSIX says that "//" behaviour is implementation-defined,
             * so remove redundant slash to avoid weird bugs.
             * indexing operation is safe, since stat would fail
             * on empty string, so s.len is >= 2 always.
             */
            if (s.s[s.len - 2] == '/') {
                s.s[s.len - 2] = '\0';
                s.len--;
            }
            failure = failure || traverse_dir(&s, mode, d, len, mask);
        }
        else {
            failure = failure || doit(*filename, st.st_mode, mode, d, len,
                                      mask);
        }
    }
    return failure ? 111 : 0;
}

int doit(char const *file, mode_t st_mode, mode_t mode,
         chmod_directive *directives, size_t directives_len, mode_t mask)
{
    mode_t target_mode;
    if (!directives_len)
        target_mode = mode;
    else
        target_mode = change_mode(st_mode, directives, directives_len, mask);
    if (chmod(file, target_mode) == -1) {
        strerr_warnwu2sys("change mode of ", file);
        return 1;
    }
    return 0;
}

int traverse_dir(stralloc *dirname, mode_t mode, chmod_directive *directives,
                 size_t directives_len, mode_t mask)
{
    int failure = 0;
    size_t filename_len;
    struct dirent *entry;
    struct stat st;
    DIR *dir = opendir(dirname->s);
    if (!dir) {
        strerr_warnwu2sys("open dir: ", dirname->s);
        return 1;
    }
    while ((entry = readdir(dir))) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;
        dirname->s[dirname->len - 1] = '/';
        filename_len = strlen(entry->d_name);
        if (!stralloc_catb(dirname, entry->d_name, filename_len + 1)) {
            strerr_warnwu4sys("build file name: ", dirname->s, "/",
                              entry->d_name);
            failure = 1;
            continue;
        } else if (stat(dirname->s, &st) == -1) {
            strerr_warnwu2sys("stat ", dirname->s);
            failure = 1;
        } else if (S_ISDIR(st.st_mode))
            failure = failure || traverse_dir(dirname, mode, directives,
                                              directives_len, mask);
        else {
            failure = failure || doit(dirname->s, st.st_mode, mode, directives,
                                      directives_len, mask);
        }
        dirname->len -= filename_len + 1;
        dirname->s[dirname->len - 1] = '\0';
    }
    failure = failure || doit(dirname->s, st.st_mode, mode, directives,
                              directives_len, mask);
    closedir(dir);
    return failure;
}
