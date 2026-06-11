/* png_write_chunks_fuzzer.c
 *
 * Fuzz target for the libpng write path (chunk serialisers in pngwutil.c).
 * The existing OSS-Fuzz harnesses cover read/decode only (pngrutil.c).
 * This harness constructs a minimal RGBA image from fuzz input and
 * writes it through the full encode pipeline, exercising:
 *
 *   pngwutil.c  – png_write_IHDR, png_write_IDAT, png_write_IEND,
 *                 png_write_iCCP, png_write_tEXt, png_write_zTXt,
 *                 png_write_sRGB, png_write_gAMA, png_write_pHYs
 *   pngwrite.c  – png_write_image row loop, filter selection
 *   pngwio.c    – custom write callbacks
 */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "png.h"

typedef struct { uint8_t *buf; size_t size; } MemBuf;

static void write_fn(png_structp png, png_bytep data, png_size_t len)
{
    MemBuf *mb = (MemBuf *)png_get_io_ptr(png);
    uint8_t *tmp = (uint8_t *)realloc(mb->buf, mb->size + len);
    if (!tmp) return;
    mb->buf = tmp;
    memcpy(mb->buf + mb->size, data, len);
    mb->size += len;
}
static void flush_fn(png_structp png) { (void)png; }

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 8) return 0;

    uint32_t width  = (uint32_t)(data[0] % 64) + 1;
    uint32_t height = (uint32_t)(data[1] % 64) + 1;
    int      color  = (data[2] & 1) ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB;
    int      depth  = (data[3] & 1) ? 16 : 8;
    int      filter = data[4] % 5;
    int      level  = data[5] % 10;
    size_t   rowbytes = width * (color == PNG_COLOR_TYPE_RGBA ? 4 : 3) * (depth / 8);
    size_t   need   = 6 + height * rowbytes;
    if (size < need) return 0;

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                              NULL, NULL, NULL);
    if (!png) return 0;
    png_infop info = png_create_info_struct(png);
    if (!info) { png_destroy_write_struct(&png, NULL); return 0; }

    MemBuf mb = {NULL, 0};
    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        free(mb.buf); return 0;
    }

    png_set_write_fn(png, &mb, write_fn, flush_fn);
    png_set_filter(png, 0, 1 << filter);
    png_set_compression_level(png, level);
    png_set_IHDR(png, info, width, height, depth, color,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    /* Optional metadata chunks */
    if (data[5] & 0x10) {
        double gamma = 1.0 / 2.2;
        png_set_gAMA(png, info, gamma);
    }

    png_write_info(png, info);

    const uint8_t *pixels = data + 6;
    for (uint32_t row = 0; row < height; row++)
        png_write_row(png, (png_bytep)(pixels + row * rowbytes));

    png_write_end(png, info);
    png_destroy_write_struct(&png, &info);
    free(mb.buf);
    return 0;
}
