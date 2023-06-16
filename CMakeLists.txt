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

# opts
option(SUBMOD "Use submodules instead of fetchcontent" OFF)

# include some magic ???
include(GNUInstallDirs)
include(FetchContent)

# add library
add_subdirectory(./lib)


if(SUBMOD)
else()
  message(INFO "fetchcontent mode!")
  # fetch library
  FetchContent_Declare(
    cli11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11
    GIT_TAG f47ab7131326ed9925eb8dca63055f1e5cfa3569
  )
  FetchContent_MakeAvailable(cli11)

#   FetchContent_Declare(
#     json
#     GIT_REPOSITORY https://github.com/nlohmann/json
#     GIT_TAG 5d2754306d67d1e654a1a34e1d2e74439a9d53b3
#   )
#   FetchContent_MakeAvailable(json)
# 
# 
#   FetchContent_Declare(
#     b64lib
#     GIT_REPOSITORY https://github.com/tobiaslocker/base64
#     GIT_TAG db7c6834bd4f733899bd93218926247a659d8924
#   )
#   FetchContent_MakeAvailable(b64lib)
endif()


# add executable
add_executable(MPARCX ./src/main.cpp)

# configure include directory
include_directories(./lib)


# enforce post-target-definition property configurations
set_property(TARGET MPARCX PROPERTY C_STANDARD_REQUIRED TRUE)

# link the library in
target_link_libraries(MPARCX PRIVATE MPARC)
target_include_directories(MPARCX PRIVATE "${cli11_SOURCE_DIR}/include")

# installer
install(TARGETS MPARCX DESTINATION CMAKE_INSTALL_BINDIR)