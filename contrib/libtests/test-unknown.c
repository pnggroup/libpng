
/* test-unknown.c - test the read side unknown chunk handling
 *
 * Last changed in libpng 1.6.0 [(PENDING RELEASE)]
 * Copyright (c) 2012 Glenn Randers-Pehrson
 * Written by John Cunningham Bowler
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * NOTES:
 *   This is a C program that is intended to be linked against libpng.  It
 *   allows the libpng unknown handling code to be tested by interpreting
 *   arguemnts to save or discard combinations of chunks.  The program is
 *   currently just a minimal validation for the built-in libpng facilities.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <png.h>

#ifdef PNG_READ_SUPPORTED

#if PNG_LIBPNG_VER < 10500
/* This deliberately lacks the PNG_CONST. */
typedef png_byte *png_const_bytep;

/* This is copied from 1.5.1 png.h: */
#define PNG_INTERLACE_ADAM7_PASSES 7
#define PNG_PASS_START_ROW(pass) (((1U&~(pass))<<(3-((pass)>>1)))&7)
#define PNG_PASS_START_COL(pass) (((1U& (pass))<<(3-(((pass)+1)>>1)))&7)
#define PNG_PASS_ROW_SHIFT(pass) ((pass)>2?(8-(pass))>>1:3)
#define PNG_PASS_COL_SHIFT(pass) ((pass)>1?(7-(pass))>>1:3)
#define PNG_PASS_ROWS(height, pass) (((height)+(((1<<PNG_PASS_ROW_SHIFT(pass))\
   -1)-PNG_PASS_START_ROW(pass)))>>PNG_PASS_ROW_SHIFT(pass))
#define PNG_PASS_COLS(width, pass) (((width)+(((1<<PNG_PASS_COL_SHIFT(pass))\
   -1)-PNG_PASS_START_COL(pass)))>>PNG_PASS_COL_SHIFT(pass))
#define PNG_ROW_FROM_PASS_ROW(yIn, pass) \
   (((yIn)<<PNG_PASS_ROW_SHIFT(pass))+PNG_PASS_START_ROW(pass))
#define PNG_COL_FROM_PASS_COL(xIn, pass) \
   (((xIn)<<PNG_PASS_COL_SHIFT(pass))+PNG_PASS_START_COL(pass))
#define PNG_PASS_MASK(pass,off) ( \
   ((0x110145AFU>>(((7-(off))-(pass))<<2)) & 0xFU) | \
   ((0x01145AF0U>>(((7-(off))-(pass))<<2)) & 0xF0U))
#define PNG_ROW_IN_INTERLACE_PASS(y, pass) \
   ((PNG_PASS_MASK(pass,0) >> ((y)&7)) & 1)
#define PNG_COL_IN_INTERLACE_PASS(x, pass) \
   ((PNG_PASS_MASK(pass,1) >> ((x)&7)) & 1)

/* These are needed too for the default build: */
#define PNG_WRITE_16BIT_SUPPORTED
#define PNG_READ_16BIT_SUPPORTED

/* This comes from pnglibconf.h afer 1.5: */
#define PNG_FP_1 100000
#define PNG_GAMMA_THRESHOLD_FIXED\
   ((png_fixed_point)(PNG_GAMMA_THRESHOLD * PNG_FP_1))
#endif

#if PNG_LIBPNG_VER < 10600
   /* 1.6.0 constifies many APIs, the following exists to allow pngvalid to be
    * compiled against earlier versions.
    */
#  define png_const_structp png_structp
#endif


/* Copied from pngpriv.h */
#define PNG_32b(b,s) ((png_uint_32)(b) << (s))
#define PNG_CHUNK(b1,b2,b3,b4) \
   (PNG_32b(b1,24) | PNG_32b(b2,16) | PNG_32b(b3,8) | PNG_32b(b4,0))

