/* contrib/powerpc-vsx/linux_aux.c
 *
 * Copyright (c) 2016 Glenn Randers-Pehrson
 * Written by Vadim Barkov, 2017.
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * SEE contrib/powerpc-vsx/README before reporting bugs
 *
 * STATUS: COMPILED
 * BUG REPORTS: png-mng-implement@sourceforge.net
 *
 * png_have_vsx implemented for Linux by reading the widely available
 * pseudo-file /proc/cpuinfo. Result is cached so if function will be called
 * multiple times only one reading is perfomed.
 *
 * This code is strict ANSI-C and is probably moderately portable; it does
 * however use <stdio.h> and it assumes that /proc/cpuinfo is never localized.
 */

#include "sys/auxv.h"
#include "png.h"

static int
png_have_vsx(png_structp png_ptr)
{
   const unsigned long auxv = getauxval( AT_HWCAP );
   if(auxv & (PPC_FEATURE_HAS_ALTIVEC|PPC_FEATURE_HAS_VSX ))
	return 1;
   else
	return 0;
}
