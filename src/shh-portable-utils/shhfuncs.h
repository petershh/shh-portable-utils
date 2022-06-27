#include <skalibs/stralloc.h>
#include <skalibs/buffer.h>

size_t byte_notin(char const *s, size_t n, char const *sep, size_t len);

int shhgetln(buffer *b, stralloc *sa, char sep);
