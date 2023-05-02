/* arm_init.c - RISC-V Vector optimized filter functions
 *
 * Copyright (c) 2023 Google LLC
 * Written by Dragoș Tiselice <dtiselice@google.com>, May 2023.
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */

#include "../pngpriv.h"

#ifdef PNG_READ_SUPPORTED

#if PNG_RISCV_VECTOR_OPT > 0
#ifdef PNG_RISCV_VECTOR_CHECK_SUPPORTED /* Do run-time checks */
/* WARNING: it is strongly recommended that you do not build libpng with
 * run-time checks for CPU features if at all possible.  In the case of the
 * RISC-V Vector instructions there is no processor-specific way of detecting
 * the presence of the required support, therefore run-time detection is
 * extremely OS specific.
 *
 * You may set the macro PNG_RISCV_VECTOR_FILE to the file name of file containing
 * a fragment of C source code which defines the png_have_neon function.  There
 * are a number of implementations in contrib/riscv-vector, but the only one that
 * has partial support is contrib/riscv-vector/linux.c - a generic Linux
 * implementation which reads /proc/cpuinfo.
 */

#ifndef PNG_RISCV_VECTOR_FILE
#  if defined(__linux__)
#    define PNG_RISCV_VECTOR_FILE "contrib/riscv-vector/linux.c"
#  else
#    error "No support for run-time RISC-V Vector checking; use compile-time options"
#  endif
#endif

static int png_have_vector(png_structp png_ptr);
#ifdef PNG_RISCV_VECTOR_FILE
#  include PNG_RISCV_VECTOR_FILE
#endif
#endif /* PNG_RISCV_VECTOR_CHECK_SUPPORTED */

void
png_init_filter_functions_vector(png_structp pp, unsigned int bpp)
{
   /* The switch statement is compiled in for RISCV_VECTOR_API, the call to
    * png_have_vector is compiled in for RISCV_VECTOR_CHECK.  If both are
    * defined the check is only performed if the API has not set the VECTOR
    * option on or off explicitly.  In this case the check controls what
    * happens.
    *
    * If the CHECK is not compiled in and the option is UNSET the behavior prior
    * to 1.6.7 was to use the NEON code - this was a bug caused by having the
    * wrong order of the 'ON' and 'default' cases.  UNSET now defaults to OFF,
    * as documented in png.h
    */
   png_debug(1, "in png_init_filter_functions_vector");
#ifdef PNG_RISCV_VECTOR_API_SUPPORTED
   switch ((pp->options >> PNG_RISCV_VECTOR) & 3)
   {
      case PNG_OPTION_UNSET:
         /* Allow the run-time check to execute if it has been enabled -
          * thus both API and CHECK can be turned on.  If it isn't supported
          * this case will fall through to the 'default' below, which just
          * returns.
          */
#endif /* PNG_RISCV_VECTOR_API_SUPPORTED */
#ifdef PNG_RISCV_VECTOR_CHECK_SUPPORTED
         {
            static volatile sig_atomic_t no_vector = -1; /* not checked */

            if (no_vector < 0)
               no_vector = !png_have_vector(pp);

            if (no_vector)
               return;
         }
#ifdef PNG_RISCV_VECTOR_API_SUPPORTED
         break;
#endif
#endif /* PNG_RISCV_VECTOR_CHECK_SUPPORTED */

#ifdef PNG_RISCV_VECTOR_API_SUPPORTED
      default: /* OFF or INVALID */
         return;

      case PNG_OPTION_ON:
         /* Option turned on */
         break;
   }
#endif

   /* IMPORTANT: any new external functions used here must be declared using
    * PNG_INTERNAL_FUNCTION in ../pngpriv.h.  This is required so that the
    * 'prefix' option to configure works:
    *
    *    ./configure --with-libpng-prefix=foobar_
    *
    * Verify you have got this right by running the above command, doing a build
    * and examining pngprefix.h; it must contain a #define for every external
    * function you add.  (Notice that this happens automatically for the
    * initialization function.)
    */
   pp->read_filter[PNG_FILTER_VALUE_UP-1] = png_read_filter_row_up_vector;

   if (bpp == 3)
   {
      pp->read_filter[PNG_FILTER_VALUE_AVG-1] = png_read_filter_row_avg3_vector;
      pp->read_filter[PNG_FILTER_VALUE_PAETH-1] = png_read_filter_row_paeth3_vector;
      if (__riscv_vsetvlmax_e8m1() > 16) {
         pp->read_filter[PNG_FILTER_VALUE_SUB-1] = png_read_filter_row_sub3_vector_128;
      } else {
         pp->read_filter[PNG_FILTER_VALUE_SUB-1] = png_read_filter_row_sub3_vector_256;
      }
   }
   else if (bpp == 4)
   {
      pp->read_filter[PNG_FILTER_VALUE_AVG-1] = png_read_filter_row_avg4_vector;
      pp->read_filter[PNG_FILTER_VALUE_PAETH-1] = png_read_filter_row_paeth4_vector;
      if (__riscv_vsetvlmax_e8m1() > 16) {
         pp->read_filter[PNG_FILTER_VALUE_SUB-1] = png_read_filter_row_sub4_vector_128;
      } else {
         pp->read_filter[PNG_FILTER_VALUE_SUB-1] = png_read_filter_row_sub4_vector_256;
      }
   }
}

#endif /* PNG_RISCV_VECTOR_OPT > 0 */
#endif /* PNG_READ_SUPPORTED */
