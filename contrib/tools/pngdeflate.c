/* pngdeflate.c
 *
 * Copyright (c) 2013 John Cunningham Bowler
 *
 * Last changed in libpng 1.6.3 [(PENDING RELEASE)]
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * Tool to check and fix the deflate 'too far back' problem, see the usage
 * message for more information.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Define the following to use this program against your installed libpng,
 * rather than the one being built here:
 */
#ifdef PNG_FREESTANDING_TESTS
#  include <png.h>
#else
#  include "../../png.h"
#endif

#if PNG_LIBPNG_VER < 10603 /* 1.6.3 */
#  error "pngdeflate will not work with libpng versions prior to 1.6.3"
#endif

#ifdef PNG_READ_SUPPORTED
#include <zlib.h>

#ifndef PNG_MAXIMUM_INFLATE_WINDOW
#  error "pngdeflate not supported in this libpng version"
#endif

#if PNG_ZLIB_VERNUM >= 0x1240

/* Copied from pngpriv.h */
#ifdef __cplusplus
#  define png_voidcast(type, value) static_cast<type>(value)
#  define png_constcast(type, value) const_cast<type>(value)
#  define png_aligncast(type, value) \
   static_cast<type>(static_cast<void*>(value))
#  define png_aligncastconst(type, value) \
   static_cast<type>(static_cast<const void*>(value))
#else
#  define png_voidcast(type, value) (value)
#  define png_constcast(type, value) ((type)(value))
#  define png_aligncast(type, value) ((void*)(value))
#  define png_aligncastconst(type, value) ((const void*)(value))
#endif /* __cplusplus */

static int idat_error = 0;
static int verbose = 0;
static int errors = 0;
static int warnings = 0;
#ifdef PNG_MAXIMUM_INFLATE_WINDOW
   static int set_option = 0;
#endif
static const char *name = "stdin";
static uLong crc_IDAT_head; /* CRC32 of "IDAT" */
static uLong crc_IEND;
static z_stream z_idat;

/* Control structure for the temporary file */
typedef struct
{
   size_t      image_size;
   off_t       file_size;
   fpos_t      header_pos;
   fpos_t      crc_pos;
   uLong       crc_tail;  /* CRC of bytes after header */
   png_uint_32 len_tail;  /* Count thereof */
   png_byte    header[2];

   /* Image info */
   png_uint_32 width;
   png_uint_32 height;
   png_byte    bit_depth;
   png_byte    color_type;
   png_byte    compression_method;
   png_byte    filter_method;
   png_byte    interlace_method;
} IDAT_info;

static png_uint_32
mult(png_uint_32 n, png_uint_32 m)
{
   if ((n + (m-1)) / m > 0xffffffff/m)
   {
      fprintf(stderr, "%s: overflow (%lu, %u)\n", name, (unsigned long)n, m);
      exit(2);
   }

   return n * m;
}

static size_t
image_size(const IDAT_info *info)
{
   unsigned int pd = info->bit_depth;
   size_t cb;

   switch (info->color_type)
   {
      case 0: case 3:
         break;

      case 2: /* rgb */
         pd *= 3;
         break;

      case 4: /* ga */
         pd *= 2;
         break;

      case 6: /* rgba */
         pd *= 4;
         break;

      default:
         fprintf(stderr, "%s: invalid color type (%d)\n", name,
            info->color_type);
         exit(2);
   }

   switch (info->interlace_method)
   {
      case PNG_INTERLACE_ADAM7:
         /* Interlacing makes the image larger because of the replication of
          * both the filter byte and the padding to a byte boundary.
          */
         {
            int pass;

            for (cb=0, pass=0; pass<=6; ++pass)
            {
               png_uint_32 pw = PNG_PASS_COLS(info->width, pass);

               if (pw > 0)
                  cb += mult(((mult(pd, pw)+7) >> 3)+1,
                        PNG_PASS_ROWS(info->height, pass));
            }
         }
         break;

      case PNG_INTERLACE_NONE:
         cb = mult(info->height, 1+((mult(info->width, pd) + 7) >> 3));
         break;

      default:
         fprintf(stderr, "%s: invalid interlace type %d\n", name,
            info->interlace_method);
         exit(2);
   }

   return cb;
}

