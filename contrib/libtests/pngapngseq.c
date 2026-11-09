/* pngapngseq.c
 *
 * Copyright (c) 2026 Nathan Keys
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * Read synthetic APNG streams frame by frame, with the sequential reader
 * (png_read_frame_head) and with the progressive reader, and compare every
 * pixel of every frame with what was encoded.
 *
 * Two pieces of per-frame state of the sequential reader are checked:
 *
 * 1. An application may call png_read_update_info after png_read_frame_head,
 *    as it does before the first frame.  png_get_rowbytes must then describe a
 *    row of the new frame, not a row of the IHDR canvas, and the frame must
 *    decode.
 *
 * 2. The deflate stream of every frame ends with an Adler-32 checksum.  When
 *    the last chunk of a frame carries only part of the end of the stream, the
 *    image rows are complete before the stream is, and the reader has to
 *    consume the rest of the stream at the end of the frame, as it does for
 *    IDAT.  Valid streams split that way must decode, and a damaged checksum
 *    must be reported while reading the frame it belongs to, for every frame,
 *    as it is for IDAT.
 *
 * The streams are built here with stored (uncompressed) deflate blocks, so
 * where each chunk ends does not depend on the zlib in use.
 */

#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(HAVE_CONFIG_H) && !defined(PNG_NO_CONFIG_H)
#  include <config.h>
#endif

#ifdef PNG_FREESTANDING_TESTS
#  include <png.h>
#else
#  include "../../png.h"
#endif

#if defined(PNG_READ_APNG_SUPPORTED) && \
    defined(PNG_SEQUENTIAL_READ_SUPPORTED) && \
    defined(PNG_READ_INTERLACING_SUPPORTED) && \
    defined(PNG_BENIGN_ERRORS_SUPPORTED)

#define CANVAS_WIDTH  13
#define CANVAS_HEIGHT 9
#define MAX_ROWBYTES  (8 * CANVAS_WIDTH) /* 16-bit RGBA */
#define FRAME_COUNT   5

typedef struct
{
   png_uint_32 width;
   png_uint_32 height;
   png_uint_32 x_offset;
   png_uint_32 y_offset;
} frame_geometry;

/* Frame 0 is the IDAT image and covers the canvas; the others are narrower
 * than the canvas except frame 2, which follows a narrow frame.
 */
static const frame_geometry frame_list[FRAME_COUNT] =
{
   { CANVAS_WIDTH, CANVAS_HEIGHT, 0, 0 },
   { 5, 3, 4, 2 },
   { CANVAS_WIDTH, CANVAS_HEIGHT, 0, 0 },
   { 1, 1, 12, 8 },
   { 7, 6, 3, 1 }
};

typedef struct
{
   int color_type;
   int bit_depth;
} image_format;

static const image_format format_list[] =
{
   { PNG_COLOR_TYPE_GRAY, 1 },
   { PNG_COLOR_TYPE_GRAY, 2 },
   { PNG_COLOR_TYPE_GRAY, 4 },
   { PNG_COLOR_TYPE_GRAY, 8 },
   { PNG_COLOR_TYPE_GRAY, 16 },
   { PNG_COLOR_TYPE_RGB, 8 },
   { PNG_COLOR_TYPE_RGB, 16 },
   { PNG_COLOR_TYPE_PALETTE, 1 },
   { PNG_COLOR_TYPE_PALETTE, 2 },
   { PNG_COLOR_TYPE_PALETTE, 4 },
   { PNG_COLOR_TYPE_PALETTE, 8 },
   { PNG_COLOR_TYPE_GRAY_ALPHA, 8 },
   { PNG_COLOR_TYPE_GRAY_ALPHA, 16 },
   { PNG_COLOR_TYPE_RGB_ALPHA, 8 },
   { PNG_COLOR_TYPE_RGB_ALPHA, 16 }
};

#define FORMAT_COUNT (sizeof format_list / sizeof format_list[0])

