#include <string.h>

#include <unistd.h>

#include <skalibs/strerr2.h>
#include <skalibs/allreadwrite.h>

int main(void)
{
    char *logname;
    PROG = "logname";
    logname = getlogin();
    if (!logname)
        strerr_diefu1sys(111, "get login name");
    allwrite(1, logname, strlen(logname));
    fd_write(1, "\n", 1);
    return 0;
}
