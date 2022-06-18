#include <stddef.h>
#include <string.h>

#include <skalibs/allreadwrite.h>
#include <skalibs/strerr2.h>
#include <skalibs/sgetopt.h>
#include <skalibs/djbunix.h>

#define USAGE "cat [-u] [file...]"

typedef off_t (*cat_func_t)(int, int);

off_t fd_cat_slow(int from, int to);

off_t fd_cat_slow(int from, int to)
{
    char c;
    off_t transmitted = 0;
    for (;;) {
        ssize_t result = fd_read(from, &c, 1);
        if (result < 0)
            return result;
        if (result == 0)
            break;
        if (fd_write(to, &c, 1) < 0)
            return -1;
        transmitted++;
    }
    return transmitted;
}

int main(int argc, char const *const *argv)
{
    cat_func_t cat = fd_cat;
    subgetopt l = SUBGETOPT_ZERO;
    PROG = "cat";
    for (;;) {
        int opt = subgetopt_r(argc, argv, "u", &l);
        if (opt == -1)
            break;
        switch (opt) {
            case 'u':
                cat = fd_cat_slow;
                break;
            default:
                strerr_dieusage(100, USAGE);
                break;
        }
    }
    argc -= l.ind;
    argv += l.ind;
    if (argc == 0)
        if (cat(0, 1) < 0)
            strerr_diefu1sys(111, "read from stdin");
    for (char const *const *filename = argv; *filename; filename++) {
        int fd;
        if (!strcmp(*filename, "-"))
            fd = 0;
        else {
            fd = openb_read(*filename);
            if (fd < 0)
                strerr_diefu3sys(111, "open file ", *filename, " for reading");
        }
        if (cat(fd, 1) < 0)
            strerr_diefu2sys(111, "read from ", fd == 1 ? "stdin" : *filename);
        fd_close(fd);
    }

    return 0;
}
