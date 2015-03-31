
/* pngrtran.c - transforms the data in a row for PNG readers
 *
 * Last changed in libpng 1.7.0 [(PENDING RELEASE)]
 * Copyright (c) 1998-2015 Glenn Randers-Pehrson
 * (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
 * (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * This file contains functions optionally called by an application
 * in order to tell libpng how to handle data when reading a PNG.
 * Transformations that are used in both reading and writing are
 * in pngtrans.c.
 */

#include "pngpriv.h"
#define PNG_SRC_FILE PNG_SRC_FILE_pngrtran

#ifdef PNG_READ_SUPPORTED

/* Set the action on getting a CRC error for an ancillary or critical chunk. */
void PNGAPI
png_set_crc_action(png_structrp png_ptr, int crit_action, int ancil_action)
{
   png_debug(1, "in png_set_crc_action");

   if (png_ptr == NULL)
      return;

   /* Tell libpng how we react to CRC errors in critical chunks */
   switch (crit_action)
   {
      case PNG_CRC_NO_CHANGE:                        /* Leave setting as is */
         break;

      case PNG_CRC_WARN_USE:                               /* Warn/use data */
         png_ptr->flags &= ~PNG_FLAG_CRC_CRITICAL_MASK;
         png_ptr->flags |= PNG_FLAG_CRC_CRITICAL_USE;
         break;

      case PNG_CRC_QUIET_USE:                             /* Quiet/use data */
         png_ptr->flags &= ~PNG_FLAG_CRC_CRITICAL_MASK;
         png_ptr->flags |= PNG_FLAG_CRC_CRITICAL_USE |
                           PNG_FLAG_CRC_CRITICAL_IGNORE;
         break;

      case PNG_CRC_WARN_DISCARD:    /* Not a valid action for critical data */
         png_warning(png_ptr,
            "Can't discard critical data on CRC error");
      case PNG_CRC_ERROR_QUIT:                                /* Error/quit */

      case PNG_CRC_DEFAULT:
      default:
         png_ptr->flags &= ~PNG_FLAG_CRC_CRITICAL_MASK;
         break;
   }

   /* Tell libpng how we react to CRC errors in ancillary chunks */
   switch (ancil_action)
   {
      case PNG_CRC_NO_CHANGE:                       /* Leave setting as is */
         break;

      case PNG_CRC_WARN_USE:                              /* Warn/use data */
         png_ptr->flags &= ~PNG_FLAG_CRC_ANCILLARY_MASK;
         png_ptr->flags |= PNG_FLAG_CRC_ANCILLARY_USE;
         break;

      case PNG_CRC_QUIET_USE:                            /* Quiet/use data */
         png_ptr->flags &= ~PNG_FLAG_CRC_ANCILLARY_MASK;
         png_ptr->flags |= PNG_FLAG_CRC_ANCILLARY_USE |
                           PNG_FLAG_CRC_ANCILLARY_NOWARN;
         break;

      case PNG_CRC_ERROR_QUIT:                               /* Error/quit */
         png_ptr->flags &= ~PNG_FLAG_CRC_ANCILLARY_MASK;
         png_ptr->flags |= PNG_FLAG_CRC_ANCILLARY_NOWARN;
         break;

      case PNG_CRC_WARN_DISCARD:                      /* Warn/discard data */

      case PNG_CRC_DEFAULT:
      default:
         png_ptr->flags &= ~PNG_FLAG_CRC_ANCILLARY_MASK;
         break;
   }
}

#ifdef PNG_READ_TRANSFORMS_SUPPORTED
/* Is it OK to set a transformation now?  Only if png_start_read_image or
 * png_read_update_info have not been called.  It is not necessary for the IHDR
 * to have been read in all cases; the need_IHDR parameter allows for this
 * check too.
 */
static int
png_rtran_ok(png_structrp png_ptr, int need_IHDR)
{
   if (png_ptr != NULL)
   {
      if ((png_ptr->flags & PNG_FLAG_ROW_INIT) != 0)
         png_app_error(png_ptr,
            "invalid after png_start_read_image or png_read_update_info");

      else if (need_IHDR && (png_ptr->mode & PNG_HAVE_IHDR) == 0)
         png_app_error(png_ptr, "invalid before the PNG header has been read");

      else
      {
         /* Turn on failure to initialize correctly for all transforms. */
         png_ptr->flags |= PNG_FLAG_DETECT_UNINITIALIZED;

         return 1; /* Ok */
      }
   }

   return 0; /* no png_error possible! */
}
#endif

#ifdef PNG_READ_BACKGROUND_SUPPORTED
/* Handle alpha and tRNS via a background color */
void PNGFAPI
png_set_background_fixed(png_structrp png_ptr,
    png_const_color_16p background_color, int background_gamma_code,
    int need_expand, png_fixed_point background_gamma)
{
   png_debug(1, "in png_set_background_fixed");

   if (png_rtran_ok(png_ptr, 0) == 0 || background_color == NULL)
      return;

   if (background_gamma_code != PNG_BACKGROUND_GAMMA_SCREEN &&
      background_gamma_code != PNG_BACKGROUND_GAMMA_FILE &&
      background_gamma_code != PNG_BACKGROUND_GAMMA_UNIQUE)
   {
      png_app_error(png_ptr, "invalid gamma type");
      return;
   }

   png_ptr->transformations |= PNG_COMPOSE | PNG_STRIP_ALPHA;
   png_ptr->transformations &= ~PNG_ENCODE_ALPHA;
   png_ptr->flags &= ~PNG_FLAG_OPTIMIZE_ALPHA;

   png_ptr->background = *background_color;
   png_ptr->background_gamma = background_gamma;
   png_ptr->background_gamma_type = png_check_byte(png_ptr,
      background_gamma_code);

   if (need_expand != 0)
      png_ptr->flags |= PNG_FLAG_BACKGROUND_EXPAND;

   else
      png_ptr->flags &= ~PNG_FLAG_BACKGROUND_EXPAND;
}

#  ifdef PNG_FLOATING_POINT_SUPPORTED
void PNGAPI
png_set_background(png_structrp png_ptr,
    png_const_color_16p background_color, int background_gamma_code,
    int need_expand, double background_gamma)
{
   png_set_background_fixed(png_ptr, background_color, background_gamma_code,
      need_expand, png_fixed(png_ptr, background_gamma, "png_set_background"));
}
#  endif  /* FLOATING_POINT */
#endif /* READ_BACKGROUND */

/* Scale 16-bit depth files to 8-bit depth.  If both of these are set then the
 * one that pngrtran does first (scale) happens.  This is necessary to allow the
 * TRANSFORM and API behavior to be somewhat consistent, and it's simpler.
 */
#ifdef PNG_READ_SCALE_16_TO_8_SUPPORTED
void PNGAPI
png_set_scale_16(png_structrp png_ptr)
{
   png_debug(1, "in png_set_scale_16");

   if (png_rtran_ok(png_ptr, 0) == 0)
      return;

   png_ptr->transformations |= PNG_SCALE_16_TO_8;
}
#endif

#ifdef PNG_READ_STRIP_16_TO_8_SUPPORTED
/* Chop 16-bit depth files to 8-bit depth */
void PNGAPI
png_set_strip_16(png_structrp png_ptr)
{
   png_debug(1, "in png_set_strip_16");

   if (png_rtran_ok(png_ptr, 0) == 0)
      return;

   png_ptr->transformations |= PNG_16_TO_8;
}
#endif

#ifdef PNG_READ_STRIP_ALPHA_SUPPORTED
void PNGAPI
png_set_strip_alpha(png_structrp png_ptr)
{
   png_debug(1, "in png_set_strip_alpha");

   if (png_rtran_ok(png_ptr, 0) == 0)
      return;

   png_ptr->transformations |= PNG_STRIP_ALPHA;
}
#endif

#if defined(PNG_READ_ALPHA_MODE_SUPPORTED) || defined(PNG_READ_GAMMA_SUPPORTED)
static png_fixed_point
translate_gamma_flags(png_structrp png_ptr, png_fixed_point output_gamma,
   int is_screen)
{
   /* Check for flag values.  The main reason for having the old Mac value as a
    * flag is that it is pretty near impossible to work out what the correct
    * value is from Apple documentation - a working Mac system is needed to
    * discover the value!
    */
   if (output_gamma == PNG_DEFAULT_sRGB ||
      output_gamma == PNG_FP_1 / PNG_DEFAULT_sRGB)
   {
      /* If there is no sRGB support this just sets the gamma to the standard
       * sRGB value.  (This is a side effect of using this function!)
       */
#     ifdef PNG_READ_sRGB_SUPPORTED
         png_ptr->flags |= PNG_FLAG_ASSUME_sRGB;
#     else
         PNG_UNUSED(png_ptr)
#     endif
      if (is_screen != 0)
         output_gamma = PNG_GAMMA_sRGB;
      else
         output_gamma = PNG_GAMMA_sRGB_INVERSE;
   }

   else if (output_gamma == PNG_GAMMA_MAC_18 ||
      output_gamma == PNG_FP_1 / PNG_GAMMA_MAC_18)
   {
      if (is_screen != 0)
         output_gamma = PNG_GAMMA_MAC_OLD;
      else
         output_gamma = PNG_GAMMA_MAC_INVERSE;
   }

   return output_gamma;
}

#  ifdef PNG_FLOATING_POINT_SUPPORTED
static png_fixed_point
convert_gamma_value(png_structrp png_ptr, double output_gamma)
{
   /* The following silently ignores cases where fixed point (times 100,000)
    * gamma values are passed to the floating point API.  This is safe and it
    * means the fixed point constants work just fine with the floating point
    * API.  The alternative would just lead to undetected errors and spurious
    * bug reports.  Negative values fail inside the _fixed API unless they
    * correspond to the flag values.
    */
   if (output_gamma < 0 || output_gamma > 128)
      output_gamma *= .00001;

   return png_fixed(png_ptr, output_gamma, "gamma value");
}
#  endif
#endif /* READ_ALPHA_MODE || READ_GAMMA */

#ifdef PNG_READ_ALPHA_MODE_SUPPORTED
void PNGFAPI
png_set_alpha_mode_fixed(png_structrp png_ptr, int mode,
   png_fixed_point output_gamma)
{
   int compose = 0;
   png_fixed_point file_gamma;

   png_debug(1, "in png_set_alpha_mode");

   if (png_rtran_ok(png_ptr, 0) == 0)
      return;

   output_gamma = translate_gamma_flags(png_ptr, output_gamma, 1/*screen*/);

   /* Validate the value to ensure it is in a reasonable range. The value
    * is expected to be 1 or greater, but this range test allows for some
    * viewing correction values.  The intent is to weed out users of this API
    * who use the inverse of the gamma value accidentally!  Since some of these
    * values are reasonable this may have to be changed.
    */
   if (output_gamma < 70000 || output_gamma > 300000)
      png_error(png_ptr, "output gamma out of expected range");

   /* The default file gamma is the inverse of the output gamma; the output
    * gamma may be changed below so get the file value first:
    */
   file_gamma = png_reciprocal(output_gamma);

   /* There are really 8 possibilities here, composed of any combination
    * of:
    *
    *    premultiply the color channels
    *    do not encode non-opaque pixels
    *    encode the alpha as well as the color channels
    *
    * The differences disappear if the input/output ('screen') gamma is 1.0,
    * because then the encoding is a no-op and there is only the choice of
    * premultiplying the color channels or not.
    *
    * png_set_alpha_mode and png_set_background interact because both use
    * png_compose to do the work.  Calling both is only useful when
    * png_set_alpha_mode is used to set the default mode - PNG_ALPHA_PNG - along
    * with a default gamma value.  Otherwise PNG_COMPOSE must not be set.
    */
   switch (mode)
   {
      case PNG_ALPHA_PNG:        /* default: png standard */
         /* No compose, but it may be set by png_set_background! */
         png_ptr->transformations &= ~PNG_ENCODE_ALPHA;
         png_ptr->flags &= ~PNG_FLAG_OPTIMIZE_ALPHA;
         break;

      case PNG_ALPHA_ASSOCIATED: /* color channels premultiplied */
         compose = 1;
         png_ptr->transformations &= ~PNG_ENCODE_ALPHA;
         png_ptr->flags &= ~PNG_FLAG_OPTIMIZE_ALPHA;
         /* The output is linear: */
         output_gamma = PNG_FP_1;
         break;

      case PNG_ALPHA_OPTIMIZED:  /* associated, non-opaque pixels linear */
         compose = 1;
         png_ptr->transformations &= ~PNG_ENCODE_ALPHA;
         png_ptr->flags |= PNG_FLAG_OPTIMIZE_ALPHA;
         /* output_gamma records the encoding of opaque pixels! */
         break;

      case PNG_ALPHA_BROKEN:     /* associated, non-linear, alpha encoded */
         compose = 1;
         png_ptr->transformations |= PNG_ENCODE_ALPHA;
         png_ptr->flags &= ~PNG_FLAG_OPTIMIZE_ALPHA;
         break;

      default:
         png_error(png_ptr, "invalid alpha mode");
   }

   /* Only set the default gamma if the file gamma has not been set (this has
    * the side effect that the gamma in a second call to png_set_alpha_mode will
    * be ignored.)
    */
   if (png_ptr->colorspace.gamma == 0)
   {
      png_ptr->colorspace.gamma = file_gamma;
      png_ptr->colorspace.flags |= PNG_COLORSPACE_HAVE_GAMMA;
   }

   /* But always set the output gamma: */
   png_ptr->screen_gamma = output_gamma;

   /* Finally, if pre-multiplying, set the background fields to achieve the
    * desired result.
    */
   if (compose != 0)
   {
      /* And obtain alpha pre-multiplication by composing on black: */
      memset(&png_ptr->background, 0, (sizeof png_ptr->background));
      png_ptr->background_gamma = png_ptr->colorspace.gamma; /* just in case */
      png_ptr->background_gamma_type = PNG_BACKGROUND_GAMMA_FILE;
      png_ptr->flags &= ~PNG_FLAG_BACKGROUND_EXPAND;

      if ((png_ptr->transformations & PNG_COMPOSE) != 0)
         png_error(png_ptr,
            "conflicting calls to set alpha mode and background");

      png_ptr->transformations |= PNG_COMPOSE;
   }
}

#  ifdef PNG_FLOATING_POINT_SUPPORTED
void PNGAPI
png_set_alpha_mode(png_structrp png_ptr, int mode, double output_gamma)
{
   png_set_alpha_mode_fixed(png_ptr, mode, convert_gamma_value(png_ptr,
      output_gamma));
}
#  endif
#endif

#ifdef PNG_READ_QUANTIZE_SUPPORTED
/* Dither file to 8-bit.  Supply a palette, the current number
 * of elements in the palette, the maximum number of elements
 * allowed, and a histogram if possible.  If the current number
 * of colors is greater then the maximum number, the palette will be
 * modified to fit in the maximum number.  "full_quantize" indicates
 * whether we need a quantizing cube set up for RGB images, or if we
 * simply are reducing the number of colors in a paletted image.
 */

typedef struct png_dsort_struct
{
   struct png_dsort_struct * next;
   png_byte left;
   png_byte right;
} png_dsort;
typedef png_dsort *   png_dsortp;
typedef png_dsort * * png_dsortpp;

