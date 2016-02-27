/* timepng.c
 *
 * Copyright (c) 2013,2016 John Cunningham Bowler
 *
 * Last changed in libpng 1.6.22 [February 25, 2016]
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * Load an arbitrary number of PNG files (from the command line, or, if there
 * are no arguments on the command line, from stdin) then run a time test by
 * reading each file by row or by image (possibly with transforms in the latter
 * case).  The only output is a time as a floating point number of seconds with
 * 9 decimal digits.
 */
#define _POSIX_C_SOURCE 199309L /* for clock_gettime */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <time.h>

#if defined(HAVE_CONFIG_H) && !defined(PNG_NO_CONFIG_H)
#  include <config.h>
#endif

/* Define the following to use this test against your installed libpng, rather
 * than the one being built here:
 */
#ifdef PNG_FREESTANDING_TESTS
#  include <png.h>
#else
#  include "../../png.h"
#endif

typedef struct
{
   FILE *input;
   FILE *output;
}  io_data;

static PNG_CALLBACK(void, read_and_copy,
      (png_structp png_ptr, png_bytep buffer, png_size_t cb))
{
   io_data *io = (io_data*)png_get_io_ptr(png_ptr);

   if (fread(buffer, cb, 1, io->input) != 1)
      png_error(png_ptr, strerror(errno));

   if (fwrite(buffer, cb, 1, io->output) != 1)
   {
      perror("temporary file");
      fprintf(stderr, "temporary file PNG write failed\n");
      exit(1);
   }
}

static void read_by_row(png_structp png_ptr, png_infop info_ptr,
      FILE *write_ptr, FILE *read_ptr)
{
   /* These don't get freed on error, this is fine; the program immediately
    * exits.
    */
   png_bytep row = NULL, display = NULL;
   io_data io_copy;

   if (write_ptr != NULL)
   {
      /* Set up for a copy to the temporary file: */
      io_copy.input = read_ptr;
      io_copy.output = write_ptr;
      png_set_read_fn(png_ptr, &io_copy, read_and_copy);
   }

   png_read_info(png_ptr, info_ptr);

   {
      png_size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);

      row = malloc(rowbytes);
      display = malloc(rowbytes);

      if (row == NULL || display == NULL)
         png_error(png_ptr, "OOM allocating row buffers");

      {
         png_uint_32 height = png_get_image_height(png_ptr, info_ptr);
         int passes = png_set_interlace_handling(png_ptr);
         int pass;

         png_start_read_image(png_ptr);

         for (pass = 0; pass < passes; ++pass)
         {
            png_uint_32 y = height;

            /* NOTE: this trashes the row each time; interlace handling won't
             * work, but this avoids memory thrashing for speed testing and is
             * somewhat representative of an application that works row-by-row.
             */
            while (y-- > 0)
               png_read_row(png_ptr, row, display);
         }
      }
   }

   /* Make sure to read to the end of the file: */
   png_read_end(png_ptr, info_ptr);

   /* Free this up: */
   free(row);
   free(display);
}

static PNG_CALLBACK(void, no_warnings, (png_structp png_ptr,
         png_const_charp warning))
{
   (void)png_ptr;
   (void)warning;
}

static int read_png(FILE *fp, png_int_32 transforms, FILE *write_file)
{
   png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,0,0,
         no_warnings);
   png_infop info_ptr = NULL;

   if (png_ptr == NULL)
      return 0;

   if (setjmp(png_jmpbuf(png_ptr)))
   {
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      return 0;
   }

   png_set_benign_errors(png_ptr, 1/*allowed*/);
   png_init_io(png_ptr, fp);

   info_ptr = png_create_info_struct(png_ptr);

   if (info_ptr == NULL)
      png_error(png_ptr, "OOM allocating info structure");

   if (transforms < 0)
      read_by_row(png_ptr, info_ptr, write_file, fp);

   else
      png_read_png(png_ptr, info_ptr, transforms, NULL/*params*/);

   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
   return 1;
}

static int mytime(struct timespec *t)
{
   /* Do the timing using clock_gettime and the per-process timer. */
   if (!clock_gettime(CLOCK_PROCESS_CPUTIME_ID, t))
      return 1;

   perror("CLOCK_PROCESS_CPUTIME_ID");
   fprintf(stderr, "timepng: could not get the time\n");
   return 0;
}

