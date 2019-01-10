/* pngadhoc.c
 *
 * Copyright (c) 2019 Vladimir Panteleev
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * Ad-hoc libpng regression tests.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Define the following to use this test against your installed libpng, rather
 * than the one being built here:
 */
#ifdef PNG_FREESTANDING_TESTS
#  include <png.h>
#else
#  include "../../png.h"
#endif

struct buffer
{
   png_bytep data;
   size_t length;
};

static void
png_write_callback(png_structp png_ptr, png_bytep data, png_size_t length)
{
   struct buffer *buffer = (struct buffer *)png_get_io_ptr(png_ptr);
   buffer->data = realloc(buffer->data, buffer->length + length);
   if (buffer->data == NULL)
      png_error(png_ptr, "OOM while writing PNG");
   memcpy(buffer->data + buffer->length, data, length);
   buffer->length += length;
}

static void
png_read_callback(png_structp png_ptr, png_bytep data, png_size_t length)
{
   struct buffer *buffer = (struct buffer *)png_get_io_ptr(png_ptr);
   if (buffer->length < length)
      png_error(png_ptr, "EOF while reading PNG");
   memcpy(data, buffer->data, length);
   buffer->data += length;
   buffer->length -= length;
}

static int
png_test_bKGD_rgb_to_gray()
{
#if defined(PNG_SEQUENTIAL_READ_SUPPORTED) && \
   defined(PNG_bKGD_SUPPORTED) && \
   defined(PNG_READ_BACKGROUND_SUPPORTED) && \
   defined(PNG_READ_EXPAND_SUPPORTED) && \
   defined(PNG_READ_STRIP_ALPHA_SUPPORTED) && \
   defined(PNG_READ_RGB_TO_GRAY_SUPPORTED)

   png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
      0, 0, 0);
   png_infop info_ptr = NULL;
   png_byte row[4];
   struct buffer write_buffer, read_buffer;
   png_color_16 write_background = {0, 255, 255, 255, 0};
   png_color_16p read_background;

   if (png_ptr == NULL)
   {
      fprintf(stderr, "OOM allocating write structure\n");
      return 1;
   }

   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
      png_error(png_ptr, "OOM allocating info structure");

   write_buffer.data = NULL;
   write_buffer.length = 0;
   png_set_write_fn(png_ptr, &write_buffer, &png_write_callback, NULL);

   png_set_IHDR(
      png_ptr, info_ptr,
      1, 1,
      8,
      PNG_COLOR_TYPE_RGBA,
      PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_DEFAULT,
      PNG_FILTER_TYPE_DEFAULT);
   png_set_bKGD(png_ptr, info_ptr, &write_background);
   png_write_info(png_ptr, info_ptr);

   row[0] = row[1] = row[2] = 0;
   row[3] = 0; // Transparent
   png_write_row(png_ptr, row);
   png_write_end(png_ptr, NULL);
   png_destroy_write_struct(&png_ptr, &info_ptr);

   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
      0, 0, 0);
   if (png_ptr == NULL)
   {
      fprintf(stderr, "OOM allocating write structure\n");
      return 1;
   }

   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
      png_error(png_ptr, "OOM allocating info structure");

   read_buffer = write_buffer;
   png_set_read_fn(png_ptr, &read_buffer, &png_read_callback);

   png_read_info(png_ptr, info_ptr);

   png_set_rgb_to_gray(png_ptr,
      PNG_ERROR_ACTION_NONE,
      PNG_RGB_TO_GRAY_DEFAULT,
      PNG_RGB_TO_GRAY_DEFAULT);

   png_set_expand(png_ptr);
   png_set_strip_alpha(png_ptr);
   /* as per manual */
   if (png_get_bKGD(png_ptr, info_ptr, &read_background))
      png_set_background(png_ptr, read_background,
         PNG_BACKGROUND_GAMMA_FILE, 1/*needs to be expanded*/, 1);
   else
      png_error(png_ptr, "No bKGD");

   png_read_row(png_ptr, row, NULL);
   if (row[0] != 255)
      png_error(png_ptr, "Wrong color with bKGD");

   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

   free(write_buffer.data);
#endif

   return 0;
}

static int
png_test_bKGD_tRNS_4bit()
{
#if defined(PNG_SEQUENTIAL_READ_SUPPORTED) && \
   defined(PNG_bKGD_SUPPORTED) && \
   defined(PNG_READ_BACKGROUND_SUPPORTED) && \
   defined(PNG_READ_EXPAND_SUPPORTED) && \
   defined(PNG_READ_STRIP_ALPHA_SUPPORTED) && \
   defined(PNG_READ_RGB_TO_GRAY_SUPPORTED)

   png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
      0, 0, 0);
   png_infop info_ptr = NULL;
   png_byte row[1];
   struct buffer write_buffer, read_buffer;
   png_color_16 write_background = {0, 0, 0, 0, 15};
   png_color_16 trans = {0, 0, 0, 0, 10};
   png_color_16p read_background;

   if (png_ptr == NULL)
   {
      fprintf(stderr, "OOM allocating write structure\n");
      return 1;
   }

   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
      png_error(png_ptr, "OOM allocating info structure");

   write_buffer.data = NULL;
   write_buffer.length = 0;
   png_set_write_fn(png_ptr, &write_buffer, &png_write_callback, NULL);

   png_set_IHDR(
      png_ptr, info_ptr,
      1, 1,
      4,
      PNG_COLOR_TYPE_GRAY,
      PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_DEFAULT,
      PNG_FILTER_TYPE_DEFAULT);
   png_set_bKGD(png_ptr, info_ptr, &write_background);
   png_set_tRNS(png_ptr, info_ptr, NULL, 1, &trans);
   png_write_info(png_ptr, info_ptr);

   row[0] = 10 << 4;
   png_write_row(png_ptr, row);
   png_write_end(png_ptr, NULL);
   png_destroy_write_struct(&png_ptr, &info_ptr);

   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
      0, 0, 0);
   if (png_ptr == NULL)
   {
      fprintf(stderr, "OOM allocating write structure\n");
      return 1;
   }

   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
      png_error(png_ptr, "OOM allocating info structure");

   read_buffer = write_buffer;
   png_set_read_fn(png_ptr, &read_buffer, &png_read_callback);

   png_read_info(png_ptr, info_ptr);

   png_set_expand(png_ptr);
   png_set_strip_alpha(png_ptr);
   /* as per manual */
   if (png_get_bKGD(png_ptr, info_ptr, &read_background))
   {
      png_set_background(png_ptr, read_background,
         PNG_BACKGROUND_GAMMA_FILE, 1/*needs to be expanded*/, 1);
   }
   else
      png_error(png_ptr, "No bKGD");

   png_read_row(png_ptr, row, NULL);
   if (row[0] != 255)
      png_error(png_ptr, "Wrong color with bKGD/tRNS");

   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

   free(write_buffer.data);
#endif

   return 0;
}

int
main(void)
{
   int ret;

   ret = png_test_bKGD_rgb_to_gray();
   if (ret) return ret;

   ret = png_test_bKGD_tRNS_4bit();
   if (ret) return ret;

   return 0;
}
