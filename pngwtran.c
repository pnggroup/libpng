
/* pngwtran.c - transforms the data in a row for PNG writers
 *
 * Last changed in libpng 1.6.17 [(PENDING RELEASE)]
 * Copyright (c) 1998-2015 Glenn Randers-Pehrson
 * (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
 * (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */

#include "pngpriv.h"
#define PNG_SRC_FILE PNG_SRC_FILE_pngwtran

#ifdef PNG_WRITE_SUPPORTED
#ifdef PNG_WRITE_TRANSFORMS_SUPPORTED

#ifdef PNG_WRITE_PACK_SUPPORTED
/* Pack pixels into bytes.  Get the true bit depth from png_ptr.  The
 * row_info bit depth should be 8 (one pixel per byte).  The channels
 * should be 1 (this only happens on grayscale and paletted images).
 */
static void
png_do_pack(png_transform_controlp row_info, png_bytep row)
{
   png_debug(1, "in png_do_pack");

#  define png_ptr row_info->png_ptr

   /* The comment suggests the following must be true.
    * TODO: test this.
    */
   affirm(row_info->bit_depth == 8 && row_info->channels == 1);

   {
      switch (png_ptr->bit_depth)
      {
         case 1:
         {
            png_const_bytep ep = row + png_transform_rowbytes(row_info);
            png_bytep dp = row;
            unsigned int mask = 0x80, v = 0;

            while (row < ep)
            {
               if (*row++ != 0)
                  v |= mask;

               mask >>= 1;

               if (mask == 0)
               {
                  mask = 0x80;
                  *dp++ = (png_byte)/*SAFE*/v;
                  v = 0;
               }
            }

            if (mask != 0x80)
               *dp++ = (png_byte)/*SAFE*/v;

            row_info->bit_depth = 1;
            break;
         }

         case 2:
         {
            png_const_bytep ep = row + png_transform_rowbytes(row_info);
            png_bytep dp = row;
            unsigned int shift = 8, v = 0;

            while (row < ep)
            {
               shift -= 2;
               v |= (*row++ & 0x3) << shift;

               if (shift == 0)
               {
                  shift = 8;
                  *dp++ = png_check_byte(png_ptr, v);
                  v = 0;
               }
            }

            if (shift != 8)
               *dp++ = png_check_byte(png_ptr, v);

            row_info->bit_depth = 2;
            break;
         }

         case 4:
         {
            png_const_bytep ep = row + png_transform_rowbytes(row_info);
            png_bytep dp = row;
            unsigned int shift = 8, v = 0;

            while (row < ep)
            {
               shift -= 4;
               v |= ((*row++ & 0xf) << shift);

               if (shift == 0)
               {
                  shift = 8;
                  *dp++ = png_check_byte(png_ptr, v);
                  v = 0;
               }
            }

            if (shift != 8)
               *dp++ = png_check_byte(png_ptr, v);

            row_info->bit_depth = 4;
            break;
         }

         default:
            break;
      }
   }
#  undef png_ptr
}
#endif

#ifdef PNG_WRITE_SHIFT_SUPPORTED
/* Shift pixel values to take advantage of whole range.  Pass the
 * true number of bits in bit_depth.  The row should be packed
 * according to row_info->bit_depth.  Thus, if you had a row of
 * bit depth 4, but the pixels only had values from 0 to 7, you
 * would pass 3 as bit_depth, and this routine would translate the
 * data to 0 to 15.
 *
 * NOTE: this is horrible complexity for no value.  Once people suggested they
 * were selling 16-bit displays with 5:6:5 bits spread R:G:B but so far as I
 * could determine these displays produced intermediate grey (uncolored) colors,
 * which is impossible with a true 5:6:5, so most likely 5:6:5 was marketing.
 */
