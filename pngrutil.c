
/* pngrutil.c - utilities to read a PNG file
 *
 * Last changed in libpng 1.6.0 [(PENDING RELEASE)]
 * Copyright (c) 1998-2012 Glenn Randers-Pehrson
 * (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
 * (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * This file contains routines that are only called from within
 * libpng itself during the course of reading an image.
 */

#include "pngpriv.h"

#ifdef PNG_READ_SUPPORTED

#define png_strtod(p,a,b) strtod(a,b)

png_uint_32 PNGAPI
png_get_uint_31(png_const_structrp png_ptr, png_const_bytep buf)
{
   png_uint_32 uval = png_get_uint_32(buf);

   if (uval > PNG_UINT_31_MAX)
      png_error(png_ptr, "PNG unsigned integer out of range");

   return (uval);
}

#if defined(PNG_READ_gAMA_SUPPORTED) || defined(PNG_READ_cHRM_SUPPORTED)
/* The following is a variation on the above for use with the fixed
 * point values used for gAMA and cHRM.  Instead of png_error it
 * issues a warning and returns (-1) - an invalid value because both
 * gAMA and cHRM use *unsigned* integers for fixed point values.
 */
#define PNG_FIXED_ERROR (-1)

static png_fixed_point /* PRIVATE */
png_get_fixed_point(png_structrp png_ptr, png_const_bytep buf)
{
   png_uint_32 uval = png_get_uint_32(buf);

   if (uval <= PNG_UINT_31_MAX)
      return (png_fixed_point)uval; /* known to be in range */

   /* The caller can turn off the warning by passing NULL. */
   if (png_ptr != NULL)
      png_warning(png_ptr, "PNG fixed point integer out of range");

   return PNG_FIXED_ERROR;
}
#endif

#ifdef PNG_READ_INT_FUNCTIONS_SUPPORTED
/* NOTE: the read macros will obscure these definitions, so that if
 * PNG_USE_READ_MACROS is set the library will not use them internally,
 * but the APIs will still be available externally.
 *
 * The parentheses around "PNGAPI function_name" in the following three
 * functions are necessary because they allow the macros to co-exist with
 * these (unused but exported) functions.
 */

/* Grab an unsigned 32-bit integer from a buffer in big-endian format. */
png_uint_32 (PNGAPI
png_get_uint_32)(png_const_bytep buf)
{
   png_uint_32 uval =
       ((png_uint_32)(*(buf    )) << 24) +
       ((png_uint_32)(*(buf + 1)) << 16) +
       ((png_uint_32)(*(buf + 2)) <<  8) +
       ((png_uint_32)(*(buf + 3))      ) ;

   return uval;
}

/* Grab a signed 32-bit integer from a buffer in big-endian format.  The
 * data is stored in the PNG file in two's complement format and there
 * is no guarantee that a 'png_int_32' is exactly 32 bits, therefore
 * the following code does a two's complement to native conversion.
 */
png_int_32 (PNGAPI
png_get_int_32)(png_const_bytep buf)
{
   png_uint_32 uval = png_get_uint_32(buf);
   if ((uval & 0x80000000) == 0) /* non-negative */
      return uval;

   uval = (uval ^ 0xffffffff) + 1;  /* 2's complement: -x = ~x+1 */
   return -(png_int_32)uval;
}

/* Grab an unsigned 16-bit integer from a buffer in big-endian format. */
png_uint_16 (PNGAPI
png_get_uint_16)(png_const_bytep buf)
{
   /* ANSI-C requires an int value to accomodate at least 16 bits so this
    * works and allows the compiler not to worry about possible narrowing
    * on 32 bit systems.  (Pre-ANSI systems did not make integers smaller
    * than 16 bits either.)
    */
   unsigned int val =
       ((unsigned int)(*buf) << 8) +
       ((unsigned int)(*(buf + 1)));

   return (png_uint_16)val;
}

#endif /* PNG_READ_INT_FUNCTIONS_SUPPORTED */

/* Read and check the PNG file signature */
void /* PRIVATE */
png_read_sig(png_structrp png_ptr, png_inforp info_ptr)
{
   png_size_t num_checked, num_to_check;

   /* Exit if the user application does not expect a signature. */
   if (png_ptr->sig_bytes >= 8)
      return;

   num_checked = png_ptr->sig_bytes;
   num_to_check = 8 - num_checked;

#ifdef PNG_IO_STATE_SUPPORTED
   png_ptr->io_state = PNG_IO_READING | PNG_IO_SIGNATURE;
#endif

   /* The signature must be serialized in a single I/O call. */
   png_read_data(png_ptr, &(info_ptr->signature[num_checked]), num_to_check);
   png_ptr->sig_bytes = 8;

   if (png_sig_cmp(info_ptr->signature, num_checked, num_to_check))
   {
      if (num_checked < 4 &&
          png_sig_cmp(info_ptr->signature, num_checked, num_to_check - 4))
         png_error(png_ptr, "Not a PNG file");
      else
         png_error(png_ptr, "PNG file corrupted by ASCII conversion");
   }
   if (num_checked < 3)
      png_ptr->mode |= PNG_HAVE_PNG_SIGNATURE;
}

/* Read the chunk header (length + type name).
 * Put the type name into png_ptr->chunk_name, and return the length.
 */
png_uint_32 /* PRIVATE */
png_read_chunk_header(png_structrp png_ptr)
{
   png_byte buf[8];
   png_uint_32 length;

#ifdef PNG_IO_STATE_SUPPORTED
   png_ptr->io_state = PNG_IO_READING | PNG_IO_CHUNK_HDR;
#endif

   /* Read the length and the chunk name.
    * This must be performed in a single I/O call.
    */
   png_read_data(png_ptr, buf, 8);
   length = png_get_uint_31(png_ptr, buf);

   /* Put the chunk name into png_ptr->chunk_name. */
   png_ptr->chunk_name = PNG_CHUNK_FROM_STRING(buf+4);

   png_debug2(0, "Reading %lx chunk, length = %lu",
       (unsigned long)png_ptr->chunk_name, (unsigned long)length);

   /* Reset the crc and run it over the chunk name. */
   png_reset_crc(png_ptr);
   png_calculate_crc(png_ptr, buf + 4, 4);

   /* Check to see if chunk name is valid. */
   png_check_chunk_name(png_ptr, png_ptr->chunk_name);

#ifdef PNG_IO_STATE_SUPPORTED
   png_ptr->io_state = PNG_IO_READING | PNG_IO_CHUNK_DATA;
#endif

   return length;
}

/* Read data, and (optionally) run it through the CRC. */
void /* PRIVATE */
png_crc_read(png_structrp png_ptr, png_bytep buf, png_uint_32 length)
{
   if (png_ptr == NULL)
      return;

   png_read_data(png_ptr, buf, length);
   png_calculate_crc(png_ptr, buf, length);
}

/* Optionally skip data and then check the CRC.  Depending on whether we
 * are reading a ancillary or critical chunk, and how the program has set
 * things up, we may calculate the CRC on the data and print a message.
 * Returns '1' if there was a CRC error, '0' otherwise.
 */
int /* PRIVATE */
png_crc_finish(png_structrp png_ptr, png_uint_32 skip)
{
   png_uint_32 i;
   uInt istop = png_ptr->zbuf_size;

   for (i = skip; i > istop; i -= istop)
   {
      png_crc_read(png_ptr, png_ptr->zbuf, png_ptr->zbuf_size);
   }

   if (i)
   {
      png_crc_read(png_ptr, png_ptr->zbuf, i);
   }

   if (png_crc_error(png_ptr))
   {
      if (PNG_CHUNK_ANCILLIARY(png_ptr->chunk_name) ?
          !(png_ptr->flags & PNG_FLAG_CRC_ANCILLARY_NOWARN) :
          (png_ptr->flags & PNG_FLAG_CRC_CRITICAL_USE))
      {
         png_chunk_warning(png_ptr, "CRC error");
      }

      else
      {
         png_chunk_benign_error(png_ptr, "CRC error");
         return (0);
      }

      return (1);
   }

   return (0);
}

/* Compare the CRC stored in the PNG file with that calculated by libpng from
 * the data it has read thus far.
 */
int /* PRIVATE */
png_crc_error(png_structrp png_ptr)
{
   png_byte crc_bytes[4];
   png_uint_32 crc;
   int need_crc = 1;

   if (PNG_CHUNK_ANCILLIARY(png_ptr->chunk_name))
   {
      if ((png_ptr->flags & PNG_FLAG_CRC_ANCILLARY_MASK) ==
          (PNG_FLAG_CRC_ANCILLARY_USE | PNG_FLAG_CRC_ANCILLARY_NOWARN))
         need_crc = 0;
   }

   else /* critical */
   {
      if (png_ptr->flags & PNG_FLAG_CRC_CRITICAL_IGNORE)
         need_crc = 0;
   }

#ifdef PNG_IO_STATE_SUPPORTED
   png_ptr->io_state = PNG_IO_READING | PNG_IO_CHUNK_CRC;
#endif

   /* The chunk CRC must be serialized in a single I/O call. */
   png_read_data(png_ptr, crc_bytes, 4);

   if (need_crc)
   {
      crc = png_get_uint_32(crc_bytes);
      return ((int)(crc != png_ptr->crc));
   }

   else
      return (0);
}

/* png_inflate_claim: claim the zstream for some nefarious purpose that involves
 * decompression.
 */
void /* PRIVATE */
png_inflate_claim(png_structrp png_ptr)
{
   /* Implementation note: unlike 'png_deflate_claim' this internal function
    * does not take the size of the data as an argument.  Some efficiency could
    * be gained by using this when it is known *if* the zlib stream itself does
    * not record the number, however this is a chimera: the original writer of
    * the PNG may have selected a lower window size, and we really must follow
    * that because, for systems with with limited capabilities, we would
    * otherwise reject the applications attempts to use a smaller window size.
    * (zlib doesn't have an interface to say "this or lower"!)
    */
   if (!(png_ptr->flags & PNG_FLAG_ZSTREAM_IN_USE))
   {
      int ret; /* zlib return code */

      /* Do this for safety now. */
      png_ptr->flags &= ~PNG_FLAG_ZSTREAM_ENDED;

      if (png_ptr->flags & PNG_FLAG_ZSTREAM_INITIALIZED)
         ret = inflateReset(&png_ptr->zstream);

      else
      {
         ret = inflateInit(&png_ptr->zstream);

         if (ret == Z_OK)
            png_ptr->flags |= PNG_FLAG_ZSTREAM_INITIALIZED;
      }

      if (ret == Z_OK)
         png_ptr->flags |= PNG_FLAG_ZSTREAM_IN_USE;

      else
      {
         /* A problem; the flags are set ok, but we need a credible error
          * message.
          */
         if (png_ptr->zstream.msg != NULL)
            png_error(png_ptr, png_ptr->zstream.msg);

         else
            png_error(png_ptr, "zlib initialization error");
      }
   }

   else
      png_error(png_ptr, "zstream already in use (internal error)");
}

#ifdef PNG_READ_COMPRESSED_TEXT_SUPPORTED
/* png_inflate: returns one of the following error codes.  If output is not NULL
 * the uncompressed data is placed there, up to output_size.  output_size is
 * adjusted to the actual amount of uncompressed data.   If
 * PNG_INFLATE_TRUNCATED is returned partial output will have been produced; no
 * error or warning is produced in this case.
 */
#define PNG_INFLATE_OK        1
#define PNG_INFLATE_TRUNCATED 2 /* output_size will be unchanged */
#define PNG_INFLATE_ERROR     3 /* a chunk warning has been output */

static void
stash_error(png_structrp png_ptr, png_const_charp message)
{
   size_t len = strlen(message);

   if (len >= png_ptr->zbuf_size)
      len = png_ptr->zbuf_size-1;

   memcpy(png_ptr->zbuf, message, len);
   png_ptr->zbuf[len] = 0;
}

static int
png_inflate(png_structrp png_ptr, png_bytep data, png_uint_32 input_size,
    png_bytep output, png_alloc_size_t *output_size)
{
   int ret;
   png_alloc_size_t avail_out = *output_size;
   png_uint_32 avail_in = input_size;

   /* zlib can't necessarily handle more than 65535 bytes at once (i.e. it can't
    * even necessarily handle 65536 bytes) because the type uInt is "16 bits or
    * more".  Consequently it is necessary to chunk the input to zlib.  This
    * code uses ZLIB_IO_MAX, from pngpriv.h, as the maximum (the maximum value
    * that can be stored in a uInt.)  It is possible to set ZLIB_IO_MAX to a
    * lower value in pngpriv.h and this may sometimes have a performance
    * advantage, because it forces access of the input data to be separated from
    * at least some of the use by some period of time.
    */
   png_ptr->zstream.next_in = data;
   /* avail_in and avail_out are set below from 'size' */
   png_ptr->zstream.avail_in = 0;
   png_ptr->zstream.avail_out = 0;

   /* Read directly into the output if it is available (this is set to
    * png_ptr->zbuf below if output is NULL).
    */
   if (output != NULL)
      png_ptr->zstream.next_out = output;

   do
   {
      uInt avail;

      /* zlib INPUT BUFFER */
      /* The setting of 'avail_in' used to be outside the loop; by setting it
       * inside it is possible to chunk the input to zlib and simply rely on
       * zlib to advance the 'next_in' pointer.  This allows arbitrary amounts o
       * data to be passed through zlib at the unavoidable cost of requiring a
       * window save (png_memcpy of up to 32768 output bytes) every ZLIB_IO_MAX
       * input bytes.
       */
      avail_in += png_ptr->zstream.avail_in; /* not consumed last time */

      avail = ZLIB_IO_MAX;

      if (avail_in < avail)
         avail = (uInt)avail_in; /* safe: < than ZLIB_IO_MAX */

      avail_in -= avail;
      png_ptr->zstream.avail_in = avail;

      /* zlib OUTPUT BUFFER */
      avail_out += png_ptr->zstream.avail_out; /* not written last time */

      avail = ZLIB_IO_MAX; /* maximum zlib can process */

      if (output == NULL)
      {
         /* Reset the output buffer each time round if output is NULL and make
          * available the full buffer, up to 'remaining_space'
          */
         png_ptr->zstream.next_out = png_ptr->zbuf;
         if (png_ptr->zbuf_size < avail)
            avail = png_ptr->zbuf_size;
      }

      if (avail_out < avail)
         avail = (uInt)avail_out; /* safe: < ZLIB_IO_MAX */

      png_ptr->zstream.avail_out = avail;
      avail_out -= avail;

      /* zlib inflate call */
      /* In fact 'avail_out' may be 0 at this point, that happens at the end of
       * the read when the final LZ end code was not passed at the end of the
       * previous chunk of input data.  Tell zlib if we have reached the end of
       * the output buffer.
       *
       * TODO: this prevents multiple calls to inflate, but may help inflate
       * avoid overhead.  Make using Z_FINISH an option for the caller.
       */
      ret = inflate(&png_ptr->zstream, avail_out > 0 ? Z_NO_FLUSH : Z_FINISH);
   } while (ret == Z_OK);

   /* Claw back the 'size' and 'remaining_space' byte counts. */
   avail_in += png_ptr->zstream.avail_in;
   avail_out += png_ptr->zstream.avail_out;

   /* Always reset the zstream, it must be left in inflateInit state.
    *
    * TODO: don't do this on Z_BUF_ERROR and no remaining space, then the
    * caller can use png_inflate incrementally.  Also change to using the
    * same approach as in the write code - init and reset only on demand.
    */
   png_ptr->zstream.next_in = NULL;
   png_ptr->zstream.avail_in = 0;
   png_ptr->zstream.next_out = NULL;
   png_ptr->zstream.avail_out = 0;
   inflateReset(&png_ptr->zstream);

   /* Update the output size if the output was not filled.
    *
    * TODO: do this with 'size' as well, not currently a parameter.
    */
   if (avail_out > 0)
      *output_size -= avail_out;

   /* Error messages are stashed in png_ptr->zbuf, to avoid any errors in the
    * errors do this first:
    */
   png_ptr->zbuf[0] = 0;

   switch (ret)
   {
      case Z_STREAM_END:
         png_ptr->flags |= PNG_FLAG_ZSTREAM_ENDED;
         return PNG_INFLATE_OK;

      case Z_BUF_ERROR: /* no progress in zlib */
         if (avail_out > 0) /* input truncated */
         {
            stash_error(png_ptr, "LZ data truncated");
            return PNG_INFLATE_ERROR;
         }

         else if (avail_in > 0) /* output truncated */
         {
            /* TODO: currently restarting a partial read isn't supporte because
             * of the inflateReset above, change this to do the Reset/Init on
             * demand.
             */
            stash_error(png_ptr, "uncompressed data too large");
            return PNG_INFLATE_TRUNCATED;
         }

      default:
         /* Everything else is an error, expect 'Z_DATA_ERROR' and
          * zstream.msg to be non-NULL.  In any case just copy something
          * into zbuf and return the error code.
          */
         if (png_ptr->zstream.msg != NULL)
            stash_error(png_ptr, png_ptr->zstream.msg);

         else
            stash_error(png_ptr, "LZ data damaged");

         return PNG_INFLATE_ERROR;
   }
}

/*
 * Decompress trailing data in a chunk.  The assumption is that chunkdata
 * points at an allocated area holding the contents of a chunk with a
 * trailing compressed part.  What we get back is an allocated area
 * holding the original prefix part and an uncompressed version of the
 * trailing part (the malloc area passed in is freed).
 */