static int
image_windowBits(const IDAT_info *info)
{
   size_t cb = image_size(info);

   if (cb > 16384) return 15;
   if (cb >  8192) return 14;
   if (cb >  4096) return 13;
   if (cb >  2048) return 12;
   if (cb >  1024) return 11;
   if (cb >   512) return 10;
   if (cb >   256) return  9;
   return 8;
}

static void
error_handler(png_structp png_ptr, png_const_charp message)
{
   if (strcmp(message, "IDAT: invalid distance too far back") == 0)
      idat_error = 1;

   else if (errors || verbose)
      fprintf(stderr, "%s: %s\n", name, message);

   png_longjmp(png_ptr, 1);
}

static void
warning_handler(png_structp png_ptr, png_const_charp message)
{
   if (warnings || verbose)
      fprintf(stderr, "%s: %s\n", name, message);

   (void)png_ptr;
}

static int
read_png(FILE *fp)
{
   png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,0,
      error_handler, warning_handler);
   png_infop info_ptr = NULL;
   png_bytep row = NULL, display = NULL;

   if (png_ptr == NULL)
      return 0;

   if (setjmp(png_jmpbuf(png_ptr)))
   {
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      if (row != NULL) free(row);
      if (display != NULL) free(display);
      return 0;
   }

#  ifdef PNG_MAXIMUM_INFLATE_WINDOW
      png_set_option(png_ptr, PNG_MAXIMUM_INFLATE_WINDOW, set_option != 0);
#  endif

   png_init_io(png_ptr, fp);

   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
      png_error(png_ptr, "OOM allocating info structure");

   png_set_keep_unknown_chunks(png_ptr, PNG_HANDLE_CHUNK_ALWAYS, NULL, 0);

   png_read_info(png_ptr, info_ptr);

   /* Limit the decompression buffer size to 1 - this ensures that overlong
    * length codes are always detected.
    */
   png_set_compression_buffer_size(png_ptr, 1);

   {
      png_size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);

      row = png_voidcast(png_byte*, malloc(rowbytes));
      display = png_voidcast(png_byte*, malloc(rowbytes));

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
             * work, but this avoids memory thrashing for speed testing.
             */
            while (y-- > 0)
               png_read_row(png_ptr, row, display);
         }
      }
   }

   /* Make sure to read to the end of the file: */
   png_read_end(png_ptr, info_ptr);
   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
   free(row);
   free(display);
   return 1;
}

/* Chunk tags (copied from pngpriv.h) */
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

static void
rx(FILE *fp, png_bytep buf, off_t cb)
{
   if (fread(buf,cb,1,fp) != 1) {
      fprintf(stderr, "%s: failed to read %lu bytes\n", name,
         (unsigned long)cb);
      exit(2);
   }
}

static png_uint_32
r32(FILE *fp)
{
   png_byte buf[4];
   rx(fp, buf, 4);
   return ((((((png_uint_32)buf[0] << 8)+buf[1]) << 8)+buf[2]) << 8) + buf[3];
}

static void
wx(FILE *fp, png_const_bytep buf, off_t cb)
{
   if (fwrite(buf,cb,1,fp) != 1) {
      fprintf(stderr, "%s: failed to write %lu bytes\n", name,
         (unsigned long)cb);
      exit(3);
   }
}

static void
w32(FILE *fp, png_uint_32 val)
{
   png_byte buf[4];
   buf[0] = (png_byte)(val >> 24);
   buf[1] = (png_byte)(val >> 16);
   buf[2] = (png_byte)(val >>  8);
   buf[3] = (png_byte)(val);
   wx(fp, buf, 4);
}

static void
wcrc(FILE *fp, uLong crc)
{
   /* Safe cast because a CRC is 32 bits */
   w32(fp, (png_uint_32)crc);
}

