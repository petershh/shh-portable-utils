#include <string.h>

#include <skalibs/allreadwrite.h>

int main(int argc, char **argv)
{
    size_t sufflen, len;
    char *basename_start;
    if (argc < 2) {
        allwrite(1, ".\n", 2);
        return 0;
    }
    len = strlen(argv[1]);
    if (!len) {
        allwrite(1, ".\n", 2);
        return 0;
    }
    len--;
    while (len && argv[1][len] == '/')
        len--;
    if (len == 0) {
        allwrite(1, "/\n", 2);
        return 0;
    }
    len++;
    argv[1][len] = '\0';
    basename_start = strrchr(argv[1], '/');
    if (!basename_start)
        basename_start = argv[1];
    else
        basename_start++;
    len -= basename_start - argv[1];
    
    if (argc > 2) {
        sufflen = strlen(argv[2]);
        if (sufflen <= len && !strcmp(basename_start + len - sufflen, argv[2]))
            len -= sufflen;
    }
    allwrite(1, basename_start, len);
    fd_write(1, "\n", 1);
    return 0;
}