static png_const_charp
png_decompress_chunk(png_structrp png_ptr, png_uint_32 chunklength,
   png_uint_32 prefix_size,
   png_alloc_size_t *newlength /* must be initialized to the maximum! */,
   int terminate /*add a '\0' to the end of the uncompressed data*/)
{
   /* TODO: implement different limits for different types of chunk.
    *
    * The caller supplies *newlength set to the maximum length of the
    * uncompressed data, but this routine allocates space for the prefix and
    * maybe a '\0' terminator too.  We have to assume that 'prefix_size' is
    * limited only by the maximum chunk size.
    */
   png_alloc_size_t limit = PNG_SIZE_MAX;

   png_inflate_claim(png_ptr);

#  ifdef PNG_SET_CHUNK_MALLOC_LIMIT_SUPPORTED
      if (png_ptr->user_chunk_malloc_max > 0 &&
         png_ptr->user_chunk_malloc_max < limit)
         limit = png_ptr->user_chunk_malloc_max;
#  elif PNG_USER_CHUNK_MALLOC_MAX > 0
      if (PNG_USER_CHUNK_MALLOC_MAX < limit)
         limit = PNG_USER_CHUNK_MALLOC_MAX;
#  endif

   if (limit >= prefix_size + (terminate != 0))
   {
      limit -= prefix_size + (terminate != 0);

      if (limit < *newlength)
         *newlength = limit;

      switch (png_inflate(png_ptr,
         (png_bytep)png_ptr->chunkdata + prefix_size,
         chunklength - prefix_size, NULL /* output */, newlength))
      {
         case PNG_INFLATE_OK:
            /* Success (maybe) - really uncompress the chunk. */
            {
               /* Because of the limit checks above we know that the new,
                * expanded, size will fit in a size_t (let alone an
                * png_alloc_size_t).  Use png_malloc_base here to avoid an
                * extra OOM message.
                */
               png_alloc_size_t new_size = *newlength;
               png_bytep text = png_voidcast(png_bytep, png_malloc_base(
                  png_ptr, prefix_size + new_size + (terminate != 0)));

               if (text == NULL)
                  break; /* out of memory */

               /* This should now work, although it is possible a logic
                * error might result.
                */
               switch (png_inflate(png_ptr, (png_bytep)png_ptr->chunkdata +
                  prefix_size, chunklength - prefix_size, text + prefix_size,
                  newlength))
               {
                  case PNG_INFLATE_OK:
                     if (new_size == *newlength)
                     {
                        if (terminate)
                           text[prefix_size + *newlength] = 0;

                        if (prefix_size > 0)
                           png_memcpy(text, png_ptr->chunkdata, prefix_size);

                        png_free(png_ptr, png_ptr->chunkdata);
                        png_ptr->chunkdata = (png_charp)text;
                        png_ptr->flags &= ~PNG_FLAG_ZSTREAM_IN_USE;
                        return NULL;
                     }

                     /* This is an unexpected internal error. */
                     /* FALL THROUGH */

                  case PNG_INFLATE_TRUNCATED:
                     png_free(png_ptr, text);
                     png_ptr->flags &= ~PNG_FLAG_ZSTREAM_IN_USE;
                     return "libpng inflate error";

                  default: /* PNG_INFLATE_ERROR */
                     png_free(png_ptr, text);
                     png_ptr->flags &= ~PNG_FLAG_ZSTREAM_IN_USE;
                     return (png_const_charp)png_ptr->zbuf;
               }
            }

         case PNG_INFLATE_TRUNCATED:
            /* This means that there wasn't enough space to uncompress the
             * chunk.
             */
            break;

         default: /* PNG_INFLATE_ERROR */
            /* png_inflate puts the error message in zbuf. */
            png_ptr->flags &= ~PNG_FLAG_ZSTREAM_IN_USE;
            return (png_const_charp)png_ptr->zbuf;
      }
   }

   /* out of memory returns. */
   png_ptr->flags &= ~PNG_FLAG_ZSTREAM_IN_USE;
   return "insufficient memory";
}
#endif /* PNG_READ_COMPRESSED_TEXT_SUPPORTED */

/* Read and check the IDHR chunk */
void /* PRIVATE */
png_handle_IHDR(png_structrp png_ptr, png_inforp info_ptr, png_uint_32 length)
{
   png_byte buf[13];
   png_uint_32 width, height;
   int bit_depth, color_type, compression_type, filter_type;
   int interlace_type;

   png_debug(1, "in png_handle_IHDR");

   if (png_ptr->mode & PNG_HAVE_IHDR)
      png_error(png_ptr, "Out of place IHDR");

   /* Check the length */
   if (length != 13)
      png_error(png_ptr, "Invalid IHDR chunk");

   png_ptr->mode |= PNG_HAVE_IHDR;

   png_crc_read(png_ptr, buf, 13);
   png_crc_finish(png_ptr, 0);

   width = png_get_uint_31(png_ptr, buf);
   height = png_get_uint_31(png_ptr, buf + 4);
   bit_depth = buf[8];
   color_type = buf[9];
   compression_type = buf[10];
   filter_type = buf[11];
   interlace_type = buf[12];

   /* Set internal variables */
   png_ptr->width = width;
   png_ptr->height = height;
   png_ptr->bit_depth = (png_byte)bit_depth;
   png_ptr->interlaced = (png_byte)interlace_type;
   png_ptr->color_type = (png_byte)color_type;
#ifdef PNG_MNG_FEATURES_SUPPORTED
   png_ptr->filter_type = (png_byte)filter_type;
#endif
   png_ptr->compression_type = (png_byte)compression_type;

   /* Find number of channels */
   switch (png_ptr->color_type)
   {
      default: /* invalid, png_set_IHDR calls png_error */
      case PNG_COLOR_TYPE_GRAY:
      case PNG_COLOR_TYPE_PALETTE:
         png_ptr->channels = 1;
         break;

      case PNG_COLOR_TYPE_RGB:
         png_ptr->channels = 3;
         break;

      case PNG_COLOR_TYPE_GRAY_ALPHA:
         png_ptr->channels = 2;
         break;

      case PNG_COLOR_TYPE_RGB_ALPHA:
         png_ptr->channels = 4;
         break;
   }

   /* Set up other useful info */
   png_ptr->pixel_depth = (png_byte)(png_ptr->bit_depth *
   png_ptr->channels);
   png_ptr->rowbytes = PNG_ROWBYTES(png_ptr->pixel_depth, png_ptr->width);
   png_debug1(3, "bit_depth = %d", png_ptr->bit_depth);
   png_debug1(3, "channels = %d", png_ptr->channels);
   png_debug1(3, "rowbytes = %lu", (unsigned long)png_ptr->rowbytes);
   png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth,
       color_type, interlace_type, compression_type, filter_type);
}

/* Read and check the palette */
void /* PRIVATE */
png_handle_PLTE(png_structrp png_ptr, png_inforp info_ptr, png_uint_32 length)
{
   png_color palette[PNG_MAX_PALETTE_LENGTH];
   int num, i;
#ifdef PNG_POINTER_INDEXING_SUPPORTED
   png_colorp pal_ptr;
#endif

   png_debug(1, "in png_handle_PLTE");

   if (!(png_ptr->mode & PNG_HAVE_IHDR))
      png_error(png_ptr, "Missing IHDR before PLTE");

   else if (png_ptr->mode & PNG_HAVE_IDAT)
   {
      png_warning(png_ptr, "Invalid PLTE after IDAT");
      png_crc_finish(png_ptr, length);
      return;
   }

   else if (png_ptr->mode & PNG_HAVE_PLTE)
      png_error(png_ptr, "Duplicate PLTE chunk");

   png_ptr->mode |= PNG_HAVE_PLTE;

   if (!(png_ptr->color_type&PNG_COLOR_MASK_COLOR))
   {
      png_warning(png_ptr,
          "Ignoring PLTE chunk in grayscale PNG");
      png_crc_finish(png_ptr, length);
      return;
   }

#ifndef PNG_READ_OPT_PLTE_SUPPORTED
   if (png_ptr->color_type != PNG_COLOR_TYPE_PALETTE)
   {
      png_crc_finish(png_ptr, length);
      return;
   }
#endif

   if (length > 3*PNG_MAX_PALETTE_LENGTH || length % 3)
   {
      if (png_ptr->color_type != PNG_COLOR_TYPE_PALETTE)
      {
         png_warning(png_ptr, "Invalid palette chunk");
         png_crc_finish(png_ptr, length);
         return;
      }

      else
      {
         png_error(png_ptr, "Invalid palette chunk");
      }
   }

   num = (int)length / 3;

#ifdef PNG_POINTER_INDEXING_SUPPORTED
   for (i = 0, pal_ptr = palette; i < num; i++, pal_ptr++)
   {
      png_byte buf[3];

      png_crc_read(png_ptr, buf, 3);
      pal_ptr->red = buf[0];
      pal_ptr->green = buf[1];
      pal_ptr->blue = buf[2];
   }
#else
   for (i = 0; i < num; i++)
   {
      png_byte buf[3];

      png_crc_read(png_ptr, buf, 3);
      /* Don't depend upon png_color being any order */
      palette[i].red = buf[0];
      palette[i].green = buf[1];
      palette[i].blue = buf[2];
   }
#endif

   /* If we actually need the PLTE chunk (ie for a paletted image), we do
    * whatever the normal CRC configuration tells us.  However, if we
    * have an RGB image, the PLTE can be considered ancillary, so
    * we will act as though it is.
    */
#ifndef PNG_READ_OPT_PLTE_SUPPORTED
   if (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
#endif
   {
      png_crc_finish(png_ptr, 0);
   }

#ifndef PNG_READ_OPT_PLTE_SUPPORTED
   else if (png_crc_error(png_ptr))  /* Only if we have a CRC error */
   {
      /* If we don't want to use the data from an ancillary chunk,
       * we have two options: an error abort, or a warning and we
       * ignore the data in this chunk (which should be OK, since
       * it's considered ancillary for a RGB or RGBA image).
       */
      if (!(png_ptr->flags & PNG_FLAG_CRC_ANCILLARY_USE))
      {
         if (png_ptr->flags & PNG_FLAG_CRC_ANCILLARY_NOWARN)
         {
            png_chunk_benign_error(png_ptr, "CRC error");
         }

         else
         {
            png_chunk_warning(png_ptr, "CRC error");
            return;
         }
      }

      /* Otherwise, we (optionally) emit a warning and use the chunk. */
      else if (!(png_ptr->flags & PNG_FLAG_CRC_ANCILLARY_NOWARN))
      {
         png_chunk_warning(png_ptr, "CRC error");
      }
   }
#endif

   png_set_PLTE(png_ptr, info_ptr, palette, num);

#ifdef PNG_READ_tRNS_SUPPORTED
   if (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
   {
      if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_tRNS))
      {
         if (png_ptr->num_trans > (png_uint_16)num)
         {
            png_warning(png_ptr, "Truncating incorrect tRNS chunk length");
            png_ptr->num_trans = (png_uint_16)num;
         }

         if (info_ptr->num_trans > (png_uint_16)num)
         {
            png_warning(png_ptr, "Truncating incorrect info tRNS chunk length");
            info_ptr->num_trans = (png_uint_16)num;
         }
      }
   }
#endif

}

void /* PRIVATE */
png_handle_IEND(png_structrp png_ptr, png_inforp info_ptr, png_uint_32 length)
{
   png_debug(1, "in png_handle_IEND");

   if (!(png_ptr->mode & PNG_HAVE_IHDR) || !(png_ptr->mode & PNG_HAVE_IDAT))
   {
      png_error(png_ptr, "No image in file");
   }

   png_ptr->mode |= (PNG_AFTER_IDAT | PNG_HAVE_IEND);

   if (length != 0)
   {
      png_warning(png_ptr, "Incorrect IEND chunk length");
   }

   png_crc_finish(png_ptr, length);

   PNG_UNUSED(info_ptr) /* Quiet compiler warnings about unused info_ptr */
}

#ifdef PNG_READ_gAMA_SUPPORTED
void /* PRIVATE */
png_handle_gAMA(png_structrp png_ptr, png_inforp info_ptr, png_uint_32 length)
{
   png_fixed_point igamma;
   png_byte buf[4];

   png_debug(1, "in png_handle_gAMA");

   if (!(png_ptr->mode & PNG_HAVE_IHDR))
      png_error(png_ptr, "Missing IHDR before gAMA");

   else if (png_ptr->mode & PNG_HAVE_IDAT)
   {
      png_warning(png_ptr, "Invalid gAMA after IDAT");
      png_crc_finish(png_ptr, length);
      return;
   }

   else if (png_ptr->mode & PNG_HAVE_PLTE)
      /* Should be an error, but we can cope with it */
      png_warning(png_ptr, "Out of place gAMA chunk");

   if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_gAMA)
#ifdef PNG_READ_sRGB_SUPPORTED
       && !(info_ptr->valid & PNG_INFO_sRGB)
#endif
       )
   {
      png_warning(png_ptr, "Duplicate gAMA chunk");
      png_crc_finish(png_ptr, length);
      return;
   }

   if (length != 4)
   {
      png_warning(png_ptr, "Incorrect gAMA chunk length");
      png_crc_finish(png_ptr, length);
      return;
   }

   png_crc_read(png_ptr, buf, 4);

   if (png_crc_finish(png_ptr, 0))
      return;

   igamma = png_get_fixed_point(NULL, buf);

   /* Check for zero gamma or an error. */
   if (igamma <= 0)
   {
      png_warning(png_ptr,
          "Ignoring gAMA chunk with out of range gamma");

      return;
   }

#  ifdef PNG_READ_sRGB_SUPPORTED
   if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_sRGB))
   {
      if (PNG_OUT_OF_RANGE(igamma, 45500, 500))
      {
         PNG_WARNING_PARAMETERS(p)
         png_warning_parameter_signed(p, 1, PNG_NUMBER_FORMAT_fixed, igamma);
         png_formatted_warning(png_ptr, p,
             "Ignoring incorrect gAMA value @1 when sRGB is also present");
         return;
      }
   }
#  endif /* PNG_READ_sRGB_SUPPORTED */

#  ifdef PNG_READ_GAMMA_SUPPORTED
   /* Gamma correction on read is supported. */
   png_ptr->gamma = igamma;
#  endif
   /* And set the 'info' structure members. */
   png_set_gAMA_fixed(png_ptr, info_ptr, igamma);
}
#endif

#ifdef PNG_READ_sBIT_SUPPORTED
void /* PRIVATE */
png_handle_sBIT(png_structrp png_ptr, png_inforp info_ptr, png_uint_32 length)
{
   unsigned int truelen;
   png_byte buf[4];

   png_debug(1, "in png_handle_sBIT");

   buf[0] = buf[1] = buf[2] = buf[3] = 0;

   if (!(png_ptr->mode & PNG_HAVE_IHDR))
      png_error(png_ptr, "Missing IHDR before sBIT");

   else if (png_ptr->mode & PNG_HAVE_IDAT)
   {
      png_warning(png_ptr, "Invalid sBIT after IDAT");
      png_crc_finish(png_ptr, length);
      return;
   }

   else if (png_ptr->mode & PNG_HAVE_PLTE)
   {
      /* Should be an error, but we can cope with it */
      png_warning(png_ptr, "Out of place sBIT chunk");
   }

   if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_sBIT))
   {
      png_warning(png_ptr, "Duplicate sBIT chunk");
      png_crc_finish(png_ptr, length);
      return;
   }

   if (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      truelen = 3;

   else
      truelen = png_ptr->channels;

   if (length != truelen || length > 4)
   {
      png_warning(png_ptr, "Incorrect sBIT chunk length");
      png_crc_finish(png_ptr, length);
      return;
   }

   png_crc_read(png_ptr, buf, truelen);

   if (png_crc_finish(png_ptr, 0))
      return;

   if (png_ptr->color_type & PNG_COLOR_MASK_COLOR)
   {
      png_ptr->sig_bit.red = buf[0];
      png_ptr->sig_bit.green = buf[1];
      png_ptr->sig_bit.blue = buf[2];
      png_ptr->sig_bit.alpha = buf[3];
   }

   else
   {
      png_ptr->sig_bit.gray = buf[0];
      png_ptr->sig_bit.red = buf[0];
      png_ptr->sig_bit.green = buf[0];
      png_ptr->sig_bit.blue = buf[0];
      png_ptr->sig_bit.alpha = buf[1];
   }

   png_set_sBIT(png_ptr, info_ptr, &(png_ptr->sig_bit));
}
#endif

#ifdef PNG_READ_cHRM_SUPPORTED
void /* PRIVATE */
png_handle_cHRM(png_structrp png_ptr, png_inforp info_ptr, png_uint_32 length)
{
   png_byte buf[32];
   png_fixed_point x_white, y_white, x_red, y_red, x_green, y_green, x_blue,
      y_blue;

   png_debug(1, "in png_handle_cHRM");

   if (!(png_ptr->mode & PNG_HAVE_IHDR))
      png_error(png_ptr, "Missing IHDR before cHRM");

   else if (png_ptr->mode & PNG_HAVE_IDAT)
   {
      png_warning(png_ptr, "Invalid cHRM after IDAT");
      png_crc_finish(png_ptr, length);
      return;
   }

   else if (png_ptr->mode & PNG_HAVE_PLTE)
      /* Should be an error, but we can cope with it */
      png_warning(png_ptr, "Out of place cHRM chunk");

   if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_cHRM)
#  ifdef PNG_READ_sRGB_SUPPORTED
       && !(info_ptr->valid & PNG_INFO_sRGB)