static void
copy(FILE *fp, FILE *fpIn, off_t cb)
{
   png_byte buffer[1024];

   while (cb >= 1024)
   {
      rx(fpIn, buffer, 1024);
      wx(fp, buffer, 1024);
      cb -= 1024;
   }

   if (cb > 0)
   {
      rx(fpIn, buffer, cb);
      wx(fp, buffer, cb);
   }
}

static void
skip_bytes(FILE *fpIn, png_uint_32 cb)
{
   png_byte buffer[1024];

   while (cb >= 1024)
   {
      rx(fpIn, buffer, 1024);
      cb -= 1024;
   }

   if (cb > 0)
      rx(fpIn, buffer, cb);
}

static void
safe_getpos(FILE *fp, fpos_t *pos)
{
   if (fgetpos(fp, pos))
   {
      perror("tmpfile");
      fprintf(stderr, "%s: tmpfile fgetpos failed\n", name);
      exit(3);
   }
}

static void
safe_setpos(FILE *fp, fpos_t *pos)
{
   if (fflush(fp))
   {
      perror("tmpfile");
      fprintf(stderr, "%s: tmpfile fflush failed\n", name);
      exit(3);
   }

   if (fsetpos(fp, pos))
   {
      perror("tmpfile");
      fprintf(stderr, "%s: tmpfile fsetpos failed\n", name);
      exit(3);
   }
}

static void
idat_update(FILE *fp, IDAT_info *info)
{
   uLong crc;

   safe_setpos(fp, &info->header_pos);
   wx(fp, info->header, 2);

   crc = crc32(crc_IDAT_head, info->header, 2);
   crc = crc32_combine(crc, info->crc_tail, info->len_tail);

   safe_setpos(fp, &info->crc_pos);
   wcrc(fp, crc);
}

static void
set_bits(const char *file, FILE *fp, IDAT_info *info, int bits)
{
   int byte1 = (info->header[0] & 0xf) + ((bits-8) << 4);
   int byte2 = info->header[1] & 0xe0;

   /* The checksum calculation: */
   byte2 += 0x1f - ((byte1 << 8) + byte2) % 0x1f;

   info->header[0] = (png_byte)byte1;
   info->header[1] = (png_byte)byte2;

   if (verbose)
      fprintf(stderr, "%s: trying windowBits %d (Z_CMF = 0x%x)\n", file, bits,
         byte1);

   idat_update(fp, info);
}

static void
ptagchar(png_uint_32 ch)
{
 ch &= 0xff;
 if (isprint(ch))
    putc(ch, stderr);

 else
    fprintf(stderr, "[%02x]", ch);
}

static void
ptag(png_uint_32 tag)
{
   if (tag != 0)
   {
      ptag(tag >> 8);
      ptagchar(tag);
   }
}

