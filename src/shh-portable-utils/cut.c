#include <string.h>
#include <stdlib.h>

#include <skalibs/buffer.h>
#include <skalibs/skamisc.h>
#include <skalibs/disize.h>
#include <skalibs/sgetopt.h>
#include <skalibs/stralloc.h>
#include <skalibs/genalloc.h>
#include <skalibs/djbunix.h>
#include <skalibs/strerr2.h>
#include <skalibs/types.h>
#include <skalibs/bytestr.h>

#include "shhfuncs.h"

#define USAGE "cut -b list [-n] [file...] | -c list [file...] | -f list [-d delim] [-s] [file..]"

typedef void (*cut_func_t)(buffer*, genalloc*, char, int);

void byte_cut(buffer*, genalloc*, char, int);

void field_cut(buffer*, genalloc*, char, int);

void parse(genalloc*, char const*);

int cmp(void const *, void const *);

void simplify_range(genalloc*);

int main (int argc, char const *const *argv)
{
    cut_func_t cut = byte_cut;
    char buf[BUFFER_INSIZE];
    char delim = '\t';
    int mode_was_chosen = 0;
    int print_nodelim = 1;
    genalloc what = GENALLOC_ZERO;
    buffer buffer_in_;
    buffer *buffer_in;
    subgetopt l = SUBGETOPT_ZERO;
    PROG = "cut";

    for (;;) {
        int opt = subgetopt_r(argc, argv, "b:c:f:nd:s", &l);
        if (opt == -1)
            break;
        switch (opt) {
            case 'c':
                /* Fallthrough. This is a big TODO: supporting characters
                 * as per POSIX requires locale support, which I plan to
                 * add later. Locale support will be configurable at build time:
                 * embedded systems do not need this feature, but locales
                 * apparently will impose additional code and complexity.
                 */
            case 'b':
                if (mode_was_chosen)
                    strerr_dieusage(100, USAGE);
                mode_was_chosen = 1;
                cut = byte_cut;
                parse(&what, l.arg);
                break;
            case 'f': 
                if (mode_was_chosen)
                    strerr_dieusage(100, USAGE);
                mode_was_chosen = 1;
                parse(&what, l.arg);
                cut = field_cut;
                break;
            case 'n':
                break; /* See comment on -c */
            case 'd':
                delim = *l.arg;
                break;
            case 's':
                print_nodelim = 0;
                break;
            default:
                strerr_dieusage(100, USAGE);
        }
    }
    argc -= l.ind;
    argv += l.ind;

    if (!genalloc_len(disize, &what))
        strerr_dieusage(100, USAGE);
    simplify_range(&what);

    if (!argc) {
        cut(buffer_0, &what, delim, print_nodelim);
        return 0;
    }

    for (char const *const *filename = argv; *filename; filename++) {
        int fd = 0;
        if (!strcmp(*filename, "-"))
            buffer_in = buffer_0;
        else {
            fd = openb_read(*filename);
            if (fd < 0)
                strerr_diefu3sys(111, "Open file ", *filename, " for reading");
            buffer_init(&buffer_in_, buffer_flush1read, fd, buf, BUFFER_INSIZE);
            buffer_in = &buffer_in_;
        }
        cut(buffer_in, &what, delim, print_nodelim);
        if (fd > 0)
            fd_close(fd);
    }
    return 0;
}

