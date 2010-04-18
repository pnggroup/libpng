
/* png.h - header file for PNG reference library
 *
 * libpng version 1.5.0beta18 - April 18, 2010
 * Copyright (c) 1998-2010 Glenn Randers-Pehrson
 * (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
 * (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)
 *
 * This code is released under the libpng license (See LICENSE, below)
 *
 * Authors and maintainers:
 *   libpng versions 0.71, May 1995, through 0.88, January 1996: Guy Schalnat
 *   libpng versions 0.89c, June 1996, through 0.96, May 1997: Andreas Dilger
 *   libpng versions 0.97, January 1998, through 1.5.0beta18 - April 18, 2010: Glenn
 *   See also "Contributing Authors", below.
 *
 * Note about libpng version numbers:
 *
 *   Due to various miscommunications, unforeseen code incompatibilities
 *   and occasional factors outside the authors' control, version numbering
 *   on the library has not always been consistent and straightforward.
 *   The following table summarizes matters since version 0.89c, which was
 *   the first widely used release:
 *
 *    source                 png.h  png.h  shared-lib
 *    version                string   int  version
 *    -------                ------ -----  ----------
 *    0.89c "1.0 beta 3"     0.89      89  1.0.89
 *    0.90  "1.0 beta 4"     0.90      90  0.90  [should have been 2.0.90]
 *    0.95  "1.0 beta 5"     0.95      95  0.95  [should have been 2.0.95]
 *    0.96  "1.0 beta 6"     0.96      96  0.96  [should have been 2.0.96]
 *    0.97b "1.00.97 beta 7" 1.00.97   97  1.0.1 [should have been 2.0.97]
 *    0.97c                  0.97      97  2.0.97
 *    0.98                   0.98      98  2.0.98
 *    0.99                   0.99      98  2.0.99
 *    0.99a-m                0.99      99  2.0.99
 *    1.00                   1.00     100  2.1.0 [100 should be 10000]
 *    1.0.0      (from here on, the   100  2.1.0 [100 should be 10000]
 *    1.0.1       png.h string is   10001  2.1.0
 *    1.0.1a-e    identical to the  10002  from here on, the shared library
 *    1.0.2       source version)   10002  is 2.V where V is the source code
 *    1.0.2a-b                      10003  version, except as noted.
 *    1.0.3                         10003
 *    1.0.3a-d                      10004
 *    1.0.4                         10004
 *    1.0.4a-f                      10005
 *    1.0.5 (+ 2 patches)           10005
 *    1.0.5a-d                      10006
 *    1.0.5e-r                      10100 (not source compatible)
 *    1.0.5s-v                      10006 (not binary compatible)
 *    1.0.6 (+ 3 patches)           10006 (still binary incompatible)
 *    1.0.6d-f                      10007 (still binary incompatible)
 *    1.0.6g                        10007
 *    1.0.6h                        10007  10.6h (testing xy.z so-numbering)
 *    1.0.6i                        10007  10.6i
 *    1.0.6j                        10007  2.1.0.6j (incompatible with 1.0.0)
 *    1.0.7beta11-14        DLLNUM  10007  2.1.0.7beta11-14 (binary compatible)
 *    1.0.7beta15-18           1    10007  2.1.0.7beta15-18 (binary compatible)
 *    1.0.7rc1-2               1    10007  2.1.0.7rc1-2 (binary compatible)
 *    1.0.7                    1    10007  (still compatible)
 *    1.0.8beta1-4             1    10008  2.1.0.8beta1-4
 *    1.0.8rc1                 1    10008  2.1.0.8rc1
 *    1.0.8                    1    10008  2.1.0.8
 *    1.0.9beta1-6             1    10009  2.1.0.9beta1-6
 *    1.0.9rc1                 1    10009  2.1.0.9rc1
 *    1.0.9beta7-10            1    10009  2.1.0.9beta7-10
 *    1.0.9rc2                 1    10009  2.1.0.9rc2
 *    1.0.9                    1    10009  2.1.0.9
 *    1.0.10beta1              1    10010  2.1.0.10beta1
 *    1.0.10rc1                1    10010  2.1.0.10rc1
 *    1.0.10                   1    10010  2.1.0.10
 *    1.0.11beta1-3            1    10011  2.1.0.11beta1-3
 *    1.0.11rc1                1    10011  2.1.0.11rc1
 *    1.0.11                   1    10011  2.1.0.11
 *    1.0.12beta1-2            2    10012  2.1.0.12beta1-2
 *    1.0.12rc1                2    10012  2.1.0.12rc1
 *    1.0.12                   2    10012  2.1.0.12
 *    1.1.0a-f                 -    10100  2.1.1.0a-f (branch abandoned)
 *    1.2.0beta1-2             2    10200  2.1.2.0beta1-2
 *    1.2.0beta3-5             3    10200  3.1.2.0beta3-5
 *    1.2.0rc1                 3    10200  3.1.2.0rc1
 *    1.2.0                    3    10200  3.1.2.0
 *    1.2.1beta1-4             3    10201  3.1.2.1beta1-4
 *    1.2.1rc1-2               3    10201  3.1.2.1rc1-2
 *    1.2.1                    3    10201  3.1.2.1
 *    1.2.2beta1-6            12    10202  12.so.0.1.2.2beta1-6
 *    1.0.13beta1             10    10013  10.so.0.1.0.13beta1
 *    1.0.13rc1               10    10013  10.so.0.1.0.13rc1
 *    1.2.2rc1                12    10202  12.so.0.1.2.2rc1
 *    1.0.13                  10    10013  10.so.0.1.0.13
 *    1.2.2                   12    10202  12.so.0.1.2.2
 *    1.2.3rc1-6              12    10203  12.so.0.1.2.3rc1-6
 *    1.2.3                   12    10203  12.so.0.1.2.3
 *    1.2.4beta1-3            13    10204  12.so.0.1.2.4beta1-3
 *    1.0.14rc1               13    10014  10.so.0.1.0.14rc1
 *    1.2.4rc1                13    10204  12.so.0.1.2.4rc1
 *    1.0.14                  10    10014  10.so.0.1.0.14
 *    1.2.4                   13    10204  12.so.0.1.2.4
 *    1.2.5beta1-2            13    10205  12.so.0.1.2.5beta1-2
 *    1.0.15rc1-3             10    10015  10.so.0.1.0.15rc1-3
 *    1.2.5rc1-3              13    10205  12.so.0.1.2.5rc1-3
 *    1.0.15                  10    10015  10.so.0.1.0.15
 *    1.2.5                   13    10205  12.so.0.1.2.5
 *    1.2.6beta1-4            13    10206  12.so.0.1.2.6beta1-4
 *    1.0.16                  10    10016  10.so.0.1.0.16
 *    1.2.6                   13    10206  12.so.0.1.2.6
 *    1.2.7beta1-2            13    10207  12.so.0.1.2.7beta1-2
 *    1.0.17rc1               10    10017  12.so.0.1.0.17rc1
 *    1.2.7rc1                13    10207  12.so.0.1.2.7rc1
 *    1.0.17                  10    10017  12.so.0.1.0.17
 *    1.2.7                   13    10207  12.so.0.1.2.7
 *    1.2.8beta1-5            13    10208  12.so.0.1.2.8beta1-5
 *    1.0.18rc1-5             10    10018  12.so.0.1.0.18rc1-5
 *    1.2.8rc1-5              13    10208  12.so.0.1.2.8rc1-5
 *    1.0.18                  10    10018  12.so.0.1.0.18
 *    1.2.8                   13    10208  12.so.0.1.2.8
 *    1.2.9beta1-3            13    10209  12.so.0.1.2.9beta1-3
 *    1.2.9beta4-11           13    10209  12.so.0.9[.0]
 *    1.2.9rc1                13    10209  12.so.0.9[.0]
 *    1.2.9                   13    10209  12.so.0.9[.0]
 *    1.2.10beta1-7           13    10210  12.so.0.10[.0]
 *    1.2.10rc1-2             13    10210  12.so.0.10[.0]
 *    1.2.10                  13    10210  12.so.0.10[.0]
 *    1.4.0beta1-5            14    10400  14.so.0.0[.0]
 *    1.2.11beta1-4           13    10211  12.so.0.11[.0]
 *    1.4.0beta7-8            14    10400  14.so.0.0[.0]
 *    1.2.11                  13    10211  12.so.0.11[.0]
 *    1.2.12                  13    10212  12.so.0.12[.0]
 *    1.4.0beta9-14           14    10400  14.so.0.0[.0]
 *    1.2.13                  13    10213  12.so.0.13[.0]
 *    1.4.0beta15-36          14    10400  14.so.0.0[.0]
 *    1.4.0beta37-87          14    10400  14.so.14.0[.0]
 *    1.4.0rc01               14    10400  14.so.14.0[.0]
 *    1.4.0beta88-109         14    10400  14.so.14.0[.0]
 *    1.4.0rc02-08            14    10400  14.so.14.0[.0]
 *    1.4.0                   14    10400  14.so.14.0[.0]
 *    1.4.1beta01-03          14    10401  14.so.14.1[.0]
 *    1.4.1rc01               14    10401  14.so.14.1[.0]
 *    1.4.1beta04-12          14    10401  14.so.14.1[.0]
 *    1.4.1                   14    10401  14.so.14.1[.0]
 *    1.5.0beta01-18          15    10500  15.so.15.0[.0]
 *
 *   Henceforth the source version will match the shared-library major
 *   and minor numbers; the shared-library major version number will be
 *   used for changes in backward compatibility, as it is intended.  The
 *   PNG_LIBPNG_VER macro, which is not used within libpng but is available
 *   for applications, is an unsigned integer of the form xyyzz corresponding
 *   to the source version x.y.z (leading zeros in y and z).  Beta versions
 *   were given the previous public release number plus a letter, until
 *   version 1.0.6j; from then on they were given the upcoming public
 *   release number plus "betaNN" or "rcN".
 *
 *   Binary incompatibility exists only when applications make direct access
 *   to the info_ptr or png_ptr members through png.h, and the compiled
 *   application is loaded with a different version of the library.
 *
 *   DLLNUM will change each time there are forward or backward changes
 *   in binary compatibility (e.g., when a new feature is added).
 *
 * See libpng.txt or libpng.3 for more information.  The PNG specification
 * is available as a W3C Recommendation and as an ISO Specification,
 * <http://www.w3.org/TR/2003/REC-PNG-20031110/
 */

/*
 * COPYRIGHT NOTICE, DISCLAIMER, and LICENSE:
 *
 * If you modify libpng you may insert additional notices immediately following
 * this sentence.
 *
 * This code is released under the libpng license.
 *
 * libpng versions 1.2.6, August 15, 2004, through 1.5.0beta18, April 18, 2010, are
 * Copyright (c) 2004, 2006-2010 Glenn Randers-Pehrson, and are
 * distributed according to the same disclaimer and license as libpng-1.2.5
 * with the following individual added to the list of Contributing Authors:
 *
 *    Cosmin Truta
 *
 * libpng versions 1.0.7, July 1, 2000, through 1.2.5, October 3, 2002, are
 * Copyright (c) 2000-2002 Glenn Randers-Pehrson, and are
 * distributed according to the same disclaimer and license as libpng-1.0.6
 * with the following individuals added to the list of Contributing Authors:
 *
 *    Simon-Pierre Cadieux
 *    Eric S. Raymond
 *    Gilles Vollant
 *
 * and with the following additions to the disclaimer:
 *
 *    There is no warranty against interference with your enjoyment of the
 *    library or against infringement.  There is no warranty that our
 *    efforts or the library will fulfill any of your particular purposes
 *    or needs.  This library is provided with all faults, and the entire
 *    risk of satisfactory quality, performance, accuracy, and effort is with
 *    the user.
 *
 * libpng versions 0.97, January 1998, through 1.0.6, March 20, 2000, are
 * Copyright (c) 1998, 1999, 2000 Glenn Randers-Pehrson, and are
 * distributed according to the same disclaimer and license as libpng-0.96,
 * with the following individuals added to the list of Contributing Authors:
 *
 *    Tom Lane
 *    Glenn Randers-Pehrson
 *    Willem van Schaik
 *
 * libpng versions 0.89, June 1996, through 0.96, May 1997, are
 * Copyright (c) 1996, 1997 Andreas Dilger
 * Distributed according to the same disclaimer and license as libpng-0.88,
 * with the following individuals added to the list of Contributing Authors:
 *
 *    John Bowler
 *    Kevin Bracey
 *    Sam Bushell
 *    Magnus Holmgren
 *    Greg Roelofs
 *    Tom Tanner
 *
 * libpng versions 0.5, May 1995, through 0.88, January 1996, are
 * Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.
 *
 * For the purposes of this copyright and license, "Contributing Authors"
 * is defined as the following set of individuals:
 *
 *    Andreas Dilger
 *    Dave Martindale
 *    Guy Eric Schalnat
 *    Paul Schmidt
 *    Tim Wegner
 *
 * The PNG Reference Library is supplied "AS IS".  The Contributing Authors
 * and Group 42, Inc. disclaim all warranties, expressed or implied,
 * including, without limitation, the warranties of merchantability and of
 * fitness for any purpose.  The Contributing Authors and Group 42, Inc.
 * assume no liability for direct, indirect, incidental, special, exemplary,
 * or consequential damages, which may result from the use of the PNG
 * Reference Library, even if advised of the possibility of such damage.
 *
 * Permission is hereby granted to use, copy, modify, and distribute this
 * source code, or portions hereof, for any purpose, without fee, subject
 * to the following restrictions:
 *
 *   1. The origin of this source code must not be misrepresented.
 *
 *   2. Altered versions must be plainly marked as such and
 *   must not be misrepresented as being the original source.
 *
 *   3. This Copyright notice may not be removed or altered from
 *      any source or altered source distribution.
 *
 * The Contributing Authors and Group 42, Inc. specifically permit, without
 * fee, and encourage the use of this source code as a component to
 * supporting the PNG file format in commercial products.  If you use this
 * source code in a product, acknowledgment is not required but would be
 * appreciated.
 */