#  endif
      )
   {
      png_warning(png_ptr, "Duplicate cHRM chunk");
      png_crc_finish(png_ptr, length);
      return;
   }

   if (length != 32)
   {
      png_warning(png_ptr, "Incorrect cHRM chunk length");
      png_crc_finish(png_ptr, length);
      return;
   }

   png_crc_read(png_ptr, buf, 32);

   if (png_crc_finish(png_ptr, 0))
      return;

   x_white = png_get_fixed_point(NULL, buf);
   y_white = png_get_fixed_point(NULL, buf + 4);
   x_red   = png_get_fixed_point(NULL, buf + 8);
   y_red   = png_get_fixed_point(NULL, buf + 12);
   x_green = png_get_fixed_point(NULL, buf + 16);
   y_green = png_get_fixed_point(NULL, buf + 20);
   x_blue  = png_get_fixed_point(NULL, buf + 24);
   y_blue  = png_get_fixed_point(NULL, buf + 28);

   if (x_white == PNG_FIXED_ERROR ||
       y_white == PNG_FIXED_ERROR ||
       x_red   == PNG_FIXED_ERROR ||
       y_red   == PNG_FIXED_ERROR ||
       x_green == PNG_FIXED_ERROR ||
       y_green == PNG_FIXED_ERROR ||
       x_blue  == PNG_FIXED_ERROR ||
       y_blue  == PNG_FIXED_ERROR)
   {
      png_warning(png_ptr, "Ignoring cHRM chunk with negative chromaticities");
      return;
   }

#ifdef PNG_READ_sRGB_SUPPORTED
   if ((info_ptr != NULL) && (info_ptr->valid & PNG_INFO_sRGB))
   {
      if (PNG_OUT_OF_RANGE(x_white, 31270,  1000) ||
          PNG_OUT_OF_RANGE(y_white, 32900,  1000) ||
          PNG_OUT_OF_RANGE(x_red,   64000,  1000) ||
          PNG_OUT_OF_RANGE(y_red,   33000,  1000) ||
          PNG_OUT_OF_RANGE(x_green, 30000,  1000) ||
          PNG_OUT_OF_RANGE(y_green, 60000,  1000) ||
          PNG_OUT_OF_RANGE(x_blue,  15000,  1000) ||
          PNG_OUT_OF_RANGE(y_blue,   6000,  1000))
      {
         PNG_WARNING_PARAMETERS(p)

         png_warning_parameter_signed(p, 1, PNG_NUMBER_FORMAT_fixed, x_white);
         png_warning_parameter_signed(p, 2, PNG_NUMBER_FORMAT_fixed, y_white);
         png_warning_parameter_signed(p, 3, PNG_NUMBER_FORMAT_fixed, x_red);
         png_warning_parameter_signed(p, 4, PNG_NUMBER_FORMAT_fixed, y_red);
         png_warning_parameter_signed(p, 5, PNG_NUMBER_FORMAT_fixed, x_green);
         png_warning_parameter_signed(p, 6, PNG_NUMBER_FORMAT_fixed, y_green);
         png_warning_parameter_signed(p, 7, PNG_NUMBER_FORMAT_fixed, x_blue);
         png_warning_parameter_signed(p, 8, PNG_NUMBER_FORMAT_fixed, y_blue);

         png_formatted_warning(png_ptr, p,
             "Ignoring incorrect cHRM white(@1,@2) r(@3,@4)g(@5,@6)b(@7,@8) "
             "when sRGB is also present");
      }
      return;
   }
#endif /* PNG_READ_sRGB_SUPPORTED */

#ifdef PNG_READ_RGB_TO_GRAY_SUPPORTED
   /* Store the _white values as default coefficients for the rgb to gray
    * operation if it is supported.  Check if the transform is already set to
    * avoid destroying the transform values.
    */
   if (!png_ptr->rgb_to_gray_coefficients_set)
   {
      /* png_set_background has not been called and we haven't seen an sRGB
       * chunk yet.  Find the XYZ of the three end points.
       */
      png_XYZ XYZ;
      png_xy xy;

      xy.redx = x_red;
      xy.redy = y_red;
      xy.greenx = x_green;
      xy.greeny = y_green;
      xy.bluex = x_blue;
      xy.bluey = y_blue;
      xy.whitex = x_white;
      xy.whitey = y_white;

      if (png_XYZ_from_xy_checked(png_ptr, &XYZ, xy))
      {
         /* The success case, because XYZ_from_xy normalises to a reference
          * white Y of 1.0 we just need to scale the numbers.  This should
          * always work just fine. It is an internal error if this overflows.
          */
         {
            png_fixed_point r, g, b;
            if (png_muldiv(&r, XYZ.redY, 32768, PNG_FP_1) &&
               r >= 0 && r <= 32768 &&
               png_muldiv(&g, XYZ.greenY, 32768, PNG_FP_1) &&
               g >= 0 && g <= 32768 &&
               png_muldiv(&b, XYZ.blueY, 32768, PNG_FP_1) &&
               b >= 0 && b <= 32768 &&
               r+g+b <= 32769)
            {
               /* We allow 0 coefficients here.  r+g+b may be 32769 if two or
                * all of the coefficients were rounded up.  Handle this by
                * reducing the *largest* coefficient by 1; this matches the
                * approach used for the default coefficients in pngrtran.c
                */
               int add = 0;

               if (r+g+b > 32768)
                  add = -1;
               else if (r+g+b < 32768)
                  add = 1;

               if (add != 0)
               {
                  if (g >= r && g >= b)
                     g += add;
                  else if (r >= g && r >= b)
                     r += add;
                  else
                     b += add;
               }

               /* Check for an internal error. */
               if (r+g+b != 32768)
                  png_error(png_ptr,
                     "internal error handling cHRM coefficients");

               png_ptr->rgb_to_gray_red_coeff   = (png_uint_16)r;
               png_ptr->rgb_to_gray_green_coeff = (png_uint_16)g;
            }

            /* This is a png_error at present even though it could be ignored -
             * it should never happen, but it is important that if it does, the
             * bug is fixed.
             */
            else
               png_error(png_ptr, "internal error handling cHRM->XYZ");
         }
      }
   }
#endif

   png_set_cHRM_fixed(png_ptr, info_ptr, x_white, y_white, x_red, y_red,
      x_green, y_green, x_blue, y_blue);
}
#endif

#ifdef PNG_READ_sRGB_SUPPORTED
void /* PRIVATE */
png_handle_sRGB(png_structrp png_ptr, png_inforp info_ptr, png_uint_32 length)
{
   int intent;
   png_byte buf[1];

   png_debug(1, "in png_handle_sRGB");

   if (!(png_ptr->mode & PNG_HAVE_IHDR))
      png_error(png_ptr, "Missing IHDR before sRGB");

   else if (png_ptr->mode & PNG_HAVE_IDAT)
   {
      png_warning(png_ptr, "Invalid sRGB after IDAT");
      png_crc_finish(png_ptr, length);
      return;
   }

   else if (png_ptr->mode & PNG_HAVE_PLTE)
      /* Should be an error, but we can cope with it */
      png_warning(png_ptr, "Out of place sRGB chunk");

   if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_sRGB))
   {
      png_warning(png_ptr, "Duplicate sRGB chunk");
      png_crc_finish(png_ptr, length);
      return;
   }

   if (length != 1)
   {
      png_warning(png_ptr, "Incorrect sRGB chunk length");
      png_crc_finish(png_ptr, length);
      return;
   }

   png_crc_read(png_ptr, buf, 1);

   if (png_crc_finish(png_ptr, 0))
      return;

   intent = buf[0];

   /* Check for bad intent */
   if (intent >= PNG_sRGB_INTENT_LAST)
   {
      png_warning(png_ptr, "Unknown sRGB intent");
      return;
   }

#if defined(PNG_READ_gAMA_SUPPORTED) && defined(PNG_READ_GAMMA_SUPPORTED)
   if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_gAMA))
   {
      if (PNG_OUT_OF_RANGE(info_ptr->gamma, 45500, 500))
      {
         PNG_WARNING_PARAMETERS(p)

         png_warning_parameter_signed(p, 1, PNG_NUMBER_FORMAT_fixed,
            info_ptr->gamma);

         png_formatted_warning(png_ptr, p,
             "Ignoring incorrect gAMA value @1 when sRGB is also present");
      }
   }
#endif /* PNG_READ_gAMA_SUPPORTED */

#ifdef PNG_READ_cHRM_SUPPORTED
   if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_cHRM))
      if (PNG_OUT_OF_RANGE(info_ptr->x_white, 31270,  1000) ||
          PNG_OUT_OF_RANGE(info_ptr->y_white, 32900,  1000) ||
          PNG_OUT_OF_RANGE(info_ptr->x_red,   64000,  1000) ||
          PNG_OUT_OF_RANGE(info_ptr->y_red,   33000,  1000) ||
          PNG_OUT_OF_RANGE(info_ptr->x_green, 30000,  1000) ||
          PNG_OUT_OF_RANGE(info_ptr->y_green, 60000,  1000) ||
          PNG_OUT_OF_RANGE(info_ptr->x_blue,  15000,  1000) ||
          PNG_OUT_OF_RANGE(info_ptr->y_blue,   6000,  1000))
      {
         png_warning(png_ptr,
             "Ignoring incorrect cHRM value when sRGB is also present");
      }
#endif /* PNG_READ_cHRM_SUPPORTED */

   /* This is recorded for use when handling the cHRM chunk above.  An sRGB
    * chunk unconditionally overwrites the coefficients for grayscale conversion
    * too.
    */
   png_ptr->is_sRGB = 1;

#  ifdef PNG_READ_RGB_TO_GRAY_SUPPORTED
      /* Don't overwrite user supplied values: */
      if (!png_ptr->rgb_to_gray_coefficients_set)
      {
         /* These numbers come from the sRGB specification (or, since one has to
          * pay much money to get a copy, the wikipedia sRGB page) the
          * chromaticity values quoted have been inverted to get the reverse
          * transformation from RGB to XYZ and the 'Y' coefficients scaled by
          * 32768 (then rounded).
          *
          * sRGB and ITU Rec-709 both truncate the values for the D65 white
          * point to four digits and, even though it actually stores five
          * digits, the PNG spec gives the truncated value.
          *
          * This means that when the chromaticities are converted back to XYZ
          * end points we end up with (6968,23435,2366), which, as described in
          * pngrtran.c, would overflow.  If the five digit precision and up is
          * used we get, instead:
          *
          *    6968*R + 23435*G + 2365*B
          *
          * (Notice that this rounds the blue coefficient down, rather than the
          * choice used in pngrtran.c which is to round the green one down.)
          */
         png_ptr->rgb_to_gray_red_coeff   =  6968; /* 0.212639005871510 */
         png_ptr->rgb_to_gray_green_coeff = 23434; /* 0.715168678767756 */
         /* png_ptr->rgb_to_gray_blue_coeff  =  2366; 0.072192315360734	*/

         /* The following keeps the cHRM chunk from destroying the
          * coefficients again in the event that it follows the sRGB chunk.
          */
         png_ptr->rgb_to_gray_coefficients_set = 1;
      }
#  endif

   png_set_sRGB_gAMA_and_cHRM(png_ptr, info_ptr, intent);
}
#endif /* PNG_READ_sRGB_SUPPORTED */

#ifdef PNG_READ_iCCP_SUPPORTED
void /* PRIVATE */
png_handle_iCCP(png_structrp png_ptr, png_inforp info_ptr, png_uint_32 length)
/* Note: this does not properly handle chunks that are > 64K under DOS */
{
   png_uint_32 keyword_length;
   png_const_charp error_message = NULL;

   png_debug(1, "in png_handle_iCCP");

   if (!(png_ptr->mode & PNG_HAVE_IHDR))
      png_error(png_ptr, "Missing IHDR before iCCP");

   else if (png_ptr->mode & PNG_HAVE_IDAT)
   {
      png_crc_finish(png_ptr, length);
      png_chunk_benign_error(png_ptr, "Invalid iCCP after IDAT");
      return;
   }

   else if (png_ptr->mode & PNG_HAVE_PLTE)
   {
      /* Ignore out-of-place iCCP chunks because other implementations may
       * *have* to do so, so it is misleading if libpng handles them.
       */
      png_crc_finish(png_ptr, length);
      png_chunk_benign_error(png_ptr, "Out of place iCCP chunk");
      return;
   }

   if (info_ptr != NULL && (info_ptr->valid & (PNG_INFO_iCCP|PNG_INFO_sRGB)))
   {
      png_crc_finish(png_ptr, length);
      png_chunk_benign_error(png_ptr, "Duplicate color profile");
      return;
   }

   png_free(png_ptr, png_ptr->chunkdata);
   /* TODO: read the chunk in pieces, validating it as we go. */
   png_ptr->chunkdata = png_voidcast(png_charp, png_malloc(png_ptr, length));
   png_crc_read(png_ptr, (png_bytep)png_ptr->chunkdata, length);

   if (png_crc_finish(png_ptr, 0))
   {
      png_free(png_ptr, png_ptr->chunkdata);
      png_ptr->chunkdata = NULL;
      return;
   }

   /* TODO: also check that the keyword contents match the spec! */
   for (keyword_length = 0;
      keyword_length < length && png_ptr->chunkdata[keyword_length] != 0;
      ++keyword_length)
      /* Empty loop to find end of name */ ;

   if (keyword_length > 79 || keyword_length < 1)
   {
      png_free(png_ptr, png_ptr->chunkdata);
      png_ptr->chunkdata = NULL;
      png_chunk_benign_error(png_ptr, "Bad iCCP keyword");
      return;
   }

   /* There should be at least one zero (the compression type byte) following
    * the separator followed by some LZ data; check for at least one byte.
    * Since a chunk length is no more than 2^31 the addition below is safe.
    */
   if (keyword_length + 3 >= length)
   {
      png_free(png_ptr, png_ptr->chunkdata);
      png_ptr->chunkdata = NULL;
      png_chunk_benign_error(png_ptr, "iCCP chunk too short");
      return;
   }

   /* We only understand '0' compression - deflate - so if we get a different
    * value we can't safely decode the chunk.
    */
   if (png_ptr->chunkdata[keyword_length+1] != PNG_COMPRESSION_TYPE_BASE)
   {
      png_free(png_ptr, png_ptr->chunkdata);
      png_ptr->chunkdata = NULL;
      png_chunk_benign_error(png_ptr, "Bad compression type in iCCP chunk");
      return;
   }

   {
      png_alloc_size_t uncompressed_length;

      /* TODO: this is slightly broken, an ICC profile can be up to 2^32-1 bytes
       * in length, but it is stored here with the keyword and the compression
       * type prefixed to it.  This means that on 32-bit systems the code can't
       * accept, or express, the maximum length.
       */
      if (PNG_SIZE_MAX > 0xffffffffU/*ICC profile maximum*/)
         uncompressed_length = 0xffffffffU;

      else
         uncompressed_length = PNG_SIZE_MAX;

      /* TODO: at present png_decompress_chunk imposes a single application
       * level memory limit, this should be split to different values for iCCP
       * and text chunks.
       */
      error_message = png_decompress_chunk(png_ptr, length, keyword_length+2,
         &uncompressed_length, 0/*do not terminate*/);

      if (error_message == NULL)
      {
         /* It worked; png_ptr->chunkdata now contains the full uncompressed ICC
          * profile.  The cast below is safe because of the checks above, the
          * final uncompressed length can be at most 2^32-1
          */
         png_uint_32 profile_length = (png_uint_32)uncompressed_length;
         png_bytep profile = (png_bytep)png_ptr->chunkdata + (keyword_length+2);

         /* Now perform some basic checks.
          *
          * TODO: do a lot more checks, the format is pretty simple, at least
          * check that the whole tag table is there.
          */
         if (profile_length > 132 && png_get_uint_32(profile) == profile_length)
            png_set_iCCP(png_ptr, info_ptr, png_ptr->chunkdata,
                PNG_COMPRESSION_TYPE_BASE, profile, profile_length);

         else
            error_message = "Malformed ICC profile";
      }
   }

   png_free(png_ptr, png_ptr->chunkdata);
   png_ptr->chunkdata = NULL;

   if (error_message != NULL)
      png_chunk_benign_error(png_ptr, error_message);
}
#endif /* PNG_READ_iCCP_SUPPORTED */

