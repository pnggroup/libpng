
/* png.h - header file for png reference library
   libpng 1.0 beta 1 - version 0.71
   June 26, 1995

   Note: This is a beta version.  It reads and writes valid files
   on the platforms I have, but it has had limited portability
   testing.  Furthermore, you will probably have to modify the
   includes below to get it to work on your system, and you
   may have to supply the correct compiler flags in the makefile.
   Read the readme.txt for more information, and how to contact
   me if you have any problems, or if you want your compiler/
   platform to be supported in the next official libpng release.

   See readme.txt for more information

   Copyright (c) 1995 Guy Eric Schalnat, Group 42, Inc.
   Contributing Authors:
      Guy Eric Schalnat

   The PNG Reference Library is supplied "AS IS". The Contributing Authors
   and Group 42, Inc. disclaim all warranties, expressed or implied,
   including, without limitation, the warranties of merchantability and of
   fitness for any purpose. The Contributing Authors and Group 42, Inc.
   assume no liability for damages, direct or consequential, which may
   result from the use of the PNG Reference Library.

   Permission is hereby granted to use, copy, modify, and distribute this
   source code, or portions hereof, for any purpose, without fee, subject
   to the following restrictions:
   1. The origin of this source code must not be misrepresented.
   2. Altered versions must be plainly marked as such and must not be
     misrepresented as being the original source.
   3. This Copyright notice may not be removed or altered from any source or
     altered source distribution.

   The Contributing Authors and Group 42, Inc. specifically permit, without
   fee, and encourage the use of this source code as a component to
   supporting the PNG file format in commercial products. If you use this
   source code in a product, acknowledgment is not required but would be
   appreciated.
   */

#ifndef _PNG_H
#define _PNG_H

/* This is not the place to learn how to use libpng.  The file libpng.txt
   describes how to use libpng, and the file example.c summarizes it
   with some code to build around.  This file is useful for looking
   at the actual function definitions and structure components. */

/* This file is arranged in several sections.  The first section contains
   all the definitions for libpng.  The second section details the functions
   most users will use.  The third section describes the stub files that
   users will most likely need to change.  The last section contains
   functions used internally by the code.

   Any machine specific code is near the front of this file, so if you
   are configuring libpng for a machine, you may want to read the section
   starting here down to where it starts to typedef png_color, png_text,
   and png_info */

/* this is the size of the compression buffer, and thus the size of
   an IDAT chunk.  Make this whatever size you feel is best for your
   machine.  One of these will be allocated per png_struct.  When this
   is full, it writes the data to the disk, and does some other
   calculations.  Making this an extreamly small size will slow
   the library down, but you may want to experiment to determine
   where it becomes significant, if you are concerned with memory
   usage.  Note that zlib allocates at least 32Kb also.  For readers,
   this describes the size of the buffer available to read the data in.
   Unless this gets smaller then the size of a row (compressed),
   it should not make much difference how big this is.  */

#define PNG_ZBUF_SIZE 8192;

/* include the compression library's header */
#include "zlib.h"

/* While libpng currently uses zlib for it's compression, it has been designed
   to stand on it's own.  Towards this end, there are two defines that are
   used to help portability between machines.  To make it simpler to
   setup libpng on a machine, this currently uses zlib's definitions, so
   any changes should be made in zlib.  Libpng will check zlib's settings
   and adjust it's own accordingly. */

/* if you are running on a machine where you cannot allocate more then
   64K of memory, uncomment this.  While libpng will not normally need
   that much memory in a chunk (unless you load up a very large file),
   zlib needs to know how big of a chunk it can use, and libpng thus
   makes sure to check any memory allocation to verify it will fit
   into memory.
#define PNG_MAX_ALLOC_64K
*/
#ifdef MAXSEG_64K
#define PNG_MAX_ALLOC_64K
#endif

/* this macro protects us against machines that don't have function
   prototypes.  If your compiler does not handle function prototypes,
   define this macro.  I've always been able to use _NO_PROTO as the
   indicator, but you may need to drag the empty declaration out in
   front of here, or change the ifdef to suit your own needs. */
#ifndef PNGARG

#ifdef __P
#define PNGARG(arglist) __P(arglist)
#else

#ifdef _NO_PROTO
#define PNGARG(arglist)
#else
#define PNGARG(arglist) arglist
#endif /* _NO_PROTO */

#endif /* __P(arglist) */

#endif /* PNGARG */

/* enough people need this for various reasons to include it here */
#include <sys/types.h>
/* need the time information for reading tIME chunks */
#include <time.h>

/* for FILE.  If you are not using standard io, you don't need this */
#include <stdio.h>

/* include setjmp.h for error handling */
#include <setjmp.h>

/* other defines for things like memory and the like can go here.  These
   are the only files included in libpng, so if you need to change them,
   change them here.  They are only included if PNG_INTERNAL is defined. */
