/* pngcp.c
 *
 * Copyright (c) 2015 John Cunningham Bowler
 *
 * Last changed in libpng 1.6.18 [(PENDING RELEASE)]
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * This is a minimal example of copying a PNG without changes using the
 * png_read_png and png_write_png interfaces.
 *
 * For a more extensive example that uses the transforms see
 * contrib/libtests/pngimage.c in the libpng distribution.
 */
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>

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

#ifndef PNG_SETJMP_SUPPORTED
#  include <setjmp.h> /* because png.h did *not* include this */
#endif

#if (defined(PNG_READ_PNG_SUPPORTED)) && (defined(PNG_WRITE_PNG_SUPPORTED))
/* This structure is used to control the test of a single file. */
typedef enum
{
   VERBOSE,        /* switches on all messages */
   INFORMATION,
   WARNINGS,       /* switches on warnings */
   LIBPNG_WARNING,
   APP_WARNING,
   ERRORS,         /* just errors */
   APP_FAIL,       /* continuable error - no need to longjmp */
   LIBPNG_ERROR,   /* this and higher cause a longjmp */
   LIBPNG_BUG,     /* erroneous behavior in libpng */
   APP_ERROR,      /* such as out-of-memory in a callback */
   QUIET,          /* no normal messages */
   USER_ERROR,     /* such as file-not-found */
   INTERNAL_ERROR
} error_level;
#define LEVEL_MASK      0xf   /* where the level is in 'options' */

#define STRICT          0x010 /* Fail on warnings as well as errors */
#define LOG             0x020 /* Log pass/fail to stdout */
#define CONTINUE        0x040 /* Continue on APP_FAIL errors */

/* Result masks apply to the result bits in the 'results' field below; these
 * bits are simple 1U<<error_level.  A pass requires either nothing worse than
 * warnings (--relaxes) or nothing worse than information (--strict)
 */
#define RESULT_STRICT(r)   (((r) & ~((1U<<WARNINGS)-1)) == 0)
#define RESULT_RELAXED(r)  (((r) & ~((1U<<ERRORS)-1)) == 0)

struct display
{
   jmp_buf        error_return;      /* Where to go to on error */

   const char    *operation;         /* What is happening */
   const char    *filename;          /* The name of the original file */
   const char    *output_file;       /* The name of the output file */
   png_uint_32    options;           /* See display_log below */
   png_uint_32    results;           /* A mask of errors seen */

   /* Used on both read and write: */
   FILE          *fp;

   /* Used on a read, both the original read and when validating a written
    * image.
    */
   png_structp    read_pp;
   png_infop      ip;

   /* Used to write a new image (the original info_ptr is used) */
   png_structp    write_pp;
};

static void
display_init(struct display *dp)
   /* Call this only once right at the start to initialize the control
    * structure, the (struct buffer) lists are maintained across calls - the
    * memory is not freed.
    */
{
   memset(dp, 0, sizeof *dp);
   dp->options = WARNINGS; /* default to !verbose, !quiet */
   dp->filename = NULL;
   dp->fp = NULL;
   dp->read_pp = NULL;
   dp->ip = NULL;
   dp->output_file = NULL;
   dp->write_pp = NULL;
}

static void
display_clean_read(struct display *dp)
{
   if (dp->read_pp != NULL)
      png_destroy_read_struct(&dp->read_pp, NULL, NULL);

   if (dp->fp != NULL)
   {
      FILE *fp = dp->fp;
      dp->fp = NULL;
      (void)fclose(fp);
   }
}

static void
display_clean_write(struct display *dp)
{
   if (dp->fp != NULL)
   {
      FILE *fp = dp->fp;
      dp->fp = NULL;
      (void)fclose(fp);
   }

   if (dp->write_pp != NULL)
      png_destroy_write_struct(&dp->write_pp, &dp->ip);
}

static void
display_clean(struct display *dp)
{
   display_clean_read(dp);
   display_clean_write(dp);
   dp->output_file = NULL;

   /* leave the filename for error detection */
   dp->results = 0; /* reset for next time */
}

static void
display_destroy(struct display *dp)
{
   /* Release any memory held in the display. */
   display_clean(dp);
}

static struct display *
get_dp(png_structp pp)
   /* The display pointer is always stored in the png_struct error pointer */
{
   struct display *dp = (struct display*)png_get_error_ptr(pp);

   if (dp == NULL)
   {
      fprintf(stderr, "pngimage: internal error (no display)\n");
      exit(99); /* prevents a crash */
   }

   return dp;
}

/* error handling */
#ifdef __GNUC__
#  define VGATTR __attribute__((__format__ (__printf__,3,4)))
   /* Required to quiet GNUC warnings when the compiler sees a stdarg function
    * that calls one of the stdio v APIs.
    */