/*
 * A "png_get_copyright" function is available, for convenient use in "about"
 * boxes and the like:
 *
 *     printf("%s",png_get_copyright(NULL));
 *
 * Also, the PNG logo (in PNG format, of course) is supplied in the
 * files "pngbar.png" and "pngbar.jpg (88x31) and "pngnow.png" (98x31).
 */

/*
 * Libpng is OSI Certified Open Source Software.  OSI Certified is a
 * certification mark of the Open Source Initiative.
 */

/*
 * The contributing authors would like to thank all those who helped
 * with testing, bug fixes, and patience.  This wouldn't have been
 * possible without all of you.
 *
 * Thanks to Frank J. T. Wojcik for helping with the documentation.
 */

/*
 * Y2K compliance in libpng:
 * =========================
 *
 *    April 18, 2010
 *
 *    Since the PNG Development group is an ad-hoc body, we can't make
 *    an official declaration.
 *
 *    This is your unofficial assurance that libpng from version 0.71 and
 *    upward through 1.5.0beta18 are Y2K compliant.  It is my belief that earlier
 *    versions were also Y2K compliant.
 *
 *    Libpng only has three year fields.  One is a 2-byte unsigned integer
 *    that will hold years up to 65535.  The other two hold the date in text
 *    format, and will hold years up to 9999.
 *
 *    The integer is
 *        "png_uint_16 year" in png_time_struct.
 *
 *    The strings are
 *        "png_charp time_buffer" in png_struct and
 *        "near_time_buffer", which is a local character string in png.c.
 *
 *    There are seven time-related functions:
 *        png.c: png_convert_to_rfc_1123() in png.c
 *          (formerly png_convert_to_rfc_1152() in error)
 *        png_convert_from_struct_tm() in pngwrite.c, called in pngwrite.c
 *        png_convert_from_time_t() in pngwrite.c
 *        png_get_tIME() in pngget.c
 *        png_handle_tIME() in pngrutil.c, called in pngread.c
 *        png_set_tIME() in pngset.c
 *        png_write_tIME() in pngwutil.c, called in pngwrite.c
 *
 *    All handle dates properly in a Y2K environment.  The
 *    png_convert_from_time_t() function calls gmtime() to convert from system
 *    clock time, which returns (year - 1900), which we properly convert to
 *    the full 4-digit year.  There is a possibility that applications using
 *    libpng are not passing 4-digit years into the png_convert_to_rfc_1123()
 *    function, or that they are incorrectly passing only a 2-digit year
 *    instead of "year - 1900" into the png_convert_from_struct_tm() function,
 *    but this is not under our control.  The libpng documentation has always
 *    stated that it works with 4-digit years, and the APIs have been
 *    documented as such.
 *
 *    The tIME chunk itself is also Y2K compliant.  It uses a 2-byte unsigned
 *    integer to hold the year, and can hold years as large as 65535.
 *
 *    zlib, upon which libpng depends, is also Y2K compliant.  It contains
 *    no date-related code.
 *
 *       Glenn Randers-Pehrson
 *       libpng maintainer
 *       PNG Development Group
 */

#ifndef PNG_H
#define PNG_H

/* This is not the place to learn how to use libpng.  The file libpng.txt
 * describes how to use libpng, and the file example.c summarizes it
 * with some code on which to build.  This file is useful for looking
 * at the actual function definitions and structure components.
 */

/* Version information for png.h - this should match the version in png.c */
#define PNG_LIBPNG_VER_STRING "1.5.0beta18"
#define PNG_HEADER_VERSION_STRING \
     " libpng version 1.5.0beta18 - April 18, 2010\n"

#define PNG_LIBPNG_VER_SONUM   15
#define PNG_LIBPNG_VER_DLLNUM  15

/* These should match the first 3 components of PNG_LIBPNG_VER_STRING: */
#define PNG_LIBPNG_VER_MAJOR   1
#define PNG_LIBPNG_VER_MINOR   5
#define PNG_LIBPNG_VER_RELEASE 0
/* This should match the numeric part of the final component of
 * PNG_LIBPNG_VER_STRING, omitting any leading zero:
 */

#define PNG_LIBPNG_VER_BUILD  18

/* Release Status */
#define PNG_LIBPNG_BUILD_ALPHA    1
#define PNG_LIBPNG_BUILD_BETA     2
#define PNG_LIBPNG_BUILD_RC       3
#define PNG_LIBPNG_BUILD_STABLE   4
#define PNG_LIBPNG_BUILD_RELEASE_STATUS_MASK 7

/* Release-Specific Flags */
#define PNG_LIBPNG_BUILD_PATCH    8 /* Can be OR'ed with
                                       PNG_LIBPNG_BUILD_STABLE only */
#define PNG_LIBPNG_BUILD_PRIVATE 16 /* Cannot be OR'ed with
                                       PNG_LIBPNG_BUILD_SPECIAL */
#define PNG_LIBPNG_BUILD_SPECIAL 32 /* Cannot be OR'ed with
                                       PNG_LIBPNG_BUILD_PRIVATE */

#define PNG_LIBPNG_BUILD_BASE_TYPE PNG_LIBPNG_BUILD_BETA

/* Careful here.  At one time, Guy wanted to use 082, but that would be octal.
 * We must not include leading zeros.
 * Versions 0.7 through 1.0.0 were in the range 0 to 100 here (only
 * version 1.0.0 was mis-numbered 100 instead of 10000).  From
 * version 1.0.1 it's    xxyyzz, where x=major, y=minor, z=release
 */
#define PNG_LIBPNG_VER 10500 /* 1.5.0 */

/* Library configuration: these options cannot be changed after
 * the library has been built.
 */
#ifndef PNGLCONF_H
#   include "pnglibconf.h"
#endif

#ifndef PNG_VERSION_INFO_ONLY
/* Standard header files (not needed for the version info) */
#  ifdef PNG_STDIO_SUPPORTED
#    include <stdio.h>
#  endif
#  ifdef PNG_SETJMP_SUPPORTED
#    include <setjmp.h>
#  endif

/* Include the compression library's header */
#  include "zlib.h"

/* Machine specific configuration. */
#  include "pngconf.h"
#endif

/*
 * Added at libpng-1.2.8
 *
 * Ref MSDN: Private as priority over Special
 * VS_FF_PRIVATEBUILD File *was not* built using standard release
 * procedures. If this value is given, the StringFileInfo block must
 * contain a PrivateBuild string.
 *
 * VS_FF_SPECIALBUILD File *was* built by the original company using
 * standard release procedures but is a variation of the standard
 * file of the same version number. If this value is given, the
 * StringFileInfo block must contain a SpecialBuild string.
 */

#ifdef PNG_USER_PRIVATEBUILD /* From pnglibconf.h */
#  define PNG_LIBPNG_BUILD_TYPE \
       (PNG_LIBPNG_BUILD_BASE_TYPE | PNG_LIBPNG_BUILD_PRIVATE)
#else
#  ifdef PNG_LIBPNG_SPECIALBUILD
#    define PNG_LIBPNG_BUILD_TYPE \
         (PNG_LIBPNG_BUILD_BASE_TYPE | PNG_LIBPNG_BUILD_SPECIAL)
#  else
#    define PNG_LIBPNG_BUILD_TYPE (PNG_LIBPNG_BUILD_BASE_TYPE)
#  endif
#endif

#ifndef PNG_VERSION_INFO_ONLY

/* Inhibit C++ name-mangling for libpng functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Version information for C files, stored in png.c.  This had better match
 * the version above.
 */
#define png_libpng_ver png_get_header_ver(NULL)

/* This file is arranged in several sections:
 *
 * 1. Any configuration options that can be specified by for the application
 *    code when it is built.  (Build time configuration is in pnglibconf.h)
 * 2. Type definitions (base types are defined in pngconf.h), structure
 *    definitions.
 * 3. Exported library functions.
 *
 * The library source code has additional files (principally pngpriv.h) that
 * allow configuration of the library.
 */
/* Section 1: run time configuration
 * See pnglibconf.h for build time configuration
 *
 * Run time configuration allows the application to choose between
 * implementations of certain arithmetic APIs.  The default is set
 * at build time and recorded in pnglibconf.h, but it is safe to
 * override these (and only these) settings.  Note that this won't
 * change what the library does, only application code, and the
 * settings can (and probably should) be made on a per-file basis
 * by setting the #defines before including png.h
 *
 * Use macros to read integers from PNG data or use the exported
 * functions?
 *   PNG_USE_READ_MACROS: use the macros (see below)  Note that
 *     the macros evaluate their argument multiple times.
 *   PNG_NO_USE_READ_MACROS: call the relevant library function.
 *
 * Use the alternative algorithm for compositing alpha samples that
 * does not use division?
 *   PNG_READ_COMPOSITE_NODIV_SUPPORTED: use the 'no division'
 *      algorithm.
 *   PNG_NO_READ_COMPOSITE_NODIV: use the 'division' algorithm.
 *
 * How to handle benign errors if PNG_ALLOW_BENIGN_ERRORS is
 * false?
 *   PNG_ALLOW_BENIGN_ERRORS: map calls to the benign error
 *      APIs to png_warning.
 * Otherwise the calls are mapped to png_error.
 */

/* Section 2: type definitions, including structures and compile time
 * constants.
 * See pngconf.h for base types that vary by machine/system
 */
/* Three color definitions.  The order of the red, green, and blue, (and the
 * exact size) is not important, although the size of the fields need to
 * be png_byte or png_uint_16 (as defined below).
 */
typedef struct png_color_struct
{
   png_byte red;
   png_byte green;
   png_byte blue;
} png_color;
typedef png_color FAR * png_colorp;
typedef png_color FAR * FAR * png_colorpp;

typedef struct png_color_16_struct
{
   png_byte index;    /* used for palette files */
   png_uint_16 red;   /* for use in red green blue files */
   png_uint_16 green;
   png_uint_16 blue;
   png_uint_16 gray;  /* for use in grayscale files */
} png_color_16;
typedef png_color_16 FAR * png_color_16p;
typedef png_color_16 FAR * FAR * png_color_16pp;

typedef struct png_color_8_struct
{
   png_byte red;   /* for use in red green blue files */
   png_byte green;
   png_byte blue;
   png_byte gray;  /* for use in grayscale files */
   png_byte alpha; /* for alpha channel files */
} png_color_8;
typedef png_color_8 FAR * png_color_8p;
typedef png_color_8 FAR * FAR * png_color_8pp;

/*
 * The following two structures are used for the in-core representation
 * of sPLT chunks.
 */
typedef struct png_sPLT_entry_struct
{
   png_uint_16 red;
   png_uint_16 green;
   png_uint_16 blue;
   png_uint_16 alpha;
   png_uint_16 frequency;
} png_sPLT_entry;
typedef png_sPLT_entry FAR * png_sPLT_entryp;
typedef png_sPLT_entry FAR * FAR * png_sPLT_entrypp;

/*  When the depth of the sPLT palette is 8 bits, the color and alpha samples
 *  occupy the LSB of their respective members, and the MSB of each member
 *  is zero-filled.  The frequency member always occupies the full 16 bits.
 */

typedef struct png_sPLT_struct
{
   png_charp name;           /* palette name */
   png_byte depth;           /* depth of palette samples */
   png_sPLT_entryp entries;  /* palette entries */
   png_int_32 nentries;      /* number of palette entries */
} png_sPLT_t;
typedef png_sPLT_t FAR * png_sPLT_tp;
typedef png_sPLT_t FAR * FAR * png_sPLT_tpp;

