/* mips/check.h - MSA optimised filter functions
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

#include "loongarch/check.h"

#if PNG_MIPS_MSA_IMPLEMENTATION == 1 ||\
    PNG_MIPS_MMI_IMPLEMENTATION > 0 ||\
    PNG_MIPS_LSX_IMPLEMENTATION > 0
#  define PNG_TARGET_CODE_IMPLEMENTATION "mips/mips_init.c"
   /*PNG_TARGET_STORES_DATA*/
#  define PNG_TARGET_IMPLEMENTS_FILTERS
   /*PNG_TARGET_IMPLEMENTS_EXPAND_PALETTE*/
#  define PNG_TARGET_ROW_ALIGNMENT 16
#endif /* MIPS MSA or Loongson MMI or Loongarch SX */
