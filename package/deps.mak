#
# This file has been generated by tools/gen-deps.sh
#

src/shh-portable-utils/basename.o src/shh-portable-utils/basename.lo: src/shh-portable-utils/basename.c
src/shh-portable-utils/byte_notin.o src/shh-portable-utils/byte_notin.lo: src/shh-portable-utils/byte_notin.c src/shh-portable-utils/shhfuncs.h
src/shh-portable-utils/cat.o src/shh-portable-utils/cat.lo: src/shh-portable-utils/cat.c
src/shh-portable-utils/chgrp.o src/shh-portable-utils/chgrp.lo: src/shh-portable-utils/chgrp.c
src/shh-portable-utils/chmod.o src/shh-portable-utils/chmod.lo: src/shh-portable-utils/chmod.c
src/shh-portable-utils/chown.o src/shh-portable-utils/chown.lo: src/shh-portable-utils/chown.c
src/shh-portable-utils/cut.o src/shh-portable-utils/cut.lo: src/shh-portable-utils/cut.c src/shh-portable-utils/shhfuncs.h
src/shh-portable-utils/dirname.o src/shh-portable-utils/dirname.lo: src/shh-portable-utils/dirname.c
src/shh-portable-utils/false.o src/shh-portable-utils/false.lo: src/shh-portable-utils/false.c
src/shh-portable-utils/link.o src/shh-portable-utils/link.lo: src/shh-portable-utils/link.c
src/shh-portable-utils/shhgetln.o src/shh-portable-utils/shhgetln.lo: src/shh-portable-utils/shhgetln.c src/shh-portable-utils/shhfuncs.h
src/shh-portable-utils/tee.o src/shh-portable-utils/tee.lo: src/shh-portable-utils/tee.c
src/shh-portable-utils/true.o src/shh-portable-utils/true.lo: src/shh-portable-utils/true.c
src/shh-portable-utils/uname.o src/shh-portable-utils/uname.lo: src/shh-portable-utils/uname.c
src/shh-portable-utils/uniq.o src/shh-portable-utils/uniq.lo: src/shh-portable-utils/uniq.c src/shh-portable-utils/shhfuncs.h

basename: EXTRA_LIBS := -lskarnet
basename: src/shh-portable-utils/basename.o
cat: EXTRA_LIBS := -lskarnet
cat: src/shh-portable-utils/cat.o
chgrp: EXTRA_LIBS := ${SOCKET_LIB} -lskarnet
chgrp: src/shh-portable-utils/chgrp.o ${LIBNSSS}
chmod: EXTRA_LIBS := -lskarnet
chmod: src/shh-portable-utils/chmod.o
chown: EXTRA_LIBS := ${SOCKET_LIB} -lskarnet
chown: src/shh-portable-utils/chown.o ${LIBNSSS}
cut: EXTRA_LIBS := -lskarnet
cut: src/shh-portable-utils/cut.o src/shh-portable-utils/shhgetln.o
dirname: EXTRA_LIBS := -lskarnet
dirname: src/shh-portable-utils/dirname.o
false: EXTRA_LIBS :=
false: src/shh-portable-utils/false.o
link: EXTRA_LIBS := -lskarnet
link: src/shh-portable-utils/link.o
tee: EXTRA_LIBS := -lskarnet
tee: src/shh-portable-utils/tee.o
true: EXTRA_LIBS :=
true: src/shh-portable-utils/true.o
uname: EXTRA_LIBS := -lskarnet
uname: src/shh-portable-utils/uname.o
uniq: EXTRA_LIBS := -lskarnet
uniq: src/shh-portable-utils/uniq.o src/shh-portable-utils/shhgetln.o src/shh-portable-utils/byte_notin.o
