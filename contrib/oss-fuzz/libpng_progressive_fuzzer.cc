/* Differential fuzz testing for the progressive (push) PNG reader.
 * SPDX-License-Identifier: libpng-2.0
 *
 * Compare decoded pixels when input is fed as a single block versus
 * one byte at a time. Valid PNGs must decode identically in both cases.
 */
#include <png.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#define MAX_DIM 256u
#define MAX_PIXELS (MAX_DIM * MAX_DIM)
#define MAX_DATA (1u << 20)

typedef struct {
    unsigned char *pixels;
    size_t rowbytes;
    png_uint_32 width, height;
    size_t row_callbacks;
    int header_seen, end_seen, diagnostic_seen;
} DecodeCtx;

static void on_error(png_structp p, png_const_charp msg) {
    (void)msg;
    DecodeCtx *c = (DecodeCtx*)png_get_error_ptr(p);
    if (c) c->diagnostic_seen = 1;
    png_longjmp(p, 1);
}
static void on_warning(png_structp p, png_const_charp msg) {
    (void)msg;
    /* Reaching IEND after a warning does not establish a valid image.
     * For example, bad Adler checksums can leave different partial rows.
     */
    DecodeCtx *c = (DecodeCtx*)png_get_error_ptr(p);
    if (c) c->diagnostic_seen = 1;
}
static void info_callback(png_structp p, png_infop info) {
    DecodeCtx *c = (DecodeCtx*)png_get_progressive_ptr(p);
    png_uint_32 w, h;
    int depth, color_type, interlace, compression, filter;
    if (!png_get_IHDR(p, info, &w, &h, &depth, &color_type, &interlace,
                      &compression, &filter)) png_error(p, "bad IHDR");
    if (w == 0 || h == 0 || w > MAX_DIM || h > MAX_DIM) png_error(p, "dimensions limited");
    png_set_interlace_handling(p);
    png_read_update_info(p, info);
    size_t stride = png_get_rowbytes(p, info);
    if (!stride || stride > (size_t)MAX_DIM * 8 ||
        stride > (size_t)(MAX_PIXELS * 8) / h)
        png_error(p, "allocation limited");
    c->pixels = (unsigned char*)calloc((size_t)h, stride);
    if (!c->pixels) png_error(p, "OOM");
    c->width = w; c->height = h; c->rowbytes = stride;
    c->header_seen = 1;
}
static void row_callback(png_structp p, png_bytep row, png_uint_32 rownum, int pass) {
    DecodeCtx *c = (DecodeCtx*)png_get_progressive_ptr(p);
    if (!c->header_seen || rownum >= c->height || !c->pixels ||
        pass < 0 || pass > 6) {
        fprintf(stderr, "PROGRESSIVE_CALLBACK_INVALID row=%u height=%u pass=%d\n",
                rownum, c->height, pass);
        abort();
    }
    if (row) png_progressive_combine_row(p, c->pixels + (size_t)rownum * c->rowbytes, row);
    c->row_callbacks++;
}
static void end_callback(png_structp p, png_infop info) {
    (void)info;
    DecodeCtx *c = (DecodeCtx*)png_get_progressive_ptr(p);
    c->end_seen = 1;
}
static void decode_progressive(const uint8_t *data, size_t size, size_t stride, DecodeCtx *c) {
    png_structp p = png_create_read_struct(PNG_LIBPNG_VER_STRING, c, on_error, on_warning);
    if (!p) return;
    png_infop info = png_create_info_struct(p);
    if (!info) { png_destroy_read_struct(&p, NULL, NULL); return; }
    if (setjmp(png_jmpbuf(p)) == 0) {
        png_set_user_limits(p, MAX_DIM, MAX_DIM);
        png_set_chunk_cache_max(p, 32);
        png_set_chunk_malloc_max(p, (png_alloc_size_t)(1u << 20));
        png_set_progressive_read_fn(p, c, info_callback, row_callback, end_callback);
        for (size_t i = 0; i < size && !c->end_seen; ) {
            size_t count = size-i;
            if (count > stride) count = stride;
            png_process_data(p, info, (png_bytep)(data+i), count);
            i += count;
        }
    }
    png_destroy_read_struct(&p, &info, NULL);
}
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 8 || size > MAX_DATA) return 0;
    DecodeCtx a = {}, b = {};
    decode_progressive(data, size, size, &a);
    decode_progressive(data, size, 1, &b);
    int complete_a = a.end_seen && !a.diagnostic_seen;
    int complete_b = b.end_seen && !b.diagnostic_seen;
    /* Completion must not depend on how data is split between calls. */
    if (complete_a != complete_b) {
        fprintf(stderr, "PROGRESSIVE_COMPLETION_MISMATCH bytes=%zu a=%d b=%d\n",
                size, complete_a, complete_b);
        abort();
    }
    /* Malformed inputs may error midway; compare pixels only on completion. */
    if (complete_a && complete_b) {
        if (a.width != b.width || a.height != b.height || a.rowbytes != b.rowbytes ||
            a.row_callbacks != b.row_callbacks ||
            !a.pixels || !b.pixels ||
            memcmp(a.pixels, b.pixels, a.rowbytes * a.height) != 0) {
            fprintf(stderr, "PROGRESSIVE_SPLIT_MISMATCH bytes=%zu rows_a=%zu rows_b=%zu\n",
                    size, a.row_callbacks, b.row_callbacks);
            abort();
        }
    }
    free(a.pixels);
    free(b.pixels);
    return 0;
}
