#include <errno.h>

#include <sys/types.h>
#include <sys/resource.h>

#include <skalibs/strerr2.h>
#include <skalibs/sgetopt.h>
#include <skalibs/types.h>

#include "shhfuncs.h"

#define USAGE "renice [-g|-p|-u] -n increment ID..."

int main (int argc, char const *const *argv)
{
    int incr = 0;
    int which = PRIO_PROCESS;
    int failure = 0;
    id_t who = 0;
    subgetopt l = SUBGETOPT_ZERO;
    
    PROG = "renice";

    for (;;) {
        int opt = subgetopt_r(argc, argv, "n:gpu", &l);
        if (opt == -1) {
            int current_prio;
            if (l.ind >= argc || !argv[l.ind])
                break;
            switch (which) {
                case PRIO_PGRP:
                    {
                        pid_t pgid;
                        if (!pid_scan(argv[l.ind], &pgid))
                            strerr_dief2x(100, "Invalid process group id: ",
                                          argv[l.ind]);
                        who = pgid;
                        break;
                    }
                case PRIO_PROCESS:
                    {
                        pid_t pid;
                        if (!pid_scan(argv[l.ind], &pid))
                            strerr_dief2x(100, "Invalid process id: ",
                                          argv[l.ind]);
                        who = pid;
                        break;
                    }
                case PRIO_USER:
                    who = parse_user(argv[l.ind]);
                    if (who == -1)
                        strerr_dief2x(100, "Invalid user: ", argv[l.ind]);
                    break;
            }
            errno = 0;
            current_prio = getpriority(which, who);
            if (errno) {
                strerr_warnwu1sys("get priority");
            }
            if (setpriority(which, who, current_prio + incr) == -1) {
                strerr_warnwu1sys("set priority");
            }
            l.ind++;
            l.pos = 0;
            continue;
        }
        switch (opt) {
            case 'n':
                if (!int32_scan(l.arg, &incr))
                    strerr_dieusage(100, USAGE);
                break;
            case 'g':
                which = PRIO_PGRP;
                break;
            case 'p':
                which = PRIO_PROCESS;
                break;
            case 'u':
                which = PRIO_USER;
                break;
            default:
                strerr_dieusage(100, USAGE);
        }
    }
    return failure ? 111: 0;
}
