# MPARC

![Logo is the MPARC archive behind an #ffffff background lmao](./img/logo.png)

MPAR reimplemented in C instead of C++.

Archive format, but you can make it into a key value database store somehow.

The code I written may be banned in embedded systems and absolutely destroyed in projects where dynamic allocations are not allowed.

## Enough of the boring stuff

You only need libc for standard functions, but (the boehm garbage collector/dmalloc) can be used to debug memory issues.

> Also this can be built with the old makefile (Makefile.dumb) (trust me, I am the old method), autotools (Less broken, but still yes you need to make it yourself) or CMake (it works, please do it in the build/ directory).