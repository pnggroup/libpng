
/* filter_vector_intrinsics.c - Vector extension optimised filter functions
 *
 * Copyright (c) 2021 Manfred Schlaegl
 * Copyright (c) 2018 Cosmin Truta
 * Copyright (c) 2016 Glenn Randers-Pehrson
 * Written by Manfred Schlaegl, October 2021.
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */

#include <stdio.h>
#include <stdint.h>
#include "../pngpriv.h"

#ifdef PNG_READ_SUPPORTED

/* This code requires -march containing 'v' on the command line: */
#if PNG_RISCV_VECTOR_IMPLEMENTATION == 1 /* intrinsics code from pngpriv.h */

#if PNG_RISCV_VECTOR_OPT > 0


/* Mapping of asm mnemonics for different RISC-V Vector versions */
#if PNG_RISCV_VECTOR_COMPAT == 7
/* RISC-V Vector draft 0.7.1 */
#define VLE8_V    "vlbu.v"
#define VSE8_V    "vsb.v"
#define VNSRL_WI  "vnsrl.vi"

#elif PNG_RISCV_VECTOR_COMPAT == 8
/* RISC-V Vector draft 0.8 */
#define VLE8_V    "vlbu.v"
#define VSE8_V    "vsb.v"
#define VNSRL_WI  "vnsrl.wi"

#elif (                                   \
    ( PNG_RISCV_VECTOR_COMPAT == 9 )  ||  \
    ( PNG_RISCV_VECTOR_COMPAT == 10 ) ||  \
    ( !defined(PNG_RISCV_VECTOR_COMPAT) ) \
)
/* RISC-V Vector drafts 0.9, 0.10 and release 1.0 */
#define VLE8_V    "vle8.v"
#define VSE8_V    "vse8.v"
#define VNSRL_WI  "vnsrl.wi"

#else
#error "Invalid value for PNG_RISCV_VECTOR_COMPAT!"
#endif /* PNG_RISCV_VECTOR_COMPAT */


/*
 * The implementation implicitly assumes VLEN >= 32bit(bpp=4) which is valid
 * according to
 *  * riscv-v-spec-0.7.1/0.8.1/0.9 -- Chapter 2 ("VLEN >= SLEN >= 32")
 *  * riscv-v-spec-1.0 -- Chapter 18
 */

void png_read_filter_row_up_vector(png_row_infop row_info, png_bytep row,
                                png_const_bytep prev_row)
{
   size_t rowbytes = row_info->rowbytes;
   unsigned int vl = 0;

   /*
    * row:      | x |
    * prev_row: | b |
    *
    * b = b + x
    *
    * b .. [v0-v7](e8)
    * x .. [v8-v15](e8)
    */

   while (rowbytes) {
      asm volatile ("vsetvli   %0, %1, e8, m8" : "=r" (vl) : "r" (rowbytes));

      /* b = *row */
      asm volatile (VLE8_V"    v0, (%0)" : : "r" (row));

      /* x = *prev_row */
      asm volatile (VLE8_V"    v8, (%0)" : : "r" (prev_row));
      prev_row += vl;

      /* b = b + x */
      asm volatile ("vadd.vv   v0, v0, v8");

      /* *row = b */
      asm volatile (VSE8_V"    v0, (%0)" : : "r" (row));
      row += vl;

      rowbytes -= vl;
   }
}


static inline void png_read_filter_row_sub_vector(png_row_infop row_info, png_bytep row,
                                  unsigned int bpp)
{
   png_bytep rp_end = row + row_info->rowbytes;

   /*
    * row:      | a | x |
    *
    * a = a + x
    *
    * a .. [v0](e8)
    * x .. [v8](e8)
    */

   asm volatile ("vsetvli      zero, %0, e8, m1" : : "r" (bpp));

   /* a = *row */
   asm volatile (VLE8_V"       v0, (%0)" : : "r" (row));
   row += bpp;

   while (row < rp_end) {

      /* x = *row */
      asm volatile (VLE8_V"       v8, (%0)" : : "r" (row));
      /* a = a + x */
      asm volatile ("vadd.vv      v0, v0, v8");

      /* *row = a */
      asm volatile (VSE8_V"       v0, (%0)" : : "r" (row));
      row += bpp;
   }
}