void PNGAPI
png_set_quantize(png_structrp png_ptr, png_colorp palette,
    int num_palette, int maximum_colors, png_const_uint_16p histogram,
    int full_quantize)
{
   png_debug(1, "in png_set_quantize");

   if (png_rtran_ok(png_ptr, 0) == 0)
      return;

   png_ptr->transformations |= PNG_QUANTIZE;

   if (full_quantize == 0)
   {
      int i;

      png_ptr->quantize_index = (png_bytep)png_malloc(png_ptr,
          (png_uint_32)(num_palette * (sizeof (png_byte))));
      for (i = 0; i < num_palette; i++)
         png_ptr->quantize_index[i] = png_check_byte(png_ptr, i);
   }

   if (num_palette > maximum_colors)
   {
      if (histogram != NULL)
      {
         /* This is easy enough, just throw out the least used colors.
          * Perhaps not the best solution, but good enough.
          */

         int i;

         /* Initialize an array to sort colors */
         png_ptr->quantize_sort = (png_bytep)png_malloc(png_ptr,
             (png_uint_32)(num_palette * (sizeof (png_byte))));

         /* Initialize the quantize_sort array */
         for (i = 0; i < num_palette; i++)
            png_ptr->quantize_sort[i] = png_check_byte(png_ptr, i);

         /* Find the least used palette entries by starting a
          * bubble sort, and running it until we have sorted
          * out enough colors.  Note that we don't care about
          * sorting all the colors, just finding which are
          * least used.
          */

         for (i = num_palette - 1; i >= maximum_colors; i--)
         {
            int done; /* To stop early if the list is pre-sorted */
            int j;

            done = 1;
            for (j = 0; j < i; j++)
            {
               if (histogram[png_ptr->quantize_sort[j]]
                   < histogram[png_ptr->quantize_sort[j + 1]])
               {
                  png_byte t;

                  t = png_ptr->quantize_sort[j];
                  png_ptr->quantize_sort[j] = png_ptr->quantize_sort[j + 1];
                  png_ptr->quantize_sort[j + 1] = t;
                  done = 0;
               }
            }

            if (done != 0)
               break;
         }

         /* Swap the palette around, and set up a table, if necessary */
         if (full_quantize != 0)
         {
            int j = num_palette;

            /* Put all the useful colors within the max, but don't
             * move the others.
             */
            for (i = 0; i < maximum_colors; i++)
            {
               if ((int)png_ptr->quantize_sort[i] >= maximum_colors)
               {
                  do
                     j--;
                  while ((int)png_ptr->quantize_sort[j] >= maximum_colors);

                  palette[i] = palette[j];
               }
            }
         }
         else
         {
            int j = num_palette;

            /* Move all the used colors inside the max limit, and
             * develop a translation table.
             */
            for (i = 0; i < maximum_colors; i++)
            {
               /* Only move the colors we need to */
               if ((int)png_ptr->quantize_sort[i] >= maximum_colors)
               {
                  png_color tmp_color;

                  do
                     j--;
                  while ((int)png_ptr->quantize_sort[j] >= maximum_colors);

                  tmp_color = palette[j];
                  palette[j] = palette[i];
                  palette[i] = tmp_color;
                  /* Indicate where the color went */
                  png_ptr->quantize_index[j] = png_check_byte(png_ptr, i);
                  png_ptr->quantize_index[i] = png_check_byte(png_ptr, j);
               }
            }

            /* Find closest color for those colors we are not using */
            for (i = 0; i < num_palette; i++)
            {
               if ((int)png_ptr->quantize_index[i] >= maximum_colors)
               {
                  int min_d, k, min_k, d_index;

                  /* Find the closest color to one we threw out */
                  d_index = png_ptr->quantize_index[i];
                  min_d = PNG_COLOR_DIST(palette[d_index], palette[0]);
                  for (k = 1, min_k = 0; k < maximum_colors; k++)
                  {
                     int d;

                     d = PNG_COLOR_DIST(palette[d_index], palette[k]);

                     if (d < min_d)
                     {
                        min_d = d;
                        min_k = k;
                     }
                  }
                  /* Point to closest color */
                  png_ptr->quantize_index[i] = png_check_byte(png_ptr, min_k);
               }
            }
         }
         png_free(png_ptr, png_ptr->quantize_sort);
         png_ptr->quantize_sort = NULL;
      }
      else
      {
         /* This is much harder to do simply (and quickly).  Perhaps
          * we need to go through a median cut routine, but those
          * don't always behave themselves with only a few colors
          * as input.  So we will just find the closest two colors,
          * and throw out one of them (chosen somewhat randomly).
          * [We don't understand this at all, so if someone wants to
          *  work on improving it, be our guest - AED, GRP]
          */
         int i;
         int max_d;
         int num_new_palette;
         png_dsortp t;
         png_dsortpp hash;

         t = NULL;

         /* Initialize palette index arrays */
         png_ptr->index_to_palette = (png_bytep)png_malloc(png_ptr,
             (png_uint_32)(num_palette * (sizeof (png_byte))));
         png_ptr->palette_to_index = (png_bytep)png_malloc(png_ptr,
             (png_uint_32)(num_palette * (sizeof (png_byte))));

         /* Initialize the sort array */
         for (i = 0; i < num_palette; i++)
         {
            png_ptr->index_to_palette[i] = png_check_byte(png_ptr, i);
            png_ptr->palette_to_index[i] = png_check_byte(png_ptr, i);
         }

         hash = (png_dsortpp)png_calloc(png_ptr, (png_uint_32)(769 *
             (sizeof (png_dsortp))));

         num_new_palette = num_palette;

         /* Initial wild guess at how far apart the farthest pixel
          * pair we will be eliminating will be.  Larger
          * numbers mean more areas will be allocated, Smaller
          * numbers run the risk of not saving enough data, and
          * having to do this all over again.
          *
          * I have not done extensive checking on this number.
          */
         max_d = 96;

         while (num_new_palette > maximum_colors)
         {
            for (i = 0; i < num_new_palette - 1; i++)
            {
               int j;

               for (j = i + 1; j < num_new_palette; j++)
               {
                  int d;

                  d = PNG_COLOR_DIST(palette[i], palette[j]);

                  if (d <= max_d)
                  {

                     t = (png_dsortp)png_malloc_warn(png_ptr,
                         (png_uint_32)(sizeof (png_dsort)));

                     if (t == NULL)
                         break;

                     t->next = hash[d];
                     t->left = png_check_byte(png_ptr, i);
                     t->right = png_check_byte(png_ptr, j);
                     hash[d] = t;
                  }
               }
               if (t == NULL)
                  break;
            }

            if (t != NULL)
            for (i = 0; i <= max_d; i++)
            {
               if (hash[i] != NULL)
               {
                  png_dsortp p;

                  for (p = hash[i]; p; p = p->next)
                  {
                     if ((int)png_ptr->index_to_palette[p->left]
                         < num_new_palette &&
                         (int)png_ptr->index_to_palette[p->right]
                         < num_new_palette)
                     {
                        int j, next_j;

                        if (num_new_palette & 0x01)
                        {
                           j = p->left;
                           next_j = p->right;
                        }
                        else
                        {
                           j = p->right;
                           next_j = p->left;
                        }

                        num_new_palette--;
                        palette[png_ptr->index_to_palette[j]]
                            = palette[num_new_palette];
                        if (full_quantize == 0)
                        {
                           int k;

                           for (k = 0; k < num_palette; k++)
                           {
                              if (png_ptr->quantize_index[k] ==
                                  png_ptr->index_to_palette[j])
                                 png_ptr->quantize_index[k] =
                                     png_ptr->index_to_palette[next_j];

                              if ((int)png_ptr->quantize_index[k] ==
                                  num_new_palette)
                                 png_ptr->quantize_index[k] =
                                     png_ptr->index_to_palette[j];
                           }
                        }

                        png_ptr->index_to_palette[png_ptr->palette_to_index
                            [num_new_palette]] = png_ptr->index_to_palette[j];

                        png_ptr->palette_to_index[png_ptr->index_to_palette[j]]
                            = png_ptr->palette_to_index[num_new_palette];

                        png_ptr->index_to_palette[j] =
                            png_check_byte(png_ptr, num_new_palette);

                        png_ptr->palette_to_index[num_new_palette] =
                            png_check_byte(png_ptr, j);
                     }
                     if (num_new_palette <= maximum_colors)
                        break;
                  }
                  if (num_new_palette <= maximum_colors)
                     break;
               }
            }

            for (i = 0; i < 769; i++)
            {
               if (hash[i] != NULL)
               {
                  png_dsortp p = hash[i];
                  while (p)
                  {
                     t = p->next;
                     png_free(png_ptr, p);
                     p = t;
                  }
               }
               hash[i] = 0;
            }
            max_d += 96;
         }
         png_free(png_ptr, hash);
         png_free(png_ptr, png_ptr->palette_to_index);
         png_free(png_ptr, png_ptr->index_to_palette);
         png_ptr->palette_to_index = NULL;
         png_ptr->index_to_palette = NULL;
      }
      num_palette = maximum_colors;
   }
   if (png_ptr->palette == NULL)
   {
      png_ptr->palette = palette;
   }
   png_ptr->num_palette = png_check_u16(png_ptr, num_palette);

   if (full_quantize != 0)
   {
      int i;
      png_bytep distance;
      int total_bits = PNG_QUANTIZE_RED_BITS + PNG_QUANTIZE_GREEN_BITS +
          PNG_QUANTIZE_BLUE_BITS;
      int num_red = (1 << PNG_QUANTIZE_RED_BITS);
      int num_green = (1 << PNG_QUANTIZE_GREEN_BITS);
      int num_blue = (1 << PNG_QUANTIZE_BLUE_BITS);
      png_size_t num_entries = ((png_size_t)1 << total_bits);

      png_ptr->palette_lookup = (png_bytep)png_calloc(png_ptr,
          (png_uint_32)(num_entries * (sizeof (png_byte))));

      distance = (png_bytep)png_malloc(png_ptr, (png_uint_32)(num_entries *
          (sizeof (png_byte))));

      memset(distance, 0xff, num_entries * (sizeof (png_byte)));

      for (i = 0; i < num_palette; i++)
      {
         int ir, ig, ib;
         int r = (palette[i].red >> (8 - PNG_QUANTIZE_RED_BITS));
         int g = (palette[i].green >> (8 - PNG_QUANTIZE_GREEN_BITS));
         int b = (palette[i].blue >> (8 - PNG_QUANTIZE_BLUE_BITS));

         for (ir = 0; ir < num_red; ir++)
         {
            /* int dr = abs(ir - r); */
            int dr = ((ir > r) ? ir - r : r - ir);
            int index_r = (ir << (PNG_QUANTIZE_BLUE_BITS +
                PNG_QUANTIZE_GREEN_BITS));

            for (ig = 0; ig < num_green; ig++)
            {
               /* int dg = abs(ig - g); */
               int dg = ((ig > g) ? ig - g : g - ig);
               int dt = dr + dg;
               int dm = ((dr > dg) ? dr : dg);
               int index_g = index_r | (ig << PNG_QUANTIZE_BLUE_BITS);

               for (ib = 0; ib < num_blue; ib++)
               {
                  int d_index = index_g | ib;
                  /* int db = abs(ib - b); */
                  int db = ((ib > b) ? ib - b : b - ib);
                  int dmax = ((dm > db) ? dm : db);
                  int d = dmax + dt + db;

                  if (d < (int)distance[d_index])
                  {
                     distance[d_index] = png_check_byte(png_ptr, d);
                     png_ptr->palette_lookup[d_index] = png_check_byte(png_ptr,
                        i);
                  }
               }
            }
         }
      }

      png_free(png_ptr, distance);
   }
}
#endif /* READ_QUANTIZE */

#ifdef PNG_READ_GAMMA_SUPPORTED
void PNGFAPI
png_set_gamma_fixed(png_structrp png_ptr, png_fixed_point scrn_gamma,
   png_fixed_point file_gamma)
{
   png_debug(1, "in png_set_gamma_fixed");

   if (png_rtran_ok(png_ptr, 0) == 0)
      return;

   /* New in libpng-1.5.4 - reserve particular negative values as flags. */
   scrn_gamma = translate_gamma_flags(png_ptr, scrn_gamma, 1/*screen*/);
   file_gamma = translate_gamma_flags(png_ptr, file_gamma, 0/*file*/);

   /* Checking the gamma values for being >0 was added in 1.5.4 along with the
    * premultiplied alpha support; this actually hides an undocumented feature
    * of the previous implementation which allowed gamma processing to be
    * disabled in background handling.  There is no evidence (so far) that this
    * was being used; however, png_set_background itself accepted and must still
    * accept '0' for the gamma value it takes, because it isn't always used.
    *
    * Since this is an API change (albeit a very minor one that removes an
    * undocumented API feature) the following checks were only enabled in
    * libpng-1.6.0.
    */
   if (file_gamma <= 0)
      png_error(png_ptr, "invalid file gamma in png_set_gamma");

   if (scrn_gamma <= 0)
      png_error(png_ptr, "invalid screen gamma in png_set_gamma");

   /* Set the gamma values unconditionally - this overrides the value in the PNG
    * file if a gAMA chunk was present.  png_set_alpha_mode provides a
    * different, easier, way to default the file gamma.
    */
   png_ptr->colorspace.gamma = file_gamma;
   png_ptr->colorspace.flags |= PNG_COLORSPACE_HAVE_GAMMA;
   png_ptr->screen_gamma = scrn_gamma;
}

#  ifdef PNG_FLOATING_POINT_SUPPORTED
void PNGAPI
png_set_gamma(png_structrp png_ptr, double scrn_gamma, double file_gamma)
{
   png_set_gamma_fixed(png_ptr, convert_gamma_value(png_ptr, scrn_gamma),
      convert_gamma_value(png_ptr, file_gamma));
}
#  endif /* FLOATING_POINT */
#endif /* READ_GAMMA */

#ifdef PNG_READ_EXPAND_SUPPORTED
/* Expand paletted images to RGB, expand grayscale images of
 * less than 8-bit depth to 8-bit depth, and expand tRNS chunks
 * to alpha channels.
 */
void PNGAPI
png_set_expand(png_structrp png_ptr)
{
   png_debug(1, "in png_set_expand");

   if (png_rtran_ok(png_ptr, 0) == 0)
      return;

   png_ptr->transformations |= (PNG_EXPAND | PNG_EXPAND_tRNS);
}

/* GRR 19990627:  the following three functions currently are identical
 *  to png_set_expand().  However, it is entirely reasonable that someone
 *  might wish to expand an indexed image to RGB but *not* expand a single,
 *  fully transparent palette entry to a full alpha channel--perhaps instead
 *  convert tRNS to the grayscale/RGB format (16-bit RGB value), or replace
 *  the transparent color with a particular RGB value, or drop tRNS entirely.
 *  IOW, a future version of the library may make the transformations flag
 *  a bit more fine-grained, with separate bits for each of these three
 *  functions.
 *
 *  More to the point, these functions make it obvious what libpng will be
 *  doing, whereas "expand" can (and does) mean any number of things.
 *
 *  GRP 20060307: In libpng-1.2.9, png_set_gray_1_2_4_to_8() was modified
 *  to expand only the sample depth but not to expand the tRNS to alpha
 *  and its name was changed to png_set_expand_gray_1_2_4_to_8().
 */

/* Expand paletted images to RGB. */
void PNGAPI
png_set_palette_to_rgb(png_structrp png_ptr)
{
   png_debug(1, "in png_set_palette_to_rgb");

   if (png_rtran_ok(png_ptr, 0) == 0)
      return;

   png_ptr->transformations |= (PNG_EXPAND | PNG_EXPAND_tRNS);
}

/* Expand grayscale images of less than 8-bit depth to 8 bits. */
void PNGAPI
png_set_expand_gray_1_2_4_to_8(png_structrp png_ptr)
{
   png_debug(1, "in png_set_expand_gray_1_2_4_to_8");

   if (png_rtran_ok(png_ptr, 0) == 0)
      return;

   png_ptr->transformations |= PNG_EXPAND;
}

/* Expand tRNS chunks to alpha channels. */
void PNGAPI
png_set_tRNS_to_alpha(png_structrp png_ptr)
{
   png_debug(1, "in png_set_tRNS_to_alpha");

   if (png_rtran_ok(png_ptr, 0) == 0)
      return;

   png_ptr->transformations |= (PNG_EXPAND | PNG_EXPAND_tRNS);
}
#endif /* READ_EXPAND */

#ifdef PNG_READ_EXPAND_16_SUPPORTED
/* Expand to 16-bit channels, expand the tRNS chunk too (because otherwise
 * it may not work correctly.)
 */
void PNGAPI
png_set_expand_16(png_structrp png_ptr)
{
   png_debug(1, "in png_set_expand_16");

   if (png_rtran_ok(png_ptr, 0) == 0)
      return;

   png_ptr->transformations |= (PNG_EXPAND_16 | PNG_EXPAND | PNG_EXPAND_tRNS);
}
#endif

#ifdef PNG_READ_GRAY_TO_RGB_SUPPORTED
void PNGAPI
png_set_gray_to_rgb(png_structrp png_ptr)
{
   png_debug(1, "in png_set_gray_to_rgb");

   if (png_rtran_ok(png_ptr, 0) == 0)
      return;

   /* Because rgb must be 8 bits or more: */
   png_set_expand_gray_1_2_4_to_8(png_ptr);
   png_ptr->transformations |= PNG_GRAY_TO_RGB;
}
#endif

