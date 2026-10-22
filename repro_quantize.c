/*
 * repro_quantize.c — deterministic reproduction of the png_set_quantize()
 * out-of-bounds access on unpatched libpng.
 *
 * Usage:
 *   ./repro_quantize <num_palette> <maximum_colors> [histogram 0|1]
 *
 * Cases that crash on unpatched libpng (SIGSEGV):
 *   num_palette > 256   (e.g. 257 257)   -> owned-palette memcpy overflows
 *                                           its 768-byte buffer
 *   num_palette < 0     (e.g. -1 256)    -> memcpy of ~12.9 GB
 *   maximum_colors <= 0 (e.g. 256 -1)    -> num_palette becomes negative,
 *                                           then the same oversized memcpy
 *
 * Boundary cases that are SAFE on both patched and unpatched libpng:
 *   1 256, 256 256, 256 255
 *
 * On unpatched libpng the -1 and >256 cases fault immediately (the huge
 * memcpy reads past a small buffer).  With the guard-page malloc interposer
 * (guard_malloc.c) every out-of-bounds case faults deterministically.
 *
 * Note: no PNG file is required; png_set_quantize() is called on a fresh
 * read struct.
 */
#include "png.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
   int num_palette = (argc > 1) ? atoi(argv[1]) : 257;
   int maximum_colors = (argc > 2) ? atoi(argv[2]) : 257;
   int use_hist = (argc > 3) ? atoi(argv[3]) : 0;
   png_structp png_ptr;
   png_infop info_ptr;
   png_color *palette;
   png_uint_16 *hist = NULL;
   int i, n;

   n = (num_palette < 0) ? 4 : (num_palette > 0 ? num_palette : 1);
   palette = (png_color *)malloc((size_t)n * sizeof(png_color));
   if (palette == NULL) return 2;
   for (i = 0; i < n; i++)
   {
      palette[i].red   = (png_byte)i;
      palette[i].green = (png_byte)(i >> 1);
      palette[i].blue  = (png_byte)(i >> 2);
   }
   if (use_hist)
   {
      hist = (png_uint_16 *)malloc((size_t)n * sizeof(png_uint_16));
      if (hist == NULL) return 2;
      for (i = 0; i < n; i++)
         hist[i] = (png_uint_16)(n - i);
   }

   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (png_ptr == NULL) return 2;
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL) { png_destroy_read_struct(&png_ptr, NULL, NULL); return 2; }

   if (setjmp(png_jmpbuf(png_ptr)))
   {
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      fprintf(stderr, "libpng raised an error (png_error/longjmp)\n");
      return 1;
   }

   fprintf(stderr, "png_set_quantize(num_palette=%d, maximum_colors=%d, hist=%d)\n",
       num_palette, maximum_colors, use_hist);
   png_set_quantize(png_ptr, palette, num_palette, maximum_colors, hist, 0);

   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
   free(palette);
   free(hist);
   fprintf(stderr, "OK: call completed without fault\n");
   return 0;
}