#ifdef PNG_INTERNAL
#include <stdlib.h>
#include <ctype.h>
#ifdef BSD
#include <strings.h>
#else
#include <string.h>
#endif
#include <math.h>

/* other defines specific to compilers can go here.  Try to keep
   them inside an appropriate ifdef/endif pair for portability */

/* for some reason, Borland C++ defines memcmp, etc. in mem.h, not
   stdlib.h like it should (I think).  Or perhaps this is a C++
   feature */
#ifdef __TURBOC__
#include <mem.h>
#include "alloc.h"
#endif

#ifdef _MSC_VER
#include <malloc.h>
#endif

/* this controls how fine the dithering gets.  As this allocates
   a largish chunk of memory (32K), those who are not as concerned
   with dithering quality can decrease some or all of these */
#define PNG_DITHER_RED_BITS 5
#define PNG_DITHER_GREEN_BITS 5
#define PNG_DITHER_BLUE_BITS 5

/* this controls how fine the gamma correction becomes when you
   are only interested in 8 bits anyway.  Increasing this value
   results in more memory being used, and more pow() functions
   being called to fill in the gamma tables.  Don't get this
   value less then 8, and even that may not work (I haven't tested
   it). */

#define PNG_MAX_GAMMA_8 11

#endif /* PNG_INTERNAL */

/* some typedefs to get us started.  These should be safe on most of the
   common platforms.  The typedefs should be at least as large
   as the numbers suggest (a png_uint_32 must be at least 32 bits long),
   but they don't have to be exactly that size. */

typedef unsigned long png_uint_32;
typedef long png_int_32;
typedef unsigned short png_uint_16;
typedef short png_int_16;
typedef unsigned char png_byte;

/* this is usually size_t. it is typedef'ed just in case you need it to
   change (I'm not sure if you will or not, so I thought I'd be safe) */
typedef size_t png_size_t;

/* three color definitions.  The order of the red, green, and blue, (and the
   exact size) is not important, although the size of the fields need to
   be png_byte or png_uint_16 (as defined below).  While png_color_8 and
   png_color_16 have more fields then they need, they are never used in
   arrays, so the size isn't that important.  I thought about using
   unions, but it looked too clumsy, so I left it. If you're using C++,
   you can union red, index, and gray, if you want. */
typedef struct png_color_struct
{
   png_byte red;
   png_byte green;
   png_byte blue;
} png_color;

typedef struct png_color_16_struct
{
   png_byte index; /* used for palette files */
   png_uint_16 red; /* for use in red green blue files */
   png_uint_16 green;
   png_uint_16 blue;
   png_uint_16 gray; /* for use in grayscale files */
} png_color_16;

typedef struct png_color_8_struct
{
   png_byte red; /* for use in red green blue files */
   png_byte green;
   png_byte blue;
   png_byte gray; /* for use in grayscale files */
   png_byte alpha; /* for alpha channel files */
} png_color_8;

/* png_text holds the text in a png file, and whether they are compressed
   or not.  If compression is -1, the text is not compressed.  */
typedef struct png_text_struct
{
   int compression; /* compression value, -1 if uncompressed */
   char *key; /* keyword */
   char *text; /* comment */
   png_uint_32 text_length; /* length of text field */
} png_text;

/* png_time is a way to hold the time in an machine independent way.
   Two conversions are provided, both from time_t and struct tm.  There
   is no portable way to convert to either of these structures, as far
   as I know.  If you know of a portable way, send it to me. */
typedef struct png_time_struct
{
   png_uint_16 year; /* full year, as in, 1995 */
   png_byte month; /* month of year, 1 - 12 */
   png_byte day; /* day of month, 1 - 31 */
   png_byte hour; /* hour of day, 0 - 23 */
   png_byte minute; /* minute of hour, 0 - 59 */
   png_byte second; /* second of minute, 0 - 60 (for leap seconds) */
} png_time;

/* png_info is a structure that holds the information in a png file.
   If you are reading the file, This structure will tell you what is
   in the png file.  If you are writing the file, fill in the information
   you want to put into the png file, then call png_write_info().
   The names chosen should be very close to the PNG
   specification, so consult that document for information
   about the meaning of each field. */
