
/* pngset.c - storage of image information into info struct
 *
 * libpng 1.0.5h - December 10, 1999
 * For conditions of distribution and use, see copyright notice in png.h
 * Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.
 * Copyright (c) 1996, 1997 Andreas Dilger
 * Copyright (c) 1998, 1999 Glenn Randers-Pehrson
 *
 * The functions here are used during reads to store data from the file
 * into the info struct, and during writes to store application data
 * into the info struct for writing into the file.  This abstracts the
 * info struct and allows us to change the structure in the future.
 */

#define PNG_INTERNAL
#include "png.h"

#if defined(PNG_bKGD_SUPPORTED)
void
png_set_bKGD(png_structp png_ptr, png_infop info_ptr, png_color_16p background)
{
   png_debug1(1, "in %s storage function\n", "bKGD");
   if (png_ptr == NULL || info_ptr == NULL)
      return;

   png_memcpy(&(info_ptr->background), background, sizeof(png_color_16));
   info_ptr->valid |= PNG_INFO_bKGD;
}
#endif

#if defined(PNG_cHRM_SUPPORTED)
#ifdef PNG_FLOATING_POINT_SUPPORTED
void
png_set_cHRM(png_structp png_ptr, png_infop info_ptr,
   double white_x, double white_y, double red_x, double red_y,
   double green_x, double green_y, double blue_x, double blue_y)
{
   png_debug1(1, "in %s storage function\n", "cHRM");
   if (png_ptr == NULL || info_ptr == NULL)
      return;

   info_ptr->x_white = (float)white_x;
   info_ptr->y_white = (float)white_y;
   info_ptr->x_red   = (float)red_x;
   info_ptr->y_red   = (float)red_y;
   info_ptr->x_green = (float)green_x;
   info_ptr->y_green = (float)green_y;
   info_ptr->x_blue  = (float)blue_x;
   info_ptr->y_blue  = (float)blue_y;
   info_ptr->valid |= PNG_INFO_cHRM;
}
#endif
#ifdef PNG_FIXED_POINT_SUPPORTED
void
png_set_cHRM_fixed(png_structp png_ptr, png_infop info_ptr,
   png_uint_32 white_x, png_uint_32 white_y, png_uint_32 red_x,
   png_uint_32 red_y, png_uint_32 green_x, png_uint_32 green_y,
   png_uint_32 blue_x, png_uint_32 blue_y)
{
   png_debug1(1, "in %s storage function\n", "cHRM");
   if (png_ptr == NULL || info_ptr == NULL)
      return;

   info_ptr->int_x_white = white_x;
   info_ptr->int_y_white = white_y;
   info_ptr->int_x_red   = red_x;
   info_ptr->int_y_red   = red_y;
   info_ptr->int_x_green = green_x;
   info_ptr->int_y_green = green_y;
   info_ptr->int_x_blue  = blue_x;
   info_ptr->int_y_blue  = blue_y;
   info_ptr->valid |= PNG_INFO_cHRM;
}
#endif
#endif

#if defined(PNG_gAMA_SUPPORTED)
#ifdef PNG_FLOATING_POINT_SUPPORTED
void
png_set_gAMA(png_structp png_ptr, png_infop info_ptr, double file_gamma)
{
   png_debug1(1, "in %s storage function\n", "gAMA");
   if (png_ptr == NULL || info_ptr == NULL)
      return;

   info_ptr->gamma = (float)file_gamma;
#ifdef PNG_FIXED_POINT_SUPPORTED
   info_ptr->int_gamma = (int)(file_gamma*100000.+.5);
#endif
   info_ptr->valid |= PNG_INFO_gAMA;
}
#endif
#endif
#ifdef PNG_FIXED_POINT_SUPPORTED
void
png_set_gAMA_fixed(png_structp png_ptr, png_infop info_ptr, png_uint_32 int_gamma)
{
   png_debug1(1, "in %s storage function\n", "gAMA");
   if (png_ptr == NULL || info_ptr == NULL)
      return;

#ifdef PNG_FLOATING_POINT_SUPPORTED
   info_ptr->gamma = (float)int_gamma/100000.;
#endif
   info_ptr->int_gamma = int_gamma;
   info_ptr->valid |= PNG_INFO_gAMA;
}
#endif

