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
- (**Optional**) [nsss](https://skarnet.org/software/nsss) if you have musl
  and want nsswitch-like functionality, or if you just want sane nsswitch on
  your system. This is build-time and run-time dependency.

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
    - [uutils](https://github.com/uutils/coreutils): there are many reasons:
        1. They reimplement GNU Coreutils with all its bloat.
        2. This project uses Rust. This language does not produce small
           binaries, while system software *must* be small and efficient - do
           not waste user's resources!
        3. Also, this language encourages dependencies which are downloaded via
           [their own repository](https://crates.io). Dependencies are a great
           responsibility on their own, and downloading them via de facto
           unmoderated repository opens [giant possibilities for supply chain
           attacks](https://drewdevault.com/2022/05/12/Supply-chain-when-will-we-learn.html).
           shh-utils uses limited number of dependencies and author reviews
           its code. These dependencies are meant to be supplied by your
           software distribution's package manager. shh-utils author is
           committed to cooperation with distribution maintainers. Dependencies
           are cleary defined, and the package is easy to build.
        4. This project aims to be cross-platform. I don't see the point of
           running Unix utilities on Windows. Some utilities (chmod, chown)
           assume POSIX and make no sense on Windows. Also, Windows terminal
           capabilities will always be inferior to Unix. If you want Unix
           utilities -- install Unix.
    - [Busybox](https://busybox.net), [Toybox](https://landley.net/toybox):
      these projects provide multicall binaries. This approach has its
      advantages, but it also has its drawbacks. The goal of shh-utils is
      provide standalone binaries.
    - [sbase](https://core.suckless.org/sbase/): suckless stuff, which is good,
      but they use standard C interfaces like stdio, which I deem bad.
      shh-utils use better designed skalibs interfaces when appropriate.
- Last but not least: NIH syndrome of course!
