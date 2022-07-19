#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <skalibs/strerr2.h>
#include <skalibs/sgetopt.h>
#include <skalibs/djbunix.h>
#include <skalibs/stralloc.h>

#define USAGE "ln [-fs] [-L|-P] source_file target_file |  ln [-fs] [-L|-P] source_file... target_dir"

int make_link(char const*, char const*, int, int, int);

int main(int argc, char const *const *argv)
{
    struct stat st;
    int result;
    int failure = 0;
    int create_symlink = 0;
    int force_unlink = 0;
    int flag = 0;
    stralloc s = STRALLOC_ZERO;
    subgetopt l = SUBGETOPT_ZERO;

    PROG = "ln";

    for (;;) {
        int opt = subgetopt_r(argc, argv, "fsLP", &l);
        if (opt == -1)
            break;
        switch (opt) {
            case 'f':
                force_unlink = 1;
                break;
            case 's':
                create_symlink = 1;
                break;
            case 'L':
                flag = AT_SYMLINK_FOLLOW;
                break;
            case 'P':
                flag = 0;
                break;
            default:
                strerr_dieusage(100, USAGE);
        }
    }
    argc -= l.ind;
    argv += l.ind;

    if (argc < 2)
        strerr_dieusage(100, USAGE);

    result = lstat(argv[argc - 1], &st);
    if (result == -1 && errno != ENOENT) {
        strerr_diefu2sys(111, "lstat ", argv[argc - 1]);
    }

    if (result == -1 || (!S_ISDIR(st.st_mode))) {
        if (argc != 2)
            strerr_dieusage(100, USAGE);
        if (!S_ISDIR(st.st_mode)) {
            failure = failure || make_link(argv[0], argv[1], create_symlink,
                                           force_unlink, flag);
        }
        return failure ? 111 : 0;
    }

    for (size_t i = 0; i < argc - 1; i++) {
        s.len = 0;
        if (!stralloc_cats(&s, argv[argc - 1])) {
            strerr_warnwu2sys("Build target filename for ", argv[i]);
            failure = 1;
            continue;
        }
        if (!stralloc_append(&s, '/'))  {
            strerr_warnwu2sys("Build target filename for ", argv[i]);
            failure = 1;
            continue;
        }
        if (!sabasename(&s, argv[i], strlen(argv[i]))) {
            strerr_warnwu2sys("Build target filename for ", argv[i]);
            failure = 1;
            continue;
        }
        if (!stralloc_0(&s)) {
            strerr_warnwu2sys("Build target filename for ", argv[i]);
            failure = 1;
            continue;
        }

        failure = failure || make_link(argv[i], s.s, create_symlink,
                                       force_unlink, flag);
    }

    return failure ? 111 : 0;
}

int make_link(char const *src, char const *target, int create_symlink,
              int force_unlink, int flag)
{
    int result;
    if (create_symlink)
        result = symlink(src, target);
    else
        result = linkat(AT_FDCWD, src, AT_FDCWD, target, flag);

    if (result == -1) {
        if (!force_unlink || errno != EEXIST) {
            strerr_warnwu4sys("create link ", target, " to ", src);
            return 1;
        }
        if (unlink(target) == -1) {
            strerr_warnwu2sys("remove ", target);
            return 1;
        }

        if (create_symlink)
            result = symlink(src, target);
        else
            result = linkat(AT_FDCWD, src, AT_FDCWD, target, flag);

        if (result == -1) {
            strerr_warnwu4sys("create link ", target, " to ", src);
            return 1;
        }
    }
    return 0;
}
