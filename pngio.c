
/* pngio.c - default functions for data I/O and error/warning messages

   libpng 1.0 beta 3 - version 0.82
   For conditions of distribution and use, see copyright notice in png.h
   Copyright (c) 1995 Guy Eric Schalnat, Group 42, Inc.
   Some portions Copyright (C) 1995 Andreas Dilger
   Sept 24, 1995

   This file provides a location for all input/output.  Users which need
   special handling are expected to write functions which have the same
   arguments as these, and perform similar functions, but possibly have
   different I/O methods.  Note that you shouldn't change these functions,
   but rather write replacement functions and then change them at run
   time with png_set_write_fn(...) or png_set_read_fn(...), etc */

#include "png.h"


/* Write the data to whatever output you are using.  The default
   routine writes to a file pointer.  If you need to write to something
   else, this is a good example of how to do it.  Note that this routine
   sometimes gets called with very small lengths, so you should implement
   some kind of simple buffering if you are using unbuffered writes.  This
   should never be asked to write more then 64K on a 16 bit machine.  The
   cast to png_size_t is there for insurance. */
#ifndef USE_FAR_KEYWORD
static void
png_write_data(png_struct *png_ptr, png_bytef *data, png_uint_32 length)
{
   png_uint_32 check;

   check = fwrite(data, 1, (png_size_t)length, png_ptr->fp);
   if (check != length)
   {
      (*(png_ptr->error_fn))(png_ptr, "Write error");
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
png_write_data(png_struct *png_ptr, png_bytef *data, png_uint_32 length)
{
   png_uint_32 check;
   png_byte *n_data;
    
   /* Check if data really is near. If so, use usual code. */
#ifdef _MSC_VER
   /* do it this way just to quiet warning */
   FP_OFF(n_data) = FP_OFF(data);  
   if(FP_SEG(n_data) == FP_SEG(data))
#else
   /* this works in MSC also but with lost segment warning */
   n_data = (png_byte *)data;  
   if((png_bytef *)n_data == data) 
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
         written = MIN(NEAR_BUF_SIZE,remaining);
         png_memcpy(buf,data,written); /* copy far buffer to near buffer */
         err = fwrite(buf, 1, written, png_ptr->fp);
         if(err != written)
            break;
         else
            check += err;   
         data += written;
         remaining -= written;
      }
      while(remaining != 0);      
   }
   if (check != length)
   {
      (*(png_ptr->error_fn))(png_ptr, "Write error");
   }
}

#endif


/* Read the data from whatever input you are using.  The default
   routine reads from a file pointer.  If you need to read from something
   else, this is a good example of how to do it.  Note that this routine
   sometimes gets called with very small lengths, so you should implement
   some kind of simple buffering if you are using unbuffered reads.  This
   should never be asked to read more then 64K on a 16 bit machine.  The
   cast to png_size_t is there for insurance. */
#ifndef USE_FAR_KEYWORD
static void
png_read_data(png_struct *png_ptr, png_bytef *data, png_uint_32 length)
{
   png_uint_32 check;

   check = fread(data, 1, (size_t)length, png_ptr->fp);
   if (check != length)
   {
      (*(png_ptr->error_fn))(png_ptr, "Read error");
   }
}
#else
static void
png_read_data(png_struct *png_ptr, png_bytef *data, png_uint_32 length)
{
   png_uint_32 check;
   png_byte *n_data;
    
   /* Check if data really is near. If so, use usual code. */
#ifdef _MSC_VER
   /* do it this way just to quiet warning */
   FP_OFF(n_data) = FP_OFF(data);  
   if(FP_SEG(n_data) == FP_SEG(data))
#else
   /* this works in MSC also but with lost segment warning */
   n_data = (png_byte *)data;  
   if((png_bytef *)n_data == data) 
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
         read = MIN(NEAR_BUF_SIZE,remaining);
         err = fread(buf, 1, read, png_ptr->fp);
         png_memcpy(data,buf,read); /* copy far buffer to near buffer */
         if(err != read)
            break;
         else
            check += err;   
         data += read;
         remaining -= read;
      }
      while(remaining != 0);      
   }
   if (check != length)
   {
      (*(png_ptr->error_fn))(png_ptr, "Read error");
   }
}
#endif


/* This function does nothing, and is used for the write function on
   a read, and vice-versa.  It is an error if this function is
   actually called. */
static void
png_empty_rw(png_struct *png_ptr, png_bytef *data, png_uint_32 length)
{
   if (png_ptr->read_data_fn == png_empty_rw)
   {
     (*(png_ptr->error_fn))(png_ptr, "Invalid read on a write device");
   }
   else /* if (png_ptr->write_data_fn == png_empty_rw) */
   {
     (*(png_ptr->error_fn))(png_ptr, "Invalid write on a read device");
   }
}


#if defined(PNG_WRITE_FLUSH_SUPPORTED)
/* This function does nothing, and is supplied for user I/O functions which
   do not do any buffering.  This function is set when NULL is supplied for
   the flush function pointer. */
static void
png_empty_flush(png_struct *png_ptr)
{
}


/* Write out any remaining output that is stored in the output buffers.
   If you supply a new write routine, you will probably need to supply a
   replacement for this routine as well. */
static void
png_output_flush(png_struct *png_ptr)
{
   fflush(png_ptr->fp);
}
#endif /* PNG_WRITE_FLUSH_SUPPORTED */