#define png_IHDR PNG_CHUNK( 73,  72,  68,  82)
#define png_IDAT PNG_CHUNK( 73,  68,  65,  84)
#define png_IEND PNG_CHUNK( 73,  69,  78,  68)
#define png_PLTE PNG_CHUNK( 80,  76,  84,  69)
#define png_bKGD PNG_CHUNK( 98,  75,  71,  68)
#define png_cHRM PNG_CHUNK( 99,  72,  82,  77)
#define png_gAMA PNG_CHUNK(103,  65,  77,  65)
#define png_hIST PNG_CHUNK(104,  73,  83,  84)
#define png_iCCP PNG_CHUNK(105,  67,  67,  80)
#define png_iTXt PNG_CHUNK(105,  84,  88, 116)
#define png_oFFs PNG_CHUNK(111,  70,  70, 115)
#define png_pCAL PNG_CHUNK(112,  67,  65,  76)
#define png_sCAL PNG_CHUNK(115,  67,  65,  76)
#define png_pHYs PNG_CHUNK(112,  72,  89, 115)
#define png_sBIT PNG_CHUNK(115,  66,  73,  84)
#define png_sPLT PNG_CHUNK(115,  80,  76,  84)
#define png_sRGB PNG_CHUNK(115,  82,  71,  66)
#define png_sTER PNG_CHUNK(115,  84,  69,  82)
#define png_tEXt PNG_CHUNK(116,  69,  88, 116)
#define png_tIME PNG_CHUNK(116,  73,  77,  69)
#define png_tRNS PNG_CHUNK(116,  82,  78,  83)
#define png_zTXt PNG_CHUNK(122,  84,  88, 116)
#define png_vpAg PNG_CHUNK('v', 'p', 'A', 'g')

/* Test on flag values as defined in the spec (section 5.4): */
#define PNG_CHUNK_ANCILLARY(c )   (1 & ((c) >> 29))
#define PNG_CHUNK_CRITICAL(c)     (!PNG_CHUNK_ANCILLARY(c))
#define PNG_CHUNK_PRIVATE(c)      (1 & ((c) >> 21))
#define PNG_CHUNK_RESERVED(c)     (1 & ((c) >> 13))
#define PNG_CHUNK_SAFE_TO_COPY(c) (1 & ((c) >>  5))

/* Chunk information */
#define PNG_INFO_tEXt 0x10000000U
#define PNG_INFO_iTXt 0x20000000U
#define PNG_INFO_zTXt 0x40000000U

#define PNG_INFO_sTER 0x01000000U
#define PNG_INFO_vpAg 0x02000000U

#define ABSENT  0
#define START   1
#define END     2

static struct
{
   char        name[5];
   png_uint_32 flag;
   png_uint_32 tag;
   int         unknown;    /* Chunk not known to libpng */
   int         all;        /* Chunk set by the '-1' option */
   int         position;   /* position in pngtest.png */
   int         keep;       /* unknown handling setting */
} chunk_info[] = {
   /* Critical chunks */
   { "IDAT", PNG_INFO_IDAT, png_IDAT, 0, 0,  START, 0 }, /* must be [0] */
   { "PLTE", PNG_INFO_PLTE, png_PLTE, 0, 0, ABSENT, 0 },

   /* Non-critical chunks that libpng handles */
   { "bKGD", PNG_INFO_bKGD, png_bKGD, 0, 1,  START, 0 },
   { "cHRM", PNG_INFO_cHRM, png_cHRM, 0, 1,  START, 0 },
   { "gAMA", PNG_INFO_gAMA, png_gAMA, 0, 1,  START, 0 },
   { "hIST", PNG_INFO_hIST, png_hIST, 0, 1, ABSENT, 0 },
   { "iCCP", PNG_INFO_iCCP, png_iCCP, 0, 1, ABSENT, 0 },
   { "iTXt", PNG_INFO_iTXt, png_iTXt, 0, 1, ABSENT, 0 },
   { "oFFs", PNG_INFO_oFFs, png_oFFs, 0, 1,  START, 0 },
   { "pCAL", PNG_INFO_pCAL, png_pCAL, 0, 1,  START, 0 },
   { "pHYs", PNG_INFO_pHYs, png_pHYs, 0, 1,  START, 0 },
   { "sBIT", PNG_INFO_sBIT, png_sBIT, 0, 1,  START, 0 },
   { "sCAL", PNG_INFO_sCAL, png_sCAL, 0, 1,  START, 0 },
   { "sPLT", PNG_INFO_sPLT, png_sPLT, 0, 1, ABSENT, 0 },
   { "sRGB", PNG_INFO_sRGB, png_sRGB, 0, 1,  START, 0 },
   { "tEXt", PNG_INFO_tEXt, png_tEXt, 0, 1,  START, 0 },
   { "tIME", PNG_INFO_tIME, png_tIME, 0, 1,  START, 0 },
   { "tRNS", PNG_INFO_tRNS, png_tRNS, 0, 0, ABSENT, 0 },
   { "zTXt", PNG_INFO_zTXt, png_zTXt, 0, 1,    END, 0 },

   /* No libpng handling */
   { "sTER", PNG_INFO_sTER, png_sTER, 1, 1,  START, 0 },
   { "vpAg", PNG_INFO_vpAg, png_vpAg, 1, 0,  START, 0 },
};

