# Process this file with autoconf to produce a configure script.
m4_define(PNGPKG_VERSION,[1.2.8-automake])
AC_INIT([libpng], PNGPKG_VERSION, png-implement@ccrc.wustl.edu)
AM_INIT_AUTOMAKE([libpng], PNGPKG_VERSION)
AC_CONFIG_SRCDIR([pngget.c])
AM_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

# Checks for header files.
AC_PATH_X
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h malloc.h stdlib.h string.h strings.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_STRTOD

AC_CHECK_FUNCS([memset pow strrchr])
AC_CHECK_FUNC(sqrt, , AC_CHECK_LIB(m, sqrt,, [cannot find sqrt]))
AC_CHECK_LIB(z, zlibVersion,, AC_ERROR([ZLib not installed]))

AC_ARG_WITH(pkgconfigdir, AC_HELP_STRING([--with-pkgconfigdir],[Use the specified pkgconfig dir (default is prefix/lib/pkgconfig)]),[PKGCONFIGDIR=${withval}],[PKGCONFIGDIR='${prefix}/lib/pkgconfig'])

AC_MSG_CHECKING([pkgconfig directory])
AC_MSG_RESULT([$PKGCONFIGDIR])
AC_SUBST([PKGCONFIGDIR])

AC_SUBST(LIBPNG12_VERSION_INFO,0:0:0)
AC_SUBST(LIBPNG3_VERSION_INFO,3:0:0)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
