struct png_struct_def
{
#ifdef PNG_SETJMP_SUPPORTED
   jmp_buf jmpbuf;            /* used in png_error */
#endif
   png_error_ptr error_fn;    /* function for printing errors and aborting */
   png_error_ptr warning_fn;  /* function for printing warnings */
   png_voidp error_ptr;       /* user supplied struct for error functions */
   png_rw_ptr write_data_fn;  /* function for writing output data */
   png_rw_ptr read_data_fn;   /* function for reading input data */
   png_voidp io_ptr;          /* ptr to application struct for I/O functions*/

#if defined(PNG_READ_USER_TRANSFORM_SUPPORTED)
   png_user_transform_ptr read_user_transform_fn; /* user read transform */
#endif

#if defined(PNG_WRITE_USER_TRANSFORM_SUPPORTED)
   png_user_transform_ptr write_user_transform_fn; /* user write transform */
#endif

#if defined(PNG_READ_USER_TRANSFORM_SUPPORTED) || \
    defined(PNG_WRITE_USER_TRANSFORM_SUPPORTED)
   png_voidp user_transform_ptr; /* user supplied struct for user transform */
   png_byte user_transform_depth;    /* bit depth of user transformed pixels */
   png_byte user_transform_channels; /* channels in user transformed pixels */
#endif

#if defined(PNG_READ_USER_CHUNKS_SUPPORTED)
   png_voidp user_chunk_ptr;
   png_user_chunk_ptr read_user_chunk_fn; /* user read chunk handler */
#endif

   png_uint_32 mode;          /* tells us where we are in the PNG file */
   png_uint_32 flags;         /* flags indicating various things to libpng */
   png_uint_32 transformations; /* which transformations to perform */

   z_stream zstream;          /* pointer to decompression structure (below) */
   png_bytep zbuf;            /* buffer for zlib */
   png_size_t zbuf_size;      /* size of zbuf */
   int zlib_level;            /* holds zlib compression level */
   int zlib_method;           /* holds zlib compression method */
   int zlib_window_bits;      /* holds zlib compression window bits */
   int zlib_mem_level;        /* holds zlib compression memory level */
   int zlib_strategy;         /* holds zlib compression strategy */

   png_uint_32 width;         /* width of image in pixels */
   png_uint_32 height;        /* height of image in pixels */
   png_uint_32 num_rows;      /* number of rows in current pass */
   png_uint_32 usr_width;     /* width of row at start of write */
   png_uint_32 rowbytes;      /* size of row in bytes */
   png_uint_32 irowbytes;     /* size of current interlaced row in bytes */
   png_uint_32 iwidth;        /* width of current interlaced row in pixels */
   png_uint_32 row_number;    /* current row in interlace pass */
   png_bytep prev_row;        /* buffer to save previous (unfiltered) row */
   png_bytep row_buf;         /* buffer to save current (unfiltered) row */
   png_bytep sub_row;         /* buffer to save "sub" row when filtering */
   png_bytep up_row;          /* buffer to save "up" row when filtering */
   png_bytep avg_row;         /* buffer to save "avg" row when filtering */
   png_bytep paeth_row;       /* buffer to save "Paeth" row when filtering */
   png_row_info row_info;     /* used for transformation routines */