#if defined(PNG_hIST_SUPPORTED)
void
png_set_hIST(png_structp png_ptr, png_infop info_ptr, png_uint_16p hist)
{
   png_debug1(1, "in %s storage function\n", "hIST");
   if (png_ptr == NULL || info_ptr == NULL)
      return;

   info_ptr->hist = hist;
   info_ptr->valid |= PNG_INFO_hIST;
}
#endif

void
png_set_IHDR(png_structp png_ptr, png_infop info_ptr,
   png_uint_32 width, png_uint_32 height, int bit_depth,
   int color_type, int interlace_type, int compression_type,
   int filter_type)
{
   int rowbytes_per_pixel;
   png_debug1(1, "in %s storage function\n", "IHDR");
   if (png_ptr == NULL || info_ptr == NULL)
      return;

   info_ptr->width = width;
   info_ptr->height = height;
   info_ptr->bit_depth = (png_byte)bit_depth;
   info_ptr->color_type =(png_byte) color_type;
   info_ptr->compression_type = (png_byte)compression_type;
   info_ptr->filter_type = (png_byte)filter_type;
   info_ptr->interlace_type = (png_byte)interlace_type;
   if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      info_ptr->channels = 1;
   else if (info_ptr->color_type & PNG_COLOR_MASK_COLOR)
      info_ptr->channels = 3;
   else
      info_ptr->channels = 1;
   if (info_ptr->color_type & PNG_COLOR_MASK_ALPHA)
      info_ptr->channels++;
   info_ptr->pixel_depth = (png_byte)(info_ptr->channels * info_ptr->bit_depth);

   /* check for overflow */
   rowbytes_per_pixel = (info_ptr->pixel_depth + 7) >> 3;
   if (( width > PNG_MAX_UINT/rowbytes_per_pixel))
   {
      png_warning(png_ptr,
         "Width too large to process image data; rowbytes will overflow.");
      info_ptr->rowbytes = (png_size_t)0;
   }
   else
      info_ptr->rowbytes = (info_ptr->width * info_ptr->pixel_depth + 7) >> 3;
}

#if defined(PNG_oFFs_SUPPORTED)
void
png_set_oFFs(png_structp png_ptr, png_infop info_ptr,
   png_int_32 offset_x, png_int_32 offset_y, int unit_type)
{
   png_debug1(1, "in %s storage function\n", "oFFs");
   if (png_ptr == NULL || info_ptr == NULL)
      return;

   info_ptr->x_offset = offset_x;
   info_ptr->y_offset = offset_y;
   info_ptr->offset_unit_type = (png_byte)unit_type;
   info_ptr->valid |= PNG_INFO_oFFs;
}
#endif

#if defined(PNG_pCAL_SUPPORTED)
void
png_set_pCAL(png_structp png_ptr, png_infop info_ptr,
   png_charp purpose, png_int_32 X0, png_int_32 X1, int type, int nparams,
   png_charp units, png_charpp params)
{
   png_uint_32 length;
   int i;

   png_debug1(1, "in %s storage function\n", "pCAL");
   if (png_ptr == NULL || info_ptr == NULL)
      return;

   length = png_strlen(purpose) + 1;
   png_debug1(3, "allocating purpose for info (%d bytes)\n", length);
   info_ptr->pcal_purpose = (png_charp)png_malloc(png_ptr, length);
   png_memcpy(info_ptr->pcal_purpose, purpose, (png_size_t)length);

   png_debug(3, "storing X0, X1, type, and nparams in info\n");
   info_ptr->pcal_X0 = X0;
   info_ptr->pcal_X1 = X1;
   info_ptr->pcal_type = (png_byte)type;
   info_ptr->pcal_nparams = (png_byte)nparams;

   length = png_strlen(units) + 1;
   png_debug1(3, "allocating units for info (%d bytes)\n", length);
   info_ptr->pcal_units = (png_charp)png_malloc(png_ptr, length);
   png_memcpy(info_ptr->pcal_units, units, (png_size_t)length);

   info_ptr->pcal_params = (png_charpp)png_malloc(png_ptr,
      (png_uint_32)((nparams + 1) * sizeof(png_charp)));
   info_ptr->pcal_params[nparams] = NULL;

   for (i = 0; i < nparams; i++)
   {
      length = png_strlen(params[i]) + 1;
      png_debug2(3, "allocating parameter %d for info (%d bytes)\n", i, length);
      info_ptr->pcal_params[i] = (png_charp)png_malloc(png_ptr, length);
      png_memcpy(info_ptr->pcal_params[i], params[i], (png_size_t)length);
   }

   info_ptr->valid |= PNG_INFO_pCAL;
}
#endif