void png_read_filter_row_sub3_vector(png_row_infop row_info, png_bytep row,
                                  png_const_bytep prev_row)
{
   png_read_filter_row_sub_vector(row_info, row, 3);
}

void png_read_filter_row_sub4_vector(png_row_infop row_info, png_bytep row,
                                  png_const_bytep prev_row)
{
   png_read_filter_row_sub_vector(row_info, row, 4);
}


static inline void png_read_filter_row_avg_vector(png_row_infop row_info, png_bytep row,
                                  png_const_bytep prev_row, unsigned int bpp)
{
   png_bytep rp_end = row + row_info->rowbytes;

   /*
    * row:      | a | x |
    * prev_row: |   | b |
    *
    * a ..   [v2](e8)
    * b ..   [v4](e8)
    * x ..   [v8](e8)
    * tmp .. [v12-v13](e16)
    */

   /* first pixel */

   asm volatile ("vsetvli      zero, %0, e8, m1" : : "r" (bpp));

   /* b = *prev_row */
   asm volatile (VLE8_V"       v4, (%0)" : : "r" (prev_row));
   prev_row += bpp;

   /* x = *row */
   asm volatile (VLE8_V"       v8, (%0)" : : "r" (row));

   /* b = b / 2 */
   asm volatile ("vsrl.vi      v4, v4, 1");
   /* a = x + b */
   asm volatile ("vadd.vv      v2, v4, v8");

   /* *row = a */
   asm volatile (VSE8_V"       v2, (%0)" : : "r" (row));
   row += bpp;


   /* remaining pixels */

   while (row < rp_end) {

      /* b = *prev_row */
      asm volatile (VLE8_V"       v4, (%0)" : : "r" (prev_row));
      prev_row += bpp;

      /* x = *row */
      asm volatile (VLE8_V"       v8, (%0)" : : "r" (row));

      /* tmp = a + b */
      asm volatile ("vwaddu.vv    v12, v2, v4"); /* add with widening */
      /* a = tmp/2 */
      asm volatile (VNSRL_WI"     v2, v12, 1");  /* divide/shift with narrowing */
      /* a += x */
      asm volatile ("vadd.vv      v2, v2, v8");

      /* *row = a */
      asm volatile (VSE8_V"       v2, (%0)" : : "r" (row));
      row += bpp;
   }
}

void png_read_filter_row_avg3_vector(png_row_infop row_info, png_bytep row,
                                  png_const_bytep prev_row)
{
   png_read_filter_row_avg_vector(row_info, row, prev_row, 3);
}

void png_read_filter_row_avg4_vector(png_row_infop row_info, png_bytep row,
                                  png_const_bytep prev_row)
{
   png_read_filter_row_avg_vector(row_info, row, prev_row, 4);
}


