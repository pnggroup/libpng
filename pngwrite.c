
/* pngwrite.c - general routines to write a PNG file
 *
 * Last changed in libpng 1.7.0 [(PENDING RELEASE)]
 * Copyright (c) 1998-2015 Glenn Randers-Pehrson
 * (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
 * (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */

#include "pngpriv.h"
#if defined(PNG_SIMPLIFIED_WRITE_SUPPORTED) && defined(PNG_STDIO_SUPPORTED)
#  include <errno.h>
#endif
#define PNG_SRC_FILE PNG_SRC_FILE_pngwrite

#ifdef PNG_WRITE_SUPPORTED

#ifdef PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED
/* Write out all the unknown chunks for the current given location */
static void
write_unknown_chunks(png_structrp png_ptr, png_const_inforp info_ptr,
   unsigned int where)
{
   if (info_ptr->unknown_chunks_num != 0)
   {
      png_const_unknown_chunkp up;

      png_debug(5, "writing extra chunks");

      for (up = info_ptr->unknown_chunks;
           up < info_ptr->unknown_chunks + info_ptr->unknown_chunks_num;
           ++up)
         if ((up->location & where) != 0)
      {
         /* If per-chunk unknown chunk handling is enabled use it, otherwise
          * just write the chunks the application has set.
          */
#ifdef PNG_SET_UNKNOWN_CHUNKS_SUPPORTED
         int keep = png_handle_as_unknown(png_ptr, up->name);

         /* NOTE: this code is radically different from the read side in the
          * matter of handling an ancillary unknown chunk.  In the read side
          * the default behavior is to discard it, in the code below the default
          * behavior is to write it.  Critical chunks are, however, only
          * written if explicitly listed or if the default is set to write all
          * unknown chunks.
          *
          * The default handling is also slightly weird - it is not possible to
          * stop the writing of all unsafe-to-copy chunks!
          *
          * TODO: REVIEW: this would seem to be a bug.
          */
         if (keep != PNG_HANDLE_CHUNK_NEVER &&
             ((up->name[3] & 0x20) /* safe-to-copy overrides everything */ ||
              keep == PNG_HANDLE_CHUNK_ALWAYS ||
              (keep == PNG_HANDLE_CHUNK_AS_DEFAULT &&
               png_ptr->unknown_default == PNG_HANDLE_CHUNK_ALWAYS)))
#endif
         {
            /* TODO: review, what is wrong with a zero length unknown chunk? */
            if (up->size == 0)
               png_warning(png_ptr, "Writing zero-length unknown chunk");

            png_write_chunk(png_ptr, up->name, up->data, up->size);
         }
      }
   }
}
#endif /* WRITE_UNKNOWN_CHUNKS */

/* Writes all the PNG information.  This is the suggested way to use the
 * library.  If you have a new chunk to add, make a function to write it,
 * and put it in the correct location here.  If you want the chunk written
 * after the image data, put it in png_write_end().  I strongly encourage
 * you to supply a PNG_INFO_ flag, and check info_ptr->valid before writing
 * the chunk, as that will keep the code from breaking if you want to just
 * write a plain PNG file.  If you have long comments, I suggest writing
 * them in png_write_end(), and compressing them.
 */
void PNGAPI
png_write_info_before_PLTE(png_structrp png_ptr, png_const_inforp info_ptr)
{
   png_debug(1, "in png_write_info_before_PLTE");

   if (png_ptr == NULL || info_ptr == NULL)
      return;

   if ((png_ptr->mode & PNG_HAVE_IHDR) == 0)
   {
      int color_type = PNG_COLOR_TYPE_FROM_FORMAT(info_ptr->format);

      /* Write PNG signature; doesn't set PNG_HAVE_PNG_SIGNATURE if it has
       * already been written (or rather, if at least 3 bytes have already been
       * written; undocumented wackiness, it means the 'PNG' at the start can be
       * replace by, e.g. "FOO" or "BAR" or "MNG").
       */
      png_write_sig(png_ptr);

#     ifdef PNG_MNG_FEATURES_SUPPORTED
         if ((png_ptr->mode & PNG_HAVE_PNG_SIGNATURE) != 0 &&
             png_ptr->mng_features_permitted != 0)
         {
            png_app_error(png_ptr,
                  "MNG features are not allowed in a PNG datastream");
            /* Recovery: disable MNG features: */
            png_ptr->mng_features_permitted = 0;
         }
#     endif /* MNG_FEATURES */

      /* Write IHDR information. */
      png_write_IHDR(png_ptr, info_ptr->width, info_ptr->height,
         info_ptr->bit_depth, color_type, info_ptr->compression_type,
         info_ptr->filter_type, info_ptr->interlace_type);

#     ifdef PNG_WRITE_TRANSFORMS_SUPPORTED
         /* This are used for checking later on: */
         png_ptr->info_format = info_ptr->format;
#     endif /* WRITE_TRANSFORMS */

      /* This sets the flag that prevents re-entry to the 'before PLTE' case: */
      affirm((png_ptr->mode & PNG_HAVE_IHDR) != 0);

      /* The rest of these check to see if the valid field has the appropriate
       * flag set, and if it does, writes the chunk.
       *
       * 1.6.0: COLORSPACE support controls the writing of these chunks too, and
       * the chunks will be written if the WRITE routine is there and
       * information is available in the COLORSPACE.  (See
       * png_colorspace_sync_info in png.c for where the valid flags get set.)
       *
       * Under certain circumstances the colorspace can be invalidated without
       * syncing the info_struct 'valid' flags; this happens if libpng detects
       * an error and calls png_error while the color space is being set, yet
       * the application continues writing the PNG.  So check the 'invalid'
       * flag here too.
       */
#     ifdef PNG_WRITE_gAMA_SUPPORTED /* enables GAMMA */
         if ((info_ptr->colorspace.flags & PNG_COLORSPACE_INVALID) == 0 &&
             (info_ptr->colorspace.flags & PNG_COLORSPACE_FROM_gAMA) != 0 &&
             (info_ptr->valid & PNG_INFO_gAMA) != 0)
         {
            /* This is the inverse of the test in png.c: */
            affirm(info_ptr->colorspace.gamma >= 16 &&
                   info_ptr->colorspace.gamma <= 625000000);
            png_write_gAMA_fixed(png_ptr, info_ptr->colorspace.gamma);
         }
#     endif /* WRITE_gAMA */

      /* Write only one of sRGB or an ICC profile.  If a profile was supplied
       * and it matches one of the known sRGB ones issue a warning.
       */
#     ifdef PNG_WRITE_iCCP_SUPPORTED /* enables COLORSPACE, GAMMA */
         if ((info_ptr->colorspace.flags & PNG_COLORSPACE_INVALID) == 0 &&
             (info_ptr->valid & PNG_INFO_iCCP) != 0)
         {
#           ifdef PNG_WRITE_sRGB_SUPPORTED
               /* The app must have supplied an sRGB iCCP profile (and one that
                * is recognized and therefore known to be correct) so we write
                * that profile, even though it increases the size of the PNG
                * significantly.  A warning is reasonable:
                */
               if ((info_ptr->valid & PNG_INFO_sRGB) != 0)
                  png_app_warning(png_ptr,
                     "profile matches sRGB but writing iCCP instead");
#           endif /* WRITE_sRGB */

            png_write_iCCP(png_ptr, info_ptr->iccp_name,
               info_ptr->iccp_profile);
         }
#        ifdef PNG_WRITE_sRGB_SUPPORTED
            else /* iCCP not written */
#        endif /* WRITE_sRGB */
#     endif /* WRITE_iCCP */

#     ifdef PNG_WRITE_sRGB_SUPPORTED /* enables COLORSPACE, GAMMA */
         if ((info_ptr->colorspace.flags & PNG_COLORSPACE_INVALID) == 0 &&
             (info_ptr->valid & PNG_INFO_sRGB) != 0)
            png_write_sRGB(png_ptr, info_ptr->colorspace.rendering_intent);
#     endif /* WRITE_sRGB */

#     ifdef PNG_WRITE_sBIT_SUPPORTED
         if ((info_ptr->valid & PNG_INFO_sBIT) != 0)
            png_write_sBIT(png_ptr, &(info_ptr->sig_bit), color_type);
#     endif /* WRITE_sBIT */

#     ifdef PNG_WRITE_cHRM_SUPPORTED /* enables COLORSPACE */
         if ((info_ptr->colorspace.flags & PNG_COLORSPACE_INVALID) == 0 &&
            (info_ptr->colorspace.flags & PNG_COLORSPACE_FROM_cHRM) != 0 &&
            (info_ptr->valid & PNG_INFO_cHRM) != 0)
            png_write_cHRM_fixed(png_ptr, &info_ptr->colorspace.end_points_xy);
#     endif /* WRITE_cHRM */

#     ifdef PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED
         /* The third arugment must encode only one bit, otherwise chunks will
          * be written twice because the test in write_unknown_chunks is
          * 'location & where'.
          */
         write_unknown_chunks(png_ptr, info_ptr, PNG_HAVE_IHDR);
#     endif
   }

   else /* 1.7.0: flag multiple calls; previously ignored */
      png_app_error(png_ptr,
            "png_write_info_before_PLTE called more than once");
}

