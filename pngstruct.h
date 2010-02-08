/* The structure that holds the information to read and write PNG files.
 * The only people who need to care about what is inside of this are the
 * people who will be modifying the library for their own special needs.
 * It should NOT be accessed directly by an application, except to store
 * the jmp_buf.
 */

struct png_struct_def
{
#ifdef PNG_SETJMP_SUPPORTED
   jmp_buf jmpbuf PNG_DEPSTRUCT;            /* used in png_error */
   png_longjmp_ptr longjmp_fn PNG_DEPSTRUCT;/* setjmp non-local goto
                                               function. */
#endif
   png_error_ptr error_fn PNG_DEPSTRUCT;    /* function for printing
                                               errors and aborting */
   png_error_ptr warning_fn PNG_DEPSTRUCT;  /* function for printing
                                               warnings */
   png_voidp error_ptr PNG_DEPSTRUCT;       /* user supplied struct for
                                               error functions */
   png_rw_ptr write_data_fn PNG_DEPSTRUCT;  /* function for writing
                                               output data */
   png_rw_ptr read_data_fn PNG_DEPSTRUCT;   /* function for reading
                                               input data */
   png_voidp io_ptr PNG_DEPSTRUCT;          /* ptr to application struct
                                               for I/O functions */

#ifdef PNG_READ_USER_TRANSFORM_SUPPORTED
   png_user_transform_ptr read_user_transform_fn PNG_DEPSTRUCT; /* user read
                                                                 transform */
#endif

#ifdef PNG_WRITE_USER_TRANSFORM_SUPPORTED
   png_user_transform_ptr write_user_transform_fn PNG_DEPSTRUCT; /* user write
                                                                  transform */
#endif

/* These were added in libpng-1.0.2 */
#ifdef PNG_USER_TRANSFORM_PTR_SUPPORTED
#if defined(PNG_READ_USER_TRANSFORM_SUPPORTED) || \
    defined(PNG_WRITE_USER_TRANSFORM_SUPPORTED)
   png_voidp user_transform_ptr PNG_DEPSTRUCT; /* user supplied struct
                                                  for user transform */
   png_byte user_transform_depth PNG_DEPSTRUCT;    /* bit depth of user
                                                      transformed pixels */
   png_byte user_transform_channels PNG_DEPSTRUCT; /* channels in user
                                                      transformed pixels */
#endif
#endif

   png_uint_32 mode PNG_DEPSTRUCT;          /* tells us where we are in
                                               the PNG file */
   png_uint_32 flags PNG_DEPSTRUCT;         /* flags indicating various
                                               things to libpng */
   png_uint_32 transformations PNG_DEPSTRUCT; /* which transformations
                                                 to perform */

   z_stream zstream PNG_DEPSTRUCT;          /* pointer to decompression
                                               structure (below) */
   png_bytep zbuf PNG_DEPSTRUCT;            /* buffer for zlib */
   png_size_t zbuf_size PNG_DEPSTRUCT;      /* size of zbuf */
   int zlib_level PNG_DEPSTRUCT;            /* holds zlib compression level */
   int zlib_method PNG_DEPSTRUCT;           /* holds zlib compression method */
   int zlib_window_bits PNG_DEPSTRUCT;      /* holds zlib compression window
                                               bits */
   int zlib_mem_level PNG_DEPSTRUCT;        /* holds zlib compression memory
                                               level */
   int zlib_strategy PNG_DEPSTRUCT;         /* holds zlib compression
                                               strategy */

