/*-
 * pngstest.c
 *
 * Copyright (c) 2012 John Cunningham Bowler
 *
 * Last changed in libpng 1.6.0 [(PENDING RELEASE)]
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * Test for the PNG 'simplified' APIs.
 */
#define _ISOC90_SOURCE 1
#define MALLOC_CHECK_ 2/*glibc facility: turn on debugging*/

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>

#if (defined HAVE_CONFIG_H) && !(defined PNG_NO_CONFIG_H)
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

#include "../tools/sRGB.h"

/* The following is to support direct compilation of this file as C++ */
#ifdef __cplusplus
#  define voidcast(type, value) static_cast<type>(value)
#else
#  define voidcast(type, value) (value)
#endif /* __cplusplus */

/* Generate random bytes.  This uses a boring repeatable algorithm and it
 * is implemented here so that it gives the same set of numbers on every
 * architecture.  It's a linear congruential generator (Knuth or Sedgewick
 * "Algorithms") but it comes from the 'feedback taps' table in Horowitz and
 * Hill, "The Art of Electronics".
 */
static void
make_random_bytes(png_uint_32* seed, void* pv, size_t size)
{
   png_uint_32 u0 = seed[0], u1 = seed[1];
   png_bytep bytes = voidcast(png_bytep, pv);

   /* There are thirty three bits, the next bit in the sequence is bit-33 XOR
    * bit-20.  The top 1 bit is in u1, the bottom 32 are in u0.
    */
   size_t i;
   for (i=0; i<size; ++i)
   {
      /* First generate 8 new bits then shift them in at the end. */
      png_uint_32 u = ((u0 >> (20-8)) ^ ((u1 << 7) | (u0 >> (32-7)))) & 0xff;
      u1 <<= 8;
      u1 |= u0 >> 24;
      u0 <<= 8;
      u0 |= u;
      *bytes++ = (png_byte)u;
   }

   seed[0] = u0;
   seed[1] = u1;
}

static void
random_color(png_colorp color)
{
   static png_uint_32 color_seed[2] = { 0x12345678, 0x9abcdef };
   make_random_bytes(color_seed, color, sizeof *color);
}

/* Math support - neither Cygwin nor Visual Studio have C99 support and we need
 * a predictable rounding function, so make one here:
 */
static double
closestinteger(double x)
{
   return floor(x + .5);
}

/* Cast support: remove GCC whines. */
static png_byte
u8d(double d)
{
   d = closestinteger(d);
   return (png_byte)d;
}

static png_uint_16
u16d(double d)
{
   d = closestinteger(d);
   return (png_uint_16)d;
}

/* sRGB support: use exact calculations rounded to the nearest int, see the
 * fesetround() call in main().
 */
static png_byte
sRGB(double linear /*range 0.0 .. 1.0*/)
{
   return u8d(255 * sRGB_from_linear(linear));
}

static png_byte
isRGB(int fixed_linear)
{
   return sRGB(fixed_linear / 65535.);
}

static png_uint_16
ilineara(int fixed_srgb, int alpha)
{
   return u16d((257 * alpha) * linear_from_sRGB(fixed_srgb / 255.));
}

static double
YfromRGBint(int ir, int ig, int ib)
{
   double r = ir;
   double g = ig;
   double b = ib;
   return YfromRGB(r, g, b);
}

/* The error that results from using a 2.2 power law in place of the correct
 * sRGB transform, given an 8-bit value which might be either sRGB or power-law.
 */
static int
power_law_error8(int value)
{
   if (value > 0 && value < 255)
   {
      double vd = value / 255.;
      double e = fabs(
         pow(linear_from_sRGB(vd), 1/2.2) - sRGB_from_linear(pow(vd, 2.2)));

      /* Always allow an extra 1 here for rounding errors */
      e = 1+floor(255 * e);
      return (int)e;
   }

   return 0;
}

static int error_in_sRGB_roundtrip = 56; /* by experiment */
static int
power_law_error16(int value)
{
   if (value > 0 && value < 65535)
   {
      /* Round trip the value through an 8-bit representation but using
       * non-matching to/from convertions.
       */
      double vd = value / 65535.;
      double e = fabs(
         pow(sRGB_from_linear(vd), 2.2) - linear_from_sRGB(pow(vd, 1/2.2)));

      /* Always allow an extra 1 here for rounding errors */
      e = error_in_sRGB_roundtrip+floor(65535 * e);
      return (int)e;
   }

   return 0;
}

static int
compare_8bit(int v1, int v2, int error_limit, int multiple_algorithms)
{
   int e = abs(v1-v2);
   int ev1, ev2;

   if (e <= error_limit)
      return 1;

   if (!multiple_algorithms)
      return 0;

   ev1 = power_law_error8(v1);
   if (e <= ev1)
      return 1;

   ev2 = power_law_error8(v2);
   if (e <= ev2)
      return 1;

   return 0;
}

static int
compare_16bit(int v1, int v2, int error_limit, int multiple_algorithms)
{
   int e = abs(v1-v2);
   int ev1, ev2;

   if (e <= error_limit)
      return 1;

   /* "multiple_algorithms" in this case means that a color-map has been
    * involved somewhere, so we can deduce that the values were forced to 8-bit
    * (like the via_linear case for 8-bit.)
    */
   if (!multiple_algorithms)
      return 0;

   ev1 = power_law_error16(v1);
   if (e <= ev1)
      return 1;

   ev2 = power_law_error16(v2);
   if (e <= ev2)
      return 1;

   return 0;
}

#define READ_FILE 1      /* else memory */
#define USE_STDIO 2      /* else use file name */
/* 4: unused */
#define VERBOSE 8
#define KEEP_TMPFILES 16 /* else delete temporary files */
#define KEEP_GOING 32

static void
print_opts(png_uint_32 opts)
{
   if (opts & READ_FILE)
      printf(" --file");
   if (opts & USE_STDIO)
      printf(" --stdio");
   if (opts & VERBOSE)
      printf(" --verbose");
   if (opts & KEEP_TMPFILES)
      printf(" --preserve");
   if (opts & KEEP_GOING)
      printf(" --keep-going");
}

#define FORMAT_NO_CHANGE 0x80000000 /* additional flag */

/* A name table for all the formats - defines the format of the '+' arguments to
 * pngstest.
 */
#define FORMAT_COUNT 64
#define FORMAT_MASK 0x3f
static PNG_CONST char * PNG_CONST format_names[FORMAT_COUNT] =
{
   "sRGB-gray",
   "sRGB-gray+alpha",
   "sRGB-rgb",
   "sRGB-rgb+alpha",
   "linear-gray",
   "linear-gray+alpha",
   "linear-rgb",
   "linear-rgb+alpha",

   "color-mapped-sRGB-gray",
   "color-mapped-sRGB-gray+alpha",
   "color-mapped-sRGB-rgb",
   "color-mapped-sRGB-rgb+alpha",
   "color-mapped-linear-gray",
   "color-mapped-linear-gray+alpha",
   "color-mapped-linear-rgb",
   "color-mapped-linear-rgb+alpha",

   "sRGB-gray",
   "sRGB-gray+alpha",
   "sRGB-bgr",
   "sRGB-bgr+alpha",
   "linear-gray",
   "linear-gray+alpha",
   "linear-bgr",
   "linear-bgr+alpha",

   "color-mapped-sRGB-gray",
   "color-mapped-sRGB-gray+alpha",
   "color-mapped-sRGB-bgr",
   "color-mapped-sRGB-bgr+alpha",
   "color-mapped-linear-gray",
   "color-mapped-linear-gray+alpha",
   "color-mapped-linear-bgr",
   "color-mapped-linear-bgr+alpha",

   "sRGB-gray",
   "alpha+sRGB-gray",
   "sRGB-rgb",
   "alpha+sRGB-rgb",
   "linear-gray",
   "alpha+linear-gray",
   "linear-rgb",
   "alpha+linear-rgb",

   "color-mapped-sRGB-gray",
   "color-mapped-alpha+sRGB-gray",
   "color-mapped-sRGB-rgb",
   "color-mapped-alpha+sRGB-rgb",
   "color-mapped-linear-gray",
   "color-mapped-alpha+linear-gray",
   "color-mapped-linear-rgb",
   "color-mapped-alpha+linear-rgb",

   "sRGB-gray",
   "alpha+sRGB-gray",
   "sRGB-bgr",
   "alpha+sRGB-bgr",
   "linear-gray",
   "alpha+linear-gray",
   "linear-bgr",
   "alpha+linear-bgr",

   "color-mapped-sRGB-gray",
   "color-mapped-alpha+sRGB-gray",
   "color-mapped-sRGB-bgr",
   "color-mapped-alpha+sRGB-bgr",
   "color-mapped-linear-gray",
   "color-mapped-alpha+linear-gray",
   "color-mapped-linear-bgr",
   "color-mapped-alpha+linear-bgr",
};

/* Decode an argument to a format number. */
static png_uint_32
formatof(const char *arg)
{
   char *ep;
   unsigned long format = strtoul(arg, &ep, 0);

   if (ep > arg && *ep == 0 && format < FORMAT_COUNT)
      return (png_uint_32)format;

   else for (format=0; format < FORMAT_COUNT; ++format)
   {
      if (strcmp(format_names[format], arg) == 0)
         return (png_uint_32)format;
   }

   fprintf(stderr, "pngstest: format name '%s' invalid\n", arg);
   return FORMAT_COUNT;
}