#ifdef PNG_READ_sPLT_SUPPORTED
void /* PRIVATE */
png_handle_sPLT(png_structrp png_ptr, png_inforp info_ptr, png_uint_32 length)
/* Note: this does not properly handle chunks that are > 64K under DOS */
{
   png_bytep entry_start;
   png_sPLT_t new_palette;
   png_sPLT_entryp pp;
   png_uint_32 data_length;
   int entry_size, i;
   png_uint_32 skip = 0;
   png_size_t slength;
   png_uint_32 dl;
   png_size_t max_dl;

   png_debug(1, "in png_handle_sPLT");

#ifdef PNG_USER_LIMITS_SUPPORTED

   if (png_ptr->user_chunk_cache_max != 0)
   {
      if (png_ptr->user_chunk_cache_max == 1)
      {
         png_crc_finish(png_ptr, length);
         return;
      }

      if (--png_ptr->user_chunk_cache_max == 1)
      {
         png_warning(png_ptr, "No space in chunk cache for sPLT");
         png_crc_finish(png_ptr, length);
         return;
      }
   }
#endif

   if (!(png_ptr->mode & PNG_HAVE_IHDR))
      png_error(png_ptr, "Missing IHDR before sPLT");

   else if (png_ptr->mode & PNG_HAVE_IDAT)
   {
      png_warning(png_ptr, "Invalid sPLT after IDAT");
      png_crc_finish(png_ptr, length);
      return;
   }

#ifdef PNG_MAX_MALLOC_64K
   if (length > (png_uint_32)65535L)
   {
      png_warning(png_ptr, "sPLT chunk too large to fit in memory");
      skip = length - (png_uint_32)65535L;
      length = (png_uint_32)65535L;
   }
#endif

   png_free(png_ptr, png_ptr->chunkdata);
   png_ptr->chunkdata = (png_charp)png_malloc(png_ptr, length + 1);

   /* WARNING: this may break if size_t is less than 32 bits; it is assumed
    * that the PNG_MAX_MALLOC_64K test is enabled in this case, but this is a
    * potential breakage point if the types in pngconf.h aren't exactly right.
    */
   png_crc_read(png_ptr, (png_bytep)png_ptr->chunkdata, length);

   if (png_crc_finish(png_ptr, skip))
   {
      png_free(png_ptr, png_ptr->chunkdata);
      png_ptr->chunkdata = NULL;
      return;
   }

   slength = length;
   png_ptr->chunkdata[slength] = 0x00;

   for (entry_start = (png_bytep)png_ptr->chunkdata; *entry_start;
       entry_start++)
      /* Empty loop to find end of name */ ;

   ++entry_start;

   /* A sample depth should follow the separator, and we should be on it  */
   if (entry_start > (png_bytep)png_ptr->chunkdata + slength - 2)
   {
      png_free(png_ptr, png_ptr->chunkdata);
      png_ptr->chunkdata = NULL;
      png_warning(png_ptr, "malformed sPLT chunk");
      return;
   }

   new_palette.depth = *entry_start++;
   entry_size = (new_palette.depth == 8 ? 6 : 10);
   /* This must fit in a png_uint_32 because it is derived from the original
    * chunk data length (and use 'length', not 'slength' here for clarity -
    * they are guaranteed to be the same, see the tests above.)
    */
   data_length = length - (png_uint_32)(entry_start -
      (png_bytep)png_ptr->chunkdata);

   /* Integrity-check the data length */
   if (data_length % entry_size)
   {
      png_free(png_ptr, png_ptr->chunkdata);
      png_ptr->chunkdata = NULL;
      png_warning(png_ptr, "sPLT chunk has bad length");
      return;
   }

   dl = (png_int_32)(data_length / entry_size);
   max_dl = PNG_SIZE_MAX / png_sizeof(png_sPLT_entry);

   if (dl > max_dl)
   {
       png_warning(png_ptr, "sPLT chunk too long");
       return;
   }

   new_palette.nentries = (png_int_32)(data_length / entry_size);

   new_palette.entries = (png_sPLT_entryp)png_malloc_warn(
       png_ptr, new_palette.nentries * png_sizeof(png_sPLT_entry));

   if (new_palette.entries == NULL)
   {
       png_warning(png_ptr, "sPLT chunk requires too much memory");
       return;
   }

#ifdef PNG_POINTER_INDEXING_SUPPORTED
   for (i = 0; i < new_palette.nentries; i++)
   {
      pp = new_palette.entries + i;

      if (new_palette.depth == 8)
      {
         pp->red = *entry_start++;
         pp->green = *entry_start++;
         pp->blue = *entry_start++;
         pp->alpha = *entry_start++;
      }

      else
      {
         pp->red   = png_get_uint_16(entry_start); entry_start += 2;
         pp->green = png_get_uint_16(entry_start); entry_start += 2;
         pp->blue  = png_get_uint_16(entry_start); entry_start += 2;
         pp->alpha = png_get_uint_16(entry_start); entry_start += 2;
      }

      pp->frequency = png_get_uint_16(entry_start); entry_start += 2;
   }
#else
   pp = new_palette.entries;

   for (i = 0; i < new_palette.nentries; i++)
   {

      if (new_palette.depth == 8)
      {
         pp[i].red   = *entry_start++;
         pp[i].green = *entry_start++;
         pp[i].blue  = *entry_start++;
         pp[i].alpha = *entry_start++;
      }

      else
      {
         pp[i].red   = png_get_uint_16(entry_start); entry_start += 2;
         pp[i].green = png_get_uint_16(entry_start); entry_start += 2;
         pp[i].blue  = png_get_uint_16(entry_start); entry_start += 2;
         pp[i].alpha = png_get_uint_16(entry_start); entry_start += 2;
      }

      pp[i].frequency = png_get_uint_16(entry_start); entry_start += 2;
   }
#endif

   /* Discard all chunk data except the name and stash that */
   new_palette.name = png_ptr->chunkdata;

   png_set_sPLT(png_ptr, info_ptr, &new_palette, 1);

   png_free(png_ptr, png_ptr->chunkdata);
   png_ptr->chunkdata = NULL;
   png_free(png_ptr, new_palette.entries);
}
#endif /* PNG_READ_sPLT_SUPPORTED */

#ifdef PNG_READ_tRNS_SUPPORTED
void /* PRIVATE */
png_handle_tRNS(png_structrp png_ptr, png_inforp info_ptr, png_uint_32 length)
{
   png_byte readbuf[PNG_MAX_PALETTE_LENGTH];

   png_debug(1, "in png_handle_tRNS");

   if (!(png_ptr->mode & PNG_HAVE_IHDR))
      png_error(png_ptr, "Missing IHDR before tRNS");

   else if (png_ptr->mode & PNG_HAVE_IDAT)
   {
      png_warning(png_ptr, "Invalid tRNS after IDAT");
      png_crc_finish(png_ptr, length);
      return;
   }

   else if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_tRNS))
   {
      png_warning(png_ptr, "Duplicate tRNS chunk");
      png_crc_finish(png_ptr, length);
      return;
   }

   if (png_ptr->color_type == PNG_COLOR_TYPE_GRAY)
   {
      png_byte buf[2];

      if (length != 2)
      {
         png_warning(png_ptr, "Incorrect tRNS chunk length");
         png_crc_finish(png_ptr, length);
         return;
      }

      png_crc_read(png_ptr, buf, 2);
      png_ptr->num_trans = 1;
      png_ptr->trans_color.gray = png_get_uint_16(buf);
   }

   else if (png_ptr->color_type == PNG_COLOR_TYPE_RGB)
   {
      png_byte buf[6];

      if (length != 6)
      {
         png_warning(png_ptr, "Incorrect tRNS chunk length");
         png_crc_finish(png_ptr, length);
         return;
      }

      png_crc_read(png_ptr, buf, length);
      png_ptr->num_trans = 1;
      png_ptr->trans_color.red = png_get_uint_16(buf);
      png_ptr->trans_color.green = png_get_uint_16(buf + 2);
      png_ptr->trans_color.blue = png_get_uint_16(buf + 4);
   }

   else if (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
   {
      if (!(png_ptr->mode & PNG_HAVE_PLTE))
      {
         /* Should be an error, but we can cope with it. */
         png_warning(png_ptr, "Missing PLTE before tRNS");
      }

      if (length > (png_uint_32)png_ptr->num_palette ||
          length > PNG_MAX_PALETTE_LENGTH)
      {
         png_warning(png_ptr, "Incorrect tRNS chunk length");
         png_crc_finish(png_ptr, length);
         return;
      }

      if (length == 0)
      {
         png_warning(png_ptr, "Zero length tRNS chunk");
         png_crc_finish(png_ptr, length);
         return;
      }

      png_crc_read(png_ptr, readbuf, length);
      png_ptr->num_trans = (png_uint_16)length;
   }

   else
   {
      png_warning(png_ptr, "tRNS chunk not allowed with alpha channel");
      png_crc_finish(png_ptr, length);
      return;
   }

   if (png_crc_finish(png_ptr, 0))
   {
      png_ptr->num_trans = 0;
      return;
   }

   png_set_tRNS(png_ptr, info_ptr, readbuf, png_ptr->num_trans,
       &(png_ptr->trans_color));
}
#endif

#ifdef PNG_READ_bKGD_SUPPORTED
void /* PRIVATE */
png_handle_bKGD(png_structrp png_ptr, png_inforp info_ptr, png_uint_32 length)
{
   unsigned int truelen;
   png_byte buf[6];
   png_color_16 background;

   png_debug(1, "in png_handle_bKGD");

   if (!(png_ptr->mode & PNG_HAVE_IHDR))
      png_error(png_ptr, "Missing IHDR before bKGD");

   else if (png_ptr->mode & PNG_HAVE_IDAT)
   {
      png_warning(png_ptr, "Invalid bKGD after IDAT");
      png_crc_finish(png_ptr, length);
      return;
   }

   else if (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE &&
       !(png_ptr->mode & PNG_HAVE_PLTE))
   {
      png_warning(png_ptr, "Missing PLTE before bKGD");
      png_crc_finish(png_ptr, length);
      return;
   }

   else if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_bKGD))
   {
      png_warning(png_ptr, "Duplicate bKGD chunk");
      png_crc_finish(png_ptr, length);
      return;
   }

   if (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      truelen = 1;

   else if (png_ptr->color_type & PNG_COLOR_MASK_COLOR)
      truelen = 6;

   else
      truelen = 2;

   if (length != truelen)
   {
      png_warning(png_ptr, "Incorrect bKGD chunk length");
      png_crc_finish(png_ptr, length);
      return;
   }

   png_crc_read(png_ptr, buf, truelen);

   if (png_crc_finish(png_ptr, 0))
      return;

   /* We convert the index value into RGB components so that we can allow
    * arbitrary RGB values for background when we have transparency, and
    * so it is easy to determine the RGB values of the background color
    * from the info_ptr struct.
    */
   if (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
   {
      background.index = buf[0];

      if (info_ptr && info_ptr->num_palette)
      {
         if (buf[0] >= info_ptr->num_palette)
         {
            png_warning(png_ptr, "Incorrect bKGD chunk index value");
            return;
         }

         background.red = (png_uint_16)png_ptr->palette[buf[0]].red;
         background.green = (png_uint_16)png_ptr->palette[buf[0]].green;
         background.blue = (png_uint_16)png_ptr->palette[buf[0]].blue;
      }

      else
         background.red = background.green = background.blue = 0;

      background.gray = 0;
   }

   else if (!(png_ptr->color_type & PNG_COLOR_MASK_COLOR)) /* GRAY */
   {
      background.index = 0;
      background.red =
      background.green =
      background.blue =
      background.gray = png_get_uint_16(buf);
   }

   else
   {
      background.index = 0;
      background.red = png_get_uint_16(buf);
      background.green = png_get_uint_16(buf + 2);
      background.blue = png_get_uint_16(buf + 4);
      background.gray = 0;
   }

   png_set_bKGD(png_ptr, info_ptr, &background);
}
#endif

#ifdef PNG_READ_hIST_SUPPORTED
void /* PRIVATE */
png_handle_hIST(png_structrp png_ptr, png_inforp info_ptr, png_uint_32 length)
{
   unsigned int num, i;
   png_uint_16 readbuf[PNG_MAX_PALETTE_LENGTH];

   png_debug(1, "in png_handle_hIST");

   if (!(png_ptr->mode & PNG_HAVE_IHDR))
      png_error(png_ptr, "Missing IHDR before hIST");

   else if (png_ptr->mode & PNG_HAVE_IDAT)
   {
      png_warning(png_ptr, "Invalid hIST after IDAT");
      png_crc_finish(png_ptr, length);
      return;
   }

   else if (!(png_ptr->mode & PNG_HAVE_PLTE))
   {
      png_warning(png_ptr, "Missing PLTE before hIST");
      png_crc_finish(png_ptr, length);
      return;
   }

   else if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_hIST))
   {
      png_warning(png_ptr, "Duplicate hIST chunk");
      png_crc_finish(png_ptr, length);
      return;
   }

   num = length / 2 ;

   if (num != (unsigned int)png_ptr->num_palette || num >
       (unsigned int)PNG_MAX_PALETTE_LENGTH)
   {
      png_warning(png_ptr, "Incorrect hIST chunk length");
      png_crc_finish(png_ptr, length);
      return;
   }

   for (i = 0; i < num; i++)
   {
      png_byte buf[2];

      png_crc_read(png_ptr, buf, 2);
      readbuf[i] = png_get_uint_16(buf);
   }

   if (png_crc_finish(png_ptr, 0))
      return;

   png_set_hIST(png_ptr, info_ptr, readbuf);
}
#endif

#ifdef PNG_READ_pHYs_SUPPORTED
void /* PRIVATE */
png_handle_pHYs(png_structrp png_ptr, png_inforp info_ptr, png_uint_32 length)
{
   png_byte buf[9];
   png_uint_32 res_x, res_y;
   int unit_type;

   png_debug(1, "in png_handle_pHYs");

   if (!(png_ptr->mode & PNG_HAVE_IHDR))
      png_error(png_ptr, "Missing IHDR before pHYs");

   else if (png_ptr->mode & PNG_HAVE_IDAT)
   {
      png_warning(png_ptr, "Invalid pHYs after IDAT");
      png_crc_finish(png_ptr, length);
      return;
   }

   else if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_pHYs))
   {
      png_warning(png_ptr, "Duplicate pHYs chunk");
      png_crc_finish(png_ptr, length);
      return;
   }

   if (length != 9)
   {
      png_warning(png_ptr, "Incorrect pHYs chunk length");
      png_crc_finish(png_ptr, length);
      return;
   }

   png_crc_read(png_ptr, buf, 9);

   if (png_crc_finish(png_ptr, 0))
      return;

   res_x = png_get_uint_32(buf);
   res_y = png_get_uint_32(buf + 4);
   unit_type = buf[8];
   png_set_pHYs(png_ptr, info_ptr, res_x, res_y, unit_type);
}
#endif

#ifdef PNG_READ_oFFs_SUPPORTED
void /* PRIVATE */
png_handle_oFFs(png_structrp png_ptr, png_inforp info_ptr, png_uint_32 length)
{
   png_byte buf[9];
   png_int_32 offset_x, offset_y;
   int unit_type;

   png_debug(1, "in png_handle_oFFs");

   if (!(png_ptr->mode & PNG_HAVE_IHDR))
      png_error(png_ptr, "Missing IHDR before oFFs");

   else if (png_ptr->mode & PNG_HAVE_IDAT)
   {
      png_warning(png_ptr, "Invalid oFFs after IDAT");
      png_crc_finish(png_ptr, length);
      return;
   }

   else if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_oFFs))
   {
      png_warning(png_ptr, "Duplicate oFFs chunk");
      png_crc_finish(png_ptr, length);
      return;
   }

   if (length != 9)
   {
      png_warning(png_ptr, "Incorrect oFFs chunk length");
      png_crc_finish(png_ptr, length);
      return;
   }

   png_crc_read(png_ptr, buf, 9);

   if (png_crc_finish(png_ptr, 0))
      return;

   offset_x = png_get_int_32(buf);
   offset_y = png_get_int_32(buf + 4);
   unit_type = buf[8];
   png_set_oFFs(png_ptr, info_ptr, offset_x, offset_y, unit_type);
}
#endif

#ifdef PNG_READ_pCAL_SUPPORTED
/* Read the pCAL chunk (described in the PNG Extensions document) */
void /* PRIVATE */
png_handle_pCAL(png_structrp png_ptr, png_inforp info_ptr, png_uint_32 length)
{
   png_int_32 X0, X1;
   png_byte type, nparams;
   png_charp buf, units, endptr;
   png_charpp params;
   int i;

   png_debug(1, "in png_handle_pCAL");

   if (!(png_ptr->mode & PNG_HAVE_IHDR))
      png_error(png_ptr, "Missing IHDR before pCAL");

   else if (png_ptr->mode & PNG_HAVE_IDAT)
   {
      png_warning(png_ptr, "Invalid pCAL after IDAT");
      png_crc_finish(png_ptr, length);
      return;
   }

   else if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_pCAL))
   {
      png_warning(png_ptr, "Duplicate pCAL chunk");
      png_crc_finish(png_ptr, length);
      return;
   }

   png_debug1(2, "Allocating and reading pCAL chunk data (%u bytes)",
       length + 1);
   png_free(png_ptr, png_ptr->chunkdata);
   png_ptr->chunkdata = (png_charp)png_malloc_warn(png_ptr, length + 1);

   if (png_ptr->chunkdata == NULL)
   {
      png_warning(png_ptr, "No memory for pCAL purpose");
      return;
   }

   png_crc_read(png_ptr, (png_bytep)png_ptr->chunkdata, length);

   if (png_crc_finish(png_ptr, 0))
   {
      png_free(png_ptr, png_ptr->chunkdata);
      png_ptr->chunkdata = NULL;
      return;
   }

   png_ptr->chunkdata[length] = 0x00; /* Null terminate the last string */

   png_debug(3, "Finding end of pCAL purpose string");
   for (buf = png_ptr->chunkdata; *buf; buf++)
      /* Empty loop */ ;

   endptr = png_ptr->chunkdata + length;

   /* We need to have at least 12 bytes after the purpose string
    * in order to get the parameter information.
    */
   if (endptr <= buf + 12)
   {
      png_warning(png_ptr, "Invalid pCAL data");
      png_free(png_ptr, png_ptr->chunkdata);
      png_ptr->chunkdata = NULL;
      return;
   }

   png_debug(3, "Reading pCAL X0, X1, type, nparams, and units");
   X0 = png_get_int_32((png_bytep)buf+1);
   X1 = png_get_int_32((png_bytep)buf+5);
   type = buf[9];
   nparams = buf[10];
   units = buf + 11;

   png_debug(3, "Checking pCAL equation type and number of parameters");
   /* Check that we have the right number of parameters for known
    * equation types.
    */
   if ((type == PNG_EQUATION_LINEAR && nparams != 2) ||
       (type == PNG_EQUATION_BASE_E && nparams != 3) ||
       (type == PNG_EQUATION_ARBITRARY && nparams != 3) ||
       (type == PNG_EQUATION_HYPERBOLIC && nparams != 4))
   {
      png_warning(png_ptr, "Invalid pCAL parameters for equation type");
      png_free(png_ptr, png_ptr->chunkdata);
      png_ptr->chunkdata = NULL;
      return;
   }

   else if (type >= PNG_EQUATION_LAST)
   {
      png_warning(png_ptr, "Unrecognized equation type for pCAL chunk");
   }

   for (buf = units; *buf; buf++)
      /* Empty loop to move past the units string. */ ;

   png_debug(3, "Allocating pCAL parameters array");

   params = (png_charpp)png_malloc_warn(png_ptr,
       (png_size_t)(nparams * png_sizeof(png_charp)));

   if (params == NULL)
   {
      png_free(png_ptr, png_ptr->chunkdata);
      png_ptr->chunkdata = NULL;
      png_warning(png_ptr, "No memory for pCAL params");
      return;
   }

   /* Get pointers to the start of each parameter string. */
   for (i = 0; i < (int)nparams; i++)
   {
      buf++; /* Skip the null string terminator from previous parameter. */

      png_debug1(3, "Reading pCAL parameter %d", i);

      for (params[i] = buf; buf <= endptr && *buf != 0x00; buf++)
         /* Empty loop to move past each parameter string */ ;

      /* Make sure we haven't run out of data yet */
      if (buf > endptr)
      {
         png_warning(png_ptr, "Invalid pCAL data");
         png_free(png_ptr, png_ptr->chunkdata);
         png_ptr->chunkdata = NULL;
         png_free(png_ptr, params);
         return;
      }
   }

   png_set_pCAL(png_ptr, info_ptr, png_ptr->chunkdata, X0, X1, type, nparams,
      units, params);

   png_free(png_ptr, png_ptr->chunkdata);
   png_ptr->chunkdata = NULL;
   png_free(png_ptr, params);
}
#endif

