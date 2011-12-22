
/* pngread.c - read a PNG file
 *
 * Last changed in libpng 1.6.0 [(PENDING RELEASE)]
 * Copyright (c) 1998-2011 Glenn Randers-Pehrson
 * (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
 * (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * This file contains routines that an application calls directly to
 * read a PNG file or stream.
 */

#include "pngpriv.h"
#if defined PNG_SIMPLIFIED_READ_SUPPORTED && defined PNG_STDIO_SUPPORTED
#  include <errno.h>
#endif

#ifdef PNG_READ_SUPPORTED

/* Create a PNG structure for reading, and allocate any memory needed. */
PNG_FUNCTION(png_structp,PNGAPI
png_create_read_struct,(png_const_charp user_png_ver, png_voidp error_ptr,
    png_error_ptr error_fn, png_error_ptr warn_fn),PNG_ALLOCATED)
{
#ifndef PNG_USER_MEM_SUPPORTED
   png_structp png_ptr = png_create_png_struct(user_png_ver, error_ptr,
      error_fn, warn_fn, NULL, NULL, NULL);
#else
   return png_create_read_struct_2(user_png_ver, error_ptr, error_fn,
       warn_fn, NULL, NULL, NULL);
}

/* Alternate create PNG structure for reading, and allocate any memory
 * needed.
 */
PNG_FUNCTION(png_structp,PNGAPI
png_create_read_struct_2,(png_const_charp user_png_ver, png_voidp error_ptr,
    png_error_ptr error_fn, png_error_ptr warn_fn, png_voidp mem_ptr,
    png_malloc_ptr malloc_fn, png_free_ptr free_fn),PNG_ALLOCATED)
{
   png_structp png_ptr = png_create_png_struct(user_png_ver, error_ptr,
      error_fn, warn_fn, mem_ptr, malloc_fn, free_fn);
#endif /* PNG_USER_MEM_SUPPORTED */

   if (png_ptr != NULL)
   {
      int ok = 0;

      /* TODO: why does this happen here on read, but in png_write_IHDR on
       * write?
       */
      png_ptr->zstream.zalloc = png_zalloc;
      png_ptr->zstream.zfree = png_zfree;
      png_ptr->zstream.opaque = png_ptr;

      switch (inflateInit(&png_ptr->zstream))
      {
         case Z_OK:
            ok = 1;
            break;

         case Z_MEM_ERROR:
            png_warning(png_ptr, "zlib memory error");
            break;

         case Z_STREAM_ERROR:
            png_warning(png_ptr, "zlib stream error");
            break;

         case Z_VERSION_ERROR:
            png_warning(png_ptr, "zlib version error");
            break;

         default:
            png_warning(png_ptr, "Unknown zlib error");
            break;
      }

      if (ok)
      {
         png_ptr->zstream.next_out = png_ptr->zbuf;
         png_ptr->zstream.avail_out = png_ptr->zbuf_size;

         /* TODO: delay this, it can be done in png_init_io (if the app doesn't
          * do it itself) avoiding setting the default function if it is not
          * required.
          */
         png_set_read_fn(png_ptr, NULL, NULL);

         return png_ptr;
      }

      /* Else something went wrong in the zlib initialization above; it would
       * much simplify this code if the creation of the zlib stuff was to be
       * delayed until it is needed.
       */
      png_destroy_read_struct(&png_ptr, NULL, NULL);
   }

   return NULL;
}


#ifdef PNG_SEQUENTIAL_READ_SUPPORTED
/* Read the information before the actual image data.  This has been
 * changed in v0.90 to allow reading a file that already has the magic
 * bytes read from the stream.  You can tell libpng how many bytes have
 * been read from the beginning of the stream (up to the maximum of 8)
 * via png_set_sig_bytes(), and we will only check the remaining bytes
 * here.  The application can then have access to the signature bytes we
 * read if it is determined that this isn't a valid PNG file.
 */