/* Bitset/test functions for formats */
#define FORMAT_SET_COUNT (FORMAT_COUNT / 32)
typedef struct
{
   png_uint_32 bits[FORMAT_SET_COUNT];
}
format_list;

static void format_init(format_list *pf)
{
   int i;
   for (i=0; i<FORMAT_SET_COUNT; ++i)
      pf->bits[i] = 0; /* All off */
}

#if 0 /* currently unused */
static void format_clear(format_list *pf)
{
   int i;
   for (i=0; i<FORMAT_SET_COUNT; ++i)
      pf->bits[i] = 0;
}
#endif

static int format_is_initial(format_list *pf)
{
   int i;
   for (i=0; i<FORMAT_SET_COUNT; ++i)
      if (pf->bits[i] != 0)
         return 0;

   return 1;
}

static int format_set(format_list *pf, png_uint_32 format)
{
   if (format < FORMAT_COUNT)
      return pf->bits[format >> 5] |= ((png_uint_32)1) << (format & 31);

   return 0;
}

#if 0 /* currently unused */
static int format_unset(format_list *pf, png_uint_32 format)
{
   if (format < FORMAT_COUNT)
      return pf->bits[format >> 5] &= ~((png_uint_32)1) << (format & 31);

   return 0;
}
#endif

static int format_isset(format_list *pf, png_uint_32 format)
{
   return format < FORMAT_COUNT &&
      (pf->bits[format >> 5] & (((png_uint_32)1) << (format & 31))) != 0;
}

static void format_default(format_list *pf, int redundant)
{
   if (redundant)
   {
      int i;
      
      /* set everything, including flags that are pointless */
      for (i=0; i<FORMAT_SET_COUNT; ++i)
         pf->bits[i] = ~(png_uint_32)0;
   }

   else
   {
      png_uint_32 f;

      for (f=0; f<FORMAT_COUNT; ++f)
      {
         /* Eliminate redundant settings. */
         /* BGR is meaningless if no color: */
         if ((f & PNG_FORMAT_FLAG_COLOR) == 0 && (f & PNG_FORMAT_FLAG_BGR) != 0)
            continue;

         /* AFIRST is meaningless if no alpha: */
         if ((f & PNG_FORMAT_FLAG_ALPHA) == 0 &&
            (f & PNG_FORMAT_FLAG_AFIRST) != 0)
            continue;

         format_set(pf, f);
      }
   }
}

/* THE Image STRUCTURE */
/* The super-class of a png_image, contains the decoded image plus the input
 * data necessary to re-read the file with a different format.
 */
typedef struct
{
   png_image   image;
   png_uint_32 opts;
   const char *file_name;
   int         stride_extra;
   FILE       *input_file;
   png_voidp   input_memory;
   png_size_t  input_memory_size;
   png_bytep   buffer;
   ptrdiff_t   stride;
   png_size_t  bufsize;
   png_size_t  allocsize;
   /* png_color   background; */
   char        tmpfile_name[32];
   png_uint_16 colormap[256*4];
}
Image;

/* Initializer: also sets the permitted error limit for 16-bit operations. */
static void
newimage(Image *image)
{
   memset(image, 0, sizeof *image);
}

/* Reset the image to be read again - only needs to rewind the FILE* at present.
 */
static void
resetimage(Image *image)
{
   if (image->input_file != NULL)
      rewind(image->input_file);
}

/* Free the image buffer; the buffer is re-used on a re-read, this is just for
 * cleanup.
 */
static void
freebuffer(Image *image)
{
   if (image->buffer) free(image->buffer);
   image->buffer = NULL;
   image->bufsize = 0;
   image->allocsize = 0;
}

/* Delete function; cleans out all the allocated data and the temporary file in
 * the image.
 */
static void
freeimage(Image *image)
{
   freebuffer(image);
   png_image_free(&image->image);

   if (image->input_file != NULL)
   {
      fclose(image->input_file);
      image->input_file = NULL;
   }

   if (image->input_memory != NULL)
   {
      free(image->input_memory);
      image->input_memory = NULL;
      image->input_memory_size = 0;
   }

   if (image->tmpfile_name[0] != 0 && (image->opts & KEEP_TMPFILES) == 0)
   {
      remove(image->tmpfile_name);
      image->tmpfile_name[0] = 0;
   }
}

/* This is actually a re-initializer; allows an image structure to be re-used by
 * freeing everything that relates to an old image.
 */
static void initimage(Image *image, png_uint_32 opts, const char *file_name,
   int stride_extra)
{
   freeimage(image);
   memset(&image->image, 0, sizeof image->image);
   image->opts = opts;
   image->file_name = file_name;
   image->stride_extra = stride_extra;
}

/* Make sure the image buffer is big enough; allows re-use of the buffer if the
 * image is re-read.
 */
#define BUFFER_INIT8 73
static void
allocbuffer(Image *image)
{
   png_size_t size = PNG_IMAGE_BUFFER_SIZE(image->image, image->stride);

   if (size+32 > image->bufsize)
   {
      freebuffer(image);
      image->buffer = voidcast(png_bytep, malloc(size+32));
      if (image->buffer == NULL)
      {
         fflush(stdout);
         fprintf(stderr,
            "simpletest: out of memory allocating %lu(+32) byte buffer\n",
            (unsigned long)size);
         exit(1);
      }
      image->bufsize = size+32;
   }

   memset(image->buffer, 95, image->bufsize);
   memset(image->buffer+16, BUFFER_INIT8, size);
   image->allocsize = size;
}

/* Make sure 16 bytes match the given byte. */
static int
check16(png_const_bytep bp, int b)
{
   int i = 16;

   do
      if (*bp != b) return 1;
   while (--i);

   return 0;
}

/* Check for overwrite in the image buffer. */
static void
checkbuffer(Image *image, const char *arg)
{
   if (check16(image->buffer, 95))
   {
      fflush(stdout);
      fprintf(stderr, "%s: overwrite at start of image buffer\n", arg);
      exit(1);
   }

   if (check16(image->buffer+16+image->allocsize, 95))
   {
      fflush(stdout);
      fprintf(stderr, "%s: overwrite at end of image buffer\n", arg);
      exit(1);
   }
}

/* ERROR HANDLING */
/* Log a terminal error, also frees the libpng part of the image if necessary.
 */
static int
logerror(Image *image, const char *a1, const char *a2, const char *a3)
{
   fflush(stdout);
   if (image->image.warning_or_error)
      fprintf(stderr, "%s%s%s: %s\n", a1, a2, a3, image->image.message);

   else
      fprintf(stderr, "%s%s%s\n", a1, a2, a3);

   if (image->image.opaque != NULL)
   {
      fprintf(stderr, "%s: image opaque pointer non-NULL on error\n",
         image->file_name);
      png_image_free(&image->image);
   }

   return 0;
}

/* Log an error and close a file (just a utility to do both things in one
 * function call.)
 */
static int
logclose(Image *image, FILE *f, const char *name, const char *operation)
{
   int e = errno;

   fclose(f);
   return logerror(image, name, operation, strerror(e));
}

/* Make sure the png_image has been freed - validates that libpng is doing what
 * the spec says and freeing the image.
 */
static int
checkopaque(Image *image)
{
   if (image->image.opaque != NULL)
   {
      png_image_free(&image->image);
      return logerror(image, image->file_name, ": opaque not NULL", "");
   }

   else
      return 1;
}

/* IMAGE COMPARISON/CHECKING */
/* Compare the pixels of two images, which should be the same but aren't.  The
 * images must have been checked for a size match.
 */
typedef struct
{
   png_uint_32 format;
   png_uint_16 r16, g16, b16, y16, a16;
   png_byte    r8, g8, b8, y8, a8;
} Pixel;

/* This is not particularly fast, but it works.  The input has pixels stored
 * either as pre-multiplied linear 16-bit or as sRGB encoded non-pre-multiplied
 * 8-bit values.  The routine reads either and does exact conversion to the
 * other format.
 *
 * Grayscale values are mapped r==g==b=y.  Non-alpha images have alpha
 * 65535/255.  Color images have a correctly calculated Y value using the sRGB Y
 * calculation.
 *
 * Colors are looked up in the color map if required.
 *
 * The API returns false if an error is detected; this can only be if the alpha
 * value is less than the component in the linear case.
 */
