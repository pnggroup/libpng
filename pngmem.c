
/* pngmem.c - stub functions for memory allocation

	libpng 1.0 beta 2 - version 0.85
   For conditions of distribution and use, see copyright notice in png.h
   Copyright (c) 1995 Guy Eric Schalnat, Group 42, Inc.
   December 19, 1995

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


png_voidp
png_large_malloc(png_structp png_ptr, png_uint_32 size)
{
   png_voidp ret;
   if (!png_ptr || !size)
		return ((voidp)0);

#ifdef PNG_MAX_MALLOC_64K
   if (size > (png_uint_32)65536L)
      png_error(png_ptr, "Cannot Allocate > 64K");
#endif

#if defined(__TURBOC__) && !defined(__FLAT__)
	ret = farmalloc(size);
#else
	ret = malloc(size);
#endif

   if (ret == NULL)
   {
      png_error(png_ptr, "Out of Memory");
   }

   return ret;
}

/* free a pointer allocated by png_large_malloc().  In the default
  configuration, png_ptr is not used, but is passed in case it
  is needed.  If ptr is NULL, return without taking any action. */
void
png_large_free(png_structp png_ptr, png_voidp ptr)
{
   if (!png_ptr)
		return;

   if (ptr != NULL)
   {
#if defined(__TURBOC__) && !defined(__FLAT__)
		farfree(ptr);
#else
		free(ptr);
#endif
   }
}


/* Allocate memory.  This is called for smallish blocks only  It
	should not get anywhere near 64K.  On segmented machines, this
	must come from the local heap (for zlib). */
void *
png_malloc(png_structp png_ptr, png_uint_32 size)
{
   void *ret;

   if (!png_ptr || !size)
   {
      return ((void *)0);
   }

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
png_realloc(png_structp png_ptr, void * ptr, png_uint_32 size,
	png_uint_32 old_size)
{
	void *ret;

	if (!png_ptr || !old_size || !ptr || !size)
		return ((void *)0);

#ifdef PNG_MAX_MALLOC_64K
	if (size > (png_uint_32)65536L)
		png_error(png_ptr, "Cannot Allocate > 64K");
#endif

	ret = realloc(ptr, (png_size_t)size);

	if (!ret)
	{
		png_error(png_ptr, "Out of Memory 7");
	}

	return ret;
}

/* free a pointer allocated by png_malloc().  In the default
  configuration, png_ptr is not used, but is passed incase it
  is needed.  If ptr is NULL, return without taking any action. */
void
png_free(png_structp png_ptr, void * ptr)
{
	if (!png_ptr)
		return;

	if (ptr != (void *)0)
		free(ptr);
}