static int perform_one_test(FILE *fp, int nfiles, png_int_32 transforms)
{
   int i;
   struct timespec before, after;

   /* Clear out all errors: */
   rewind(fp);

   if (mytime(&before))
   {
      for (i=0; i<nfiles; ++i)
      {
         if (read_png(fp, transforms, NULL/*write*/))
         {
            if (ferror(fp))
            {
               perror("temporary file");
               fprintf(stderr, "file %d: error reading PNG data\n", i);
               return 0;
            }
         }

         else
         {
            perror("temporary file");
            fprintf(stderr, "file %d: error from libpng\n", i);
            return 0;
         }
      }
   }

   else
      return 0;

   if (mytime(&after))
   {
      /* Work out the time difference and print it - this is the only output,
       * so flush it immediately.
       */
      unsigned long s = after.tv_sec - before.tv_sec;
      long ns = after.tv_nsec - before.tv_nsec;

      if (ns < 0)
      {
         --s;
         ns += 1000000000;

         if (ns < 0)
         {
            fprintf(stderr, "timepng: bad clock from kernel\n");
            return 0;
         }
      }

      printf("%lu.%.9ld\n", s, ns);
      fflush(stdout);
      if (ferror(stdout))
      {
         fprintf(stderr, "timepng: error writing output\n");
         return 0;
      }

      /* Successful return */
      return 1;
   }

   else
      return 0;
}

static int add_one_file(FILE *fp, char *name)
{
   FILE *ip = fopen(name, "rb");

   if (ip != NULL)
   {
      /* Read the file using libpng; this detects errors and also deals with
       * files which contain data beyond the end of the file.
       */
      int ok = 0;
      fpos_t pos;

      if (fgetpos(fp, &pos))
      {
         /* Fatal error reading the start: */
         perror("temporary file");
         fprintf(stderr, "temporary file fgetpos error\n");
         exit(1);
      }

      if (read_png(ip, -1/*by row*/, fp/*output*/))
      {
         if (ferror(ip))
         {
            perror(name);
            fprintf(stderr, "%s: read error\n", name);
         }

         else
            ok = 1; /* read ok */
      }

      (void)fclose(ip);

      /* An error in the output is fatal; exit immediately: */
      if (ferror(fp))
      {
         perror("temporary file");
         fprintf(stderr, "temporary file write error\n");
         exit(1);
      }

      if (ok)
         return 1;

      /* Did not read the file successfully, simply rewind the temporary
       * file.  This must happen after the ferror check above to avoid clearing
       * the error.
       */
      if (fsetpos(fp, &pos))
      {
         perror("temporary file");
         fprintf(stderr, "temporary file fsetpos error\n");
         exit(1);
      }
   }

   else
   {
      /* file open error: */
      perror(name);
      fprintf(stderr, "%s: open failed\n", name);
      return 0;
   }

   return 1;
}

int main(int argc, char **argv)
{
   int ok = 0;
   int transforms = -1; /* by row */
   FILE *fp = tmpfile();

   if (fp != NULL)
   {
      int err = 0;
      int nfiles = 0;

      /* Remove and handle options first: */
      while (argc > 1 && argv[1][0] == '-' && argv[1][1] == '-')
      {
         const char *opt = *++argv + 2;

         --argc;

         /* Options turn on the by-image processing and maybe set some
          * transforms:
          */
         if (transforms == -1)
            transforms = PNG_TRANSFORM_IDENTITY;

         if (strcmp(opt, "by-image") == 0)
         {
            /* handled above */
         }

#        define OPT(name) else if (strcmp(opt, #name) == 0)\
            transforms |= PNG_TRANSFORM_ ## name

         OPT(STRIP_16);
         OPT(STRIP_ALPHA);
         OPT(PACKING);
         OPT(PACKSWAP);
         OPT(EXPAND);
         OPT(INVERT_MONO);
         OPT(SHIFT);
         OPT(BGR);
         OPT(SWAP_ALPHA);
         OPT(SWAP_ENDIAN);
         OPT(INVERT_ALPHA);
         OPT(STRIP_FILLER);
         OPT(STRIP_FILLER_BEFORE);
         OPT(STRIP_FILLER_AFTER);
         OPT(GRAY_TO_RGB);
         OPT(EXPAND_16);
         OPT(SCALE_16);
      }

      if (argc > 1)
      {
         int i;

         for (i=1; i<argc; ++i)
         {
            if (add_one_file(fp, argv[i]))
               ++nfiles;
         }
      }

      else
      {
         char filename[FILENAME_MAX+1];

         while (fgets(filename, FILENAME_MAX+1, stdin))
         {
            size_t len = strlen(filename);

            if (filename[len-1] == '\n')
            {
               filename[len-1] = 0;
               if (add_one_file(fp, filename))
                  ++nfiles;
            }

            else
            {
               fprintf(stderr, "timepng: file name too long: ...%s\n",
                  filename+len-32);
               err = 1;
               break;
            }
         }

         if (ferror(stdin))
         {
            fprintf(stderr, "timepng: stdin: read error\n");
            err = 1;
         }
      }

      if (!err)
      {
         if (nfiles > 0)
            ok = perform_one_test(fp, nfiles, transforms);

         else
            fprintf(stderr, "usage: timepng [options] {files}\n"
                            "   or: ls files | timepng [options]\n");
      }

      (void)fclose(fp);
   }

   else
      fprintf(stderr, "timepng: could not open temporary file\n");

   /* Exit code 0 on success. */
   return ok == 0;
}
