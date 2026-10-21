/* pngreadend.c
 *
 * Copyright (c) 2026 Cosmin Truta
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * Call png_read_end right after png_read_info, without reading the image.
 * This call sequence ends in a libpng error on every input, even on a valid
 * PNG.  The test passes on such an error, and fails only on a crash or on a
 * report from a memory sanitizer.
 *
 * Before the fix for CVE-2026-46675, png_read_end resumed inflation from input
 * left over in the read buffer of a zTXt chunk, which a subsequent pCAL chunk
 * had freed by requesting a larger one (stale_zTXt_input.png), or in an
 * automatic array local to png_handle_iCCP after that function had returned
 * (stale_iCCP_input.png).  Both test files declare an invalid zlib window size
 * on purpose, so that the stale read happens in libpng's own window check,
 * which the sanitizer instruments, rather than inside zlib, which is usually
 * built without it.
 */

#include <stdio.h>
#include <stdlib.h>

#if defined(HAVE_CONFIG_H) && !defined(PNG_NO_CONFIG_H)
#  include <config.h>
#endif

#ifdef PNG_FREESTANDING_TESTS
#  include <png.h>
#else
#  include "../../png.h"
#endif

/* AddressSanitizer detects an access to an automatic array after its function
 * has returned only if it puts such arrays on a separate stack, which is
 * optional.
 */
const char *__asan_default_options(void);

const char *
__asan_default_options(void)
{
   return "detect_stack_use_after_return=1";
}

#if defined(PNG_SEQUENTIAL_READ_SUPPORTED) && defined(PNG_STDIO_SUPPORTED)
static int
read_info_then_end(const char *file_name)
{
   FILE *fp;
   png_structp png_ptr;
   png_infop info_ptr;

   fp = fopen(file_name, "rb");
   if (fp == NULL)
   {
      fprintf(stderr, "pngreadend: %s: cannot open\n", file_name);
      return 1;
   }

   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (png_ptr == NULL)
   {
      fprintf(stderr, "pngreadend: png_create_read_struct failed\n");
      fclose(fp);
      return 1;
   }

   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      fprintf(stderr, "pngreadend: png_create_info_struct failed\n");
      png_destroy_read_struct(&png_ptr, NULL, NULL);
      fclose(fp);
      return 1;
   }

   if (setjmp(png_jmpbuf(png_ptr)) == 0)
   {
      png_init_io(png_ptr, fp);
      png_read_info(png_ptr, info_ptr);
      png_read_end(png_ptr, NULL);
   }

   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
   fclose(fp);
   return 0;
}
#endif

int
main(int argc, char **argv)
{
#if defined(PNG_SEQUENTIAL_READ_SUPPORTED) && defined(PNG_STDIO_SUPPORTED)
   int result = 0;
   int i;

   for (i = 1; i < argc; ++i)
   {
      printf("Testing png_read_end after png_read_info on %s\n", argv[i]);
      fflush(stdout);
      if (read_info_then_end(argv[i]) != 0)
         result = 1;
   }

   return result;
#else
   (void)argc;
   (void)argv;
   fprintf(stderr, "pngreadend: no support for sequential read or stdio\n");
   return 77;
#endif
}