static void
png_do_shift(png_transform_controlp row_info, png_bytep row)
{
   png_debug(1, "in png_do_shift");

#  define png_ptr row_info->png_ptr

   if (!(row_info->flags & PNG_INDEXED) && (row_info->channels-1) <= 3)
   {
      png_const_color_8p bit_depth = &png_ptr->shift;
      int shift_start[4], shift_dec[4];
      int channels = 0;

      if (row_info->channels == 3 || row_info->channels == 4)
      {
         shift_start[channels] = row_info->bit_depth - bit_depth->red;
         shift_dec[channels] = bit_depth->red;
         channels++;

         shift_start[channels] = row_info->bit_depth - bit_depth->green;
         shift_dec[channels] = bit_depth->green;
         channels++;

         shift_start[channels] = row_info->bit_depth - bit_depth->blue;
         shift_dec[channels] = bit_depth->blue;
         channels++;
      }

      else /* 1 or 2 channels */
      {
         shift_start[channels] = row_info->bit_depth - bit_depth->gray;
         shift_dec[channels] = bit_depth->gray;
         channels++;
      }

      if (row_info->channels == 2 || row_info->channels == 4)
      {
         shift_start[channels] = row_info->bit_depth - bit_depth->alpha;
         shift_dec[channels] = bit_depth->alpha;
         channels++;
      }

      /* With low res depths, could only be grayscale, so one channel */
      if (row_info->bit_depth < 8)
      {
         png_bytep bp = row;
         png_size_t i;
         unsigned int mask;
         size_t row_bytes = png_transform_rowbytes(row_info);

         affirm(row_info->channels == 1);

         if (bit_depth->gray == 1 && row_info->bit_depth == 2)
            mask = 0x55;

         else if (row_info->bit_depth == 4 && bit_depth->gray == 3)
            mask = 0x11;

         else
            mask = 0xff;

         for (i = 0; i < row_bytes; i++, bp++)
         {
            int j;
            unsigned int v, out;

            v = *bp;
            out = 0;

            for (j = shift_start[0]; j > -shift_dec[0]; j -= shift_dec[0])
            {
               if (j > 0)
                  out |= v << j;

               else
                  out |= (v >> (-j)) & mask;
            }

            *bp = png_check_byte(png_ptr, out);
         }
      }

      else if (row_info->bit_depth == 8)
      {
         png_bytep bp = row;
         png_uint_32 i;
         png_uint_32 istop = channels * row_info->width;

         for (i = 0; i < istop; i++, bp++)
         {

            const unsigned int c = i%channels;
            int j;
            unsigned int v, out;

            v = *bp;
            out = 0;

            for (j = shift_start[c]; j > -shift_dec[c]; j -= shift_dec[c])
            {
               if (j > 0)
                  out |= v << j;

               else
                  out |= v >> (-j);
            }

            *bp = png_check_byte(png_ptr, out);
         }
      }

      else
      {
         png_bytep bp;
         png_uint_32 i;
         png_uint_32 istop = channels * row_info->width;

         for (bp = row, i = 0; i < istop; i++)
         {
            const unsigned int c = i%channels;
            int j;
            unsigned int value, v;

            v = png_get_uint_16(bp);
            value = 0;

            for (j = shift_start[c]; j > -shift_dec[c]; j -= shift_dec[c])
            {
               if (j > 0)
                  value |= v << j;

               else
                  value |= v >> (-j);
            }
            *bp++ = png_check_byte(png_ptr, value >> 8);
            *bp++ = PNG_BYTE(value);
         }
      }
   }

#  undef png_ptr
}
#endif

#ifdef PNG_WRITE_SWAP_ALPHA_SUPPORTED
static void
png_do_write_swap_alpha(png_transform_controlp row_info, png_bytep row)
{
   png_debug(1, "in png_do_write_swap_alpha");

#  define png_ptr row_info->png_ptr
   {
      if (row_info->channels == 4)
      {
         if (row_info->bit_depth == 8)
         {
            png_const_bytep ep = row + png_transform_rowbytes(row_info) - 4;

            /* This converts from ARGB to RGBA */
            while (row <= ep)
            {
               png_byte save = row[0];
               row[0] = row[1];
               row[1] = row[2];
               row[2] = row[3];
               row[3] = save;
               row += 4;
            }

            debug(row == ep+4);
         }

#ifdef PNG_WRITE_16BIT_SUPPORTED
         else if (row_info->bit_depth == 16)
         {
            /* This converts from AARRGGBB to RRGGBBAA */
            png_const_bytep ep = row + png_transform_rowbytes(row_info) - 8;

            while (row <= ep)
            {
               png_byte s0 = row[0];
               png_byte s1 = row[1];
               memmove(row, row+2, 6);
               row[6] = s0;
               row[7] = s1;
               row += 8;
            }

            debug(row == ep+8);
         }
#endif /* WRITE_16BIT */
      }

      else if (row_info->channels == 2)
      {
         if (row_info->bit_depth == 8)
         {
            /* This converts from AG to GA */
            png_const_bytep ep = row + png_transform_rowbytes(row_info) - 2;

            /* This converts from ARGB to RGBA */
            while (row <= ep)
            {
               png_byte save = *row;
               *row = row[1], ++row;
               *row++ = save;
            }

            debug(row == ep+2);
         }

#ifdef PNG_WRITE_16BIT_SUPPORTED
         else
         {
            /* This converts from AAGG to GGAA */
            png_const_bytep ep = row + png_transform_rowbytes(row_info) - 4;

            while (row <= ep)
            {
               png_byte save = row[0];
               row[0] = row[2];
               row[2] = save;

               save = row[1];
               row[1] = row[3];
               row[3] = save;

               row += 4;
            }

            debug(row == ep+4);
         }
#endif /* WRITE_16BIT */
      }
   }

#  undef png_ptr
}
#endif