   png_uint_32 width PNG_DEPSTRUCT;         /* width of image in pixels */
   png_uint_32 height PNG_DEPSTRUCT;        /* height of image in pixels */
   png_uint_32 num_rows PNG_DEPSTRUCT;      /* number of rows in current pass */
   png_uint_32 usr_width PNG_DEPSTRUCT;     /* width of row at start of write */
   png_size_t rowbytes PNG_DEPSTRUCT;       /* size of row in bytes */
   png_uint_32 iwidth PNG_DEPSTRUCT;        /* width of current interlaced
                                               row in pixels */
   png_uint_32 row_number PNG_DEPSTRUCT;    /* current row in interlace pass */
   png_bytep prev_row PNG_DEPSTRUCT;        /* buffer to save previous
                                               (unfiltered) row */
   png_bytep row_buf PNG_DEPSTRUCT;         /* buffer to save current
                                               (unfiltered) row */
   png_bytep sub_row PNG_DEPSTRUCT;         /* buffer to save "sub" row
                                               when filtering */
   png_bytep up_row PNG_DEPSTRUCT;          /* buffer to save "up" row
                                               when filtering */
   png_bytep avg_row PNG_DEPSTRUCT;         /* buffer to save "avg" row
                                               when filtering */
   png_bytep paeth_row PNG_DEPSTRUCT;       /* buffer to save "Paeth" row
                                               when filtering */
   png_row_info row_info PNG_DEPSTRUCT;     /* used for transformation
                                               routines */

   png_uint_32 idat_size PNG_DEPSTRUCT;     /* current IDAT size for read */
   png_uint_32 crc PNG_DEPSTRUCT;           /* current chunk CRC value */
   png_colorp palette PNG_DEPSTRUCT;        /* palette from the input file */
   png_uint_16 num_palette PNG_DEPSTRUCT;   /* number of color entries in
                                               palette */
   png_uint_16 num_trans PNG_DEPSTRUCT;     /* number of transparency values */
   png_byte chunk_name[5] PNG_DEPSTRUCT;    /* null-terminated name of current
                                               chunk */
   png_byte compression PNG_DEPSTRUCT;      /* file compression type
                                               (always 0) */
   png_byte filter PNG_DEPSTRUCT;           /* file filter type (always 0) */
   png_byte interlaced PNG_DEPSTRUCT;       /* PNG_INTERLACE_NONE,
                                               PNG_INTERLACE_ADAM7 */
   png_byte pass PNG_DEPSTRUCT;             /* current interlace pass (0 - 6) */
   png_byte do_filter PNG_DEPSTRUCT;        /* row filter flags (see
                                               PNG_FILTER_ below ) */
   png_byte color_type PNG_DEPSTRUCT;       /* color type of file */
   png_byte bit_depth PNG_DEPSTRUCT;        /* bit depth of file */
   png_byte usr_bit_depth PNG_DEPSTRUCT;    /* bit depth of users row */
   png_byte pixel_depth PNG_DEPSTRUCT;      /* number of bits per pixel */
   png_byte channels PNG_DEPSTRUCT;         /* number of channels in file */
   png_byte usr_channels PNG_DEPSTRUCT;     /* channels at start of write */
   png_byte sig_bytes PNG_DEPSTRUCT;        /* magic bytes read/written from
                                               start of file */

#if defined(PNG_READ_FILLER_SUPPORTED) || defined(PNG_WRITE_FILLER_SUPPORTED)
   png_uint_16 filler PNG_DEPSTRUCT;           /* filler bytes for pixel
                                                  expansion */
#endif

#ifdef PNG_bKGD_SUPPORTED
   png_byte background_gamma_type PNG_DEPSTRUCT;
#  ifdef PNG_FLOATING_POINT_SUPPORTED
   float background_gamma PNG_DEPSTRUCT;
#  endif
   png_color_16 background PNG_DEPSTRUCT;   /* background color in
                                               screen gamma space */
#ifdef PNG_READ_GAMMA_SUPPORTED
   png_color_16 background_1 PNG_DEPSTRUCT; /* background normalized
                                               to gamma 1.0 */
#endif
#endif /* PNG_bKGD_SUPPORTED */

#ifdef PNG_WRITE_FLUSH_SUPPORTED
   png_flush_ptr output_flush_fn PNG_DEPSTRUCT; /* Function for flushing
                                               output */
   png_uint_32 flush_dist PNG_DEPSTRUCT;    /* how many rows apart to flush,
                                               0 - no flush */
   png_uint_32 flush_rows PNG_DEPSTRUCT;    /* number of rows written since
                                               last flush */
#endif

#if defined(PNG_READ_GAMMA_SUPPORTED) || defined(PNG_READ_BACKGROUND_SUPPORTED)
   int gamma_shift PNG_DEPSTRUCT;      /* number of "insignificant" bits
                                          16-bit gamma */
#ifdef PNG_FLOATING_POINT_SUPPORTED
   float gamma PNG_DEPSTRUCT;          /* file gamma value */
   float screen_gamma PNG_DEPSTRUCT;   /* screen gamma value
                                          (display_exponent) */
#endif
#endif

#if defined(PNG_READ_GAMMA_SUPPORTED) || defined(PNG_READ_BACKGROUND_SUPPORTED)
   png_bytep gamma_table PNG_DEPSTRUCT;     /* gamma table for 8-bit
                                               depth files */
   png_bytep gamma_from_1 PNG_DEPSTRUCT;    /* converts from 1.0 to screen */
   png_bytep gamma_to_1 PNG_DEPSTRUCT;      /* converts from file to 1.0 */
   png_uint_16pp gamma_16_table PNG_DEPSTRUCT; /* gamma table for 16-bit
                                                  depth files */
   png_uint_16pp gamma_16_from_1 PNG_DEPSTRUCT; /* converts from 1.0 to
                                                   screen */
   png_uint_16pp gamma_16_to_1 PNG_DEPSTRUCT; /* converts from file to 1.0 */
#endif

#if defined(PNG_READ_GAMMA_SUPPORTED) || defined(PNG_sBIT_SUPPORTED)
   png_color_8 sig_bit PNG_DEPSTRUCT;       /* significant bits in each
                                               available channel */
#endif

#if defined(PNG_READ_SHIFT_SUPPORTED) || defined(PNG_WRITE_SHIFT_SUPPORTED)
   png_color_8 shift PNG_DEPSTRUCT;         /* shift for significant bit
                                               tranformation */
#endif

#if defined(PNG_tRNS_SUPPORTED) || defined(PNG_READ_BACKGROUND_SUPPORTED) \
 || defined(PNG_READ_EXPAND_SUPPORTED) || defined(PNG_READ_BACKGROUND_SUPPORTED)
   png_bytep trans_alpha PNG_DEPSTRUCT;           /* alpha values for
                                                     paletted files */
   png_color_16 trans_color PNG_DEPSTRUCT;  /* transparent color for
                                               non-paletted files */
#endif

