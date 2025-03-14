/* palette_neon_intrinsics.c - RISC-V Vector optimized palette expansion
 * functions
 *
 * Copyright (c) 2023 Google LLC
 * Written by Dragoș Tiselice <dtiselice@google.com>, May 2023.
 *            Filip Wasil     <f.wasil@samsung.com>, March 2025.
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */

#include "../pngpriv.h"

#if PNG_RISCV_RVV_IMPLEMENTATION == 1

#include <riscv_vector.h>

void
png_riffle_palette_rvv(png_structrp png_ptr)
{
   png_const_bytep palette = (png_const_bytep)png_ptr->palette;
   png_bytep riffled_palette = png_ptr->riffled_palette;
   png_const_bytep trans_alpha = png_ptr->trans_alpha;

   size_t len = 256;
   const int bpp_in = 3;
   const int bpp_out = 4;
   size_t len_pal = bpp_in * len;
   size_t len_rpal = bpp_out * len;

   vuint8m1x3_t rgb;
   vuint8m1_t a;

   for (size_t vl; len > 0; len -= vl, palette += vl * bpp_in, trans_alpha += vl, riffled_palette += vl * bpp_out) {
      vl = __riscv_vsetvl_e8m1 (len);

      a = __riscv_vle8_v_u8m1(trans_alpha, vl);
      rgb = __riscv_vlseg3e8_v_u8m1x3(palette, vl);

      __riscv_vsseg4e8_v_u8m1x4(
         riffled_palette,
         __riscv_vcreate_v_u8m1x4(
            __riscv_vget_v_u8m1x3_u8m1(rgb, 0),
            __riscv_vget_v_u8m1x3_u8m1(rgb, 1),
            __riscv_vget_v_u8m1x3_u8m1(rgb, 2),
            a),
      vl);
   }
}

int
png_do_expand_palette_rgba8_rvv(png_structrp png_ptr, png_row_infop row_info,
    png_const_bytep row, png_bytepp ssp, png_bytepp ddp)
{
   size_t len = (size_t)row_info->width;
   size_t vl = __riscv_vsetvl_e8m1(len);
   const size_t bpp = 4;
   const png_const_bytep palette = (png_const_bytep)png_ptr->palette;

   png_bytep sp = *ssp - vl;
   png_bytep dp = *ddp - vl * bpp;

   vuint8m1x4_t rgba;

   for (; len > 0; len -= vl, dp -= vl * bpp, sp -= vl) {
      vl = __riscv_vsetvl_e8m1(len);
      vuint16m2_t indices = __riscv_vwmulu_vx_u16m2(__riscv_vle8_v_u8m1(sp, vl), bpp, vl);
      rgba = __riscv_vluxseg4ei16_v_u8m1x4(palette, indices, vl);
      __riscv_vsseg4e8_v_u8m1x4(dp, rgba, vl);
   }

   *ssp = *ssp - (size_t)row_info->width;
   *ddp = *ddp - (size_t)row_info->width * bpp;

   return (size_t)row_info->width;
}

int
png_do_expand_palette_rgb8_rvv(png_structrp png_ptr, png_row_infop row_info,
    png_const_bytep row, png_bytepp ssp, png_bytepp ddp)
{
   size_t len = (size_t)row_info->width;
   size_t vl = __riscv_vsetvl_e8m1(len);
   const size_t bpp = 3;
   const png_const_bytep palette = (png_const_bytep)png_ptr->palette;

   png_bytep sp = *ssp - vl;
   png_bytep dp = *ddp - vl * bpp;

   vuint8m1x3_t rgb;

   for (; len > 0; len -= vl, dp -= vl * bpp, sp -= vl) {
      vl = __riscv_vsetvl_e8m1(len);
      vuint16m2_t indices = __riscv_vwmulu_vx_u16m2(__riscv_vle8_v_u8m1(sp, vl), bpp, vl);
      rgb = __riscv_vluxseg3ei16_v_u8m1x3(palette, indices, vl);
      __riscv_vsseg3e8_v_u8m1x3(dp, rgb, vl);
   }

   *ssp = *ssp - (size_t)row_info->width;
   *ddp = *ddp - (size_t)row_info->width * bpp;

   return (size_t)row_info->width;
}

#endif /* PNG_RISCV_RVV_IMPLEMENTATION */
