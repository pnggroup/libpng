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
int check_png(char * file_name)
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
   png_structp png_ptr;
   png_infop info_ptr;

   /* open the file */
   fp = fopen(file_name, "rb");
   if (!fp)
      return;

   /* Create and initialize the png_struct with the desired error handler
      functions.  If you want to use the default stderr and longjump method,
      you can supply NULL for the last three parameters.  We also check that
      the header file is compatible with the library version.
    */
   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
      (void *)user_error_ptr, user_error_fn, user_warning_fn);

   if (!png_ptr)
   {
      fclose(fp);
      return;
   }

   info_ptr = png_create_info_struct();
   if (!info_ptr)
   {
      fclose(fp);
      png_destroy_read_struct(&png_ptr,  (png_infopp)NULL, (png_infopp)NULL);
      return;
   }

   /* set error handling if you are using the setjmp/longjmp method */
   if (setjmp(png_ptr->jmpbuf))
   {
      /* Free all of the memory associated with the png_ptr and info_ptr */
      png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
      fclose(fp);
      /* If we get here, we had a problem reading the file */
      return;
   }

   /* set up the input control if you are using standard C streams */
   png_init_io(png_ptr, fp);

   /* if you are using replacement read functions, instead of calling
      png_init_io() here you would call */
   png_set_read_fn(png_ptr, (void *)user_io_ptr, user_read_fn);
   /* where user_io_ptr is a structure you want available to the callbacks */

   /* read the file information */
   png_read_info(png_ptr, info_ptr);

   /* set up the transformations you want.  Note that these are
      all optional.  Only call them if you want them */

   /* expand paletted colors into true RGB triplets */
   if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      png_set_expand(png_ptr);

   /* expand grayscale images to the full 8 bits */
   if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY && info_ptr->bit_depth < 8)
      png_set_expand(png_ptr);

   /* expand paletted or RGB images with transparency to full alpha channels
    * so the data will be available as RGBA quartets */
   if (info_ptr->valid & PNG_INFO_tRNS)
      png_set_expand(png_ptr);

   /* Set the background color to draw transparent and alpha
      images over.  It is possible to set the red, green, and blue
      components directly for paletted images. */

   png_color_16 my_background;

   if (info_ptr->valid & PNG_INFO_bKGD)
      png_set_background(png_ptr, &(info_ptr->background),
                         PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
   else
      png_set_background(png_ptr, &my_background,
                         PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);

   /* tell libpng to handle the gamma conversion for you */
   if (info_ptr->valid & PNG_INFO_gAMA)
      png_set_gamma(png_ptr, screen_gamma, info_ptr->gamma);
   else
      png_set_gamma(png_ptr, screen_gamma, 0.45);

   /* tell libpng to strip 16 bit/color files down to 8 bits/color */
   if (info_ptr->bit_depth == 16)
      png_set_strip_16(png_ptr);

   /* dither rgb files down to 8 bit palette & reduce palettes
      to the number of colors available on your screen */
   if (info_ptr->color_type & PNG_COLOR_MASK_COLOR)
   {
      if (info_ptr->valid & PNG_INFO_PLTE)
         png_set_dither(png_ptr, info_ptr->palette, info_ptr->num_palette,
                        max_screen_colors, info_ptr->histogram);
      else
      {
         png_color std_color_cube[MAX_SCREEN_COLORS] =
            {/* ... colors ... */};

         png_set_dither(png_ptr, std_color_cube, MAX_SCREEN_COLORS,
            MAX_SCREEN_COLORS, NULL);
      }
   }

   /* invert monocrome files to have 0 as white and 1 as black */
   if (info_ptr->bit_depth == 1 && info_ptr->color_type == PNG_COLOR_GRAY)
      png_set_invert(png_ptr);

   /* shift the pixels down to their true bit depth */
   if (info_ptr->valid & PNG_INFO_sBIT &&
      info_ptr->bit_depth > info_ptr->sig_bit)
      png_set_shift(png_ptr, &(info_ptr->sig_bit));

   /* pack multiple pixels with bit depths of 1, 2, and 4 into bytes
      (useful only for paletted and grayscale images) */
   if (info_ptr->bit_depth < 8)
      png_set_packing(png_ptr);

   /* flip the rgb pixels to bgr */
   if (info_ptr->color_type == PNG_COLOR_TYPE_RGB ||
      info_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
      png_set_bgr(png_ptr);

   /* swap bytes of 16 bit files to least significant bit first */
   if (info_ptr->bit_depth == 16)
      png_set_swap(png_ptr);

   /* add a filler byte to RGB files (before or after each RGB triplet) */
   if (info_ptr->bit_depth == 8 && info_ptr->color_type == PNG_COLOR_TYPE_RGB)
      png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

   /* turn on interlace handling if you are not using png_read_image() */
   number_passes = png_set_interlace_handling(png_ptr);

   /* optional call to gamma correct and add the background to the palette
      and update info structure. */
   png_read_update_info(png_ptr, info_ptr);

   /* allocate the memory to hold the image using the fields
      of png_info. */

   /* the easiest way to read the image */
   png_bytep row_pointers[height];

   for (row = 0; row < height; row++)
   {
     row_pointers[row] = malloc(info_ptr->rowbytes);
   }

   png_read_image(png_ptr, row_pointers);

   /* the other way to read images - deal with interlacing */

   for (pass = 0; pass < number_passes; pass++)
   {
      /* Read the image using the "sparkle" effect. */
      png_read_rows(png_ptr, row_pointers, NULL, number_of_rows);

      /* If you are only reading on row at a time, this works */
      for (y = 0; y < height; y++)
      {
         png_bytep row_pointers = row[y];
         png_read_rows(png_ptr, &row_pointers, NULL, 1);
      }

      /* to get the rectangle effect, use the third parameter */
      png_read_rows(png_ptr, NULL, row_pointers, number_of_rows);

      /* if you want to display the image after every pass, do
         so here */
   }

   /* read the rest of the file, getting any additional chunks in info_ptr */
   png_read_end(png_ptr, info_ptr);

   /* clean up after the read, and free any memory allocated */
   png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

   /* close the file */
   fclose(fp);

   /* that's it */
   return;
}

/* progressively read a file */

int
initialize_png_reader(png_structp *png_ptr, png_infop *info_ptr)
{
   /* Create and initialize the png_struct with the desired error handler
      functions.  If you want to use the default stderr and longjump method,
      you can supply NULL for the last three parameters.  We also check that
      the library version is compatible in case we are using dynamically
      linked libraries.
    */
   *png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
       (void *)user_error_ptr, user_error_fn, user_warning_fn);

   if (! *png_ptr)
   {
      *info_ptr = NULL;
      return ERROR;
   }

   *info_ptr = png_create_info_struct(png_ptr);

   if (! *info_ptr)
   {
      png_destroy_read_struct(png_ptr, info_ptr, (png_infopp)NULL);
      return ERROR;
   }

   if (setjmp((*png_ptr)->jmpbuf))
   {
      png_destroy_read_struct(png_ptr, info_ptr, (png_infopp)NULL);
      return ERROR;
   }

   /* this one's new.  You will need to provide all three
      function callbacks, even if you aren't using them all.
      These functions shouldn't be dependent on global or
      static variables if you are decoding several images
      simultaneously.  You should store stream specific data
      in a separate struct, given as the second parameter,
      and retrieve the pointer from inside the callbacks using
      the function png_get_progressive_ptr(png_ptr). */
   png_set_progressive_read_fn(*png_ptr, (void *)stream_data,
      info_callback, row_callback, end_callback);

   return OK;
}

