/* pngrcb.c - callbacks while reading a png file

   libpng 1.0 beta 4 - version 0.90
   For conditions of distribution and use, see copyright notice in png.h
   Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.
   January 10, 1997
   */

#define PNG_INTERNAL
#include "png.h"

void
png_read_IHDR(png_structp png_ptr, png_infop info,
   png_uint_32 width, png_uint_32 height, int bit_depth,
   int color_type, int compression_type, int filter_type,
   int interlace_type)
{
   if (!info)
      return;

   info->width = width;
   info->height = height;
   info->bit_depth = (png_byte)bit_depth;
   info->color_type =(png_byte) color_type;
   info->compression_type = (png_byte)compression_type;
   info->filter_type = (png_byte)filter_type;
   info->interlace_type = (png_byte)interlace_type;
   if (info->color_type == PNG_COLOR_TYPE_PALETTE)
      info->channels = 1;
   else if (info->color_type & PNG_COLOR_MASK_COLOR)
      info->channels = 3;
   else
      info->channels = 1;
   if (info->color_type & PNG_COLOR_MASK_ALPHA)
      info->channels++;
   info->pixel_depth = (png_byte)(info->channels * info->bit_depth);
   info->rowbytes = ((info->width * info->pixel_depth + 7) >> 3);
}

void
png_read_PLTE(png_structp png_ptr, png_infop info,
   png_colorp palette, int num)
{
   if (!info)
      return;

   info->palette = palette;
   info->num_palette = (png_uint_16)num;
   info->valid |= PNG_INFO_PLTE;
}

#if defined(PNG_READ_gAMA_SUPPORTED)
void
png_read_gAMA(png_structp png_ptr, png_infop info, double gamma)
{
   if (!info)
      return;

   info->gamma = (float)gamma;
   info->valid |= PNG_INFO_gAMA;
}
#endif

#if defined(PNG_READ_sBIT_SUPPORTED)
void
png_read_sBIT(png_structp png_ptr, png_infop info,
   png_color_8p sig_bit)
{
   if (!info)
      return;

   png_memcpy(&(info->sig_bit), sig_bit, sizeof (png_color_8));
   info->valid |= PNG_INFO_sBIT;
}
#endif

#if defined(PNG_READ_cHRM_SUPPORTED)
void
png_read_cHRM(png_structp png_ptr, png_infop info,
   double white_x, double white_y, double red_x, double red_y,
   double green_x, double green_y, double blue_x, double blue_y)
{
   if (!info)
      return;

   info->x_white = (float)white_x;
   info->y_white = (float)white_y;
   info->x_red   = (float)red_x;
   info->y_red   = (float)red_y;
   info->x_green = (float)green_x;
   info->y_green = (float)green_y;
   info->x_blue  = (float)blue_x;
   info->y_blue  = (float)blue_y;
   info->valid |= PNG_INFO_cHRM;
}
#endif

#if defined(PNG_READ_tRNS_SUPPORTED)
void
png_read_tRNS(png_structp png_ptr, png_infop info,
   png_bytep trans, int num_trans,   png_color_16p trans_values)
{
   if (!info)
      return;

   if (trans)
   {
      info->trans = trans;
   }
   else
   {
      png_memcpy(&(info->trans_values), trans_values,
         sizeof(png_color_16));
   }
   info->num_trans = (png_uint_16)num_trans;
   info->valid |= PNG_INFO_tRNS;
}
#endif

#if defined(PNG_READ_bKGD_SUPPORTED)
void
png_read_bKGD(png_structp png_ptr, png_infop info,
   png_color_16p background)
{
   if (!info)
      return;

   png_memcpy(&(info->background), background, sizeof(png_color_16));
   info->valid |= PNG_INFO_bKGD;
}
#endif

#if defined(PNG_READ_hIST_SUPPORTED)
void
png_read_hIST(png_structp png_ptr, png_infop info, png_uint_16p hist)
{
   if (!info)
      return;

   info->hist = hist;
   info->valid |= PNG_INFO_hIST;
}
#endif

#if defined(PNG_READ_pHYs_SUPPORTED)
void
png_read_pHYs(png_structp png_ptr, png_infop info,
   png_uint_32 res_x, png_uint_32 res_y, int unit_type)
{
   if (!info)
      return;

   info->x_pixels_per_unit = res_x;
   info->y_pixels_per_unit = res_y;
   info->phys_unit_type = (png_byte)unit_type;
   info->valid |= PNG_INFO_pHYs;
}
#endif

#if defined(PNG_READ_oFFs_SUPPORTED)
void
png_read_oFFs(png_structp png_ptr, png_infop info,
   png_uint_32 offset_x, png_uint_32 offset_y, int unit_type)
{
   if (!info)
      return;

   info->x_offset = offset_x;
   info->y_offset = offset_y;
   info->offset_unit_type = (png_byte)unit_type;
   info->valid |= PNG_INFO_oFFs;
}
#endif

#if defined(PNG_READ_tIME_SUPPORTED)
void
png_read_tIME(png_structp png_ptr, png_infop info,
   png_timep mod_time)
{
   if (!info)
      return;

   png_memcpy(&(info->mod_time), mod_time, sizeof (png_time));
   info->valid |= PNG_INFO_tIME;
}
#endif

#if defined(PNG_READ_tEXt_SUPPORTED) || defined(PNG_READ_zTXt_SUPPORTED)
void
png_read_zTXt(png_structp png_ptr, png_infop info,
   png_charp key, png_charp text, png_uint_32 text_len, int compression)
{
   if (!info)
      return;

   if (info->max_text <= info->num_text)
   {
      if (info->text)
      {
         png_uint_32 old_max;

         old_max = info->max_text;
         info->max_text = info->num_text + 16;
         {
            png_textp old_text;

            old_text = info->text;
            info->text = (png_textp)png_malloc(png_ptr,
               info->max_text * sizeof (png_text));
            png_memcpy(info->text, old_text,
               (png_size_t)(old_max * sizeof (png_text)));
            png_free(png_ptr, old_text);
         }
      }
      else
      {
         info->max_text = 16;
         info->num_text = 0;
         info->text = (png_textp)png_malloc(png_ptr,
            info->max_text * sizeof (png_text));
      }
   }

   info->text[info->num_text].key = key;
   info->text[info->num_text].text = text;
   info->text[info->num_text].text_length = text_len;
   info->text[info->num_text].compression = compression;
   info->num_text++;
}
#endif

#if defined(PNG_READ_tEXt_SUPPORTED)
void
png_read_tEXt(png_structp png_ptr, png_infop info,
   png_charp key, png_charp text, png_uint_32 text_len)
{
   if (!info)
      return;

   png_read_zTXt(png_ptr, info, key, text, text_len, -1);
}
#endif

