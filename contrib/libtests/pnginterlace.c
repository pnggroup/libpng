/* pnginterlace.c
 *
 * Copyright (c) 2026 Yashvardhan Thanvi
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h.
 *
 * Compare simplified reads of identical flat and Adam7 images.
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

/* The configure test harness uses 77 to indicate a skipped test. */
#ifdef HAVE_CONFIG_H
#  define SKIP 77
#else
#  define SKIP 0
#endif

#if defined(PNG_SIMPLIFIED_READ_SUPPORTED) && \
    defined(PNG_WRITE_SUPPORTED) && \
    defined(PNG_WRITE_INTERLACING_SUPPORTED) && \
    defined(PNG_WRITE_16BIT_SUPPORTED) && defined(PNG_WRITE_gAMA_SUPPORTED) && \
    (defined(PNG_FIXED_POINT_SUPPORTED) || \
    defined(PNG_FLOATING_POINT_SUPPORTED)) && defined(PNG_SETJMP_SUPPORTED)

/* The largest test image is 17x19 RGBA at 16 bits per channel, which takes
 * less than 3000 bytes even when stored without compression.
 */
#define MEM_BUF_SIZE 8192

typedef struct
{
   png_byte data[MEM_BUF_SIZE];
   size_t len;
} mem_buf;

static void PNGCBAPI
mem_write(png_struct *png_ptr, png_byte *buf, size_t length)
{
   mem_buf *mb = (mem_buf *)png_get_io_ptr(png_ptr);

   if (length > MEM_BUF_SIZE - mb->len)
      png_error(png_ptr, "pnginterlace: write overflow");

   memcpy(mb->data + mb->len, buf, length);
   mb->len += length;
}

static void PNGCBAPI
mem_flush(png_struct *png_ptr)
{
   (void)png_ptr;
}

static int
make_image(mem_buf *mb, unsigned int width, unsigned int height,
    int color_type, int bit_depth, int interlace)
{
   png_struct *png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL,
       NULL, NULL);
   png_info *info = png == NULL ? NULL : png_create_info_struct(png);
   png_byte pixels[19][17*8];
   png_byte *rows[19];
   unsigned int channels = (color_type & PNG_COLOR_MASK_COLOR) != 0 ? 3 : 1;
   unsigned int bytes = bit_depth == 16 ? 2 : 1;
   unsigned int x, y, c;

   if ((color_type & PNG_COLOR_MASK_ALPHA) != 0)
      ++channels;
   if (png == NULL || info == NULL)
   {
      fprintf(stderr, "pnginterlace: out of memory\n");
      png_destroy_write_struct(&png, &info);
      return 0;
   }
   if (setjmp(png_jmpbuf(png)))
   {
      png_destroy_write_struct(&png, &info);
      return 0;
   }

   for (y=0; y<height; ++y)
   {
      rows[y] = pixels[y];
      for (x=0; x<width; ++x)
         for (c=0; c<channels; ++c)
         {
            unsigned int value = (x*3911 + y*1549 + c*12713) & 65535;
            unsigned int offset = (x*channels+c)*bytes;
            if ((color_type & PNG_COLOR_MASK_ALPHA) != 0 && c == channels-1)
               value = (x+y)%3 == 0 ? 0 : ((x+y)%3 == 1 ? 65535 : 32768);
            pixels[y][offset] = (png_byte)(value >> 8);
            if (bytes == 2)
               pixels[y][offset+1] = (png_byte)value;
         }
   }

   mb->len = 0;
   png_set_write_fn(png, mb, mem_write, mem_flush);
   png_set_IHDR(png, info, width, height, bit_depth, color_type, interlace,
       PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
#ifdef PNG_FIXED_POINT_SUPPORTED
   png_set_gAMA_fixed(png, info, 100000);
#else
   png_set_gAMA(png, info, 1.0);
#endif
   png_write_info(png, info);
   png_write_image(png, rows);
   png_write_end(png, info);
   png_destroy_write_struct(&png, &info);
   return 1;
}

static int
read_image(const mem_buf *mb, unsigned int width, unsigned int height,
    png_uint_32 format, png_int_32 stride, png_byte *buffer)
{
   png_image image;
   memset(&image, 0, sizeof image);
   image.version = PNG_IMAGE_VERSION;
   if (!png_image_begin_read_from_memory(&image, mb->data, mb->len))
      goto error;
   if (image.width != width || image.height != height)
      goto error;
   image.format = format;
   if (!png_image_finish_read(&image, NULL, buffer, stride, NULL))
      goto error;
   png_image_free(&image);
   return 1;

error:
   fprintf(stderr, "pnginterlace: read failed: %s\n", image.message);
   png_image_free(&image);
   return 0;
}