int
process_data(png_structp *png_ptr, png_infop *info_ptr,
   png_bytep buffer, png_uint_32 length)
{
   if (setjmp((*png_ptr)->jmpbuf))
   {
      /* Free the png_ptr and info_ptr memory on error */
      png_destroy_read_struct(png_ptr, info_ptr, (png_infopp)NULL);
      return ERROR;
   }

   /* this one's new also.  Simply give it chunks of data as
      they arrive from the data stream (in order, of course).
      On Segmented machines, don't give it any more than 64K.
      The library seems to run fine with sizes of 4K, although
      you can give it much less if necessary (I assume you can
      give it chunks of 1 byte, but I haven't tried with less
      than 256 bytes yet).  When this function returns, you may
      want to display any rows that were generated in the row
      callback, if you aren't already displaying them there. */
   png_process_data(*png_ptr, *info_ptr, buffer, length);
   return OK;
}

info_callback(png_structp png_ptr, png_infop info)
{
/* do any setup here, including setting any of the transformations
   mentioned in the Reading PNG files section.  For now, you _must_
   call either png_start_read_image() or png_read_update_info()
   after all the transformations are set (even if you don't set
   any).  You may start getting rows before png_process_data()
   returns, so this is your last chance to prepare for that. */
}

row_callback(png_structp png_ptr, png_bytep new_row,
   png_uint_32 row_num, int pass)
{
/* this function is called for every row in the image.  If the
   image is interlacing, and you turned on the interlace handler,
   this function will be called for every row in every pass.
   Some of these rows will not be changed from the previous pass.
   When the row is not changed, the new_row variable will be NULL.
   The rows and passes are called in order, so you don't really
   need the row_num and pass, but I'm supplying them because it
   may make your life easier.

   For the non-NULL rows of interlaced images, you must call
   png_progressive_combine_row() passing in the row and the
   old row.  You can call this function for NULL rows (it will
   just return) and for non-interlaced images (it just does the
   memcpy for you) if it will make the code easier.  Thus, you
   can just do this for all cases: */

   png_progressive_combine_row(png_ptr, old_row, new_row);

/* where old_row is what was displayed for previous rows.  Note
   that the first pass (pass == 0 really) will completely cover
   the old row, so the rows do not have to be initialized.  After
   the first pass (and only for interlaced images), you will have
   to pass the current row, and the function will combine the
   old row and the new row. */
}