typedef struct png_info_struct
{
   /* the following are necessary for every png file */
   png_uint_32 width; /* with of file */
   png_uint_32 height; /* height of file */
   png_byte bit_depth; /* 1, 2, 4, 8, or 16 */
   png_byte color_type; /* use the PNG_COLOR_TYPE_ defines */
   png_byte compression_type; /* must be 0 */
   png_byte filter_type; /* must be 0 */
   png_byte interlace_type; /* 0 for non-interlaced, 1 for interlaced */
   png_uint_32 valid; /* the PNG_INFO_ defines, OR'd together */
   /* the following is informational only on read, and not used on
      writes */
   png_byte channels; /* number of channels of data per pixel */
   png_byte pixel_depth; /* number of bits per pixel */
   png_uint_32 rowbytes; /* bytes needed for untransformed row */
   /* the rest are optional.  If you are reading, check the valid
      field to see if the information in these are valid.  If you
      are writing, set the valid field to those chunks you want
      written, and initialize the appropriate fields below */
   float gamma; /* gamma value of file, if gAMA chunk is valid */
   png_color_8 sig_bit; /* significant bits */
   float x_white; /* cHRM chunk values */
   float y_white;
   float x_red;
   float y_red;
   float x_green;
   float y_green;
   float x_blue;
   float y_blue;
   png_color *palette; /* palette of file */
   png_uint_16 num_palette; /* number of values in palette */
   png_byte *trans; /* tRNS values for palette image */
    png_uint_16 num_trans; /* number of trans values */
   png_color_16 trans_values; /* tRNS values for non-palette image */
   png_color_16 background; /* background color of image */
   png_uint_16 *hist; /* histogram of palette usage */
   png_uint_32 x_pixels_per_unit; /* x resolution */
   png_uint_32 y_pixels_per_unit; /* y resolution */
   png_byte phys_unit_type; /* resolution type */
   png_uint_32 x_offset; /* x offset on page */
   png_uint_32 y_offset; /* y offset on page */
   png_byte offset_unit_type; /* offset units type */
   png_time mod_time; /* modification time */
   int num_text; /* number of comments */
   int max_text; /* size of text array */
   png_text *text; /* array of comments */
} png_info;

#define PNG_RESOLUTION_UNKNOWN 0
#define PNG_RESOLUTION_METER 1

#define PNG_OFFSET_PIXEL 0
#define PNG_OFFSET_MICROMETER 1

/* these describe the color_type field in png_info */

/* color type masks */
#define PNG_COLOR_MASK_PALETTE 1
#define PNG_COLOR_MASK_COLOR 2
#define PNG_COLOR_MASK_ALPHA 4

/* color types.  Note that not all combinations are legal */
#define PNG_COLOR_TYPE_PALETTE \
   (PNG_COLOR_MASK_COLOR | PNG_COLOR_MASK_PALETTE)
#define PNG_COLOR_TYPE_RGB (PNG_COLOR_MASK_COLOR)
#define PNG_COLOR_TYPE_GRAY 0
#define PNG_COLOR_TYPE_RGB_ALPHA \
   (PNG_COLOR_MASK_COLOR | PNG_COLOR_MASK_ALPHA)
#define PNG_COLOR_TYPE_GRAY_ALPHA (PNG_COLOR_MASK_ALPHA)

/* These determine if a chunks information is present in a read operation, or
   if the chunk should be written in a write operation.  */
#define PNG_INFO_gAMA 0x0001
#define PNG_INFO_sBIT 0x0002
#define PNG_INFO_cHRM 0x0004
#define PNG_INFO_PLTE 0x0008
#define PNG_INFO_tRNS 0x0010
#define PNG_INFO_bKGD 0x0020
#define PNG_INFO_hIST 0x0040
#define PNG_INFO_pHYs 0x0080
#define PNG_INFO_oFFs 0x0100
#define PNG_INFO_tIME 0x0200

/* this is used for the transformation routines, as some of them
   change these values for the row.  It also should enable using
   the routines for other uses. */
typedef struct png_row_info_struct
{
   png_uint_32 width; /* width of row */
   png_uint_32 rowbytes; /* number of bytes in row */
   png_byte color_type; /* color type of row */
   png_byte bit_depth; /* bit depth of row */
   png_byte channels; /* number of channels (1, 2, 3, or 4) */
   png_byte pixel_depth; /* bits per pixel (depth * channels) */
} png_row_info;

/* The structure that holds the information to read and write png files.
   The only people who need to care about what is inside of this are the
   people who will be modifying the library for their own special needs.
   */