#if defined(PNG_READ_sCAL_SUPPORTED) || defined(PNG_WRITE_sCAL_SUPPORTED)
#ifdef PNG_FLOATING_POINT_SUPPORTED
void
png_set_sCAL(png_structp png_ptr, png_infop info_ptr,
             png_charp unit, double width, double height)
{
   png_uint_32 length;

   png_debug1(1, "in %s storage function\n", "sCAL");
   if (png_ptr == NULL || info_ptr == NULL)
      return;

   length = png_strlen(unit) + 1;
   png_debug1(3, "allocating unit for info (%d bytes)\n", length);
   info_ptr->scal_unit = (png_charp)png_malloc(png_ptr, length);
   png_memcpy(info_ptr->scal_unit, unit, (png_size_t)length);
   info_ptr->scal_pixel_width = width;
   info_ptr->scal_pixel_height = height;

   info_ptr->valid |= PNG_INFO_sCAL;
}
#endif
void
png_set_sCAL_s(png_structp png_ptr, png_infop info_ptr,
             png_charp unit, png_charp swidth, png_charp sheight)
{
   png_uint_32 length;

   png_debug1(1, "in %s storage function\n", "sCAL");
   if (png_ptr == NULL || info_ptr == NULL)
      return;

   length = png_strlen(unit) + 1;
   png_debug1(3, "allocating unit for info (%d bytes)\n", length);
   info_ptr->scal_unit = (png_charp)png_malloc(png_ptr, length);
   png_memcpy(info_ptr->scal_unit, unit, (png_size_t)length);

   length = png_strlen(swidth) + 1;
   png_debug1(3, "allocating unit for info (%d bytes)\n", length);
   info_ptr->scal_s_width = (png_charp)png_malloc(png_ptr, length);
   png_memcpy(info_ptr->scal_s_width, swidth, (png_size_t)length);

   length = png_strlen(sheight) + 1;
   png_debug1(3, "allocating unit for info (%d bytes)\n", length);
   info_ptr->scal_s_width = (png_charp)png_malloc(png_ptr, length);
   png_memcpy(info_ptr->scal_s_height, sheight, (png_size_t)length);

   info_ptr->valid |= PNG_INFO_sCAL;
}
#endif

#if defined(PNG_pHYs_SUPPORTED)
void
png_set_pHYs(png_structp png_ptr, png_infop info_ptr,
   png_uint_32 res_x, png_uint_32 res_y, int unit_type)
{
   png_debug1(1, "in %s storage function\n", "pHYs");
   if (png_ptr == NULL || info_ptr == NULL)
      return;

   info_ptr->x_pixels_per_unit = res_x;
   info_ptr->y_pixels_per_unit = res_y;
   info_ptr->phys_unit_type = (png_byte)unit_type;
   info_ptr->valid |= PNG_INFO_pHYs;
}
#endif

void
png_set_PLTE(png_structp png_ptr, png_infop info_ptr,
   png_colorp palette, int num_palette)
{
   png_debug1(1, "in %s storage function\n", "PLTE");
   if (png_ptr == NULL || info_ptr == NULL)
      return;

   info_ptr->palette = palette;
   info_ptr->num_palette = (png_uint_16)num_palette;
   info_ptr->valid |= PNG_INFO_PLTE;
}

