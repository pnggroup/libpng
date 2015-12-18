
/* pngwutil.c - utilities to write a PNG file
 *
 * Last changed in libpng 1.7.0 [(PENDING RELEASE)]
 * Copyright (c) 1998-2002,2004,2006-2015 Glenn Randers-Pehrson
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
png_write_chunk_data(png_structrp png_ptr, png_const_voidp data,
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
   png_const_voidp data, png_size_t length)
{
   if (png_ptr == NULL)
      return;

   /* On 64 bit architectures 'length' may not fit in a png_uint_32. */
   if (length > PNG_UINT_31_MAX)
      png_error(png_ptr, "length exceeds PNG maximum");

   png_write_chunk_header(png_ptr, chunk_name, (png_uint_32)/*SAFE*/length);
   png_write_chunk_data(png_ptr, data, length);
   png_write_chunk_end(png_ptr);
}

/* This is the API that calls the internal function above. */
void PNGAPI
png_write_chunk(png_structrp png_ptr, png_const_bytep chunk_string,
   png_const_voidp data, png_size_t length)
{
   png_write_complete_chunk(png_ptr, PNG_CHUNK_FROM_STRING(chunk_string), data,
      length);
}

/* This is used below to find the size of an image to pass to png_deflate_claim,
 * so it only needs to be accurate if the size is less than 16384 bytes (the
 * point at which a lower LZ window size can be used.)
 */
static png_alloc_size_t
png_image_size(png_const_structrp png_ptr)
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
optimize_cmf(png_const_structrp png_ptr, png_bytep data,
      png_alloc_size_t data_size)
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

            data[0] = png_check_byte(png_ptr, z_cmf);
            tmp = data[1] & 0xe0;
            tmp += 0x1f - ((z_cmf << 8) + tmp) % 0x1f;
            data[1] = png_check_byte(png_ptr, tmp);
         }
      }
   }

   PNG_UNUSED(png_ptr)
}
#endif /* WRITE_OPTIMIZE_CMF */

/* Release memory used by the deflate mechanism */
static void
png_deflateEnd(png_const_structrp png_ptr, z_stream *zs, int check)
{
   if (zs->state != NULL)
   {
      int ret = deflateEnd(zs);

      /* Z_DATA_ERROR means there was pending output. */
      if ((ret != Z_OK && (check || ret != Z_DATA_ERROR)) || zs->state != NULL)
      {
         png_zstream_error(zs, ret);

         if (check)
            png_error(png_ptr, zs->msg);

         else
            png_warning(png_ptr, zs->msg);

         zs->state = NULL;
      }
   }
}

/* compression_buffer (new in 1.6.0) is just a linked list of temporary buffers. * From 1.6.0 it is retained in png_struct so that it will be correctly freed in
 * the event of a write error (previous implementations just leaked memory.)
 *
 * From 1.7.0 the size is fixed to the same as the (uncompressed) row buffer
 * size.  This avoids allocating a large chunk of memory when compressing small
 * images.  This type is also opaque outside this file.
 */
typedef struct png_compression_buffer
{
   struct png_compression_buffer *next;
   png_byte                       output[PNG_ROW_BUFFER_SIZE];
} png_compression_buffer, *png_compression_bufferp;

/* png_compression_buffer methods */
/* Deleting a compression buffer deletes the whole list: */
static void
png_free_compression_buffer(png_const_structrp png_ptr,
      png_compression_bufferp *listp)
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

/* Return the next compression buffer in the list, allocating it if necessary.
 * The caller must update 'end' if required; this just moves down the list.
 */
static png_compression_bufferp
png_get_compression_buffer(png_const_structrp png_ptr,
      png_compression_bufferp *end)
{
   png_compression_bufferp next = *end;

   if (next == NULL)
   {
      next = png_voidcast(png_compression_bufferp, png_malloc_base(png_ptr,
               sizeof *next));

      /* Check for OOM: this is a recoverable error for non-critical chunks, let
       * the caller decide what to do rather than issuing a png_error here.
       */
      if (next != NULL)
      {
         next->next = NULL; /* initialize the buffer */
         *end = next;
      }
   }

   return next; /* may still be NULL on OOM */
}

/* This structure is used to hold all the data for zlib compression of a single
 * stream of data.  It may be re-used, it stores the compressed data internally
 * and can handle arbitrary input and output.
 *
 * 'list' is the output data contained in compression buffers, 'end' points to
 * list at the start and is advanced down the compression buffer list (extending
 * it as required) as the data is written.  If 'end' points into a compression
 * buffer (does not point to 'list') that is the buffer in use in
 * z_stream::{next,avail}_out.
 *
 * Compression may be performed in multiple steps, '*end' always points to the
 * compression buffer *after* the one that is in use, so 'end' is pointing
 * *into* the one in use.
 *
 *    end(on entry) .... end ....... end(on exit)
 *          |             |                |
 *          |             |                |
 *          V        +----V-----+    +-----V----+    +----------+
 *         list ---> |   next --+--> |   next --+--> |   next   |
 *                   | output[] |    | output[] |    | output[] |
 *                   +----------+    +----------+    +----------+
 *                                     [in use]        [unused]
 *
 * These invariants should always hold:
 *
 * 1) If zs.state is NULL decompression is not in progress, list may be non-NULL
 *    but end could be anything;
 *
 * 2) Otherwise if zs.next_out is NULL list will be NULL and end will point at
 *    list, len, overflow and start will be 0;
 *
 * 3) Otherwise list is non-NULL and end points at the 'next' element of an
 *    in-use compression buffer.  zs.next_out points into the 'output' element
 *    of the same buffer.  {overflow, len} is the amount of compressed data, len
 *    being the low 31 bits, overflow being the higher bits.  start is used for
 *    writing and is the index of the first byte in list->output to write,
 *    {overflow, len} does not include start.
 */
typedef struct
{
   z_stream                 zs;       /* zlib compression data */
   png_compression_bufferp  list;     /* Head of the buffer list */
   png_compression_bufferp *end;      /* Pointer to last 'next' pointer */
   png_uint_32              len;      /* Bottom 31 bits of data length */
   unsigned int             overflow; /* Top bits of data length */
   unsigned int             start;    /* Start of data in first block */
}  png_zlib_compress, *png_zlib_compressp;

/* png_zlib_compress methods */
/* Initialize the compress structure.  The z_stream itself is not initialized,
 * however the the 'user' fields are set, including {next,avail}_{in,out}.  The
 * initialization does not change 'list', however it does set 'end' to point to
 * it, effectively truncating the list.
 */
static void
png_zlib_compress_init(png_structrp png_ptr, png_zlib_compressp pz)
{
   /* png_zlib_compress z_stream: */
   pz->zs.zalloc = png_zalloc;
   pz->zs.zfree = png_zfree;
   /* NOTE: this does not destroy 'restrict' because in all the functions herein
    * *png_ptr is only ever accessed via *either* pz->zs.opaque *or* a passed in
    * png_ptr.
    */
   pz->zs.opaque = png_ptr;

   pz->zs.next_in = NULL;
   pz->zs.avail_in = 0U;
   pz->zs.total_in = 0U;

   pz->zs.next_out = NULL;
   pz->zs.avail_out = 0U;
   pz->zs.total_out = 0U;

   pz->zs.msg = PNGZ_MSG_CAST("zlib success"); /* safety */

   /* pz->list preserved */
   pz->end = &pz->list;
   pz->len = 0U;
   pz->overflow = 0U;
   pz->start = 0U;
}

/* Return the png_ptr: this is defined here for all the remaining
 * png_zlib_compress methods because they are only ever called with zs
 * initialized.
 */
#define png_ptr png_voidcast(png_const_structrp, pz->zs.opaque)

/* Destroy one zlib compress structure. */
static void
png_zlib_compress_destroy(png_zlib_compressp pz, int check)
{
   /* If the 'opaque' pointer is NULL this png_zlib_compress was never
    * initialized, so do nothing.
    */
   if (png_ptr != NULL)
   {
      if (pz->zs.state != NULL)
         png_deflateEnd(png_ptr, &pz->zs, check);

      pz->end = &pz->list; /* safety */
      png_free_compression_buffer(png_ptr, &pz->list);
   }
}

/* Ensure that space is available for output, returns the amount of space
 * available, 0 on OOM.  This updates pz->zs.avail_out (etc) as required.
 */
static uInt
png_zlib_compress_avail_out(png_zlib_compressp pz)
{
   uInt avail_out = pz->zs.avail_out;

   if (avail_out == 0U)
   {
      png_compression_bufferp next;

      affirm(pz->end == &pz->list || (pz->end != NULL && pz->list != NULL));
      next = png_get_compression_buffer(png_ptr, pz->end);

      if (next != NULL)
      {
         pz->zs.next_out = next->output;
         pz->zs.avail_out = avail_out = sizeof next->output;
         pz->end = &next->next;
      }

      /* else return 0: OOM */
   }

   else
      affirm(pz->end != NULL && pz->list != NULL);

   return avail_out;
}

/* Compress the given data given an initialized png_zlib_compress structure.
 * This may be called multiple times, interleaved with writes as required.
 *
 * The input data is passed in in pz->zs.next_in, however the length of the data
 * is in 'input_len' (to avoid the zlib uInt limit) and pz->zs.avail_in is
 * overwritten (and left at 0).
 *
 * The output information is used and the amount of compressed data is added on
 * to pz->{overflow,len}.
 *
 * If 'limit' is a limit on the amount of data to add to the output (not the
 * total amount).  The function will retun Z_BUF_ERROR if the limit is reached
 * and the function will never produce more (additional) compressed data than
 * the limit.
 *
 * All of zstream::next_in[input] is consumed if a success code is returned
 * (Z_OK or Z_STREAM_END if flush is Z_FINISH), otherwise next_in may be used to
 * determine how much was compressed.
 *
 * pz->overflow is not checked for overflow, so if 'limit' is not set overflow
 * is possible.  The caller must guard against this when supplying a limit of 0.
 */