#ifdef PNG_WRITE_INVERT_ALPHA_SUPPORTED
static void
png_do_write_invert_alpha(png_transform_controlp row_info, png_bytep row)
{
   png_debug(1, "in png_do_write_invert_alpha");

#  define png_ptr row_info->png_ptr
   {
      if (row_info->channels == 4)
      {
         if (row_info->bit_depth == 8)
         {
            /* This inverts the alpha channel in RGBA */
            png_const_bytep ep = row + png_transform_rowbytes(row_info) - 1;

            row += 3; /* alpha channel */
            while (row <= ep)
               *row ^= 0xff, row += 4;
         }

#ifdef PNG_WRITE_16BIT_SUPPORTED
         else if (row_info->bit_depth == 16)
         {
            /* This inverts the alpha channel in RRGGBBAA */
            png_const_bytep ep = row + png_transform_rowbytes(row_info) - 2;

            row += 6;

            while (row <= ep)
               row[0] ^= 0xff, row[1] ^= 0xff, row += 8;
         }
#endif /* WRITE_16BIT */
      }

      else if (row_info->channels == 2)
      {
         if (row_info->bit_depth == 8)
         {
            /* This inverts the alpha channel in GA */
            png_const_bytep ep = row + png_transform_rowbytes(row_info) - 1;

            ++row;

            while (row <= ep)
               *row ^= 0xff, row += 2;
         }

#ifdef PNG_WRITE_16BIT_SUPPORTED
         else
         {
            /* This inverts the alpha channel in GGAA */
            png_const_bytep ep = row + png_transform_rowbytes(row_info) - 2;

            row += 2;

            while (row <= ep)
               row[0] ^= 0xff, row[1] ^= 0xff, row += 4;
         }
#endif /* WRITE_16BIT */
      }
   }
#  undef png_ptr
}
#endif

/* Transform the data according to the user's wishes.  The order of
 * transformations is significant.
 */
void /* PRIVATE */
png_do_write_transformations(png_structrp png_ptr, png_row_infop row_info_in)
{
   png_transform_control display;

   png_debug(1, "in png_do_write_transformations");

   if (png_ptr == NULL)
      return;

#ifdef PNG_WRITE_USER_TRANSFORM_SUPPORTED
   if ((png_ptr->transformations & PNG_USER_TRANSFORM) != 0)
      if (png_ptr->write_user_transform_fn != NULL)
         (*(png_ptr->write_user_transform_fn)) /* User write transform
                                                 function */
             (png_ptr,  /* png_ptr */
              row_info_in,  /* row_info: */
                /*  png_uint_32 width;       width of row */
                /*  png_size_t rowbytes;     number of bytes in row */
                /*  png_byte color_type;     color type of pixels */
                /*  png_byte bit_depth;      bit depth of samples */
                /*  png_byte channels;       number of channels (1-4) */
                /*  png_byte pixel_depth;    bits per pixel (depth*channels) */
             png_ptr->row_buf + 1);      /* start of pixel data for row */
#endif

   png_init_transform_control(png_ptr, &display, row_info_in);

#ifdef PNG_WRITE_FILLER_SUPPORTED
   if ((png_ptr->transformations & PNG_FILLER) != 0)
      png_do_strip_channel(&display, png_ptr->row_buf + 1,
         !(png_ptr->flags & PNG_FLAG_FILLER_AFTER));
#endif

#ifdef PNG_WRITE_PACKSWAP_SUPPORTED
   if ((png_ptr->transformations & PNG_PACKSWAP) != 0)
      png_do_packswap(&display, png_ptr->row_buf + 1);
#endif

#ifdef PNG_WRITE_PACK_SUPPORTED
   if ((png_ptr->transformations & PNG_PACK) != 0)
      png_do_pack(&display, png_ptr->row_buf + 1);
#endif

#ifdef PNG_WRITE_SWAP_SUPPORTED
#  ifdef PNG_16BIT_SUPPORTED
   if ((png_ptr->transformations & PNG_SWAP_BYTES) != 0)
      png_do_swap(&display, png_ptr->row_buf + 1);
#  endif
#endif

#ifdef PNG_WRITE_SHIFT_SUPPORTED
   if ((png_ptr->transformations & PNG_SHIFT) != 0)
      png_do_shift(&display, png_ptr->row_buf + 1);
#endif

#ifdef PNG_WRITE_SWAP_ALPHA_SUPPORTED
   if ((png_ptr->transformations & PNG_SWAP_ALPHA) != 0)
      png_do_write_swap_alpha(&display, png_ptr->row_buf + 1);
#endif

#ifdef PNG_WRITE_INVERT_ALPHA_SUPPORTED
   if ((png_ptr->transformations & PNG_INVERT_ALPHA) != 0)
      png_do_write_invert_alpha(&display, png_ptr->row_buf + 1);
#endif

#ifdef PNG_WRITE_BGR_SUPPORTED
   if ((png_ptr->transformations & PNG_BGR) != 0)
      png_do_bgr(&display, png_ptr->row_buf + 1);
#endif

#ifdef PNG_WRITE_INVERT_SUPPORTED
   if ((png_ptr->transformations & PNG_INVERT_MONO) != 0)
      png_do_invert(&display, png_ptr->row_buf + 1);
#endif

   /* Clear the flags; they are irrelevant because the write code is
    * reversing transformations to get PNG data but the shared transformation
    * code assumes input PNG data.  Only PNG_INDEXED is required.
    */
   if ((display.flags & PNG_BAD_INDEX) != 0)
      png_error(png_ptr, "palette data has out of range index");

   display.flags &= PNG_INDEXED;
   png_end_transform_control(row_info_in, &display);
}
#endif /* WRITE_TRANSFORMS */
#endif /* WRITE */