static int 
get_pixel(Image *image, Pixel *pixel, png_const_bytep pp)
{
   png_uint_32 format = image->image.format;
   int result = 1;

   if (format & PNG_FORMAT_FLAG_COLORMAP)
      pp = (png_bytep)image->colormap + PNG_IMAGE_SAMPLE_SIZE(format) * *pp;

   pixel->format = format;

   /* Initialize the alpha values for opaque: */
   pixel->a8 = 255;
   pixel->a16 = 65535;

   switch (PNG_IMAGE_SAMPLE_COMPONENT_SIZE(format))
   {
      default:
         fflush(stdout);
         fprintf(stderr, "pngstest: impossible sample component size: %lu\n",
            (unsigned long)PNG_IMAGE_SAMPLE_COMPONENT_SIZE(format));
         exit(1);

      case sizeof (png_uint_16):
         {
            png_const_uint_16p up = (png_const_uint_16p)pp;

            if ((format & PNG_FORMAT_FLAG_AFIRST) != 0 &&
               (format & PNG_FORMAT_FLAG_ALPHA) != 0)
               pixel->a16 = *up++;

            if ((format & PNG_FORMAT_FLAG_COLOR) != 0)
            {
               if ((format & PNG_FORMAT_FLAG_BGR) != 0)
               {
                  pixel->b16 = *up++;
                  pixel->g16 = *up++;
                  pixel->r16 = *up++;
               }

               else
               {
                  pixel->r16 = *up++;
                  pixel->g16 = *up++;
                  pixel->b16 = *up++;
               }

               /* Because the 'Y' calculation is linear the pre-multiplication
                * of the r16,g16,b16 values can be ignored.
                */
               pixel->y16 = u16d(YfromRGBint(pixel->r16, pixel->g16,
                  pixel->b16));
            }

            else
               pixel->r16 = pixel->g16 = pixel->b16 = pixel->y16 = *up++;

            if ((format & PNG_FORMAT_FLAG_AFIRST) == 0 &&
               (format & PNG_FORMAT_FLAG_ALPHA) != 0)
               pixel->a16 = *up++;

            /* 'a1' is 1/65535 * 1/alpha, for alpha in the range 0..1 */
            if (pixel->a16 == 0)
            {
               if (pixel->r16 > 0 || pixel->g16 > 0 || pixel->b16 > 0)
                  result = 0;

               pixel->r8 = pixel->g8 = pixel->b8 = pixel->y8 = 255;
               pixel->a8 = 0;
            }

            else
            {
               double a1 = 1. / pixel->a16;

               if (pixel->a16 < pixel->r16)
                  result = 0, pixel->r8 = 255;
               else
                  pixel->r8 = sRGB(pixel->r16 * a1);

               if (pixel->a16 < pixel->g16)
                  result = 0, pixel->g8 = 255;
               else
                  pixel->g8 = sRGB(pixel->g16 * a1);

               if (pixel->a16 < pixel->b16)
                  result = 0, pixel->b8 = 255;
               else
                  pixel->b8 = sRGB(pixel->b16 * a1);

               if (pixel->a16 < pixel->y16)
                  result = 0, pixel->y8 = 255;
               else
                  pixel->y8 = sRGB(pixel->y16 * a1);

               /* The 8-bit alpha value is just a16/257. */
               pixel->a8 = u8d(pixel->a16 / 257.);
            }
         }
         break;

      case sizeof (png_byte):
         {
            double y;

            if ((format & PNG_FORMAT_FLAG_AFIRST) != 0 &&
               (format & PNG_FORMAT_FLAG_ALPHA) != 0)
               pixel->a8 = *pp++;

            if ((format & PNG_FORMAT_FLAG_COLOR) != 0)
            {
               if ((format & PNG_FORMAT_FLAG_BGR) != 0)
               {
                  pixel->b8 = *pp++;
                  pixel->g8 = *pp++;
                  pixel->r8 = *pp++;
               }

               else
               {
                  pixel->r8 = *pp++;
                  pixel->g8 = *pp++;
                  pixel->b8 = *pp++;
               }

               /* The y8 value requires convert to linear, convert to &, convert
                * to sRGB:
                */
               y = YfromRGB(linear_from_sRGB(pixel->r8/255.),
                  linear_from_sRGB(pixel->g8/255.),
                  linear_from_sRGB(pixel->b8/255.));

               pixel->y8 = sRGB(y);
            }

            else
            {
               pixel->r8 = pixel->g8 = pixel->b8 = pixel->y8 = *pp++;
               y = linear_from_sRGB(pixel->y8/255.);
            }

            if ((format & PNG_FORMAT_FLAG_AFIRST) == 0 &&
               (format & PNG_FORMAT_FLAG_ALPHA) != 0)
               pixel->a8 = *pp++;

            pixel->r16 = ilineara(pixel->r8, pixel->a8);
            pixel->g16 = ilineara(pixel->g8, pixel->a8);
            pixel->b16 = ilineara(pixel->b8, pixel->a8);
            pixel->y16 = u16d((257 * pixel->a8) * y);
            pixel->a16 = (png_uint_16)(pixel->a8 * 257);
         }
         break;
   }

   return result;
}

/* Two pixels are equal if the value of the left equals the value of the right
 * as defined by the format of the right, or if it is close enough given the
 * permitted error limits.  If the formats match the values should (exactly!)
 *
 * If the right pixel has no alpha channel but the left does, it was removed
 * somehow.  For an 8-bit *output* removal uses the background color if given
 * else the default (the value filled in to the row buffer by allocbuffer()
 * above.)
 *
 * The result of this function is NULL if the pixels match else a reason why
 * they don't match.
 *
 * Error values below are inflated because some of the conversions are done
 * inside libpng using a simple power law transform of .45455 and others are
 * done in the simplified API code using the correct sRGB tables.  This needs
 * to be made consistent.
 */
static int error_to_linear = 811; /* by experiment */
static int error_to_linear_grayscale = 424; /* by experiment */
static int error_to_sRGB = 6; /* by experiment */
static int error_to_sRGB_grayscale = 11; /* by experiment */
static int error_in_compose = 0;
static int error_via_linear = 14; /* by experiment */
static int error_in_premultiply = 1;