#ifdef PNG_READ_sCAL_SUPPORTED
/* Read the sCAL chunk */
void /* PRIVATE */
png_handle_sCAL(png_structrp png_ptr, png_inforp info_ptr, png_uint_32 length)
{
   png_size_t slength, i;
   int state;

   png_debug(1, "in png_handle_sCAL");

   if (!(png_ptr->mode & PNG_HAVE_IHDR))
      png_error(png_ptr, "Missing IHDR before sCAL");

   else if (png_ptr->mode & PNG_HAVE_IDAT)
   {
      png_warning(png_ptr, "Invalid sCAL after IDAT");
      png_crc_finish(png_ptr, length);
      return;
   }

   else if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_sCAL))
   {
      png_warning(png_ptr, "Duplicate sCAL chunk");
      png_crc_finish(png_ptr, length);
      return;
   }

   /* Need unit type, width, \0, height: minimum 4 bytes */
   else if (length < 4)
   {
      png_warning(png_ptr, "sCAL chunk too short");
      png_crc_finish(png_ptr, length);
      return;
   }

   png_debug1(2, "Allocating and reading sCAL chunk data (%u bytes)",
      length + 1);

   png_ptr->chunkdata = (png_charp)png_malloc_warn(png_ptr, length + 1);

   if (png_ptr->chunkdata == NULL)
   {
      png_warning(png_ptr, "Out of memory while processing sCAL chunk");
      png_crc_finish(png_ptr, length);
      return;
   }

   png_crc_read(png_ptr, (png_bytep)png_ptr->chunkdata, length);
   slength = length;
   png_ptr->chunkdata[slength] = 0x00; /* Null terminate the last string */

   if (png_crc_finish(png_ptr, 0))
   {
      png_free(png_ptr, png_ptr->chunkdata);
      png_ptr->chunkdata = NULL;
      return;
   }

   /* Validate the unit. */
   if (png_ptr->chunkdata[0] != 1 && png_ptr->chunkdata[0] != 2)
   {
      png_warning(png_ptr, "Invalid sCAL ignored: invalid unit");
      png_free(png_ptr, png_ptr->chunkdata);
      png_ptr->chunkdata = NULL;
      return;
   }

   /* Validate the ASCII numbers, need two ASCII numbers separated by
    * a '\0' and they need to fit exactly in the chunk data.
    */
   i = 1;
   state = 0;

   if (!png_check_fp_number(png_ptr->chunkdata, slength, &state, &i) ||
       i >= slength || png_ptr->chunkdata[i++] != 0)
      png_warning(png_ptr, "Invalid sCAL chunk ignored: bad width format");

   else if (!PNG_FP_IS_POSITIVE(state))
      png_warning(png_ptr, "Invalid sCAL chunk ignored: non-positive width");

   else
   {
      png_size_t heighti = i;

      state = 0;
      if (!png_check_fp_number(png_ptr->chunkdata, slength, &state, &i) ||
          i != slength)
         png_warning(png_ptr, "Invalid sCAL chunk ignored: bad height format");

      else if (!PNG_FP_IS_POSITIVE(state))
         png_warning(png_ptr,
            "Invalid sCAL chunk ignored: non-positive height");

      else
         /* This is the (only) success case. */
         png_set_sCAL_s(png_ptr, info_ptr, png_ptr->chunkdata[0],
            png_ptr->chunkdata+1, png_ptr->chunkdata+heighti);
   }

   /* Clean up - just free the temporarily allocated buffer. */
   png_free(png_ptr, png_ptr->chunkdata);
   png_ptr->chunkdata = NULL;
}
#endif

#ifdef PNG_READ_tIME_SUPPORTED
void /* PRIVATE */
png_handle_tIME(png_structrp png_ptr, png_inforp info_ptr, png_uint_32 length)
{
   png_byte buf[7];
   png_time mod_time;

   png_debug(1, "in png_handle_tIME");

   if (!(png_ptr->mode & PNG_HAVE_IHDR))
      png_error(png_ptr, "Out of place tIME chunk");

   else if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_tIME))
   {
      png_warning(png_ptr, "Duplicate tIME chunk");
      png_crc_finish(png_ptr, length);
      return;
   }

   if (png_ptr->mode & PNG_HAVE_IDAT)
      png_ptr->mode |= PNG_AFTER_IDAT;

   if (length != 7)
   {
      png_warning(png_ptr, "Incorrect tIME chunk length");
      png_crc_finish(png_ptr, length);
      return;
   }

   png_crc_read(png_ptr, buf, 7);

   if (png_crc_finish(png_ptr, 0))
      return;

   mod_time.second = buf[6];
   mod_time.minute = buf[5];
   mod_time.hour = buf[4];
   mod_time.day = buf[3];
   mod_time.month = buf[2];
   mod_time.year = png_get_uint_16(buf);

   png_set_tIME(png_ptr, info_ptr, &mod_time);
}
#endif

#ifdef PNG_READ_tEXt_SUPPORTED
/* Note: this does not properly handle chunks that are > 64K under DOS */
void /* PRIVATE */
png_handle_tEXt(png_structrp png_ptr, png_inforp info_ptr, png_uint_32 length)
{
   png_textp text_ptr;
   png_charp key;
   png_charp text;
   png_uint_32 skip = 0;
   int ret;

   png_debug(1, "in png_handle_tEXt");

#ifdef PNG_USER_LIMITS_SUPPORTED
   if (png_ptr->user_chunk_cache_max != 0)
   {
      if (png_ptr->user_chunk_cache_max == 1)
      {
         png_crc_finish(png_ptr, length);
         return;
      }

      if (--png_ptr->user_chunk_cache_max == 1)
      {
         png_warning(png_ptr, "No space in chunk cache for tEXt");
         png_crc_finish(png_ptr, length);
         return;
      }
   }
#endif

   if (!(png_ptr->mode & PNG_HAVE_IHDR))
      png_error(png_ptr, "Missing IHDR before tEXt");

   if (png_ptr->mode & PNG_HAVE_IDAT)
      png_ptr->mode |= PNG_AFTER_IDAT;

#ifdef PNG_MAX_MALLOC_64K
   if (length > (png_uint_32)65535L)
   {
      png_warning(png_ptr, "tEXt chunk too large to fit in memory");
      skip = length - (png_uint_32)65535L;
      length = (png_uint_32)65535L;
   }
#endif

   png_free(png_ptr, png_ptr->chunkdata);

   png_ptr->chunkdata = (png_charp)png_malloc_warn(png_ptr, length + 1);

   if (png_ptr->chunkdata == NULL)
   {
     png_warning(png_ptr, "No memory to process text chunk");
     return;
   }

   png_crc_read(png_ptr, (png_bytep)png_ptr->chunkdata, length);

   if (png_crc_finish(png_ptr, skip))
   {
      png_free(png_ptr, png_ptr->chunkdata);
      png_ptr->chunkdata = NULL;
      return;
   }

   key = png_ptr->chunkdata;

   key[length] = 0x00;

   for (text = key; *text; text++)
      /* Empty loop to find end of key */ ;

   if (text != key + length)
      text++;

   text_ptr = (png_textp)png_malloc_warn(png_ptr,
       png_sizeof(png_text));

   if (text_ptr == NULL)
   {
      png_warning(png_ptr, "Not enough memory to process text chunk");
      png_free(png_ptr, png_ptr->chunkdata);
      png_ptr->chunkdata = NULL;
      return;
   }

   text_ptr->compression = PNG_TEXT_COMPRESSION_NONE;
   text_ptr->key = key;
   text_ptr->lang = NULL;
   text_ptr->lang_key = NULL;
   text_ptr->itxt_length = 0;
   text_ptr->text = text;
   text_ptr->text_length = png_strlen(text);

   ret = png_set_text_2(png_ptr, info_ptr, text_ptr, 1);

   png_free(png_ptr, png_ptr->chunkdata);
   png_ptr->chunkdata = NULL;
   png_free(png_ptr, text_ptr);

   if (ret)
      png_warning(png_ptr, "Insufficient memory to process text chunk");
}
#endif

#ifdef PNG_READ_zTXt_SUPPORTED
/* Note: this does not correctly handle chunks that are > 64K under DOS */
void /* PRIVATE */
png_handle_zTXt(png_structrp png_ptr, png_inforp info_ptr, png_uint_32 length)
{
   png_const_charp errmsg = NULL;
   png_uint_32 keyword_length;

   png_debug(1, "in png_handle_zTXt");

#ifdef PNG_USER_LIMITS_SUPPORTED
   if (png_ptr->user_chunk_cache_max != 0)
   {
      if (png_ptr->user_chunk_cache_max == 1)
      {
         png_crc_finish(png_ptr, length);
         return;
      }

      if (--png_ptr->user_chunk_cache_max == 1)
      {
         png_warning(png_ptr, "No space in chunk cache for zTXt");
         png_crc_finish(png_ptr, length);
         return;
      }
   }
#endif

   if (!(png_ptr->mode & PNG_HAVE_IHDR))
      png_error(png_ptr, "Missing IHDR before zTXt");

   if (png_ptr->mode & PNG_HAVE_IDAT)
      png_ptr->mode |= PNG_AFTER_IDAT;

   png_free(png_ptr, png_ptr->chunkdata);
   png_ptr->chunkdata = png_voidcast(png_charp, png_malloc_warn(png_ptr,
      length));

   if (png_ptr->chunkdata == NULL)
   {
      png_chunk_benign_error(png_ptr, "insufficient memory");
      return;
   }

   png_crc_read(png_ptr, (png_bytep)png_ptr->chunkdata, length);

   if (png_crc_finish(png_ptr, 0))
   {
      png_free(png_ptr, png_ptr->chunkdata);
      png_ptr->chunkdata = NULL;
      return;
   }

   /* TODO: also check that the keyword contents match the spec! */
   for (keyword_length = 0;
      keyword_length < length && png_ptr->chunkdata[keyword_length] != 0;
      ++keyword_length)
      /* Empty loop to find end of name */ ;

   if (keyword_length > 79 || keyword_length < 1)
      errmsg = "bad keyword";

   /* zTXt must have some LZ data after the keyword, although it may expand to
    * zero bytes; we need a '\0' at the end of the keyword, the compression type
    * then the LZ data:
    */
   else if (keyword_length + 3 > length)
      errmsg = "truncated";

   else if (png_ptr->chunkdata[keyword_length+1] != PNG_COMPRESSION_TYPE_BASE)
      errmsg = "unknown compression type";

   else
   {
      png_alloc_size_t uncompressed_length = PNG_SIZE_MAX;

      /* TODO: at present png_decompress_chunk imposes a single application
       * level memory limit, this should be split to different values for iCCP
       * and text chunks.
       */
      errmsg = png_decompress_chunk(png_ptr, length, keyword_length+2,
         &uncompressed_length, 1/*terminate*/);

      if (errmsg == NULL)
      {
         png_text text;

         /* It worked; png_ptr->chunkdata now looks like a tEXt chunk except for
          * the extra compression type byte and the fact that it isn't
          * necessarily '\0' terminated.
          */
         png_ptr->chunkdata[uncompressed_length+(keyword_length+2)] = 0;

         text.compression = PNG_TEXT_COMPRESSION_zTXt;
         text.key = png_ptr->chunkdata;
         text.text = png_ptr->chunkdata + keyword_length+2;
         text.text_length = uncompressed_length;
         text.itxt_length = 0;
         text.lang = NULL;
         text.lang_key = NULL;

         if (png_set_text_2(png_ptr, info_ptr, &text, 1))
            errmsg = "insufficient memory to store zTXt chunk";
      }
   }

   png_free(png_ptr, png_ptr->chunkdata);
   png_ptr->chunkdata = NULL;

   if (errmsg != NULL)
      png_chunk_benign_error(png_ptr, errmsg);
}
#endif

#ifdef PNG_READ_iTXt_SUPPORTED
/* Note: this does not correctly handle chunks that are > 64K under DOS */
void /* PRIVATE */
png_handle_iTXt(png_structrp png_ptr, png_inforp info_ptr, png_uint_32 length)
{
   png_const_charp errmsg = NULL;
   png_uint_32 prefix_length;

   png_debug(1, "in png_handle_iTXt");

#ifdef PNG_USER_LIMITS_SUPPORTED
   if (png_ptr->user_chunk_cache_max != 0)
   {
      if (png_ptr->user_chunk_cache_max == 1)
      {
         png_crc_finish(png_ptr, length);
         return;
      }

      if (--png_ptr->user_chunk_cache_max == 1)
      {
         png_warning(png_ptr, "No space in chunk cache for iTXt");
         png_crc_finish(png_ptr, length);
         return;
      }
   }
#endif

   if (!(png_ptr->mode & PNG_HAVE_IHDR))
      png_error(png_ptr, "Missing IHDR before iTXt");

   if (png_ptr->mode & PNG_HAVE_IDAT)
      png_ptr->mode |= PNG_AFTER_IDAT;

   png_free(png_ptr, png_ptr->chunkdata);
   png_ptr->chunkdata = png_voidcast(png_charp, png_malloc_warn(png_ptr,
      length+1));

   if (png_ptr->chunkdata == NULL)
   {
      png_chunk_benign_error(png_ptr, "insufficient memory");
      return;
   }

   png_crc_read(png_ptr, (png_bytep)png_ptr->chunkdata, length);

   if (png_crc_finish(png_ptr, 0))
   {
      png_free(png_ptr, png_ptr->chunkdata);
      png_ptr->chunkdata = NULL;
      return;
   }

   /* First the keyword. */
   for (prefix_length=0;
      prefix_length < length && png_ptr->chunkdata[prefix_length] != 0;
      ++prefix_length)
      /* Empty loop */ ;

   /* Perform a basic check on the keyword length here. */
   if (prefix_length > 79 || prefix_length < 1)
      errmsg = "bad keyword";

   /* Expect keyword, compression flag, compression type, language, translated
    * keyword (both may be empty but are 0 terminated) then the text, which may
    * be empty.
    */
   else if (prefix_length + 5 > length)
      errmsg = "truncated";

   else if (png_ptr->chunkdata[prefix_length+1] == 0 ||
      (png_ptr->chunkdata[prefix_length+1] == 1 &&
      png_ptr->chunkdata[prefix_length+2] == PNG_COMPRESSION_TYPE_BASE))
   {
      int compressed = png_ptr->chunkdata[prefix_length+1] != 0;
      png_uint_32 language_offset, translated_keyword_offset;
      png_alloc_size_t uncompressed_length = 0;

      /* Now the language tag */
      prefix_length += 3;
      language_offset = prefix_length;

      for (; prefix_length < length && png_ptr->chunkdata[prefix_length] != 0;
         ++prefix_length)
         /* Empty loop */ ;

      /* WARNING: the length may be invalid here, this is checked below. */
      translated_keyword_offset = ++prefix_length;

      for (; prefix_length < length && png_ptr->chunkdata[prefix_length] != 0;
         ++prefix_length)
         /* Empty loop */ ;

      /* prefix_length should now be at the trailing '\0' of the translated
       * keyword, but it may already be over the end.  None of this arithmetic
       * can overflow because chunks are at most 2^31 bytes long, but on 16-bit
       * systems the available allocaton may overflow.
       */
      ++prefix_length;

      if (!compressed && prefix_length <= length)
         uncompressed_length = length - prefix_length;

      else if (compressed && prefix_length < length)
      {
         uncompressed_length = PNG_SIZE_MAX;

         /* TODO: at present png_decompress_chunk imposes a single application
          * level memory limit, this should be split to different values for
          * iCCP and text chunks.
          */
         errmsg = png_decompress_chunk(png_ptr, length, prefix_length,
            &uncompressed_length, 1/*terminate*/);
      }

      else
         errmsg = "truncated";

      if (errmsg == NULL)
      {
         png_text text;

         png_ptr->chunkdata[uncompressed_length+prefix_length] = 0;

         if (compressed)
            text.compression = PNG_ITXT_COMPRESSION_NONE;

         else
            text.compression = PNG_ITXT_COMPRESSION_zTXt;

         text.key = png_ptr->chunkdata;
         text.lang = png_ptr->chunkdata + language_offset;
         text.lang_key = png_ptr->chunkdata + translated_keyword_offset;
         text.text = png_ptr->chunkdata + prefix_length;
         text.text_length = 0;
         text.itxt_length = uncompressed_length;

         if (png_set_text_2(png_ptr, info_ptr, &text, 1))
            errmsg = "insufficient memory";
      }
   }

   else
      errmsg = "bad compression info";

   png_free(png_ptr, png_ptr->chunkdata);
   png_ptr->chunkdata = NULL;

   if (errmsg != NULL)
      png_chunk_benign_error(png_ptr, errmsg);
}
#endif

