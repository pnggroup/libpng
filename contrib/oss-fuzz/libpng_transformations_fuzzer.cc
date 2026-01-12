#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <png.h>

/* Memory buffer structure for PNG reading */
struct png_mem_buffer {
    const uint8_t *data;
    size_t size;
    size_t pos;
};

/* Custom read function for memory buffer */
static void png_read_from_buffer(png_structp png_ptr, png_bytep data, png_size_t length) {
    struct png_mem_buffer *buf = (struct png_mem_buffer*)png_get_io_ptr(png_ptr);
    if (buf->pos + length > buf->size) {
        png_error(png_ptr, "Read error");
    }
    memcpy(data, buf->data + buf->pos, length);
    buf->pos += length;
}

/* Test PNG transformations to trigger uncovered code paths */
static void test_png_transformations(const uint8_t *data, size_t size) {
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) return;

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return;
    }

    struct png_mem_buffer buffer = {data, size, 0};
    png_set_read_fn(png_ptr, &buffer, png_read_from_buffer);

    png_read_info(png_ptr, info_ptr);

    int width = png_get_image_width(png_ptr, info_ptr);
    int height = png_get_image_height(png_ptr, info_ptr);
    int color_type = png_get_color_type(png_ptr, info_ptr);
    int bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    /* Sanity check dimensions */
    if (width <= 0 || height <= 0 || width > 2048 || height > 2048) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return;
    }

    /* Target 1: 16-bit depth processing (triggers png_do_chop) */
    if (bit_depth == 16) {
        png_set_scale_16(png_ptr);
    }

    /* Target 2: Color quantization (triggers png_do_quantize) */
    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
        png_colorp palette = (png_colorp)malloc(256 * sizeof(png_color));
        if (palette) {
            int i;
            for (i = 0; i < 256; i++) {
                palette[i].red = palette[i].green = palette[i].blue = (png_byte)i;
            }
            png_set_quantize(png_ptr, palette, 256, 256, NULL, 0);
            free(palette);
        }
    }

    /* Target 3: Alpha channel transformations */
    if (color_type & PNG_COLOR_MASK_ALPHA) {
        png_set_invert_alpha(png_ptr);  /* png_do_read_invert_alpha */
        png_set_swap_alpha(png_ptr);    /* png_do_read_swap_alpha */
    }

    /* Target 4: Bit shifting (triggers png_do_unshift) */
    if (bit_depth < 8) {
        png_set_packing(png_ptr);
    }

    /* Target 5: Alpha encoding (triggers png_do_encode_alpha) */
    png_set_alpha_mode(png_ptr, PNG_ALPHA_STANDARD, PNG_GAMMA_LINEAR);

    /* Update transformations */
    png_read_update_info(png_ptr, info_ptr);

    /* Read image data to execute transformations */
    size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    png_bytep row = (png_bytep)malloc(rowbytes);
    if (row) {
        int y;
        for (y = 0; y < height && y < 100; y++) { /* Limit rows for performance */
            png_read_row(png_ptr, row, NULL);
        }
        free(row);
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
}

/* Test png_image API and colormap functions */
static void test_png_image_colormap(const uint8_t *data, size_t size) {
    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = PNG_IMAGE_VERSION;

    if (!png_image_begin_read_from_memory(&image, data, size)) {
        return;
    }

    /* Limit image size for performance */
    if (image.width > 1024 || image.height > 1024) {
        png_image_free(&image);
        return;
    }

    /* Test different colormap formats to trigger uncovered functions */
    png_uint_32 formats[] = {
        PNG_FORMAT_GRAY,
        PNG_FORMAT_GA,
        PNG_FORMAT_RGB,
        PNG_FORMAT_RGBA,
        PNG_FORMAT_RGB_COLORMAP,
        PNG_FORMAT_RGBA_COLORMAP
    };

    int i;
    for (i = 0; i < 6; i++) {
        image.format = formats[i];

        size_t buffer_size = PNG_IMAGE_SIZE(image);
        if (buffer_size > 0 && buffer_size < 5*1024*1024) { /* Limit to 5MB */
            void *img_buffer = malloc(buffer_size);
            if (img_buffer) {
                if (image.format & PNG_FORMAT_FLAG_COLORMAP) {
                    png_bytep colormap = (png_bytep)malloc(PNG_IMAGE_COLORMAP_SIZE(image));
                    if (colormap) {
                        /* This triggers png_image_finish_read and colormap generation */
                        png_image_finish_read(&image, NULL, img_buffer, 0, colormap);
                        free(colormap);
                    }
                } else {
                    png_image_finish_read(&image, NULL, img_buffer, 0, NULL);
                }
                free(img_buffer);
            }
        }

        /* Reset for next format */
        if (i < 5) {
            png_image_free(&image);
            memset(&image, 0, sizeof(image));
            image.version = PNG_IMAGE_VERSION;
            if (!png_image_begin_read_from_memory(&image, data, size)) {
                break;
            }
        }
    }

    png_image_free(&image);
}


/* Test png_read_png API */
static void test_png_read_png_api(const uint8_t *data, size_t size) {
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) return;

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return;
    }

    struct png_mem_buffer buffer = {data, size, 0};
    png_set_read_fn(png_ptr, &buffer, png_read_from_buffer);

    /* Set up transformations before reading */
    png_set_scale_16(png_ptr);
    png_set_packing(png_ptr);
    png_set_expand(png_ptr);

    /* Use png_read_png which should trigger OSS_FUZZ_png_read_png path */
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 8) return 0;

    /* Test 1: Standard PNG reading with transformations */
    test_png_transformations(data, size);

    /* Test 2: PNG image API with colormap processing */
    test_png_image_colormap(data, size);

    /* Test 3: png_read_png API */
    test_png_read_png_api(data, size);

    return 0;
}
