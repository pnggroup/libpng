
/* pngtest.c - a simple test program to test libpng

   libpng 1.0 beta 6 - version 0.96
   For conditions of distribution and use, see copyright notice in png.h
   Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.
   Copyright (c) 1996, 1997 Andreas Dilger
   May 12, 1997
   */

#include <stdio.h>
#include <stdlib.h>

/* Makes pngtest verbose so we can find problems (needs to be before png.h) */
#ifndef PNG_DEBUG
#define PNG_DEBUG 0
#endif

#include "png.h"

#ifdef __TURBOC__
#include <mem.h>
#endif

/* defined so I can write to a file on gui/windowing platforms */
/*  #define STDERR stderr  */
#define STDERR stdout   /* for DOS */

/* input and output filenames */
#ifdef RISCOS
char *inname = "pngtest_png";
char *outname = "pngout_png";
#else
char *inname = "pngtest.png";
char *outname = "pngout.png";
#endif

char inbuf[256], outbuf[256];

int
main(int argc, char *argv[])
{
   FILE *fpin, *fpout;
   png_structp read_ptr, write_ptr;
   png_infop read_info_ptr, write_info_ptr, end_info_ptr;
   png_bytep row_buf;
   png_uint_32 y;
   png_uint_32 width, height;
   int num_pass, pass;
   int bit_depth, color_type;
#ifdef USE_FAR_KEYWORD
   jmp_buf jmpbuf;
#endif   
   row_buf = (png_bytep)NULL;

   fprintf(STDERR, "Testing libpng version %s\n", PNG_LIBPNG_VER_STRING);

   if (strcmp(png_libpng_ver, PNG_LIBPNG_VER_STRING))
   {
      fprintf(STDERR,
         "Warning: versions are different between png.h and png.c\n");
      fprintf(STDERR, "  png.h version: %s\n", PNG_LIBPNG_VER_STRING);
      fprintf(STDERR, "  png.c version: %s\n\n", png_libpng_ver);
   }

   if (argc > 1)
     inname = argv[1];

   if (argc > 2)
     outname = argv[2];

   if (argc > 3)
   {
     fprintf(stderr, "usage: %s [infile.png] [outfile.png]\n", argv[0]);
     exit(1);
   }

   if ((fpin = fopen(inname, "rb")) == NULL)
   {
      fprintf(STDERR, "Could not find input file %s\n", inname);
      return 1;
   }

   if ((fpout = fopen(outname, "wb")) == NULL)
   {
      fprintf(STDERR, "Could not open output file %s\n", outname);
      fclose(fpin);
      return 1;
   }

   png_debug(0, "Allocating read and write structures\n");
   read_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL,
      (png_error_ptr)NULL, (png_error_ptr)NULL);
   write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL,
      (png_error_ptr)NULL, (png_error_ptr)NULL);
   png_debug(0, "Allocating read_info, write_info and end_info structures\n");
   read_info_ptr = png_create_info_struct(read_ptr);
   write_info_ptr = png_create_info_struct(read_ptr);
   end_info_ptr = png_create_info_struct(read_ptr);

   png_debug(0, "Setting jmpbuf for read struct\n");
#ifdef USE_FAR_KEYWORD
   if (setjmp(jmpbuf))
#else
   if (setjmp(read_ptr->jmpbuf))
#endif
   {
      fprintf(STDERR, "libpng read error\n");
      png_destroy_read_struct(&read_ptr, &read_info_ptr, &end_info_ptr);
      png_destroy_write_struct(&write_ptr, &write_info_ptr);
      fclose(fpin);
      fclose(fpout);
      return 1;
   }

   png_debug(0, "Setting jmpbuf for write struct\n");
#ifdef USE_FAR_KEYWORD
   png_memcpy(read_ptr->jmpbuf,jmpbuf,sizeof(jmp_buf));
   if (setjmp(jmpbuf))
#else
   if (setjmp(write_ptr->jmpbuf))
#endif
   {
      fprintf(STDERR, "libpng write error\n");
      png_destroy_read_struct(&read_ptr, &read_info_ptr, &end_info_ptr);
      png_destroy_write_struct(&write_ptr, &write_info_ptr);
      fclose(fpin);
      fclose(fpout);
      return 1;
   }