#ifdef PNG_TEXT_SUPPORTED
/* png_text holds the contents of a text/ztxt/itxt chunk in a PNG file,
 * and whether that contents is compressed or not.  The "key" field
 * points to a regular zero-terminated C string.  The "text", "lang", and
 * "lang_key" fields can be regular C strings, empty strings, or NULL pointers.
 * However, the * structure returned by png_get_text() will always contain
 * regular zero-terminated C strings (possibly empty), never NULL pointers,
 * so they can be safely used in printf() and other string-handling functions.
 */
typedef struct png_text_struct
{
   int  compression;       /* compression value:
                             -1: tEXt, none
                              0: zTXt, deflate
                              1: iTXt, none
                              2: iTXt, deflate  */
   png_charp key;          /* keyword, 1-79 character description of "text" */
   png_charp text;         /* comment, may be an empty string (ie "")
                              or a NULL pointer */
   png_size_t text_length; /* length of the text string */
   png_size_t itxt_length; /* length of the itxt string */
   png_charp lang;         /* language code, 0-79 characters
                              or a NULL pointer */
   png_charp lang_key;     /* keyword translated UTF-8 string, 0 or more
                              chars or a NULL pointer */
} png_text;
typedef png_text FAR * png_textp;
typedef png_text FAR * FAR * png_textpp;
#endif

/* Supported compression types for text in PNG files (tEXt, and zTXt).
 * The values of the PNG_TEXT_COMPRESSION_ defines should NOT be changed. */
#define PNG_TEXT_COMPRESSION_NONE_WR -3
#define PNG_TEXT_COMPRESSION_zTXt_WR -2
#define PNG_TEXT_COMPRESSION_NONE    -1
#define PNG_TEXT_COMPRESSION_zTXt     0
#define PNG_ITXT_COMPRESSION_NONE     1
#define PNG_ITXT_COMPRESSION_zTXt     2
#define PNG_TEXT_COMPRESSION_LAST     3  /* Not a valid value */

/* png_time is a way to hold the time in an machine independent way.
 * Two conversions are provided, both from time_t and struct tm.  There
 * is no portable way to convert to either of these structures, as far
 * as I know.  If you know of a portable way, send it to me.  As a side
 * note - PNG has always been Year 2000 compliant!
 */
typedef struct png_time_struct
{
   png_uint_16 year; /* full year, as in, 1995 */
   png_byte month;   /* month of year, 1 - 12 */
   png_byte day;     /* day of month, 1 - 31 */
   png_byte hour;    /* hour of day, 0 - 23 */
   png_byte minute;  /* minute of hour, 0 - 59 */
   png_byte second;  /* second of minute, 0 - 60 (for leap seconds) */
} png_time;
typedef png_time FAR * png_timep;
typedef png_time FAR * FAR * png_timepp;

#if defined(PNG_UNKNOWN_CHUNKS_SUPPORTED) || \
    defined(PNG_HANDLE_AS_UNKNOWN_SUPPORTED)
/* png_unknown_chunk is a structure to hold queued chunks for which there is
 * no specific support.  The idea is that we can use this to queue
 * up private chunks for output even though the library doesn't actually
 * know about their semantics.
 */
typedef struct png_unknown_chunk_t
{
    png_byte name[5];
    png_byte *data;
    png_size_t size;

    /* libpng-using applications should NOT directly modify this byte. */
    png_byte location; /* mode of operation at read time */
}
png_unknown_chunk;
typedef png_unknown_chunk FAR * png_unknown_chunkp;
typedef png_unknown_chunk FAR * FAR * png_unknown_chunkpp;
#endif

typedef struct png_info_def png_info;
typedef png_info FAR * png_infop;
typedef png_info FAR * FAR * png_infopp;

/* Maximum positive integer used in PNG is (2^31)-1 */
#define PNG_UINT_31_MAX ((png_uint_32)0x7fffffffL)
#define PNG_UINT_32_MAX ((png_uint_32)(-1))
#define PNG_SIZE_MAX ((png_size_t)(-1))

/* These describe the color_type field in png_info. */
/* color type masks */
#define PNG_COLOR_MASK_PALETTE    1
#define PNG_COLOR_MASK_COLOR      2
#define PNG_COLOR_MASK_ALPHA      4

/* color types.  Note that not all combinations are legal */
#define PNG_COLOR_TYPE_GRAY 0
#define PNG_COLOR_TYPE_PALETTE  (PNG_COLOR_MASK_COLOR | PNG_COLOR_MASK_PALETTE)
#define PNG_COLOR_TYPE_RGB        (PNG_COLOR_MASK_COLOR)
#define PNG_COLOR_TYPE_RGB_ALPHA  (PNG_COLOR_MASK_COLOR | PNG_COLOR_MASK_ALPHA)
#define PNG_COLOR_TYPE_GRAY_ALPHA (PNG_COLOR_MASK_ALPHA)
/* aliases */
#define PNG_COLOR_TYPE_RGBA  PNG_COLOR_TYPE_RGB_ALPHA
#define PNG_COLOR_TYPE_GA  PNG_COLOR_TYPE_GRAY_ALPHA

/* This is for compression type. PNG 1.0-1.2 only define the single type. */
#define PNG_COMPRESSION_TYPE_BASE 0 /* Deflate method 8, 32K window */
#define PNG_COMPRESSION_TYPE_DEFAULT PNG_COMPRESSION_TYPE_BASE

/* This is for filter type. PNG 1.0-1.2 only define the single type. */
#define PNG_FILTER_TYPE_BASE      0 /* Single row per-byte filtering */
#define PNG_INTRAPIXEL_DIFFERENCING 64 /* Used only in MNG datastreams */
#define PNG_FILTER_TYPE_DEFAULT   PNG_FILTER_TYPE_BASE

/* These are for the interlacing type.  These values should NOT be changed. */
#define PNG_INTERLACE_NONE        0 /* Non-interlaced image */
#define PNG_INTERLACE_ADAM7       1 /* Adam7 interlacing */
#define PNG_INTERLACE_LAST        2 /* Not a valid value */

/* These are for the oFFs chunk.  These values should NOT be changed. */
#define PNG_OFFSET_PIXEL          0 /* Offset in pixels */
#define PNG_OFFSET_MICROMETER     1 /* Offset in micrometers (1/10^6 meter) */
#define PNG_OFFSET_LAST           2 /* Not a valid value */

/* These are for the pCAL chunk.  These values should NOT be changed. */
#define PNG_EQUATION_LINEAR       0 /* Linear transformation */
#define PNG_EQUATION_BASE_E       1 /* Exponential base e transform */
#define PNG_EQUATION_ARBITRARY    2 /* Arbitrary base exponential transform */
#define PNG_EQUATION_HYPERBOLIC   3 /* Hyperbolic sine transformation */
#define PNG_EQUATION_LAST         4 /* Not a valid value */

/* These are for the sCAL chunk.  These values should NOT be changed. */
#define PNG_SCALE_UNKNOWN         0 /* unknown unit (image scale) */
#define PNG_SCALE_METER           1 /* meters per pixel */
#define PNG_SCALE_RADIAN          2 /* radians per pixel */
#define PNG_SCALE_LAST            3 /* Not a valid value */

/* These are for the pHYs chunk.  These values should NOT be changed. */
#define PNG_RESOLUTION_UNKNOWN    0 /* pixels/unknown unit (aspect ratio) */
#define PNG_RESOLUTION_METER      1 /* pixels/meter */
#define PNG_RESOLUTION_LAST       2 /* Not a valid value */

/* These are for the sRGB chunk.  These values should NOT be changed. */
#define PNG_sRGB_INTENT_PERCEPTUAL 0
#define PNG_sRGB_INTENT_RELATIVE   1
#define PNG_sRGB_INTENT_SATURATION 2
#define PNG_sRGB_INTENT_ABSOLUTE   3
#define PNG_sRGB_INTENT_LAST       4 /* Not a valid value */

/* This is for text chunks */
#define PNG_KEYWORD_MAX_LENGTH     79

/* Maximum number of entries in PLTE/sPLT/tRNS arrays */
#define PNG_MAX_PALETTE_LENGTH    256

/* These determine if an ancillary chunk's data has been successfully read
 * from the PNG header, or if the application has filled in the corresponding
 * data in the info_struct to be written into the output file.  The values
 * of the PNG_INFO_<chunk> defines should NOT be changed.
 */
#define PNG_INFO_gAMA 0x0001
#define PNG_INFO_sBIT 0x0002
#define PNG_INFO_cHRM 0x0004
#define PNG_INFO_PLTE 0x0008
#define PNG_INFO_tRNS 0x0010
#define PNG_INFO_bKGD 0x0020
#define PNG_INFO_hIST 0x0040
#define PNG_INFO_pHYs 0x0080
#define PNG_INFO_oFFs 0x0100
#define PNG_INFO_tIME 0x0200
#define PNG_INFO_pCAL 0x0400
#define PNG_INFO_sRGB 0x0800   /* GR-P, 0.96a */
#define PNG_INFO_iCCP 0x1000   /* ESR, 1.0.6 */
#define PNG_INFO_sPLT 0x2000   /* ESR, 1.0.6 */
#define PNG_INFO_sCAL 0x4000   /* ESR, 1.0.6 */
#define PNG_INFO_IDAT 0x8000L  /* ESR, 1.0.6 */

/* This is used for the transformation routines, as some of them
 * change these values for the row.  It also should enable using
 * the routines for other purposes.
 */
typedef struct png_row_info_struct
{
   png_uint_32 width; /* width of row */
   png_size_t rowbytes; /* number of bytes in row */
   png_byte color_type; /* color type of row */
   png_byte bit_depth; /* bit depth of row */
   png_byte channels; /* number of channels (1, 2, 3, or 4) */
   png_byte pixel_depth; /* bits per pixel (depth * channels) */
} png_row_info;

typedef png_row_info FAR * png_row_infop;
typedef png_row_info FAR * FAR * png_row_infopp;

/* These are the function types for the I/O functions and for the functions
 * that allow the user to override the default I/O functions with his or her
 * own.  The png_error_ptr type should match that of user-supplied warning
 * and error functions, while the png_rw_ptr type should match that of the
 * user read/write data functions.
 */
typedef struct png_struct_def png_struct;
typedef png_struct FAR * png_structp;

typedef PNG_CALLBACK(void, *png_error_ptr, (png_structp, png_const_charp),);
typedef PNG_CALLBACK(void, *png_rw_ptr, (png_structp, png_bytep, png_size_t),);
typedef PNG_CALLBACK(void, *png_flush_ptr, (png_structp),);
typedef PNG_CALLBACK(void, *png_read_status_ptr, (png_structp, png_uint_32,
    int),);
typedef PNG_CALLBACK(void, *png_write_status_ptr, (png_structp, png_uint_32,
    int),);

#ifdef PNG_PROGRESSIVE_READ_SUPPORTED
typedef PNG_CALLBACK(void, *png_progressive_info_ptr,
    (png_structp, png_infop),);
typedef PNG_CALLBACK(void, *png_progressive_end_ptr, (png_structp, png_infop),);
typedef PNG_CALLBACK(void, *png_progressive_row_ptr, (png_structp, png_bytep,
    png_uint_32, int),);
#endif

#if defined(PNG_READ_USER_TRANSFORM_SUPPORTED) || \
    defined(PNG_WRITE_USER_TRANSFORM_SUPPORTED)
typedef PNG_CALLBACK(void, *png_user_transform_ptr, (png_structp, png_row_infop,
    png_bytep),);
#endif

#ifdef PNG_USER_CHUNKS_SUPPORTED
typedef PNG_CALLBACK(int, *png_user_chunk_ptr, (png_structp,
    png_unknown_chunkp),);
#endif
#ifdef PNG_UNKNOWN_CHUNKS_SUPPORTED
typedef PNG_CALLBACK(void, *png_unknown_chunk_ptr, (png_structp),);
#endif
#ifdef PNG_SETJMP_SUPPORTED
/* This must match the function definition in <setjmp.h>, and the
 * application must include this before png.h to obtain the definition
 * of jmp_buf.  The function is required to be PNG_NORETURN.
 */
typedef void (PNGCAPI *png_longjmp_ptr) PNGARG((jmp_buf, int)) PNG_NORETURN;
#endif

