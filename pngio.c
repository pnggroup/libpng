
/* pngstub.c - stub functions for i/o and memory allocation

	libpng 1.0 beta 2 - version 0.85
   For conditions of distribution and use, see copyright notice in png.h
   Copyright (c) 1995 Guy Eric Schalnat, Group 42, Inc.
   December 19, 1995

   This file provides a location for all input/output.  Users which need
	special handling are expected to write functions which have the same
	arguments as these, and perform similar functions, but possibly have
	different I/O methods.  Note that you shouldn't change these functions,
	but rather write replacement functions and then change them at run
	time with png_set_write_fn(...) or png_set_read_fn(...), etc */

#define PNG_INTERNAL
#include "png.h"

/* Write the data to whatever output you are using.  The default
   routine writes to a file pointer.  If you need to write to something
	else, this is a good example of how to do it.  Note that this routine
	sometimes gets called with very small lengths, so you should implement
	some kind of simple buffering if you are using unbuffered writes.  This
	should never be asked to write more then 64K on a 16 bit machine.  The
	cast to png_size_t is there for insurance. */

void
png_write_data(png_structp png_ptr, png_bytep data, png_uint_32 length)
{
	if (png_ptr->write_data_fn)
		(*(png_ptr->write_data_fn))(png_ptr, data, length);
	else
		png_error(png_ptr, "Call to NULL write function");
}

#ifndef USE_FAR_KEYWORD
void
png_default_write_data(png_structp png_ptr, png_bytep data, png_uint_32 length)
{
   png_uint_32 check;

   check = fwrite(data, 1, (png_size_t)length, png_ptr->fp);
   if (check != length)
   {
      png_error(png_ptr, "Write Error");
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

void
png_default_write_data(png_structp png_ptr, png_bytep data, png_uint_32 length)
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
      check = fwrite(n_data, 1, (png_size_t)length, png_ptr->fp);
   }
	else
   {
      png_byte buf[NEAR_BUF_SIZE];
      png_size_t written, remaining, err;
      check = 0;
      remaining = (png_size_t)length;
		do
		{
			written = MIN(NEAR_BUF_SIZE, remaining);
			png_memcpy(buf, data, written); /* copy far buffer to near buffer */
         err = fwrite(buf, 1, written, png_ptr->fp);
			if (err != written)
            break;
			else
            check += err;
         data += written;
         remaining -= written;
      }
		while (remaining != 0);
	}
	if (check != length)
	{
		png_error(png_ptr, "Write Error");
	}
}

#endif

/* Read the data from whatever input you are using.  The default
   routine reads from a file pointer.  If you need to read from something
   else, this is the place to do it.  We suggest saving the old code
   for future use.  Note that this routine sometimes gets called with
   very small lengths, so you should implement some kind of simple
   buffering if you are using unbuffered reads.  This should
   never be asked to read more then 64K on a 16 bit machine.  The cast
	to png_size_t is there for insurance, but if you are having problems
	with it, you can take it out.  Just be sure to cast length to whatever
	fread needs in that spot if you don't have a function prototype for
	it. */
void
png_read_data(png_structp png_ptr, png_bytep data, png_uint_32 length)
{
#ifdef PNG_PROGRESSIVE_READ_SUPPORTED
	if (png_ptr->read_mode == PNG_READ_PUSH_MODE)
	{
   	png_push_fill_buffer(png_ptr, data, length);
	}
   else
#endif
	{
		if (png_ptr->read_data_fn)
			(*(png_ptr->read_data_fn))(png_ptr, data, length);
		else
			png_error(png_ptr, "Call to NULL read function");
	}
}

#ifndef USE_FAR_KEYWORD
void
png_default_read_data(png_structp png_ptr, png_bytep data, png_uint_32 length)
{
   png_uint_32 check;

   check = fread(data, 1, (size_t)length, png_ptr->fp);
   if (check != length)
   {
      png_error(png_ptr, "Read Error");
   }
}
#else
void
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
   if ((PNG_BYTEP )n_data == data)