#ifdef PNG_WRITE_TEXT_SUPPORTED
static void
png_write_text(png_structrp png_ptr, png_const_inforp info_ptr)
   /* Text chunk helper */
{
   int i;

   /* Check to see if we need to write text chunks */
   for (i = 0; i < info_ptr->num_text; i++)
   {
      png_debug2(2, "Writing text chunk %d, type %d", i,
            info_ptr->text[i].compression);

      /* An internationalized chunk? */
      if (info_ptr->text[i].compression > 0)
      {
#        ifdef PNG_WRITE_iTXt_SUPPORTED
            /* Write international chunk */
            png_write_iTXt(png_ptr, info_ptr->text[i].compression,
                  info_ptr->text[i].key, info_ptr->text[i].lang,
                  info_ptr->text[i].lang_key, info_ptr->text[i].text);
#        else /* !WRITE_iTXT */
            png_app_error(png_ptr, "Unable to write international text");
#        endif /* !WRITE_iTXT */

         /* Mark this chunk as written */
         if (info_ptr->text[i].compression == PNG_TEXT_COMPRESSION_NONE)
             info_ptr->text[i].compression = PNG_TEXT_COMPRESSION_NONE_WR;
         else
            info_ptr->text[i].compression = PNG_TEXT_COMPRESSION_zTXt_WR;
      }

      /* If we want a compressed text chunk */
      else if (info_ptr->text[i].compression == PNG_TEXT_COMPRESSION_zTXt)
      {
#        ifdef PNG_WRITE_zTXt_SUPPORTED
            /* Write compressed chunk */
            png_write_zTXt(png_ptr, info_ptr->text[i].key,
                  info_ptr->text[i].text, info_ptr->text[i].compression);
#        else /* !WRITE_zTXT */
            png_app_error(png_ptr, "Unable to write compressed text");
#        endif /* !WRITE_zTXT */

         /* Mark this chunk as written */
         info_ptr->text[i].compression = PNG_TEXT_COMPRESSION_zTXt_WR;
      }

      else if (info_ptr->text[i].compression == PNG_TEXT_COMPRESSION_NONE)
      {
#        ifdef PNG_WRITE_tEXt_SUPPORTED
            /* Write uncompressed chunk */
            png_write_tEXt(png_ptr, info_ptr->text[i].key,
                  info_ptr->text[i].text, 0);
#        else /* !WRITE_tEXt */
            /* Can't get here TODO: why not? */
            png_app_error(png_ptr, "Unable to write uncompressed text");
#        endif /* !WRITE_tEXt */

         /* Mark this chunk as written */
         info_ptr->text[i].compression = PNG_TEXT_COMPRESSION_NONE_WR;
      }
   }
}
#endif /* WRITE_TEXT */

