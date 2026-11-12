/* pngbigrow.c - row transforms on rows whose width*channels wraps 32 bits
 *
 * Copyright (c) 2026 galdo
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * png_do_swap() and png_do_shift() computed their loop bounds as
 * png_uint_32 products (width*channels), which wrap to a small value
 * once the row has more than 2^32-1 samples.  A row of that size is
 * only reachable after png_set_user_limits() raises PNG_USER_WIDTH_MAX,
 * but it is then a perfectly ordinary row and the transforms silently
 * under-process it: the samples past the wrapped count keep their input
 * values in the written PNG.
 *
 * This program writes such rows through the ordinary write API with the
 * row filter disabled and compression level 0, so the bytes libpng emits
 * are the transformed row itself (inside the IDAT deflate stream).  It
 * counts the output samples that carry the transformed pattern versus
 * the untransformed one:
 *
 *   - a small control width (no wrap) must transform every sample;
 *   - width = 0x40000001 with 4 channels makes width*channels wrap to 4
 *     in png_uint_32, so an unpatched libpng transforms 4 samples of
 *     4,294,967,300 and leaves the rest untouched.
 *
 * The wrap cases need a row of 4 GiB (8-bit) or 8 GiB (16-bit) and an
 * equally sized libpng row buffer; a case that cannot fit is skipped
 * (when the allocation fails, or when the available memory leaves no
 * headroom for both buffers), and if no wrap case can run at all the
 * test exits 77.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__unix__) || defined(__APPLE__)
#  include <unistd.h>
#endif

#include "png.h"

#define CASE_WIDTH      ((png_uint_32)0x40000001U) /* width*4 wraps to 4 */
#define CONTROL_WIDTH   ((png_uint_32)1000U)
#define CHANNELS        4 /* RGBA: the 4-channel product is the one at risk */
#define PNG_USER_LIMIT  ((png_uint_32)0x7fffffffU)

/* ------------------------------------------------------------------ */
/* Streaming extraction of the row bytes from a written PNG.           */
/*                                                                     */
/* The PNG chunk layer and the IDAT deflate layer are both parsed on    */
/* the fly; the deflate stream is expected to consist of stored blocks  */
/* only, because the test writes with compression level 0.  The         */
/* recovered byte stream is one PNG row filter byte followed by the     */
/* row samples, because the test writes with PNG_FILTER_NONE.           */
/* ------------------------------------------------------------------ */

enum walker_state
{
   W_SIG,          /* PNG signature (8 bytes) */
   W_CHUNK_LEN,    /* chunk length (4 bytes) */
   W_CHUNK_TYPE,   /* chunk type (4 bytes) */
   W_CHUNK_DATA,   /* chunk data (chunk_len bytes) */
   W_CHUNK_CRC,    /* chunk CRC (4 bytes) */
   W_DONE
};

enum inflate_state
{
   I_ZHDR,         /* zlib CMF/FLG (2 bytes) */
   I_BLOCKHDR,     /* 3 header bits + padding, at a byte boundary */
   I_LEN_LO,
   I_LEN_HI,
   I_NLEN_LO,
   I_NLEN_HI,
   I_DATA,         /* stored block payload */
   I_END
};

typedef struct
{
   /* PNG framing */
   enum walker_state state;
   unsigned char chdr[8]; /* staging for the fixed-size chunk fields */
   size_t chdr_have;
   unsigned long chunk_len, chunk_have;
   unsigned char chunk_type[4];
   unsigned long idat_total;

   /* deflate framing (inside IDAT) */
   enum inflate_state istate;
   int zh_have; /* zlib CMF/FLG bytes seen */
   unsigned char nlen_lo;
   unsigned long block_len, block_have;
   int block_final;

   /* row stream */
   int filter_byte;              /* -1 until seen */
   size_t rowbytes;              /* row samples only, no filter byte */
   size_t row_seen;
   size_t sample_bytes;
   unsigned char sample[8], pending[8];
   size_t pending_have;

   /* classification */
   unsigned char ref_raw[8], ref_exp[8];
   size_t n_exp, n_raw, n_other;
   size_t first_raw, last_raw, first_other, last_other;

   /* errors */
   int bad;
   const char *bad_msg;
} walker;

