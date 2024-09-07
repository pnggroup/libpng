
/* intel_init.c - SSE2 optimized filter functions
 *
 * Copyright (c) 2018 Cosmin Truta
 * Copyright (c) 2016-2017 Glenn Randers-Pehrson
 * Written by Mike Klein and Matt Sarett, Google, Inc.
 * Derived from arm/arm_init.c
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */
#if defined(__SSE4_1__) || defined(__AVX__)
   /* We are not actually using AVX, but checking for AVX is the best way we can
    * detect SSE4.1 and SSSE3 on MSVC.
    */
#  define PNG_INTEL_SSE_IMPLEMENTATION 3
#elif defined(__SSSE3__)
#  define PNG_INTEL_SSE_IMPLEMENTATION 2
#elif defined(__SSE2__) || defined(_M_X64) || defined(_M_AMD64) ||\
      (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#  define PNG_INTEL_SSE_IMPLEMENTATION 1
#else
#  define PNG_INTEL_SSE_IMPLEMENTATION 0
#endif

#if PNG_INTEL_SSE_IMPLEMENTATION > 0 && defined(PNG_READ_SUPPORTED)
#  define png_hardware_impl "intel-sse"

#include "filter_sse2_intrinsics.c"

static void
png_init_filter_functions_sse2(png_structp pp, unsigned int bpp)
{
   /* The techniques used to implement each of these filters in SSE operate on
    * one pixel at a time.
    * So they generally speed up 3bpp images about 3x, 4bpp images about 4x.
    * They can scale up to 6 and 8 bpp images and down to 2 bpp images,
    * but they'd not likely have any benefit for 1bpp images.
    * Most of these can be implemented using only MMX and 64-bit registers,
    * but they end up a bit slower than using the equally-ubiquitous SSE2.
   */
   png_debug(1, "in png_init_filter_functions_sse2");
   if (bpp == 3)
   {
      pp->read_filter[PNG_FILTER_VALUE_SUB-1] = png_read_filter_row_sub3_sse2;
      pp->read_filter[PNG_FILTER_VALUE_AVG-1] = png_read_filter_row_avg3_sse2;
      pp->read_filter[PNG_FILTER_VALUE_PAETH-1] =
         png_read_filter_row_paeth3_sse2;
   }
   else if (bpp == 4)
   {
      pp->read_filter[PNG_FILTER_VALUE_SUB-1] = png_read_filter_row_sub4_sse2;
      pp->read_filter[PNG_FILTER_VALUE_AVG-1] = png_read_filter_row_avg4_sse2;
      pp->read_filter[PNG_FILTER_VALUE_PAETH-1] =
          png_read_filter_row_paeth4_sse2;
   }

   /* No need optimize PNG_FILTER_VALUE_UP.  The compiler should
    * autovectorize.
    */
}

#define png_hardware_init_filter_functions_impl png_init_filter_functions_sse2

#endif /* PNG_INTEL_SSE_IMPLEMENTATION > 0 */
