#include <errno.h>

#include <unistd.h>

#include <skalibs/strerr2.h>
#include <skalibs/sgetopt.h>
#include <skalibs/uint32.h>

#define USAGE "nice [-n increment] utility [argument...]"

int main(int argc, char **argv)
{
    int incr = 0;
    subgetopt l = SUBGETOPT_ZERO;
    PROG = "nice";

    for (;;) {
        int opt = subgetopt_r(argc, (char const *const *)argv, "n:", &l);
        if (opt == -1)
            break;
        switch (opt) {
            case 'n':
                if (!int320_scan(l.arg, &incr))
                    strerr_dieusage(100, USAGE);
                break;
            default:
                strerr_dieusage(100, USAGE);
        }
    }
    argc -= l.ind;
    argv += l.ind;

    if (!argc)
        strerr_dieusage(100, USAGE);

    errno = 0;
    nice(incr);

    if (errno == EPERM)
        strerr_warnwu1sys("change process priority");

    execvp(argv[0], argv);

    if (errno == ENOENT)
        strerr_diefu2sys(127, "exec into ", argv[0]);
    else
        strerr_diefu2sys(126, "exec into ", argv[0]);

    return 111;
}