static int
fix_one(FILE *fp, FILE *fpIn, IDAT_info *info, png_uint_32 max_IDAT, int strip)
{
   int state = 0;
      /*   0: at beginning, before first IDAT
       *   1: read first CMF header byte
       *   2: read second byte, in first IDAT
       *   3: after first IDAT
       *  +4: saw deflate stream end.
       */
   int         truncated_idat = 0; /* Count of spurious IDAT bytes */
   uLong       crc_idat = 0;       /* Running CRC of current IDAT */
   png_uint_32 len_IDAT = 0;       /* Length of current IDAT */
   fpos_t      pos_IDAT_length;    /* fpos_t of length field in current IDAT */

   /* The signature: */
   {
      png_byte buf[8];
      rx(fpIn, buf, 8);
      wx(fp, buf, 8);
   }

   info->file_size = 45; /* signature + IHDR + IEND */

   for (;;) /* Chunk for loop */
   {
      png_uint_32 len = r32(fpIn);
      png_uint_32 tag = r32(fpIn);

      if (tag == png_IHDR)
      {
         /* Need width, height, color type, bit depth and interlace for the
          * file.
          */
         info->width = r32(fpIn);
         info->height = r32(fpIn);
         rx(fpIn, &info->bit_depth, 1);
         rx(fpIn, &info->color_type, 1);
         rx(fpIn, &info->compression_method, 1);
         rx(fpIn, &info->filter_method, 1);
         rx(fpIn, &info->interlace_method, 1);

         /* And write the information. */
         w32(fp, len);
         w32(fp, tag);
         w32(fp, info->width);
         w32(fp, info->height);
         wx(fp, &info->bit_depth, 1);
         wx(fp, &info->color_type, 1);
         wx(fp, &info->compression_method, 1);
         wx(fp, &info->filter_method, 1);
         wx(fp, &info->interlace_method, 1);

         /* Copy the CRC */
         copy(fp, fpIn, 4);
      }

      else if (tag == png_IEND)
      {
         /* Ok, write an IEND chunk and finish. */
         w32(fp, 0);
         w32(fp, png_IEND);
         wcrc(fp, crc_IEND);
         break;
      }

      else if (tag == png_IDAT && len > 0)
      {
         /* Write the chunk header now if it hasn't been written yet */
         if (len_IDAT == 0)
         {
            /* The length is set at the end: */
            safe_getpos(fp, &pos_IDAT_length);
            w32(fp, max_IDAT); /* length, not yet written */
            w32(fp, png_IDAT);

            if (state == 0) /* Start of first IDAT */
            {
               safe_getpos(fp, &info->header_pos);
               /* This will become info->crc_tail: */
               crc_idat = crc32(0L, Z_NULL, 0);
            }

            else
               crc_idat = crc_IDAT_head;
         }

         /* Do the zlib 2-byte header, it gets written out but not added
          * to the CRC (yet):
          */
         while (len > 0 && state < 2)
         {
            rx(fpIn, info->header + state, 1);
            wx(fp, info->header + state, 1);
            ++len_IDAT;
            --len;

            if (state++ == 1)
            {
               /* The zlib stream is used to validate the compressed IDAT
                * data in the most relaxed way possible.
                */
               png_byte bdummy;
               int ret;

               z_idat.next_in = info->header;
               z_idat.avail_in = 2;
               z_idat.next_out = &bdummy; /* Else Z_STREAM_ERROR! */
               z_idat.avail_out = 0;

               ret = inflate(&z_idat, Z_NO_FLUSH);
               if (ret != Z_OK || z_idat.avail_in != 0)
               {
                  fprintf(stderr,
                     "%s: unexpected/invalid inflate result %d \"%s\"\n",
                     name, ret, z_idat.msg);
                  return 1;
               }
            }
         } /* while in zlib header */

         /* Process further bytes in the IDAT chunk */
         while (len > 0 && state < 4)
         {
            png_byte b;

            rx(fpIn, &b, 1);
            --len;

            /* Do this 1 byte at a time to maximize the chance of
             * detecting errors (in particular zlib can skip the
             * 'too-far-back' error if the output buffer is bigger than
             * the window size.)
             */
            z_idat.next_in = &b;
            z_idat.avail_in = 1;

            do
            {
               int ret;
               png_byte bout;

               z_idat.next_out = &bout;
               z_idat.avail_out = 1;

               ret = inflate(&z_idat, Z_SYNC_FLUSH);

               if (z_idat.avail_out == 0)
                  ++info->image_size;

               switch (ret)
               {
                  case Z_OK:
                     /* Just keep going */
                     break;

                  case Z_BUF_ERROR:
                     if (z_idat.avail_in > 0)
                     {
                        fprintf(stderr,
                           "%s: unexpected buffer error \"%s\"\n",
                           name, z_idat.msg);
                        return 1;
                     }
                     goto end_loop;

                  case Z_STREAM_END:
                     /* End of stream */
                     state |= 4;
                     goto end_loop;

                  default:
                     fprintf(stderr, "%s: bad zlib stream %d, \"%s\"\n",
                        name, ret, z_idat.msg);
                     return 1;
               }
            } while (z_idat.avail_in > 0 || z_idat.avail_out == 0);

            /* The byte need not be consumed, if, for example, there is a
             * spurious byte after the end of the zlib data.
             */
         end_loop:
            if (z_idat.avail_in == 0)
            {
               /* Write it and update the length information and running
                * CRC.
                */
               wx(fp, &b, 1);
               crc_idat = crc32(crc_idat, &b, 1);
               ++len_IDAT;
            }

            else
               ++truncated_idat;

            if (len_IDAT >= max_IDAT || state >= 4)
            {
               /* Either the IDAT chunk is full or we've seen the end of
                * the deflate stream, or both.  Flush the chunk and handle
                * the details of the first chunk.
                */
               fpos_t save;

               if ((state & 3) < 3) /* First IDAT */
               {
                  safe_getpos(fp, &info->crc_pos);
                  info->crc_tail = crc_idat;
                  info->len_tail = len_IDAT-2;
               }

               /* This is not the correct value for the first IDAT! */
               wcrc(fp, crc_idat);
               state |= 3;

               /* Update the length if it is not max_IDAT: */
               if (len_IDAT != max_IDAT)
               {
                  safe_getpos(fp, &save);
                  safe_setpos(fp, &pos_IDAT_length);
                  w32(fp, len_IDAT);
                  safe_setpos(fp, &save);
               }

               /* Add this IDAT to the file size: */
               info->file_size += 12 + len_IDAT;
            }
         } /* while len > 0 && state < 4 */

         /* The above loop only exits on 0 bytes left or end of stream. If
          * the stream ended with bytes left, discard them:
          */
         if (len > 0)
         {
            truncated_idat += len;
            /* Skip those bytes and the CRC */
            skip_bytes(fpIn, len+4);
         }

         else
            skip_bytes(fpIn, 4); /* The CRC */
      } /* IDAT and len > 0 */

      else
      {
         int skip = 0;

         if (tag == png_IDAT)
            skip = 1;

         else if (state == 0)
         {
            /* Chunk before IDAT */
            if (!skip) switch (strip)
            {
               case 0: /* Don't strip */
                  break;

               case 1:  /* Keep gAMA, sRGB */
                  if (tag == png_gAMA || tag == png_sRGB)
                     break;
                  /* Fall through */

               default: /* Keep only IHDR, PLTE, tRNS */
                  if (tag == png_IHDR || tag == png_PLTE || tag == png_tRNS)
                     break;

                  skip = 1;
                  break;
            }
         }

         else if (state >= 4)
         {
            /* Keep nothing after IDAT if stripping: */
            skip = strip;
         }

         else
         {
            /* This is either an unterminated deflate stream or a spurious
             * non-IDAT chunk in the list of IDAT chunks.  Both are fatal
             * errors.
             */
            fprintf(stderr, "%s: tag '", name);
            ptag(tag);
            fprintf(stderr, "' after unterminated IDAT\n");
            break;
         }

         /* Skip or send? */
         if (skip)
         {
            if (tag != png_IDAT && (tag & 0x20000000) == 0)
            {
               fprintf(stderr, "%s: unknown critical chunk '", name);
               ptag(tag);
               fprintf(stderr, "'\n");
               return 1;
            }

            /* Skip this tag */
            if (fseek(fpIn, len+4, SEEK_CUR))
            {
               perror(name);
               fprintf(stderr, "%s: seek failed\n", name);
               return 1;
            }
         }

         else /* Keep this tag */
         {
            w32(fp, len);
            w32(fp, tag);
            copy(fp, fpIn, len+4);
            info->file_size += 12+len;
         }
      } /* Not IDAT or len == 0 */
   } /* Chunk for loop */

   /* Break out of the loop on error or end */
   if (state >= 4)
   {
      if (truncated_idat)
         fprintf(stderr, "%s: removed %d bytes from end of IDAT\n", name,
            truncated_idat);

      return 0; /* success */
   }

   /* This is somewhat generic but it works: */
   fprintf(stderr, "%s: unterminated/truncated PNG (%d)\n", name, state);

   return 1;
}

