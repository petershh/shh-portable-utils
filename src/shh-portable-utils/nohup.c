#include <stdlib.h>
#include <errno.h>

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include <skalibs/strerr2.h>
#include <skalibs/stralloc.h>
#include <skalibs/djbunix.h>

#define USAGE "nohup utility [argument...]"

int open_nohupout(void);

int main(int argc, char **argv)
{
    int new_fd;
    int stdout_tty = 0, stdout_closed = 0;
    int stderr_tty = 0;

    PROG = "nohup";
    if (argc < 2)
        strerr_dieusage(127, USAGE);

    if (signal(SIGHUP, SIG_IGN) == SIG_ERR)
        strerr_diefu1sys(127, "set SIGHUP disposition");

    stdout_tty = isatty(1);
    if (errno == EBADF)
        stdout_closed = 1;

    stderr_tty = isatty(2);

    /* configure io streams
     *
     * stdin */
    if (stdout_tty) {
        new_fd = open_nohupout();

        if (dup2(new_fd, 1) == -1)
            strerr_diefu1sys(127, "redirect stdin");

        close(new_fd);
    }

    /* stderr */

    if (stderr_tty) {
        if (!stdout_closed) {
            if (dup2(1, 2) == -1)
                strerr_diefu1sys(127, "redirect stderr to stdin");
        } else {
            new_fd = open_nohupout();
            if (dup2(new_fd, 2) == -1)
                strerr_diefu1sys(127, "redirect stderr");
            close(new_fd);
        }
    }

    execvp(argv[1], argv + 1);
    
    if (errno == ENOENT)
        return 127;
    else
        return 126;
}

int open_nohupout(void)
{
    char *homedir;
    int homedir_fd;
    int fd = open3("nohup.out", O_WRONLY | O_CREAT | O_APPEND, 0600);
    if (fd == -1) {
        strerr_warnwu1sys("open nohup.out");
        homedir = getenv("HOME");
        if (!homedir)
            strerr_diefu1x(127, "get $HOME value");
        homedir_fd = open2(homedir, O_RDONLY);
        if (homedir_fd == -1)
            strerr_diefu1sys(127, "open home directory");
        fd = openat(homedir_fd, "nohup.out", O_WRONLY | O_CREAT | O_APPEND,
                        0600);
        if (fd == -1)
            strerr_diefu3sys(127, "open ", homedir, "/nohup.out");
        close(homedir_fd);
    }
    return fd;
}