#else
#  define VGATTR
#endif
static void VGATTR
display_log(struct display *dp, error_level level, const char *fmt, ...)
   /* 'level' is as above, fmt is a stdio style format string.  This routine
    * does not return if level is above LIBPNG_WARNING
    */
{
   dp->results |= 1U << level;

   if (level > (error_level)(dp->options & LEVEL_MASK))
   {
      const char *lp;
      va_list ap;

      switch (level)
      {
         case INFORMATION:    lp = "information"; break;
         case LIBPNG_WARNING: lp = "warning(libpng)"; break;
         case APP_WARNING:    lp = "warning(pngimage)"; break;
         case APP_FAIL:       lp = "error(continuable)"; break;
         case LIBPNG_ERROR:   lp = "error(libpng)"; break;
         case LIBPNG_BUG:     lp = "bug(libpng)"; break;
         case APP_ERROR:      lp = "error(pngimage)"; break;
         case USER_ERROR:     lp = "error(user)"; break;

         case INTERNAL_ERROR: /* anything unexpected is an internal error: */
         case VERBOSE: case WARNINGS: case ERRORS: case QUIET:
         default:             lp = "bug(pngimage)"; break;
      }

      fprintf(stderr, "%s: %s: %s",
         dp->filename != NULL ? dp->filename : "<stdin>", lp, dp->operation);

      fprintf(stderr, ": ");

      va_start(ap, fmt);
      vfprintf(stderr, fmt, ap);
      va_end(ap);

      fputc('\n', stderr);
   }
   /* else do not output any message */

   /* Errors cause this routine to exit to the fail code */
   if (level > APP_FAIL || (level > ERRORS && !(dp->options & CONTINUE)))
      longjmp(dp->error_return, level);
}

/* error handler callbacks for libpng */
static void PNGCBAPI
display_warning(png_structp pp, png_const_charp warning)
{
   display_log(get_dp(pp), LIBPNG_WARNING, "%s", warning);
}

static void PNGCBAPI
display_error(png_structp pp, png_const_charp error)
{
   struct display *dp = get_dp(pp);

   display_log(dp, LIBPNG_ERROR, "%s", error);
}

static void
display_start_read(struct display *dp, const char *filename)
{
   if (filename != NULL)
   {
      dp->filename = filename;
      dp->fp = fopen(filename, "rb");
   }

   else
   {
      dp->filename = "<stdin>";
      dp->fp = stdin;
   }

   if (dp->fp == NULL)
      display_log(dp, APP_ERROR, "file open failed (%s)", strerror(errno));
}

static void PNGCBAPI
read_function(png_structp pp, png_bytep data, png_size_t size)
{
   struct display *dp = get_dp(pp);

   if (fread(data, size, 1U, dp->fp) != 1U)
   {
      if (feof(dp->fp))
         display_log(dp, USER_ERROR, "PNG file truncated");
      else
         display_log(dp, APP_ERROR, "PNG file read failed (%s)",
               strerror(errno));
   }
}

static void
read_png(struct display *dp, const char *filename)
{
   dp->operation = "read";
   display_clean_read(dp); /* safety */
   display_start_read(dp, filename);

   dp->read_pp = png_create_read_struct(PNG_LIBPNG_VER_STRING, dp,
      display_error, display_warning);
   if (dp->read_pp == NULL)
      display_log(dp, LIBPNG_ERROR, "failed to create read struct");

   /* The png_read_png API requires us to make the info struct, but it does the
    * call to png_read_info.
    */
   dp->ip = png_create_info_struct(dp->read_pp);
   if (dp->ip == NULL)
      display_log(dp, LIBPNG_ERROR, "failed to create info struct");

   /* Set the IO handling */
   png_set_read_fn(dp->read_pp, dp, read_function);

#  ifdef PNG_SET_UNKNOWN_CHUNKS_SUPPORTED
      png_set_keep_unknown_chunks(dp->read_pp, PNG_HANDLE_CHUNK_ALWAYS, NULL,
            0);
#  endif /* SET_UNKNOWN_CHUNKS */

#  ifdef PNG_SET_USER_LIMITS_SUPPORTED
      /* Remove the user limits, if any */
      png_set_user_limits(dp->read_pp, 0x7fffffff, 0x7fffffff);
#  endif /* SET_USER_LIMITS */

   /* Now read the PNG. */
   png_read_png(dp->read_pp, dp->ip, 0U/*transforms*/, NULL/*params*/);
   display_clean_read(dp);
   dp->operation = "none";
}

static void
display_start_write(struct display *dp, const char *filename)
{
   if (filename != NULL)
   {
      dp->output_file = filename;
      dp->fp = fopen(filename, "wb");
   }

   else
   {
      dp->output_file = "<stdout>";
      dp->fp = stdout;
   }

   if (dp->fp == NULL)
      display_log(dp, APP_ERROR, "%s: file open failed (%s)", dp->output_file,
            strerror(errno));
}

static void PNGCBAPI
write_function(png_structp pp, png_bytep data, png_size_t size)
{
   struct display *dp = get_dp(pp);

   if (fwrite(data, size, 1U, dp->fp) != 1U)
      display_log(dp, APP_ERROR, "%s: PNG file write failed (%s)",
            dp->output_file, strerror(errno));
}