static FILE *fpIn;

static int
fix_file(FILE *fp, const char *file, png_uint_32 max_IDAT, int inplace,
   int strip, int optimize, const char *output)
{
   IDAT_info info;
   int imageBits, oldBits, bestBits, lowBits, newBits, ok_read;

   memset(&info, 0, sizeof info);

   name = file;
   idat_error = 0;

   /* fpIn is closed by the caller if necessary */
   fpIn = fopen(file, "rb");
   if (fpIn == NULL)
   {
      perror(file);
      fprintf(stderr, "%s: open failed\n", file);
      return 1;
   }

   /* With no arguments just check this file */
   if (optimize == 0 && strip == 0 && output == NULL)
      return !read_png(fpIn);

   /* Otherwise, maybe, fix it */
   if (fix_one(fp, fpIn, &info, max_IDAT, strip))
      return 1;

   /* oldBits may be invalid, imageBits is always OK, newBits always records the
    * actual window bits of the temporary file (fp).
    */
   bestBits = imageBits = image_windowBits(&info);
   newBits = oldBits = 8+(info.header[0] >> 4);
   ok_read = 0; /* records a successful read */

   /* Find the optimal (lowest) newBits */
   if (optimize)
      for (lowBits=8; lowBits < bestBits;)
   {
      /* This will always set 'newBits' to a value lower than 'bestBits' because
       * 'lowBits' is less than 'bestBits':
       */
      newBits = (bestBits + lowBits) >> 1;

      set_bits(file, fp, &info, newBits);

      rewind(fp);
      idat_error = 0;

      if (!read_png(fp))
      {
         /* If idat_error is *not* set this is some other problem */
         if (!idat_error)
            return 1;

         /* This is the hypothetical case where the IDAT has too much data *and*
          * the window size is wrong.  In fact this should never happen because
          * of the way libpng handles a deflate stream that produces extra data.
          */
         if (newBits >= imageBits)
         {
            fprintf(stderr, "%s: imageBits(%d) too low (%d)\n", file, imageBits,
               newBits);
            return 1;
         }

         if (lowBits <= newBits)
            lowBits = newBits+1;
      }

      else
      {
         bestBits = newBits;
         ok_read = 1;
      }
   }

   else if (bestBits > oldBits)
   {
      /* See if the original value is ok */
      rewind(fp);
      idat_error = 0;

      if (read_png(fp))
      {
         ok_read = 1;
         bestBits = oldBits;
      }

      else if (!idat_error)
         return 1;

      /* Otherwise there is an IDAT error and no optimization is being done, so
       * just use imageBits (which is already set in bestBits).
       */
   }

   if (newBits != bestBits)
   {
      /* Update the header to the required value */
      newBits = bestBits;

      set_bits(file, fp, &info, newBits);
   }

   if (!ok_read)
   {
      /* bestBits has not been checked */
      idat_error = 0;
      rewind(fp);
      ok_read = read_png(fp);

      if (idat_error)
      {
         /* This should never happen */
         fprintf(stderr, "%s: imageBits(%d) too low [%d]\n", file, imageBits,
            newBits);
         return 1;
      }

      /* This means that the PNG has some other error */
      if (!ok_read)
         return 1;
   }

   /* Have a valid newBits */
   if (optimize)
      printf("%2d %2d %2d %s %s %d %s\n", newBits, oldBits, imageBits,
         newBits < imageBits ? "<" : "=",
         newBits < oldBits ? "reduce  " :
            (newBits > oldBits ? "INCREASE" : "ok      "),
         newBits - oldBits, name);

#  ifdef PNG_MAXIMUM_INFLATE_WINDOW
      /* Because setting libpng to use the maximum window bits breaks the
       * read_png test above.
       */
      if (set_option)
         return 0;
#  endif

   if (output != NULL || (inplace && (bestBits != oldBits || strip)))
   {
      FILE *fpOut;
      
      if (output != NULL)
         fpOut = fopen(output, "wb");

      else
      {
         fpOut = freopen(file, "wb", fpIn);
         fpIn = NULL;
      }
      
      if (fpOut == NULL)
      {
         perror(output);
         fprintf(stderr, "%s: %s: open failed\n", file, output);
         exit(3);
      }

      rewind(fp);
      copy(fpOut, fp, info.file_size);

      if (fflush(fpOut) || ferror(fpOut) || fclose(fpOut))
      {
         perror(output != NULL ? output : file);
         fprintf(stderr, "%s: %s: close failed\n", file, output);
         if (output != NULL)
            remove(output);
         exit(3);
      }
   }

   return 0;
}

