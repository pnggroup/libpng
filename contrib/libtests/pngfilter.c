/* pngfilter.c
 *
 * Copyright (c) 2026 Akash Deshmukh
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * Check that png_set_filter selects the requested row filter, whether it is
 * given as a filter value (PNG_FILTER_VALUE_UP) or as a filter mask
 * (PNG_FILTER_UP).  Each image is written with a single filter allowed, and
 * without compression, so that the filter type byte of every row can be read
 * directly from the stored zlib blocks of the IDAT data.
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

#if defined(PNG_WRITE_SUPPORTED) && defined(PNG_WRITE_FILTER_SUPPORTED) && \
    defined(PNG_WRITE_CUSTOMIZE_COMPRESSION_SUPPORTED)

#define WIDTH 8
#define HEIGHT 8
#define ROWBYTES (1 + WIDTH)   /* filter type byte, then 8-bit gray pixels */

static png_byte png_data[4096];
static size_t png_size;

static void PNGCBAPI
write_to_memory(png_structp png_ptr, png_bytep data, size_t length)
{
   if (length > (sizeof png_data) - png_size)
      png_error(png_ptr, "test image too large");

   memcpy(png_data + png_size, data, length);
   png_size += length;
}

static void PNGCBAPI
flush_memory(png_structp png_ptr)
{
   (void)png_ptr;
}

static png_uint_32
get_uint_32(png_const_bytep p)
{
   return ((png_uint_32)p[0] << 24) | ((png_uint_32)p[1] << 16) |
       ((png_uint_32)p[2] << 8) | (png_uint_32)p[3];
}

/* Writes an image with png_set_filter(png_ptr, PNG_FILTER_TYPE_BASE, filters).
 * Returns 0 on success.
 */
static int
write_image(int filters)
{
   png_structp png_ptr;
   png_infop info_ptr;
   png_byte rows[HEIGHT][WIDTH];
   int x, y;

   for (y = 0; y < HEIGHT; ++y)
      for (x = 0; x < WIDTH; ++x)
         rows[y][x] = (png_byte)(x * 29 + y * 13 + x * y);

   png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (png_ptr == NULL)
      return 1;

   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL || setjmp(png_jmpbuf(png_ptr)))
   {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return 1;
   }

   png_size = 0;
   png_set_write_fn(png_ptr, NULL, write_to_memory, flush_memory);
   png_set_IHDR(png_ptr, info_ptr, WIDTH, HEIGHT, 8, PNG_COLOR_TYPE_GRAY,
       PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
   png_set_compression_level(png_ptr, 0);
   png_set_filter(png_ptr, PNG_FILTER_TYPE_BASE, filters);
   png_write_info(png_ptr, info_ptr);
   for (y = 0; y < HEIGHT; ++y)
      png_write_row(png_ptr, rows[y]);
   png_write_end(png_ptr, info_ptr);
   png_destroy_write_struct(&png_ptr, &info_ptr);
   return 0;
}

/* Extracts the uncompressed image data of the image in png_data, which must
 * consist of stored zlib blocks.  Returns its size, or 0 on failure.
 */
static size_t
get_image_data(png_bytep out, size_t out_size)
{
   png_byte zdata[sizeof png_data];
   size_t zsize = 0, size = 0, pos = 8;   /* after the PNG signature */

   while (pos + 12 <= png_size)
   {
      png_uint_32 length = get_uint_32(png_data + pos);

      if (length > png_size - pos - 12)
         return 0;

      if (memcmp(png_data + pos + 4, "IDAT", 4) == 0)
      {
         memcpy(zdata + zsize, png_data + pos + 8, length);
         zsize += length;
      }

      pos += 12 + length;
   }

   pos = 2;   /* after the zlib header */
   for (;;)
   {
      unsigned int header, length;

      if (pos + 5 > zsize)
         return 0;

      header = zdata[pos];
      if ((header & 0x06) != 0)   /* not a stored block */
         return 0;

      length = zdata[pos + 1] | (zdata[pos + 2] << 8);
      pos += 5;
      if (length > zsize - pos || length > out_size - size)
         return 0;

      memcpy(out + size, zdata + pos, length);
      size += length;
      pos += length;

      if ((header & 0x01) != 0)   /* final block */
         return size;
   }
}

static int
test_filter(const char *name, int filters, png_byte expected)
{
   png_byte data[HEIGHT * ROWBYTES];
   int y;

   if (write_image(filters) != 0)
   {
      fprintf(stderr, "pngfilter: %s: cannot write the test image\n", name);
      return 1;
   }

   if (get_image_data(data, sizeof data) != sizeof data)
   {
      fprintf(stderr, "pngfilter: %s: unexpected IDAT data\n", name);
      return 1;
   }

   for (y = 0; y < HEIGHT; ++y)
   {
      if (data[y * ROWBYTES] != expected)
      {
         fprintf(stderr, "pngfilter: %s: row %d uses filter %d, not %d\n",
             name, y, data[y * ROWBYTES], expected);
         return 1;
      }
   }

   return 0;
}

int
main(void)
{
   int failures = 0;

   failures += test_filter("PNG_FILTER_VALUE_NONE", PNG_FILTER_VALUE_NONE,
       PNG_FILTER_VALUE_NONE);
   failures += test_filter("PNG_FILTER_VALUE_SUB", PNG_FILTER_VALUE_SUB,
       PNG_FILTER_VALUE_SUB);
   failures += test_filter("PNG_FILTER_VALUE_UP", PNG_FILTER_VALUE_UP,
       PNG_FILTER_VALUE_UP);
   failures += test_filter("PNG_FILTER_VALUE_AVG", PNG_FILTER_VALUE_AVG,
       PNG_FILTER_VALUE_AVG);
   failures += test_filter("PNG_FILTER_VALUE_PAETH", PNG_FILTER_VALUE_PAETH,
       PNG_FILTER_VALUE_PAETH);

   failures += test_filter("PNG_FILTER_NONE", PNG_FILTER_NONE,
       PNG_FILTER_VALUE_NONE);
   failures += test_filter("PNG_FILTER_SUB", PNG_FILTER_SUB,
       PNG_FILTER_VALUE_SUB);
   failures += test_filter("PNG_FILTER_UP", PNG_FILTER_UP,
       PNG_FILTER_VALUE_UP);
   failures += test_filter("PNG_FILTER_AVG", PNG_FILTER_AVG,
       PNG_FILTER_VALUE_AVG);
   failures += test_filter("PNG_FILTER_PAETH", PNG_FILTER_PAETH,
       PNG_FILTER_VALUE_PAETH);

   if (failures != 0)
      return 1;

   printf("pngfilter: all tests passed\n");
   return 0;
}

#else /* !(WRITE && WRITE_FILTER && WRITE_CUSTOMIZE_COMPRESSION) */
int
main(void)
{
   fprintf(stderr, "pngfilter: no support for writing with chosen filters\n");
   return 77;
}
#endif /* WRITE && WRITE_FILTER && WRITE_CUSTOMIZE_COMPRESSION */