static int
png_compress(
   png_zlib_compressp pz,
   png_alloc_size_t input_len,   /* Length of data to be compressed */
   png_uint_32 limit,            /* Limit on amount of compressed data made */
   int flush)                    /* Flush parameter at end of input */
{
   const int unlimited = (limit == 0U);

   /* Sanity checking: */
   affirm(pz->zs.state != NULL &&
          (pz->zs.next_out == NULL
           ? pz->end == &pz->list && pz->len == 0U && pz->overflow == 0U
           : pz->list != NULL && pz->end != NULL));
   implies(pz->zs.next_out == NULL, pz->zs.avail_out == 0);

   for (;;)
   {
      uInt extra;

      /* OUTPUT: make sure some space is available: */
      if (png_zlib_compress_avail_out(pz) == 0U)
         return Z_MEM_ERROR;

      /* INPUT: limit the deflate call input to ZLIB_IO_MAX: */
      /* Adjust the input counters: */
      {
         uInt avail_in = ZLIB_IO_MAX;

         if (avail_in > input_len)
            avail_in = (uInt)/*SAFE*/input_len;

         input_len -= avail_in;
         pz->zs.avail_in = avail_in;
      }

      if (!unlimited && pz->zs.avail_out > limit)
      {
         extra = (uInt)/*SAFE*/(pz->zs.avail_out - limit); /* unused bytes */
         pz->zs.avail_out = (uInt)/*SAFE*/limit;
         limit = 0U;
      }

      else
      {
         extra = 0U;
         limit -= pz->zs.avail_out; /* limit >= 0U */
      }

      pz->len += pz->zs.avail_out; /* maximum that can be produced */

      /* Compress the data */
      {
         int ret = deflate(&pz->zs, input_len > 0U ? Z_NO_FLUSH : flush);

         /* Claw back input data that was not consumed (because avail_in is
          * reset above every time round the loop) and correct the output
          * length.
          */
         input_len += pz->zs.avail_in;
         pz->zs.avail_in = 0; /* safety */
         pz->len -= pz->zs.avail_out;

         if (pz->len & 0x80000000U)
            ++pz->overflow, pz->len &= 0x7FFFFFFFU;

         limit += pz->zs.avail_out;
         pz->zs.avail_out += extra;

         /* Check the error code: */
         switch (ret)
         {
            case Z_OK:
               if (pz->zs.avail_out > extra)
               {
                  /* zlib had output space, so all the input should have been
                   * consumed:
                   */
                  affirm(input_len == 0U /* else unexpected stop */ &&
                         flush != Z_FINISH/* ret != Z_STREAM_END */);
                  return Z_OK;
               }

               else
               {
                  /* zlib ran out of output space, produce some more.  If the
                   * limit is 0 at this point, however, no more space is
                   * available.
                   */
                  if (unlimited || limit > 0U)
                     break; /* Allocate more output */

                  /* No more output space available, but the input may have all
                   * been consumed.
                   */
                  if (input_len == 0U && flush != Z_FINISH)
                     return Z_OK;

                  /* Input all consumed, but insufficient space to flush the
                   * output; this is the Z_BUF_ERROR case.
                   */
                  return Z_BUF_ERROR;
               }

            case Z_STREAM_END:
               affirm(input_len == 0U && flush == Z_FINISH);
               return Z_STREAM_END;

            case Z_BUF_ERROR:
               /* This means that we are flushing all the output; expect
                * avail_out and input_len to be 0.
                */
               affirm(input_len == 0U && pz->zs.avail_out == extra);
               /* Allocate another buffer */
               break;

            default:
               /* An error */
               return ret;
         }
      }
   }
}

#undef png_ptr /* remove definition using a png_zlib_compressp */

/* All the compression state is held here, it is allocated when required.  This
 * ensures that the read code doesn't carry the overhead of the much less
 * frequently used write stuff.
 *
 * TODO: make png_create_write_struct allocate this stuff after the main
 * png_struct.
 */
typedef struct png_zlib_state
{
   png_zlib_compress        s;       /* Primary compression state */

#  ifdef PNG_SELECT_FILTER_METHODICALLY_SUPPORTED
      /* The 'methodical' method uses up to PNG_FILTER_VALUE_LAST of these to
       * test each possible filter:
       */
      png_zlib_compress     filter[PNG_FILTER_VALUE_LAST];
#  endif /* SELECT_FILTER_METHODICALLY */

   png_compression_bufferp  stash;   /* Unused compression buffers */

#  ifdef PNG_WRITE_FLUSH_SUPPORTED
      png_uint_32   flush_dist; /* how many rows apart to flush, 0 - no flush */
      png_uint_32   flush_rows; /* number of rows written since last flush */
#  endif /* WRITE_FLUSH */

#ifdef PNG_WRITE_FILTER_SUPPORTED
   unsigned int filter_mask    :8; /* mask of filters to consider on write */
   unsigned int filters        :8; /* Filters for current row */
   unsigned int filter_oom     :1; /* ran out of memory */
#endif /* WRITE_FILTER */

   /* Zlib parameters to be used for IDAT and (possibly) text/ICC profile
    * compression.
    */
   int zlib_level;                   /* holds zlib compression level */
   int zlib_method;                  /* holds zlib compression method */
   int zlib_window_bits;             /* holds zlib compression window bits */
   int zlib_mem_level;               /* holds zlib compression memory level */
   int zlib_strategy;                /* holds zlib compression strategy */

   /* The same, but these are the values actually set into the z_stream: */
   int zlib_set_level;
   int zlib_set_method;
   int zlib_set_window_bits;
   int zlib_set_mem_level;
   int zlib_set_strategy;

#  ifdef PNG_WRITE_CUSTOMIZE_ZTXT_COMPRESSION_SUPPORTED
      int zlib_text_level;       /* holds zlib compression level */
      int zlib_text_method;      /* holds zlib compression method */
      int zlib_text_window_bits; /* holds zlib compression window bits */
      int zlib_text_mem_level;   /* holds zlib compression memory level */
      int zlib_text_strategy;    /* holds zlib compression strategy */
#  endif /* WRITE_CUSTOMIZE_ZTXT_COMPRESSION */
}  png_zlib_state;

/* This returns the zlib compression state and has the side effect of
 * initializing it if it does not exist.
 */
static png_zlib_statep
png_get_zlib_state(png_structrp png_ptr)
{
   if (png_ptr != NULL)
   {
      png_zlib_state *ps = png_ptr->zlib_state;

      if (ps == NULL && !png_ptr->read_struct)
      {
         png_ptr->zlib_state = ps = png_voidcast(png_zlib_state*,
               png_malloc(png_ptr, sizeof *ps));

         /* Clear to NULL/0: */
         memset(ps, 0, sizeof *ps);

         png_zlib_compress_init(png_ptr, &ps->s);

#        ifdef PNG_SELECT_FILTER_METHODICALLY_SUPPORTED
            {
               unsigned int i;

               for (i=0; i<PNG_FILTER_VALUE_LAST; ++i)
                  ps->filter[i].zs.opaque = NULL;
            }
#        endif /* SELECT_FILTER_METHODICALLY */

         ps->stash = NULL;

#        ifdef PNG_WRITE_FLUSH_SUPPORTED
            /* Set this to prevent flushing by making it larger than the number
             * of rows in the largest interlaced PNG; PNG_UINT_31_MAX times
             * (1/8+1/8+1/8+1/4+1/4+1/2+1/2); 1.875, or 15/8
             */
            ps->flush_dist = 0xEFFFFFFFU;
            ps->flush_rows = 0U;
#        endif /* WRITE_FLUSH */

#        ifdef PNG_WRITE_FILTER_SUPPORTED
            ps->filter_mask = PNG_NO_FILTERS; /* unset */
            ps->filters = 0U;
            ps->filter_oom = 0U;
#        endif /* WRITE_FILTER */

         /* Zlib parameters to be used for IDAT and (possibly) text/ICC profile
          * compression.
          */
         ps->zlib_level = PNG_Z_DEFAULT_COMPRESSION;
         ps->zlib_method = Z_DEFLATED;
         ps->zlib_window_bits = 15; /* 8..15 permitted, 15 is the default */
         ps->zlib_mem_level = 8; /* 1..9 permitted, 8 is the default */
         ps->zlib_strategy = -1/*unset (invalid value)*/;

#        ifdef PNG_WRITE_COMPRESSED_TEXT_SUPPORTED
            ps->zlib_text_level = PNG_TEXT_Z_DEFAULT_COMPRESSION;
            ps->zlib_text_method = Z_DEFLATED;
            ps->zlib_text_window_bits = 15;
            ps->zlib_text_mem_level = 8;
            ps->zlib_text_strategy = PNG_TEXT_Z_DEFAULT_STRATEGY;
#        endif /* WRITE_COMPRESSED_TEXT */
      }

      return ps;
   }

   return NULL;
}

/* Internal API to clean up all the deflate related stuff, including the buffer
 * lists.
 */
static void /* PRIVATE */
png_deflate_release(png_structrp png_ptr, png_zlib_statep ps, int check)
{
   /* This must happen before ps->s is destroyed below because the structures
    * may be shared:
    */
#  ifdef PNG_SELECT_FILTER_METHODICALLY_SUPPORTED
      unsigned int i;

      /* Note that png_zlib_compress_destroy checks the 'opaque' pointer and
       * does nothing if it is NULL.
       */
      for (i=0U; i<PNG_FILTER_VALUE_LAST; ++i)
         if (ps->filter[i].zs.state != ps->s.zs.state)
         {
            png_zlib_compress_destroy(&ps->filter[i], 0/*check*/);
            ps->filter[i].zs.opaque = NULL;
         }
#  endif /* SELECT_FILTER_METHODICALLY */

   /* The main z_stream opaque pointer needs to remain set to png_ptr; it is
    * only set once.
    */
   png_zlib_compress_destroy(&ps->s, check);
   png_free_compression_buffer(png_ptr, &ps->stash);
}

void /* PRIVATE */
png_deflate_destroy(png_structrp png_ptr)
{
   png_zlib_statep ps = png_ptr->zlib_state;

   if (ps != NULL)
   {
      png_deflate_release(png_ptr, ps, 0/*check*/);
      png_ptr->zlib_state = NULL;
      png_free(png_ptr, ps);
   }
}

