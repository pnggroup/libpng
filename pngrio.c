
/* pngrio.c - functions for data input

   libpng 1.0 beta 3 - version 0.89
   For conditions of distribution and use, see copyright notice in png.h
   Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.
   May 25, 1996

   This file provides a location for all input.  Users which need
   special handling are expected to write a function which has the same
   arguments as this, and perform a similar function, but possibly has
   a different input method.  Note that you shouldn't change this
   function, but rather write a replacement function and then make
   libpng use it at run time with png_set_read_fn(...) */

#define PNG_INTERNAL
#include "png.h"

/* Read the data from whatever input you are using.  The default routine
   reads from a file pointer.  Note that this routine sometimes gets called
   with very small lengths, so you should implement some kind of simple
   buffering if you are using unbuffered reads.  This should never be asked
   to read more then 64K on a 16 bit machine.  The cast to png_size_t is
   there to quiet some compilers */
void
png_read_data(png_structp png_ptr, png_bytep data, png_uint_32 length)
{
   if (png_ptr->read_data_fn)
      (*(png_ptr->read_data_fn))(png_ptr, data, length);
   else
      png_error(png_ptr, "Call to NULL read function");
}

/* This is the function which does the actual reading of data.  If you are
   not reading from a standard C stream, you should create a replacement
   read_data function and use it at run time with png_set_read_fn(), rather
   than changing the library. */
#ifndef USE_FAR_KEYWORD
static void
png_default_read_data(png_structp png_ptr, png_bytep data, png_uint_32 length)
{
   png_uint_32 check;

   check = fread(data, 1, (size_t)length, (FILE *)png_ptr->io_ptr);
   if (check != length)
   {
      png_error(png_ptr, "Read Error");
   }
}
#else
/* this is the model-independent version. Since the standard I/O library
   can't handle far buffers in the medium and small models, we have to copy
   the data.
*/
 
#define NEAR_BUF_SIZE 1024
#define MIN(a,b) (a <= b ? a : b)
 
#ifdef _MSC_VER
/* for FP_OFF */
#include <dos.h>
#endif
 
static void
png_default_read_data(png_structp png_ptr, png_bytep data, png_uint_32 length)
{
   png_uint_32 check;
   png_byte *n_data;

   /* Check if data really is near. If so, use usual code. */
#ifdef _MSC_VER
   /* do it this way just to quiet warning */
   FP_OFF(n_data) = FP_OFF(data);
   if (FP_SEG(n_data) == FP_SEG(data))
#else
   /* this works in MSC also but with lost segment warning */
   n_data = (png_byte *)data;
   if ((png_bytep)n_data == data)
#endif
   {
      check = fread(n_data, 1, (size_t)length, (FILE *)png_ptr->io_ptr);
   }
   else
   {
      png_byte buf[NEAR_BUF_SIZE];
      png_size_t read, remaining, err;
      check = 0;
      remaining = (png_size_t)length;
      do
      {
         read = MIN(NEAR_BUF_SIZE, remaining);
         err = fread(buf, 1, read, (FILE *)png_ptr->io_ptr);
         png_memcpy(data, buf, read); /* copy far buffer to near buffer */
         if(err != read)
            break;
         else
            check += err;
         data += read;
         remaining -= read;
      }
      while (remaining != 0);
   }
   if (check != length)
   {
      png_error(png_ptr, "read Error");
   }
}
#endif

/* This function allows the application to supply a new input function
   for libpng if standard C streams aren't being used.

   This function takes as its arguments:
   png_ptr      - pointer to a png input data structure
   io_ptr       - pointer to user supplied structure containing info about
                  the input functions.  May be NULL.
   read_data_fn - pointer to a new input function which takes as it's
                  arguments a pointer to a png_struct, a pointer to
                  a location where input data can be stored, and a 32-bit
                  unsigned int which is the number of bytes to be read.
                  To exit and output any fatal error messages the new write
                  function should call png_error(png_ptr, "Error msg"). */
void
png_set_read_fn(png_structp png_ptr, png_voidp io_ptr,
   png_rw_ptr read_data_fn)
{
   png_ptr->io_ptr = io_ptr;

   if (read_data_fn)
      png_ptr->read_data_fn = read_data_fn;
   else
      png_ptr->read_data_fn = png_default_read_data;

   /* It is an error to write to a read device */
   png_ptr->write_data_fn = NULL;

#if defined(PNG_WRITE_FLUSH_SUPPORTED)
   png_ptr->output_flush_fn = NULL;
#endif /* PNG_WRITE_FLUSH_SUPPORTED */
}

