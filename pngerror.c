
/* pngstub.c - stub functions for i/o and memory allocation

   libpng 1.0 beta 2 - version 0.81
   For conditions of distribution and use, see copyright notice in png.h
   Copyright (c) 1995 Guy Eric Schalnat, Group 42, Inc.
   August 24, 1995

   This file provides a location for all error handling.  Users which
   need special error handling are expected to modify the code in this
   file to meet their needs.  See the instructions at each function. */

#define PNG_INTERNAL
#include "png.h"

/* This function is called whenever there is an error.  Replace with
   however you wish to handle the error.  Note that this function
   MUST NOT return, or the program will crash */
void
png_error(png_structf *png_ptr, char *message)
{
   fprintf(stderr, "libpng error: %s\n", message);
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

