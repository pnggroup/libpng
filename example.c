/* example.c - an example of using libpng */

/* this is an example of how to use libpng to read and write
   png files.  The file libpng.txt is much more verbose then
   this.  If you have not read it, do so first.  This was
   designed to be a starting point of an implementation.
   This is not officially part of libpng, and therefore
   does not require a copyright notice.

   This file does not currently compile, because it is missing
   certain parts, like allocating memory to hold an image.
   You will have to supply these parts to get it to compile.
   */

#include <png.h>

/* check to see if a file is a png file using png_check_sig() */
int check_png(char *file_name)
{
   FILE *fp;
   char buf[8];
   int ret;

   fp = fopen(file_name, "rb");
   if (!fp)
      return 0;
   ret = fread(buf, 1, 8, fp);
   fclose(fp);

   if (ret != 8)
      return 0;

   ret = png_check_sig(buf, 8);

   return (ret);
}

/* read a png file.  You may want to return an error code if the read
   fails (depending upon the failure). */
void read_png(char *file_name)
{
   FILE *fp;
   png_struct *png_ptr;
   png_info *info_ptr;

   /* open the file */
   fp = fopen(file_name, "rb");
   if (!fp)
      return;

   /* allocate the necessary structures */
   png_ptr = malloc(sizeof (png_struct));
   if (!png_ptr)
   {
      fclose(fp);
      return;
   }

   info_ptr = malloc(sizeof (png_info));
   if (!info_ptr)
   {
      fclose(fp);
      free(png_ptr);
      return;
   }

   /* set error handling */
   if (setjmp(png_ptr->jmpbuf))
   {
      png_read_destroy(png_ptr, info_ptr, (png_info *)0);
      fclose(fp);
      free(png_ptr);
      free(info_ptr);
      /* If we get here, we had a problem reading the file */
      return;
   }

   /* initialize the structures, info first for error handling */
   png_info_init(info_ptr);
   png_read_init(png_ptr);

   /* set up the input control for the default input and message functions.
    * If we were to replace both the input and message functions we don't
    * need to call png_init_io first. */
   png_init_io(png_ptr, fp);

   /* if you are using replacement read functions, here you would call */
   io_ptr = (user_io_struct *)malloc(sizeof(user_io_struct));
   png_set_read_fn(png_ptr, (void *)io_ptr, user_read_fn);

   /* if you are using replacement message functions, here you would call */
   msg_ptr = (user_msg_struct *)malloc(sizeof(user_msg_struct));
   png_set_read_fn(png_ptr, (void *)msg_ptr, user_error_fn, user_warning_fn);

   /* read the file information */
   png_read_info(png_ptr, info_ptr);

   /* set up the transformations you want.  Note that these are
      all optional.  Only call them if you want them */

   /* expand paletted colors into true rgb */
   if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      png_set_expand(png_ptr);

   /* expand grayscale images to the full 8 bits */
   if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY &&
      info_ptr->bit_depth < 8)
      png_set_expand(png_ptr);

   /* expand images with transparency to full alpha channels */
   if (info_ptr->valid & PNG_INFO_tRNS)
      png_set_expand(png_ptr);

   /* Set the background color to draw transparent and alpha
      images over */
   png_color_16 my_background;

   if (info_ptr->valid & PNG_INFO_bKGD)
      png_set_background(png_ptr, &(info_ptr->background),
         PNG_GAMMA_FILE, 1, 1.0);
   else
      png_set_background(png_ptr, &my_background,
         PNG_GAMMA_SCREEN, 0, 1.0);

   /* tell libpng to handle the gamma conversion for you */
   if (info_ptr->valid & PNG_INFO_gAMA)
      png_set_gamma(png_ptr, screen_gamma, info_ptr->gamma);
   else
      png_set_gamma(png_ptr, screen_gamma, 0.45);

   /* tell libpng to strip 16 bit depth files down to 8 bits */
   if (info_ptr->bit_depth == 16)
      png_set_strip_16(png_ptr);

   /* dither rgb files down to 8 bit palettes & reduce palettes
      to the number of colors available on your screen */
   if (info_ptr->color_type & PNG_COLOR_MASK_COLOR)
   {
      if (info_ptr->valid & PNG_INFO_PLTE)
         png_set_dither(png_ptr, info_ptr->palette,
            info_ptr->num_palette, max_screen_colors,
               info_ptr->histogram);
      else
      {
         png_color std_color_cube[MAX_SCREEN_COLORS] =
            {/* ... colors ... */};

         png_set_dither(png_ptr, std_color_cube, MAX_SCREEN_COLORS,
            MAX_SCREEN_COLORS, NULL);
      }
   }

   /* invert monocrome files */
   if (info_ptr->bit_depth == 1 &&
      info_ptr->color_type == PNG_COLOR_GRAY)
      png_set_invert(png_ptr);

   /* shift the pixels down to their true bit depth */
   if (info_ptr->valid & PNG_INFO_sBIT)
      png_set_shift(png_ptr, &(info_ptr->sig_bit));

   /* pack pixels into bytes */
   if (info_ptr->bit_depth < 8)
      png_set_packing(png_ptr);

   /* flip the rgb pixels to bgr */
   if (info_ptr->color_type == PNG_COLOR_TYPE_RGB ||
      info_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
      png_set_bgr(png_ptr);

   /* swap bytes of 16 bit files to least significant bit first */
   if (info_ptr->bit_depth == 16)
      png_set_swap(png_ptr);

   /* add a filler byte to rgb files */
   if (info_ptr->bit_depth == 8 &&
      info_ptr->color_type == PNG_COLOR_TYPE_RGB)
      png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

   /* turn on interlace handling if you are not using png_read_image() */
   if (info_ptr->interlace_type)
      number_passes = png_set_interlace_handling(png_ptr);
   else
      number_passes = 1;

   /* optional call to update palette with transformations */
   png_start_read_image(png_ptr);

   /* optional call to update the info structure */
   png_read_update_info(png_ptr, info_ptr);

   /* allocate the memory to hold the image using the fields
      of png_info. */

   /* the easiest way to read the image */
   png_bytef *row_pointers[height];
   png_read_image(png_ptr, row_pointers);

   /* the other way to read images - deal with interlacing */

   for (pass = 0; pass < number_passes; pass++)
   {
      /* Read the image using the "sparkle" effect. */
      png_read_rows(png_ptr, row_pointers, NULL, number_of_rows);

      /* If you are only reading on row at a time, this works */
      for (y = 0; y < height; y++)
      {
         png_bytef *row_pointers = row[y];
         png_read_rows(png_ptr, &row_pointers, NULL, 1);
      }

      /* to get the rectangle effect, use the third parameter */
      png_read_rows(png_ptr, NULL, row_pointers, number_of_rows);

      /* if you want to display the image after every pass, do
         so here */
   }

   /* read the rest of the file, getting any additional chunks
      in info_ptr */
   png_read_end(png_ptr, info_ptr);

   /* if you had allocated any memory structures for custom input or
      messaging routines you need to free them before png_read_destroy */
   free(png_get_io_ptr(png_ptr));
   free(png_get_msg_ptr(png_ptr));

   /* clean up after the read, and free any memory allocated */
   png_read_destroy(png_ptr, info_ptr, (png_info *)0);

   /* free the structures */
   free(png_ptr);
   free(info_ptr);

   /* close the file */
   fclose(fp);

   /* that's it */
   return;
}