static const char *
cmppixel(Pixel *a, Pixel *b, const png_color *background, int via_linear,
   int multiple_algorithms)
{
   int error_limit = 0;

   if (b->format & PNG_FORMAT_FLAG_LINEAR)
   {
      /* If the input was non-opaque then use the pre-multiplication error
       * limit.
       */
      if ((a->format & PNG_FORMAT_FLAG_ALPHA) && a->a16 < 65535 &&
         error_limit < error_in_premultiply)
         error_limit = error_in_premultiply;

      if (b->format & PNG_FORMAT_FLAG_ALPHA)
      {
         if ((b->format & PNG_FORMAT_FLAG_COLORMAP) == 0 ||
            (a->format & PNG_FORMAT_FLAG_COLORMAP) != 0 ||
            (a->format & PNG_FORMAT_FLAG_ALPHA) == 0)
         {
            /* Expect an exact match. */
            if (b->a16 != a->a16)
               return "linear alpha mismatch";
         }

         else
         {
            /* Transform from non-color-mapped format with alpha to color-map
             * with alpha.  Most alphs is lost.
             */
            if (b->format & PNG_FORMAT_FLAG_COLOR)
            {
               /* Color; three levels of alpha (only!) */
               if (abs(b->a16 - a->a16) > 16384)
                  return "linear color-mapped color alpha mismatch";
            }

            else
            {
               /* Grayscale (GA palette), 6 levels of alpha. */
               if (abs(b->a16 - a->a16) > 6554)
                  return "linear color-mapped gray alpha mismatch";
            }

            /* If the alpha ends up as zero skip any check on the color
             * components.
             */
            if (b->a16 == 0 && b->y16 == 0)
               return NULL;
         }
      }

      else if (a->format & PNG_FORMAT_FLAG_ALPHA)
      {
         /* An alpha channel has been removed, the destination is linear so the
          * removal algorithm is just the premultiplication - compose on black -
          * and the 16-bit colors are correct already.
          */
      }

      if (b->format & PNG_FORMAT_FLAG_COLOR)
      {
         const char *err = "linear color mismatch";

         /* Check for an exact match. */
         if (a->r16 == b->r16 && a->g16 == b->g16 && a->b16 == b->b16)
            return NULL;

         /* Not an exact match; allow drift only if the input is 8-bit */
         if (!(a->format & PNG_FORMAT_FLAG_LINEAR))
         {
            if (error_limit < error_to_linear)
            {
               error_limit = error_to_linear;
               err = "sRGB to linear conversion error";
            }
         }

         /* Well, ok, if the file is color-mapped the color-mapping probably
          * used colors spaced at 51 in sRGB space, so there is massive drift to
          * be allowed here.
          */
         if (b->format & PNG_FORMAT_FLAG_COLORMAP)
         {
            /* If the input (a) was detectably grayscale then just permit the
             * grayscale errors; we require libpng to at least do this.
             */
            if ((a->format & PNG_FORMAT_FLAG_COLOR) == 0)
            {
               png_byte v = isRGB(a->y16);

               if (b->r8 == v && b->g8 == v && b->b8 == v)
                  return NULL;

               if ((a->format & PNG_FORMAT_FLAG_ALPHA) != 0 &&
                  (b->format & PNG_FORMAT_FLAG_ALPHA) == 0) /* alpha removed */
               {
                  /* Alpha was removed by compose-on-black; fix up the pixel a
                   * '8-bit' values to match.
                   */
                  a->r8 = isRGB(a->r16);
                  a->g8 = isRGB(a->g16);
                  a->b8 = isRGB(a->b16);
                  a->y8 = isRGB(a->y16);

                  if (b->y8 == 255 && a->y8 == 254)
                     return NULL; /* transparency hacked 254->255 */

                  else if (b->y8 == 254 && a->a8 != 0)
                     return "possible error in transparency hack (color)";
               }

               if ((b->format & PNG_FORMAT_FLAG_ALPHA) != 0)
               {
                  /* GA color-map; limited accuracy for opaque pixels, +/- 26
                   * accuracy for partially transparent ones.
                   */
                  if (error_limit < 1)
                     error_limit = 1;

                  if (a->a8 > 0 && a->a8 < 255)
                  {
                     if (error_limit < 26)
                        error_limit = 26;
                  }
               }
            }

            else /* input is not detectably grayscale */
            {
               /* The input was forced into an sRGB 666 color-map; error +/-26,
                * guess the error limit from the actual input values.
                */
               int red = (isRGB(a->r16)+25)/51;
               int green = (isRGB(a->g16)+25)/51;
               int blue = (isRGB(a->b16)+25)/51;

               if ((red-1)*51 <= b->r8 && (red+1)*51 >= b->r8 &&
                  (green-1)*51 <= b->g8 && (green+1)*51 >= b->g8 &&
                  (blue-1)*51 <= b->b8 && (blue+1)*51 >= b->b8)
                  return NULL;

               return "666 color-map error";
            }

            /* Now compare the 8-bit values, not the 16-bit ones. */
            if (compare_8bit(a->r8, b->r8, error_limit, multiple_algorithms) &&
               compare_8bit(a->g8, b->g8, error_limit, multiple_algorithms) &&
               compare_8bit(a->b8, b->b8, error_limit, multiple_algorithms))
               return NULL;

            return "linear color-map color mismatch";
         }

         else if (compare_16bit(a->r16, b->r16, error_limit,
               multiple_algorithms) &&
            compare_16bit(a->g16, b->g16, error_limit, multiple_algorithms) &&
            compare_16bit(a->b16, b->b16, error_limit, multiple_algorithms))
            return NULL;

         return err;
      }

      else /* b is grayscale */
      {
         const char *err = "linear gray mismatch";

         /* Check for an exact match. */
         if (a->y16 == b->y16 && a->a16 == b->a16)
            return NULL;

         /* Not an exact match; allow drift only if the input is 8-bit or if it
          * has been converted from color.
          */
         if (!(a->format & PNG_FORMAT_FLAG_LINEAR))
         {
            /* Converted to linear, check for that drift. */
            if (error_limit < error_to_linear)
            {
               error_limit = error_to_linear;
               err = "8-bit gray to linear conversion error";
            }

            if (abs(a->y16-b->y16) <= error_to_linear)
               return NULL;

         }

         if (a->format & PNG_FORMAT_FLAG_COLOR)
         {
            /* Converted to grayscale, allow drift */
            if (error_limit < error_to_linear_grayscale)
            {
               error_limit = error_to_linear_grayscale;
               err = "color to linear gray conversion error";
            }
         }

         if (b->format & PNG_FORMAT_FLAG_COLORMAP)
         {
            /* Forced into a colormap, since the format is a grayscale one we
             * can calculate the permitted error from the sRGB bucket the value
             * should fall into.
             */
            png_byte v = a->y8;

            if (b->y8 == v && a->a8 == b->a8)
               return NULL;

            if ((a->format & PNG_FORMAT_FLAG_ALPHA) != 0 &&
               (b->format & PNG_FORMAT_FLAG_ALPHA) == 0) /* alpha removed */
            {
               /* Alpha was removed by compose-on-black; fix up the pixel a
                * '8-bit' values to match.
                */
               a->r8 = isRGB(a->r16);
               a->g8 = isRGB(a->g16);
               a->b8 = isRGB(a->b16);
               a->y8 = isRGB(a->y16);

               if (b->y8 == 255 && a->y8 == 254)
                  return NULL; /* transparency hacked 254->255 */

               else if (b->y8 == 254 && a->a8 != 0)
                  return "possible error in transparency hack (gray)";
            }

            if ((b->format & PNG_FORMAT_FLAG_ALPHA) != 0)
            {
               /* GA color-map; limited accuracy for opaque pixels, +/- 26
                * accuracy for partially transparent ones.
                */
               if (error_limit < 1)
                  error_limit = 1;

               if (a->a8 > 0 && a->a8 < 255)
               {
                  if (error_limit < 26)
                     error_limit = 26;
               }
            }

            /* And compare the 8-bit values, not the 16-bit ones. */
            if (compare_8bit(a->y8, b->y8, error_limit, multiple_algorithms))
               return NULL;

            return "linear color-map gray mismatch";
         }

         else if (compare_16bit(a->y16, b->y16, error_limit,
            multiple_algorithms))
            return NULL;

         return err;
      }
   }

   else /* RHS is 8-bit */
   {
      const char *err;

      /* For 8-bit to 8-bit use 'error_via_linear'; this handles the cases where
       * the original image is compared with the output of another conversion:
       * see where the parameter is set to non-zero below.
       */
      if (!(a->format & PNG_FORMAT_FLAG_LINEAR) && via_linear)
         error_limit = error_via_linear;

      if (b->format & PNG_FORMAT_FLAG_COLOR)
         err = "8-bit color mismatch";
      
      else
         err = "8-bit gray mismatch";

      /* If the original data had an alpha channel and was not pre-multiplied
       * pre-multiplication may lose precision in non-opaque pixel values.  If
       * the output is linear the premultiplied 16-bit values will be used, but
       * if 'via_linear' is set an intermediate 16-bit pre-multiplied form has
       * been used and this must be taken into account here.
       */
      if (via_linear && (a->format & PNG_FORMAT_FLAG_ALPHA) &&
         !(a->format & PNG_FORMAT_FLAG_LINEAR) &&
         a->a16 < 65535)
      {
         if (a->a16 > 0)
         {
            /* First calculate the rounded 16-bit component values, (r,g,b) or y
             * as appropriate, then back-calculate the 8-bit values for
             * comparison below.
             */
            if (a->format & PNG_FORMAT_FLAG_COLOR)
            {
               double r = closestinteger((65535. * a->r16) / a->a16)/65535;
               double g = closestinteger((65535. * a->g16) / a->a16)/65535;
               double blue = closestinteger((65535. * a->b16) / a->a16)/65535;

               a->r16 = u16d(r * a->a16);
               a->g16 = u16d(g * a->a16);
               a->b16 = u16d(blue * a->a16);
               a->y16 = u16d(YfromRGBint(a->r16, a->g16, a->b16));

               a->r8 = u8d(r * 255);
               a->g8 = u8d(g * 255);
               a->b8 = u8d(blue * 255);
               a->y8 = u8d(255 * YfromRGB(r, g, blue));
            }

            else
            {
               double y = closestinteger((65535. * a->y16) / a->a16)/65535.;

               a->b16 = a->g16 = a->r16 = a->y16 = u16d(y * a->a16);
               a->b8 = a->g8 = a->r8 = a->y8 = u8d(255 * y);
            }
         }

         else
         {
            a->r16 = a->g16 = a->b16 = a->y16 = 0;
            a->r8 = a->g8 = a->b8 = a->y8 = 255;
         }
      }


      if (b->format & PNG_FORMAT_FLAG_ALPHA)
      {
         /* Expect an exact match on the 8 bit value. */
         if (b->a8 != a->a8)
            return "8-bit alpha mismatch";

         /* If the input was not color-mapped but the output is transparent
          * pixels will have been forced to just one palette entry, with the
          * value 255,255,255,0.
          */
         if ((a->format & PNG_FORMAT_FLAG_COLORMAP) == 0 &&
            (b->format & PNG_FORMAT_FLAG_COLORMAP) != 0 &&
             (a->format & PNG_FORMAT_FLAG_ALPHA) != 0 &&
             a->a16 == 0)
         {
            if (b->format & PNG_FORMAT_FLAG_COLOR)
            {
               if (b->r8 == 255 && b->g8 == 255 && b->b8 == 255 && b->a8 == 0)
                  return NULL;

               return "bad RGB color-map transparent entry";
            }

            else if (b->y8 == 255 && b->a8 == 0)
               return NULL;

            return "bad gray color-map transparent entry";
         }

         /* If the *input* was linear+alpha as well libpng will have converted
          * the non-premultiplied format directly to the sRGB non-premultiplied
          * format and the precision loss on an intermediate pre-multiplied
          * format will have been avoided.  In this case we will get spurious
          * values in the non-opaque pixels.
          */
         if (!via_linear && (a->format & PNG_FORMAT_FLAG_LINEAR) != 0 &&
            (a->format & PNG_FORMAT_FLAG_ALPHA) != 0 &&
            a->a16 < 65535)
         {
            /* We don't know the original values (libpng has already removed
             * them) but we can make sure they are in range here by doing a
             * comparison on the pre-multiplied values instead.
             */
            if (a->a16 > 0)
            {
               if (b->format & PNG_FORMAT_FLAG_COLOR)
               {
                  double r, g, blue;

                  r = (255. * b->r16)/b->a16;
                  b->r8 = u8d(r);

                  g = (255. * b->g16)/b->a16;
                  b->g8 = u8d(g);

                  blue = (255. * b->b16)/b->a16;
                  b->b8 = u8d(blue);

                  b->y8 = u8d(YfromRGB(r, g, blue));
               }

               else
               {
                  b->r8 = b->g8 = b->b8 = b->y8 =
                     u8d((255. * b->y16)/b->a16);
               }
            }

            else
               b->r8 = b->g8 = b->b8 = b->y8 = 255;
         }
      }

      else if (a->format & PNG_FORMAT_FLAG_ALPHA)
      {
         png_uint_32 alpha;

         /* An alpha channel has been removed; the background will have been
          * composed in.  Adjust the 'a' pixel to represent this by doing the
          * correct compose.  Set the error limit, above, to an appropriate
          * value for the compose operation.
          */
         if (error_limit < error_in_compose)
            error_limit = error_in_compose;

         alpha = 65535 - a->a16; /* for the background */

         if (b->format & PNG_FORMAT_FLAG_COLOR) /* background is rgb */
         {
            err = "8-bit color compose error";

            if (via_linear)
            {
               /* The 16-bit values are already correct (being pre-multiplied),
                * just recalculate the 8-bit values.
                */
               a->r8 = isRGB(a->r16);
               a->g8 = isRGB(a->g16);
               a->b8 = isRGB(a->b16);
               a->y8 = isRGB(a->y16);

               /* There should be no libpng error in this (ideally) */
               error_limit = 0;
            }

            else if (background == NULL)
            {
               double add = alpha * linear_from_sRGB(BUFFER_INIT8/255.);
               double r, g, blue, y;

               r = a->r16 + add;
               a->r16 = u16d(r);
               a->r8 = sRGB(r/65535);

               g = a->g16 + add;
               a->g16 = u16d(g);
               a->g8 = sRGB(g/65535);

               blue = a->b16 + add;
               a->b16 = u16d(blue);
               a->b8 = sRGB(blue/65535);

               y = YfromRGB(r, g, blue);
               a->y16 = u16d(y);
               a->y8 = sRGB(y/65535);
            }

            else
            {
               double r, g, blue, y;

               r = a->r16 + alpha * linear_from_sRGB(background->red/255.);
               a->r16 = u16d(r);
               a->r8 = sRGB(r/65535);

               g = a->g16 + alpha * linear_from_sRGB(background->green/255.);
               a->g16 = u16d(g);
               a->g8 = sRGB(g/65535);

               blue = a->b16 + alpha * linear_from_sRGB(background->blue/255.);
               a->b16 = u16d(blue);
               a->b8 = sRGB(blue/65535);

               y = YfromRGB(r, g, blue);
               a->y16 = u16d(y * 65535);
               a->y8 = sRGB(y);
            }
         }

         else /* background is gray */
         {
            err = "8-bit gray compose error";

            if (via_linear)
            {
               a->r8 = a->g8 = a->b8 = a->y8 = isRGB(a->y16);
               error_limit = 0;
            }

            else
            {
               /* When the output is gray the background comes from just the
                * green channel.
                */
               double y = a->y16 + alpha * linear_from_sRGB(
                  (background == NULL ? BUFFER_INIT8 : background->green)/255.);

               a->r16 = a->g16 = a->b16 = a->y16 = u16d(y);
               a->r8 = a->g8 = a->b8 = a->y8 = sRGB(y/65535);
            }
         }

         /* NOTE: the alpha channel is the original one, so logpixel will show
          * the original alpha but the composed color channels.  This gives
          * linear values that are apparently wrong on error, but is useful.
          */
      }

      if (b->format & PNG_FORMAT_FLAG_COLOR)
      {

         /* Check for an exact match. */
         if (a->r8 == b->r8 && a->g8 == b->g8 && a->b8 == b->b8)
            return NULL;

         /* Check for linear to 8-bit conversion. */
         if (a->format & PNG_FORMAT_FLAG_LINEAR)
         {
            if (error_limit < error_to_sRGB)
            {
               err = "linear to sRGB conversion error";
               error_limit = error_to_sRGB;
            }
         }

         /* Check for color-map trashing. */
         if (b->format & PNG_FORMAT_FLAG_COLORMAP)
         {
            /* The data has been forced into an RGB666 colormap.  Unless the
             * original was detectably grayscale or color-mapped (we expect
             * color maps to be preserved.)
             */
            if ((a->format & PNG_FORMAT_FLAG_COLOR) == 0)
            {
               /* grayscale input, currently generates no additional errors */
            }

            else if ((a->format & PNG_FORMAT_FLAG_COLORMAP) == 0)
            {
               /* color-map input */
               if (error_limit < 26)
                  error_limit = 26;
            }

            else
            {
               /* Color-map to color-map: expect no errors. */
            }
         }

         if (compare_8bit(a->r8, b->r8, error_limit, multiple_algorithms) &&
            compare_8bit(a->g8, b->g8, error_limit, multiple_algorithms) &&
            compare_8bit(a->b8, b->b8, error_limit, multiple_algorithms))
            return NULL;

         return err;
      }

      else /* b is grayscale */
      {
         /* Check for an exact match. */
         if (a->y8 == b->y8)
            return NULL;

         /* Not an exact match; allow drift only if the input is linear or if it
          * has been converted from color.
          */
         if (a->format & PNG_FORMAT_FLAG_LINEAR)
         {
            /* Converted to linear, check for that drift. */
            if (error_limit < error_to_sRGB)
            {
               error_limit = error_to_sRGB;
               err = "linear to 8-bit gray conversion error";
            }
         }

         if (a->format & PNG_FORMAT_FLAG_COLOR)
         {
            /* Converted to grayscale, allow drift */
            if (error_limit < error_to_sRGB_grayscale)
            {
               error_limit = error_to_sRGB_grayscale;
               err = "color to 8-bit gray conversion error";
            }
         }

         if (compare_8bit(a->y8, b->y8, error_limit, multiple_algorithms))
            return NULL;

         return err;
      }
   }
}

