
/* pngstub.c - stub functions for i/o and memory allocation

   libpng 1.0 beta 1 - version 0.71
   For conditions of distribution and use, see copyright notice in png.h
   Copyright (c) 1995 Guy Eric Schalnat, Group 42, Inc.
   June 26, 1995

   This file provides a location for all input/output, memory location,
   and error handling.  Users which need special handling in these areas
   are expected to modify the code in this file to meet their needs.  See
   the instructions at each function. */

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
void
png_write_data(png_struct *png_ptr, png_byte *data, png_uint_32 length)
{
   png_uint_32 check;

   check = fwrite(data, 1, (png_size_t)length, png_ptr->fp);
   if (check != length)
   {
      png_error(png_ptr, "Write Error");
   }
}

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
png_read_data(png_struct *png_ptr, png_byte *data, png_uint_32 length)
{
   png_uint_32 check;

   check = fread(data, 1, (size_t)length, png_ptr->fp);
   if (check != length)
   {
      png_error(png_ptr, "Read Error");
   }
}

/* Initialize the input/output for the png file.  If you change
   the read and write routines, you will probably need to change
   this routine (or write your own).  If you change the parameters
   of this routine, remember to change png.h also. */
void
png_init_io(png_struct *png_ptr, FILE *fp)
{
   png_ptr->fp = fp;
}

/* Allocate memory.  For reasonable files, size should never exceed
   64K.  However, zlib may allocate more then 64K if you don't tell
   it not to.  See zconf.h and png.h for more information. zlib does
   need to allocate exactly 64K, so whatever you call here must
   have the ability to do that. */

/* Borland compilers have this habit of not giving you 64K chunks
   that start on the segment in DOS mode.  This has not been observed
   in Windows, and of course it doesn't matter in 32 bit mode, as there
   are no segments.  Now libpng doesn't need that much memory normally,
   but zlib does, so we have to normalize it, if necessary.  It would be
   better if zlib worked in less then 64K, but it doesn't, so we
   have to deal with it.  Truely, we are misusing farmalloc here,
   as it is designed for use with huge pointers, which don't care
   about segments.  So we allocate a large amount of memory, and
   divvy off segments when needed.
   */
#ifdef __TURBOC__
#ifndef __WIN32__

/* NUM_SEG is the number of segments allocated at once */
#define NUM_SEG 4
typedef struct borland_seg_struct
{
   void *mem_ptr;
   void *seg_ptr[NUM_SEG];
   int seg_used[NUM_SEG];
   int num_used;
} borland_seg;

borland_seg *save_array;
int num_save_array;
int max_save_array;

#endif
#endif

void *
png_large_malloc(png_struct *png_ptr, png_uint_32 size)
{
   void *ret;

#ifdef PNG_MAX_MALLOC_64K
   if (size > (png_uint_32)65536L)
      png_error(png_ptr, "Cannot Allocate > 64K");
#endif

#ifdef __TURBOC__
#  ifdef __WIN32__
   ret = farmalloc(size);
#  else

   if (size == 65536L)
   {
      unsigned long offset;
      if (!save_array)
      {
         ret = farmalloc(size);
         offset = (unsigned long)(ret);
         offset &= 0xffffL;
      }
      else
      {
         ret = (void *)0;
      }
      if (save_array || offset)
      {
         int i, j;

         if (ret)
            farfree(ret);
         ret = (void *)0;

         if (!save_array)
         {
            unsigned long offset;
            png_byte huge *ptr;
            int i;

            num_save_array = 1;
            save_array = malloc(num_save_array * sizeof (borland_seg));
            if (!save_array)
               png_error(png_ptr, "Out of Memory");
            save_array->mem_ptr = farmalloc(
               (unsigned long)(NUM_SEG) * 65536L + 65528L);
            if (!save_array->mem_ptr)
               png_error(png_ptr, "Out of Memory");
            offset = (unsigned long)(ret);
            offset &= 0xffffL;
            ptr = save_array->mem_ptr;
            if (offset)
               ptr += 65536L - offset;
            for (i = 0; i < NUM_SEG; i++, ptr += 65536L)
            {
               save_array->seg_ptr[i] = ptr;
               save_array->seg_used[i] = 0;
            }
            save_array->num_used = 0;
         }

         for (i = 0; i < num_save_array; i++)
         {
            for (j = 0; j < NUM_SEG; j++)
            {
               if (!save_array[i].seg_used[j])
               {
                  ret = save_array[i].seg_ptr[j];
                  save_array[i].seg_used[j] = 1;
                  save_array[i].num_used++;
                  break;
               }
            }
            if (ret)
               break;
         }

         if (!ret)
         {
            unsigned long offset;
            png_byte huge *ptr;

            save_array = realloc(save_array,
               (num_save_array + 1) * sizeof (borland_seg));
            if (!save_array)
               png_error(png_ptr, "Out of Memory");
            save_array[num_save_array].mem_ptr = farmalloc(
               (unsigned long)(NUM_SEG) * 65536L + 65528L);
            if (!save_array[num_save_array].mem_ptr)
               png_error(png_ptr, "Out of Memory");
            offset = (unsigned long)(ret);
            offset &= 0xffffL;
            ptr = save_array[num_save_array].mem_ptr;
            if (offset)
               ptr += 65536L - offset;
            for (i = 0; i < NUM_SEG; i++, ptr += 65536L)
            {
               save_array[num_save_array].seg_ptr[i] = ptr;
               save_array[num_save_array].seg_used[i] = 0;
            }
            ret = save_array[num_save_array].seg_ptr[0];
            save_array[num_save_array].seg_used[0] = 1;
            save_array[num_save_array].num_used = 1;
            num_save_array++;
         }
      }
   }
   else
   {
      ret = farmalloc(size);
   }

#  endif /* __WIN32__ */
#else /* __TURBOC__ */
#  ifdef _MSC_VER
   ret = halloc(size, 1);
#  else
   /* everybody else, so normal malloc should do it. */
   ret = malloc(size);
#  endif
#endif

   if (!ret)
   {
      png_error(png_ptr, "Out of Memory");
   }

   return ret;
}