static void
walker_init(walker *w, size_t rowbytes, size_t sample_bytes,
    const unsigned char *raw, const unsigned char *exp)
{
   memset(w, 0, sizeof *w);
   w->state = W_SIG;
   w->istate = I_ZHDR;
   w->filter_byte = -1;
   w->rowbytes = rowbytes;
   w->sample_bytes = sample_bytes;
   memcpy(w->ref_raw, raw, sample_bytes);
   memcpy(w->ref_exp, exp, sample_bytes);
   w->first_raw = w->last_raw = (size_t)-1;
   w->first_other = w->last_other = (size_t)-1;
}

static void
walker_sample(walker *w)
{
   size_t n = w->row_seen / w->sample_bytes;

   w->row_seen += w->sample_bytes;
   if (w->sample[0] == w->ref_exp[0] &&
       (w->sample_bytes == 1 || w->sample[1] == w->ref_exp[1]))
      w->n_exp++;

   else if (w->sample[0] == w->ref_raw[0] &&
       (w->sample_bytes == 1 || w->sample[1] == w->ref_raw[1]))
   {
      if (w->first_raw == (size_t)-1)
         w->first_raw = n;
      w->last_raw = n;
      w->n_raw++;
   }

   else
   {
      if (w->first_other == (size_t)-1)
         w->first_other = n;
      w->last_other = n;
      w->n_other++;
   }
}

static void
walker_row_byte(walker *w, unsigned char b)
{
   if (w->filter_byte < 0)
   {
      w->filter_byte = b;
      return; /* the row filter byte: PNG_FILTER_VALUE_NONE expected */
   }

   if (w->row_seen + w->pending_have >= w->rowbytes)
   {
      if (!w->bad)
      {
         w->bad = 1;
         w->bad_msg = "row stream longer than rowbytes";
      }
      return;
   }

   w->pending[w->pending_have++] = b;
   if (w->pending_have == w->sample_bytes)
   {
      memcpy(w->sample, w->pending, w->sample_bytes);
      w->pending_have = 0;
      walker_sample(w);
   }
}

/* Feed one deflate byte (an IDAT payload byte) to the inflater. */
static void
walker_idat_byte(walker *w, unsigned char b)
{
   switch (w->istate)
   {
      case I_ZHDR:
         if (++w->zh_have == 2)
            w->istate = I_BLOCKHDR;
         break;

      case I_BLOCKHDR:
         w->block_final = b & 1;
         if (((b >> 1) & 3) != 0 && !w->bad)
         {
            w->bad = 1;
            w->bad_msg = "unexpected non-stored deflate block";
            w->istate = I_END;
            break;
         }
         w->istate = I_LEN_LO;
         break;

      case I_LEN_LO:
         w->block_len = b;
         w->istate = I_LEN_HI;
         break;

      case I_LEN_HI:
         w->block_len |= (unsigned long)b << 8;
         w->istate = I_NLEN_LO;
         break;

      case I_NLEN_LO:
         w->nlen_lo = b;
         w->istate = I_NLEN_HI;
         break;

      case I_NLEN_HI:
         if ((w->block_len ^ 0xffffUL) !=
             ((unsigned long)w->nlen_lo | ((unsigned long)b << 8)))
         {
            if (!w->bad)
            {
               w->bad = 1;
               w->bad_msg = "stored block LEN/NLEN mismatch";
            }
            w->istate = I_END;
            break;
         }
         w->block_have = 0;
         if (w->block_len == 0 && w->block_final)
            w->istate = I_END;
         else
            w->istate = I_DATA;
         break;

      case I_DATA:
         walker_row_byte(w, b);
         if (++w->block_have == w->block_len)
            w->istate = w->block_final ? I_END : I_BLOCKHDR;
         break;

      case I_END:
         break; /* trailing adler32: ignored */
   }
}

