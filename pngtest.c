/* pngtest.c - a simple test program to test libpng

   libpng 1.0 beta 3 - version 0.89
   For conditions of distribution and use, see copyright notice in png.h
   Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.
   May 25, 1996
*/

#include <stdio.h>
#include <stdlib.h>
#include "png.h"

#ifdef __TURBOC__
#include <mem.h>
#endif

/* defined so I can write to a file on gui/windowing platforms */
/*  #define STDERR stderr  */
#define STDERR stdout   /* for DOS */

/* input and output filenames */
#ifdef RISCOS
char *inname = "pngtest_pn";
char *outname = "pngout_png";
#else
char *inname = "pngtest.png";
char *outname = "pngout.png";
#endif

char inbuf[256], outbuf[256];

int main(int argc, char *argv[])
{
   FILE *fpin, *fpout;
   png_structp read_ptr;
   png_structp write_ptr;
   png_infop info_ptr;
   png_infop end_info;
   png_bytep row_buf;
   png_byte *near_row_buf;
   png_uint_32 rowbytes;
   png_uint_32 y;
   int channels, num_pass, pass;

   row_buf = (png_bytep)NULL;
   near_row_buf = (png_byte *)NULL;

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

   fpin = fopen(inname, "rb");
   if (!fpin)
   {
      fprintf(STDERR, "Could not find input file %s\n", inname);
      return 1;
   }

   fpout = fopen(outname, "wb");
   if (!fpout)
   {
      fprintf(STDERR, "Could not open output file %s\n", outname);
      fclose(fpin);
      return 1;
   }

   read_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (void *)NULL,
      (png_error_ptr)NULL,  (png_error_ptr)NULL);
   write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, (void *)NULL,
      (png_error_ptr)NULL, (png_error_ptr)NULL);
   info_ptr = png_create_info_struct(read_ptr);
   end_info = png_create_info_struct(read_ptr);

   if (setjmp(read_ptr->jmpbuf))
   {
      fprintf(STDERR, "libpng read error\n");
      png_destroy_read_struct(&read_ptr, &info_ptr, &end_info);
      png_destroy_write_struct(&write_ptr, (png_infopp)NULL);
      fclose(fpin);
      fclose(fpout);
      return 1;
   }

   if (setjmp(write_ptr->jmpbuf))
   {
      fprintf(STDERR, "libpng write error\n");
      png_destroy_read_struct(&read_ptr, &info_ptr, &end_info);
      png_destroy_write_struct(&write_ptr, (png_infopp)NULL);
      fclose(fpin);
      fclose(fpout);
      return 1;
   }

   png_init_io(read_ptr, fpin);
   png_init_io(write_ptr, fpout);

   png_read_info(read_ptr, info_ptr);
   png_write_info(write_ptr, info_ptr);

   if ((info_ptr->color_type & PNG_COLOR_TYPE_PALETTE)==PNG_COLOR_TYPE_PALETTE)
      channels = 1;
   else
      channels = 3;
   if (info_ptr->color_type & PNG_COLOR_MASK_ALPHA)
      channels++;

   rowbytes = ((info_ptr->width * info_ptr->bit_depth * channels + 7) >> 3);
   near_row_buf = (png_byte *)malloc((size_t)rowbytes);
   row_buf = (png_bytep)near_row_buf;
   if (!row_buf)
   {
      fprintf(STDERR, "No memory to allocate row buffer\n");
      png_destroy_read_struct(&read_ptr, &info_ptr, &end_info);
      png_destroy_write_struct(&write_ptr, (png_infopp)NULL);
      fclose(fpin);
      fclose(fpout);
      return 1;
   }

   if (info_ptr->interlace_type)
   {
      num_pass = png_set_interlace_handling(read_ptr);
      num_pass = png_set_interlace_handling(write_ptr);
   }
   else
   {
      num_pass = 1;
   }

   for (pass = 0; pass < num_pass; pass++)
   {
      for (y = 0; y < info_ptr->height; y++)
      {
#ifdef TESTING
         fprintf(STDERR, "Processing line #%ld\n", y);
#endif
         png_read_rows(read_ptr, (png_bytepp)&row_buf, (png_bytepp)0, 1);
         png_write_rows(write_ptr, (png_bytepp)&row_buf, 1);
      }
   }

   png_read_end(read_ptr, end_info);
   png_write_end(write_ptr, end_info);

   png_destroy_read_struct(&read_ptr, &info_ptr, &end_info);
   png_destroy_write_struct(&write_ptr, (png_infopp)NULL);

   fclose(fpin);
   fclose(fpout);

   free((void *)near_row_buf);

   fpin = fopen(inname, "rb");

   if (!fpin)
   {
      fprintf(STDERR, "Could not find file %s\n", inname);
      return 1;
   }

   fpout = fopen(outname, "rb");
   if (!fpout)
   {
      fprintf(STDERR, "Could not find file %s\n", outname);
      fclose(fpin);
      return 1;
   }

   while (1)
   {
      int num_in, num_out;

      num_in = fread(inbuf, 1, 256, fpin);
      num_out = fread(outbuf, 1, 256, fpout);

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

      if (memcmp(inbuf, outbuf, num_in))
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

