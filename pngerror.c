
/* pngerror.c - stub functions for i/o and memory allocation

	libpng 1.0 beta 2 - version 0.85
   For conditions of distribution and use, see copyright notice in png.h
   Copyright (c) 1995 Guy Eric Schalnat, Group 42, Inc.
   December 19, 1995

   This file provides a location for all error handling.  Users which
   need special error handling are expected to modify the code in this
   file to meet their needs.  See the instructions at each function. */

#define PNG_INTERNAL
#include "png.h"

/* This function is called whenever there is an error.  Replace with
	however you wish to handle the error.  Note that this function
	MUST NOT return, or the program will crash */
void
png_error(png_structp png_ptr, png_const_charp message)
{
	if (png_ptr->error_fn)
		(*(png_ptr->error_fn))(png_ptr, message);

	/* if the following returns or doesn't exist, use the default function,
	   which will not return */
	png_default_error(png_ptr, message);
}

void
png_warning(png_structp png_ptr, png_const_charp message)
{
	if (png_ptr->warning_fn)
		(*(png_ptr->warning_fn))(png_ptr, message);
	else
		png_default_warning(png_ptr, message);
}

void
png_default_error(png_structp png_ptr, png_const_charp message)
{
#ifndef PNG_NO_STDIO
	fprintf(stderr, "libpng error: %s\n", message);
#endif

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
png_default_warning(png_structp png_ptr, png_const_charp message)
{
	if (!png_ptr)
		return;

#ifndef PNG_NO_STDIO
	fprintf(stderr, "libpng warning: %s\n", message);
#endif
}

/* This function is called when the application wants to use another
	method of handling errors and warnings.  Note that the error function must
	NOT return to the calling routine or serious problems will occur. The
	error return method used in the default routine calls
	longjmp(png_ptr->jmpbuf, 1) */
void
png_set_message_fn(png_structp png_ptr, png_voidp msg_ptr, png_msg_ptr error_fn,
	png_msg_ptr warning_fn)
{
	png_ptr->msg_ptr = msg_ptr;

	png_ptr->error_fn = error_fn;
	png_ptr->warning_fn = warning_fn;
}


/* This function returns a pointer to the msg_ptr associated with the user
	functions.  The application should free any memory associated with this
	pointer before png_write_destroy and png_read_destroy are called. */
png_voidp
png_get_msg_ptr(png_structp png_ptr)
{
	return png_ptr->msg_ptr;
}