static void
walker_feed(walker *w, const unsigned char *data, size_t len)
{
   while (len > 0 && w->state != W_DONE && !w->bad)
   {
      size_t n;

      switch (w->state)
      {
         case W_SIG:
            n = 8 - w->chdr_have;
            if (n > len)
               n = len;
            memcpy(w->chdr + w->chdr_have, data, n);
            w->chdr_have += n;
            data += n;
            len -= n;
            if (w->chdr_have == 8)
            {
               w->chdr_have = 0;
               w->state = W_CHUNK_LEN;
            }
            break;

         case W_CHUNK_LEN:
         case W_CHUNK_TYPE:
         case W_CHUNK_CRC:
            n = 4 - w->chdr_have;
            if (n > len)
               n = len;
            memcpy(w->chdr + w->chdr_have, data, n);
            w->chdr_have += n;
            data += n;
            len -= n;
            if (w->chdr_have == 4)
            {
               w->chdr_have = 0;
               if (w->state == W_CHUNK_LEN)
               {
                  w->chunk_len = ((unsigned long)w->chdr[0] << 24) |
                      ((unsigned long)w->chdr[1] << 16) |
                      ((unsigned long)w->chdr[2] << 8) | w->chdr[3];
                  w->chunk_have = 0;
                  w->state = W_CHUNK_TYPE;
               }
               else if (w->state == W_CHUNK_TYPE)
               {
                  memcpy(w->chunk_type, w->chdr, 4);
                  if (w->chunk_len == 0)
                     w->state = W_CHUNK_CRC;
                  else
                     w->state = W_CHUNK_DATA;
               }
               else /* W_CHUNK_CRC: a whole chunk has been consumed */
               {
                  if (w->chunk_type[0] == 'I' && w->chunk_type[1] == 'E' &&
                      w->chunk_type[2] == 'N' && w->chunk_type[3] == 'D')
                     w->state = W_DONE;
                  else
                     w->state = W_CHUNK_LEN;
               }
            }
            break;

         case W_CHUNK_DATA:
         {
            unsigned char is_idat = (w->chunk_type[0] == 'I' &&
                w->chunk_type[1] == 'D' && w->chunk_type[2] == 'A' &&
                w->chunk_type[3] == 'T');

            n = w->chunk_len - w->chunk_have;
            if (n > len)
               n = len;
            if (is_idat)
            {
               size_t i;

               w->idat_total += n;
               for (i = 0; i < n; ++i)
                  walker_idat_byte(w, data[i]);
            }
            w->chunk_have += n;
            data += n;
            len -= n;
            if (w->chunk_have == w->chunk_len)
               w->state = W_CHUNK_CRC;
            break;
         }

         case W_DONE:
            break;
      }
   }
}

static void PNGCBAPI
sink_write(png_structp png_ptr, png_bytep data, size_t len)
{
   walker *w = (walker *)png_get_io_ptr(png_ptr);

   walker_feed(w, data, len);
}

static void PNGCBAPI
sink_flush(png_structp png_ptr)
{
   (void)png_ptr;
}

/* ------------------------------------------------------------------ */
/* Test cases                                                          */
/* ------------------------------------------------------------------ */

typedef struct
{
   const char *name;
   png_uint_32 width;
   int bit_depth;
   int use_swap;
   int use_shift;
   int sbit; /* significant bits per channel, for png_set_shift */
} casedef;

static void
make_expected(const casedef *c, const unsigned char *raw,
    unsigned char *exp)
{
   if (c->use_swap)
   {
      /* png_do_swap: exchange the two bytes of each 16-bit sample. */
      exp[0] = raw[1];
      exp[1] = raw[0];
   }

   else if (c->bit_depth == 8)
   {
      /* png_do_shift: bit replication of the sbit significant bits. */
      unsigned int v = raw[0];
      unsigned int out = 0;
      int j;

      for (j = 8 - c->sbit; j > -c->sbit; j -= c->sbit)
      {
         if (j > 0)
            out |= v << j;
         else
            out |= v >> (-j);
      }
      exp[0] = (unsigned char)(out & 0xff);
   }

   else
   {
      unsigned int v = ((unsigned int)raw[0] << 8) | raw[1];
      unsigned int out = 0;
      int j;

      for (j = 16 - c->sbit; j > -c->sbit; j -= c->sbit)
      {
         if (j > 0)
            out |= v << j;
         else
            out |= v >> (-j);
      }
      exp[0] = (unsigned char)((out >> 8) & 0xff);
      exp[1] = (unsigned char)(out & 0xff);
   }
}