void PNGAPI
png_write_info(png_structrp png_ptr, png_const_inforp info_ptr)
{
   png_debug(1, "in png_write_info");

   if (png_ptr == NULL || info_ptr == NULL)
      return;

   if ((png_ptr->mode & (PNG_HAVE_PLTE+PNG_HAVE_IDAT)) != 0)
   {
      png_app_error(png_ptr, "late call to png_write_info");
      return;
   }

   /* The app may do this for us, and in 1.7.0 multiple calls are flagged as an
    * application error, so this code must check:
    */
   if ((png_ptr->mode & PNG_HAVE_IHDR) == 0)
      png_write_info_before_PLTE(png_ptr, info_ptr);

   if ((info_ptr->valid & PNG_INFO_PLTE) != 0)
      png_write_PLTE(png_ptr, info_ptr->palette, info_ptr->num_palette);

   /* Validate the consistency of the PNG being produced; a palette must have
    * been written if a palette mapped PNG is to be valid:
    */
   if ((png_ptr->mode & PNG_HAVE_PLTE) == 0 &&
       png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      png_error(png_ptr, "Valid palette required for paletted images");

#  ifdef PNG_WRITE_tRNS_SUPPORTED
      if ((info_ptr->valid & PNG_INFO_tRNS) !=0)
      {
         png_write_tRNS(png_ptr, info_ptr->trans_alpha,
            &(info_ptr->trans_color), info_ptr->num_trans,
            PNG_COLOR_TYPE_FROM_FORMAT(info_ptr->format));
      }
#  endif /* WRITE_tRNS */

#  ifdef PNG_WRITE_bKGD_SUPPORTED
      if ((info_ptr->valid & PNG_INFO_bKGD) != 0)
         png_write_bKGD(png_ptr, &(info_ptr->background),
            PNG_COLOR_TYPE_FROM_FORMAT(info_ptr->format));
#  endif /* WRITE_bKGD */

#  ifdef PNG_WRITE_hIST_SUPPORTED
      if ((info_ptr->valid & PNG_INFO_hIST) != 0)
         png_write_hIST(png_ptr, info_ptr->hist, info_ptr->num_palette);
#  endif /* WRITE_hIST */

#  ifdef PNG_WRITE_oFFs_SUPPORTED
      if ((info_ptr->valid & PNG_INFO_oFFs) != 0)
         png_write_oFFs(png_ptr, info_ptr->x_offset, info_ptr->y_offset,
             info_ptr->offset_unit_type);
#  endif /* WRITE_oFFs */

#  ifdef PNG_WRITE_pCAL_SUPPORTED
      if ((info_ptr->valid & PNG_INFO_pCAL) != 0)
         png_write_pCAL(png_ptr, info_ptr->pcal_purpose, info_ptr->pcal_X0,
             info_ptr->pcal_X1, info_ptr->pcal_type, info_ptr->pcal_nparams,
             info_ptr->pcal_units, info_ptr->pcal_params);
#  endif /* WRITE_pCAL */

#  ifdef PNG_WRITE_sCAL_SUPPORTED
      if ((info_ptr->valid & PNG_INFO_sCAL) != 0)
         png_write_sCAL_s(png_ptr, info_ptr->scal_unit, info_ptr->scal_s_width,
               info_ptr->scal_s_height);
#  endif /* WRITE_sCAL */

#  ifdef PNG_WRITE_pHYs_SUPPORTED
      if ((info_ptr->valid & PNG_INFO_pHYs) != 0)
         png_write_pHYs(png_ptr, info_ptr->x_pixels_per_unit,
             info_ptr->y_pixels_per_unit, info_ptr->phys_unit_type);
#  endif /* WRITE_pHYs */

#  ifdef PNG_WRITE_tIME_SUPPORTED
      if ((info_ptr->valid & PNG_INFO_tIME) != 0)
         png_write_tIME(png_ptr, &(info_ptr->mod_time));
#  endif /* WRITE_tIME */

#  ifdef PNG_WRITE_sPLT_SUPPORTED
      if ((info_ptr->valid & PNG_INFO_sPLT) != 0)
      {
         int i;

         for (i = 0; i < info_ptr->splt_palettes_num; i++)
            png_write_sPLT(png_ptr, info_ptr->splt_palettes + i);
      }
#  endif /* WRITE_sPLT */

#  ifdef PNG_WRITE_TEXT_SUPPORTED
      if (info_ptr->num_text > 0)
         png_write_text(png_ptr, info_ptr);
#  endif /* WRITE_TEXT */

#  ifdef PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED
      write_unknown_chunks(png_ptr, info_ptr, PNG_HAVE_PLTE);
#  endif /* WRITE_UNKNOWN_CHUNKS */
}

/* Writes the end of the PNG file.  If you don't want to write comments or
 * time information, you can pass NULL for info.  If you already wrote these
 * in png_write_info(), do not write them again here.  If you have long
 * comments, I suggest writing them here, and compressing them.
 */
void PNGAPI
png_write_end(png_structrp png_ptr, png_inforp info_ptr)
{
   png_debug(1, "in png_write_end");

   if (png_ptr == NULL)
      return;

   if ((png_ptr->mode &
         (PNG_HAVE_IHDR+PNG_HAVE_IDAT+PNG_AFTER_IDAT+PNG_HAVE_IEND)) != 
         (PNG_HAVE_IHDR+PNG_HAVE_IDAT+PNG_AFTER_IDAT))
   {
      /* Out of place png_write_end: */
      if ((png_ptr->mode & PNG_HAVE_IHDR) == 0)
         png_error(png_ptr, "Missing call to png_write_info");

      else if ((png_ptr->mode & PNG_HAVE_IDAT) == 0 && png_ptr->zowner == 0)
      {
         /* TODO: write unknown IDAT here, for the moment allow the app to write
          * IDAT then call write_end:
          */
         png_app_error(png_ptr, "No IDATs written into file");
         png_ptr->mode |= PNG_HAVE_IDAT+PNG_AFTER_IDAT;
      }

      else if ((png_ptr->mode & PNG_AFTER_IDAT) == 0)
      {
         affirm(png_ptr->zowner == png_IDAT);
         png_error(png_ptr, "incomplete PNG image"); /* unrecoverable */
      }

      else if ((png_ptr->mode & PNG_HAVE_IEND) != 0)
      {
         png_app_error(png_ptr, "multiple calls to png_write_end");
         return;
      }

      else
         impossible("not reached");
   }

   /* And double check that the image rows were all written; this is actually
    * a harmless error on an interlaced image because the image rows with
    * data were all passed in or the above check would not work.
    *
    * Don't do this if the IDAT came from unknowns (TBD) or the app, above.
    *
    * The check depends on the precise logic in png_write_finish_row.
    */
   else if (png_ptr->interlaced ? png_ptr->pass != PNG_INTERLACE_ADAM7_PASSES :
         png_ptr->row_number != png_ptr->height)
      png_app_error(png_ptr, "png_write_row not called to last row");

   /* See if user wants us to write information chunks */
   if (info_ptr != NULL)
   {
#     ifdef PNG_WRITE_tIME_SUPPORTED
         /* Check to see if user has supplied a time chunk */
         if ((info_ptr->valid & PNG_INFO_tIME) != 0)
            png_write_tIME(png_ptr, &(info_ptr->mod_time));
#     endif

#     ifdef PNG_WRITE_TEXT_SUPPORTED
         if (info_ptr->num_text > 0)
            png_write_text(png_ptr, info_ptr);
#     endif /* WRITE_TEXT */

#     ifdef PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED
         write_unknown_chunks(png_ptr, info_ptr, PNG_AFTER_IDAT);
#     endif
   }

   /* Write end of PNG file */
   png_write_IEND(png_ptr);

   /* This flush, added in libpng-1.0.8, removed from libpng-1.0.9beta03,
    * and restored again in libpng-1.2.30, may cause some applications that
    * do not set png_ptr->output_flush_fn to crash.  If your application
    * experiences a problem, please try building libpng with
    * PNG_WRITE_FLUSH_AFTER_IEND_SUPPORTED defined, and report the event to
    * png-mng-implement at lists.sf.net .
    */
#  ifdef PNG_WRITE_FLUSH_SUPPORTED
#     ifdef PNG_WRITE_FLUSH_AFTER_IEND_SUPPORTED
         png_flush(png_ptr);
#     endif
#  endif
}

#ifdef PNG_CONVERT_tIME_SUPPORTED
void PNGAPI
png_convert_from_struct_tm(png_timep ptime, PNG_CONST struct tm * ttime)
{
   png_debug(1, "in png_convert_from_struct_tm");

   ptime->year = png_check_u16(0/*TODO: fixme*/, 1900 + ttime->tm_year);
   ptime->month = png_check_byte(0/*TODO: fixme*/, ttime->tm_mon + 1);
   ptime->day = png_check_byte(0/*TODO: fixme*/, ttime->tm_mday);
   ptime->hour = png_check_byte(0/*TODO: fixme*/, ttime->tm_hour);
   ptime->minute = png_check_byte(0/*TODO: fixme*/, ttime->tm_min);
   ptime->second = png_check_byte(0/*TODO: fixme*/, ttime->tm_sec);
}

void PNGAPI
png_convert_from_time_t(png_timep ptime, time_t ttime)
{
   struct tm *tbuf;

   png_debug(1, "in png_convert_from_time_t");

   tbuf = gmtime(&ttime);
   png_convert_from_struct_tm(ptime, tbuf);
}
#endif

/* Initialize png_ptr structure, and allocate any memory needed */
PNG_FUNCTION(png_structp,PNGAPI
png_create_write_struct,(png_const_charp user_png_ver, png_voidp error_ptr,
    png_error_ptr error_fn, png_error_ptr warn_fn),PNG_ALLOCATED)
{
#ifndef PNG_USER_MEM_SUPPORTED
   png_structrp png_ptr = png_create_png_struct(user_png_ver, error_ptr,
       error_fn, warn_fn, NULL, NULL, NULL);
#else
   return png_create_write_struct_2(user_png_ver, error_ptr, error_fn,
       warn_fn, NULL, NULL, NULL);
}

/* Alternate initialize png_ptr structure, and allocate any memory needed */
PNG_FUNCTION(png_structp,PNGAPI
png_create_write_struct_2,(png_const_charp user_png_ver, png_voidp error_ptr,
    png_error_ptr error_fn, png_error_ptr warn_fn, png_voidp mem_ptr,
    png_malloc_ptr malloc_fn, png_free_ptr free_fn),PNG_ALLOCATED)
{
   png_structrp png_ptr = png_create_png_struct(user_png_ver, error_ptr,
       error_fn, warn_fn, mem_ptr, malloc_fn, free_fn);
#endif /* USER_MEM */

   if (png_ptr != NULL)
   {
      /* Set the zlib control values to defaults; they can be overridden by the
       * application after the struct has been created.
       */
      png_ptr->zbuffer_size = PNG_ZBUF_SIZE;

      /* The 'zlib_strategy' setting is irrelevant because png_default_claim in
       * pngwutil.c defaults it according to whether or not filters will be
       * used, and ignores this setting.
       */
      png_ptr->zlib_strategy = PNG_Z_DEFAULT_STRATEGY;
      png_ptr->zlib_level = PNG_Z_DEFAULT_COMPRESSION;
      png_ptr->zlib_mem_level = 8;
      png_ptr->zlib_window_bits = 15;
      png_ptr->zlib_method = 8;

#ifdef PNG_WRITE_COMPRESSED_TEXT_SUPPORTED
      png_ptr->zlib_text_strategy = PNG_TEXT_Z_DEFAULT_STRATEGY;
      png_ptr->zlib_text_level = PNG_TEXT_Z_DEFAULT_COMPRESSION;
      png_ptr->zlib_text_mem_level = 8;
      png_ptr->zlib_text_window_bits = 15;
      png_ptr->zlib_text_method = 8;
#endif /* WRITE_COMPRESSED_TEXT */

      /* This is a highly dubious configuration option; by default it is off,
       * but it may be appropriate for private builds that are testing
       * extensions not conformant to the current specification, or of
       * applications that must not fail to write at all costs!
       */
#ifdef PNG_BENIGN_WRITE_ERRORS_SUPPORTED
      /* In stable builds only warn if an application error can be completely
       * handled.
       */
      png_ptr->flags |= PNG_FLAG_BENIGN_ERRORS_WARN;
#endif

      /* App warnings are warnings in release (or release candidate) builds but
       * are errors during development.
       */
#if PNG_RELEASE_BUILD
      png_ptr->flags |= PNG_FLAG_APP_WARNINGS_WARN;
#endif
   }

   return png_ptr;
}


/* Write a few rows of image data.  If the image is interlaced,
 * either you will have to write the 7 sub images, or, if you
 * have called png_set_interlace_handling(), you will have to
 * "write" the image seven times.
 */
void PNGAPI
png_write_rows(png_structrp png_ptr, png_bytepp row,
    png_uint_32 num_rows)
{
   png_debug(1, "in png_write_rows");

   if (png_ptr == NULL || row == NULL)
      return;

   /* Loop through the rows */
   while (num_rows-- > 0)
      png_write_row(png_ptr, *row++);
}

/* Write the image.  You only need to call this function once, even
 * if you are writing an interlaced image.
 */
void PNGAPI
png_write_image(png_structrp png_ptr, png_bytepp image)
{
   int num_pass; /* pass variables */

   if (png_ptr == NULL || image == NULL)
      return;

   png_debug(1, "in png_write_image");

#ifdef PNG_WRITE_INTERLACING_SUPPORTED
   /* Initialize interlace handling.  If image is not interlaced,
    * this will set pass to 1
    */
   num_pass = png_set_interlace_handling(png_ptr);
#else
   num_pass = 1;

   if (png_ptr->interlaced)
   {
      png_app_error(png_ptr, "no interlace support");
      return;
   }
#endif

   /* Loop through passes */
   while (num_pass-- > 0)
   {
      png_bytepp  rp = image; /* points to current row */
      png_uint_32 num_rows = png_ptr->height;

      /* Loop through image */
      while (num_rows-- > 0)
         png_write_row(png_ptr, *rp++);
   }
}

/* Called to advance to the next row.  A row may not have been output when
 * libpng handles the interlace passes because the app hands libpng every image
 * row for every pass.
 *
 * This function also writes the current row, if the pointer to the bytes is
 * non-NULL, and does so with the appropriate 'flush' argument to zlib.
 */
static void
png_write_finish_row(png_structrp png_ptr, png_byte filter_byte,
   png_const_bytep row, png_alloc_size_t row_bytes)
{
   const png_uint_32 height = png_ptr->height;
   png_uint_32 row_number = png_ptr->row_number;
   int flush = Z_NO_FLUSH;

   png_debug(1, "in png_write_finish_row");

   if (png_ptr->interlaced)
   {
      unsigned int pass = png_ptr->pass;

#     ifdef PNG_WRITE_INTERLACING_SUPPORTED
         if (png_ptr->do_interlace)
         {
            /* libpng is doing the de-interlace. */
            /* Z_FINISH must be set on the last row present in the image, not
             * the actual last row.
             *
             * NOTE: this means that the application need not call libpng all
             * the way to the end of the image, but this is double checked
             * below in png_write_end where png_ptr->pass is checked.
             */
            if (pass == PNG_LAST_PASS(png_ptr->width, height) &&
                PNG_LAST_PASS_ROW(row_number, pass, height) &&
                PNG_ROW_IN_INTERLACE_PASS(row_number, pass))
               flush = Z_FINISH; /* end of image */

            if (++row_number == height)
            {
               ++pass;
               row_number = 0;
               png_ptr->pass = pass & 0x7;
            }
         } /* libpng doing interlace */

         else /* app is doing the interlacing */
#     endif /* WRITE_INTERLACING */
      /* The application has to hand us interlaced rows.  In this case
       * row_number is the row number in the pass (this is not the
       * behavior in the read code, where it is always the row number in
       * the image.)
       *
       * Note that for any image row 0 of pass 0 is always present, so the
       * check after the 'if' is not required at the start.
       */
      {
         affirm(row != NULL);

         if (++row_number == PNG_PASS_ROWS(height, pass))
         {
            const png_uint_32 width = png_ptr->width;

            /* Next pass, but it may not be present because of the width. */
            row_number = 0;

            for (;;)
            {
               if (++pass == PNG_INTERLACE_ADAM7_PASSES)
               {
                  flush = Z_FINISH;
                  break; /* end of image */
               }

               if (PNG_PASS_IN_IMAGE(width, height, pass))
                  break;
            }

            png_ptr->pass = pass & 0x7;
         } /* end of pass */
      } /* app doing interlace */
   } /* writing an interlaced PNG */

   else /* PNG not interlaced */ if (++row_number == height)
      flush = Z_FINISH;

   png_ptr->row_number = row_number;

   if (row != NULL)
   {
      png_compress_IDAT(png_ptr, &filter_byte, 1, Z_NO_FLUSH);
      png_compress_IDAT(png_ptr, row, row_bytes, flush);

#     ifdef PNG_WRITE_FLUSH_SUPPORTED
         if (flush == Z_NO_FLUSH &&
             ++png_ptr->flush_rows >= png_ptr->flush_dist &&
             png_ptr->flush_dist > 0)
            png_write_flush(png_ptr);
#     endif /* WRITE_FLUSH */
   }

   /* The calculations above should ensure that Z_FINISH is set on the last row
    * written, the following rows are empty pass rows, so the deflate stream
    * should already have been closed:
    */
   else
      affirm(flush == Z_NO_FLUSH || png_ptr->zowner == 0);
}

/* Called by user to write a row of image data */
void PNGAPI
png_write_row(png_structrp png_ptr, png_const_bytep row)
{
   png_uint_32 row_number, row_width;
   unsigned int pass;
#  ifdef PNG_WRITE_FILTER_SUPPORTED
      int last_pass_row, first_pass_row;
#  endif

   if (png_ptr == NULL)
      return;

   png_debug2(1, "in png_write_row (row %u, pass %d)",
      png_ptr->row_number, png_ptr->pass);

   row_number = png_ptr->row_number;
   row_width = png_ptr->width;
   pass = png_ptr->pass;

   /* Unlike the read code initialization happens automatically: */
   if (row_number == 0 && pass == 0)
   {
      png_init_row_info(png_ptr); /* doesn't change row/pass/width */

      /* If the app takes a png_info from a read operation and if the app has
       * performed transforms on the data the png_info can contain IHDR
       * information that cannot be represented in PNG.  The code that writes
       * the IHDR takes the color type from the png_info::format.  The app adds
       * transforms, before or after writing the IHDR, then the IHDR color_type
       * stored in png_struct::color_type is used in png_init_row_info above to
       * work out the actual row format.
       *
       * Prior to 1.7.0 this was not verified (there was no easy way to do so).
       * Now we can check it here, however this is an:
       *
       * API CHANGE: in 1.7.0 an error may be flagged against bogus info_struct
       * formats even though the app had removed them itself.  It's just a
       * warning at present.
       */
#     ifdef PNG_WRITE_TRANSFORMS_SUPPORTED
         /* The test is that either the row_format produced by the write
          * transforms exactly matches that in the original info_struct::format
          * or that the info_struct::format was a simple mapping of the
          * color_type that ended up in the IHDR:
          */
         if (png_ptr->row_format != png_ptr->info_format &&
             PNG_FORMAT_FROM_COLOR_TYPE(png_ptr->color_type) !=
               png_ptr->info_format)
            png_app_warning(png_ptr, "info_struct format does not match IHDR");
#     endif /* WRITE_TRANSFORMS */
   }

   else if (row_number >= png_ptr->height || pass >= PNG_INTERLACE_ADAM7_PASSES)
      png_error(png_ptr, "Too many calls to png_write_row");

   /* If interlaced and not interested in row, return.  Note that the
    * assumption here, as in the read code, is that if the app wants to write an
    * interlaced image when libpng does not support WRITE_INTERLACING the app
    * will only provide the rows actually in the pass.
    */
   if (png_ptr->interlaced)
   {
#     ifdef PNG_WRITE_INTERLACING_SUPPORTED
         if (png_ptr->do_interlace)
         {
            /* libpng is doing the de-interlace. */
            if (!PNG_ROW_IN_INTERLACE_PASS(row_number, pass) ||
                PNG_PASS_COLS(row_width, pass) == 0)
            {
               /* Not in the pass; advance to the next row.  Notice that because
                * the app is expected to call us once for every image row in
                * every pass it is sufficient to just add one to row_number
                * here.
                */
               png_write_finish_row(png_ptr, 0, NULL, 0);
               return;
            }

            /* Else: this row must be output, row_number is the row in the
             * image.
             */
            last_pass_row =
               PNG_LAST_PASS_ROW(row_number, pass, png_ptr->height);
            first_pass_row = row_number == PNG_PASS_START_ROW(pass);
            debug(row_number >= PNG_PASS_START_ROW(pass));
         }

         else /* app is doing the interlacing */
#     endif /* WRITE_INTERLACING */
      {
         /* The interlaced rows come from the application and they have the
          * correct width, row_number is the row number in the pass, not the
          * number of the corresponding (expanded) image row.
          */
         row_width = PNG_PASS_COLS(row_width, pass);
#        ifdef PNG_WRITE_FILTER_SUPPORTED
            last_pass_row =
               row_number+1 >= PNG_PASS_ROWS(png_ptr->height, pass);
            first_pass_row = row_number == 0;
#        endif /* WRITE_INTERLACING */
      }
   } /* writing an interlaced PNG */

#  ifdef PNG_WRITE_FILTER_SUPPORTED
      else /* not an interlaced PNG */
      {
         last_pass_row = row_number+1 >= png_ptr->height;
         first_pass_row = row_number == 0;
      }
#  endif /* WRITE_INTERLACING */

   /* 1.7.0: pretty much everything except the PNG row filter happens under the
    * control of the transform code.
    *
    * png_struct::row_format is the *input* row format.  During the transforms
    * the png_transform_control::{sp,dp} pointers are used in the normal fashion
    * with dp initially set to png_struct::row_buffer.
    *
    * After the transforms if there is no filtering to be done (WRITE_FILTER is
    * not supported) 'sp' is written directly; it may be png_struct::row_buffer
    * or it may be the original 'row' parameter to this routine.  There is no
    * need to save the transformed (PNG format) row.
    *
    * If there is filtering to be done then either the original row or
    * png_transform_control::sp is filtered against the previous row, which is
    * in png_struct::alt_buffer, the result is written with the appropriate
    * filter byte and then the original row or png_transform_control::sp is
    * saved to png_struct::alt_buffer.
    *
    * Thus there are four control flow possibilities depending on the pair of
    * compile time flags [WRITE_TRANSFORM_MECH,WRITE_FILTER].  The simplest
    * write code, which requires no internal buffer, arises when both are
    * compiled out.  alt_buffer is only required if WRITE_FILTER is supported
    * and row_buffer is required when either are supported.
    */
   {
      png_byte filter_byte;
      unsigned int output_bpp; /* bits per pixel */
      png_const_bytep output_row;
      png_alloc_size_t output_bytes;

#     ifdef PNG_TRANSFORM_MECH_SUPPORTED
         if (png_ptr->transform_list != NULL) /* else no transforms */
         {
            png_transform_control tc;

            /* The initial values are the memory format, this was worked out in 
             * png_init_row_info above.
             */
            memset(&tc, 0, sizeof tc);
            tc.png_ptr = png_ptr;
            tc.sp = row;
            tc.dp = png_ptr->row_buffer;

            if (tc.dp == NULL)
               png_ptr->row_buffer = png_voidcast(png_bytep, tc.dp =
                  png_malloc(png_ptr, png_ptr->row_allocated_bytes));

            tc.width = row_width; /* width of interlaced row */
            tc.format = png_ptr->row_format;
            tc.range = png_ptr->row_range;
            tc.bit_depth = png_ptr->row_bit_depth;
            /* tc.init == 0 */
            /* tc.caching: not used */
            /* tc.palette: not used */

            /* Run the list. */
            png_run_transform_list_backwards(png_ptr, &tc);

            /* Make sure the format that resulted is compatible with PNG: */
            affirm((tc.format & PNG_BIC_MASK(PNG_FORMAT_FLAG_ALPHA +
               PNG_FORMAT_FLAG_COLOR + PNG_FORMAT_FLAG_LINEAR +
               PNG_FORMAT_FLAG_COLORMAP)) == 0);

            /* Now we must have the PNG format from the IHDR: */
            affirm(png_ptr->bit_depth == tc.bit_depth &&
               png_ptr->color_type == PNG_COLOR_TYPE_FROM_FORMAT(tc.format));

            /* If libpng is handling the interlacing the row width must now
             * match the width required for this pass.
             */
            affirm(tc.width == (!png_ptr->do_interlace ?
               row_width : PNG_PASS_COLS(row_width, pass)));

            row_width = tc.width;
            output_row = png_voidcast(png_const_bytep, tc.sp);
         }

         else /* no transforms */
#     endif /* TRANSFORM_MECH */
         output_row = row;

      output_bpp = PNG_PIXEL_DEPTH(*png_ptr);
      output_bytes = PNG_ROWBYTES(output_bpp, row_width);

#     ifdef PNG_WRITE_FILTER_SUPPORTED
         {
            png_const_bytep unfiltered_row = output_row;
            png_bytep alt_buffer = png_ptr->alt_buffer;

            /* If necessary find an appropriate filter and apply it to get the
             * filtered row.  The function may return the original argument, it
             * fills in 'filter_byte' appropriately.
             */
            if (png_ptr->next_filter != PNG_FILTER_NONE)
               output_row = png_write_filter_row(png_ptr, output_row,
                  first_pass_row, alt_buffer, output_bytes,
                  (output_bpp+7)>>3 /* bytes per pixel */, &filter_byte);

            else
               filter_byte = PNG_FILTER_VALUE_NONE;

            /* If the available filters require it, or ever did (as evidenced by
             * the presence of 'alt_buffer', store the unfiltered row in
             * alt_buffer.  Note that this does not happen on the last row of a
             * pass, or the image.
             */
            if (!last_pass_row && (alt_buffer != NULL || (png_ptr->next_filter &
                 (PNG_FILTER_UP+PNG_FILTER_AVG+PNG_FILTER_PAETH)) != 0))
            {
               if (unfiltered_row == row) /* Must be copied */
               {
                  if (alt_buffer == NULL)
                     png_ptr->alt_buffer = alt_buffer = png_voidcast(png_bytep,
                        png_malloc(png_ptr, png_ptr->row_allocated_bytes));

                  memcpy(alt_buffer, unfiltered_row, output_bytes);
               }

               else /* Can be swapped */
               {
                  png_bytep tmp = png_ptr->row_buffer;

                  affirm(unfiltered_row == tmp);
                  png_ptr->row_buffer = alt_buffer; /* may be NULL */
                  png_ptr->alt_buffer = tmp;
               }
            }
         }
#     else /* !WRITE_FILTER: no previous row to store */
         filter_byte = PNG_FILTER_VALUE_NONE;
#     endif /* !WRITE_FILTER */

      png_write_finish_row(png_ptr, filter_byte, output_row, output_bytes);
   }

   /* API CHANGE: 1.7.0: this is now called after png_struct::row_number and
    * png_struct::pass have been updated and, at the end of the image, after the
    * deflate stream has been closed.  The order of the call with respect to the
    * flush operation has also changed.  The callback can't discover any of this
    * unless it relies on the write callbacks to find the row data, and that was
    * never predictable.
    *
    *
    * TODO: API CHANGE: pass the row bytes to this function, it would be more
    * useful.
    */
   if (png_ptr->write_row_fn != NULL)
      (*(png_ptr->write_row_fn))(png_ptr, row_number, pass);
}

#ifdef PNG_WRITE_FLUSH_SUPPORTED
/* Set the automatic flush interval or 0 to turn flushing off */
void PNGAPI
png_set_flush(png_structrp png_ptr, int nrows)
{
   png_debug(1, "in png_set_flush");

   if (png_ptr == NULL)
      return;

   png_ptr->flush_dist = (nrows < 0 ? 0 : nrows);
}

/* Flush the current output buffers now */
void PNGAPI
png_write_flush(png_structrp png_ptr)
{
   png_debug(1, "in png_write_flush");

   if (png_ptr == NULL)
      return;

   /* Before the start of the IDAT and after the end of the image zowner will be
    * something other than png_IDAT:
    */
   if (png_ptr->zowner == png_IDAT)
   {
      png_compress_IDAT(png_ptr, NULL, 0, Z_SYNC_FLUSH);
      png_ptr->flush_rows = 0;
      png_flush(png_ptr);
   }
}
#endif /* WRITE_FLUSH */

/* Free any memory used in png_ptr struct without freeing the struct itself. */
static void
png_write_destroy(png_structrp png_ptr)
{
   png_debug(1, "in png_write_destroy");

   /* Free any memory zlib uses */
   if (png_ptr->zstream.state != NULL)
   {
      int ret = deflateEnd(&png_ptr->zstream);

      if (ret != Z_OK)
      {
         png_zstream_error(png_ptr, ret);
         png_warning(png_ptr, png_ptr->zstream.msg);
      }
   }

   /* Free our memory.  png_free checks NULL for us. */
   png_free_buffer_list(png_ptr, &png_ptr->zbuffer_list);
   png_free(png_ptr, png_ptr->row_buffer);
   png_ptr->row_buffer = NULL;
#ifdef PNG_WRITE_FILTER_SUPPORTED
   png_free(png_ptr, png_ptr->alt_buffer);
   png_ptr->alt_buffer = NULL;
   png_free(png_ptr, png_ptr->write_row[0]);
   png_ptr->write_row[0] = NULL;
   png_free(png_ptr, png_ptr->write_row[1]);
   png_ptr->write_row[1] = NULL;
#endif

#ifdef PNG_TRANSFORM_MECH_SUPPORTED
   png_transform_free(png_ptr, &png_ptr->transform_list);
#endif

#ifdef PNG_SET_UNKNOWN_CHUNKS_SUPPORTED
   png_free(png_ptr, png_ptr->chunk_list);
   png_ptr->chunk_list = NULL;
#endif

   /* The error handling and memory handling information is left intact at this
    * point: the jmp_buf may still have to be freed.  See png_destroy_png_struct
    * for how this happens.
    */
}

/* Free all memory used by the write.
 * In libpng 1.6.0 this API changed quietly to no longer accept a NULL value for
 * *png_ptr_ptr.  Prior to 1.6.0 it would accept such a value and it would free
 * the passed in info_structs but it would quietly fail to free any of the data
 * inside them.  In 1.6.0 it quietly does nothing (it has to be quiet because it
 * has no png_ptr.)
 */
void PNGAPI
png_destroy_write_struct(png_structpp png_ptr_ptr, png_infopp info_ptr_ptr)
{
   png_debug(1, "in png_destroy_write_struct");

   if (png_ptr_ptr != NULL)
   {
      png_structrp png_ptr = *png_ptr_ptr;

      if (png_ptr != NULL) /* added in libpng 1.6.0 */
      {
         png_destroy_info_struct(png_ptr, info_ptr_ptr);

         *png_ptr_ptr = NULL;
         png_write_destroy(png_ptr);
         png_destroy_png_struct(png_ptr);
      }
   }
}

#ifdef PNG_WRITE_FILTER_SUPPORTED
/* Allow the application to select one or more row filters to use. */
void PNGAPI
png_set_filter(png_structrp png_ptr, int method, int filters)
{
   png_debug(1, "in png_set_filter");

   if (png_ptr == NULL)
      return;

   if (png_ptr->read_struct)
   {
      png_app_error(png_ptr, "png_set_filter: cannot be used when reading");
      return;
   }

   if (method != png_ptr->filter_method)
   {
      png_app_error(png_ptr, "png_set_filter: method does not match IHDR");
      return;
   }

   /* PNG and MNG use the same base adaptive filter types: */
   if (method != PNG_FILTER_TYPE_BASE && method != PNG_INTRAPIXEL_DIFFERENCING)
   {
      png_app_error(png_ptr, "png_set_filter: unsupported method");
      return;
   }

   /* Notice that PNG_NO_FILTERS is 0 and passes this test; this is OK
    * because filters then gets set to PNG_FILTER_NONE, as is required.
    */
   if (filters < PNG_FILTER_VALUE_LAST)
      filters = 0x08 << filters;

   else if ((filters & PNG_BIC_MASK(PNG_ALL_FILTERS)) != 0)
   {
      png_app_error(png_ptr, "png_set_filter: invalid filters mask/value");

      /* Prior to 1.7.0 this ignored the error and just used the bits that are
       * present, now it does nothing; this seems a lot safer.
       */
      return;
   }

   /* Finally store the value.  */
   png_ptr->next_filter = PNG_BYTE(filters);
}
#endif /* WRITE_FILTER */

#ifdef PNG_WRITE_WEIGHTED_FILTER_SUPPORTED      /* GRR 970116 */
/* Legacy API that weighted the filter metric by the number of times it had been
 * used before.
 */
#ifdef PNG_FLOATING_POINT_SUPPORTED
PNG_FUNCTION(void,PNGAPI
png_set_filter_heuristics,(png_structrp png_ptr, int heuristic_method,
    int num_weights, png_const_doublep filter_weights,
    png_const_doublep filter_costs),PNG_DEPRECATED)
{
   png_app_warning(png_ptr, "weighted filter heuristics not implemented");
   PNG_UNUSED(heuristic_method)
   PNG_UNUSED(num_weights)
   PNG_UNUSED(filter_weights)
   PNG_UNUSED(filter_costs)
}
#endif /* FLOATING_POINT */

#ifdef PNG_FIXED_POINT_SUPPORTED
PNG_FUNCTION(void,PNGAPI
png_set_filter_heuristics_fixed,(png_structrp png_ptr, int heuristic_method,
    int num_weights, png_const_fixed_point_p filter_weights,
    png_const_fixed_point_p filter_costs),PNG_DEPRECATED)
{
   png_app_warning(png_ptr, "weighted filter heuristics not implemented");
   PNG_UNUSED(heuristic_method)
   PNG_UNUSED(num_weights)
   PNG_UNUSED(filter_weights)
   PNG_UNUSED(filter_costs)
}
#endif /* FIXED_POINT */
#endif /* WRITE_WEIGHTED_FILTER */

#ifdef PNG_WRITE_CUSTOMIZE_COMPRESSION_SUPPORTED
void PNGAPI
png_set_compression_level(png_structrp png_ptr, int level)
{
   png_debug(1, "in png_set_compression_level");

   if (png_ptr == NULL)
      return;

   png_ptr->zlib_level = level;
}

void PNGAPI
png_set_compression_mem_level(png_structrp png_ptr, int mem_level)
{
   png_debug(1, "in png_set_compression_mem_level");

   if (png_ptr == NULL)
      return;

   png_ptr->zlib_mem_level = mem_level;
}

void PNGAPI
png_set_compression_strategy(png_structrp png_ptr, int strategy)
{
   png_debug(1, "in png_set_compression_strategy");

   if (png_ptr == NULL)
      return;

   /* The flag setting here prevents the libpng dynamic selection of strategy.
    */
   png_ptr->flags |= PNG_FLAG_ZLIB_CUSTOM_STRATEGY;
   png_ptr->zlib_strategy = strategy;
}

/* If PNG_WRITE_OPTIMIZE_CMF_SUPPORTED is defined, libpng will use a
 * smaller value of window_bits if it can do so safely.
 */
void PNGAPI
png_set_compression_window_bits(png_structrp png_ptr, int window_bits)
{
   if (png_ptr == NULL)
      return;

   /* Prior to 1.6.0 this would warn but then set the window_bits value. This
    * meant that negative window bits values could be selected that would cause
    * libpng to write a non-standard PNG file with raw deflate or gzip
    * compressed IDAT or ancillary chunks.  Such files can be read and there is
    * no warning on read, so this seems like a very bad idea.
    */
   if (window_bits > 15)
   {
      png_warning(png_ptr, "Only compression windows <= 32k supported by PNG");
      window_bits = 15;
   }

   else if (window_bits < 8)
   {
      png_warning(png_ptr, "Only compression windows >= 256 supported by PNG");
      window_bits = 8;
   }

   png_ptr->zlib_window_bits = window_bits;
}

void PNGAPI
png_set_compression_method(png_structrp png_ptr, int method)
{
   png_debug(1, "in png_set_compression_method");

   if (png_ptr == NULL)
      return;

   /* This would produce an invalid PNG file if it worked, but it doesn't and
    * deflate will fault it, so it is harmless to just warn here.
    */
   if (method != 8)
      png_warning(png_ptr, "Only compression method 8 is supported by PNG");

   png_ptr->zlib_method = method;
}
#endif /* WRITE_CUSTOMIZE_COMPRESSION */

/* The following were added to libpng-1.5.4 */
#ifdef PNG_WRITE_CUSTOMIZE_ZTXT_COMPRESSION_SUPPORTED
void PNGAPI
png_set_text_compression_level(png_structrp png_ptr, int level)
{
   png_debug(1, "in png_set_text_compression_level");

   if (png_ptr == NULL)
      return;

   png_ptr->zlib_text_level = level;
}

void PNGAPI
png_set_text_compression_mem_level(png_structrp png_ptr, int mem_level)
{
   png_debug(1, "in png_set_text_compression_mem_level");

   if (png_ptr == NULL)
      return;

   png_ptr->zlib_text_mem_level = mem_level;
}

void PNGAPI
png_set_text_compression_strategy(png_structrp png_ptr, int strategy)
{
   png_debug(1, "in png_set_text_compression_strategy");

   if (png_ptr == NULL)
      return;

   png_ptr->zlib_text_strategy = strategy;
}

/* If PNG_WRITE_OPTIMIZE_CMF_SUPPORTED is defined, libpng will use a
 * smaller value of window_bits if it can do so safely.
 */
void PNGAPI
png_set_text_compression_window_bits(png_structrp png_ptr, int window_bits)
{
   if (png_ptr == NULL)
      return;

   if (window_bits > 15)
   {
      png_warning(png_ptr, "Only compression windows <= 32k supported by PNG");
      window_bits = 15;
   }

   else if (window_bits < 8)
   {
      png_warning(png_ptr, "Only compression windows >= 256 supported by PNG");
      window_bits = 8;
   }

   png_ptr->zlib_text_window_bits = window_bits;
}

void PNGAPI
png_set_text_compression_method(png_structrp png_ptr, int method)
{
   png_debug(1, "in png_set_text_compression_method");

   if (png_ptr == NULL)
      return;

   if (method != 8)
      png_warning(png_ptr, "Only compression method 8 is supported by PNG");

   png_ptr->zlib_text_method = method;
}
#endif /* WRITE_CUSTOMIZE_ZTXT_COMPRESSION */
/* end of API added to libpng-1.5.4 */

void PNGAPI
png_set_write_status_fn(png_structrp png_ptr, png_write_status_ptr write_row_fn)
{
   if (png_ptr == NULL)
      return;

   png_ptr->write_row_fn = write_row_fn;
}

#ifdef PNG_WRITE_PNG_SUPPORTED
void PNGAPI
png_write_png(png_structrp png_ptr, png_inforp info_ptr,
    int transforms, voidp params)
{
   if (png_ptr == NULL || info_ptr == NULL)
      return;

   if ((info_ptr->valid & PNG_INFO_IDAT) == 0)
   {
      png_app_error(png_ptr, "no rows for png_write_image to write");
      return;
   }

   /* Write the file header information. */
   png_write_info(png_ptr, info_ptr);

   /* ------ these transformations don't touch the info structure ------- */

   /* Invert monochrome pixels */
   if ((transforms & PNG_TRANSFORM_INVERT_MONO) != 0)
#ifdef PNG_WRITE_INVERT_SUPPORTED
      png_set_invert_mono(png_ptr);
#else
      png_app_error(png_ptr, "PNG_TRANSFORM_INVERT_MONO not supported");
#endif

   /* Shift the pixels up to a legal bit depth and fill in
    * as appropriate to correctly scale the image.
    */
   if ((transforms & PNG_TRANSFORM_SHIFT) != 0)
#ifdef PNG_WRITE_SHIFT_SUPPORTED
      if ((info_ptr->valid & PNG_INFO_sBIT) != 0)
         png_set_shift(png_ptr, &info_ptr->sig_bit);
#else
      png_app_error(png_ptr, "PNG_TRANSFORM_SHIFT not supported");
#endif

   /* Pack pixels into bytes */
   if ((transforms & PNG_TRANSFORM_PACKING) != 0)
#ifdef PNG_WRITE_PACK_SUPPORTED
      png_set_packing(png_ptr);
#else
      png_app_error(png_ptr, "PNG_TRANSFORM_PACKING not supported");
#endif

   /* Swap location of alpha bytes from ARGB to RGBA */
   if ((transforms & PNG_TRANSFORM_SWAP_ALPHA) != 0)
#ifdef PNG_WRITE_SWAP_ALPHA_SUPPORTED
      png_set_swap_alpha(png_ptr);
#else
      png_app_error(png_ptr, "PNG_TRANSFORM_SWAP_ALPHA not supported");
#endif

   /* Remove a filler (X) from XRGB/RGBX/AG/GA into to convert it into
    * RGB, note that the code expects the input color type to be G or RGB; no
    * alpha channel.
    */
   if ((transforms & (PNG_TRANSFORM_STRIP_FILLER_AFTER|
      PNG_TRANSFORM_STRIP_FILLER_BEFORE)) != 0)
   {
#ifdef PNG_WRITE_FILLER_SUPPORTED
      if ((transforms & PNG_TRANSFORM_STRIP_FILLER_AFTER) != 0)
      {
         if ((transforms & PNG_TRANSFORM_STRIP_FILLER_BEFORE) != 0)
            png_app_error(png_ptr,
               "PNG_TRANSFORM_STRIP_FILLER: BEFORE+AFTER not supported");

         /* Continue if ignored - this is the pre-1.6.10 behavior */
         png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);
      }

      else if ((transforms & PNG_TRANSFORM_STRIP_FILLER_BEFORE) != 0)
         png_set_filler(png_ptr, 0, PNG_FILLER_BEFORE);
#else
      png_app_error(png_ptr, "PNG_TRANSFORM_STRIP_FILLER not supported");
#endif
   }

   /* Flip BGR pixels to RGB */
   if ((transforms & PNG_TRANSFORM_BGR) != 0)
#ifdef PNG_WRITE_BGR_SUPPORTED
      png_set_bgr(png_ptr);
#else
      png_app_error(png_ptr, "PNG_TRANSFORM_BGR not supported");
#endif

   /* Swap bytes of 16-bit files to most significant byte first */
   if ((transforms & PNG_TRANSFORM_SWAP_ENDIAN) != 0)
#ifdef PNG_WRITE_SWAP_SUPPORTED
      png_set_swap(png_ptr);
#else
      png_app_error(png_ptr, "PNG_TRANSFORM_SWAP_ENDIAN not supported");
#endif

   /* Swap bits of 1, 2, 4 bit packed pixel formats */
   if ((transforms & PNG_TRANSFORM_PACKSWAP) != 0)
#ifdef PNG_WRITE_PACKSWAP_SUPPORTED
      png_set_packswap(png_ptr);
#else
      png_app_error(png_ptr, "PNG_TRANSFORM_PACKSWAP not supported");
#endif

   /* Invert the alpha channel from opacity to transparency */
   if ((transforms & PNG_TRANSFORM_INVERT_ALPHA) != 0)
#ifdef PNG_WRITE_INVERT_ALPHA_SUPPORTED
      png_set_invert_alpha(png_ptr);
#else
      png_app_error(png_ptr, "PNG_TRANSFORM_INVERT_ALPHA not supported");
#endif

   /* ----------------------- end of transformations ------------------- */

   /* Write the bits */
   png_write_image(png_ptr, info_ptr->row_pointers);

   /* It is REQUIRED to call this to finish writing the rest of the file */
   png_write_end(png_ptr, info_ptr);

   PNG_UNUSED(params)
}
#endif /* WRITE_PNG */


