
/* pngstub.c - stub functions for i/o and memory allocation

   libpng 1.0 beta 2 - version 0.81
   For conditions of distribution and use, see copyright notice in png.h
   Copyright (c) 1995 Guy Eric Schalnat, Group 42, Inc.
   August 24, 1995

   This file provides a location for all input/output.  Users which need
   special handling are expected to modify the code in this file to meet
   their needs.  See the instructions at each function. */

#define PNG_INTERNAL
#include "png.h"

/* Write the data to whatever output you are using.  The default
   routine writes to a file pointer.  If you need to write to something
   else, this is the place to do it.  We suggest saving the old code
   for future use, possibly in a #define.  Note that this routine sometimes
   gets called with very small lengths, so you should implement some kind
   of simple buffering if you are using unbuffered writes.  This should
   never be asked to write more then 64K on a 16 bit machine.  The cast
   to png_size_t is there for insurance, but if you are having problems
   with it, you can take it out.  Just be sure to cast length to whatever
   fwrite needs in that spot if you don't have a function prototype for
   it. */

#ifndef USE_FAR_KEYWORD
void
png_write_data(png_struct *png_ptr, png_bytef *data, png_uint_32 length)
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
#ifndef USE_FAR_KEYWORD
void
png_read_data(png_struct *png_ptr, png_bytef *data, png_uint_32 length)
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
      png_error(png_ptr, "read Error");
   }
}
#endif

/* Initialize the input/output for the png file.  If you change
   the read and write routines, you will probably need to change
   this routine (or write your own).  If you change the parameters
   of this routine, remember to change png.h also. */
void
png_init_io(png_struct *png_ptr, FILE *fp)
{
   png_ptr->fp = fp;
}

