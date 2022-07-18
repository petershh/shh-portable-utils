#include <string.h>

#include <sys/utsname.h>

#include <skalibs/strerr2.h>
#include <skalibs/sgetopt.h>
#include <skalibs/buffer.h>

#define USAGE "uname [-amnrsv]" 

int main (int argc, char const *const *argv)
{
    struct utsname data;
    int flags = 0;
    int have_written = 0;
    subgetopt l = SUBGETOPT_ZERO;
    for (;;) {
        int opt = subgetopt_r(argc, argv, "amnrsv", &l);
        if (opt == -1)
            break;
        switch (opt) {
            case 'a':
                flags |= 31;
                break;
            case 's':
                flags |= 1;
                break;
            case 'n':
                flags |= 2;
                break;
            case 'r':
                flags |= 4;
                break;
            case 'v':
                flags |= 8;
                break;
            case 'm':
                flags |= 16;
                break;
            default:
                strerr_dieusage(100, USAGE);
        }
    }
    argc -= l.ind;
    argv += l.ind;
    if (argc)
        strerr_dieusage(100, USAGE);

    if (!flags)
        flags = 1;
    
    if (uname(&data) == -1)
        strerr_diefu1sys(111, "uname");

    if (flags & 1) {
        buffer_puts(buffer_1small, data.sysname);
        have_written = 1;
    }

    if (flags & 2) {
        if (have_written)
            buffer_puts(buffer_1small, " ");
        else
            have_written = 1;
        buffer_puts(buffer_1small, data.nodename);
    }

    if (flags & 4) {
        if (have_written)
            buffer_puts(buffer_1small, " ");
        else
            have_written = 1;
        buffer_puts(buffer_1small, data.release);
    }

    if (flags & 8) {
        if (have_written)
            buffer_puts(buffer_1small, " ");
        else
            have_written = 1;
        buffer_puts(buffer_1small, data.version);
    }

    if (flags & 16) {
        if (have_written)
            buffer_puts(buffer_1small, " ");
        else
            have_written = 1;
        buffer_puts(buffer_1small, data.machine);
    }

    buffer_putsflush(buffer_1small, "\n");

    return 0;
}