#ifdef PNG_SIMPLIFIED_WRITE_SUPPORTED
#ifdef PNG_STDIO_SUPPORTED /* currently required for png_image_write_* */
/* Initialize the write structure - general purpose utility. */
static int
png_image_write_init(png_imagep image)
{
   png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, image,
          png_safe_error, png_safe_warning);

   if (png_ptr != NULL)
   {
      png_infop info_ptr = png_create_info_struct(png_ptr);

      if (info_ptr != NULL)
      {
         png_controlp control = png_voidcast(png_controlp,
            png_malloc_warn(png_ptr, (sizeof *control)));

         if (control != NULL)
         {
            memset(control, 0, (sizeof *control));

            control->png_ptr = png_ptr;
            control->info_ptr = info_ptr;
            control->for_write = 1;

            image->opaque = control;
            return 1;
         }

         /* Error clean up */
         png_destroy_info_struct(png_ptr, &info_ptr);
      }

      png_destroy_write_struct(&png_ptr, NULL);
   }

   return png_image_error(image, "png_image_write_: out of memory");
}

/* Arguments to png_image_write_main: */
typedef struct
{
   /* Arguments: */
   png_imagep      image;
   png_const_voidp buffer;
   png_int_32      row_stride;
   png_const_voidp colormap;
   int             convert_to_8bit;
   /* Local variables: */
   png_const_voidp first_row;
   ptrdiff_t       row_bytes;
   png_voidp       local_row;
} png_image_write_control;

