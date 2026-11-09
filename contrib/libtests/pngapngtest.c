/* pngapngtest.c
 *
 * Copyright (c) 2026 Cosmin Truta
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * Write a multi-frame APNG file and read it back, using a tracked allocator,
 * and verify that every allocation is released again.  Before the fix,
 * png_write_start_row replaced row_buf and prev_row on every frame without
 * releasing the buffers of the previous frame, so the leak grew linearly with
 * the frame count.
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

#define NUM_PTRS 4096

static void *ptrs[NUM_PTRS];
static size_t num_ptrs;
static int failed;

/* Allocation hooks: every block handed to libpng is recorded, and the record
 * is removed again when libpng releases it.
 */
static void *
track_malloc(png_struct *png_ptr, png_alloc_size_t size)
{
   void *p;

   (void)png_ptr;

   if (size == 0)
      size = 1;

   p = malloc((size_t)size);
   if (p != NULL)
   {
      if (num_ptrs >= NUM_PTRS)
      {
         fprintf(stderr, "pngapngtest: too many live allocations\n");
         exit(1);
      }
      ptrs[num_ptrs++] = p;
   }

   return p;
}

static void
track_free(png_struct *png_ptr, void *ptr)
{
   size_t i;

   (void)png_ptr;

   if (ptr == NULL)
      return;

   for (i = 0; i < num_ptrs; ++i)
   {
      if (ptrs[i] == ptr)
      {
         ptrs[i] = ptrs[--num_ptrs];
         free(ptr);
         return;
      }
   }

   fprintf(stderr, "pngapngtest: freeing an untracked pointer\n");
   exit(1);
}

static void
check_leaks(const char *what)
{
   if (num_ptrs != 0)
   {
      fprintf(stderr, "pngapngtest: %s leaked %lu block(s)\n", what,
          (unsigned long)num_ptrs);
      failed = 1;
   }
}

/* In-memory output. */
struct memory_output
{
   png_byte *data;
   size_t size;
   size_t used;
};

static void
output_write(png_struct *png_ptr, png_byte *data, size_t size)
{
   struct memory_output *out = png_get_io_ptr(png_ptr);

   if (out->used + size > out->size)
   {
      size_t want = (out->used + size) * 2 + 1024;
      png_byte *grown = realloc(out->data, want);

      if (grown == NULL)
      {
         fprintf(stderr, "pngapngtest: out of memory\n");
         exit(1);
      }
      out->data = grown;
      out->size = want;
   }

   memcpy(out->data + out->used, data, size);
   out->used += size;
}

static void
output_flush(png_struct *png_ptr)
{
   (void)png_ptr;
}

/* In-memory input. */
struct memory_input
{
   const png_byte *data;
   size_t size;
   size_t offset;
};

static void
input_read(png_struct *png_ptr, png_byte *data, size_t size)
{
   struct memory_input *in = png_get_io_ptr(png_ptr);

   if (size > in->size - in->offset)
      png_error(png_ptr, "read past the end of the test data");

   memcpy(data, in->data + in->offset, size);
   in->offset += size;
}

