cmake_minimum_required(VERSION 3.10)

# Utilities
MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

# Fix behavior of CMAKE_C_STANDARD when targeting macOS.
if(POLICY CMP0025)
    cmake_policy(SET CMP0025 NEW)
endif()

# set the project name
project(MPARC CXX)

# set cmake standards
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

# include some magic ???
include(GNUInstallDirs)
include(FetchContent)

# add external libraries
# FetchContent_Declare(b64
# GIT_REPOSITORY https://github.com/tobiaslocker/base64
# GIT_TAG db7c6834bd4f733899bd93218926247a659d8924
# )
file(DOWNLOAD https://raw.githubusercontent.com/tobiaslocker/base64/db7c6834bd4f733899bd93218926247a659d8924/include/base64.hpp "${CMAKE_SOURCE_DIR}/base64.hpp" SHOW_PROGRESS)


# Test some stuff
SUBDIRLIST(subdirs "${CMAKE_SOURCE_DIR}")
FOREACH(subdir ${SUBDIRS})
    message(INFO "DIR|${CMAKE_SOURCE_DIR}> ${subdir}")
ENDFOREACH()

# continue again

FetchContent_Declare(json
GIT_REPOSITORY https://github.com/nlohmann/json
GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

# add library
add_library(MPARC ./mparc.cpp) # Main one
target_link_libraries(MPARC PRIVATE nlohmann_json::nlohmann_json)

# enforce post-target-definition property configurations
set_property(TARGET MPARC PROPERTY C_STANDARD_REQUIRED TRUE)
set_property(TARGET MPARC PROPERTY CXX_STANDARD_REQUIRED TRUE)

# install
install(TARGETS MPARC DESTINATION CMAKE_INSTALL_LIBDIR)