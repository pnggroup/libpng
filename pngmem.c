
/* pngmem.c - stub functions for memory allocation

   libpng 1.0 beta 2 - version 0.81
   For conditions of distribution and use, see copyright notice in png.h
   Copyright (c) 1995 Guy Eric Schalnat, Group 42, Inc.
   August 24, 1995

   This file provides a location for all memory allocation.  Users which
   need special memory handling are expected to modify the code in this file
   to meet their needs.  See the instructions at each function. */

#define PNG_INTERNAL
#include "png.h"

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
#ifndef __FLAT__

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
#endif

voidpf
png_large_malloc(png_structf *png_ptr, png_uint_32 size)
{
   voidpf ret;
   if (!png_ptr || !size)
      return ((void *)0);

#ifdef PNG_MAX_MALLOC_64K
   if (size > (png_uint_32)65536L)
      (*(png_ptr->error_fn))(png_ptr, "Cannot Allocate > 64K");
#endif

#ifdef __TURBOC__
#  if defined(__WIN32__) || defined(__FLAT__)
   ret = malloc(size);
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
               (*(png_ptr->error_fn))(png_ptr, "Out of Memory 1");
            save_array->mem_ptr = farmalloc(
               (unsigned long)(NUM_SEG) * 65536L + 65532L);
            if (!save_array->mem_ptr)
               (*(png_ptr->error_fn))(png_ptr, "Out of Memory 2");
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
               (*(png_ptr->error_fn))(png_ptr, "Out of Memory 3");
            save_array[num_save_array].mem_ptr = farmalloc(
               (unsigned long)(NUM_SEG) * 65536L + 65532L);
            if (!save_array[num_save_array].mem_ptr)
               (*(png_ptr->error_fn))(png_ptr, "Out of Memory 4");
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

   if (ret == NULL)
   {
      (*(png_ptr->error_fn))(png_ptr, "Out of Memory");
   }

   return ret;
}

/* free a pointer allocated by png_large_malloc().  In the default
  configuration, png_ptr is not used, but is passed in case it
  is needed.  If ptr is NULL, return without taking any action. */
void
png_large_free(png_structf *png_ptr, voidpf ptr)
{
   if (!png_ptr)
      return;

   if (ptr != (void *)0)
   {
#ifdef __TURBOC__
#  if defined(__WIN32__) || defined(__FLAT__)
      if (ptr)
         free(ptr);
#  else
      int i, j;

      for (i = 0; i < num_save_array; i++)
      {
         for (j = 0; j < NUM_SEG; j++)
         {
            if (ptr == save_array[i].seg_ptr[j])
            {
               save_array[i].seg_used[j] = 0;
               ptr = 0;
               save_array[i].num_used--;
               if (!save_array[i].num_used)
               {
                  int k;

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

      if (ptr)
         farfree(ptr);
#  endif
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

   if (!png_ptr || !size)
      return ((void *)0);

#ifdef PNG_MAX_MALLOC_64K
   if (size > (png_uint_32)65536L)
      (*(png_ptr->error_fn))(png_ptr, "Cannot Allocate > 64K");
#endif

   ret = malloc((png_size_t)size);

   if (!ret)
   {
      (*(png_ptr->error_fn))(png_ptr, "Out of Memory 6");
   }

   return ret;
}

/* Reallocate memory.  This will not get near 64K on a
   even marginally reasonable file. */
void *
png_realloc(png_struct *png_ptr, void *ptr, png_uint_32 size,
   png_uint_32 old_size)
{
   void *ret;

   if (!png_ptr || !old_size || !ptr || !size)
      return ((void *)0);

#ifdef PNG_MAX_MALLOC_64K
   if (size > (png_uint_32)65536L)
      (*(png_ptr->error_fn))(png_ptr, "Cannot Allocate > 64K");
#endif

   ret = realloc(ptr, (png_size_t)size);

   if (!ret)
   {
      (*(png_ptr->error_fn))(png_ptr, "Out of Memory 7");
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

