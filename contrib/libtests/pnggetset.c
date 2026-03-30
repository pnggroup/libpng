/* pnggetset.c
 *
 * Copyright (c) 2026 Cosmin Truta
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * Test the get-then-set roundtrip pattern for PLTE, tRNS, and hIST.
 *
 * Passing the internal pointer returned by a getter back into the
 * corresponding setter is a natural API usage pattern.  A previous
 * version had a use-after-free on this path because the setter freed
 * the internal buffer before copying from the caller-supplied pointer.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(HAVE_CONFIG_H) && !defined(PNG_NO_CONFIG_H)
#  include <config.h>
#endif

#ifdef PNG_FREESTANDING_TESTS
#  include <png.h>
#else
#  include "../../png.h"
#endif

/* Test: get the PLTE, pass it straight back to set, verify roundtrip. */
static int
test_plte_roundtrip(void)
{
   png_structp png_ptr;
   png_infop info_ptr;
   png_color palette[4];
   png_colorp got_palette = NULL;
   int num_palette = 0;
   int i;

   png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
       NULL, NULL, NULL);
   if (png_ptr == NULL)
   {
      fprintf(stderr, "pnggetset: png_create_write_struct failed\n");
      return 1;
   }

   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      fprintf(stderr, "pnggetset: png_create_info_struct failed\n");
      png_destroy_write_struct(&png_ptr, NULL);
      return 1;
   }

   if (setjmp(png_jmpbuf(png_ptr)))
   {
      fprintf(stderr, "pnggetset: libpng error in test_plte_roundtrip\n");
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return 1;
   }

   /* Set up a palette-color image header. */
   png_set_IHDR(png_ptr, info_ptr, 1, 1, 8, PNG_COLOR_TYPE_PALETTE,
       PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

   /* Populate with recognizable values. */
   for (i = 0; i < 4; i++)
   {
      palette[i].red   = (png_byte)(i * 10);
      palette[i].green = (png_byte)(i * 20);
      palette[i].blue  = (png_byte)(i * 30);
   }
   png_set_PLTE(png_ptr, info_ptr, palette, 4);

   /* Get the internal pointer and feed it straight back. */
   png_get_PLTE(png_ptr, info_ptr, &got_palette, &num_palette);
   if (got_palette == NULL || num_palette != 4)
   {
      fprintf(stderr, "pnggetset: png_get_PLTE returned unexpected values\n");
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return 1;
   }

   /* This is the critical call: the pointer aliases info_ptr->palette. */
   png_set_PLTE(png_ptr, info_ptr, got_palette, num_palette);

   /* Verify the data survived the roundtrip. */
   got_palette = NULL;
   num_palette = 0;
   png_get_PLTE(png_ptr, info_ptr, &got_palette, &num_palette);
   if (got_palette == NULL || num_palette != 4)
   {
      fprintf(stderr, "pnggetset: PLTE lost after roundtrip\n");
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return 1;
   }
   for (i = 0; i < 4; i++)
   {
      if (got_palette[i].red   != (png_byte)(i * 10) ||
          got_palette[i].green != (png_byte)(i * 20) ||
          got_palette[i].blue  != (png_byte)(i * 30))
      {
         fprintf(stderr,
             "pnggetset: PLTE entry %d corrupted after roundtrip\n", i);
         png_destroy_write_struct(&png_ptr, &info_ptr);
         return 1;
      }
   }

   png_destroy_write_struct(&png_ptr, &info_ptr);
   return 0;
}

#ifdef PNG_hIST_SUPPORTED
/* Test: get the hIST, pass it straight back to set, verify roundtrip. */
static int
test_hist_roundtrip(void)
{
   png_structp png_ptr;
   png_infop info_ptr;
   png_color palette[4];
   png_uint_16 hist[4];
   png_uint_16p got_hist = NULL;
   int i;

   png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
       NULL, NULL, NULL);
   if (png_ptr == NULL)
   {
      fprintf(stderr, "pnggetset: png_create_write_struct failed\n");
      return 1;
   }

   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      fprintf(stderr, "pnggetset: png_create_info_struct failed\n");
      png_destroy_write_struct(&png_ptr, NULL);
      return 1;
   }

   if (setjmp(png_jmpbuf(png_ptr)))
   {
      fprintf(stderr, "pnggetset: libpng error in test_hist_roundtrip\n");
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return 1;
   }

   /* Set up a palette-color image header. */
   memset(palette, 0, sizeof palette);
   png_set_IHDR(png_ptr, info_ptr, 1, 1, 8, PNG_COLOR_TYPE_PALETTE,
       PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
   png_set_PLTE(png_ptr, info_ptr, palette, 4);

   /* Populate with recognizable values. */
   for (i = 0; i < 4; i++)
      hist[i] = (png_uint_16)(i * 100 + 42);

   png_set_hIST(png_ptr, info_ptr, hist);

   /* Get the internal pointer and feed it straight back. */
   if (png_get_hIST(png_ptr, info_ptr, &got_hist) == 0 || got_hist == NULL)
   {
      fprintf(stderr, "pnggetset: png_get_hIST returned unexpected values\n");
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return 1;
   }

   /* This is the critical call: the pointer aliases info_ptr->hist. */
   png_set_hIST(png_ptr, info_ptr, got_hist);

   /* Verify the data survived the roundtrip. */
   got_hist = NULL;
   if (png_get_hIST(png_ptr, info_ptr, &got_hist) == 0 || got_hist == NULL)
   {
      fprintf(stderr, "pnggetset: hIST lost after roundtrip\n");
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return 1;
   }
   for (i = 0; i < 4; i++)
   {
      if (got_hist[i] != (png_uint_16)(i * 100 + 42))
      {
         fprintf(stderr,
             "pnggetset: hIST entry %d corrupted after roundtrip\n", i);
         png_destroy_write_struct(&png_ptr, &info_ptr);
         return 1;
      }
   }

   png_destroy_write_struct(&png_ptr, &info_ptr);
   return 0;
}
#endif /* PNG_hIST_SUPPORTED */

