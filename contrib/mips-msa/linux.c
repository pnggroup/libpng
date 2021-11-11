
/* contrib/mips-msa/linux.c
 *
 * Copyright (c) 2020 Cosmin Truta
 * Copyright (c) 2016 Glenn Randers-Pehrson
 * Written by Mandar Sahastrabuddhe, 2016.
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * SEE contrib/mips-msa/README before reporting bugs
 *
 * STATUS: SUPPORTED
 * BUG REPORTS: png-mng-implement@sourceforge.net
 *
 * png_have_msa implemented for Linux by reading the widely available
 * pseudo-file /proc/cpuinfo.
 *
 * This code is strict ANSI-C and is probably moderately portable; it does
 * however use <stdio.h> and it assumes that /proc/cpuinfo is never localized.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>

static int
png_have_msa(png_structp png_ptr)
{
    Elf64_auxv_t aux;
    int fd;
    int has_msa = 0;

    fd = open("/proc/self/auxv", O_RDONLY);
    if (fd >= 0) {
       while (read(fd, &aux, sizeof(Elf64_auxv_t)) == sizeof(Elf64_auxv_t)) {
          if (aux.a_type == AT_HWCAP) {
             uint64_t hwcap = aux.a_un.a_val;

             has_msa = (hwcap >> 1) & 1;
             break;
          }
       }
       close (fd);
    }
#ifdef PNG_WARNINGS_SUPPORTED
    else
      png_warning(png_ptr, "open /proc/cpuinfo failed");
#endif

    return has_msa;
}