/* One sample's raw (input) bytes. */
static void
make_raw(const casedef *c, unsigned char *raw)
{
   if (c->use_swap)
   {
      raw[0] = 0x12;
      raw[1] = 0x34;
   }
   else if (c->bit_depth == 8)
      raw[0] = 0x01;
   else
   {
      raw[0] = 0x00;
      raw[1] = 0x01;
   }
}

/* Estimate the memory that is really available, or (size_t)-1 if that
 * cannot be determined.  The wrap cases need two row buffers (the caller's
 * row and libpng's own), and malloc alone is not a safe probe: on
 * overcommitting systems it succeeds and the process is killed later,
 * which would take the whole test run down with it.
 */
static size_t
available_memory(void)
{
#if defined(__unix__) || defined(__APPLE__)
   long pages = sysconf(_SC_AVPHYS_PAGES);
   long page_size = sysconf(_SC_PAGESIZE);

   if (pages > 0 && page_size > 0)
      return (size_t)pages * (size_t)page_size;
#endif
   return (size_t)-1;
}

/* Returns 0 pass, 1 fail, 77 skip. */
static int
run_case(const casedef *c)
{
   unsigned char raw[8], exp[8];
   size_t sample_bytes = (size_t)(c->bit_depth / 8);
   size_t rowbytes = (size_t)c->width * CHANNELS * sample_bytes;
   size_t true_samples = (size_t)c->width * CHANNELS;
   png_uint_32 wrapped = (png_uint_32)(c->width * (png_uint_32)CHANNELS);
   unsigned char *row;
   size_t off;
   png_structp png_ptr;
   png_infop info_ptr;
   png_color_8 sbit;
   walker w;
   int rc = 0;

   make_raw(c, raw);
   make_expected(c, raw, exp);

   printf("%s: width=%lu channels=%d bit_depth=%d\n", c->name,
       (unsigned long)c->width, CHANNELS, c->bit_depth);
   printf("  width*channels: png_uint_32 %lu%s, true %lu\n",
       (unsigned long)wrapped,
       ((size_t)wrapped != true_samples ? " (WRAPPED)" : ""),
       (unsigned long)true_samples);

   {
      /* Two row buffers (the row passed to png_write_row and libpng's own
       * row_buf) plus slack; require some headroom on top of that.
       */
      size_t needed = 2 * rowbytes + 64 * 1024 * 1024;
      size_t avail = available_memory();

      if (avail != (size_t)-1 && avail < needed + needed / 4)
      {
         printf("  SKIP: this case needs about %lu bytes, only %lu available\n",
             (unsigned long)needed, (unsigned long)avail);
         return 77;
      }
   }

   row = (unsigned char *)malloc(rowbytes);
   if (row == NULL)
   {
      printf("  SKIP: cannot allocate %lu byte row\n",
          (unsigned long)rowbytes);
      return 77;
   }

   /* Fill the whole row with the raw sample pattern so that every
    * output sample can be classified.
    */
   {
      unsigned char block[256];

      for (off = 0; off < sizeof block; off += sample_bytes)
         memcpy(block + off, raw, sample_bytes);

      for (off = 0; off + sizeof block <= rowbytes; off += sizeof block)
         memcpy(row + off, block, sizeof block);

      for (; off + sample_bytes <= rowbytes; off += sample_bytes)
         memcpy(row + off, raw, sample_bytes);
   }

   walker_init(&w, rowbytes, sample_bytes, raw, exp);

   png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   info_ptr = png_create_info_struct(png_ptr);
   if (png_ptr == NULL || info_ptr == NULL || setjmp(png_jmpbuf(png_ptr)))
   {
      printf("  FAIL: libpng error while writing the row\n");
      png_destroy_write_struct(&png_ptr, &info_ptr);
      free(row);
      return 1;
   }

   png_set_write_fn(png_ptr, &w, sink_write, sink_flush);
   png_set_user_limits(png_ptr, PNG_USER_LIMIT, PNG_USER_LIMIT);
   png_set_IHDR(png_ptr, info_ptr, c->width, 1, c->bit_depth,
       PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
       PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

   /* Emit the transformed row verbatim: no row filter, no compression. */
   png_set_filter(png_ptr, PNG_FILTER_TYPE_BASE, PNG_FILTER_NONE);
   png_set_compression_level(png_ptr, 0);

   png_write_info(png_ptr, info_ptr);

   /* png_set_swap and png_set_shift only take effect once png_ptr has the
    * image bit depth and color type, which png_write_info copies from the
    * IHDR written above; before that the setters are no-ops or reject the
    * values.  They still apply to every row written afterwards.
    */
   if (c->use_swap)
      png_set_swap(png_ptr);

   if (c->use_shift)
   {
      sbit.red = sbit.green = sbit.blue = sbit.alpha =
          (png_byte)c->sbit;
      sbit.gray = 0;
      png_set_shift(png_ptr, &sbit);
   }

   png_write_row(png_ptr, row);
   png_write_end(png_ptr, info_ptr);
   png_destroy_write_struct(&png_ptr, &info_ptr);
   free(row);

   if (w.bad)
   {
      printf("  FAIL: output stream: %s\n", w.bad_msg);
      return 1;
   }

   if (w.filter_byte != 0)
   {
      printf("  FAIL: row filter byte is %d, expected 0\n", w.filter_byte);
      return 1;
   }

   if (w.row_seen != rowbytes)
   {
      printf("  FAIL: recovered %lu row bytes, expected %lu\n",
          (unsigned long)w.row_seen, (unsigned long)rowbytes);
      return 1;
   }

   printf("  output samples: %lu total, %lu transformed, %lu untransformed,"
       " %lu other\n", (unsigned long)true_samples,
       (unsigned long)w.n_exp, (unsigned long)w.n_raw,
       (unsigned long)w.n_other);

   if (w.n_raw > 0)
      printf("  untransformed samples: first #%lu, last #%lu\n",
          (unsigned long)w.first_raw, (unsigned long)w.last_raw);

   if (w.n_other > 0)
      printf("  unexpected samples: first #%lu, last #%lu\n",
          (unsigned long)w.first_other, (unsigned long)w.last_other);

   if (w.n_exp != true_samples)
   {
      printf("  FAIL: transform applied to %lu of %lu samples\n",
          (unsigned long)w.n_exp, (unsigned long)true_samples);
      rc = 1;
   }
   else
      printf("  PASS: transform applied to every sample\n");

   return rc;
}

int
main(void)
{
   static const casedef cases[] =
   {
      /* Control widths: width*channels cannot wrap; every sample must be
       * transformed.  These also validate the expected patterns.
       */
      { "swap-control",  CONTROL_WIDTH, 16, 1, 0, 0 },
      { "shift8-control", CONTROL_WIDTH, 8, 0, 1, 4 },
      { "shift16-control", CONTROL_WIDTH, 16, 0, 1, 12 },
      /* Wrap widths: width*channels = 0x100000004, which is 4 in
       * png_uint_32.  png_do_swap/png_do_shift used to process 4 samples
       * and leave the rest of the row untouched.
       */
      { "swap-wrap",     CASE_WIDTH, 16, 1, 0, 0 },
      { "shift8-wrap",   CASE_WIDTH, 8, 0, 1, 4 },
      { "shift16-wrap",  CASE_WIDTH, 16, 0, 1, 12 }
   };
   int result = 0;
   int ran_wrap = 0;
   size_t i;

   for (i = 0; i < sizeof cases / sizeof cases[0]; ++i)
   {
      int rc = run_case(&cases[i]);

      if (rc == 77 && cases[i].width == CASE_WIDTH)
         continue; /* skip: no memory for the big row */

      if (rc == 77)
      {
         /* The control rows are tiny; failing to allocate one is a real
          * failure, not a skip.
          */
         printf("Testing %s... FAIL (out of memory on a small row)\n",
             cases[i].name);
         result = 1;
         continue;
      }

      if (cases[i].width == CASE_WIDTH)
         ran_wrap = 1;

      if (rc != 0)
         result = 1;
   }

   if (!ran_wrap)
   {
      printf("SKIP: not enough memory for any wrap-width row\n");
      return 77;
   }

   return result;
}
