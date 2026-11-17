/* pngtextlimit.c
 *
 * Copyright (c) 2026 Cosmin Truta
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * Test the per-chunk-type decompression limit for text chunks:
 * png_set_chunk_text_max limits the decompressed size of zTXt and iTXt
 * chunks separately from png_set_chunk_malloc_max, which continues to
 * govern the other decompressed chunks, in particular iCCP.
 *
 * The arguments are three crafted PNG files, in this order:
 *
 *   text-amplify.png: a zTXt chunk that expands to 7,900,000 bytes from
 *      7,691 compressed bytes.  This is below the default chunk malloc
 *      limit of 8,000,000 bytes, but above the default text limit of
 *      1,000,000 bytes.
 *   iccp-large.png: an iCCP chunk holding a 2,097,152-byte ICC profile,
 *      above the text limit but below the chunk malloc limit.
 *   normal.png: a small tEXt chunk and a small zTXt chunk, 16 bytes of
 *      text in total, below every limit.
 *
 * Benign chunk errors are treated as warnings, so that a chunk ignored
 * because of a limit can be distinguished from a chunk that was stored.
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

#if defined(PNG_SEQUENTIAL_READ_SUPPORTED) && defined(PNG_STDIO_SUPPORTED) &&\
    defined(PNG_BENIGN_ERRORS_SUPPORTED)
typedef struct
{
   png_alloc_size_t text_bytes; /* total stored text, from png_get_text */
   png_alloc_size_t iccp_bytes; /* stored ICC profile size, 0 if none */
   int              error;      /* a libpng error occurred while reading */
} read_result;

static read_result
read_header(const char *file_name, png_alloc_size_t text_max, int set_text_max)
{
   read_result result;
   FILE *fp;
   png_structp png_ptr;
   png_infop info_ptr;

   result.text_bytes = 0;
   result.iccp_bytes = 0;
   result.error = 0;

   fp = fopen(file_name, "rb");
   if (fp == NULL)
   {
      fprintf(stderr, "pngtextlimit: %s: cannot open\n", file_name);
      result.error = 1;
      return result;
   }

   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   info_ptr = png_ptr != NULL ? png_create_info_struct(png_ptr) : NULL;
   if (png_ptr == NULL || info_ptr == NULL)
   {
      fprintf(stderr, "pngtextlimit: png_create_read_struct failed\n");
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      fclose(fp);
      result.error = 1;
      return result;
   }

   /* Chunks that exceed a limit are ignored with a benign error; make it
    * a warning so that the stored result can be inspected below.
    */
   png_set_benign_errors(png_ptr, 1);

   if (set_text_max)
   {
#     ifdef PNG_SET_USER_LIMITS_SUPPORTED
         png_set_chunk_text_max(png_ptr, text_max);

         /* The getter must report the value that was set, with 0 mapped
          * to the system limit, as in png_set_chunk_malloc_max.
          */
         {
            png_alloc_size_t expected = text_max;

            if (expected == 0U) /* unlimited */
            {
#              ifdef PNG_MAX_MALLOC_64K
                  expected = 65536U;
#              else
                  expected = PNG_SIZE_MAX;
#              endif
            }

            if (png_get_chunk_text_max(png_ptr) != expected)
            {
               fprintf(stderr, "pngtextlimit: png_get_chunk_text_max did "
                   "not return the value set\n");
               result.error = 1;
            }
         }
#     else
         fprintf(stderr, "pngtextlimit: no run-time user limits\n");
         result.error = 1;
#     endif
   }

#  ifdef PNG_SET_USER_LIMITS_SUPPORTED
   else
   {
      /* Without a call to the setter the getter must report the
       * compile-time default, PNG_USER_CHUNK_TEXT_MAX.
       */
#     if PNG_USER_CHUNK_TEXT_MAX > 0
         png_alloc_size_t expected = PNG_USER_CHUNK_TEXT_MAX;
#     elif defined PNG_MAX_MALLOC_64K
         png_alloc_size_t expected = 65536U;
#     else
         png_alloc_size_t expected = PNG_SIZE_MAX;
#     endif

      if (png_get_chunk_text_max(png_ptr) != expected)
      {
         fprintf(stderr, "pngtextlimit: png_get_chunk_text_max did not "
             "return the default\n");
         result.error = 1;
      }
   }
#  endif

   if (setjmp(png_jmpbuf(png_ptr)) == 0)
   {
      png_textp text_ptr;
      int num_text = 0, i;
      png_charp name;
      png_bytep profile;
      png_uint_32 proflen = 0;
      int intent;

      png_init_io(png_ptr, fp);
      png_read_info(png_ptr, info_ptr);

      num_text = png_get_text(png_ptr, info_ptr, &text_ptr, NULL);
      for (i = 0; i < num_text; ++i)
         result.text_bytes += text_ptr[i].text_length +
             text_ptr[i].itxt_length;

      if (png_get_iCCP(png_ptr, info_ptr, &name, &intent, &profile,
              &proflen))
         result.iccp_bytes = proflen;
   }

   else
      result.error = 1;

   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
   fclose(fp);
   return result;
}