#if defined(PNG_sBIT_SUPPORTED)
void
png_set_sBIT(png_structp png_ptr, png_infop info_ptr,
   png_color_8p sig_bit)
{
   png_debug1(1, "in %s storage function\n", "sBIT");
   if (png_ptr == NULL || info_ptr == NULL)
      return;

   png_memcpy(&(info_ptr->sig_bit), sig_bit, sizeof (png_color_8));
   info_ptr->valid |= PNG_INFO_sBIT;
}
#endif

#if defined(PNG_sRGB_SUPPORTED)
void
png_set_sRGB(png_structp png_ptr, png_infop info_ptr, int intent)
{
   png_debug1(1, "in %s storage function\n", "sRGB");
   if (png_ptr == NULL || info_ptr == NULL)
      return;

   info_ptr->srgb_intent = (png_byte)intent;
   info_ptr->valid |= PNG_INFO_sRGB;
}

void
png_set_sRGB_gAMA_and_cHRM(png_structp png_ptr, png_infop info_ptr,
   int intent)
{
#if defined(PNG_gAMA_SUPPORTED)
#ifdef PNG_FLOATING_POINT_SUPPORTED
   float file_gamma;
#endif
#ifdef PNG_FIXED_POINT_SUPPORTED
   png_uint_32 int_file_gamma;
#endif
#endif
#if defined(PNG_cHRM_SUPPORTED)
#ifdef PNG_FLOATING_POINT_SUPPORTED
   float white_x, white_y, red_x, red_y, green_x, green_y, blue_x, blue_y;
#endif
#ifdef PNG_FIXED_POINT_SUPPORTED
   png_uint_32 int_white_x, int_white_y, int_red_x, int_red_y, int_green_x,
      int_green_y, int_blue_x, int_blue_y;
#endif
#endif
   png_debug1(1, "in %s storage function\n", "sRGB_gAMA_and_cHRM");
   if (png_ptr == NULL || info_ptr == NULL)
      return;

   png_set_sRGB(png_ptr, info_ptr, intent);

#if defined(PNG_gAMA_SUPPORTED)
#ifdef PNG_FLOATING_POINT_SUPPORTED
   file_gamma = (float).45455;
   png_set_gAMA(png_ptr, info_ptr, file_gamma);
#endif
#ifdef PNG_FIXED_POINT_SUPPORTED
   int_file_gamma = 45455L;
   png_set_gAMA_fixed(png_ptr, info_ptr, int_file_gamma);
#endif
#endif

#if defined(PNG_cHRM_SUPPORTED)
#ifdef PNG_FIXED_POINT_SUPPORTED
   int_white_x = 31270L;
   int_white_y = 32900L;
   int_red_x   = 64000L;
   int_red_y   = 33000L;
   int_green_x = 30000L;
   int_green_y = 60000L;
   int_blue_x  = 15000L;
   int_blue_y  =  6000L;

   png_set_cHRM_fixed(png_ptr, info_ptr,
      int_white_x, int_white_y, int_red_x, int_red_y, int_green_x, int_green_y,
      int_blue_x, int_blue_y);
#endif
#ifdef PNG_FLOATING_POINT_SUPPORTED
   white_x = (float).3127;
   white_y = (float).3290;
   red_x   = (float).64;
   red_y   = (float).33;
   green_x = (float).30;
   green_y = (float).60;
   blue_x  = (float).15;
   blue_y  = (float).06;

   png_set_cHRM(png_ptr, info_ptr,
      white_x, white_y, red_x, red_y, green_x, green_y, blue_x, blue_y);
#endif
#endif
}
#endif