typedef struct png_struct_def
{
   jmp_buf jmpbuf; /* used in png_error */
   png_byte mode; /* used to determine where we are in the png file */
   png_byte color_type; /* color type of file */
   png_byte bit_depth; /* bit depth of file */
   png_byte interlaced; /* interlace type of file */
   png_byte compession; /* compression type of file */
   png_byte filter; /* filter type */
   png_byte channels; /* number of channels in file */
   png_byte pixel_depth; /* number of bits per pixel */
   png_byte usr_bit_depth; /* bit depth of users row */
   png_byte usr_channels; /* channels at start of write */
   png_byte gamma_shift; /* amount of shift for 16 bit gammas */
   png_byte pass; /* current pass (0 - 6) */
   png_byte row_init; /* 1 if png_read_start_row() has been called */
   png_byte background_gamma_type;
   png_byte background_expand;
   png_byte zlib_finished;
   png_byte user_palette;
   png_uint_16 num_palette; /* number of entries in palette */
   png_uint_16 num_trans; /* number of transparency values */
   png_uint_32 transformations; /* which transformations to perform */
   png_uint_32 crc; /* current crc value */
   png_uint_32 width; /* width of file */
   png_uint_32 height; /* height of file */
   png_uint_32 num_rows; /* number of rows in current pass */
   png_uint_32 rowbytes; /* size of row in bytes */
   png_uint_32 usr_width; /* width of row at start of write */
   png_uint_32 iwidth; /* interlaced width */
   png_uint_32 irowbytes; /* interlaced rowbytes */
   png_uint_32 row_number; /* current row in pass */
   png_uint_32 idat_size; /* current idat size for read */
   png_uint_32 zbuf_size; /* size of zbuf */
   png_color *palette; /* files palette */
   png_byte *palette_lookup; /* lookup table for dithering */
   png_byte *gamma_table; /* gamma table for 8 bit depth files */
   png_byte *gamma_from_1; /* converts from 1.0 to screen */
   png_byte *gamma_to_1; /* converts from file to 1.0 */
   png_byte *trans; /* transparency values for paletted files */
   png_byte *dither_index; /* index translation for palette files */
   png_uint_16 **gamma_16_table; /* gamma table for 16 bit depth files */
   png_uint_16 **gamma_16_from_1; /* converts from 1.0 to screen */
   png_uint_16 **gamma_16_to_1; /* converts from file to 1.0 */
   png_uint_16 *hist; /* histogram */
   png_byte *zbuf; /* buffer for zlib */
   png_byte *row_buf; /* row buffer */
   png_byte *prev_row; /* previous row */
   png_byte *save_row; /* place to save row before filtering */
   z_stream *zstream; /* pointer to decompression structure (below) */
   float gamma; /* file gamma value */
   float display_gamma; /* display gamma value */
   float background_gamma;
   png_color_8 shift; /* shift for significant bit tranformation */
   png_color_8 sig_bit; /* significant bits in file */
   png_color_16 trans_values; /* transparency values for non-paletted files */
   png_color_16 background; /* background color, gamma corrected for screen */
   png_color_16 background_1; /* background normalized to gamma 1.0 */
   png_row_info row_info; /* used for transformation routines */
   z_stream zstream_struct; /* decompression structure */
   FILE *fp; /* used for png_read and png_write */
} png_struct;


/* Here are the function definitions most commonly used.  This is not
   the place to find out how to use libpng.  See libpng.txt for the
   full explanation, see example.c for the summary.  This just provides
   a simple one line of the use of each function. */

/* check the first 1 - 8 bytes to see if it is a png file */
extern int png_check_sig PNGARG((png_byte *sig, int num));

/* initialize png structure for reading, and allocate any memory needed */
extern void png_read_init PNGARG((png_struct *png_ptr));

/* initialize png structure for writing, and allocate any memory needed */
extern void png_write_init PNGARG((png_struct *png_ptr));

/* initialize the info structure */
extern void png_info_init PNGARG((png_info *info));

/* Writes all the png information before the image. */
extern void png_write_info PNGARG((png_struct *png_ptr, png_info *info));

/* read the information before the actual image data. */
extern void png_read_info PNGARG((png_struct *png_ptr, png_info *info));

/* convert from a struct tm to png_time */
extern void png_convert_from_struct_tm PNGARG((png_time *ptime,
   struct tm *ttime));

/* convert from time_t to png_time.  Uses gmtime() */
extern void png_convert_from_time_t PNGARG((png_time *ptime, time_t ttime));

/* Expand the data to 24 bit RGB, or 8 bit Grayscale,
   with alpha if necessary. */
extern void png_set_expand PNGARG((png_struct *png_ptr));

/* Use blue, green, red order for pixels. */
extern void png_set_bgr PNGARG((png_struct *png_ptr));

/* Add a filler byte to rgb images after the colors. */
extern void png_set_rgbx PNGARG((png_struct *png_ptr));

/* Add a filler byte to rgb images before the colors. */
extern void png_set_xrgb PNGARG((png_struct *png_ptr));

/* Swap bytes in 16 bit depth files. */
extern void png_set_swap PNGARG((png_struct *png_ptr));

/* Use 1 byte per pixel in 1, 2, or 4 bit depth files. */
extern void png_set_packing PNGARG((png_struct *png_ptr));

/* Converts files to legal bit depths. */
extern void png_set_shift PNGARG((png_struct *png_ptr,
   png_color_8 *true_bits));

/* Have the code handle the interlacing.  Returns the number of passes. */
extern int png_set_interlace_handling PNGARG((png_struct *png_ptr));

/* Invert monocrome files */
extern void png_set_invert_mono PNGARG((png_struct *png_ptr));

