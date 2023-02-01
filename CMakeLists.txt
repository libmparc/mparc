cmake_minimum_required(VERSION 3.10)

# Fix behavior of CMAKE_C_STANDARD when targeting macOS.
if (POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif ()

# set the project name
project(MPARCX C)

# set cmake standards
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

# include some magic ???
include(GNUInstallDirs)

# add library
add_subdirectory(./lib)

# add executable
add_executable(MPARCX ./src/mainv2.c)

# configure include directory
include_directories(./lib)

# enforce post-target-definition property configurations
set_property(TARGET MPARCX PROPERTY C_STANDARD_REQUIRED TRUE)

# link the library in
target_link_libraries(MPARCX MPARC)

# installer
install(TARGETS MPARCX DESTINATION CMAKE_INSTALL_BINDIR)