/* How the deflate stream of each frame is cut into IDAT or fdAT chunks. */
typedef struct
{
   size_t chunk_max;  /* largest chunk payload, 0 for one chunk per frame */
   size_t tail;       /* >0: the last 'tail' stream bytes get a chunk alone */
   int corrupt_frame; /* >=0: damage the Adler-32 of this frame */
} chunk_layout;

static const chunk_layout valid_layouts[] =
{
   { 0, 0, -1 },
   { 0, 1, -1 }, /* rows complete, three checksum bytes in the frame chunk */
   { 0, 4, -1 }, /* rows complete, the whole checksum in the last chunk */
   { 0, 7, -1 }, /* the last row needs the last chunk */
   { 5, 4, -1 },
   { 3, 4, -1 }
};

#define VALID_LAYOUT_COUNT (sizeof valid_layouts / sizeof valid_layouts[0])

typedef struct
{
   png_byte *data;
   size_t size;
   size_t allocated;
} byte_buffer;

static void
buffer_append(byte_buffer *b, const void *data, size_t length)
{
   if (b->size + length > b->allocated)
   {
      size_t allocated = b->allocated != 0 ? b->allocated : 256;
      png_byte *new_data;

      while (allocated < b->size + length)
         allocated *= 2;

      new_data = (png_byte *)realloc(b->data, allocated);
      if (new_data == NULL)
      {
         fprintf(stderr, "pngapngseq: out of memory\n");
         exit(1);
      }

      b->data = new_data;
      b->allocated = allocated;
   }

   if (length > 0)
      memcpy(b->data + b->size, data, length);
   b->size += length;
}

static void
put_uint_32(png_byte *p, png_uint_32 value)
{
   p[0] = (png_byte)((value >> 24) & 0xff);
   p[1] = (png_byte)((value >> 16) & 0xff);
   p[2] = (png_byte)((value >> 8) & 0xff);
   p[3] = (png_byte)(value & 0xff);
}

static void
buffer_append_uint_32(byte_buffer *b, png_uint_32 value)
{
   png_byte bytes[4];

   put_uint_32(bytes, value);
   buffer_append(b, bytes, 4);
}

static png_uint_32 crc_table[256];

static void
make_crc_table(void)
{
   png_uint_32 n;

   for (n = 0; n < 256; n++)
   {
      png_uint_32 c = n;
      int k;

      for (k = 0; k < 8; k++)
         c = (c & 1) != 0 ? 0xedb88320U ^ (c >> 1) : c >> 1;

      crc_table[n] = c;
   }
}

static png_uint_32
chunk_crc(const png_byte *data, size_t length)
{
   png_uint_32 c = 0xffffffffU;

   while (length-- > 0)
      c = crc_table[(c ^ *data++) & 0xff] ^ (c >> 8);

   return (c ^ 0xffffffffU) & 0xffffffffU;
}

static void
write_chunk(byte_buffer *b, const char *type, const png_byte *data,
    size_t length)
{
   size_t start;

   buffer_append_uint_32(b, (png_uint_32)length);
   start = b->size;
   buffer_append(b, type, 4);
   buffer_append(b, data, length);
   buffer_append_uint_32(b, chunk_crc(b->data + start, length + 4));
}

static unsigned int
channel_count(int color_type)
{
   switch (color_type)
   {
      case PNG_COLOR_TYPE_RGB:        return 3;
      case PNG_COLOR_TYPE_GRAY_ALPHA: return 2;
      case PNG_COLOR_TYPE_RGB_ALPHA:  return 4;
      default:                        return 1;
   }
}

static unsigned int
pixel_bits(const image_format *format)
{
   return channel_count(format->color_type) * (unsigned int)format->bit_depth;
}

/* Bytes in a row of 'width' pixels (PNG_ROWBYTES is internal to libpng). */
static size_t
row_bytes(unsigned int bits, png_uint_32 width)
{
   return ((size_t)width * bits + 7) >> 3;
}

static unsigned int
palette_size(int bit_depth)
{
   return bit_depth < 8 ? 1U << bit_depth : 16U;
}