/* Handle alpha and tRNS by replacing with a background color. */
#define PNG_BACKGROUND_GAMMA_SCREEN 0
#define PNG_BACKGROUND_GAMMA_FILE 1
#define PNG_BACKGROUND_GAMMA_UNIQUE 2
#define PNG_BACKGROUND_GAMMA_UNKNOWN 3
extern void png_set_background PNGARG((png_struct *png_ptr,
   png_color_16 *background_color, int background_gamma_code,
   int need_expand, float background_gamma));

/* strip the second byte of information from a 16 bit depth file. */
extern void png_set_strip_16 PNGARG((png_struct *png_ptr));

/* convert a grayscale file into rgb. */
extern void png_set_gray_to_rgb PNGARG((png_struct *png_ptr));

/* Turn on dithering, and reduce the palette to the number of colors available. */
extern void png_set_dither PNGARG((png_struct *png_ptr, png_color *palette,
   int num_palette, int maximum_colors, png_uint_16 *histogram, int full_dither));

/* Handle gamma correction. */
extern void png_set_gamma PNGARG((png_struct *png_ptr, float screen_gamma,
   float default_file_gamma));

/* optional update palette with requested transformations */
void png_start_read_image PNGARG((png_struct *png_ptr));

/* read a one or more rows of image data.*/
extern void png_read_rows PNGARG((png_struct *png_ptr,
   png_byte **row,
   png_byte **display_row, png_uint_32 num_rows));

/* read a row of data.*/
extern void png_read_row PNGARG((png_struct *png_ptr,
   png_byte *row,
   png_byte *display_row));

/* read the whole image into memory at once. */
extern void png_read_image PNGARG((png_struct *png_ptr,
   png_byte **image));

/* write a row of image data */
extern void png_write_row PNGARG((png_struct *png_ptr,
   png_byte *row));

/* write a few rows of image data */
extern void png_write_rows PNGARG((png_struct *png_ptr,
   png_byte **row,
   png_uint_32 num_rows));

/* write the image data */
extern void png_write_image PNGARG((png_struct *png_ptr, png_byte **image));

/* writes the end of the png file. */
extern void png_write_end PNGARG((png_struct *png_ptr, png_info *info));

/* read the end of the png file. */
extern void png_read_end PNGARG((png_struct *png_ptr, png_info *info));

/* free all memory used by the read */
extern void png_read_destroy PNGARG((png_struct *png_ptr, png_info *info,
   png_info *end_info));

/* free any memory used in png struct */
extern void png_write_destroy PNGARG((png_struct *png_ptr));


/* These next functions are stubs of typical c functions for input/output,
   memory, and error handling.  They are in the file pngstub.c, and are
   set up to be easily modified for users that need to.  See the file
   pngstub.c for more information */

/* Write the data to whatever output you are using. */
extern void png_write_data PNGARG((png_struct *png_ptr, png_byte *data,
   png_uint_32 length));

/* Read data from whatever input you are using */
extern void png_read_data PNGARG((png_struct *png_ptr, png_byte *data,
   png_uint_32 length));

/* Initialize the input/output for the png file. */
extern void png_init_io PNGARG((png_struct *png_ptr, FILE *fp));

/* Allocate memory in larger chunks. */
extern void *png_large_malloc PNGARG((png_struct *png_ptr, png_uint_32 size));

/* free's a pointer allocated by png_large_malloc() */
extern void png_large_free PNGARG((png_struct *png_ptr, void *ptr));

/* Allocate memory. */
extern void *png_malloc PNGARG((png_struct *png_ptr, png_uint_32 size));

/* Reallocate memory. */
extern void *png_realloc PNGARG((png_struct *png_ptr, void *ptr,
   png_uint_32 size));

/* free's a pointer allocated by png_malloc() */
extern void png_free PNGARG((png_struct *png_ptr, void *ptr));

/* Fatal error in libpng - can't continue */
extern void png_error PNGARG((png_struct *png_ptr, char *error));

/* Non-fatal error in libpng.  Can continue, but may have a problem. */
extern void png_warning PNGARG((png_struct *png_ptr, char *message));


/* These next functions are used internally in the code.  If you use
   them, make sure you read and understand the png spec.  More information
   about them can be found in the files where the functions are.
   Feel free to move any of these outside the PNG_INTERNAL define if
   you just need a few of them, but if you need access to more, you should
   define PNG_INTERNAL inside your code, so everyone who includes png.h
   won't get yet another definition the compiler has to deal with. */

#ifdef PNG_INTERNAL

/* various modes of operation.  Note that after an init, mode is set to
   zero automatically */
#define PNG_BEFORE_IHDR 0
#define PNG_HAVE_IHDR 1
#define PNG_HAVE_PLTE 2
#define PNG_HAVE_IDAT 3
#define PNG_AT_LAST_IDAT 4
#define PNG_AFTER_IDAT 5
#define PNG_AFTER_IEND 6

