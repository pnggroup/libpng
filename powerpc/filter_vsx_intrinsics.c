
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

#include <altivec.h>

#if PNG_POWERPC_VSX_OPT > 0

/* Functions in this file look at most 3 pixels (a,b,c) to predict the 4th (d).
 * They're positioned like this:
 *    prev:  c b
 *    row:   a d
 * The Sub filter predicts d=a, Avg d=(a+b)/2, and Paeth predicts d to be
 * whichever of a, b, or c is closest to p=a+b-c.
 * ( this is taken from ../intel/filter_sse2_intrinsics.c )
 */


void png_read_filter_row_up_vsx(png_row_infop row_info, png_bytep row,
                                png_const_bytep prev_row)
{
   png_size_t i;
   png_size_t unaligned_top = 16 - ((png_size_t)row % 16);
   png_size_t istop = row_info->rowbytes - unaligned_top;
   png_bytep rp = row;
   png_const_bytep pp = prev_row;

   vector unsigned char rp_vec;
   vector unsigned char pp_vec;

   /* Altivec operations require 16-byte aligned data 
    * but input can be unaligned. So we calculate 
    * unaligned part as usual.
    */
   for (i = 0; i < unaligned_top; i++)
   {
      *rp = (png_byte)(((int)(*rp) + (int)(*pp++)) & 0xff);
      rp++;
   }

   /* Using SIMD while we can */
   while( istop >= 16 )
   {
      rp_vec = vec_ld(0,rp);
      pp_vec = vec_ld(0,pp);
   
      rp_vec = vec_add(rp_vec,pp_vec);

      vec_st(rp_vec,0,rp);

      pp += 16;
      rp += 16;
      istop -= 16;
   }

   if(istop % 16 > 0)
   {
      /* If byte count of row is not divisible by 16
       * we will process remaining part as usual
       */
      for (i = 0; i < istop; i++)
      {
         *rp = (png_byte)(((int)(*rp) + (int)(*pp++)) & 0xff);
         rp++;
      }
}

}

void png_read_filter_row_sub_vsx(png_row_infop row_info, png_bytep row,
                                  png_const_bytep prev_row)
{
   const unsigned int bpp = 4;
   png_size_t i;
   png_size_t istop = row_info->rowbytes;
   png_bytep rp = row + bpp;

   PNG_UNUSED(prev_row)

   for (i = bpp; i < istop; i++)
   {
      *rp = (png_byte)(((int)(*rp) + (int)(*(rp-4))) & 0xff);
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