/* The encoded value of channel 'c' of pixel (x, y) of frame 'frame'. */
static unsigned int
sample(unsigned int frame, png_uint_32 x, png_uint_32 y, unsigned int c,
    unsigned int limit)
{
   png_uint_32 h = 2166136261U;

   h = ((h ^ frame) * 16777619U) & 0xffffffffU;
   h = ((h ^ x) * 16777619U) & 0xffffffffU;
   h = ((h ^ y) * 16777619U) & 0xffffffffU;
   h = ((h ^ c) * 16777619U) & 0xffffffffU;
   return (unsigned int)((h ^ (h >> 16)) % limit);
}

/* Store pixel (x, y) of 'frame' as the index'th pixel of a packed row. */
static void
put_pixel(png_byte *row, png_uint_32 index, const image_format *format,
    unsigned int frame, png_uint_32 x, png_uint_32 y)
{
   unsigned int channels = channel_count(format->color_type);
   unsigned int depth = (unsigned int)format->bit_depth;
   unsigned int limit = format->color_type == PNG_COLOR_TYPE_PALETTE ?
       palette_size(format->bit_depth) : (depth == 16 ? 65536U : 1U << depth);
   unsigned int c;

   for (c = 0; c < channels; c++)
   {
      unsigned int value = sample(frame, x, y, c, limit);
      size_t bit = ((size_t)index * channels + c) * depth;

      if (depth == 16)
      {
         row[bit >> 3] = (png_byte)(value >> 8);
         row[(bit >> 3) + 1] = (png_byte)(value & 0xff);
      }

      else if (depth == 8)
         row[bit >> 3] = (png_byte)value;

      else
         row[bit >> 3] = (png_byte)(row[bit >> 3] |
             (value << (8 - depth - (bit & 7))));
   }
}

/* Row 'y' of 'frame' as png_read_image delivers it with no transforms. */
static void
expected_row(png_byte *row, const image_format *format, unsigned int frame,
    png_uint_32 width, png_uint_32 y)
{
   png_uint_32 x;

   memset(row, 0, MAX_ROWBYTES);
   for (x = 0; x < width; x++)
      put_pixel(row, x, format, frame, x, y);
}

/* The filtered (filter type None) image data of one frame. */
static void
frame_data(byte_buffer *raw, const image_format *format, int interlace,
    unsigned int frame, png_uint_32 width, png_uint_32 height)
{
   png_byte row[1 + MAX_ROWBYTES];
   int pass;

   for (pass = 0; pass < (interlace != 0 ? 7 : 1); pass++)
   {
      png_uint_32 pass_width = width, pass_height = height;
      png_uint_32 px, py;
      size_t rowbytes;

      if (interlace != 0)
      {
         pass_width = PNG_PASS_COLS(width, pass);
         pass_height = PNG_PASS_ROWS(height, pass);
      }

      if (pass_width == 0 || pass_height == 0)
         continue;

      rowbytes = row_bytes(pixel_bits(format), pass_width);
      for (py = 0; py < pass_height; py++)
      {
         png_uint_32 y = interlace != 0 ? PNG_ROW_FROM_PASS_ROW(py, pass) : py;

         memset(row, 0, sizeof row);
         for (px = 0; px < pass_width; px++)
         {
            png_uint_32 x = interlace != 0 ?
                PNG_COL_FROM_PASS_COL(px, pass) : px;

            put_pixel(row + 1, px, format, frame, x, y);
         }

         buffer_append(raw, row, rowbytes + 1);
      }
   }
}

/* A zlib stream of stored blocks, so every byte boundary is known. */
static void
zlib_stored(byte_buffer *z, const byte_buffer *raw)
{
   static const png_byte zlib_header[2] = { 0x78, 0x01 };
   png_uint_32 a = 1, b = 0;
   size_t offset = 0, i;

   buffer_append(z, zlib_header, 2);
   do
   {
      size_t length = raw->size - offset;
      png_byte header[5];

      if (length > 65535)
         length = 65535;

      header[0] = (png_byte)(offset + length == raw->size);
      header[1] = (png_byte)(length & 0xff);
      header[2] = (png_byte)((length >> 8) & 0xff);
      header[3] = (png_byte)(~length & 0xff);
      header[4] = (png_byte)((~length >> 8) & 0xff);
      buffer_append(z, header, 5);
      buffer_append(z, raw->data + offset, length);
      offset += length;
   }
   while (offset < raw->size);

   for (i = 0; i < raw->size; i++)
   {
      a = (a + raw->data[i]) % 65521U;
      b = (b + a) % 65521U;
   }

   buffer_append_uint_32(z, (b << 16) | a);
}

