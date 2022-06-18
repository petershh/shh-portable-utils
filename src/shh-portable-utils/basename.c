#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{
    size_t pathlen, sufflen, i;
    char *basename_start;
    if (argc < 2) { write(1, ".\n", 2); return 0; }
    pathlen = strlen(argv[1]);
    if(!pathlen) { write(1, ".\n", 2); return 0; }
    for (i = 1; i < pathlen; i++)
        if (argv[1][pathlen - i] != '/') break;
    if (i == pathlen) {
        write(1, "/\n", 2);
        return 0;
    }
    i--;
    argv[1][pathlen - i] = '\0';
    pathlen = pathlen - i;
    for(i = 1; i < pathlen; i++)
        if (argv[1][pathlen - i] == '/') break;
    if (i == pathlen)
        basename_start = argv[1];
    else {
        basename_start = argv[1] + pathlen - i + 1;
        pathlen = i - 1;
    }
    if (argc > 2) {
        sufflen = strlen(argv[2]);
        if (!strcmp(basename_start + pathlen - sufflen, argv[2]))
            pathlen -= sufflen;
    }
    write(1, basename_start, pathlen);
    write(1, "\n", 1);
    return 0;
}
