# Process this file with autoconf to produce a configure script.

# Prelude.
#AC_PREREQ([2.59])
AC_INIT([pavarotti], [0.0.0], [viking AT pillageandplunder.net])

# unique source file --- primitive safety check
AC_CONFIG_SRCDIR([src/main.c])

# place to put some extra build scripts installed
#AC_CONFIG_AUX_DIR([build-aux])

# fairly severe build strictness
# change foreign to gnu or gnits to comply with gnu standards
AM_INIT_AUTOMAKE([-Wall -Werror foreign 1.9.6])

# Checks for programs.
CFLAGS="-g3"
AC_PROG_CC
AM_PROG_CC_C_O

# Checks for libraries.
AC_CHECK_LIB([sqlite3], [sqlite3_open])
AC_CHECK_LIB([id3tag], [id3_file_open])
AM_PATH_GTK_2_0(2.2.0,,AC_MSG_ERROR(pavarotti needs GTK+ 2.2.0))

PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h stdio.h string.h errno.h unistd.h sys/stat.h sys/types.h])
AC_CHECK_HEADERS([sqlite3.h],,
                 [
                   echo
                   echo "ERROR: sqlite3 headers not found!"
                   exit
                 ])
AC_CHECK_HEADERS([id3tag.h],,
                 [
                   echo
                   echo "ERROR: id3tag headers not found!"
                   exit
                 ])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC

# Output files
AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 tests/Makefile])

AC_OUTPUT