static void
write_data_chunk(byte_buffer *png, unsigned int frame, png_uint_32 *sequence,
    const png_byte *data, size_t length)
{
   if (frame == 0)
      write_chunk(png, "IDAT", data, length);

   else
   {
      byte_buffer fdat = { NULL, 0, 0 };

      buffer_append_uint_32(&fdat, (*sequence)++);
      buffer_append(&fdat, data, length);
      write_chunk(png, "fdAT", fdat.data, fdat.size);
      free(fdat.data);
   }
}

static void
build_apng(byte_buffer *png, const image_format *format, int interlace,
    const chunk_layout *layout)
{
   static const png_byte signature[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
   png_byte data[3 * 256];
   png_uint_32 sequence = 0;
   unsigned int frame;

   buffer_append(png, signature, 8);

   put_uint_32(data, CANVAS_WIDTH);
   put_uint_32(data + 4, CANVAS_HEIGHT);
   data[8] = (png_byte)format->bit_depth;
   data[9] = (png_byte)format->color_type;
   data[10] = 0;
   data[11] = 0;
   data[12] = (png_byte)interlace;
   write_chunk(png, "IHDR", data, 13);

   if (format->color_type == PNG_COLOR_TYPE_PALETTE)
   {
      unsigned int i, n = palette_size(format->bit_depth);

      for (i = 0; i < n; i++)
      {
         data[3 * i] = (png_byte)((i * 37) & 0xff);
         data[3 * i + 1] = (png_byte)((i * 91) & 0xff);
         data[3 * i + 2] = (png_byte)((255 - i * 13) & 0xff);
      }

      write_chunk(png, "PLTE", data, 3 * n);
   }

   put_uint_32(data, FRAME_COUNT);
   put_uint_32(data + 4, 0);
   write_chunk(png, "acTL", data, 8);

   for (frame = 0; frame < FRAME_COUNT; frame++)
   {
      const frame_geometry *g = &frame_list[frame];
      byte_buffer raw = { NULL, 0, 0 }, z = { NULL, 0, 0 };
      size_t offset, split;

      put_uint_32(data, sequence++);
      put_uint_32(data + 4, g->width);
      put_uint_32(data + 8, g->height);
      put_uint_32(data + 12, g->x_offset);
      put_uint_32(data + 16, g->y_offset);
      data[20] = 0; data[21] = 1;  /* delay 1/10 s */
      data[22] = 0; data[23] = 10;
      data[24] = PNG_fcTL_DISPOSE_OP_NONE;
      data[25] = PNG_fcTL_BLEND_OP_SOURCE;
      write_chunk(png, "fcTL", data, 26);

      frame_data(&raw, format, interlace, frame, g->width, g->height);
      zlib_stored(&z, &raw);
      if (layout->corrupt_frame == (int)frame)
         z.data[z.size - 1] ^= 1;

      split = z.size - layout->tail;
      offset = 0;
      while (offset < z.size)
      {
         size_t length = (offset < split ? split : z.size) - offset;

         if (layout->chunk_max != 0 && length > layout->chunk_max &&
             offset < split)
            length = layout->chunk_max;

         write_data_chunk(png, frame, &sequence, z.data + offset, length);
         offset += length;
      }

      free(raw.data);
      free(z.data);
   }

   write_chunk(png, "IEND", NULL, 0);
}

typedef struct
{
   const png_byte *data;
   size_t size;
   size_t offset;
} memory_source;

typedef struct
{
   char message[200];       /* the libpng error, or the test's own */
   unsigned int frames;     /* frames read and verified */
} read_result;

static void
read_memory(png_structp png_ptr, png_bytep out, size_t length)
{
   memory_source *source = (memory_source *)png_get_io_ptr(png_ptr);

   if (length > source->size - source->offset)
      png_error(png_ptr, "read past the end of the test stream");

   memcpy(out, source->data + source->offset, length);
   source->offset += length;
}

static void
record_error(png_structp png_ptr, png_const_charp message)
{
   read_result *result = (read_result *)png_get_error_ptr(png_ptr);

   strncpy(result->message, message, sizeof result->message - 1);
   result->message[sizeof result->message - 1] = 0;
   png_longjmp(png_ptr, 1);
}

static void
ignore_warning(png_structp png_ptr, png_const_charp message)
{
   (void)png_ptr;
   (void)message;
}

/* Check the frame geometry and the row size, then every pixel. */
static int
check_frame(char *message, const image_format *format, unsigned int frame,
    png_uint_32 width, png_uint_32 height, size_t rowbytes,
    png_byte (*image)[MAX_ROWBYTES])
{
   const frame_geometry *g = &frame_list[frame];
   png_byte expect[MAX_ROWBYTES];
   png_uint_32 y;

   if (width != g->width || height != g->height)
   {
      sprintf(message, "frame %u: frame size %lux%lu, encoded %lux%lu",
          frame, (unsigned long)width, (unsigned long)height,
          (unsigned long)g->width, (unsigned long)g->height);
      return 0;
   }

   if (rowbytes != row_bytes(pixel_bits(format), width))
   {
      sprintf(message, "frame %u: png_get_rowbytes %lu, a %lu-pixel row is "
          "%lu bytes", frame, (unsigned long)rowbytes, (unsigned long)width,
          (unsigned long)row_bytes(pixel_bits(format), width));
      return 0;
   }

   if (image == NULL)
      return 1;

   for (y = 0; y < height; y++)
   {
      expected_row(expect, format, frame, width, y);
      if (memcmp(image[y], expect, rowbytes) != 0)
      {
         sprintf(message, "frame %u: row %lu differs from the encoded row",
             frame, (unsigned long)y);
         return 0;
      }
   }

   return 1;
}

#define UPDATE_INFO_EACH_FRAME 1

static png_byte frame_image[CANVAS_HEIGHT][MAX_ROWBYTES];

static int
read_sequential(const byte_buffer *png, const image_format *format,
    int interlace, int flags, read_result *result)
{
   png_structp png_ptr;
   png_infop info_ptr;
   memory_source source;

   memset(result, 0, sizeof *result);
   source.data = png->data;
   source.size = png->size;
   source.offset = 0;

   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, result,
       record_error, ignore_warning);
   if (png_ptr == NULL)
   {
      strcpy(result->message, "png_create_read_struct failed");
      return 0;
   }

   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      strcpy(result->message, "png_create_info_struct failed");
      png_destroy_read_struct(&png_ptr, NULL, NULL);
      return 0;
   }

   if (setjmp(png_jmpbuf(png_ptr)) != 0)
   {
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      return 0;
   }

   /* A damaged checksum is an error, not a warning, in every frame. */
   png_set_benign_errors(png_ptr, 0);
   png_set_read_fn(png_ptr, &source, read_memory);
   png_read_info(png_ptr, info_ptr);

   if (interlace != 0)
      (void)png_set_interlace_handling(png_ptr);

   png_read_update_info(png_ptr, info_ptr);

   if (png_get_valid(png_ptr, info_ptr, PNG_INFO_acTL) == 0 ||
       png_get_num_frames(png_ptr, info_ptr) != FRAME_COUNT)
      png_error(png_ptr, "acTL not read as encoded");

   for (result->frames = 0; result->frames < FRAME_COUNT; result->frames++)
   {
      png_bytep rows[CANVAS_HEIGHT];
      png_uint_32 width, height, x_offset, y_offset, y;
      png_uint_16 delay_num, delay_den;
      png_byte dispose_op, blend_op;
      char message[200];

      png_read_frame_head(png_ptr, info_ptr);

      if (result->frames > 0 && (flags & UPDATE_INFO_EACH_FRAME) != 0)
         png_read_update_info(png_ptr, info_ptr);

      if (png_get_next_frame_fcTL(png_ptr, info_ptr, &width, &height,
          &x_offset, &y_offset, &delay_num, &delay_den, &dispose_op,
          &blend_op) == 0)
         png_error(png_ptr, "no fcTL for the frame");

      if (check_frame(message, format, result->frames, width, height,
          png_get_rowbytes(png_ptr, info_ptr), NULL) == 0)
         png_error(png_ptr, message);

      for (y = 0; y < height; y++)
      {
         memset(frame_image[y], 0, sizeof frame_image[y]);
         rows[y] = frame_image[y];
      }

      png_read_image(png_ptr, rows);

      if (check_frame(message, format, result->frames, width, height,
          png_get_rowbytes(png_ptr, info_ptr), frame_image) == 0)
         png_error(png_ptr, message);
   }

   png_read_end(png_ptr, NULL);
   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
   return 1;
}