/* This function is called when we haven't found a handler for a
 * chunk.  If there isn't a problem with the chunk itself (ie bad
 * chunk name, CRC, or a critical chunk), the chunk is silently ignored
 * -- unless the PNG_FLAG_UNKNOWN_CHUNKS_SUPPORTED flag is on in which
 * case it will be saved away to be written out later.
 */
void /* PRIVATE */
png_handle_unknown(png_structrp png_ptr, png_inforp info_ptr, png_uint_32 length)
{
   png_uint_32 skip = 0;

   png_debug(1, "in png_handle_unknown");

#ifdef PNG_USER_LIMITS_SUPPORTED
   if (png_ptr->user_chunk_cache_max != 0)
   {
      if (png_ptr->user_chunk_cache_max == 1)
      {
         png_crc_finish(png_ptr, length);
         return;
      }

      if (--png_ptr->user_chunk_cache_max == 1)
      {
         png_warning(png_ptr, "No space in chunk cache for unknown chunk");
         png_crc_finish(png_ptr, length);
         return;
      }
   }
#endif

   if (png_ptr->mode & PNG_HAVE_IDAT)
   {
      if (png_ptr->chunk_name != png_IDAT)
         png_ptr->mode |= PNG_AFTER_IDAT;
   }

   if (PNG_CHUNK_CRITICAL(png_ptr->chunk_name))
   {
#ifdef PNG_HANDLE_AS_UNKNOWN_SUPPORTED
      if (png_chunk_unknown_handling(png_ptr, png_ptr->chunk_name) !=
          PNG_HANDLE_CHUNK_ALWAYS
#ifdef PNG_READ_USER_CHUNKS_SUPPORTED
          && png_ptr->read_user_chunk_fn == NULL
#endif
          )
#endif
         png_chunk_error(png_ptr, "unknown critical chunk");
   }

#ifdef PNG_READ_UNKNOWN_CHUNKS_SUPPORTED
   if ((png_ptr->flags & PNG_FLAG_KEEP_UNKNOWN_CHUNKS)
#ifdef PNG_READ_USER_CHUNKS_SUPPORTED
       || (png_ptr->read_user_chunk_fn != NULL)
#endif
       )
   {
#ifdef PNG_MAX_MALLOC_64K
      if (length > 65535)
      {
         png_warning(png_ptr, "unknown chunk too large to fit in memory");
         skip = length - 65535;
         length = 65535;
      }
#endif

      /* TODO: this code is very close to the unknown handling in pngpread.c,
       * maybe it can be put into a common utility routine?
       * png_struct::unknown_chunk is just used as a temporary variable, along
       * with the data into which the chunk is read.  These can be eliminated.
       */
      PNG_CSTRING_FROM_CHUNK(png_ptr->unknown_chunk.name, png_ptr->chunk_name);
      png_ptr->unknown_chunk.size = (png_size_t)length;

      if (length == 0)
         png_ptr->unknown_chunk.data = NULL;

      else
      {
         png_ptr->unknown_chunk.data = (png_bytep)png_malloc(png_ptr, length);
         png_crc_read(png_ptr, png_ptr->unknown_chunk.data, length);
      }

#ifdef PNG_READ_USER_CHUNKS_SUPPORTED
      if (png_ptr->read_user_chunk_fn != NULL)
      {
         /* Callback to user unknown chunk handler */
         int ret;

         ret = (*(png_ptr->read_user_chunk_fn))
             (png_ptr, &png_ptr->unknown_chunk);

         if (ret < 0)
            png_chunk_error(png_ptr, "error in user chunk");

         if (ret == 0)
         {
            if (PNG_CHUNK_CRITICAL(png_ptr->chunk_name))
            {
#ifdef PNG_HANDLE_AS_UNKNOWN_SUPPORTED
               if (png_chunk_unknown_handling(png_ptr, png_ptr->chunk_name) !=
                   PNG_HANDLE_CHUNK_ALWAYS)
#endif
                  png_chunk_error(png_ptr, "unknown critical chunk");
            }

            png_set_unknown_chunks(png_ptr, info_ptr,
                &png_ptr->unknown_chunk, 1);
         }
      }

      else
#endif
         png_set_unknown_chunks(png_ptr, info_ptr, &png_ptr->unknown_chunk, 1);

      png_free(png_ptr, png_ptr->unknown_chunk.data);
      png_ptr->unknown_chunk.data = NULL;
   }

   else
#endif
      skip = length;

   png_crc_finish(png_ptr, skip);

#ifndef PNG_READ_USER_CHUNKS_SUPPORTED
   PNG_UNUSED(info_ptr) /* Quiet compiler warnings about unused info_ptr */
#endif
}

/* This function is called to verify that a chunk name is valid.
 * This function can't have the "critical chunk check" incorporated
 * into it, since in the future we will need to be able to call user
 * functions to handle unknown critical chunks after we check that
 * the chunk name itself is valid.
 */

/* Bit hacking: the test for an invalid byte in the 4 byte chunk name is:
 *
 * ((c) < 65 || (c) > 122 || ((c) > 90 && (c) < 97))
 */

void /* PRIVATE */
png_check_chunk_name(png_structrp png_ptr, png_uint_32 chunk_name)
{
   int i;

   png_debug(1, "in png_check_chunk_name");

   for (i=1; i<=4; ++i)
   {
      int c = chunk_name & 0xff;

      if (c < 65 || c > 122 || (c > 90 && c < 97))
         png_chunk_error(png_ptr, "invalid chunk type");

      chunk_name >>= 8;
   }
}

/* Combines the row recently read in with the existing pixels in the row.  This
 * routine takes care of alpha and transparency if requested.  This routine also
 * handles the two methods of progressive display of interlaced images,
 * depending on the 'display' value; if 'display' is true then the whole row
 * (dp) is filled from the start by replicating the available pixels.  If
 * 'display' is false only those pixels present in the pass are filled in.
 */
void /* PRIVATE */
png_combine_row(png_const_structrp png_ptr, png_bytep dp, int display)
{
   unsigned int pixel_depth = png_ptr->transformed_pixel_depth;
   png_const_bytep sp = png_ptr->row_buf + 1;
   png_uint_32 row_width = png_ptr->width;
   unsigned int pass = png_ptr->pass;
   png_bytep end_ptr = 0;
   png_byte end_byte = 0;
   unsigned int end_mask;

   png_debug(1, "in png_combine_row");

   /* Added in 1.5.6: it should not be possible to enter this routine until at
    * least one row has been read from the PNG data and transformed.
    */
   if (pixel_depth == 0)
      png_error(png_ptr, "internal row logic error");

   /* Added in 1.5.4: the pixel depth should match the information returned by
    * any call to png_read_update_info at this point.  Do not continue if we got
    * this wrong.
    */
   if (png_ptr->info_rowbytes != 0 && png_ptr->info_rowbytes !=
          PNG_ROWBYTES(pixel_depth, row_width))
      png_error(png_ptr, "internal row size calculation error");

   /* Don't expect this to ever happen: */
   if (row_width == 0)
      png_error(png_ptr, "internal row width error");

   /* Preserve the last byte in cases where only part of it will be overwritten,
    * the multiply below may overflow, we don't care because ANSI-C guarantees
    * we get the low bits.
    */
   end_mask = (pixel_depth * row_width) & 7;
   if (end_mask != 0)
   {
      /* end_ptr == NULL is a flag to say do nothing */
      end_ptr = dp + PNG_ROWBYTES(pixel_depth, row_width) - 1;
      end_byte = *end_ptr;
#     ifdef PNG_READ_PACKSWAP_SUPPORTED
         if (png_ptr->transformations & PNG_PACKSWAP) /* little-endian byte */
            end_mask = 0xff << end_mask;

         else /* big-endian byte */
#     endif
         end_mask = 0xff >> end_mask;
      /* end_mask is now the bits to *keep* from the destination row */
   }

   /* For non-interlaced images this reduces to a png_memcpy(). A png_memcpy()
    * will also happen if interlacing isn't supported or if the application
    * does not call png_set_interlace_handling().  In the latter cases the
    * caller just gets a sequence of the unexpanded rows from each interlace
    * pass.
    */
#ifdef PNG_READ_INTERLACING_SUPPORTED
   if (png_ptr->interlaced && (png_ptr->transformations & PNG_INTERLACE) &&
      pass < 6 && (display == 0 ||
      /* The following copies everything for 'display' on passes 0, 2 and 4. */
      (display == 1 && (pass & 1) != 0)))
   {
      /* Narrow images may have no bits in a pass; the caller should handle
       * this, but this test is cheap:
       */
      if (row_width <= PNG_PASS_START_COL(pass))
         return;

      if (pixel_depth < 8)
      {
         /* For pixel depths up to 4 bpp the 8-pixel mask can be expanded to fit
          * into 32 bits, then a single loop over the bytes using the four byte
          * values in the 32-bit mask can be used.  For the 'display' option the
          * expanded mask may also not require any masking within a byte.  To
          * make this work the PACKSWAP option must be taken into account - it
          * simply requires the pixels to be reversed in each byte.
          *
          * The 'regular' case requires a mask for each of the first 6 passes,
          * the 'display' case does a copy for the even passes in the range
          * 0..6.  This has already been handled in the test above.
          *
          * The masks are arranged as four bytes with the first byte to use in
          * the lowest bits (little-endian) regardless of the order (PACKSWAP or
          * not) of the pixels in each byte.
          *
          * NOTE: the whole of this logic depends on the caller of this function
          * only calling it on rows appropriate to the pass.  This function only
          * understands the 'x' logic; the 'y' logic is handled by the caller.
          *
          * The following defines allow generation of compile time constant bit
          * masks for each pixel depth and each possibility of swapped or not
          * swapped bytes.  Pass 'p' is in the range 0..6; 'x', a pixel index,
          * is in the range 0..7; and the result is 1 if the pixel is to be
          * copied in the pass, 0 if not.  'S' is for the sparkle method, 'B'
          * for the block method.
          *
          * With some compilers a compile time expression of the general form:
          *
          *    (shift >= 32) ? (a >> (shift-32)) : (b >> shift)
          *
          * Produces warnings with values of 'shift' in the range 33 to 63
          * because the right hand side of the ?: expression is evaluated by
          * the compiler even though it isn't used.  Microsoft Visual C (various
          * versions) and the Intel C compiler are known to do this.  To avoid
          * this the following macros are used in 1.5.6.  This is a temporary
          * solution to avoid destabilizing the code during the release process.
          */
#        if PNG_USE_COMPILE_TIME_MASKS
#           define PNG_LSR(x,s) ((x)>>((s) & 0x1f))
#           define PNG_LSL(x,s) ((x)<<((s) & 0x1f))
#        else
#           define PNG_LSR(x,s) ((x)>>(s))
#           define PNG_LSL(x,s) ((x)<<(s))
#        endif
#        define S_COPY(p,x) (((p)<4 ? PNG_LSR(0x80088822,(3-(p))*8+(7-(x))) :\
           PNG_LSR(0xaa55ff00,(7-(p))*8+(7-(x)))) & 1)
#        define B_COPY(p,x) (((p)<4 ? PNG_LSR(0xff0fff33,(3-(p))*8+(7-(x))) :\
           PNG_LSR(0xff55ff00,(7-(p))*8+(7-(x)))) & 1)

         /* Return a mask for pass 'p' pixel 'x' at depth 'd'.  The mask is
          * little endian - the first pixel is at bit 0 - however the extra
          * parameter 's' can be set to cause the mask position to be swapped
          * within each byte, to match the PNG format.  This is done by XOR of
          * the shift with 7, 6 or 4 for bit depths 1, 2 and 4.
          */
#        define PIXEL_MASK(p,x,d,s) \
            (PNG_LSL(((PNG_LSL(1U,(d)))-1),(((x)*(d))^((s)?8-(d):0))))

         /* Hence generate the appropriate 'block' or 'sparkle' pixel copy mask.
          */
#        define S_MASKx(p,x,d,s) (S_COPY(p,x)?PIXEL_MASK(p,x,d,s):0)
#        define B_MASKx(p,x,d,s) (B_COPY(p,x)?PIXEL_MASK(p,x,d,s):0)

         /* Combine 8 of these to get the full mask.  For the 1-bpp and 2-bpp
          * cases the result needs replicating, for the 4-bpp case the above
          * generates a full 32 bits.
          */
#        define MASK_EXPAND(m,d) ((m)*((d)==1?0x01010101:((d)==2?0x00010001:1)))

#        define S_MASK(p,d,s) MASK_EXPAND(S_MASKx(p,0,d,s) + S_MASKx(p,1,d,s) +\
            S_MASKx(p,2,d,s) + S_MASKx(p,3,d,s) + S_MASKx(p,4,d,s) +\
            S_MASKx(p,5,d,s) + S_MASKx(p,6,d,s) + S_MASKx(p,7,d,s), d)

#        define B_MASK(p,d,s) MASK_EXPAND(B_MASKx(p,0,d,s) + B_MASKx(p,1,d,s) +\
            B_MASKx(p,2,d,s) + B_MASKx(p,3,d,s) + B_MASKx(p,4,d,s) +\
            B_MASKx(p,5,d,s) + B_MASKx(p,6,d,s) + B_MASKx(p,7,d,s), d)

#if PNG_USE_COMPILE_TIME_MASKS
         /* Utility macros to construct all the masks for a depth/swap
          * combination.  The 's' parameter says whether the format is PNG
          * (big endian bytes) or not.  Only the three odd-numbered passes are
          * required for the display/block algorithm.
          */
#        define S_MASKS(d,s) { S_MASK(0,d,s), S_MASK(1,d,s), S_MASK(2,d,s),\
            S_MASK(3,d,s), S_MASK(4,d,s), S_MASK(5,d,s) }

#        define B_MASKS(d,s) { B_MASK(1,d,s), S_MASK(3,d,s), S_MASK(5,d,s) }

#        define DEPTH_INDEX(d) ((d)==1?0:((d)==2?1:2))

         /* Hence the pre-compiled masks indexed by PACKSWAP (or not), depth and
          * then pass:
          */
         static PNG_CONST png_uint_32 row_mask[2/*PACKSWAP*/][3/*depth*/][6] =
         {
            /* Little-endian byte masks for PACKSWAP */
            { S_MASKS(1,0), S_MASKS(2,0), S_MASKS(4,0) },
            /* Normal (big-endian byte) masks - PNG format */
            { S_MASKS(1,1), S_MASKS(2,1), S_MASKS(4,1) }
         };

         /* display_mask has only three entries for the odd passes, so index by
          * pass>>1.
          */
         static PNG_CONST png_uint_32 display_mask[2][3][3] =
         {
            /* Little-endian byte masks for PACKSWAP */
            { B_MASKS(1,0), B_MASKS(2,0), B_MASKS(4,0) },
            /* Normal (big-endian byte) masks - PNG format */
            { B_MASKS(1,1), B_MASKS(2,1), B_MASKS(4,1) }
         };

#        define MASK(pass,depth,display,png)\
            ((display)?display_mask[png][DEPTH_INDEX(depth)][pass>>1]:\
               row_mask[png][DEPTH_INDEX(depth)][pass])

#else /* !PNG_USE_COMPILE_TIME_MASKS */
         /* This is the runtime alternative: it seems unlikely that this will
          * ever be either smaller or faster than the compile time approach.
          */
#        define MASK(pass,depth,display,png)\
            ((display)?B_MASK(pass,depth,png):S_MASK(pass,depth,png))
#endif /* !PNG_USE_COMPILE_TIME_MASKS */

         /* Use the appropriate mask to copy the required bits.  In some cases
          * the byte mask will be 0 or 0xff, optimize these cases.  row_width is
          * the number of pixels, but the code copies bytes, so it is necessary
          * to special case the end.
          */
         png_uint_32 pixels_per_byte = 8 / pixel_depth;
         png_uint_32 mask;

#        ifdef PNG_READ_PACKSWAP_SUPPORTED
            if (png_ptr->transformations & PNG_PACKSWAP)
               mask = MASK(pass, pixel_depth, display, 0);

            else
