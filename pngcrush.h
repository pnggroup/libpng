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

#if !defined(PNG_USE_PNGGCCRD) && !defined(PNG_USE_PNGVCRD) && \
    !defined(PNG_NO_ASSEMBLER_CODE)
#define PNG_NO_ASSEMBLER_CODE
#endif

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
#define PNG_READ_PACK_SUPPORTED
#define PNG_READ_SHIFT_SUPPORTED

#define PNG_WRITE_PACK_SUPPORTED
#define PNG_WRITE_SHIFT_SUPPORTED

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

/* GRR 20050220:  added these, which apparently aren't defined anywhere else */
#ifndef PNG_UINT_IHDR
#  define PNG_UINT_IHDR (((png_uint_32)  73<<24) | \
                         ((png_uint_32)  72<<16) | \
                         ((png_uint_32)  68<< 8) | \
                         ((png_uint_32)  82    ))
#endif

#ifndef PNG_UINT_IDAT
#  define PNG_UINT_IDAT (((png_uint_32)  73<<24) | \
                         ((png_uint_32)  68<<16) | \
                         ((png_uint_32)  65<< 8) | \
                         ((png_uint_32)  84    ))
#endif

#ifndef PNG_UINT_IEND
#  define PNG_UINT_IEND (((png_uint_32)  73<<24) | \
                         ((png_uint_32)  69<<16) | \
                         ((png_uint_32)  78<< 8) | \
                         ((png_uint_32)  68    ))
#endif

#ifndef PNG_UINT_PLTE
#  define PNG_UINT_PLTE (((png_uint_32)  80<<24) | \
                         ((png_uint_32)  76<<16) | \
                         ((png_uint_32)  84<< 8) | \
                         ((png_uint_32)  69    ))
#endif

#ifndef PNG_UINT_bKGD
#  define PNG_UINT_bKGD (((png_uint_32)  98<<24) | \
                         ((png_uint_32)  75<<16) | \
                         ((png_uint_32)  71<< 8) | \
                         ((png_uint_32)  68    ))
#endif

#ifndef PNG_UINT_cHRM
#  define PNG_UINT_cHRM (((png_uint_32)  99<<24) | \
                         ((png_uint_32)  72<<16) | \
                         ((png_uint_32)  82<< 8) | \
                         ((png_uint_32)  77    ))
#endif

#ifndef PNG_UINT_gAMA
#  define PNG_UINT_gAMA (((png_uint_32) 103<<24) | \
                         ((png_uint_32)  65<<16) | \
                         ((png_uint_32)  77<< 8) | \
                         ((png_uint_32)  65    ))
#endif

#ifndef PNG_UINT_hIST
#  define PNG_UINT_hIST (((png_uint_32) 104<<24) | \
                         ((png_uint_32)  73<<16) | \
                         ((png_uint_32)  83<< 8) | \
                         ((png_uint_32)  84    ))
#endif

#ifndef PNG_UINT_iCCP
#  define PNG_UINT_iCCP (((png_uint_32) 105<<24) | \
                         ((png_uint_32)  67<<16) | \
                         ((png_uint_32)  67<< 8) | \
                         ((png_uint_32)  80    ))
#endif

#ifndef PNG_UINT_iTXt
#  define PNG_UINT_iTXt (((png_uint_32) 105<<24) | \
                         ((png_uint_32)  84<<16) | \
                         ((png_uint_32)  88<< 8) | \
                         ((png_uint_32) 116    ))
#endif

#ifndef PNG_UINT_oFFs
#  define PNG_UINT_oFFs (((png_uint_32) 111<<24) | \
                         ((png_uint_32)  70<<16) | \
                         ((png_uint_32)  70<< 8) | \
                         ((png_uint_32) 115    ))
#endif

#ifndef PNG_UINT_pCAL
#  define PNG_UINT_pCAL (((png_uint_32) 112<<24) | \
                         ((png_uint_32)  67<<16) | \
                         ((png_uint_32)  65<< 8) | \
                         ((png_uint_32)  76    ))
#endif

#ifndef PNG_UINT_sCAL
#  define PNG_UINT_sCAL (((png_uint_32) 115<<24) | \
                         ((png_uint_32)  67<<16) | \
                         ((png_uint_32)  65<< 8) | \
                         ((png_uint_32)  76    ))
#endif

#ifndef PNG_UINT_pHYs
#  define PNG_UINT_pHYs (((png_uint_32) 112<<24) | \
                         ((png_uint_32)  72<<16) | \
                         ((png_uint_32)  89<< 8) | \
                         ((png_uint_32) 115    ))
#endif

#ifndef PNG_UINT_sBIT
#  define PNG_UINT_sBIT (((png_uint_32) 115<<24) | \
                         ((png_uint_32)  66<<16) | \
                         ((png_uint_32)  73<< 8) | \
                         ((png_uint_32)  84    ))
#endif

#ifndef PNG_UINT_sPLT
#  define PNG_UINT_sPLT (((png_uint_32) 115<<24) | \
                         ((png_uint_32)  80<<16) | \
                         ((png_uint_32)  76<< 8) | \
                         ((png_uint_32)  84    ))
#endif

#ifndef PNG_UINT_sRGB
#  define PNG_UINT_sRGB (((png_uint_32) 115<<24) | \
                         ((png_uint_32)  82<<16) | \
                         ((png_uint_32)  71<< 8) | \
                         ((png_uint_32)  66    ))
#endif

#ifndef PNG_UINT_tEXt
#  define PNG_UINT_tEXt (((png_uint_32) 116<<24) | \
                         ((png_uint_32)  69<<16) | \
                         ((png_uint_32)  88<< 8) | \
                         ((png_uint_32) 116    ))
#endif

#ifndef PNG_UINT_tIME
#  define PNG_UINT_tIME (((png_uint_32) 116<<24) | \
                         ((png_uint_32)  73<<16) | \
                         ((png_uint_32)  77<< 8) | \
                         ((png_uint_32)  69    ))
#endif

#ifndef PNG_UINT_tRNS
#  define PNG_UINT_tRNS (((png_uint_32) 116<<24) | \
                         ((png_uint_32)  82<<16) | \
                         ((png_uint_32)  78<< 8) | \
                         ((png_uint_32)  83    ))
#endif

#ifndef PNG_UINT_zTXt
#  define PNG_UINT_zTXt (((png_uint_32) 122<<24) | \
                         ((png_uint_32)  84<<16) | \
                         ((png_uint_32)  88<< 8) | \
                         ((png_uint_32) 116    ))
#endif

#endif /* !PNGCRUSH_H */