/* write a png file */
void write_png(char *file_name, ... other image information ...)
{
   FILE *fp;
   png_struct *png_ptr;
   png_info *info_ptr;

   /* open the file */
   fp = fopen(file_name, "wb");
   if (!fp)
      return;

   /* allocate the necessary structures */
   png_ptr = malloc(sizeof (png_struct));
   if (!png_ptr)
   {
      fclose(fp);
      return;
   }

   info_ptr = malloc(sizeof (png_info));
   if (!info_ptr)
   {
      fclose(fp);
      free(png_ptr);
      return;
   }

   /* set error handling */
   if (setjmp(png_ptr->jmpbuf))
   {
      png_write_destroy(png_ptr);
      fclose(fp);
      free(png_ptr);
      free(info_ptr);
      /* If we get here, we had a problem reading the file */
      return;
   }

   /* initialize the structures */
   png_info_init(info_ptr);
   png_write_init(png_ptr);

   /* set up the output control for the default output and message functions.
    * If we were to replace both the output and message functions we don't
    * need to call png_init_io first. */
   png_init_io(png_ptr, fp);

   /* if you are using replacement write functions, here you would call */
   io_ptr = (user_io_struct *)malloc(sizeof(user_io_struct));
   png_set_write_fn(png_ptr, (void *)io_ptr, user_write_fn, user_flush_fn);

   /* if you are using replacement message functions, here you would call */
   msg_ptr = (user_msg_struct *)malloc(sizeof(user_msg_struct));
   png_set_read_fn(png_ptr, (void *)msg_ptr, user_error_fn, user_warning_fn);

   /* set the file information here */
   info_ptr->width = ;
   info_ptr->height = ;
   etc.

   /* set the palette if there is one */
   info_ptr->valid |= PNG_INFO_PLTE;
   info_ptr->palette = malloc(256 * sizeof (png_color));
   info_ptr->num_palette = 256;
   ... set palette colors ...

   /* optional significant bit chunk */
   info_ptr->valid |= PNG_INFO_sBIT;
   /* if we are dealing with a grayscale image then */
   info_ptr->sig_bit.gray = true_bit_depth;
   /* otherwise, if we are dealing with a color image then */
   info_ptr->sig_bit.red = true_red_bit_depth;
   info_ptr->sig_bit.green = true_green_bit_depth;
   info_ptr->sig_bit.blue = true_blue_bit_depth;
   /* if the image has an alpha channel then */
   info_ptr->sig_bit.alpha = true_alpha_bit_depth;

   /* optional gamma chunk is a good idea if you can write one */
   info_ptr->valid |= PNG_INFO_gAMA;
   info_ptr->gamma = gamma;

   /* other optional chunks */

   /* write the file information */
   png_write_info(png_ptr, info_ptr);

   /* set up the transformations you want.  Note that these are
      all optional.  Only call them if you want them */

   /* invert monocrome pixels */
   png_set_invert(png_ptr);

   /* shift the pixels up to a legal bit depth and fill in
      as appropriate to correctly scale the image */
   png_set_shift(png_ptr, &(info_ptr->sig_bit));

   /* pack pixels into bytes */
   png_set_packing(png_ptr);

   /* flip bgr pixels to rgb */
   png_set_bgr(png_ptr);

   /* swap bytes of 16 bit files to most significant bit first */
   png_set_swap(png_ptr);

   /* get rid of filler bytes, pack rgb into 3 bytes.  The
      filler number is not used. */
   png_set_filler(png_ptr, 0, PNG_FILLER_BEFORE);

   /* turn on interlace handling if you are not using png_write_image() */
   if (interlacing)
      number_passes = png_set_interlace_handling(png_ptr);
   else
      number_passes = 1;

   /* the easiest way to write the image */
   png_bytef *row_pointers[height];
   png_write_image(png_ptr, row_pointers);

   /* the other way to write the image - deal with interlacing */

   for (pass = 0; pass < number_passes; pass++)
   {
      /* Write a few rows at a time. */
      png_write_rows(png_ptr, row_pointers, number_of_rows);

      /* If you are only writing one row at a time, this works */
      for (y = 0; y < height; y++)
      {
         png_bytef *row_pointers = row[y];
         png_write_rows(png_ptr, &row_pointers, 1);
      }
   }

   /* write the rest of the file */
   png_write_end(png_ptr, info_ptr);

   /* if you had allocated any memory structures for custom output or
      messaging routines you need to free them before png_write_destroy */
   free(png_get_io_ptr(png_ptr));
   free(png_get_msg_ptr(png_ptr));

   /* clean up after the write, and free any memory allocated */
   png_write_destroy(png_ptr);

   /* if you malloced the palette, free it here */
   if (info_ptr->palette)
      free(info_ptr->palette);

   /* free the structures */
   free(png_ptr);
   free(info_ptr);

   /* close the file */
   fclose(fp);

   /* that's it */
   return;
}