/* defines for the transformations the png library does on the image data */
#define PNG_BGR 0x0001
#define PNG_INTERLACE 0x0002
#define PNG_PACK 0x0004
#define PNG_SHIFT 0x0008
#define PNG_SWAP_BYTES 0x0010
#define PNG_INVERT_MONO 0x0020
#define PNG_DITHER 0x0040
#define PNG_BACKGROUND 0x0080
#define PNG_XRGB 0x0100
#define PNG_16_TO_8 0x0200
#define PNG_RGBA 0x0400
#define PNG_EXPAND 0x0800
#define PNG_GAMMA 0x1000
#define PNG_GRAY_TO_RGB 0x2000

/* save typing and make code easier to understand */
#define PNG_COLOR_DIST(c1, c2) (abs((int)((c1).red) - (int)((c2).red)) + \
   abs((int)((c1).green) - (int)((c2).green)) + \
   abs((int)((c1).blue) - (int)((c2).blue)))

/* variables defined in png.c - only it needs to define PNG_NO_EXTERN */
#ifndef PNG_NO_EXTERN
/* place to hold the signiture string for a png file. */
extern png_byte png_sig[];

/* constant strings for known chunk types.  If you need to add a chunk,
   add a string holding the name here.  See png.c for more details */
extern png_byte png_IHDR[];
extern png_byte png_IDAT[];
extern png_byte png_IEND[];
extern png_byte png_PLTE[];
extern png_byte png_gAMA[];
extern png_byte png_sBIT[];
extern png_byte png_cHRM[];
extern png_byte png_tRNS[];
extern png_byte png_bKGD[];
extern png_byte png_hIST[];
extern png_byte png_tEXt[];
extern png_byte png_zTXt[];
extern png_byte png_pHYs[];
extern png_byte png_oFFs[];
extern png_byte png_tIME[];
/* Structures to facilitate easy interlacing.  See png.c for more details */
extern int png_pass_start[];
extern int png_pass_inc[];
extern int png_pass_ystart[];
extern int png_pass_yinc[];
/* these are not currently used.  If you need them, see png.c
extern int png_pass_width[];
extern int png_pass_height[];
*/
extern int png_pass_mask[];
extern int png_pass_dsp_mask[];

#endif /* PNG_NO_EXTERN */

/* Function to allocate memory for zlib. */
extern voidp png_zalloc PNGARG((voidp png_ptr, uInt items, uInt size));

/* function to free memory for zlib */
extern void png_zfree PNGARG((voidp png_ptr, voidp ptr));

/* reset the crc variable */
extern void png_reset_crc PNGARG((png_struct *png_ptr));

/* calculate the crc over a section of data.  Note that while we
   are passing in a 32 bit value for length, on 16 bit machines, you
   would need to use huge pointers to access all that data.  See the
   code in png.c for more information. */
extern void png_calculate_crc PNGARG((png_struct *png_ptr, png_byte *ptr,
   png_uint_32 length));

/* place a 32 bit number into a buffer in png byte order.  We work
   with unsigned numbers for convenience, you may have to cast
   signed numbers (if you use any, most png data is unsigned). */
extern void png_save_uint_32 PNGARG((png_byte *buf, png_uint_32 i));

/* place a 16 bit number into a buffer in png byte order */
extern void png_save_uint_16 PNGARG((png_byte *buf, png_uint_16 i));

/* write a 32 bit number */
extern void png_write_uint_32 PNGARG((png_struct *png_ptr, png_uint_32 i));

/* write a 16 bit number */
extern void png_write_uint_16 PNGARG((png_struct *png_ptr, png_uint_16 i));

/* Write a png chunk.  */
extern void png_write_chunk PNGARG((png_struct *png_ptr, png_byte *type,
   png_byte *data, png_uint_32 length));

/* Write the start of a png chunk. */
extern void png_write_chunk_start PNGARG((png_struct *png_ptr, png_byte *type,
   png_uint_32 total_length));

/* write the data of a png chunk started with png_write_chunk_start(). */
extern void png_write_chunk_data PNGARG((png_struct *png_ptr, png_byte *data,
   png_uint_32 length));

/* finish a chunk started with png_write_chunk_start() */
extern void png_write_chunk_end PNGARG((png_struct *png_ptr));

/* simple function to write the signiture */
extern void png_write_sig PNGARG((png_struct *png_ptr));

/* write various chunks */

/* Write the IHDR chunk, and update the png_struct with the necessary
   information. */
extern void png_write_IHDR PNGARG((png_struct *png_ptr, png_uint_32 width,
   png_uint_32 height,
   int bit_depth, int color_type, int compression_type, int filter_type,
   int interlace_type));

extern void png_write_PLTE PNGARG((png_struct *png_ptr, png_color *palette,
   int number));

extern void png_write_IDAT PNGARG((png_struct *png_ptr, png_byte *data,
   png_uint_32 length));

extern void png_write_IEND PNGARG((png_struct *png_ptr));