/* Transform masks for the high-level interface */
#define PNG_TRANSFORM_IDENTITY       0x0000    /* read and write */
#define PNG_TRANSFORM_STRIP_16       0x0001    /* read only */
#define PNG_TRANSFORM_STRIP_ALPHA    0x0002    /* read only */
#define PNG_TRANSFORM_PACKING        0x0004    /* read and write */
#define PNG_TRANSFORM_PACKSWAP       0x0008    /* read and write */
#define PNG_TRANSFORM_EXPAND         0x0010    /* read only */
#define PNG_TRANSFORM_INVERT_MONO    0x0020    /* read and write */
#define PNG_TRANSFORM_SHIFT          0x0040    /* read and write */
#define PNG_TRANSFORM_BGR            0x0080    /* read and write */
#define PNG_TRANSFORM_SWAP_ALPHA     0x0100    /* read and write */
#define PNG_TRANSFORM_SWAP_ENDIAN    0x0200    /* read and write */
#define PNG_TRANSFORM_INVERT_ALPHA   0x0400    /* read and write */
#define PNG_TRANSFORM_STRIP_FILLER   0x0800    /* write only */
/* Added to libpng-1.2.34 */
#define PNG_TRANSFORM_STRIP_FILLER_BEFORE PNG_TRANSFORM_STRIP_FILLER
#define PNG_TRANSFORM_STRIP_FILLER_AFTER 0x1000 /* write only */
/* Added to libpng-1.4.0 */
#define PNG_TRANSFORM_GRAY_TO_RGB   0x2000      /* read only */

/* Flags for MNG supported features */
#define PNG_FLAG_MNG_EMPTY_PLTE     0x01
#define PNG_FLAG_MNG_FILTER_64      0x04
#define PNG_ALL_MNG_FEATURES        0x05

/* NOTE: prior to 1.5 these functions had no 'API' style declaration,
 * this allowed the zlib default functions to be used on Windows
 * platforms.  In 1.5 the zlib default malloc (which just calls malloc and
 * ignores the first argument) should be completely compatible with the
 * following.
 */
typedef PNG_CALLBACK(png_voidp, *png_malloc_ptr, (png_structp,
    png_alloc_size_t),);
typedef PNG_CALLBACK(void, *png_free_ptr, (png_structp, png_voidp),);

/* This triggers a compiler error in png.c, if png.c and png.h
 * do not agree upon the version number.
 */
typedef png_structp version_1_5_0beta18;

typedef png_struct FAR * FAR * png_structpp;

/* Section 3: exported functions
 * Here are the function definitions most commonly used.  This is not
 * the place to find out how to use libpng.  See libpng.txt for the
 * full explanation, see example.c for the summary.  This just provides
 * a simple one line description of the use of each function.
 */

/* Returns the version number of the library */
extern PNG_EXPORT(png_uint_32,png_access_version_number,(void),,1);

/* Tell lib we have already handled the first <num_bytes> magic bytes.
 * Handling more than 8 bytes from the beginning of the file is an error.
 */
extern PNG_EXPORT(void,png_set_sig_bytes,(png_structp png_ptr,
    int num_bytes),,2);

/* Check sig[start] through sig[start + num_to_check - 1] to see if it's a
 * PNG file.  Returns zero if the supplied bytes match the 8-byte PNG
 * signature, and non-zero otherwise.  Having num_to_check == 0 or
 * start > 7 will always fail (ie return non-zero).
 */
extern PNG_EXPORT(int,png_sig_cmp,(png_bytep sig, png_size_t start,
    png_size_t num_to_check),,3);

/* Simple signature checking function.  This is the same as calling
 * png_check_sig(sig, n) := !png_sig_cmp(sig, 0, n).
 */
#define png_check_sig(sig,n) !png_sig_cmp((sig), 0, (n))

/* Allocate and initialize png_ptr struct for reading, and any other memory. */
extern PNG_EXPORT(png_structp,png_create_read_struct,
    (png_const_charp user_png_ver, png_voidp error_ptr,
    png_error_ptr error_fn, png_error_ptr warn_fn),PNG_ALLOCATED,4);

/* Allocate and initialize png_ptr struct for writing, and any other memory */
extern PNG_EXPORT(png_structp,png_create_write_struct,
    (png_const_charp user_png_ver, png_voidp error_ptr,
    png_error_ptr error_fn, png_error_ptr warn_fn),PNG_ALLOCATED,5);

extern PNG_EXPORT(png_size_t,png_get_compression_buffer_size,(png_structp
    png_ptr),,6);

extern PNG_EXPORT(void,png_set_compression_buffer_size,(png_structp png_ptr,
    png_size_t size),,7);

/* Moved from pngconf.h in 1.4.0 and modified to ensure setjmp/longjmp
 * match up.
 */
#ifdef PNG_SETJMP_SUPPORTED
/* This function returns the jmp_buf built in to *png_ptr.  It must be
 * supplied with an appropriate 'longjmp' function to use on that jmp_buf
 * unless the default error function is overridden in which case NULL is
 * acceptable.  The size of the jmp_buf is checked against the actual size
 * allocated by the library - the call will return NULL on a mismatch
 * indicating an ABI mismatch.
 */
extern PNG_EXPORT(jmp_buf*, png_set_longjmp_fn, (png_structp png_ptr,
    png_longjmp_ptr longjmp_fn, size_t jmp_buf_size),,8);
#  define png_jmpbuf(png_ptr) \
      (*png_set_longjmp_fn((png_ptr), longjmp, sizeof (jmp_buf)))
#else
#  define png_jmpbuf(png_ptr) \
      (LIBPNG_WAS_COMPILED_WITH__PNG_NO_SETJMP)
#endif
/* This function should be used by libpng applications in place of
 * longjmp(png_ptr->jmpbuf, val).  If longjmp_fn() has been set, it
 * will use it; otherwise it will call PNG_ABORT().  This function was
 * added in libpng-1.5.0.
 */
extern PNG_EXPORT(void, png_longjmp, (png_structp png_ptr, int val),
    PNG_NORETURN,9);

/* Reset the compression stream */
extern PNG_EXPORT(int,png_reset_zstream,(png_structp png_ptr),,10);

/* New functions added in libpng-1.0.2 (not enabled by default until 1.2.0) */
#ifdef PNG_USER_MEM_SUPPORTED
extern PNG_EXPORT(png_structp,png_create_read_struct_2,
    (png_const_charp user_png_ver, png_voidp error_ptr,
    png_error_ptr error_fn, png_error_ptr warn_fn, png_voidp mem_ptr,
    png_malloc_ptr malloc_fn, png_free_ptr free_fn),PNG_ALLOCATED,11);
extern PNG_EXPORT(png_structp,png_create_write_struct_2,
    (png_const_charp user_png_ver, png_voidp error_ptr,
    png_error_ptr error_fn, png_error_ptr warn_fn, png_voidp mem_ptr,
    png_malloc_ptr malloc_fn, png_free_ptr free_fn),PNG_ALLOCATED,12);
#endif

/* Write the PNG file signature. */
extern PNG_EXPORT(void,png_write_sig,(png_structp png_ptr),,13);

/* Write a PNG chunk - size, type, (optional) data, CRC. */
extern PNG_EXPORT(void,png_write_chunk,(png_structp png_ptr,
    png_bytep chunk_name, png_bytep data, png_size_t length),,14);

/* Write the start of a PNG chunk - length and chunk name. */
extern PNG_EXPORT(void,png_write_chunk_start,(png_structp png_ptr,
    png_bytep chunk_name, png_uint_32 length),,15);

/* Write the data of a PNG chunk started with png_write_chunk_start(). */
extern PNG_EXPORT(void,png_write_chunk_data,(png_structp png_ptr,
    png_bytep data, png_size_t length),,16);

/* Finish a chunk started with png_write_chunk_start() (includes CRC). */
extern PNG_EXPORT(void,png_write_chunk_end,(png_structp png_ptr),,17);

/* Allocate and initialize the info structure */
extern PNG_EXPORT(png_infop,png_create_info_struct,(png_structp png_ptr),
    PNG_ALLOCATED,18);

extern PNG_EXPORT(void,png_info_init_3,(png_infopp info_ptr,
    png_size_t png_info_struct_size),,19);

/* Writes all the PNG information before the image. */
extern PNG_EXPORT(void,png_write_info_before_PLTE,(png_structp png_ptr,
    png_infop info_ptr),,20);
extern PNG_EXPORT(void,png_write_info,(png_structp png_ptr,
    png_infop info_ptr),,21);

#ifdef PNG_SEQUENTIAL_READ_SUPPORTED
/* Read the information before the actual image data. */
extern PNG_EXPORT(void,png_read_info,(png_structp png_ptr,
    png_infop info_ptr),,22);
#endif

#ifdef PNG_TIME_RFC1123_SUPPORTED
extern PNG_EXPORT(png_charp,png_convert_to_rfc1123,(png_structp png_ptr,
    png_timep ptime),,23);
#endif

#ifdef PNG_CONVERT_tIME_SUPPORTED
/* Convert from a struct tm to png_time */
extern PNG_EXPORT(void,png_convert_from_struct_tm,(png_timep ptime,
    struct tm FAR * ttime),,24);

/* Convert from time_t to png_time.  Uses gmtime() */
extern PNG_EXPORT(void,png_convert_from_time_t,(png_timep ptime,
    time_t ttime),,25);
#endif /* PNG_CONVERT_tIME_SUPPORTED */

#ifdef PNG_READ_EXPAND_SUPPORTED
/* Expand data to 24-bit RGB, or 8-bit grayscale, with alpha if available. */
extern PNG_EXPORT(void,png_set_expand,(png_structp png_ptr),,26);
extern PNG_EXPORT(void,png_set_expand_gray_1_2_4_to_8,(png_structp png_ptr),,
    27);
extern PNG_EXPORT(void,png_set_palette_to_rgb,(png_structp png_ptr),,28);
extern PNG_EXPORT(void,png_set_tRNS_to_alpha,(png_structp png_ptr),,29);
#endif

#if defined(PNG_READ_BGR_SUPPORTED) || defined(PNG_WRITE_BGR_SUPPORTED)
/* Use blue, green, red order for pixels. */
extern PNG_EXPORT(void,png_set_bgr,(png_structp png_ptr),,30);
#endif

#ifdef PNG_READ_GRAY_TO_RGB_SUPPORTED
/* Expand the grayscale to 24-bit RGB if necessary. */
extern PNG_EXPORT(void,png_set_gray_to_rgb,(png_structp png_ptr),,31);
#endif

#ifdef PNG_READ_RGB_TO_GRAY_SUPPORTED
/* Reduce RGB to grayscale. */
#  ifdef PNG_FLOATING_POINT_SUPPORTED
extern PNG_EXPORT(void,png_set_rgb_to_gray,(png_structp png_ptr,
    int error_action, double red, double green ),,32);
#  endif
extern PNG_EXPORT(void,png_set_rgb_to_gray_fixed,(png_structp png_ptr,
    int error_action, png_fixed_point red, png_fixed_point green),,33);
extern PNG_EXPORT(png_byte,png_get_rgb_to_gray_status,(png_structp png_ptr),,
    34);
#endif

extern PNG_EXPORT(void,png_build_grayscale_palette,(int bit_depth,
    png_colorp palette),,35);

#ifdef PNG_READ_STRIP_ALPHA_SUPPORTED
extern PNG_EXPORT(void,png_set_strip_alpha,(png_structp png_ptr),,36);
#endif

#if defined(PNG_READ_SWAP_ALPHA_SUPPORTED) || \
    defined(PNG_WRITE_SWAP_ALPHA_SUPPORTED)
extern PNG_EXPORT(void,png_set_swap_alpha,(png_structp png_ptr),,37);
#endif

#if defined(PNG_READ_INVERT_ALPHA_SUPPORTED) || \
    defined(PNG_WRITE_INVERT_ALPHA_SUPPORTED)
extern PNG_EXPORT(void,png_set_invert_alpha,(png_structp png_ptr),,38);
#endif

#if defined(PNG_READ_FILLER_SUPPORTED) || defined(PNG_WRITE_FILLER_SUPPORTED)
/* Add a filler byte to 8-bit Gray or 24-bit RGB images. */
extern PNG_EXPORT(void,png_set_filler,(png_structp png_ptr,
    png_uint_32 filler, int flags),,39);
/* The values of the PNG_FILLER_ defines should NOT be changed */
#  define PNG_FILLER_BEFORE 0
#  define PNG_FILLER_AFTER 1
/* Add an alpha byte to 8-bit Gray or 24-bit RGB images. */
extern PNG_EXPORT(void,png_set_add_alpha,(png_structp png_ptr,
    png_uint_32 filler, int flags),,40);
#endif /* PNG_READ_FILLER_SUPPORTED || PNG_WRITE_FILLER_SUPPORTED */

#if defined(PNG_READ_SWAP_SUPPORTED) || defined(PNG_WRITE_SWAP_SUPPORTED)
/* Swap bytes in 16-bit depth files. */
extern PNG_EXPORT(void,png_set_swap,(png_structp png_ptr),,41);
#endif

