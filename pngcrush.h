/* pngcrush.h */

/* Special defines for pngcrush, mostly just to reduce the size of the
   static executable. */

#define PNG_NO_FLOATING_POINT_SUPPORTED /* undef this if you want to be able
                                           to reduce color to gray */
#define PNG_NO_READ_cHRM
#define PNG_NO_WRITE_cHRM
#define PNG_NO_READ_hIST
#define PNG_NO_WRITE_hIST
#define PNG_NO_READ_iCCP
#define PNG_NO_WRITE_iCCP
#define PNG_NO_READ_pCAL
#define PNG_NO_WRITE_pCAL
#define PNG_NO_READ_sCAL
#define PNG_NO_WRITE_sCAL
#define PNG_NO_READ_sPLT
#define PNG_NO_WRITE_sPLT
#define PNG_NO_READ_tIME
#define PNG_NO_WRITE_tIME

#define PNG_NO_INFO_IMAGE
#define PNG_NO_READ_USER_CHUNKS
#define PNG_NO_EASY_ACCESS
#define PNG_NO_READ_EMPTY_PLTE
#define PNG_NO_WRITE_TRANSFORMS
#define PNG_NO_PROGRESSIVE_READ
#define PNG_NO_WRITE_WEIGHTED_FILTER
#define PNG_READ_USER_TRANSFORM_SUPPORTED
#define PNG_READ_STRIP_ALPHA_SUPPORTED
#define PNG_READ_EXPAND_SUPPORTED
#define PNG_READ_FILLER_SUPPORTED
#ifndef PNG_NO_FLOATING_POINT_SUPPORTED
#  define PNG_READ_GRAY_TO_RGB_SUPPORTED
#  define PNG_READ_RGB_TO_GRAY_SUPPORTED
#  define PNG_READ_BACKGROUND_SUPPORTED
#  define PNG_READ_GAMMA_SUPPORTED
#else
#  define PNG_NO_READ_RGB_TO_GRAY
#endif
#define PNG_ZBUF_SIZE 524288       /* increases the IDAT size */
#define PNG_NO_GLOBAL_ARRAYS
#define TOO_FAR 32767     /* Improves zlib/deflate compression */