/* Initialize the compressor for the appropriate type of compression. */
static png_zlib_statep
png_deflate_claim(png_structrp png_ptr, png_uint_32 owner,
      png_alloc_size_t data_size)
{
   png_zlib_statep ps = png_get_zlib_state(png_ptr);

   affirm(ps != NULL && png_ptr->zowner == 0);

   {
      int level = ps->zlib_level;
      int method = ps->zlib_method;
      int windowBits = ps->zlib_window_bits;
      int memLevel = ps->zlib_mem_level;
      int strategy = ps->zlib_strategy;
      int ret; /* zlib return code */

      if (owner != png_IDAT)
      {
#        ifdef PNG_WRITE_CUSTOMIZE_ZTXT_COMPRESSION_SUPPORTED
            level = ps->zlib_text_level;
            method = ps->zlib_text_method;
            windowBits = ps->zlib_text_window_bits;
            memLevel = ps->zlib_text_mem_level;
            strategy = ps->zlib_text_strategy;
#        else /* !WRITE_CUSTOMIZE_ZTXT_COMPRESSION */
            /* If customization is not supported the values all come from the
             * IDAT values except for the strategy, which is fixed to the
             * default.  (This is the pre-1.6.0 behavior too, although it was
             * implemented in a very different way.)
             */
            strategy = Z_DEFAULT_STRATEGY;
#        endif /* !WRITE_CUSTOMIZE_ZTXT_COMPRESSION */
      }

      /* Adjust 'windowBits' down if larger than 'data_size'; to stop this
       * happening just pass 32768 as the data_size parameter.  Notice that zlib
       * requires an extra 262 bytes in the window in addition to the data to be
       * able to see the whole of the data, so if data_size+262 takes us to the
       * next windowBits size we need to fix up the value later.  (Because even
       * though deflate needs the extra window, inflate does not!)
       */
      if (data_size <= 16384U)
      {
         /* IMPLEMENTATION NOTE: this 'half_window_size' stuff is only here to
          * work round a Microsoft Visual C misbehavior which, contrary to C-90,
          * widens the result of the following shift to 64-bits if (and,
          * apparently, only if) it is used in a test.
          */
         unsigned int half_window_size = 1U << (windowBits-1);

         while (data_size + 262U <= half_window_size)
         {
            half_window_size >>= 1;
            --windowBits;
         }
      }

      /* Check against the previous initialized values, if any. */
      if (ps->s.zs.state != NULL &&
         (ps->zlib_set_level != level ||
          ps->zlib_set_method != method ||
          ps->zlib_set_window_bits != windowBits ||
          ps->zlib_set_mem_level != memLevel ||
          ps->zlib_set_strategy != strategy))
         png_deflateEnd(png_ptr, &ps->s.zs, 0/*check*/);

      /* For safety clear out the input and output pointers (currently zlib
       * doesn't use them on Init, but it might in the future).
       */
      ps->s.zs.next_in = NULL;
      ps->s.zs.avail_in = 0;
      ps->s.zs.next_out = NULL;
      ps->s.zs.avail_out = 0;

      /* The length fields must be cleared too and the lists reset: */
      ps->s.overflow = ps->s.len = ps->s.start = 0U;

      if (ps->s.list != NULL) /* error in prior chunk writing */
      {
         debug(ps->stash == NULL);
         ps->stash = ps->s.list;
         ps->s.list = NULL;
      }

      ps->s.end = &ps->s.list;

      /* Now initialize if required, setting the new parameters, otherwise just
       * do a simple reset to the previous parameters.
       */
      if (ps->s.zs.state != NULL)
         ret = deflateReset(&ps->s.zs);

      else
         ret = deflateInit2(&ps->s.zs, level, method, windowBits, memLevel,
               strategy);

      /* The return code is from either deflateReset or deflateInit2; they have
       * pretty much the same set of error codes.
       */
      if (ret == Z_OK && ps->s.zs.state != NULL)
         png_ptr->zowner = owner;

      else
      {
         png_zstream_error(&ps->s.zs, ret);
         png_error(png_ptr, ps->s.zs.msg);
      }
   }

   return ps;
}

#ifdef PNG_WRITE_COMPRESSED_TEXT_SUPPORTED /* includes iCCP */
/* Compress the block of data at the end of a chunk.  This claims and releases
 * png_struct::z_stream.  It returns the amount of data in the chunk list or
 * zero on error (a zlib stream always contains some bytes!)
 *
 * prefix_len is the amount of (uncompressed) data before the start of the
 * compressed data.  The routine will return 0 if the total of the compressed
 * data and the prefix exceeds PNG_UINT_MAX_31.
 *
 * NOTE: this function may not return; it only returns 0 if
 * png_chunk_report(PNG_CHUNK_WRITE_ERROR) returns (not the default).
 */
static int /* success */
png_compress_chunk_data(png_structrp png_ptr, png_uint_32 chunk_name,
      png_uint_32 prefix_len, png_const_voidp input, png_alloc_size_t input_len)
{
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
   png_zlib_statep ps = png_deflate_claim(png_ptr, chunk_name, input_len);

   affirm(ps != NULL);

   /* The data compression function always returns so that we can clean up. */
   ps->s.zs.next_in = PNGZ_INPUT_CAST(png_voidcast(const Bytef*, input));

   /* Use the stash, if available: */
   debug(ps->s.list == NULL);
   ps->s.list = ps->stash;
   ps->stash = NULL;

   {
      int ret = png_compress(&ps->s, input_len, PNG_UINT_31_MAX-prefix_len,
            Z_FINISH);

      ps->s.zs.next_out = NULL; /* safety */
      ps->s.zs.avail_out = 0;
      ps->s.zs.next_in = NULL;
      ps->s.zs.avail_in = 0;
      png_ptr->zowner = 0; /* release png_ptr::zstream */

      /* Since Z_FINISH was passed as the flush parameter any result other than
       * Z_STREAM_END is an error.  In any case in the event of an error free
       * the whole compression state; the only expected error is Z_MEM_ERROR.
       */
      if (ret != Z_STREAM_END)
      {
         png_zlib_compress_destroy(&ps->s, 0/*check*/);

         /* This is not very likely given the PNG_UINT_31_MAX limit above, but
          * if code is added to limit the size of the chunks produced it can
          * start to happen.
          */
         if (ret == Z_BUF_ERROR)
            ps->s.zs.msg = PNGZ_MSG_CAST("compressed chunk too long");

         else
            png_zstream_error(&ps->s.zs, ret);

         png_chunk_report(png_ptr, ps->s.zs.msg, PNG_CHUNK_WRITE_ERROR);
         return 0;
      }
   }

   /* png_compress is meant to guarantee this on a successful return: */
   affirm(ps->s.overflow == 0U && ps->s.len <= PNG_UINT_31_MAX - prefix_len);

#  ifdef PNG_WRITE_OPTIMIZE_CMF_SUPPORTED
      /* Fix up the deflate header, if required (and possible): */
      if (ps->s.len >= 2U)
         optimize_cmf(png_ptr, ps->s.list->output, input_len);
#  endif /* WRITE_OPTIMIZE_CMF */

   return 1;
}

/* Return the length of the compressed data; this is effectively a debug
 * function to catch inconsistencies caused by internal errors.  It will
 * disappear in a release build.
 */
#if PNG_RELEASE_BUILD
#  define png_length_compressed_chunk_data(pp, p) ((pp)->zlib_state->s.len)
#else /* !RELEASE_BUILD */
static png_uint_32
png_length_compressed_chunk_data(png_structrp png_ptr, png_uint_32 p)
{
   png_zlib_statep ps = png_ptr->zlib_state;

   debug(ps != NULL && ps->s.overflow == 0U && ps->s.len <= PNG_UINT_31_MAX-p);
   return ps->s.len;
}
#endif /* !RELEASE_BUILD */

/* Write all the data produced by the above function; the caller must write the
 * prefix and chunk header.
 */