#ifdef PNG_PROGRESSIVE_READ_SUPPORTED
typedef struct
{
   read_result *result;
   const image_format *format;
   int interlace;
   png_infop info_ptr;
   png_uint_32 width;
   png_uint_32 height;
} progressive_state;

static void
progressive_frame_start(png_structp png_ptr, progressive_state *state,
    png_uint_32 width, png_uint_32 height)
{
   char message[200];

   state->width = width;
   state->height = height;
   memset(frame_image, 0, sizeof frame_image);

   if (check_frame(message, state->format, state->result->frames, width,
       height, png_get_rowbytes(png_ptr, state->info_ptr), NULL) == 0)
      png_error(png_ptr, message);
}

static void
progressive_info(png_structp png_ptr, png_infop info_ptr)
{
   progressive_state *state =
       (progressive_state *)png_get_progressive_ptr(png_ptr);

   if (state->interlace != 0)
      (void)png_set_interlace_handling(png_ptr);

   png_read_update_info(png_ptr, info_ptr);
   progressive_frame_start(png_ptr, state,
       png_get_image_width(png_ptr, info_ptr),
       png_get_image_height(png_ptr, info_ptr));
}

static void
progressive_frame_info(png_structp png_ptr, png_uint_32 frame_num)
{
   progressive_state *state =
       (progressive_state *)png_get_progressive_ptr(png_ptr);

   if (frame_num != state->result->frames)
      png_error(png_ptr, "frame_info_fn: unexpected frame number");

   progressive_frame_start(png_ptr, state,
       png_get_next_frame_width(png_ptr, state->info_ptr),
       png_get_next_frame_height(png_ptr, state->info_ptr));
}