/* This function does nothing, and is called if the user supplies null
   when setting the error function. */
static void
png_empty_error(png_structf *png_ptr, char *message)
{
#ifdef USE_FAR_KEYWORD
   {
      jmp_buf jmpbuf;
      png_memcpy(jmpbuf,png_ptr->jmpbuf,sizeof(jmp_buf));
      longjmp(jmpbuf, 1);
   }
#else
   longjmp(png_ptr->jmpbuf, 1);
#endif
}


/* This function is called whenever there is an error.  Replace with a
   function you wish to handle the error.  Note that this function
   MUST NOT return, or the program will crash.  To be consistent with
   the examples for the library and this function, you could call
   longjmp(pnt_ptr->jmpbuf) to return to the program at the location of
   the last setjmp(png_ptr->jmpbuf) after an error. */
static void
png_error(png_structf *png_ptr, char *message)
{
   fprintf(stderr, "libpng error: %s\n", message);

   png_empty_error(png_ptr, message);
}


/* This function does nothing, and is called if the user supplies null
   when setting the warning function. */
static void
png_empty_warning(png_struct *png_ptr, char *message)
{
}


/* This function is called when there is a warning, but the library
   thinks it can continue anyway.  You don't have to do anything here
   if you don't want to.  In the default configuration, png_ptr is
   not used, but it is passed in case it may be useful. */
static void
png_warning(png_struct *png_ptr, char *message)
{
   fprintf(stderr, "libpng warning: %s\n", message);
}


/* This function allows the application to supply new output functions for
   libpng if standard C streams aren't being used.  The new write function
   should call (*(png_ptr->error_fn))("Error message") to exit and output
   any fatal error messages if it cannot output the correct number of bytes.
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
png_set_write_fn(png_struct *png_ptr, void *io_ptr, png_rw_ptr write_data_fn,
   png_flush_ptr output_flush_fn)
{
   png_ptr->io_ptr = io_ptr;

   if (write_data_fn == NULL)
   {
      (*(png_ptr->error_fn))(png_ptr, "NULL write function pointer given");
   }

   png_ptr->write_data_fn = write_data_fn;

#if defined(PNG_WRITE_FLUSH_SUPPORTED)
   if (output_flush_fn == NULL)
   {
      png_ptr->output_flush_fn = png_empty_flush;
   }
   else
   {
      png_ptr->output_flush_fn = output_flush_fn;
   }
#endif /* PNG_WRITE_FLUSH_SUPPORTED */

   /* It is an error to write to a read device */
   png_ptr->read_data_fn = png_empty_rw;
}


/* This function allows the application to supply a new input function
   for libpng if standard C streams aren't being used.  The new read function
   should call  (*(png_ptr->error_fn))("Error message") to exit and output
   any fatal error messages if the desired number of bytes is not available.
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

   if (read_data_fn == NULL)
   {
      (*(png_ptr->error_fn))(png_ptr, "NULL read function pointer given");
   }
   png_ptr->read_data_fn = read_data_fn;

   /* It is an error to write to a read device */
   png_ptr->write_data_fn = png_empty_rw;

#if defined(PNG_WRITE_FLUSH_SUPPORTED)
   png_ptr->output_flush_fn = png_empty_flush;
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


/* This function is called when the application wants to use another
   method of handling errors and warnings.  Note that the error function must
   NOT return to the calling routine or serious problems will occur. The
   error return method used in the default routine calls
   longjmp(png_ptr->jmpbuf, 1), or exits if setjmp(png_ptr->jmpbuf) has never
   been called. */
void
png_set_msg_fn(png_struct *png_ptr, void *msg_ptr, png_msg_ptr error_fn,
   png_msg_ptr warning_fn)
{
   png_ptr->msg_ptr = msg_ptr;

   if (error_fn == NULL)
   {
      png_ptr->error_fn = png_empty_error;
   }
   else
   {
      png_ptr->error_fn = error_fn;
   }

   if (warning_fn == NULL)
   {
      png_ptr->warning_fn = png_empty_warning;
   }
   else
   {
      png_ptr->warning_fn = warning_fn;
   }
}


/* This function returns a pointer to the msg_ptr associated with the user
   functions.  The application should free any memory associated with this
   pointer before png_write_destroy and png_read_destroy are called. */
void *
png_get_msg_ptr(png_struct *png_ptr)
{
   return png_ptr->msg_ptr;
}


/* Initialize the default input/output functions for the png file.
   If you change the read, write or message routines, can call
   either png_set_read_fn(...), png_set_write_fn(...), etc. to change
   individual functions after a call to png_init_io(...).   You can also
   call one of png_set_read_fn(...) or png_set_write_fn(...), AND
   png_set_msg_fn(...) instead of png_init_io(...) if you aren't using
   any of the default libpng functions. */
void
png_init_io(png_struct *png_ptr, FILE *fp)
{
   png_ptr->fp = fp;
   png_ptr->error_fn = png_error;
   png_ptr->warning_fn = png_warning;
   png_ptr->write_data_fn = png_write_data;
   png_ptr->read_data_fn = png_read_data;

#if defined(PNG_WRITE_FLUSH_SUPPORTED)
   png_ptr->output_flush_fn = png_output_flush;
#endif /* PNG_WRITE_FLUSH_SUPPORTED */
}
