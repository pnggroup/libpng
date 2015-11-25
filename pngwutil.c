
/* pngwutil.c - utilities to write a PNG file
 *
 * Last changed in libpng 1.7.0 [(PENDING RELEASE)]
 * Copyright (c) 1998-2015 Glenn Randers-Pehrson
 * (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
 * (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */

#include "pngpriv.h"
#define PNG_SRC_FILE PNG_SRC_FILE_pngwutil

#ifdef PNG_WRITE_SUPPORTED

#ifdef PNG_WRITE_INT_FUNCTIONS_SUPPORTED
/* Place a 32-bit number into a buffer in PNG byte order.  We work
 * with unsigned numbers for convenience, although one supported
 * ancillary chunk uses signed (two's complement) numbers.
 */
void PNGAPI
png_save_uint_32(png_bytep buf, png_uint_32 i)
{
   buf[0] = PNG_BYTE(i >> 24);
   buf[1] = PNG_BYTE(i >> 16);
   buf[2] = PNG_BYTE(i >> 8);
   buf[3] = PNG_BYTE(i);
}

/* Place a 16-bit number into a buffer in PNG byte order.
 * The parameter is declared unsigned int, not png_uint_16,
 * just to avoid potential problems on pre-ANSI C compilers.
 */
void PNGAPI
png_save_uint_16(png_bytep buf, unsigned int i)
{
   buf[0] = PNG_BYTE(i >> 8);
   buf[1] = PNG_BYTE(i);
}
#endif /* WRITE_INT_FUNCTIONS */

/* Simple function to write the signature.  If we have already written
 * the magic bytes of the signature, or more likely, the PNG stream is
 * being embedded into another stream and doesn't need its own signature,
 * we should call png_set_sig_bytes() to tell libpng how many of the
 * bytes have already been written.
 */
void PNGAPI
png_write_sig(png_structrp png_ptr)
{
   png_byte png_signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};

#ifdef PNG_IO_STATE_SUPPORTED
   /* Inform the I/O callback that the signature is being written */
   png_ptr->io_state = PNG_IO_WRITING | PNG_IO_SIGNATURE;
#endif

   /* Write the rest of the 8 byte signature */
   png_write_data(png_ptr, &png_signature[png_ptr->sig_bytes],
      (png_size_t)(8 - png_ptr->sig_bytes));

   if (png_ptr->sig_bytes < 3)
      png_ptr->mode |= PNG_HAVE_PNG_SIGNATURE;
}

/* Write the start of a PNG chunk.  The type is the chunk type.
 * The total_length is the sum of the lengths of all the data you will be
 * passing in png_write_chunk_data().
 */
static void
png_write_chunk_header(png_structrp png_ptr, png_uint_32 chunk_name,
    png_uint_32 length)
{
   png_byte buf[8];

#if defined(PNG_DEBUG) && (PNG_DEBUG > 0)
   PNG_CSTRING_FROM_CHUNK(buf, chunk_name);
   png_debug2(0, "Writing %s chunk, length = %lu", buf, (unsigned long)length);
#endif

   if (png_ptr == NULL)
      return;

#ifdef PNG_IO_STATE_SUPPORTED
   /* Inform the I/O callback that the chunk header is being written.
    * PNG_IO_CHUNK_HDR requires a single I/O call.
    */
   png_ptr->io_state = PNG_IO_WRITING | PNG_IO_CHUNK_HDR;
#endif

   /* Write the length and the chunk name */
   png_save_uint_32(buf, length);
   png_save_uint_32(buf + 4, chunk_name);
   png_write_data(png_ptr, buf, 8);

   /* Put the chunk name into png_ptr->chunk_name */
   png_ptr->chunk_name = chunk_name;

   /* Reset the crc and run it over the chunk name */
   png_reset_crc(png_ptr);

   png_calculate_crc(png_ptr, buf + 4, 4);

#ifdef PNG_IO_STATE_SUPPORTED
   /* Inform the I/O callback that chunk data will (possibly) be written.
    * PNG_IO_CHUNK_DATA does NOT require a specific number of I/O calls.
    */
   png_ptr->io_state = PNG_IO_WRITING | PNG_IO_CHUNK_DATA;
#endif
}

void PNGAPI
png_write_chunk_start(png_structrp png_ptr, png_const_bytep chunk_string,
    png_uint_32 length)
{
   png_write_chunk_header(png_ptr, PNG_CHUNK_FROM_STRING(chunk_string), length);
}

/* Write the data of a PNG chunk started with png_write_chunk_header().
 * Note that multiple calls to this function are allowed, and that the
 * sum of the lengths from these calls *must* add up to the total_length
 * given to png_write_chunk_header().
 */
void PNGAPI
png_write_chunk_data(png_structrp png_ptr, png_const_bytep data,
    png_size_t length)
{
   /* Write the data, and run the CRC over it */
   if (png_ptr == NULL)
      return;

   if (data != NULL && length > 0)
   {
      png_write_data(png_ptr, data, length);

      /* Update the CRC after writing the data,
       * in case the user I/O routine alters it.
       */
      png_calculate_crc(png_ptr, data, length);
   }
}

/* Finish a chunk started with png_write_chunk_header(). */
void PNGAPI
png_write_chunk_end(png_structrp png_ptr)
{
   png_byte buf[4];

   if (png_ptr == NULL) return;

#ifdef PNG_IO_STATE_SUPPORTED
   /* Inform the I/O callback that the chunk CRC is being written.
    * PNG_IO_CHUNK_CRC requires a single I/O function call.
    */
   png_ptr->io_state = PNG_IO_WRITING | PNG_IO_CHUNK_CRC;
#endif

   /* Write the crc in a single operation */
   png_save_uint_32(buf, png_ptr->crc);

   png_write_data(png_ptr, buf, (png_size_t)4);
}

/* Write a PNG chunk all at once.  The type is an array of ASCII characters
 * representing the chunk name.  The array must be at least 4 bytes in
 * length, and does not need to be null terminated.  To be safe, pass the
 * pre-defined chunk names here, and if you need a new one, define it
 * where the others are defined.  The length is the length of the data.
 * All the data must be present.  If that is not possible, use the
 * png_write_chunk_start(), png_write_chunk_data(), and png_write_chunk_end()
 * functions instead.
 */
static void
png_write_complete_chunk(png_structrp png_ptr, png_uint_32 chunk_name,
   png_const_bytep data, png_size_t length)
{
   if (png_ptr == NULL)
      return;

   /* On 64 bit architectures 'length' may not fit in a png_uint_32. */
   if (length > PNG_UINT_31_MAX)
      png_error(png_ptr, "length exceeds PNG maximum");

   png_write_chunk_header(png_ptr, chunk_name, (png_uint_32)length);
   png_write_chunk_data(png_ptr, data, length);
   png_write_chunk_end(png_ptr);
}

/* This is the API that calls the internal function above. */
void PNGAPI
png_write_chunk(png_structrp png_ptr, png_const_bytep chunk_string,
   png_const_bytep data, png_size_t length)
{
   png_write_complete_chunk(png_ptr, PNG_CHUNK_FROM_STRING(chunk_string), data,
      length);
}

/* This is used below to find the size of an image to pass to png_deflate_claim,
 * so it only needs to be accurate if the size is less than 16384 bytes (the
 * point at which a lower LZ window size can be used.)
 */
static png_alloc_size_t
png_image_size(png_structrp png_ptr)
{
   /* Only return sizes up to the maximum of a png_uint_32; do this by limiting
    * the width and height used to 15 bits.
    */
   const png_uint_32 h = png_ptr->height;
   const png_uint_32 w = png_ptr->width;
   const unsigned int pd = PNG_PIXEL_DEPTH(*png_ptr);
   png_alloc_size_t rowbytes = PNG_ROWBYTES(pd, w);

   if (rowbytes < 32768 && h < 32768)
   {
      if (png_ptr->interlaced != 0)
      {
         /* Interlacing makes the image larger because of the replication of
          * both the filter byte and the padding to a byte boundary.
          */
         png_alloc_size_t cb_base;
         int pass;

         for (cb_base=0, pass=0; pass<PNG_INTERLACE_ADAM7_PASSES; ++pass)
         {
            png_uint_32 pw = PNG_PASS_COLS(w, pass);

            if (pw > 0)
               cb_base += (PNG_ROWBYTES(pd, pw)+1) * PNG_PASS_ROWS(h, pass);
         }

         return cb_base;
      }

      else
         return (rowbytes+1) * h;
   }

   else
      return 0xffffffffU;
}

#ifdef PNG_WRITE_OPTIMIZE_CMF_SUPPORTED
   /* This is the code to hack the first two bytes of the deflate stream (the
    * deflate header) to correct the windowBits value to match the actual data
    * size.  Note that the second argument is the *uncompressed* size but the
    * first argument is the *compressed* data (and it must be deflate
    * compressed.)
    */
static void
optimize_cmf(png_bytep data, png_alloc_size_t data_size)
{
   /* Optimize the CMF field in the zlib stream.  The resultant zlib stream is
    * still compliant to the stream specification.
    */
   if (data_size <= 16384) /* else windowBits must be 15 */
   {
      unsigned int z_cmf = data[0];  /* zlib compression method and flags */

      if ((z_cmf & 0x0f) == 8 && (z_cmf & 0xf0) <= 0x70)
      {
         unsigned int z_cinfo;
         unsigned int half_z_window_size;

         z_cinfo = z_cmf >> 4;
         half_z_window_size = 1U << (z_cinfo + 7);

         if (data_size <= half_z_window_size) /* else no change */
         {
            unsigned int tmp;

            do
            {
               half_z_window_size >>= 1;
               --z_cinfo;
            }
            while (z_cinfo > 0 && data_size <= half_z_window_size);

            z_cmf = (z_cmf & 0x0f) | (z_cinfo << 4);

            data[0] = png_check_byte(0/*TODO: fixme*/, z_cmf);
            tmp = data[1] & 0xe0;
            tmp += 0x1f - ((z_cmf << 8) + tmp) % 0x1f;
            data[1] = png_check_byte(0/*TODO: fixme*/, tmp);
         }
      }
   }
}
#endif /* WRITE_OPTIMIZE_CMF */