#if defined(PNG_READ_PACK_SUPPORTED) || defined(PNG_WRITE_PACK_SUPPORTED)
/* Use 1 byte per pixel in 1, 2, or 4-bit depth files. */
extern PNG_EXPORT(void,png_set_packing,(png_structp png_ptr),,42);
#endif

#if defined(PNG_READ_PACKSWAP_SUPPORTED) || \
    defined(PNG_WRITE_PACKSWAP_SUPPORTED)
/* Swap packing order of pixels in bytes. */
extern PNG_EXPORT(void,png_set_packswap,(png_structp png_ptr),,43);
#endif

#if defined(PNG_READ_SHIFT_SUPPORTED) || defined(PNG_WRITE_SHIFT_SUPPORTED)
/* Converts files to legal bit depths. */
extern PNG_EXPORT(void,png_set_shift,(png_structp png_ptr,
    png_color_8p true_bits),,44);
#endif

#if defined(PNG_READ_INTERLACING_SUPPORTED) || \
    defined(PNG_WRITE_INTERLACING_SUPPORTED)
/* Have the code handle the interlacing.  Returns the number of passes. */
extern PNG_EXPORT(int,png_set_interlace_handling,(png_structp png_ptr),,45);
#endif

#if defined(PNG_READ_INVERT_SUPPORTED) || defined(PNG_WRITE_INVERT_SUPPORTED)
/* Invert monochrome files */
extern PNG_EXPORT(void,png_set_invert_mono,(png_structp png_ptr),,46);
#endif

#ifdef PNG_READ_BACKGROUND_SUPPORTED
/* Handle alpha and tRNS by replacing with a background color. */
#  ifdef PNG_FLOATING_POINT_SUPPORTED
extern PNG_EXPORT(void,png_set_background,(png_structp png_ptr,
    png_color_16p background_color, int background_gamma_code,
    int need_expand, double background_gamma),,47);
#  endif
#  define PNG_BACKGROUND_GAMMA_UNKNOWN 0
#  define PNG_BACKGROUND_GAMMA_SCREEN  1
#  define PNG_BACKGROUND_GAMMA_FILE    2
#  define PNG_BACKGROUND_GAMMA_UNIQUE  3
#endif

#ifdef PNG_READ_16_TO_8_SUPPORTED
/* Strip the second byte of information from a 16-bit depth file. */
extern PNG_EXPORT(void,png_set_strip_16,(png_structp png_ptr),,48);
#endif

#ifdef PNG_READ_QUANTIZE_SUPPORTED
/* Turn on quantizing, and reduce the palette to the number of colors
 * available.
 */
extern PNG_EXPORT(void,png_set_quantize,(png_structp png_ptr,
    png_colorp palette, int num_palette, int maximum_colors,
    png_uint_16p histogram, int full_quantize),,49);
#endif

#ifdef PNG_READ_GAMMA_SUPPORTED
/* Handle gamma correction. Screen_gamma=(display_exponent) */
#  ifdef PNG_FLOATING_POINT_SUPPORTED
extern PNG_EXPORT(void,png_set_gamma,(png_structp png_ptr, 
    double screen_gamma, double default_file_gamma),,50);
#  endif
#endif


#ifdef PNG_WRITE_FLUSH_SUPPORTED
/* Set how many lines between output flushes - 0 for no flushing */
extern PNG_EXPORT(void,png_set_flush,(png_structp png_ptr, int nrows),,51);
/* Flush the current PNG output buffer */
extern PNG_EXPORT(void,png_write_flush,(png_structp png_ptr),,52);
#endif

/* Optional update palette with requested transformations */
extern PNG_EXPORT(void,png_start_read_image,(png_structp png_ptr),,53);

/* Optional call to update the users info structure */
extern PNG_EXPORT(void,png_read_update_info,(png_structp png_ptr,
    png_infop info_ptr),,54);

#ifdef PNG_SEQUENTIAL_READ_SUPPORTED
/* Read one or more rows of image data. */
extern PNG_EXPORT(void,png_read_rows,(png_structp png_ptr, png_bytepp row,
    png_bytepp display_row, png_uint_32 num_rows),,55);
#endif

#ifdef PNG_SEQUENTIAL_READ_SUPPORTED
/* Read a row of data. */
extern PNG_EXPORT(void,png_read_row,(png_structp png_ptr, png_bytep row,
    png_bytep display_row),,56);
#endif

#ifdef PNG_SEQUENTIAL_READ_SUPPORTED
/* Read the whole image into memory at once. */
extern PNG_EXPORT(void,png_read_image,(png_structp png_ptr,
    png_bytepp image),,57);
#endif

/* Write a row of image data */
extern PNG_EXPORT(void,png_write_row,(png_structp png_ptr, png_bytep row),,58);

/* Write a few rows of image data */
extern PNG_EXPORT(void,png_write_rows,(png_structp png_ptr,
    png_bytepp row, png_uint_32 num_rows),,59);

/* Write the image data */
extern PNG_EXPORT(void,png_write_image,(png_structp png_ptr,
    png_bytepp image),,60);

/* Write the end of the PNG file. */
extern PNG_EXPORT(void,png_write_end,(png_structp png_ptr,
    png_infop info_ptr),,61);

#ifdef PNG_SEQUENTIAL_READ_SUPPORTED
/* Read the end of the PNG file. */
extern PNG_EXPORT(void,png_read_end,(png_structp png_ptr,
    png_infop info_ptr),,62);
#endif

/* Free any memory associated with the png_info_struct */
extern PNG_EXPORT(void,png_destroy_info_struct,(png_structp png_ptr,
    png_infopp info_ptr_ptr),,63);

/* Free any memory associated with the png_struct and the png_info_structs */
extern PNG_EXPORT(void,png_destroy_read_struct,(png_structpp png_ptr_ptr,
    png_infopp info_ptr_ptr, png_infopp end_info_ptr_ptr),,64);

/* Free any memory associated with the png_struct and the png_info_structs */
extern PNG_EXPORT(void,png_destroy_write_struct,(png_structpp png_ptr_ptr,
    png_infopp info_ptr_ptr),,65);

/* Set the libpng method of handling chunk CRC errors */
extern PNG_EXPORT(void,png_set_crc_action,(png_structp png_ptr,
    int crit_action, int ancil_action),,66);

/* Values for png_set_crc_action() to say how to handle CRC errors in
 * ancillary and critical chunks, and whether to use the data contained
 * therein.  Note that it is impossible to "discard" data in a critical
 * chunk.  For versions prior to 0.90, the action was always error/quit,
 * whereas in version 0.90 and later, the action for CRC errors in ancillary
 * chunks is warn/discard.  These values should NOT be changed.
 *
 *      value                       action:critical     action:ancillary
 */
#define PNG_CRC_DEFAULT       0  /* error/quit          warn/discard data */
#define PNG_CRC_ERROR_QUIT    1  /* error/quit          error/quit        */
#define PNG_CRC_WARN_DISCARD  2  /* (INVALID)           warn/discard data */
#define PNG_CRC_WARN_USE      3  /* warn/use data       warn/use data     */
#define PNG_CRC_QUIET_USE     4  /* quiet/use data      quiet/use data    */
#define PNG_CRC_NO_CHANGE     5  /* use current value   use current value */

/* These functions give the user control over the scan-line filtering in
 * libpng and the compression methods used by zlib.  These functions are
 * mainly useful for testing, as the defaults should work with most users.
 * Those users who are tight on memory or want faster performance at the
 * expense of compression can modify them.  See the compression library
 * header file (zlib.h) for an explination of the compression functions.
 */

/* Set the filtering method(s) used by libpng.  Currently, the only valid
 * value for "method" is 0.
 */
extern PNG_EXPORT(void,png_set_filter,(png_structp png_ptr, int method,
    int filters),,67);

/* Flags for png_set_filter() to say which filters to use.  The flags
 * are chosen so that they don't conflict with real filter types
 * below, in case they are supplied instead of the #defined constants.
 * These values should NOT be changed.
 */
#define PNG_NO_FILTERS     0x00
#define PNG_FILTER_NONE    0x08
#define PNG_FILTER_SUB     0x10
#define PNG_FILTER_UP      0x20
#define PNG_FILTER_AVG     0x40
#define PNG_FILTER_PAETH   0x80
#define PNG_ALL_FILTERS (PNG_FILTER_NONE | PNG_FILTER_SUB | PNG_FILTER_UP | \
                         PNG_FILTER_AVG | PNG_FILTER_PAETH)

/* Filter values (not flags) - used in pngwrite.c, pngwutil.c for now.
 * These defines should NOT be changed.
 */
#define PNG_FILTER_VALUE_NONE  0
#define PNG_FILTER_VALUE_SUB   1
#define PNG_FILTER_VALUE_UP    2
#define PNG_FILTER_VALUE_AVG   3
#define PNG_FILTER_VALUE_PAETH 4
#define PNG_FILTER_VALUE_LAST  5

#ifdef PNG_WRITE_WEIGHTED_FILTER_SUPPORTED /* EXPERIMENTAL */
/* The "heuristic_method" is given by one of the PNG_FILTER_HEURISTIC_
 * defines, either the default (minimum-sum-of-absolute-differences), or
 * the experimental method (weighted-minimum-sum-of-absolute-differences).
 *
 * Weights are factors >= 1.0, indicating how important it is to keep the
 * filter type consistent between rows.  Larger numbers mean the current
 * filter is that many times as likely to be the same as the "num_weights"
 * previous filters.  This is cumulative for each previous row with a weight.
 * There needs to be "num_weights" values in "filter_weights", or it can be
 * NULL if the weights aren't being specified.  Weights have no influence on
 * the selection of the first row filter.  Well chosen weights can (in theory)
 * improve the compression for a given image.
 *
 * Costs are factors >= 1.0 indicating the relative decoding costs of a
 * filter type.  Higher costs indicate more decoding expense, and are
 * therefore less likely to be selected over a filter with lower computational
 * costs.  There needs to be a value in "filter_costs" for each valid filter
 * type (given by PNG_FILTER_VALUE_LAST), or it can be NULL if you aren't
 * setting the costs.  Costs try to improve the speed of decompression without
 * unduly increasing the compressed image size.
 *
 * A negative weight or cost indicates the default value is to be used, and
 * values in the range [0.0, 1.0) indicate the value is to remain unchanged.
 * The default values for both weights and costs are currently 1.0, but may
 * change if good general weighting/cost heuristics can be found.  If both
 * the weights and costs are set to 1.0, this degenerates the WEIGHTED method
 * to the UNWEIGHTED method, but with added encoding time/computation.
 */
#  ifdef PNG_FLOATING_POINT_SUPPORTED
extern PNG_EXPORT(void,png_set_filter_heuristics,(png_structp png_ptr,
    int heuristic_method, int num_weights, png_doublep filter_weights,
    png_doublep filter_costs),,68);
#  endif
#endif /*  PNG_WRITE_WEIGHTED_FILTER_SUPPORTED */

/* Heuristic used for row filter selection.  These defines should NOT be
 * changed.
 */
#define PNG_FILTER_HEURISTIC_DEFAULT    0  /* Currently "UNWEIGHTED" */
#define PNG_FILTER_HEURISTIC_UNWEIGHTED 1  /* Used by libpng < 0.95 */
#define PNG_FILTER_HEURISTIC_WEIGHTED   2  /* Experimental feature */
#define PNG_FILTER_HEURISTIC_LAST       3  /* Not a valid value */

/* Set the library compression level.  Currently, valid values range from
 * 0 - 9, corresponding directly to the zlib compression levels 0 - 9
 * (0 - no compression, 9 - "maximal" compression).  Note that tests have
 * shown that zlib compression levels 3-6 usually perform as well as level 9
 * for PNG images, and do considerably fewer caclulations.  In the future,
 * these values may not correspond directly to the zlib compression levels.
 */
extern PNG_EXPORT(void,png_set_compression_level,(png_structp png_ptr,
    int level),,69);

extern PNG_EXPORT(void,png_set_compression_mem_level,(png_structp png_ptr,
    int mem_level),,70);

extern PNG_EXPORT(void,png_set_compression_strategy,(png_structp png_ptr,
    int strategy),,71);

extern PNG_EXPORT(void,png_set_compression_window_bits,(png_structp png_ptr,
    int window_bits),,72);

extern PNG_EXPORT(void,png_set_compression_method,(png_structp png_ptr,
    int method),,73);

/* These next functions are called for input/output, memory, and error
 * handling.  They are in the file pngrio.c, pngwio.c, and pngerror.c,
 * and call standard C I/O routines such as fread(), fwrite(), and
 * fprintf().  These functions can be made to use other I/O routines
 * at run time for those applications that need to handle I/O in a
 * different manner by calling png_set_???_fn().  See libpng.txt for
 * more information.
 */