end_callback(png_structp png_ptr, png_infop info)
{
/* this function is called when the whole image has been read,
   including any chunks after the image (up to and including
   the IEND).  You will usually have the same info chunk as you
   had in the header, although some data may have been added
   to the comments and time fields.

   Most people won't do much here, perhaps setting a flag that
   marks the image as finished. */
}

/* write a png file */
void write_png(char *file_name, ... other image information ...)
{
   FILE *fp;
   png_structp png_ptr;
   png_infop info_ptr;

   /* open the file */
   fp = fopen(file_name, "wb");
   if (!fp)
      return;

   /* Create and initialize the png_struct with the desired error handler
      functions.  If you want to use the default stderr and longjump method,
      you can supply NULL for the last three parameters.  We also check that
      the library version is compatible in case we are using dynamically
      linked libraries.
    */
   png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
      (void *)user_error_ptr, user_error_fn, user_warning_fn);

   if (!png_ptr)
   {
      fclose(fp);
      return;
   }

   info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr)
   {
      fclose(fp);
      png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
      return;
   }

   /* set error handling */
   if (setjmp(png_ptr->jmpbuf))
   {
      /* If we get here, we had a problem reading the file */
      fclose(fp);
      png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
      return;
   }

   /* set up the output control if you are using standard C streams */
   png_init_io(png_ptr, fp);

   /* if you are using replacement message functions, here you would call */
   png_set_message_fn(png_ptr, (void *)msg_ptr, user_error_fn, user_warning_fn);
   /* where msg_ptr is a structure you want available to the callbacks */

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
  
   /* optional gamma chunk is strongly suggested if you have any guess
      as to the correct gamma of the image */
   info_ptr->valid |= PNG_INFO_gAMA;
   info_ptr->gamma = gamma;

   /* other optional chunks like cHRM, bKGD, tRNS, tEXt, tIME, oFFs, pHYs, */

   /* write the file header information */
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

   /* the easiest way to write the image (you may choose to allocate the
      memory differently, however) */
   png_byte row_pointers[height][width];

   png_write_image(png_ptr, row_pointers);

   /* the other way to write the image - deal with interlacing */

   for (pass = 0; pass < number_passes; pass++)
   {
      /* Write a few rows at a time. */
      png_write_rows(png_ptr, row_pointers, number_of_rows);

      /* If you are only writing one row at a time, this works */
      for (y = 0; y < height; y++)
      {
         png_bytep row_pointers = row[y];
         png_write_rows(png_ptr, &row_pointers, 1);
      }
   }

   /* You can write optional chunks like tEXt, tIME at the end as well.
    * Note that if you wrote tEXt or zTXt chunks before the image, and
    * you aren't writing out more at the end, you have to set
    * info_ptr->num_text = 0 or they will be written out again.
    */

   /* write the rest of the file */
   png_write_end(png_ptr, info_ptr);

   /* if you malloced the palette, free it here */
   if (info_ptr->palette)
      free(info_ptr->palette);

   /* if you allocated any text comments, free them here */

   /* clean up after the write, and free any memory allocated */
   png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);

   /* close the file */
   fclose(fp);

   /* that's it */
   return;
}