static void
png_write_compressed_chunk_data(png_structrp png_ptr)
{
   png_zlib_statep ps = png_ptr->zlib_state;
   png_compression_bufferp next;
   png_uint_32 output_len;

   affirm(ps != NULL && ps->s.overflow == 0U);
   next = ps->s.list;

   for (output_len = ps->s.len; output_len > 0U; next = next->next)
   {
      png_uint_32 size = PNG_ROW_BUFFER_SIZE;

      /* If this affirm fails there is a bug in the calculation of
       * output_length above, or in the buffer_limit code in png_compress.
       */
      affirm(next != NULL && output_len > 0U);

      if (size > output_len)
         size = output_len;

      png_write_chunk_data(png_ptr, next->output, size);

      output_len -= size;
   }

   /* Release the list back to the stash. */
   debug(ps->stash == NULL);
   ps->stash = ps->s.list;
   ps->s.list = NULL;
   ps->s.end = &ps->s.list;
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
 * The 'new_key' buffer must be at least 80 characters in size (for the keyword
 * plus a trailing '\0').  If this routine returns 0 then there was no keyword,
 * or a valid one could not be generated, and the caller must CHUNK_WRITE_ERROR.
 */
static unsigned int
png_check_keyword(png_structrp png_ptr, png_const_charp key, png_bytep new_key)
{
   png_const_charp orig_key = key;
   unsigned int key_len = 0;
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
      png_app_warning(png_ptr, "keyword truncated");

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
      png_chunk_report(png_ptr, "Invalid sRGB rendering intent specified",
            PNG_CHUNK_WRITE_ERROR);

   buf[0] = png_check_byte(png_ptr, srgb_intent);
   png_write_complete_chunk(png_ptr, png_sRGB, buf, (png_size_t)1);
}
#endif

#ifdef PNG_WRITE_iCCP_SUPPORTED
/* Write an iCCP chunk */
void /* PRIVATE */
png_write_iCCP(png_structrp png_ptr, png_const_charp name,
    png_const_voidp profile)
{
   png_uint_32 name_len;
   png_uint_32 profile_len;
   png_byte new_name[81]; /* 1 byte for the compression byte */

   png_debug(1, "in png_write_iCCP");

   affirm(profile != NULL);

   profile_len = png_get_uint_32(profile);
   name_len = png_check_keyword(png_ptr, name, new_name);

   if (name_len == 0)
   {
      png_chunk_report(png_ptr, "iCCP: invalid keyword", PNG_CHUNK_WRITE_ERROR);
      return;
   }

   ++name_len; /* trailing '\0' */
   new_name[name_len++] = PNG_COMPRESSION_TYPE_BASE;

   if (png_compress_chunk_data(png_ptr, png_iCCP, name_len, profile,
            profile_len))
   {
      png_write_chunk_header(png_ptr, png_iCCP,
            name_len+png_length_compressed_chunk_data(png_ptr, name_len));
      png_write_chunk_data(png_ptr, new_name, name_len);
      png_write_compressed_chunk_data(png_ptr);
      png_write_chunk_end(png_ptr);
   }
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
   unsigned int key_len;
   png_byte new_key[80];

   png_debug(1, "in png_write_tEXt");

   key_len = png_check_keyword(png_ptr, key, new_key);

   if (key_len == 0)
   {
      png_chunk_report(png_ptr, "tEXt: invalid keyword", PNG_CHUNK_WRITE_ERROR);
      return;
   }

   if (text == NULL || *text == '\0')
      text_len = 0;

   else
      text_len = strlen(text);

   if (text_len > PNG_UINT_31_MAX - (key_len+1))
   {
      png_chunk_report(png_ptr, "tEXt: text too long", PNG_CHUNK_WRITE_ERROR);
      return;
   }

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
   unsigned int key_len;
   png_byte new_key[81];

   png_debug(1, "in png_write_zTXt");

   if (compression != PNG_TEXT_COMPRESSION_zTXt)
      png_app_warning(png_ptr, "zTXt: invalid compression type ignored");

   key_len = png_check_keyword(png_ptr, key, new_key);

   if (key_len == 0)
   {
      png_chunk_report(png_ptr, "zTXt: invalid keyword", PNG_CHUNK_WRITE_ERROR);
      return;
   }

   /* Add the compression method and 1 for the keyword separator. */
   ++key_len;
   new_key[key_len++] = PNG_COMPRESSION_TYPE_BASE;

   if (png_compress_chunk_data(png_ptr, png_zTXt, key_len, text, strlen(text)))
   {
      png_write_chunk_header(png_ptr, png_zTXt,
            key_len+png_length_compressed_chunk_data(png_ptr, key_len));
      png_write_chunk_data(png_ptr, new_key, key_len);
      png_write_compressed_chunk_data(png_ptr);
      png_write_chunk_end(png_ptr);
   }

   /* else chunk report already issued and ignored */
}
#endif

#ifdef PNG_WRITE_iTXt_SUPPORTED
/* Write an iTXt chunk */
void /* PRIVATE */
png_write_iTXt(png_structrp png_ptr, int compression, png_const_charp key,
    png_const_charp lang, png_const_charp lang_key, png_const_charp text)
{
   png_uint_32 key_len, prefix_len, data_len;
   png_size_t lang_len, lang_key_len, text_len;
   png_byte new_key[82]; /* 80 bytes for the key, 2 byte compression info */

   png_debug(1, "in png_write_iTXt");

   key_len = png_check_keyword(png_ptr, key, new_key);

   if (key_len == 0)
   {
      png_chunk_report(png_ptr, "iTXt: invalid keyword", PNG_CHUNK_WRITE_ERROR);
      return;
   }

   debug(new_key[key_len] == 0);
   ++key_len; /* terminating 0 added by png_check_keyword */

   /* Set the compression flag */
   switch (compression)
   {
      case PNG_ITXT_COMPRESSION_NONE:
      case PNG_TEXT_COMPRESSION_NONE:
         compression = new_key[key_len++] = 0; /* no compression */
         break;

      case PNG_TEXT_COMPRESSION_zTXt:
      case PNG_ITXT_COMPRESSION_zTXt:
         compression = new_key[key_len++] = 1; /* compressed */
         break;

      default:
         png_chunk_report(png_ptr, "iTXt: invalid compression",
               PNG_CHUNK_WRITE_ERROR);
         return;
   }

   new_key[key_len++] = PNG_COMPRESSION_TYPE_BASE;

   /* We leave it to the application to meet PNG-1.0 requirements on the
    * contents of the text.  PNG-1.0 through PNG-1.2 discourage the use of
    * any non-Latin-1 characters except for NEWLINE (yes, this is really weird
    * in an 'international' text string.  ISO PNG, however, specifies that the
    * text is UTF-8 and this *IS NOT YET CHECKED*, so invalid sequences may be
    * present.
    *
    * The NUL character is forbidden by PNG-1.0 through PNG-1.2 and ISO PNG.
    *
    * TODO: validate the language tag correctly (see the spec.)
    */
   if (lang == NULL) lang = ""; /* empty language is valid */
   lang_len = strlen(lang)+1U;
   if (lang_key == NULL) lang_key = ""; /* may be empty */
   lang_key_len = strlen(lang_key)+1U;
   if (text == NULL) text = ""; /* may be empty */

   if (lang_len > PNG_UINT_31_MAX-key_len ||
       lang_key_len > PNG_UINT_31_MAX-key_len-lang_len)
   {
      png_chunk_report(png_ptr, "iTXt: prefix too long", PNG_CHUNK_WRITE_ERROR);
      return;
   }

   prefix_len = (png_uint_32)/*SAFE*/(key_len+lang_len+lang_key_len);
   text_len = strlen(text); /* no trailing '\0' */

   if (compression != 0)
   {
      if (png_compress_chunk_data(png_ptr, png_iTXt, prefix_len, text,
               text_len))
         data_len = png_length_compressed_chunk_data(png_ptr, prefix_len);

      else
         return; /* chunk report already issued and ignored */
   }

   else
   {
      if (text_len > PNG_UINT_31_MAX-prefix_len)
      {
         png_chunk_report(png_ptr, "iTXt: text too long",
               PNG_CHUNK_WRITE_ERROR);
         return;
      }

      data_len = (png_uint_32)/*SAFE*/text_len;
   }

   png_write_chunk_header(png_ptr, png_iTXt, prefix_len+data_len);
   png_write_chunk_data(png_ptr, new_key, key_len);
   png_write_chunk_data(png_ptr, lang, lang_len);
   png_write_chunk_data(png_ptr, lang_key, lang_key_len);

   if (compression != 0)
      png_write_compressed_chunk_data(png_ptr);

   else
      png_write_chunk_data(png_ptr, text, data_len);

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

   /* Duplicate tIME chunks are invalid; this works round a bug in png_write_png
    * where it would write the tIME chunk once before and once after the IDAT.
    */
   if (png_ptr->wrote_tIME)
      return;

   png_save_uint_16(buf, mod_time->year);
   buf[2] = mod_time->month;
   buf[3] = mod_time->day;
   buf[4] = mod_time->hour;
   buf[5] = mod_time->minute;
   buf[6] = mod_time->second;

   png_write_complete_chunk(png_ptr, png_tIME, buf, (png_size_t)7);
   png_ptr->wrote_tIME = 1U;
}
#endif

/* These two #defines simplify writing code that depends on one or the other of
 * the options being both supported and on:
 */
#ifdef PNG_SELECT_FILTER_METHODICALLY_SUPPORTED
#  define methodical_option\
      ((png_ptr->options >> PNG_SELECT_FILTER_METHODICALLY) & 3U)
#else
#  define methodical_option PNG_OPTION_OFF
#endif

#ifdef PNG_SELECT_FILTER_HEURISTICALLY_SUPPORTED
#  define heuristic_option\
      ((png_ptr->options >> PNG_SELECT_FILTER_HEURISTICALLY) & 3U)
#else /* !SELECT_FILTER_HEURISTICALLY */
#  define heuristic_option PNG_OPTION_OFF
#endif /* !SELECT_FILTER_HEURISTICALLY */

/* Handle the writing of IDAT chunks from the png_zlib_state in
 * png_ptr->zlib_state.
 */
static void
png_start_IDAT(png_structrp png_ptr)
{
   png_zlib_statep ps = png_get_zlib_state(png_ptr);

#  ifdef PNG_WRITE_FILTER_SUPPORTED
      /* Default both filter_mask and zlib_strategy here, now that png_ptr has
       * all the IHDR fields set.
       */
      if (ps->filter_mask == PNG_NO_FILTERS/*unset*/)
      {
         /* If there is no filter selection algorithm enabled then the only
          * option is PNG_FILTER_NONE.
          */
         if (methodical_option == PNG_OPTION_OFF &&
               heuristic_option == PNG_OPTION_OFF)
            ps->filter_mask = PNG_FILTER_NONE;

         else
            ps->filter_mask = PNG_ALL_FILTERS;
      }
#  endif /* WRITE_FILTER */

   if (ps->zlib_strategy == (-1)/*unset*/)
   {
#     ifdef PNG_WRITE_FILTER_SUPPORTED
         if (ps->filter_mask != PNG_FILTER_NONE)
            ps->zlib_strategy = PNG_Z_DEFAULT_STRATEGY;
         else
#     endif /* WRITE_FILTER */

      /* The default with no filters: */
      ps->zlib_strategy = PNG_Z_DEFAULT_NOFILTER_STRATEGY;
   }

   /* This always succeeds or does a png_error: */
   png_deflate_claim(png_ptr, png_IDAT, png_image_size(png_ptr));
}

static void
png_end_IDAT(png_structrp png_ptr)
{
   png_zlib_statep ps = png_ptr->zlib_state;

   png_ptr->zowner = 0U; /* release the stream */

   if (ps != NULL)
      png_deflate_release(png_ptr, ps, 1/*check*/);
}

static void
png_write_IDAT(png_structrp png_ptr, int flush)
{
   png_zlib_statep ps = png_ptr->zlib_state;

   /* Check for a correctly initialized list, the requirement that the end
    * pointer is NULL means that the end of the list can be easily detected.
    */
   affirm(ps != NULL && ps->s.end != NULL && *ps->s.end == NULL);

   /* Write IDAT chunks while either 'flush' is true or there are at
    * least png_ptr->IDAT_size bytes available to be written.
    */
   for (;;)
   {
      png_uint_32 len = png_ptr->IDAT_size;

      if (ps->s.overflow == 0U)
      {
         png_uint_32 avail = ps->s.len;

         if (avail < len)
         {
            /* When end_of_image is true everything gets written, otherwise
             * there must be at least IDAT_size bytes available.
             */
            if (!flush)
               return;

            if (avail == 0)
               break;

            len = avail;
         }
      }

      png_write_chunk_header(png_ptr, png_IDAT, len);

      /* Write bytes from the buffer list, adjusting {overflow,len} as they are
       * written.
       */
      do
      {
         png_compression_bufferp next = ps->s.list;
         unsigned int avail = sizeof next->output;
         unsigned int start = ps->s.start;
         unsigned int written;

         affirm(next != NULL);

         if (next->next == NULL) /* end of list */
         {
            /* The z_stream should always be pointing into this output buffer,
             * the buffer may not be full:
             */
            debug(ps->s.zs.next_out + ps->s.zs.avail_out ==
                  next->output + sizeof next->output);
            avail -= ps->s.zs.avail_out;
         }

         else /* not end of list */
            debug(ps->s.zs.next_out < next->output ||
                  ps->s.zs.next_out > next->output + sizeof next->output);

         /* First, if this is the very first IDAT (PNG_HAVE_IDAT not set)
          * optimize the CINFO field:
          */
#        ifdef PNG_WRITE_OPTIMIZE_CMF_SUPPORTED
            if ((png_ptr->mode & PNG_HAVE_IDAT) == 0U &&
                avail >= start+2U /* enough for the zlib header */)
            {
               debug(start == 0U);
               optimize_cmf(png_ptr, next->output+start,
                     png_image_size(png_ptr));
            }

            else /* always expect to see at least 2 bytes: */
               debug((png_ptr->mode & PNG_HAVE_IDAT) != 0U);
#        endif /* WRITE_OPTIMIZE_CMF */

         if (avail <= start+len)
         {
            /* Write all of this buffer: */
            affirm(avail > start); /* else overflow on the subtract */
            written = avail-start;
            png_write_chunk_data(png_ptr, next->output+start, written);

            /* At the end there are no buffers in the list but the z_stream
             * still points into the old (just released) buffer.  This can
             * happen when the old buffer is not full if the compressed bytes
             * exactly match the IDAT length; it should always happen when
             * end_of_image is set.
             */
            ps->s.list = next->next;

            if (next->next == NULL)
            {
               debug(avail == start+len);
               ps->s.end = &ps->s.list;
               ps->s.zs.next_out = NULL;
               ps->s.zs.avail_out = 0U;
            }

            next->next = ps->stash;
            ps->stash = next;
            ps->s.start = 0U;
         }

         else /* write only part of this buffer */
         {
            written = len;
            png_write_chunk_data(png_ptr, next->output+start, written);
            ps->s.start = (unsigned int)/*SAFE*/(start + written);
            UNTESTED
         }

         /* 'written' bytes were written: */
         len -= written;

         if (written <= ps->s.len)
            ps->s.len -= written;

         else
         {
            affirm(ps->s.overflow > 0U);
            --ps->s.overflow;
            ps->s.len += 0x80000000U - written;
         }
      }
      while (len > 0U);

      png_write_chunk_end(png_ptr);
      png_ptr->mode |= PNG_HAVE_IDAT;
   }

   /* avail == 0 && flush */
   png_end_IDAT(png_ptr);
   png_ptr->mode |= PNG_AFTER_IDAT;
}

/* This is is a convenience wrapper to handle IDAT compression; it takes a
 * pointer to the input data and places no limit on the size of the output but
 * is otherwise the same as png_compress().  It also handles the use of the
 * stash (only used for IDAT compression.)
 */
static int
png_compress_IDAT_data(png_const_structrp png_ptr, png_zlib_statep ps,
      png_zlib_compressp pz, png_const_voidp input, uInt input_len, int flush)
{
   affirm(png_ptr->zowner == png_IDAT && pz->end != NULL && *pz->end == NULL);

   /* z_stream::{next,avail}_out are set by png_compress to point into the
    * buffer list.  next_in must be set here, avail_in comes from the input_len
    * parameter:
    */
   pz->zs.next_in = PNGZ_INPUT_CAST(png_voidcast(const Bytef*, input));
   *pz->end = ps->stash; /* May be NULL */
   ps->stash = NULL;

   /* zlib buffers the output, the maximum amount of compressed data that can be
    * produced here is governed by the amount of buffering.
    */
   {
      int ret = png_compress(pz, input_len, 0U/*unlimited*/, flush);

      affirm(pz->end != NULL && ps->stash == NULL);
      ps->stash = *pz->end; /* May be NULL */
      *pz->end = NULL;

      /* Z_FINISH should give Z_STREAM_END, everything else should give Z_OK, in
       * either case all the input should have been consumed:
       */
      implies(ret == Z_OK || ret == Z_FINISH, pz->zs.avail_in == 0U &&
            (ret == Z_STREAM_END) == (flush == Z_FINISH));
      pz->zs.next_in = NULL;
      pz->zs.avail_in = 0U; /* safety */

      return ret;
   }
}

/* Compress some image data using the main png_zlib_compress.  Write the result
 * out if there is sufficient data.  png_start_IDAT must have been called.
 */
static void
png_compress_IDAT(png_structrp png_ptr, png_const_voidp input, uInt input_len,
      int flush)
{
   png_zlib_statep ps = png_ptr->zlib_state;
   int ret = png_compress_IDAT_data(png_ptr, ps, &ps->s, input, input_len,
         flush);

   /* Check the return code. */
   if (ret == Z_OK || ret == Z_STREAM_END)
      png_write_IDAT(png_ptr, flush == Z_FINISH);

   else /* ret != Z_OK && ret != Z_STREAM_END */
   {
      /* This is an error condition.  It is fatal. */
      png_end_IDAT(png_ptr);
      png_zstream_error(&ps->s.zs, ret);
      png_error(png_ptr, ps->s.zs.msg);
   }
}

#ifdef PNG_WRITE_FLUSH_SUPPORTED
/* Set the automatic flush interval or 0 to turn flushing off */
void PNGAPI
png_set_flush(png_structrp png_ptr, int nrows)
{
   png_zlib_statep ps = png_get_zlib_state(png_ptr);

   png_debug(1, "in png_set_flush");

   if (ps != NULL)
   {
      if (nrows <= 0)
         ps->flush_dist = 0xEFFFFFFFU;
      
      else
         ps->flush_dist = nrows;
   }
}

/* Flush the current output buffers now */
void PNGAPI
png_write_flush(png_structrp png_ptr)
{
   png_debug(1, "in png_write_flush");

   /* Before the start of the IDAT and after the end of the image zowner will be
    * something other than png_IDAT:
    */
   if (png_ptr != NULL && png_ptr->zlib_state != NULL &&
         png_ptr->zowner == png_IDAT)
      png_ptr->zlib_state->flush_rows = 0xEFFFFFFF;
}

/* Return the correct flush to use */
static int
row_flush(png_zlib_statep ps, unsigned int row_info_flags)
{
   if (PNG_IDAT_END(row_info_flags))
      return Z_FINISH;

   else if ((row_info_flags & png_row_end) != 0 &&
         ps->flush_rows >= ps->flush_dist)
      return Z_SYNC_FLUSH;

   else
      return Z_NO_FLUSH;
}
#else /* !WRITE_FLUSH */
#  define row_flush(ps, ri) (PNG_IDAT_END(ri) ? Z_FINISH : Z_NO_FLUSH)
#endif /* !WRITE_FLUSH */

static void
write_filtered_row(png_structrp png_ptr, png_const_voidp filtered_row,
   unsigned int row_bytes, unsigned int filter /*if at start of row*/,
   int flush)
{
   /* This handles writing a row that has been filtered, or did not need to be
    * filtered.  If the data row has a partial pixel it must have been handled
    * correctly in the caller; filters generate a full 8 bits even if the pixel
    * only has one significant bit!
    */
   debug(row_bytes > 0);
   affirm(row_bytes <= ZLIB_IO_MAX); /* I.e. it fits in a uInt */

   if (filter < PNG_FILTER_VALUE_LAST) /* start of row */
   {
      png_byte buffer[1];

      buffer[0] = PNG_BYTE(filter);
      png_compress_IDAT(png_ptr, buffer, 1U/*len*/, Z_NO_FLUSH);
   }

   png_compress_IDAT(png_ptr, filtered_row, row_bytes, flush);
}

static void
write_unfiltered_rowbits(png_structrp png_ptr, png_const_bytep filtered_row,
   unsigned int row_bits, png_byte filter /*if at start of row*/,
   int flush)
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
            row_bits == 0U ? flush : Z_NO_FLUSH);
      filter = PNG_FILTER_VALUE_LAST; /* written */
   }

   /* Handle a partial byte. */
   if (row_bits > 0U)
   {
      png_byte buffer[1];

      buffer[0] = PNG_BYTE(filtered_row[row_bytes] & ~(0xFFU >> row_bits));
      write_filtered_row(png_ptr, buffer, 1U, filter, flush);
   }
}

