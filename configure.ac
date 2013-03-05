# configure.ac

dnl Process this file with autoconf to produce a configure script.
dnl
dnl Minor upgrades (compatible ABI): increment the package version
dnl (third field in two places below) and set the PNGLIB_RELEASE
dnl variable.
dnl
dnl Major upgrades (incompatible ABI): increment the package major
dnl version (second field, or first if desired), set the minor
dnl to 0, set PNGLIB_MAJOR below *and* follow the instructions in
dnl Makefile.am to upgrade the package name.

dnl This is here to prevent earlier autoconf from being used, it
dnl should not be necessary to regenerate configure if the time
dnl stamps are correct
AC_PREREQ([2.68])

dnl Version number stuff here:

AC_INIT([libpng],[1.7.0beta06],[png-mng-implement@lists.sourceforge.net])
AC_CONFIG_MACRO_DIR([scripts])

# libpng does not follow GNU file name conventions (hence 'foreign')
# color-tests requires automake 1.11 or later
# silent-rules requires automake 1.11 or later
# dist-xz requires automake 1.11 or later
# 1.12.2 fixes a security issue in 1.11.2 and 1.12.1
# 1.13 is required for parallel tests
AM_INIT_AUTOMAKE([1.13 foreign dist-xz color-tests silent-rules])
# The following line causes --disable-maintainer-mode to be the default to
# configure, this is necessary because libpng distributions cannot rely on the
# time stamps of the autotools generated files being correct
AM_MAINTAINER_MODE

dnl configure.ac and Makefile.am expect automake 1.11.2 or a compatible later
dnl version; aclocal.m4 will generate a failure if you use a prior version of
dnl automake, so the following is not necessary (and is not defined anyway):
dnl AM_PREREQ([1.11.2])
dnl stop configure from automagically running automake

PNGLIB_VERSION=1.7.0beta06
PNGLIB_MAJOR=1
PNGLIB_MINOR=7
PNGLIB_RELEASE=0

dnl End of version number stuff

