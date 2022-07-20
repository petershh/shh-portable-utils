#include <sys/types.h>
#include <grp.h>

#include <skalibs/strerr2.h>
#include <skalibs/types.h>

gid_t parse_group(char const *group)
{
    gid_t result;
    struct group *grp = getgrnam(group);
    if (grp)
        return grp->gr_gid;
    if (!gid_scan(group, &result))
        return -1;
    return result;
}