/* Basic image formats; control the data but not the layout thereof. */
#define BASE_FORMATS\
   (PNG_FORMAT_FLAG_ALPHA|PNG_FORMAT_FLAG_COLOR|PNG_FORMAT_FLAG_LINEAR)

static void
print_pixel(char string[64], Pixel *pixel)
{
   switch (pixel->format & BASE_FORMATS)
   {
      case 0: /* 8-bit, one channel */
         sprintf(string, "%s(%d)", format_names[pixel->format], pixel->y8);
         break;

      case PNG_FORMAT_FLAG_ALPHA:
         sprintf(string, "%s(%d,%d)", format_names[pixel->format], pixel->y8,
            pixel->a8);
         break;

      case PNG_FORMAT_FLAG_COLOR:
         sprintf(string, "%s(%d,%d,%d)", format_names[pixel->format],
            pixel->r8, pixel->g8, pixel->b8);
         break;

      case PNG_FORMAT_FLAG_COLOR|PNG_FORMAT_FLAG_ALPHA:
         sprintf(string, "%s(%d,%d,%d,%d)", format_names[pixel->format],
            pixel->r8, pixel->g8, pixel->b8, pixel->a8);
         break;

      case PNG_FORMAT_FLAG_LINEAR:
         sprintf(string, "%s(%d)", format_names[pixel->format], pixel->y16);
         break;

      case PNG_FORMAT_FLAG_LINEAR|PNG_FORMAT_FLAG_ALPHA:
         sprintf(string, "%s(%d,%d)", format_names[pixel->format], pixel->y16,
            pixel->a16);
         break;

      case PNG_FORMAT_FLAG_LINEAR|PNG_FORMAT_FLAG_COLOR:
         sprintf(string, "%s(%d,%d,%d)", format_names[pixel->format],
            pixel->r16, pixel->g16, pixel->b16);
         break;

      case PNG_FORMAT_FLAG_LINEAR|PNG_FORMAT_FLAG_COLOR|PNG_FORMAT_FLAG_ALPHA:
         sprintf(string, "%s(%d,%d,%d,%d)", format_names[pixel->format],
            pixel->r16, pixel->g16, pixel->b16, pixel->a16);
         break;

      default:
         sprintf(string, "invalid-format");
         break;
   }
}

static int
logpixel(Image *original, Image *copy, png_uint_32 x, png_uint_32 y, Pixel *a,
   Pixel *b, const char *reason)
{
   char pixel_a[64], pixel_b[64];

   print_pixel(pixel_a, a);
   print_pixel(pixel_b, b);
   if (original->file_name != copy->file_name)
   {
      char error_buffer[256];
      sprintf(error_buffer,
         "(%lu,%lu) %s:\n\t%s ->\n\t\t%s\n\tUse --preserve and examine: ",
         (unsigned long)x, (unsigned long)y, reason, pixel_a, pixel_b);
      return logerror(original, original->file_name, error_buffer,
         copy->file_name);
   }

   else
   {
      char error_buffer[256];
      sprintf(error_buffer,
         "(%lu,%lu) %s:\n\t%s ->\n\t\t%s.\n"
         "\tThe error happened when reading the original file with this format",
         (unsigned long)x, (unsigned long)y, reason, pixel_a, pixel_b);
      return logerror(original, original->file_name, error_buffer, "");
   }
}

static int
badpixel(Image *ia, png_uint_32 x, png_uint_32 y, Pixel *pa, const char *reason)
{
   char pixel_a[64];
   char error_buffer[128];

   print_pixel(pixel_a, pa);
   sprintf(error_buffer, "(%lu,%lu) %s: ", (unsigned long)x, (unsigned long)y,
      reason);
   return logerror(ia, ia->file_name, error_buffer, pixel_a);
}

/* Compare two images, the original 'a', which was written out then read back in
 * to * give image 'b'.  The formats may have been changed.
 */
