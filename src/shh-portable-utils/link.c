#include <unistd.h>

#include <skalibs/strerr2.h>

#define USAGE "link file1 file2"

int main(int argc, char const *const *argv)
{
    PROG = "link";
    if (argc != 3)
        strerr_dieusage(100, USAGE);

    if (link(argv[1], argv[2]) == -1)
        strerr_diefu4sys(111, "link ", argv[2], " to ", argv[1]);

    return 0;
}
