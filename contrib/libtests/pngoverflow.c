/* pngoverflow.c
 *
 * Copyright (c) 2026 Mohammad Seet
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * Test the overflow-checking primitives: png_mul_size,
 * png_rowbytes_checked, and the updated PNG_ROWBYTES macro.
 */

/* pngpriv.h must be included before any system header or png.h */
#include "../../pngpriv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int errors = 0;

static void
check_mul_size(size_t a, size_t b, size_t expected, int expect_zero,
    const char *desc)
{
   size_t result = png_mul_size(a, b);

   if (expect_zero)
   {
      if (result != 0)
      {
         fprintf(stderr, "FAIL: %s: png_mul_size(%lu, %lu) = %lu,"
             " expected 0 (overflow)\n",
             desc, (unsigned long)a, (unsigned long)b,
             (unsigned long)result);
         errors++;
      }
   }

   else
   {
      if (result != expected)
      {
         fprintf(stderr, "FAIL: %s: png_mul_size(%lu, %lu) = %lu,"
             " expected %lu\n",
             desc, (unsigned long)a, (unsigned long)b,
             (unsigned long)result, (unsigned long)expected);
         errors++;
      }
   }
}

static void
check_rowbytes(unsigned int pixel_depth, png_uint_32 width,
    size_t expected, const char *desc)
{
   size_t result = PNG_ROWBYTES(pixel_depth, width);

   if (result != expected)
   {
      fprintf(stderr, "FAIL: %s: PNG_ROWBYTES(%u, %lu) = %lu,"
          " expected %lu\n",
          desc, pixel_depth, (unsigned long)width,
          (unsigned long)result, (unsigned long)expected);
      errors++;
   }
}

int
main(void)
{
   printf("Testing overflow-checking primitives...\n");

   /* png_mul_size: normal cases */
   check_mul_size(0, 0, 0, 0, "0*0");
   check_mul_size(1, 0, 0, 0, "1*0");
   check_mul_size(0, 1, 0, 0, "0*1");
   check_mul_size(1, 1, 1, 0, "1*1");
   check_mul_size(100, 200, 20000, 0, "100*200");
   check_mul_size(1000000, 8, 8000000, 0, "1M*8");

   /* png_mul_size: overflow cases */
   check_mul_size(PNG_SIZE_MAX, 2, 0, 1, "SIZE_MAX*2");
   check_mul_size(2, PNG_SIZE_MAX, 0, 1, "2*SIZE_MAX");
   check_mul_size(PNG_SIZE_MAX / 2 + 1, 2, 0, 1, "SIZE_MAX/2+1 * 2");

   /* png_mul_size: boundary */
   check_mul_size(PNG_SIZE_MAX / 2, 2, (PNG_SIZE_MAX / 2) * 2, 0,
       "SIZE_MAX/2 * 2");
   check_mul_size(PNG_SIZE_MAX / 8, 8, (PNG_SIZE_MAX / 8) * 8, 0,
       "SIZE_MAX/8 * 8");

   /* PNG_ROWBYTES: normal cases (>= 8 bits per pixel) */
   check_rowbytes(8, 100, 100, "8bpp 100w");
   check_rowbytes(16, 100, 200, "16bpp 100w");
   check_rowbytes(24, 100, 300, "24bpp 100w");
   check_rowbytes(32, 100, 400, "32bpp 100w");
   check_rowbytes(64, 100, 800, "64bpp 100w");

   /* PNG_ROWBYTES: sub-byte pixels */
   check_rowbytes(1, 1, 1, "1bpp 1w");
   check_rowbytes(1, 8, 1, "1bpp 8w");
   check_rowbytes(1, 9, 2, "1bpp 9w");
   check_rowbytes(2, 4, 1, "2bpp 4w");
   check_rowbytes(4, 2, 1, "4bpp 2w");
   check_rowbytes(4, 3, 2, "4bpp 3w");

   /* PNG_ROWBYTES: large widths that fit on this platform.  On 64-bit
    * systems png_uint_32 * 8 always fits in size_t; on 32-bit systems
    * the IHDR check limits width so that width * 8 <= SIZE_MAX.
    */
   check_rowbytes(32, 1000000, 4000000, "32bpp 1Mpx");
   check_rowbytes(64, 1000000, 8000000, "64bpp 1Mpx");

   /* Verify png_mul_size overflow detection with size_t-range values.
    * These cannot be tested through PNG_ROWBYTES because png_uint_32
    * width limits how large the inputs can be.
    */
   check_mul_size(PNG_SIZE_MAX / 4 + 1, 8, 0, 1,
       "SIZE_MAX/4+1 * 8 overflow");

   /* PNG_ROWBYTES >= 8 path at large png_uint_32 widths.  These
    * exercise the updated macro's png_mul_size integration at scale.
    *
    * Use a runtime sizeof check instead of a preprocessor #if on
    * SIZE_MAX, because SIZE_MAX is a constant expression, not a
    * preprocessor integer constant, in many C89 environments.
    */
   if (sizeof(size_t) > 4)
   {
      /* 64-bit size_t: png_uint_32 * 8 always fits. */
      check_rowbytes(8, (png_uint_32)0xFFFFFFFEUL,
          (size_t)0xFFFFFFFEUL, "8bpp max-1 width 64bit");
      check_rowbytes(24, (png_uint_32)0xFFFFFFFEUL,
          (size_t)0xFFFFFFFEUL * 3, "24bpp max-1 width 64bit");
      check_rowbytes(32, (png_uint_32)0xFFFFFFFEUL,
          (size_t)0xFFFFFFFEUL * 4, "32bpp max-1 width 64bit");
      check_rowbytes(64, (png_uint_32)0xFFFFFFFEUL,
          (size_t)0xFFFFFFFEUL * 8, "64bpp max-1 width 64bit");
   }

   else
   {
      /* 32-bit size_t: 8 bpp still fits, higher bpp overflow to 0. */
      check_rowbytes(8, (png_uint_32)0xFFFFFFFEUL,
          (size_t)0xFFFFFFFEUL, "8bpp max-1 width 32bit");
      check_rowbytes(32, (png_uint_32)0xFFFFFFFEUL,
          0, "32bpp max-1 width 32bit overflow");
      check_rowbytes(64, (png_uint_32)0xFFFFFFFEUL,
          0, "64bpp max-1 width 32bit overflow");
   }

   /* NOTE: png_rowbytes_checked (the function that calls png_error on
    * overflow) is PNG_INTERNAL and not exported by the shared library,
    * so it cannot be tested from an external test binary.  On 64-bit
    * targets, overflow through png_rowbytes_checked is also unreachable
    * because png_uint_32 * 8 always fits in a 64-bit size_t.
    */

   if (errors == 0)
   {
      printf("PASS: all overflow checks passed\n");
      return 0;
   }

   fprintf(stderr, "FAIL: %d overflow check(s) failed\n", errors);
   return 1;
}