#if defined(PNG_iCCP_SUPPORTED)
void
png_set_iCCP(png_structp png_ptr, png_infop info_ptr,
             png_charp name, int compression_type,
             png_charp profile, int proflen)
{
   png_debug1(1, "in %s storage function\n", "iCCP");
   if (png_ptr == NULL || info_ptr == NULL || name == NULL || profile == NULL)
      return;

   info_ptr->iccp_name = png_malloc(png_ptr, png_strlen(name)+1);
   strcpy(info_ptr->iccp_name, name);
   info_ptr->iccp_profile = png_malloc(png_ptr, proflen);
   memcpy(info_ptr->iccp_profile, profile, proflen);
   info_ptr->iccp_proflen = (png_uint_32)proflen;
   /* Compression is always zero but is here so the API and info structure
    * does not have to change * if we introduce multiple compression types */
   info_ptr->iccp_compression = (png_byte)compression_type;
   info_ptr->valid |= PNG_INFO_iCCP;
}
#endif

#if defined(PNG_TEXT_SUPPORTED)
void
png_set_text(png_structp png_ptr, png_infop info_ptr, png_textp text_ptr,
   int num_text)
{
   int i;

   png_debug1(1, "in %s storage function\n", (png_ptr->chunk_name[0] == '\0' ?
      "text" : (png_const_charp)png_ptr->chunk_name));

   if (png_ptr == NULL || info_ptr == NULL || num_text == 0)
      return;

   /* Make sure we have enough space in the "text" array in info_struct
    * to hold all of the incoming text_ptr objects.
    */
   if (info_ptr->num_text + num_text > info_ptr->max_text)
   {
      if (info_ptr->text != NULL)
      {
         png_textp old_text;
         int old_max;

         old_max = info_ptr->max_text;
         info_ptr->max_text = info_ptr->num_text + num_text + 8;
         old_text = info_ptr->text;
         info_ptr->text = (png_textp)png_malloc(png_ptr,
            (png_uint_32)(info_ptr->max_text * sizeof (png_text)));
         png_memcpy(info_ptr->text, old_text, (png_size_t)(old_max *
            sizeof(png_text)));
         png_free(png_ptr, old_text);
      }
      else
      {
         info_ptr->max_text = num_text + 8;
         info_ptr->num_text = 0;
         info_ptr->text = (png_textp)png_malloc(png_ptr,
            (png_uint_32)(info_ptr->max_text * sizeof (png_text)));
      }
      png_debug1(3, "allocated %d entries for info_ptr->text\n",
         info_ptr->max_text);
   }

   for (i = 0; i < num_text; i++)
   {
      png_textp textp = &(info_ptr->text[info_ptr->num_text]);
      png_charp key,text;

      if (text_ptr[i].key == (png_charp)NULL)
          continue;

#ifdef PNG_iTXt_SUPPORTED
      textp->lang = text_ptr[i].lang;
      textp->translated_key = text_ptr[i].translated_key;
#else
      textp->lang = NULL;
      textp->translated_key = NULL;
#endif

      if (text_ptr[i].text[0] == '\0')
      {
         textp->text_length = 0;
         textp->compression = PNG_TEXT_COMPRESSION_NONE;
      }
      else
      {
         textp->text_length = png_strlen(text_ptr[i].text);
         textp->compression = text_ptr[i].compression;
      }
      key=text_ptr[i].key;
      for (text = key; *text++;)
        /* empty loop to find the byte after the zero byte after the
           end of key */ ;

      textp->key = (png_charp)png_malloc(png_ptr,
         (png_uint_32)(text+textp->text_length - key)+1);
      /* Caution: the calling program, not libpng, is responsible for
         freeing this, if libpng wasn't the caller. */
      png_debug2(2, "Allocated %d bytes at %x in png_set_text\n",
         text+textp->text_length-key+1, textp->key);

      png_memcpy(textp->key, text_ptr[i].key,
         (png_size_t)(text - key));  /* includes the zero-byte separator */

      textp->text = textp->key + (text-key);
      if(textp->text_length)
      {
         png_memcpy(textp->text, text_ptr[i].text,
            (png_size_t)(textp->text_length));
         *(textp->text+textp->text_length) = '\0';
      }
      else
         textp->text--;
      info_ptr->text[info_ptr->num_text]= *textp;
      info_ptr->num_text++;
      png_debug1(3, "transferred text chunk %d\n", info_ptr->num_text);
   }
}
#endif