#        endif
            mask = MASK(pass, pixel_depth, display, 1);

         for (;;)
         {
            png_uint_32 m;

            /* It doesn't matter in the following if png_uint_32 has more than
             * 32 bits because the high bits always match those in m<<24; it is,
             * however, essential to use OR here, not +, because of this.
             */
            m = mask;
            mask = (m >> 8) | (m << 24); /* rotate right to good compilers */
            m &= 0xff;

            if (m != 0) /* something to copy */
            {
               if (m != 0xff)
                  *dp = (png_byte)((*dp & ~m) | (*sp & m));
               else
                  *dp = *sp;
            }

            /* NOTE: this may overwrite the last byte with garbage if the image
             * is not an exact number of bytes wide; libpng has always done
             * this.
             */
            if (row_width <= pixels_per_byte)
               break; /* May need to restore part of the last byte */

            row_width -= pixels_per_byte;
            ++dp;
            ++sp;
         }
      }

      else /* pixel_depth >= 8 */
      {
         unsigned int bytes_to_copy, bytes_to_jump;

         /* Validate the depth - it must be a multiple of 8 */
         if (pixel_depth & 7)
            png_error(png_ptr, "invalid user transform pixel depth");

         pixel_depth >>= 3; /* now in bytes */
         row_width *= pixel_depth;

         /* Regardless of pass number the Adam 7 interlace always results in a
          * fixed number of pixels to copy then to skip.  There may be a
          * different number of pixels to skip at the start though.
          */
         {
            unsigned int offset = PNG_PASS_START_COL(pass) * pixel_depth;

            row_width -= offset;
            dp += offset;
            sp += offset;
         }

         /* Work out the bytes to copy. */
         if (display)
         {
            /* When doing the 'block' algorithm the pixel in the pass gets
             * replicated to adjacent pixels.  This is why the even (0,2,4,6)
             * passes are skipped above - the entire expanded row is copied.
             */
            bytes_to_copy = (1<<((6-pass)>>1)) * pixel_depth;

            /* But don't allow this number to exceed the actual row width. */
            if (bytes_to_copy > row_width)
               bytes_to_copy = row_width;
         }

         else /* normal row; Adam7 only ever gives us one pixel to copy. */
            bytes_to_copy = pixel_depth;

         /* In Adam7 there is a constant offset between where the pixels go. */
         bytes_to_jump = PNG_PASS_COL_OFFSET(pass) * pixel_depth;

         /* And simply copy these bytes.  Some optimization is possible here,
          * depending on the value of 'bytes_to_copy'.  Special case the low
          * byte counts, which we know to be frequent.
          *
          * Notice that these cases all 'return' rather than 'break' - this
          * avoids an unnecessary test on whether to restore the last byte
          * below.
          */
         switch (bytes_to_copy)
         {
            case 1:
               for (;;)
               {
                  *dp = *sp;

                  if (row_width <= bytes_to_jump)
                     return;

                  dp += bytes_to_jump;
                  sp += bytes_to_jump;
                  row_width -= bytes_to_jump;
               }

            case 2:
               /* There is a possibility of a partial copy at the end here; this
                * slows the code down somewhat.
                */
               do
               {
                  dp[0] = sp[0], dp[1] = sp[1];

                  if (row_width <= bytes_to_jump)
                     return;

                  sp += bytes_to_jump;
                  dp += bytes_to_jump;
                  row_width -= bytes_to_jump;
               }
               while (row_width > 1);

               /* And there can only be one byte left at this point: */
               *dp = *sp;
               return;

            case 3:
               /* This can only be the RGB case, so each copy is exactly one
                * pixel and it is not necessary to check for a partial copy.
                */
               for(;;)
               {
                  dp[0] = sp[0], dp[1] = sp[1], dp[2] = sp[2];

                  if (row_width <= bytes_to_jump)
                     return;

                  sp += bytes_to_jump;
                  dp += bytes_to_jump;
                  row_width -= bytes_to_jump;
               }

            default:
#if PNG_ALIGN_TYPE != PNG_ALIGN_NONE
               /* Check for double byte alignment and, if possible, use a
                * 16-bit copy.  Don't attempt this for narrow images - ones that
                * are less than an interlace panel wide.  Don't attempt it for
                * wide bytes_to_copy either - use the png_memcpy there.
                */
               if (bytes_to_copy < 16 /*else use png_memcpy*/ &&
                  png_isaligned(dp, png_uint_16) &&
                  png_isaligned(sp, png_uint_16) &&
                  bytes_to_copy % sizeof (png_uint_16) == 0 &&
                  bytes_to_jump % sizeof (png_uint_16) == 0)
               {
                  /* Everything is aligned for png_uint_16 copies, but try for
                   * png_uint_32 first.
                   */
                  if (png_isaligned(dp, png_uint_32) &&
                     png_isaligned(sp, png_uint_32) &&
                     bytes_to_copy % sizeof (png_uint_32) == 0 &&
                     bytes_to_jump % sizeof (png_uint_32) == 0)
                  {
                     png_uint_32p dp32 = (png_uint_32p)dp;
                     png_const_uint_32p sp32 = (png_const_uint_32p)sp;
                     unsigned int skip = (bytes_to_jump-bytes_to_copy) /
                        sizeof (png_uint_32);

                     do
                     {
                        size_t c = bytes_to_copy;
                        do
                        {
                           *dp32++ = *sp32++;
                           c -= sizeof (png_uint_32);
                        }
                        while (c > 0);

                        if (row_width <= bytes_to_jump)
                           return;

                        dp32 += skip;
                        sp32 += skip;
                        row_width -= bytes_to_jump;
                     }
                     while (bytes_to_copy <= row_width);

                     /* Get to here when the row_width truncates the final copy.
                      * There will be 1-3 bytes left to copy, so don't try the
                      * 16-bit loop below.
                      */
                     dp = (png_bytep)dp32;
                     sp = (png_const_bytep)sp32;
                     do
                        *dp++ = *sp++;
                     while (--row_width > 0);
                     return;
                  }

                  /* Else do it in 16-bit quantities, but only if the size is
                   * not too large.
                   */
                  else
                  {
                     png_uint_16p dp16 = (png_uint_16p)dp;
                     png_const_uint_16p sp16 = (png_const_uint_16p)sp;
                     unsigned int skip = (bytes_to_jump-bytes_to_copy) /
                        sizeof (png_uint_16);

                     do
                     {
                        size_t c = bytes_to_copy;
                        do
                        {
                           *dp16++ = *sp16++;
                           c -= sizeof (png_uint_16);
                        }
                        while (c > 0);

                        if (row_width <= bytes_to_jump)
                           return;

                        dp16 += skip;
                        sp16 += skip;
                        row_width -= bytes_to_jump;
                     }
                     while (bytes_to_copy <= row_width);

                     /* End of row - 1 byte left, bytes_to_copy > row_width: */
                     dp = (png_bytep)dp16;
                     sp = (png_const_bytep)sp16;
                     do
                        *dp++ = *sp++;
                     while (--row_width > 0);
                     return;
                  }
               }
#endif /* PNG_ALIGN_ code */

               /* The true default - use a png_memcpy: */
               for (;;)
               {
                  png_memcpy(dp, sp, bytes_to_copy);

                  if (row_width <= bytes_to_jump)
                     return;

                  sp += bytes_to_jump;
                  dp += bytes_to_jump;
                  row_width -= bytes_to_jump;
                  if (bytes_to_copy > row_width)
                     bytes_to_copy = row_width;
               }
         }

         /* NOT REACHED*/
      } /* pixel_depth >= 8 */

      /* Here if pixel_depth < 8 to check 'end_ptr' below. */
   }
   else
#endif

   /* If here then the switch above wasn't used so just png_memcpy the whole row
    * from the temporary row buffer (notice that this overwrites the end of the
    * destination row if it is a partial byte.)
    */
   png_memcpy(dp, sp, PNG_ROWBYTES(pixel_depth, row_width));

   /* Restore the overwritten bits from the last byte if necessary. */
   if (end_ptr != NULL)
      *end_ptr = (png_byte)((end_byte & end_mask) | (*end_ptr & ~end_mask));
}

#ifdef PNG_READ_INTERLACING_SUPPORTED
void /* PRIVATE */
png_do_read_interlace(png_row_infop row_info, png_bytep row, int pass,
   png_uint_32 transformations /* Because these may affect the byte layout */)
{
   /* Arrays to facilitate easy interlacing - use pass (0 - 6) as index */
   /* Offset to next interlace block */
   static PNG_CONST int png_pass_inc[7] = {8, 8, 4, 4, 2, 2, 1};

   png_debug(1, "in png_do_read_interlace");
   if (row != NULL && row_info != NULL)
   {
      png_uint_32 final_width;

      final_width = row_info->width * png_pass_inc[pass];

      switch (row_info->pixel_depth)
      {
         case 1:
         {
            png_bytep sp = row + (png_size_t)((row_info->width - 1) >> 3);
            png_bytep dp = row + (png_size_t)((final_width - 1) >> 3);
            int sshift, dshift;
            int s_start, s_end, s_inc;
            int jstop = png_pass_inc[pass];
            png_byte v;
            png_uint_32 i;
            int j;

#ifdef PNG_READ_PACKSWAP_SUPPORTED
            if (transformations & PNG_PACKSWAP)
            {
                sshift = (int)((row_info->width + 7) & 0x07);
                dshift = (int)((final_width + 7) & 0x07);
                s_start = 7;
                s_end = 0;
                s_inc = -1;
            }

            else
#endif
            {
                sshift = 7 - (int)((row_info->width + 7) & 0x07);
                dshift = 7 - (int)((final_width + 7) & 0x07);
                s_start = 0;
                s_end = 7;
                s_inc = 1;
            }

            for (i = 0; i < row_info->width; i++)
            {
               v = (png_byte)((*sp >> sshift) & 0x01);
               for (j = 0; j < jstop; j++)
               {
                  unsigned int tmp = *dp & (0x7f7f >> (7 - dshift));
                  tmp |= v << dshift;
                  *dp = (png_byte)(tmp & 0xff);

                  if (dshift == s_end)
                  {
                     dshift = s_start;
                     dp--;
                  }

                  else
                     dshift += s_inc;
               }

               if (sshift == s_end)
               {
                  sshift = s_start;
                  sp--;
               }

               else
                  sshift += s_inc;
            }
            break;
         }

         case 2:
         {
            png_bytep sp = row + (png_uint_32)((row_info->width - 1) >> 2);
            png_bytep dp = row + (png_uint_32)((final_width - 1) >> 2);
            int sshift, dshift;
            int s_start, s_end, s_inc;
            int jstop = png_pass_inc[pass];
            png_uint_32 i;

#ifdef PNG_READ_PACKSWAP_SUPPORTED
            if (transformations & PNG_PACKSWAP)
            {
               sshift = (int)(((row_info->width + 3) & 0x03) << 1);
               dshift = (int)(((final_width + 3) & 0x03) << 1);
               s_start = 6;
               s_end = 0;
               s_inc = -2;
            }

            else
#endif
            {
               sshift = (int)((3 - ((row_info->width + 3) & 0x03)) << 1);
               dshift = (int)((3 - ((final_width + 3) & 0x03)) << 1);
               s_start = 0;
               s_end = 6;
               s_inc = 2;
            }

            for (i = 0; i < row_info->width; i++)
            {
               png_byte v;
               int j;

               v = (png_byte)((*sp >> sshift) & 0x03);
               for (j = 0; j < jstop; j++)
               {
                  unsigned int tmp = *dp & (0x3f3f >> (6 - dshift));
                  tmp |= v << dshift;
                  *dp = (png_byte)(tmp & 0xff);

                  if (dshift == s_end)
                  {
                     dshift = s_start;
                     dp--;
                  }

                  else
                     dshift += s_inc;
               }

               if (sshift == s_end)
               {
                  sshift = s_start;
                  sp--;
               }

               else
                  sshift += s_inc;
            }
            break;
         }

         case 4:
         {
            png_bytep sp = row + (png_size_t)((row_info->width - 1) >> 1);
            png_bytep dp = row + (png_size_t)((final_width - 1) >> 1);
            int sshift, dshift;
            int s_start, s_end, s_inc;
            png_uint_32 i;
            int jstop = png_pass_inc[pass];

#ifdef PNG_READ_PACKSWAP_SUPPORTED
            if (transformations & PNG_PACKSWAP)
            {
               sshift = (int)(((row_info->width + 1) & 0x01) << 2);
               dshift = (int)(((final_width + 1) & 0x01) << 2);
               s_start = 4;
               s_end = 0;
               s_inc = -4;
            }

            else
#endif
            {
               sshift = (int)((1 - ((row_info->width + 1) & 0x01)) << 2);
               dshift = (int)((1 - ((final_width + 1) & 0x01)) << 2);
               s_start = 0;
               s_end = 4;
               s_inc = 4;
            }

            for (i = 0; i < row_info->width; i++)
            {
               png_byte v = (png_byte)((*sp >> sshift) & 0x0f);
               int j;

               for (j = 0; j < jstop; j++)
               {
                  unsigned int tmp = *dp & (0xf0f >> (4 - dshift));
                  tmp |= v << dshift;
                  *dp = (png_byte)(tmp & 0xff);

                  if (dshift == s_end)
                  {
                     dshift = s_start;
                     dp--;
                  }

                  else
                     dshift += s_inc;
               }

               if (sshift == s_end)
               {
                  sshift = s_start;
                  sp--;
               }

               else
                  sshift += s_inc;
            }
            break;
         }

         default:
         {
            png_size_t pixel_bytes = (row_info->pixel_depth >> 3);

            png_bytep sp = row + (png_size_t)(row_info->width - 1)
                * pixel_bytes;

            png_bytep dp = row + (png_size_t)(final_width - 1) * pixel_bytes;

            int jstop = png_pass_inc[pass];
            png_uint_32 i;

            for (i = 0; i < row_info->width; i++)
            {
               png_byte v[8];
               int j;

               png_memcpy(v, sp, pixel_bytes);

               for (j = 0; j < jstop; j++)
               {
                  png_memcpy(dp, v, pixel_bytes);
                  dp -= pixel_bytes;
               }

               sp -= pixel_bytes;
            }
            break;
         }
      }

      row_info->width = final_width;
      row_info->rowbytes = PNG_ROWBYTES(row_info->pixel_depth, final_width);
   }
#ifndef PNG_READ_PACKSWAP_SUPPORTED
   PNG_UNUSED(transformations)  /* Silence compiler warning */
#endif
}
#endif /* PNG_READ_INTERLACING_SUPPORTED */

static void
png_read_filter_row_sub(png_row_infop row_info, png_bytep row,
   png_const_bytep prev_row)
{
   png_size_t i;
   png_size_t istop = row_info->rowbytes;
   unsigned int bpp = (row_info->pixel_depth + 7) >> 3;
   png_bytep rp = row + bpp;

   PNG_UNUSED(prev_row)

   for (i = bpp; i < istop; i++)
   {
      *rp = (png_byte)(((int)(*rp) + (int)(*(rp-bpp))) & 0xff);
      rp++;
   }
}

static void
png_read_filter_row_up(png_row_infop row_info, png_bytep row,
   png_const_bytep prev_row)
{
   png_size_t i;
   png_size_t istop = row_info->rowbytes;
   png_bytep rp = row;
   png_const_bytep pp = prev_row;

   for (i = 0; i < istop; i++)
   {
      *rp = (png_byte)(((int)(*rp) + (int)(*pp++)) & 0xff);
      rp++;
   }
}

static void
png_read_filter_row_avg(png_row_infop row_info, png_bytep row,
   png_const_bytep prev_row)
{
   png_size_t i;
   png_bytep rp = row;
   png_const_bytep pp = prev_row;
   unsigned int bpp = (row_info->pixel_depth + 7) >> 3;
   png_size_t istop = row_info->rowbytes - bpp;

   for (i = 0; i < bpp; i++)
   {
      *rp = (png_byte)(((int)(*rp) +
         ((int)(*pp++) / 2 )) & 0xff);

      rp++;
   }

   for (i = 0; i < istop; i++)
   {
      *rp = (png_byte)(((int)(*rp) +
         (int)(*pp++ + *(rp-bpp)) / 2 ) & 0xff);

      rp++;
   }
}

static void
png_read_filter_row_paeth_1byte_pixel(png_row_infop row_info, png_bytep row,
   png_const_bytep prev_row)
{
   png_bytep rp_end = row + row_info->rowbytes;
   int a, c;

   /* First pixel/byte */
   c = *prev_row++;
   a = *row + c;
   *row++ = (png_byte)a;

   /* Remainder */
   while (row < rp_end)
   {
      int b, pa, pb, pc, p;

      a &= 0xff; /* From previous iteration or start */
      b = *prev_row++;

      p = b - c;
      pc = a - c;

#     ifdef PNG_USE_ABS
         pa = abs(p);
         pb = abs(pc);
         pc = abs(p + pc);
#     else
         pa = p < 0 ? -p : p;
         pb = pc < 0 ? -pc : pc;
         pc = (p + pc) < 0 ? -(p + pc) : p + pc;
#     endif

      /* Find the best predictor, the least of pa, pb, pc favoring the earlier
       * ones in the case of a tie.
       */
      if (pb < pa) pa = pb, a = b;
      if (pc < pa) a = c;

      /* Calculate the current pixel in a, and move the previous row pixel to c
       * for the next time round the loop
       */
      c = b;
      a += *row;
      *row++ = (png_byte)a;
   }
}

static void
png_read_filter_row_paeth_multibyte_pixel(png_row_infop row_info, png_bytep row,
   png_const_bytep prev_row)
{
   int bpp = (row_info->pixel_depth + 7) >> 3;
   png_bytep rp_end = row + bpp;

   /* Process the first pixel in the row completely (this is the same as 'up'
    * because there is only one candidate predictor for the first row).
    */
   while (row < rp_end)
   {
      int a = *row + *prev_row++;
      *row++ = (png_byte)a;
   }

   /* Remainder */
   rp_end += row_info->rowbytes - bpp;

   while (row < rp_end)
   {
      int a, b, c, pa, pb, pc, p;

      c = *(prev_row - bpp);
      a = *(row - bpp);
      b = *prev_row++;

      p = b - c;
      pc = a - c;

#     ifdef PNG_USE_ABS
         pa = abs(p);
         pb = abs(pc);
         pc = abs(p + pc);
#     else
         pa = p < 0 ? -p : p;
         pb = pc < 0 ? -pc : pc;
         pc = (p + pc) < 0 ? -(p + pc) : p + pc;
#     endif

      if (pb < pa) pa = pb, a = b;
      if (pc < pa) a = c;

      c = b;
      a += *row;
      *row++ = (png_byte)a;
   }
}

