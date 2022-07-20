#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>
#include <unistd.h>
#include <dirent.h>

#include <skalibs/strerr2.h>
#include <skalibs/sgetopt.h>
#include <skalibs/skamisc.h>
#include <skalibs/djbunix.h>

#include "shhfuncs.h"

#define USAGE "chgrp [-h] group file... | chgrp -R [-H|-L|-P] group file..."

int traverse_dir(stralloc*, gid_t, unsigned int);

int main (int argc, char const *const *argv)
{
    gid_t gid;
    struct stat st;
    unsigned int failure = 0;
    unsigned int symlink_nofollow = 0;
    unsigned int recurse = 0;
    unsigned int symlink_follow_mode = 0;
    subgetopt l = SUBGETOPT_ZERO;
    PROG = "chgrp";
    for (;;) {
        int opt = subgetopt_r(argc, argv, "hRHLP", &l);
        if (opt == -1)
            break;
        switch (opt) {
            case 'h':
                symlink_nofollow = 1;
                break;
            case 'R':
                recurse = 1;
                break;
            case 'H':
                symlink_follow_mode = 1;
                break;
            case 'L':
                symlink_follow_mode = 2;
                break;
            case 'P':
                symlink_follow_mode = 0;
                break;
            default:
                strerr_dieusage(100, USAGE);
        }
    }

    argc -= l.ind;
    argv += l.ind;

    if (symlink_nofollow && recurse)
        strerr_dieusage(100, USAGE);

    if (argc < 2)
        strerr_dieusage(100, USAGE);

    gid = parse_group(argv[0]);

    if (gid == -1)
        strerr_dief3x(100, "group ", argv[0], " was not found");

    for (char const *const *filename = argv + 1; *filename; filename++) {
        if (recurse) {
            if (lstat(*filename, &st) == -1) {
                strerr_warnwu2sys("lstat ", *filename);
                failure = 1;
                continue;
            }
            if (S_ISDIR(st.st_mode)) {
                satmp.len = 0;
                if (!stralloc_catb(&satmp, *filename, strlen(*filename) + 1)) {
                    strerr_warnwu2sys("build file name: ", *filename);
                    failure = 1;
                    continue;
                }
                failure = failure || traverse_dir(&satmp, gid,
                                                  symlink_follow_mode);
            }
            else if (S_ISLNK(st.st_mode) && symlink_follow_mode > 1) {
                if (stat(*filename, &st) == -1) {
                    strerr_warnwu2sys("stat ", *filename);
                    failure = 1;
                    continue;
                }
                if (S_ISDIR(st.st_mode)) {
                    satmp.len = 0;
                    if (!stralloc_catb(&satmp, *filename,
                                       strlen(*filename) + 1)) {
                        strerr_warnwu2sys("build file name: ", *filename);
                        failure = 1;
                        continue;
                    }
                    failure = failure || traverse_dir(&satmp, gid,
                                                      symlink_follow_mode);
                } else
                    if (chown(*filename, -1, gid) == -1) {
                        strerr_warnwu2sys("change group ownership of ",
                                          *filename);
                        failure = 1;
                        continue;
                    }
            } else {
                if (lchown(*filename, -1, gid)) {
                    strerr_warnwu2sys("change group ownership of ", *filename);
                    failure = 1;
                }
            }
        } else {
            int result;
            if (symlink_nofollow)
                 result = lchown(*filename, -1, gid);
            else
                result = chown(*filename, -1, gid);
            if (result == -1) {
                strerr_warnwu2sys("change group ownership of ", *filename);
                failure = 1;
            }
        }
    }
    return failure ? 111 : 0;
}

int traverse_dir(stralloc *dirname, gid_t gid, unsigned int symlink_follow_mode)
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
    for (;;) {
        entry = readdir(dir);
        if (!entry) {
            if (errno) {
                strerr_warnwu2sys("read dir ", dirname->s);
                failure = 1;
            }
            break;
        }
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;
        dirname->s[dirname->len - 1] = '/';
        filename_len = strlen(entry->d_name);
        if (!stralloc_catb(dirname, entry->d_name, filename_len + 1)) {
            strerr_warnwu4sys("build file name: ", dirname->s, "/",
                              entry->d_name);
            failure = 1;
            continue;
        }
        if (lstat(dirname->s, &st) == -1) {
            strerr_warnwu2sys("lstat ", dirname->s);
            failure = 1;
        } else if (S_ISDIR(st.st_mode))
            failure = failure || traverse_dir(dirname, gid,
                                              symlink_follow_mode);
        else if (S_ISLNK(st.st_mode) && symlink_follow_mode == 2) {
            if (stat(dirname->s, &st) == -1) {
                strerr_warnwu2sys("stat ", dirname->s);
                failure = 1;
            } else if (S_ISDIR(st.st_mode))
                failure = failure || traverse_dir(dirname, gid,
                                                  symlink_follow_mode);
            else if (chown(dirname->s, -1, gid) == -1) {
                strerr_warnwu2sys("change group ownership of ", dirname->s);
                failure = 1;
            }
        } else if (lchown(dirname->s, -1, gid) == -1) {
            strerr_warnwu2sys("change group ownership of ", dirname->s);
            failure = 1;
        }
        dirname->len -= filename_len + 1;
        dirname->s[dirname->len - 1] = '\0';
    }
    if (chown(dirname->s, -1, gid) == -1) {
        strerr_warnwu2sys("change group ownership of ", dirname->s);
        failure = 1;
    }
    return failure;
}