static int
compare_two_images(Image *a, Image *b, int via_linear,
   png_const_colorp background)
{
   png_uint_32 width = a->image.width;
   png_uint_32 height = a->image.height;
   png_uint_32 formata = a->image.format;
   png_uint_32 formatb = b->image.format;
   ptrdiff_t stridea = a->stride;
   ptrdiff_t strideb = b->stride;
   png_const_bytep rowa = a->buffer+16;
   png_const_bytep rowb = b->buffer+16;
   png_byte channels;
   int fast_track = 0;
   int two_algorithms = ((formata ^ formatb) & PNG_FORMAT_FLAG_COLORMAP) != 0;
   int result = 1;
   unsigned int check_alpha = 0; /* must be zero or one */
   png_byte swap_mask[4];
   png_uint_32 x, y;
   png_const_bytep ppa, ppb;

   /* This should never happen: */
   if (width != b->image.width || height != b->image.height)
      return logerror(a, a->file_name, ": width x height changed: ",
         b->file_name);

   /* Find the first row and inter-row space. */
   if (!(formata & PNG_FORMAT_FLAG_COLORMAP) &&
      (formata & PNG_FORMAT_FLAG_LINEAR))
      stridea *= 2;

   if (!(formatb & PNG_FORMAT_FLAG_COLORMAP) &&
      (formatb & PNG_FORMAT_FLAG_LINEAR))
      strideb *= 2;

   if (stridea < 0) rowa += (height-1) * (-stridea);
   if (strideb < 0) rowb += (height-1) * (-strideb);

   /* The following are used only if the formats match, except that 'channels'
    * is a flag for matching formats.
    */
   channels = 0;
   swap_mask[3] = swap_mask[2] = swap_mask[1] = swap_mask[0] = 0;

   /* Set up the masks if no base format change, or if the format change was
    * just to add an alpha channel (note that this ignores whether or not the
    * image is color-mapped.)
    */
   if (((formata & BASE_FORMATS) == (formatb & BASE_FORMATS)) ||
      ((formata | PNG_FORMAT_FLAG_ALPHA) & BASE_FORMATS) ==
         (formatb & BASE_FORMATS))
   {
      png_byte astart = 0; /* index of first component */
      png_byte bstart = 0;

      /* Set to the actual number of channels in 'a' */
      if (formata & PNG_FORMAT_FLAG_COLOR)
         channels = 3U;
      else
         channels = 1U;

      if (formata & PNG_FORMAT_FLAG_ALPHA)
      {
         /* Both formats have an alpha channel */
         if (formata & PNG_FORMAT_FLAG_AFIRST)
         {
            astart = 1;

            if (formatb & PNG_FORMAT_FLAG_AFIRST)
            {
               bstart = 1;
               swap_mask[0] = 0;
            }

            else
               swap_mask[0] = channels; /* 'b' alpha is at end */
         }

         else if (formatb & PNG_FORMAT_FLAG_AFIRST)
         {
            /* 'a' alpha is at end, 'b' is at start (0) */
            bstart = 1;
            swap_mask[channels] = 0;
         }

         else
            swap_mask[channels] = channels;

         ++channels;
      }

      else if (formatb & PNG_FORMAT_FLAG_ALPHA)
      {
         /* Only 'b' has an alpha channel */
         check_alpha = 1;
         if (formatb & PNG_FORMAT_FLAG_AFIRST)
         {
            bstart = 1;
            /* Put the location of the alpha channel in swap_mask[3], since it
             * cannot be used if 'a' does not have an alpha channel.
             */
            swap_mask[3] = 0;
         }

         else
            swap_mask[3] = channels;
      }

      if (formata & PNG_FORMAT_FLAG_COLOR)
      {
         unsigned int swap = 0;

         /* Colors match, but are they swapped? */
         if ((formata ^ formatb) & PNG_FORMAT_FLAG_BGR) /* Swapped. */
            swap = 2;

         swap_mask[astart+0] = (png_byte)(bstart+(0^swap));
         swap_mask[astart+1] = (png_byte)(bstart+1);
         swap_mask[astart+2] = (png_byte)(bstart+(2^swap));
      }

      else /* grayscale: 1 channel */
         swap_mask[astart] = bstart;

      /* Now work out if the fast-track match is possible - the byte
       * representations need to be equivalent (apart from the addition of an
       * opaque alpha channel) but allow indirection via a color-map
       */
      {
         png_uint_32 f = (formata & formatb);

         if (formata & PNG_FORMAT_FLAG_COLORMAP)
            fast_track += 4; /* image a color-mapped */

         if (formatb & PNG_FORMAT_FLAG_COLORMAP)
            fast_track += 8; /* image b color-mapped */

         if (fast_track == 12)
         {
            /* Do the color-maps match, entry by entry?   Always do this the
             * slow way unless the maps are identical, the number of entries
             * must match.
             */
            unsigned int entries = a->image.colormap_entries;

            if (entries == b->image.colormap_entries)
            {
               unsigned int entry = 0;

               while (entry < entries)
               {
                  Pixel pixel_a, pixel_b;
                  png_byte p = (png_byte)entry;

                  if (!get_pixel(a, &pixel_a, &p))
                     return badpixel(a, entry, 0, &pixel_a,
                        "bad palette entry value");

                  if (!get_pixel(b, &pixel_b, &p))
                     return badpixel(b, entry, 0, &pixel_b,
                        "bad palette entry value");

                  if (cmppixel(&pixel_a, &pixel_b, background, via_linear,
                     0/*multiple_algorithms*/) != NULL)
                     break;

                  ++entry;
               }

               /* both sides color-mapped, color-maps match */
               if (entry == entries)
                  fast_track += 1;

               /* else color-map entries are mismatched so compare pixel by
                * pixel.
                */
            }
         }

         else if (f & PNG_FORMAT_FLAG_LINEAR)
            fast_track += 2; /* linear */

         else if (!((formata | formatb) & PNG_FORMAT_FLAG_LINEAR))
            fast_track += 3; /* sRGB */
      }
   }

   ppa = rowa;
   ppb = rowb;
   for (x=y=0; y<height;)
   {
      /* Do the fast test if possible. */
      switch (fast_track)
      {
         case 8+4+1: /* both sides color-mapped and color-maps match */
            while (x < width)
            {
               if (ppa[0] != ppb[0])
                  break;

               /* This pixel matches, advance to the next. */
               ++ppa;
               ++ppb;
               ++x;
            }
            break;

         case 2: /* both sides double byte, neither color-mapped */
            {
               png_const_uint_16p lppa = (png_const_uint_16p)ppa;
               png_const_uint_16p lppb = (png_const_uint_16p)ppb;

               while (x < width) switch (channels)
               {
                  case 4:
                     if (lppa[3] != lppb[swap_mask[3]])
                        goto linear_mismatch;
                  case 3:
                     if (lppa[2] != lppb[swap_mask[2]])
                        goto linear_mismatch;
                  case 2:
                     if (lppa[1] != lppb[swap_mask[1]])
                        goto linear_mismatch;
                  case 1:
                     if (lppa[0] != lppb[swap_mask[0]])
                        goto linear_mismatch;

                     /* The pixels apparently match, but if an alpha channel has
                      * been added (in b) it must be 65535 too.
                      */
                     if (check_alpha && 65535 != lppb[swap_mask[3]])
                        goto linear_mismatch;

                     /* This pixel matches, advance to the next. */
                     lppa += channels;
                     lppb += channels + check_alpha;
                     ++x;
                  default:
                     goto linear_mismatch;
               }

            linear_mismatch:
               ppa = (png_const_bytep)lppa;
               ppb = (png_const_bytep)lppb;
            }
            break;

         case 4+2: /* both sides double byte, imagea is color-mapped */
            {
               png_const_uint_16p lppb = (png_const_uint_16p)ppb;

               while (x < width)
               {
                  png_const_uint_16p lppa = a->colormap + channels * *ppa;

                  switch (channels)
                  {
                     case 4:
                        if (lppa[3] != lppb[swap_mask[3]])
                           goto linear_colormapa_mismatch;
                     case 3:
                        if (lppa[2] != lppb[swap_mask[2]])
                           goto linear_colormapa_mismatch;
                     case 2:
                        if (lppa[1] != lppb[swap_mask[1]])
                           goto linear_colormapa_mismatch;
                     case 1:
                        if (lppa[0] != lppb[swap_mask[0]])
                           goto linear_colormapa_mismatch;

                        /* The pixels apparently match, but if an alpha channel
                         * has been added (in b) it must be 65535 too.
                         */
                        if (check_alpha && 65535 != lppb[swap_mask[3]])
                           goto linear_colormapa_mismatch;

                        /* This pixel matches, advance to the next. */
                        ppa += 1;
                        lppb += channels + check_alpha;
                        ++x;
                     default:
                        goto linear_colormapa_mismatch;
                  }
               }

            linear_colormapa_mismatch:
               ppb = (png_const_bytep)lppb;
            }
            break;

         case 8+2: /* both sides double byte, imageb color-mapped */
            {
               png_const_uint_16p lppa = (png_const_uint_16p)ppa;

               while (x < width)
               {
                  png_const_uint_16p lppb = b->colormap + channels * *ppb;

                  switch (channels)
                  {
                     case 4:
                        if (lppa[3] != lppb[swap_mask[3]])
                           goto linear_colormapb_mismatch;
                     case 3:
                        if (lppa[2] != lppb[swap_mask[2]])
                           goto linear_colormapb_mismatch;
                     case 2:
                        if (lppa[1] != lppb[swap_mask[1]])
                           goto linear_colormapb_mismatch;
                     case 1:
                        if (lppa[0] != lppb[swap_mask[0]])
                           goto linear_colormapb_mismatch;

                        /* The pixels apparently match, but if an alpha channel
                         * has been added (in b) it must be 65535 too.
                         */
                        if (check_alpha && 65535 != lppb[swap_mask[3]])
                           goto linear_colormapb_mismatch;

                        /* This pixel matches, advance to the next. */
                        lppa += channels;
                        ppb += 1;
                        ++x;
                     default:
                        goto linear_colormapb_mismatch;
                  }
               }

            linear_colormapb_mismatch:
               ppa = (png_const_bytep)lppa;
            }
            break;

         case 3: /* both sides sRGB, neither color-mapped */
            while (x < width) switch (channels)
            {
               case 4:
                  if (ppa[3] != ppb[swap_mask[3]])
                     goto sRGB_mismatch;
               case 3:
                  if (ppa[2] != ppb[swap_mask[2]])
                     goto sRGB_mismatch;
               case 2:
                  if (ppa[1] != ppb[swap_mask[1]])
                     goto sRGB_mismatch;
               case 1:
                  if (ppa[0] != ppb[swap_mask[0]])
                     goto sRGB_mismatch;

                  /* The pixels apparently match, but if an alpha channel has
                   * been added (in b) it must be 1.0 too.
                   */
                  if (check_alpha && 255 != ppb[swap_mask[3]])
                     goto sRGB_mismatch;

                  /* This pixel matches, advance to the next. */
                  ppa += channels;
                  ppb += channels + check_alpha;
                  ++x;
               default:
                  goto sRGB_mismatch;
            }

         sRGB_mismatch:
            break;

         case 4+3: /* both sides sRGB, imagea color-mapped */
            while (x < width)
            {
               png_const_bytep colormap_a = (png_const_bytep)a->colormap;
               
               switch (channels)
               {
                  case 4:
                     if (colormap_a[ppa[3]] != ppb[swap_mask[3]])
                        goto sRGB_colormapa_mismatch;
                  case 3:
                     if (colormap_a[ppa[2]] != ppb[swap_mask[2]])
                        goto sRGB_colormapa_mismatch;
                  case 2:
                     if (colormap_a[ppa[1]] != ppb[swap_mask[1]])
                        goto sRGB_colormapa_mismatch;
                  case 1:
                     if (colormap_a[ppa[0]] != ppb[swap_mask[0]])
                        goto sRGB_mismatch;

                     /* The pixels apparently match, but if an alpha channel has
                      * been added (in b) it must be 1.0 too.
                      */
                     if (check_alpha && 255 != ppb[swap_mask[3]])
                        goto sRGB_colormapa_mismatch;

                     /* This pixel matches, advance to the next. */
                     ppa += 1;
                     ppb += channels + check_alpha;
                     ++x;
                  default:
                     goto sRGB_colormapa_mismatch;
               }
            }

         sRGB_colormapa_mismatch:
            break;

         case 8+3: /* both sides sRGB, imageb color-mapped */
            while (x < width)
            {
               png_const_bytep colormap_b = (png_const_bytep)b->colormap;
               
               switch (channels)
               {
                  case 4:
                     if (ppa[3] != colormap_b[ppb[swap_mask[3]]])
                        goto sRGB_colormapb_mismatch;
                  case 3:
                     if (ppa[2] != colormap_b[ppb[swap_mask[2]]])
                        goto sRGB_colormapb_mismatch;
                  case 2:
                     if (ppa[1] != colormap_b[ppb[swap_mask[1]]])
                        goto sRGB_colormapb_mismatch;
                  case 1:
                     if (ppa[0] != colormap_b[ppb[swap_mask[0]]])
                        goto sRGB_colormapb_mismatch;

                     /* The pixels apparently match, but if an alpha channel has
                      * been added (in b) it must be 1.0 too.
                      */
                     if (check_alpha && 255 != colormap_b[ppb[swap_mask[3]]])
                        goto sRGB_colormapb_mismatch;

                     /* This pixel matches, advance to the next. */
                     ppa += channels;
                     ppb += 1;
                     ++x;
                  default:
                     goto sRGB_colormapb_mismatch;
               }
            }

         sRGB_colormapb_mismatch:
            break;

         default: /* formats do not match */
            break;
      }

      /* If at the end of the row advance to the next row, if not at the end
       * compare the pixels the slow way.
       */
      if (x < width)
      {
         Pixel pixel_a, pixel_b;
         const char *mismatch;

         if (!get_pixel(a, &pixel_a, ppa))
            return badpixel(a, x, y, &pixel_a, "bad pixel value");

         if (!get_pixel(b, &pixel_b, ppb))
            return badpixel(b, x, y, &pixel_b, "bad pixel value");

         mismatch = cmppixel(&pixel_a, &pixel_b, background, via_linear,
            two_algorithms);

         if (mismatch != NULL)
         {
            (void)logpixel(a, b, x, y, &pixel_a, &pixel_b, mismatch);

            if ((a->opts & KEEP_GOING) == 0)
               return 0;

            result = 0;
         }

         ++x;
      }

      if (x >= width)
      {
         x = 0;
         ++y;
         rowa += stridea;
         rowb += strideb;
         ppa = rowa;
         ppb = rowb;
      }
   }

   return result;
}

