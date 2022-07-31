#include <sys/stat.h>

#include <skalibs/genalloc.h>
#include <skalibs/strerr2.h>

#include "shhfuncs.h"

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