static void
usage(const char *prog, int rc)
{
   fprintf(stderr,
      "Usage: %s {[options] png-file}\n", prog);
   fprintf(stderr,
      "  Tests, optimizes and fixes the zlib header in PNG files.\n"
      "  Optionally, when fixing, strips ancilliary chunks from the file.\n");
   fprintf(stderr,
      "\nOptions:\n"
#  ifdef PNG_MAXIMUM_INFLATE_WINDOW
      "  --test: Test the PNG_MAXIMUM_INFLATE_WINDOW option.\n"
#  endif
      "  --optimize (-o): Find the smallest deflate window size for the file.\n"
      "                   Also outputs a summary for each file.\n"
      "  --strip (-s): Remove chunks except for IHDR, PLTE, IEND, tRNS, gAMA,\n"
      "                sRGB.  If given twice remove gAMA and sRGB as well.\n"
      "  --errors (-e): Output errors from libpng (except too-far-back).\n");
   fprintf(stderr,
      "  --warnings (-w): Output warnings from libpng.\n"
      "  --verbose (-v): Output more verbose messages.\n"
      "  --max=<number>: Output IDAT chunks sized <mumber>.  If not given the\n"
      "                  the IDAT chunks will be the maximum size permitted\n"
      "                  (2^31-1 bytes.)\n"
      "  --out=<file>: Save the result for the next PNG to <file>.\n"
      "  --inplace (-i): Modify the file in place.\n");
   fprintf(stderr,
      "\nExit codes:\n"
      "  0: Success, all files pass the test, all output written ok.\n"
      "  1: At least one file had a read error, all files checked.\n"
      "  2: A file had an unrecoverable error (integer overflow, bad format),\n"
      "     the program exited immediately, without processing further files.\n"
      "  3: An IO or out of memory error, or a file could not be opened.h\n");
   fprintf(stderr,
      "\nDescription:\n"
      "  %s checks each PNG file on the command line for errors.\n"
      "  By default it is silent and just exits with an error code (as above)\n"
      "  if any error is detected.  With --optimize, --strip or --out,\n"
      "  however, the zlib \"invalid distance too far back\" error is fixed\n"
      "  and the program exits with a 0 success code unless some other error\n"
      "  is encountered.\n"
      "\n", prog);
   fprintf(stderr,
      "  Use --errors to display the other errors, use --optimize to test\n"
      "  different values for the deflate \"window bits\" parameter and find\n"
      "  the smallest that works.\n"
      "\n"
      "  Notice that some PNG files with the zlib header problem can still be\n"
      "  read by libpng.  This program will still detect the error.\n"
      "\n");
   fprintf(stderr,
      "  The output produced with --optimize is as follows:\n"
      "\n"
      "     opt-bits curr-bits image-bits opt-flag opt-type change file\n"
      "\n"
      "   opt-bits:   The minimum window bits (8-15) that works, if the file\n"
      "               is written this is the value that will be stored.\n"
      "   curr-bits:  The value currently stored in the file.\n");
   fprintf(stderr,
      "   image-bits: The window bits value corresponding to the size of the\n"
      "               uncompressed PNG image data.  When --optimize is not\n"
      "               given but --strip is this value will be used if lower\n"
      "               than the current value.\n"
      "   opt-flag: < if the optimized bit value is less than that implied by\n"
      "               the PNG image size (opt-bits < image-bits)\n"
      "             = if optimization is not possible (opt-bits = image-bits)\n"
      "   opt-type: reduce   if opts-bits < curr-bits\n");
   fprintf(stderr,
      "             ok       if opt-bits = curr-bits (no change required)\n"
      "             INCREASE if opt-bits > curr-bits (the file has the bug)\n"
      "   change:     opt-bits - curr-bits, so negative if optimization is\n"
      "               possible, 0 if no change is required, positive if the\n"
      "               bug is present.\n"
      "   file:       The file name.\n");

   exit(rc);
}

