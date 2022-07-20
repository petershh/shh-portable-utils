#include <sys/resource.h>

#include <skalibs/strerr2.h>
#include <skalibs/sgetopt.h>
#include <skalibs/types.h>

#define "renice [-g|-p|-u] -n increment ID..."

int main (int argc, char const *const *argv)
{
    int incr;
    subgetopt l = SUBGETOPT_ZERO;
    
    PROG = "renice";

    for (;;)
}
