/* pngimage.c
 *
 * Copyright (c) 2014 John Cunningham Bowler
 *
 * Last changed in libpng 1.6.9 [January 30, 2014]
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * Test the png_read_png and png_write_png interfaces.  Given a PNG file load it
 * using png_read_png and then write with png_write_png.  Test all possible
 * transforms.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#if defined(HAVE_CONFIG_H) && !defined(PNG_NO_CONFIG_H)
#  include <config.h>
#endif

/* Define the following to use this test against your installed libpng, rather
 * than the one being built here:
 */
#ifdef PNG_FREESTANDING_TESTS
#  include <png.h>
#else
#  include "../../png.h"
#endif

#if defined(PNG_INFO_IMAGE_SUPPORTED) && defined(PNG_READ_SUPPORTED)
#ifdef PNG_WRITE_SUPPORTED
/* File data, held in a linked list of buffers - not all of these are in use. */
struct buffer_list
{
   struct buffer_list *next;         /* next buffer in list */
   png_byte            buffer[1024]; /* the actual buffer */
};

struct buffer
{
   struct buffer_list  *last;      /* last buffer in use */
   size_t               end_count; /* bytes in the last buffer */
   struct buffer_list   first;     /* the very first buffer */
};

static struct buffer *
get_buffer(png_structp png_ptr)
{
   return (struct buffer*)png_get_io_ptr(png_ptr);
}

static void PNGCBAPI
write_function(png_structp png_ptr, png_bytep data, png_size_t size)
{
   static struct buffer write_buffer; /* Preallocated list for write */
   struct buffer *buffer = get_buffer(png_ptr);

   if (buffer == NULL)
   {
      /* This is the first write for this PNG, use the global buffer but rewind
       * it to the start of the buffer list:
       */
      write_buffer.last = &write_buffer.first;
      write_buffer.end_count = 0;

      buffer = &write_buffer;
      png_set_write_fn(png_ptr, buffer, write_function, NULL/*flush*/);
   }

   /* Write the data into the buffer, adding buffers as required */
   while (size > 0)
   {
      struct buffer_list *last = buffer->last;
      size_t avail;

      if (buffer->end_count >= sizeof last->buffer)
      {

         if (last->next == NULL)
         {
            struct buffer_list *add =
               (struct buffer_list*)malloc(sizeof *add);

            if (add == NULL)
               png_error(png_ptr, "pngimage: out of memory buffering output");

            add->next = NULL;
            last->next = add;
         }

         last = last->next;
         buffer->last = last;
         buffer->end_count = 0;
      }

      avail = (sizeof last->buffer) - buffer->end_count;
      if (avail > size)
         avail = size;

      memcpy(last->buffer + buffer->end_count, data, avail);
      buffer->end_count -= avail;
      size -= avail;
      data += avail;
   }
}

static int
write_png(png_infop info_ptr, int transforms)
{
   png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,0,0,0);

   if (png_ptr == NULL)
   {
      fprintf(stderr, "png_image: failed to create write png_struct\n");
      return 0;
   }

   if (setjmp(png_jmpbuf(png_ptr)))
   {
      png_destroy_write_struct(&png_ptr, NULL);
      return 0;
   }

   /* Passing NULL for io_ptr causes write_function to initialize the buffer on
    * the first write.
    */
   png_set_write_fn(png_ptr, NULL/*io_ptr*/, write_function, NULL/*flush*/);

#  ifdef PNG_SET_USER_LIMITS_SUPPORTED
      /* Remove the user limits, if any */
      png_set_user_limits(png_ptr, 0x7fffffff, 0x7fffffff);
#  endif

   png_write_png(png_ptr, info_ptr, transforms, NULL/*params*/);

   png_destroy_write_struct(&png_ptr, NULL);
   return 1; /* success */
}

static void
do_write_tests(png_infop info_ptr, int read_transforms)
{
   /* TODO: fix this, it currently crashes (probably because of the re-use of
    * the info_ptr!)
    */
   if (!write_png(info_ptr, 0/*transform*/))
      exit(1);

   (void)read_transforms;
}
#endif

static int
read_png(FILE *fp /*input image*/, png_bytepp rows /*may be NULL*/,
   int transforms, int test_write)
{
   png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,0,0,0);
   png_infop info_ptr = NULL;

   if (png_ptr == NULL)
      return 0;

   if (setjmp(png_jmpbuf(png_ptr)))
   {
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      return 0;
   }

#  ifdef PNG_SET_USER_LIMITS_SUPPORTED
      /* Remove the user limits, if any */
      png_set_user_limits(png_ptr, 0x7fffffff, 0x7fffffff);
#  endif

#  ifdef PNG_STDIO_SUPPORTED
      rewind(fp);
      png_init_io(png_ptr, fp);
#  else
      fprintf(stderr, "png_image: no stdio support, test skipped\n");
      exit(77);
#  endif

   /* The png_read_png API requires us to make the info struct, but it does the
    * call to png_read_info.
    */
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
      png_error(png_ptr, "OOM allocating info structure");

   /* The test uses either local allocation of the info structure or allocation
    * by libpng depending on the passed in row pointer buffer.
    */
   if (rows != NULL)
      png_set_rows(png_ptr, info_ptr, rows);

   png_read_png(png_ptr, info_ptr, transforms, NULL);

   /* Now get the rows if necessary. */
   if (rows == NULL)
   {
      rows = png_get_rows(png_ptr, info_ptr);
      if (rows == NULL)
         png_error(png_ptr, "pngimage: no image allocated");
   }

   if (test_write)
   {
#     ifdef PNG_WRITE_SUPPORTED
         /* This just exits on error */
         do_write_tests(info_ptr, transforms);
#     endif
   }

   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
   return 1;
}

int
main(int argc, const char * const *argv)
{
   /* For each file on the command line test it with a range of transforms */
   int i;

   for (i=1; i<argc;)
   {
      const char *name = argv[i++];
      FILE *fp = fopen(name, "rb");
      unsigned int transform;

      if (fp == NULL)
      {
         fprintf(stderr, "pngimage: %s: open failed (%s)\n", name,
            strerror(errno));
         exit(99/*hard fail, not libpng*/);
      }

      /* There are 65536 possible combinations.  For the moment test all of them
       * (this needs to be reduced for a practical test!)
       */
      for (transform=0; transform<0xffff; ++transform)
         if (!read_png(fp, NULL/*rows*/, (int)/*SAFE*/transform, 1/*write*/))
         {
            fprintf(stderr, "pngimage: %s: %x: transform failed\n", name,
               transform);
            /* Exit on the first detected error */
            exit(1);
         }

      fclose(fp);
   }

   /* Here on success */
   return 0;
}
#else /* !PNG_INFO_IMAGE_SUPPORTED || !PNG_READ_SUPPORTED */
int
main(void)
{
   fprintf(stderr, "pngimage: no support for png_read/write_image\n");
   return 77;
}
#endif
