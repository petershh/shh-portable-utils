#include <string.h>

#include <skalibs/strerr2.h>
#include <skalibs/allreadwrite.h>

#define USAGE "dirname string"

int main(int argc, char **argv)
{
    size_t len;
    char *dirname_end;
    PROG = "dirname";
    if (argc < 2)
        strerr_dieusage(100, USAGE);

    len = strlen(argv[1]);
    if (!len) {
        allwrite(1, ".\n", 2);
        return 0;
    }

    while (len && argv[1][len - 1] == '/')
        len--;
    if (len == 0) {
        allwrite(1, "/\n", 2);
        return 0;
    }

    argv[1][len] = '\0';

    dirname_end = strrchr(argv[1], '/');
    if (!dirname_end)
        allwrite(1, ".\n", 2);
    else {
        allwrite(1, argv[1], dirname_end - argv[1]);
        fd_write(1, "\n", 1);
    }

    return 0;
}
