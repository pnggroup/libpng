/* contrib/riscv-vector/linux.c
 *
 * Copyright (c) 2023 Google LLC
 * Written by Dragoș Tiselice <dtiselice@google.com>, May 2023.
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
 * however use <stdio.h> and it assumes that /proc/cpuinfo is never localized.
 */

#include <stdio.h>
#include <string.h>

static int
png_have_vector(png_structp png_ptr) {
   FILE* f = fopen("/proc/cpuinfo", "rb");

   if (f == NULL) {
#ifdef PNG_WARNINGS_SUPPORTED
      png_warning(png_ptr, "/proc/cpuinfo open failed");
#endif
      return 0;
   }

   char line[256];

   while (fgets(line, sizeof line, f)) {
      if (strncmp(line, "isa", 3) != 0) {
         continue;
      }

      char* isa = strstr(line, "rv");

      if (isa == NULL) {
         continue;
      }

      if (strchr(isa + 2, 'v') != NULL) {
         return 1;
      }
   }

   fclose(f);

   return 0;
}
