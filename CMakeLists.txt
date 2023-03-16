cmake_minimum_required(VERSION 3.10)

# Fix behavior of CMAKE_C_STANDARD when targeting macOS.
if (POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif ()

# set the project name
project(MPARCX CXX)

# set cmake standards
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set/option others
option(USE_FETCHCONTENT_ALL "Fetch content everything" OFF)
option(USE_EXTERNAL_ALL "External usage the libraries (???)" OFF)
option(USE_EXTERNAL_JSON "No external ones" OFF)

# include some magic ???
include(GNUInstallDirs)

# add library
add_subdirectory(./thirdparty)
add_subdirectory(./lib)

# add executable
add_executable(MPARCX ./src/main.cpp)

# configure include directory
include_directories(./lib)

# enforce post-target-definition property configurations
set_property(TARGET MPARCX PROPERTY C_STANDARD_REQUIRED TRUE)

# link the library in
target_link_libraries(MPARCX MPARC)

# installer
install(TARGETS MPARCX DESTINATION CMAKE_INSTALL_BINDIR)