static void
progressive_row(png_structp png_ptr, png_bytep new_row, png_uint_32 row_num,
    int pass)
{
   progressive_state *state =
       (progressive_state *)png_get_progressive_ptr(png_ptr);

   (void)pass;
   if (new_row != NULL && row_num < state->height)
      png_progressive_combine_row(png_ptr, frame_image[row_num], new_row);
}

static void
progressive_frame_end(png_structp png_ptr, png_uint_32 frame_num)
{
   progressive_state *state =
       (progressive_state *)png_get_progressive_ptr(png_ptr);
   char message[200];

   if (frame_num != state->result->frames)
      png_error(png_ptr, "frame_end_fn: unexpected frame number");

   if (check_frame(message, state->format, frame_num, state->width,
       state->height, png_get_rowbytes(png_ptr, state->info_ptr),
       frame_image) == 0)
      png_error(png_ptr, message);

   state->result->frames++;
}

static void
progressive_end(png_structp png_ptr, png_infop info_ptr)
{
   (void)png_ptr;
   (void)info_ptr;
}

static int
read_progressive(const byte_buffer *png, const image_format *format,
    int interlace, read_result *result)
{
   png_structp png_ptr;
   png_infop info_ptr;
   progressive_state state;
   size_t offset;

   memset(result, 0, sizeof *result);

   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, result,
       record_error, ignore_warning);
   if (png_ptr == NULL)
   {
      strcpy(result->message, "png_create_read_struct failed");
      return 0;
   }

   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      strcpy(result->message, "png_create_info_struct failed");
      png_destroy_read_struct(&png_ptr, NULL, NULL);
      return 0;
   }

   if (setjmp(png_jmpbuf(png_ptr)) != 0)
   {
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      return 0;
   }

   state.result = result;
   state.format = format;
   state.interlace = interlace;
   state.info_ptr = info_ptr;
   state.width = state.height = 0;

   png_set_benign_errors(png_ptr, 0);
   png_set_progressive_read_fn(png_ptr, &state, progressive_info,
       progressive_row, progressive_end);
   png_set_progressive_frame_fn(png_ptr, progressive_frame_info,
       progressive_frame_end);

   /* Small pieces, so chunk and row boundaries fall inside a buffer. */
   for (offset = 0; offset < png->size; offset += 7)
   {
      size_t length = png->size - offset < 7 ? png->size - offset : 7;

      png_process_data(png_ptr, info_ptr, png->data + offset, length);
   }

   if (result->frames != FRAME_COUNT)
      png_error(png_ptr, "not every frame was delivered");

   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
   return 1;
}
#endif /* PROGRESSIVE_READ */