/* Write png_uint_16 input to a 16-bit PNG; the png_ptr has already been set to
 * do any necessary byte swapping.  The component order is defined by the
 * png_image format value.
 */
static int
png_write_image_16bit(png_voidp argument)
{
   png_image_write_control *display = png_voidcast(png_image_write_control*,
      argument);
   png_imagep image = display->image;
   png_structrp png_ptr = image->opaque->png_ptr;

   png_const_uint_16p input_row = png_voidcast(png_const_uint_16p,
      display->first_row);
   png_uint_16p output_row = png_voidcast(png_uint_16p, display->local_row);
   png_uint_16p row_end;
   const int channels = (image->format & PNG_FORMAT_FLAG_COLOR) != 0 ? 3 : 1;
   int aindex = 0;
   png_uint_32 y = image->height;

   if ((image->format & PNG_FORMAT_FLAG_ALPHA) != 0)
   {
#     ifdef PNG_SIMPLIFIED_WRITE_AFIRST_SUPPORTED
         if ((image->format & PNG_FORMAT_FLAG_AFIRST) != 0)
         {
            aindex = -1;
            ++input_row; /* To point to the first component */
            ++output_row;
         }

         else
#     endif
         aindex = channels;
   }

   else
      png_error(png_ptr, "png_write_image: internal call error");

   /* Work out the output row end and count over this, note that the increment
    * above to 'row' means that row_end can actually be beyond the end of the
    * row; this is correct.
    */
   row_end = output_row + image->width * (channels+1);

   while (y-- > 0)
   {
      png_const_uint_16p in_ptr = input_row;
      png_uint_16p out_ptr = output_row;

      while (out_ptr < row_end)
      {
         const png_uint_16 alpha = in_ptr[aindex];
         png_uint_32 reciprocal = 0;
         int c;

         out_ptr[aindex] = alpha;

         /* Calculate a reciprocal.  The correct calculation is simply
          * component/alpha*65535 << 15. (I.e. 15 bits of precision); this
          * allows correct rounding by adding .5 before the shift.  'reciprocal'
          * is only initialized when required.
          */
         if (alpha > 0 && alpha < 65535)
            reciprocal = ((0xffff<<15)+(alpha>>1))/alpha;

         c = channels;
         do /* always at least one channel */
         {
            png_uint_16 component = *in_ptr++;

            /* The following gives 65535 for an alpha of 0, which is fine,
             * otherwise if 0/0 is represented as some other value there is more
             * likely to be a discontinuity which will probably damage
             * compression when moving from a fully transparent area to a
             * nearly transparent one.  (The assumption here is that opaque
             * areas tend not to be 0 intensity.)
             */
            if (component >= alpha)
               component = 65535;

            /* component<alpha, so component/alpha is less than one and
             * component*reciprocal is less than 2^31.
             */
            else if (component > 0 && alpha < 65535)
            {
               png_uint_32 calc = component * reciprocal;
               calc += 16384; /* round to nearest */
               component = png_check_u16(png_ptr, calc >> 15);
            }

            *out_ptr++ = component;
         }
         while (--c > 0);

         /* Skip to next component (skip the intervening alpha channel) */
         ++in_ptr;
         ++out_ptr;
      }

      png_write_row(png_ptr, png_voidcast(png_const_bytep, display->local_row));
      input_row += display->row_bytes/(sizeof (png_uint_16));
   }

   return 1;
}

