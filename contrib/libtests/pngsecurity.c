/* pngsecurity.c — verify that PNG_SECURITY_CHECK catches row buffer overflows.
 *
 * Copyright (c) 2026 the libpng contributors.
 * For conditions of distribution and use, see the disclaimer and license
 * in png.h.
 *
 * This test creates minimal PNG images using the write API, then reads
 * them back with a deliberately corrupted row_buf_capacity to simulate
 * a max_pixel_depth miscalculation.  The PNG_SECURITY_CHECK macro in the
 * decode pipeline must detect the overflow attempt and call png_error()
 * before any out-of-bounds write occurs.
 *
 * Because the test accesses png_struct internals it must be compiled as
 * part of the library build (include pngpriv.h, link png_static).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

/* Internal header — gates pngstruct.h for row_buf_capacity access. */
#include "pngpriv.h"

/* ---------- error / warning handlers ---------- */

static jmp_buf jump_buf;
static int error_caught;
static char error_msg[256];

static void
test_error_fn(png_structp png_ptr, png_const_charp msg)
{
   (void)png_ptr;
   error_caught = 1;
   strncpy(error_msg, msg, sizeof(error_msg) - 1);
   error_msg[sizeof(error_msg) - 1] = '\0';
   longjmp(jump_buf, 1);
}

static void
test_warning_fn(png_structp png_ptr, png_const_charp msg)
{
   (void)png_ptr;
   (void)msg;
}

/* ---------- in-memory PNG write / read ---------- */

struct mem_buf {
   png_byte *data;
   size_t    size;
   size_t    capacity;
   size_t    read_pos;
};

static void
mem_write_fn(png_structp png_ptr, png_bytep data, size_t length)
{
   struct mem_buf *buf = (struct mem_buf *)png_get_io_ptr(png_ptr);
   if (buf->size + length > buf->capacity)
   {
      size_t new_cap = (buf->capacity == 0) ? 4096 : buf->capacity * 2;
      while (new_cap < buf->size + length)
         new_cap *= 2;
      buf->data = (png_byte *)realloc(buf->data, new_cap);
      if (buf->data == NULL)
         png_error(png_ptr, "realloc failed");
      buf->capacity = new_cap;
   }
   memcpy(buf->data + buf->size, data, length);
   buf->size += length;
}

static void
mem_flush_fn(png_structp png_ptr)
{
   (void)png_ptr;
}

static void
mem_read_fn(png_structp png_ptr, png_bytep out, size_t count)
{
   struct mem_buf *buf = (struct mem_buf *)png_get_io_ptr(png_ptr);
   if (buf->read_pos + count > buf->size)
   {
      png_error(png_ptr, "read past end of memory buffer");
      return;
   }
   memcpy(out, buf->data + buf->read_pos, count);
   buf->read_pos += count;
}

/* ---------- PNG generation ---------- */

/* Write a minimal width x 1 grayscale PNG into buf. */
static int
generate_gray_png(struct mem_buf *buf, unsigned int width)
{
   png_structp png_ptr;
   png_infop info_ptr;
   png_byte *row;
   unsigned int i;

   buf->size = 0;
   buf->read_pos = 0;

   row = (png_byte *)malloc(width);
   if (row == NULL) return 0;
   for (i = 0; i < width; i++)
      row[i] = (png_byte)(i & 0xFF);

   png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                     NULL, test_error_fn, test_warning_fn);
   if (png_ptr == NULL) { free(row); return 0; }

   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      png_destroy_write_struct(&png_ptr, NULL);
      free(row);
      return 0;
   }

   if (setjmp(jump_buf))
   {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      free(row);
      return 0;
   }

   png_set_write_fn(png_ptr, buf, mem_write_fn, mem_flush_fn);
   png_set_IHDR(png_ptr, info_ptr, width, 1, 8, PNG_COLOR_TYPE_GRAY,
                PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
                PNG_FILTER_TYPE_DEFAULT);
   png_write_info(png_ptr, info_ptr);
   png_write_row(png_ptr, row);
   png_write_end(png_ptr, info_ptr);
   png_destroy_write_struct(&png_ptr, &info_ptr);
   free(row);
   return 1;
}

/* ---------- test cases ---------- */

/* Decode normally — must succeed. */
static int
test_normal_decode(struct mem_buf *buf)
{
   png_structp png_ptr;
   png_infop info_ptr;
   png_byte row[256];
   png_bytep row_ptrs[1];

   row_ptrs[0] = row;
   buf->read_pos = 0;

   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                    NULL, test_error_fn, test_warning_fn);
   if (png_ptr == NULL) return 0;

   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      png_destroy_read_struct(&png_ptr, NULL, NULL);
      return 0;
   }

   error_caught = 0;
   if (setjmp(jump_buf))
   {
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      return 0;
   }

   png_set_read_fn(png_ptr, buf, mem_read_fn);
   png_read_info(png_ptr, info_ptr);
   png_read_image(png_ptr, row_ptrs);
   png_read_end(png_ptr, info_ptr);
   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
   return 1;
}

/* Decode with corrupted row_buf_capacity — must fire PNG_SECURITY_CHECK. */
static int
test_capacity_check(struct mem_buf *buf, const char *expected_substr)
{
   png_structp png_ptr;
   png_infop info_ptr;
   png_byte row[256];
   int caught;

   buf->read_pos = 0;

   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                    NULL, test_error_fn, test_warning_fn);
   if (png_ptr == NULL) return 0;

   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      png_destroy_read_struct(&png_ptr, NULL, NULL);
      return 0;
   }

   error_caught = 0;
   caught = 0;
   if (setjmp(jump_buf))
   {
      if (error_caught && strstr(error_msg, expected_substr) != NULL)
         caught = 1;
      else
         fprintf(stderr, "  unexpected error: \"%s\"\n", error_msg);

      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      return caught;
   }

   png_set_read_fn(png_ptr, buf, mem_read_fn);
   png_read_info(png_ptr, info_ptr);
   png_read_update_info(png_ptr, info_ptr);

   /* Corrupt row_buf_capacity — simulates a max_pixel_depth miscalculation
    * that allocated a buffer too small for the actual row data.
    */
   png_ptr->row_buf_capacity = 0;

   /* Should trigger PNG_SECURITY_CHECK before any write to row_buf. */
   png_read_row(png_ptr, row, NULL);

   /* If we reach here the check did not fire. */
   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
   return 0;
}

/* ---------- main ---------- */

int
main(void)
{
   struct mem_buf buf;
   int pass = 0;
   int fail = 0;

   memset(&buf, 0, sizeof(buf));

   /* Generate a 4-pixel-wide grayscale test image. */
   if (!generate_gray_png(&buf, 4))
   {
      fprintf(stderr, "FAIL: could not generate test PNG\n");
      return 1;
   }

   /* Test 1 — normal decode must succeed. */
   printf("  sequential read (normal)  ... ");
   fflush(stdout);
   if (test_normal_decode(&buf))
   {
      printf("PASS\n");
      pass++;
   }
   else
   {
      printf("FAIL\n");
      fail++;
   }

   /* Test 2 — corrupted capacity must be caught. */
   printf("  sequential read (capacity=0) ... ");
   fflush(stdout);
   if (test_capacity_check(&buf, "row buffer capacity"))
   {
      printf("PASS\n");
      pass++;
   }
   else
   {
      printf("FAIL\n");
      fail++;
   }

   free(buf.data);

   printf("\npngsecurity: %d passed, %d failed\n", pass, fail);
   return fail != 0;
}
