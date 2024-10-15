/* pngwio.c - functions for data output
 *
 * Copyright (c) 2018 Cosmin Truta
 * Copyright (c) 1998-2002,2004,2006-2014,2016,2018 Glenn Randers-Pehrson
 * Copyright (c) 1996-1997 Andreas Dilger
 * Copyright (c) 1995-1996 Guy Eric Schalnat, Group 42, Inc.
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * This file provides a location for all output.  Users who need
 * special handling are expected to write functions that have the same
 * arguments as these and perform similar functions, but that possibly
 * use different output methods.  Note that you shouldn't change these
 * functions, but rather write replacement functions and then change
 * them at run time with png_set_write_fn(...).
 */

#include "pngpriv.h"

#ifdef PNG_WRITE_SUPPORTED

/* Write the data to whatever output you are using.  The default routine
 * writes to a file pointer.  Note that this routine sometimes gets called
 * with very small lengths, so you should implement some kind of simple
 * buffering if you are using unbuffered writes.  This should never be asked
 * to write more than 64K on a 16-bit machine.
 */
static int
invalid(png_const_structrp png_ptr)
{
   if (png_ptr == NULL)
      return 1;

   if ((png_ptr->mode & PNG_IS_READ_STRUCT) != 0)
   {
      png_app_error(png_ptr, "API: invalid in read");
      return 1;
   }

   return 0;
}

void /* PRIVATE */
png_write_data(png_structrp png_ptr, png_const_bytep data, size_t length)
{
   if (invalid(png_ptr))
      return;

   /* NOTE: write_data_fn must not change the buffer! */
   if (png_ptr->write_data_fn != NULL )
      png_ptr->write_data_fn(png_ptr, png_constcast(png_bytep, data), length);

   else
      png_app_error(png_ptr, "API: no write function");
}

#ifdef PNG_STDIO_SUPPORTED
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

/* A C stream (FILE*) implementation of png_write_data. */
void PNGCBAPI
png_stdio_write(png_structp png_ptr, png_bytep data, size_t length)
{
   if (!invalid_stdio(png_ptr))
   {
      if (png_ptr->fwrite != NULL)
      {
         size_t written =
            (png_ptr->fwrite)(data, 1U, length, png_ptr->stdio_ptr);

         if (written != length)
            png_error(png_ptr, "C stdio: write error");
      }
      else /* This should be impossible: */
         png_error(png_ptr, "API(internal): missing fwrite");
   }
}
#endif /* STDIO */

#ifdef PNG_WRITE_FLUSH_SUPPORTED
/* If 'output_flush_fn' has been set it is called here to flush any pending
 * data.  If it is not set this does nothing apart from validating that this is
 * a write png_struct.
 */
void /* PRIVATE */
png_flush(png_structrp png_ptr)
{
   if (!invalid(png_ptr) && png_ptr->output_flush_fn != NULL)
      (*(png_ptr->output_flush_fn))(png_ptr);
}

#  ifdef PNG_STDIO_SUPPORTED
void PNGCBAPI
png_stdio_flush(png_structp png_ptr)
{
   if (!invalid_stdio(png_ptr))
   {
      if (png_ptr->fflush != NULL)
      {
         if ((png_ptr->fflush)(png_ptr->stdio_ptr))
            png_error(png_ptr, "C stdio: write error");
      }
      else /* This should be impossible */
         png_error(png_ptr, "API(internal): missing fflush");
   }
}
#  endif /* STDIO */
#endif /* WRITE_FLUSH */

/* This API is an alternative to png_init_io (see png.c) which must be used if
 * the caller of libpng is using something other than FILE* as the output device
 * when writing a PNG.
 *
 * If 'output_flush_fn' is NULL and WRITE_FLUSH is supported no ffflush
 * operations will be done.  The caller must ensure any buffers used by the
 * caller's 'write_data_fn' are flushed appropriately.
 */
void PNGAPI
png_set_write_fn(png_structrp png_ptr, png_voidp io_ptr,
    png_rw_ptr write_data_fn, png_flush_ptr output_flush_fn)
{
   if (invalid(png_ptr))
      return;

#  ifdef PNG_STDIO_SUPPORTED
      /* Ensure none of the stdio settings remain set: */
      png_ptr->stdio_ptr = NULL;
#     ifdef PNG_SEQUENTIAL_READ_SUPPORTED
         png_ptr->fread = NULL;
#     endif
      png_ptr->fwrite = NULL;
#     ifdef PNG_WRITE_FLUSH_SUPPORTED
         png_ptr->fflush = NULL;
#     endif
#  endif /* STDIO */

#  ifdef PNG_READ_SUPPORTED
      png_ptr->read_data_fn = NULL;
#  endif

   png_ptr->write_data_fn = write_data_fn;

#  ifdef PNG_WRITE_FLUSH_SUPPORTED
      png_ptr->output_flush_fn = output_flush_fn;
#  else
      PNG_UNUSED(output_flush_fn)
#  endif /* WRITE_FLUSH */

   png_ptr->io_ptr = io_ptr;
}
#endif /* WRITE */
