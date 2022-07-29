#include <string.h>

#include <skalibs/bytestr.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/strerr2.h>
#include <skalibs/sgetopt.h>
#include <skalibs/uint32.h>
#include <skalibs/djbunix.h>

#include "shhfuncs.h"

#define USAGE "wc [-c|-m] [-lw] [file...]"

int count(int, int*, int*, int*, char const*);

void output(int, int, int, int, char const*);

int main(int argc, char const *const *argv)
{
    int flags = 0;
    int bytes = 0, words = 0, newlines = 0;
    int total_bytes = 0, total_words = 0, total_newlines = 0;
    int failure = 0, failure_here = 0;
    subgetopt l = SUBGETOPT_ZERO;
    PROG = "wc";
    for (;;) {
        int opt = subgetopt_r(argc, argv, "cmlw", &l);
        if (opt == -1)
            break;
        switch (opt) {
            case 'm':
                /* Fallthrough. See cut.c code comments for details. */
            case 'c':
                flags |= 4;
                break;
            case 'l':
                flags |= 1;
                break;
            case 'w':
                flags |= 2;
                break;
            default:
                strerr_dieusage(100, USAGE);
        }
    }
    argc -= l.ind;
    argv += l.ind;

    if (!flags)
        flags = 7;

    if (!argc) {
        failure = count(0, &bytes, &words, &newlines, "stdin");
        output(bytes, words, newlines, flags, 0);
    }

    for (char const *const *filename = argv; *filename; filename++) {
        if (!strcmp(*filename, "-"))
            failure_here = count(0, &bytes, &words, &newlines, "stdin");
        else {
            int fd = openb_read(*filename);
            if (fd == -1) {
                strerr_warnwu3sys("open ", *filename, " for reading");
                failure = 1;
                continue;
            }
            failure_here = count(fd, &bytes, &words, &newlines, *filename);
            fd_close(fd);
        }
        if (!failure_here) {
            output(bytes, words, newlines, flags, *filename);
            total_bytes += bytes;
            total_words += words;
            total_newlines += newlines;
        }
        failure = failure || failure_here;
    }

    if (argc > 1)
        output(total_bytes, total_words, total_newlines, flags, "total");

    return failure ? 111 : 0;
}

int count(int fd, int *bytes, int *words, int *newlines, char const *name)
{
    char buf[512];
    size_t count;
    size_t i;
    int prev_block_word = 0;

    *bytes = *words = *newlines = 0;

    for (;;) {
        count = fd_read(fd, buf, 512);
        if (count == -1) {
            strerr_warnwu2sys("read from ", name);
            return 1;
        }
        if (count == 0)
            break;

        *bytes += count;

        i = byte_notin(buf, count, " \t\r\f\v", 5);
        while (i < count) {
            if (buf[i] == '\n') {
                *newlines += 1;
                i++;
            }
            else {
                if (prev_block_word)
                    prev_block_word = 0;
                else
                    *words += 1;
                i += byte_in(buf + i, count - i, " \t\r\f\v\n", 6);
            }
            i += byte_notin(buf + i, count - i, " \t\r\f\v", 5);
        }
        if (!memchr(" \t\r\f\v\n", buf[count - 1], 6))
            prev_block_word = 1;
    }
    return 0;
}

void output(int bytes, int words, int newlines, int flags, char const *name)
{
    char fmt[UINT32_FMT];
    int first_time = 1;

    if (flags & 1) {
        first_time = 0;
        allwrite(1, fmt, int32_fmt(fmt, newlines));
    }

    if (flags & 2) {
        if (first_time)
            first_time = 0;
        else
            fd_write(1, " ", 1);
        allwrite(1, fmt, int32_fmt(fmt, words));
    }

    if (flags & 4) {
        if (first_time)
            first_time = 0;
        else
            fd_write(1, " ", 1);
        allwrite(1, fmt, int32_fmt(fmt, bytes));
    }

    if (name) {
        fd_write(1, " ", 1);
        allwrite(1, name, strlen(name));
    }
    fd_write(1, "\n", 1);
}
