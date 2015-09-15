
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

#ifdef PNG_WRITE_INTERLACING_SUPPORTED
/* Pick out the correct pixels for the interlace pass.
 * The basic idea here is to go through the row with a source
 * pointer and a destination pointer (sp and dp), and copy the
 * correct pixels for the pass.  As the row gets compacted,
 * sp will always be >= dp, so we should never overwrite anything.
 * See the default: case for the easiest code to understand.
 */
static void
png_do_write_interlace_lbd(png_transformp *transform, png_transform_controlp tc)
{
   const png_const_structrp png_ptr = tc->png_ptr;
   const unsigned int pass = png_ptr->pass;
   const png_uint_32 row_width = tc->width;
   const png_uint_32 output_width = PNG_PASS_COLS(row_width, pass);
   png_uint_32 i = PNG_PASS_START_COL(pass);


   png_debug(1, "in png_do_write_interlace");
   debug(!tc->init);

   /* The data can be used in place (tc->sp) if the width isn't changed or
    * the first pixel in the output is the first in the input and there is
    * only one pixel in the output; this covers the last pass (PNG pass 7,
    * libpng 6) and PNG passes 1, 3 and 5 with narrow images.
    */
   tc->width = output_width;

   if (row_width != output_width && (output_width != 1 || i > 0))
   {
      /* For passes before the last the pixels must be picked from the input
       * row (tc->sp) and placed into the output row (tc->dp).
       */
      png_const_bytep sp = png_voidcast(png_const_bytep, tc->sp);
      png_bytep dp = png_voidcast(png_bytep, tc->dp);
      const unsigned int inc = PNG_PASS_COL_OFFSET(pass);
      unsigned int B = (*transform)->args & 0x3; /* 0, 1 or 2 */

      /* The row data will be moved, so do this now before 'dp' is advanced:
       */
      tc->sp = dp;

      /* For pixels less than one byte wide the correct pixels have to be
       * extracted from the input bytes.  Because we are reading data in
       * the application memory format we cannot rely on the PNG big
       * endian order.  Notice that this was apparently broken before
       * 1.7.0.
       *
       * In libpng 1.7.0 libpng uses a classic bit-pump to optimize the
       * extraction.  In all passes before the last (6/7) no two pixels
       * are adjacent in the input, so we are always extracting 1 bit.
       * At present the code uses an 8-bit buffer to avoid coding for
       * different byte sexes, but this could easily be changed.
       *
       * 'i' is the bit-index of bit in the input (sp[]), so,
       * considering the 1-bit per pixel case, sp[i>>3] is the byte
       * and the bit is bit (i&7) (0 lowest) on swapped (little endian)
       * data or 7-(i&7) on PNG default (big-endian) data.
       *
       * Define these macros, where:
       *
       *    B: the log2 bit depth (0, 1, 2 for 1bpp, 2bpp or 4bpp) of
       *       the data; this should be a constant.
       *   sp: the source pointer (sp) (a png_const_bytep)
       *    i: the pixel index in the input (png_uint_32)
       *    j: the bit index in the output (unsigned int)
       *
       * Unlike 'i', 'j' is interpreted directly; for LSB bytes it counts
       * up, for MSB it counts down.
       *
       * NOTE: this could all be expanded to eliminate the code below by
       * the time honoured copy'n'paste into three separate functions.  This
       * might be worth doing in the future.
       */
#     define PIXEL_MASK     ((1U << (1<<B))-1U)
#     define BIT_MASK       ((1U << (3-(B)))-1U) /* within a byte */
#     define SP_BYTE        (sp[i>>(3-(B))]) /* byte to use */
#     define SP_OFFSET_LSB  ((BIT_MASK &  i) << (B))
#     define SP_OFFSET_MSB  ((BIT_MASK & ~i) << (B))
#     define SP_PIXEL(sex)  ((SP_BYTE >> SP_OFFSET_ ## sex) & PIXEL_MASK)
      {
         unsigned int j;
         unsigned int d;

#        ifdef PNG_WRITE_PACKSWAP_SUPPORTED
            if (tc->format & PNG_FORMAT_FLAG_SWAPPED)
               for (j = 0, d = 0; i < row_width; i += inc)
            {  /* little-endian */
               d |= SP_PIXEL(LSB) << j;
               j += 1<<B;
               if (j == 8) *dp++ = png_check_byte(png_ptr, d), j = 0, d = 0;
            }

            else
#        endif /* WRITE_PACKSWAP */
         for (j = 8, d = 0; i < row_width; i += inc)
         {  /* big-endian */
            j -= 1<<B;
            d |= SP_PIXEL(MSB) << j;
            if (j == 0) *dp++ = png_check_byte(png_ptr, d), j = 8, d = 0;
         }

         /* The end condition: if j is not 0 the last byte was not
          * written:
          */
         if (j != 0) *dp = png_check_byte(png_ptr, d);
      }
#     undef PIXEL_MASK
#     undef BIT_MASK
#     undef SP_BYTE
#     undef SP_OFFSET_MSB
#     undef SP_OFFSET_LSB
#     undef SP_PIXEL
   }

   /* The transform is removed on the last pass.  It can be removed earlier
    * in other cases if the row width (the image width) is only 1, however
    * this does not seem worth the overhead to check; PNG pass 5(4) happens
    * if there are just three rows.
    */
   else /* the source can be used in place */ if (pass == 6)
      (*transform)->fn = NULL; /* remove me to caller */
}