void png_read_filter_row_paeth_vector(png_row_infop row_info, png_bytep row,
                                    png_const_bytep prev_row, unsigned int bpp)
{
   png_bytep rp_end = row + row_info->rowbytes;

   /*
    * row:      | a | x |
    * prev_row: | c | b |
    *
    * mask ..   [v0]
    * a ..      [v2](e8)
    * b ..      [v4](e8)
    * c ..      [v6](e8)
    * x ..      [v8](e8)
    * p ..      [v12-v13](e16)
    * pa ..     [v16-v17](e16)
    * pb ..     [v20-v21](e16)
    * pc ..     [v24-v25](e16)
    * tmpmask ..[v31]
    */


   /* first pixel */

   asm volatile ("vsetvli      zero, %0, e8, m1" : : "r" (bpp));

   /* a = *row + *prev_row */
   asm volatile (VLE8_V"       v2, (%0)" : : "r" (row));
   asm volatile (VLE8_V"       v6, (%0)" : : "r" (prev_row));
   prev_row += bpp;
   asm volatile ("vadd.vv      v2, v2, v6");

   /* *row = a */
   asm volatile (VSE8_V"       v2, (%0)" : : "r" (row));
   row += bpp;


   /* remaining pixels */

   while (row < rp_end) {

      /* b = *prev_row */
      asm volatile (VLE8_V"       v4, (%0)" : : "r" (prev_row));
      prev_row += bpp;

      /* x = *row */
      asm volatile (VLE8_V"       v8, (%0)" : : "r" (row));

      /* sub (widening to 16bit) */
      /* p = b - c */
      asm volatile ("vwsubu.vv    v12, v4, v6");
      /* pc = a - c */
      asm volatile ("vwsubu.vv    v24, v2, v6");

      /* switch to widened */
      asm volatile ("vsetvli      zero, %0, e16, m2" : : "r" (bpp));

      /* pa = abs(p) -> pa = p < 0 ? -p : p */
      asm volatile ("vmv.v.v      v16, v12");             /* pa = p */
      asm volatile ("vmslt.vx     v0, v16, zero");        /* set mask[i] if pa[i] < 0 */
      asm volatile ("vrsub.vx     v16, v16, zero, v0.t"); /* invert negative values in pa; vd[i] = 0 - vs2[i] (if mask[i])
                                                           * could be replaced by vneg in rvv >= 1.0
                                                           */

      /* pb = abs(p) -> pb = pc < 0 ? -pc : pc */
      asm volatile ("vmv.v.v      v20, v24");             /* pb = pc */
      asm volatile ("vmslt.vx     v0, v20, zero");        /* set mask[i] if pc[i] < 0 */
      asm volatile ("vrsub.vx     v20, v20, zero, v0.t"); /* invert negative values in pb; vd[i] = 0 - vs2[i] (if mask[i])
                                                           * could be replaced by vneg in rvv >= 1.0
                                                           */

      /* pc = abs(p + pc) -> pc = (p + pc) < 0 ? -(p + pc) : p + pc */
      asm volatile ("vadd.vv      v24, v24, v12");        /* pc = p + pc */
      asm volatile ("vmslt.vx     v0, v24, zero");        /* set mask[i] if pc[i] < 0 */
      asm volatile ("vrsub.vx     v24, v24, zero, v0.t"); /* invert negative values in pc; vd[i] = 0 - vs2[i] (if mask[i])
                                                           * could be replaced by vneg in rvv >= 1.0
                                                           */

      /*
       * if (pb < pa) {
       *   pa = pb;
       *   a = b;   (see (*1))
       * }
       */
      asm volatile ("vmslt.vv     v0, v20, v16");         /* set mask[i] if pb[i] < pa[i] */
      asm volatile ("vmerge.vvm   v16, v16, v20, v0");    /* pa[i] = pb[i] (if mask[i]) */


      /*
       * if (pc < pa)
       *   a = c;   (see (*2))
       */
      asm volatile ("vmslt.vv     v31, v24, v16");        /* set tmpmask[i] if pc[i] < pa[i] */

      /* switch to narrow */
      asm volatile ("vsetvli      zero, %0, e8, m1" : : "r" (bpp));

      /* (*1) */
      asm volatile ("vmerge.vvm   v2, v2, v4, v0");       /* a = b (if mask[i]) */

      /* (*2) */
      asm volatile ("vmand.mm     v0, v31, v31");         /* mask = tmpmask
                                                           * vmand works for rvv 0.7 up to 1.0
                                                           * could be replaced by vmcpy in 0.7.1/0.8.1
                                                           * or vmmv.m in 1.0
                                                           */
      asm volatile ("vmerge.vvm   v2, v2, v6, v0");       /* a = c (if mask[i]) */

      /* a += x */
      asm volatile ("vadd.vv      v2, v2, v8");

      /* *row = a */
      asm volatile (VSE8_V"       v2, (%0)" : : "r" (row));
      row += bpp;


      /* prepare next iteration (prev_row is already in a) */
      /* c = b */
      asm volatile ("vmv.v.v      v6, v4");
   }
}

void png_read_filter_row_paeth3_vector(png_row_infop row_info, png_bytep row,
                                  png_const_bytep prev_row)
{
   png_read_filter_row_paeth_vector(row_info, row, prev_row, 3);
}

void png_read_filter_row_paeth4_vector(png_row_infop row_info, png_bytep row,
                                  png_const_bytep prev_row)
{
   png_read_filter_row_paeth_vector(row_info, row, prev_row, 4);
}

#endif /* PNG_RISCV_VECTOR_OPT > 0 */
#endif /* PNG_RISCV_VECTOR_IMPLEMENTATION == 1 (intrinsics) */
#endif /* READ */