#ifdef PNG_READ_RGB_TO_GRAY_SUPPORTED
void PNGFAPI
png_set_rgb_to_gray_fixed(png_structrp png_ptr, int error_action,
    png_fixed_point red, png_fixed_point green)
{
   png_debug(1, "in png_set_rgb_to_gray");

   /* Need the IHDR here because of the check on color_type below. */
   /* TODO: fix this */
   if (png_rtran_ok(png_ptr, 1) == 0)
      return;

   switch (error_action)
   {
      case PNG_ERROR_ACTION_NONE:
         png_ptr->transformations |= PNG_RGB_TO_GRAY;
         break;

      case PNG_ERROR_ACTION_WARN:
         png_ptr->transformations |= PNG_RGB_TO_GRAY_WARN;
         break;

      case PNG_ERROR_ACTION_ERROR:
         png_ptr->transformations |= PNG_RGB_TO_GRAY_ERR;
         break;

      default:
         png_error(png_ptr, "invalid error action to rgb_to_gray");
         break;
   }

   if (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
#ifdef PNG_READ_EXPAND_SUPPORTED
      png_ptr->transformations |= PNG_EXPAND;
#else
   {
      /* Make this an error in 1.6 because otherwise the application may assume
       * that it just worked and get a memory overwrite.
       */
      png_error(png_ptr,
        "Cannot do RGB_TO_GRAY without EXPAND_SUPPORTED");

      /* png_ptr->transformations &= ~PNG_RGB_TO_GRAY; */
   }
#endif
   {
      if (red >= 0 && green >= 0 && red + green <= PNG_FP_1)
      {
         png_uint_16 red_int, green_int;

         /* NOTE: this calculation does not round, but this behavior is retained
          * for consistency; the inaccuracy is very small.  The code here always
          * overwrites the coefficients, regardless of whether they have been
          * defaulted or set already.
          */
         red_int = png_check_u16(png_ptr,
            ((png_uint_32)/*SAFE*/red*32768)/100000);
         green_int = png_check_u16(png_ptr,
            ((png_uint_32)/*SAFE*/green*32768)/100000);

         png_ptr->rgb_to_gray_red_coeff   = red_int;
         png_ptr->rgb_to_gray_green_coeff = green_int;
#        if defined(PNG_COLORS_SPACE_SUPPORTED) || defined(PNG_GAMMA_SUPPORTED)
            png_ptr->colorspace.flags |= PNG_COLORSPACE_RGB_TO_GRAY_SET;
#        endif
      }

      else
      {
         if (red >= 0 && green >= 0)
            png_app_warning(png_ptr,
               "ignoring out of range rgb_to_gray coefficients");

         /* Use the defaults, from the cHRM chunk if set, else the historical
          * values which are close to the sRGB/HDTV/ITU-Rec 709 values.  See
          * png_do_rgb_to_gray for more discussion of the values.  In this case
          * the coefficients are not marked as 'set' and are not overwritten if
          * something has already provided a default.
          */
         if (png_ptr->rgb_to_gray_red_coeff == 0 &&
            png_ptr->rgb_to_gray_green_coeff == 0)
         {
            png_ptr->rgb_to_gray_red_coeff   = 6968;
            png_ptr->rgb_to_gray_green_coeff = 23434;
            /* png_ptr->rgb_to_gray_blue_coeff  = 2366; */
         }
      }
   }
}

#ifdef PNG_FLOATING_POINT_SUPPORTED
/* Convert a RGB image to a grayscale of the same width.  This allows us,
 * for example, to convert a 24 bpp RGB image into an 8 bpp grayscale image.
 */

void PNGAPI
png_set_rgb_to_gray(png_structrp png_ptr, int error_action, double red,
   double green)
{
   png_set_rgb_to_gray_fixed(png_ptr, error_action,
      png_fixed(png_ptr, red, "rgb to gray red coefficient"),
      png_fixed(png_ptr, green, "rgb to gray green coefficient"));
}
#endif /* FLOATING POINT */

#endif /* RGB_TO_GRAY */

#if defined(PNG_READ_USER_TRANSFORM_SUPPORTED) || \
    defined(PNG_WRITE_USER_TRANSFORM_SUPPORTED)
void PNGAPI
png_set_read_user_transform_fn(png_structrp png_ptr, png_user_transform_ptr
    read_user_transform_fn)
{
   png_debug(1, "in png_set_read_user_transform_fn");

#ifdef PNG_READ_USER_TRANSFORM_SUPPORTED
   png_ptr->transformations |= PNG_USER_TRANSFORM;
   png_ptr->read_user_transform_fn = read_user_transform_fn;
#endif
}
#endif

#ifdef PNG_READ_TRANSFORMS_SUPPORTED
#ifdef PNG_READ_GAMMA_SUPPORTED
/* In the case of gamma transformations only do transformations on images where
 * the [file] gamma and screen_gamma are not close reciprocals, otherwise it
 * slows things down slightly, and also needlessly introduces small errors.
 */
static int /* PRIVATE */
png_gamma_threshold(png_fixed_point screen_gamma, png_fixed_point file_gamma)
{
   /* PNG_GAMMA_THRESHOLD is the threshold for performing gamma
    * correction as a difference of the overall transform from 1.0
    *
    * We want to compare the threshold with s*f - 1, if we get
    * overflow here it is because of wacky gamma values so we
    * turn on processing anyway.
    */
   png_fixed_point gtest;
   return !png_muldiv(&gtest, screen_gamma, file_gamma, PNG_FP_1) ||
       png_gamma_significant(gtest);
}
#endif

/* Initialize everything needed for the read.  This includes modifying
 * the palette.
 */
#if defined(PNG_READ_BACKGROUND_SUPPORTED) ||\
   defined(PNG_READ_ALPHA_MODE_SUPPORTED)
static void
gamma_correct_background(png_const_structrp png_ptr,
   unsigned int value, unsigned int depth,
   png_uint_16p backgroundp, png_uint_16p background_1p,
   png_fixed_point gamma_correct, png_fixed_point gamma_to_1)
{
   switch (depth)
   {
      case 8:
         if (gamma_correct != PNG_FP_1)
            *backgroundp = png_gamma_8bit_correct(png_ptr, value,
               gamma_correct);

         else
            *backgroundp = png_check_u16(png_ptr, value);

         if (gamma_to_1 != PNG_FP_1)
            *background_1p = png_gamma_16bit_correct(png_ptr, value*257,
               gamma_to_1);

         else
            *background_1p = png_check_u16(png_ptr, value*257);

         return;

      case 16:
         if (gamma_correct != PNG_FP_1)
            *backgroundp = png_gamma_16bit_correct(png_ptr, value,
               gamma_correct);

         else
            *backgroundp = png_check_u16(png_ptr, value);

         if (gamma_to_1 != PNG_FP_1)
            *background_1p = png_gamma_16bit_correct(png_ptr, value,
               gamma_to_1);

         else
            *background_1p = png_check_u16(png_ptr, value);

         return;

      default:
         /* Low bit depth gray levels; do no harm. */
         break;
   }

   *backgroundp = png_check_u16(png_ptr, value);
   *background_1p = 0; /* should not be used */
}

static void /* PRIVATE */
png_init_background_transformations(png_structrp png_ptr)
   /* Set the png_ptr->background and png_ptr->background_1 members correctly
    * for the bit depth and format.
    */
{
   /* png_ptr->background is only assigned by png_set_background and
    * png_set_alpha_mode (which just zeros out the fields.)  png_set_background
    * can set the PNG_FLAG_BACKGROUND_EXPAND flag if the input value is in the
    * file format, for example if it comes from a bKGD chunk.
    *
    * Under some circumstances deficiencies in the current libpng code mean that
    * the bit depth of the values must differ from the final bit depth; the bit
    * depth has to match that at which the processing of the image pixels
    * happens and this is not always the final bit depth.  This is fixed up
    * here.
    *
    * First find the required depth.
    */
   unsigned int bit_depth, required_bit_depth;
   unsigned int color_type = png_ptr->color_type;
   const png_uint_32 transform = png_ptr->transformations;
   const int need_expand = (png_ptr->flags & PNG_FLAG_BACKGROUND_EXPAND) != 0;

   if (color_type & PNG_COLOR_MASK_PALETTE)
      required_bit_depth = bit_depth = 8;

   else
   {
      required_bit_depth = bit_depth = png_ptr->bit_depth;

      /* But not PNG_EXPAND_16 at present because it happens after the compose
       * operation where the background is used!
       */
      if (bit_depth < 8 && (transform & PNG_EXPAND) != 0)
         required_bit_depth = 8;
   }

   /* bit_depth and color_type now refer to the original file data and
    * required_bit_depth is correct for the processing libpng does, however it
    * does not necessarily match the output the application gets, fix that and
    * the color type here:
    */
   if (need_expand == 0)
   {
      /* The background bit_depth and color_type need correcting */
      if ((transform & PNG_EXPAND) != 0)
         color_type &= ~PNG_COLOR_MASK_PALETTE;

      /* The RGB<->gray transformations do the to gray operation first, then the
       * from gray.
       */
      if ((transform & PNG_RGB_TO_GRAY) != 0)
         color_type &= ~PNG_COLOR_MASK_COLOR;

      if ((transform & PNG_GRAY_TO_RGB) != 0)
         color_type |= PNG_COLOR_MASK_COLOR;

      bit_depth = required_bit_depth;

      /* The expansion to 16 bits and the scaling back from 16 bits per
       * component to only 8 happens after the background processing (at
       * present) so these transforms only affect the screen value, not the
       * required value.  Note that the 16_TO_8 conversions happen before the 8
       * to 16 one, so in theory both could occur - the order of the tests below
       * must be correct!
       *
       * TODO: Note that the second of these changes cause an input 16-bit
       * background value to be temporarily crushed to 8-bits per component,
       * losing precision.  This is a bug and should be fixed.
       */
      if (bit_depth == 16 &&
         (transform & (PNG_16_TO_8|PNG_SCALE_16_TO_8)) != 0)
         bit_depth = 8;

      if (bit_depth == 8 && (color_type & PNG_COLOR_MASK_PALETTE) == 0 &&
         (transform & PNG_EXPAND_16) != 0)
         bit_depth = 16;
   }

   /* Double check the input value: when 'need_expand' is false the app is
    * providing a background value for us an it should have 'bit_depth' data in
    * it.  Unfortunately this may not be the case; we can't check in
    * png_set_background because we don't know what transforms the app will end
    * up asking for, so we have to check here.  Prior to 1.7.0 no check was
    * performed and the result could potentially be garbage.
    */
   if (bit_depth < 16) /* Else range changes always succeed */
   {
      if (color_type == PNG_COLOR_TYPE_PALETTE)
      {
         /* If the PNG is indexed and the need_expand flag was true the
          * background color is a palette index and this index must be in range.
          * If, however, need_expand is false the background is an RGB value and
          * it must be in the 8 bit range.  This duplicates the tests below,
          * but this code will probably all disappear in the very near future;
          * it is just way to error prone.
          */
         if (need_expand)
         {
            if (png_ptr->background.index >= png_ptr->num_palette ||
               png_ptr->palette != NULL)
               png_app_error(png_ptr, "background has invalid palette index");
         }

         else if ((png_ptr->transformations & PNG_RGB_TO_GRAY) != 0)
         {
            if (png_ptr->background.gray > 255)
               png_app_error(png_ptr,
                  "palette background gray value out of range");
         }

         else if (png_ptr->background.red > 255 ||
               png_ptr->background.green > 255 ||
               png_ptr->background.blue > 255)
            png_app_error(png_ptr, "palette background RGB value out of range");
      }

      else
      {
         const unsigned int mask = ~((1U << bit_depth) - 1);

         if ((color_type & PNG_COLOR_MASK_COLOR) != 0)
         {
            if ((png_ptr->background.red & mask) != 0 ||
               (png_ptr->background.green & mask) != 0 ||
               (png_ptr->background.blue & mask) != 0)
               png_app_error(png_ptr, "background RGB value out of range");
         }

         else if ((png_ptr->background.gray & mask) != 0)
               png_app_error(png_ptr, "background gray value out of range");
      }
   }

   /* Now make the background have the correct format. This involves reading the
    * correct fields from png_ptr->background, adjusting the bit depth of the
    * result and potentially gamma correcting the value then calculating the
    * png_ptr->background_1 values too.
    */
   {
      unsigned int mult = 1;
      png_fixed_point gamma_to_1, gamma_correct;

      switch (png_ptr->background_gamma_type)
      {
         case PNG_BACKGROUND_GAMMA_SCREEN:
            gamma_to_1 = png_ptr->screen_gamma;
            gamma_correct = PNG_FP_1;
            break;

         case PNG_BACKGROUND_GAMMA_FILE:
            gamma_to_1 = png_reciprocal(png_ptr->colorspace.gamma);
            gamma_correct = png_reciprocal2(png_ptr->colorspace.gamma,
               png_ptr->screen_gamma);
            break;

         case PNG_BACKGROUND_GAMMA_UNIQUE:
            gamma_to_1 = png_reciprocal(png_ptr->background_gamma);
            gamma_correct = png_reciprocal2(png_ptr->background_gamma,
               png_ptr->screen_gamma);
            break;

         default:
            gamma_to_1 = PNG_FP_1;
            gamma_correct = PNG_FP_1;
            break;
      }

#     define CORRECT(v, c)\
         gamma_correct_background(png_ptr, (v)*mult, bit_depth,\
            &png_ptr->background.c, &png_ptr->background_1.c,\
            gamma_correct, gamma_to_1);\
         if (bit_depth > required_bit_depth)\
            png_ptr->background.c =\
               png_check_u16(png_ptr, PNG_DIV257(png_ptr->background.c))

      /* The multiplier 'mult' scales the values to 'required_depth',
       * 'bit_depth' is the depth of the resultant values.
       */
      while (bit_depth < required_bit_depth)
         mult += mult << bit_depth, bit_depth <<= 1;

      /* In the event that this still leaves the background bit depth greater
       * than the libpng required depth scale the values back to the 8-bit
       * range, the test below verifies that this is correct.
       */
      affirm(bit_depth <= required_bit_depth ||
              (bit_depth == 16 && required_bit_depth == 8));

      if ((color_type & PNG_COLOR_MASK_COLOR) != 0)
      {
         png_ptr->flags &= ~PNG_FLAG_BACKGROUND_IS_GRAY; /* checked below */

         /* If need_expand was passed to png_set_background the background value
          * was in the file format, therefore if the file is a palette file the
          * background will have been an index into the palette.  Notice that if
          * need_expand was false then the color is RGB even if the output still
          * has a palette.
          */
         if (need_expand && (color_type & PNG_COLOR_MASK_PALETTE) != 0)
         {
            unsigned int background_index = png_ptr->background.index;

            if (background_index < png_ptr->num_palette &&
               png_ptr->palette != NULL)
            {
               /* In fact 'mult' is always 1 at present in this case */
               CORRECT(png_ptr->palette[background_index].red, red);
               CORRECT(png_ptr->palette[background_index].green, green);
               CORRECT(png_ptr->palette[background_index].blue, blue);
            }

            else
            {
               png_app_error(png_ptr, "out of range background index");
               memset(&png_ptr->background, 0, sizeof png_ptr->background);
               memset(&png_ptr->background_1, 0, sizeof png_ptr->background_1);
            }
         }

         else
         {
            CORRECT(png_ptr->background.red, red);
            CORRECT(png_ptr->background.green, green);
            CORRECT(png_ptr->background.blue, blue);
         }

         if (png_ptr->background.red == png_ptr->background.blue &&
            png_ptr->background.red == png_ptr->background.green)
         {
            png_ptr->flags |= PNG_FLAG_BACKGROUND_IS_GRAY;
            png_ptr->background.gray = png_ptr->background.red;
            png_ptr->background_1.gray = png_ptr->background_1.red;
         }

         else
            png_ptr->background.gray = png_ptr->background_1.gray = 0;
      }

      else
      {
         png_ptr->flags |= PNG_FLAG_BACKGROUND_IS_GRAY;

         CORRECT(png_ptr->background.gray, gray);

         png_ptr->background.red =
         png_ptr->background.green =
         png_ptr->background.blue = png_ptr->background.gray;

         png_ptr->background_1.red =
         png_ptr->background_1.green =
         png_ptr->background_1.blue = png_ptr->background_1.gray;
      }
#     undef CORRECT
   }
}
#endif /* READ_BACKGROUND || READ_ALPHA_MODE */

static void /* PRIVATE */
png_init_palette_transformations(png_structrp png_ptr)
{
   int input_has_alpha = 0;
   int input_has_transparency = 0;

   if (png_ptr->num_trans > 0)
   {
      int i;

      /* Ignore if all the entries are opaque (unlikely!) */
      for (i=0; i<png_ptr->num_trans; ++i)
      {
         if (png_ptr->trans_alpha[i] == 255)
            continue;
         else if (png_ptr->trans_alpha[i] == 0)
            input_has_transparency = 1;
         else
         {
            input_has_transparency = 1;
            input_has_alpha = 1;
            break;
         }
      }
   }

   /* If no alpha we can optimize. */
   if (input_has_alpha == 0)
   {
      /* Any alpha means background and associative alpha processing is
       * required, however if the alpha is 0 or 1 throughout OPTIMIZE_ALPHA
       * and ENCODE_ALPHA are irrelevant.
       */
      png_ptr->transformations &= ~PNG_ENCODE_ALPHA;
      png_ptr->flags &= ~PNG_FLAG_OPTIMIZE_ALPHA;

      if (input_has_transparency == 0)
         png_ptr->transformations &= ~PNG_COMPOSE;
   }
}

static void /* PRIVATE */
png_init_rgb_transformations(png_structrp png_ptr)
{
   /* Added to libpng-1.5.4: check the color type to determine whether there
    * is any alpha or transparency in the image and simply cancel the
    * background and alpha mode stuff if there isn't.
    */
   int input_has_alpha = (png_ptr->color_type & PNG_COLOR_MASK_ALPHA) != 0;
   int input_has_transparency = png_ptr->num_trans > 0;

   /* If no alpha we can optimize. */
   if (input_has_alpha == 0)
   {
      /* Any alpha means background and associative alpha processing is
       * required, however if the alpha is 0 or 1 throughout OPTIMIZE_ALPHA
       * and ENCODE_ALPHA are irrelevant.
       */
#     ifdef PNG_READ_ALPHA_MODE_SUPPORTED
         png_ptr->transformations &= ~PNG_ENCODE_ALPHA;
         png_ptr->flags &= ~PNG_FLAG_OPTIMIZE_ALPHA;
#     endif

      if (input_has_transparency == 0)
         png_ptr->transformations &= ~PNG_COMPOSE;
   }
}

void /* PRIVATE */
png_init_read_transformations(png_structrp png_ptr)
{
   png_debug(1, "in png_init_read_transformations");

   /* This internal function is called from png_read_start_row in pngrutil.c
    * and it is called before the 'rowbytes' calculation is done, so the code
    * in here can change or update the transformations flags.
    *
    * First do updates that do not depend on the details of the PNG image data
    * being processed.
    */

#ifdef PNG_READ_GAMMA_SUPPORTED
   /* Prior to 1.5.4 these tests were performed from png_set_gamma, 1.5.4 adds
    * png_set_alpha_mode and this is another source for a default file gamma so
    * the test needs to be performed later - here.  In addition prior to 1.5.4
    * the tests were repeated for the PALETTE color type here - this is no
    * longer necessary (and doesn't seem to have been necessary before.)
    */
   {
      /* The following temporary indicates if overall gamma correction is
       * required.
       */
      int gamma_correction = 0;

      if (png_ptr->colorspace.gamma != 0) /* has been set */
      {
         if (png_ptr->screen_gamma != 0) /* screen set too */
            gamma_correction = png_gamma_threshold(png_ptr->colorspace.gamma,
               png_ptr->screen_gamma);

         else
            /* Assume the output matches the input; a long time default behavior
             * of libpng, although the standard has nothing to say about this.
             */
            png_ptr->screen_gamma = png_reciprocal(png_ptr->colorspace.gamma);
      }

      else if (png_ptr->screen_gamma != 0)
         /* The converse - assume the file matches the screen, note that this
          * perhaps undesireable default can (from 1.5.4) be changed by calling
          * png_set_alpha_mode (even if the alpha handling mode isn't required
          * or isn't changed from the default.)
          */
         png_ptr->colorspace.gamma = png_reciprocal(png_ptr->screen_gamma);

      else /* neither are set */
         /* Just in case the following prevents any processing - file and screen
          * are both assumed to be linear and there is no way to introduce a
          * third gamma value other than png_set_background with 'UNIQUE', and,
          * prior to 1.5.4
          */
         png_ptr->screen_gamma = png_ptr->colorspace.gamma = PNG_FP_1;

      /* We have a gamma value now. */
      png_ptr->colorspace.flags |= PNG_COLORSPACE_HAVE_GAMMA;

      /* Now turn the gamma transformation on or off as appropriate.  Notice
       * that PNG_GAMMA just refers to the file->screen correction.  Alpha
       * composition may independently cause gamma correction because it needs
       * linear data (e.g. if the file has a gAMA chunk but the screen gamma
       * hasn't been specified.)  In any case this flag may get turned off in
       * the code immediately below if the transform can be handled outside the
       * row loop.
       */
      if (gamma_correction != 0)
         png_ptr->transformations |= PNG_GAMMA;

      else
         png_ptr->transformations &= ~PNG_GAMMA;
   }
#endif

   /* Certain transformations have the effect of preventing other
    * transformations that happen afterward in png_do_read_transformations;
    * resolve the interdependencies here.  From the code of
    * png_do_read_transformations the order is:
    *
    *  1) PNG_EXPAND (including PNG_EXPAND_tRNS)
    *  2) PNG_STRIP_ALPHA (if no compose)
    *  3) PNG_RGB_TO_GRAY
    *  4) PNG_GRAY_TO_RGB iff !PNG_FLAG_BACKGROUND_IS_GRAY
    *  5) PNG_COMPOSE
    *  6) PNG_GAMMA
    *  7) PNG_STRIP_ALPHA (if compose)
    *  8) PNG_ENCODE_ALPHA
    *  9) PNG_SCALE_16_TO_8
    * 10) PNG_16_TO_8
    * 11) PNG_QUANTIZE (converts to palette)
    * 12) PNG_EXPAND_16
    * 13) PNG_GRAY_TO_RGB iff PNG_FLAG_BACKGROUND_IS_GRAY
    * 14) PNG_INVERT_MONO
    * 15) PNG_INVERT_ALPHA
    * 16) PNG_SHIFT
    * 17) PNG_PACK
    * 18) PNG_BGR
    * 19) PNG_PACKSWAP
    * 20) PNG_FILLER (includes PNG_ADD_ALPHA)
    * 21) PNG_SWAP_ALPHA
    * 22) PNG_SWAP_BYTES
    * 23) PNG_USER_TRANSFORM [must be last]
    */
#ifdef PNG_READ_STRIP_ALPHA_SUPPORTED
   if ((png_ptr->transformations & PNG_STRIP_ALPHA) != 0)
   {
      if ((png_ptr->transformations & PNG_FILLER) == 0)
         png_ptr->transformations &= ~(PNG_INVERT_ALPHA|PNG_SWAP_ALPHA);

      if ((png_ptr->transformations & PNG_COMPOSE) == 0)
      {
         /* Stripping the alpha channel happens immediately after the 'expand'
          * transformations, before all other transformations, so it cancels out
          * the alpha handling.  It has the side effect negating the effect of
          * PNG_EXPAND_tRNS too:
          */
         png_ptr->transformations &= ~(PNG_ENCODE_ALPHA | PNG_EXPAND_tRNS);
         png_ptr->flags &= ~PNG_FLAG_OPTIMIZE_ALPHA;

         /* Kill the tRNS chunk itself too.  Prior to 1.5.4 this did not happen
          * so transparency information would remain just so long as it wasn't
          * expanded.  This produces unexpected API changes if the set of things
          * that do PNG_EXPAND_tRNS changes (perfectly possible given the
          * documentation - which says ask for what you want, accept what you
          * get.)  This makes the behavior consistent from 1.5.4:
          */
         png_ptr->num_trans = 0;
      }
   }
#endif /* STRIP_ALPHA supported, no COMPOSE */

#ifdef PNG_READ_ALPHA_MODE_SUPPORTED
   /* If the screen gamma is about 1.0 then the OPTIMIZE_ALPHA and ENCODE_ALPHA
    * settings will have no effect.
    */
   if (png_gamma_significant(png_ptr->screen_gamma) == 0)
   {
      png_ptr->transformations &= ~PNG_ENCODE_ALPHA;
      png_ptr->flags &= ~PNG_FLAG_OPTIMIZE_ALPHA;
   }
#endif

#ifdef PNG_READ_RGB_TO_GRAY_SUPPORTED
   /* Make sure the coefficients for the rgb to gray conversion are set
    * appropriately.
    */
   if ((png_ptr->transformations & PNG_RGB_TO_GRAY) != 0)
      png_colorspace_set_rgb_coefficients(png_ptr);
#endif

   if (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      png_init_palette_transformations(png_ptr);

   else
      png_init_rgb_transformations(png_ptr);

#ifdef PNG_READ_BACKGROUND_SUPPORTED
   /* Set up the background information if required. It is only used if
    * PNG_COMPOSE is specified.
    */
   if ((png_ptr->transformations & PNG_COMPOSE) != 0)
      png_init_background_transformations(png_ptr);
#endif

   /* For indexed PNG data (PNG_COLOR_TYPE_PALETTE) many of the transformations
    * can be performed directly on the palette, and some (such as rgb to gray)
    * can be optimized inside the palette.  This is particularly true of the
    * composite (background and alpha) stuff, which can be pretty much all done
    * in the palette even if the result is expanded to RGB or gray afterward.
    *
    * NOTE: this is Not Yet Implemented, the code behaves as in 1.5.1 and
    * earlier and the palette stuff is actually handled on the first row.  This
    * leads to the reported bug that the palette returned by png_get_PLTE is not
    * updated.
    */
#if 0 /* NYI */
   png_do_palette_transformations(png_ptr);
#endif

#ifdef PNG_READ_GAMMA_SUPPORTED
   /* This needs to change - in the palette image case a whole set of tables are
    * built when it would be quicker to just calculate the correct value for
    * each palette entry directly.  Also, the test is too tricky - why check
    * PNG_RGB_TO_GRAY if PNG_GAMMA is not set?  The answer seems to be that
    * PNG_GAMMA is cancelled even if the gamma is known?  The test excludes the
    * PNG_COMPOSE case, so apparently if there is no *overall* gamma correction
    * the gamma tables will not be built even if composition is required on a
    * gamma encoded value.
    *
    * In 1.5.4 this is addressed below by an additional check on the individual
    * file gamma - if it is not 1.0 both RGB_TO_GRAY and COMPOSE need the
    * tables.
    */
   if ((png_ptr->transformations & PNG_GAMMA) != 0
      || ((png_ptr->transformations & PNG_RGB_TO_GRAY) != 0
         && (png_gamma_significant(png_ptr->colorspace.gamma) != 0 ||
            png_gamma_significant(png_ptr->screen_gamma) != 0))
      || ((png_ptr->transformations & PNG_COMPOSE) != 0
         && (png_gamma_significant(png_ptr->colorspace.gamma) != 0
            || png_gamma_significant(png_ptr->screen_gamma) != 0
#  ifdef PNG_READ_BACKGROUND_SUPPORTED
            || (png_ptr->background_gamma_type == PNG_BACKGROUND_GAMMA_UNIQUE
               && png_gamma_significant(png_ptr->background_gamma) != 0)
#  endif
      )) || ((png_ptr->transformations & PNG_ENCODE_ALPHA) != 0
         && png_gamma_significant(png_ptr->screen_gamma) != 0)
      )
   {
      png_build_gamma_tables(png_ptr, png_ptr->bit_depth);

#ifdef PNG_READ_BACKGROUND_SUPPORTED
      if ((png_ptr->transformations & PNG_COMPOSE) != 0)
      {
         /* Issue a warning about this combination: because RGB_TO_GRAY is
          * optimized to do the gamma transform if present yet do_background has
          * to do the same thing if both options are set a
          * double-gamma-correction happens.  This is true in all versions of
          * libpng to date.
          */
         if ((png_ptr->transformations & PNG_RGB_TO_GRAY) != 0)
            png_warning(png_ptr,
               "libpng does not support gamma+background+rgb_to_gray");

         if ((png_ptr->color_type == PNG_COLOR_TYPE_PALETTE) != 0)
         {
            unsigned int i, num_palette = png_ptr->num_palette;
            png_color back;
            png_color_16 back_1 = png_ptr->background_1;
            png_colorp palette = png_ptr->palette;

            back.red = png_check_byte(png_ptr, png_ptr->background.red);
            back.green = png_check_byte(png_ptr, png_ptr->background.green);
            back.blue = png_check_byte(png_ptr, png_ptr->background.blue);

            for (i = 0; i < num_palette; i++)
            {
               if (i < png_ptr->num_trans && png_ptr->trans_alpha[i] != 0xff)
               {
                  if (png_ptr->trans_alpha[i] == 0)
                  {
                     palette[i] = back;
                  }
                  else /* if (png_ptr->trans_alpha[i] != 0xff) */
                  {
                     png_uint_16 v, w;
                     unsigned int alpha = png_ptr->trans_alpha[i] * 257U;
                     unsigned int shift = png_ptr->gamma_shift;
                     unsigned int add = (shift > 0 ? 1U<<(shift-1) : 0);

                     if (png_ptr->gamma_to_1 != NULL)
                     {
                        v = png_ptr->gamma_to_1[palette[i].red];
                        png_composite_16(w, v, alpha, back_1.red);
                        palette[i].red = png_ptr->gamma_from_1[(w+add)>>shift];

                        v = png_ptr->gamma_to_1[palette[i].green];
                        png_composite_16(w, v, alpha, back_1.green);
                        palette[i].green =
                           png_ptr->gamma_from_1[(w+add)>>shift];

                        v = png_ptr->gamma_to_1[palette[i].blue];
                        png_composite_16(w, v, alpha, back_1.blue);
                        palette[i].blue = png_ptr->gamma_from_1[(w+add)>>shift];
                     }
                  }
               }
               else if (png_ptr->gamma_table != NULL)
               {
                  palette[i].red = png_ptr->gamma_table[palette[i].red];
                  palette[i].green = png_ptr->gamma_table[palette[i].green];
                  palette[i].blue = png_ptr->gamma_table[palette[i].blue];
               }
            }

            /* Prevent the transformations being done again.
             *
             * NOTE: this is highly dubious; it removes the transformations in
             * place.  This seems inconsistent with the general treatment of the
             * transformations elsewhere.
             */
            png_ptr->transformations &= ~(PNG_COMPOSE | PNG_GAMMA);
         } /* color_type == PNG_COLOR_TYPE_PALETTE */
      }/* png_ptr->transformations & PNG_BACKGROUND */

      else
      /* Transformation does not include PNG_BACKGROUND */
#endif /* READ_BACKGROUND */
      if (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE
#ifdef PNG_READ_RGB_TO_GRAY_SUPPORTED
         /* RGB_TO_GRAY needs to have non-gamma-corrected values! */
         && ((png_ptr->transformations & PNG_EXPAND) == 0 ||
         (png_ptr->transformations & PNG_RGB_TO_GRAY) == 0)
#endif
         )
      {
         png_colorp palette = png_ptr->palette;
         int num_palette = png_ptr->num_palette;
         int i;

         /* NOTE: there are other transformations that should probably be in
          * here too.
          */
         if (png_ptr->gamma_table != NULL)
         {
            for (i = 0; i < num_palette; i++)
            {
               palette[i].red = png_ptr->gamma_table[palette[i].red];
               palette[i].green = png_ptr->gamma_table[palette[i].green];
               palette[i].blue = png_ptr->gamma_table[palette[i].blue];
            }
         }

         /* Done the gamma correction. */
         png_ptr->transformations &= ~PNG_GAMMA;
      } /* color_type == PALETTE && !PNG_BACKGROUND transformation */
   }
#ifdef PNG_READ_BACKGROUND_SUPPORTED
   else
#endif
#endif /* READ_GAMMA */

#ifdef PNG_READ_BACKGROUND_SUPPORTED
   /* No GAMMA transformation (see the hanging else 4 lines above) */
   if ((png_ptr->transformations & PNG_COMPOSE) != 0 &&
       (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE))
   {
      int i;
      int istop = (int)png_ptr->num_trans;
      png_color back;
      png_colorp palette = png_ptr->palette;

      back.red   = png_check_byte(png_ptr, png_ptr->background.red);
      back.green = png_check_byte(png_ptr, png_ptr->background.green);
      back.blue  = png_check_byte(png_ptr, png_ptr->background.blue);

      for (i = 0; i < istop; i++)
      {
         if (png_ptr->trans_alpha[i] == 0)
         {
            palette[i] = back;
         }

         else if (png_ptr->trans_alpha[i] != 0xff)
         {
            /* The png_composite() macro is defined in png.h */
            png_composite(palette[i].red, palette[i].red,
                png_ptr->trans_alpha[i], back.red);

            png_composite(palette[i].green, palette[i].green,
                png_ptr->trans_alpha[i], back.green);

            png_composite(palette[i].blue, palette[i].blue,
                png_ptr->trans_alpha[i], back.blue);
         }
      }

      png_ptr->transformations &= ~PNG_COMPOSE;
   }
#endif /* READ_BACKGROUND */

#ifdef PNG_READ_SHIFT_SUPPORTED
   if ((png_ptr->transformations & PNG_SHIFT) != 0 &&
       (png_ptr->transformations & PNG_EXPAND) == 0 &&
       (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE))
   {
      int i;
      int istop = png_ptr->num_palette;
      int shift = 8 - png_ptr->sig_bit.red;

      png_ptr->transformations &= ~PNG_SHIFT;

      /* significant bits can be in the range 1 to 7 for a meaninful result, if
       * the number of significant bits is 0 then no shift is done (this is an
       * error condition which is silently ignored.)
       */
      if (shift > 0 && shift < 8)
         for (i=0; i<istop; ++i)
         {
            int component = png_ptr->palette[i].red;

            component >>= shift;
            png_ptr->palette[i].red = png_check_byte(png_ptr, component);
         }

      shift = 8 - png_ptr->sig_bit.green;
      if (shift > 0 && shift < 8)
         for (i=0; i<istop; ++i)
         {
            int component = png_ptr->palette[i].green;

            component >>= shift;
            png_ptr->palette[i].green = png_check_byte(png_ptr, component);
         }

      shift = 8 - png_ptr->sig_bit.blue;
      if (shift > 0 && shift < 8)
         for (i=0; i<istop; ++i)
         {
            int component = png_ptr->palette[i].blue;

            component >>= shift;
            png_ptr->palette[i].blue = png_check_byte(png_ptr, component);
         }
   }
#endif  /* READ_SHIFT */
}

/* Modify the info structure to reflect the transformations.  The
 * info should be updated so a PNG file could be written with it,
 * assuming the transformations result in valid PNG data.
 */
void /* PRIVATE */
png_read_transform_info(png_structrp png_ptr, png_inforp info_ptr)
{
   png_debug(1, "in png_read_transform_info");

#ifdef PNG_READ_EXPAND_SUPPORTED
   if ((png_ptr->transformations & PNG_EXPAND) != 0)
   {
      if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      {
         /* This check must match what actually happens in
          * png_do_expand_palette; if it ever checks the tRNS chunk to see if
          * it is all opaque we must do the same (at present it does not.)
          */
         if (png_ptr->num_trans > 0)
            info_ptr->color_type = PNG_COLOR_TYPE_RGB_ALPHA;

         else
            info_ptr->color_type = PNG_COLOR_TYPE_RGB;

         info_ptr->bit_depth = 8;
         info_ptr->num_trans = 0;

         if (png_ptr->palette == NULL)
            png_error (png_ptr, "Palette is NULL in indexed image");
      }
      else
      {
         if (png_ptr->num_trans != 0)
         {
            if ((png_ptr->transformations & PNG_EXPAND_tRNS) != 0)
               info_ptr->color_type |= PNG_COLOR_MASK_ALPHA;
         }
         if (info_ptr->bit_depth < 8)
            info_ptr->bit_depth = 8;

         info_ptr->num_trans = 0;
      }
   }
#endif

#if defined(PNG_READ_BACKGROUND_SUPPORTED) ||\
   defined(PNG_READ_ALPHA_MODE_SUPPORTED)
   /* The following is almost certainly wrong unless the background value is in
    * the screen space!
    */
   if ((png_ptr->transformations & PNG_COMPOSE) != 0)
      info_ptr->background = png_ptr->background;
#endif

#ifdef PNG_READ_GAMMA_SUPPORTED
   /* The following used to be conditional on PNG_GAMMA (prior to 1.5.4),
    * however it seems that the code in png_init_read_transformations, which has
    * been called before this from png_read_update_info->png_read_start_row
    * sometimes does the gamma transform and cancels the flag.
    *
    * TODO: this looks wrong; the info_ptr should end up with a gamma equal to
    * the screen_gamma value.  The following probably results in weirdness if
    * the info_ptr is used by the app after the rows have been read.
    */
   info_ptr->colorspace.gamma = png_ptr->colorspace.gamma;
#endif

   if (info_ptr->bit_depth == 16)
   {
#  ifdef PNG_READ_16BIT_SUPPORTED
#     ifdef PNG_READ_SCALE_16_TO_8_SUPPORTED
         if ((png_ptr->transformations & PNG_SCALE_16_TO_8) != 0)
            info_ptr->bit_depth = 8;
#     endif

#     ifdef PNG_READ_STRIP_16_TO_8_SUPPORTED
         if ((png_ptr->transformations & PNG_16_TO_8) != 0)
            info_ptr->bit_depth = 8;
#     endif

#  else
      /* No 16 bit support: force chopping 16-bit input down to 8, in this case
       * the app program can chose if both APIs are available by setting the
       * correct scaling to use.
       */
#     ifdef PNG_READ_STRIP_16_TO_8_SUPPORTED
         /* For compatibility with previous versions use the strip method by
          * default.  This code works because if PNG_SCALE_16_TO_8 is already
          * set the code below will do that in preference to the chop.
          */
         png_ptr->transformations |= PNG_16_TO_8;
         info_ptr->bit_depth = 8;
#     else

#        ifdef PNG_READ_SCALE_16_TO_8_SUPPORTED
            png_ptr->transformations |= PNG_SCALE_16_TO_8;
            info_ptr->bit_depth = 8;
#        else

            CONFIGURATION ERROR: you must enable at least one 16 to 8 method
#        endif
#    endif
#endif /* !READ_16BIT */
   }

#ifdef PNG_READ_GRAY_TO_RGB_SUPPORTED
   if ((png_ptr->transformations & PNG_GRAY_TO_RGB) != 0)
      info_ptr->color_type = png_check_byte(png_ptr, info_ptr->color_type |
         PNG_COLOR_MASK_COLOR);
#endif

#ifdef PNG_READ_RGB_TO_GRAY_SUPPORTED
   if ((png_ptr->transformations & PNG_RGB_TO_GRAY) != 0)
      info_ptr->color_type = png_check_byte(png_ptr, info_ptr->color_type &
         ~PNG_COLOR_MASK_COLOR);
#endif

#ifdef PNG_READ_QUANTIZE_SUPPORTED
   if ((png_ptr->transformations & PNG_QUANTIZE) != 0)
   {
      if (((info_ptr->color_type == PNG_COLOR_TYPE_RGB) ||
          (info_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA)) &&
          png_ptr->palette_lookup != 0 && info_ptr->bit_depth == 8)
      {
         info_ptr->color_type = PNG_COLOR_TYPE_PALETTE;
      }
   }
#endif

#ifdef PNG_READ_EXPAND_16_SUPPORTED
   if ((png_ptr->transformations & PNG_EXPAND_16) != 0 &&
       info_ptr->bit_depth == 8 &&
       info_ptr->color_type != PNG_COLOR_TYPE_PALETTE)
   {
      info_ptr->bit_depth = 16;
   }
#endif

#ifdef PNG_READ_PACK_SUPPORTED
   if ((png_ptr->transformations & PNG_PACK) != 0 &&
       (info_ptr->bit_depth < 8))
      info_ptr->bit_depth = 8;
#endif

   if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      info_ptr->channels = 1;

   else if ((info_ptr->color_type & PNG_COLOR_MASK_COLOR) != 0)
      info_ptr->channels = 3;

   else
      info_ptr->channels = 1;

#ifdef PNG_READ_STRIP_ALPHA_SUPPORTED
   if ((png_ptr->transformations & PNG_STRIP_ALPHA) != 0)
   {
      info_ptr->color_type = png_check_byte(png_ptr, info_ptr->color_type &
         ~PNG_COLOR_MASK_ALPHA);
      info_ptr->num_trans = 0;
   }
#endif

   if ((info_ptr->color_type & PNG_COLOR_MASK_ALPHA) != 0)
      info_ptr->channels++;

#ifdef PNG_READ_FILLER_SUPPORTED
   /* STRIP_ALPHA and FILLER allowed:  MASK_ALPHA bit stripped above */
   if ((png_ptr->transformations & PNG_FILLER) != 0 &&
       (info_ptr->color_type == PNG_COLOR_TYPE_RGB ||
       info_ptr->color_type == PNG_COLOR_TYPE_GRAY))
   {
      info_ptr->channels++;
      /* If adding a true alpha channel not just filler */
      if ((png_ptr->transformations & PNG_ADD_ALPHA) != 0)
         info_ptr->color_type |= PNG_COLOR_MASK_ALPHA;
   }
#endif

#if defined(PNG_USER_TRANSFORM_PTR_SUPPORTED) && \
defined(PNG_READ_USER_TRANSFORM_SUPPORTED)
   if ((png_ptr->transformations & PNG_USER_TRANSFORM) != 0)
   {
      if (info_ptr->bit_depth < png_ptr->user_transform_depth)
         info_ptr->bit_depth = png_ptr->user_transform_depth;

      if (info_ptr->channels < png_ptr->user_transform_channels)
         info_ptr->channels = png_ptr->user_transform_channels;
   }
#endif

   info_ptr->pixel_depth = png_check_byte(png_ptr, info_ptr->channels *
       info_ptr->bit_depth);

   info_ptr->rowbytes = PNG_ROWBYTES(info_ptr->pixel_depth, info_ptr->width);

   /* Adding in 1.5.4: cache the above value in png_struct so that we can later
    * check in png_rowbytes that the user buffer won't get overwritten.  Note
    * that the field is not always set - if png_read_update_info isn't called
    * the application has to either not do any transforms or get the calculation
    * right itself.
    */
   png_ptr->info_rowbytes = info_ptr->rowbytes;

#ifndef PNG_READ_EXPAND_SUPPORTED
   if (png_ptr != NULL)
      return;
#endif
}

#if defined (PNG_READ_PACK_SUPPORTED) || defined (PNG_READ_EXPAND_SUPPORTED)
/* Unpack pixels of 1, 2, or 4 bits per pixel into 1 byte per pixel,
 * without changing the actual values.  Thus, if you had a row with
 * a bit depth of 1, you would end up with bytes that only contained
 * the numbers 0 or 1.  If you would rather they contain 0 and 255, use
 * png_do_shift() after this.
 */
static void
png_do_unpack(png_transform_controlp row_info, png_bytep row)
{
   png_debug(1, "in png_do_unpack");

#  define png_ptr row_info->png_ptr
   if (row_info->bit_depth < 8)
   {
      switch (row_info->bit_depth)
      {
         case 1:
         {
            png_const_bytep sp = row + png_transform_rowbytes(row_info) - 1;
            /* Because we copy from the last pixel down the shift required
             * at the start is 8-pixels_in_last_byte, which is just:
             */
            unsigned int shift = 0x7 & -row_info->width;
            png_bytep dp;

            row_info->flags |= PNG_BITS_SHIFTED;
            row_info->bit_depth = 8;
            dp = row + png_transform_rowbytes(row_info);

            while (dp > row)
            {
               *--dp = (*sp >> shift) & 0x01;
               shift = 0x7 & (shift+1);
               if (shift == 0)
                  --sp;
            }

            debug(dp == row && shift == 0 && sp == row-1);
            break;
         }

         case 2:
         {
            png_const_bytep sp = row + png_transform_rowbytes(row_info) - 1;
            unsigned int shift = 7 & -(row_info->width << 1);
            png_bytep dp;

            row_info->flags |= PNG_BITS_SHIFTED;
            row_info->bit_depth = 8;
            dp = row + png_transform_rowbytes(row_info);

            while (dp > row)
            {
               *--dp = (*sp >> shift) & 0x03;
               shift = 0x7 & (shift+2);
               if (shift == 0)
                  --sp;
            }

            debug(dp == row && shift == 0 && sp == row-1);
            break;
         }

         case 4:
         {
            png_const_bytep sp = row + png_transform_rowbytes(row_info) - 1;
            unsigned int shift = 7 & -(row_info->width << 2);
            png_bytep dp;

            row_info->flags |= PNG_BITS_SHIFTED;
            row_info->bit_depth = 8;
            dp = row + png_transform_rowbytes(row_info);

            while (dp > row)
            {
               *--dp = (*sp >> shift) & 0x0f;
               shift = 0x7 & (shift+4);
               if (shift == 0)
                  --sp;
            }

            debug(dp == row && shift == 0 && sp == row-1);
            break;
         }

         default:
            break;
      }
   }
#  undef png_ptr
}
#endif /* READ_PACK || READ_EXPAND */

#ifdef PNG_READ_SHIFT_SUPPORTED
/* Reverse the effects of png_do_shift.  This routine merely shifts the
 * pixels back to their significant bits values.  Thus, if you have
 * a row of bit depth 8, but only 5 are significant, this will shift
 * the values back to 0 through 31.
 */
static void
png_do_unshift(png_transform_controlp row_info, png_bytep row)
{
   png_debug(1, "in png_do_unshift");

   /* The palette case has already been handled in the _init routine. */
   if (!(row_info->flags & PNG_INDEXED))
   {
      png_const_structrp png_ptr = row_info->png_ptr;
      unsigned int shift[4];
      unsigned int channels = 0;
      unsigned int bit_depth = row_info->bit_depth;

      if (row_info->channels > 2) /* at least three channels: color */
      {
         shift[channels++] = bit_depth - png_ptr->shift.red;
         shift[channels++] = bit_depth - png_ptr->shift.green;
         shift[channels++] = bit_depth - png_ptr->shift.blue;
      }

      else
      {
         shift[channels++] = bit_depth - png_ptr->shift.gray;
      }

      if (row_info->channels > channels) /* one more channel: alpha */
         shift[channels++] = bit_depth - png_ptr->shift.alpha;

      debug(row_info->channels == channels);

      {
         unsigned int c, have_shift;

         for (c = have_shift = 0; c < channels; ++c)
         {
            /* A shift of more than the bit depth is an error condition but it
             * gets ignored here.
             */
            if (shift[c] <= 0 || shift[c] >= bit_depth)
               shift[c] = 0;

            else
               have_shift = 1;
         }

         if (have_shift == 0)
            return;
      }

      switch (bit_depth)
      {
         default:
            /* Must be 1bpp gray: should not be here! */
            impossible("unshift bit depth");
            /* NOTREACHED */
            break;

         case 2:
         /* Must be 2bpp gray */
         debug(channels == 1 && shift[0] == 1);
         {
            png_bytep bp = row;
            png_bytep bp_end = bp + png_transform_rowbytes(row_info);

            while (bp < bp_end)
               *bp = (*bp >> 1) & 0x55, ++bp;

            row_info->flags |= PNG_BITS_SHIFTED;
            break;
         }

         case 4:
         /* Must be 4bpp gray */
         debug(channels == 1);
         {
            png_bytep bp = row;
            png_bytep bp_end = bp + png_transform_rowbytes(row_info);
            unsigned int gray_shift = shift[0];
            unsigned int mask =  0xf >> gray_shift; /* <= 4 bits */

            mask |= mask << 4; /* <= 8 bits */

            while (bp < bp_end)
               *bp = (png_byte)/*SAFE*/((*bp >> gray_shift) & mask), ++bp;

            row_info->flags |= PNG_BITS_SHIFTED;
            break;
         }

         case 8:
         /* Single byte components, G, GA, RGB, RGBA */
         {
            png_bytep bp = row;
            png_bytep bp_end = bp + png_transform_rowbytes(row_info);
            unsigned int channel = 0;

            while (bp < bp_end)
            {
               *bp = (png_byte)/*SAFE*/(*bp >> shift[channel]), ++bp;
               if (++channel >= channels)
                  channel = 0;
            }

            row_info->flags |= PNG_BITS_SHIFTED;
            break;
         }

         case 16:
         /* Double byte components, G, GA, RGB, RGBA */
         {
            png_bytep bp = row;
            png_bytep bp_end = bp + png_transform_rowbytes(row_info);
            unsigned int channel = 0;

            while (bp < bp_end)
            {
               unsigned int value = bp[0];

               value = (value << 8) + bp[1]; /* <= 16 bits */
               value >>= shift[channel];
               if (++channel >= channels)
                  channel = 0;
               *bp++ = (png_byte)/*SAFE*/(value >> 8);
               *bp++ = PNG_BYTE(value);
            }

            row_info->flags |= PNG_BITS_SHIFTED;
            break;
         }
      }
   }
}
#endif

#ifdef PNG_READ_SCALE_16_TO_8_SUPPORTED
/* Scale rows of bit depth 16 down to 8 accurately */
static void
png_do_scale_16_to_8(png_transform_controlp row_info, png_bytep row)
{
   png_debug(1, "in png_do_scale_16_to_8");

#  define png_ptr row_info->png_ptr
   if (row_info->bit_depth == 16)
   {
      png_const_bytep sp = row; /* source */
      png_bytep dp = row; /* destination */
      png_bytep ep = dp + png_transform_rowbytes(row_info); /* end+1 */

      while (sp < ep)
      {
         /* The input is an array of 16 bit components, these must be scaled to
          * 8 bits each.  For a 16 bit value V the required value (from the PNG
          * specification) is:
          *
          *    (V * 255) / 65535
          *
          * This reduces to round(V / 257), or floor((V + 128.5)/257)
          *
          * Represent V as the two byte value vhi.vlo.  Make a guess that the
          * result is the top byte of V, vhi, then the correction to this value
          * is:
          *
          *    error = floor(((V-vhi.vhi) + 128.5) / 257)
          *          = floor(((vlo-vhi) + 128.5) / 257)
          *
          * This can be approximated using integer arithmetic (and a signed
          * shift):
          *
          *    error = (vlo-vhi+128) >> 8;
          *
          * The approximate differs from the exact answer only when (vlo-vhi) is
          * 128; it then gives a correction of +1 when the exact correction is
          * 0.  This gives 128 errors.  The exact answer (correct for all 16 bit
          * input values) is:
          *
          *    error = (vlo-vhi+128)*65535 >> 24;
          *
          * An alternative arithmetic calculation which also gives no errors is:
          *
          *    (V * 255 + 32895) >> 16
          */
         png_int_32 tmp = *sp++; /* must be signed! */
         tmp += ((*sp++ - tmp + 128) * 65535) >> 24;
         *dp++ = png_check_byte(png_ptr, tmp);
      }

      row_info->bit_depth = 8;
   }
#  undef png_ptr
}
#endif

#ifdef PNG_READ_STRIP_16_TO_8_SUPPORTED
static void
/* Simply discard the low byte.  This was the default behavior prior
 * to libpng-1.5.4.
 */
png_do_chop(png_transform_controlp row_info, png_bytep row)
{
   png_debug(1, "in png_do_chop");

#  define png_ptr row_info->png_ptr

   if (row_info->bit_depth == 16)
   {
      png_const_bytep sp = row; /* source */
      png_const_bytep ep = sp + png_transform_rowbytes(row_info); /* end+1 */
      png_bytep dp = row; /* destination */

      while (sp < ep)
      {
         *dp++ = *sp;
         sp += 2; /* skip low byte */
      }

      row_info->bit_depth = 8;
   }
#  undef png_ptr
}
#endif

#ifdef PNG_READ_SWAP_ALPHA_SUPPORTED
static void
png_do_read_swap_alpha(png_transform_controlp row_info, png_bytep row)
{
   png_debug(1, "in png_do_read_swap_alpha");

#  define png_ptr row_info->png_ptr
   debug(!(row_info->flags & PNG_ALPHA_SWAPPED));

   if (!(row_info->flags & PNG_INDEXED))
   {
      if (row_info->channels == 4)
      {
         /* This converts from RGBA to ARGB */
         if (row_info->bit_depth == 8)
         {
            png_bytep dp = row + png_transform_rowbytes(row_info);

            while (dp >= row+4/*safe*/)
            {
               png_byte alpha = *--dp;
               *dp = dp[-1], --dp;
               *dp = dp[-1], --dp;
               *dp = dp[-1], --dp;
               *dp = alpha;
            }

            debug(dp == row);
            row_info->flags |= PNG_ALPHA_SWAPPED;
         }

         /* This converts from RRGGBBAA to AARRGGBB */
         else
         {
            png_bytep dp = row + png_transform_rowbytes(row_info);

            while (dp >= row+8/*safe*/)
            {
               png_byte alpha_last = *--dp;
               png_byte alpha_first = dp[-1];

               /* dp points to the second alpha byte */
               *dp = dp[-2], --dp;
               *dp = dp[-2], --dp;
               *dp = dp[-2], --dp;
               *dp = dp[-2], --dp;
               *dp = dp[-2], --dp;
               *dp = dp[-2], --dp;
               *dp = alpha_last, --dp;
               *dp = alpha_first;
            }

            debug(dp == row);
            row_info->flags |= PNG_ALPHA_SWAPPED;
         }
      }

      else if (row_info->channels == 2)
      {
         /* This converts from GA to AG */
         if (row_info->bit_depth == 8)
         {
            png_bytep dp = row + png_transform_rowbytes(row_info);

            while (dp >= row+1/*safe*/)
            {
               png_byte alpha = *--dp;

               *dp = dp[-1], --dp;
               *dp = alpha;
            }

            debug(dp == row);
            row_info->flags ^= PNG_ALPHA_SWAPPED;
         }

         /* This converts from GGAA to AAGG */
         else
         {
            png_bytep dp = row + png_transform_rowbytes(row_info);

            while (dp >= row+4/*safe*/)
            {
               png_byte alpha_last = *--dp;
               png_byte alpha_first = dp[-1];

               /* dp points to the second alpha byte */
               *dp = dp[-2], --dp;
               *dp = dp[-2], --dp;
               *dp = alpha_last, --dp;
               *dp = alpha_first;
            }

            debug(dp == row);
            row_info->flags ^= PNG_ALPHA_SWAPPED;
         }
      }
   }
#  undef png_ptr
}
#endif

#ifdef PNG_READ_INVERT_ALPHA_SUPPORTED
static void
png_do_read_invert_alpha(png_transform_controlp row_info, png_bytep row)
{
   png_debug(1, "in png_do_read_invert_alpha");

#  define png_ptr row_info->png_ptr
   debug(!(row_info->flags & PNG_ALPHA_SWAPPED));
   if (row_info->channels == 4)
   {
      if (row_info->bit_depth == 8)
      {
         /* This inverts the fourth channel in RGBA */
         png_bytep ep = row + png_transform_rowbytes(row_info);

         for (row += 3; row < ep; row += 4)
            *row ^= 0xff;

         row_info->flags ^= PNG_ALPHA_INVERTED;
      }

#ifdef PNG_READ_16BIT_SUPPORTED
      /* This inverts the alpha channel in RRGGBBAA */
      else
      {
         /* Need 2 bytes for each pixel, so subtract 1 from ep here: */
         png_bytep ep = row + png_transform_rowbytes(row_info) - 1;

         for (row += 6; row < ep; row += 8)
         {
            row[0] ^= 0xff;
            row[1] ^= 0xff;
         }

         row_info->flags ^= PNG_ALPHA_INVERTED;
      }
#endif
   }
   else if (row_info->channels == 2)
   {
      if (row_info->bit_depth == 8)
      {
         /* This inverts the alpha channel in GA */
         png_bytep ep = row + png_transform_rowbytes(row_info);

         for (row += 1; row < ep; row += 2)
            *row ^= 0xff;

         row_info->flags ^= PNG_ALPHA_INVERTED;
      }

#ifdef PNG_READ_16BIT_SUPPORTED
      else
      {
         /* This inverts the alpha channel in GGAA */
         /* Need 2 bytes for each pixel, so subtract 1 from ep here: */
         png_bytep ep = row + png_transform_rowbytes(row_info) - 1;

         for (row += 2; row < ep; row += 4)
         {
            row[0] ^= 0xff;
            row[1] ^= 0xff;
         }

         row_info->flags ^= PNG_ALPHA_INVERTED;
      }
#endif
   }
#  undef png_ptr
}
#endif

#ifdef PNG_READ_FILLER_SUPPORTED
/* Add filler channel to 1 and 3 channel non-indexed data */
static void
png_do_read_filler(png_transform_controlp row_info, png_bytep row)
{
   png_debug(1, "in png_do_read_filler");

   /* TODO: remove these checks, this code will work on any number of
    * channels but, at present, png_set_filler relies on this function
    * not doing anything in inappropriate cases.
    */
   if (!(row_info->flags & PNG_INDEXED) &&
       (row_info->channels == 1 || row_info->channels == 3) &&
       (row_info->bit_depth == 8
#ifdef PNG_READ_16BIT_SUPPORTED
        || row_info->bit_depth == 16
#endif
       ))
   {
      png_const_structrp png_ptr = row_info->png_ptr;
      png_bytep sp = row + png_transform_rowbytes(row_info); /*input*/
      png_bytep dp;

      ++(row_info->channels);
      dp = row + png_transform_rowbytes(row_info); /*output*/

      if (row_info->bit_depth == 8)
      {
         const png_byte fb = PNG_BYTE(png_ptr->filler);

         /* Add a filler before or after the current channels. */
         if ((png_ptr->flags & PNG_FLAG_FILLER_AFTER) != 0)
         {
            if (row_info->channels == 2)
            {
               while (dp >= row+2)
               {
                  *--dp = fb;
                  *--dp = *--sp;
               }

               debug(dp == row && sp == row);
            }

            else /* channels == 4 */
            {
               while (dp >= row+4)
               {
                  *--dp = fb;
                  *--dp = *--sp;
                  *--dp = *--sp;
                  *--dp = *--sp;
               }

               debug(dp == row && sp == row);
            }
         }

         else /* filler before */
         {
            if (row_info->channels == 2)
            {
               while (dp >= row+2)
               {
                  *--dp = *--sp;
                  *--dp = fb;
               }

               debug(dp == row && sp == row);
            }

            else /* channels == 4 */
            {
               while (dp >= row+4)
               {
                  *--dp = *--sp;
                  *--dp = *--sp;
                  *--dp = *--sp;
                  *--dp = fb;
               }

               debug(dp == row && sp == row);
            }
         }
      }

#  ifdef PNG_READ_16BIT_SUPPORTED
      else /* bit_depth == 16 */
      {
         /* Two byte pixels values: */
         const png_byte fb_first = PNG_BYTE(png_ptr->filler >> 8);
         const png_byte fb_last = PNG_BYTE(png_ptr->filler);

         /* Add a filler before or after the current channels. */
         if ((png_ptr->flags & PNG_FLAG_FILLER_AFTER) != 0)
         {
            if (row_info->channels == 2)
            {
               while (dp >= row+4)
               {
                  /* 2 channel bytes, 2 filler bytes */
                  *--dp = fb_last;
                  *--dp = fb_first;
                  *--dp = *--sp;
                  *--dp = *--sp;
               }

               debug(sp == row && dp == row);
            }

            else /* channels == 4 */
            {
               while (dp >= row+8)
               {
                  /* 6 channel bytes, 2 filler bytes */
                  *--dp = fb_last;
                  *--dp = fb_first;
                  dp -= 6, sp -= 6;
                  memmove(dp, sp, 6);
               }

               debug(sp == row && dp == row);
            }
         }

         else /* filler before */
         {
            if (row_info->channels == 2)
            {
               while (dp >= row+4)
               {
                  /* 2 channel bytes, 2 filler bytes */
                  *--dp = *--sp;
                  *--dp = *--sp;
                  *--dp = fb_last;
                  *--dp = fb_first;
               }

               debug(sp == row && dp == row);
            }

            else /* channels == 4 */
            {
               while (dp >= row+8)
               {
                  /* 6 channel bytes, 2 filler bytes */
                  dp -= 6, sp -= 6;
                  memmove(dp, sp, 6);
                  *--dp = fb_last;
                  *--dp = fb_first;
               }

               debug(sp == row && dp == row);
            }
         }

         if (!(png_ptr->transformations & PNG_ADD_ALPHA))
            row_info->flags |= PNG_FILLER_IN_ALPHA;
      }
#  endif
   }
}
#endif

#ifdef PNG_READ_GRAY_TO_RGB_SUPPORTED
/* Expand grayscale files to RGB, with or without alpha */
static void
png_do_gray_to_rgb(png_transform_controlp row_info, png_bytep row)
{
   png_debug(1, "in png_do_gray_to_rgb");

#  define png_ptr row_info->png_ptr

   if (!(row_info->flags & PNG_INDEXED) &&
       (row_info->bit_depth == 8 || row_info->bit_depth == 16) &&
       (row_info->channels == 1 || row_info->channels == 2))
   {
      png_bytep sp = row + png_transform_rowbytes(row_info);
      png_bytep dp;

      debug(!(row_info->flags & PNG_ALPHA_SWAPPED));

      row_info->channels += 2;
      dp = row + png_transform_rowbytes(row_info);

      if (row_info->channels == 3)
      {
         if (row_info->bit_depth == 8)
         {
            /* This changes G to RGB */
            while (sp > row)
            {
               *--dp = *--sp;
               *--dp = *sp;
               *--dp = *sp;
            }

            debug(dp == row && sp == row);
         }

         else
         {
            /* This changes GG to RRGGBB */
            while (sp > row)
            {
               const png_byte hi = *--sp;
               const png_byte lo = *--sp;
               *--dp = hi;
               *--dp = lo;
               *--dp = hi;
               *--dp = lo; /* it's off to work we go */
               *--dp = hi;
               *--dp = lo;
            }

            debug(dp == row && sp == row);
         }
      }

      else
      {
         debug(row_info->channels == 4);

         if (row_info->bit_depth == 8)
         {
            /* This changes GA to RGBA */
            while (sp > row)
            {
               *--dp = *--sp; /* A */
               *--dp = *--sp; /* G -> B */
               *--dp = *sp;   /* G -> G */
               *--dp = *sp;   /* G -> R */
            }

            debug(dp == row && sp == row);
         }

         else
         {
            /* This changes GGAA to RRGGBBAA */
            while (sp > row)
            {
               *--dp = *--sp;
               *--dp = *--sp; /* A */
               {
                  const png_byte hi = *--sp;
                  const png_byte lo = *--sp;
                  *--dp = hi;
                  *--dp = lo;
                  *--dp = hi;
                  *--dp = lo;
                  *--dp = hi;
                  *--dp = lo;
               }
            }

            debug(dp == row && sp == row);
         }
      }
   }
#  undef png_ptr
}
#endif

#ifdef PNG_READ_RGB_TO_GRAY_SUPPORTED
/* Reduce RGB files to grayscale, with or without alpha
 * using the equation given in Poynton's ColorFAQ of 1998-01-04 at
 * <http://www.inforamp.net/~poynton/>  (THIS LINK IS DEAD June 2008 but
 * versions dated 1998 through November 2002 have been archived at
 * http://web.archive.org/web/20000816232553/http://www.inforamp.net/
 * ~poynton/notes/colour_and_gamma/ColorFAQ.txt )
 * Charles Poynton poynton at poynton.com
 *
 *     Y = 0.212671 * R + 0.715160 * G + 0.072169 * B
 *
 *  which can be expressed with integers as
 *
 *     Y = (6969 * R + 23434 * G + 2365 * B)/32768
 *
 * Poynton's current link (as of January 2003 through July 2011):
 * <http://www.poynton.com/notes/colour_and_gamma/>
 * has changed the numbers slightly:
 *
 *     Y = 0.2126*R + 0.7152*G + 0.0722*B
 *
 *  which can be expressed with integers as
 *
 *     Y = (6966 * R + 23436 * G + 2366 * B)/32768
 *
 *  Historically, however, libpng uses numbers derived from the ITU-R Rec 709
 *  end point chromaticities and the D65 white point.  Depending on the
 *  precision used for the D65 white point this produces a variety of different
 *  numbers, however if the four decimal place value used in ITU-R Rec 709 is
 *  used (0.3127,0.3290) the Y calculation would be:
 *
 *     Y = (6968 * R + 23435 * G + 2366 * B)/32768
 *
 *  While this is correct the rounding results in an overflow for white, because
 *  the sum of the rounded coefficients is 32769, not 32768.  Consequently
 *  libpng uses, instead, the closest non-overflowing approximation:
 *
 *     Y = (6968 * R + 23434 * G + 2366 * B)/32768
 *
 *  Starting with libpng-1.5.5, if the image being converted has a cHRM chunk
 *  (including an sRGB chunk) then the chromaticities are used to calculate the
 *  coefficients.  See the chunk handling in pngrutil.c for more information.
 *
 *  In all cases the calculation is to be done in a linear colorspace.  If no
 *  gamma information is available to correct the encoding of the original RGB
 *  values this results in an implicit assumption that the original PNG RGB
 *  values were linear.
 *
 *  Other integer coefficents can be used via png_set_rgb_to_gray().  Because
 *  the API takes just red and green coefficients the blue coefficient is
 *  calculated to make the sum 32768.  This will result in different rounding
 *  to that used above.
 */
static int
png_do_rgb_to_gray(png_transform_controlp row_info, png_bytep row)
{
   int rgb_error = 0;

   png_debug(1, "in png_do_rgb_to_gray");

   if (!(row_info->flags & PNG_INDEXED) &&
       (row_info->channels == 3 || row_info->channels == 4))
   {
      png_const_structrp png_ptr = row_info->png_ptr;
      PNG_CONST png_uint_32 rc = png_ptr->rgb_to_gray_red_coeff;
      PNG_CONST png_uint_32 gc = png_ptr->rgb_to_gray_green_coeff;
      PNG_CONST png_uint_32 bc = 32768 - rc - gc;
      PNG_CONST png_uint_32 row_width = row_info->width;
      PNG_CONST int have_alpha = row_info->channels == 4;
      png_bytep sp = row;
      png_bytep dp = row;

      /* NOTE: rc+gc+bc == 32768 and is a (png_uint_32) value, so the worst
       * case calculation below (for white) is:
       *
       *    32768*65535+16384
       *
       * Which still fits in 32 (unsigned) bits, and:
       *
       *    (32768*65535+16384) >> 15
       *
       * is 65535 (always).  Consequently the calculation below is marked
       * SAFE.  Likewise for a png_byte value the maximum is 255.
       */
      if (row_info->bit_depth == 8)
      {
         /* Notice that gamma to/from 1 are not necessarily inverses (if
          * there is an overall gamma correction).  Prior to 1.5.5 this code
          * checked the linearized values for equality; this doesn't match
          * the documentation, the original values must be checked.
          */
         if (png_ptr->gamma_from_1 != NULL && png_ptr->gamma_to_1 != NULL)
         {
            PNG_CONST unsigned int shift = 15 + png_ptr->gamma_shift;
            PNG_CONST png_uint_32 add = 1U << (shift-1);
            png_uint_32 i;

            for (i = 0; i < row_width; i++)
            {
               png_byte red   = *(sp++);
               png_byte green = *(sp++);
               png_byte blue  = *(sp++);

               if (red != green || red != blue)
               {
                  /* gamma_to_1 is (png_uint_16[]) */
                  unsigned int red_1 = png_ptr->gamma_to_1[red];
                  unsigned int green_1 = png_ptr->gamma_to_1[green];
                  unsigned int blue_1 = png_ptr->gamma_to_1[blue];

                  rgb_error |= 1;
                  *(dp++) = png_ptr->gamma_from_1[
                      /*SAFE*/(rc*red_1 + gc*green_1 + bc*blue_1 + add)>>shift];
               }

               else
               {
                  /* If there is no overall correction the table will not be
                   * set.
                   */
                  if (png_ptr->gamma_table != NULL)
                     red = png_ptr->gamma_table[red];

                  *(dp++) = red;
               }

               if (have_alpha != 0)
                  *(dp++) = *(sp++);
            }
         }

         else
         {
            png_uint_32 i;

            for (i = 0; i < row_width; i++)
            {
               png_byte red   = *(sp++);
               png_byte green = *(sp++);
               png_byte blue  = *(sp++);

               if (red != green || red != blue)
               {
                  rgb_error |= 1;
                  *(dp++) = (png_byte)/*SAFE*/
                     ((rc*red+gc*green+bc*blue+16384) >> 15);
               }

               else
                  *(dp++) = red;

               if (have_alpha != 0)
                  *(dp++) = *(sp++);
            }
         }
      }

      else /* RGB bit_depth == 16 */
      {
         if (png_ptr->gamma_16_to_1 != NULL && png_ptr->gamma_16_from_1 != NULL)
         {
            unsigned int shift = png_ptr->gamma_shift;
            unsigned int add = (shift > 0 ? (1U << (shift-1)) : 0);
            png_uint_32 i;

            for (i = 0; i < row_width; i++)
            {
               unsigned int red, green, blue;
               png_uint_16 w;

               red = *sp++ << 8, red |= *sp++;
               green = *sp++ << 8, green |= *sp++;
               blue = *sp++ << 8, blue |= *sp++;

               if (red == green && red == blue)
               {
                  if (png_ptr->gamma_16_table != NULL)
                     w = png_ptr->gamma_16_table[/*SAFE*/(red+add) >> shift];

                  else
                     w = (png_uint_16)/*SAFE*/red;
               }

               else
               {
                  red   = png_ptr->gamma_16_to_1[/*SAFE*/(red+add) >> shift];
                  green = png_ptr->gamma_16_to_1[/*SAFE*/(green+add) >> shift];
                  blue  = png_ptr->gamma_16_to_1[/*SAFE*/(blue+add) >> shift];
                  w = png_ptr->gamma_16_from_1[/*SAFE*/
                     (((rc*red + gc*green + bc*blue + 16384)>>15)+add)>>shift];
                  rgb_error |= 1;
               }

               *(dp++) = (png_byte)/*SAFE*/(w>>8);
               *(dp++) = PNG_BYTE(w);

               if (have_alpha != 0)
               {
                  *(dp++) = *(sp++);
                  *(dp++) = *(sp++);
               }
            }
         }

         else
         {
            png_uint_32 i;

            for (i = 0; i < row_width; i++)
            {
               unsigned int red, green, blue, w;

               red = *sp++ << 8, red |= *sp++;
               green = *sp++ << 8, green |= *sp++;
               blue = *sp++ << 8, blue |= *sp++;

               if (red != green || red != blue)
                  rgb_error |= 1;

               /* From 1.5.5 in the 16 bit case do the accurate conversion even
                * in the 'fast' case - this is because this is where the code
                * ends up when handling linear 16 bit data.
                */
               w = (rc*red+gc*green+bc*blue+16384) >> 15;
               *(dp++) = (png_byte)/*SAFE*/(w>>8);
               *(dp++) = PNG_BYTE(w);

               if (have_alpha != 0)
               {
                  *(dp++) = *(sp++);
                  *(dp++) = *(sp++);
               }
            }
         }
      }

      row_info->channels -= 2;
   }

   return rgb_error;
}
#endif

#if defined(PNG_READ_BACKGROUND_SUPPORTED) ||\
   defined(PNG_READ_ALPHA_MODE_SUPPORTED)
/* Replace any alpha or transparency with the supplied background color.
 * "background" is already in the screen gamma, while "background_1" is
 * at a gamma of 1.0.  Paletted files have already been taken care of.
 */
static void
png_do_compose(png_transform_controlp row_info, png_bytep row)
{
   png_const_structrp png_ptr = row_info->png_ptr;
   png_const_bytep gamma_table = png_ptr->gamma_table;
   png_const_bytep gamma_from_1 = png_ptr->gamma_from_1;
   png_const_uint_16p gamma_to_1 = png_ptr->gamma_to_1;
   png_const_uint_16p gamma_16 = png_ptr->gamma_16_table;
   png_const_uint_16p gamma_16_from_1 = png_ptr->gamma_16_from_1;
   png_const_uint_16p gamma_16_to_1 = png_ptr->gamma_16_to_1;
   PNG_CONST unsigned int shift = png_ptr->gamma_shift;
   PNG_CONST unsigned int add = (shift > 0 ? 1U<<(shift-1) : 0);
   PNG_CONST int optimize = (png_ptr->flags & PNG_FLAG_OPTIMIZE_ALPHA) != 0;

   png_bytep sp;
   png_uint_32 i;
   png_uint_32 row_width = row_info->width;

   png_debug(1, "in png_do_compose");

   if (!(row_info->flags & PNG_INDEXED)) {
      switch (row_info->channels)
      {
         case 1 /*GRAY*/:
         {
            switch (row_info->bit_depth)
            {
               case 1:
               {
                  int bit_shift = 7;
                  sp = row;
                  for (i = 0; i < row_width; i++)
                  {
                     if (((*sp >> bit_shift) & 0x01) ==
                             png_ptr->trans_color.gray)
                     {
                        unsigned int tmp = *sp & (0x7f7f >> (7 - bit_shift));
                        tmp |= png_ptr->background.gray << bit_shift;
                        *sp = png_check_byte(png_ptr, tmp);
                     }

                     if (bit_shift == 0)
                     {
                        bit_shift = 7;
                        sp++;
                     }

                     else
                        bit_shift--;
                  }
                  break;
               }

               case 2:
               {
#if 0
                  if (gamma_table != NULL)
                  {
                     int bit_shift = 6;
                     sp = row;
                     for (i = 0; i < row_width; i++)
                     {
                        if (((*sp >> bit_shift) & 0x03) ==
                                png_ptr->trans_color.gray)
                        {
                           unsigned int tmp = *sp & (0x3f3f >> (6 - bit_shift));
                           tmp |= png_ptr->background.gray << bit_shift;
                           *sp = png_check_byte(png_ptr, tmp);
                        }

                        else
                        {
                           unsigned int p = (*sp >> bit_shift) & 0x03;
                           unsigned int g = (gamma_table [p | (p << 2) |
                               (p << 4) | (p << 6)] >> 6) & 0x03;
                           unsigned int tmp = *sp & (0x3f3f >> (6 - bit_shift));
                           tmp |= g << bit_shift;
                           *sp = png_check_byte(png_ptr, tmp);
                        }

                        if (bit_shift == 0)
                        {
                           bit_shift = 6;
                           sp++;
                        }

                        else
                           bit_shift -= 2;
                     }
                  }

                  else
#endif
                  {
                     int bit_shift = 6;
                     sp = row;
                     for (i = 0; i < row_width; i++)
                     {
                        if (((*sp >> bit_shift) & 0x03) ==
                                png_ptr->trans_color.gray)
                        {
                           unsigned int tmp = *sp & (0x3f3f >> (6 - bit_shift));
                           tmp |= png_ptr->background.gray << bit_shift;
                           *sp = png_check_byte(png_ptr, tmp);
                        }

                        if (bit_shift == 0)
                        {
                           bit_shift = 6;
                           sp++;
                        }

                        else
                           bit_shift -= 2;
                     }
                  }
                  break;
               }

               case 4:
               {
#if 0
                  if (gamma_table != NULL)
                  {
                     int bit_shift = 4;
                     sp = row;
                     for (i = 0; i < row_width; i++)
                     {
                        if (((*sp >> bit_shift) & 0x0f) ==
                                png_ptr->trans_color.gray)
                        {
                           unsigned int tmp = *sp & (0xf0f >> (4 - bit_shift));
                           tmp |= png_ptr->background.gray << bit_shift;
                           *sp = png_check_byte(png_ptr, tmp);
                        }

                        else
                        {
                           unsigned int p = (*sp >> bit_shift) & 0x0f;
                           unsigned int g = (gamma_table[p | (p << 4)] >> 4) &
                              0x0f;
                           unsigned int tmp = *sp & (0xf0f >> (4 - bit_shift));
                           tmp |= g << bit_shift;
                           *sp = png_check_byte(png_ptr, tmp);
                        }

                        if (bit_shift == 0)
                        {
                           bit_shift = 4;
                           sp++;
                        }

                        else
                           bit_shift -= 4;
                     }
                  }

                  else
#endif
                  {
                     int bit_shift = 4;
                     sp = row;
                     for (i = 0; i < row_width; i++)
                     {
                        if (((*sp >> bit_shift) & 0x0f) ==
                                png_ptr->trans_color.gray)
                        {
                           unsigned int tmp = *sp & (0xf0f >> (4 - bit_shift));
                           tmp |= png_ptr->background.gray << bit_shift;
                           *sp = png_check_byte(png_ptr, tmp);
                        }

                        if (bit_shift == 0)
                        {
                           bit_shift = 4;
                           sp++;
                        }

                        else
                           bit_shift -= 4;
                     }
                  }
                  break;
               }

               case 8:
               {
                  if (gamma_table != NULL)
                  {
                     sp = row;
                     for (i = 0; i < row_width; i++, sp++)
                     {
                        if (*sp == png_ptr->trans_color.gray)
                           *sp = png_check_byte(png_ptr,
                              png_ptr->background.gray);

                        else
                           *sp = gamma_table[*sp];
                     }
                  }

                  else
                  {
                     sp = row;
                     for (i = 0; i < row_width; i++, sp++)
                     {
                        if (*sp == png_ptr->trans_color.gray)
                           *sp = png_check_byte(png_ptr,
                              png_ptr->background.gray);
                     }
                  }
                  break;
               }

               case 16:
               {
                  if (gamma_16 != NULL)
                  {
                     sp = row;
                     for (i = 0; i < row_width; i++, sp += 2)
                     {
                        png_uint_16 v;

                        v = png_check_u16(png_ptr, ((*sp) << 8) + *(sp + 1));

                        if (v == png_ptr->trans_color.gray)
                        {
                           /* Background is already in screen gamma */
                           *sp = png_check_byte(png_ptr,
                              png_ptr->background.gray >> 8);
                           *(sp + 1) = PNG_BYTE(png_ptr->background.gray);
                        }

                        else
                        {
                           v = gamma_16[(v+add) >> shift];
                           *sp = png_check_byte(png_ptr, v >> 8);
                           *(sp + 1) = PNG_BYTE(v);
                        }
                     }
                  }

                  else
                  {
                     sp = row;
                     for (i = 0; i < row_width; i++, sp += 2)
                     {
                        png_uint_16 v;

                        v = png_check_u16(png_ptr, ((*sp) << 8) + *(sp + 1));

                        if (v == png_ptr->trans_color.gray)
                        {
                           *sp = png_check_byte(png_ptr,
                              png_ptr->background.gray >> 8);
                           *(sp + 1) = PNG_BYTE(png_ptr->background.gray);
                        }
                     }
                  }
                  break;
               }

               default:
                  break;
            }
            break;
         }

         case 3 /*RGB*/:
         {
            if (row_info->bit_depth == 8)
            {
               if (gamma_table != NULL)
               {
                  sp = row;
                  for (i = 0; i < row_width; i++, sp += 3)
                  {
                     if (*sp == png_ptr->trans_color.red &&
                         *(sp + 1) == png_ptr->trans_color.green &&
                         *(sp + 2) == png_ptr->trans_color.blue)
                     {
                        *sp = png_check_byte(png_ptr, png_ptr->background.red);
                        *(sp + 1) = png_check_byte(png_ptr,
                           png_ptr->background.green);
                        *(sp + 2) = png_check_byte(png_ptr,
                           png_ptr->background.blue);
                     }

                     else
                     {
                        *sp = gamma_table[*sp];
                        *(sp + 1) = gamma_table[*(sp + 1)];
                        *(sp + 2) = gamma_table[*(sp + 2)];
                     }
                  }
               }

               else
               {
                  sp = row;
                  for (i = 0; i < row_width; i++, sp += 3)
                  {
                     if (*sp == png_ptr->trans_color.red &&
                         *(sp + 1) == png_ptr->trans_color.green &&
                         *(sp + 2) == png_ptr->trans_color.blue)
                     {
                        *sp = png_check_byte(png_ptr, png_ptr->background.red);
                        *(sp + 1) = png_check_byte(png_ptr,
                           png_ptr->background.green);
                        *(sp + 2) = png_check_byte(png_ptr,
                           png_ptr->background.blue);
                     }
                  }
               }
            }

            else /* if (row_info->bit_depth == 16) */
            {
               if (gamma_16 != NULL)
               {
                  sp = row;
                  for (i = 0; i < row_width; i++, sp += 6)
                  {
                     png_uint_16 r = png_check_u16(png_ptr,
                        ((*sp) << 8) + *(sp + 1));

                     png_uint_16 g = png_check_u16(png_ptr,
                         ((*(sp + 2)) << 8) + *(sp + 3));

                     png_uint_16 b = png_check_u16(png_ptr,
                         ((*(sp + 4)) << 8) + *(sp + 5));

                     if (r == png_ptr->trans_color.red &&
                         g == png_ptr->trans_color.green &&
                         b == png_ptr->trans_color.blue)
                     {
                        /* Background is already in screen gamma */
                        *sp = png_check_byte(png_ptr,
                           png_ptr->background.red >> 8);
                        *(sp + 1) = PNG_BYTE(png_ptr->background.red);
                        *(sp + 2) = png_check_byte(png_ptr,
                           png_ptr->background.green >> 8);
                        *(sp + 3) = PNG_BYTE(png_ptr->background.green);
                        *(sp + 4) = png_check_byte(png_ptr,
                           png_ptr->background.blue >> 8);
                        *(sp + 5) = PNG_BYTE(png_ptr->background.blue);
                     }

                     else
                     {
                        png_uint_16 v = gamma_16[(r+add) >> shift];
                        *sp = (png_byte)/*SAFE*/(v >> 8);
                        *(sp + 1) = PNG_BYTE(v);

                        v = gamma_16[(g+add) >> shift];
                        *(sp + 2) = (png_byte)/*SAFE*/(v >> 8);
                        *(sp + 3) = PNG_BYTE(v);

                        v = gamma_16[(b+add) >> shift];
                        *(sp + 4) = (png_byte)/*SAFE*/(v >> 8);
                        *(sp + 5) = PNG_BYTE(v);
                     }
                  }
               }

               else
               {
                  sp = row;
                  for (i = 0; i < row_width; i++, sp += 6)
                  {
                     unsigned int r = ((*sp) << 8) + *(sp + 1);
                     unsigned int g = ((*(sp + 2)) << 8) + *(sp + 3);
                     unsigned int b = ((*(sp + 4)) << 8) + *(sp + 5);

                     if (r == png_ptr->trans_color.red &&
                         g == png_ptr->trans_color.green &&
                         b == png_ptr->trans_color.blue)
                     {
                        *sp = (png_byte)/*SAFE*/(png_ptr->background.red >> 8);
                        *(sp + 1) = PNG_BYTE(png_ptr->background.red);
                        *(sp + 2) = (png_byte)/*SAFE*/(
                           png_ptr->background.green >> 8);
                        *(sp + 3) = PNG_BYTE(png_ptr->background.green);
                        *(sp + 4) = (png_byte)/*SAFE*/(
                           png_ptr->background.blue >> 8);
                        *(sp + 5) = PNG_BYTE(png_ptr->background.blue);
                        /*UNTESTED*/
                     }
                  }
                  /*UNTESTED*/
               }
            }
            break;
         }

         case 2 /*GRAY_ALPHA*/:
         {
            if (row_info->bit_depth == 8)
            {
               if (gamma_to_1 != NULL && gamma_from_1 != NULL &&
                   gamma_table != NULL)
               {
                  sp = row;
                  for (i = 0; i < row_width; i++, sp += 2)
                  {
                     unsigned int a = *(sp + 1);

                     if (a == 0xff)
                        *sp = gamma_table[*sp];

                     else if (a == 0)
                     {
                        /* Background is already in screen gamma */
                        *sp = png_check_byte(png_ptr, png_ptr->background.gray);
                     }

                     else
                     {
                        unsigned int v, w;

                        v = gamma_to_1[*sp];
                        png_composite_16(w, v, 257*a,
                           png_ptr->background_1.gray);

                        if (optimize == 0)
                           w = gamma_from_1[(w+add)>>shift];

                        else /* alpha pixels linear and approximate */
                           w = PNG_DIV257(w);

                        *sp = png_check_byte(png_ptr, w);
                     }
                  }
               }

               else
               {
                  sp = row;
                  for (i = 0; i < row_width; i++, sp += 2)
                  {
                     png_byte a = *(sp + 1);

                     if (a == 0)
                        *sp = png_check_byte(png_ptr, png_ptr->background.gray);

                     else if (a < 0xff)
                        png_composite(*sp, *sp, a, png_ptr->background.gray);
                  }
               }
            }

            else /* if (png_ptr->bit_depth == 16) */
            {
               if (gamma_16 != NULL && gamma_16_from_1 != NULL &&
                   gamma_16_to_1 != NULL)
               {
                  sp = row;
                  for (i = 0; i < row_width; i++, sp += 4)
                  {
                     png_uint_16 a = png_check_u16(png_ptr,
                         ((*(sp + 2)) << 8) + *(sp + 3));

                     if (a == 65535)
                     {
                        unsigned int v;

                        v = gamma_16[((sp[0]<<8)+sp[1]+add) >> shift];
                        *sp = (png_byte)/*SAFE*/(v >> 8);
                        *(sp + 1) = PNG_BYTE(v);
                     }

                     else if (a == 0)
                     {
                        /* Background is already in screen gamma */
                        *sp = png_check_byte(png_ptr,
                           png_ptr->background.gray >> 8);
                        *(sp + 1) = PNG_BYTE(png_ptr->background.gray);
                     }

                     else
                     {
                        png_uint_16 g, v, w;

                        g = gamma_16_to_1[((sp[0]<<8)+sp[1]+add) >> shift];
                        png_composite_16(v, g, a, png_ptr->background_1.gray);

                        if (optimize == 0)
                           w = gamma_16_from_1[(v+add) >> shift];

                        else
                           w = v;

                        *sp = png_check_byte(png_ptr, w >> 8);
                        *(sp + 1) = PNG_BYTE(w);
                     }
                  }
               }

               else
               {
                  sp = row;
                  for (i = 0; i < row_width; i++, sp += 4)
                  {
                     png_uint_16 a = png_check_u16(png_ptr,
                         ((*(sp + 2)) << 8) + *(sp + 3));

                     if (a == 0)
                     {
                        *sp = png_check_byte(png_ptr,
                           png_ptr->background.gray >> 8);
                        *(sp + 1) = PNG_BYTE(png_ptr->background.gray);
                     }

                     else if (a < 0xffff)
                     {
                        unsigned int g, v;

                        g = ((*sp) << 8) + *(sp + 1);
                        png_composite_16(v, g, a, png_ptr->background.gray);
                        *sp = png_check_byte(png_ptr, v >> 8);
                        *(sp + 1) = PNG_BYTE(v);
                     }
                  }
               }
            }
            break;
         }

         case 4 /*RGB_ALPHA*/:
         {
            if (row_info->bit_depth == 8)
            {
               if (gamma_to_1 != NULL && gamma_from_1 != NULL &&
                   gamma_table != NULL)
               {
                  sp = row;
                  for (i = 0; i < row_width; i++, sp += 4)
                  {
                     png_byte a = *(sp + 3);

                     if (a == 0xff)
                     {
                        *sp = gamma_table[*sp];
                        *(sp + 1) = gamma_table[*(sp + 1)];
                        *(sp + 2) = gamma_table[*(sp + 2)];
                     }

                     else if (a == 0)
                     {
                        /* Background is already in screen gamma */
                        *sp = png_check_byte(png_ptr, png_ptr->background.red);
                        *(sp + 1) = png_check_byte(png_ptr,
                           png_ptr->background.green);
                        *(sp + 2) = png_check_byte(png_ptr,
                           png_ptr->background.blue);
                     }

                     else
                     {
                        unsigned int v, w;
                        unsigned int alpha = a * 257U;

                        v = gamma_to_1[*sp];
                        png_composite_16(w, v, alpha,
                           png_ptr->background_1.red);

                        if (optimize == 0)
                           w = gamma_from_1[(w+add)>>shift];
                        
                        else
                           w = PNG_DIV257(w);

                        *sp = png_check_byte(png_ptr, w);

                        v = gamma_to_1[*(sp + 1)];
                        png_composite_16(w, v, alpha,
                           png_ptr->background_1.green);

                        if (optimize == 0)
                           w = gamma_from_1[(w+add)>>shift];
                        
                        else
                           w = PNG_DIV257(w);

                        *(sp + 1) = png_check_byte(png_ptr, w);

                        v = gamma_to_1[*(sp + 2)];
                        png_composite_16(w, v, alpha,
                           png_ptr->background_1.blue);

                        if (optimize == 0)
                           w = gamma_from_1[(w+add)>>shift];
                        
                        else
                           w = PNG_DIV257(w);

                        *(sp + 2) = png_check_byte(png_ptr, w);
                     }
                  }
               }

               else
               {
                  sp = row;
                  for (i = 0; i < row_width; i++, sp += 4)
                  {
                     png_byte a = *(sp + 3);

                     if (a == 0)
                     {
                        *sp = png_check_byte(png_ptr, png_ptr->background.red);
                        *(sp + 1) = png_check_byte(png_ptr,
                           png_ptr->background.green);
                        *(sp + 2) = png_check_byte(png_ptr,
                           png_ptr->background.blue);
                     }

                     else if (a < 0xff)
                     {
                        png_composite(*sp, *sp, a, png_ptr->background.red);

                        png_composite(*(sp + 1), *(sp + 1), a,
                            png_ptr->background.green);

                        png_composite(*(sp + 2), *(sp + 2), a,
                            png_ptr->background.blue);
                     }
                  }
               }
            }

            else /* if (row_info->bit_depth == 16) */
            {
               if (gamma_16 != NULL && gamma_16_from_1 != NULL &&
                   gamma_16_to_1 != NULL)
               {
                  sp = row;
                  for (i = 0; i < row_width; i++, sp += 8)
                  {
                     png_uint_16 a = png_check_u16(png_ptr,
                        ((*(sp + 6)) << 8) + *(sp + 7));

                     if (a == 65535)
                     {
                        png_uint_16 v;

                        v = gamma_16[((sp[0]<<8)+sp[1]+add) >> shift];
                        *sp = png_check_byte(png_ptr, v >> 8);
                        *(sp + 1) = PNG_BYTE(v);

                        v = gamma_16[((sp[2]<<8)+sp[3]+add) >> shift];
                        *(sp + 2) = png_check_byte(png_ptr, v >> 8);
                        *(sp + 3) = PNG_BYTE(v);

                        v = gamma_16[((sp[4]<<8)+sp[5]+add) >> shift];
                        *(sp + 4) = png_check_byte(png_ptr, v >> 8);
                        *(sp + 5) = PNG_BYTE(v);
                     }

                     else if (a == 0)
                     {
                        /* Background is already in screen gamma */
                        *sp = png_check_byte(png_ptr,
                           png_ptr->background.red >> 8);
                        *(sp + 1) = PNG_BYTE(png_ptr->background.red);
                        *(sp + 2) = png_check_byte(png_ptr,
                           png_ptr->background.green >> 8);
                        *(sp + 3) = PNG_BYTE(png_ptr->background.green);
                        *(sp + 4) = png_check_byte(png_ptr,
                           png_ptr->background.blue >> 8);
                        *(sp + 5) = PNG_BYTE(png_ptr->background.blue);
                     }

                     else
                     {
                        png_uint_16 v, w;

                        v = gamma_16_to_1[((sp[0]<<8)+sp[1]+add) >> shift];
                        png_composite_16(w, v, a, png_ptr->background_1.red);

                        if (optimize == 0)
                           w = gamma_16_from_1[(w+add) >> shift];

                        *sp = png_check_byte(png_ptr, w >> 8);
                        *(sp + 1) = PNG_BYTE(w);

                        v = gamma_16_to_1[((sp[2]<<8)+sp[3]+add) >> shift];
                        png_composite_16(w, v, a, png_ptr->background_1.green);

                        if (optimize == 0)
                           w = gamma_16_from_1[(w+add) >> shift];

                        *(sp + 2) = png_check_byte(png_ptr, w >> 8);
                        *(sp + 3) = PNG_BYTE(w);

                        v = gamma_16_to_1[((sp[4]<<8)+sp[5]+add) >> shift];
                        png_composite_16(w, v, a, png_ptr->background_1.blue);

                        if (optimize == 0)
                           w = gamma_16_from_1[(w+add) >> shift];

                        *(sp + 4) = png_check_byte(png_ptr, w >> 8);
                        *(sp + 5) = PNG_BYTE(w);
                     }
                  }
               }

               else
               {
                  sp = row;
                  for (i = 0; i < row_width; i++, sp += 8)
                  {
                     png_uint_16 a = png_check_u16(png_ptr,
                        ((*(sp + 6)) << 8) + *(sp + 7));

                     if (a == 0)
                     {
                        *sp = png_check_byte(png_ptr,
                           png_ptr->background.red >> 8);
                        *(sp + 1) = PNG_BYTE(png_ptr->background.red);
                        *(sp + 2) = png_check_byte(png_ptr,
                           png_ptr->background.green >> 8);
                        *(sp + 3) = PNG_BYTE(png_ptr->background.green);
                        *(sp + 4) = png_check_byte(png_ptr,
                           png_ptr->background.blue >> 8);
                        *(sp + 5) = PNG_BYTE(png_ptr->background.blue);
                     }

                     else if (a < 0xffff)
                     {
                        png_uint_16 v;

                        png_uint_16 r = png_check_u16(png_ptr,
                           ((*sp) << 8) + *(sp + 1));
                        png_uint_16 g = png_check_u16(png_ptr,
                            ((*(sp + 2)) << 8) + *(sp + 3));
                        png_uint_16 b = png_check_u16(png_ptr,
                            ((*(sp + 4)) << 8) + *(sp + 5));

                        png_composite_16(v, r, a, png_ptr->background.red);
                        *sp = png_check_byte(png_ptr, v >> 8);
                        *(sp + 1) = PNG_BYTE(v);

                        png_composite_16(v, g, a, png_ptr->background.green);
                        *(sp + 2) = png_check_byte(png_ptr, v >> 8);
                        *(sp + 3) = PNG_BYTE(v);

                        png_composite_16(v, b, a, png_ptr->background.blue);
                        *(sp + 4) = png_check_byte(png_ptr, v >> 8);
                        *(sp + 5) = PNG_BYTE(v);
                     }
                  }
               }
            }
            break;
         }

         default:
            break;
      }
   }
}
#endif /* READ_BACKGROUND || READ_ALPHA_MODE */

#ifdef PNG_READ_GAMMA_SUPPORTED
/* Gamma correct the image, avoiding the alpha channel.  Make sure
 * you do this after you deal with the transparency issue on grayscale
 * or RGB images. If your bit depth is 8, use gamma_table, if it
 * is 16, use gamma_16_table and gamma_shift.  Build these with
 * build_gamma_table().
 */
static void
png_do_gamma(png_transform_controlp row_info, png_bytep row)
{
   png_const_structrp png_ptr = row_info->png_ptr;
   png_const_bytep gamma_table = png_ptr->gamma_table;
   png_const_uint_16p gamma_16_table = png_ptr->gamma_16_table;
   int shift = png_ptr->gamma_shift;
   int add = (shift > 0 ? 1U << (shift-1) : 0);

   png_bytep sp;
   png_uint_32 i;
   png_uint_32 row_width=row_info->width;

   png_debug(1, "in png_do_gamma");

   /* Prior to libpng 1.7.0 this code would attempt to gamma correct 2 and 4 bit
    * gray level values, the results are ridiculously inaccurate.  In 1.7.0 the
    * code is removed and a warning is introduced to catch cases where an
    * application might actually try it.
    */
   if (((row_info->bit_depth == 8 && gamma_table != NULL) ||
       (row_info->bit_depth == 16 && gamma_16_table != NULL)))
   {
      if (!(row_info->flags & PNG_INDEXED)) switch (row_info->channels)
      {
         case 3 /*RGB*/:
         {
            if (row_info->bit_depth == 8)
            {
               sp = row;
               for (i = 0; i < row_width; i++)
               {
                  *sp = gamma_table[*sp];
                  sp++;
                  *sp = gamma_table[*sp];
                  sp++;
                  *sp = gamma_table[*sp];
                  sp++;
               }
            }

            else /* if (row_info->bit_depth == 16) */
            {
               sp = row;
               for (i = 0; i < row_width; i++)
               {
                  png_uint_16 v;

                  v = gamma_16_table[((sp[0]<<8)+sp[1]+add) >> shift];
                  *sp = png_check_byte(png_ptr, v >> 8);
                  *(sp + 1) = PNG_BYTE(v);
                  sp += 2;

                  v = gamma_16_table[((sp[0]<<8)+sp[1]+add) >> shift];
                  *sp = png_check_byte(png_ptr, v >> 8);
                  *(sp + 1) = PNG_BYTE(v);
                  sp += 2;

                  v = gamma_16_table[((sp[0]<<8)+sp[1]+add) >> shift];
                  *sp = png_check_byte(png_ptr, v >> 8);
                  *(sp + 1) = PNG_BYTE(v);
                  sp += 2;
               }
            }
            break;
         }

         case 4 /*RGB_ALPHA*/:
         {
            if (row_info->bit_depth == 8)
            {
               sp = row;
               for (i = 0; i < row_width; i++)
               {
                  *sp = gamma_table[*sp];
                  sp++;

                  *sp = gamma_table[*sp];
                  sp++;

                  *sp = gamma_table[*sp];
                  sp++;

                  sp++;
               }
            }

            else /* if (row_info->bit_depth == 16) */
            {
               sp = row;
               for (i = 0; i < row_width; i++)
               {
                  png_uint_16 v;

                  v = gamma_16_table[((sp[0]<<8)+sp[1]+add) >> shift];
                  *sp = png_check_byte(png_ptr, v >> 8);
                  *(sp + 1) = PNG_BYTE(v);
                  sp += 2;

                  v = gamma_16_table[((sp[0]<<8)+sp[1]+add) >> shift];
                  *sp = png_check_byte(png_ptr, v >> 8);
                  *(sp + 1) = PNG_BYTE(v);
                  sp += 2;

                  v = gamma_16_table[((sp[0]<<8)+sp[1]+add) >> shift];
                  *sp = png_check_byte(png_ptr, v >> 8);
                  *(sp + 1) = PNG_BYTE(v);
                  sp += 4;
               }
            }
            break;
         }

         case 2 /*GRAY_ALPHA*/:
         {
            if (row_info->bit_depth == 8)
            {
               sp = row;
               for (i = 0; i < row_width; i++)
               {
                  *sp = gamma_table[*sp];
                  sp += 2;
               }
            }

            else /* if (row_info->bit_depth == 16) */
            {
               sp = row;
               for (i = 0; i < row_width; i++)
               {
                  png_uint_16 v;

                  v = gamma_16_table[((sp[0]<<8)+sp[1]+add) >> shift];
                  *sp = png_check_byte(png_ptr, v >> 8);
                  *(sp + 1) = PNG_BYTE(v);
                  sp += 4;
               }
            }
            break;
         }

         case 1 /*GRAY*/:
         {
            if (row_info->bit_depth == 8)
            {
               sp = row;
               for (i = 0; i < row_width; i++)
               {
                  *sp = gamma_table[*sp];
                  sp++;
               }
            }

            else /*row_info->bit_depth == 16 */
            {
               sp = row;
               for (i = 0; i < row_width; i++)
               {
                  png_uint_16 v;

                  v = gamma_16_table[((sp[0]<<8)+sp[1]+add) >> shift];
                  *sp = png_check_byte(png_ptr, v >> 8);
                  *(sp + 1) = PNG_BYTE(v);
                  sp += 2;
               }
            }
            break;
         }

         default:
            break;
      }
   }
}
#endif

#ifdef PNG_READ_ALPHA_MODE_SUPPORTED
/* Encode the alpha channel to the output gamma (the input channel is always
 * linear.)  Called only with color types that have an alpha channel.  Needs the
 * from_1 tables.
 */
static void
png_do_encode_alpha(png_transform_controlp row_info, png_bytep row)
{
   int step = row_info->channels;

   png_debug(1, "in png_do_encode_alpha");

   if ((step == 2 || step == 4) && !(row_info->flags & PNG_INDEXED))
   {
      png_const_structrp png_ptr = row_info->png_ptr;
      PNG_CONST unsigned int shift = png_ptr->gamma_shift;
      PNG_CONST unsigned int add = (shift > 0 ? 1U<<(shift-1) : 0);
      png_uint_32 row_width = row_info->width;

      if (row_info->bit_depth == 8)
      {
         PNG_CONST png_bytep gamma_from_1 = png_ptr->gamma_from_1;

         affirm(gamma_from_1 != NULL);

         {
            /* The alpha channel is the last component: */
            row += step - 1;

            for (; row_width > 0; --row_width, row += step)
               *row = gamma_from_1[(257U**row+add)>>shift];
         }
      }

      else if (row_info->bit_depth == 16)
      {
         PNG_CONST png_uint_16p gamma_16_from_1 = png_ptr->gamma_16_from_1;

         affirm(gamma_16_from_1 != NULL);

         {
            step *= 2;

            /* The alpha channel is the last component: */
            row += step - 2;

            for (; row_width > 0; --row_width, row += step)
            {
               png_uint_16 v;

               v = gamma_16_from_1[((row[0]<<8)+row[1]+add) >> shift];
               *row = png_check_byte(png_ptr, v >> 8);
               *(row + 1) = PNG_BYTE(v);
            }
         }
      }
   }
}
#endif

#ifdef PNG_READ_EXPAND_SUPPORTED
/* Expands a palette row to an RGB or RGBA row depending
 * upon whether you supply trans and num_trans.
 */
static void
png_do_expand_palette(png_transform_controlp row_info, png_bytep row)
{
   png_debug(1, "in png_do_expand_palette");

   if ((row_info->flags & PNG_INDEXED) && row_info->channels == 1)
   {
      png_const_structrp png_ptr = row_info->png_ptr;

      /* Unpack packed pixels into 1-per-byte: */
      if (row_info->bit_depth < 8)
      {
         png_do_unpack(row_info, row);
         debug(row_info->flags & PNG_BITS_SHIFTED);
         row_info->flags &= ~PNG_BITS_SHIFTED;
      }

      affirm(row_info->bit_depth == 8);

      {  /* 8-bit per index, unpack to RGB or RGBA */
         png_const_colorp palette = png_ptr->palette;
         const int num_palette = png_ptr->num_palette;
         int num_trans = png_ptr->num_trans;
         int bad_index = 0;
         png_const_bytep sp = row + png_transform_rowbytes(row_info);
         png_bytep dp;

         if (num_trans > num_palette)
            num_trans = num_palette; /* 11.3.2.1: tRNS no longer than palette */

         if (num_trans > 0) /* Unpack to RGBA */
         {
            png_const_bytep trans_alpha = png_ptr->trans_alpha;
            
            row_info->channels = 4;
            dp = row + png_transform_rowbytes(row_info);

            while (dp >= row+4)
            {
               const int index = *--sp;

               if (index < num_trans)
                  *--dp = trans_alpha[index];

               else
                  *--dp = 0xff;

               if (index < num_palette)
               {
                  *--dp = palette[index].blue;
                  *--dp = palette[index].green;
                  *--dp = palette[index].red;
               }

               else
               {
                  bad_index = index;
                  *--dp = 0; /* default to black */
                  *--dp = 0;
                  *--dp = 0;
               }
            }

            debug(dp == row && sp == row);
         }

         else /* Unpack to RGB */
         {
            row_info->channels = 3;
            dp = row + png_transform_rowbytes(row_info);

            while (dp >= row+3)
            {
               const int index = *--sp;

               if (index < num_palette)
               {
                  *--dp = palette[index].blue;
                  *--dp = palette[index].green;
                  *--dp = palette[index].red;
               }

               else
               {
                  bad_index = index;
                  *--dp = 0; /* default to black */
                  *--dp = 0;
                  *--dp = 0;
               }
            }

            debug(sp == row && sp == row);
         }

         /* At this point we have squirted new RGB or RGBA values into
          * the row, this zaps all the error flags *and* PNG_INDEXED,
          * if a bad index we detected we record that (it's not a good idea
          * to output a warning on every row!)
          */
         if (bad_index)
            row_info->flags = PNG_BAD_INDEX;
         else
            row_info->flags = 0;
      }
   }
}

/* Like do_unpack except that the packed data is expanded to the full 8-bit
 * range; scaled up.  This is not a good thing to do on an indexed image;
 * the indices will be invalid.
 */
static void
png_do_expand_channels(png_transform_controlp row_info, png_bytep row)
{
   png_debug(1, "in png_do_expand_channels");

#  define png_ptr row_info->png_ptr
   debug(!(row_info->flags & PNG_BITS_SHIFTED));

   if (row_info->bit_depth < 8)
   {
      switch (row_info->bit_depth)
      {
         case 1:
         {
            png_const_bytep sp = row + png_transform_rowbytes(row_info)-1;
            unsigned int shift = 0x7 & -row_info->width;
            png_bytep dp;

            row_info->bit_depth = 8;
            dp = row + png_transform_rowbytes(row_info);

            while (dp > row)
            {
               *--dp = (png_byte)/*SAFE*/(((*sp >> shift) & 0x01) * 255);
               shift = 0x7 & (shift+1);
               if (shift == 0)
                  --sp;
            }

            debug(dp == row && shift == 0 && sp == row-1);
            break;
         }

         case 2:
         {
            png_const_bytep sp = row + png_transform_rowbytes(row_info)-1;
            unsigned int shift = 7 & -(row_info->width << 1);
            png_bytep dp;

            row_info->bit_depth = 8;
            dp = row + png_transform_rowbytes(row_info);

            while (dp > row)
            {
               *--dp = (png_byte)/*SAFE*/(((*sp >> shift) & 0x03) * 85);
               shift = 0x7 & (shift+2);
               if (shift == 0)
                  --sp;
            }

            debug(dp == row && shift == 0 && sp == row-1);
            break;
         }

         case 4:
         {
            png_const_bytep sp = row + png_transform_rowbytes(row_info)-1;
            unsigned int shift = 7 & -(row_info->width << 2);
            png_bytep dp;

            row_info->bit_depth = 8;
            dp = row + png_transform_rowbytes(row_info);

            while (dp > row)
            {
               *--dp = (png_byte)/*SAFE*/(((*sp >> shift) & 0x0f) * 17);
               shift = 0x7 & (shift+4);
               if (shift == 0)
                  --sp;
            }

            debug(dp == row && shift == 0 && sp == row-1);
            break;
         }

         default:
            break;
      }
   }
#  undef png_ptr
}

/* If the bit depth < 8, it is expanded to 8.  Also, if the already
 * expanded transparency value is supplied, an alpha channel is built.
 */
static void
png_do_expand(png_transform_controlp row_info, png_bytep row)
{
   png_const_structrp png_ptr = row_info->png_ptr;

   png_debug(1, "in png_do_expand");

   if (row_info->channels == 1 && !(row_info->flags & PNG_INDEXED))
   {
      /* Grayscale (1 channel), tRNS expansion requires that the data
       * be expanded to 8-bit pixels and the tRNS 'gray' value is expanded
       * to match.  ISO PNG 11.3.2.1 suggests that only the low order bits
       * are considered when the bit depth is less than 16.
       */
      if (png_ptr->num_trans != 0 &&
         (png_ptr->transformations & PNG_EXPAND_tRNS) != 0)
      {
         unsigned int gray = png_ptr->trans_color.gray;

         switch (row_info->bit_depth)
         {
            case 1:
               gray &= 0x1;
               gray |= gray << 1;
               /*FALL THROUGH*/
            case 2:
               gray &= 0x3;
               gray |= gray << 2;
               /*FALL THROUGH*/
            case 4:
               gray &= 0xf;
               gray |= gray << 4;
               png_do_expand_channels(row_info, row);
               affirm(row_info->bit_depth == 8);
               /*FALL THROUGH*/
            case 8:
               gray &= 0xff;
               {
                  png_const_bytep sp = row + png_transform_rowbytes(row_info);
                  png_bytep dp;

                  row_info->channels = 2;
                  dp = row + png_transform_rowbytes(row_info);

                  while (dp >= row+2)
                  {
                     const png_byte g = *--sp;

                     *--dp = (g == gray) ? 0 : 0xff;
                     *--dp = g;
                  }

                  debug(dp == row && sp == row);
               }
               break;

            case 16:
               {
                  png_const_bytep sp = row + png_transform_rowbytes(row_info);
                  png_bytep dp;

                  row_info->channels = 2;
                  dp = row + png_transform_rowbytes(row_info);

                  while (dp >= row+4)
                  {
                     dp -= 4;
                     sp -= 2;

                     {
                        const unsigned int g = (sp[0] << 8) | sp[1];

                        dp[2] = dp[3] = (g == gray) ? 0 : 0xff;
                        dp[1] = PNG_BYTE(g);
                        dp[0] = PNG_BYTE(g >> 8);
                     }
                  }

                  debug(dp == row && sp == row);
               }
               break;

            default:
               impossible("bit depth invalid");
         }
      }

      else if (row_info->bit_depth < 8) /* but no tRNS */
         png_do_expand_channels(row_info, row);
   }

   else if (row_info->channels == 3 &&
            png_ptr->num_trans != 0 &&
            (png_ptr->transformations & PNG_EXPAND_tRNS) != 0)
   {
      if (row_info->bit_depth == 8)
      {
         png_const_bytep sp = row + png_transform_rowbytes(row_info);
         png_bytep dp;
         png_uint_32 trans =
            ((((png_ptr->trans_color.blue & 0xff) << 8) |
              (png_ptr->trans_color.green & 0xff)     ) << 8) |
            (png_ptr->trans_color.red & 0xff);

         row_info->channels = 4;
         dp = row + png_transform_rowbytes(row_info);

         while (dp >= row+4)
         {
            png_uint_32 pixel = *--sp; /* B */
            pixel = (pixel << 8) | *--sp; /* G */
            pixel = (pixel << 8) | *--sp; /* R */

            *--dp = (pixel == trans) ? 0 : 0xff;
            *--dp = PNG_BYTE(pixel >> 16); /* B */
            *--dp = PNG_BYTE(pixel >>  8); /* G */
            *--dp = PNG_BYTE(pixel      ); /* R */
         }

         debug(dp == row && sp == row);
      }

      else if (row_info->bit_depth == 16)
      {
         /* The full 6 bytes of the input RRGGBB need to be compared against
          * the transparent color value.  Allow the compiler to choose how to
          * do this by using the standard library routines.
          */
         png_const_bytep sp = row + png_transform_rowbytes(row_info);
         png_bytep dp;
         png_byte trans[6];

         trans[0] = PNG_BYTE(png_ptr->trans_color.red >> 8);
         trans[1] = PNG_BYTE(png_ptr->trans_color.red);
         trans[2] = PNG_BYTE(png_ptr->trans_color.green >> 8);
         trans[3] = PNG_BYTE(png_ptr->trans_color.green);
         trans[4] = PNG_BYTE(png_ptr->trans_color.blue >> 8);
         trans[5] = PNG_BYTE(png_ptr->trans_color.blue);

         row_info->channels = 4;
         dp = row + png_transform_rowbytes(row_info);

         while (dp >= row+8)
         {
            png_byte alpha;
            dp -= 8;
            sp -= 6;

            alpha = memcmp(trans, sp, 6) ? 0xff : 0;
            memmove(dp, sp, 6);
            dp[7] = dp[6] = alpha;
         }

         debug(dp == row && sp == row);
      }
   }
}
#endif

#ifdef PNG_READ_EXPAND_16_SUPPORTED
/* If the bit depth is 8 and the color type is not a palette type expand the
 * whole row to 16 bits.  Has no effect otherwise.
 */
static void
png_do_expand_16(png_transform_controlp row_info, png_bytep row)
{
   png_debug(1, "in png_do_expand16");

#  define png_ptr row_info->png_ptr
   if (row_info->bit_depth == 8 && !(row_info->flags & PNG_INDEXED))
   {
      /* The rows have a sequence of bytes containing [0..255] and we need
       * to turn it into another row containing [0..65535], to do this we
       * calculate:
       *
       *  (input / 255) * 65535
       *
       *  Which happens to be exactly input * 257 and this can be achieved
       *  simply by byte replication in place (copying backwards).
       */
      png_const_bytep sp = row + png_transform_rowbytes(row_info);
      png_bytep dp;
      
      row_info->bit_depth = 16;
      dp = row + png_transform_rowbytes(row_info);

      while (dp > sp)
         dp[-2] = dp[-1] = *--sp, dp -= 2;

      debug(dp == row && sp == row);
   }
#  undef png_ptr
}
#endif

#ifdef PNG_READ_QUANTIZE_SUPPORTED
static void
png_do_quantize(png_transform_controlp row_info, png_bytep row)
{
   png_bytep sp, dp;
   png_uint_32 i;
   png_uint_32 row_width=row_info->width;

   png_debug(1, "in png_do_quantize");

   if (row_info->bit_depth == 8)
   {
      png_const_bytep palette_lookup = row_info->png_ptr->palette_lookup;

      if (row_info->channels == 3 && palette_lookup)
      {
         int r, g, b, p;
         sp = row;
         dp = row;
         for (i = 0; i < row_width; i++)
         {
            r = *sp++;
            g = *sp++;
            b = *sp++;

            /* This looks real messy, but the compiler will reduce
             * it down to a reasonable formula.  For example, with
             * 5 bits per color, we get:
             * p = (((r >> 3) & 0x1f) << 10) |
             *    (((g >> 3) & 0x1f) << 5) |
             *    ((b >> 3) & 0x1f);
             */
            p = (((r >> (8 - PNG_QUANTIZE_RED_BITS)) &
                ((1 << PNG_QUANTIZE_RED_BITS) - 1)) <<
                (PNG_QUANTIZE_GREEN_BITS + PNG_QUANTIZE_BLUE_BITS)) |
                (((g >> (8 - PNG_QUANTIZE_GREEN_BITS)) &
                ((1 << PNG_QUANTIZE_GREEN_BITS) - 1)) <<
                (PNG_QUANTIZE_BLUE_BITS)) |
                ((b >> (8 - PNG_QUANTIZE_BLUE_BITS)) &
                ((1 << PNG_QUANTIZE_BLUE_BITS) - 1));

            *dp++ = palette_lookup[p];
         }

         row_info->flags |= PNG_INDEXED;
         row_info->channels = 1;
      }

      else if (row_info->channels == 4 && palette_lookup != NULL)
      {
         int r, g, b, p;
         sp = row;
         dp = row;
         for (i = 0; i < row_width; i++)
         {
            r = *sp++;
            g = *sp++;
            b = *sp++;
            sp++;

            p = (((r >> (8 - PNG_QUANTIZE_RED_BITS)) &
                ((1 << PNG_QUANTIZE_RED_BITS) - 1)) <<
                (PNG_QUANTIZE_GREEN_BITS + PNG_QUANTIZE_BLUE_BITS)) |
                (((g >> (8 - PNG_QUANTIZE_GREEN_BITS)) &
                ((1 << PNG_QUANTIZE_GREEN_BITS) - 1)) <<
                (PNG_QUANTIZE_BLUE_BITS)) |
                ((b >> (8 - PNG_QUANTIZE_BLUE_BITS)) &
                ((1 << PNG_QUANTIZE_BLUE_BITS) - 1));

            *dp++ = palette_lookup[p];
         }

         row_info->flags |= PNG_INDEXED;
         row_info->channels = 1;
      }

      else if (row_info->channels == 1 && (row_info->flags & PNG_INDEXED) &&
         row_info->png_ptr->quantize_index != NULL)
      {
         png_const_bytep quantize_lookup = row_info->png_ptr->quantize_index;

         sp = row;

         for (i = 0; i < row_width; i++, sp++)
         {
            *sp = quantize_lookup[*sp];
         }
      }
   }
}
#endif /* READ_QUANTIZE */

/* Transform the row.  The order of transformations is significant,
 * and is very touchy.  If you add a transformation, take care to
 * decide how it fits in with the other transformations here.
 */
void /* PRIVATE */
png_do_read_transformations(png_structrp png_ptr, png_row_infop row_info_in)
{
   png_transform_control display;

   png_debug(1, "in png_do_read_transformations");

   affirm(png_ptr->row_buf != NULL);

   /* The following is debugging; prior to 1.5.4 the code was never compiled in;
    * in 1.5.4 PNG_FLAG_DETECT_UNINITIALIZED was added and the macro
    * PNG_WARN_UNINITIALIZED_ROW removed.  In 1.6 the new flag is set only for
    * all transformations, however in practice the ROW_INIT always gets done on
    * demand, if necessary.
    */
   if ((png_ptr->flags & PNG_FLAG_DETECT_UNINITIALIZED) != 0 &&
       (png_ptr->flags & PNG_FLAG_ROW_INIT) == 0)
   {
      /* Application has failed to call either png_read_start_image() or
       * png_read_update_info() after setting transforms that expand pixels.
       * This check added to libpng-1.2.19 (but not enabled until 1.5.4).
       */
      png_error(png_ptr, "missing png_read_start_image or update_info");
   }

   /* Ok, it looks genuine, set up the control structure from the supplied
    * row_info.
    */
   png_init_transform_control(png_ptr, &display, row_info_in);

#ifdef PNG_READ_EXPAND_SUPPORTED
   if ((png_ptr->transformations & PNG_EXPAND) != 0)
   {
      if (display.flags & PNG_INDEXED)
         png_do_expand_palette(&display, png_ptr->row_buf + 1);

      else
         png_do_expand(&display, png_ptr->row_buf + 1);
   }
#endif

#ifdef PNG_READ_STRIP_ALPHA_SUPPORTED
   if ((png_ptr->transformations & PNG_STRIP_ALPHA) != 0 &&
       (png_ptr->transformations & PNG_COMPOSE) == 0 &&
       (display.channels == 4 || display.channels == 2))
      png_do_strip_channel(&display, png_ptr->row_buf + 1,
         0 /* at_start == false, because SWAP_ALPHA happens later */);
#endif

#ifdef PNG_READ_RGB_TO_GRAY_SUPPORTED
   if ((png_ptr->transformations & PNG_RGB_TO_GRAY) != 0)
   {
      int rgb_error = png_do_rgb_to_gray(&display, png_ptr->row_buf + 1);

      if (rgb_error != 0)
      {
         png_ptr->rgb_to_gray_status=1;
         if ((png_ptr->transformations & PNG_RGB_TO_GRAY) ==
             PNG_RGB_TO_GRAY_WARN)
            png_warning(png_ptr, "png_do_rgb_to_gray found nongray pixel");

         if ((png_ptr->transformations & PNG_RGB_TO_GRAY) ==
             PNG_RGB_TO_GRAY_ERR)
            png_error(png_ptr, "png_do_rgb_to_gray found nongray pixel");
      }
   }
#endif

/* From Andreas Dilger e-mail to png-implement, 26 March 1998:
 *
 *   In most cases, the "simple transparency" should be done prior to doing
 *   gray-to-RGB, or you will have to test 3x as many bytes to check if a
 *   pixel is transparent.  You would also need to make sure that the
 *   transparency information is upgraded to RGB.
 *
 *   To summarize, the current flow is:
 *   - Gray + simple transparency -> compare 1 or 2 gray bytes and composite
 *                                   with background "in place" if transparent,
 *                                   convert to RGB if necessary
 *   - Gray + alpha -> composite with gray background and remove alpha bytes,
 *                                   convert to RGB if necessary
 *
 *   To support RGB backgrounds for gray images we need:
 *   - Gray + simple transparency -> convert to RGB + simple transparency,
 *                                   compare 3 or 6 bytes and composite with
 *                                   background "in place" if transparent
 *                                   (3x compare/pixel compared to doing
 *                                   composite with gray bkgrnd)
 *   - Gray + alpha -> convert to RGB + alpha, composite with background and
 *                                   remove alpha bytes (3x float
 *                                   operations/pixel compared with composite
 *                                   on gray background)
 *
 *  Greg's change will do this.  The reason it wasn't done before is for
 *  performance, as this increases the per-pixel operations.  If we would check
 *  in advance if the background was gray or RGB, and position the gray-to-RGB
 *  transform appropriately, then it would save a lot of work/time.
 */

#ifdef PNG_READ_GRAY_TO_RGB_SUPPORTED
   /* If gray -> RGB, do so now only if background is non-gray; else do later
    * for performance reasons
    */
   if ((png_ptr->transformations & PNG_GRAY_TO_RGB) != 0 &&
       (png_ptr->flags & PNG_FLAG_BACKGROUND_IS_GRAY) == 0)
      png_do_gray_to_rgb(&display, png_ptr->row_buf + 1);
#endif

#if defined(PNG_READ_BACKGROUND_SUPPORTED) || \
   defined(PNG_READ_ALPHA_MODE_SUPPORTED)
   if ((png_ptr->transformations & PNG_COMPOSE) != 0)
      png_do_compose(&display, png_ptr->row_buf + 1);
#endif

#ifdef PNG_READ_GAMMA_SUPPORTED
   if ((png_ptr->transformations & PNG_GAMMA) != 0 &&
#ifdef PNG_READ_RGB_TO_GRAY_SUPPORTED
      /* Because RGB_TO_GRAY does the gamma transform. */
      (png_ptr->transformations & PNG_RGB_TO_GRAY) == 0 &&
#endif
#if defined(PNG_READ_BACKGROUND_SUPPORTED) ||\
   defined(PNG_READ_ALPHA_MODE_SUPPORTED)
      /* Because PNG_COMPOSE does the gamma transform if there is something to
       * do (if there is an alpha channel or transparency.)
       * WARNING: prior to 1.7.0 this was checking png_ptr->color_type, which
       * probably means that the gamma would get dropped if the alpha
       * channel was stripped yet PNG_COMPOSE was also set.
       */
       !((png_ptr->transformations & PNG_COMPOSE) != 0 &&
       (png_ptr->num_trans != 0 ||
        display.channels == 2 || display.channels == 4)) &&
#endif
      /* Because png_init_read_transformations transforms the palette, unless
       * RGB_TO_GRAY will do the transform.  Note that this does need to check
       * the original color type because the expand_palette call preceeds this
       * check.
       */
       (png_ptr->color_type != PNG_COLOR_TYPE_PALETTE))
      png_do_gamma(&display, png_ptr->row_buf + 1);
#endif

#ifdef PNG_READ_STRIP_ALPHA_SUPPORTED
   if ((png_ptr->transformations & PNG_STRIP_ALPHA) != 0 &&
       (png_ptr->transformations & PNG_COMPOSE) != 0 &&
       (display.channels == 2 || display.channels == 4))
      png_do_strip_channel(&display, png_ptr->row_buf + 1,
          0 /* at_start == false, because SWAP_ALPHA happens later */);
#endif

#ifdef PNG_READ_ALPHA_MODE_SUPPORTED
   if ((png_ptr->transformations & PNG_ENCODE_ALPHA) != 0 &&
       (display.channels == 2 || display.channels == 4))
      png_do_encode_alpha(&display, png_ptr->row_buf + 1);
#endif

#ifdef PNG_READ_SCALE_16_TO_8_SUPPORTED
   if ((png_ptr->transformations & PNG_SCALE_16_TO_8) != 0)
      png_do_scale_16_to_8(&display, png_ptr->row_buf + 1);
#endif

#ifdef PNG_READ_STRIP_16_TO_8_SUPPORTED
   /* There is no harm in doing both of these because only one has any effect,
    * by putting the 'scale' option first if the app asks for scale (either by
    * calling the API or in a TRANSFORM flag) this is what happens.
    */
   if ((png_ptr->transformations & PNG_16_TO_8) != 0)
      png_do_chop(&display, png_ptr->row_buf + 1);
#endif

#ifdef PNG_READ_QUANTIZE_SUPPORTED
   if ((png_ptr->transformations & PNG_QUANTIZE) != 0)
      png_do_quantize(&display, png_ptr->row_buf + 1);
#endif /* READ_QUANTIZE */

#ifdef PNG_READ_EXPAND_16_SUPPORTED
   /* Do the expansion now, after all the arithmetic has been done.  Notice
    * that previous transformations can handle the PNG_EXPAND_16 flag if this
    * is efficient (particularly true in the case of gamma correction, where
    * better accuracy results faster!)
    */
   if ((png_ptr->transformations & PNG_EXPAND_16) != 0)
      png_do_expand_16(&display, png_ptr->row_buf + 1);
#endif

#ifdef PNG_READ_GRAY_TO_RGB_SUPPORTED
   /* NOTE: moved here in 1.5.4 (from much later in this list.) */
   if ((png_ptr->transformations & PNG_GRAY_TO_RGB) != 0 &&
       (png_ptr->flags & PNG_FLAG_BACKGROUND_IS_GRAY) != 0)
      png_do_gray_to_rgb(&display, png_ptr->row_buf + 1);
#endif

#ifdef PNG_READ_INVERT_SUPPORTED
   if ((png_ptr->transformations & PNG_INVERT_MONO) != 0)
      png_do_invert(&display, png_ptr->row_buf + 1);
#endif

#ifdef PNG_READ_INVERT_ALPHA_SUPPORTED
   if ((png_ptr->transformations & PNG_INVERT_ALPHA) != 0)
      png_do_read_invert_alpha(&display, png_ptr->row_buf + 1);
#endif

#ifdef PNG_READ_SHIFT_SUPPORTED
   if ((png_ptr->transformations & PNG_SHIFT) != 0)
      png_do_unshift(&display, png_ptr->row_buf + 1);
#endif

#ifdef PNG_READ_PACK_SUPPORTED
   if ((png_ptr->transformations & PNG_PACK) != 0)
      png_do_unpack(&display, png_ptr->row_buf + 1);
#endif

#ifdef PNG_READ_CHECK_FOR_INVALID_INDEX_SUPPORTED
   /* Added at libpng-1.5.10 */
   if ((display.flags & PNG_INDEXED) != 0 && png_ptr->num_palette_max >= 0)
      png_do_check_palette_indexes(png_ptr, &display);
#endif

#ifdef PNG_READ_BGR_SUPPORTED
   if ((png_ptr->transformations & PNG_BGR) != 0)
      png_do_bgr(&display, png_ptr->row_buf + 1);
#endif

#ifdef PNG_READ_PACKSWAP_SUPPORTED
   if ((png_ptr->transformations & PNG_PACKSWAP) != 0)
      png_do_packswap(&display, png_ptr->row_buf + 1);
#endif

#ifdef PNG_READ_FILLER_SUPPORTED
   if ((png_ptr->transformations & PNG_FILLER) != 0)
      png_do_read_filler(&display, png_ptr->row_buf + 1);
#endif

#ifdef PNG_READ_SWAP_ALPHA_SUPPORTED
   if ((png_ptr->transformations & PNG_SWAP_ALPHA) != 0)
      png_do_read_swap_alpha(&display, png_ptr->row_buf + 1);
#endif

#ifdef PNG_READ_16BIT_SUPPORTED
#ifdef PNG_READ_SWAP_SUPPORTED
   if ((png_ptr->transformations & PNG_SWAP_BYTES) != 0)
      png_do_swap(&display, png_ptr->row_buf + 1);
#endif
#endif

   /* The user transform expects a png_row_info, and it would be inconvenient
    * to change this.
    */
   png_end_transform_control(row_info_in, &display);

#ifdef PNG_READ_USER_TRANSFORM_SUPPORTED
   if ((png_ptr->transformations & PNG_USER_TRANSFORM) != 0)
   {
      if (png_ptr->read_user_transform_fn != NULL)
         (*(png_ptr->read_user_transform_fn)) /* User read transform function */
             (png_ptr,     /* png_ptr */
              row_info_in, /* row_info: */
                /*  png_uint_32 width;       width of row */
                /*  png_size_t rowbytes;     number of bytes in row */
                /*  png_byte color_type;     color type of pixels */
                /*  png_byte bit_depth;      bit depth of samples */
                /*  png_byte channels;       number of channels (1-4) */
                /*  png_byte pixel_depth;    bits per pixel (depth*channels) */
             png_ptr->row_buf + 1);    /* start of pixel data for row */

#ifdef PNG_USER_TRANSFORM_PTR_SUPPORTED
      if (png_ptr->user_transform_depth != 0)
         row_info_in->bit_depth = png_ptr->user_transform_depth;

      if (png_ptr->user_transform_channels != 0)
         row_info_in->channels = png_ptr->user_transform_channels;
#endif
      row_info_in->pixel_depth = png_check_byte(png_ptr,
         row_info_in->bit_depth * row_info_in->channels);

      row_info_in->rowbytes =
         PNG_ROWBYTES(row_info_in->pixel_depth, row_info_in->width);
   }
#endif
}
#endif /* READ_TRANSFORMS */
#endif /* READ */
