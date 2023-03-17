#!/bin/sh
CCL="$(cat ccflegs.txt 2> /dev/null)";
if test $? -ne 0; then
    echo "Unable to get the CFlags from ccflegs.txt";
    exit 1;
fi
DES="$(cat ./thirdparty/descriptorfleg.txt 2> /dev/null)";
if test $? -ne 0; then
    echo "Unable to get those extra dependecies in from ./thirdparty/descriptorfleg.txt";
    exit 1;
fi
g++ ./lib/mparc.cpp $CFLAGS $CCL $DES -c
exit $?;
