/* loongarch_lsx_init.c - LSX optimized filter functions
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 * All rights reserved.
 * Contributed by Jin Bo <jinbo@loongson.cn>
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */
#ifdef PNG_READ_SUPPORTED
#ifdef __loongarch_sx
#define png_hardware_impl "loongarch-sx"

#include <sys/auxv.h>

#include "filter_lsx_intrinsics.c"

#define LA_HWCAP_LSX    (1<<4)
static int png_has_lsx(void)
{
    int flags = 0;
    int flag  = (int)getauxval(AT_HWCAP);

    if (flag & LA_HWCAP_LSX)
        return 1;

    return 0;
}

static void
png_init_filter_functions_lsx(png_structp pp, unsigned int bpp)
{
   if (png_has_lsx())
   {
      pp->read_filter[PNG_FILTER_VALUE_UP-1] = png_read_filter_row_up_lsx;
      if (bpp == 3)
      {
         pp->read_filter[PNG_FILTER_VALUE_SUB-1] = png_read_filter_row_sub3_lsx;
         pp->read_filter[PNG_FILTER_VALUE_AVG-1] = png_read_filter_row_avg3_lsx;
         pp->read_filter[PNG_FILTER_VALUE_PAETH-1] = png_read_filter_row_paeth3_lsx;
      }
      else if (bpp == 4)
      {
         pp->read_filter[PNG_FILTER_VALUE_SUB-1] = png_read_filter_row_sub4_lsx;
         pp->read_filter[PNG_FILTER_VALUE_AVG-1] = png_read_filter_row_avg4_lsx;
         pp->read_filter[PNG_FILTER_VALUE_PAETH-1] = png_read_filter_row_paeth4_lsx;
      }
   }
}

#define png_init_hardware_filter_functions png_init_filter_functions_lsx

#endif /* __loongarch_sx */
#endif /* READ */