static void
write_png(struct display *dp, const char *destname)
{
   dp->operation = "write";
   display_clean_write(dp); /* safety */
   display_start_write(dp, destname);

   dp->write_pp = png_create_write_struct(PNG_LIBPNG_VER_STRING, dp,
      display_error, display_warning);

   if (dp->write_pp == NULL)
      display_log(dp, APP_ERROR, "failed to create write png_struct");

   png_set_write_fn(dp->write_pp, dp, write_function, NULL/*flush*/);

#  ifdef PNG_SET_UNKNOWN_CHUNKS_SUPPORTED
      png_set_keep_unknown_chunks(dp->write_pp, PNG_HANDLE_CHUNK_ALWAYS, NULL,
            0);
#  endif /* SET_UNKNOWN_CHUNKS */

#  ifdef PNG_SET_USER_LIMITS_SUPPORTED
      /* Remove the user limits, if any */
      png_set_user_limits(dp->write_pp, 0x7fffffff, 0x7fffffff);
#  endif

   /* This just uses the 'read' info_struct directly, it contains the image. */
   png_write_png(dp->write_pp, dp->ip, 0U/*transforms*/, NULL/*params*/);

   /* Make sure the file was written ok: */
   {
      FILE *fp = dp->fp;
      dp->fp = NULL;
      if (fclose(fp))
         display_log(dp, APP_ERROR, "%s: write failed (%s)", destname,
               strerror(errno));
   }

   /* Clean it on the way out - if control returns to the caller then the
    * written_file contains the required data.
    */
   display_clean_write(dp);
   dp->operation = "none";
}

static void
cp_one_file(struct display *dp, const char *filename, const char *destname)
{
   /* Read it then write it: */
   read_png(dp, filename);
   write_png(dp, destname);
}

static int
cppng(struct display *dp, const char *file, const char *dest)
   /* Exists solely to isolate the setjmp clobbers */
{
   int ret = setjmp(dp->error_return);

   if (ret == 0)
   {
      cp_one_file(dp, file, dest);
      return 0;
   }

   else if (ret < ERRORS) /* shouldn't longjmp on warnings */
      display_log(dp, INTERNAL_ERROR, "unexpected return code %d", ret);

   return ret;
}

int
main(const int argc, const char * const * const argv)
{
   /* For each file on the command line test it with a range of transforms */
   int option_end, ilog = 0;
   struct display d;

   display_init(&d);

   for (option_end=1; option_end<argc; ++option_end)
   {
      const char *name = argv[option_end];

      if (strcmp(name, "--verbose") == 0)
         d.options = (d.options & ~LEVEL_MASK) | VERBOSE;

      else if (strcmp(name, "--warnings") == 0)
         d.options = (d.options & ~LEVEL_MASK) | WARNINGS;

      else if (strcmp(name, "--errors") == 0)
         d.options = (d.options & ~LEVEL_MASK) | ERRORS;

      else if (strcmp(name, "--quiet") == 0)
         d.options = (d.options & ~LEVEL_MASK) | QUIET;

      else if (strcmp(name, "--strict") == 0)
         d.options |= STRICT;

      else if (strcmp(name, "--relaxed") == 0)
         d.options &= ~STRICT;

      else if (strcmp(name, "--log") == 0)
      {
         ilog = option_end; /* prevent display */
         d.options |= LOG;
      }

      else if (strcmp(name, "--nolog") == 0)
         d.options &= ~LOG;

      else if (strcmp(name, "--continue") == 0)
         d.options |= CONTINUE;

      else if (strcmp(name, "--stop") == 0)
         d.options &= ~CONTINUE;

      else if (name[0] == '-' && name[1] == '-')
      {
         fprintf(stderr, "pngimage: %s: unknown option\n", name);
         return 99;
      }

      else
         break; /* Not an option */
   }

   {
      int errors = 0;
      int i = option_end;

      {
         const char *infile = NULL;
         const char *outfile = NULL;

         if (i < argc)
         {
            infile = argv[i++];
            if (i < argc)
               outfile = argv[i++];
         }

         {
            int ret = cppng(&d, infile, outfile);

            if (ret > QUIET) /* abort on user or internal error */
               return 99;
         }

         /* Here on any return, including failures, except user/internal issues
          */
         {
            const int pass = (d.options & STRICT) ?
               RESULT_STRICT(d.results) : RESULT_RELAXED(d.results);

            if (!pass)
               ++errors;

            if (d.options & LOG)
            {
               int j;

               printf("%s: pngimage", pass ? "PASS" : "FAIL");

               for (j=1; j<option_end; ++j) if (j != ilog)
                  printf(" %s", argv[j]);

               if (infile != NULL)
                  printf(" %s", infile);

               printf("\n");
            }
         }

         display_clean(&d);
      }

      /* Release allocated memory */
      display_destroy(&d);

      return errors != 0;
   }
}
#else /* !READ_PNG || !WRITE_PNG */
int
main(void)
{
   fprintf(stderr, "pngcp: no support for png_read/write_image\n");
   return 77;
}
#endif