void parse(genalloc *what, char const *raw)
{
    char const sep[] = {' ', '\t', ','};
    size_t i = 0;
    for (;;) {
        size_t newpos;
        disize interval = DISIZE_ZERO;
        if (raw[i] == '-') {
            interval.left = 1;
        } else {
            newpos = size_scan(raw + i, &interval.left);
            if (!newpos || !interval.left)
                strerr_dief2x(100, "invalid list: ", raw);
            i += newpos;
        }

        if (raw[i] == '-') {
            i++;
            newpos = size_scan(raw + i, &interval.right);
            if (newpos) {
                if (interval.right < interval.left)
                    strerr_dief2x(100, "invalid list: ", raw);
                i += newpos;
            }
        } else
            interval.right = interval.left;
        newpos = byte_chr(sep, sizeof(sep), raw[i]);

        if (!genalloc_append(disize, what, &interval))
            strerr_diefu1sys(111, "parse interval list");

        if (newpos < sizeof(sep))
            i++;
        else if (!raw[i])
            break;
        else
            strerr_dief2x(100, "invalid list: ", raw);
    }
}

int cmp(void const *a, void const *b)
{
    disize const *da = (disize const *)a;
    disize const *db = (disize const *)b;
    return da->left - db->left;
}

void simplify_range(genalloc *range)
{
    size_t cur = 0;
    size_t len = genalloc_len(disize, range);
    disize *s = genalloc_s(disize, range);
    qsort(s, genalloc_len(disize, range), sizeof(disize), cmp);
    for (size_t i = 1; i < len; i++) {
        if (!s[i].right)
            break;
        if (s[i].left > s[cur].right) {
            cur++;
            s[cur] = s[i];
        }
        else if (s[i].right > s[cur].right)
            s[cur].right = s[i].right;
    }
    genalloc_setlen(disize, range, cur + 1);
}


void byte_cut(buffer *in, genalloc *what, char delim, int print_nodelim)
{
    disize const *s = genalloc_s(disize, what);
    (void)delim;            /* not applicable */
    (void)print_nodelim;    /* not applicable */
    for (;;) {
        satmp.len = 0;
        int result = shhgetln(in, &satmp, '\n');
        if (result < 0)
            strerr_diefu1sys(111, "read line");
        if (!result)
            break;
        for (size_t i = 0; i < genalloc_len(disize, what); i++) {
            if (s[i].left > satmp.len - 1)
                break;
            if (s[i].right > satmp.len - 1 || !s[i].right) {
                buffer_put(buffer_1, satmp.s + s[i].left - 1,
                           satmp.len - s[i].left);
                break;
            }
            buffer_put(buffer_1, satmp.s + s[i].left - 1,
                       s[i].right - s[i].left + 1);
        }
        buffer_put(buffer_1, "\n", 1);
    }
}

void field_cut(buffer *in, genalloc *what, char delim, int print_nodelim)
{
    disize const *s = genalloc_s(disize, what);
    size_t len = genalloc_len(disize, what);
    for (;;) {
        size_t cur_field_end;
        int was_field_printed = 0;
        size_t cur_field_begin = 0;
        size_t cur_field = 1;
        size_t i = 0;
        satmp.len = 0;
        int result = shhgetln(in, &satmp, '\n');
        if (result < 0)
            strerr_diefu1sys(111, "read line");
        if (!result)
            break;

        cur_field_end = byte_chr(satmp.s, satmp.len - 1, delim);
        if (cur_field_end == satmp.len - 1 && print_nodelim) {
            buffer_put(in, satmp.s, satmp.len);
            continue;
        }

        while (cur_field_end < satmp.len - 1 && i < len) {
            if (s[i].left <= cur_field && 
                (cur_field <= s[i].right || !s[i].right)) {
                if (was_field_printed)
                    buffer_put(buffer_1, (char const *)&delim, 1);
                was_field_printed = 1;
                buffer_put(buffer_1, satmp.s + cur_field_begin,
                           cur_field_end - cur_field_begin);
            }
            if (cur_field <= s[i].right || !s[i].right) {
                cur_field_begin = cur_field_end + 1;
                cur_field_end = cur_field_begin +
                                byte_chr(satmp.s + cur_field_begin,
                                         satmp.len - 1 - cur_field_begin,
                                         delim);
                cur_field++;
            } else
                i++;
        }
        buffer_put(buffer_1, "\n", 1);
    }
}
