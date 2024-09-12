/* check.h - LSX optimized filter functions
 * copied from loongarch_lsx_init.c
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 * All rights reserved.
 * Contributed by Jin Bo <jinbo@loongson.cn>
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * Modified 2024 by John Bowler, changes
 * Copyright (c) 2024 John Bowler, licensed under the libpng license:
 */
#if defined(__loongarch_sx) && defined(__GLIBC__)
   /* The code uses the GNU glibc specific function getauxval so this is
    * required:
    */
#  if __GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 16)
#     define PNG_MIPS_LSX_IMPLEMENTATION 1
#  else
#     define PNG_MIPS_LSX_IMPLEMENTATION 0
#  endif /* glibc >= 2.16 */
#else
#  define PNG_MIPS_LSX_IMPLEMENTATION 0
#endif /* __loongarch_sx */