/* Given 16-bit input (1 to 4 channels) write 8-bit output.  If an alpha channel
 * is present it must be removed from the components, the components are then
 * written in sRGB encoding.  No components are added or removed.
 *
 * Calculate an alpha reciprocal to reverse pre-multiplication.  As above the
 * calculation can be done to 15 bits of accuracy; however, the output needs to
 * be scaled in the range 0..255*65535, so include that scaling here.
 */
#define UNP_RECIPROCAL(alpha) ((((0xffff*0xff)<<7)+(alpha>>1))/alpha)

static png_byte
png_unpremultiply(png_const_structrp png_ptr, png_uint_32 component,
   png_uint_32 alpha, png_uint_32 reciprocal/*from the above macro*/)
{
   /* The following gives 1.0 for an alpha of 0, which is fine, otherwise if 0/0
    * is represented as some other value there is more likely to be a
    * discontinuity which will probably damage compression when moving from a
    * fully transparent area to a nearly transparent one.  (The assumption here
    * is that opaque areas tend not to be 0 intensity.)
    *
    * There is a rounding problem here; if alpha is less than 128 it will end up
    * as 0 when scaled to 8 bits.  To avoid introducing spurious colors into the
    * output change for this too.
    */
   if (component >= alpha || alpha < 128)
      return 255;

   /* component<alpha, so component/alpha is less than one and
    * component*reciprocal is less than 2^31.
    */
   else if (component > 0)
   {
      /* The test is that alpha/257 (rounded) is less than 255, the first value
       * that becomes 255 is 65407.
       * NOTE: this must agree with the PNG_DIV257 macro (which must, therefore,
       * be exact!)  [Could also test reciprocal != 0]
       */
      if (alpha < 65407)
      {
         component *= reciprocal;
         component += 64; /* round to nearest */
         component >>= 7;
      }

      else
         component *= 255;

      /* Convert the component to sRGB. */
      return PNG_sRGB_FROM_LINEAR(png_ptr, component);
   }

   else
      return 0;

   PNG_UNUSEDRC(png_ptr)
}