   png_uint_32 idat_size;     /* current IDAT size for read */
   png_uint_32 crc;           /* current chunk CRC value */
   png_colorp palette;        /* palette from the input file */
   png_uint_16 num_palette;   /* number of color entries in palette */
   png_uint_16 num_trans;     /* number of transparency values */
   png_byte chunk_name[5];    /* null-terminated name of current chunk */
   png_byte compression;      /* file compression type (always 0) */
   png_byte filter;           /* file filter type (always 0) */
   png_byte interlaced;       /* PNG_INTERLACE_NONE, PNG_INTERLACE_ADAM7 */
   png_byte pass;             /* current interlace pass (0 - 6) */
   png_byte do_filter;        /* row filter flags (see PNG_FILTER_ below ) */
   png_byte color_type;       /* color type of file */
   png_byte bit_depth;        /* bit depth of file */
   png_byte usr_bit_depth;    /* bit depth of users row */
   png_byte pixel_depth;      /* number of bits per pixel */
   png_byte channels;         /* number of channels in file */
   png_byte usr_channels;     /* channels at start of write */
   png_byte sig_bytes;        /* magic bytes read/written from start of file */

#if defined(PNG_READ_FILLER_SUPPORTED) || defined(PNG_WRITE_FILLER_SUPPORTED)
   png_uint_16 filler;           /* filler bytes for pixel expansion */
#endif

#if defined(PNG_READ_bKGD_SUPPORTED)
   png_byte background_gamma_type;
#ifdef PNG_FLOATING_POINT_SUPPORTED
   float background_gamma;
#endif
   png_color_16 background;   /* background color in screen gamma space */
#  if defined(PNG_READ_GAMMA_SUPPORTED)
     png_color_16 background_1; /* background normalized to gamma 1.0 */
#  endif /* PNG_READ_GAMMA && PNG_READ_bKGD_SUPPORTED */
#endif /* PNG_READ_bKGD_SUPPORTED */

#if defined(PNG_WRITE_FLUSH_SUPPORTED)
   png_flush_ptr output_flush_fn;/* Function for flushing output */
   png_uint_32 flush_dist;    /* how many rows apart to flush, 0 - no flush */
   png_uint_32 flush_rows;    /* number of rows written since last flush */
#endif

#if defined(PNG_READ_GAMMA_SUPPORTED) || defined(PNG_READ_BACKGROUND_SUPPORTED)
   int gamma_shift;      /* number of "insignificant" bits 16-bit gamma */
#ifdef PNG_FLOATING_POINT_SUPPORTED
   float gamma;          /* file gamma value */
   float screen_gamma;   /* screen gamma value (display_exponent) */
#endif
#endif

#if defined(PNG_READ_GAMMA_SUPPORTED) || defined(PNG_READ_BACKGROUND_SUPPORTED)
   png_bytep gamma_table;     /* gamma table for 8-bit depth files */
   png_bytep gamma_from_1;    /* converts from 1.0 to screen */
   png_bytep gamma_to_1;      /* converts from file to 1.0 */
   png_uint_16pp gamma_16_table; /* gamma table for 16-bit depth files */
   png_uint_16pp gamma_16_from_1; /* converts from 1.0 to screen */
   png_uint_16pp gamma_16_to_1; /* converts from file to 1.0 */
#endif

#if defined(PNG_READ_GAMMA_SUPPORTED) || defined (PNG_READ_sBIT_SUPPORTED)
   png_color_8 sig_bit;       /* significant bits in each available channel */
#endif

#if defined(PNG_READ_SHIFT_SUPPORTED) || defined(PNG_WRITE_SHIFT_SUPPORTED)
   png_color_8 shift;         /* shift for significant bit tranformation */
#endif

#if defined(PNG_READ_tRNS_SUPPORTED) || defined(PNG_READ_BACKGROUND_SUPPORTED) \
 || defined(PNG_READ_EXPAND_SUPPORTED) || defined(PNG_READ_BACKGROUND_SUPPORTED)
   png_bytep trans;           /* transparency values for paletted files */
   png_color_16 trans_values; /* transparency values for non-paletted files */
#endif