static void
png_do_write_interlace_byte(png_transformp *transform,
   png_transform_controlp tc)
{
   const png_const_structrp png_ptr = tc->png_ptr;
   const unsigned int pass = png_ptr->pass;
   const png_uint_32 row_width = tc->width;
   const png_uint_32 output_width = PNG_PASS_COLS(row_width, pass);
   png_uint_32 i = PNG_PASS_START_COL(pass);

   png_debug(1, "in png_do_write_interlace");
   debug(!tc->init);

   /* The data can be used in place (tc->sp) if the width isn't changed or
    * the first pixel in the output is the first in the input and there is
    * only one pixel in the output; this covers the last pass (PNG pass 7,
    * libpng 6) and PNG passes 1, 3 and 5 with narrow images.
    */
   tc->width = output_width;

   if (row_width != output_width && (output_width != 1 || i > 0))
   {
      /* For passes before the last the pixels must be picked from the input
       * row (tc->sp) and placed into the output row (tc->dp).
       */
      png_const_bytep sp = png_voidcast(png_const_bytep, tc->sp);
      png_bytep dp = png_voidcast(png_bytep, tc->dp);
      const unsigned int inc = PNG_PASS_COL_OFFSET(pass);
      unsigned int cbytes = (*transform)->args;

      /* The row data will be moved, so do this now before 'dp' is advanced:
       */
      tc->sp = dp;

      /* Loop through the input copying each pixel to the correct place
       * in the output.  Note that the loop may be executed 0 times if
       * this is called on a narrow image that does not contain this
       * pass.
       */
      for (sp += i * cbytes; i < row_width;
           i += inc, sp += inc * cbytes, dp += cbytes)
         if (dp != sp) /* cannot happen in practice */
            memcpy(dp, sp, cbytes);
   }

   /* The transform is removed on the last pass.  It can be removed earlier
    * in other cases if the row width (the image width) is only 1, however
    * this does not seem worth the overhead to check; PNG pass 5(4) happens
    * if there are just three rows.
    */
   else /* the source can be used in place */ if (pass == 6)
      (*transform)->fn = NULL; /* remove me to caller */
}

static void
png_init_write_interlace(png_transformp *transform, png_transform_controlp tc)
{
#  define png_ptr (tc->png_ptr)

   png_debug(1, "in png_do_write_interlace");
   debug(tc->init);

   /* Do nothing on PNG_TC_INIT_FORMAT because we don't change the format, bit
    * depth or gamma of the data.
    */
   if (tc->init == PNG_TC_INIT_FINAL)
   {
      png_transformp tf = *transform;
      unsigned int pixel_depth = PNG_TC_PIXEL_DEPTH(*tc);
      png_uint_16 B = 0;

      switch (pixel_depth)
      {
         case 4: /* B == 2 */
            ++B;
            /* FALL THROUGH */
         case 2: /* B == 1 */
            ++B;
            /* FALL THROUGH */
         case 1: /* B == 0 */
            /* This is the low bit depth case: */
            tf->args = B;
            tf->fn = png_do_write_interlace_lbd;
            break;

         default:
            affirm((pixel_depth & 7) == 0);
            pixel_depth >>= 3;
            affirm(pixel_depth > 0 && pixel_depth <= 8);
            tf->args = pixel_depth & 0xf;
            tf->fn = png_do_write_interlace_byte;
            break;
      }
   }
#  undef png_ptr
}

