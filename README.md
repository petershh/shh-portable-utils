# shh-portable-utils

Shhh!

This project aims to create a set of small, correct and POSIX-conformant set of
default utilities for the UNIX system, like echo and chmod.

Powered by skalibs.
Works best with musl libc.

## Dependencies
- POSIX system with default C programming environment.
- GNU Make, version 3.81 or later;
- [skalibs](https://skarnet.org/software/skalibs). This is a build-time
  dependency and a run-time dependency if you do not link binaries statically.

## Compilation and installation
```
./configure && make && sudo make install
```
will do. Alternatively, it can be built with muon build system and samurai (not
guaranteed to work!):
```
muon setup build && samu -C build && muon install -C build
```

## License
This is free software licensed under MIT license. See COPYING for details.

## TODO
- Draw the rest of the freaking owl!
- Create shh-linux-utils, a set of base utilities that cannot be implemented in
  a nonportable fashion. This utilities are going to be Linux-specific, I
  primarily use Linux distros and BSDs are unlikely to adopt them.
- I18n. I believe that utilities should be able to display text in the user's
  native language. But, since language support drastically increases size of
  programs, there should be a possibility to disable this at compile-tume.
- Documentation, obviously. For now, POSIX will do.
- Move repository to another server.

## Why
- As far as the author knows, none of existing projects provide POSIX-conformant
  set of utilities, while there is a demand for such set.
- Existing projects do not satisfy the author for some reasons:
    - [GNU Coreutils](https//www.gnu.prg/software/coreutils): well, this is GNU,
      which is translated as 'bloated, inefficient, insecure'.
    - [Busybox](https://busybox.net), [Toybox](https://landley.net/toybox):
      these projects provide multicall binaries. This approach has its
      advantages, but it also has its drawbacks. The goal of shh-utils is
      provide standalone binaries.
    - [sbase](https://core.suckless.org/sbase/): Idk, need to check this out.
- Last but not least: NIH syndrome of course!