#ifdef PNG_STDIO_SUPPORTED
/* Initialize the input/output for the PNG file to the default functions. */
extern PNG_EXPORT(void,png_init_io,(png_structp png_ptr, png_FILE_p fp),,74);
#endif

/* Replace the (error and abort), and warning functions with user
 * supplied functions.  If no messages are to be printed you must still
 * write and use replacement functions. The replacement error_fn should
 * still do a longjmp to the last setjmp location if you are using this
 * method of error handling.  If error_fn or warning_fn is NULL, the
 * default function will be used.
 */

extern PNG_EXPORT(void,png_set_error_fn,(png_structp png_ptr,
    png_voidp error_ptr, png_error_ptr error_fn,
    png_error_ptr warning_fn),,75);

/* Return the user pointer associated with the error functions */
extern PNG_EXPORT(png_voidp,png_get_error_ptr,(png_structp png_ptr),,76);

/* Replace the default data output functions with a user supplied one(s).
 * If buffered output is not used, then output_flush_fn can be set to NULL.
 * If PNG_WRITE_FLUSH_SUPPORTED is not defined at libpng compile time
 * output_flush_fn will be ignored (and thus can be NULL).
 * It is probably a mistake to use NULL for output_flush_fn if
 * write_data_fn is not also NULL unless you have built libpng with
 * PNG_WRITE_FLUSH_SUPPORTED undefined, because in this case libpng's
 * default flush function, which uses the standard *FILE structure, will
 * be used.
 */
extern PNG_EXPORT(void,png_set_write_fn,(png_structp png_ptr,
    png_voidp io_ptr, png_rw_ptr write_data_fn,
    png_flush_ptr output_flush_fn),,77);

/* Replace the default data input function with a user supplied one. */
extern PNG_EXPORT(void,png_set_read_fn,(png_structp png_ptr,
    png_voidp io_ptr, png_rw_ptr read_data_fn),,78);

/* Return the user pointer associated with the I/O functions */
extern PNG_EXPORT(png_voidp,png_get_io_ptr,(png_structp png_ptr),,79);

extern PNG_EXPORT(void,png_set_read_status_fn,(png_structp png_ptr,
    png_read_status_ptr read_row_fn),,80);

extern PNG_EXPORT(void,png_set_write_status_fn,(png_structp png_ptr,
    png_write_status_ptr write_row_fn),,81);

#ifdef PNG_USER_MEM_SUPPORTED
/* Replace the default memory allocation functions with user supplied one(s). */
extern PNG_EXPORT(void,png_set_mem_fn,(png_structp png_ptr,
    png_voidp mem_ptr, png_malloc_ptr malloc_fn, png_free_ptr free_fn),,82);
/* Return the user pointer associated with the memory functions */
extern PNG_EXPORT(png_voidp,png_get_mem_ptr,(png_structp png_ptr),,83);
#endif

#ifdef PNG_READ_USER_TRANSFORM_SUPPORTED
extern PNG_EXPORT(void,png_set_read_user_transform_fn,(png_structp png_ptr,
    png_user_transform_ptr read_user_transform_fn),,84);
#endif

#ifdef PNG_WRITE_USER_TRANSFORM_SUPPORTED
extern PNG_EXPORT(void,png_set_write_user_transform_fn,(png_structp png_ptr,
    png_user_transform_ptr write_user_transform_fn),,85);
#endif

#if defined(PNG_READ_USER_TRANSFORM_SUPPORTED) || \
    defined(PNG_WRITE_USER_TRANSFORM_SUPPORTED)
extern PNG_EXPORT(void,png_set_user_transform_info,(png_structp png_ptr,
    png_voidp user_transform_ptr, int user_transform_depth,
    int user_transform_channels),,86);
/* Return the user pointer associated with the user transform functions */
extern PNG_EXPORT(png_voidp,png_get_user_transform_ptr,
    (png_structp png_ptr),,87);
#endif

#ifdef PNG_USER_CHUNKS_SUPPORTED
extern PNG_EXPORT(void,png_set_read_user_chunk_fn,(png_structp png_ptr,
    png_voidp user_chunk_ptr, png_user_chunk_ptr read_user_chunk_fn),,88);
extern PNG_EXPORT(png_voidp,png_get_user_chunk_ptr,(png_structp png_ptr),,89);
#endif

#ifdef PNG_PROGRESSIVE_READ_SUPPORTED
/* Sets the function callbacks for the push reader, and a pointer to a
 * user-defined structure available to the callback functions.
 */
extern PNG_EXPORT(void,png_set_progressive_read_fn,(png_structp png_ptr,
    png_voidp progressive_ptr, png_progressive_info_ptr info_fn,
    png_progressive_row_ptr row_fn, png_progressive_end_ptr end_fn),,90);

/* Returns the user pointer associated with the push read functions */
extern PNG_EXPORT(png_voidp,png_get_progressive_ptr,(png_structp png_ptr),,91);

/* Function to be called when data becomes available */
extern PNG_EXPORT(void,png_process_data,(png_structp png_ptr,
    png_infop info_ptr, png_bytep buffer, png_size_t buffer_size),,92);

/* Function that combines rows.  Not very much different than the
 * png_combine_row() call.  Is this even used?????
 */
extern PNG_EXPORT(void,png_progressive_combine_row,(png_structp png_ptr,
    png_bytep old_row, png_bytep new_row),,93);
#endif /* PNG_PROGRESSIVE_READ_SUPPORTED */

extern PNG_EXPORT(png_voidp,png_malloc,(png_structp png_ptr,
    png_alloc_size_t size),PNG_ALLOCATED,94);
/* Added at libpng version 1.4.0 */
extern PNG_EXPORT(png_voidp,png_calloc,(png_structp png_ptr,
    png_alloc_size_t size),PNG_ALLOCATED,95);

/* Added at libpng version 1.2.4 */
extern PNG_EXPORT(png_voidp,png_malloc_warn,(png_structp png_ptr,
    png_alloc_size_t size),PNG_ALLOCATED,96);

/* Frees a pointer allocated by png_malloc() */
extern PNG_EXPORT(void,png_free,(png_structp png_ptr, png_voidp ptr),,97);

/* Free data that was allocated internally */
extern PNG_EXPORT(void,png_free_data,(png_structp png_ptr, png_infop info_ptr,
    png_uint_32 free_me, int num),,98);
/* Reassign responsibility for freeing existing data, whether allocated
 * by libpng or by the application */
extern PNG_EXPORT(void,png_data_freer,(png_structp png_ptr, png_infop info_ptr,
    int freer, png_uint_32 mask),,99);
/* Assignments for png_data_freer */
#define PNG_DESTROY_WILL_FREE_DATA 1
#define PNG_SET_WILL_FREE_DATA 1
#define PNG_USER_WILL_FREE_DATA 2
/* Flags for png_ptr->free_me and info_ptr->free_me */
#define PNG_FREE_HIST 0x0008
#define PNG_FREE_ICCP 0x0010
#define PNG_FREE_SPLT 0x0020
#define PNG_FREE_ROWS 0x0040
#define PNG_FREE_PCAL 0x0080
#define PNG_FREE_SCAL 0x0100
#define PNG_FREE_UNKN 0x0200
#define PNG_FREE_LIST 0x0400
#define PNG_FREE_PLTE 0x1000
#define PNG_FREE_TRNS 0x2000
#define PNG_FREE_TEXT 0x4000
#define PNG_FREE_ALL  0x7fff
#define PNG_FREE_MUL  0x4220 /* PNG_FREE_SPLT|PNG_FREE_TEXT|PNG_FREE_UNKN */

#ifdef PNG_USER_MEM_SUPPORTED
extern PNG_EXPORT(png_voidp,png_malloc_default,(png_structp png_ptr,
    png_alloc_size_t size),PNG_ALLOCATED,100);
extern PNG_EXPORT(void,png_free_default,(png_structp png_ptr,
    png_voidp ptr),,101);
#endif

#ifdef PNG_ERROR_TEXT_SUPPORTED
/* Fatal error in PNG image of libpng - can't continue */
extern PNG_EXPORT(void,png_error,(png_structp png_ptr,
    png_const_charp error_message),PNG_NORETURN,102);

/* The same, but the chunk name is prepended to the error string. */
extern PNG_EXPORT(void,png_chunk_error,(png_structp png_ptr,
    png_const_charp error_message),PNG_NORETURN,103);

#else
/* Fatal error in PNG image of libpng - can't continue */
extern PNG_EXPORT(void,png_err,(png_structp png_ptr),PNG_NORETURN,104);
#endif

/* Non-fatal error in libpng.  Can continue, but may have a problem. */
extern PNG_EXPORT(void,png_warning,(png_structp png_ptr,
    png_const_charp warning_message),,105);

/* Non-fatal error in libpng, chunk name is prepended to message. */
extern PNG_EXPORT(void,png_chunk_warning,(png_structp png_ptr,
    png_const_charp warning_message),,106);

#ifdef PNG_BENIGN_ERRORS_SUPPORTED
/* Benign error in libpng.  Can continue, but may have a problem.
 * User can choose whether to handle as a fatal error or as a warning. */
#  undef png_benign_error
extern PNG_EXPORT(void,png_benign_error,(png_structp png_ptr,
    png_const_charp warning_message),,107);

/* Same, chunk name is prepended to message. */
#  undef png_chunk_benign_error
extern PNG_EXPORT(void,png_chunk_benign_error,(png_structp png_ptr,
    png_const_charp warning_message),,108);

extern PNG_EXPORT(void,png_set_benign_errors,(png_structp png_ptr,
    int allowed),,109);
#else
#  ifdef PNG_ALLOW_BENIGN_ERRORS
#    define png_benign_error png_warning
#    define png_chunk_benign_error png_chunk_warning
#  else
#    define png_benign_error png_error
#    define png_chunk_benign_error png_chunk_error
#  endif
#endif

/* The png_set_<chunk> functions are for storing values in the png_info_struct.
 * Similarly, the png_get_<chunk> calls are used to read values from the
 * png_info_struct, either storing the parameters in the passed variables, or
 * setting pointers into the png_info_struct where the data is stored.  The
 * png_get_<chunk> functions return a non-zero value if the data was available
 * in info_ptr, or return zero and do not change any of the parameters if the
 * data was not available.
 *
 * These functions should be used instead of directly accessing png_info
 * to avoid problems with future changes in the size and internal layout of
 * png_info_struct.
 */
/* Returns "flag" if chunk data is valid in info_ptr. */
extern PNG_EXPORT(png_uint_32,png_get_valid,(png_structp png_ptr,
    png_infop info_ptr, png_uint_32 flag),,110);

/* Returns number of bytes needed to hold a transformed row. */
extern PNG_EXPORT(png_size_t,png_get_rowbytes,(png_structp png_ptr,
    png_infop info_ptr),,111);

#ifdef PNG_INFO_IMAGE_SUPPORTED
/* Returns row_pointers, which is an array of pointers to scanlines that was
 * returned from png_read_png().
 */
extern PNG_EXPORT(png_bytepp,png_get_rows,(png_structp png_ptr,
    png_infop info_ptr),,112);
/* Set row_pointers, which is an array of pointers to scanlines for use
 * by png_write_png().
 */
extern PNG_EXPORT(void,png_set_rows,(png_structp png_ptr, png_infop info_ptr,
    png_bytepp row_pointers),,113);
#endif

/* Returns number of color channels in image. */
extern PNG_EXPORT(png_byte,png_get_channels,(png_structp png_ptr,
    png_infop info_ptr),,114);

#ifdef PNG_EASY_ACCESS_SUPPORTED
/* Returns image width in pixels. */
extern PNG_EXPORT(png_uint_32, png_get_image_width,(png_structp png_ptr,
    png_infop info_ptr),,115);

/* Returns image height in pixels. */
extern PNG_EXPORT(png_uint_32, png_get_image_height,(png_structp png_ptr,
    png_infop info_ptr),,116);

/* Returns image bit_depth. */
extern PNG_EXPORT(png_byte, png_get_bit_depth,(png_structp png_ptr,
    png_infop info_ptr),,117);

/* Returns image color_type. */
extern PNG_EXPORT(png_byte, png_get_color_type,(png_structp png_ptr,
    png_infop info_ptr),,118);

/* Returns image filter_type. */
extern PNG_EXPORT(png_byte, png_get_filter_type,(png_structp png_ptr,
    png_infop info_ptr),,119);

/* Returns image interlace_type. */
extern PNG_EXPORT(png_byte, png_get_interlace_type,(png_structp png_ptr,
    png_infop info_ptr),,120);

/* Returns image compression_type. */
extern PNG_EXPORT(png_byte, png_get_compression_type,(png_structp png_ptr,
    png_infop info_ptr),,121);