#define NINFO ((int)((sizeof chunk_info)/(sizeof chunk_info[0])))

static int
find(const char *name)
{
   int i = NINFO;
   while (--i >= 0)
   {
      if (memcmp(chunk_info[i].name, name, 4) == 0)
         break;
   }

   return i;
}

static int
findb(const png_byte *name)
{
   int i = NINFO;
   while (--i >= 0)
   {
      if (memcmp(chunk_info[i].name, name, 4) == 0)
         break;
   }

   return i;
}

static int
find_by_flag(png_uint_32 flag)
{
   int i = NINFO;

   while (--i >= 0) if (chunk_info[i].flag == flag) return i;

   fprintf(stderr, "test-unknown: internal error\n");
   exit(4);
}

static int
ancillary(const char *name)
{
   return PNG_CHUNK_ANCILLARY(PNG_CHUNK(name[0], name[1], name[2], name[3]));
}

static int
ancillaryb(const png_byte *name)
{
   return PNG_CHUNK_ANCILLARY(PNG_CHUNK(name[0], name[1], name[2], name[3]));
}

static int error_count = 0;
static int warning_count = 0;

static void
error(png_structp png_ptr, const char *message)
{
   fprintf(stderr, "libpng error: %s\n", message);
   exit(1);
   (void)png_ptr;
}

static void
warning(png_structp png_ptr, const char *message)
{
   ++warning_count;
   fprintf(stderr, "libpng warning: %s\n", message);
   (void)png_ptr;
}

static png_uint_32
get_valid(const char *file, png_const_structp png_ptr, png_const_infop info_ptr)
{
   png_uint_32 flags = png_get_valid(png_ptr, info_ptr, (png_uint_32)~0);

   /* Map the text chunks back into the flags */
   {
      png_textp text;
      png_uint_32 ntext = png_get_text(png_ptr, info_ptr, &text, NULL);

      while (ntext-- > 0) switch (text[ntext].compression)
      {
         case -1:
            flags |= PNG_INFO_tEXt;
            break;
         case 0:
            flags |= PNG_INFO_zTXt;
            break;
         case 1:
         case 2:
            flags |= PNG_INFO_iTXt;
            break;
         default:
            fprintf(stderr, "%s: unknown text compression %d\n", file,
               text[ntext].compression);
            exit(1);
      }
   }

   return flags;
}

static png_uint_32
get_unknown(const char *file, int def, png_const_structp png_ptr,
   png_const_infop info_ptr)
{
   /* Create corresponding 'unknown' flags */
   png_uint_32 flags = 0;
   {
      png_unknown_chunkp unknown;
      int num_unknown = png_get_unknown_chunks(png_ptr, info_ptr, &unknown);

      while (--num_unknown >= 0)
      {
         int chunk = findb(unknown[num_unknown].name);

         /* Chunks not know to test-unknown must be validated here; since they
          * must also be unknown to libpng the 'def' behavior should have been
          * used.
          */
         if (chunk < 0) switch (def)
         {
            default: /* impossible */
            case PNG_HANDLE_CHUNK_AS_DEFAULT:
            case PNG_HANDLE_CHUNK_NEVER:
               fprintf(stderr, "%s: %s: %s: unknown chunk saved\n",
                  file, def ? "discard" : "default", unknown[num_unknown].name);
               ++error_count;
               break;

            case PNG_HANDLE_CHUNK_IF_SAFE:
               if (!ancillaryb(unknown[num_unknown].name))
               {
                  fprintf(stderr,
                     "%s: if-safe: %s: unknown critical chunk saved\n",
                     file, unknown[num_unknown].name);
                  ++error_count;
                  break;
               }
               /* FALL THROUGH (safe) */
            case PNG_HANDLE_CHUNK_ALWAYS:
               break;
         }

         else
            flags |= chunk_info[chunk].flag;
      }
   }

   return flags;
}

