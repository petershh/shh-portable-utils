#include <unistd.h>

#include <skalibs/strerr2.h>

#define USAGE "unlink file"

int main(int argc, char const *const *argv)
{
    PROG = "unlink";

    if (argc != 2)
        strerr_dieusage(100, USAGE);

    if (unlink(argv[1]) == -1)
        strerr_diefu2sys(111, "unlink ", argv[1]);

    return 0;
}