static int
png_write_image_8bit(png_voidp argument)
{
   png_image_write_control *display = png_voidcast(png_image_write_control*,
      argument);
   png_imagep image = display->image;
   png_structrp png_ptr = image->opaque->png_ptr;

   png_const_uint_16p input_row = png_voidcast(png_const_uint_16p,
      display->first_row);
   png_bytep output_row = png_voidcast(png_bytep, display->local_row);
   png_uint_32 y = image->height;
   const int channels = (image->format & PNG_FORMAT_FLAG_COLOR) != 0 ? 3 : 1;

   if ((image->format & PNG_FORMAT_FLAG_ALPHA) != 0)
   {
      png_bytep row_end;
      int aindex;

#     ifdef PNG_SIMPLIFIED_WRITE_AFIRST_SUPPORTED
         if ((image->format & PNG_FORMAT_FLAG_AFIRST) != 0)
         {
            aindex = -1;
            ++input_row; /* To point to the first component */
            ++output_row;
         }

         else
#     endif
         aindex = channels;

      /* Use row_end in place of a loop counter: */
      row_end = output_row + image->width * (channels+1);

      while (y-- > 0)
      {
         png_const_uint_16p in_ptr = input_row;
         png_bytep out_ptr = output_row;

         while (out_ptr < row_end)
         {
            png_uint_16 alpha = in_ptr[aindex];
            png_byte alphabyte = png_check_byte(png_ptr, PNG_DIV257(alpha));
            png_uint_32 reciprocal = 0;
            int c;

            /* Scale and write the alpha channel. */
            out_ptr[aindex] = alphabyte;

            if (alphabyte > 0 && alphabyte < 255)
               reciprocal = UNP_RECIPROCAL(alpha);

            c = channels;
            do /* always at least one channel */
               *out_ptr++ = png_unpremultiply(png_ptr, *in_ptr++, alpha,
                  reciprocal);
            while (--c > 0);

            /* Skip to next component (skip the intervening alpha channel) */
            ++in_ptr;
            ++out_ptr;
         } /* while out_ptr < row_end */

         png_write_row(png_ptr, png_voidcast(png_const_bytep,
            display->local_row));
         input_row += display->row_bytes/(sizeof (png_uint_16));
      } /* while y */
   }

   else
   {
      /* No alpha channel, so the row_end really is the end of the row and it
       * is sufficient to loop over the components one by one.
       */
      png_bytep row_end = output_row + image->width * channels;

      while (y-- > 0)
      {
         png_const_uint_16p in_ptr = input_row;
         png_bytep out_ptr = output_row;

         while (out_ptr < row_end)
         {
            png_uint_32 component = *in_ptr++;

            component *= 255;
            *out_ptr++ = PNG_sRGB_FROM_LINEAR(png_ptr, component);
         }

         png_write_row(png_ptr, output_row);
         input_row += display->row_bytes/(sizeof (png_uint_16));
      }
   }

   return 1;
}

static void
png_image_set_PLTE(png_image_write_control *display)
{
   const png_imagep image = display->image;
   const void *cmap = display->colormap;
   const int entries = image->colormap_entries > 256 ? 256 :
      (int)image->colormap_entries;

   /* NOTE: the caller must check for cmap != NULL and entries != 0 */
   const png_uint_32 format = image->format;
   const int channels = PNG_IMAGE_SAMPLE_CHANNELS(format);

#  if defined(PNG_FORMAT_BGR_SUPPORTED) &&\
      defined(PNG_SIMPLIFIED_WRITE_AFIRST_SUPPORTED)
      const int afirst = (format & PNG_FORMAT_FLAG_AFIRST) != 0 &&
         (format & PNG_FORMAT_FLAG_ALPHA) != 0;
#  else
#     define afirst 0
#  endif

#  ifdef PNG_FORMAT_BGR_SUPPORTED
      const int bgr = (format & PNG_FORMAT_FLAG_BGR) != 0 ? 2 : 0;
#  else
#     define bgr 0
#  endif

   int i, num_trans;
   png_color palette[256];
   png_byte tRNS[256];

   memset(tRNS, 255, (sizeof tRNS));
   memset(palette, 0, (sizeof palette));

   for (i=num_trans=0; i<entries; ++i)
   {
      /* This gets automatically converted to sRGB with reversal of the
       * pre-multiplication if the color-map has an alpha channel.
       */
      if ((format & PNG_FORMAT_FLAG_LINEAR) != 0)
      {
         png_const_uint_16p entry = png_voidcast(png_const_uint_16p, cmap);

         entry += i * channels;

         if ((channels & 1) != 0) /* no alpha */
         {
            if (channels >= 3) /* RGB */
            {
               palette[i].blue = PNG_sRGB_FROM_LINEAR(
                  display->image->opaque->png_ptr, 255 * entry[(2 ^ bgr)]);
               palette[i].green = PNG_sRGB_FROM_LINEAR(
                  display->image->opaque->png_ptr, 255 * entry[1]);
               palette[i].red = PNG_sRGB_FROM_LINEAR(
                  display->image->opaque->png_ptr, 255 * entry[bgr]);
            }

            else /* Gray */
               palette[i].blue = palette[i].red = palette[i].green =
                  PNG_sRGB_FROM_LINEAR(display->image->opaque->png_ptr,
                     255 * *entry);
         }

         else /* alpha */
         {
            png_uint_16 alpha = entry[afirst ? 0 : channels-1];
            png_byte alphabyte = png_check_byte(
               display->image->opaque->png_ptr, PNG_DIV257(alpha));
            png_uint_32 reciprocal = 0;

            /* Calculate a reciprocal, as in the png_write_image_8bit code above
             * this is designed to produce a value scaled to 255*65535 when
             * divided by 128 (i.e. asr 7).
             */
            if (alphabyte > 0 && alphabyte < 255)
               reciprocal = (((0xffff*0xff)<<7)+(alpha>>1))/alpha;

            tRNS[i] = alphabyte;
            if (alphabyte < 255)
               num_trans = i+1;

            if (channels >= 3) /* RGB */
            {
               palette[i].blue = png_unpremultiply(
                  display->image->opaque->png_ptr, entry[afirst + (2 ^ bgr)],
                  alpha, reciprocal);
               palette[i].green = png_unpremultiply(
                  display->image->opaque->png_ptr, entry[afirst + 1], alpha,
                  reciprocal);
               palette[i].red = png_unpremultiply(
                  display->image->opaque->png_ptr, entry[afirst + bgr], alpha,
                  reciprocal);
            }

            else /* gray */
               palette[i].blue = palette[i].red = palette[i].green =
                  png_unpremultiply(display->image->opaque->png_ptr,
                     entry[afirst], alpha, reciprocal);
         }
      }

      else /* Color-map has sRGB values */
      {
         png_const_bytep entry = png_voidcast(png_const_bytep, cmap);

         entry += i * channels;

         switch (channels)
         {
            case 4:
               tRNS[i] = entry[afirst ? 0 : 3];
               if (tRNS[i] < 255)
                  num_trans = i+1;
               /* FALL THROUGH */
            case 3:
               palette[i].blue = entry[afirst + (2 ^ bgr)];
               palette[i].green = entry[afirst + 1];
               palette[i].red = entry[afirst + bgr];
               break;

            case 2:
               tRNS[i] = entry[1 ^ afirst];
               if (tRNS[i] < 255)
                  num_trans = i+1;
               /* FALL THROUGH */
            case 1:
               palette[i].blue = palette[i].red = palette[i].green =
                  entry[afirst];
               break;

            default:
               break;
         }
      }
   }

#  ifdef afirst
#     undef afirst
#  endif
#  ifdef bgr
#     undef bgr
#  endif

   png_set_PLTE(image->opaque->png_ptr, image->opaque->info_ptr, palette,
      entries);

   if (num_trans > 0)
      png_set_tRNS(image->opaque->png_ptr, image->opaque->info_ptr, tRNS,
         num_trans, NULL);

   image->colormap_entries = entries;
}

