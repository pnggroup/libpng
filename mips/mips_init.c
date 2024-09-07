
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
 */
#if defined(PNG_READ_SUPPORTED) && defined(PNG_ALIGNED_MEMORY_SUPPORTED)

#if defined(__mips_msa) && (__mips_isa_rev >= 5)
#  ifndef PNG_MIPS_MSA_IMPLEMENTATION
#     if defined(__mips_msa)
#        if defined(__clang__)
#        elif defined(__GNUC__)
#           if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 7)
#              define PNG_MIPS_MSA_IMPLEMENTATION 2
#           endif /* no GNUC support */
#        endif /* __GNUC__ */
#     else /* !defined __mips_msa */
#        define PNG_MIPS_MSA_IMPLEMENTATION 2
#     endif /* __mips_msa */
#  endif /* !PNG_MIPS_MSA_IMPLEMENTATION */

#  ifndef PNG_MIPS_MSA_IMPLEMENTATION
#     define PNG_MIPS_MSA_IMPLEMENTATION 1
#  endif
#else
#  define PNG_MIPS_MSA_IMPLEMENTATION 0
#endif /* __mips_msa && __mips_isa_rev >= 5 */

#if defined(__mips_loongson_mmi) && (_MIPS_SIM == _ABI64)
#  ifndef PNG_MIPS_MMI_IMPLEMENTATION
#     if defined(__mips_loongson_mmi) && (_MIPS_SIM == _ABI64)
#        define PNG_MIPS_MMI_IMPLEMENTATION 2
#     else /* !defined __mips_loongson_mmi  || _MIPS_SIM != _ABI64 */
#        define PNG_MIPS_MMI_IMPLEMENTATION 0
#     endif /* __mips_loongson_mmi  && _MIPS_SIM == _ABI64 */
#  endif /* !PNG_MIPS_MMI_IMPLEMENTATION */
#else
#   define PNG_MIPS_MMI_IMPLEMENTATION 0
#endif /* __mips_loongson_mmi && _MIPS_SIM == _ABI64 */

#if PNG_MIPS_MSA_IMPLEMENTATION == 1 || PNG_MIPS_MMI_IMPLEMENTATION > 0
/* MIPS supports two optimizations: MMI and MSA. When both are available the
 * appropriate optimization is chosen at runtime using the png_set_option
 * settings.
 *
 * NOTE: see also the separate loongson code...
 */
#if PNG_MIPS_MSA_IMPLEMENATION == 1
#  include "filter_msa_intrinsics.c"
#endif
#if PNG_MIPS_MMI_IMPLEMENTATION > 0
#  include "filter_mmi_inline_assembly.c"
#endif

static void
png_init_filter_functions_mips(png_structp pp, unsigned int bpp)
{
#  if PNG_MIPS_MMI_IMPLEMENTATION  > 0
      /* Check the option if MSA is also supported: */
#     if PNG_MIPS_MSA_IMPLEMENATION == 1
#        define png_hardware_impl "mips-msa+msi"
         /* NOTE: if this is false the code below will not be executed. */
         if (((pp->options >> PNG_MIPS_USE_MMI) & 3) == PNG_OPTION_ON)
#     else
#        define png_hardware_impl "mips-mmi"
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
#  else /* !(PNG_MIPS_MMI_IMPLEMENTATION > 0) */
#     define png_hardware_impl "mips-msa"
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

#define png_init_filter_functions_impl png_init_filter_functions_mips

#endif /* PNG_MIPS_MSA_IMPLEMENTATION == 1 || PNG_MIPS_MMI_IMPLEMENTATION > 0 */
#endif /* READ && ALIGNED_MEMORY */
