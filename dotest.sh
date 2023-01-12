#!/bin/sh

# CUSTOM PRINTER
printer(){
    echo "CRUDE MPARC TESTING RIG>>>" "$@"
}

# VARS
FAILURE_EXIT_CODE=1



printer "Building binaries"
make -f [mM]akefile.dumb || exit $FAILURE_EXIT_CODE # build everything

printer "Testing binaries"
./tmparc.exe || exit $FAILURE_EXIT_CODE # test the test executable
printer "Testing C++ binaries"
./cxmparc.exe || exit $FAILURE_EXIT_CODE # test the c++ test executable

printer "Making archive files"
make -f [mM]akefile.dumb mkar || exit $FAILURE_EXIT_CODE # make archive

printer "Cleaning environment"
make -f [mM]akefile.dumb clean || exit $FAILURE_EXIT_CODE # clean



# cleanup
exit 0