static int
check(FILE *fp, int argc, const char **argv, png_uint_32p flags/*out*/)
{
   png_structp png_ptr;
   png_infop info_ptr, end_ptr;
   int i, def = PNG_HANDLE_CHUNK_AS_DEFAULT, npasses, ipass;
   png_uint_32 height;

   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, error, warning);
   if (png_ptr == NULL)
   {
      fprintf(stderr, "%s: could not allocate png struct\n", argv[argc]);
      exit(1);
   }

   info_ptr = png_create_info_struct(png_ptr);
   end_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL || end_ptr == NULL)
   {
      fprintf(stderr, "%s: could not allocate png info\n", argv[argc]);
      exit(1);
   }

   png_init_io(png_ptr, fp);

   /* Handle each argument in turn; multiple settings are possible for the same
    * chunk and multiple calls will occur (the last one should override all
    * preceding ones).
    */
   for (i=1; i<argc; ++i)
   {
      const char *equals = strchr(argv[i], '=');

      if (equals != NULL)
      {
         int chunk, option;

         if (strcmp(equals+1, "default") == 0)
            option = PNG_HANDLE_CHUNK_AS_DEFAULT;
         else if (strcmp(equals+1, "discard") == 0)
            option = PNG_HANDLE_CHUNK_NEVER;
         else if (strcmp(equals+1, "if-safe") == 0)
            option = PNG_HANDLE_CHUNK_IF_SAFE;
         else if (strcmp(equals+1, "save") == 0)
            option = PNG_HANDLE_CHUNK_ALWAYS;
         else
         {
            fprintf(stderr, "%s: unrecognized chunk option\n", argv[i]);
            exit(2);
         }

         switch (equals - argv[i])
         {
            case 4: /* chunk name */
               chunk = find(argv[i]);

               if (chunk >= 0)
               {
                  /* These #if tests have the effect of skipping the arguments
                   * if SAVE support is unavailable - we can't do a useful test
                   * in this case, so we just check the arguments!  This could
                   * be improved in the future by using the read callback.
                   */
#                 ifdef PNG_SAVE_UNKNOWN_CHUNKS_SUPPORTED
                     png_byte name[5];

                     memcpy(name, chunk_info[chunk].name, 5);
                     png_set_keep_unknown_chunks(png_ptr, option, name, 1);
                     chunk_info[chunk].keep = option;
#                 else
                     (void)option;
#                 endif
                  continue;
               }

               break;

            case 7: /* default */
               if (memcmp(argv[i], "default", 7) == 0)
               {
#                 ifdef PNG_SAVE_UNKNOWN_CHUNKS_SUPPORTED
                     png_set_keep_unknown_chunks(png_ptr, option, NULL, 0);
                     def = option;
#                 endif
                  continue;
               }

               break;

            case 3: /* all */
               if (memcmp(argv[i], "all", 3) == 0)
               {
#                 ifdef PNG_SAVE_UNKNOWN_CHUNKS_SUPPORTED
                     png_set_keep_unknown_chunks(png_ptr, option, NULL, -1);
                     def = option;

                     for (chunk = 0; chunk < NINFO; ++chunk)
                        if (chunk_info[chunk].all)
                           chunk_info[chunk].keep = option;
#                 endif
                  continue;
               }

               break;

            default: /* some misplaced = */

               break;
         }
      }

      fprintf(stderr, "%s: unrecognized chunk argument\n", argv[i]);
      exit(2);
   }

   png_read_info(png_ptr, info_ptr);

   switch (png_get_interlace_type(png_ptr, info_ptr))
   {
      case PNG_INTERLACE_NONE:
         npasses = 1;
         break;

      case PNG_INTERLACE_ADAM7:
         npasses = PNG_INTERLACE_ADAM7_PASSES;
         break;

      default:
         fprintf(stderr, "%s: invalid interlace type\n", argv[argc]);
         exit(1);
   }

   /* Skip the image data, if IDAT is not being handled then don't do this
    * because it will cause a CRC error.
    */
   if (chunk_info[0/*IDAT*/].keep == PNG_HANDLE_CHUNK_AS_DEFAULT)
   {
      png_start_read_image(png_ptr);
      height = png_get_image_height(png_ptr, info_ptr);

      if (npasses > 1)
      {
         png_uint_32 width = png_get_image_width(png_ptr, info_ptr);

         for (ipass=0; ipass<npasses; ++ipass)
         {
            png_uint_32 wPass = PNG_PASS_COLS(width, ipass);

            if (wPass > 0)
            {
               png_uint_32 y;

               for (y=0; y<height; ++y) if (PNG_ROW_IN_INTERLACE_PASS(y, ipass))
                  png_read_row(png_ptr, NULL, NULL);
            }
         }
      } /* interlaced */

      else /* not interlaced */
      {
         png_uint_32 y;

         for (y=0; y<height; ++y)
            png_read_row(png_ptr, NULL, NULL);
      }
   }

   png_read_end(png_ptr, end_ptr);

   flags[0] = get_valid(argv[argc], png_ptr, info_ptr);
   flags[1] = get_unknown(argv[argc], def, png_ptr, info_ptr);

   /* Only png_read_png sets PNG_INFO_IDAT! */
   flags[chunk_info[0/*IDAT*/].keep != PNG_HANDLE_CHUNK_AS_DEFAULT] |=
      PNG_INFO_IDAT;

   flags[2] = get_valid(argv[argc], png_ptr, end_ptr);
   flags[3] = get_unknown(argv[argc], def, png_ptr, end_ptr);

   png_destroy_read_struct(&png_ptr, &info_ptr, &end_ptr);

   return def;
}