static int
compare_images(unsigned int width, unsigned int height,
    int color_type, int bit_depth, png_uint_32 format, int extra, int negative)
{
   static mem_buf flat, interlaced;
   png_int_32 stride = (png_int_32)(width*PNG_IMAGE_PIXEL_CHANNELS(format))
       + extra;
   size_t row_bytes = (size_t)stride*PNG_IMAGE_PIXEL_COMPONENT_SIZE(format);
   size_t pixel_bytes = width*PNG_IMAGE_PIXEL_SIZE(format);
   size_t size = row_bytes*height;
   png_byte *a = (png_byte *)malloc(size+64);
   png_byte *b = (png_byte *)malloc(size+64);
   size_t i;
   unsigned int y;
   int ok = 0;

   if (a == NULL || b == NULL)
   {
      fprintf(stderr, "pnginterlace: out of memory\n");
      goto done;
   }
   if (!make_image(&flat, width, height, color_type, bit_depth,
       PNG_INTERLACE_NONE) ||
       !make_image(&interlaced, width, height, color_type, bit_depth,
       PNG_INTERLACE_ADAM7))
      goto done;
   memset(a, 0xa5, size+64);
   memset(b, 0xa5, size+64);
   if (negative)
      stride = -stride;
   if (!read_image(&flat, width, height, format, stride, a+32) ||
       !read_image(&interlaced, width, height, format, stride, b+32))
      goto done;
   if (memcmp(a, b, size+64) != 0)
      goto done;

   /* Padding and both allocation guards must survive each read. */
   for (i=0; i<32; ++i)
      if (a[i] != 0xa5 || a[32+size+i] != 0xa5)
         goto done;
   for (y=0; y<height; ++y)
      for (i=pixel_bytes; i<row_bytes; ++i)
         if (a[32+y*row_bytes+i] != 0xa5)
            goto done;
   ok = 1;

done:
   free(a);
   free(b);
   return ok;
}

int
main(void)
{
   static const unsigned int sizes[][2] = {
      {1, 1}, {1, 9}, {9, 1}, {2, 2}, {3, 5}, {9, 13}, {16, 16}, {17, 19}
   };
   static const struct {
      int color_type;
      png_uint_32 format;
   } formats[] = {
      {PNG_COLOR_TYPE_GRAY, PNG_FORMAT_GRAY},
      {PNG_COLOR_TYPE_GRAY_ALPHA, PNG_FORMAT_GA},
#ifdef PNG_SIMPLIFIED_READ_AFIRST_SUPPORTED
      {PNG_COLOR_TYPE_GRAY_ALPHA, PNG_FORMAT_AG},
#endif
      {PNG_COLOR_TYPE_RGB, PNG_FORMAT_RGB},
#ifdef PNG_SIMPLIFIED_READ_BGR_SUPPORTED
      {PNG_COLOR_TYPE_RGB, PNG_FORMAT_BGR},
#endif
      {PNG_COLOR_TYPE_RGB_ALPHA, PNG_FORMAT_RGBA},
#ifdef PNG_SIMPLIFIED_READ_BGR_SUPPORTED
      {PNG_COLOR_TYPE_RGB_ALPHA, PNG_FORMAT_BGRA},
#endif
#ifdef PNG_SIMPLIFIED_READ_AFIRST_SUPPORTED
      {PNG_COLOR_TYPE_RGB_ALPHA, PNG_FORMAT_ARGB},
#  ifdef PNG_SIMPLIFIED_READ_BGR_SUPPORTED
      {PNG_COLOR_TYPE_RGB_ALPHA, PNG_FORMAT_ABGR}
#  endif
#endif
   };
   static const int padding[] = {0, 7, 33000};
   unsigned int s, f, p, negative, mode, count = 0, failures = 0;
   unsigned int failures_by_mode[3] = {0, 0, 0};

   for (s=0; s<sizeof sizes/sizeof sizes[0]; ++s)
      for (f=0; f<sizeof formats/sizeof formats[0]; ++f)
         for (p=0; p<sizeof padding/sizeof padding[0]; ++p)
            for (negative=0; negative<2; ++negative)
               for (mode=0; mode<3; ++mode)
               {
                  png_uint_32 format = formats[f].format;
                  int depth = mode == 1 ? 8 : 16;
                  if (mode == 2)
                     format |= PNG_FORMAT_FLAG_LINEAR;
                  ++count;
                  if (!compare_images(sizes[s][0], sizes[s][1],
                      formats[f].color_type, depth, format, padding[p],
                      (int)negative))
                  {
                     if (failures < 10)
                        fprintf(stderr,
                            "%ux%u: depth=%d format=%u padding=%d bottom=%u\n",
                            sizes[s][0], sizes[s][1], depth,
                            (unsigned int)format, padding[p], negative);
                     ++failures;
                     ++failures_by_mode[mode];
                  }
               }
   printf("%u comparisons, %u failures\n", count, failures);
   printf("16-to-8 failures: %u; 8-bit control failures: %u; "
       "linear-16 control failures: %u\n", failures_by_mode[0],
       failures_by_mode[1], failures_by_mode[2]);
   return failures != 0;
}

#else

int
main(void)
{
   fprintf(stderr, "pnginterlace: required read/write features are disabled\n");
   return SKIP;
}

#endif