static int
write_apng(struct memory_output *out)
{
   enum { width = 32, height = 16, num_frames = 3 };
   png_struct *png_ptr;
   png_info *info_ptr;
   png_byte *row = NULL;
   int frame;

   out->data = NULL;
   out->size = 0;
   out->used = 0;

   png_ptr = png_create_write_struct_2(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL,
       NULL, track_malloc, track_free);
   if (png_ptr == NULL)
   {
      fprintf(stderr, "pngapngtest: png_create_write_struct_2 failed\n");
      return 0;
   }

   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      png_destroy_write_struct(&png_ptr, NULL);
      fprintf(stderr, "pngapngtest: png_create_info_struct failed\n");
      return 0;
   }

   if (setjmp(png_jmpbuf(png_ptr)))
   {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      free(row);
      check_leaks("writing the APNG");
      return 0;
   }

   png_set_write_fn(png_ptr, out, output_write, output_flush);
   png_set_IHDR(png_ptr, info_ptr, width, height, 8,
       PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
       PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
   png_set_acTL(png_ptr, info_ptr, num_frames, 0);
   png_write_info(png_ptr, info_ptr);

   row = malloc((size_t)width * 4);
   if (row == NULL)
   {
      fprintf(stderr, "pngapngtest: out of memory\n");
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return 0;
   }

   for (frame = 0; frame < num_frames; ++frame)
   {
      int y;

      png_write_frame_head(png_ptr, info_ptr, NULL, width, height, 0, 0,
          1, 10, PNG_fcTL_DISPOSE_OP_NONE, PNG_fcTL_BLEND_OP_SOURCE);

      for (y = 0; y < height; ++y)
      {
         memset(row, frame * 40 + y, (size_t)width * 4);
         png_write_row(png_ptr, row);
      }

      png_write_frame_tail(png_ptr, info_ptr);
   }

   png_write_end(png_ptr, info_ptr);
   free(row);
   png_destroy_write_struct(&png_ptr, &info_ptr);
   check_leaks("writing the APNG");

   return 1;
}

static int
read_apng(const struct memory_output *out)
{
   png_struct *png_ptr;
   png_info *info_ptr;
   png_byte *row = NULL;
   struct memory_input in;
   png_uint_32 frames;
   png_uint_32 frame;

   in.data = out->data;
   in.size = out->used;
   in.offset = 0;

   png_ptr = png_create_read_struct_2(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL,
       NULL, track_malloc, track_free);
   if (png_ptr == NULL)
   {
      fprintf(stderr, "pngapngtest: png_create_read_struct_2 failed\n");
      return 0;
   }

   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      png_destroy_read_struct(&png_ptr, NULL, NULL);
      fprintf(stderr, "pngapngtest: png_create_info_struct failed\n");
      return 0;
   }

   if (setjmp(png_jmpbuf(png_ptr)))
   {
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      free(row);
      check_leaks("reading the APNG");
      return 0;
   }

   png_set_read_fn(png_ptr, &in, input_read);
   png_read_info(png_ptr, info_ptr);
   frames = png_get_num_frames(png_ptr, info_ptr);

   if (frames != 3)
   {
      fprintf(stderr, "pngapngtest: expected 3 frames, found %lu\n",
          (unsigned long)frames);
      failed = 1;
   }

   row = malloc((size_t)32 * 4);
   if (row == NULL)
   {
      fprintf(stderr, "pngapngtest: out of memory\n");
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      return 0;
   }

   for (frame = 0; frame < frames; ++frame)
   {
      png_uint_32 frame_height;
      png_uint_32 y;

      png_read_frame_head(png_ptr, info_ptr);
      frame_height = png_get_image_height(png_ptr, info_ptr);

      for (y = 0; y < frame_height; ++y)
         png_read_row(png_ptr, row, NULL);
   }

   png_read_end(png_ptr, info_ptr);
   free(row);
   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
   check_leaks("reading the APNG");

   return 1;
}

int
main(void)
{
#if defined(PNG_APNG_SUPPORTED) && defined(PNG_SEQUENTIAL_READ_SUPPORTED) && \
    defined(PNG_WRITE_FILTER_SUPPORTED)
   struct memory_output out;

   printf("Testing APNG frame writing and reading\n");
   fflush(stdout);

   if (write_apng(&out) != 0)
   {
      (void)read_apng(&out);
      free(out.data);
   }

   if (failed != 0)
   {
      fprintf(stderr, "pngapngtest: FAIL\n");
      return 1;
   }

   printf("pngapngtest: PASS\n");
   return 0;
#else
   fprintf(stderr, "pngapngtest: APNG or sequential read not supported\n");
   return 77;
#endif
}
