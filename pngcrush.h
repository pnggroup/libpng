/* pngcrush.h */

/*
 * This software is released under the zlib/libpng license (see LICENSE,
 * in pngcrush.c).
 */

/* Special defines for pngcrush, mostly just to reduce the size of the
   static executable. */

#ifndef PNGCRUSH_H
#define PNGCRUSH_H

/*
#include <malloc.h>
*/

#ifndef PNG_NO_ZALLOC_ZERO
#  define PNG_NO_ZALLOC_ZERO  /* speeds it up a little */
#endif

#ifndef PNG_USER_MEM_SUPPORTED
#  define PNG_USER_MEM_SUPPORTED
#endif

#define PNG_MNG_FEATURES_SUPPORTED /* extra filter type */

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

# define PNG_NO_READ_cHRM
# define PNG_NO_READ_hIST
# define PNG_NO_READ_iCCP
# define PNG_NO_READ_pCAL
# define PNG_NO_READ_sCAL
# define PNG_NO_READ_sPLT
# define PNG_NO_READ_tIME
#define PNG_NO_ASSEMBLER_CODE
#define PNG_NO_CHECK_cHRM
#define PNG_NO_READ_BGR
#define PNG_NO_READ_DITHER
#define PNG_NO_READ_EMPTY_PLTE
#define PNG_NO_PROGRESSIVE_READ
#define PNG_NO_READ_COMPOSITED_NODIV
#define PNG_NO_READ_INVERT_ALPHA
#define PNG_NO_READ_PREMULTIPLY_ALPHA
#define PNG_NO_READ_SWAP
#define PNG_NO_READ_SWAP_ALPHA
#define PNG_READ_USER_TRANSFORM_SUPPORTED
#define PNG_READ_STRIP_ALPHA_SUPPORTED
#define PNG_READ_EXPAND_SUPPORTED
#define PNG_READ_FILLER_SUPPORTED
#define PNG_READ_PACK_SUPPORTED
#define PNG_READ_SHIFT_SUPPORTED

# define PNG_NO_WRITE_cHRM
# define PNG_NO_WRITE_hIST
# define PNG_NO_WRITE_iCCP
# define PNG_NO_WRITE_pCAL
# define PNG_NO_WRITE_sCAL
# define PNG_NO_WRITE_sPLT
# define PNG_NO_WRITE_tIME
#define PNG_NO_WRITE_TRANSFORMS
#define PNG_WRITE_PACK_SUPPORTED
#define PNG_WRITE_SHIFT_SUPPORTED
#define PNG_NO_WRITE_WEIGHTED_FILTER

#define PNG_NO_ERROR_NUMBERS
#define PNG_NO_INFO_IMAGE
#define PNG_EASY_ACCESS

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

/* This allows png_default_error() to return, when it is called after our
   own exception handling, which only returns after "Too many IDAT's",
   or anything else that we might want to handle as a warning instead of
   an error. */
#define PNG_ABORT()

#endif /* !PNGCRUSH_H */