#ifdef PNG_ARM_NEON

#ifdef __linux__
#include <stdio.h>
#include <elf.h>
#include <asm/hwcap.h>

static int png_have_hwcap(unsigned cap)
{
   FILE *f = fopen("/proc/self/auxv", "r");
   Elf32_auxv_t aux;
   int have_cap = 0;

   if (!f)
      return 0;

   while (fread(&aux, sizeof(aux), 1, f) > 0)
   {
      if (aux.a_type == AT_HWCAP &&
          aux.a_un.a_val & cap)
      {
         have_cap = 1;
         break;
      }
   }

   fclose(f);

   return have_cap;
}
#endif /* __linux__ */

static void
png_init_filter_functions_neon(png_structrp pp, unsigned int bpp)
{
#ifdef __linux__
   if (!png_have_hwcap(HWCAP_NEON))
      return;
#endif

   pp->read_filter[PNG_FILTER_VALUE_UP-1] = png_read_filter_row_up_neon;

   if (bpp == 3)
   {
      pp->read_filter[PNG_FILTER_VALUE_SUB-1] = png_read_filter_row_sub3_neon;
      pp->read_filter[PNG_FILTER_VALUE_AVG-1] = png_read_filter_row_avg3_neon;
      pp->read_filter[PNG_FILTER_VALUE_PAETH-1] =
         png_read_filter_row_paeth3_neon;
   }

   else if (bpp == 4)
   {
      pp->read_filter[PNG_FILTER_VALUE_SUB-1] = png_read_filter_row_sub4_neon;
      pp->read_filter[PNG_FILTER_VALUE_AVG-1] = png_read_filter_row_avg4_neon;
      pp->read_filter[PNG_FILTER_VALUE_PAETH-1] =
          png_read_filter_row_paeth4_neon;
   }
}
#endif /* PNG_ARM_NEON */

static void
png_init_filter_functions(png_structrp pp)
{
   unsigned int bpp = (pp->pixel_depth + 7) >> 3;

   pp->read_filter[PNG_FILTER_VALUE_SUB-1] = png_read_filter_row_sub;
   pp->read_filter[PNG_FILTER_VALUE_UP-1] = png_read_filter_row_up;
   pp->read_filter[PNG_FILTER_VALUE_AVG-1] = png_read_filter_row_avg;
   if (bpp == 1)
      pp->read_filter[PNG_FILTER_VALUE_PAETH-1] =
         png_read_filter_row_paeth_1byte_pixel;
   else
      pp->read_filter[PNG_FILTER_VALUE_PAETH-1] =
         png_read_filter_row_paeth_multibyte_pixel;

#ifdef PNG_ARM_NEON
   png_init_filter_functions_neon(pp, bpp);
#endif
}

void /* PRIVATE */
png_read_filter_row(png_structrp pp, png_row_infop row_info, png_bytep row,
   png_const_bytep prev_row, int filter)
{
   if (pp->read_filter[0] == NULL)
      png_init_filter_functions(pp);
   if (filter > PNG_FILTER_VALUE_NONE && filter < PNG_FILTER_VALUE_LAST)
      pp->read_filter[filter-1](row_info, row, prev_row);
}

#ifdef PNG_SEQUENTIAL_READ_SUPPORTED
void /* PRIVATE */
png_read_finish_row(png_structrp png_ptr)
{
#ifdef PNG_READ_INTERLACING_SUPPORTED
   /* Arrays to facilitate easy interlacing - use pass (0 - 6) as index */

   /* Start of interlace block */
   static PNG_CONST png_byte png_pass_start[7] = {0, 4, 0, 2, 0, 1, 0};

   /* Offset to next interlace block */
   static PNG_CONST png_byte png_pass_inc[7] = {8, 8, 4, 4, 2, 2, 1};

   /* Start of interlace block in the y direction */
   static PNG_CONST png_byte png_pass_ystart[7] = {0, 0, 4, 0, 2, 0, 1};

   /* Offset to next interlace block in the y direction */
   static PNG_CONST png_byte png_pass_yinc[7] = {8, 8, 8, 4, 4, 2, 2};
#endif /* PNG_READ_INTERLACING_SUPPORTED */

   png_debug(1, "in png_read_finish_row");
   png_ptr->row_number++;
   if (png_ptr->row_number < png_ptr->num_rows)
      return;

#ifdef PNG_READ_INTERLACING_SUPPORTED
   if (png_ptr->interlaced)
   {
      png_ptr->row_number = 0;

      /* TO DO: don't do this if prev_row isn't needed (requires
       * read-ahead of the next row's filter byte.
       */
      png_memset(png_ptr->prev_row, 0, png_ptr->rowbytes + 1);

      do
      {
         png_ptr->pass++;

         if (png_ptr->pass >= 7)
            break;

         png_ptr->iwidth = (png_ptr->width +
            png_pass_inc[png_ptr->pass] - 1 -
            png_pass_start[png_ptr->pass]) /
            png_pass_inc[png_ptr->pass];

         if (!(png_ptr->transformations & PNG_INTERLACE))
         {
            png_ptr->num_rows = (png_ptr->height +
                png_pass_yinc[png_ptr->pass] - 1 -
                png_pass_ystart[png_ptr->pass]) /
                png_pass_yinc[png_ptr->pass];
         }

         else  /* if (png_ptr->transformations & PNG_INTERLACE) */
            break; /* libpng deinterlacing sees every row */

      } while (png_ptr->num_rows == 0 || png_ptr->iwidth == 0);

      if (png_ptr->pass < 7)
         return;
   }
#endif /* PNG_READ_INTERLACING_SUPPORTED */

   /* Here after at the end of the last row of the last pass. */
   if (!(png_ptr->flags & PNG_FLAG_ZSTREAM_ENDED))
   {
      Byte extra;
      int ret;

      png_ptr->zstream.next_out = &extra;
      png_ptr->zstream.avail_out = 1;

      for (;;)
      {
         if (!(png_ptr->zstream.avail_in))
         {
            while (png_ptr->idat_size == 0)
            {
               png_crc_finish(png_ptr, 0);
               png_ptr->idat_size = png_read_chunk_header(png_ptr);
               if (png_ptr->chunk_name != png_IDAT)
                  png_error(png_ptr, "Not enough image data");
            }

            png_ptr->zstream.avail_in = png_ptr->zbuf_size;
            png_ptr->zstream.next_in = png_ptr->zbuf;

            if (png_ptr->zbuf_size > png_ptr->idat_size)
               png_ptr->zstream.avail_in = png_ptr->idat_size;

            png_crc_read(png_ptr, png_ptr->zbuf, png_ptr->zstream.avail_in);
            png_ptr->idat_size -= png_ptr->zstream.avail_in;
         }

         ret = inflate(&png_ptr->zstream, Z_NO_FLUSH);

         if (ret == Z_STREAM_END)
         {
            if (!(png_ptr->zstream.avail_out) || png_ptr->zstream.avail_in ||
                png_ptr->idat_size)
               png_warning(png_ptr, "Extra compressed data");

            png_ptr->flags |= PNG_FLAG_ZSTREAM_ENDED;
            break;
         }

         if (ret != Z_OK)
            png_error(png_ptr, png_ptr->zstream.msg ? png_ptr->zstream.msg :
                "Decompression Error");

         if (!(png_ptr->zstream.avail_out))
         {
            png_warning(png_ptr, "Extra compressed data");
            break;
         }
      }
      png_ptr->zstream.avail_out = 0;
   }

   if (png_ptr->idat_size || png_ptr->zstream.avail_in)
      png_warning(png_ptr, "Extra compression data");

   png_ptr->flags &= ~PNG_FLAG_ZSTREAM_IN_USE;
   png_ptr->mode |= PNG_AFTER_IDAT;
}
#endif /* PNG_SEQUENTIAL_READ_SUPPORTED */

void /* PRIVATE */
png_read_start_row(png_structrp png_ptr)
{
#ifdef PNG_READ_INTERLACING_SUPPORTED
   /* Arrays to facilitate easy interlacing - use pass (0 - 6) as index */

   /* Start of interlace block */
   static PNG_CONST png_byte png_pass_start[7] = {0, 4, 0, 2, 0, 1, 0};

   /* Offset to next interlace block */
   static PNG_CONST png_byte png_pass_inc[7] = {8, 8, 4, 4, 2, 2, 1};

   /* Start of interlace block in the y direction */
   static PNG_CONST png_byte png_pass_ystart[7] = {0, 0, 4, 0, 2, 0, 1};

   /* Offset to next interlace block in the y direction */
   static PNG_CONST png_byte png_pass_yinc[7] = {8, 8, 8, 4, 4, 2, 2};
#endif

   int max_pixel_depth;
   png_size_t row_bytes;

   png_debug(1, "in png_read_start_row");

#ifdef PNG_READ_TRANSFORMS_SUPPORTED
   png_init_read_transformations(png_ptr);
#endif
#ifdef PNG_READ_INTERLACING_SUPPORTED
   if (png_ptr->interlaced)
   {
      if (!(png_ptr->transformations & PNG_INTERLACE))
         png_ptr->num_rows = (png_ptr->height + png_pass_yinc[0] - 1 -
             png_pass_ystart[0]) / png_pass_yinc[0];

      else
         png_ptr->num_rows = png_ptr->height;

      png_ptr->iwidth = (png_ptr->width +
          png_pass_inc[png_ptr->pass] - 1 -
          png_pass_start[png_ptr->pass]) /
          png_pass_inc[png_ptr->pass];
   }

   else
#endif /* PNG_READ_INTERLACING_SUPPORTED */
   {
      png_ptr->num_rows = png_ptr->height;
      png_ptr->iwidth = png_ptr->width;
   }

   max_pixel_depth = png_ptr->pixel_depth;

   /* WARNING: * png_read_transform_info (pngrtran.c) performs a simpliar set of
    * calculations to calculate the final pixel depth, then
    * png_do_read_transforms actually does the transforms.  This means that the
    * code which effectively calculates this value is actually repeated in three
    * separate places.  They must all match.  Innocent changes to the order of
    * transformations can and will break libpng in a way that causes memory
    * overwrites.
    *
    * TODO: fix this.
    */
#ifdef PNG_READ_PACK_SUPPORTED
   if ((png_ptr->transformations & PNG_PACK) && png_ptr->bit_depth < 8)
      max_pixel_depth = 8;
#endif

#ifdef PNG_READ_EXPAND_SUPPORTED
   if (png_ptr->transformations & PNG_EXPAND)
   {
      if (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      {
         if (png_ptr->num_trans)
            max_pixel_depth = 32;

         else
            max_pixel_depth = 24;
      }

      else if (png_ptr->color_type == PNG_COLOR_TYPE_GRAY)
      {
         if (max_pixel_depth < 8)
            max_pixel_depth = 8;

         if (png_ptr->num_trans)
            max_pixel_depth *= 2;
      }

      else if (png_ptr->color_type == PNG_COLOR_TYPE_RGB)
      {
         if (png_ptr->num_trans)
         {
            max_pixel_depth *= 4;
            max_pixel_depth /= 3;
         }
      }
   }
#endif

#ifdef PNG_READ_EXPAND_16_SUPPORTED
   if (png_ptr->transformations & PNG_EXPAND_16)
   {
#     ifdef PNG_READ_EXPAND_SUPPORTED
         /* In fact it is an error if it isn't supported, but checking is
          * the safe way.
          */
         if (png_ptr->transformations & PNG_EXPAND)
         {
            if (png_ptr->bit_depth < 16)
               max_pixel_depth *= 2;
         }
         else
#     endif
         png_ptr->transformations &= ~PNG_EXPAND_16;
   }
#endif

#ifdef PNG_READ_FILLER_SUPPORTED
   if (png_ptr->transformations & (PNG_FILLER))
   {
      if (png_ptr->color_type == PNG_COLOR_TYPE_GRAY)
      {
         if (max_pixel_depth <= 8)
            max_pixel_depth = 16;

         else
            max_pixel_depth = 32;
      }

      else if (png_ptr->color_type == PNG_COLOR_TYPE_RGB ||
         png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      {
         if (max_pixel_depth <= 32)
            max_pixel_depth = 32;

         else
            max_pixel_depth = 64;
      }
   }
#endif

#ifdef PNG_READ_GRAY_TO_RGB_SUPPORTED
   if (png_ptr->transformations & PNG_GRAY_TO_RGB)
   {
      if (
#ifdef PNG_READ_EXPAND_SUPPORTED
          (png_ptr->num_trans && (png_ptr->transformations & PNG_EXPAND)) ||
#endif
#ifdef PNG_READ_FILLER_SUPPORTED
          (png_ptr->transformations & (PNG_FILLER)) ||
#endif
          png_ptr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
      {
         if (max_pixel_depth <= 16)
            max_pixel_depth = 32;

         else
            max_pixel_depth = 64;
      }

      else
      {
         if (max_pixel_depth <= 8)
         {
            if (png_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
               max_pixel_depth = 32;

            else
               max_pixel_depth = 24;
         }

         else if (png_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
            max_pixel_depth = 64;

         else
            max_pixel_depth = 48;
      }
   }
#endif

#if defined(PNG_READ_USER_TRANSFORM_SUPPORTED) && \
defined(PNG_USER_TRANSFORM_PTR_SUPPORTED)
   if (png_ptr->transformations & PNG_USER_TRANSFORM)
   {
      int user_pixel_depth = png_ptr->user_transform_depth *
         png_ptr->user_transform_channels;

      if (user_pixel_depth > max_pixel_depth)
         max_pixel_depth = user_pixel_depth;
   }
#endif

   /* This value is stored in png_struct and double checked in the row read
    * code.
    */
   png_ptr->maximum_pixel_depth = (png_byte)max_pixel_depth;
   png_ptr->transformed_pixel_depth = 0; /* calculated on demand */

   /* Align the width on the next larger 8 pixels.  Mainly used
    * for interlacing
    */
   row_bytes = ((png_ptr->width + 7) & ~((png_uint_32)7));
   /* Calculate the maximum bytes needed, adding a byte and a pixel
    * for safety's sake
    */
   row_bytes = PNG_ROWBYTES(max_pixel_depth, row_bytes) +
       1 + ((max_pixel_depth + 7) >> 3);

#ifdef PNG_MAX_MALLOC_64K
   if (row_bytes > (png_uint_32)65536L)
      png_error(png_ptr, "This image requires a row greater than 64KB");
#endif

   if (row_bytes + 48 > png_ptr->old_big_row_buf_size)
   {
     png_free(png_ptr, png_ptr->big_row_buf);
     png_free(png_ptr, png_ptr->big_prev_row);

     if (png_ptr->interlaced)
        png_ptr->big_row_buf = (png_bytep)png_calloc(png_ptr,
            row_bytes + 48);

     else
        png_ptr->big_row_buf = (png_bytep)png_malloc(png_ptr, row_bytes + 48);

     png_ptr->big_prev_row = (png_bytep)png_malloc(png_ptr, row_bytes + 48);

#ifdef PNG_ALIGNED_MEMORY_SUPPORTED
     /* Use 16-byte aligned memory for row_buf with at least 16 bytes
      * of padding before and after row_buf; treat prev_row similarly.
      * NOTE: the alignment is to the start of the pixels, one beyond the start
      * of the buffer, because of the filter byte.  Prior to libpng 1.5.6 this
      * was incorrect; the filter byte was aligned, which had the exact
      * opposite effect of that intended.
      */
     {
        png_bytep temp = png_ptr->big_row_buf + 32;
        int extra = (int)((temp - (png_bytep)0) & 0x0f);
        png_ptr->row_buf = temp - extra - 1/*filter byte*/;

        temp = png_ptr->big_prev_row + 32;
        extra = (int)((temp - (png_bytep)0) & 0x0f);
        png_ptr->prev_row = temp - extra - 1/*filter byte*/;
     }

#else
     /* Use 31 bytes of padding before and 17 bytes after row_buf. */
     png_ptr->row_buf = png_ptr->big_row_buf + 31;
     png_ptr->prev_row = png_ptr->big_prev_row + 31;
#endif
     png_ptr->old_big_row_buf_size = row_bytes + 48;
   }

#ifdef PNG_MAX_MALLOC_64K
   if (png_ptr->rowbytes > 65535)
      png_error(png_ptr, "This image requires a row greater than 64KB");

#endif
   if (png_ptr->rowbytes > (PNG_SIZE_MAX - 1))
      png_error(png_ptr, "Row has too many bytes to allocate in memory");

   png_memset(png_ptr->prev_row, 0, png_ptr->rowbytes + 1);

   png_debug1(3, "width = %u,", png_ptr->width);
   png_debug1(3, "height = %u,", png_ptr->height);
   png_debug1(3, "iwidth = %u,", png_ptr->iwidth);
   png_debug1(3, "num_rows = %u,", png_ptr->num_rows);
   png_debug1(3, "rowbytes = %lu,", (unsigned long)png_ptr->rowbytes);
   png_debug1(3, "irowbytes = %lu",
       (unsigned long)PNG_ROWBYTES(png_ptr->pixel_depth, png_ptr->iwidth) + 1);

   /* Finally claim the zstream for the inflate of the IDAT data. */
   png_inflate_claim(png_ptr);

   png_ptr->flags |= PNG_FLAG_ROW_INIT;
}
#endif /* PNG_READ_SUPPORTED */
