#include <stdlib.h>

#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include <skalibs/strerr2.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/djbunix.h>
#include <skalibs/sgetopt.h>

#define BUFSIZ 8192

#define USAGE "tee [-ai] [file...]"

int main(int argc, char const *const *argv)
{
    char buf[BUFSIZ];
    ssize_t nbytes;
    int failure = 0;
    int *fds = NULL;
    int write_mode = O_TRUNC;
    int ignore_sigint = 0;
    subgetopt l = SUBGETOPT_ZERO;
    PROG = "tee";
    for (;;) {
        int opt = subgetopt_r(argc, argv, "ai", &l);
        if (opt == -1)
            break;
        switch (opt) {
            case 'a':
                write_mode = O_APPEND;
                break;
            case 'i':
                ignore_sigint = 1;
                break;
            default:
                strerr_dieusage(100, USAGE);
        }
    }
    argc -= l.ind;
    argv += l.ind;

    if (ignore_sigint)
        if (signal(SIGINT, SIG_IGN) == SIG_ERR)
            strerr_diefu1sys(111, "set SIGINT handler");

    if (argc > 0) {
        fds = (int*)malloc(argc * sizeof(int));
        if (!fds)
            strerr_diefu1sys(111, "allocate memory");
    }

    for (size_t i = 0; i < argc; i++) {
        fds[i] = open3(argv[i], O_WRONLY | O_CREAT | write_mode, 0666);
        if (fds[i] == -1) {
            strerr_warnwu3sys("open ", argv[i], " for reading");
            failure = 1;
        }
    }

    for (;;) {
        nbytes = read(0, buf, BUFSIZ);
        if (nbytes == -1)
            strerr_diefu1sys(111, "read from stdin ");
        if (nbytes == 0)
            break;
        for (size_t i = 0; i < argc; i++) {
            if (fds[i] == -1)
                continue;
            if (allwrite(fds[i], buf, nbytes) < 0) {
                strerr_warnwu2sys("write to ", argv[i]);
                fd_close(fds[i]);
                fds[i] = -1;
                failure = 1;
            }
        }
        if (allwrite(1, buf, nbytes) < 0) {
            strerr_warnwu1sys("write to stdout");
            failure = 1;
        }
    }
    return failure ? 111: 0;
}