extern void png_write_gAMA PNGARG((png_struct *png_ptr, float gamma));

extern void png_write_sBIT PNGARG((png_struct *png_ptr, png_color_8 *sbit,
   int color_type));

extern void png_write_cHRM PNGARG((png_struct *png_ptr,
   float white_x, float white_y,
   float red_x, float red_y, float green_x, float green_y,
   float blue_x, float blue_y));

extern void png_write_tRNS PNGARG((png_struct *png_ptr, png_byte *trans,
   png_color_16 *values, int number, int color_type));

extern void png_write_bKGD PNGARG((png_struct *png_ptr, png_color_16 *values,
   int color_type));

extern void png_write_hIST PNGARG((png_struct *png_ptr, png_uint_16 *hist,
   int number));

extern void png_write_tEXt PNGARG((png_struct *png_ptr, char *key,
   char *text, png_uint_32 text_len));

extern void png_write_zTXt PNGARG((png_struct *png_ptr, char *key,
   char *text, png_uint_32 text_len, int compression));

extern void png_write_pHYs PNGARG((png_struct *png_ptr,
   png_uint_32 x_pixels_per_unit,
   png_uint_32 y_pixels_per_unit,
   int unit_type));

extern void png_write_oFFs PNGARG((png_struct *png_ptr,
   png_uint_32 x_offset,
   png_uint_32 y_offset,
   int unit_type));

extern void png_write_tIME PNGARG((png_struct *png_ptr, png_time *mod_time));

/* Internal use only.   Called when finished processing a row of data */
extern void png_write_finish_row PNGARG((png_struct *png_ptr));

/* Internal use only.   Called before first row of data */
extern void png_write_start_row PNGARG((png_struct *png_ptr));

/* callbacks for png chunks */
extern void png_read_IHDR PNGARG((png_struct *png_ptr, png_info *info,
   png_uint_32 width, png_uint_32 height, int bit_depth,
   int color_type, int compression_type, int filter_type,
   int interlace_type));
extern void png_read_PLTE PNGARG((png_struct *png_ptr, png_info *info,
   png_color *palette, int num));
extern void png_read_gAMA PNGARG((png_struct *png_ptr, png_info *info,
   float gamma));
extern void png_read_sBIT PNGARG((png_struct *png_ptr, png_info *info,
   png_color_8 *sig_bit));
extern void png_read_cHRM PNGARG((png_struct *png_ptr, png_info *info,
   float white_x, float white_y, float red_x, float red_y,
   float green_x, float green_y, float blue_x, float blue_y));
extern void png_read_tRNS PNGARG((png_struct *png_ptr, png_info *info,
   png_byte *trans, int num_trans,   png_color_16 *trans_values));
extern void png_read_bKGD PNGARG((png_struct *png_ptr, png_info *info,
   png_color_16 *background));
extern void png_read_hIST PNGARG((png_struct *png_ptr, png_info *info,
   png_uint_16 *hist));
extern void png_read_pHYs PNGARG((png_struct *png_ptr, png_info *info,
   png_uint_32 res_x, png_uint_32 res_y, int unit_type));
extern void png_read_oFFs PNGARG((png_struct *png_ptr, png_info *info,
   png_uint_32 offset_x, png_uint_32 offset_y, int unit_type));
extern void png_read_tIME PNGARG((png_struct *png_ptr, png_info *info,
   png_time *mod_time));
extern void png_read_tEXt PNGARG((png_struct *png_ptr, png_info *info,
   char *key, char *text, png_uint_32 text_len));
extern void png_read_zTXt PNGARG((png_struct *png_ptr, png_info *info,
   char *key, char *text, png_uint_32 text_len, int compression));

void
png_build_gamma_table PNGARG((png_struct *png_ptr));

/* combine a row of data, dealing with alpha, etc. if requested */
extern void png_combine_row PNGARG((png_struct *png_ptr, png_byte *row,
   int mask));
/* expand an interlaced row */
extern void png_do_read_interlace PNGARG((png_row_info *row_info,
   png_byte *row, int pass));
/* grab pixels out of a row for an interlaced pass */
extern void png_do_write_interlace PNGARG((png_row_info *row_info,
   png_byte *row, int pass));

/* unfilter a row */
extern void png_read_filter_row PNGARG((png_row_info *row_info,
   png_byte *row, png_byte *prev_row, int filter));
/* filter a row, and place the correct filter byte in the row */
extern void png_write_filter_row PNGARG((png_row_info *row_info,
   png_byte *row, png_byte *prev_row));
/* finish a row while reading, dealing with interlacing passes, etc. */
extern void png_read_finish_row PNGARG((png_struct *png_ptr));
/* initialize the row buffers, etc. */
extern void png_read_start_row PNGARG((png_struct *png_ptr));

/* these are the functions that do the transformations */
extern void png_do_read_rgbx PNGARG((png_row_info *row_info,
   png_byte *row));
