/* contrib/powerpc-vsx/linux.c
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "png.h"

#ifndef MAXLINE
#  define MAXLINE 1024
#endif

static int
png_have_vsx(png_structp png_ptr)
{
   FILE *f;

   const char *string = "altivec supported";
   char input[MAXLINE];
   char *token = NULL;

   f = fopen("/proc/cpuinfo", "r");
   if (f != NULL)
   {
      memset(input,0,MAXLINE);
      while(fgets(input,MAXLINE,f) != NULL)
      {
         token = strstr(input,string);
	 if(token != NULL)
	    return cachedResult;
      }
   }
#ifdef PNG_WARNINGS_SUPPORTED
   else
      png_warning(png_ptr, "/proc/cpuinfo open failed");
#endif
   return cachedResult;
}