static int failures;
static int checks;

static void
report(const image_format *format, int interlace, const chunk_layout *layout,
    const char *reader, const read_result *result, const char *problem)
{
   fprintf(stderr, "pngapngseq: FAIL color_type %d bit_depth %d interlace %d "
       "chunk_max %lu tail %lu corrupt_frame %d, %s: %s (frame %u: %s)\n",
       format->color_type, format->bit_depth, interlace,
       (unsigned long)layout->chunk_max, (unsigned long)layout->tail,
       layout->corrupt_frame, reader, problem, result->frames,
       result->message[0] != 0 ? result->message : "no error");
   failures++;
}

/* Returns 1, after reporting, if a valid stream was not read completely. */
static int
expect_success(const image_format *format, int interlace,
    const chunk_layout *layout, const char *reader, int ok,
    const read_result *result)
{
   checks++;
   if (ok == 0 || result->frames != FRAME_COUNT)
   {
      report(format, interlace, layout, reader, result,
          "a valid APNG was not read correctly");
      return 1;
   }

   return 0;
}

/* Each test reads every format, interlaced and not, and returns the number of
 * reads that went wrong.
 */
typedef int (*per_format_test)(const image_format *format, int interlace);

/* Control: one chunk per frame, no png_read_update_info after the first
 * frame.  This does not depend on either per-frame state.
 */
static int
valid_sequential_control(const image_format *format, int interlace)
{
   read_result result;
   byte_buffer png = { NULL, 0, 0 };
   int ok;

   build_apng(&png, format, interlace, &valid_layouts[0]);
   ok = read_sequential(&png, format, interlace, 0, &result);
   free(png.data);

   if (expect_success(format, interlace, &valid_layouts[0], "sequential", ok,
       &result) != 0)
      return 1;

   return 0;
}

#ifdef PNG_PROGRESSIVE_READ_SUPPORTED
/* Control: the progressive reader resets its state per frame already. */
static int
valid_progressive_control(const image_format *format, int interlace)
{
   int failed = 0;
   unsigned int i;

   for (i = 0; i < VALID_LAYOUT_COUNT; i++)
   {
      read_result result;
      byte_buffer png = { NULL, 0, 0 };
      int ok;

      build_apng(&png, format, interlace, &valid_layouts[i]);
      ok = read_progressive(&png, format, interlace, &result);
      if (expect_success(format, interlace, &valid_layouts[i], "progressive",
          ok, &result) != 0)
         failed++;
      free(png.data);
   }

   return failed;
}
#endif

