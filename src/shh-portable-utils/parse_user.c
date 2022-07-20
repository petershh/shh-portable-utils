#include <sys/types.h>
#include <pwd.h>

#include <skalibs/strerr2.h>
#include <skalibs/types.h>

#include "shhfuncs.h"

uid_t parse_user(char const *user)
{
    uid_t result;
    struct passwd *pwd = getpwnam(user);
    if (pwd)
        return pwd->pw_uid;
    if (!uid_scan(user, &result))
        strerr_dief3x(100, "user ", user, " was not found");
    return result;
}
