#!/bin/sh
g++ ./lib/mparc.cpp $CFLAGS $(cat ccflegs.txt 2> /dev/null) $(cat ./thirdparty/descriptorfleg.txt 2> /dev/null) -c
exit $?;
