#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include <skalibs/buffer.h>
#include <skalibs/bytestr.h>
#include <skalibs/djbunix.h>
#include <skalibs/sgetopt.h>
#include <skalibs/skamisc.h>
#include <skalibs/stralloc.h>
#include <skalibs/strerr2.h>
#include <skalibs/uint32.h>

#define USAGE "uniq [-c|-d|-u] [-f fields] [-s char] [input_file [output_file]]"

#define byte_cmp(s1, l1, s2, l2) memcmp((s1), (s2), (l1) < (l2) ? (l1) : (l2))

char *prepare_string(stralloc const *sa, int fields, int chars);

size_t byte_notin(char const *s, size_t n, char const *sep, size_t len);

int shhgetln(buffer *b, stralloc *sa, char sep);

char *prepare_string(stralloc const *sa, int fields, int chars)
{
    size_t pos = 0;
    for (int j = 0; j < fields; j++) {
        pos += byte_notin(sa->s + pos, sa->len - pos, " \t", 2);
        if (pos >= sa->len)
            return sa->s + sa->len;
        pos += byte_in(sa->s + pos, sa->len - pos, " \t", 2);
        if (pos >= sa->len)
            return sa->s + sa->len;
    }
    return sa->s + (pos + chars > sa->len ? sa->len : pos + chars);
}

size_t byte_notin(char const *s, size_t n, char const *sep, size_t len)
{
    char const *t = s;
    while (n) {
        n--;
        if (!memchr(sep, *t, len))
            break;
        t++;
    }
    return t - s;
}

int main(int argc, char const *const *argv)
{
    char *prev_line_start, *curr_line_start;
    int counter, result;
    char buf_in[BUFFER_INSIZE];
    char buf_out[BUFFER_OUTSIZE];
    buffer buffer_in_, buffer_out_;
    buffer *buffer_in, *buffer_out;
    stralloc prev_line = STRALLOC_ZERO;
    stralloc curr_line = STRALLOC_ZERO;
    subgetopt l = SUBGETOPT_ZERO;
    int chars = 0, fields = 0;
    int do_count = 0, pr_repeated = 1, pr_unique = 1;

    PROG = "uniq";
    for (;;) {
        int opt = subgetopt_r(argc, argv, "cdf:s:u", &l);
        if (opt == -1)
            break;
        switch (opt) {
            case 'c':
                do_count = 1;
                break;
            case 'd':
                pr_unique = 0;
                break;
            case 'f':
                if (!int32_scan(l.arg, &fields))
                    strerr_dieusage(100, USAGE);
                break;
            case 's':
                if (!int32_scan(l.arg, &chars))
                    strerr_dieusage(100, USAGE);
                break;
            case 'u':
                pr_repeated = 0;
                break;
            default:
                strerr_dieusage(100, USAGE);
                break;
        }
    }
    argc -= l.ind;
    argv += l.ind;

    if (argc >= 3)
        strerr_dieusage(100, USAGE);

    if (argc >= 2) {
        int out = open_create(argv[1]);
        if (out < 0)
            strerr_diefu3sys(111, "open file ", argv[1], " for writing");
        buffer_init(&buffer_out_, buffer_write, out, buf_out, BUFFER_OUTSIZE);
        buffer_out = &buffer_out_;
    } else
        buffer_out = buffer_1;

    if (argc >= 1) {
        if (strcmp(argv[0], "-")) {
            int in = openb_read(argv[0]);
            if (in < 0)
                strerr_diefu3sys(111, "open file ", argv[0], " for reading");
            buffer_init(&buffer_in_, buffer_read, in, buf_in, BUFFER_INSIZE);
            buffer_in = &buffer_in_;
        } else
            buffer_in = buffer_0;
    } else
        buffer_in = buffer_0;

    result = shhgetln(buffer_in, &prev_line, '\n');
    if (result < 0)
        strerr_diefu1sys(111, "read line");

    if (!result) {
        if (!pr_unique)
            if (do_count)
                buffer_puts(buffer_out, "1 ");
        buffer_putflush(buffer_out, prev_line.s, prev_line.len);
        return 0;
    }

    prev_line.s[prev_line.len - 1] = '\0';
    prev_line.len--;
    counter = 1;
    prev_line_start = prepare_string(&prev_line, fields, chars);

    for (;;) {
        curr_line.len = 0;
        result = shhgetln(buffer_in, &curr_line, '\n');

        if (result < 0)
            strerr_diefu1sys(111, "read line");
        if (!result)
            break;

        curr_line.s[curr_line.len - 1] = '\0';
        curr_line.len--;

        curr_line_start = prepare_string(&curr_line, fields, chars);
        if (!strcmp(prev_line_start, curr_line_start))
            counter++;
        else {
            if ((counter > 1 && pr_repeated) || (counter == 1 && pr_unique)) {
                if (do_count) {
                    char fmt[UINT32_FMT];
                    buffer_put(buffer_out, fmt, int32_fmt(fmt, counter));
                    buffer_puts(buffer_out, " ");
                }
                buffer_put(buffer_out, prev_line.s, prev_line.len);
                buffer_putsflush(buffer_out, "\n");
            }
            prev_line_start = prev_line.s + (curr_line_start - curr_line.s);
            stralloc_copy(&prev_line, &curr_line);
            stralloc_0(&prev_line);
            prev_line.len--;
            counter = 1;
        }
    }
    if ((counter > 1 && pr_repeated) || (counter == 1 && pr_unique)) {
        if (do_count) {
            char fmt[UINT32_FMT];
            buffer_put(buffer_out, fmt, int32_fmt(fmt, counter));
            buffer_puts(buffer_out, " ");
        }
        buffer_put(buffer_out, prev_line.s, prev_line.len);
        buffer_putsflush(buffer_out, "\n");
    }
    return 0;
}

int shhgetln(buffer *b, stralloc *sa, char sep)
{
    size_t start = sa->len;
    for (;;) {
        ssize_t r = skagetln_nofill(b, sa, sep);
        if (r)
            return r;
        r = buffer_fill(b);
        if (r < 0)
            return r;
        if (!r) {
            if (sa->s && (sa->len > start)) {
                if (!stralloc_append(sa, sep))
                    return -1;
                return 1;
            } else
                return 0;
        }
    }
}
