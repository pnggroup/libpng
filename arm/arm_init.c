
/* arm_init.c - NEON optimised filter functions
 *
 * Copyright (c) 2018-2022 Cosmin Truta
 * Copyright (c) 2014,2016 Glenn Randers-Pehrson
 * Written by Mans Rullgard, 2011.
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */
#if (defined(__ARM_NEON__) || defined(__ARM_NEON)) &&\
   defined(PNG_ALIGNED_MEMORY_SUPPORTED) && defined(PNG_READ_SUPPORTED)

#define png_hardware_impl "arm-neon"

#if defined(_MSC_VER) && !defined(__clang__) && defined(_M_ARM64)
#  include <arm64_neon.h>
#else
#  include <arm_neon.h>
#endif

/* Obtain the definitions of the actual filter functions: */
#include "filter_neon_intrinsics.c"

static void
png_init_filter_functions_neon(png_structp pp, unsigned int bpp)
{
   png_debug(1, "in png_init_filter_functions_neon");

   /* IMPORTANT: DO NOT DEFINE EXTERNAL FUNCTIONS HERE
    *
    * This is because external functions must be declared with
    * PNG_INTERNAL_FUNCTION in pngpriv.h; without this the PNG_PREFIX option to
    * the build will not work (it will not know about these symbols).
    */
   pp->read_filter[PNG_FILTER_VALUE_UP-1] = png_read_filter_row_up_neon;

   if (bpp == 3)
   {
      pp->read_filter[PNG_FILTER_VALUE_SUB-1] = png_read_filter_row_sub3_neon;
      pp->read_filter[PNG_FILTER_VALUE_AVG-1] = png_read_filter_row_avg3_neon;
      pp->read_filter[PNG_FILTER_VALUE_PAETH-1] =
         png_read_filter_row_paeth3_neon;
   }

   else if (bpp == 4)
   {
      pp->read_filter[PNG_FILTER_VALUE_SUB-1] = png_read_filter_row_sub4_neon;
      pp->read_filter[PNG_FILTER_VALUE_AVG-1] = png_read_filter_row_avg4_neon;
      pp->read_filter[PNG_FILTER_VALUE_PAETH-1] =
          png_read_filter_row_paeth4_neon;
   }
}

#define png_hardware_init_filter_functions_impl png_init_filter_functions_neon

#ifndef PNG_WIP_DISABLE_PALETTE /*TODO*/
#include "palette_neon_intrinsics.c"
#endif /*TODO*/

/* TODO:
 *    png_hardware_free_data_impl
 *       Must be defined if the implementation stores data in
 *       png_struct::hardware_data.  Need not be defined otherwise.
 *
 *    png_hardware_init_palette_support_impl
 *       Contains code to initialize a palette transformation.  This returns
 *       true if something has been set up.  Only called if the state contains
 *       png_hardware_palette, need not be defined, may cancel the state flag
 *       in the png_struct to prevent further calls.
 *
 *    png_hardware_do_expand_palette
 *       Handles palette expansion.  Need not be defined, only called if the
 *       state contains png_hardware_palette, may set this flag to zero, may
 *       return false to indicate that the expansion was not done.
 */

#endif /* READ, ALIGNEDMEMORY && ARM_NEON */
