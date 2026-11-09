/* pngstride.c
 *
 * Copyright (c) 2026 Akash Deshmukh
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * Check the validation of the image size and of the 'row_stride' argument
 * in the simplified API, on both the read and the write side.  Valid layouts,
 * including bottom-up ones, must be accepted.  Invalid ones must be refused
 * with an error, before the buffer is accessed: the buffers passed along with
 * an oversized layout are deliberately much smaller than that layout implies,
 * so that an access to them is reported by a memory sanitizer.
 */

#include <stdio.h>
#include <string.h>

#if defined(HAVE_CONFIG_H) && !defined(PNG_NO_CONFIG_H)
#  include <config.h>
#endif

#ifdef PNG_FREESTANDING_TESTS
#  include <png.h>
#else
#  include "../../png.h"
#endif

#if defined(PNG_SIMPLIFIED_READ_SUPPORTED) && \
    defined(PNG_SIMPLIFIED_WRITE_SUPPORTED)

#define WIDTH 4
#define HEIGHT 3

static png_byte pixels[2 * WIDTH * HEIGHT]; /* room for 16-bit gray */
static png_byte png_data[1024];   /* the image read back by test_read */
static png_alloc_size_t png_size;
static png_byte png_scratch[1024];

static int failures = 0;

static void
init_image(png_image *image, png_uint_32 width, png_uint_32 height,
    png_uint_32 format)
{
   memset(image, 0, (sizeof *image));
   image->version = PNG_IMAGE_VERSION;
   image->width = width;
   image->height = height;
   image->format = format;
}

static void
check(const char *test, int result, int expected, const png_image *image)
{
   if ((result != 0) != expected)
   {
      fprintf(stderr, "pngstride: %s: %s (%s)\n", test,
          expected ? "unexpected failure" : "unexpected success",
          image->message);
      ++failures;
   }
}

static int
write_image(png_uint_32 width, png_uint_32 height, png_uint_32 format,
    png_int_32 row_stride)
{
   png_image image;
   png_alloc_size_t size = (sizeof png_data);
   int result;

   init_image(&image, width, height, format);
   result = png_image_write_to_memory(&image, png_data, &size, 0, pixels,
       row_stride, NULL);
   if (result != 0)
      png_size = size;
   return result;
}

static void
test_write(const char *test, png_uint_32 width, png_uint_32 height,
    png_uint_32 format, png_int_32 row_stride, int expected)
{
   png_image image;
   png_alloc_size_t size = (sizeof png_scratch);

   init_image(&image, width, height, format);
   check(test, png_image_write_to_memory(&image, png_scratch, &size, 0,
       pixels, row_stride, NULL), expected, &image);
}

static void
test_read(const char *test, png_int_32 row_stride, int expected)
{
   png_image image;

   memset(&image, 0, (sizeof image));
   image.version = PNG_IMAGE_VERSION;
   if (png_image_begin_read_from_memory(&image, png_data, png_size) == 0)
   {
      fprintf(stderr, "pngstride: %s: cannot begin read (%s)\n", test,
          image.message);
      ++failures;
      return;
   }

   image.format = PNG_FORMAT_GRAY;
   check(test, png_image_finish_read(&image, NULL, pixels, row_stride, NULL),
       expected, &image);
   png_image_free(&image);
}

int
main(void)
{
   if (write_image(WIDTH, HEIGHT, PNG_FORMAT_GRAY, 0) == 0)
   {
      fprintf(stderr, "pngstride: cannot write the test image\n");
      return 1;
   }

   test_write("write, default stride", WIDTH, HEIGHT, PNG_FORMAT_GRAY, 0, 1);
   test_write("write, bottom-up", WIDTH, HEIGHT, PNG_FORMAT_GRAY, -WIDTH, 1);
   test_write("write, padded rows", 2, HEIGHT, PNG_FORMAT_GRAY, WIDTH, 1);
   test_write("write, stride too small", WIDTH, HEIGHT, PNG_FORMAT_GRAY,
       WIDTH - 1, 0);
   test_write("write, zero width", 0, HEIGHT, PNG_FORMAT_GRAY, 0, 0);
   test_write("write, buffer over 32 bits", WIDTH, 0x40000001U,
       PNG_FORMAT_GRAY, WIDTH, 0);
   test_write("write, 16-bit buffer over 32 bits", WIDTH, 0x20000000U,
       PNG_FORMAT_LINEAR_Y, WIDTH, 0);
   test_write("write, row stride over 31 bits", 0x80000000U, 1,
       PNG_FORMAT_GRAY, 0, 0);
   /* The step between rows, in bytes, must fit in a ptrdiff_t: this refuses
    * the largest row stride of a 16-bit format where ptrdiff_t has 32 bits.
    */
   test_write("write, 16-bit row step over ptrdiff_t", 1, 1,
       PNG_FORMAT_LINEAR_Y, 0x7fffffff, (sizeof (ptrdiff_t)) > 4);

   test_read("read, default stride", 0, 1);
   test_read("read, bottom-up", -WIDTH, 1);
   test_read("read, stride too small", WIDTH - 1, 0);
   test_read("read, buffer over 32 bits", 0x7fffffff, 0);

   if (failures != 0)
      return 1;

   printf("pngstride: all tests passed\n");
   return 0;
}

#else /* !(SIMPLIFIED_READ && SIMPLIFIED_WRITE) */
int
main(void)
{
   fprintf(stderr, "pngstride: no support for simplified read and write\n");
   return 77;
}
#endif /* SIMPLIFIED_READ && SIMPLIFIED_WRITE */