/* Returns image resolution in pixels per meter, from pHYs chunk data. */
extern PNG_EXPORT(png_uint_32, png_get_pixels_per_meter,(png_structp png_ptr,
    png_infop info_ptr),,122);
extern PNG_EXPORT(png_uint_32, png_get_x_pixels_per_meter,
    (png_structp png_ptr, png_infop info_ptr),,123);
extern PNG_EXPORT(png_uint_32, png_get_y_pixels_per_meter,
    (png_structp png_ptr, png_infop info_ptr),,124);

/* Returns pixel aspect ratio, computed from pHYs chunk data.  */
#  ifdef PNG_FLOATING_POINT_SUPPORTED
extern PNG_EXPORT(float, png_get_pixel_aspect_ratio,
    (png_structp png_ptr, png_infop info_ptr),,125);
#  endif

/* Returns image x, y offset in pixels or microns, from oFFs chunk data. */
extern PNG_EXPORT(png_int_32, png_get_x_offset_pixels,
    (png_structp png_ptr, png_infop info_ptr),,126);
extern PNG_EXPORT(png_int_32, png_get_y_offset_pixels,
    (png_structp png_ptr, png_infop info_ptr),,127);
extern PNG_EXPORT(png_int_32, png_get_x_offset_microns,
    (png_structp png_ptr, png_infop info_ptr),,128);
extern PNG_EXPORT(png_int_32, png_get_y_offset_microns,
    (png_structp png_ptr, png_infop info_ptr),,129);

#endif /* PNG_EASY_ACCESS_SUPPORTED */

/* Returns pointer to signature string read from PNG header */
extern PNG_EXPORT(png_bytep,png_get_signature,(png_structp png_ptr,
    png_infop info_ptr),,130);

#ifdef PNG_bKGD_SUPPORTED
extern PNG_EXPORT(png_uint_32,png_get_bKGD,(png_structp png_ptr,
    png_infop info_ptr, png_color_16p *background),,131);
#endif

#ifdef PNG_bKGD_SUPPORTED
extern PNG_EXPORT(void,png_set_bKGD,(png_structp png_ptr,
    png_infop info_ptr, png_color_16p background),,132);
#endif

#ifdef PNG_cHRM_SUPPORTED
#  ifdef PNG_FLOATING_POINT_SUPPORTED
extern PNG_EXPORT(png_uint_32,png_get_cHRM,(png_structp png_ptr,
    png_infop info_ptr, double *white_x, double *white_y,
    double *red_x, double *red_y, double *green_x, double *green_y,
    double *blue_x, double *blue_y),,133);
#  endif
#  ifdef PNG_FIXED_POINT_SUPPORTED
extern PNG_EXPORT(png_uint_32,png_get_cHRM_fixed,(png_structp png_ptr,
    png_infop info_ptr,
    png_fixed_point *int_white_x, png_fixed_point *int_white_y,
    png_fixed_point *int_red_x, png_fixed_point *int_red_y,
    png_fixed_point *int_green_x, png_fixed_point *int_green_y,
    png_fixed_point *int_blue_x, png_fixed_point *int_blue_y),,134);
#  endif
#endif

#ifdef PNG_cHRM_SUPPORTED
#  ifdef PNG_FLOATING_POINT_SUPPORTED
extern PNG_EXPORT(void,png_set_cHRM,(png_structp png_ptr,
    png_infop info_ptr, double white_x, double white_y,
    double red_x, double red_y, double green_x, double green_y,
    double blue_x, double blue_y),,135);
#  endif
#  ifdef PNG_FIXED_POINT_SUPPORTED
extern PNG_EXPORT(void,png_set_cHRM_fixed,(png_structp png_ptr,
    png_infop info_ptr,
    png_fixed_point int_white_x, png_fixed_point int_white_y,
    png_fixed_point int_red_x, png_fixed_point int_red_y,
    png_fixed_point int_green_x, png_fixed_point int_green_y,
    png_fixed_point int_blue_x, png_fixed_point int_blue_y),,136);
#  endif
#endif

#ifdef PNG_gAMA_SUPPORTED
#  ifdef PNG_FLOATING_POINT_SUPPORTED
extern PNG_EXPORT(png_uint_32,png_get_gAMA,(png_structp png_ptr,
    png_infop info_ptr, double *file_gamma),,137);
#  endif
extern PNG_EXPORT(png_uint_32,png_get_gAMA_fixed,(png_structp png_ptr,
    png_infop info_ptr, png_fixed_point *int_file_gamma),,138);
#endif

#ifdef PNG_gAMA_SUPPORTED
#  ifdef PNG_FLOATING_POINT_SUPPORTED
extern PNG_EXPORT(void,png_set_gAMA,(png_structp png_ptr,
    png_infop info_ptr, double file_gamma),,139);
#  endif
extern PNG_EXPORT(void,png_set_gAMA_fixed,(png_structp png_ptr,
    png_infop info_ptr, png_fixed_point int_file_gamma),,140);
#endif

#ifdef PNG_hIST_SUPPORTED
extern PNG_EXPORT(png_uint_32,png_get_hIST,(png_structp png_ptr,
    png_infop info_ptr, png_uint_16p *hist),,141);
#endif

#ifdef PNG_hIST_SUPPORTED
extern PNG_EXPORT(void,png_set_hIST,(png_structp png_ptr,
    png_infop info_ptr, png_uint_16p hist),,142);
#endif

extern PNG_EXPORT(png_uint_32,png_get_IHDR,(png_structp png_ptr,
    png_infop info_ptr, png_uint_32 *width, png_uint_32 *height,
    int *bit_depth, int *color_type, int *interlace_method,
    int *compression_method, int *filter_method),,143);

extern PNG_EXPORT(void,png_set_IHDR,(png_structp png_ptr,
    png_infop info_ptr, png_uint_32 width, png_uint_32 height, int bit_depth,
    int color_type, int interlace_method, int compression_method,
    int filter_method),,144);

#ifdef PNG_oFFs_SUPPORTED
extern PNG_EXPORT(png_uint_32,png_get_oFFs,(png_structp png_ptr,
    png_infop info_ptr, png_int_32 *offset_x, png_int_32 *offset_y,
    int *unit_type),,145);
#endif

#ifdef PNG_oFFs_SUPPORTED
extern PNG_EXPORT(void,png_set_oFFs,(png_structp png_ptr,
    png_infop info_ptr, png_int_32 offset_x, png_int_32 offset_y,
    int unit_type),,146);
#endif

#ifdef PNG_pCAL_SUPPORTED
extern PNG_EXPORT(png_uint_32,png_get_pCAL,(png_structp png_ptr,
    png_infop info_ptr, png_charp *purpose, png_int_32 *X0, png_int_32 *X1,
    int *type, int *nparams, png_charp *units, png_charpp *params),,147);
#endif

#ifdef PNG_pCAL_SUPPORTED
extern PNG_EXPORT(void,png_set_pCAL,(png_structp png_ptr,
    png_infop info_ptr, png_charp purpose, png_int_32 X0, png_int_32 X1,
    int type, int nparams, png_charp units, png_charpp params),,148);
#endif

#ifdef PNG_pHYs_SUPPORTED
extern PNG_EXPORT(png_uint_32,png_get_pHYs,(png_structp png_ptr,
    png_infop info_ptr, png_uint_32 *res_x, png_uint_32 *res_y,
    int *unit_type),,149);
#endif

#ifdef PNG_pHYs_SUPPORTED
extern PNG_EXPORT(void,png_set_pHYs,(png_structp png_ptr,
    png_infop info_ptr, png_uint_32 res_x, png_uint_32 res_y,
    int unit_type),,150);
#endif

extern PNG_EXPORT(png_uint_32,png_get_PLTE,(png_structp png_ptr,
    png_infop info_ptr, png_colorp *palette, int *num_palette),,151);

extern PNG_EXPORT(void,png_set_PLTE,(png_structp png_ptr,
    png_infop info_ptr, png_colorp palette, int num_palette),,152);

#ifdef PNG_sBIT_SUPPORTED
extern PNG_EXPORT(png_uint_32,png_get_sBIT,(png_structp png_ptr,
    png_infop info_ptr, png_color_8p *sig_bit),,153);
#endif

#ifdef PNG_sBIT_SUPPORTED
extern PNG_EXPORT(void,png_set_sBIT,(png_structp png_ptr,
    png_infop info_ptr, png_color_8p sig_bit),,154);
#endif

#ifdef PNG_sRGB_SUPPORTED
extern PNG_EXPORT(png_uint_32,png_get_sRGB,(png_structp png_ptr,
    png_infop info_ptr, int *intent),,155);
#endif

#ifdef PNG_sRGB_SUPPORTED
extern PNG_EXPORT(void,png_set_sRGB,(png_structp png_ptr,
    png_infop info_ptr, int intent),,156);
extern PNG_EXPORT(void,png_set_sRGB_gAMA_and_cHRM,(png_structp png_ptr,
    png_infop info_ptr, int intent),,157);
#endif

#ifdef PNG_iCCP_SUPPORTED
extern PNG_EXPORT(png_uint_32,png_get_iCCP,(png_structp png_ptr,
    png_infop info_ptr, png_charpp name, int *compression_type,
    png_charpp profile, png_uint_32 *proflen),,158);
    /* Note to maintainer: profile should be png_bytepp */
#endif

#ifdef PNG_iCCP_SUPPORTED
extern PNG_EXPORT(void,png_set_iCCP,(png_structp png_ptr,
    png_infop info_ptr, png_charp name, int compression_type,
    png_charp profile, png_uint_32 proflen),,159);
    /* Note to maintainer: profile should be png_bytep */
#endif

#ifdef PNG_sPLT_SUPPORTED
extern PNG_EXPORT(png_uint_32,png_get_sPLT,(png_structp png_ptr,
    png_infop info_ptr, png_sPLT_tpp entries),,160);
#endif

#ifdef PNG_sPLT_SUPPORTED
extern PNG_EXPORT(void,png_set_sPLT,(png_structp png_ptr,
    png_infop info_ptr, png_sPLT_tp entries, int nentries),,161);
#endif

#ifdef PNG_TEXT_SUPPORTED
/* png_get_text also returns the number of text chunks in *num_text */
extern PNG_EXPORT(png_uint_32,png_get_text,(png_structp png_ptr,
    png_infop info_ptr, png_textp *text_ptr, int *num_text),,162);
#endif

/* Note while png_set_text() will accept a structure whose text,
 * language, and  translated keywords are NULL pointers, the structure
 * returned by png_get_text will always contain regular
 * zero-terminated C strings.  They might be empty strings but
 * they will never be NULL pointers.
 */

#ifdef PNG_TEXT_SUPPORTED
extern PNG_EXPORT(void,png_set_text,(png_structp png_ptr,
    png_infop info_ptr, png_textp text_ptr, int num_text),,163);
#endif

#ifdef PNG_tIME_SUPPORTED
extern PNG_EXPORT(png_uint_32,png_get_tIME,(png_structp png_ptr,
    png_infop info_ptr, png_timep *mod_time),,164);
#endif

#ifdef PNG_tIME_SUPPORTED
extern PNG_EXPORT(void,png_set_tIME,(png_structp png_ptr,
    png_infop info_ptr, png_timep mod_time),,165);
#endif

#ifdef PNG_tRNS_SUPPORTED
extern PNG_EXPORT(png_uint_32,png_get_tRNS,(png_structp png_ptr,
    png_infop info_ptr, png_bytep *trans_alpha, int *num_trans,
    png_color_16p *trans_color),,166);
#endif

#ifdef PNG_tRNS_SUPPORTED
extern PNG_EXPORT(void,png_set_tRNS,(png_structp png_ptr,
    png_infop info_ptr, png_bytep trans_alpha, int num_trans,
    png_color_16p trans_color),,167);
#endif

#ifdef PNG_sCAL_SUPPORTED
#  ifdef PNG_FLOATING_POINT_SUPPORTED
extern PNG_EXPORT(png_uint_32,png_get_sCAL,(png_structp png_ptr,
    png_infop info_ptr, int *unit, double *width, double *height),,168);
#  else
#    ifdef PNG_FIXED_POINT_SUPPORTED
extern PNG_EXPORT(png_uint_32,png_get_sCAL_s,(png_structp png_ptr,
    png_infop info_ptr, int *unit, png_charpp swidth,
    png_charpp sheight),,169);
#    endif
#  endif
#endif /* PNG_sCAL_SUPPORTED */

#ifdef PNG_sCAL_SUPPORTED
#  ifdef PNG_FLOATING_POINT_SUPPORTED
extern PNG_EXPORT(void,png_set_sCAL,(png_structp png_ptr,
    png_infop info_ptr, int unit, double width, double height),,170);
