/* png_info is a structure that holds the information in a PNG file so
 * that the application can find out the characteristics of the image.
 * If you are reading the file, this structure will tell you what is
 * in the PNG file.  If you are writing the file, fill in the information
 * you want to put into the PNG file, then call png_write_info().
 * The names chosen should be very close to the PNG specification, so
 * consult that document for information about the meaning of each field.
 *
 * With libpng < 0.95, it was only possible to directly set and read the
 * the values in the png_info_struct, which meant that the contents and
 * order of the values had to remain fixed.  With libpng 0.95 and later,
 * however, there are now functions that abstract the contents of
 * png_info_struct from the application, so this makes it easier to use
 * libpng with dynamic libraries, and even makes it possible to use
 * libraries that don't have all of the libpng ancillary chunk-handing
 * functionality.
 *
 * In any case, the order of the parameters in png_info_struct should NOT
 * be changed for as long as possible to keep compatibility with applications
 * that use the old direct-access method with png_info_struct.
 *
 * The following members may have allocated storage attached that should be
 * cleaned up before the structure is discarded: palette, trans, text,
 * pcal_purpose, pcal_units, pcal_params, hist, iccp_name, iccp_profile,
 * splt_palettes, scal_unit, row_pointers, and unknowns.   By default, these
 * are automatically freed when the info structure is deallocated, if they were
 * allocated internally by libpng.  This behavior can be changed by means
 * of the png_data_freer() function.
 *
 * More allocation details: all the chunk-reading functions that
 * change these members go through the corresponding png_set_*
 * functions.  A function to clear these members is available: see
 * png_free_data().  The png_set_* functions do not depend on being
 * able to point info structure members to any of the storage they are
 * passed (they make their own copies), EXCEPT that the png_set_text
 * functions use the same storage passed to them in the text_ptr or
 * itxt_ptr structure argument, and the png_set_rows and png_set_unknowns
 * functions do not make their own copies.
 */
