cmake_minimum_required(VERSION 3.10)

# set the project name
project(MPARC C)

# set cmake standards
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

# add executable
add_executable(MPARC main.c mparc.c)