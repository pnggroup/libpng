/* loongson_mmi_init.c - MMI optimized filter functions
 *
 * Copyright (c) 2019 Loongson Technology Co. Ltd.
 * Written by Zhang Lixia, Yin Shiyou
 * Derived from arm/arm_init.c
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */

#include "../pngpriv.h"

#ifdef PNG_READ_SUPPORTED
#if PNG_LOONGSON_MMI_IMPLEMENTATION > 0

void
png_init_filter_functions_mmi(png_structp pp, unsigned int bpp)
{
   /* The techniques used to implement each of these filters in MMI operate on
    * one pixel at a time.
    * So they generally speed up 3bpp images about 3x, 4bpp images about 4x.
    * They can scale up to 6 and 8 bpp images and down to 2 bpp images,
    * but they'd not likely have any benefit for 1bpp images.
    * Most of these can be implemented using only MMI and 64-bit registers.
    */
   png_debug(1, "in png_init_filter_functions_mmi");
   pp->read_filter[PNG_FILTER_VALUE_UP-1] = png_read_filter_row_up_mmi;
   if (bpp == 3)
   {
      pp->read_filter[PNG_FILTER_VALUE_SUB-1] = png_read_filter_row_sub3_mmi;
      pp->read_filter[PNG_FILTER_VALUE_AVG-1] = png_read_filter_row_avg3_mmi;
      pp->read_filter[PNG_FILTER_VALUE_PAETH-1] =
          png_read_filter_row_paeth3_mmi;
   }
   else if (bpp == 4)
   {
      pp->read_filter[PNG_FILTER_VALUE_SUB-1] = png_read_filter_row_sub4_mmi;
      pp->read_filter[PNG_FILTER_VALUE_AVG-1] = png_read_filter_row_avg4_mmi;
      pp->read_filter[PNG_FILTER_VALUE_PAETH-1] =
          png_read_filter_row_paeth4_mmi;
   }
   else if (bpp == 6)
   {
      pp->read_filter[PNG_FILTER_VALUE_SUB-1] = png_read_filter_row_sub6_mmi;
   }
   else if (bpp == 8)
   {
      pp->read_filter[PNG_FILTER_VALUE_SUB-1] = png_read_filter_row_sub8_mmi;
   }
   /* No need optimize PNG_FILTER_VALUE_UP.  The compiler should
    * autovectorize.
    */
}

#endif /* PNG_LOONGSON_MMI_IMPLEMENTATION > 0 */
#endif /* PNG_READ_SUPPORTED */
