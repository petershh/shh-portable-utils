#include <string.h>

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

int traverse_dir(stralloc*, mode_t, genalloc*, mode_t);

int main(int argc, char const *const *argv)
{
    struct stat st;
    mode_t mask;
    mode_t mode = 0;
    int recurse = 0;
    int failure = 0;
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

    if ('0' <= argv[0][0] && argv[0][0] <= '7')
        mode = parse_mode_octal(argv[0]);
    else {
        parse_mode_symbolic(argv[0], &directives);
        if (!genalloc_len(chmod_directive, &directives))
            strerr_dieusage(100, USAGE);
    }

    for (char const *const *filename = argv + 1; *filename; filename++) {
        if (stat(*filename, &st)) {
            strerr_warnwu2sys("stat ", *filename);
            failure = 1;
            continue;
        }
        if (S_ISDIR(st.st_mode) && recurse) {
            satmp.len = 0;
            if (!stralloc_catb(&satmp, *filename, strlen(*filename) + 1)) {
                strerr_warnwu2sys("build file name ", *filename);
                failure = 1;
                continue;
            }
            failure = failure || traverse_dir(&satmp, mode, &directives,
                                              mask);
        }
        else
            failure = failure || change_mode(*filename, mode, &directives,
                                             mask);
    }
    return failure ? 111 : 0;
}

int traverse_dir(stralloc *dirname, mode_t mode, genalloc *directives,
                 mode_t mask)
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
            failure = failure || traverse_dir(dirname, mode, directives, mask);
        else
            failure = failure || change_mode(dirname->s, mode, directives,
                                             mask);
        dirname->len -= filename_len + 1;
        dirname->s[dirname->len - 1] = '\0';
    }
    failure = failure || change_mode(dirname->s, mode, directives, mask);
    closedir(dir);
    return failure;
}