typedef struct png_info_struct
{
   /* the following are necessary for every PNG file */
   png_uint_32 width PNG_DEPSTRUCT;  /* width of image in pixels (from IHDR) */
   png_uint_32 height PNG_DEPSTRUCT; /* height of image in pixels (from IHDR) */
   png_uint_32 valid PNG_DEPSTRUCT;  /* valid chunk data (see PNG_INFO_
                                        below) */
   png_size_t rowbytes PNG_DEPSTRUCT; /* bytes needed to hold an untransformed
                                         row */
   png_colorp palette PNG_DEPSTRUCT;      /* array of color values
                                             (valid & PNG_INFO_PLTE) */
   png_uint_16 num_palette PNG_DEPSTRUCT; /* number of color entries in
                                             "palette" (PLTE) */
   png_uint_16 num_trans PNG_DEPSTRUCT;   /* number of transparent palette
                                             color (tRNS) */
   png_byte bit_depth PNG_DEPSTRUCT;      /* 1, 2, 4, 8, or 16 bits/channel
                                             (from IHDR) */
   png_byte color_type PNG_DEPSTRUCT;     /* see PNG_COLOR_TYPE_ below
                                             (from IHDR) */
   /* The following three should have been named *_method not *_type */
   png_byte compression_type PNG_DEPSTRUCT; /* must be
                                             PNG_COMPRESSION_TYPE_BASE (IHDR) */
   png_byte filter_type PNG_DEPSTRUCT;    /* must be PNG_FILTER_TYPE_BASE
                                             (from IHDR) */
   png_byte interlace_type PNG_DEPSTRUCT; /* One of PNG_INTERLACE_NONE,
                                             PNG_INTERLACE_ADAM7 */

   /* The following is informational only on read, and not used on writes. */
   png_byte channels PNG_DEPSTRUCT;       /* number of data channels per
                                             pixel (1, 2, 3, 4) */
   png_byte pixel_depth PNG_DEPSTRUCT;    /* number of bits per pixel */
   png_byte spare_byte PNG_DEPSTRUCT;     /* to align the data, and for
                                             future use */
   png_byte signature[8] PNG_DEPSTRUCT;   /* magic bytes read by libpng
                                             from start of file */

   /* The rest of the data is optional.  If you are reading, check the
    * valid field to see if the information in these are valid.  If you
    * are writing, set the valid field to those chunks you want written,
    * and initialize the appropriate fields below.
    */

#if defined(PNG_gAMA_SUPPORTED) && defined(PNG_FLOATING_POINT_SUPPORTED)
   /* The gAMA chunk describes the gamma characteristics of the system
    * on which the image was created, normally in the range [1.0, 2.5].
    * Data is valid if (valid & PNG_INFO_gAMA) is non-zero.
    */
   float gamma PNG_DEPSTRUCT; /* gamma value of image,
                                 if (valid & PNG_INFO_gAMA) */
#endif

#ifdef PNG_sRGB_SUPPORTED
    /* GR-P, 0.96a */
    /* Data valid if (valid & PNG_INFO_sRGB) non-zero. */
   png_byte srgb_intent PNG_DEPSTRUCT; /* sRGB rendering intent
                                          [0, 1, 2, or 3] */
#endif

#ifdef PNG_TEXT_SUPPORTED
   /* The tEXt, and zTXt chunks contain human-readable textual data in
    * uncompressed, compressed, and optionally compressed forms, respectively.
    * The data in "text" is an array of pointers to uncompressed,
    * null-terminated C strings. Each chunk has a keyword that describes the
    * textual data contained in that chunk.  Keywords are not required to be
    * unique, and the text string may be empty.  Any number of text chunks may
    * be in an image.
    */
   int num_text PNG_DEPSTRUCT; /* number of comments read/to write */
   int max_text PNG_DEPSTRUCT; /* current size of text array */
   png_textp text PNG_DEPSTRUCT; /* array of comments read/to write */
#endif /* PNG_TEXT_SUPPORTED */

#ifdef PNG_tIME_SUPPORTED
   /* The tIME chunk holds the last time the displayed image data was
    * modified.  See the png_time struct for the contents of this struct.
    */
   png_time mod_time PNG_DEPSTRUCT;
#endif

#ifdef PNG_sBIT_SUPPORTED
   /* The sBIT chunk specifies the number of significant high-order bits
    * in the pixel data.  Values are in the range [1, bit_depth], and are
    * only specified for the channels in the pixel data.  The contents of
    * the low-order bits is not specified.  Data is valid if
    * (valid & PNG_INFO_sBIT) is non-zero.
    */
   png_color_8 sig_bit PNG_DEPSTRUCT; /* significant bits in color channels */
#endif

#if defined(PNG_tRNS_SUPPORTED) || defined(PNG_READ_EXPAND_SUPPORTED) || \
defined(PNG_READ_BACKGROUND_SUPPORTED)
   /* The tRNS chunk supplies transparency data for paletted images and
    * other image types that don't need a full alpha channel.  There are
    * "num_trans" transparency values for a paletted image, stored in the
    * same order as the palette colors, starting from index 0.  Values
    * for the data are in the range [0, 255], ranging from fully transparent
    * to fully opaque, respectively.  For non-paletted images, there is a
    * single color specified that should be treated as fully transparent.
    * Data is valid if (valid & PNG_INFO_tRNS) is non-zero.
    */
   png_bytep trans_alpha PNG_DEPSTRUCT;    /* alpha values for paletted
                                              image */
   png_color_16 trans_color PNG_DEPSTRUCT; /* transparent color for
                                              non-palette image */
#endif

#if defined(PNG_bKGD_SUPPORTED) || defined(PNG_READ_BACKGROUND_SUPPORTED)
   /* The bKGD chunk gives the suggested image background color if the
    * display program does not have its own background color and the image
    * is needs to composited onto a background before display.  The colors
    * in "background" are normally in the same color space/depth as the
    * pixel data.  Data is valid if (valid & PNG_INFO_bKGD) is non-zero.
    */
   png_color_16 background PNG_DEPSTRUCT;
#endif

#ifdef PNG_oFFs_SUPPORTED
   /* The oFFs chunk gives the offset in "offset_unit_type" units rightwards
    * and downwards from the top-left corner of the display, page, or other
    * application-specific co-ordinate space.  See the PNG_OFFSET_ defines
    * below for the unit types.  Valid if (valid & PNG_INFO_oFFs) non-zero.
    */
   png_int_32 x_offset PNG_DEPSTRUCT; /* x offset on page */
   png_int_32 y_offset PNG_DEPSTRUCT; /* y offset on page */
   png_byte offset_unit_type PNG_DEPSTRUCT; /* offset units type */
#endif

#ifdef PNG_pHYs_SUPPORTED
   /* The pHYs chunk gives the physical pixel density of the image for
    * display or printing in "phys_unit_type" units (see PNG_RESOLUTION_
    * defines below).  Data is valid if (valid & PNG_INFO_pHYs) is non-zero.
    */
   png_uint_32 x_pixels_per_unit PNG_DEPSTRUCT; /* horizontal pixel density */
   png_uint_32 y_pixels_per_unit PNG_DEPSTRUCT; /* vertical pixel density */
   png_byte phys_unit_type PNG_DEPSTRUCT; /* resolution type (see
                                             PNG_RESOLUTION_ below) */
#endif

#ifdef PNG_hIST_SUPPORTED
   /* The hIST chunk contains the relative frequency or importance of the
    * various palette entries, so that a viewer can intelligently select a
    * reduced-color palette, if required.  Data is an array of "num_palette"
    * values in the range [0,65535]. Data valid if (valid & PNG_INFO_hIST)
    * is non-zero.
    */
   png_uint_16p hist PNG_DEPSTRUCT;
#endif

#ifdef PNG_cHRM_SUPPORTED
   /* The cHRM chunk describes the CIE color characteristics of the monitor
    * on which the PNG was created.  This data allows the viewer to do gamut
    * mapping of the input image to ensure that the viewer sees the same
    * colors in the image as the creator.  Values are in the range
    * [0.0, 0.8].  Data valid if (valid & PNG_INFO_cHRM) non-zero.
    */
#ifdef PNG_FLOATING_POINT_SUPPORTED
   float x_white PNG_DEPSTRUCT;
   float y_white PNG_DEPSTRUCT;
   float x_red PNG_DEPSTRUCT;
   float y_red PNG_DEPSTRUCT;
   float x_green PNG_DEPSTRUCT;
   float y_green PNG_DEPSTRUCT;
   float x_blue PNG_DEPSTRUCT;
   float y_blue PNG_DEPSTRUCT;
#endif
#endif

#ifdef PNG_pCAL_SUPPORTED
   /* The pCAL chunk describes a transformation between the stored pixel
    * values and original physical data values used to create the image.
    * The integer range [0, 2^bit_depth - 1] maps to the floating-point
    * range given by [pcal_X0, pcal_X1], and are further transformed by a
    * (possibly non-linear) transformation function given by "pcal_type"
    * and "pcal_params" into "pcal_units".  Please see the PNG_EQUATION_
    * defines below, and the PNG-Group's PNG extensions document for a
    * complete description of the transformations and how they should be
    * implemented, and for a description of the ASCII parameter strings.
    * Data values are valid if (valid & PNG_INFO_pCAL) non-zero.
    */
   png_charp pcal_purpose PNG_DEPSTRUCT;  /* pCAL chunk description string */
   png_int_32 pcal_X0 PNG_DEPSTRUCT;      /* minimum value */
   png_int_32 pcal_X1 PNG_DEPSTRUCT;      /* maximum value */
   png_charp pcal_units PNG_DEPSTRUCT;    /* Latin-1 string giving physical
                                             units */
   png_charpp pcal_params PNG_DEPSTRUCT;  /* ASCII strings containing
                                             parameter values */
   png_byte pcal_type PNG_DEPSTRUCT;      /* equation type
                                             (see PNG_EQUATION_ below) */
   png_byte pcal_nparams PNG_DEPSTRUCT;   /* number of parameters given
                                             in pcal_params */
#endif

/* New members added in libpng-1.0.6 */
   png_uint_32 free_me PNG_DEPSTRUCT;     /* flags items libpng is
                                             responsible for freeing */

#if defined(PNG_UNKNOWN_CHUNKS_SUPPORTED) || \
 defined(PNG_HANDLE_AS_UNKNOWN_SUPPORTED)
   /* Storage for unknown chunks that the library doesn't recognize. */
   png_unknown_chunkp unknown_chunks PNG_DEPSTRUCT;
   png_size_t unknown_chunks_num PNG_DEPSTRUCT;
#endif

#ifdef PNG_iCCP_SUPPORTED
   /* iCCP chunk data. */
   png_charp iccp_name PNG_DEPSTRUCT;     /* profile name */
   png_charp iccp_profile PNG_DEPSTRUCT;  /* International Color Consortium
                                             profile data */
                            /* Note to maintainer: should be png_bytep */
   png_uint_32 iccp_proflen PNG_DEPSTRUCT;  /* ICC profile data length */
   png_byte iccp_compression PNG_DEPSTRUCT; /* Always zero */
#endif

#ifdef PNG_sPLT_SUPPORTED
   /* Data on sPLT chunks (there may be more than one). */
   png_sPLT_tp splt_palettes PNG_DEPSTRUCT;
   png_uint_32 splt_palettes_num PNG_DEPSTRUCT;
#endif

#ifdef PNG_sCAL_SUPPORTED
   /* The sCAL chunk describes the actual physical dimensions of the
    * subject matter of the graphic.  The chunk contains a unit specification
    * a byte value, and two ASCII strings representing floating-point
    * values.  The values are width and height corresponsing to one pixel
    * in the image.  This external representation is converted to double
    * here.  Data values are valid if (valid & PNG_INFO_sCAL) is non-zero.
    */
   png_byte scal_unit PNG_DEPSTRUCT;         /* unit of physical scale */
#ifdef PNG_FLOATING_POINT_SUPPORTED
   double scal_pixel_width PNG_DEPSTRUCT;    /* width of one pixel */
   double scal_pixel_height PNG_DEPSTRUCT;   /* height of one pixel */
#endif
#ifdef PNG_FIXED_POINT_SUPPORTED
   png_charp scal_s_width PNG_DEPSTRUCT;     /* string containing height */
   png_charp scal_s_height PNG_DEPSTRUCT;    /* string containing width */
#endif
#endif

#ifdef PNG_INFO_IMAGE_SUPPORTED
   /* Memory has been allocated if (valid & PNG_ALLOCATED_INFO_ROWS)
      non-zero */
   /* Data valid if (valid & PNG_INFO_IDAT) non-zero */
   png_bytepp row_pointers PNG_DEPSTRUCT;        /* the image bits */
#endif

#if defined(PNG_FIXED_POINT_SUPPORTED) && defined(PNG_gAMA_SUPPORTED)
   png_fixed_point int_gamma PNG_DEPSTRUCT; /* gamma of image,
                                               if (valid & PNG_INFO_gAMA) */
#endif

#if defined(PNG_cHRM_SUPPORTED) && defined(PNG_FIXED_POINT_SUPPORTED)
   png_fixed_point int_x_white PNG_DEPSTRUCT;
   png_fixed_point int_y_white PNG_DEPSTRUCT;
   png_fixed_point int_x_red PNG_DEPSTRUCT;
   png_fixed_point int_y_red PNG_DEPSTRUCT;
   png_fixed_point int_x_green PNG_DEPSTRUCT;
   png_fixed_point int_y_green PNG_DEPSTRUCT;
   png_fixed_point int_x_blue PNG_DEPSTRUCT;
   png_fixed_point int_y_blue PNG_DEPSTRUCT;
#endif

} png_info;
