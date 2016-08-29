
/* filter_msa_intrinsics.c - MSA optimised filter functions
 *
 * Copyright (c) 2016 Glenn Randers-Pehrson
 * Written by Mandar Sahastrabuddhe, August 2016.
 * Last changed in libpng 1.6.25 [(PENDING RELEASE)]
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */
#include <stdio.h>
#include <stdint.h>
#include "../pngpriv.h"

#ifdef PNG_READ_SUPPORTED

/* This code requires -mfpu=msa on the command line: */
#if PNG_MIPS_MSA_IMPLEMENTATION == 1 /* intrinsics code from pngpriv.h */

#include <msa.h>

/* libpng row pointers are not necessarily aligned to any particular boundary,
 * however this code will only work with appropriate alignment. mips/mips_init.c
 * checks for this (and will not compile unless it is done). This code uses
 * variants of png_aligncast to avoid compiler warnings.
 */
#define png_ptr(type,pointer) png_aligncast(type *,pointer)
#define png_ptrc(type,pointer) png_aligncastconst(const type *,pointer)

/* The following relies on a variable 'temp_pointer' being declared with type
 * 'type'.  This is written this way just to hide the GCC strict aliasing
 * warning; note that the code is safe because there never is an alias between
 * the input and output pointers.
 */
#define png_ldr(type,pointer)\
   (temp_pointer = png_ptr(type,pointer), *temp_pointer)

#if PNG_MIPS_MSA_OPT > 0

#define LD_B(RTYPE, psrc) *((RTYPE *) (psrc))
#define LD_UB(...) LD_B(v16u8, __VA_ARGS__)
#define LD_B2(RTYPE, psrc, stride, out0, out1)  \
{                                               \
    out0 = LD_B(RTYPE, (psrc));                 \
    out1 = LD_B(RTYPE, (psrc) + stride);        \
}
#define LD_UB2(...) LD_B2(v16u8, __VA_ARGS__)
#define LD_B4(RTYPE, psrc, stride, out0, out1, out2, out3)   \
{                                                            \
    LD_B2(RTYPE, (psrc), stride, out0, out1);                \
    LD_B2(RTYPE, (psrc) + 2 * stride , stride, out2, out3);  \
}
#define LD_UB4(...) LD_B4(v16u8, __VA_ARGS__)

#define ST_B(RTYPE, in, pdst) *((RTYPE *) (pdst)) = (in)
#define ST_UB(...) ST_B(v16u8, __VA_ARGS__)
#define ST_B2(RTYPE, in0, in1, pdst, stride)  \
{                                             \
    ST_B(RTYPE, in0, (pdst));                 \
    ST_B(RTYPE, in1, (pdst) + stride);        \
}
#define ST_UB2(...) ST_B2(v16u8, __VA_ARGS__)
#define ST_B4(RTYPE, in0, in1, in2, in3, pdst, stride)    \
{                                                         \
    ST_B2(RTYPE, in0, in1, (pdst), stride);               \
    ST_B2(RTYPE, in2, in3, (pdst) + 2 * stride, stride);  \
}
#define ST_UB4(...) ST_B4(v16u8, __VA_ARGS__)

#define ADD2(in0, in1, in2, in3, out0, out1)  \
{                                             \
    out0 = in0 + in1;                         \
    out1 = in2 + in3;                         \
}
#define ADD3(in0, in1, in2, in3, in4, in5,  \
             out0, out1, out2)              \
{                                           \
    ADD2(in0, in1, in2, in3, out0, out1);   \
    out2 = in4 + in5;                       \
}
#define ADD4(in0, in1, in2, in3, in4, in5, in6, in7,  \
             out0, out1, out2, out3)                  \
{                                                     \
    ADD2(in0, in1, in2, in3, out0, out1);             \
    ADD2(in4, in5, in6, in7, out2, out3);             \
}

void png_read_filter_row_up_msa(png_row_infop row_info, png_bytep row,
                                png_const_bytep prev_row)
{
   png_size_t i, cnt, cnt16, cnt32;
   png_size_t istop = row_info->rowbytes;
   png_bytep rp = row;
   png_const_bytep pp = prev_row;
   v16u8 src0, src1, src2, src3, src4, src5, src6, src7;

   for (i = 0; i < (istop >> 6); i++)
   {
      LD_UB4(rp, 16, src0, src1, src2, src3);
      LD_UB4(pp, 16, src4, src5, src6, src7);
      pp += 64;

	  ADD4(src0, src4, src1, src5, src2, src6, src3, src7,
	       src0, src1, src2, src3);

      ST_UB4(src0, src1, src2, src3, rp, 16);
      rp += 64;
   }

   if (istop & 0x3F)
   {
      cnt32 = istop & 0x20;
      cnt16 = istop & 0x10;
      cnt = istop & 0xF;

      if(cnt32)
      {
         if (cnt16 && cnt)
         {
            LD_UB4(rp, 16, src0, src1, src2, src3);
            LD_UB4(pp, 16, src4, src5, src6, src7);

            ADD4(src0, src4, src1, src5, src2, src6, src3, src7,
	             src0, src1, src2, src3);

            ST_UB4(src0, src1, src2, src3, rp, 16);
            rp += 64;
         }
         else if (cnt16 || cnt)
         {
            LD_UB2(rp, 16, src0, src1);
            LD_UB2(pp, 16, src4, src5);
            pp += 32;
            src2 = LD_UB(rp + 32);
            src6 = LD_UB(pp);

            ADD3(src0, src4, src1, src5, src2, src6, src0, src1, src2);

            ST_UB2(src0, src1, rp, 16);
            rp += 32;
            ST_UB(src2, rp);
            rp += 16;
         }
         else
         {
            LD_UB2(rp, 16, src0, src1);
            LD_UB2(pp, 16, src4, src5);

			ADD2(src0, src4, src1, src5, src0, src1);

            ST_UB2(src0, src1, rp, 16);
            rp += 32;
         }
      }
      else if (cnt16 && cnt)
      {
         LD_UB2(rp, 16, src0, src1);
         LD_UB2(pp, 16, src4, src5);

         ADD2(src0, src4, src1, src5, src0, src1);

         ST_UB2(src0, src1, rp, 16);
         rp += 32;
      }
      else if (cnt16 || cnt)
      {
         src0 = LD_UB(rp);
         src4 = LD_UB(pp);
         pp += 16;

         src0 += src4;

         ST_UB(src0, rp);
         rp += 16;
      }
   }
}

#endif /* PNG_MIPS_MSA_OPT > 0 */
#endif /* PNG_MIPS_MSA_IMPLEMENTATION == 1 (intrinsics) */
#endif /* READ */
