
/* contrib/riscv-vector/linux.c
 *
 * Copyright (c) 2021 Manfred Schlaegl
 * Copyright (c) 2020 Cosmin Truta
 * Copyright (c) 2016 Glenn Randers-Pehrson
 * Written by Manfred Schlaegl, October 2021.
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * SEE contrib/riscv-vector/README before reporting bugs
 *
 * STATUS: SUPPORTED
 * BUG REPORTS: png-mng-implement@sourceforge.net
 *
 * png_have_vector implemented for Linux by reading the widely available
 * pseudo-file /proc/cpuinfo.
 *
 * This code is strict ANSI-C and is probably moderately portable; it does
 * however use <stdio.h>, <string.h> and it assumes that /proc/cpuinfo is
 * never localized.
 */

#include <stdio.h>
#include <string.h>

#define MAX_LINE_SIZE 256

static int
png_have_vector(png_structp png_ptr)
{
   int ret = 0;

   FILE *f = fopen("/proc/cpuinfo", "rb");
   if (f == NULL) {
#ifdef PNG_WARNINGS_SUPPORTED
      png_warning(png_ptr, "/proc/cpuinfo open failed");
#endif
      return 0;
   }

   while(!feof(f))
   {
      char line[MAX_LINE_SIZE];

      /* read line */
      int i = 0;
      while (i < (MAX_LINE_SIZE - 1))
      {
         char ch = fgetc(f);
         if (ch == '\n' || ch == EOF)
            break;
         line[i++] = ch;
      }
      line[i] = '\0';

      /* does line start with "isa"? */
      if (strncmp("isa", line, 3) != 0)
         continue;

      /* find value starting with "rv" */
      char *val = strstr(line, "rv");
      if (val == NULL)
         continue;

      /* skip "rv" */
      val += 2;

      /* check for vector 'v' */
      val = strchr(line, 'v');
      if (val != NULL)
      {
         /* found */
         ret = 1;
         break;
      }
   }

   fclose(f);
   return ret;
}
