#!/bin/bash
# The comprehensive testing script



# VARS
FAILURE_EXIT_CODE=1
SANITIZERS="${SANITIZERS:=}"
LDFLAGS="${LDFLAGS:=}"
CPPFLAGS="${CPPFLAGS:=}"
CC=${CC:="gcc"}
CXX=${CXX:="g++"}


# CUSTOM PRINTER
printer(){
    echo "CRUDE MPARC TESTING RIG>>>" "$@";
};

# CUSTOM DIE
die(){
    printer "NO I FAILED, GOTTA DIE: " "$@";
    exit $FAILURE_EXIT_CODE;
};


perform_makefile(){
    printer "Building binaries";
    make -f [mM]akefile.dumb CC="$CC" CXX="$CXX" LDFLAGS="$LDFLAGS" CPPFLAGS="$CPPFLAGS" SANITIZERS="$SANITIZERS" || die "Makefile build failed"; # build everything

    printer "Testing binaries";
    ./tmparc.exe || die "Binaries failed"; # test the test executable
    printer "Testing C++ binaries";
    ./cxmparc.exe || die "C++ binaries failed"; # test the c++ test executable

    printer "Making archive files";
    make -f [mM]akefile.dumb mkar || die "Failed to make archive"; # make archive

    printer "Cleaning environment";
    make -f [mM]akefile.dumb clean || die "Failed to clean (HOW?)"; # clean
}

perform_cmake(){
    # you can set this
    CMAKE_BUILD_DIR=${CMAKE_BUILD_DIR:="./cmk"};
    CURRENT_PRE_CMAKE_BUILD_DIR=`pwd`;

    # test if dir exists
    if test -d "$CMAKE_BUILD_DIR"; then
        die "CMake build directory $CMAKE_BUILD_DIR already exists, please remove it ($CMAKE_BUILD_DIR)";
    fi

    printer "Making and going into CMake build directory $CMAKE_BUILD_DIR";
    mkdir "$CMAKE_BUILD_DIR";
    cd "$CMAKE_BUILD_DIR";


    printer "Generating a Makefile project from CMake";
    cmake ..; # sudo make me a makefile
    printer "Running the generated makefile";
    make; # run makefile

    printer "Cleaning CMake artifacts for the Ninja build"
    rm -rf *;

    printer "Generating a Ninja project from CMake";
    cmake .. -G Ninja; # sudo make me a build.ninja
    printer "Running the generated build.ninja file";
    ninja; # run ninja, run build.ninja


    printer "Going out of CMake build directory";
    cd "$CURRENT_PRE_CMAKE_BUILD_DIR";

    printer "Destroying CMake build directory $CMAKE_BUILD_DIR";
    rm -rf "$CMAKE_BUILD_DIR";
}

perform_scons(){
    printer "Building binaries with scons";
    scons || die "Scons failed to build"; # scons build

    printer "Cleaning binaries with scons";
    scons -c || die "Scons failed to clean"; # scons clean

    printer "Cleaning other mess";
    make -f [mM]akefile.dumb clean || die "Failed to clean others (why)"; # makefile clean
}



perform_makefile;
printf "\n\n";
perform_cmake;
printf "\n\n";
perform_scons;



# cleanup
exit 0;