void PNGAPI
png_read_info(png_structp png_ptr, png_infop info_ptr)
{
   png_debug(1, "in png_read_info");

   if (png_ptr == NULL || info_ptr == NULL)
      return;

   /* Read and check the PNG file signature. */
   png_read_sig(png_ptr, info_ptr);

   for (;;)
   {
      png_uint_32 length = png_read_chunk_header(png_ptr);
      png_uint_32 chunk_name = png_ptr->chunk_name;

      /* This should be a binary subdivision search or a hash for
       * matching the chunk name rather than a linear search.
       */
      if (chunk_name == png_IDAT)
         if (png_ptr->mode & PNG_AFTER_IDAT)
            png_ptr->mode |= PNG_HAVE_CHUNK_AFTER_IDAT;

      if (chunk_name == png_IHDR)
         png_handle_IHDR(png_ptr, info_ptr, length);

      else if (chunk_name == png_IEND)
         png_handle_IEND(png_ptr, info_ptr, length);

#ifdef PNG_HANDLE_AS_UNKNOWN_SUPPORTED
      else if (png_chunk_unknown_handling(png_ptr, chunk_name) !=
         PNG_HANDLE_CHUNK_AS_DEFAULT)
      {
         if (chunk_name == png_IDAT)
            png_ptr->mode |= PNG_HAVE_IDAT;

         png_handle_unknown(png_ptr, info_ptr, length);

         if (chunk_name == png_PLTE)
            png_ptr->mode |= PNG_HAVE_PLTE;

         else if (chunk_name == png_IDAT)
         {
            if (!(png_ptr->mode & PNG_HAVE_IHDR))
               png_error(png_ptr, "Missing IHDR before IDAT");

            else if (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE &&
                !(png_ptr->mode & PNG_HAVE_PLTE))
               png_error(png_ptr, "Missing PLTE before IDAT");

            break;
         }
      }
#endif
      else if (chunk_name == png_PLTE)
         png_handle_PLTE(png_ptr, info_ptr, length);

      else if (chunk_name == png_IDAT)
      {
         if (!(png_ptr->mode & PNG_HAVE_IHDR))
            png_error(png_ptr, "Missing IHDR before IDAT");

         else if (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE &&
             !(png_ptr->mode & PNG_HAVE_PLTE))
            png_error(png_ptr, "Missing PLTE before IDAT");

         png_ptr->idat_size = length;
         png_ptr->mode |= PNG_HAVE_IDAT;
         break;
      }

#ifdef PNG_READ_bKGD_SUPPORTED
      else if (chunk_name == png_bKGD)
         png_handle_bKGD(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_cHRM_SUPPORTED
      else if (chunk_name == png_cHRM)
         png_handle_cHRM(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_gAMA_SUPPORTED
      else if (chunk_name == png_gAMA)
         png_handle_gAMA(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_hIST_SUPPORTED
      else if (chunk_name == png_hIST)
         png_handle_hIST(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_oFFs_SUPPORTED
      else if (chunk_name == png_oFFs)
         png_handle_oFFs(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_pCAL_SUPPORTED
      else if (chunk_name == png_pCAL)
         png_handle_pCAL(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_sCAL_SUPPORTED
      else if (chunk_name == png_sCAL)
         png_handle_sCAL(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_pHYs_SUPPORTED
      else if (chunk_name == png_pHYs)
         png_handle_pHYs(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_sBIT_SUPPORTED
      else if (chunk_name == png_sBIT)
         png_handle_sBIT(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_sRGB_SUPPORTED
      else if (chunk_name == png_sRGB)
         png_handle_sRGB(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_iCCP_SUPPORTED
      else if (chunk_name == png_iCCP)
         png_handle_iCCP(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_sPLT_SUPPORTED
      else if (chunk_name == png_sPLT)
         png_handle_sPLT(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_tEXt_SUPPORTED
      else if (chunk_name == png_tEXt)
         png_handle_tEXt(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_tIME_SUPPORTED
      else if (chunk_name == png_tIME)
         png_handle_tIME(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_tRNS_SUPPORTED
      else if (chunk_name == png_tRNS)
         png_handle_tRNS(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_zTXt_SUPPORTED
      else if (chunk_name == png_zTXt)
         png_handle_zTXt(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_iTXt_SUPPORTED
      else if (chunk_name == png_iTXt)
         png_handle_iTXt(png_ptr, info_ptr, length);
#endif

      else
         png_handle_unknown(png_ptr, info_ptr, length);
   }
}
#endif /* PNG_SEQUENTIAL_READ_SUPPORTED */

/* Optional call to update the users info_ptr structure */
void PNGAPI
png_read_update_info(png_structp png_ptr, png_infop info_ptr)
{
   png_debug(1, "in png_read_update_info");

   if (png_ptr == NULL)
      return;

   png_read_start_row(png_ptr);

#ifdef PNG_READ_TRANSFORMS_SUPPORTED
   png_read_transform_info(png_ptr, info_ptr);
#else
   PNG_UNUSED(info_ptr)
#endif
}

#ifdef PNG_SEQUENTIAL_READ_SUPPORTED
/* Initialize palette, background, etc, after transformations
 * are set, but before any reading takes place.  This allows
 * the user to obtain a gamma-corrected palette, for example.
 * If the user doesn't call this, we will do it ourselves.
 */
void PNGAPI
png_start_read_image(png_structp png_ptr)
{
   png_debug(1, "in png_start_read_image");

   if (png_ptr != NULL)
     png_read_start_row(png_ptr);
}
#endif /* PNG_SEQUENTIAL_READ_SUPPORTED */

#ifdef PNG_SEQUENTIAL_READ_SUPPORTED
void PNGAPI
png_read_row(png_structp png_ptr, png_bytep row, png_bytep dsp_row)
{
   int ret;

   png_row_info row_info;

   if (png_ptr == NULL)
      return;

   png_debug2(1, "in png_read_row (row %lu, pass %d)",
       (unsigned long)png_ptr->row_number, png_ptr->pass);

   /* png_read_start_row sets the information (in particular iwidth) for this
    * interlace pass.
    */
   if (!(png_ptr->flags & PNG_FLAG_ROW_INIT))
      png_read_start_row(png_ptr);

   /* 1.5.6: row_info moved out of png_struct to a local here. */
   row_info.width = png_ptr->iwidth; /* NOTE: width of current interlaced row */
   row_info.color_type = png_ptr->color_type;
   row_info.bit_depth = png_ptr->bit_depth;
   row_info.channels = png_ptr->channels;
   row_info.pixel_depth = png_ptr->pixel_depth;
   row_info.rowbytes = PNG_ROWBYTES(row_info.pixel_depth, row_info.width);

   if (png_ptr->row_number == 0 && png_ptr->pass == 0)
   {
   /* Check for transforms that have been set but were defined out */
#if defined(PNG_WRITE_INVERT_SUPPORTED) && !defined(PNG_READ_INVERT_SUPPORTED)
   if (png_ptr->transformations & PNG_INVERT_MONO)
      png_warning(png_ptr, "PNG_READ_INVERT_SUPPORTED is not defined");
#endif

#if defined(PNG_WRITE_FILLER_SUPPORTED) && !defined(PNG_READ_FILLER_SUPPORTED)
   if (png_ptr->transformations & PNG_FILLER)
      png_warning(png_ptr, "PNG_READ_FILLER_SUPPORTED is not defined");
#endif

#if defined(PNG_WRITE_PACKSWAP_SUPPORTED) && \
    !defined(PNG_READ_PACKSWAP_SUPPORTED)
   if (png_ptr->transformations & PNG_PACKSWAP)
      png_warning(png_ptr, "PNG_READ_PACKSWAP_SUPPORTED is not defined");
#endif

#if defined(PNG_WRITE_PACK_SUPPORTED) && !defined(PNG_READ_PACK_SUPPORTED)
   if (png_ptr->transformations & PNG_PACK)
      png_warning(png_ptr, "PNG_READ_PACK_SUPPORTED is not defined");
#endif

#if defined(PNG_WRITE_SHIFT_SUPPORTED) && !defined(PNG_READ_SHIFT_SUPPORTED)
   if (png_ptr->transformations & PNG_SHIFT)
      png_warning(png_ptr, "PNG_READ_SHIFT_SUPPORTED is not defined");
#endif

#if defined(PNG_WRITE_BGR_SUPPORTED) && !defined(PNG_READ_BGR_SUPPORTED)
   if (png_ptr->transformations & PNG_BGR)
      png_warning(png_ptr, "PNG_READ_BGR_SUPPORTED is not defined");
#endif

#if defined(PNG_WRITE_SWAP_SUPPORTED) && !defined(PNG_READ_SWAP_SUPPORTED)
   if (png_ptr->transformations & PNG_SWAP_BYTES)
      png_warning(png_ptr, "PNG_READ_SWAP_SUPPORTED is not defined");
#endif
   }

#ifdef PNG_READ_INTERLACING_SUPPORTED
   /* If interlaced and we do not need a new row, combine row and return.
    * Notice that the pixels we have from previous rows have been transformed
    * already; we can only combine like with like (transformed or
    * untransformed) and, because of the libpng API for interlaced images, this
    * means we must transform before de-interlacing.
    */
   if (png_ptr->interlaced && (png_ptr->transformations & PNG_INTERLACE))
   {
      switch (png_ptr->pass)
      {
         case 0:
            if (png_ptr->row_number & 0x07)
            {
               if (dsp_row != NULL)
                  png_combine_row(png_ptr, dsp_row, 1/*display*/);
               png_read_finish_row(png_ptr);
               return;
            }
            break;

         case 1:
            if ((png_ptr->row_number & 0x07) || png_ptr->width < 5)
            {
               if (dsp_row != NULL)
                  png_combine_row(png_ptr, dsp_row, 1/*display*/);

               png_read_finish_row(png_ptr);
               return;
            }
            break;

         case 2:
            if ((png_ptr->row_number & 0x07) != 4)
            {
               if (dsp_row != NULL && (png_ptr->row_number & 4))
                  png_combine_row(png_ptr, dsp_row, 1/*display*/);

               png_read_finish_row(png_ptr);
               return;
            }
            break;

         case 3:
            if ((png_ptr->row_number & 3) || png_ptr->width < 3)
            {
               if (dsp_row != NULL)
                  png_combine_row(png_ptr, dsp_row, 1/*display*/);

               png_read_finish_row(png_ptr);
               return;
            }
            break;

         case 4:
            if ((png_ptr->row_number & 3) != 2)
            {
               if (dsp_row != NULL && (png_ptr->row_number & 2))
                  png_combine_row(png_ptr, dsp_row, 1/*display*/);

               png_read_finish_row(png_ptr);
               return;
            }
            break;
         case 5:
            if ((png_ptr->row_number & 1) || png_ptr->width < 2)
            {
               if (dsp_row != NULL)
                  png_combine_row(png_ptr, dsp_row, 1/*display*/);

               png_read_finish_row(png_ptr);
               return;
            }
            break;

         default:
         case 6:
            if (!(png_ptr->row_number & 1))
            {
               png_read_finish_row(png_ptr);
               return;
            }
            break;
      }
   }
#endif

   if (!(png_ptr->mode & PNG_HAVE_IDAT))
      png_error(png_ptr, "Invalid attempt to read row data");

   png_ptr->zstream.next_out = png_ptr->row_buf;
   png_ptr->zstream.avail_out =
       (uInt)(PNG_ROWBYTES(png_ptr->pixel_depth,
       png_ptr->iwidth) + 1);

   do
   {
      if (!(png_ptr->zstream.avail_in))
      {
         while (!png_ptr->idat_size)
         {
            png_crc_finish(png_ptr, 0);

            png_ptr->idat_size = png_read_chunk_header(png_ptr);
            if (png_ptr->chunk_name != png_IDAT)
               png_error(png_ptr, "Not enough image data");
         }
         png_ptr->zstream.avail_in = (uInt)png_ptr->zbuf_size;
         png_ptr->zstream.next_in = png_ptr->zbuf;
         if (png_ptr->zbuf_size > png_ptr->idat_size)
            png_ptr->zstream.avail_in = (uInt)png_ptr->idat_size;
         png_crc_read(png_ptr, png_ptr->zbuf,
             (png_size_t)png_ptr->zstream.avail_in);
         png_ptr->idat_size -= png_ptr->zstream.avail_in;
      }

      ret = inflate(&png_ptr->zstream, Z_PARTIAL_FLUSH);

      if (ret == Z_STREAM_END)
      {
         if (png_ptr->zstream.avail_out || png_ptr->zstream.avail_in ||
            png_ptr->idat_size)
            png_benign_error(png_ptr, "Extra compressed data");
         png_ptr->mode |= PNG_AFTER_IDAT;
         png_ptr->flags |= PNG_FLAG_ZLIB_FINISHED;
         break;
      }

      if (ret != Z_OK)
         png_error(png_ptr, png_ptr->zstream.msg ? png_ptr->zstream.msg :
             "Decompression error");

   } while (png_ptr->zstream.avail_out);

   if (png_ptr->row_buf[0] > PNG_FILTER_VALUE_NONE)
   {
      if (png_ptr->row_buf[0] < PNG_FILTER_VALUE_LAST)
         png_read_filter_row(png_ptr, &row_info, png_ptr->row_buf + 1,
            png_ptr->prev_row + 1, png_ptr->row_buf[0]);
      else
         png_error(png_ptr, "bad adaptive filter value");
   }

   /* libpng 1.5.6: the following line was copying png_ptr->rowbytes before
    * 1.5.6, while the buffer really is this big in current versions of libpng
    * it may not be in the future, so this was changed just to copy the
    * interlaced count:
    */
   png_memcpy(png_ptr->prev_row, png_ptr->row_buf, row_info.rowbytes + 1);

#ifdef PNG_MNG_FEATURES_SUPPORTED
   if ((png_ptr->mng_features_permitted & PNG_FLAG_MNG_FILTER_64) &&
       (png_ptr->filter_type == PNG_INTRAPIXEL_DIFFERENCING))
   {
      /* Intrapixel differencing */
      png_do_read_intrapixel(&row_info, png_ptr->row_buf + 1);
   }
#endif


#ifdef PNG_READ_TRANSFORMS_SUPPORTED
   if (png_ptr->transformations)
      png_do_read_transformations(png_ptr, &row_info);
#endif

   /* The transformed pixel depth should match the depth now in row_info. */
   if (png_ptr->transformed_pixel_depth == 0)
   {
      png_ptr->transformed_pixel_depth = row_info.pixel_depth;
      if (row_info.pixel_depth > png_ptr->maximum_pixel_depth)
         png_error(png_ptr, "sequential row overflow");
   }

   else if (png_ptr->transformed_pixel_depth != row_info.pixel_depth)
      png_error(png_ptr, "internal sequential row size calculation error");

#ifdef PNG_READ_INTERLACING_SUPPORTED
   /* Blow up interlaced rows to full size */
   if (png_ptr->interlaced &&
      (png_ptr->transformations & PNG_INTERLACE))
   {
      if (png_ptr->pass < 6)
         png_do_read_interlace(&row_info, png_ptr->row_buf + 1, png_ptr->pass,
            png_ptr->transformations);

      if (dsp_row != NULL)
         png_combine_row(png_ptr, dsp_row, 1/*display*/);

      if (row != NULL)
         png_combine_row(png_ptr, row, 0/*row*/);
   }

   else
#endif
   {
      if (row != NULL)
         png_combine_row(png_ptr, row, -1/*ignored*/);

      if (dsp_row != NULL)
         png_combine_row(png_ptr, dsp_row, -1/*ignored*/);
   }
   png_read_finish_row(png_ptr);

   if (png_ptr->read_row_fn != NULL)
      (*(png_ptr->read_row_fn))(png_ptr, png_ptr->row_number, png_ptr->pass);
}
#endif /* PNG_SEQUENTIAL_READ_SUPPORTED */

#ifdef PNG_SEQUENTIAL_READ_SUPPORTED
/* Read one or more rows of image data.  If the image is interlaced,
 * and png_set_interlace_handling() has been called, the rows need to
 * contain the contents of the rows from the previous pass.  If the
 * image has alpha or transparency, and png_handle_alpha()[*] has been
 * called, the rows contents must be initialized to the contents of the
 * screen.
 *
 * "row" holds the actual image, and pixels are placed in it
 * as they arrive.  If the image is displayed after each pass, it will
 * appear to "sparkle" in.  "display_row" can be used to display a
 * "chunky" progressive image, with finer detail added as it becomes
 * available.  If you do not want this "chunky" display, you may pass
 * NULL for display_row.  If you do not want the sparkle display, and
 * you have not called png_handle_alpha(), you may pass NULL for rows.
 * If you have called png_handle_alpha(), and the image has either an
 * alpha channel or a transparency chunk, you must provide a buffer for
 * rows.  In this case, you do not have to provide a display_row buffer
 * also, but you may.  If the image is not interlaced, or if you have
 * not called png_set_interlace_handling(), the display_row buffer will
 * be ignored, so pass NULL to it.
 *
 * [*] png_handle_alpha() does not exist yet, as of this version of libpng
 */

void PNGAPI
png_read_rows(png_structp png_ptr, png_bytepp row,
    png_bytepp display_row, png_uint_32 num_rows)
{
   png_uint_32 i;
   png_bytepp rp;
   png_bytepp dp;

   png_debug(1, "in png_read_rows");

   if (png_ptr == NULL)
      return;

   rp = row;
   dp = display_row;
   if (rp != NULL && dp != NULL)
      for (i = 0; i < num_rows; i++)
      {
         png_bytep rptr = *rp++;
         png_bytep dptr = *dp++;

         png_read_row(png_ptr, rptr, dptr);
      }

   else if (rp != NULL)
      for (i = 0; i < num_rows; i++)
      {
         png_bytep rptr = *rp;
         png_read_row(png_ptr, rptr, NULL);
         rp++;
      }

   else if (dp != NULL)
      for (i = 0; i < num_rows; i++)
      {
         png_bytep dptr = *dp;
         png_read_row(png_ptr, NULL, dptr);
         dp++;
      }
}
#endif /* PNG_SEQUENTIAL_READ_SUPPORTED */

#ifdef PNG_SEQUENTIAL_READ_SUPPORTED
/* Read the entire image.  If the image has an alpha channel or a tRNS
 * chunk, and you have called png_handle_alpha()[*], you will need to
 * initialize the image to the current image that PNG will be overlaying.
 * We set the num_rows again here, in case it was incorrectly set in
 * png_read_start_row() by a call to png_read_update_info() or
 * png_start_read_image() if png_set_interlace_handling() wasn't called
 * prior to either of these functions like it should have been.  You can
 * only call this function once.  If you desire to have an image for
 * each pass of a interlaced image, use png_read_rows() instead.
 *
 * [*] png_handle_alpha() does not exist yet, as of this version of libpng
 */
void PNGAPI
png_read_image(png_structp png_ptr, png_bytepp image)
{
   png_uint_32 i, image_height;
   int pass, j;
   png_bytepp rp;

   png_debug(1, "in png_read_image");

   if (png_ptr == NULL)
      return;

#ifdef PNG_READ_INTERLACING_SUPPORTED
   if (!(png_ptr->flags & PNG_FLAG_ROW_INIT))
   {
      pass = png_set_interlace_handling(png_ptr);
      /* And make sure transforms are initialized. */
      png_start_read_image(png_ptr);
   }
   else
   {
      if (png_ptr->interlaced && !(png_ptr->transformations & PNG_INTERLACE))
      {
         /* Caller called png_start_read_image or png_read_update_info without
          * first turning on the PNG_INTERLACE transform.  We can fix this here,
          * but the caller should do it!
          */
         png_warning(png_ptr, "Interlace handling should be turned on when "
            "using png_read_image");
         /* Make sure this is set correctly */
         png_ptr->num_rows = png_ptr->height;
      }

      /* Obtain the pass number, which also turns on the PNG_INTERLACE flag in
       * the above error case.
       */
      pass = png_set_interlace_handling(png_ptr);
   }
#else
   if (png_ptr->interlaced)
      png_error(png_ptr,
          "Cannot read interlaced image -- interlace handler disabled");

   pass = 1;
#endif

   image_height=png_ptr->height;

   for (j = 0; j < pass; j++)
   {
      rp = image;
      for (i = 0; i < image_height; i++)
      {
         png_read_row(png_ptr, *rp, NULL);
         rp++;
      }
   }
}
#endif /* PNG_SEQUENTIAL_READ_SUPPORTED */

#ifdef PNG_SEQUENTIAL_READ_SUPPORTED
/* Read the end of the PNG file.  Will not read past the end of the
 * file, will verify the end is accurate, and will read any comments
 * or time information at the end of the file, if info is not NULL.
 */
void PNGAPI
png_read_end(png_structp png_ptr, png_infop info_ptr)
{
   png_debug(1, "in png_read_end");

   if (png_ptr == NULL)
      return;

   png_crc_finish(png_ptr, 0); /* Finish off CRC from last IDAT chunk */

   do
   {
      png_uint_32 length = png_read_chunk_header(png_ptr);
      png_uint_32 chunk_name = png_ptr->chunk_name;

      if (chunk_name == png_IHDR)
         png_handle_IHDR(png_ptr, info_ptr, length);

      else if (chunk_name == png_IEND)
         png_handle_IEND(png_ptr, info_ptr, length);

#ifdef PNG_HANDLE_AS_UNKNOWN_SUPPORTED
      else if (png_chunk_unknown_handling(png_ptr, chunk_name) !=
         PNG_HANDLE_CHUNK_AS_DEFAULT)
      {
         if (chunk_name == png_IDAT)
         {
            if ((length > 0) || (png_ptr->mode & PNG_HAVE_CHUNK_AFTER_IDAT))
               png_benign_error(png_ptr, "Too many IDATs found");
         }
         png_handle_unknown(png_ptr, info_ptr, length);
         if (chunk_name == png_PLTE)
            png_ptr->mode |= PNG_HAVE_PLTE;
      }
#endif

      else if (chunk_name == png_IDAT)
      {
         /* Zero length IDATs are legal after the last IDAT has been
          * read, but not after other chunks have been read.
          */
         if ((length > 0) || (png_ptr->mode & PNG_HAVE_CHUNK_AFTER_IDAT))
            png_benign_error(png_ptr, "Too many IDATs found");

         png_crc_finish(png_ptr, length);
      }
      else if (chunk_name == png_PLTE)
         png_handle_PLTE(png_ptr, info_ptr, length);

#ifdef PNG_READ_bKGD_SUPPORTED
      else if (chunk_name == png_bKGD)
         png_handle_bKGD(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_cHRM_SUPPORTED
      else if (chunk_name == png_cHRM)
         png_handle_cHRM(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_gAMA_SUPPORTED
      else if (chunk_name == png_gAMA)
         png_handle_gAMA(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_hIST_SUPPORTED
      else if (chunk_name == png_hIST)
         png_handle_hIST(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_oFFs_SUPPORTED
      else if (chunk_name == png_oFFs)
         png_handle_oFFs(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_pCAL_SUPPORTED
      else if (chunk_name == png_pCAL)
         png_handle_pCAL(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_sCAL_SUPPORTED
      else if (chunk_name == png_sCAL)
         png_handle_sCAL(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_pHYs_SUPPORTED
      else if (chunk_name == png_pHYs)
         png_handle_pHYs(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_sBIT_SUPPORTED
      else if (chunk_name == png_sBIT)
         png_handle_sBIT(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_sRGB_SUPPORTED
      else if (chunk_name == png_sRGB)
         png_handle_sRGB(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_iCCP_SUPPORTED
      else if (chunk_name == png_iCCP)
         png_handle_iCCP(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_sPLT_SUPPORTED
      else if (chunk_name == png_sPLT)
         png_handle_sPLT(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_tEXt_SUPPORTED
      else if (chunk_name == png_tEXt)
         png_handle_tEXt(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_tIME_SUPPORTED
      else if (chunk_name == png_tIME)
         png_handle_tIME(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_tRNS_SUPPORTED
      else if (chunk_name == png_tRNS)
         png_handle_tRNS(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_zTXt_SUPPORTED
      else if (chunk_name == png_zTXt)
         png_handle_zTXt(png_ptr, info_ptr, length);
#endif

#ifdef PNG_READ_iTXt_SUPPORTED
      else if (chunk_name == png_iTXt)
         png_handle_iTXt(png_ptr, info_ptr, length);
#endif

      else
         png_handle_unknown(png_ptr, info_ptr, length);
   } while (!(png_ptr->mode & PNG_HAVE_IEND));
}
#endif /* PNG_SEQUENTIAL_READ_SUPPORTED */

/* Free all memory used in the read struct */
static void
png_read_destroy(png_structp png_ptr)
{
   png_debug(1, "in png_read_destroy");

#ifdef PNG_READ_GAMMA_SUPPORTED
   png_destroy_gamma_table(png_ptr);
#endif

   png_free(png_ptr, png_ptr->zbuf);
   png_free(png_ptr, png_ptr->big_row_buf);
   png_free(png_ptr, png_ptr->big_prev_row);
   png_free(png_ptr, png_ptr->chunkdata);

#ifdef PNG_READ_QUANTIZE_SUPPORTED
   png_free(png_ptr, png_ptr->palette_lookup);
   png_free(png_ptr, png_ptr->quantize_index);
#endif

   if (png_ptr->free_me & PNG_FREE_PLTE)
      png_zfree(png_ptr, png_ptr->palette);
   png_ptr->free_me &= ~PNG_FREE_PLTE;

#if defined(PNG_tRNS_SUPPORTED) || \
    defined(PNG_READ_EXPAND_SUPPORTED) || defined(PNG_READ_BACKGROUND_SUPPORTED)
   if (png_ptr->free_me & PNG_FREE_TRNS)
      png_free(png_ptr, png_ptr->trans_alpha);
   png_ptr->free_me &= ~PNG_FREE_TRNS;
#endif

#ifdef PNG_READ_hIST_SUPPORTED
   if (png_ptr->free_me & PNG_FREE_HIST)
      png_free(png_ptr, png_ptr->hist);
   png_ptr->free_me &= ~PNG_FREE_HIST;
#endif

   inflateEnd(&png_ptr->zstream);

#ifdef PNG_PROGRESSIVE_READ_SUPPORTED
   png_free(png_ptr, png_ptr->save_buffer);
#endif

#ifdef PNG_PROGRESSIVE_READ_SUPPORTED
#ifdef PNG_TEXT_SUPPORTED
   png_free(png_ptr, png_ptr->current_text);
#endif /* PNG_TEXT_SUPPORTED */
#endif /* PNG_PROGRESSIVE_READ_SUPPORTED */

   /* NOTE: the 'setjmp' buffer may still be allocated and the memory and error
    * callbacks are still set at this point.  They are required to complete the
    * destruction of the png_struct itself.
    */
}

/* Free all memory used by the read */
void PNGAPI
png_destroy_read_struct(png_structpp png_ptr_ptr, png_infopp info_ptr_ptr,
    png_infopp end_info_ptr_ptr)
{
   png_structp png_ptr = NULL;

   png_debug(1, "in png_destroy_read_struct");

   if (png_ptr_ptr != NULL)
      png_ptr = *png_ptr_ptr;

   if (png_ptr == NULL)
      return;

   /* libpng 1.6.0: use the API to destroy info structs to ensure consistent
    * behavior.  Prior to 1.6.0 libpng did extra 'info' destruction in this API.
    * The extra was, apparently, unnecessary yet this hides memory leak bugs.
    */
   png_destroy_info_struct(png_ptr, end_info_ptr_ptr);
   png_destroy_info_struct(png_ptr, info_ptr_ptr);

   *png_ptr_ptr = NULL;
   png_read_destroy(png_ptr);
   png_destroy_png_struct(png_ptr);
}

void PNGAPI
png_set_read_status_fn(png_structp png_ptr, png_read_status_ptr read_row_fn)
{
   if (png_ptr == NULL)
      return;

   png_ptr->read_row_fn = read_row_fn;
}


#ifdef PNG_SEQUENTIAL_READ_SUPPORTED
#ifdef PNG_INFO_IMAGE_SUPPORTED
void PNGAPI
png_read_png(png_structp png_ptr, png_infop info_ptr,
                           int transforms,
                           voidp params)
{
   int row;

   if (png_ptr == NULL || info_ptr == NULL)
      return;

   /* png_read_info() gives us all of the information from the
    * PNG file before the first IDAT (image data chunk).
    */
   png_read_info(png_ptr, info_ptr);
   if (info_ptr->height > PNG_UINT_32_MAX/png_sizeof(png_bytep))
      png_error(png_ptr, "Image is too high to process with png_read_png()");

   /* -------------- image transformations start here ------------------- */

#ifdef PNG_READ_SCALE_16_TO_8_SUPPORTED
   /* Tell libpng to strip 16-bit/color files down to 8 bits per color.
    */
   if (transforms & PNG_TRANSFORM_SCALE_16)
   {
     /* Added at libpng-1.5.4. "strip_16" produces the same result that it
      * did in earlier versions, while "scale_16" is now more accurate.
      */
      png_set_scale_16(png_ptr);
   }
#endif

#ifdef PNG_READ_STRIP_16_TO_8_SUPPORTED
   /* If both SCALE and STRIP are required pngrtran will effectively cancel the
    * latter by doing SCALE first.  This is ok and allows apps not to check for
    * which is supported to get the right answer.
    */
   if (transforms & PNG_TRANSFORM_STRIP_16)
      png_set_strip_16(png_ptr);
#endif

#ifdef PNG_READ_STRIP_ALPHA_SUPPORTED
   /* Strip alpha bytes from the input data without combining with
    * the background (not recommended).
    */
   if (transforms & PNG_TRANSFORM_STRIP_ALPHA)
      png_set_strip_alpha(png_ptr);
#endif

#if defined(PNG_READ_PACK_SUPPORTED) && !defined(PNG_READ_EXPAND_SUPPORTED)
   /* Extract multiple pixels with bit depths of 1, 2, or 4 from a single
    * byte into separate bytes (useful for paletted and grayscale images).
    */
   if (transforms & PNG_TRANSFORM_PACKING)
      png_set_packing(png_ptr);
#endif

#ifdef PNG_READ_PACKSWAP_SUPPORTED
   /* Change the order of packed pixels to least significant bit first
    * (not useful if you are using png_set_packing).
    */
   if (transforms & PNG_TRANSFORM_PACKSWAP)
      png_set_packswap(png_ptr);
#endif

#ifdef PNG_READ_EXPAND_SUPPORTED
   /* Expand paletted colors into true RGB triplets
    * Expand grayscale images to full 8 bits from 1, 2, or 4 bits/pixel
    * Expand paletted or RGB images with transparency to full alpha
    * channels so the data will be available as RGBA quartets.
    */
   if (transforms & PNG_TRANSFORM_EXPAND)
      if ((png_ptr->bit_depth < 8) ||
          (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE) ||
          (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)))
         png_set_expand(png_ptr);
#endif

   /* We don't handle background color or gamma transformation or quantizing.
    */

#ifdef PNG_READ_INVERT_SUPPORTED
   /* Invert monochrome files to have 0 as white and 1 as black
    */
   if (transforms & PNG_TRANSFORM_INVERT_MONO)
      png_set_invert_mono(png_ptr);
#endif

#ifdef PNG_READ_SHIFT_SUPPORTED
   /* If you want to shift the pixel values from the range [0,255] or
    * [0,65535] to the original [0,7] or [0,31], or whatever range the
    * colors were originally in:
    */
   if ((transforms & PNG_TRANSFORM_SHIFT)
       && png_get_valid(png_ptr, info_ptr, PNG_INFO_sBIT))
   {
      png_color_8p sig_bit;

      png_get_sBIT(png_ptr, info_ptr, &sig_bit);
      png_set_shift(png_ptr, sig_bit);
   }
#endif

#ifdef PNG_READ_BGR_SUPPORTED
   /* Flip the RGB pixels to BGR (or RGBA to BGRA) */
   if (transforms & PNG_TRANSFORM_BGR)
      png_set_bgr(png_ptr);
#endif

#ifdef PNG_READ_SWAP_ALPHA_SUPPORTED
   /* Swap the RGBA or GA data to ARGB or AG (or BGRA to ABGR) */
   if (transforms & PNG_TRANSFORM_SWAP_ALPHA)
      png_set_swap_alpha(png_ptr);
#endif

#ifdef PNG_READ_SWAP_SUPPORTED
   /* Swap bytes of 16-bit files to least significant byte first */
   if (transforms & PNG_TRANSFORM_SWAP_ENDIAN)
      png_set_swap(png_ptr);
#endif

/* Added at libpng-1.2.41 */
#ifdef PNG_READ_INVERT_ALPHA_SUPPORTED
   /* Invert the alpha channel from opacity to transparency */
   if (transforms & PNG_TRANSFORM_INVERT_ALPHA)
      png_set_invert_alpha(png_ptr);
#endif

/* Added at libpng-1.2.41 */
#ifdef PNG_READ_GRAY_TO_RGB_SUPPORTED
   /* Expand grayscale image to RGB */
   if (transforms & PNG_TRANSFORM_GRAY_TO_RGB)
      png_set_gray_to_rgb(png_ptr);
#endif

/* Added at libpng-1.5.4 */
#ifdef PNG_READ_EXPAND_16_SUPPORTED
   if (transforms & PNG_TRANSFORM_EXPAND_16)
      png_set_expand_16(png_ptr);
#endif

   /* We don't handle adding filler bytes */

   /* We use png_read_image and rely on that for interlace handling, but we also
    * call png_read_update_info therefore must turn on interlace handling now:
    */
   (void)png_set_interlace_handling(png_ptr);

   /* Optional call to gamma correct and add the background to the palette
    * and update info structure.  REQUIRED if you are expecting libpng to
    * update the palette for you (i.e., you selected such a transform above).
    */
   png_read_update_info(png_ptr, info_ptr);

   /* -------------- image transformations end here ------------------- */

   png_free_data(png_ptr, info_ptr, PNG_FREE_ROWS, 0);
   if (info_ptr->row_pointers == NULL)
   {
      png_uint_32 iptr;

      info_ptr->row_pointers = (png_bytepp)png_malloc(png_ptr,
          info_ptr->height * png_sizeof(png_bytep));
      for (iptr=0; iptr<info_ptr->height; iptr++)
         info_ptr->row_pointers[iptr] = NULL;

      info_ptr->free_me |= PNG_FREE_ROWS;

      for (row = 0; row < (int)info_ptr->height; row++)
         info_ptr->row_pointers[row] = (png_bytep)png_malloc(png_ptr,
            png_get_rowbytes(png_ptr, info_ptr));
   }

   png_read_image(png_ptr, info_ptr->row_pointers);
   info_ptr->valid |= PNG_INFO_IDAT;

   /* Read rest of file, and get additional chunks in info_ptr - REQUIRED */
   png_read_end(png_ptr, info_ptr);

   PNG_UNUSED(transforms)   /* Quiet compiler warnings */
   PNG_UNUSED(params)

}
#endif /* PNG_INFO_IMAGE_SUPPORTED */
#endif /* PNG_SEQUENTIAL_READ_SUPPORTED */

#ifdef PNG_SIMPLIFIED_READ_SUPPORTED
/* SIMPLIFIED READ
 *
 * This code currently relies on the sequential reader, though it could easily
 * be made to work with the progressive one.
 */
/* Do all the *safe* initialization - 'safe' means that png_error won't be
 * called, so setting up the jmp_buf is not required.  This means that anything
 * called from here must *not* call png_malloc - it has to call png_malloc_warn
 * instead so that control is returned safely back to this routine.
 */
static int
png_image_read_init(png_imagep image)
{
   png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, image,
          png_safe_error, png_safe_warning);

   if (png_ptr != NULL)
   {
      png_infop info_ptr = png_create_info_struct(png_ptr);

      if (info_ptr != NULL)
      {
         png_controlp control = png_voidcast(png_controlp,
            png_malloc_warn(png_ptr, sizeof *control));

         if (control != NULL)
         {
            png_memset(control, 0, sizeof *control);

            control->png_ptr = png_ptr;
            control->info_ptr = info_ptr;
            control->for_write = 0;

            image->opaque = control;
            return 1;
         }

         /* Error clean up */
         png_destroy_info_struct(png_ptr, &info_ptr);
      }

      png_destroy_read_struct(&png_ptr, NULL, NULL);
   }

   return png_image_error(image, "png_image_read: out of memory");
}

/* Utility to find the base format of a PNG file from a png_struct. */
static png_uint_32
png_image_format(png_structp png_ptr, png_infop info_ptr)
{
   png_uint_32 format = 0;

   if (png_ptr->color_type & PNG_COLOR_MASK_COLOR)
      format |= PNG_FORMAT_FLAG_COLOR;

   if (png_ptr->color_type & PNG_COLOR_MASK_ALPHA)
      format |= PNG_FORMAT_FLAG_ALPHA;

   else if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_tRNS))
      format |= PNG_FORMAT_FLAG_ALPHA;

   if (png_ptr->bit_depth == 16)
      format |= PNG_FORMAT_FLAG_LINEAR;

   return format;
}

/* Do the main body of a 'png_image_begin_read' function; read the PNG file
 * header and fill in all the information.  This is executed in a safe context,
 * unlike the init routine above.
 */
static int
png_image_read_header(png_voidp argument)
{
   png_imagep image = png_voidcast(png_imagep, argument);
   png_structp png_ptr = image->opaque->png_ptr;
   png_infop info_ptr = image->opaque->info_ptr;

   png_read_info(png_ptr, info_ptr);

   /* Do this the fast way; just read directly out of png_struct. */
   image->width = png_ptr->width;
   image->height = png_ptr->height;

   {
      png_uint_32 format = png_image_format(png_ptr, info_ptr);

      image->format = format;
      image->flags = 0;

      /* Now try to work out whether the color data does not match sRGB. */
      if ((format & PNG_FORMAT_FLAG_COLOR) != 0 &&
         (info_ptr->valid & PNG_INFO_sRGB) == 0)
      {
         /* gamma is irrelevant because libpng does gamma correction, what
          * matters is if the cHRM chunk doesn't match or, in the absence of
          * cRHM, if the iCCP profile appears to have different end points.
          */
         if (info_ptr->valid & PNG_INFO_cHRM)
         {
            /* TODO: this is a copy'n'paste from pngrutil.c, make a common
             * checking function.  This checks for a 1% error.
             */
            /* The cHRM chunk is used in preference to iCCP */
            if (PNG_OUT_OF_RANGE(info_ptr->x_white, 31270,  1000) ||
                PNG_OUT_OF_RANGE(info_ptr->y_white, 32900,  1000) ||
                PNG_OUT_OF_RANGE(info_ptr->x_red,   64000,  1000) ||
                PNG_OUT_OF_RANGE(info_ptr->y_red,   33000,  1000) ||
                PNG_OUT_OF_RANGE(info_ptr->x_green, 30000,  1000) ||
                PNG_OUT_OF_RANGE(info_ptr->y_green, 60000,  1000) ||
                PNG_OUT_OF_RANGE(info_ptr->x_blue,  15000,  1000) ||
                PNG_OUT_OF_RANGE(info_ptr->y_blue,   6000,  1000))
               image->flags |= PNG_IMAGE_FLAG_COLORSPACE_NOT_sRGB;
         }

         else if (info_ptr->valid & PNG_INFO_iCCP)
         {
#        if 0
            /* TODO: IMPLEMENT THIS! Remember to remove iCCP from
                 the chunks_to_ignore list */
            /* Here if we just have an iCCP chunk. */
            if (!png_iCCP_is_sRGB(png_ptr, info_ptr))
#        endif
               image->flags |= PNG_IMAGE_FLAG_COLORSPACE_NOT_sRGB;
         }
      }
   }

   return 1;
}

#ifdef PNG_STDIO_SUPPORTED
int PNGAPI
png_image_begin_read_from_stdio(png_imagep image, FILE* file)
{
   if (image != NULL)
   {
      if (file != NULL)
      {
         if (png_image_read_init(image))
         {
            /* This is slightly evil, but png_init_io doesn't do anything other
             * than this and we haven't changed the standard IO functions so
             * this saves a 'safe' function.
             */
            image->opaque->png_ptr->io_ptr = file;
            return png_safe_execute(image, png_image_read_header, image);
         }
      }

      else
         return png_image_error(image,
            "png_image_begin_read_from_stdio: invalid argument");
   }

   return 0;
}

int PNGAPI
png_image_begin_read_from_file(png_imagep image, const char *file_name)
{
   if (image != NULL)
   {
      if (file_name != NULL)
      {
         FILE *fp = fopen(file_name, "rb");

         if (fp != NULL)
         {
            if (png_image_read_init(image))
            {
               image->opaque->png_ptr->io_ptr = fp;
               image->opaque->owned_file = 1;
               return png_safe_execute(image, png_image_read_header, image);
            }

            /* Clean up: just the opened file. */
            (void)fclose(fp);
         }

         else
            return png_image_error(image, strerror(errno));
      }

      else
         return png_image_error(image,
            "png_image_begin_read_from_file: invalid argument");
   }

   return 0;
}
#endif /* PNG_STDIO_SUPPORTED */

static void PNGCBAPI
png_image_memory_read(png_structp png_ptr, png_bytep out, png_size_t need)
{
   if (png_ptr != NULL)
   {
      png_imagep image = png_voidcast(png_imagep, png_ptr->io_ptr);
      if (image != NULL)
      {
         png_controlp cp = image->opaque;
         if (cp != NULL)
         {
            png_const_bytep memory = cp->memory;
            png_size_t size = cp->size;

            if (memory != NULL && size >= need)
            {
               png_memcpy(out, memory, need);
               cp->memory = memory + need;
               cp->size = size - need;
               return;
            }

            png_error(png_ptr, "read beyond end of data");
         }
      }

      png_error(png_ptr, "invalid memory read");
   }
}

int PNGAPI png_image_begin_read_from_memory(png_imagep image,
   png_const_voidp memory, png_size_t size)
{
   if (image != NULL)
   {
      if (memory != NULL && size > 0)
      {
         if (png_image_read_init(image))
         {
            /* Now set the IO functions to read from the memory buffer and
             * store it into io_ptr.  Again do this in-place to avoid calling a
             * libpng function that requires error handling.
             */
            image->opaque->memory = png_voidcast(png_const_bytep, memory);
            image->opaque->size = size;
            image->opaque->png_ptr->io_ptr = image;
            image->opaque->png_ptr->read_data_fn = png_image_memory_read;

            return png_safe_execute(image, png_image_read_header, image);
         }
      }

      else
         return png_image_error(image,
            "png_image_begin_read_from_memory: invalid argument");
   }

   return 0;
}

/* Arguments to png_image_finish_read: */
typedef struct
{
   /* Arguments: */
   png_imagep image;
   png_voidp  buffer;
   png_int_32 row_stride;
   png_colorp background;
   /* Local variables: */
   png_bytep  local_row;
   png_bytep  first_row;
   ptrdiff_t  row_bytes; /* unsigned arithmetic step between rows */
} png_image_read_control;

/* Just the row reading part of png_image_read. */
static int
png_image_read_composite(png_voidp argument)
{
   png_image_read_control *display = png_voidcast(png_image_read_control*,
      argument);
   png_imagep image = display->image;
   png_structp png_ptr = image->opaque->png_ptr;
   png_byte interlace_type = png_ptr->interlaced;
   int passes;

   switch (interlace_type)
   {
      case PNG_INTERLACE_NONE:
         passes = 1;
         break;

      case PNG_INTERLACE_ADAM7:
         passes = PNG_INTERLACE_ADAM7_PASSES;
         break;

      default:
         png_error(png_ptr, "unknown interlace type");
   }

   {
      png_uint_32 height = image->height;
      png_uint_32 width = image->width;
      unsigned int channels = (image->format & PNG_FORMAT_FLAG_COLOR) ? 3 : 1;
      int pass;

      for (pass = 0; pass < passes; ++pass)
      {
         png_bytep        row = display->first_row;
         unsigned int     startx, stepx, stepy;
         png_uint_32      y;

         if (interlace_type == PNG_INTERLACE_ADAM7)
         {
            /* The row may be empty for a short image: */
            if (PNG_PASS_COLS(width, pass) == 0)
               continue;

            startx = PNG_PASS_START_COL(pass);
            stepx = PNG_PASS_COL_OFFSET(pass);
            y = PNG_PASS_START_ROW(pass);
            stepy = PNG_PASS_ROW_OFFSET(pass);
         }

         else
         {
            y = 0;
            startx = 0;
            stepx = stepy = 1;
         }

         /* The following are invariants across all the rows: */
         startx *= channels;
         stepx *= channels;
         
         for (; y<height; y += stepy)
            {
               png_bytep inrow = display->local_row;
               png_bytep outrow = row + startx;
               png_const_bytep end_row = row + width * channels;

               /* Read the row, which is packed: */
               png_read_row(png_ptr, inrow, NULL);

               /* Now do the composition on each pixel in this row. */
               for (; outrow < end_row; outrow += stepx)
               {
                  png_byte alpha = inrow[channels];

                  if (alpha > 0) /* else no change to the output */
                  {
                     unsigned int c;

                     for (c=0; c<channels; ++c)
                     {
                        png_uint_32 component = inrow[c];

                        if (alpha < 255) /* else just use component */
                        {
                           /* This is PNG_OPTIMIZED_ALPHA, the component value
                            * is a linear 8-bit value.  Combine this with the
                            * current outrow[c] value which is sRGB encoded.
                            * Arithmetic here is 16-bits to preserve the output
                            * values correctly.
                            */
                           component *= 257*255; /* =65535 */
                           component += (255-alpha)*png_sRGB_table[outrow[c]];

                           /* So 'component' is scaled by 255*65535 and is
                            * therefore appropriate for the sRGB to linear
                            * conversion table.
                            */
                           component = PNG_sRGB_FROM_LINEAR(component);
                        }

                        outrow[c] = (png_byte)component;
                     }
                  }

                  inrow += channels+1; /* components and alpha channel */
               }

               row += display->row_bytes;
            }
      }
   }

   return 1;
}

/* The do_local_background case; called when all the following transforms are to
 * be done:
 *
 * PNG_RGB_TO_GRAY
 * PNG_COMPOSITE
 * PNG_GAMMA
 *
 * This is a work-round for the fact that both the PNG_RGB_TO_GRAY and
 * PNG_COMPOSITE code performs gamma correction, so we get double gamma
 * correction.  The fix-up is to prevent the PNG_COMPOSITE operation happening
 * inside libpng, so this routine sees an 8 or 16-bit gray+alpha row and handles
 * the removal or pre-multiplication of the alpha channel.
 */
static int
png_image_read_background(png_voidp argument)
{
   png_image_read_control *display = png_voidcast(png_image_read_control*,
      argument);
   png_imagep image = display->image;
   png_structp png_ptr = image->opaque->png_ptr;
   png_infop info_ptr = image->opaque->info_ptr;
   png_byte interlace_type = png_ptr->interlaced;
   png_uint_32 height = image->height;
   png_uint_32 width = image->width;
   int pass, passes;

   /* Double check the convoluted logic below.  We expect to get here with
    * libpng doing rgb to gray and gamma correction but background processing
    * left to the png_image_read_background function.  The rows libpng produce
    * might be 8 or 16-bit but should always have two channels; gray plus alpha.
    */
   if ((png_ptr->transformations & PNG_RGB_TO_GRAY) == 0)
      png_error(png_ptr, "lost rgb to gray");

   if ((png_ptr->transformations & PNG_COMPOSE) != 0)
      png_error(png_ptr, "unexpected compose");

   /* The palette code zaps PNG_GAMMA in place... */
   if ((png_ptr->color_type & PNG_COLOR_MASK_PALETTE) == 0 &&
      (png_ptr->transformations & PNG_GAMMA) == 0)
      png_error(png_ptr, "lost gamma correction");

   if (png_get_channels(png_ptr, info_ptr) != 2)
      png_error(png_ptr, "lost/gained channels");

   switch (interlace_type)
   {
      case PNG_INTERLACE_NONE:
         passes = 1;
         break;

      case PNG_INTERLACE_ADAM7:
         passes = PNG_INTERLACE_ADAM7_PASSES;
         break;

      default:
         png_error(png_ptr, "unknown interlace type");
   }

   switch (png_get_bit_depth(png_ptr, info_ptr))
   {
      default:
         png_error(png_ptr, "unexpected bit depth");
         break;

      case 8:
         /* 8-bit sRGB gray values with an alpha channel; the alpha channel is
          * to be removed by composing on a backgroundi: either the row if
          * display->background is NULL or display->background.green if not.
          * Unlike the code above ALPHA_OPTIMIZED has *not* been done.
          */
         for (pass = 0; pass < passes; ++pass)
         {
            png_bytep        row = display->first_row;
            unsigned int     startx, stepx, stepy;
            png_uint_32      y;

            if (interlace_type == PNG_INTERLACE_ADAM7)
            {
               /* The row may be empty for a short image: */
               if (PNG_PASS_COLS(width, pass) == 0)
                  continue;

               startx = PNG_PASS_START_COL(pass);
               stepx = PNG_PASS_COL_OFFSET(pass);
               y = PNG_PASS_START_ROW(pass);
               stepy = PNG_PASS_ROW_OFFSET(pass);
            }

            else
            {
               y = 0;
               startx = 0;
               stepx = stepy = 1;
            }
            
            if (display->background == NULL)
            {
               for (; y<height; y += stepy)
                  {
                     png_bytep inrow = display->local_row;
                     png_bytep outrow = row + startx;
                     png_const_bytep end_row = row + width;

                     /* Read the row, which is packed: */
                     png_read_row(png_ptr, inrow, NULL);

                     /* Now do the composition on each pixel in this row. */
                     for (; outrow < end_row; outrow += stepx)
                     {
                        png_byte alpha = inrow[1];

                        if (alpha > 0) /* else no change to the output */
                        {
                           png_uint_32 component = inrow[0];

                           if (alpha < 255) /* else just use component */
                           {
                              /* Since PNG_OPTIMIZED_ALPHA was not set it is
                               * necessary to invert the sRGB transfer
                               * function and multiply the alpha out.
                               */
                              component = png_sRGB_table[component] * alpha;
                              component += png_sRGB_table[outrow[0]] *
                                 (255-alpha);
                              component = PNG_sRGB_FROM_LINEAR(component);
                           }

                           outrow[0] = (png_byte)component;
                        }

                        inrow += 2; /* gray and alpha channel */
                     }

                     row += display->row_bytes;
                  }
            }

            else /* constant background value */
            {
               png_byte background8 = display->background->green;
               png_uint_16 background = png_sRGB_table[background8];

               for (; y<height; y += stepy)
                  {
                     png_bytep inrow = display->local_row;
                     png_bytep outrow = row + startx;
                     png_const_bytep end_row = row + width;

                     /* Read the row, which is packed: */
                     png_read_row(png_ptr, inrow, NULL);

                     /* Now do the composition on each pixel in this row. */
                     for (; outrow < end_row; outrow += stepx)
                     {
                        png_byte alpha = inrow[1];

                        if (alpha > 0) /* else use background */
                        {
                           png_uint_32 component = inrow[0];

                           if (alpha < 255) /* else just use component */
                           {
                              component = png_sRGB_table[component] * alpha;
                              component += background * (255-alpha);
                              component = PNG_sRGB_FROM_LINEAR(component);
                           }

                           outrow[0] = (png_byte)component;
                        }

                        else
                           outrow[0] = background8;

                        inrow += 2; /* gray and alpha channel */
                     }

                     row += display->row_bytes;
                  }
            }
         }
         break;

      case 16:
         /* 16-bit linear with pre-multiplied alpha; the pre-multiplication must
          * still be done and, maybe, the alpha channel removed.  This code also
          * handles the alpha-first option.
          */
         {
            unsigned int outchannels = png_get_channels(png_ptr, info_ptr);
            int preserve_alpha = (image->format & PNG_FORMAT_FLAG_ALPHA) != 0;
            int swap_alpha = 0;

            if (preserve_alpha && (image->format & PNG_FORMAT_FLAG_AFIRST))
               swap_alpha = 1;

            for (pass = 0; pass < passes; ++pass)
            {
               png_uint_16p     row = (png_uint_16p)display->first_row;
               unsigned int     startx, stepx, stepy; /* all in pixels */
               png_uint_32      y;

               if (interlace_type == PNG_INTERLACE_ADAM7)
               {
                  /* The row may be empty for a short image: */
                  if (PNG_PASS_COLS(width, pass) == 0)
                     continue;

                  startx = PNG_PASS_START_COL(pass);
                  stepx = PNG_PASS_COL_OFFSET(pass);
                  y = PNG_PASS_START_ROW(pass);
                  stepy = PNG_PASS_ROW_OFFSET(pass);
               }

               else
               {
                  y = 0;
                  startx = 0;
                  stepx = stepy = 1;
               }

               startx *= outchannels;
               stepx *= outchannels;
               
               for (; y<height; y += stepy)
                  {
                     png_uint_16p inrow;
                     png_uint_16p outrow = row + startx;
                     png_uint_16p end_row = row + width * outchannels;

                     /* Read the row, which is packed: */
                     png_read_row(png_ptr, display->local_row, NULL);
                     inrow = (png_uint_16p)display->local_row;

                     /* Now do the pre-multiplication on each pixel in this row.
                      */
                     for (; outrow < end_row; outrow += stepx)
                     {
                        png_uint_32 component = inrow[0];
                        png_uint_16 alpha = inrow[1];

                        if (alpha > 0) /* else 0 */
                        {
                           if (alpha < 65535) /* else just use component */
                           {
                              component *= alpha;
                              component += 32767;
                              component /= 65535;
                           }
                        }

                        else
                           component = 0;

                        outrow[swap_alpha] = (png_uint_16)component;
                        if (outchannels > 1)
                           outrow[1 ^ swap_alpha] = alpha;

                        inrow += 2; /* components and alpha channel */
                     }

                     row += display->row_bytes;
                  }
            }
         }
         break;
   }

   return 1;
}

/* The guts of png_image_finish_read as a png_safe_execute callback. */
static int
png_image_read_end(png_voidp argument)
{
   png_image_read_control *display = png_voidcast(png_image_read_control*,
      argument);
   png_imagep image = display->image;
   png_structp png_ptr = image->opaque->png_ptr;
   png_infop info_ptr = image->opaque->info_ptr;

   png_uint_32 format = image->format;
   int linear = (format & PNG_FORMAT_FLAG_LINEAR) != 0;
   int do_local_compose = 0;
   int do_local_background = 0; /* to avoid double gamma correction bug */
   int passes = 0;

   /* Add transforms to ensure the correct output format is produced then check
    * that the required implementation support is there.  Always expand; always
    * need 8 bits minimum, no palette and expanded tRNS.
    */
   png_set_expand(png_ptr);
   
   /* Now check the format to see if it was modified. */
   {
      png_uint_32 base_format = png_image_format(png_ptr, info_ptr);
      png_uint_32 change = format ^ base_format;
      png_fixed_point output_gamma;
      int mode; /* alpha mode */

      /* Do this first so that we have a record if rgb to gray is happening. */
      if (change & PNG_FORMAT_FLAG_COLOR)
      {
         /* gray<->color transformation required. */
         if (format & PNG_FORMAT_FLAG_COLOR)
            png_set_gray_to_rgb(png_ptr);

         else
         {
            /* libpng can't do both rgb to gray and
             * background/pre-multiplication if there is also significant gamma
             * correction, because both operations require linear colors and
             * the code only supports one transform doing the gamma correction.
             * Handle this by doing the pre-multiplication or background
             * operation in this code, if necessary.
             *
             * TODO: fix this by rewriting pngrtran.c (!)
             *
             * For the moment (given that fixing this in pngrtran.c is an
             * enormous change) 'do_local_background' is used to indicate that
             * the problem exists.
             */
            if (base_format & PNG_FORMAT_FLAG_ALPHA)
               do_local_background = 1/*maybe*/;

            png_set_rgb_to_gray_fixed(png_ptr, PNG_ERROR_ACTION_NONE,
               PNG_RGB_TO_GRAY_DEFAULT, PNG_RGB_TO_GRAY_DEFAULT);
         }

         change &= ~PNG_FORMAT_FLAG_COLOR;
      }

      /* Set the gamma appropriately, linear for 16-bit input, sRGB otherwise.
       */
      {
         png_fixed_point input_gamma_default;

         if (base_format & PNG_FORMAT_FLAG_LINEAR)
            input_gamma_default = PNG_GAMMA_LINEAR;
         else
            input_gamma_default = PNG_DEFAULT_sRGB;

         /* Call png_set_alpha_mode to set the default for the input gamma; the
          * output gamma is set by a second call below.
          */
         png_set_alpha_mode_fixed(png_ptr, PNG_ALPHA_PNG, input_gamma_default);
      }

      if (linear)
      {
         /* If there *is* an alpha channel in the input it must be multiplied
          * out; use PNG_ALPHA_STANDARD, otherwise just use PNG_ALPHA_PNG.
          */
         if (base_format & PNG_FORMAT_FLAG_ALPHA)
            mode = PNG_ALPHA_STANDARD; /* associated alpha */

         else
            mode = PNG_ALPHA_PNG;

         output_gamma = PNG_GAMMA_LINEAR;
      }

      else
      {
         mode = PNG_ALPHA_PNG;
         output_gamma = PNG_DEFAULT_sRGB;
      }

      /* If 'do_local_background' is set check for the presence of gamma
       * correction; this is part of the work-round for the libpng bug
       * described above.
       *
       * TODO: fix libpng and remove this.
       */
      if (do_local_background)
      {
         png_fixed_point gtest;

         /* This is 'png_gamma_threshold' from pngrtran.c; the test used for
          * gamma correction, the screen gamma hasn't been set on png_struct
          * yet; it's set below.  png_struct::gamma, however, is set to the
          * final value.
          */
         if (png_muldiv(&gtest, output_gamma, png_ptr->gamma, PNG_FP_1) &&
            !png_gamma_significant(gtest))
            do_local_background = 0;

         else if (mode == PNG_ALPHA_STANDARD)
         {
            do_local_background = 2/*required*/;
            mode = PNG_ALPHA_PNG; /* prevent libpng doing it */
         }

         /* else leave as 1 for the checks below */
      }

      /* If the bit-depth changes then handle that here. */
      if (change & PNG_FORMAT_FLAG_LINEAR)
      {
         if (linear /*16-bit output*/)
            png_set_expand_16(png_ptr);

         else /* 8-bit output */
            png_set_scale_16(png_ptr);

         change &= ~PNG_FORMAT_FLAG_LINEAR;
      }

      /* Now the background/alpha channel changes. */
      if (change & PNG_FORMAT_FLAG_ALPHA)
      {
         /* Removing an alpha channel requires composition for the 8-bit
          * formats; for the 16-bit it is already done, above, by the
          * pre-multiplication and the channel just needs to be stripped.
          */
         if (base_format & PNG_FORMAT_FLAG_ALPHA)
         {
            /* If RGB->gray is happening the alpha channel must be left and the
             * operation completed locally.
             *
             * TODO: fix libpng and remove this.
             */
            if (do_local_background)
               do_local_background = 2/*required*/;

            /* 16-bit output: just remove the channel */
            else if (linear) /* compose on black (well, pre-multiply) */
               png_set_strip_alpha(png_ptr);

            /* 8-bit output: do an appropriate compose */
            else if (display->background != NULL)
            {
               png_color_16 c;

               c.index = 0; /*unused*/
               c.red = display->background->red;
               c.green = display->background->green;
               c.blue = display->background->blue;
               c.gray = display->background->green;

               /* This is always an 8-bit sRGB value, using the 'green' channel
                * for gray is much better than calculating the luminance here;
                * we can get off-by-one errors in that calculation relative to
                * the app expectations and that will show up in transparent
                * pixels.
                */
               png_set_background_fixed(png_ptr, &c,
                  PNG_BACKGROUND_GAMMA_SCREEN, 0/*need_expand*/,
                  0/*gamma: not used*/);
            }

            else /* compose on row: implemented below. */
            {
               do_local_compose = 1;
               /* This leaves the alpha channel in the output, so it has to be
                * removed by the code below.  Set the encoding to the 'OPTIMIZE'
                * one so the code only has to hack on the pixels that require
                * composition.
                */
               mode = PNG_ALPHA_OPTIMIZED;
            }
         }

         else /* output needs an alpha channel */
         {
            /* This is tricky because it happens before the swap operation has
             * been accomplished; however, the swap does *not* swap the added
             * alpha channel (weird API), so it must be added in the correct
             * place.
             */
            png_uint_32 filler; /* opaque filler */
            int where;

            if (linear)
               filler = 65535;

            else
               filler = 255;

#           ifdef PNG_FORMAT_AFIRST_SUPPORTED
               if (format & PNG_FORMAT_FLAG_AFIRST)
               {
                  where = PNG_FILLER_BEFORE;
                  change &= ~PNG_FORMAT_FLAG_AFIRST;
               }

               else
#           endif
               where = PNG_FILLER_AFTER;

            png_set_add_alpha(png_ptr, filler, where);
         }

         /* This stops the (irrelevant) call to swap_alpha below. */
         change &= ~PNG_FORMAT_FLAG_ALPHA;
      }

      /* Now set the alpha mode correctly; this is always done, even if there is
       * no alpha channel in either the input or the output because it correctly
       * sets the output gamma.
       */
      png_set_alpha_mode_fixed(png_ptr, mode, output_gamma);

#     ifdef PNG_FORMAT_BGR_SUPPORTED
         if (change & PNG_FORMAT_FLAG_BGR)
         {
            /* Check only the output format; PNG is never BGR; don't do this if
             * the output is gray, but fix up the 'format' value in that case.
             */
            if (format & PNG_FORMAT_FLAG_COLOR)
               png_set_bgr(png_ptr);

            else
               format &= ~PNG_FORMAT_FLAG_BGR;

            change &= ~PNG_FORMAT_FLAG_BGR;
         }
#     endif

#     ifdef PNG_FORMAT_AFIRST_SUPPORTED
         if (change & PNG_FORMAT_FLAG_AFIRST)
         {
            /* Only relevant if there is an alpha channel - it's particularly
             * important to handle this correctly because do_local_compose may
             * be set above and then libpng will keep the alpha channel for this
             * code to remove.
             */
            if (format & PNG_FORMAT_FLAG_ALPHA)
            {
               /* Disable this if doing a local background,
                * TODO: remove this when local background is no longer required.
                */
               if (do_local_background != 2)
                  png_set_swap_alpha(png_ptr);
            }

            else
               format &= ~PNG_FORMAT_FLAG_AFIRST;

            change &= ~PNG_FORMAT_FLAG_AFIRST;
         }
#     endif

      /* If the *output* is 16-bit then we need to check for a byte-swap on this
       * architecture.
       */
      if (linear)
      {
         PNG_CONST png_uint_16 le = 0x0001;

         if (*(png_const_bytep)&le)
            png_set_swap(png_ptr);
      }

      /* If change is not now 0 some transformation is missing - error out. */
      if (change)
         png_error(png_ptr, "png_read_image: unsupported transformation");
   }

#  ifdef PNG_HANDLE_AS_UNKNOWN_SUPPORTED
      /* Prepare the reader to ignore all recognized chunks whose data will not
       * be used, i.e., all chunks recognized by libpng except for those
       * involved in basic image reading:
       *
       *    IHDR, PLTE, IDAT, IEND
       *
       * Or image data handling:
       *
       *    tRNS, bKGD, gAMA, cHRM, sRGB, iCCP and sBIT.
       *
       * This provides a small performance improvement and eliminates any
       * potential vulnerability to security problems in the unused chunks.
       *
       * TODO: make it so that this is an explicit list to process, not a list
       * to ignore?
       */
      {
          static PNG_CONST png_byte chunks_to_ignore[] = {
              104,  73,  83,  84, '\0',  /* hIST */
              105,  84,  88, 116, '\0',  /* iTXt */
              111,  70,  70, 115, '\0',  /* oFFs */
              112,  67,  65,  76, '\0',  /* pCAL */
              112,  72,  89, 115, '\0',  /* pHYs */
              115,  67,  65,  76, '\0',  /* sCAL */
              115,  80,  76,  84, '\0',  /* sPLT */
              116,  69,  88, 116, '\0',  /* tEXt */
              116,  73,  77,  69, '\0',  /* tIME */
              122,  84,  88, 116, '\0'   /* zTXt */
          };

          /* Ignore unknown chunks */
          png_set_keep_unknown_chunks(png_ptr, 1 /* PNG_HANDLE_CHUNK_NEVER */,
            NULL, 0);

          /* Ignore known but unused chunks */
          png_set_keep_unknown_chunks(png_ptr, 1 /* PNG_HANDLE_CHUNK_NEVER */,
            chunks_to_ignore, (sizeof chunks_to_ignore)/5);
       }
#  endif /* PNG_HANDLE_AS_UNKNOWN_SUPPORTED */

   /* Update the 'info' structure and make sure the result is as required; first
    * make sure to turn on the interlace handling if it will be required
    * (because it can't be turned on *after* the call to png_read_update_info!)
    *
    * TODO: remove the do_local_background fixup below.
    */
   if (!do_local_compose && do_local_background != 2)
      passes = png_set_interlace_handling(png_ptr);

   png_read_update_info(png_ptr, info_ptr);

   {
      png_uint_32 info_format = 0;

      if (info_ptr->color_type & PNG_COLOR_MASK_COLOR)
         info_format |= PNG_FORMAT_FLAG_COLOR;

      if (info_ptr->color_type & PNG_COLOR_MASK_ALPHA)
      {
         /* do_local_compose removes this channel below. */
         if (!do_local_compose)
         {
            /* do_local_background does the same if required. */
            if (do_local_background != 2 ||
               (format & PNG_FORMAT_FLAG_ALPHA) != 0)
               info_format |= PNG_FORMAT_FLAG_ALPHA;
         }
      }

      else if (do_local_compose) /* internal error */
         png_error(png_ptr, "png_image_read: alpha channel lost");

      if (info_ptr->bit_depth == 16)
         info_format |= PNG_FORMAT_FLAG_LINEAR;

#     ifdef PNG_FORMAT_BGR_SUPPORTED
         if (png_ptr->transformations & PNG_BGR)
            info_format |= PNG_FORMAT_FLAG_BGR;
#     endif

#     ifdef PNG_FORMAT_AFIRST_SUPPORTED
         if (png_ptr->transformations & PNG_SWAP_ALPHA ||
            ((png_ptr->transformations & PNG_ADD_ALPHA) != 0 &&
            (png_ptr->flags & PNG_FLAG_FILLER_AFTER) == 0))
            info_format |= PNG_FORMAT_FLAG_AFIRST;
#     endif

      /* This is actually an internal error. */
      if (info_format != format)
         png_error(png_ptr, "png_read_image: invalid transformations");
   }

   /* Now read the rows.  If do_local_compose is set then it is necessary to use
    * a local row buffer.  The output will be GA, RGBA or BGRA and must be
    * converted to G, RGB or BGR as appropriate.  The 'local_row' member of the
    * display acts as a flag.
    */
   {
      png_bytep first_row = png_voidcast(png_bytep, display->buffer);
      ptrdiff_t row_bytes = display->row_stride;

      if (linear)
         row_bytes *= sizeof (png_uint_16);

      /* The following expression is designed to work correctly whether it gives
       * a signed or an unsigned result.
       */
      if (row_bytes < 0)
         first_row += (image->height-1) * (-row_bytes);

      display->first_row = first_row;
      display->row_bytes = row_bytes;
   }

   if (do_local_compose)
   {
      int result;
      png_bytep row = png_voidcast(png_bytep, png_malloc(png_ptr,
         png_get_rowbytes(png_ptr, info_ptr)));

      display->local_row = row;
      result = png_safe_execute(image, png_image_read_composite, display);
      display->local_row = NULL;
      png_free(png_ptr, row);

      return result;
   }

   else if (do_local_background == 2)
   {
      int result;
      png_bytep row = png_voidcast(png_bytep, png_malloc(png_ptr,
         png_get_rowbytes(png_ptr, info_ptr)));

      display->local_row = row;
      result = png_safe_execute(image, png_image_read_background, display);
      display->local_row = NULL;
      png_free(png_ptr, row);

      return result;
   }

   else
   {
      png_alloc_size_t row_bytes = display->row_bytes;

      while (--passes >= 0)
      {
         png_uint_32      y = image->height;
         png_bytep        row = display->first_row;
         
         while (y-- > 0)
         {
            png_read_row(png_ptr, row, NULL);
            row += row_bytes;
         }
      }

      return 1;
   }
}

int PNGAPI
png_image_finish_read(png_imagep image, png_colorp background, void *buffer,
   png_int_32 row_stride)
{
   if (image != NULL)
   {
      png_uint_32 check;

      if (row_stride == 0)
         row_stride = PNG_IMAGE_ROW_STRIDE(*image);

      if (row_stride < 0)
         check = -row_stride;

      else
         check = row_stride;

      if (buffer != NULL && check >= PNG_IMAGE_ROW_STRIDE(*image))
      {
         int result;
         png_image_read_control display;

         png_memset(&display, 0, sizeof display);
         display.image = image;
         display.buffer = buffer;
         display.row_stride = row_stride;
         display.background = background;
         display.local_row = NULL;
         result = png_safe_execute(image, png_image_read_end, &display);
         png_image_free(image);
         return result;
      }

      else
         return png_image_error(image,
            "png_image_finish_read: invalid argument");
   }

   return 0;
}

#endif /* PNG_SIMPLIFIED_READ_SUPPORTED */
#endif /* PNG_READ_SUPPORTED */
