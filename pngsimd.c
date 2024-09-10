/* pngsimd.c - hardware (cpu/arch) specific code
 *
 * Copyright (c) 2018-2024 Cosmin Truta
 * Copyright (c) 1998-2002,2004,2006-2018 Glenn Randers-Pehrson
 * Copyright (c) 1996-1997 Andreas Dilger
 * Copyright (c) 1995-1996 Guy Eric Schalnat, Group 42, Inc.
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * NOTE: this code is copied from libpng1.6 pngpriv.h.
 */
#include "pngpriv.h"

#ifdef PNG_TARGET_CODE_IMPLEMENTATION
/* This is set by pngtarget.h iff there is some target code to be compiled.
 */

/* Each piece of separate hardware support code must have a "init" file defined
 * in PNG_TARGET_CODE_IMPLEMENTATION and included here.
 *
 * The "check" header set PNG_TARGET_CODE_IMPLEMENTATION and that file *MUST*
 * supply macro definitions as follows.  Note that all functions must be static
 * to avoid clashes with other implementations.
 *
 *    png_target_impl
 *       string constant
 *       REQUIRED
 *       This must be a string naming the implemenation.
 *
 *    png_target_free_data_impl
 *       static void png_target_free_data_impl(png_structrp)
 *       REQUIRED if PNG_TARGET_STORES_DATA is defined
 *       UNDEFINED if PNG_TARGET_STORES_DATA is not defined
 *       A function to free data stored in png_struct::target_data.
 *
 *    png_target_init_filter_functions_impl
 *       OPTIONAL 
 *       Contains code to overwrite the png_struct::read_filter array, see
 *       the definition of png_init_filter_functions.  Need not be defined,
 *       only called if target_state contains png_target_filters.
 *
 *    png_target_init_palette_support_impl
 *       static function
 *       OPTIONAL
 *       Contains code to initialize a palette transformation.  This returns
 *       true if something has been set up.  Only called if the state contains
 *       png_target_palette, need not be defined, may cancel the state flag
 *       in the png_struct to prevent further calls.
 *
 *    png_target_do_expand_palette_impl
 *       static function
 *       OPTIONAL
 *       Handles palette expansion.  Need not be defined, only called if the
 *       state contains png_target_palette, may set this flag to zero, may
 *       return false to indicate that the expansion was not done.
 *
 * Either png_target_init_filter_functions_impl or
 * png_target_do_expand_palette_impl must be defined.
 */

/* This will fail in an obvious way with a meaningful error message if the file
 * does not exist:
 */
#include PNG_TARGET_CODE_IMPLEMENTATION

#ifndef png_target_impl
#  error HARDWARE: PNG_TARGET_CODE_IMPLEMENTATION defined but not png_hareware_impl
#endif

#if defined(PNG_TARGET_STORES_DATA) != defined(png_target_free_data_impl)
#  error HARDWARE: PNG_TARGET_STORES_DATA !match png_target_free_data_impl
#endif

#if !defined(png_target_init_filter_functions_impl) &&\
    !defined(png_target_init_palette_support)
#  error HARDWARE: target specifc code turned on but none provided
#endif

void
png_target_init(png_structrp pp)
{
   /* Initialize png_struct::target_state if required. */
#  ifdef png_target_init_filter_functions_impl
#     define F png_target_filters
#  else
#     define F 0U
#  endif
#  ifdef png_target_do_expand_palette_impl
#     define P png_target_palette
#  else
#     define P 0U
#  endif

#  if F|P
      pp->target_state = F|P;
#  else
      PNG_UNUSED(pp);
#  endif
}

#ifdef PNG_TARGET_STORES_DATA
#ifndef png_target_free_data_impl
#  error PNG_TARGET_STORES_DATA defined without implementation
#endif
void
png_target_free_data(png_structrp pp)
{
   /* Free any data allocated in the png_struct::target_data.
    */
   if (pp->target_data != NULL)
   {
         png_target_free_data_impl(pp);
      if (pp->target_data != NULL)
         png_error(pp, png_target_impl ": allocated data not released");
   }
}
#endif

#ifdef PNG_TARGET_IMPLEMENTS_FILTERS
#ifndef png_target_init_filter_functions_impl
#  error PNG_TARGET_IMPLEMENTS_FILTERS defined without implementation
#endif
void
png_target_init_filter_functions(png_structp pp, unsigned int bpp)
{
   if (((pp->options >> PNG_TARGET_SPECIFIC_CODE) & 3) == PNG_OPTION_ON &&
       (pp->target_state & png_target_filters) != 0)
      png_target_init_filter_functions_impl(pp, bpp);
}
#endif /* filters */

#ifdef PNG_TARGET_IMPLEMENTS_EXPAND_PALETTE
#ifndef png_target_init_palette_support_impl
#  error PNG_TARGET_IMPLEMENTS_EXPAND_PALETTE defined without implementation
#endif
void
png_target_init_palette_support(png_structrp pp)
{
   if (((pp->options >> PNG_TARGET_SPECIFIC_CODE) & 3) == PNG_OPTION_ON &&
       (pp->target_state & png_target_palette) != 0 &&
      !png_target_init_palette_support_impl(pp, bpp))
      png_ptr->target_state &= ~png_target_palette;
}

#ifndef png_target_do_expand_palette_impl
#  error PNG_TARGET_IMPLEMENTS_EXPAND_PALETTE defined without implementation
#endif
int
png_target_do_expand_palette(png_structrp pp, png_row_infop rip,
   png_const_bytep row, const png_bytepp ssp, const png_bytepp ddp)
{
   if (((pp->options >> PNG_TARGET_SPECIFIC_CODE) & 3) == PNG_OPTION_ON &&
       (pp->target_state & png_target_palette) != 0)
      return png_target_do_expand_palette_impl(pp, rip, row, ssp, ddp);
}
#endif /* palette */

/*
 *    png_target_init_impl
 *       Set the mask of png_target_support values to
 *       png_struct::target_state.  If the value is non-0 hardware support
 *       will be recorded as enabled.
 *
 *    png_target_free_data_impl
 *       Must be defined if the implementation stores data in
 *       png_struct::target_data.  Need not be defined otherwise.
 *
 *    png_target_init_filter_functions_impl
 *       Contains code to overwrite the png_struct::read_filter array, see
 *       the definition of png_init_filter_functions.  Need not be defined,
 *       only called if the state contains png_target_filters.
 *
 *    png_target_init_palette_support_impl
 *       Contains code to initialize a palette transformation.  This returns
 *       true if something has been set up.  Only called if the state contains
 *       png_target_palette, need not be defined, may cancel the state flag
 *       in the png_struct to prevent further calls.
 *
 *    png_target_do_expand_palette
 *       Handles palette expansion.  Need not be defined, only called if the
 *       state contains png_target_palette, may set this flag to zero, may
 *       return false to indicate that the expansion was not done.
 */
#endif /* PNG_TARGET_ARCH */
