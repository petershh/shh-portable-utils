#include <sys/types.h>

#include <skalibs/stralloc.h>
#include <skalibs/genalloc.h>
#include <skalibs/buffer.h>

#define S_ISALL (S_IRWXU | S_IRWXG | S_IRWXO | S_ISUID | S_ISGID | S_ISVTX)

struct chmod_directive_s {
    mode_t who;
    int action;
    mode_t perm;
    int permcopy;
    int dir_x;
};

typedef struct chmod_directive_s chmod_directive;

#define CHMOD_DIRECTIVE_ZERO { 0, 0, 0, 0, 0 }

int parse_mode_octal(char const*, mode_t*);
int parse_mode_symbolic(char const*, genalloc*);
mode_t change_mode(mode_t, chmod_directive*, size_t, mode_t);


size_t byte_notin(char const *s, size_t n, char const *sep, size_t len);

int shhgetln(buffer *b, stralloc *sa, char sep);


uid_t parse_user(char const*);
gid_t parse_group(char const*);