#ifdef PNG_WRITE_FILTER_SUPPORTED
static void
filter_block_singlebyte(unsigned int row_bytes, png_bytep sub_row,
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
filter_block_multibyte(unsigned int row_bytes,
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
filter_block(png_const_bytep prev_row, png_bytep prev_pixels,
      png_const_bytep unfiltered_row, unsigned int row_bits,
      const unsigned int bpp, png_bytep sub_row, png_bytep up_row,
      png_bytep avg_row, png_bytep paeth_row)
{
   const unsigned int row_bytes = row_bits >> 3; /* complete bytes */

   if (bpp <= 8U)
   {
      /* There may be a partial byte at the end. */
      if (row_bytes > 0)
         filter_block_singlebyte(row_bytes, sub_row, up_row, avg_row, paeth_row,
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
               sub_row == NULL ? NULL : sub_row+row_bytes,
               up_row == NULL ? NULL : up_row+row_bytes,
               avg_row == NULL ? NULL : avg_row+row_bytes,
               paeth_row == NULL ? NULL : paeth_row+row_bytes,
               buffer, buffer+1U, prev_pixels);
      }
   }

   else
      filter_block_multibyte(row_bytes, bpp >> 3,
            sub_row, up_row, avg_row, paeth_row,
            unfiltered_row, prev_row, prev_pixels);
}

static void
filter_row(png_structrp png_ptr, png_const_bytep prev_row,
      png_bytep prev_pixels, png_const_bytep unfiltered_row,
      unsigned int row_bits, unsigned int bpp, unsigned int filter,
      int start_of_row, int flush)
{
   /* filters_to_try identifies a single filter and it is not PNG_FILTER_NONE.
    */
   png_byte filtered_row[PNG_ROW_BUFFER_SIZE];

   affirm((row_bits+7U) >> 3 <= PNG_ROW_BUFFER_SIZE &&
          filter >= PNG_FILTER_VALUE_SUB && filter <= PNG_FILTER_VALUE_PAETH);
   debug((row_bits % bpp) == 0U);

   filter_block(prev_row, prev_pixels, unfiltered_row, row_bits, bpp,
         filter == PNG_FILTER_VALUE_SUB   ? filtered_row : NULL,
         filter == PNG_FILTER_VALUE_UP    ? filtered_row : NULL,
         filter == PNG_FILTER_VALUE_AVG   ? filtered_row : NULL,
         filter == PNG_FILTER_VALUE_PAETH ? filtered_row : NULL);

   write_filtered_row(png_ptr, filtered_row, (row_bits+7U)>>3,
         start_of_row ? filter : PNG_FILTER_VALUE_LAST, flush);
}

#ifdef PNG_SELECT_FILTER_HEURISTICALLY_SUPPORTED
static png_byte
select_filter_heuristically(png_structrp png_ptr, unsigned int filters_to_try,
      png_const_bytep prev_row, png_bytep prev_pixels,
      png_const_bytep unfiltered_row, unsigned int row_bits, unsigned int bpp,
      int flush)
{
   const unsigned int row_bytes = (row_bits+7U) >> 3;
   png_byte test_buffers[4][PNG_ROW_BUFFER_SIZE]; /* for each filter */

   affirm(row_bytes <= PNG_ROW_BUFFER_SIZE);
   debug((row_bits % bpp) == 0U);

   filter_block(prev_row, prev_pixels, unfiltered_row, row_bits, bpp,
         test_buffers[PNG_FILTER_VALUE_SUB-1U],
         test_buffers[PNG_FILTER_VALUE_UP-1U],
         test_buffers[PNG_FILTER_VALUE_AVG-1U],
         test_buffers[PNG_FILTER_VALUE_PAETH-1U]);

   /* Now check each buffer and the original row to see which is best; this is
    * the heuristic.  The test is on the number of separate code values in the
    * buffer.  Since the buffer is either the full row or PNG_ROW_BUFFER_SIZE
    * bytes (or slightly less for RGB) we either find the true number of codes
    * generated or we expect a count of average 8 per code.
    */
   {
      unsigned int filter_max = 257U;
      png_byte best_filter, test_filter;
      png_const_bytep best_row, test_row;

      for (best_filter = test_filter = PNG_FILTER_VALUE_NONE,
            best_row = test_row = unfiltered_row;
           test_filter < PNG_FILTER_VALUE_LAST;
           test_row = test_buffers[test_filter], ++test_filter)
         if ((filters_to_try & PNG_FILTER_MASK(test_filter)) != 0U)
      {
         unsigned int count = 1U, x;
         png_byte code[256];

         memset(code, 0, sizeof code);
         code[test_filter] = 1U;

         for (x=0U; x < row_bytes; ++x)
         {
            const png_byte b = test_row[x];
            if (code[b] == 0) code[b] = 1U, ++count;
         }

         if (count < filter_max)
            filter_max = count, best_filter = test_filter, best_row = test_row;
      }

      /* Calling write_unfiltered_rowbits is necessary here to deal with the
       * clearly of a partial byte at the end.
       */
      if (best_filter == PNG_FILTER_VALUE_NONE)
         write_unfiltered_rowbits(png_ptr, unfiltered_row, row_bits,
               PNG_FILTER_VALUE_NONE, flush);

      else
         write_filtered_row(png_ptr, best_row, row_bytes, best_filter,
               flush);

      return best_filter;
   }
}
#endif /* SELECT_FILTER_HEURISTICALLY */

#ifdef PNG_SELECT_FILTER_METHODICALLY_SUPPORTED
/* With the 'methodical' method multiple png_zlib_compress structures exist,
 * these functions handle the creation and destruction ('release') of these
 * structures.  Note that the structures have not been initialized with the
 * opaque and alloc functions; this is done on demand and the 'opaque' pointer
 * is set to NULL if the compress structure is not in use.
 */
static void
png_zlib_filter_release(png_structrp png_ptr, png_zlib_statep ps, png_byte i)
{
   /* Make sure this filter really is in use: */
   if (ps->filter[i].zs.opaque != NULL) /* else not initialized */
   {
      /* First put the buffer list back into the cache, when this function is
       * called the list should be correctly terminated at *end.
       */
      {
         png_compression_bufferp list = ps->filter[i].list;

         if (list != NULL)
         {
            ps->filter[i].list = NULL;

            /* Return the list to the stash. */
            affirm(ps->filter[i].end != NULL);
            
            /* In the normal case 'end' is the end of this list and it is
             * pre-pended to the cache.  In the error case (png_error during a
             * deflate operation) the list will be the entire stash and the
             * stash will be NULL.
             *
             * If both pointers are non-NULL clean up by making the 'end'
             * pointer NULL (freeing anything it points to).  This is
             * unexpected.
             */
            if (ps->stash != NULL)
            {
               debug(*ps->filter[i].end == NULL);
               /* Clean up on error: */
               png_free_compression_buffer(png_ptr, ps->filter[i].end);
               *ps->filter[i].end = ps->stash;
            }

            ps->stash = list;
         }

         ps->filter[i].end = &ps->filter[i].list; /* safety */
      }

      /* Now use the standard 'destroy' function to handle the z_stream; the
       * list has already been made NULL above.  If the structure is sharing
       * state with the main compress structure do not free it!
       */
      if (ps->filter[i].zs.state != ps->s.zs.state)
         png_zlib_compress_destroy(&ps->filter[i], 0/*check*/);

      /* Then this indicates that the structure is not in use: */
      ps->filter[i].zs.opaque = NULL;
   }

   else
      debug(ps->filter[i].list == NULL);
}

static int /* success */
png_zlib_filter_compress(png_structrp png_ptr, png_zlib_statep ps, png_byte i,
      png_const_voidp input, uInt input_len, int flush)
{
   png_zlib_compressp pz = &ps->filter[i];
   int ret = png_compress_IDAT_data(png_ptr, ps, pz, input, input_len, flush);

   if (ret == Z_OK || ret == Z_STREAM_END)
      return 1; /* success */

   else
   {
      /* If ret is not Z_OK then this stream gets aborted, this is recoverable
       * so long as this is not the only stream left.  There are only two likely
       * causes of failure; an internal libpng bug or out-of-memory.  Given an
       * assumption of infalibility this means that the app is out of memory and
       * it makes sense to release as much as possible.  Note that it is
       * conceivable that OOM may cause an error other than Z_MEM_ERROR, though
       * this is unlikely.
       */
      png_zstream_error(&pz->zs, ret);
      png_warning(png_ptr, pz->zs.msg);
      png_zlib_filter_release(png_ptr, ps, i);
      return 0; /* failure */
   }
}

static int /* success */
png_zlib_filter_init(png_structrp png_ptr, png_zlib_statep ps, png_byte i,
      int copy)
{
   png_zlib_compressp pz = &ps->filter[i];

   /* Make sure that we don't overwrite previously allocated stuff: */
   debug(pz->zs.opaque == NULL && pz->list == NULL);

   /* Initialize the list and count fields: */
   pz->end = &pz->list;
   pz->len = 0U;
   pz->overflow = 0U;
   pz->start = 0U;

   /* If 'copy' is true a complete copy is made of the main z_stream, otherwise
    * the stream is shared.  deflateCopy actually does a memcpy over the
    * destination z_stream, so no further initialization is required.
    */
   if (copy)
   {
      int ret = deflateCopy(&pz->zs, &ps->s.zs);

      if (ret != Z_OK)
      {
         /* If this fails and png_chunk_report returns we can continue because
          * the caller handles the error:
          */
         pz->zs.opaque = NULL;
         png_zstream_error(&pz->zs, ret);
         png_chunk_report(png_ptr, pz->zs.msg, PNG_CHUNK_WRITE_ERROR);
         return 0;
      }
   }

   else
      pz->zs = ps->s.zs; /* see png_zlib_filter_release */

   /* Either way the {next,avail}_{in.out} fields got copied, however they must
    * not be used so:
    */
   ps->filter[i].zs.next_in = ps->filter[i].zs.next_out = NULL;
   ps->filter[i].zs.avail_in = ps->filter[i].zs.avail_out = 0U;
   ps->filter[i].zs.msg = PNGZ_MSG_CAST("zlib copy ok"); /* safety */

   /* If there is a partial buffer in the main stream a partial buffer is
    * required here:
    */
   {
      uInt start = ps->s.zs.avail_out;

      if (start > 0U && start < sizeof ps->s.list->output)
      {
         uInt avail_out;

         start = (uInt)/*SAFE*/(sizeof ps->s.list->output) - start;
         pz->list = ps->stash;
         ps->stash = NULL;
         avail_out = png_zlib_compress_avail_out(pz);
         ps->stash = *pz->end;
         *pz->end = NULL;

         if (avail_out >= start)
         {
            pz->zs.next_out += start;
            pz->zs.avail_out -= start;
            pz->start = start;
         }

         else /* OOM */
         {
            png_warning(png_ptr, "filter selection: out of memory");
            png_zlib_filter_release(png_ptr, ps, i);
            return 0; /* failure */
         }
      }
   }

   /* Finally compress the filter byte into the copied/shared z_stream. */
   {
      png_byte b[1];

      b[0] = i;
      return png_zlib_filter_compress(png_ptr, ps, i, b, 1U, Z_NO_FLUSH);
   }
}

/* Revert to using the main z_stream.  This just moves the given filter (which
 * must have been initialized) back to the main stream leaving the filter ready
 * to be released.
 */
static void
png_zlib_filter_revert(png_structrp png_ptr, png_zlib_statep ps, png_byte i)
{
   png_zlib_compressp pz = &ps->filter[i];

   affirm(pz->zs.opaque != NULL);

   /* First merge the buffer lists. */
   if (pz->overflow || pz->len > 0U)
   {
      affirm(pz->list != NULL);
      debug(ps->s.end != NULL && *ps->s.end == NULL);

      /* The deflate operation produced some output, if pz->start is non-zero
       * the first buffer in pz->list must be merged with the current buffer in
       * the main z_stream, if pz->zs.next_out still points into this buffer the
       * pointer must be updated to point to the old buffer.
       */
      if (pz->start > 0U)
      {
         /* Copy everything after pz->start into the old buffer. */
         memcpy(ps->s.zs.next_out, pz->list->output + pz->start,
               ps->s.zs.avail_out);

         /* Unlink the remainder of the list, if any, and append it to the
          * output.
          */
         if (pz->list->next != NULL)
         {
            debug(pz->end != &pz->list->next);
            *ps->s.end = pz->list->next;
            pz->list->next = NULL;
            ps->s.end = pz->end;
            pz->end = &pz->list->next; /* To be deleted later */
         }

         /* If pz->s.next_out still points into the first buffer (the case for
          * the final row of small images) update it to point to the old buffer
          * instead so that the copy below works.
          */
         if (pz->zs.next_out >= pz->list->output &&
               pz->zs.next_out <= pz->list->output + (sizeof pz->list->output))
         {
            debug(pz->overflow == 0U &&
                  pz->len + pz->start < (sizeof pz->list->output) &&
                  ps->s.zs.avail_out > pz->zs.avail_out);
            pz->zs.next_out = ps->s.zs.next_out + ps->s.zs.avail_out -
               pz->zs.avail_out;
         }
      }

      else
      {
         /* Nothing to copy, the whole new list is appended to the existing one.
          */
         *ps->s.end = pz->list;
         pz->list = NULL;
         ps->s.end = pz->end;
         pz->end = &pz->list;
      }

      /* Update the length fields; 'start' remains correct. */
      ps->s.overflow += pz->overflow;
      if (((ps->s.len += pz->len) & 0x80000000U) != 0)
         ++ps->s.overflow, ps->s.len &= 0x7FFFFFFFU;
   }

   else
   {
      /* deflate produced no additional output; all the state is in the
       * z_stream.  Copy it back without changing anything else.
       */
      debug(pz->zs.avail_out == ps->s.zs.avail_out);
      pz->zs.next_out = ps->s.zs.next_out;
   }

   /* The buffer list has been fixed, the z_stream must be copied.  All fields
    * are relevant.  This is done as a simple swap.
    */
   {
      z_stream zs = ps->s.zs;

      zs.next_in = zs.next_out = NULL;
      zs.avail_in = zs.avail_out = 0U;
      zs.msg = PNGZ_MSG_CAST("invalid");

      ps->s.zs = pz->zs;
      pz->zs = zs;
   }
}

/* As above but release all the filters as well. */
static void
png_zlib_filter_revert_and_release(png_structrp png_ptr, png_zlib_statep ps,
      png_byte i)
{
   /* The other filters must be released first to correctly handle the
    * non-copied one:
    */
   png_byte f;

   for (f=0U; f < PNG_FILTER_VALUE_LAST; ++f)
      if (f != i)
         png_zlib_filter_release(png_ptr, ps, f);

   png_zlib_filter_revert(png_ptr, ps, i);
   png_zlib_filter_release(png_ptr, ps, i);
}

static png_byte /* filters being tried */
select_filter_methodically_init(png_structrp png_ptr,
      const unsigned int filters_to_try)
{
   png_zlib_statep ps = png_ptr->zlib_state;

   affirm(ps != NULL);

   /* Now activate the decompressor for each filter in the list.  Skip the first
    * filter; this will share the main state.
    */
   {
      unsigned int filters = 0U;
      png_byte filter, first_filter = PNG_FILTER_VALUE_LAST;

      for (filter=0U; filter < PNG_FILTER_VALUE_LAST; ++filter)
         if ((filters_to_try & PNG_FILTER_MASK(filter)) != 0U)
         {
            if (first_filter == PNG_FILTER_VALUE_LAST)
               first_filter = filter;

            else if (png_zlib_filter_init(png_ptr, ps, filter, 1/*copy*/))
               filters |= PNG_FILTER_MASK(filter);

            else /* OOM, probably; give up */
            {
               ps->filter_oom = 1U;
               break;
            }
         }

      /* If none of that worked abort the filter selection by returning just the
       * first filter.  Note that a filter value is returned here.
       */
      if (filters == 0U)
         return first_filter;

      /* Finally initialize the first filter. */
      if (png_zlib_filter_init(png_ptr, ps, first_filter, 0/*!copy*/))
         return PNG_ALL_FILTERS & (filters | PNG_FILTER_MASK(first_filter));

      /* This is an error condition but there is still a working z_stream
       * structure.  The z_stream has had the filter byte written to it, so teh
       * standard code cannot be used.  Simply fake the multi-filter case.  The
       * low three bits ensure that there are multiple bits in the result.
       */
      ps->filter_oom = 1U;
      return PNG_ALL_FILTERS & (filters | 0x7U);
   }
}

static void
select_filter_methodically(png_structrp png_ptr, png_const_bytep prev_row,
      png_bytep prev_pixels, png_const_bytep unfiltered_row,
      unsigned int row_bits, unsigned int bpp, unsigned int filters_to_try,
      int end_of_row, int flush)
{
   png_zlib_statep ps = png_ptr->zlib_state;
   const unsigned int row_bytes = (row_bits+7U) >> 3;
   png_byte test_buffers[4][PNG_ROW_BUFFER_SIZE]; /* for each filter */

   affirm(row_bytes <= PNG_ROW_BUFFER_SIZE && ps != NULL);
   debug((row_bits % bpp) == 0U && filters_to_try > 0x7U);

   filter_block(prev_row, prev_pixels, unfiltered_row, row_bits, bpp,
         test_buffers[PNG_FILTER_VALUE_SUB-1U],
         test_buffers[PNG_FILTER_VALUE_UP-1U],
         test_buffers[PNG_FILTER_VALUE_AVG-1U],
         test_buffers[PNG_FILTER_VALUE_PAETH-1U]);

   /* Add each test buffer, and the unfiltered row if required, to the current
    * list.
    */
   {
      png_byte filter, ok_filter = PNG_FILTER_VALUE_LAST;

      for (filter=0U; filter < PNG_FILTER_VALUE_LAST; ++filter)
         if ((filters_to_try & PNG_FILTER_MASK(filter)) != 0U)
         {
            if (png_zlib_filter_compress(png_ptr, ps, filter,
                filter == PNG_FILTER_VALUE_NONE ?
                  unfiltered_row : test_buffers[filter-1],
                row_bytes, flush))
               ok_filter = filter;

            else /* remove this filter from the test list: */
               filters_to_try &= PNG_BIC_MASK(PNG_FILTER_MASK(filter));
         }

      /* If nothing worked then there is no recovery possible. */
      if (ok_filter == PNG_FILTER_VALUE_LAST)
         png_error(png_ptr, "filter selection: everything failed");

      /* At end_of_row choose the best filter; it is stored in ok_filter. */
      if (end_of_row)
      {
         png_uint_32 o, l;

         o = l = 0xFFFFFFFFU;
         ok_filter = PNG_FILTER_VALUE_LAST;

         for (filter=0U; filter < PNG_FILTER_VALUE_LAST; ++filter)
            if ((filters_to_try & PNG_FILTER_MASK(filter)) != 0U)
               if (ps->filter[filter].overflow < o ||
                   (ps->filter[filter].overflow == o &&
                    ps->filter[filter].len < l))
               {
                  ok_filter = filter;
                  o = ps->filter[filter].overflow;
                  l = ps->filter[filter].len;
               }
      }

      /* Keep going if there is more than one filter left, otherwise, if there
       * is only one left (because of OOM killing filters) swap back to the
       * main-line code using 'ok_filter'.
       */
      else if ((filters_to_try & (filters_to_try-1U)) != 0U)
         ok_filter = PNG_FILTER_VALUE_LAST; /* keep going */

      /* Swap back to the mainline code at end of row or when the available
       * filter count drops to one because of OOM.
       */
      if (ok_filter < PNG_FILTER_VALUE_LAST)
      {
         png_zlib_filter_revert_and_release(png_ptr, ps, ok_filter);
         png_write_IDAT(png_ptr, flush == Z_FINISH);
         ps->filters = ok_filter;
      }

      else
      {
         ps->filters = PNG_ALL_FILTERS & (filters_to_try &= PNG_ALL_FILTERS);
         debug((filters_to_try & (filters_to_try-1U)) != 0U);
      }
   }
}
#endif /* SELECT_FILTER_METHODICALLY */

/* This filters the row, chooses which filter to use, if it has not already
 * been specified by the application, and then writes the row out with the
 * chosen filter.
 */
void /* PRIVATE */
png_write_filter_row(png_structrp png_ptr, png_bytep prev_pixels,
      png_const_bytep unfiltered_row, png_uint_32 x,
      unsigned int width/*pixels*/, unsigned int row_info_flags)
{
   png_zlib_statep ps;
   png_bytep prev_row = png_ptr->row_buffer;
   const unsigned int bpp = png_ptr->row_output_pixel_depth;
   const unsigned int row_bits = width * bpp;
   unsigned int filters_to_try;
   int flush;

   /* These invariants are expected from the caller: */
   affirm(width < 65536U && bpp <= 64U && width < 65536U/bpp &&
         row_bits <= 8U*PNG_ROW_BUFFER_SIZE);

   /* Set up the IDAT zlib compression if not set up yet: */
   if (png_ptr->zowner != png_IDAT)
      png_start_IDAT(png_ptr);

   ps = png_ptr->zlib_state;
   affirm(ps != NULL);
   flush = row_flush(ps, row_info_flags);

   if (x == 0U) /* start of row */
   {
      /* Now work out the filters to try for this row: */
      filters_to_try = ps->filter_mask;

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
            if (!(row_info_flags & png_pass_first_row))
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

      if ((row_info_flags & png_pass_first_row) != 0U)
      {
         /* On the first row UP and NONE are the same, PAETH and SUB are the
          * same, so if both members of a pair occur together eliminate the one
          * that depends on the previous row.  This will avoid the filter
          * selection code while allowing the app to ensure all the filters can
          * be used (prev_row is allocated) on the first row.
          */
#        define match(mask) (filters_to_try & (mask)) == mask
         if (match(PNG_FILTER_NONE+PNG_FILTER_UP))
            filters_to_try &= PNG_BIC_MASK(PNG_FILTER_UP);

         if (match(PNG_FILTER_SUB+PNG_FILTER_PAETH))
            filters_to_try &= PNG_BIC_MASK(PNG_FILTER_PAETH);
#        undef match
      }

      /* If there is no selection algorithm enabled choose the first filter
       * in the list, otherwise do algorithm-specific initialization.
       */
      if ((filters_to_try & (filters_to_try-1U)) != 0U)
      {
         /* Multiple filters in the list. */
#        ifdef PNG_SELECT_FILTER_METHODICALLY_SUPPORTED
            if (!ps->filter_oom &&
                (methodical_option == PNG_OPTION_ON ||
                 (methodical_option != PNG_OPTION_OFF &&
                  heuristic_option != PNG_OPTION_ON)))
               filters_to_try =
                  select_filter_methodically_init(png_ptr, filters_to_try);

            else /* don't do methodical selection */
#        endif /* SELECT_FILTER_METHODICALLY */
#        ifdef PNG_SELECT_FILTER_HEURISTICALLY_SUPPORTED
            if (heuristic_option != PNG_OPTION_OFF) /* use heuristics */
            {
               /* The heuristic must select a single filter based on the first
                * block of pixels; it updates zbuffer_filter to a single filter
                * value.
                */
               ps->filters = select_filter_heuristically(png_ptr,
                     filters_to_try,
                     (row_info_flags & png_pass_first_row) ? NULL : prev_row,
                     prev_pixels, unfiltered_row, row_bits, bpp, flush);

               /* This has selected one filter and has already processed it but
                * the current row must still be retained regardless if prev_row
                * is non-NULL.
                */
               goto copy_row;
            }

            else /* don't use heuristic selection */
#        endif /* SELECT_FILTER_HEURISTICALLY */
         filters_to_try &= -filters_to_try;
      }

      /* If there is just one bit set in filters_to_try convert it to the filter
       * value and store that.
       */
      if ((filters_to_try & (filters_to_try-1U)) == 0U) switch (filters_to_try)
      {
         case PNG_FILTER_NONE:  filters_to_try = PNG_FILTER_VALUE_NONE;  break;
         case PNG_FILTER_SUB:   filters_to_try = PNG_FILTER_VALUE_SUB;   break;
         case PNG_FILTER_UP:    filters_to_try = PNG_FILTER_VALUE_UP;    break;
         case PNG_FILTER_AVG:   filters_to_try = PNG_FILTER_VALUE_AVG;   break;
         case PNG_FILTER_PAETH: filters_to_try = PNG_FILTER_VALUE_PAETH; break;
         default:
            impossible("bad filter mask");
      }

      ps->filters = PNG_ALL_FILTERS & filters_to_try;
   } /* start of row */

   else
   {
      if (prev_row != NULL)
      {
         /* Advance prev_row to the corresponding pixel above row[x], must use
          * png_calc_rowbytes here otherwise the calculation using x might
          * overflow.
          */
         debug(((x * bpp) & 7U) == 0U);
         prev_row += png_calc_rowbytes(png_ptr, bpp, x);
      }

      filters_to_try = ps->filters;
   }

   /* Now choose the correct filter implementation according to the number of
    * filters in the filters_to_try list.  The prev_row parameter is made NULL
    * on the first row because it is uninitialized at that point.
    */
   if (filters_to_try == PNG_FILTER_VALUE_NONE)
      write_unfiltered_rowbits(png_ptr, unfiltered_row, row_bits,
            x == 0 ? PNG_FILTER_VALUE_NONE : PNG_FILTER_VALUE_LAST, flush);

   else
   {
      png_const_bytep prev =
         (row_info_flags & png_pass_first_row) ? NULL : prev_row;

      /* Is just one bit set in 'filters_to_try'? */
      if (filters_to_try < PNG_FILTER_VALUE_LAST)
         filter_row(png_ptr, prev, prev_pixels, unfiltered_row, row_bits, bpp,
               filters_to_try, x == 0, flush);

      else
#        ifdef PNG_SELECT_FILTER_METHODICALLY_SUPPORTED
            select_filter_methodically(png_ptr, prev, prev_pixels,
                  unfiltered_row, row_bits, bpp, filters_to_try,
                  (row_info_flags & png_row_end) != 0U, flush);
#        else
            impossible("bad filter select logic");
#        endif /* SELECT_FILTER_METHODICALLY */
   }

#  ifdef PNG_WRITE_FLUSH_SUPPORTED
      if (flush == Z_SYNC_FLUSH)
      {
         png_flush(png_ptr);
         ps->flush_rows = 0U;
      }
#  endif /* WRITE_FLUSH */

   /* Copy the current row into the previous row buffer, if available, unless
    * this is the last row in the pass, when there is no point.  Note that
    * prev_row may have garbage in a partial byte at the end.
    */
copy_row:
   if (prev_row != NULL && !(row_info_flags & png_pass_last_row))
      memcpy(prev_row, unfiltered_row, (row_bits + 7U) >> 3);
}

/* Allow the application to select one or more row filters to use. */
void PNGAPI
png_set_filter(png_structrp png_ptr, int method, int filtersIn)
{
   unsigned int filters;

   png_debug(1, "in png_set_filter");

   if (png_ptr == NULL)
      return;

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

   debug(filters != 0U && (filters & PNG_BIC_MASK(PNG_ALL_FILTERS)) == 0U);

   {
      png_zlib_statep ps = png_get_zlib_state(png_ptr);

      if (ps != NULL)
         ps->filter_mask = png_check_bits(png_ptr, filters, 8);

      else
         png_app_error(png_ptr, "png_set_filter: invalid on read struct");
   }
}
#else /* !WRITE_FILTER */
void /* PRIVATE */
png_write_filter_row(png_structrp png_ptr, png_bytep prev_pixels,
      png_const_bytep unfiltered_row, png_uint_32 x,
      unsigned int width/*pixels*/, unsigned int row_info_flags)
{
   const unsigned int bpp = png_ptr->row_output_pixel_depth;
   int flush;
   png_uint_32 row_bits;

   row_bits = width;
   row_bits *= bpp;
   /* These invariants are expected from the caller: */
   affirm(width < 65536U && bpp <= 64U && width < 65536U/bpp &&
         row_bits <= 8U*PNG_ROW_BUFFER_SIZE);

   /* Set up the IDAT zlib compression if not set up yet: */
   if (png_ptr->zowner != png_IDAT)
      png_start_IDAT(png_ptr);

   affirm(png_ptr->zlib_state != NULL);
   flush = row_flush(png_ptr->zlib_state, row_info_flags);

   write_unfiltered_rowbits(png_ptr, unfiltered_row, row_bits,
         x == 0 ? PNG_FILTER_VALUE_NONE : PNG_FILTER_VALUE_LAST, flush);

#  ifdef PNG_WRITE_FLUSH_SUPPORTED
      if (flush == Z_SYNC_FLUSH)
      {
         png_flush(png_ptr);
         png_ptr->zlib_state->flush_rows = 0U;
      }
#  endif /* WRITE_FLUSH */

   PNG_UNUSED(prev_pixels);
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
   png_zlib_statep ps = png_get_zlib_state(png_ptr);

   png_debug(1, "in png_set_compression_level");

   if (ps != NULL)
      ps->zlib_level = level;
}

void PNGAPI
png_set_compression_mem_level(png_structrp png_ptr, int mem_level)
{
   png_zlib_statep ps = png_get_zlib_state(png_ptr);

   png_debug(1, "in png_set_compression_mem_level");

   if (ps != NULL)
      ps->zlib_mem_level = mem_level;
}

void PNGAPI
png_set_compression_strategy(png_structrp png_ptr, int strategy)
{
   png_zlib_statep ps = png_get_zlib_state(png_ptr);

   png_debug(1, "in png_set_compression_strategy");

   if (ps != NULL)
      ps->zlib_strategy = strategy;
}

/* If PNG_WRITE_OPTIMIZE_CMF_SUPPORTED is defined, libpng will use a
 * smaller value of window_bits if it can do so safely.
 */
void PNGAPI
png_set_compression_window_bits(png_structrp png_ptr, int window_bits)
{
   png_zlib_statep ps = png_get_zlib_state(png_ptr);

   if (ps != NULL)
   {
      /* Prior to 1.6.0 this would warn but then set the window_bits value. This
       * meant that negative window bits values could be selected that would
       * cause libpng to write a non-standard PNG file with raw deflate or gzip
       * compressed IDAT or ancillary chunks.  Such files can be read and there
       * is no warning on read, so this seems like a very bad idea.
       */
      if (window_bits > 15)
      {
         png_app_warning(png_ptr,
               "Only compression windows <= 32k supported by PNG");
         window_bits = 15;
      }

      else if (window_bits < 8)
      {
         png_app_warning(png_ptr,
               "Only compression windows >= 256 supported by PNG");
         window_bits = 8;
      }

      ps->zlib_window_bits = window_bits;
   }
}

void PNGAPI
png_set_compression_method(png_structrp png_ptr, int method)
{
   png_zlib_statep ps = png_get_zlib_state(png_ptr);

   png_debug(1, "in png_set_compression_method");

   if (ps != NULL)
   {
      /* This used to just warn, this seems unhelpful and might result in bogus
       * PNG files if zlib starts accepting other methods.
       */
      if (method == 8)
         ps->zlib_method = method;

      else
         png_app_error(png_ptr,
               "Only compression method 8 is supported by PNG");
   }
}
#endif /* WRITE_CUSTOMIZE_COMPRESSION */

/* The following were added to libpng-1.5.4 */
#ifdef PNG_WRITE_CUSTOMIZE_ZTXT_COMPRESSION_SUPPORTED
void PNGAPI
png_set_text_compression_level(png_structrp png_ptr, int level)
{
   png_zlib_statep ps = png_get_zlib_state(png_ptr);

   png_debug(1, "in png_set_text_compression_level");

   if (ps != NULL)
      ps->zlib_text_level = level;
}

void PNGAPI
png_set_text_compression_mem_level(png_structrp png_ptr, int mem_level)
{
   png_zlib_statep ps = png_get_zlib_state(png_ptr);

   png_debug(1, "in png_set_text_compression_mem_level");

   if (ps != NULL)
      ps->zlib_text_mem_level = mem_level;
}

void PNGAPI
png_set_text_compression_strategy(png_structrp png_ptr, int strategy)
{
   png_zlib_statep ps = png_get_zlib_state(png_ptr);

   png_debug(1, "in png_set_text_compression_strategy");

   if (ps != NULL)
      ps->zlib_text_strategy = strategy;
}

/* If PNG_WRITE_OPTIMIZE_CMF_SUPPORTED is defined, libpng will use a
 * smaller value of window_bits if it can do so safely.
 */
void PNGAPI
png_set_text_compression_window_bits(png_structrp png_ptr, int window_bits)
{
   png_zlib_statep ps = png_get_zlib_state(png_ptr);

   if (ps != NULL)
   {
      if (window_bits > 15)
      {
         png_app_warning(png_ptr,
               "Only compression windows <= 32k supported by PNG");
         window_bits = 15;
      }

      else if (window_bits < 8)
      {
         png_app_error(png_ptr,
               "Only compression windows >= 256 supported by PNG");
         window_bits = 8;
      }

      ps->zlib_text_window_bits = window_bits;
   }
}

void PNGAPI
png_set_text_compression_method(png_structrp png_ptr, int method)
{
   png_zlib_statep ps = png_get_zlib_state(png_ptr);

   png_debug(1, "in png_set_text_compression_method");

   if (ps != NULL)
   {
      if (method == 8)
         ps->zlib_text_method = method;

      else
         png_app_error(png_ptr,
               "Only compression method 8 is supported by PNG");

   }
}
#endif /* WRITE_CUSTOMIZE_ZTXT_COMPRESSION */
/* end of API added to libpng-1.5.4 */

#endif /* WRITE */