/* Read the file; how the read gets done depends on which of input_file and
 * input_memory have been set.
 */
static int
read_file(Image *image, png_uint_32 format, png_const_colorp background)
{
   memset(&image->image, 0, sizeof image->image);
   image->image.version = PNG_IMAGE_VERSION;

   if (image->input_memory != NULL)
   {
      if (!png_image_begin_read_from_memory(&image->image, image->input_memory,
         image->input_memory_size))
         return logerror(image, "memory init: ", image->file_name, "");
   }

   else if (image->input_file != NULL)
   {
      if (!png_image_begin_read_from_stdio(&image->image, image->input_file))
         return logerror(image, "stdio init: ", image->file_name, "");
   }

   else
   {
      if (!png_image_begin_read_from_file(&image->image, image->file_name))
         return logerror(image, "file init: ", image->file_name, "");
   }

   /* Have an initialized image with all the data we need plus, maybe, an
    * allocated file (myfile) or buffer (mybuffer) that need to be freed.
    */
   {
      int result;
      png_uint_32 image_format;

      /* Print both original and output formats. */
      image_format = image->image.format;

      if (image->opts & VERBOSE)
      {
         printf("%s %lu x %lu %s -> %s", image->file_name,
            (unsigned long)image->image.width,
            (unsigned long)image->image.height,
            format_names[image_format & FORMAT_MASK],
            (format & FORMAT_NO_CHANGE) != 0 || image->image.format == format
            ? "no change" : format_names[format & FORMAT_MASK]);

         if (background != NULL)
            printf(" background(%d,%d,%d)\n", background->red,
               background->green, background->blue);
         else
            printf("\n");

         fflush(stdout);
      }

      /* 'NO_CHANGE' combined with the color-map flag forces the base format
       * flags to be set on read to ensure that the original representation is
       * not lost in the pass through a colormap format.
       */
      if ((format & FORMAT_NO_CHANGE) != 0)
      {
         if ((format & PNG_FORMAT_FLAG_COLORMAP) != 0 &&
            (image_format & PNG_FORMAT_FLAG_COLORMAP) != 0)
            format = (image_format & ~BASE_FORMATS) | (format & BASE_FORMATS);

         else
            format = image_format;
      }

      image->image.format = format;

      image->stride = PNG_IMAGE_ROW_STRIDE(image->image) + image->stride_extra;
      allocbuffer(image);

      result = png_image_finish_read(&image->image, background,
         image->buffer+16, (png_int_32)image->stride, image->colormap);

      checkbuffer(image, image->file_name);

      if (result)
         return checkopaque(image);

      else
         return logerror(image, image->file_name, ": image read failed", "");
   }
}

/* Reads from a filename, which must be in image->file_name, but uses
 * image->opts to choose the method.  The file is always read in its native
 * format (the one the simplified API suggests).
 */
static int
read_one_file(Image *image)
{
   if (!(image->opts & READ_FILE) || (image->opts & USE_STDIO))
   {
      /* memory or stdio. */
      FILE *f = fopen(image->file_name, "rb");

      if (f != NULL)
      {
         if (image->opts & READ_FILE)
            image->input_file = f;

         else /* memory */
         {
            if (fseek(f, 0, SEEK_END) == 0)
            {
               long int cb = ftell(f);

               if (cb >= 0 && (unsigned long int)cb < (size_t)~(size_t)0)
               {
                  png_bytep b = voidcast(png_bytep, malloc((size_t)cb));

                  if (b != NULL)
                  {
                     rewind(f);

                     if (fread(b, (size_t)cb, 1, f) == 1)
                     {
                        fclose(f);
                        image->input_memory_size = cb;
                        image->input_memory = b;
                     }

                     else
                     {
                        free(b);
                        return logclose(image, f, image->file_name,
                           ": read failed");
                     }
                  }

                  else
                     return logclose(image, f, image->file_name,
                        ": out of memory");
               }

               else
                  return logclose(image, f, image->file_name, ": tell failed");
            }

            else
               return logclose(image, f, image->file_name, ": seek failed: ");
         }
      }

      else
         return logerror(image, image->file_name, ": open failed: ",
            strerror(errno));
   }

   return read_file(image, FORMAT_NO_CHANGE, NULL);
}