extern void png_do_write_rgbx PNGARG((png_row_info *row_info,
   png_byte *row));
extern void png_do_read_xrgb PNGARG((png_row_info *row_info,
   png_byte *row));
extern void png_do_write_xrgb PNGARG((png_row_info *row_info,
   png_byte *row));
extern void png_do_swap PNGARG((png_row_info *row_info, png_byte *row));
extern void png_do_unpack PNGARG((png_row_info *row_info, png_byte *row));
extern void png_do_unshift PNGARG((png_row_info *row_info, png_byte *row,
   png_color_8 *sig_bits));
extern void png_do_invert PNGARG((png_row_info *row_info, png_byte *row));
extern void png_do_gray_to_rgb PNGARG((png_row_info *row_info,
   png_byte *row));
extern void png_do_chop PNGARG((png_row_info *row_info, png_byte *row));
extern void png_do_dither PNGARG((png_row_info *row_info,
   png_byte *row, png_byte *palette_lookup, png_byte *dither_lookup));
extern void png_do_bgr PNGARG((png_row_info *row_info, png_byte *row));
extern void png_do_pack PNGARG((png_row_info *row_info,
   png_byte *row, png_byte bit_depth));
extern void png_do_shift PNGARG((png_row_info *row_info, png_byte *row,
   png_color_8 *bit_depth));
extern void png_do_background PNGARG((png_row_info *row_info, png_byte *row,
   png_color_16 *trans_values, png_color_16 *background,
   png_color_16 *background_1,
   png_byte *gamma_table, png_byte *gamma_from_1, png_byte *gamma_to_1,
   png_uint_16 **gamma_16, png_uint_16 **gamma_16_from_1,
   png_uint_16 **gamma_16_to_1, int gamma_shift));
extern void png_do_gamma PNGARG((png_row_info *row_info, png_byte *row,
   png_byte *gamma_table, png_uint_16 **gamma_16_table,
   int gamma_shift));
extern void png_do_expand_palette PNGARG((png_row_info *row_info,
   png_byte *row, png_color *palette, png_byte *trans, int num_trans));
extern void png_do_expand PNGARG((png_row_info *row_info,
   png_byte *row, png_color_16 *trans_value));

/* unpack 16 and 32 bit values from a string */
extern png_uint_32 png_get_uint_32 PNGARG((png_byte *buf));
extern png_uint_16 png_get_uint_16 PNGARG((png_byte *buf));

/* read bytes into buf, and update png_ptr->crc */
extern void png_crc_read PNGARG((png_struct *png_ptr, png_byte *buf,
   png_uint_32 length));
/* skip length bytes, and update png_ptr->crc */
extern void png_crc_skip PNGARG((png_struct *png_ptr, png_uint_32 length));

/* the following decodes the appropriate chunks, and does error correction,
   then calls the appropriate callback for the chunk if it is valid */

/* decode the IHDR chunk */
extern void png_handle_IHDR PNGARG((png_struct *png_ptr, png_info *info,
   png_uint_32 length));
extern void png_handle_PLTE PNGARG((png_struct *png_ptr, png_info *info,
   png_uint_32 length));
extern void png_handle_gAMA PNGARG((png_struct *png_ptr, png_info *info,
   png_uint_32 length));
extern void png_handle_sBIT PNGARG((png_struct *png_ptr, png_info *info, 
   png_uint_32 length));
extern void png_handle_cHRM PNGARG((png_struct *png_ptr, png_info *info, 
   png_uint_32 length));
extern void png_handle_tRNS PNGARG((png_struct *png_ptr, png_info *info, 
   png_uint_32 length));
extern void png_handle_bKGD PNGARG((png_struct *png_ptr, png_info *info, 
   png_uint_32 length));
extern void png_handle_hIST PNGARG((png_struct *png_ptr, png_info *info, 
   png_uint_32 length));
extern void png_handle_pHYs PNGARG((png_struct *png_ptr, png_info *info, 
   png_uint_32 length));
extern void png_handle_oFFs PNGARG((png_struct *png_ptr, png_info *info, 
   png_uint_32 length));
extern void png_handle_tIME PNGARG((png_struct *png_ptr, png_info *info, 
   png_uint_32 length));
extern void png_handle_tEXt PNGARG((png_struct *png_ptr, png_info *info, 
   png_uint_32 length));
extern void png_handle_zTXt PNGARG((png_struct *png_ptr, png_info *info, 
   png_uint_32 length));

/* handle the transformations for reading and writing */
extern void png_do_read_transformations PNGARG((png_struct *png_ptr));
extern void png_do_write_transformations PNGARG((png_struct *png_ptr));

extern void png_init_read_transformations PNGARG((png_struct *png_ptr));


#endif /* PNG_INTERNAL */

/* do not put anything past this line */
#endif /* _PNG_H */
