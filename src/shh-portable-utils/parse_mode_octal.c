#include <skalibs/types.h>
#include <skalibs/strerr2.h>

#include "shhfuncs.h"

mode_t parse_mode_octal(char const *raw_mode)
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