   png_read_status_ptr read_row_fn;   /* called after each row is decoded */
   png_write_status_ptr write_row_fn; /* called after each row is encoded */
#ifdef PNG_PROGRESSIVE_READ_SUPPORTED
   png_progressive_info_ptr info_fn; /* called after header data fully read */
   png_progressive_row_ptr row_fn;   /* called after each prog. row is decoded */
   png_progressive_end_ptr end_fn;   /* called after image is complete */
   png_bytep save_buffer_ptr;        /* current location in save_buffer */
   png_bytep save_buffer;            /* buffer for previously read data */
   png_bytep current_buffer_ptr;     /* current location in current_buffer */
   png_bytep current_buffer;         /* buffer for recently used data */
   png_uint_32 push_length;          /* size of current input chunk */
   png_uint_32 skip_length;          /* bytes to skip in input data */
   png_size_t save_buffer_size;      /* amount of data now in save_buffer */
   png_size_t save_buffer_max;       /* total size of save_buffer */
   png_size_t buffer_size;           /* total amount of available input data */
   png_size_t current_buffer_size;   /* amount of data now in current_buffer */
   int process_mode;                 /* what push library is currently doing */
   int cur_palette;                  /* current push library palette index */

#  if defined(PNG_READ_TEXT_SUPPORTED)
     png_size_t current_text_size;   /* current size of text input data */
     png_size_t current_text_left;   /* how much text left to read in input */
     png_charp current_text;         /* current text chunk buffer */
     png_charp current_text_ptr;     /* current location in current_text */
#  endif /* PNG_PROGRESSIVE_READ_SUPPORTED && PNG_READ_TEXT_SUPPORTED */

#endif /* PNG_PROGRESSIVE_READ_SUPPORTED */

#if defined(__TURBOC__) && !defined(_Windows) && !defined(__FLAT__)
/* for the Borland special 64K segment handler */
   png_bytepp offset_table_ptr;
   png_bytep offset_table;
   png_uint_16 offset_table_number;
   png_uint_16 offset_table_count;
   png_uint_16 offset_table_count_free;
#endif

#if defined(PNG_READ_DITHER_SUPPORTED)
   png_bytep palette_lookup;         /* lookup table for dithering */
   png_bytep dither_index;           /* index translation for palette files */
#endif

#if defined(PNG_READ_DITHER_SUPPORTED) || defined(PNG_READ_hIST_SUPPORTED)
   png_uint_16p hist;                /* histogram */
#endif

#if defined(PNG_WRITE_WEIGHTED_FILTER_SUPPORTED)
   png_byte heuristic_method;        /* heuristic for row filter selection */
   png_byte num_prev_filters;        /* number of weights for previous rows */
   png_bytep prev_filters;           /* filter type(s) of previous row(s) */
   png_uint_16p filter_weights;      /* weight(s) for previous line(s) */
   png_uint_16p inv_filter_weights;  /* 1/weight(s) for previous line(s) */
   png_uint_16p filter_costs;        /* relative filter calculation cost */
   png_uint_16p inv_filter_costs;    /* 1/relative filter calculation cost */
#endif

#if defined(PNG_TIME_RFC1123_SUPPORTED)
   png_charp time_buffer;            /* String to hold RFC 1123 time text */
#endif

#ifdef PNG_USER_MEM_SUPPORTED
   png_voidp mem_ptr;                /* user supplied struct for mem functions */
   png_malloc_ptr malloc_fn;         /* function for allocating memory */
   png_free_ptr free_fn;             /* function for freeing memory */
#endif

#if defined(PNG_READ_RGB_TO_GRAY_SUPPORTED)
   png_byte rgb_to_gray_status;
   png_uint_16 rgb_to_gray_red_coeff;
   png_uint_16 rgb_to_gray_green_coeff;
   png_uint_16 rgb_to_gray_blue_coeff;
#endif

#if defined(PNG_READ_EMPTY_PLTE_SUPPORTED) || \
    defined(PNG_WRITE_EMPTY_PLTE_SUPPORTED)
   png_byte empty_plte_permitted;
#endif

#if defined(PNG_UNKNOWN_CHUNKS_SUPPORTED)
   int num_chunk_list;
   png_bytep chunk_list;
#endif

#if defined(PNG_READ_GAMMA_SUPPORTED) || defined(PNG_READ_BACKGROUND_SUPPORTED)
   png_fixed_point int_gamma;
#endif

   png_uint_32 free_me;       /* flags items libpng is responsible for freeing */
};
