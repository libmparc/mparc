cmake_minimum_required(VERSION 3.10)

# Fix behavior of CMAKE_C_STANDARD when targeting macOS.
if (POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif ()

# set the project name
project(MPARC C)

# set cmake standards
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
set(C_STANDARD_REQUIRED ON)

# add executable
add_executable(MPARC main.c mparc.c)