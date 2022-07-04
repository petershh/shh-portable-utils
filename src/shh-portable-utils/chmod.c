#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <skalibs/sgetopt.h>
#include <skalibs/strerr2.h>
#include <skalibs/types.h>
#include <skalibs/skamisc.h>
#include <skalibs/stralloc.h>
#include <skalibs/genalloc.h>

#define USAGE "chmod [-R] mode file..."

struct chmod_directive_s {
    int who;
    int action;
    int perm;
};

typedef struct chmod_directive_s chmod_directive;

#define CHMOD_DIRECTIVE_ZERO { 0, 0, 0 }

mode_t parse_octal(char const*);

void parse_symbolic(char const*, genalloc*);

int change_mode(char const*, mode_t, genalloc*, mode_t);

int traverse_dir(char const*, mode_t, genalloc*, mode_t);

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
        if (S_ISDIR(st.st_mode) && recurse)
            failure ||= traverse_dir(*filename, mode, &directives, mask);
        else
            failure ||= change_mode(filename, mode, &directives, mask);
    }
    return failure ? 111 : 0;
}


mode_t parse_octal(char const *raw_mode)
{
    mode_t mode = 0;
    unsigned int m;
    if (!uint0_oscan(raw_mode, &m))
        strerr_dief2x("invalid mode: ", raw_mode);

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
                d.who |= 1;
            else if (*p == 'g')
                d.who |= 2;
            else if (*p == 'o')
                d.who |= 4;
            else if (*p == 'a')
                d.who |= 7;
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
                d.perm |= 1;
            else if (*p == 'g')
                d.perm |= 2;
            else if (*p == 'o')
                d.perm |= 4;
            else if (*p == 'r')
                d.perm |= 8;
            else if (*p == 'w')
                d.perm |= 16;
            else if (*p == 'x')
                d.perm |= 32;
            else if (*p == 'X')
                d.perm |= 64;
            else if (*p == 's')
                d.perm |= 128;
            else if (*p == 't')
                d.perm |= 256;
            else if (*p == ',') {
                p++;
                break;
            }
            else if (*p == '\0') {
                if (!genalloc_append(chmod_directive, directives, &d))
                    strerr_diefu1sys(111, "parse mode");
                return;
            }
            p++;
        }

        if (!genalloc_append(chmod_directive, directives, &d))
            strerr_diefu1sys(111, "parse mode");

        for (;;) {
            d.action = d.perm = 0;
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
                    d.perm |= 1;
                else if (*p == 'g')
                    d.perm |= 2;
                else if (*p == 'o')
                    d.perm |= 4;
                else if (*p == 'r')
                    d.perm |= 8;
                else if (*p == 'w')
                    d.perm |= 16;
                else if (*p == 'x')
                    d.perm |= 32;
                else if (*p == 'X')
                    d.perm |= 64;
                else if (*p == 's')
                    d.perm |= 128;
                else if (*p == 't')
                    d.perm |= 256;
                else if (*p == ',')
                    break;
                else if (*p == '\0') {
                    if (!genalloc_append(chmod_directive, directives, &d))
                        strerr_diefu1sys(111, "parse mode");
                    return;
                }
                p++;
            }
            if (!genalloc_append(chmod_directive, directives, &d))
                strerr_diefu1sys(111, "parse mode");
        }
    }
}

int change_mode(char const *file, mode_t mode, genalloc *directives,
                mode_t mask)
{
    if (!genalloc_len(chmod_directive, directives)) {
        if (chown(file, mode) == -1) {
            strerr_warnwu2sys("change mode of ", file);
            return 1;
        }
    } else {
        struct stat st;
        mode_t target_mode;
        chmod_directive const *s = genalloc_s(chmod_directive, directives);
        size_t len = genalloc_len(chmod_directive, directives);
        if (stat(file, &st) == -1) {
            strerr_warnwu2sys("stat ", file);
            return 1;
        }
        target_mode = st.st_mode;
        for (size_t i = 0; i < len; i++) {
            switch (s[i].action) {
                case '+':
                    if (!s[i].perm)
                        break;
                    if (!s[i].who) {
                        if (s[i].perm & 1)
                            // TODO
                    }
                case '-':
                    if (!s[i].perm)
                        break;
                case '=':
            }
        }
    }
    return 0;
}

int traverse_dir(char const *file, mode_t mode, genalloc *directives
                 mode_t mask)
{
    strerr_dief1x(128, "not implemented yet");
}
