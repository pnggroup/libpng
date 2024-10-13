/* pngrio.c - functions for data input
 *
 * Copyright (c) 2018 Cosmin Truta
 * Copyright (c) 1998-2002,2004,2006-2016,2018 Glenn Randers-Pehrson
 * Copyright (c) 1996-1997 Andreas Dilger
 * Copyright (c) 1995-1996 Guy Eric Schalnat, Group 42, Inc.
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * This file provides a location for all input.  Users who need
 * special handling are expected to write a function that has the same
 * arguments as this and performs a similar function, but that possibly
 * has a different input method.  Note that you shouldn't change this
 * function, but rather write a replacement function and then make
 * libpng use it at run time with png_set_read_fn(...).
 */

#include "pngpriv.h"

#ifdef PNG_READ_SUPPORTED

/* Read the data from whatever input you are using.  The default routine
 * reads from a file pointer.  Note that this routine sometimes gets called
 * with very small lengths, so you should implement some kind of simple
 * buffering if you are using unbuffered reads.  This should never be asked
 * to read more than 64K on a 16-bit machine.
 */
static int
invalid(png_const_structrp png_ptr)
{
   if (png_ptr == NULL)
      return 1;

   if ((png_ptr->mode & PNG_IS_READ_STRUCT) == 0)
   {
      png_app_error(png_ptr, "API: invalid in write");
      return 1;
   }

   return 0;
}

void /* PRIVATE */
png_read_data(png_structrp png_ptr, png_bytep data, size_t length)
{
   if (invalid(png_ptr))
      return;

   if (png_ptr->read_data_fn != NULL)
      png_ptr->read_data_fn(png_ptr, data, length);

   else
      png_app_error(png_ptr, "API: no read function");
}

#if defined(PNG_STDIO_SUPPORTED) && defined (PNG_SEQUENTIAL_READ_SUPPORTED)
/* This is the function that does the actual reading of data.  If you are
 * not reading from a standard C stream, you should create a replacement
 * read_data function and use it at run time with png_set_read_fn(), rather
 * than changing the library.
 */
static int
invalid_stdio(png_const_structrp png_ptr)
{
   if (invalid(png_ptr))
      return 1;

   if (png_ptr->stdio_ptr == NULL)
   {
      png_app_error(png_ptr, "API: C stdio: no (FILE*)");
      return 1;
   }

   return 0;
}

void PNGCBAPI
png_stdio_read(png_structp png_ptr, png_bytep data, size_t length)
{
   if (!invalid_stdio(png_ptr))
   {
      if (png_ptr->fread != NULL)
      {
         size_t read =
            (png_ptr->fread)(data, 1U, length, png_ptr->stdio_ptr);

         if (read != length)
            png_error(png_ptr, "C stdio: read error");
      }
      else /* This should be impossible: */
         png_error(png_ptr, "API(internal): missing fread");
   }
}
#endif /* STDIO && SEQUENTIAL_READ */

/* This API is an alternative to png_init_io (see png.c) which must be used if
 * the caller of libpng is using something other than FILE* as the input device
 * when reading a PNG.
 */
void PNGAPI
png_set_read_fn(png_structrp png_ptr, png_voidp io_ptr, png_rw_ptr read_data_fn)
{
   if (invalid(png_ptr))
      return;

#  ifdef PNG_STDIO_SUPPORTED
      /* Ensure none of the stdio settings remain set: */
      png_ptr->stdio_ptr = NULL;

#     ifdef PNG_SEQUENTIAL_READ_SUPPORTED
         png_ptr->fread = NULL;
#     endif /* SEQUENTIAL_READ */
#     ifdef PNG_WRITE_SUPPORTED
         png_ptr->fwrite = NULL;
#     endif /* WRITE */
#     ifdef PNG_WRITE_FLUSH_SUPPORTED
         png_ptr->fflush = NULL;
#     endif /* WRITE_FLUSH */
#  endif /* STDIO */

   png_ptr->io_ptr = io_ptr;
   png_ptr->read_data_fn = read_data_fn;

#  ifdef PNG_WRITE_SUPPORTED
      png_ptr->write_data_fn = NULL;
#  endif /* WRITE */
#  ifdef PNG_WRITE_FLUSH_SUPPORTED
      png_ptr->output_flush_fn = NULL;
#  endif /* WRITE_FLUSH */
}
#endif /* READ */