static void
check_error(const char *file, png_uint_32 flags, const char *message)
{
   while (flags)
   {
      png_uint_32 flag = flags & -flags;
      int i = find_by_flag(flag);

      fprintf(stderr, "%s: chunk %s: %s\n", file, chunk_info[i].name, message);
      ++error_count;

      flags &= ~flag;
   }
}

static void
check_handling(const char *file, int def, png_uint_32 chunks, png_uint_32 known,
   png_uint_32 unknown, const char *position)
{
   while (chunks)
   {
      png_uint_32 flag = chunks & -chunks;
      int i = find_by_flag(flag);
      int keep = chunk_info[i].keep;
      const char *type;
      const char *errorx = NULL;

      if (chunk_info[i].unknown)
      {
         if (keep == PNG_HANDLE_CHUNK_AS_DEFAULT)
         {
            type = "UNKNOWN (default)";
            keep = def;
         }

         else
            type = "UNKNOWN (specified)";

         if (flag & known)
            errorx = "chunk processed";

         else switch (keep)
         {
            case PNG_HANDLE_CHUNK_AS_DEFAULT:
               if (flag & unknown)
                  errorx = "DEFAULT: unknown chunk saved";
               break;

            case PNG_HANDLE_CHUNK_NEVER:
               if (flag & unknown)
                  errorx = "DISCARD: unknown chunk saved";
               break;

            case PNG_HANDLE_CHUNK_IF_SAFE:
               if (ancillary(chunk_info[i].name))
               {
                  if (!(flag & unknown))
                     errorx = "IF-SAFE: unknown ancillary chunk lost";
               }

               else if (flag & unknown)
                  errorx = "IF-SAFE: unknown critical chunk saved";
               break;

            case PNG_HANDLE_CHUNK_ALWAYS:
               if (!(flag & unknown))
                  errorx = "SAVE: unknown chunk lost";
               break;

            default:
               errorx = "internal error: bad keep";
               break;
         }
      } /* unknown chunk */

      else /* known chunk */
      {
         type = "KNOWN";

         if (flag & known)
         {
            /* chunk was processed, it won't have been saved because that is
             * caught below when checking for inconsistent processing.
             */
            if (keep != PNG_HANDLE_CHUNK_AS_DEFAULT)
               errorx = "!DEFAULT: known chunk processed";
         }

         else /* not processed */ switch (keep)
         {
            case PNG_HANDLE_CHUNK_AS_DEFAULT:
               errorx = "DEFAULT: known chunk not processed";
               break;

            case PNG_HANDLE_CHUNK_NEVER:
               if (flag & unknown)
                  errorx = "DISCARD: known chunk saved";
               break;

            case PNG_HANDLE_CHUNK_IF_SAFE:
               if (ancillary(chunk_info[i].name))
               {
                  if (!(flag & unknown))
                     errorx = "IF-SAFE: known ancillary chunk lost";
               }

               else if (flag & unknown)
                  errorx = "IF-SAFE: known critical chunk saved";
               break;

            case PNG_HANDLE_CHUNK_ALWAYS:
               if (!(flag & unknown))
                  errorx = "SAVE: known chunk lost";
               break;

            default:
               errorx = "internal error: bad keep (2)";
               break;
         }
      }

      if (errorx != NULL)
      {
         ++error_count;
         fprintf(stderr, "%s: %s %s %s: %s\n",
            file, type, chunk_info[i].name, position, errorx);
      }

      chunks &= ~flag;
   }
}