static int
check(const char *what, read_result result, png_alloc_size_t text_bytes,
    png_alloc_size_t iccp_bytes)
{
   if (result.error == 0 && result.text_bytes == text_bytes &&
       result.iccp_bytes == iccp_bytes)
   {
      printf("pngtextlimit: %s: ok (text %lu bytes, iCCP %lu bytes)\n",
          what, (unsigned long)result.text_bytes,
          (unsigned long)result.iccp_bytes);
      return 0;
   }

   fprintf(stderr, "pngtextlimit: %s: FAILED (error %d, text %lu bytes, "
       "iCCP %lu bytes; expected text %lu bytes, iCCP %lu bytes)\n",
       what, result.error, (unsigned long)result.text_bytes,
       (unsigned long)result.iccp_bytes, (unsigned long)text_bytes,
       (unsigned long)iccp_bytes);
   return 1;
}
#endif

int
main(int argc, char **argv)
{
#if defined(PNG_SEQUENTIAL_READ_SUPPORTED) && defined(PNG_STDIO_SUPPORTED) &&\
    defined(PNG_BENIGN_ERRORS_SUPPORTED)
   int result = 0;

   if (argc != 4)
   {
      fprintf(stderr, "usage: pngtextlimit text-amplify.png iccp-large.png "
          "normal.png\n");
      return 2;
   }

   /* 1. By default the 7,900,000-byte zTXt exceeds the text limit and is
    *    ignored, even though it fits in the chunk malloc limit.
    */
   result |= check("default text limit",
       read_header(argv[1], 0, 0), 0, 0);

   /* 2. With the text limit raised to the chunk malloc limit the same
    *    chunk is stored in full.
    */
   result |= check("raised text limit",
       read_header(argv[1], 8000000, 1), 7900000, 0);

   /* 3. With the text limit removed (0, unlimited) the chunk malloc
    *    limit alone governs, and the chunk is stored in full.
    */
   result |= check("removed text limit",
       read_header(argv[1], 0, 1), 7900000, 0);

   /* 4. A 2,097,152-byte ICC profile is not a text chunk: it is stored
    *    under the defaults, although it exceeds the text limit.
    */
   result |= check("ICC profile over the text limit",
       read_header(argv[2], 0, 0), 0, 2097152);

   /* 5. Ordinary small text chunks are stored unchanged. */
   result |= check("small text chunks",
       read_header(argv[3], 0, 0), 16, 0);

   return result;
#else
   (void)argc;
   (void)argv;
   fprintf(stderr, "pngtextlimit: no support for sequential read, stdio "
       "or benign errors\n");
   return 77;
#endif
}