void /* PRIVATE */
png_set_write_interlace(png_structrp png_ptr)
{
   /* This is checked in the caller: */
   debug(png_ptr->interlaced == PNG_INTERLACE_ADAM7);
   png_add_transform(png_ptr, 0, png_init_write_interlace, PNG_TR_INTERLACE);
}
#endif /* WRITE_INTERLACING */

#ifdef PNG_WRITE_PACK_SUPPORTED
/* Pack pixels into bytes. */
static void
png_do_write_pack(png_transformp *transform, png_transform_controlp tc)
{
   png_alloc_size_t rowbytes = PNG_TC_ROWBYTES(*tc);
   png_const_bytep sp = png_voidcast(png_const_bytep, tc->sp);
   png_const_bytep ep = png_upcast(png_const_bytep, tc->sp) + rowbytes;
   png_bytep dp = png_voidcast(png_bytep, tc->dp);

   png_debug(1, "in png_do_pack");

#  define png_ptr tc->png_ptr

   switch ((*transform)->args)
   {
      case 1:
      {
         unsigned int mask = 0x80, v = 0;

         while (sp < ep)
         {
            if (*sp++ != 0)
               v |= mask;

            mask >>= 1;

            if (mask == 0)
            {
               mask = 0x80;
               *dp++ = PNG_BYTE(v);
               v = 0;
            }
         }

         if (mask != 0x80)
            *dp++ = PNG_BYTE(v);
         break;
      }

      case 2:
      {
         unsigned int shift = 8, v = 0;

         while (sp < ep)
         {
            shift -= 2;
            v |= (*sp++ & 0x3) << shift;

            if (shift == 0)
            {
               shift = 8;
               *dp++ = PNG_BYTE(v);
               v = 0;
            }
         }

         if (shift != 8)
            *dp++ = PNG_BYTE(v);
         break;
      }

      case 4:
      {
         unsigned int shift = 8, v = 0;

         while (sp < ep)
         {
            shift -= 4;
            v |= ((*sp++ & 0xf) << shift);

            if (shift == 0)
            {
               shift = 8;
               *dp++ = PNG_BYTE(v);
               v = 0;
            }
         }

         if (shift != 8)
            *dp++ = PNG_BYTE(v);
         break;
      }

      default:
         impossible("bit depth");
   }

   if ((tc->format & PNG_FORMAT_FLAG_COLORMAP) == 0 &&
       --(tc->range) == 0)
      tc->format &= PNG_BIC_MASK(PNG_FORMAT_FLAG_RANGE);

   tc->bit_depth = (*transform)->args;
   tc->sp = tc->dp;
#  undef png_ptr
}

void /* PRIVATE */
png_init_write_pack(png_transformp *transform, png_transform_controlp tc)
{
#  define png_ptr tc->png_ptr
   debug(tc->init);
#  undef png_ptr

   /* The init routine is called *forward* so the transform control we get has
    * the required bit depth and the transform routine will increase it to 8
    * bits per channel.  The code doesn't really care how many channels there
    * are, but the only way to get a channel depth of less than 8 is to have
    * just one channel.
    */
   if (tc->bit_depth < 8) /* else no packing/unpacking */
   {
      if (tc->init == PNG_TC_INIT_FINAL)
      {
         (*transform)->fn = png_do_write_pack;
         /* Record this for the backwards run: */
         (*transform)->args = tc->bit_depth & 0xf;
      }

      if ((tc->format & PNG_FORMAT_FLAG_COLORMAP) == 0)
      {
         tc->range++;
         tc->format |= PNG_FORMAT_FLAG_RANGE; /* forwards: backwards cancels */
      }

      tc->bit_depth = 8;
   }

   else /* the transform is not applicable */
      (*transform)->fn = NULL;
}
#endif /* WRITE_PACK */
