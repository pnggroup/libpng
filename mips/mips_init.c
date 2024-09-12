/* mips_init.c - MSA optimised filter functions
 *
 * Copyright (c) 2018-2024 Cosmin Truta
 * Copyright (c) 2016 Glenn Randers-Pehrson
 * Written by Mandar Sahastrabuddhe, 2016
 * Updated by guxiwei, 2023
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * Modified 2024 by John Bowler, changes
 * Copyright (c) 2024 John Bowler, licensed under the libpng license
 */
/* MIPS supports three optimizations: MSA, MSI and LSX (Loongarch SX). When two
 * or more are available the appropriate optimization is chosen at runtime using
 * the png_set_option settings and/or runtime checks.
 */
#if PNG_MIPS_MSA_IMPLEMENATION == 1
#  include "filter_msa_intrinsics.c"
#endif
#if PNG_MIPS_MMI_IMPLEMENTATION > 0
#  include "filter_mmi_inline_assembly.c"
#endif
#if PNG_MIPS_LSX_IMPLEMENTATION > 0
#  include "loongarch/loongarch_lsx_init.c"
#endif

static void
png_init_filter_functions_mips(png_structp pp, unsigned int bpp)
{
#  if PNG_MIPS_LSX_IMPLEMENTATION > 0
#     define png_target_impl_lsx "+lsx"

      /* TODO: put in an option check. */
      if (png_init_filter_functions_lsx(pp, bpp))
         return;
      /* Else fall through to see if something else is available: */
#  else
#     define png_target_impl_lsx ""
#  endif

#  if PNG_MIPS_MMI_IMPLEMENTATION > 0
      /* Check the option if MSA is also supported: */
#     define png_target_impl_mmi "+mmi"
#     if PNG_MIPS_MSA_IMPLEMENATION == 1
         /* NOTE: if this is false the code below will not be executed. */
         if (((pp->options >> PNG_MIPS_USE_MMI) & 3) == PNG_OPTION_ON)
#     endif
      {
         /* This is the MMI implementation: */
         pp->read_filter[PNG_FILTER_VALUE_UP-1] = png_read_filter_row_up_mmi;
         if (bpp == 3)
         {
            pp->read_filter[PNG_FILTER_VALUE_SUB-1] =
               png_read_filter_row_sub3_mmi;
            pp->read_filter[PNG_FILTER_VALUE_AVG-1] =
               png_read_filter_row_avg3_mmi;
            pp->read_filter[PNG_FILTER_VALUE_PAETH-1] =
               png_read_filter_row_paeth3_mmi;
         }
         else if (bpp == 4)
         {
            pp->read_filter[PNG_FILTER_VALUE_SUB-1] =
               png_read_filter_row_sub4_mmi;
            pp->read_filter[PNG_FILTER_VALUE_AVG-1] =
               png_read_filter_row_avg4_mmi;
            pp->read_filter[PNG_FILTER_VALUE_PAETH-1] =
                png_read_filter_row_paeth4_mmi;
         }
         return;
      }
#  else /* PNG_MIPS_MMI_IMPLEMENTATION == 0 */
#     define png_target_impl_mmi ""
#  endif

#  if PNG_MIPS_MSA_IMPLEMENATION == 1
#     define png_target_impl_msa "+msa"
      pp->read_filter[PNG_FILTER_VALUE_UP-1] = png_read_filter_row_up_msa;

      if (bpp == 3)
      {
         pp->read_filter[PNG_FILTER_VALUE_SUB-1] = png_read_filter_row_sub3_msa;
         pp->read_filter[PNG_FILTER_VALUE_AVG-1] = png_read_filter_row_avg3_msa;
         pp->read_filter[PNG_FILTER_VALUE_PAETH-1] =
            png_read_filter_row_paeth3_msa;
      }

      else if (bpp == 4)
      {
         pp->read_filter[PNG_FILTER_VALUE_SUB-1] = png_read_filter_row_sub4_msa;
         pp->read_filter[PNG_FILTER_VALUE_AVG-1] = png_read_filter_row_avg4_msa;
         pp->read_filter[PNG_FILTER_VALUE_PAETH-1] =
            png_read_filter_row_paeth4_msa;
      }
#  endif /* PNG_MIPS_MSA_IMPLEMENTATION == 1 */
}

#if defined(png_target_impl_msa) || defined(png_target_impl_msi) ||\
    defined(png_target_impl_lsx)
#  define png_target_impl "mips"\
      png_target_impl_msa png_target_impl_mmi png_target_impl_lsx
#  define png_target_init_filter_functions_impl png_init_filter_functions_mips
#else
#  error HARDWARE: MIPS: no implementations defined
#endif