#endif
   {
      check = fread(n_data, 1, (size_t)length, png_ptr->fp);
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
         err = fread(buf, 1, read, png_ptr->fp);
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

#if defined(PNG_WRITE_FLUSH_SUPPORTED)
void
png_flush(png_struct *png_ptr)
{
	if (png_ptr->output_flush_fn)
		(*(png_ptr->output_flush_fn))(png_ptr);
}

void
png_default_flush(png_struct *png_ptr)
{
	if (png_ptr->fp)
		fflush(png_ptr->fp);
}
#endif

/* This function allows the application to supply new output functions for
	libpng if standard C streams aren't being used.

	This function takes as its arguments:
	png_ptr       - pointer to a png output data structure
	io_ptr        - pointer to user supplied structure containing info about
						 the output functions.  May be NULL.
	write_data_fn - pointer to a new output function which takes as its
						 arguments a pointer to a png_struct, a pointer to
						 data to be written, and a 32-bit unsigned int which is
						 the number of bytes to be written.  The new write
						 function should call (*(png_ptr->error_fn))("Error msg")
						 to exit and output any fatal error messages.
	flush_data_fn - pointer to a new flush function which takes as its
						 arguments a pointer to a png_struct.  After a call to
						 the flush function, there should be no data in any buffers
						 or pending transmission.  If the output method doesn't do
						 any buffering of ouput, this parameter can be NULL.  If
						 PNG_WRITE_FLUSH_SUPPORTED is not defined at libpng
						 compile time, output_flush_fn will be ignored, although
						 it must be supplied for compatibility. */
void
png_set_write_fn(png_structp png_ptr, png_voidp io_ptr, png_rw_ptr write_data_fn,
	png_flush_ptr output_flush_fn)
{
	png_ptr->io_ptr = io_ptr;

	if (write_data_fn)
		png_ptr->write_data_fn = write_data_fn;
	else
		png_ptr->write_data_fn = png_default_write_data;

#if defined(PNG_WRITE_FLUSH_SUPPORTED)
	if (output_flush_fn == NULL)
		png_ptr->output_flush_fn = png_default_flush;
	else
		png_ptr->output_flush_fn = output_flush_fn;
#endif /* PNG_WRITE_FLUSH_SUPPORTED */

	/* It is an error to read while writing a png file */
	png_ptr->read_data_fn = NULL;
}


/* This function allows the application to supply a new input function
	for libpng if standard C streams aren't being used.

	This function takes as its arguments:
	png_ptr      - pointer to a png input data structure
	io_ptr       - pointer to user supplied structure containing info about
						the input functions.  May be NULL.
	read_data_fn - pointer to a new input function which takes as it's
						arguments a pointer to a png_struct, a pointer to
						a location where input data can be stored, and a 32-bit
						unsigned int which is the number of bytes to be read. */
void
png_set_read_fn(png_struct *png_ptr, void *io_ptr, png_rw_ptr read_data_fn)
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


/* This function returns a pointer to the io_ptr associated with the user
	functions.  The application should free any memory associated with this
	pointer before png_write_destroy and png_read_destroy are called. */
void *
png_get_io_ptr(png_struct *png_ptr)
{
	return png_ptr->io_ptr;
}

/* Initialize the input/output for the png file.  If you change
	the read and write routines, you will probably need to change
   this routine (or write your own).  If you change the parameters
   of this routine, remember to change png.h also. */
void
png_init_io(png_structp png_ptr, FILE *fp)
{
	png_ptr->fp = fp;
	png_ptr->read_data_fn = png_default_read_data;
	png_ptr->write_data_fn = png_default_write_data;
#ifdef PNG_WRITE_FLUSH_SUPPORTED
	png_ptr->output_flush_fn = png_default_flush;
#endif
}

