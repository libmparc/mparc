#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([MPARC], [0.1], [https://github.com/MOALib/mparc/issues])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([./lib/mparc.c], [./src/mainv2.c])
# AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])

# Checks for programs.
# AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([inttypes.h stdint.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([memchr memset mkdir strcspn strrchr strspn strtoull])

AC_OUTPUT
