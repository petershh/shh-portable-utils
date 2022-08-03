#include <sys/stat.h>

#include <skalibs/strerr2.h>

#include "shhfuncs.h"

/*
 * Algorithm for chmod mode parsing was inspired by sbase's chmod
 * implementation (https://core.suckless.org/sbase), so let's credit them here.
 */

mode_t change_mode(mode_t initial_mode, chmod_directive *directives,
                   size_t directives_len, mode_t mask)
{
    mode_t cur_mode, who, perm, clear;

    cur_mode = initial_mode;

    for (size_t i = 0; i < directives_len; i++) {
        if (directives[i].who) {
            who = directives[i].who;
            clear = directives[i].who;
        }
        else {
            who = ~mask;
            clear = S_ISALL;
        }
        perm = directives[i].perm;

        if (directives[i].permcopy & 1) {
            if (cur_mode & S_IRUSR)
                perm |= S_IRUSR | S_IRGRP | S_IROTH;
            if (cur_mode & S_IWUSR)
                perm |= S_IWUSR | S_IWGRP | S_IWOTH;
            if (cur_mode & S_IXUSR)
                perm |= S_IXUSR | S_IXGRP | S_IXOTH;
        }
        if (directives[i].permcopy & 2) {
            if (cur_mode & S_IRGRP)
                perm |= S_IRUSR | S_IRGRP | S_IROTH;
            if (cur_mode & S_IWGRP)
                perm |= S_IWUSR | S_IWGRP | S_IWOTH;
            if (cur_mode & S_IXGRP)
                perm |= S_IXUSR | S_IXGRP | S_IXOTH;
        }
        if (directives[i].permcopy & 4) {
            if (cur_mode & S_IROTH)
                perm |= S_IRUSR | S_IRGRP | S_IROTH;
            if (cur_mode & S_IWOTH)
                perm |= S_IWUSR | S_IWGRP | S_IWOTH;
            if (cur_mode & S_IXOTH)
                perm |= S_IXUSR | S_IXGRP | S_IXOTH;
        }

        if (directives[i].dir_x && (S_ISDIR(cur_mode)
                                    || (cur_mode
                                        & (S_IXUSR | S_IXGRP | S_IXOTH))))
            perm |= S_IXUSR | S_IXGRP | S_IXOTH;

        switch (directives[i].action) {
            case '=':
                cur_mode &= ~clear;
                /* fallthrough */
            case '+':
                cur_mode |= (who & perm);
                break;
            case '-':
                cur_mode &= ~(who & perm);
        }
    }
    return cur_mode;
}