#ifdef USE_FAR_KEYWORD
   png_memcpy(write_ptr->jmpbuf,jmpbuf,sizeof(jmp_buf));
#endif
   png_debug(0, "Initializing input and output streams\n");
   png_init_io(read_ptr, fpin);
   png_init_io(write_ptr, fpout);

   png_debug(0, "Reading info struct\n");
   png_read_info(read_ptr, read_info_ptr);

   png_debug(0, "Transferring info struct\n");
   {
      int interlace_type, compression_type, filter_type;

      if (png_get_IHDR(read_ptr, read_info_ptr, &width, &height, &bit_depth,
          &color_type, &interlace_type, &compression_type, &filter_type))
      {
         png_set_IHDR(write_ptr, write_info_ptr, width, height, bit_depth,
            color_type, interlace_type, compression_type, filter_type);
      }
   }
#if defined(PNG_READ_bKGD_SUPPORTED) && defined(PNG_WRITE_bKGD_SUPPORTED)
   {
      png_color_16p background;

      if (png_get_bKGD(read_ptr, read_info_ptr, &background))
      {
         png_set_bKGD(write_ptr, write_info_ptr, background);
      }
   }
#endif
#if defined(PNG_READ_cHRM_SUPPORTED) && defined(PNG_WRITE_cHRM_SUPPORTED)
   {
      double white_x, white_y, red_x, red_y, green_x, green_y, blue_x, blue_y;

      if (png_get_cHRM(read_ptr, read_info_ptr, &white_x, &white_y, &red_x,
         &red_y, &green_x, &green_y, &blue_x, &blue_y))
      {
         png_set_cHRM(write_ptr, write_info_ptr, white_x, white_y, red_x,
            red_y, green_x, green_y, blue_x, blue_y);
      }
   }
#endif
#if defined(PNG_READ_gAMA_SUPPORTED) && defined(PNG_WRITE_gAMA_SUPPORTED)
   {
      double gamma;

      if (png_get_gAMA(read_ptr, read_info_ptr, &gamma))
      {
         png_set_gAMA(write_ptr, write_info_ptr, gamma);
      }
   }
#endif
#if defined(PNG_READ_hIST_SUPPORTED) && defined(PNG_WRITE_hIST_SUPPORTED)
   {
      png_uint_16p hist;

      if (png_get_hIST(read_ptr, read_info_ptr, &hist))
      {
         png_set_hIST(write_ptr, write_info_ptr, hist);
      }
   }
#endif
#if defined(PNG_READ_oFFs_SUPPORTED) && defined(PNG_WRITE_oFFs_SUPPORTED)
   {
      png_uint_32 offset_x, offset_y;
      int unit_type;

      if (png_get_oFFs(read_ptr, read_info_ptr,&offset_x,&offset_y,&unit_type))
      {
         png_set_oFFs(write_ptr, write_info_ptr, offset_x, offset_y, unit_type);
      }
   }
#endif
#if defined(PNG_READ_pCAL_SUPPORTED) && defined(PNG_WRITE_pCAL_SUPPORTED)
   {
      png_charp purpose, units;
      png_charpp params;
      png_int_32 X0, X1;
      int type, nparams;

      if (png_get_pCAL(read_ptr, read_info_ptr, &purpose, &X0, &X1, &type,
         &nparams, &units, &params))
      {
         png_set_pCAL(write_ptr, write_info_ptr, purpose, X0, X1, type,
            nparams, units, params);
      }
   }
#endif
#if defined(PNG_READ_pHYs_SUPPORTED) && defined(PNG_WRITE_pHYs_SUPPORTED)
   {
      png_uint_32 res_x, res_y;
      int unit_type;

      if (png_get_pHYs(read_ptr, read_info_ptr, &res_x, &res_y, &unit_type))
      {
         png_set_pHYs(write_ptr, write_info_ptr, res_x, res_y, unit_type);
      }
   }
#endif
   {
      png_colorp palette;
      int num_palette;

      if (png_get_PLTE(read_ptr, read_info_ptr, &palette, &num_palette))
      {
         png_set_PLTE(write_ptr, write_info_ptr, palette, num_palette);
      }
   }
#if defined(PNG_READ_sBIT_SUPPORTED) && defined(PNG_WRITE_sBIT_SUPPORTED)
   {
      png_color_8p sig_bit;

      if (png_get_sBIT(read_ptr, read_info_ptr, &sig_bit))
      {
         png_set_sBIT(write_ptr, write_info_ptr, sig_bit);
      }
   }