static int
png_image_write_main(png_voidp argument)
{
   png_image_write_control *display = png_voidcast(png_image_write_control*,
      argument);
   png_imagep image = display->image;
   png_structrp png_ptr = image->opaque->png_ptr;
   png_inforp info_ptr = image->opaque->info_ptr;
   png_uint_32 format = image->format;

   /* The following four ints are actually booleans */
   int colormap = (format & PNG_FORMAT_FLAG_COLORMAP);
   int linear = !colormap && (format & PNG_FORMAT_FLAG_LINEAR); /* input */
   int alpha = !colormap && (format & PNG_FORMAT_FLAG_ALPHA);
   int write_16bit = linear && !colormap && (display->convert_to_8bit == 0);

#  ifdef PNG_BENIGN_ERRORS_SUPPORTED
      /* Make sure we error out on any bad situation */
      png_set_benign_errors(png_ptr, 0/*error*/);
#  endif

   /* Default the 'row_stride' parameter if required. */
   if (display->row_stride == 0)
      display->row_stride = PNG_IMAGE_ROW_STRIDE(*image);

   /* Set the required transforms then write the rows in the correct order. */
   if ((format & PNG_FORMAT_FLAG_COLORMAP) != 0)
   {
      if (display->colormap != NULL && image->colormap_entries > 0)
      {
         png_uint_32 entries = image->colormap_entries;

         png_set_IHDR(png_ptr, info_ptr, image->width, image->height,
            entries > 16 ? 8 : (entries > 4 ? 4 : (entries > 2 ? 2 : 1)),
            PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

         png_image_set_PLTE(display);
      }

      else
         png_error(image->opaque->png_ptr,
            "no color-map for color-mapped image");
   }

   else
      png_set_IHDR(png_ptr, info_ptr, image->width, image->height,
         write_16bit ? 16 : 8,
         ((format & PNG_FORMAT_FLAG_COLOR) ? PNG_COLOR_MASK_COLOR : 0) +
         ((format & PNG_FORMAT_FLAG_ALPHA) ? PNG_COLOR_MASK_ALPHA : 0),
         PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

   /* Counter-intuitively the data transformations must be called *after*
    * png_write_info, not before as in the read code, but the 'set' functions
    * must still be called before.  Just set the color space information, never
    * write an interlaced image.
    */

   if (write_16bit != 0)
   {
      /* The gamma here is 1.0 (linear) and the cHRM chunk matches sRGB. */
      png_set_gAMA_fixed(png_ptr, info_ptr, PNG_GAMMA_LINEAR);

      if ((image->flags & PNG_IMAGE_FLAG_COLORSPACE_NOT_sRGB) == 0)
         png_set_cHRM_fixed(png_ptr, info_ptr,
            /* color      x       y */
            /* white */ 31270, 32900,
            /* red   */ 64000, 33000,
            /* green */ 30000, 60000,
            /* blue  */ 15000,  6000
         );
   }

   else if ((image->flags & PNG_IMAGE_FLAG_COLORSPACE_NOT_sRGB) == 0)
      png_set_sRGB(png_ptr, info_ptr, PNG_sRGB_INTENT_PERCEPTUAL);

   /* Else writing an 8-bit file and the *colors* aren't sRGB, but the 8-bit
    * space must still be gamma encoded.
    */
   else
      png_set_gAMA_fixed(png_ptr, info_ptr, PNG_GAMMA_sRGB_INVERSE);

   /* Write the file header. */
   png_write_info(png_ptr, info_ptr);

   /* Now set up the data transformations (*after* the header is written),
    * remove the handled transformations from the 'format' flags for checking.
    *
    * First check for a little endian system if writing 16 bit files.
    */
   if (write_16bit != 0)
   {
      PNG_CONST png_uint_16 le = 0x0001;

      if ((*(png_const_bytep) & le) != 0)
         png_set_swap(png_ptr);
   }

#  ifdef PNG_SIMPLIFIED_WRITE_BGR_SUPPORTED
      if ((format & PNG_FORMAT_FLAG_BGR) != 0)
      {
         if (colormap == 0 && (format & PNG_FORMAT_FLAG_COLOR) != 0)
            png_set_bgr(png_ptr);
         format &= PNG_BIC_MASK(PNG_FORMAT_FLAG_BGR);
      }
#  endif

#  ifdef PNG_SIMPLIFIED_WRITE_AFIRST_SUPPORTED
      if ((format & PNG_FORMAT_FLAG_AFIRST) != 0)
      {
         if (colormap == 0 && (format & PNG_FORMAT_FLAG_ALPHA) != 0)
            png_set_swap_alpha(png_ptr);
         format &= PNG_BIC_MASK(PNG_FORMAT_FLAG_AFIRST);
      }
#  endif

   /* If there are 16 or fewer color-map entries we wrote a lower bit depth
    * above, but the application data is still byte packed.
    */
   if (colormap != 0 && image->colormap_entries <= 16)
      png_set_packing(png_ptr);

   /* That should have handled all (both) the transforms. */
   if ((format & PNG_BIC_MASK(PNG_FORMAT_FLAG_COLOR | PNG_FORMAT_FLAG_LINEAR |
         PNG_FORMAT_FLAG_ALPHA | PNG_FORMAT_FLAG_COLORMAP)) != 0)
      png_error(png_ptr, "png_write_image: unsupported transformation");

   {
      png_const_bytep row = png_voidcast(png_const_bytep, display->buffer);
      ptrdiff_t row_bytes = display->row_stride;

      if (linear != 0)
         row_bytes *= (sizeof (png_uint_16));

      if (row_bytes < 0)
         row += (image->height-1) * (-row_bytes);

      display->first_row = row;
      display->row_bytes = row_bytes;
   }

   /* Apply 'fast' options if the flag is set. */
   if ((image->flags & PNG_IMAGE_FLAG_FAST) != 0)
   {
#     ifdef PNG_WRITE_FILTER_SUPPORTED
         png_set_filter(png_ptr, PNG_FILTER_TYPE_BASE, PNG_NO_FILTERS);
#     endif /* WRITE_FILTER */
      /* NOTE: determined by experiment using pngstest, this reflects some
       * balance between the time to write the image once and the time to read
       * it about 50 times.  The speed-up in pngstest was about 10-20% of the
       * total (user) time on a heavily loaded system.
       */
#     ifdef PNG_WRITE_CUSTOMIZE_COMPRESSION_SUPPORTED
         png_set_compression_level(png_ptr, 3);
#     endif /* WRITE_CUSTOMIZE_COMPRESSION */
   }

   /* Check for the cases that currently require a pre-transform on the row
    * before it is written.  This only applies when the input is 16-bit and
    * either there is an alpha channel or it is converted to 8-bit.
    */
   if ((linear != 0 && alpha != 0 ) ||
       (colormap == 0 && display->convert_to_8bit != 0))
   {
      png_bytep row = png_voidcast(png_bytep, png_malloc(png_ptr,
         png_get_rowbytes(png_ptr, info_ptr)));
      int result;

      display->local_row = row;
      if (write_16bit != 0)
         result = png_safe_execute(image, png_write_image_16bit, display);
      else
         result = png_safe_execute(image, png_write_image_8bit, display);
      display->local_row = NULL;

      png_free(png_ptr, row);

      /* Skip the 'write_end' on error: */
      if (result == 0)
         return 0;
   }

   /* Otherwise this is the case where the input is in a format currently
    * supported by the rest of the libpng write code; call it directly.
    */
   else
   {
      png_const_bytep row = png_voidcast(png_const_bytep, display->first_row);
      ptrdiff_t row_bytes = display->row_bytes;
      png_uint_32 y = image->height;

      while (y-- > 0)
      {
         png_write_row(png_ptr, row);
         row += row_bytes;
      }
   }

   png_write_end(png_ptr, info_ptr);
   return 1;
}

int PNGAPI
png_image_write_to_stdio(png_imagep image, FILE *file, int convert_to_8bit,
   const void *buffer, png_int_32 row_stride, const void *colormap)
{
   /* Write the image to the given (FILE*). */
   if (image != NULL && image->version == PNG_IMAGE_VERSION)
   {
      if (file != NULL)
      {
         if (png_image_write_init(image) != 0 &&
             png_image_init_io(image, file) != 0)
         {
            png_image_write_control display;
            int result;

            memset(&display, 0, (sizeof display));
            display.image = image;
            display.buffer = buffer;
            display.row_stride = row_stride;
            display.colormap = colormap;
            display.convert_to_8bit = convert_to_8bit;

            result = png_safe_execute(image, png_image_write_main, &display);
            png_image_free(image);
            return result;
         }

         else
            return 0;
      }

      else
         return png_image_error(image,
            "png_image_write_to_stdio: invalid argument");
   }

   else if (image != NULL)
      return png_image_error(image,
         "png_image_write_to_stdio: incorrect PNG_IMAGE_VERSION");

   else
      return 0;
}

int PNGAPI
png_image_write_to_file(png_imagep image, const char *file_name,
   int convert_to_8bit, const void *buffer, png_int_32 row_stride,
   const void *colormap)
{
   /* Write the image to the named file. */
   if (image != NULL && image->version == PNG_IMAGE_VERSION)
   {
      if (file_name != NULL)
      {
         FILE *fp = fopen(file_name, "wb");

         if (fp != NULL)
         {
            if (png_image_write_to_stdio(image, fp, convert_to_8bit, buffer,
               row_stride, colormap) != 0)
            {
               int error; /* from fflush/fclose */

               /* Make sure the file is flushed correctly. */
               if (fflush(fp) == 0 && ferror(fp) == 0)
               {
                  if (fclose(fp) == 0)
                     return 1;

                  error = errno; /* from fclose */
               }

               else
               {
                  error = errno; /* from fflush or ferror */
                  (void)fclose(fp);
               }

               (void)remove(file_name);
               /* The image has already been cleaned up; this is just used to
                * set the error (because the original write succeeded).
                */
               return png_image_error(image, strerror(error));
            }

            else
            {
               /* Clean up: just the opened file. */
               (void)fclose(fp);
               (void)remove(file_name);
               return 0;
            }
         }

         else
            return png_image_error(image, strerror(errno));
      }

      else
         return png_image_error(image,
            "png_image_write_to_file: invalid argument");
   }

   else if (image != NULL)
      return png_image_error(image,
         "png_image_write_to_file: incorrect PNG_IMAGE_VERSION");

   else
      return 0;
}
#endif /* STDIO */
#endif /* SIMPLIFIED_WRITE */
#endif /* WRITE */
