/* pnghardware.c - hardware (cpu/arch) specific code
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

#ifdef PNG_HARDWARE_SUPPORTED
/* Each piece of separate hardware support code must have a "code" file which
 * is loaded here.  The file must contain all the checks required to determine
 * if the code will work and these checks must be mutually exclusive between
 * the various implementations.  "arch/code" is loaded for every platform; there
 * must be no architecture specific code in pnghardware.c.
 *
 * "code" must DEFINE (not declare) the required functions and these must be
 * static to avoid the need for PNG_PREFIX handling.  The functions need not
 * have unique names because only one "code" should evaluate to anything.
 *
 * A "failed" "code" should not define anything.  Any additional "check.h" file
 * may be defined so that checks can be shared across a single architecture,
 * as in the MIPS case.
 *
 * The "successful" "code" must define surrogates for the internal png_hardware
 * functions defined in pngpriv.h and these must take exactly the same arguments
 * and return exactly the same result code.  If a surrogate is not defined by
 * the end of the includes the actual implementation will just return 0.
 *
 * Note that these must be macro definitions so that the actual implementation
 * just compiles the code required.
 *
 *    png_hardware_impl
 *       This must be a string defining the implemenation.  It must be defined
 *       or none of the other definitions will be used.
 *
 *    png_hardware_init_impl
 *       Set the mask of png_hardware_support values to
 *       png_struct::hardware_state.  If the value is non-0 hardware support
 *       will be recorded as enabled.
 *
 *    png_hardware_free_data_impl
 *       Must be defined if the implementation stores data in
 *       png_struct::hardware_data.  Need not be defined otherwise.
 *
 *    png_hardware_init_filter_functions_impl
 *       Contains code to overwrite the png_struct::read_filter array, see
 *       the definition of png_init_filter_functions.  Need not be defined,
 *       only called if the state contains png_hardware_filters.
 *
 *    png_hardware_init_palette_support_impl
 *       Contains code to initialize a palette transformation.  This returns
 *       true if something has been set up.  Only called if the state contains
 *       png_hardware_palette, need not be defined, may cancel the state flag
 *       in the png_struct to prevent further calls.
 *
 *    png_hardware_do_expand_palette_impl
 *       Handles palette expansion.  Need not be defined, only called if the
 *       state contains png_hardware_palette, may set this flag to zero, may
 *       return false to indicate that the expansion was not done.
 *
 * NEW code: add new code to the START of the two lists below.  "check.h"
 * entries come first then "code.c" includes.
 * this comment.
 */

#include "loongarch/loongarch_lsx_init.c"
#include "mips/mips_init.c"
#include "powerpc/powerpc_init.c"
#include "intel/intel_init.c"
#define PNG_WIP_DISABLE_PALETTE
#include "arm/arm_init.c"

#ifndef png_hardware_impl
#  if defined(png_hardware_init_impl) ||\
      defined(png_hardware_init_filter_functions_impl) ||\
      defined(png_hardware_init_palette_support_impl) ||\
      defined(png_hardware_free_data_impl) ||\
      defined(png_hardware_filter_functions_impl) ||\
      defined(png_hardware_do_expand_palette_impl) ||\
      defined(png_hardware_)
#     error HARDWARE: hardware implemenations defined but not png_hardware_impl
#  endif

#  define png_hardware_impl "none"
#endif

void
png_hardware_init(png_structrp pp)
{
   /* Initialize png_struct::hardware_state if required. */
#  ifdef png_hardware_init_filter_functions_impl
#     define F png_hardware_filters
#  else
#     define F 0U
#  endif
#  ifdef png_hardware_init_palette_support
#     define P png_hardware_palette
#  else
#     define P 0U
#  endif

#  if F|P
      pp->hardware_state = F|P;
#  else
      PNG_UNUSED(pp);
#  endif
}

void
png_hardware_free_data(png_structrp pp)
{
   /* Free any data allocated in the png_struct::hardware_data.
    */
   if (pp->hardware_data != NULL)
   {
#     ifdef png_hardware_free_data_impl
         png_hardware_free_data_impl(pp);
#     endif
      if (pp->hardware_data != NULL)
         png_error(pp, png_hardware_impl ": allocated data not released");
   }
}

void
png_hardware_init_filter_functions(png_structp pp, unsigned int bpp)
{
#  ifdef png_hardware_init_filter_functions_impl
      png_hardware_init_filter_functions_impl(pp, bpp);
#  else
      PNG_UNUSED(pp);
      PNG_UNUSED(bpp);
#  endif
}

void
png_hardware_init_palette_support(png_structrp pp)
{
#  ifdef png_hardware_init_palette_support_impl
      if (!png_hardware_init_palette_support_impl(pp, bpp))
         png_ptr->hardware_state &= ~png_hardware_palette;
#  else
      PNG_UNUSED(pp);
#  endif
}

int
png_hardware_do_expand_palette(png_structrp pp, png_row_infop rip,
   png_const_bytep row, const png_bytepp ssp, const png_bytepp ddp)
{
#  ifdef png_hardware_do_expand_palette_impl
      return png_hardware_do_expand_palette_impl(pp, rip, row, ssp, ddp);
#  else
      png_error(pp, png_hardware_impl ": unexpected call to do_expand_palette");
      PNG_UNUSED(rip);
      PNG_UNUSED(row);
      PNG_UNUSED(ssp);
      PNG_UNUSED(ddp);
#  endif
}

/*
 *    png_hardware_init_impl
 *       Set the mask of png_hardware_support values to
 *       png_struct::hardware_state.  If the value is non-0 hardware support
 *       will be recorded as enabled.
 *
 *    png_hardware_free_data_impl
 *       Must be defined if the implementation stores data in
 *       png_struct::hardware_data.  Need not be defined otherwise.
 *
 *    png_hardware_init_filter_functions_impl
 *       Contains code to overwrite the png_struct::read_filter array, see
 *       the definition of png_init_filter_functions.  Need not be defined,
 *       only called if the state contains png_hardware_filters.
 *
 *    png_hardware_init_palette_support_impl
 *       Contains code to initialize a palette transformation.  This returns
 *       true if something has been set up.  Only called if the state contains
 *       png_hardware_palette, need not be defined, may cancel the state flag
 *       in the png_struct to prevent further calls.
 *
 *    png_hardware_do_expand_palette
 *       Handles palette expansion.  Need not be defined, only called if the
 *       state contains png_hardware_palette, may set this flag to zero, may
 *       return false to indicate that the expansion was not done.
 */
#endif /* HARDWARE */