#ifdef PNG_tRNS_SUPPORTED
/* Test: get the tRNS, pass it straight back to set, verify roundtrip. */
static int
test_trns_roundtrip(void)
{
   png_structp png_ptr;
   png_infop info_ptr;
   png_color palette[4];
   png_byte trans_alpha[4];
   png_color_16 trans_color;
   png_bytep got_alpha = NULL;
   png_color_16p got_color = NULL;
   int num_trans = 0;
   int i;

   png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
       NULL, NULL, NULL);
   if (png_ptr == NULL)
   {
      fprintf(stderr, "pnggetset: png_create_write_struct failed\n");
      return 1;
   }

   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      fprintf(stderr, "pnggetset: png_create_info_struct failed\n");
      png_destroy_write_struct(&png_ptr, NULL);
      return 1;
   }

   if (setjmp(png_jmpbuf(png_ptr)))
   {
      fprintf(stderr, "pnggetset: libpng error in test_trns_roundtrip\n");
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return 1;
   }

   /* Set up a palette-color image. */
   memset(palette, 0, sizeof palette);
   png_set_IHDR(png_ptr, info_ptr, 1, 1, 8, PNG_COLOR_TYPE_PALETTE,
       PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
   png_set_PLTE(png_ptr, info_ptr, palette, 4);

   /* Populate tRNS with recognizable values. */
   for (i = 0; i < 4; i++)
      trans_alpha[i] = (png_byte)(0xff - i * 0x11);
   memset(&trans_color, 0, sizeof trans_color);

   png_set_tRNS(png_ptr, info_ptr, trans_alpha, 4, &trans_color);

   /* Get the internal pointer and feed it straight back. */
   png_get_tRNS(png_ptr, info_ptr, &got_alpha, &num_trans, &got_color);
   if (got_alpha == NULL || num_trans != 4)
   {
      fprintf(stderr, "pnggetset: png_get_tRNS returned unexpected values\n");
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return 1;
   }

   /* This is the critical call: the pointer aliases info_ptr->trans_alpha. */
   png_set_tRNS(png_ptr, info_ptr, got_alpha, num_trans, got_color);

   /* Verify the data survived the roundtrip. */
   got_alpha = NULL;
   num_trans = 0;
   png_get_tRNS(png_ptr, info_ptr, &got_alpha, &num_trans, &got_color);
   if (got_alpha == NULL || num_trans != 4)
   {
      fprintf(stderr, "pnggetset: tRNS lost after roundtrip\n");
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return 1;
   }
   for (i = 0; i < 4; i++)
   {
      if (got_alpha[i] != (png_byte)(0xff - i * 0x11))
      {
         fprintf(stderr,
             "pnggetset: tRNS entry %d corrupted after roundtrip\n", i);
         png_destroy_write_struct(&png_ptr, &info_ptr);
         return 1;
      }
   }

   png_destroy_write_struct(&png_ptr, &info_ptr);
   return 0;
}
#endif /* PNG_tRNS_SUPPORTED */

int
main(void)
{
   int result = 0;

   printf("Testing PLTE get-then-set roundtrip... ");
   fflush(stdout);
   if (test_plte_roundtrip() != 0)
   {
      printf("FAIL\n");
      result = 1;
   }
   else
      printf("PASS\n");

#ifdef PNG_hIST_SUPPORTED
   printf("Testing hIST get-then-set roundtrip... ");
   fflush(stdout);
   if (test_hist_roundtrip() != 0)
   {
      printf("FAIL\n");
      result = 1;
   }
   else
      printf("PASS\n");
#endif

#ifdef PNG_tRNS_SUPPORTED
   printf("Testing tRNS get-then-set roundtrip... ");
   fflush(stdout);
   if (test_trns_roundtrip() != 0)
   {
      printf("FAIL\n");
      result = 1;
   }
   else
      printf("PASS\n");
#endif

   return result;
}