#endif
#if (defined(PNG_READ_tEXt_SUPPORTED) && defined(PNG_WRITE_tEXt_SUPPORTED)) || \
    (defined(PNG_READ_zTXt_SUPPORTED) && defined(PNG_WRITE_zTXt_SUPPORTED))
   {
      png_textp text_ptr;
      int num_text;

      if (png_get_text(read_ptr, read_info_ptr, &text_ptr, &num_text) > 0)
      {
         png_debug1(0, "Handling %d tEXt/zTXt chunks\n", num_text);
         png_set_text(write_ptr, write_info_ptr, text_ptr, num_text);
      }
   }
#endif
#if defined(PNG_READ_tIME_SUPPORTED) && defined(PNG_WRITE_tIME_SUPPORTED)
   {
      png_timep mod_time;

      if (png_get_tIME(read_ptr, read_info_ptr, &mod_time))
      {
         png_set_tIME(write_ptr, write_info_ptr, mod_time);
      }
   }
#endif
#if defined(PNG_READ_tRNS_SUPPORTED) && defined(PNG_WRITE_tRNS_SUPPORTED)
   {
      png_bytep trans;
      int num_trans;
      png_color_16p trans_values;

      if (png_get_tRNS(read_ptr, read_info_ptr, &trans, &num_trans,
         &trans_values))
      {
         png_set_tRNS(write_ptr, write_info_ptr, trans, num_trans,
            trans_values);
      }
   }
#endif

   png_debug(0, "\nWriting info struct\n");
   png_write_info(write_ptr, write_info_ptr);

   row_buf = (png_bytep)png_malloc(read_ptr, 
      png_get_rowbytes(read_ptr, read_info_ptr));
   if (row_buf == NULL)
   {
      fprintf(STDERR, "No memory to allocate row buffer\n");
      png_destroy_read_struct(&read_ptr, &read_info_ptr, (png_infopp)NULL);
      png_destroy_write_struct(&write_ptr, &write_info_ptr);
      fclose(fpin);
      fclose(fpout);
      return 1;
   }

   num_pass = png_set_interlace_handling(read_ptr);
   png_set_interlace_handling(write_ptr);

   for (pass = 0; pass < num_pass; pass++)
   {
      for (y = 0; y < height; y++)
      {
         png_read_rows(read_ptr, (png_bytepp)&row_buf, (png_bytepp)NULL, 1);
         png_write_rows(write_ptr, (png_bytepp)&row_buf, 1);
      }
   }

   png_debug(0, "Reading and writing end_info data\n");
   png_read_end(read_ptr, end_info_ptr);
   png_write_end(write_ptr, end_info_ptr);

   png_debug(0, "Destroying data structs\n");
   png_destroy_read_struct(&read_ptr, &read_info_ptr, &end_info_ptr);
   png_destroy_write_struct(&write_ptr, &write_info_ptr);

   fclose(fpin);
   fclose(fpout);

   png_free(read_ptr, row_buf);

   png_debug(0, "Opening files for comparison\n");
   if ((fpin = fopen(inname, "rb")) == NULL)
   {
      fprintf(STDERR, "Could not find file %s\n", inname);
      return 1;
   }

   if ((fpout = fopen(outname, "rb")) == NULL)
   {
      fprintf(STDERR, "Could not find file %s\n", outname);
      fclose(fpin);
      return 1;
   }

   while (1)
   {
      png_size_t num_in, num_out;

      num_in = fread(inbuf, 1, 1, fpin);
      num_out = fread(outbuf, 1, 1, fpout);

      if (num_in != num_out)
      {
         fprintf(STDERR, "Files %s and %s are of a different size\n",
                 inname, outname);
         fclose(fpin);
         fclose(fpout);
         return 1;
      }

      if (!num_in)
         break;

      if (png_memcmp(inbuf, outbuf, num_in))
      {
         fprintf(STDERR, "Files %s and %s are different\n", inname, outname);
         fclose(fpin);
         fclose(fpout);
         return 1;
      }
   }

   fclose(fpin);
   fclose(fpout);
   fprintf(STDERR, "libpng passes test\n");

   return 0;
}

