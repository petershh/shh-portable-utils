#include <skalibs/genalloc.h>
#include <skalibs/strerr2.h>

#include "shhfuncs.h"

int parse_mode_symbolic(char const *raw, genalloc *directives)
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
                return 1;
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
                break;
            }
            else if (*p == '\0') {
                if (d.perm && d.permcopy)
                    return 1;
                if (!genalloc_append(chmod_directive, directives, &d))
                    return 2;
                return 0;
            }
            p++;
        }

        if (d.perm && d.permcopy)
            return 1;
        if (!genalloc_append(chmod_directive, directives, &d))
            return 2;

        for (;;) {
            d.action = d.perm = d.permcopy = d.dir_x = 0;
            if (*p == '+' || *p == '-' || *p == '=')
                d.action = *p;
            else if (*p == ',') {
                p++;
                break;
            }
            else if (*p == '\0')
                return 0;
            else
                return 1;

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
                        return 1;
                    if (!genalloc_append(chmod_directive, directives, &d))
                        return 2;
                    return 0;
                }
                p++;
            }

            if (d.perm && d.permcopy)
                return 1;
            if (!genalloc_append(chmod_directive, directives, &d))
                return 2;
        }
    }
    return 0;
}

