
/* pngmem.c - stub functions for memory allocation
 *
 * Last changed in libpng 1.6.0 [(PENDING RELEASE)]
 * Copyright (c) 1998-2011 Glenn Randers-Pehrson
 * (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
 * (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * This file provides a location for all memory allocation.  Users who
 * need special memory handling are expected to supply replacement
 * functions for png_malloc() and png_free(), and to use
 * png_create_read_struct_2() and png_create_write_struct_2() to
 * identify the replacement functions.
 */

#include "pngpriv.h"

#if defined(PNG_READ_SUPPORTED) || defined(PNG_WRITE_SUPPORTED)
/* Free a png_struct */
void /* PRIVATE */
png_destroy_png_struct(png_structp png_ptr)
{
   if (png_ptr != NULL)
   {
      /* png_free might call png_error and may certainly call
       * png_get_mem_ptr, so fake a temporary png_struct to support this.
       */
      png_struct dummy_struct = *png_ptr;
      memset(png_ptr, 0, sizeof *png_ptr);
      png_free(&dummy_struct, png_ptr);

#     ifdef PNG_SETJMP_SUPPORTED
         /* We may have a jmp_buf left to deallocate. */
         png_free_jmpbuf(&dummy_struct);
#     endif
   }
}

/* Allocate memory.  For reasonable files, size should never exceed
 * 64K.  However, zlib may allocate more then 64K if you don't tell
 * it not to.  See zconf.h and png.h for more information.  zlib does
 * need to allocate exactly 64K, so whatever you call here must
 * have the ability to do that.
 */
PNG_FUNCTION(png_voidp,PNGAPI
png_calloc,(png_structp png_ptr, png_alloc_size_t size),PNG_ALLOCATED)
{
   png_voidp ret;

   ret = png_malloc(png_ptr, size);

   if (ret != NULL)
      png_memset(ret, 0, size);

   return ret;
}

/* png_malloc_base, an internal function added at libpng 1.6.0, does the work of
 * allocating memory, taking into account limits and PNG_USER_MEM_SUPPORTED.
 * Checking and error handling must happen outside this routine; it returns NULL
 * if the allocation cannot be done (for any reason.)
 */
PNG_FUNCTION(png_voidp /* PRIVATE */,
png_malloc_base,(png_structp png_ptr, png_alloc_size_t size),PNG_ALLOCATED)
{
   /* Moved to png_malloc_base from png_malloc_default in 1.6.0; the DOS
    * allocators have also been removed in 1.6.0, so any 16-bit system now has
    * to implement a user memory handler.  This checks to be sure it isn't
    * called with big numbers.
    */
   if (size > 0 && size <= ~(size_t)0
#     ifdef PNG_MAX_MALLOC_64K
         && size <= 65536U
#     endif
      )
   {
      if (png_ptr != NULL && png_ptr->malloc_fn != NULL)
         return png_ptr->malloc_fn(png_ptr, size);

      else
         return malloc((size_t)size); /* checked for truncation above */
   }

   else
      return NULL;
}

/* Various functions that have different error handling are derived from this.
 * png_malloc always exists, but if PNG_USER_MEM_SUPPORTED is defined a separate
 * function png_malloc_default is also provided.
 */
PNG_FUNCTION(png_voidp,PNGAPI
png_malloc,(png_structp png_ptr, png_alloc_size_t size),PNG_ALLOCATED)
{
   png_voidp ret;

   if (png_ptr == NULL)
      return NULL;

   ret = png_malloc_base(png_ptr, size);

   if (ret == NULL)
       png_error(png_ptr, "Out of memory"); /* 'm' means png_malloc */

   return ret;
}

#ifdef PNG_USER_MEM_SUPPORTED
PNG_FUNCTION(png_voidp,PNGAPI
png_malloc_default,(png_structp png_ptr, png_alloc_size_t size),
   PNG_ALLOCATED PNG_DEPRECATED)
{
   png_voidp ret;

   if (png_ptr == NULL)
      return NULL;

   /* Passing 'NULL' here bypasses the application provided memory handler. */
   ret = png_malloc_base(NULL/*use malloc*/, size);

   if (ret == NULL)
      png_error(png_ptr, "Out of Memory"); /* 'M' means png_malloc_default */

   return ret;
}
#endif /* PNG_USER_MEM_SUPPORTED */

/* This function was added at libpng version 1.2.3.  The png_malloc_warn()
 * function will issue a png_warning and return NULL instead of issuing a
 * png_error, if it fails to allocate the requested memory.
 */
PNG_FUNCTION(png_voidp,PNGAPI
png_malloc_warn,(png_structp png_ptr, png_alloc_size_t size),PNG_ALLOCATED)
{
   if (png_ptr != NULL)
   {
      png_voidp ret = png_malloc_base(png_ptr, size);

      if (ret != NULL)
         return ret;

      png_warning(png_ptr, "Out of memory");
   }

   return NULL;
}

/* Free a pointer allocated by png_malloc().  If ptr is NULL, return
 * without taking any action.
 */
void PNGAPI
png_free(png_structp png_ptr, png_voidp ptr)
{
   if (png_ptr == NULL || ptr == NULL)
      return;

#ifdef PNG_USER_MEM_SUPPORTED
   if (png_ptr->free_fn != NULL)
      png_ptr->free_fn(png_ptr, ptr);

   else
      png_free_default(png_ptr, ptr);
}

PNG_FUNCTION(void,PNGAPI
png_free_default,(png_structp png_ptr, png_voidp ptr),PNG_DEPRECATED)
{
   if (png_ptr == NULL || ptr == NULL)
      return;
#endif /* PNG_USER_MEM_SUPPORTED */

   free(ptr);
}

#ifdef PNG_USER_MEM_SUPPORTED
/* This function is called when the application wants to use another method
 * of allocating and freeing memory.
 */
void PNGAPI
png_set_mem_fn(png_structp png_ptr, png_voidp mem_ptr, png_malloc_ptr
  malloc_fn, png_free_ptr free_fn)
{
   if (png_ptr != NULL)
   {
      png_ptr->mem_ptr = mem_ptr;
      png_ptr->malloc_fn = malloc_fn;
      png_ptr->free_fn = free_fn;
   }
}

/* This function returns a pointer to the mem_ptr associated with the user
 * functions.  The application should free any memory associated with this
 * pointer before png_write_destroy and png_read_destroy are called.
 */
png_voidp PNGAPI
png_get_mem_ptr(png_const_structp png_ptr)
{
   if (png_ptr == NULL)
      return NULL;

   return png_ptr->mem_ptr;
}
#endif /* PNG_USER_MEM_SUPPORTED */
#endif /* PNG_READ_SUPPORTED || PNG_WRITE_SUPPORTED */
