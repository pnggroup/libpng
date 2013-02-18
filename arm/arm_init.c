
/* arm_init.c - NEON optimised filter functions
 *
 * Copyright (c) 2013 Glenn Randers-Pehrson
 * Written by Mans Rullgard, 2011.
 * Last changed in libpng 1.5.15 [%RDATE%]
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */
/* Below, after checking __linux__, various non-C90 POSIX 1003.1 functions are
 * called.
 */
#define _POSIX_SOURCE 1

#include "../pngpriv.h"

/* __arm__ is defined by GCC, MSVC defines _M_ARM to the ARM version number,
 * Andoid intends to define __ANDROID__, however there are bugs in their
 * toolchain; use -D__ANDROID__ to work round this.
 */
#if defined(__linux__) && defined(__arm__)
#define CHECK_NEON
#include <signal.h> /* for sig_atomic_t */

#ifdef __ANDROID__
/* Linux provides access to information about CPU capabilites via
 * /proc/self/auxv, however Android blocks this while still claiming to be
 * Linux.  The Andoid NDK, however, provides appropriate support.
 *
 * Documentation: http://www.kandroid.org/ndk/docs/CPU-ARM-NEON.html
 */
#include <cpu-features.h>

static int
png_have_neon(png_structp png_ptr)
{
   /* This is a whole lot easier than the mess below, however it is probably
    * implemented as below, therefore it is better to cache the result (these
    * function calls may be slow!)
    */
   return andoid_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
      (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0;
}
#else
/* The generic __linux__ implementation requires reading /proc/self/auxv and
 * looking at each element for one that records NEON capabilities.
 */
#include <unistd.h> /* for POSIX 1003.1 */
#include <errno.h>  /* for EINTR */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <elf.h>
#include <asm/hwcap.h>

/* A read call may be interrupted, in which case it returns -1 and sets errno to
 * EINTR if nothing was done, otherwise (if something was done) a partial read
 * may result.
 */
static size_t
safe_read(png_structp png_ptr, int fd, void *buffer_in, size_t nbytes)
{
   size_t ntotal = 0;
   char *buffer = png_voidcast(char*, buffer_in);

   while (nbytes > 0)
   {
      unsigned int nread;
      int iread;

      /* Passing nread > INT_MAX to read is implementation defined in POSIX
       * 1003.1, therefore despite the unsigned argument portable code must
       * limit the value to INT_MAX!
       */
      if (nbytes > INT_MAX)
         nread = INT_MAX;

      else
         nread = (unsigned int)/*SAFE*/nbytes;

      iread = read(fd, buffer, nread);

      if (iread == -1)
      {
         /* This is the devil in the details, a read can terminate early with 0
          * bytes read because of EINTR, yet it still returns -1 otherwise end
          * of file cannot be distinguished.
          */
         if (errno != EINTR)
         {
            png_warning(png_ptr, "/proc read failed");
            return 0; /* I.e. a permanent failure */
         }
      }

      else if (iread < 0)
      {
         /* Not a valid 'read' result: */
         png_warning(png_ptr, "OS /proc read bug");
         return 0;
      }

      else if (iread > 0)
      {
         /* Continue reading until a permanent failure, or EOF */
         buffer += iread;
         nbytes -= (unsigned int)/*SAFE*/iread;
         ntotal += (unsigned int)/*SAFE*/iread;
      }

      else
         return ntotal;
   }

   return ntotal; /* nbytes == 0 */
}

static int
png_have_neon(png_structp png_ptr)
{
   int fd = open("/proc/self/auxv", O_RDONLY);
   Elf32_auxv_t aux;

   /* Failsafe: failure to open means no NEON */
   if (fd == -1)
   {
      png_warning(png_ptr, "/proc/self/auxv open failed");
      return 0;
   }

   while (safe_read(png_ptr, fd, &aux, sizeof aux) == sizeof aux)
   {
      if (aux.a_type == AT_HWCAP && (aux.a_un.a_val & HWCAP_NEON) != 0)
      {
         close(fd);
         return 1;
      }
   }

   close(fd);
   return 0;
}
#endif /* !__ANDROID__ */
#endif /* __linux__ && __arm__ */

void
png_init_filter_functions_neon(png_structp pp, unsigned int bpp)
{
#ifdef __arm__
#ifdef CHECK_NEON
   static volatile sig_atomic_t no_neon = -1; /* not checked */

   if (no_neon < 0)
      no_neon = !png_have_neon(pp);

   if (no_neon)
      return;
#endif

   /* IMPORTANT: any new external functions used here must be declared using
    * PNG_INTERNAL_FUNCTION in ../pngpriv.h.  This is required so that the
    * 'prefix' option to configure works:
    *
    *    ./configure --with-libpng-prefix=foobar_
    *
    * Verify you have got this right by running the above command, doing a build
    * and examining pngprefix.h; it must contain a #define for every external
    * function you add.  (Notice that this happens automatically for the
    * initialization function.)
    */
   pp->read_filter[PNG_FILTER_VALUE_UP-1] = png_read_filter_row_up_neon;

   if (bpp == 3)
   {
      pp->read_filter[PNG_FILTER_VALUE_SUB-1] = png_read_filter_row_sub3_neon;
      pp->read_filter[PNG_FILTER_VALUE_AVG-1] = png_read_filter_row_avg3_neon;
      pp->read_filter[PNG_FILTER_VALUE_PAETH-1] =
         png_read_filter_row_paeth3_neon;
   }

   else if (bpp == 4)
   {
      pp->read_filter[PNG_FILTER_VALUE_SUB-1] = png_read_filter_row_sub4_neon;
      pp->read_filter[PNG_FILTER_VALUE_AVG-1] = png_read_filter_row_avg4_neon;
      pp->read_filter[PNG_FILTER_VALUE_PAETH-1] =
          png_read_filter_row_paeth4_neon;
   }
#else
   PNG_UNUSED(pp)
   PNG_UNUSED(bpp)
#endif
}