AC_CONFIG_SRCDIR([pngget.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_LANG([C])
AC_PROG_CC
AM_PROG_AS
LT_PATH_LD
AC_PROG_CPP
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

dnl libtool/libtoolize; version 2.4.2 is the tested version, this or any
dnl compatible later version may be used
LT_INIT([win32-dll])
LT_PREREQ([2.4.2])

# Some awks crash when confronted with pnglibconf.dfa, do a test run now
# to make sure this doesn't happen
AC_MSG_CHECKING([that AWK works])
if ${AWK} -f ${srcdir}/scripts/options.awk out="/dev/null" version=search\
   ${srcdir}/pngconf.h ${srcdir}/scripts/pnglibconf.dfa\
   ${srcdir}/pngusr.dfa 1>&2
then
   AC_MSG_RESULT([ok])
else
   AC_MSG_FAILURE([failed], 1)
fi

# This is a remnant of the old cc -E validation, where it may have been
# necessary to use a different preprocessor for .dfn files
DFNCPP="$CPP"
AC_SUBST(DFNCPP)

# -Werror cannot be passed to GCC in CFLAGS because configure will fail (it
# checks the compiler with a program that generates a warning), add the
# following option to deal with this
AC_ARG_VAR(PNG_COPTS,
   [additional flags for the C compiler, use this for options that would]
   [cause configure itself to fail])
AC_ARG_ENABLE(werror,
   AS_HELP_STRING([[[--enable-werror[=OPT]]]],
      [Pass -Werror or the given argument to the compiler if it is supported]),
   [test "$enable_werror" = "yes" && enable_werror="-Werror"
    if test "$enable_werror" != "no"; then
      sav_CFLAGS="$CFLAGS"
      CFLAGS="$enable_werror $CFLAGS"
      AC_MSG_CHECKING([if the compiler allows $enable_werror])
      AC_COMPILE_IFELSE(
         [AC_LANG_SOURCE([
            [int main(int argc, char **argv){]
            [return argv[argc-1][0];]
            [}]])],
         AC_MSG_RESULT(yes)
         PNG_COPTS="$PNG_COPTS $enable_werror",
         AC_MSG_RESULT(no))
      CFLAGS="$sav_CFLAGS"
    fi],)

# Checks for header files.
AC_HEADER_STDC
# Headers used in pngpriv.h:
AC_CHECK_HEADERS([assert.h stdlib.h string.h float.h fp.h math.h m68881.h])
# Additional headers used in pngconf.h:
AC_CHECK_HEADERS([limits.h stddef.h stdio.h setjmp.h time.h])
# Used by pngread.c (for the simplified API):
AC_CHECK_HEADERS([errno.h])
# The following are not required for the standard 'make' build of libpng:
# Additional requirements of contrib/libtests/makepng.c:
AC_CHECK_HEADERS([ctype.h])
# Additional requirements of contrib/libtests/pngvalid.c:
AC_CHECK_HEADERS([fenv.h])

# Checks for typedefs, structures, and compiler characteristics.
# Most of these have been removed from 1.7.0 because they checked for non-ANSI-C
# behaviors and the result were, anyway, not used by earlier versions of libpng.
AC_C_RESTRICT

# Checks for library functions; these need to be in order, least demanding to
# most because the -l directives are added to the head of the list at each
# check.
# ANSI-C functions: all are required but they don't have to be implemented in
# libc, it is perfectly ok to configure with an extra, wrapper, library preset
# in LIBS.  NOTE: this list may not be complete
AC_CHECK_FUNCS([memset memcpy strncpy],,
               AC_MSG_FAILURE([missing ANSI-C functions]))

# floor and ceil are required for FLOATING_POINT, but they can be turned off on
# the configure command line (and they are independent.)  Default is to 'on'
AC_ARG_ENABLE([floating-point],[Enable floating point APIs],,
              [enable_floating_point=yes])
AC_ARG_ENABLE([fixed-point],[Enable fixed point APIs],,
              [enable_fixed_point=yes])
AC_ARG_ENABLE([floating-arithmetic],[Use floating point arithmetic internally],,
              [enable_floating_arithmetic=yes])

# Check for a requirement for math library support
if test "$enable_floating_arithmetic" = yes
then
   AC_SEARCH_LIBS([floor],[m],,
                  [AC_MSG_WARN([not found])
                   enable_floating_arithmetic=no])
   AC_SEARCH_LIBS([frexp],[m],,
                  [AC_MSG_WARN([not found])
                   enable_floating_arithmetic=no])
   AC_SEARCH_LIBS([modf],[m],,
                  [AC_MSG_WARN([not found])
                   enable_floating_arithmetic=no])
   AC_SEARCH_LIBS([atof],[m],,
                  [AC_MSG_WARN([not found])
                   enable_floating_arithmetic=no])
   AC_SEARCH_LIBS([pow],[m],,
                  [AC_MSG_WARN([not found])
                   enable_floating_arithmetic=no])
fi

# Set the relevant defines if required (only if required), this will override
# anything on the command line.
if test "$enable_floating_point" != yes
then
   AC_MSG_NOTICE([Floating point APIs disabled])
   AC_DEFINE([PNG_NO_FLOATING_POINT],[1],
             [No support for floating point APIs])
fi
if test "$enable_fixed_point" != yes
then
   AC_MSG_NOTICE([Fixed point APIs disabled])
   AC_DEFINE([PNG_NO_FIXED_POINT],[1],
             [No support for fixed point APIs])
fi
if test "$enable_floating_arithmetic" != yes
then
   AC_MSG_NOTICE([Fixed point arithmetic will be used])
   AC_DEFINE([PNG_NO_FLOATING_ARITHMETIC],[1],
             [No support for floating point arithmetic])
fi

# The following is for pngvalid, to ensure it catches FP errors even on
# platforms that don't enable FP exceptions, the function appears in the math
# library (typically), it's not an error if it is not found.
AC_SEARCH_LIBS([feenableexcept],[m],AC_DEFINE([HAVE_FEENABLEEXCEPT],[1],
      [Define to 1 if the feenableexcept function is available]))
# Required by pngvalid:
AC_SEARCH_LIBS([ceil],[m])

# zlib checks and arguments;
#  package zlib: argument is the name of the library
#  zlib-header: the name of the header file
#  zlib-prefix: the prefix for library functions, if any
AC_ARG_WITH([zlib],
   AS_HELP_STRING([[[--with-zlib=<name>]]],
                  [base name of the zlib implementation (e.g. 'z')]),,
   [with_zlib='z'])

z_prefix=
AC_ARG_WITH([zlib-prefix],
   AS_HELP_STRING([[[--with-zlib-prefix=<prefix>]]],
                  [prefix for zlib API functions]),
   [if test "$withval" = "" -o "$withval" = yes; then
      with_zlib_prefix='z_'
      AC_DEFINE([PNG_ZLIB_PREFIX],[z_],[Standard zlib prefix])
   elif test "${withval}" != "no"; then
      AC_DEFINE_UNQUOTED([PNG_ZLIB_PREFIX],[${withval}],
         [User defined zlib prefix])
   else
      with_zlib_prefix=
   fi
   if test "$with_zlib_prefix" != ""; then
      AC_DEFINE([Z_PREFIX],[1],[Tell zlib.h to prefix function names])
      z_prefix="#define Z_PREFIX 1"
   fi])

AC_ARG_WITH([zlib-header],
   AS_HELP_STRING([[[--with-zlib-header='<file>' or '"file"']]],
      [C include argument to locate zlib header file, e.g. <zlib.h>, take
       care to put single quotes round this on the command line]),
   [AC_DEFINE_UNQUOTED([PNG_ZLIB_HEADER],[${withval}],[Zlib header file])],
   with_zlib_header='<zlib.h>')

# AC_CHECK_LIB isn't quite enough for this because we need the special header
# file too.
LIBS="-l${with_zlib} $LIBS"
AC_MSG_CHECKING(
   [[Library '${with_zlib}' and #include ${with_zlib_header} for symbol '${with_zlib_prefix}zlibVersion']])
AC_LINK_IFELSE(
   [AC_LANG_PROGRAM(
      [[
${z_prefix}
#include ${with_zlib_header}
      ]],
      [[(void)${with_zlib_prefix}zlibVersion();]])],
   AC_MSG_RESULT([ok]),
   AC_MSG_FAILURE([not found]))

AC_MSG_CHECKING([if using Solaris linker])
SLD=`$LD --version 2>&1 | grep Solaris`
if test "$SLD"; then
    have_solaris_ld=yes
    AC_MSG_RESULT(yes)
else
    have_solaris_ld=no
    AC_MSG_RESULT(no)
fi
AM_CONDITIONAL(HAVE_SOLARIS_LD, test "$have_solaris_ld" = "yes")

AC_MSG_CHECKING([if libraries can be versioned])
# Special case for PE/COFF platforms: ld reports
# support for version-script, but doesn't actually
# DO anything with it.
case $host in
*cygwin* | *mingw32* | *interix* )
    have_ld_version_script=no
    AC_MSG_RESULT(no)
;;
* )

if test "$have_solaris_ld" = "yes"; then
    GLD=`$LD --help < /dev/null 2>&1 | grep 'M mapfile'`
else
    GLD=`$LD --help < /dev/null 2>/dev/null | grep version-script`
fi

if test "$GLD"; then
    have_ld_version_script=yes
    AC_MSG_RESULT(yes)
else
    have_ld_version_script=no
    AC_MSG_RESULT(no)
    AC_MSG_WARN(*** You have not enabled versioned symbols.)
fi
;;
esac

AM_CONDITIONAL(HAVE_LD_VERSION_SCRIPT, test "$have_ld_version_script" = "yes")

if test "$have_ld_version_script" = "yes"; then
    AC_MSG_CHECKING([for symbol prefix])
    SYMBOL_PREFIX=`echo "PREFIX=__USER_LABEL_PREFIX__" \
                  | ${CPP-${CC-gcc} -E} - 2>&1 \
                  | ${EGREP-grep} "^PREFIX=" \
                  | ${SED-sed} -e "s:^PREFIX=::" -e "s:__USER_LABEL_PREFIX__::"`
    AC_SUBST(SYMBOL_PREFIX)
    AC_MSG_RESULT($SYMBOL_PREFIX)
fi

# Substitutions for .in files
AC_SUBST(PNGLIB_VERSION)
AC_SUBST(PNGLIB_MAJOR)
AC_SUBST(PNGLIB_MINOR)
AC_SUBST(PNGLIB_RELEASE)

# Additional arguments (and substitutions)
# Allow the pkg-config directory to be set
AC_ARG_WITH(pkgconfigdir,
   AS_HELP_STRING([[[--with-pkgconfigdir]]],
      [Use the specified pkgconfig dir (default is libdir/pkgconfig)]),
   [pkgconfigdir=${withval}],
   [pkgconfigdir='${libdir}/pkgconfig'])

AC_SUBST([pkgconfigdir])
AC_MSG_NOTICE([[pkgconfig directory is ${pkgconfigdir}]])

# Make the *-config binary config scripts optional
AC_ARG_WITH(binconfigs,
   AS_HELP_STRING([[[--with-binconfigs]]],
      [Generate shell libpng-config scripts as well as pkg-config data]
      [@<:@default=yes@:>@]),
   [if test "${withval}" = no; then
      binconfigs=
      AC_MSG_NOTICE([[libpng-config scripts will not be built]])
    else
      binconfigs='${binconfigs}'
    fi],
   [binconfigs='${binconfigs}'])
AC_SUBST([binconfigs])

# Support for prefixes to the API function names; this will generate defines
# at the start of the build to rename exported library functions
AC_ARG_WITH(libpng-prefix,
   AS_HELP_STRING([[[--with-libpng-prefix]]],
      [prefix libpng exported function (API) names with the given value]),
   [if test "${withval:-no}" != "no"; then
      AC_SUBST([PNG_PREFIX], [${withval}])
    fi])
AM_CONDITIONAL([DO_PNG_PREFIX], [test "${with_libpng_prefix:-no}" != "no"])

# Because GCC by default assembles code with an executable stack, even though it
# compiles C code with a non-executable stack, it is necessary to do a fixup
# here (this may by GCC specific)
AC_SUBST([AM_CCASFLAGS], [-Wa,--noexecstack])

AC_ARG_ENABLE([arm-neon],
   AS_HELP_STRING([[[--enable-arm-neon]]],
      [Enable ARM NEON optimizations: =off, check, api, on:]
      [off: disable the optimizations; check: use internal checking code]
      [(deprecated and poorly supported); api: disable by default, enable by]
      [a call to png_set_option; on: turn on unconditionally.  The]
      [default is 'on' if __ARM_NEON__ is defined, otherwise 'off'.]),
   [case "$enableval" in
      off)
         AC_DEFINE([PNG_NO_ARM_NEON], [],
                   [Disable ARM Neon optimizations]);;
      check)
         AC_DEFINE([PNG_ARM_NEON_CHECK_SUPPORTED], [],
                   [Check for ARM Neon support at run-time]);;
      api)
         AC_DEFINE([PNG_ARM_NEON_API_SUPPORTED], [],
                   [Turn on ARM Neon optimizations at run-time]);;
      on)
         AC_DEFINE([PNG_ARM_NEON_SUPPORTED], [],
                   [Enable ARM Neon optimizations]);;
      *)
         AC_MSG_ERROR([arm-neon: option value required (off/check/api/on)]);;
   esac])
AM_CONDITIONAL([PNG_ARM_NEON], [test "${enable_arm_neon:-no}" != "no"])

AC_MSG_NOTICE([[Extra options for compiler: $PNG_COPTS]])

# Config files, substituting as above
AC_CONFIG_FILES([Makefile libpng.pc])
AC_CONFIG_FILES([libpng-config], [chmod +x libpng-config])

AC_OUTPUT