#  else
#    ifdef PNG_FIXED_POINT_SUPPORTED
extern PNG_EXPORT(void,png_set_sCAL_s,(png_structp png_ptr,
    png_infop info_ptr, int unit, png_charp swidth,
    png_charp sheight),,171);
#    endif
#  endif
#endif /* PNG_sCAL_SUPPORTED || PNG_WRITE_sCAL_SUPPORTED */

#ifdef PNG_HANDLE_AS_UNKNOWN_SUPPORTED
/* Provide a list of chunks and how they are to be handled, if the built-in
   handling or default unknown chunk handling is not desired.  Any chunks not
   listed will be handled in the default manner.  The IHDR and IEND chunks
   must not be listed.
      keep = 0: follow default behaviour
           = 1: do not keep
           = 2: keep only if safe-to-copy
           = 3: keep even if unsafe-to-copy
*/
extern PNG_EXPORT(void, png_set_keep_unknown_chunks,(png_structp png_ptr,
    int keep, png_bytep chunk_list, int num_chunks),,172);
extern PNG_EXPORT(int,png_handle_as_unknown,(png_structp png_ptr,
    png_bytep chunk_name),,173);
#endif
#ifdef PNG_UNKNOWN_CHUNKS_SUPPORTED
extern PNG_EXPORT(void, png_set_unknown_chunks,(png_structp png_ptr,
    png_infop info_ptr, png_unknown_chunkp unknowns,
    int num_unknowns),,174);
extern PNG_EXPORT(void, png_set_unknown_chunk_location,
    (png_structp png_ptr, png_infop info_ptr, int chunk,
    int location),,175);
extern PNG_EXPORT(png_uint_32,png_get_unknown_chunks,(png_structp png_ptr,
    png_infop info_ptr, png_unknown_chunkpp entries),,176);
#endif

/* Png_free_data() will turn off the "valid" flag for anything it frees.
 * If you need to turn it off for a chunk that your application has freed,
 * you can use png_set_invalid(png_ptr, info_ptr, PNG_INFO_CHNK);
 */
extern PNG_EXPORT(void, png_set_invalid,(png_structp png_ptr,
    png_infop info_ptr, int mask),,177);

#ifdef PNG_INFO_IMAGE_SUPPORTED
/* The "params" pointer is currently not used and is for future expansion. */
extern PNG_EXPORT(void, png_read_png,(png_structp png_ptr,
    png_infop info_ptr, int transforms, png_voidp params),,178);
extern PNG_EXPORT(void, png_write_png,(png_structp png_ptr,
    png_infop info_ptr, int transforms, png_voidp params),,179);
#endif

extern PNG_EXPORT(png_charp,png_get_copyright,(png_structp png_ptr),,180);
extern PNG_EXPORT(png_charp,png_get_header_ver,(png_structp png_ptr),,181);
extern PNG_EXPORT(png_charp,png_get_header_version,(png_structp png_ptr),,182);
extern PNG_EXPORT(png_charp,png_get_libpng_ver,(png_structp png_ptr),,183);

#ifdef PNG_MNG_FEATURES_SUPPORTED
extern PNG_EXPORT(png_uint_32,png_permit_mng_features,(png_structp png_ptr,
    png_uint_32 mng_features_permitted),,184);
#endif

/* For use in png_set_keep_unknown, added to version 1.2.6 */
#define PNG_HANDLE_CHUNK_AS_DEFAULT   0
#define PNG_HANDLE_CHUNK_NEVER        1
#define PNG_HANDLE_CHUNK_IF_SAFE      2
#define PNG_HANDLE_CHUNK_ALWAYS       3

/* Strip the prepended error numbers ("#nnn ") from error and warning
 * messages before passing them to the error or warning handler.
 */
#ifdef PNG_ERROR_NUMBERS_SUPPORTED
extern PNG_EXPORT(void,png_set_strip_error_numbers,(png_structp png_ptr,
    png_uint_32 strip_mode),,185);
#endif

/* Added in libpng-1.2.6 */
#ifdef PNG_SET_USER_LIMITS_SUPPORTED
extern PNG_EXPORT(void,png_set_user_limits,(png_structp png_ptr,
    png_uint_32 user_width_max, png_uint_32 user_height_max),,186);
extern PNG_EXPORT(png_uint_32,png_get_user_width_max,
    (png_structp png_ptr),,187);
extern PNG_EXPORT(png_uint_32,png_get_user_height_max,
    (png_structp png_ptr),,188);
/* Added in libpng-1.4.0 */
extern PNG_EXPORT(void,png_set_chunk_cache_max,(png_structp png_ptr,
    png_uint_32 user_chunk_cache_max),,189);
extern PNG_EXPORT(png_uint_32,png_get_chunk_cache_max,
    (png_structp png_ptr),,190);
/* Added in libpng-1.4.1 */
extern PNG_EXPORT(void,png_set_chunk_malloc_max,(png_structp png_ptr,
    png_alloc_size_t user_chunk_cache_max),,191);
extern PNG_EXPORT(png_alloc_size_t,png_get_chunk_malloc_max,
    (png_structp png_ptr),,192);
#endif

#if defined(PNG_INCH_CONVERSIONS_SUPPORTED) && defined(PNG_FLOATING_POINT_SUPPORTED)
PNG_EXPORT(png_uint_32,png_get_pixels_per_inch,(png_structp png_ptr,
    png_infop info_ptr),,193);

PNG_EXPORT(png_uint_32,png_get_x_pixels_per_inch,(png_structp png_ptr,
    png_infop info_ptr),,194);

PNG_EXPORT(png_uint_32,png_get_y_pixels_per_inch,(png_structp png_ptr,
    png_infop info_ptr),,195);

PNG_EXPORT(float,png_get_x_offset_inches,(png_structp png_ptr,
    png_infop info_ptr),,196);

PNG_EXPORT(float,png_get_y_offset_inches,(png_structp png_ptr,
    png_infop info_ptr),,197);

#  ifdef PNG_pHYs_SUPPORTED
PNG_EXPORT(png_uint_32,png_get_pHYs_dpi,(png_structp png_ptr,
    png_infop info_ptr, png_uint_32 *res_x, png_uint_32 *res_y,
    int *unit_type),,198);
#  endif /* PNG_pHYs_SUPPORTED */
#endif  /* PNG_INCH_CONVERSIONS_SUPPORTED && PNG_FLOATING_POINT_SUPPORTED */

/* Added in libpng-1.4.0 */
#ifdef PNG_IO_STATE_SUPPORTED
extern PNG_EXPORT(png_uint_32,png_get_io_state,(png_structp png_ptr),,199);

extern PNG_EXPORT(png_bytep,png_get_io_chunk_name,(png_structp png_ptr),,200);

/* The flags returned by png_get_io_state() are the following: */
#  define PNG_IO_NONE        0x0000   /* no I/O at this moment */
#  define PNG_IO_READING     0x0001   /* currently reading */
#  define PNG_IO_WRITING     0x0002   /* currently writing */
#  define PNG_IO_SIGNATURE   0x0010   /* currently at the file signature */
#  define PNG_IO_CHUNK_HDR   0x0020   /* currently at the chunk header */
#  define PNG_IO_CHUNK_DATA  0x0040   /* currently at the chunk data */
#  define PNG_IO_CHUNK_CRC   0x0080   /* currently at the chunk crc */
#  define PNG_IO_MASK_OP     0x000f   /* current operation: reading/writing */
#  define PNG_IO_MASK_LOC    0x00f0   /* current location: sig/hdr/data/crc */
#endif /* ?PNG_IO_STATE_SUPPORTED */

/* Maintainer: Put new public prototypes here ^, in libpng.3, and project
 * defs
 */

#ifdef PNG_READ_COMPOSITE_NODIV_SUPPORTED
/* With these routines we avoid an integer divide, which will be slower on
 * most machines.  However, it does take more operations than the corresponding
 * divide method, so it may be slower on a few RISC systems.  There are two
 * shifts (by 8 or 16 bits) and an addition, versus a single integer divide.
 *
 * Note that the rounding factors are NOT supposed to be the same!  128 and
 * 32768 are correct for the NODIV code; 127 and 32767 are correct for the
 * standard method.
 *
 * [Optimized code by Greg Roelofs and Mark Adler...blame us for bugs. :-) ]
 */

 /* fg and bg should be in `gamma 1.0' space; alpha is the opacity */

#  define png_composite(composite, fg, alpha, bg)         \
     { png_uint_16 temp = (png_uint_16)((png_uint_16)(fg) \
           * (png_uint_16)(alpha)                         \
           + (png_uint_16)(bg)*(png_uint_16)(255          \
           - (png_uint_16)(alpha)) + (png_uint_16)128);   \
       (composite) = (png_byte)((temp + (temp >> 8)) >> 8); }

#  define png_composite_16(composite, fg, alpha, bg)       \
     { png_uint_32 temp = (png_uint_32)((png_uint_32)(fg)  \
           * (png_uint_32)(alpha)                          \
           + (png_uint_32)(bg)*(png_uint_32)(65535L        \
           - (png_uint_32)(alpha)) + (png_uint_32)32768L); \
       (composite) = (png_uint_16)((temp + (temp >> 16)) >> 16); }

#else  /* Standard method using integer division */

#  define png_composite(composite, fg, alpha, bg)                          \
     (composite) = (png_byte)(((png_uint_16)(fg) * (png_uint_16)(alpha) +  \
     (png_uint_16)(bg) * (png_uint_16)(255 - (png_uint_16)(alpha)) +       \
     (png_uint_16)127) / 255)

#  define png_composite_16(composite, fg, alpha, bg)                         \
     (composite) = (png_uint_16)(((png_uint_32)(fg) * (png_uint_32)(alpha) + \
     (png_uint_32)(bg)*(png_uint_32)(65535L - (png_uint_32)(alpha)) +        \
     (png_uint_32)32767) / (png_uint_32)65535L)
#endif /* PNG_READ_COMPOSITE_NODIV_SUPPORTED */

#ifdef PNG_USE_READ_MACROS
/* Inline macros to do direct reads of bytes from the input buffer.
 * The png_get_int_32() routine assumes we are using two's complement
 * format for negative values, which is almost certainly true.
 */
#  define png_get_uint_32(buf) \
     (((png_uint_32)(*(buf)) << 24) + \
      ((png_uint_32)(*((buf) + 1)) << 16) + \
      ((png_uint_32)(*((buf) + 2)) << 8) + \
      ((png_uint_32)(*((buf) + 3))))
#  define png_get_uint_16(buf) \
     (((png_uint_32)(*(buf)) << 8) + \
      ((png_uint_32)(*((buf) + 1))))
#  ifdef PNG_GET_INT_32_SUPPORTED
#    define png_get_int_32(buf) \
     (((png_int_32)(*(buf)) << 24) + \
      ((png_int_32)(*((buf) + 1)) << 16) + \
      ((png_int_32)(*((buf) + 2)) << 8) + \
      ((png_int_32)(*((buf) + 3))))
#  endif
#endif
extern PNG_EXPORT(png_uint_32,png_get_uint_32,(png_bytep buf),,201);
extern PNG_EXPORT(png_uint_16,png_get_uint_16,(png_bytep buf),,202);
#  ifdef PNG_GET_INT_32_SUPPORTED
extern PNG_EXPORT(png_int_32,png_get_int_32,(png_bytep buf),,203);
#  endif
extern PNG_EXPORT(png_uint_32,png_get_uint_31,(png_structp png_ptr,
    png_bytep buf),,204);
/* No png_get_int_16 -- may be added if there's a real need for it. */

/* Place a 32-bit number into a buffer in PNG byte order (big-endian). */
extern PNG_EXPORT(void,png_save_uint_32,(png_bytep buf, png_uint_32 i),,205);
#ifdef PNG_SAVE_INT_32_SUPPORTED
extern PNG_EXPORT(void,png_save_int_32,(png_bytep buf, png_int_32 i),,206);
#endif

/* Place a 16-bit number into a buffer in PNG byte order.
 * The parameter is declared unsigned int, not png_uint_16,
 * just to avoid potential problems on pre-ANSI C compilers.
 */
extern PNG_EXPORT(void,png_save_uint_16,(png_bytep buf, unsigned int i),,207);
/* No png_save_int_16 -- may be added if there's a real need for it. */

/* The last ordinal number (this is the *last* one, the next one to
 * use is one more than this.)
 */
#ifdef PNG_EXPORT_LAST_ORDINAL
  PNG_EXPORT_LAST_ORDINAL(207);
#endif

#ifdef __cplusplus
}
#endif

#endif /* PNG_VERSION_INFO_ONLY */
/* Do not put anything past this line */
#endif /* PNG_H */
