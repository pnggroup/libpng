
/* filter_vsx_intrinsics.c - PowerPC optimised filter functions
 *
 * Copyright (c) 2016 Glenn Randers-Pehrson
 * Written by Vadim Barkov, 2017.
 * Last changed in libpng 1.6.25 [September 1, 2016]
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */
#include <stdio.h>
#include <stdint.h>
#include "../pngpriv.h"

#ifdef PNG_READ_SUPPORTED

/* This code requires -maltivec and -mabi=altivec on the command line: */
#if PNG_POWERPC_VSX_IMPLEMENTATION == 1 /* intrinsics code from pngpriv.h */

/* libpng row pointers are not necessarily aligned to any particular boundary,
 * however this code will only work with appropriate alignment.  arm/arm_init.c
 * checks for this (and will not compile unless it is done). This code uses
 * variants of png_aligncast to avoid compiler warnings.
 */
#define png_ptr(type,pointer) png_aligncast(type *,pointer)
#define png_ptrc(type,pointer) png_aligncastconst(const type *,pointer)

/*#include <altivec.h>*/

#if PNG_POWERPC_VSX_OPT > 0

void png_read_filter_row_up_vsx(png_row_infop row_info, png_bytep row,
                                png_const_bytep prev_row)
{
   png_size_t i;
   png_size_t istop = row_info->rowbytes;
   png_bytep rp = row;
   png_const_bytep pp = prev_row;

   for (i = 0; i < istop; i++)
   {
      *rp = (png_byte)(((int)(*rp) + (int)(*pp++)) & 0xff);
      rp++;
   }

}

void png_read_filter_row_sub4_vsx(png_row_infop row_info, png_bytep row,
                                  png_const_bytep prev_row)
{
   png_size_t i;
   png_size_t istop = row_info->rowbytes;
   unsigned int bpp = (row_info->pixel_depth + 7) >> 3;
   png_bytep rp = row + bpp;

   PNG_UNUSED(prev_row)

   for (i = bpp; i < istop; i++)
   {
      *rp = (png_byte)(((int)(*rp) + (int)(*(rp-bpp))) & 0xff);
      rp++;
   }
}

void png_read_filter_row_sub3_vsx(png_row_infop row_info, png_bytep row,
                                  png_const_bytep prev_row)
{
    png_size_t i;
    png_size_t istop = row_info->rowbytes;
    unsigned int bpp = (row_info->pixel_depth + 7) >> 3;
    png_bytep rp = row + bpp;

    PNG_UNUSED(prev_row)

    for (i = bpp; i < istop; i++)
    {
       *rp = (png_byte)(((int)(*rp) + (int)(*(rp-bpp))) & 0xff);
       rp++;
    }
}

void png_read_filter_row_avg4_vsx(png_row_infop row_info, png_bytep row,
                                  png_const_bytep prev_row)
{
    png_size_t i;
    png_bytep rp = row;
    png_const_bytep pp = prev_row;
    unsigned int bpp = (row_info->pixel_depth + 7) >> 3;
    png_size_t istop = row_info->rowbytes - bpp;

    for (i = 0; i < bpp; i++)
    {
       *rp = (png_byte)(((int)(*rp) +
          ((int)(*pp++) / 2 )) & 0xff);

       rp++;
    }

    for (i = 0; i < istop; i++)
    {
       *rp = (png_byte)(((int)(*rp) +
          (int)(*pp++ + *(rp-bpp)) / 2 ) & 0xff);

       rp++;
    }
}

void png_read_filter_row_avg3_vsx(png_row_infop row_info, png_bytep row,
                                  png_const_bytep prev_row)
{
    png_size_t i;
    png_bytep rp = row;
    png_const_bytep pp = prev_row;
    unsigned int bpp = (row_info->pixel_depth + 7) >> 3;
    png_size_t istop = row_info->rowbytes - bpp;

    for (i = 0; i < bpp; i++)
    {
       *rp = (png_byte)(((int)(*rp) +
          ((int)(*pp++) / 2 )) & 0xff);

       rp++;
    }

    for (i = 0; i < istop; i++)
    {
       *rp = (png_byte)(((int)(*rp) +
          (int)(*pp++ + *(rp-bpp)) / 2 ) & 0xff);

       rp++;
    }
}

void png_read_filter_row_paeth4_vsx(png_row_infop row_info,
                                    png_bytep row,
                                    png_const_bytep prev_row)
{
    unsigned int bpp = (row_info->pixel_depth + 7) >> 3;
    png_bytep rp_end = row + bpp;

    /* Process the first pixel in the row completely (this is the same as 'up'
     * because there is only one candidate predictor for the first row).
     */
    while (row < rp_end)
    {
       int a = *row + *prev_row++;
       *row++ = (png_byte)a;
    }

    /* Remainder */
    rp_end = rp_end + (row_info->rowbytes - bpp);

    while (row < rp_end)
    {
       int a, b, c, pa, pb, pc, p;

       c = *(prev_row - bpp);
       a = *(row - bpp);
       b = *prev_row++;

       p = b - c;
       pc = a - c;

 #ifdef PNG_USE_ABS
       pa = abs(p);
       pb = abs(pc);
       pc = abs(p + pc);
 #else
       pa = p < 0 ? -p : p;
       pb = pc < 0 ? -pc : pc;
       pc = (p + pc) < 0 ? -(p + pc) : p + pc;
 #endif

       if (pb < pa) pa = pb, a = b;
       if (pc < pa) a = c;

       a += *row;
       *row++ = (png_byte)a;
    }
}

void png_read_filter_row_paeth3_vsx(png_row_infop row_info,
                                    png_bytep row,
                                    png_const_bytep prev_row)
{
    unsigned int bpp = (row_info->pixel_depth + 7) >> 3;
    png_bytep rp_end = row + bpp;

    /* Process the first pixel in the row completely (this is the same as 'up'
     * because there is only one candidate predictor for the first row).
     */
    while (row < rp_end)
    {
       int a = *row + *prev_row++;
       *row++ = (png_byte)a;
    }

    /* Remainder */
    rp_end = rp_end + (row_info->rowbytes - bpp);

    while (row < rp_end)
    {
       int a, b, c, pa, pb, pc, p;

       c = *(prev_row - bpp);
       a = *(row - bpp);
       b = *prev_row++;

       p = b - c;
       pc = a - c;

 #ifdef PNG_USE_ABS
       pa = abs(p);
       pb = abs(pc);
       pc = abs(p + pc);
 #else
       pa = p < 0 ? -p : p;
       pb = pc < 0 ? -pc : pc;
       pc = (p + pc) < 0 ? -(p + pc) : p + pc;
 #endif

       if (pb < pa) pa = pb, a = b;
       if (pc < pa) a = c;

       a += *row;
       *row++ = (png_byte)a;
    }
}

#endif /* PNG_POWERPC_VSX_OPT > 0 */
#endif /* PNG_POWERPC_VSX_IMPLEMENTATION == 1 (intrinsics) */
#endif /* READ */