/* Initialize the compressor for the appropriate type of compression. */
static int
png_deflate_claim(png_structrp png_ptr, png_uint_32 owner,
   png_alloc_size_t data_size)
{
   if (png_ptr->zowner != 0)
   {
#if defined(PNG_WARNINGS_SUPPORTED) || defined(PNG_ERROR_TEXT_SUPPORTED)
      char msg[64];

      PNG_STRING_FROM_CHUNK(msg, owner);
      msg[4] = ':';
      msg[5] = ' ';
      PNG_STRING_FROM_CHUNK(msg+6, png_ptr->zowner);
      /* So the message that results is "<chunk> using zstream"; this is an
       * internal error, but is very useful for debugging.  i18n requirements
       * are minimal.
       */
      (void)png_safecat(msg, (sizeof msg), 10, " using zstream");
#endif
#if PNG_RELEASE_BUILD
         png_warning(png_ptr, msg);

         /* Attempt sane error recovery */
         if (png_ptr->zowner == png_IDAT) /* don't steal from IDAT */
         {
            png_ptr->zstream.msg = PNGZ_MSG_CAST("in use by IDAT");
            return Z_STREAM_ERROR;
         }

         png_ptr->zowner = 0;
#else
         png_error(png_ptr, msg);
#endif
   }

   {
      int level = png_ptr->zlib_level;
      int method = png_ptr->zlib_method;
      int windowBits = png_ptr->zlib_window_bits;
      int memLevel = png_ptr->zlib_mem_level;
      int strategy; /* set below */
      int ret; /* zlib return code */

      if (owner == png_IDAT)
      {
#ifdef PNG_WRITE_CUSTOMIZE_COMPRESSION_SUPPORTED
         if ((png_ptr->flags & PNG_FLAG_ZLIB_CUSTOM_STRATEGY) != 0)
            strategy = png_ptr->zlib_strategy;
         else
#endif /* WRITE_CUSTOMIZE_COMPRESSION */

#ifdef PNG_WRITE_FILTER_SUPPORTED
         if (png_ptr->filter_mask != PNG_FILTER_NONE)
            strategy = PNG_Z_DEFAULT_STRATEGY;
         else
#endif /* WRITE_FILTER */

         /* The default with no filters: */
         strategy = PNG_Z_DEFAULT_NOFILTER_STRATEGY;
      }

      else
      {
#ifdef PNG_WRITE_CUSTOMIZE_ZTXT_COMPRESSION_SUPPORTED
            level = png_ptr->zlib_text_level;
            method = png_ptr->zlib_text_method;
            windowBits = png_ptr->zlib_text_window_bits;
            memLevel = png_ptr->zlib_text_mem_level;
            strategy = png_ptr->zlib_text_strategy;
#else
            /* If customization is not supported the values all come from the
             * IDAT values except for the strategy, which is fixed to the
             * default.  (This is the pre-1.6.0 behavior too, although it was
             * implemented in a very different way.)
             */
            strategy = Z_DEFAULT_STRATEGY;
#endif
      }

      /* Adjust 'windowBits' down if larger than 'data_size'; to stop this
       * happening just pass 32768 as the data_size parameter.  Notice that zlib
       * requires an extra 262 bytes in the window in addition to the data to be
       * able to see the whole of the data, so if data_size+262 takes us to the
       * next windowBits size we need to fix up the value later.  (Because even
       * though deflate needs the extra window, inflate does not!)
       */
      if (data_size <= 16384)
      {
         /* IMPLEMENTATION NOTE: this 'half_window_size' stuff is only here to
          * work round a Microsoft Visual C misbehavior which, contrary to C-90,
          * widens the result of the following shift to 64-bits if (and,
          * apparently, only if) it is used in a test.
          */
         unsigned int half_window_size = 1U << (windowBits-1);

         while (data_size + 262 <= half_window_size)
         {
            half_window_size >>= 1;
            --windowBits;
         }
      }

      /* Check against the previous initialized values, if any. */
      if (png_ptr->zstream.state != NULL &&
         (png_ptr->zlib_set_level != level ||
         png_ptr->zlib_set_method != method ||
         png_ptr->zlib_set_window_bits != windowBits ||
         png_ptr->zlib_set_mem_level != memLevel ||
         png_ptr->zlib_set_strategy != strategy))
      {
         /* This shadows 'ret' deliberately; we ignore failures in deflateEnd:
          */
         int ret_end = deflateEnd(&png_ptr->zstream);

         if (ret_end != Z_OK || png_ptr->zstream.state != NULL)
         {
            png_zstream_error(png_ptr, ret_end);
            png_warning(png_ptr, png_ptr->zstream.msg);
            png_ptr->zstream.state = NULL; /* zlib error recovery */
         }
      }

      /* For safety clear out the input and output pointers (currently zlib
       * doesn't use them on Init, but it might in the future).
       */
      png_ptr->zstream.next_in = NULL;
      png_ptr->zstream.avail_in = 0;
      png_ptr->zstream.next_out = NULL;
      png_ptr->zstream.avail_out = 0;

      /* Now initialize if required, setting the new parameters, otherwise just
       * to a simple reset to the previous parameters.
       */
      if (png_ptr->zstream.state != NULL)
         ret = deflateReset(&png_ptr->zstream);

      else
         ret = deflateInit2(&png_ptr->zstream, level, method, windowBits,
            memLevel, strategy);

      /* The return code is from either deflateReset or deflateInit2; they have
       * pretty much the same set of error codes.
       */
      if (ret == Z_OK && png_ptr->zstream.state != NULL)
         png_ptr->zowner = owner;

      else
         png_zstream_error(png_ptr, ret);

      return ret;
   }
}

/* Clean up (or trim) a linked list of compression buffers. */
void /* PRIVATE */
png_free_buffer_list(png_structrp png_ptr, png_compression_bufferp *listp)
{
   png_compression_bufferp list = *listp;

   if (list != NULL)
   {
      *listp = NULL;

      do
      {
         png_compression_bufferp next = list->next;

         png_free(png_ptr, list);
         list = next;
      }
      while (list != NULL);
   }
}

#ifdef PNG_WRITE_COMPRESSED_TEXT_SUPPORTED
/* This pair of functions encapsulates the operation of (a) compressing a
 * text string, and (b) issuing it later as a series of chunk data writes.
 * The compression_state structure is shared context for these functions
 * set up by the caller to allow access to the relevant local variables.
 *
 * compression_buffer (new in 1.6.0) is just a linked list of zbuffer_size
 * temporary buffers.  From 1.6.0 it is retained in png_struct so that it will
 * be correctly freed in the event of a write error (previous implementations
 * just leaked memory.)
 */
typedef struct
{
   png_const_bytep      input;        /* The uncompressed input data */
   png_alloc_size_t     input_len;    /* Its length */
   png_uint_32          output_len;   /* Final compressed length */
   png_byte             output[1024]; /* First block of output */
} compression_state;

static void
png_text_compress_init(compression_state *comp, png_const_bytep input,
   png_alloc_size_t input_len)
{
   comp->input = input;
   comp->input_len = input_len;
   comp->output_len = 0;
}

/* Compress the data in the compression state input */
static int
png_text_compress(png_structrp png_ptr, png_uint_32 chunk_name,
   compression_state *comp, png_uint_32 prefix_len)
{
   int ret;

   /* To find the length of the output it is necessary to first compress the
    * input. The result is buffered rather than using the two-pass algorithm
    * that is used on the inflate side; deflate is assumed to be slower and a
    * PNG writer is assumed to have more memory available than a PNG reader.
    *
    * IMPLEMENTATION NOTE: the zlib API deflateBound() can be used to find an
    * upper limit on the output size, but it is always bigger than the input
    * size so it is likely to be more efficient to use this linked-list
    * approach.
    */
   ret = png_deflate_claim(png_ptr, chunk_name, comp->input_len);

   if (ret != Z_OK)
      return ret;

   /* Set up the compression buffers, we need a loop here to avoid overflowing a
    * uInt.  Use ZLIB_IO_MAX to limit the input.  The output is always limited
    * by the output buffer size, so there is no need to check that.  Since this
    * is ANSI-C we know that an 'int', hence a uInt, is always at least 16 bits
    * in size.
    */
   {
      png_compression_bufferp *end = &png_ptr->zbuffer_list;
      png_alloc_size_t input_len = comp->input_len; /* may be zero! */
      png_uint_32 output_len;

      /* zlib updates these for us: */
      png_ptr->zstream.next_in = PNGZ_INPUT_CAST(comp->input);
      png_ptr->zstream.avail_in = 0; /* Set below */
      png_ptr->zstream.next_out = comp->output;
      png_ptr->zstream.avail_out = (sizeof comp->output);

      output_len = png_ptr->zstream.avail_out;

      do
      {
         uInt avail_in = ZLIB_IO_MAX;

         if (avail_in > input_len)
            avail_in = (uInt)input_len;

         input_len -= avail_in;

         png_ptr->zstream.avail_in = avail_in;

         if (png_ptr->zstream.avail_out == 0)
         {
            png_compression_buffer *next;

            /* Chunk data is limited to 2^31 bytes in length, so the prefix
             * length must be counted here.
             */
            if (output_len + prefix_len > PNG_UINT_31_MAX)
            {
               ret = Z_MEM_ERROR;
               break;
            }

            /* Need a new (malloc'ed) buffer, but there may be one present
             * already.
             */
            next = *end;
            if (next == NULL)
            {
               next = png_voidcast(png_compression_bufferp, png_malloc_base
                  (png_ptr, PNG_COMPRESSION_BUFFER_SIZE(png_ptr)));

               if (next == NULL)
               {
                  ret = Z_MEM_ERROR;
                  break;
               }

               /* Link in this buffer (so that it will be freed later) */
               next->next = NULL;
               *end = next;
            }

            png_ptr->zstream.next_out = next->output;
            png_ptr->zstream.avail_out = png_ptr->zbuffer_size;
            output_len += png_ptr->zstream.avail_out;

            /* Move 'end' to the next buffer pointer. */
            end = &next->next;
         }

         /* Compress the data */
         ret = deflate(&png_ptr->zstream,
            input_len > 0 ? Z_NO_FLUSH : Z_FINISH);

         /* Claw back input data that was not consumed (because avail_in is
          * reset above every time round the loop).
          */
         input_len += png_ptr->zstream.avail_in;
         png_ptr->zstream.avail_in = 0; /* safety */
      }
      while (ret == Z_OK);

      /* There may be some space left in the last output buffer. This needs to
       * be subtracted from output_len.
       */
      output_len -= png_ptr->zstream.avail_out;
      png_ptr->zstream.avail_out = 0; /* safety */
      comp->output_len = output_len;

      /* Now double check the output length, put in a custom message if it is
       * too long.  Otherwise ensure the z_stream::msg pointer is set to
       * something.
       */
      if (output_len + prefix_len >= PNG_UINT_31_MAX)
      {
         png_ptr->zstream.msg = PNGZ_MSG_CAST("compressed data too long");
         ret = Z_MEM_ERROR;
      }

      else
         png_zstream_error(png_ptr, ret);

      /* Reset zlib for another zTXt/iTXt or image data */
      png_ptr->zowner = 0;

      /* The only success case is Z_STREAM_END, input_len must be 0; if not this
       * is an internal error.
       */
      if (ret == Z_STREAM_END && input_len == 0)
      {
#ifdef PNG_WRITE_OPTIMIZE_CMF_SUPPORTED
         /* Fix up the deflate header, if required */
         optimize_cmf(comp->output, comp->input_len);
#endif
         /* But Z_OK is returned, not Z_STREAM_END; this allows the claim
          * function above to return Z_STREAM_END on an error (though it never
          * does in the current versions of zlib.)
          */
         return Z_OK;
      }

      else
         return ret;
   }
}

/* Ship the compressed text out via chunk writes */
static void
png_write_compressed_data_out(png_structrp png_ptr, compression_state *comp)
{
   png_uint_32 output_len = comp->output_len;
   png_const_bytep output = comp->output;
   png_uint_32 avail = (sizeof comp->output);
   png_compression_buffer *next = png_ptr->zbuffer_list;

   for (;;)
   {
      if (avail > output_len)
         avail = output_len;

      png_write_chunk_data(png_ptr, output, avail);

      output_len -= avail;

      if (output_len == 0 || next == NULL)
         break;

      avail = png_ptr->zbuffer_size;
      output = next->output;
      next = next->next;
   }

   /* This is an internal error; 'next' must have been NULL! */
   affirm(output_len == 0);
}
#endif /* WRITE_COMPRESSED_TEXT */

#if defined(PNG_WRITE_TEXT_SUPPORTED) || defined(PNG_WRITE_pCAL_SUPPORTED) || \
    defined(PNG_WRITE_iCCP_SUPPORTED) || defined(PNG_WRITE_sPLT_SUPPORTED)
/* Check that the tEXt or zTXt keyword is valid per PNG 1.0 specification,
 * and if invalid, correct the keyword rather than discarding the entire
 * chunk.  The PNG 1.0 specification requires keywords 1-79 characters in
 * length, forbids leading or trailing whitespace, multiple internal spaces,
 * and the non-break space (0x80) from ISO 8859-1.  Returns keyword length.
 *
 * The 'new_key' buffer must be 80 characters in size (for the keyword plus a
 * trailing '\0').  If this routine returns 0 then there was no keyword, or a
 * valid one could not be generated, and the caller must png_error.
 */