/* png_read_update_info after png_read_frame_head, for every frame. */
static int
update_info_every_frame(const image_format *format, int interlace)
{
   int failed = 0;
   unsigned int i;

   for (i = 0; i < VALID_LAYOUT_COUNT; i++)
   {
      read_result result;
      byte_buffer png = { NULL, 0, 0 };
      int ok;

      build_apng(&png, format, interlace, &valid_layouts[i]);
      ok = read_sequential(&png, format, interlace, UPDATE_INFO_EACH_FRAME,
          &result);
      if (expect_success(format, interlace, &valid_layouts[i],
          "sequential, png_read_update_info for every frame", ok,
          &result) != 0)
         failed++;
      free(png.data);
   }

   return failed;
}

/* The end of each frame's stream in chunks of its own. */
static int
split_frame_trailer(const image_format *format, int interlace)
{
   int failed = 0;
   unsigned int i;

   for (i = 1; i < VALID_LAYOUT_COUNT; i++)
   {
      read_result result;
      byte_buffer png = { NULL, 0, 0 };
      int ok;

      build_apng(&png, format, interlace, &valid_layouts[i]);
      ok = read_sequential(&png, format, interlace, 0, &result);
      if (expect_success(format, interlace, &valid_layouts[i], "sequential",
          ok, &result) != 0)
         failed++;
      free(png.data);
   }

   return failed;
}

/* A damaged checksum in the chunk that ends a frame: the frame's rows are
 * complete before the checksum is read, which happens at the end of the
 * frame.  It must be reported while that frame is read, as for IDAT.
 */
static int
bad_checksum_reported_in_its_frame(const image_format *format, int interlace)
{
   int failed = 0;
   int frame;

   for (frame = 0; frame < FRAME_COUNT; frame++)
   {
      read_result result;
      chunk_layout layout;
      byte_buffer png = { NULL, 0, 0 };
      int ok;

      layout.chunk_max = 0;
      layout.tail = 4;
      layout.corrupt_frame = frame;
      build_apng(&png, format, interlace, &layout);

      ok = read_sequential(&png, format, interlace, 0, &result);
      checks++;
      if (ok != 0)
      {
         report(format, interlace, &layout, "sequential", &result,
             "a damaged Adler-32 was not reported");
         failed++;
      }

      else if (result.frames != (unsigned int)frame)
      {
         report(format, interlace, &layout, "sequential", &result,
             "a damaged Adler-32 was not reported in its own frame");
         failed++;
      }

      free(png.data);
   }

   return failed;
}

static int
run_test(int number, const char *name, per_format_test test)
{
   int failed = 0;
   unsigned int i;
   int interlace;

   for (i = 0; i < FORMAT_COUNT; i++)
      for (interlace = 0; interlace <= 1; interlace++)
         failed += test(&format_list[i], interlace);

   printf("%s %d /pngapngseq/%s\n", failed != 0 ? "not ok" : "ok", number,
       name);
   fflush(stdout);
   return failed != 0;
}

int
main(void)
{
   int result = 0;

   make_crc_table();
   printf("1..5\n");

   result |= run_test(1, "valid_sequential_control", valid_sequential_control);
#ifdef PNG_PROGRESSIVE_READ_SUPPORTED
   result |= run_test(2, "valid_progressive_control",
       valid_progressive_control);
#else
   printf("ok 2 /pngapngseq/valid_progressive_control # SKIP no progressive "
       "read\n");
#endif
   result |= run_test(3, "update_info_every_frame", update_info_every_frame);
   result |= run_test(4, "split_frame_trailer", split_frame_trailer);
   result |= run_test(5, "bad_checksum_reported_in_its_frame",
       bad_checksum_reported_in_its_frame);

   if (failures != 0)
   {
      fprintf(stderr, "pngapngseq: %d of %d checks failed\n", failures,
          checks);
      return 1;
   }

   printf("pngapngseq: %d checks passed\n", checks);
   return result;
}

#else /* !(READ_APNG, SEQUENTIAL_READ, READ_INTERLACING, BENIGN_ERRORS) */
int
main(void)
{
   fprintf(stderr, "pngapngseq: no support for sequential APNG read\n");
   return 77;
}
#endif