   png_read_status_ptr read_row_fn PNG_DEPSTRUCT;   /* called after each
                                                       row is decoded */
   png_write_status_ptr write_row_fn PNG_DEPSTRUCT; /* called after each
                                                       row is encoded */
#ifdef PNG_PROGRESSIVE_READ_SUPPORTED
   png_progressive_info_ptr info_fn PNG_DEPSTRUCT; /* called after header
                                                      data fully read */
   png_progressive_row_ptr row_fn PNG_DEPSTRUCT;   /* called after each
                                                      prog. row is decoded */
   png_progressive_end_ptr end_fn PNG_DEPSTRUCT;   /* called after image
                                                      is complete */
   png_bytep save_buffer_ptr PNG_DEPSTRUCT;        /* current location in
                                                      save_buffer */
   png_bytep save_buffer PNG_DEPSTRUCT;            /* buffer for previously
                                                      read data */
   png_bytep current_buffer_ptr PNG_DEPSTRUCT;     /* current location in
                                                      current_buffer */
   png_bytep current_buffer PNG_DEPSTRUCT;         /* buffer for recently
                                                      used data */
   png_uint_32 push_length PNG_DEPSTRUCT;          /* size of current input
                                                      chunk */
   png_uint_32 skip_length PNG_DEPSTRUCT;          /* bytes to skip in
                                                      input data */
   png_size_t save_buffer_size PNG_DEPSTRUCT;      /* amount of data now
                                                      in save_buffer */
   png_size_t save_buffer_max PNG_DEPSTRUCT;       /* total size of
                                                      save_buffer */
   png_size_t buffer_size PNG_DEPSTRUCT;           /* total amount of
                                                      available input data */
   png_size_t current_buffer_size PNG_DEPSTRUCT;   /* amount of data now
                                                      in current_buffer */
   int process_mode PNG_DEPSTRUCT;                 /* what push library
                                                      is currently doing */
   int cur_palette PNG_DEPSTRUCT;                  /* current push library
                                                      palette index */

#  ifdef PNG_TEXT_SUPPORTED
     png_size_t current_text_size PNG_DEPSTRUCT;   /* current size of
                                                      text input data */
     png_size_t current_text_left PNG_DEPSTRUCT;   /* how much text left
                                                      to read in input */
     png_charp current_text PNG_DEPSTRUCT;         /* current text chunk
                                                      buffer */
     png_charp current_text_ptr PNG_DEPSTRUCT;     /* current location
                                                      in current_text */
#  endif /* PNG_PROGRESSIVE_READ_SUPPORTED && PNG_TEXT_SUPPORTED */

#endif /* PNG_PROGRESSIVE_READ_SUPPORTED */

#if defined(__TURBOC__) && !defined(_Windows) && !defined(__FLAT__)
/* For the Borland special 64K segment handler */
   png_bytepp offset_table_ptr PNG_DEPSTRUCT;
   png_bytep offset_table PNG_DEPSTRUCT;
   png_uint_16 offset_table_number PNG_DEPSTRUCT;
   png_uint_16 offset_table_count PNG_DEPSTRUCT;
   png_uint_16 offset_table_count_free PNG_DEPSTRUCT;
#endif

#ifdef PNG_READ_DITHER_SUPPORTED
   png_bytep palette_lookup PNG_DEPSTRUCT; /* lookup table for dithering */
   png_bytep dither_index PNG_DEPSTRUCT;   /* index translation for palette
                                              files */
#endif

#if defined(PNG_READ_DITHER_SUPPORTED) || defined(PNG_hIST_SUPPORTED)
   png_uint_16p hist PNG_DEPSTRUCT;                /* histogram */
#endif

#ifdef PNG_WRITE_WEIGHTED_FILTER_SUPPORTED
   png_byte heuristic_method PNG_DEPSTRUCT;        /* heuristic for row
                                                      filter selection */
   png_byte num_prev_filters PNG_DEPSTRUCT;        /* number of weights
                                                      for previous rows */
   png_bytep prev_filters PNG_DEPSTRUCT;           /* filter type(s) of
                                                      previous row(s) */
   png_uint_16p filter_weights PNG_DEPSTRUCT;      /* weight(s) for previous
                                                      line(s) */
   png_uint_16p inv_filter_weights PNG_DEPSTRUCT;  /* 1/weight(s) for
                                                      previous line(s) */
   png_uint_16p filter_costs PNG_DEPSTRUCT;        /* relative filter
                                                      calculation cost */
   png_uint_16p inv_filter_costs PNG_DEPSTRUCT;    /* 1/relative filter
                                                      calculation cost */
#endif

#ifdef PNG_TIME_RFC1123_SUPPORTED
   png_charp time_buffer PNG_DEPSTRUCT; /* String to hold RFC 1123 time text */
#endif

/* New members added in libpng-1.0.6 */

