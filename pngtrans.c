
/* pngtrans.c - transforms the data in a row
   routines used by both readers and writers

   libpng 1.0 beta 1 - version 0.71
   For conditions of distribution and use, see copyright notice in png.h
   Copyright (c) 1995 Guy Eric Schalnat, Group 42, Inc.
   June 26, 1995
   */

#define PNG_INTERNAL
#include "png.h"

/* turn on bgr to rgb mapping */
void
png_set_bgr(png_struct *png_ptr)
{
   png_ptr->transformations |= PNG_BGR;
}

/* turn on 16 bit byte swapping */
void
png_set_swap(png_struct *png_ptr)
{
   if (png_ptr->bit_depth == 16)
      png_ptr->transformations |= PNG_SWAP_BYTES;
}

/* turn on pixel packing */
void
png_set_packing(png_struct *png_ptr)
{
   if (png_ptr->bit_depth < 8)
   {
      png_ptr->transformations |= PNG_PACK;
      png_ptr->usr_bit_depth = 8;
   }
}

void
png_set_shift(png_struct *png_ptr, png_color_8 *true_bits)
{
   png_ptr->transformations |= PNG_SHIFT;
   png_ptr->shift = *true_bits;
}

int
png_set_interlace_handling(png_struct *png_ptr)
{
   if (png_ptr->interlaced)
   {
      png_ptr->transformations |= PNG_INTERLACE;
      return 7;
   }

   return 1;
}

void
png_set_rgbx(png_struct *png_ptr)
{
   png_ptr->transformations |= PNG_RGBA;
   if (png_ptr->color_type == PNG_COLOR_TYPE_RGB &&
      png_ptr->bit_depth == 8)
      png_ptr->usr_channels = 4;
}

void
png_set_xrgb(png_struct *png_ptr)
{
   png_ptr->transformations |= PNG_XRGB;
   if (png_ptr->color_type == PNG_COLOR_TYPE_RGB &&
      png_ptr->bit_depth == 8)
      png_ptr->usr_channels = 4;
}

void
png_set_invert_mono(png_struct *png_ptr)
{
   png_ptr->transformations |= PNG_INVERT_MONO;
}

/* invert monocrome grayscale data */
void
png_do_invert(png_row_info *row_info, png_byte *row)
{
   if (row && row_info && row_info->bit_depth == 1 &&
      row_info->color_type == PNG_COLOR_TYPE_GRAY)
   {
      png_byte *rp;
      png_uint_32 i;

      for (i = 0, rp = row;
         i < row_info->rowbytes;
         i++, rp++)
      {
         *rp = ~(*rp);
      }
   }
}

/* swaps byte order on 16 bit depth images */
void
png_do_swap(png_row_info *row_info, png_byte *row)
{
   if (row && row_info && row_info->bit_depth == 16)
   {
      png_byte *rp, t;
      png_uint_32 i;

      for (i = 0, rp = row;
         i < row_info->width * row_info->channels;
         i++, rp += 2)
      {
         t = *rp;
         *rp = *(rp + 1);
         *(rp + 1) = t;
      }
   }
}

/* swaps red and blue */
void
png_do_bgr(png_row_info *row_info, png_byte *row)
{
   if (row && row_info && (row_info->color_type & 2))
   {
      if (row_info->color_type == 2 && row_info->bit_depth == 8)
      {
         png_byte *rp, t;
         png_uint_32 i;

         for (i = 0, rp = row;
            i < row_info->width;
            i++, rp += 3)
         {
            t = *rp;
            *rp = *(rp + 2);
            *(rp + 2) = t;
         }
      }
      else if (row_info->color_type == 6 && row_info->bit_depth == 8)
      {
         png_byte *rp, t;
         png_uint_32 i;

         for (i = 0, rp = row;
            i < row_info->width;
            i++, rp += 4)
         {
            t = *rp;
            *rp = *(rp + 2);
            *(rp + 2) = t;
         }
      }
      else if (row_info->color_type == 2 && row_info->bit_depth == 16)
      {
         png_byte *rp, t[2];
         png_uint_32 i;

         for (i = 0, rp = row;
            i < row_info->width;
            i++, rp += 6)
         {
            t[0] = *rp;
            t[1] = *(rp + 1);
            *rp = *(rp + 4);
            *(rp + 1) = *(rp + 5);
            *(rp + 4) = t[0];
            *(rp + 5) = t[1];
         }
      }
      else if (row_info->color_type == 6 && row_info->bit_depth == 16)
      {
         png_byte *rp, t[2];
         png_uint_32 i;

         for (i = 0, rp = row;
            i < row_info->width;
            i++, rp += 8)
         {
            t[0] = *rp;
            t[1] = *(rp + 1);
            *rp = *(rp + 4);
            *(rp + 1) = *(rp + 5);
            *(rp + 4) = t[0];
            *(rp + 5) = t[1];
         }
      }
   }
}