/* free a pointer allocated by png_large_malloc().  In the default
  configuration, png_ptr is not used, but is passed in case it
  is needed.  If ptr is NULL, return without taking any action. */
void
png_large_free(png_struct *png_ptr, void *ptr)
{
   if (!png_ptr)
      return;

   if (ptr != (void *)0)
   {
#ifdef __TURBOC__
#  ifndef __WIN32__
      int i, j;

      for (i = 0; i < num_save_array; i++)
      {
         for (j = 0; j < NUM_SEG; j++)
         {
            if (ptr == save_array[i].seg_ptr[j])
            {
printf("freeing pointer: i, j: %d, %d\n", i, j);
               save_array[i].seg_used[j] = 0;
               ptr = 0;
               save_array[i].num_used--;
               if (!save_array[i].num_used)
               {
                  int k;
printf("freeing array: %d\n", i);
                  num_save_array--;
                  farfree(save_array[i].mem_ptr);
                  for (k = i; k < num_save_array; k++)
                     save_array[k] = save_array[k + 1];
                  if (!num_save_array)
                  {
                     free(save_array);
                     save_array = 0;
                  }
               }
               break;
            }
         }
         if (!ptr)
            break;
      }

#  endif
      if (ptr)
         farfree(ptr);
#else
#  ifdef _MSC_VER
      hfree(ptr);
#  else
      free(ptr);
#  endif
#endif
   }
}

/* Allocate memory.  This is called for smallish blocks only  It
   should not get anywhere near 64K. */
void *
png_malloc(png_struct *png_ptr, png_uint_32 size)
{
   void *ret;

   if (!png_ptr)
      return ((void *)0);

#ifdef PNG_MAX_MALLOC_64K
   if (size > (png_uint_32)65536L)
      png_error(png_ptr, "Cannot Allocate > 64K");
#endif

   ret = malloc((png_size_t)size);

   if (!ret)
   {
      png_error(png_ptr, "Out of Memory");
   }

   return ret;
}

/* Reallocate memory.  This will not get near 64K on a
   even marginally reasonable file. */
void *
png_realloc(png_struct *png_ptr, void *ptr, png_uint_32 size)
{
   void *ret;

   if (!png_ptr)
      return ((void *)0);

#ifdef PNG_MAX_MALLOC_64K
   if (size > (png_uint_32)65536L)
      png_error(png_ptr, "Cannot Allocate > 64K");
#endif

   ret = realloc(ptr, (png_size_t)size);

   if (!ret)
   {
      png_error(png_ptr, "Out of Memory");
   }

   return ret;
}

/* free a pointer allocated by png_malloc().  In the default
  configuration, png_ptr is not used, but is passed incase it
  is needed.  If ptr is NULL, return without taking any action. */
void
png_free(png_struct *png_ptr, void *ptr)
{
   if (!png_ptr)
      return;

   if (ptr != (void *)0)
      free(ptr);
}

/* This function is called whenever there is an error.  Replace with
   however you wish to handle the error.  Note that this function
   MUST NOT return, or the program will crash */
void
png_error(png_struct *png_ptr, char *message)
{
   fprintf(stderr, "libpng error: %s\n", message);

   longjmp(png_ptr->jmpbuf, 1);
}

/* This function is called when there is a warning, but the library
   thinks it can continue anyway.  You don't have to do anything here
   if you don't want to.  In the default configuration, png_ptr is
   not used, but it is passed in case it may be useful. */
void
png_warning(png_struct *png_ptr, char *message)
{
   if (!png_ptr)
      return;

   fprintf(stderr, "libpng warning: %s\n", message);
}

