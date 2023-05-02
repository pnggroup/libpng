/* palette_neon_intrinsics.c - RISC-V Vector optimized palette expansion
 * functions
 *
 * Copyright (c) 2023 Google LLC
 * Written by Dragoș Tiselice <dtiselice@google.com>, May 2023.
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */

#include "../pngpriv.h"

#if PNG_ARM_NEON_IMPLEMENTATION == 1

#include <riscv_vector.h>

void
png_riffle_palette_vector(png_structrp png_ptr)
{
   png_const_bytep palette = (png_const_bytep)png_ptr->palette;
   png_bytep riffled_palette = png_ptr->riffled_palette;
   png_const_bytep trans_alpha = png_ptr->trans_alpha;

   size_t len = 256;

   vuint8m1_t r;
   vuint8m1_t g;
   vuint8m1_t b;

   for (size_t vl; len > 0; len -= vl, palette += vl * 3, trans_alpha += vl, riffled_palette += vl * 4) {
      vl = __riscv_vsetvl_e8m1(len);

      __riscv_vlseg3e8_v_u8m1(&r, &g, &b, palette, vl);
      vuint8m1_t a = __riscv_vle8_v_u8m1(trans_alpha, vl);

      __riscv_vsseg4e8_v_u8m1(riffled_palette, r, g, b, a, vl);
   }
}

int
png_do_expand_palette_rgba8_vector(png_structrp png_ptr, png_row_infop row_info,
    png_const_bytep row, png_bytepp ssp, png_bytepp ddp)
{
   size_t row_width = (size_t)row_info->width;
   const png_uint_32* palette = (const png_uint_32*)png_ptr->riffled_palette;

   size_t vl = __riscv_vsetvl_e8m1(row_width);
   png_bytep sp = *ssp - vl;
   png_bytep dp = *ddp - vl * 4;

   for (; row_width > 0; row_width -= vl, dp -= vl * 4, sp -= vl) {
      vl = __riscv_vsetvl_e8m1(row_width);

      vuint8m1_t indices = __riscv_vle8_v_u8m1(sp, vl);

      vuint32m4_t pixels = __riscv_vluxei8_v_u32m4(palette, indices, vl);

      __riscv_vse32_v_u32m4((unsigned int *)dp, pixels, vl);
   }

   row_width = (size_t)row_info->width;

   *ssp = *ssp - row_width;
   *ddp = *ddp - row_width * 4;

   return row_width;
}

int
png_do_expand_palette_rgb8_vector(png_structrp png_ptr, png_row_infop row_info,
    png_const_bytep row, png_bytepp ssp, png_bytepp ddp)
{
   size_t row_width = (size_t)row_info->width;
   const png_const_bytep palette = (png_const_bytep)png_ptr->palette;

   size_t vl = __riscv_vsetvl_e8m1(row_width);
   png_bytep sp = *ssp - vl;
   png_bytep dp = *ddp - vl * 3;

   vuint8m1_t r;
   vuint8m1_t g;
   vuint8m1_t b;


   for (; row_width > 0; row_width -= vl, dp -= vl * 3, sp -= vl) {
      vl = __riscv_vsetvl_e8m1(row_width);

      vuint16m2_t indices = __riscv_vwmulu_vx_u16m2(__riscv_vle8_v_u8m1(sp, vl), 3, vl);

      __riscv_vluxseg3ei16_v_u8m1(&r, &g, &b, palette, indices, vl);

      __riscv_vsseg3e8_v_u8m1(dp, r, g, b, vl);
   }

   row_width = (size_t)row_info->width;

   *ssp = *ssp - row_width;
   *ddp = *ddp - row_width * 3;

   return row_width;
}

#endif /* PNG_ARM_NEON_IMPLEMENTATION */