static png_uint_32
png_check_keyword(png_structrp png_ptr, png_const_charp key, png_bytep new_key)
{
   png_const_charp orig_key = key;
   png_uint_32 key_len = 0;
   int bad_character = 0;
   int space = 1;

   png_debug(1, "in png_check_keyword");

   if (key == NULL)
   {
      *new_key = 0;
      return 0;
   }

   while (*key && key_len < 79)
   {
      png_byte ch = (png_byte)(0xff & *key++);

      if ((ch > 32 && ch <= 126) || (ch >= 161 /*&& ch <= 255*/))
         *new_key++ = ch, ++key_len, space = 0;

      else if (space == 0)
      {
         /* A space or an invalid character when one wasn't seen immediately
          * before; output just a space.
          */
         *new_key++ = 32, ++key_len, space = 1;

         /* If the character was not a space then it is invalid. */
         if (ch != 32)
            bad_character = ch;
      }

      else if (bad_character == 0)
         bad_character = ch; /* just skip it, record the first error */
   }

   if (key_len > 0 && space != 0) /* trailing space */
   {
      --key_len, --new_key;
      if (bad_character == 0)
         bad_character = 32;
   }

   /* Terminate the keyword */
   *new_key = 0;

   if (key_len == 0)
      return 0;

#ifdef PNG_WARNINGS_SUPPORTED
   /* Try to only output one warning per keyword: */
   if (*key != 0) /* keyword too long */
      png_warning(png_ptr, "keyword truncated");

   else if (bad_character != 0)
   {
      PNG_WARNING_PARAMETERS(p)

      png_warning_parameter(p, 1, orig_key);
      png_warning_parameter_signed(p, 2, PNG_NUMBER_FORMAT_02x, bad_character);

      png_formatted_warning(png_ptr, p, "keyword \"@1\": bad character '0x@2'");
   }
#endif /* WARNINGS */

   return key_len;
}
#endif /* WRITE_TEXT || WRITE_pCAL || WRITE_iCCP || WRITE_sPLT */

/* Write the IHDR chunk, and update the png_struct with the necessary
 * information.  Note that the rest of this code depends upon this
 * information being correct.
 */
void /* PRIVATE */
png_write_IHDR(png_structrp png_ptr, png_uint_32 width, png_uint_32 height,
    int bit_depth, int color_type, int compression_type, int filter_method,
    int interlace_type)
{
   png_byte buf[13]; /* Buffer to store the IHDR info */

   png_debug(1, "in png_write_IHDR");

   /* Check that we have valid input data from the application info */
   switch (color_type)
   {
      case PNG_COLOR_TYPE_GRAY:
         switch (bit_depth)
         {
            case 1:
            case 2:
            case 4:
            case 8:
#ifdef PNG_WRITE_16BIT_SUPPORTED
            case 16:
#endif
               break;

            default:
               png_error(png_ptr, "Invalid bit depth for grayscale image");
         }
         break;

      case PNG_COLOR_TYPE_RGB:
#ifdef PNG_WRITE_16BIT_SUPPORTED
         if (bit_depth != 8 && bit_depth != 16)
#else
         if (bit_depth != 8)
#endif
            png_error(png_ptr, "Invalid bit depth for RGB image");

         break;

      case PNG_COLOR_TYPE_PALETTE:
         switch (bit_depth)
         {
            case 1:
            case 2:
            case 4:
            case 8:
               break;

            default:
               png_error(png_ptr, "Invalid bit depth for paletted image");
         }
         break;

      case PNG_COLOR_TYPE_GRAY_ALPHA:
         if (bit_depth != 8 && bit_depth != 16)
            png_error(png_ptr, "Invalid bit depth for grayscale+alpha image");

         break;

      case PNG_COLOR_TYPE_RGB_ALPHA:
#ifdef PNG_WRITE_16BIT_SUPPORTED
         if (bit_depth != 8 && bit_depth != 16)
#else
         if (bit_depth != 8)
#endif
            png_error(png_ptr, "Invalid bit depth for RGBA image");

         break;

      default:
         png_error(png_ptr, "Invalid image color type specified");
   }

   if (compression_type != PNG_COMPRESSION_TYPE_BASE)
   {
      png_app_error(png_ptr, "Invalid compression type specified");
      compression_type = PNG_COMPRESSION_TYPE_BASE;
   }

   /* Write filter_method 64 (intrapixel differencing) only if
    * 1. Libpng was compiled with PNG_MNG_FEATURES_SUPPORTED and
    * 2. Libpng did not write a PNG signature (this filter_method is only
    *    used in PNG datastreams that are embedded in MNG datastreams) and
    * 3. The application called png_permit_mng_features with a mask that
    *    included PNG_FLAG_MNG_FILTER_64 and
    * 4. The filter_method is 64 and
    * 5. The color_type is RGB or RGBA
    */
   if (
#     ifdef PNG_MNG_FEATURES_SUPPORTED
         !((png_ptr->mng_features_permitted & PNG_FLAG_MNG_FILTER_64) != 0 &&
           ((png_ptr->mode & PNG_HAVE_PNG_SIGNATURE) == 0) &&
           (color_type == PNG_COLOR_TYPE_RGB ||
            color_type == PNG_COLOR_TYPE_RGB_ALPHA) &&
           (filter_method == PNG_INTRAPIXEL_DIFFERENCING)) &&
#     endif /* MNG_FEATURES */
       filter_method != PNG_FILTER_TYPE_BASE)
   {
      png_app_error(png_ptr, "Invalid filter type specified");
      filter_method = PNG_FILTER_TYPE_BASE;
   }

   if (interlace_type != PNG_INTERLACE_NONE &&
       interlace_type != PNG_INTERLACE_ADAM7)
   {
      png_app_error(png_ptr, "Invalid interlace type specified");
      interlace_type = PNG_INTERLACE_ADAM7;
   }

   /* Save the relevant information */
   png_ptr->bit_depth = png_check_byte(png_ptr, bit_depth);
   png_ptr->color_type = png_check_byte(png_ptr, color_type);
   png_ptr->interlaced = png_check_byte(png_ptr, interlace_type);
   png_ptr->filter_method = png_check_byte(png_ptr, filter_method);
   png_ptr->compression_type = png_check_byte(png_ptr, compression_type);
   png_ptr->width = width;
   png_ptr->height = height;

   /* Pack the header information into the buffer */
   png_save_uint_32(buf, width);
   png_save_uint_32(buf + 4, height);
   buf[8] = png_check_byte(png_ptr, bit_depth);
   buf[9] = png_check_byte(png_ptr, color_type);
   buf[10] = png_check_byte(png_ptr, compression_type);
   buf[11] = png_check_byte(png_ptr, filter_method);
   buf[12] = png_check_byte(png_ptr, interlace_type);

   /* Write the chunk */
   png_write_complete_chunk(png_ptr, png_IHDR, buf, (png_size_t)13);

#  ifdef PNG_WRITE_FILTER_SUPPORTED
      /* TODO: review this setting */
      if (png_ptr->filter_mask == PNG_NO_FILTERS /* not yet set */)
      {
         if (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE ||
             png_ptr->bit_depth < 8)
            png_ptr->filter_mask = PNG_FILTER_NONE;

         else
            png_ptr->filter_mask = PNG_ALL_FILTERS;
      }
#  endif

   png_ptr->mode |= PNG_HAVE_IHDR;
}

/* Write the palette.  We are careful not to trust png_color to be in the
 * correct order for PNG, so people can redefine it to any convenient
 * structure.
 */
