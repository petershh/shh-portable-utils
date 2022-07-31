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
        mode = parse_octal(argv[0]);
    else {
        parse_symbolic(argv[0], &directives);
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


mode_t parse_octal(char const *raw_mode)
{
    mode_t mode = 0;
    unsigned int m;
    if (!uint0_oscan(raw_mode, &m))
        strerr_dief2x(100, "invalid mode: ", raw_mode);

    if (m & 0001)
        mode |= S_IXOTH;
    if (m & 0002)
        mode |= S_IWOTH;
    if (m & 0004)
        mode |= S_IROTH;
    if (m & 0010)
        mode |= S_IXGRP;
    if (m & 0020)
        mode |= S_IWGRP;
    if (m & 0040)
        mode |= S_IRGRP;
    if (m & 0100)
        mode |= S_IXUSR;
    if (m & 0200)
        mode |= S_IWUSR;
    if (m & 0400)
        mode |= S_IRUSR;
    if (m & 01000)
        mode |= S_ISVTX;
    if (m & 02000)
        mode |= S_ISGID;
    if (m & 04000)
        mode |= S_ISUID;

    return mode;
}

void parse_symbolic(char const *raw, genalloc *directives)
{
    char const *p = raw;
    for (;;) {
        chmod_directive d = CHMOD_DIRECTIVE_ZERO;
        /* stage 1: parse 'who' list */
        for (;;) {
            if (*p == 'u')
                d.who |= S_IRWXU | S_ISUID;
            else if (*p == 'g')
                d.who |= S_IRWXG | S_ISGID;
            else if (*p == 'o')
                d.who |= S_IRWXO;
            else if (*p == 'a')
                d.who |= S_IRWXU | S_IRWXG | S_IRWXO | S_ISUID | S_ISGID;
            else
                break;
            p++;
        }
        /* stage 2: actions. First cycle is out of the loop. */
        switch(*p) {
            case '+':
            case '-':
            case '=':
                d.action = *p;
                break;
            default:
                strerr_dief2x(100, "invalid mode: ", raw);
        }
        p++;
        for (;;) {
            if (*p == 'u')
                d.permcopy |= 1;
            else if (*p == 'g')
                d.permcopy |= 2;
            else if (*p == 'o')
                d.permcopy |= 4;
            else if (*p == 'r')
                d.perm |= S_IRUSR | S_IRGRP | S_IROTH;
            else if (*p == 'w')
                d.perm |= S_IWUSR | S_IWGRP | S_IWOTH;
            else if (*p == 'x')
                d.perm |= S_IXUSR | S_IXGRP | S_IXOTH;
            else if (*p == 'X')
                d.dir_x = 1;
            else if (*p == 's')
                d.perm |= S_ISUID | S_ISGID;
            else if (*p == 't')
                d.perm |= S_ISVTX;
            else if (*p == ',') {
                p++;
                break;
            }
            else if (*p == '\0') {
                if (d.perm && d.permcopy)
                    strerr_dief2x(100, "invalid mode: ", raw);
                if (!genalloc_append(chmod_directive, directives, &d))
                    strerr_diefu1sys(111, "parse mode");
                return;
            }
            p++;
        }

        if (d.perm && d.permcopy)
            strerr_dief2x(100, "invalid mode: ", raw);
        if (!genalloc_append(chmod_directive, directives, &d))
            strerr_diefu1sys(111, "parse mode");

        for (;;) {
            d.action = d.perm = d.permcopy = d.dir_x = 0;
            if (*p == '+' || *p == '-' || *p == '=')
                d.action = *p;
            else if (*p == ',')
                break;
            else if (*p == '\0')
                return;
            else
                strerr_dief2x(100, "invalid mode: ", raw);

            p++;

            for (;;) {
                if (*p == 'u')
                    d.permcopy |= 1;
                else if (*p == 'g')
                    d.permcopy |= 2;
                else if (*p == 'o')
                    d.permcopy |= 4;
                else if (*p == 'r')
                    d.perm |= S_IRUSR | S_IRGRP | S_IROTH;
                else if (*p == 'w')
                    d.perm |= S_IWUSR | S_IWGRP | S_IWOTH;
                else if (*p == 'x')
                    d.perm |= S_IXUSR | S_IXGRP | S_IXOTH;
                else if (*p == 'X')
                    d.dir_x = 1;
                else if (*p == 's')
                    d.perm |= S_ISUID | S_ISGID;
                else if (*p == 't')
                    d.perm |= S_ISVTX;
                else if (*p == ',') {
                    p++;
                    break;
                }
                else if (*p == '\0') {
                    if (d.perm && d.permcopy)
                        strerr_dief2x(100, "invalid mode: ", raw);
                    if (!genalloc_append(chmod_directive, directives, &d))
                        strerr_diefu1sys(111, "parse mode");
                    return;
                }
                p++;
            }

            if (d.perm && d.permcopy)
                strerr_dief2x(100, "invalid mode: ", raw);
            if (!genalloc_append(chmod_directive, directives, &d))
                strerr_diefu1sys(111, "parse mode");
        }
    }
}

int change_mode(char const *file, mode_t mode, genalloc *directives,
                mode_t mask)
{
    mode_t cur_mode;
    if (!genalloc_len(chmod_directive, directives))
        cur_mode = mode;
    else {
        struct stat st;
        mode_t who, perm, clear;
        chmod_directive const *s = genalloc_s(chmod_directive, directives);
        size_t len = genalloc_len(chmod_directive, directives);

        if (stat(file, &st) == -1) {
            strerr_warnwu2sys("stat ", file);
            return 1;
        }
        cur_mode = st.st_mode;

        for (size_t i = 0; i < len; i++) {
            if (s[i].who) {
                who = s[i].who;
                clear = s[i].who;
            }
            else {
                who = ~mask;
                clear = S_ISALL;
            }
            perm = s[i].perm;

            if (s[i].permcopy & 1) {
                if (cur_mode & S_IRUSR)
                    perm |= S_IRUSR | S_IRGRP | S_IROTH;
                if (cur_mode & S_IWUSR)
                    perm |= S_IWUSR | S_IWGRP | S_IWOTH;
                if (cur_mode & S_IXUSR)
                    perm |= S_IXUSR | S_IXGRP | S_IXOTH;
            }
            if (s[i].permcopy & 2) {
                if (cur_mode & S_IRGRP)
                    perm |= S_IRUSR | S_IRGRP | S_IROTH;
                if (cur_mode & S_IWGRP)
                    perm |= S_IWUSR | S_IWGRP | S_IWOTH;
                if (cur_mode & S_IXGRP)
                    perm |= S_IXUSR | S_IXGRP | S_IXOTH;
            }
            if (s[i].permcopy & 4) {
                if (cur_mode & S_IROTH)
                    perm |= S_IRUSR | S_IRGRP | S_IROTH;
                if (cur_mode & S_IWOTH)
                    perm |= S_IWUSR | S_IWGRP | S_IWOTH;
                if (cur_mode & S_IXOTH)
                    perm |= S_IXUSR | S_IXGRP | S_IXOTH;
            }

            if (s[i].dir_x && (S_ISDIR(cur_mode)
                                || (cur_mode & (S_IXUSR | S_IXGRP | S_IXOTH))))
                perm |= S_IXUSR | S_IXGRP | S_IXOTH;

            switch (s[i].action) {
                case '=':
                    cur_mode &= ~clear;
                case '+':
                    cur_mode |= (who & perm);
                    break;
                case '-':
                    cur_mode &= ~(who & perm);
            }
        }
    }
    if (chmod(file, cur_mode) == -1) {
        strerr_warnwu2sys("change mode of ", file);
        return 1;
    }
    return 0;
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
