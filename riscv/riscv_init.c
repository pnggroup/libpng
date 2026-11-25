/* riscv_init.c - RISC-V Vector optimized filter functions
 *
 * Copyright (c) 2023 Google LLC
 * Written by Dragoș Tiselice <dtiselice@google.com>, May 2023.
 *            Filip Wasil     <f.wasil@samsung.com>, March 2025.
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */
#define png_target_impl "riscv-rvv"

#include <riscv_vector.h>
#include "filter_rvv_intrinsics.c"

static void
png_init_filter_functions_rvv(png_struct *pp, unsigned int bpp)
{
   png_debug(1, "in png_init_filter_functions_rvv");

   /* IMPORTANT: DO NOT DEFINE EXTERNAL FUNCTIONS HERE
    *
    * This is because external functions must be declared with
    * PNG_INTERNAL_FUNCTION in pngpriv.h; without this the PNG_PREFIX option to
    * the build will not work (it will not know about these symbols).
    */
   pp->read_filter[PNG_FILTER_VALUE_UP-1] = png_read_filter_row_up_rvv;

   if (bpp == 3)
   {
      pp->read_filter[PNG_FILTER_VALUE_AVG-1] = png_read_filter_row_avg3_rvv;
      pp->read_filter[PNG_FILTER_VALUE_PAETH-1] = png_read_filter_row_paeth3_rvv;
      pp->read_filter[PNG_FILTER_VALUE_SUB-1] = png_read_filter_row_sub3_rvv;
   }
   else if (bpp == 4)
   {
      pp->read_filter[PNG_FILTER_VALUE_AVG-1] = png_read_filter_row_avg4_rvv;
      pp->read_filter[PNG_FILTER_VALUE_PAETH-1] = png_read_filter_row_paeth4_rvv;
      pp->read_filter[PNG_FILTER_VALUE_SUB-1] = png_read_filter_row_sub4_rvv;
   }
}

#define png_target_init_filter_functions_impl png_init_filter_functions_rvv