void /* PRIVATE */
png_write_PLTE(png_structrp png_ptr, png_const_colorp palette,
    unsigned int num_pal)
{
   png_uint_32 max_palette_length, i;
   png_const_colorp pal_ptr;
   png_byte buf[3];

   png_debug(1, "in png_write_PLTE");

   max_palette_length = (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE) ?
      (1 << png_ptr->bit_depth) : PNG_MAX_PALETTE_LENGTH;

   if ((
#     ifdef PNG_MNG_FEATURES_SUPPORTED
         (png_ptr->mng_features_permitted & PNG_FLAG_MNG_EMPTY_PLTE) == 0 &&
#     endif /* MNG_FEATURES */
       num_pal == 0) || num_pal > max_palette_length)
   {
      if (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      {
         png_error(png_ptr, "Invalid number of colors in palette");
      }

      else
      {
         png_warning(png_ptr, "Invalid number of colors in palette");
         return;
      }
   }

   if ((png_ptr->color_type & PNG_COLOR_MASK_COLOR) == 0)
   {
      png_warning(png_ptr,
          "Ignoring request to write a PLTE chunk in grayscale PNG");

      return;
   }

   png_ptr->num_palette = png_check_bits(png_ptr, num_pal, 9);
   png_debug1(3, "num_palette = %d", png_ptr->num_palette);

   png_write_chunk_header(png_ptr, png_PLTE, num_pal * 3U);

   for (i = 0, pal_ptr = palette; i < num_pal; i++, pal_ptr++)
   {
      buf[0] = pal_ptr->red;
      buf[1] = pal_ptr->green;
      buf[2] = pal_ptr->blue;
      png_write_chunk_data(png_ptr, buf, 3U);
   }

   png_write_chunk_end(png_ptr);
   png_ptr->mode |= PNG_HAVE_PLTE;
}

/* This is similar to png_text_compress, above, except that it does not require
 * all of the data at once and, instead of buffering the compressed result,
 * writes it as IDAT chunks.  Unlike png_text_compress it *can* png_error out
 * because it calls the write interface.  As a result it does its own error
 * reporting and does not return an error code.  In the event of error it will
 * just call png_error.  The input data length may exceed 32-bits.  The 'flush'
 * parameter is exactly the same as that to deflate, with the following
 * meanings:
 *
 * Z_NO_FLUSH: normal incremental output of compressed data
 * Z_SYNC_FLUSH: do a SYNC_FLUSH, used by png_write_flush
 * Z_FINISH: this is the end of the input, do a Z_FINISH and clean up
 *
 * The routine manages the acquire and release of the png_ptr->zstream by
 * checking and (at the end) clearing png_ptr->zowner; it does some sanity
 * checks on the 'mode' flags while doing this.
 */
void /* PRIVATE */
png_compress_IDAT(png_structrp png_ptr, png_const_bytep input,
   png_alloc_size_t input_len, int flush)
{
   if (png_ptr->zowner != png_IDAT)
   {
      /* First time.   Ensure we have a temporary buffer for compression and
       * trim the buffer list if it has more than one entry to free memory.
       * If 'WRITE_COMPRESSED_TEXT' is not set the list will never have been
       * created at this point, but the check here is quick and safe.
       */
      if (png_ptr->zbuffer_list == NULL)
      {
         png_ptr->zbuffer_list = png_voidcast(png_compression_bufferp,
            png_malloc(png_ptr, PNG_COMPRESSION_BUFFER_SIZE(png_ptr)));
         png_ptr->zbuffer_list->next = NULL;
      }

      else
         png_free_buffer_list(png_ptr, &png_ptr->zbuffer_list->next);

      /* It is a terminal error if we can't claim the zstream. */
      if (png_deflate_claim(png_ptr, png_IDAT, png_image_size(png_ptr)) != Z_OK)
         png_error(png_ptr, png_ptr->zstream.msg);

      /* The output state is maintained in png_ptr->zstream, so it must be
       * initialized here after the claim.
       */
      png_ptr->zstream.next_out = png_ptr->zbuffer_list->output;
      png_ptr->zstream.avail_out = png_ptr->zbuffer_size;
   }

   /* Now loop reading and writing until all the input is consumed or an error
    * terminates the operation.  The _out values are maintained across calls to
    * this function, but the input must be reset each time.
    */
   png_ptr->zstream.next_in = PNGZ_INPUT_CAST(input);
   png_ptr->zstream.avail_in = 0; /* set below */
   for (;;)
   {
      int ret;

      /* INPUT: from the row data */
      uInt avail = ZLIB_IO_MAX;

      if (avail > input_len)
         avail = (uInt)input_len; /* safe because of the check */

      png_ptr->zstream.avail_in = avail;
      input_len -= avail;

      ret = deflate(&png_ptr->zstream, input_len > 0 ? Z_NO_FLUSH : flush);

      /* Include as-yet unconsumed input */
      input_len += png_ptr->zstream.avail_in;
      png_ptr->zstream.avail_in = 0;

      /* OUTPUT: write complete IDAT chunks when avail_out drops to zero. Note
       * that these two zstream fields are preserved across the calls, therefore
       * there is no need to set these up on entry to the loop.
       */
      if (png_ptr->zstream.avail_out == 0)
      {
         png_bytep data = png_ptr->zbuffer_list->output;
         uInt size = png_ptr->zbuffer_size;

         /* Write an IDAT containing the data then reset the buffer.  The
          * first IDAT may need deflate header optimization.
          */
#ifdef PNG_WRITE_OPTIMIZE_CMF_SUPPORTED
            if ((png_ptr->mode & PNG_HAVE_IDAT) == 0 &&
                png_ptr->compression_type == PNG_COMPRESSION_TYPE_BASE)
               optimize_cmf(data, png_image_size(png_ptr));
#endif

         png_write_complete_chunk(png_ptr, png_IDAT, data, size);
         png_ptr->mode |= PNG_HAVE_IDAT;

         png_ptr->zstream.next_out = data;
         png_ptr->zstream.avail_out = size;

         /* For SYNC_FLUSH or FINISH it is essential to keep calling zlib with
          * the same flush parameter until it has finished output, for NO_FLUSH
          * it doesn't matter.
          */
         if (ret == Z_OK && flush != Z_NO_FLUSH)
            continue;
      }

      /* The order of these checks doesn't matter much; it just affects which
       * possible error might be detected if multiple things go wrong at once.
       */
      if (ret == Z_OK) /* most likely return code! */
      {
         /* If all the input has been consumed then just return.  If Z_FINISH
          * was used as the flush parameter something has gone wrong if we get
          * here.
          */
         if (input_len == 0)
         {
            if (flush == Z_FINISH)
               png_error(png_ptr, "Z_OK on Z_FINISH with output space");

            return;
         }
      }

      else if (ret == Z_STREAM_END && flush == Z_FINISH)
      {
         /* This is the end of the IDAT data; any pending output must be
          * flushed.  For small PNG files we may still be at the beginning.
          */
         png_bytep data = png_ptr->zbuffer_list->output;
         uInt size = png_ptr->zbuffer_size - png_ptr->zstream.avail_out;

#ifdef PNG_WRITE_OPTIMIZE_CMF_SUPPORTED
         if ((png_ptr->mode & PNG_HAVE_IDAT) == 0 &&
             png_ptr->compression_type == PNG_COMPRESSION_TYPE_BASE)
            optimize_cmf(data, png_image_size(png_ptr));
#endif

         png_write_complete_chunk(png_ptr, png_IDAT, data, size);
         png_ptr->zstream.avail_out = 0;
         png_ptr->zstream.next_out = NULL;
         png_ptr->mode |= PNG_HAVE_IDAT | PNG_AFTER_IDAT;

         png_ptr->zowner = 0; /* Release the stream */
         return;
      }

      else
      {
         /* This is an error condition. */
         png_zstream_error(png_ptr, ret);
         png_error(png_ptr, png_ptr->zstream.msg);
      }
   }
}

/* Write an IEND chunk */
void /* PRIVATE */
png_write_IEND(png_structrp png_ptr)
{
   png_debug(1, "in png_write_IEND");

   png_write_complete_chunk(png_ptr, png_IEND, NULL, (png_size_t)0);
   png_ptr->mode |= PNG_HAVE_IEND;
}

#if defined(PNG_WRITE_gAMA_SUPPORTED) || defined(PNG_WRITE_cHRM_SUPPORTED)
static int
png_save_int_31(png_structrp png_ptr, png_bytep buf, png_int_32 i)
   /* Save a signed value as a PNG unsigned value; the argument is required to
    * be in the range 0..0x7FFFFFFFU.  If not a *warning* is produced and false
    * is returned.  Because this is only called from png_write_cHRM_fixed and
    * png_write_gAMA_fixed below this is safe (we don't need either chunk,
    * particularly if the value is bogus.)
    *
    * The warning is png_app_error; it may return if the app tells it to but the
    * app can have it error out.  JB 20150821: I believe the checking in png.c
    * actually makes this error impossible, but this is safe.
    */
{
#ifndef __COVERITY__
   if (i >= 0 && i <= 0x7FFFFFFF)
#else
   /* Supress bogus Coverity complaint */
   if (i >= 0)
#endif
   {
      png_save_uint_32(buf, (png_uint_32)/*SAFE*/i);
      return 1;
   }

   else
   {
      png_chunk_report(png_ptr, "negative value in cHRM or gAMA",
         PNG_CHUNK_WRITE_ERROR);
      return 0;
   }
}
#endif /* WRITE_gAMA || WRITE_cHRM */

#ifdef PNG_WRITE_gAMA_SUPPORTED
/* Write a gAMA chunk */
void /* PRIVATE */
png_write_gAMA_fixed(png_structrp png_ptr, png_fixed_point file_gamma)
{
   png_byte buf[4];

   png_debug(1, "in png_write_gAMA");

   /* file_gamma is saved in 1/100,000ths */
   if (png_save_int_31(png_ptr, buf, file_gamma))
      png_write_complete_chunk(png_ptr, png_gAMA, buf, (png_size_t)4);
}
#endif

#ifdef PNG_WRITE_sRGB_SUPPORTED
/* Write a sRGB chunk */
void /* PRIVATE */
png_write_sRGB(png_structrp png_ptr, int srgb_intent)
{
   png_byte buf[1];

   png_debug(1, "in png_write_sRGB");

   if (srgb_intent >= PNG_sRGB_INTENT_LAST)
      png_warning(png_ptr,
          "Invalid sRGB rendering intent specified");

   buf[0] = png_check_byte(png_ptr, srgb_intent);
   png_write_complete_chunk(png_ptr, png_sRGB, buf, (png_size_t)1);
}
#endif

#ifdef PNG_WRITE_iCCP_SUPPORTED
/* Write an iCCP chunk */
void /* PRIVATE */
png_write_iCCP(png_structrp png_ptr, png_const_charp name,
    png_const_bytep profile)
{
   png_uint_32 name_len;
   png_uint_32 profile_len;
   png_byte new_name[81]; /* 1 byte for the compression byte */
   compression_state comp;

   png_debug(1, "in png_write_iCCP");

   /* These are all internal problems: the profile should have been checked
    * before when it was stored.
    */
   affirm(profile != NULL);

   profile_len = png_get_uint_32(profile);

   if (profile_len < 132)
      png_error(png_ptr, "ICC profile too short");

   if (profile_len & 0x03)
      png_error(png_ptr, "ICC profile length invalid (not a multiple of 4)");

   {
      png_uint_32 embedded_profile_len = png_get_uint_32(profile);

      if (profile_len != embedded_profile_len)
         png_error(png_ptr, "Profile length does not match profile");
   }

   name_len = png_check_keyword(png_ptr, name, new_name);

   if (name_len == 0)
      png_error(png_ptr, "iCCP: invalid keyword");

   new_name[++name_len] = PNG_COMPRESSION_TYPE_BASE;

   /* Make sure we include the NULL after the name and the compression type */
   ++name_len;

   png_text_compress_init(&comp, profile, profile_len);

   /* Allow for keyword terminator and compression byte */
   if (png_text_compress(png_ptr, png_iCCP, &comp, name_len) != Z_OK)
      png_error(png_ptr, png_ptr->zstream.msg);

   png_write_chunk_header(png_ptr, png_iCCP, name_len + comp.output_len);

   png_write_chunk_data(png_ptr, new_name, name_len);

   png_write_compressed_data_out(png_ptr, &comp);

   png_write_chunk_end(png_ptr);
}
#endif

#ifdef PNG_WRITE_sPLT_SUPPORTED
/* Write a sPLT chunk */
void /* PRIVATE */
png_write_sPLT(png_structrp png_ptr, png_const_sPLT_tp spalette)
{
   png_uint_32 name_len;
   png_byte new_name[80];
   png_byte entrybuf[10];
   png_size_t entry_size = (spalette->depth == 8 ? 6 : 10);
   png_size_t palette_size = entry_size * spalette->nentries;
   png_sPLT_entryp ep;

   png_debug(1, "in png_write_sPLT");

   name_len = png_check_keyword(png_ptr, spalette->name, new_name);

   if (name_len == 0)
      png_error(png_ptr, "sPLT: invalid keyword");

   /* Make sure we include the NULL after the name */
   png_write_chunk_header(png_ptr, png_sPLT,
       (png_uint_32)(name_len + 2 + palette_size));

   png_write_chunk_data(png_ptr, new_name, name_len + 1);

   png_write_chunk_data(png_ptr, &spalette->depth, 1);

   /* Loop through each palette entry, writing appropriately */
   for (ep = spalette->entries; ep<spalette->entries + spalette->nentries; ep++)
   {
      if (spalette->depth == 8)
      {
         entrybuf[0] = png_check_byte(png_ptr, ep->red);
         entrybuf[1] = png_check_byte(png_ptr, ep->green);
         entrybuf[2] = png_check_byte(png_ptr, ep->blue);
         entrybuf[3] = png_check_byte(png_ptr, ep->alpha);
         png_save_uint_16(entrybuf + 4, ep->frequency);
      }

      else
      {
         png_save_uint_16(entrybuf + 0, ep->red);
         png_save_uint_16(entrybuf + 2, ep->green);
         png_save_uint_16(entrybuf + 4, ep->blue);
         png_save_uint_16(entrybuf + 6, ep->alpha);
         png_save_uint_16(entrybuf + 8, ep->frequency);
      }

      png_write_chunk_data(png_ptr, entrybuf, entry_size);
   }

   png_write_chunk_end(png_ptr);
}
#endif

#ifdef PNG_WRITE_sBIT_SUPPORTED
/* Write the sBIT chunk */
void /* PRIVATE */
png_write_sBIT(png_structrp png_ptr, png_const_color_8p sbit, int color_type)
{
   png_byte buf[4];
   png_size_t size;

   png_debug(1, "in png_write_sBIT");

   /* Make sure we don't depend upon the order of PNG_COLOR_8 */
   if ((color_type & PNG_COLOR_MASK_COLOR) != 0)
   {
      unsigned int maxbits;

      maxbits = color_type==PNG_COLOR_TYPE_PALETTE ? 8 : png_ptr->bit_depth;

      if (sbit->red == 0 || sbit->red > maxbits ||
          sbit->green == 0 || sbit->green > maxbits ||
          sbit->blue == 0 || sbit->blue > maxbits)
      {
         png_app_error(png_ptr, "Invalid sBIT depth specified");
         return;
      }

      buf[0] = sbit->red;
      buf[1] = sbit->green;
      buf[2] = sbit->blue;
      size = 3;
   }

   else
   {
      if (sbit->gray == 0 || sbit->gray > png_ptr->bit_depth)
      {
         png_app_error(png_ptr, "Invalid sBIT depth specified");
         return;
      }

      buf[0] = sbit->gray;
      size = 1;
   }

   if ((color_type & PNG_COLOR_MASK_ALPHA) != 0)
   {
      if (sbit->alpha == 0 || sbit->alpha > png_ptr->bit_depth)
      {
         png_app_error(png_ptr, "Invalid sBIT depth specified");
         return;
      }

      buf[size++] = sbit->alpha;
   }

   png_write_complete_chunk(png_ptr, png_sBIT, buf, size);
}
#endif

#ifdef PNG_WRITE_cHRM_SUPPORTED
/* Write the cHRM chunk */
void /* PRIVATE */
png_write_cHRM_fixed(png_structrp png_ptr, const png_xy *xy)
{
   png_byte buf[32];

   png_debug(1, "in png_write_cHRM");

   /* Each value is saved in 1/100,000ths */
   if (png_save_int_31(png_ptr, buf,      xy->whitex) &&
       png_save_int_31(png_ptr, buf +  4, xy->whitey) &&
       png_save_int_31(png_ptr, buf +  8, xy->redx) &&
       png_save_int_31(png_ptr, buf + 12, xy->redy) &&
       png_save_int_31(png_ptr, buf + 16, xy->greenx) &&
       png_save_int_31(png_ptr, buf + 20, xy->greeny) &&
       png_save_int_31(png_ptr, buf + 24, xy->bluex) &&
       png_save_int_31(png_ptr, buf + 28, xy->bluey))
      png_write_complete_chunk(png_ptr, png_cHRM, buf, 32);
}
#endif

#ifdef PNG_WRITE_tRNS_SUPPORTED
/* Write the tRNS chunk */
void /* PRIVATE */
png_write_tRNS(png_structrp png_ptr, png_const_bytep trans_alpha,
    png_const_color_16p tran, int num_trans, int color_type)
{
   png_byte buf[6];

   png_debug(1, "in png_write_tRNS");

   if (color_type == PNG_COLOR_TYPE_PALETTE)
   {
      affirm(num_trans > 0 && num_trans <= PNG_MAX_PALETTE_LENGTH);
      if ((unsigned int)/*SAFE*/num_trans > png_ptr->num_palette)
      {
         /* This is an error which can only be reliably detected late. */
         png_app_error(png_ptr,
             "Invalid number of transparent colors specified");
         return;
      }

      {
#        ifdef PNG_WRITE_INVERT_ALPHA_SUPPORTED
            union
            {
               png_uint_32 u32[1];
               png_byte    b8[PNG_MAX_PALETTE_LENGTH];
            }  inverted_alpha;

            /* Invert the alpha channel (in tRNS) if required */
            if (png_ptr->write_invert_alpha)
            {
               int i;

               memcpy(inverted_alpha.b8, trans_alpha, num_trans);

               for (i=0; 4*i<num_trans; ++i)
                  inverted_alpha.u32[i] = ~inverted_alpha.u32[i];

               trans_alpha = inverted_alpha.b8;
            }
#        endif /* WRITE_INVERT_ALPHA */

         png_write_complete_chunk(png_ptr, png_tRNS, trans_alpha, num_trans);
      }
   }

   else if (color_type == PNG_COLOR_TYPE_GRAY)
   {
      /* One 16 bit value */
      affirm(tran->gray < (1 << png_ptr->bit_depth));
      png_save_uint_16(buf, tran->gray);
      png_write_complete_chunk(png_ptr, png_tRNS, buf, (png_size_t)2);
   }

   else if (color_type == PNG_COLOR_TYPE_RGB)
   {
      /* Three 16 bit values */
      png_save_uint_16(buf, tran->red);
      png_save_uint_16(buf + 2, tran->green);
      png_save_uint_16(buf + 4, tran->blue);
      affirm(png_ptr->bit_depth == 8 || (buf[0] | buf[2] | buf[4]) == 0);
      png_write_complete_chunk(png_ptr, png_tRNS, buf, (png_size_t)6);
   }

   else /* Already checked in png_set_tRNS */
      impossible("invalid tRNS");
}
#endif

#ifdef PNG_WRITE_bKGD_SUPPORTED
/* Write the background chunk */
void /* PRIVATE */
png_write_bKGD(png_structrp png_ptr, png_const_color_16p back, int color_type)
{
   png_byte buf[6];

   png_debug(1, "in png_write_bKGD");

   if (color_type == PNG_COLOR_TYPE_PALETTE)
   {
      if (
#        ifdef PNG_MNG_FEATURES_SUPPORTED
            (png_ptr->num_palette != 0 ||
            (png_ptr->mng_features_permitted & PNG_FLAG_MNG_EMPTY_PLTE) == 0) &&
#        endif /* MNG_FEATURES */
         back->index >= png_ptr->num_palette)
      {
         png_app_error(png_ptr, "Invalid background palette index");
         return;
      }

      buf[0] = back->index;
      png_write_complete_chunk(png_ptr, png_bKGD, buf, (png_size_t)1);
   }

   else if ((color_type & PNG_COLOR_MASK_COLOR) != 0)
   {
      png_save_uint_16(buf, back->red);
      png_save_uint_16(buf + 2, back->green);
      png_save_uint_16(buf + 4, back->blue);
#ifdef PNG_WRITE_16BIT_SUPPORTED
      if (png_ptr->bit_depth == 8 && (buf[0] | buf[2] | buf[4]) != 0)
#else
      if ((buf[0] | buf[2] | buf[4]) != 0)
#endif
      {
         png_app_error(png_ptr,
             "Ignoring attempt to write 16-bit bKGD chunk when bit_depth is 8");

         return;
      }

      png_write_complete_chunk(png_ptr, png_bKGD, buf, (png_size_t)6);
   }

   else
   {
      if (back->gray >= (1 << png_ptr->bit_depth))
      {
         png_app_error(png_ptr,
             "Ignoring attempt to write bKGD chunk out-of-range for bit_depth");

         return;
      }

      png_save_uint_16(buf, back->gray);
      png_write_complete_chunk(png_ptr, png_bKGD, buf, (png_size_t)2);
   }
}
#endif

#ifdef PNG_WRITE_hIST_SUPPORTED
/* Write the histogram */
void /* PRIVATE */
png_write_hIST(png_structrp png_ptr, png_const_uint_16p hist, int num_hist)
{
   int i;
   png_byte buf[3];

   png_debug(1, "in png_write_hIST");

   if (num_hist > (int)png_ptr->num_palette)
   {
      png_debug2(3, "num_hist = %d, num_palette = %d", num_hist,
          png_ptr->num_palette);

      png_warning(png_ptr, "Invalid number of histogram entries specified");
      return;
   }

   png_write_chunk_header(png_ptr, png_hIST, (png_uint_32)(num_hist * 2));

   for (i = 0; i < num_hist; i++)
   {
      png_save_uint_16(buf, hist[i]);
      png_write_chunk_data(png_ptr, buf, (png_size_t)2);
   }

   png_write_chunk_end(png_ptr);
}
#endif

#ifdef PNG_WRITE_tEXt_SUPPORTED
/* Write a tEXt chunk */
void /* PRIVATE */
png_write_tEXt(png_structrp png_ptr, png_const_charp key, png_const_charp text,
    png_size_t text_len)
{
   png_uint_32 key_len;
   png_byte new_key[80];

   png_debug(1, "in png_write_tEXt");

   key_len = png_check_keyword(png_ptr, key, new_key);

   if (key_len == 0)
      png_error(png_ptr, "tEXt: invalid keyword");

   if (text == NULL || *text == '\0')
      text_len = 0;

   else
      text_len = strlen(text);

   if (text_len > PNG_UINT_31_MAX - (key_len+1))
      png_error(png_ptr, "tEXt: text too long");

   /* Make sure we include the 0 after the key */
   png_write_chunk_header(png_ptr, png_tEXt,
       (png_uint_32)/*checked above*/(key_len + text_len + 1));
   /*
    * We leave it to the application to meet PNG-1.0 requirements on the
    * contents of the text.  PNG-1.0 through PNG-1.2 discourage the use of
    * any non-Latin-1 characters except for NEWLINE.  ISO PNG will forbid them.
    * The NUL character is forbidden by PNG-1.0 through PNG-1.2 and ISO PNG.
    */
   png_write_chunk_data(png_ptr, new_key, key_len + 1);

   if (text_len != 0)
      png_write_chunk_data(png_ptr, (png_const_bytep)text, text_len);

   png_write_chunk_end(png_ptr);
}
#endif

#ifdef PNG_WRITE_zTXt_SUPPORTED
/* Write a compressed text chunk */
void /* PRIVATE */
png_write_zTXt(png_structrp png_ptr, png_const_charp key, png_const_charp text,
    int compression)
{
   png_uint_32 key_len;
   png_byte new_key[81];
   compression_state comp;

   png_debug(1, "in png_write_zTXt");

   if (compression == PNG_TEXT_COMPRESSION_NONE)
   {
      png_write_tEXt(png_ptr, key, text, 0);
      return;
   }

   if (compression != PNG_TEXT_COMPRESSION_zTXt)
      png_error(png_ptr, "zTXt: invalid compression type");

   key_len = png_check_keyword(png_ptr, key, new_key);

   if (key_len == 0)
      png_error(png_ptr, "zTXt: invalid keyword");

   /* Add the compression method and 1 for the keyword separator. */
   new_key[++key_len] = PNG_COMPRESSION_TYPE_BASE;
   ++key_len;

   /* Compute the compressed data; do it now for the length */
   png_text_compress_init(&comp, (png_const_bytep)text,
      text == NULL ? 0 : strlen(text));

   if (png_text_compress(png_ptr, png_zTXt, &comp, key_len) != Z_OK)
      png_error(png_ptr, png_ptr->zstream.msg);

   /* Write start of chunk */
   png_write_chunk_header(png_ptr, png_zTXt, key_len + comp.output_len);

   /* Write key */
   png_write_chunk_data(png_ptr, new_key, key_len);

   /* Write the compressed data */
   png_write_compressed_data_out(png_ptr, &comp);

   /* Close the chunk */
   png_write_chunk_end(png_ptr);
}
#endif

#ifdef PNG_WRITE_iTXt_SUPPORTED
/* Write an iTXt chunk */
void /* PRIVATE */
png_write_iTXt(png_structrp png_ptr, int compression, png_const_charp key,
    png_const_charp lang, png_const_charp lang_key, png_const_charp text)
{
   png_uint_32 key_len, prefix_len;
   png_size_t lang_len, lang_key_len;
   png_byte new_key[82];
   compression_state comp;

   png_debug(1, "in png_write_iTXt");

   key_len = png_check_keyword(png_ptr, key, new_key);

   if (key_len == 0)
      png_error(png_ptr, "iTXt: invalid keyword");

   /* Set the compression flag */
   switch (compression)
   {
      case PNG_ITXT_COMPRESSION_NONE:
      case PNG_TEXT_COMPRESSION_NONE:
         compression = new_key[++key_len] = 0; /* no compression */
         break;

      case PNG_TEXT_COMPRESSION_zTXt:
      case PNG_ITXT_COMPRESSION_zTXt:
         compression = new_key[++key_len] = 1; /* compressed */
         break;

      default:
         png_error(png_ptr, "iTXt: invalid compression");
   }

   new_key[++key_len] = PNG_COMPRESSION_TYPE_BASE;
   ++key_len; /* for the keywod separator */

   /* We leave it to the application to meet PNG-1.0 requirements on the
    * contents of the text.  PNG-1.0 through PNG-1.2 discourage the use of
    * any non-Latin-1 characters except for NEWLINE.  ISO PNG, however,
    * specifies that the text is UTF-8 and this really doesn't require any
    * checking.
    *
    * The NUL character is forbidden by PNG-1.0 through PNG-1.2 and ISO PNG.
    *
    * TODO: validate the language tag correctly (see the spec.)
    */
   if (lang == NULL) lang = ""; /* empty language is valid */
   lang_len = strlen(lang)+1;
   if (lang_key == NULL) lang_key = ""; /* may be empty */
   lang_key_len = strlen(lang_key)+1;
   if (text == NULL) text = ""; /* may be empty */

   prefix_len = key_len;
   if (lang_len > PNG_UINT_31_MAX-prefix_len)
      prefix_len = PNG_UINT_31_MAX;
   else
      prefix_len = (png_uint_32)(prefix_len + lang_len);

   if (lang_key_len > PNG_UINT_31_MAX-prefix_len)
      prefix_len = PNG_UINT_31_MAX;
   else
      prefix_len = (png_uint_32)(prefix_len + lang_key_len);

   png_text_compress_init(&comp, (png_const_bytep)text, strlen(text));

   if (compression != 0)
   {
      if (png_text_compress(png_ptr, png_iTXt, &comp, prefix_len) != Z_OK)
         png_error(png_ptr, png_ptr->zstream.msg);
   }

   else
   {
      if (comp.input_len > PNG_UINT_31_MAX-prefix_len)
         png_error(png_ptr, "iTXt: uncompressed text too long");

      /* So the string will fit in a chunk: */
      comp.output_len = (png_uint_32)/*SAFE*/comp.input_len;
   }

   png_write_chunk_header(png_ptr, png_iTXt, comp.output_len + prefix_len);

   png_write_chunk_data(png_ptr, new_key, key_len);

   png_write_chunk_data(png_ptr, (png_const_bytep)lang, lang_len);

   png_write_chunk_data(png_ptr, (png_const_bytep)lang_key, lang_key_len);

   if (compression != 0)
      png_write_compressed_data_out(png_ptr, &comp);

   else
      png_write_chunk_data(png_ptr, (png_const_bytep)text, comp.output_len);

   png_write_chunk_end(png_ptr);
}
#endif /* WRITE_iTXt */

#if defined(PNG_WRITE_oFFs_SUPPORTED) ||\
    defined(PNG_WRITE_pCAL_SUPPORTED)
/* PNG signed integers are saved in 32-bit 2's complement format.  ANSI C-90
 * defines a cast of a signed integer to an unsigned integer either to preserve
 * the value, if it is positive, or to calculate:
 *
 *     (UNSIGNED_MAX+1) + integer
 *
 * Where UNSIGNED_MAX is the appropriate maximum unsigned value, so when the
 * negative integral value is added the result will be an unsigned value
 * correspnding to the 2's complement representation.
 */
static int
save_int_32(png_structrp png_ptr, png_bytep buf, png_int_32 j)
{
   png_uint_32 i = 0xFFFFFFFFU & (png_uint_32)/*SAFE & CORRECT*/j;

   if (i != 0x80000000U/*value not permitted*/)
   {
      png_save_uint_32(buf, i);
      return 1;
   }

   else
   {
      png_chunk_report(png_ptr, "invalid value in oFFS or pCAL",
         PNG_CHUNK_WRITE_ERROR);
      return 0;
   }
}
#endif /* WRITE_oFFs || WRITE_pCAL */

#ifdef PNG_WRITE_oFFs_SUPPORTED
/* Write the oFFs chunk */
void /* PRIVATE */
png_write_oFFs(png_structrp png_ptr, png_int_32 x_offset, png_int_32 y_offset,
    int unit_type)
{
   png_byte buf[9];

   png_debug(1, "in png_write_oFFs");

   if (unit_type >= PNG_OFFSET_LAST)
      png_warning(png_ptr, "Unrecognized unit type for oFFs chunk");

   if (save_int_32(png_ptr, buf, x_offset) &&
       save_int_32(png_ptr, buf + 4, y_offset))
   {
      /* unit type is 0 or 1, this has been checked already so the following
       * is safe:
       */
      buf[8] = unit_type != 0;
      png_write_complete_chunk(png_ptr, png_oFFs, buf, (png_size_t)9);
   }
}
#endif /* WRITE_oFFs */

#ifdef PNG_WRITE_pCAL_SUPPORTED
/* Write the pCAL chunk (described in the PNG extensions document) */
void /* PRIVATE */
png_write_pCAL(png_structrp png_ptr, png_charp purpose, png_int_32 X0,
    png_int_32 X1, int type, int nparams, png_const_charp units,
    png_charpp params)
{
   png_uint_32 purpose_len;
   size_t units_len;
   png_byte buf[10];
   png_byte new_purpose[80];

   png_debug1(1, "in png_write_pCAL (%d parameters)", nparams);

   if (type >= PNG_EQUATION_LAST)
      png_error(png_ptr, "Unrecognized equation type for pCAL chunk");

   purpose_len = png_check_keyword(png_ptr, purpose, new_purpose);

   if (purpose_len == 0)
      png_error(png_ptr, "pCAL: invalid keyword");

   ++purpose_len; /* terminator */

   png_debug1(3, "pCAL purpose length = %d", (int)purpose_len);
   units_len = strlen(units) + (nparams == 0 ? 0 : 1);
   png_debug1(3, "pCAL units length = %d", (int)units_len);

   if (save_int_32(png_ptr, buf, X0) &&
       save_int_32(png_ptr, buf + 4, X1))
   {
      png_size_tp params_len = png_voidcast(png_size_tp,
         png_malloc(png_ptr, nparams * sizeof (png_size_t)));
      int i;
      size_t total_len = purpose_len + units_len + 10;

      /* Find the length of each parameter, making sure we don't count the
       * null terminator for the last parameter.
       */
      for (i = 0; i < nparams; i++)
      {
         params_len[i] = strlen(params[i]) + (i == nparams - 1 ? 0 : 1);
         png_debug2(3, "pCAL parameter %d length = %lu", i,
             (unsigned long)params_len[i]);
         total_len += params_len[i];
      }

      png_debug1(3, "pCAL total length = %d", (int)total_len);
      png_write_chunk_header(png_ptr, png_pCAL, (png_uint_32)total_len);
      png_write_chunk_data(png_ptr, new_purpose, purpose_len);
      buf[8] = png_check_byte(png_ptr, type);
      buf[9] = png_check_byte(png_ptr, nparams);
      png_write_chunk_data(png_ptr, buf, (png_size_t)10);
      png_write_chunk_data(png_ptr, (png_const_bytep)units,
            (png_size_t)units_len);

      for (i = 0; i < nparams; i++)
         png_write_chunk_data(png_ptr, (png_const_bytep)params[i],
            params_len[i]);

      png_free(png_ptr, params_len);
      png_write_chunk_end(png_ptr);
   }
}
#endif /* WRITE_pCAL */

#ifdef PNG_WRITE_sCAL_SUPPORTED
/* Write the sCAL chunk */
void /* PRIVATE */
png_write_sCAL_s(png_structrp png_ptr, int unit, png_const_charp width,
    png_const_charp height)
{
   png_byte buf[64];
   png_size_t wlen, hlen, total_len;

   png_debug(1, "in png_write_sCAL_s");

   wlen = strlen(width);
   hlen = strlen(height);
   total_len = wlen + hlen + 2;

   if (total_len > 64)
   {
      png_warning(png_ptr, "Can't write sCAL (buffer too small)");
      return;
   }

   buf[0] = png_check_byte(png_ptr, unit);
   memcpy(buf + 1, width, wlen + 1);      /* Append the '\0' here */
   memcpy(buf + wlen + 2, height, hlen);  /* Do NOT append the '\0' here */

   png_debug1(3, "sCAL total length = %u", (unsigned int)total_len);
   png_write_complete_chunk(png_ptr, png_sCAL, buf, total_len);
}
#endif

#ifdef PNG_WRITE_pHYs_SUPPORTED
/* Write the pHYs chunk */
void /* PRIVATE */
png_write_pHYs(png_structrp png_ptr, png_uint_32 x_pixels_per_unit,
    png_uint_32 y_pixels_per_unit,
    int unit_type)
{
   png_byte buf[9];

   png_debug(1, "in png_write_pHYs");

   if (unit_type >= PNG_RESOLUTION_LAST)
      png_warning(png_ptr, "Unrecognized unit type for pHYs chunk");

   png_save_uint_32(buf, x_pixels_per_unit);
   png_save_uint_32(buf + 4, y_pixels_per_unit);
   buf[8] = png_check_byte(png_ptr, unit_type);

   png_write_complete_chunk(png_ptr, png_pHYs, buf, (png_size_t)9);
}
#endif

#ifdef PNG_WRITE_tIME_SUPPORTED
/* Write the tIME chunk.  Use either png_convert_from_struct_tm()
 * or png_convert_from_time_t(), or fill in the structure yourself.
 */
void /* PRIVATE */
png_write_tIME(png_structrp png_ptr, png_const_timep mod_time)
{
   png_byte buf[7];

   png_debug(1, "in png_write_tIME");

   if (mod_time->month  > 12 || mod_time->month  < 1 ||
       mod_time->day    > 31 || mod_time->day    < 1 ||
       mod_time->hour   > 23 || mod_time->second > 60)
   {
      png_warning(png_ptr, "Invalid time specified for tIME chunk");
      return;
   }

   png_save_uint_16(buf, mod_time->year);
   buf[2] = mod_time->month;
   buf[3] = mod_time->day;
   buf[4] = mod_time->hour;
   buf[5] = mod_time->minute;
   buf[6] = mod_time->second;

   png_write_complete_chunk(png_ptr, png_tIME, buf, (png_size_t)7);
}
#endif

static void
write_filtered_row(png_structrp png_ptr, png_const_bytep filtered_row,
   png_uint_32 row_bytes, png_byte filter /*if at start of row*/,
   int end_of_image)
{
   /* This handles writing a row that has been filtered, or did not need to be
    * filtered.  If the data row has a partial pixel it must have been handled
    * correctly in the caller; filters generate a full 8 bits even if the pixel
    * only has one significant bit!
    */
   debug(row_bytes > 0);

   if (filter < PNG_FILTER_VALUE_LAST) /* start of row */
   {
      png_byte buffer[1];
      
      buffer[0] = filter;
      png_compress_IDAT(png_ptr, buffer, 1U/*len*/, Z_NO_FLUSH);
   }

   png_compress_IDAT(png_ptr, filtered_row, row_bytes,
         end_of_image ? Z_FINISH : Z_NO_FLUSH);
}

static void
write_unfiltered_rowbits(png_structrp png_ptr, png_const_bytep filtered_row,
   png_uint_32 row_bits, png_byte filter /*if at start of row*/,
   int end_of_image)
{
   /* Same as above, but it correctly clears the unused bits in a partial
    * byte.
    */
   const png_uint_32 row_bytes = row_bits >> 3;

   debug(filter == PNG_FILTER_VALUE_NONE || filter == PNG_FILTER_VALUE_LAST);

   if (row_bytes > 0U)
   {
      row_bits -= row_bytes << 3;
      write_filtered_row(png_ptr, filtered_row, row_bytes, filter,
            end_of_image && row_bits == 0U);
      filter = PNG_FILTER_VALUE_LAST; /* written */
   }

   /* Handle a partial byte. */
   if (row_bits > 0U)
   {
      png_byte buffer[1];

      buffer[0] = PNG_BYTE(filtered_row[row_bytes] & ~(0xFFU >> row_bits));
      write_filtered_row(png_ptr, buffer, 1U, filter, end_of_image);
   }
}

#ifdef PNG_WRITE_FILTER_SUPPORTED
static void
filter_block_singlebyte(png_alloc_size_t row_bytes, png_bytep sub_row,
   png_bytep up_row, png_bytep avg_row, png_bytep paeth_row,
   png_const_bytep row, png_const_bytep prev_row, png_bytep prev_pixels)
{
   /* Calculate rows for all four filters where the input has one byte per pixel
    * (more accurately per filter-unit).
    */
   png_byte a = prev_pixels[0];
   png_byte c = prev_pixels[1];

   while (row_bytes-- > 0U)
   {
      const png_byte x = *row++;
      const png_byte b = prev_row == NULL ? 0U : *prev_row++;

      /* Calculate each filtered byte in turn: */
      if (sub_row != NULL) *sub_row++ = 0xFFU & (x - a);
      if (up_row != NULL) *up_row++ = 0xFFU & (x - b);
      if (avg_row != NULL) *avg_row++ = 0xFFU & (x - (a+b)/2U);

      /* Paeth is a little more difficult: */
      if (paeth_row != NULL)
      {
         int pa = b-c;   /* a+b-c - a */
         int pb = a-c;   /* a+b-c - b */
         int pc = pa+pb; /* a+b-c - c = b-c + a-c */
         png_byte p = a;

         pa = abs(pa);
         pb = abs(pb);
         if (pa > pb) pa = pb, p = b;
         if (pa > abs(pc)) p = c;

         *paeth_row++ = 0xFFU & (x - p);
      }

      /* And set a and c for the next pixel: */
      a = x;
      c = b;
   }

   /* Store a and c for the next block: */
   prev_pixels[0] = a;
   prev_pixels[1] = c;
}

static void
filter_block_multibyte(png_alloc_size_t row_bytes,
   const unsigned int bpp, png_bytep sub_row, png_bytep up_row,
   png_bytep avg_row, png_bytep paeth_row, png_const_bytep row,
   png_const_bytep prev_row, png_bytep prev_pixels)
{
   /* Calculate rows for all four filters, the input is a block of bytes such
    * that row_bytes is a multiple of bpp.  bpp can be 2, 3, 4, 6 or 8.
    * prev_pixels will be updated to the last pixels processed.
    */
   while (row_bytes >= bpp)
   {
      unsigned int i;

      for (i=0; i<bpp; ++i)
      {
         const png_byte a = prev_pixels[i];
         const png_byte c = prev_pixels[i+bpp];
         const png_byte b = prev_row == NULL ? 0U : *prev_row++;
         const png_byte x = *row++;

         /* Save for the next pixel: */
         prev_pixels[i] = x;
         prev_pixels[i+bpp] = b;

         /* Calculate each filtered byte in turn: */
         if (sub_row != NULL) *sub_row++ = 0xFFU & (x - a);
         if (up_row != NULL) *up_row++ = 0xFFU & (x - b);
         if (avg_row != NULL) *avg_row++ = 0xFFU & (x - (a+b)/2U);

         /* Paeth is a little more difficult: */
         if (paeth_row != NULL)
         {
            int pa = b-c;   /* a+b-c - a */
            int pb = a-c;   /* a+b-c - b */
            int pc = pa+pb; /* a+b-c - c = b-c + a-c */
            png_byte p = a;

            pa = abs(pa);
            pb = abs(pb);
            if (pa > pb) pa = pb, p = b;
            if (pa > abs(pc)) p = c;

            *paeth_row++ = 0xFFU & (x - p);
         }
      }

      row_bytes -= i;
   }
}

static void
filter_row(png_structrp png_ptr, png_const_bytep prev_row,
      png_bytep prev_pixels, png_const_bytep unfiltered_row,
      const png_uint_32 row_bits, unsigned int bpp, unsigned int filters_to_try,
      int start_of_row, int end_of_image)
{
   /* filters_to_try identifies a single filter and it is not PNG_FILTER_NONE.
    */
   png_uint_32 row_bytes = row_bits >> 3; /* complete bytes */
   png_byte filter = PNG_FILTER_VALUE_LAST /* not at start */;
   png_byte filtered_row[PNG_ROW_BUFFER_SIZE];

   debug((row_bits % bpp) == 0U);

   if (start_of_row) switch (filters_to_try)
   {
      case PNG_FILTER_SUB:   filter = PNG_FILTER_VALUE_SUB;   break;
      case PNG_FILTER_UP:    filter = PNG_FILTER_VALUE_UP;    break;
      case PNG_FILTER_AVG:   filter = PNG_FILTER_VALUE_AVG;   break;
      case PNG_FILTER_PAETH: filter = PNG_FILTER_VALUE_PAETH; break;
      default:
         impossible("filter list");
   }

   if (bpp <= 8U)
   {
      /* There may be a partial byte at the end. */
      if (row_bytes > 0)
         filter_block_singlebyte(row_bytes,
            filters_to_try & PNG_FILTER_SUB   ? filtered_row : NULL,
            filters_to_try & PNG_FILTER_UP    ? filtered_row : NULL,
            filters_to_try & PNG_FILTER_AVG   ? filtered_row : NULL,
            filters_to_try & PNG_FILTER_PAETH ? filtered_row : NULL,
            unfiltered_row, prev_row, prev_pixels);

      /* The partial byte must be handled correctly here; both the previous row
       * value and the current value need to have non-present bits cleared.
       */
      if ((row_bits & 7U) != 0)
      {
         const png_byte mask = PNG_BYTE(~(0xFFU >> (row_bits & 7U)));
         png_byte buffer[2];

         buffer[0] = unfiltered_row[row_bytes] & mask;

         if (prev_row != NULL)
            buffer[1U] = prev_row[row_bytes] & mask;

         else
            buffer[1U] = 0U;

         filter_block_singlebyte(1U,
            filters_to_try & PNG_FILTER_SUB   ? filtered_row+row_bytes : NULL,
            filters_to_try & PNG_FILTER_UP    ? filtered_row+row_bytes : NULL,
            filters_to_try & PNG_FILTER_AVG   ? filtered_row+row_bytes : NULL,
            filters_to_try & PNG_FILTER_PAETH ? filtered_row+row_bytes : NULL,
            buffer, buffer+1U, prev_pixels);

         ++row_bytes; /* for write_filtered_row below */
      }
   }

   else
   {
      debug((bpp & 7U) == 0U && row_bits == (row_bytes << 3));
      filter_block_multibyte(row_bytes, bpp >> 3,
            filters_to_try & PNG_FILTER_SUB   ? filtered_row : NULL,
            filters_to_try & PNG_FILTER_UP    ? filtered_row : NULL,
            filters_to_try & PNG_FILTER_AVG   ? filtered_row : NULL,
            filters_to_try & PNG_FILTER_PAETH ? filtered_row : NULL,
            unfiltered_row, prev_row, prev_pixels);
   }

   write_filtered_row(png_ptr, filtered_row, row_bytes, filter, end_of_image);
}

static void
find_filter(png_structrp png_ptr, png_const_bytep prev_row,
   png_bytep prev_pixels, png_const_bytep unfiltered_row,
   png_uint_32 row_bits, unsigned int bpp, unsigned int filters_to_try,
   int start_of_row, int end_of_image)
{
   /* filters_to_try identifies multiple filters, up to all five. */
   /* TODO: reimplement this, currently this just selects the first filter */
   filters_to_try &= -filters_to_try;
   if (filters_to_try == PNG_FILTER_NONE)
      write_unfiltered_rowbits(png_ptr, unfiltered_row, row_bits,
            start_of_row ? PNG_FILTER_VALUE_NONE : PNG_FILTER_VALUE_LAST,
            end_of_image);

   else
      filter_row(png_ptr, prev_row, prev_pixels, unfiltered_row, row_bits, bpp,
         filters_to_try & -filters_to_try, start_of_row, end_of_image);
}

/* This filters the row, chooses which filter to use, if it has not already
 * been specified by the application, and then writes the row out with the
 * chosen filter.
 */
unsigned int /* PRIVATE */
png_write_filter_row(png_structrp png_ptr, png_bytep prev_pixels,
      png_const_bytep unfiltered_row, png_uint_32 x,
      png_uint_32 width/*pixels*/, int first_row_in_pass, int last_pass_row,
      unsigned int filters_to_try, int end_of_image)
{
   png_bytep prev_row = png_ptr->row_buffer;
   const unsigned int bpp = png_ptr->row_output_pixel_depth;
   const png_uint_32 row_bits = width * bpp;

   /* These invariants are expected from the caller: */
   affirm(width < 65536U && bpp <= 64U && row_bits <= 8U*PNG_ROW_BUFFER_SIZE);

   if (x == 0U) /* start of row */
   {
      /* Delaying initialization of the filter stuff. */
      if (png_ptr->filter_mask == 0U)
         png_set_filter(png_ptr, PNG_FILTER_TYPE_BASE, PNG_ALL_FILTERS);

      /* Now work out the filters to try for this row: */
      filters_to_try = png_ptr->filter_mask; /* else caller must preserve */

      /* If this has a previous row filter in the set to try ensure the row
       * buffer exists and ensure it is empty when first allocated and at
       * the start of the pass.
       */
      if ((filters_to_try & (PNG_FILTER_UP|PNG_FILTER_AVG|PNG_FILTER_PAETH))
            != 0U)
      {
         if (prev_row == NULL)
         {
            /* Just allocate for the total output row bytes; a three-row
             * interlaced image requires less, but this is safe.
             */
            prev_row = png_voidcast(png_bytep, png_malloc(png_ptr,
                     png_calc_rowbytes(png_ptr, bpp, png_ptr->width)));
            png_ptr->row_buffer = prev_row;

            /* If that buffer would have been required for this row issue an
             * app warning and disable the filters that would have required
             * the data.
             */
            if (!first_row_in_pass)
            {
               png_app_warning(png_ptr, "Previous row filters ignored");
               /* And always turn off the filters, to prevent using
                * uninitialized data.
                */
               filters_to_try &= PNG_BIC_MASK(
                     PNG_FILTER_UP|PNG_FILTER_AVG|PNG_FILTER_PAETH);

               if (filters_to_try == 0U)
                  filters_to_try = PNG_FILTER_NONE;
            }
         }
      }

      /* The filters are pre-calculated in png_set_filter, however if the
       * image is interlaced some passes may still be too narrow or short to
       * allow certain filters.  In any case the first row of the pass
       * doesn't need to consider PAETH or UP (AVG is still different).
       */
      if (first_row_in_pass)
      {
         if ((filters_to_try & PNG_FILTER_UP) != 0U)
         {
            filters_to_try &= PNG_BIC_MASK(PNG_FILTER_UP);
            filters_to_try |= PNG_FILTER_NONE;
         }

         if ((filters_to_try & PNG_FILTER_PAETH) != 0U)
         {
            filters_to_try &= PNG_BIC_MASK(PNG_FILTER_PAETH);
            filters_to_try |= PNG_FILTER_SUB/*equialent to PAETH here*/;
         }

         /* If this leaves the AVG filter it will be used on the first row
          * this is handled in the filter implementation by setting prev_row
          * to NULL below.
          */
      }

      /* Check for a narrow image; the blocking will never return just one
       * pixel at the start unless the pass is only one pixel wide, this test
       * needs to happen after the one above on PAETH:
       */
      if (width == 1U)
      {
         if ((filters_to_try & PNG_FILTER_SUB) != 0U)
         {
            filters_to_try &= PNG_BIC_MASK(PNG_FILTER_SUB);
            filters_to_try |= PNG_FILTER_NONE;
         }
      }
   } /* start of row */

   else if (prev_row != NULL)
   {
      /* Advance prev_row to the corresponding pixel above row[x], must use
       * png_calc_rowbytes here otherwise the calculation using x might
       * overflow.
       */
      debug(((x * bpp) & 7U) == 0U);
      prev_row += png_calc_rowbytes(png_ptr, bpp, x);
   }

   /* Now choose the correct filter implementation according to the number of
    * filters in the filters_to_try list.  The prev_row parameter is made NULL
    * on the first row because it is uninitialized at that point.
    */
   if (filters_to_try == PNG_FILTER_NONE)
      write_unfiltered_rowbits(png_ptr, unfiltered_row, row_bits,
            x == 0 ? PNG_FILTER_VALUE_NONE : PNG_FILTER_VALUE_LAST,
            end_of_image);

   else if ((filters_to_try & -filters_to_try) == filters_to_try) /* 1 filter */
      filter_row(png_ptr, first_row_in_pass ? NULL : prev_row,
            prev_pixels, unfiltered_row, row_bits, bpp, filters_to_try, x == 0,
            end_of_image);

   else
      find_filter(png_ptr, first_row_in_pass ? NULL : prev_row,
            prev_pixels, unfiltered_row, row_bits, bpp, filters_to_try, x == 0,
            end_of_image);

   /* Copy the current row into the previous row buffer, if available, unless
    * this is the last row in the pass, when there is no point.  Note that
    * prev_row may have garbage in a partial byte at the end.
    */
   if (prev_row != NULL && !last_pass_row)
      memcpy(prev_row, unfiltered_row, (row_bits + 7U) >> 3);

   return filters_to_try;
}

/* Allow the application to select one or more row filters to use. */
void PNGAPI
png_set_filter(png_structrp png_ptr, int method, int filtersIn)
{
   unsigned int filters;

   png_debug(1, "in png_set_filter");

   if (png_ptr == NULL)
      return;

   if (png_ptr->read_struct)
   {
      png_app_error(png_ptr, "png_set_filter: cannot be used when reading");
      return;
   }

   if (method != png_ptr->filter_method)
   {
      png_app_error(png_ptr, "png_set_filter: method does not match IHDR");
      return;
   }

   /* PNG and MNG use the same base adaptive filter types: */
   if (method != PNG_FILTER_TYPE_BASE && method != PNG_INTRAPIXEL_DIFFERENCING)
   {
      png_app_error(png_ptr, "png_set_filter: unsupported method");
      return;
   }

   /* Notice that PNG_NO_FILTERS is 0 and passes this test; this is OK
    * because filters then gets set to PNG_FILTER_NONE, as is required.
    */
   if (filtersIn >= 0 && filtersIn < PNG_FILTER_VALUE_LAST)
      filters = 8U << filtersIn;

   else if ((filtersIn & PNG_BIC_MASK(PNG_ALL_FILTERS)) == 0)
      filters = filtersIn & PNG_ALL_FILTERS;

   else
   {
      png_app_error(png_ptr, "png_set_filter: invalid filters mask/value");

      /* Prior to 1.7.0 this ignored the error and just used the bits that
       * are present, now it does nothing; this seems a lot safer.
       */
      return;
   }

   /* New in 1.7.0: adjust the mask according to the image characteristics.
    * This used to happen on every row, doing it here means that these checks
    * happen only once every png_set_filter call, or once per image.
    */
   if (filters != PNG_FILTER_NONE)
   {
      /* Test to see if there are enough rows to allow previous-row filters to
       * work.  Note that the AVG filter is still significant because it uses
       * half the value of the previous pixel as the predictor, but it is
       * ignored in this case.
       */
      if (png_ptr->height <= (png_ptr->interlaced == PNG_INTERLACE_NONE ? 1U :
               (png_ptr->width == 1U ? 3U : 2U)))
      {
         /* Replace 'up' by the equivalent 'none': */
         if ((filters & (PNG_FILTER_UP)) != 0)
         {
            filters &= PNG_BIC_MASK(PNG_FILTER_UP);
            filters |= PNG_FILTER_NONE;
         }

         /* Replace 'paeth' by the equivalent 'sub': */
         if ((filters & PNG_FILTER_PAETH) != 0)
         {
            filters &= PNG_BIC_MASK(PNG_FILTER_PAETH);
            filters |= PNG_FILTER_SUB;
         }

         /* Remove 'avg' unless it is the only filter in which case 'none' is
          * used.  (This chooses compression speed of very short images over a
          * probably pointless compression option for a one line image; short
          * images are common, the sub-case which benefits from AVG is not.
          */
         if ((filters & PNG_FILTER_AVG) != 0)
         {
            filters &= PNG_BIC_MASK(PNG_FILTER_AVG);
            if (filters == 0U)
               filters |= PNG_FILTER_NONE;
         }
      }

      /* Also check for SUB on narrow images; it's equivalent to NONE on the
       * first pixel.
       */
      if (png_ptr->width <= (png_ptr->interlaced == PNG_INTERLACE_NONE ? 1U :
               (png_ptr->height == 1U ? 3U : 1U)))
      {
         if ((filters & PNG_FILTER_SUB) != 0)
         {
            filters &= PNG_BIC_MASK(PNG_FILTER_SUB);
            filters |= PNG_FILTER_NONE;
         }
      }
   }

   debug(filters != 0U && (filters & PNG_BIC_MASK(PNG_ALL_FILTERS)) == 0U);

   png_ptr->filter_mask = png_check_bits(png_ptr, filters, 8);
}
#else /* !WRITE_FILTER */
unsigned int /* PRIVATE */
png_write_filter_row(png_structrp png_ptr, png_bytep prev_pixels,
      png_const_bytep unfiltered_row, png_uint_32 x,
      png_uint_32 width/*pixels*/, int first_row_in_pass, int last_pass_row,
      unsigned int filters_to_try/*from previous call*/, int end_of_image)
{
   const unsigned int bpp = png_ptr->row_output_pixel_depth;
   png_uint_32 row_bits;

   row_bits = width;
   row_bits *= bpp;
   /* These invariants are expected from the caller: */
   affirm(width < 65536U && bpp <= 64U && row_bits <= 8U*PNG_ROW_BUFFER_SIZE);

   write_unfiltered_rowbits(png_ptr, unfiltered_row, row_bits,
         x == 0 ? PNG_FILTER_VALUE_NONE : PNG_FILTER_VALUE_LAST, end_of_image);

   return filters_to_try;

   PNG_UNUSED(first_row_in_pass);
   PNG_UNUSED(prev_pixels);
   PNG_UNUSED(last_pass_row);
}
#endif /* !WRITE_FILTER */

#ifdef PNG_WRITE_WEIGHTED_FILTER_SUPPORTED      /* GRR 970116 */
/* Legacy API that weighted the filter metric by the number of times it had been
 * used before.
 */
#ifdef PNG_FLOATING_POINT_SUPPORTED
PNG_FUNCTION(void,PNGAPI
png_set_filter_heuristics,(png_structrp png_ptr, int heuristic_method,
    int num_weights, png_const_doublep filter_weights,
    png_const_doublep filter_costs),PNG_DEPRECATED)
{
   png_app_warning(png_ptr, "weighted filter heuristics not implemented");
   PNG_UNUSED(heuristic_method)
   PNG_UNUSED(num_weights)
   PNG_UNUSED(filter_weights)
   PNG_UNUSED(filter_costs)
}
#endif /* FLOATING_POINT */

#ifdef PNG_FIXED_POINT_SUPPORTED
PNG_FUNCTION(void,PNGAPI
png_set_filter_heuristics_fixed,(png_structrp png_ptr, int heuristic_method,
    int num_weights, png_const_fixed_point_p filter_weights,
    png_const_fixed_point_p filter_costs),PNG_DEPRECATED)
{
   png_app_warning(png_ptr, "weighted filter heuristics not implemented");
   PNG_UNUSED(heuristic_method)
   PNG_UNUSED(num_weights)
   PNG_UNUSED(filter_weights)
   PNG_UNUSED(filter_costs)
}
#endif /* FIXED_POINT */
#endif /* WRITE_WEIGHTED_FILTER */

#ifdef PNG_WRITE_CUSTOMIZE_COMPRESSION_SUPPORTED
void PNGAPI
png_set_compression_level(png_structrp png_ptr, int level)
{
   png_debug(1, "in png_set_compression_level");

   if (png_ptr == NULL)
      return;

   png_ptr->zlib_level = level;
}

void PNGAPI
png_set_compression_mem_level(png_structrp png_ptr, int mem_level)
{
   png_debug(1, "in png_set_compression_mem_level");

   if (png_ptr == NULL)
      return;

   png_ptr->zlib_mem_level = mem_level;
}

void PNGAPI
png_set_compression_strategy(png_structrp png_ptr, int strategy)
{
   png_debug(1, "in png_set_compression_strategy");

   if (png_ptr == NULL)
      return;

   /* The flag setting here prevents the libpng dynamic selection of strategy.
    */
   png_ptr->flags |= PNG_FLAG_ZLIB_CUSTOM_STRATEGY;
   png_ptr->zlib_strategy = strategy;
}

/* If PNG_WRITE_OPTIMIZE_CMF_SUPPORTED is defined, libpng will use a
 * smaller value of window_bits if it can do so safely.
 */
void PNGAPI
png_set_compression_window_bits(png_structrp png_ptr, int window_bits)
{
   if (png_ptr == NULL)
      return;

   /* Prior to 1.6.0 this would warn but then set the window_bits value. This
    * meant that negative window bits values could be selected that would cause
    * libpng to write a non-standard PNG file with raw deflate or gzip
    * compressed IDAT or ancillary chunks.  Such files can be read and there is
    * no warning on read, so this seems like a very bad idea.
    */
   if (window_bits > 15)
   {
      png_warning(png_ptr, "Only compression windows <= 32k supported by PNG");
      window_bits = 15;
   }

   else if (window_bits < 8)
   {
      png_warning(png_ptr, "Only compression windows >= 256 supported by PNG");
      window_bits = 8;
   }

   png_ptr->zlib_window_bits = window_bits;
}

void PNGAPI
png_set_compression_method(png_structrp png_ptr, int method)
{
   png_debug(1, "in png_set_compression_method");

   if (png_ptr == NULL)
      return;

   /* This would produce an invalid PNG file if it worked, but it doesn't and
    * deflate will fault it, so it is harmless to just warn here.
    */
   if (method != 8)
      png_warning(png_ptr, "Only compression method 8 is supported by PNG");

   png_ptr->zlib_method = method;
}
#endif /* WRITE_CUSTOMIZE_COMPRESSION */

/* The following were added to libpng-1.5.4 */
#ifdef PNG_WRITE_CUSTOMIZE_ZTXT_COMPRESSION_SUPPORTED
void PNGAPI
png_set_text_compression_level(png_structrp png_ptr, int level)
{
   png_debug(1, "in png_set_text_compression_level");

   if (png_ptr == NULL)
      return;

   png_ptr->zlib_text_level = level;
}

void PNGAPI
png_set_text_compression_mem_level(png_structrp png_ptr, int mem_level)
{
   png_debug(1, "in png_set_text_compression_mem_level");

   if (png_ptr == NULL)
      return;

   png_ptr->zlib_text_mem_level = mem_level;
}

void PNGAPI
png_set_text_compression_strategy(png_structrp png_ptr, int strategy)
{
   png_debug(1, "in png_set_text_compression_strategy");

   if (png_ptr == NULL)
      return;

   png_ptr->zlib_text_strategy = strategy;
}

/* If PNG_WRITE_OPTIMIZE_CMF_SUPPORTED is defined, libpng will use a
 * smaller value of window_bits if it can do so safely.
 */
void PNGAPI
png_set_text_compression_window_bits(png_structrp png_ptr, int window_bits)
{
   if (png_ptr == NULL)
      return;

   if (window_bits > 15)
   {
      png_warning(png_ptr, "Only compression windows <= 32k supported by PNG");
      window_bits = 15;
   }

   else if (window_bits < 8)
   {
      png_warning(png_ptr, "Only compression windows >= 256 supported by PNG");
      window_bits = 8;
   }

   png_ptr->zlib_text_window_bits = window_bits;
}

void PNGAPI
png_set_text_compression_method(png_structrp png_ptr, int method)
{
   png_debug(1, "in png_set_text_compression_method");

   if (png_ptr == NULL)
      return;

   if (method != 8)
      png_warning(png_ptr, "Only compression method 8 is supported by PNG");

   png_ptr->zlib_text_method = method;
}
#endif /* WRITE_CUSTOMIZE_ZTXT_COMPRESSION */
/* end of API added to libpng-1.5.4 */

#endif /* WRITE */