int
main(int argc, const char **argv)
{
   int err, strip = 0, optimize = 0, inplace = 0, done = 0;
   png_uint_32 max_IDAT = 0x7fffffff;
   FILE *fp;
   const char *outfile = NULL;
   const char *prog = *argv;
   static const png_byte idat_bytes[4] = { 73,  68,  65,  84 };
   static const png_byte iend_bytes[4] = { 73,  69,  78,  68 };

   /* Initialize this first, could be stored as a constant: */
   crc_IEND = crc_IDAT_head = crc32(0L, Z_NULL, 0);
   crc_IDAT_head = crc32(crc_IDAT_head, idat_bytes, 4);
   crc_IEND = crc32(crc_IEND, iend_bytes, 4);

   z_idat.next_in = Z_NULL;
   z_idat.avail_in = 0;
   z_idat.zalloc = Z_NULL;
   z_idat.zfree = Z_NULL;
   z_idat.opaque = Z_NULL;

   err = inflateInit(&z_idat);
   if (err != Z_OK)
   {
      fprintf(stderr, "inflateInit failed %d \"%s\"\n", err, z_idat.msg);
      inflateEnd(&z_idat);
      return 3;
   }

   fp = tmpfile();
   if (fp == NULL)
   {
      perror("tmpfile");
      fprintf(stderr, "could not open a temporary file\n");
      return 3;
   }

   err = 0;
   while (--argc > 0)
   {
      ++argv;

      if (strcmp(*argv, "--inplace") == 0 || strcmp(*argv, "-i") == 0)
         ++inplace;

      else if (strncmp(*argv, "--max=", 6) == 0)
         max_IDAT = (png_uint_32)atol(6+*argv);

      else if (strcmp(*argv, "--optimize") == 0 || strcmp(*argv, "-o") == 0)
         ++optimize;

      else if (strncmp(*argv, "--out=", 6) == 0)
         outfile = 6+*argv;

      else if (strcmp(*argv, "--strip") == 0 || strcmp(*argv, "-s") == 0)
         ++strip;

      else if (strcmp(*argv, "--errors") == 0 || strcmp(*argv, "-e") == 0)
         ++errors;

      else if (strcmp(*argv, "--warnings") == 0 || strcmp(*argv, "-w") == 0)
         ++warnings;

      else if (strcmp(*argv, "--verbose") == 0 || strcmp(*argv, "-v") == 0)
         ++verbose;

#     ifdef PNG_MAXIMUM_INFLATE_WINDOW
         else if (strcmp(*argv, "--test") == 0)
            ++set_option;
#     endif

      else if ((*argv)[0] == '-')
         usage(prog, 3);

      else
      {
         int ret;

         err +=
            fix_file(fp, *argv, max_IDAT, inplace, strip, optimize, outfile);

         if (fpIn != NULL)
         {
            fclose(fpIn);
            fpIn = NULL;
         }

         z_idat.next_in = z_idat.next_out = Z_NULL;
         z_idat.avail_in = z_idat.avail_out = 0;
         ret = inflateReset(&z_idat);
         if (ret != Z_OK)
         {
            fprintf(stderr, "inflateReset failed %d \"%s\"\n", ret, z_idat.msg);
            inflateEnd(&z_idat);
            return 3;
         }

         rewind(fp);
         outfile = NULL;
         ++done;
      }
   }

   inflateEnd(&z_idat);

   if (!done)
      usage(prog, 0);

   return err != 0;
}

#else /* PNG_ZLIB_VERNUM < 0x1240 */
int
main(void)
{
   fprintf(stderr, "pngdeflate needs libpng with a zlib >=1.2.4 (not 0x%x)\n",
      PNG_ZLIB_VERNUM);
   return 77;
}
#endif /* PNG_ZLIB_VERNUM */

#else /* No read support */

int
main(void)
{
   fprintf(stderr, "pngdeflate does not work without read support\n");
   return 77;
}
#endif /* PNG_READ_SUPPORTED */
