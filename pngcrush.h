/* pngcrush.h */

/* Special defines for pngcrush, mostly just to reduce the size of the
   static executable. */

#ifndef PNGCRUSH_H
#define PNGCRUSH_H

/*
#include <malloc.h>
*/

#ifdef PNG_LIBPNG_VER
#define PNGCRUSH_LIBPNG_VER PNG_LIBPNG_VER
#else
/* This must agree with PNG_LIBPNG_VER; you have to define it manually
   here if you are using libpng-1.0.6h or earlier */
#define PNGCRUSH_LIBPNG_VER 10007
#endif

#ifndef PNG_NO_ZALLOC_ZERO
#  define PNG_NO_ZALLOC_ZERO  /* speeds it up a little */
#endif

#ifndef PNG_USER_MEM_SUPPORTED
#  define PNG_USER_MEM_SUPPORTED
#endif

#define PNG_MNG_FEATURES_SUPPORTED /* extra filter types */

#ifndef PNG_NO_LEGACY_SUPPORTED
#  define PNG_NO_LEGACY_SUPPORTED
#endif

#ifndef PNG_SETJMP_NOT_SUPPORTED
#  define PNG_SETJMP_NOT_SUPPORTED
#endif

#if PNGCRUSH_LIBPNG_VER > 10006
#define PNG_NO_FLOATING_POINT_SUPPORTED
#define PNG_READ_GRAY_TO_RGB_SUPPORTED
#endif

#define PNG_NO_READ_cHRM
#define PNG_NO_WRITE_cHRM
#define PNG_NO_READ_hIST
#define PNG_NO_WRITE_hIST
#define PNG_NO_READ_pCAL
#define PNG_NO_WRITE_pCAL
#define PNG_NO_READ_sCAL
#define PNG_NO_WRITE_sCAL
#define PNG_NO_READ_sPLT
#define PNG_NO_WRITE_sPLT
#define PNG_NO_READ_tIME
#define PNG_NO_WRITE_tIME

#define PNG_NO_INFO_IMAGE
#define PNG_NO_READ_USER_CHUNKS
#define PNG_EASY_ACCESS
#define PNG_NO_READ_DITHER
#define PNG_NO_READ_EMPTY_PLTE
#define PNG_NO_WRITE_TRANSFORMS
#define PNG_NO_PROGRESSIVE_READ
#define PNG_NO_WRITE_WEIGHTED_FILTER
#define PNG_NO_READ_COMPOSITED_NODIV

#define PNG_READ_USER_TRANSFORM_SUPPORTED
#define PNG_READ_STRIP_ALPHA_SUPPORTED
#define PNG_READ_EXPAND_SUPPORTED
#define PNG_READ_FILLER_SUPPORTED

#if (PNGCRUSH_LIBPNG_VER > 10002)
/* versions 0.96 through 1.0.2 have a stub png_rgb_to_gray() with the
 * wrong number of parameters */
#  define PNG_READ_RGB_TO_GRAY_SUPPORTED
#endif

#ifndef PNG_NO_iTXt_SUPPORTED
#  define PNG_iTXt_SUPPORTED
#endif

#ifndef PNG_NO_FLOATING_POINT_SUPPORTED
#  define PNG_READ_GRAY_TO_RGB_SUPPORTED
#  define PNG_READ_BACKGROUND_SUPPORTED
#  define PNG_READ_GAMMA_SUPPORTED
#else
#  if (PNGCRUSH_LIBPNG_VER < 10007)
#    define PNG_NO_READ_RGB_TO_GRAY
#  endif
#endif
#if !defined(PNG_ZBUF_SIZE) && (PNGCRUSH_LIBPNG_VER > 97)
#  define PNG_ZBUF_SIZE 524288       /* increases the IDAT size */
#endif

/* Changed in version 0.99 */
#if PNGCRUSH_LIBPNG_VER < 99
#undef PNG_CONST
#ifndef PNG_NO_CONST
#  define PNG_CONST const
#else
#  define PNG_CONST
#endif
#endif

/* This allows png_default_error() to return, when it is called after our
   own exception handling, which only returns after "Too many IDAT's",
   or anything else that we might want to handle as a warning instead of
   an error. */
#define PNG_ABORT()

#endif