int
main(int argc, const char **argv)
{
   FILE *fp;
   png_uint_32 flags[2/*default/+options*/][4/*valid,unknown{before,after}*/];
   int def, strict = 0;
   const char *program = argv[0];
   const char *count_argv[3];

   if (argc > 1 && strcmp(argv[1], "--strict") == 0)
   {
      strict = 1;
      --argc;
      ++argv;
   }

   if (--argc < 1)
   {
      fprintf(stderr, "test-unknown: usage:\n"
      " %s {(CHNK|default|all)=(default|discard|if-safe|save)} testfile.png\n",
         program);
      exit(2);
   }

#  ifndef PNG_SAVE_UNKNOWN_CHUNKS_SUPPORTED
      fprintf(stderr,
         "test-unknown: warning: no 'save' support so arguments ignored\n");
#  endif
   fp = fopen(argv[argc], "rb");
   if (fp == NULL)
   {
      perror(argv[argc]);
      exit(2);
   }

   /* First find all the chunks, known and unknown, in the test file: */
   count_argv[0] = program;
   count_argv[1] = "default=save";
   count_argv[2] = argv[argc];
   (void)check(fp, 2, count_argv, flags[0]);

   /* Now find what the various supplied options cause to change: */
   rewind(fp);
   def = check(fp, argc, argv, flags[1]);
   fclose(fp);

   /* Chunks should either be known or unknown, never both and this should apply
    * whether the chunk is before or after the IDAT (actually, the app can
    * probably change this by swapping the handling after the image, but this
    * test does not do that.)
    */
   check_error(argc[argv],
      (flags[0][0]|flags[0][2]) & (flags[0][1]|flags[0][3]),
      "chunk handled inconsistently in count tests");
   check_error(argc[argv],
      (flags[1][0]|flags[1][2]) & (flags[1][1]|flags[1][3]),
      "chunk handled inconsistently in option tests");

   /* Now find out what happened to each chunk before and after the IDAT and
    * determine if the behavior was correct.  First some basic sanity checks,
    * any known chunk should be known in the original count, any unknown chunk
    * should be either known or unknown in the original.
    */
   {
      png_uint_32 test;

      test = flags[1][0] & ~flags[0][0];
      check_error(argv[argc], test, "new known chunk before IDAT");
      test = flags[1][1] & ~(flags[0][0] | flags[0][1]);
      check_error(argv[argc], test, "new unknown chunk before IDAT");
      test = flags[1][2] & ~flags[0][2];
      check_error(argv[argc], test, "new known chunk after IDAT");
      test = flags[1][3] & ~(flags[0][2] | flags[0][3]);
      check_error(argv[argc], test, "new unknown chunk after IDAT");
   }

   /* Now each chunk in the original list should have been handled according to
    * the options set for that chunk, regardless of whether libpng knows about
    * it or not.
    */
   check_handling(argv[argc], def, flags[0][0] | flags[0][1], flags[1][0],
      flags[1][1], "before IDAT");
   check_handling(argv[argc], def, flags[0][2] | flags[0][3], flags[1][2],
      flags[1][3], "after IDAT");

   return error_count + (strict ? warning_count : 0);
}

#else
int
main(void)
{
   fprintf(stderr,
   " test ignored because libpng was not built with unknown chunk support\n");
   return 0;
}
#endif