#if defined(PNG_tIME_SUPPORTED)
void
png_set_tIME(png_structp png_ptr, png_infop info_ptr, png_timep mod_time)
{
   png_debug1(1, "in %s storage function\n", "tIME");
   if (png_ptr == NULL || info_ptr == NULL ||
       (png_ptr->mode & PNG_WROTE_tIME))
      return;

   png_memcpy(&(info_ptr->mod_time), mod_time, sizeof (png_time));
   info_ptr->valid |= PNG_INFO_tIME;
}
#endif

#if defined(PNG_tRNS_SUPPORTED)
void
png_set_tRNS(png_structp png_ptr, png_infop info_ptr,
   png_bytep trans, int num_trans, png_color_16p trans_values)
{
   png_debug1(1, "in %s storage function\n", "tRNS");
   if (png_ptr == NULL || info_ptr == NULL)
      return;

   if (trans != NULL)
   {
      info_ptr->trans = trans;
   }

   if (trans_values != NULL)
   {
      png_memcpy(&(info_ptr->trans_values), trans_values,
         sizeof(png_color_16));
      if (num_trans == 0)
        num_trans = 1;
   }
   info_ptr->num_trans = (png_uint_16)num_trans;
   info_ptr->valid |= PNG_INFO_tRNS;
}
#endif

#if defined(PNG_sPLT_SUPPORTED)
void
png_set_spalettes(png_structp png_ptr,
             png_infop info_ptr, png_spalette_p entries, int nentries)
{
    png_spalette_p        np;
    int i;

    np = (png_spalette_p)png_malloc(png_ptr,
        (info_ptr->splt_palettes_num + nentries) * sizeof(png_spalette));

    memcpy(np, info_ptr->splt_palettes,
           info_ptr->splt_palettes_num * sizeof(png_spalette));
    png_free(png_ptr, info_ptr->splt_palettes);

    for (i = 0; i < nentries; i++)
    {
        png_spalette_p to = np + info_ptr->splt_palettes_num + i;
        png_spalette_p from = entries + i;

        to->name = (png_charp)png_malloc(png_ptr,
                                        png_strlen(from->name) + 1);
        png_strcpy(to->name, from->name);
        to->entries = (png_spalette_entryp)png_malloc(png_ptr,
                                 from->nentries * sizeof(png_spalette));
        memcpy(to->entries, from->entries,
               from->nentries * sizeof(png_spalette));
    }

    info_ptr->splt_palettes = np;
    info_ptr->splt_palettes_num += nentries;
}
#endif /* PNG_sPLT_SUPPORTED */

#if defined(PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED)
void
png_set_unknown_chunks(png_structp png_ptr,
   png_infop info_ptr, png_unknown_chunkp unknowns, int nunknowns)
{
    png_unknown_chunkp np;
    int i;

    np = (png_unknown_chunkp)png_malloc(png_ptr,
        (info_ptr->unknown_chunks_num + nunknowns) * sizeof(png_unknown_chunk));

    memcpy(np, info_ptr->unknown_chunks,
           info_ptr->unknown_chunks_num * sizeof(png_unknown_chunk));
    png_free(png_ptr, info_ptr->unknown_chunks);

    for (i = 0; i < nunknowns; i++)
    {
        png_unknown_chunkp to = np + info_ptr->unknown_chunks_num + i;
        png_unknown_chunkp from = unknowns + i;

        png_strcpy(to->name, from->name);
        to->data = (png_bytep)png_malloc(png_ptr, from->size);
        memcpy(to->data, from->data, from->size);
        to->size = from->size;

        /* note our location in the read or write sequence */
        to->location = png_ptr->mode;
    }

    info_ptr->unknown_chunks = np;
    info_ptr->unknown_chunks_num += nunknowns;
}
#endif

#if defined(PNG_READ_EMPTY_PLTE_SUPPORTED)
void
png_permit_empty_plte (png_structp png_ptr, int empty_plte_permitted)
{
   png_debug1(1, "in png_permit_empty_plte\n", "");
   if (png_ptr == NULL)
      return;
   png_ptr->empty_plte_permitted=(png_byte)empty_plte_permitted;
}
#endif