   png_uint_32 free_me PNG_DEPSTRUCT;    /* flags items libpng is
                                            responsible for freeing */

#ifdef PNG_USER_CHUNKS_SUPPORTED
   png_voidp user_chunk_ptr PNG_DEPSTRUCT;
   png_user_chunk_ptr read_user_chunk_fn PNG_DEPSTRUCT; /* user read
                                                           chunk handler */
#endif

#ifdef PNG_HANDLE_AS_UNKNOWN_SUPPORTED
   int num_chunk_list PNG_DEPSTRUCT;
   png_bytep chunk_list PNG_DEPSTRUCT;
#endif

/* New members added in libpng-1.0.3 */
#ifdef PNG_READ_RGB_TO_GRAY_SUPPORTED
   png_byte rgb_to_gray_status PNG_DEPSTRUCT;
   /* These were changed from png_byte in libpng-1.0.6 */
   png_uint_16 rgb_to_gray_red_coeff PNG_DEPSTRUCT;
   png_uint_16 rgb_to_gray_green_coeff PNG_DEPSTRUCT;
   png_uint_16 rgb_to_gray_blue_coeff PNG_DEPSTRUCT;
#endif

/* New member added in libpng-1.0.4 (renamed in 1.0.9) */
#if defined(PNG_MNG_FEATURES_SUPPORTED) || \
    defined(PNG_READ_EMPTY_PLTE_SUPPORTED) || \
    defined(PNG_WRITE_EMPTY_PLTE_SUPPORTED)
/* Changed from png_byte to png_uint_32 at version 1.2.0 */
   png_uint_32 mng_features_permitted PNG_DEPSTRUCT;
#endif

/* New member added in libpng-1.0.7 */
#if defined(PNG_READ_GAMMA_SUPPORTED) || defined(PNG_READ_BACKGROUND_SUPPORTED)
   png_fixed_point int_gamma PNG_DEPSTRUCT;
#endif

/* New member added in libpng-1.0.9, ifdef'ed out in 1.0.12, enabled in 1.2.0 */
#ifdef PNG_MNG_FEATURES_SUPPORTED
   png_byte filter_type PNG_DEPSTRUCT;
#endif

/* New members added in libpng-1.2.0 */

/* New members added in libpng-1.0.2 but first enabled by default in 1.2.0 */
#ifdef PNG_USER_MEM_SUPPORTED
   png_voidp mem_ptr PNG_DEPSTRUCT;             /* user supplied struct for
                                                   mem functions */
   png_malloc_ptr malloc_fn PNG_DEPSTRUCT;      /* function for
                                                   allocating memory */
   png_free_ptr free_fn PNG_DEPSTRUCT;          /* function for
                                                   freeing memory */
#endif

/* New member added in libpng-1.0.13 and 1.2.0 */
   png_bytep big_row_buf PNG_DEPSTRUCT;         /* buffer to save current
                                                   (unfiltered) row */

#ifdef PNG_READ_DITHER_SUPPORTED
/* The following three members were added at version 1.0.14 and 1.2.4 */
   png_bytep dither_sort PNG_DEPSTRUCT;            /* working sort array */
   png_bytep index_to_palette PNG_DEPSTRUCT;       /* where the original
                                                     index currently is
                                                     in the palette */
   png_bytep palette_to_index PNG_DEPSTRUCT;       /* which original index
                                                      points to this
                                                      palette color */
#endif

/* New members added in libpng-1.0.16 and 1.2.6 */
   png_byte compression_type PNG_DEPSTRUCT;

#ifdef PNG_USER_LIMITS_SUPPORTED
   png_uint_32 user_width_max PNG_DEPSTRUCT;
   png_uint_32 user_height_max PNG_DEPSTRUCT;
   /* Added in libpng-1.4.0: Total number of sPLT, text, and unknown
    * chunks that can be stored (0 means unlimited).
    */
   png_uint_32 user_chunk_cache_max PNG_DEPSTRUCT;
   /* Total memory that a zTXt, sPLT, iTXt, iCCP, or unknown chunk
    * can occupy when decompressed.  0 means unlimited.
    */
   png_uint_32 user_chunk_malloc_max PNG_DEPSTRUCT;
#endif

/* New member added in libpng-1.0.25 and 1.2.17 */
#ifdef PNG_UNKNOWN_CHUNKS_SUPPORTED
   /* Storage for unknown chunk that the library doesn't recognize. */
   png_unknown_chunk unknown_chunk PNG_DEPSTRUCT;
#endif

/* New members added in libpng-1.2.26 */
  png_uint_32 old_big_row_buf_size PNG_DEPSTRUCT;
  png_uint_32 old_prev_row_size PNG_DEPSTRUCT;

/* New member added in libpng-1.2.30 */
  png_charp chunkdata PNG_DEPSTRUCT;  /* buffer for reading chunk data */

#ifdef PNG_IO_STATE_SUPPORTED
/* New member added in libpng-1.4.0 */
   png_uint_32 io_state PNG_DEPSTRUCT;
#endif
};