static int
write_one_file(Image *output, Image *image, int convert_to_8bit)
{
   if (image->opts & USE_STDIO)
   {
      FILE *f = tmpfile();

      if (f != NULL)
      {
         if (png_image_write_to_stdio(&image->image, f, convert_to_8bit,
            image->buffer+16, (png_int_32)image->stride, image->colormap))
         {
            if (fflush(f) == 0)
            {
               rewind(f);
               initimage(output, image->opts, "tmpfile", image->stride_extra);
               output->input_file = f;
               if (!checkopaque(image))
                  return 0;
            }

            else
               return logclose(image, f, "tmpfile", ": flush");
         }

         else
         {
            fclose(f);
            return logerror(image, "tmpfile", ": write failed", "");
         }
      }

      else
         return logerror(image, "tmpfile", ": open: ", strerror(errno));
   }

   else
   {
      static int counter = 0;
      char name[32];

      sprintf(name, "TMP%d.png", ++counter);

      if (png_image_write_to_file(&image->image, name, convert_to_8bit,
         image->buffer+16, (png_int_32)image->stride, image->colormap))
      {
         initimage(output, image->opts, output->tmpfile_name,
            image->stride_extra);
         /* Afterwards, or freeimage will delete it! */
         strcpy(output->tmpfile_name, name);

         if (!checkopaque(image))
            return 0;
      }

      else
         return logerror(image, name, ": write failed", "");
   }

   /* 'output' has an initialized temporary image, read this back in and compare
    * this against the original: there should be no change since the original
    * format was written unmodified unless 'convert_to_8bit' was specified.
    * However, if the original image was color-mapped, a simple read will zap
    * the linear, color and maybe alpha flags, this will cause spurious failures
    * under some circumstances.
    */
   if (read_file(output, image->image.format | FORMAT_NO_CHANGE, NULL))
   {
      png_uint_32 original_format = image->image.format;

      if (convert_to_8bit)
         original_format &= ~PNG_FORMAT_FLAG_LINEAR;

      if ((output->image.format & BASE_FORMATS) !=
         (original_format & BASE_FORMATS))
         return logerror(image, image->file_name, ": format changed on read: ",
            output->file_name);

      return compare_two_images(image, output, 0/*via linear*/, NULL);
   }

   else
      return logerror(output, output->tmpfile_name,
         ": read of new file failed", "");
}

static int
testimage(Image *image, png_uint_32 opts, format_list *pf)
{
   int result;
   Image copy;

   /* Copy the original data, stealing it from 'image' */
   checkopaque(image);
   copy = *image;

   copy.opts = opts;
   copy.buffer = NULL;
   copy.bufsize = 0;
   copy.allocsize = 0;

   image->input_file = NULL;
   image->input_memory = NULL;
   image->input_memory_size = 0;
   image->tmpfile_name[0] = 0;

   {
      png_uint_32 counter;
      Image output;

      newimage(&output);
      
      result = 1;

      /* Use the low bit of 'counter' to indicate whether or not to do alpha
       * removal with a background color or by composting onto the image; this
       * step gets skipped if it isn't relevant
       */
      for (counter=0; counter<2*FORMAT_COUNT; ++counter)
         if (format_isset(pf, counter >> 1))
      {
         png_uint_32 format = counter >> 1;

         png_color background_color;
         png_colorp background = NULL;

         /* If there is a format change that removes the alpha channel then
          * the background is relevant.  If the output is 8-bit color-mapped
          * then a background color *must* be provided, otherwise there are
          * two tests to do - one with a color, the other with NULL.  The
          * NULL test happens second.
          */
         if ((counter & 1) == 0)
         {
            if ((format & PNG_FORMAT_FLAG_ALPHA) == 0 &&
               (image->image.format & PNG_FORMAT_FLAG_ALPHA) != 0)
            {
               /* Alpha/transparency will be removed, the background is
                * relevant: make it a color the first time
                */
               random_color(&background_color);
               background = &background_color;

               /* BUT if the output is to a color-mapped 8-bit format then
                * the background must always be a color, so increment 'counter'
                * to skip the NULL test.
                */
               if ((format & PNG_FORMAT_FLAG_COLORMAP) != 0 &&
                  (format & PNG_FORMAT_FLAG_LINEAR) == 0)
                  ++counter;
            }

            /* Otherwise an alpha channel is not being eliminated, just leave
             * background NULL and skip the (counter & 1) NULL test.
             */
            else
               ++counter;
         }
         /* else just use NULL for background */


         resetimage(&copy);
         copy.opts = opts; /* in case read_file needs to change it */

         result = read_file(&copy, format, background);
         if (!result)
            break;

         /* Make sure the file just read matches the original file. */
         result = compare_two_images(image, &copy, 0/*via linear*/, background);
         if (!result)
            break;

         /* Write the *copy* just made to a new file to make sure the write side
          * works ok.  Check the conversion to sRGB if the copy is linear.
          */
         output.opts = opts;
         result = write_one_file(&output, &copy, 0/*convert to 8bit*/);
         if (!result)
            break;

         /* Validate against the original too; the background is needed here
          * as well so that compare_two_images knows what color was used.
          */
         result = compare_two_images(image, &output, 0, background);
         if (!result)
            break;

         if ((format & PNG_FORMAT_FLAG_LINEAR) != 0 &&
            (format & PNG_FORMAT_FLAG_COLORMAP) == 0)
         {
            /* 'output' is linear, convert to the corresponding sRGB format. */
            output.opts = opts;
            result = write_one_file(&output, &copy, 1/*convert to 8bit*/);
            if (!result)
               break;

            /* This may involve a conversion via linear; in the ideal world this
             * would round-trip correctly, but libpng 1.5.7 is not the ideal
             * world so allow a drift (error_via_linear).
             *
             * 'image' has an alpha channel but 'output' does not then there
             * will a strip-alpha-channel operation (because 'output' is
             * linear), handle this by composing on black when doing the
             * comparison.
             */
            result = compare_two_images(image, &output, 1/*via_linear*/,
               background);
            if (!result)
               break;
         }
      }

      freeimage(&output);
   }

   freeimage(&copy);

   return result;
}

int
main(int argc, char **argv)
{
   png_uint_32 opts = 0;
   format_list formats;
   const char *touch = NULL;
   int log_pass = 0;
   int redundant = 0;
   int stride_extra = 0;
   int retval = 0;
   int c;

   format_init(&formats);

   for (c=1; c<argc; ++c)
   {
      const char *arg = argv[c];

      if (strcmp(arg, "--log") == 0)
         log_pass = 1;
      else if (strcmp(arg, "--file") == 0)
         opts |= READ_FILE;
      else if (strcmp(arg, "--memory") == 0)
         opts &= ~READ_FILE;
      else if (strcmp(arg, "--stdio") == 0)
         opts |= USE_STDIO;
      else if (strcmp(arg, "--name") == 0)
         opts &= ~USE_STDIO;
      else if (strcmp(arg, "--verbose") == 0)
         opts |= VERBOSE;
      else if (strcmp(arg, "--quiet") == 0)
         opts &= ~VERBOSE;
      else if (strcmp(arg, "--preserve") == 0)
         opts |= KEEP_TMPFILES;
      else if (strcmp(arg, "--nopreserve") == 0)
         opts &= ~KEEP_TMPFILES;
      else if (strcmp(arg, "--keep-going") == 0)
         opts |= KEEP_GOING;
      else if (strcmp(arg, "--redundant") == 0)
         redundant = 1;
      else if (strcmp(arg, "--stop") == 0)
         opts &= ~KEEP_GOING;
      else if (strcmp(arg, "--touch") == 0)
      {
         if (c+1 < argc)
            touch = argv[++c];

         else
         {
            fflush(stdout);
            fprintf(stderr, "%s: %s requires a file name argument\n",
               argv[0], arg);
            exit(1);
         }
      }
      else if (arg[0] == '+')
      {
         png_uint_32 format = formatof(arg+1);

         if (format > FORMAT_COUNT)
            exit(1);

         format_set(&formats, format);
      }
      else if (arg[0] == '-')
      {
         fflush(stdout);
         fprintf(stderr, "%s: unknown option: %s\n", argv[0], arg);
         exit(1);
      }
      else
      {
         int result;
         Image image;

         if (format_is_initial(&formats))
            format_default(&formats, redundant);

         newimage(&image);
         initimage(&image, opts, arg, stride_extra);
         result = read_one_file(&image);
         if (result)
            result = testimage(&image, opts, &formats);
         freeimage(&image);

         if (log_pass)
         {
            if (result)
               printf("PASS:");

            else
            {
               printf("FAIL:");
               retval = 1;
            }

            print_opts(opts);
            printf(" %s\n", arg);
         }

         else if (!result)
            exit(1);
      }
   }

   if (retval == 0 && touch != NULL)
   {
      FILE *fsuccess = fopen(touch, "wt");

      if (fsuccess != NULL)
      {
         int error = 0;
         fprintf(fsuccess, "PNG simple API tests succeeded\n");
         fflush(fsuccess);
         error = ferror(fsuccess);

         if (fclose(fsuccess) || error)
         {
            fflush(stdout);
            fprintf(stderr, "%s: write failed\n", touch);
            exit(1);
         }
      }

      else
      {
         fflush(stdout);
         fprintf(stderr, "%s: open failed\n", touch);
         exit(1);
      }
   }

   return retval;
}
