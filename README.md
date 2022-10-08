# MPARC

MPAR reimplemented in C instead of C++.

Archive format, but you can make it into a key value database store somehow.

The code I written may be banned in embedded systems and absolutely destroyed in projects where dynamic allocations are not allowed.

## Enough of the boring stuff

You just need libc for standard functions, but (the boehm garbage collector/dmalloc) can be used to debug memory issues.

> Also this can be built with the old makefile (Makefile.dumb) and that is the most trusted one out of all, autotools (Generate your self, my environment breaks it) or CMake (it works, just do it in the build/ directory).