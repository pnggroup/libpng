/* filter_neon_intrinsics.c - RISC-V Vector optimized filter functions
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

#if PNG_RISCV_VECTOR_IMPLEMENTATION == 1 /* intrinsics code from pngpriv.h */

#include <riscv_vector.h>

void
png_read_filter_row_up_vector(png_row_infop row_info, png_bytep row,
    png_const_bytep prev_row)
{
   size_t len = row_info->rowbytes;

   for (size_t vl; len > 0; len -= vl, row += vl, prev_row += vl) {
      vl = __riscv_vsetvl_e8m8(len);

      vuint8m8_t prev_vals = __riscv_vle8_v_u8m8(prev_row, vl);
      vuint8m8_t row_vals = __riscv_vle8_v_u8m8(row, vl);

      row_vals = __riscv_vadd_vv_u8m8(row_vals, prev_vals, vl);

      __riscv_vse8_v_u8m8(row, row_vals, vl);
   }
}

static inline void
png_read_filter_row_sub_vector_128(size_t len, size_t vl, unsigned char* row)
{
   vuint8m1_t sum;
   vuint8m1_t chunk = __riscv_vmv_v_x_u8m1(0, vl);

   for (; len > 0; len -= vl, row += vl) {
      __riscv_vsetvl_e8m1(vl);

      sum = chunk;
      chunk = __riscv_vle8_v_u8m1(row, vl);

      chunk = __riscv_vadd_vv_u8m1(chunk, sum, vl);

      __riscv_vse8_v_u8m1(row, chunk, vl);
   }
}

void
png_read_filter_row_sub3_vector_128(png_row_infop row_info, png_bytep row,
    png_const_bytep prev_row)
{
   size_t len = row_info->rowbytes;

   png_read_filter_row_sub_vector_128(len, 3, row);

   PNG_UNUSED(prev_row)
}

void
png_read_filter_row_sub4_vector_128(png_row_infop row_info, png_bytep row,
    png_const_bytep prev_row)
{
   size_t len = row_info->rowbytes;

   png_read_filter_row_sub_vector_128(len, 4, row);

   PNG_UNUSED(prev_row)
}

static inline void
png_read_filter_row_avg_vector(size_t len, size_t vl, unsigned char* row,
    const unsigned char* prev_row)
{
   vuint8m1_t avg;
   vuint8m1_t chunk = __riscv_vmv_v_x_u8m1(0, vl);

   for (; len > 0; len -= vl, row += vl) {
      __riscv_vsetvl_e8m1(vl);

      vuint8m1_t prev_chunk = __riscv_vle8_v_u8m1(prev_row, vl);
      avg = chunk;
      chunk = __riscv_vle8_v_u8m1(row, vl);

      vuint8m1_t sum = __riscv_vadd_vv_u8m1(chunk, prev_chunk, vl);
      vuint8m1_t avg = __riscv_vsrl_vx_u8m1(sum, 1, vl);

      chunk = __riscv_vadd_vv_u8m1(chunk, avg, vl);

      __riscv_vse8_v_u8m1(row, chunk, vl);
   }
}

void
png_read_filter_row_avg3_vector(png_row_infop row_info, png_bytep row,
    png_const_bytep prev_row)
{
   size_t len = row_info->rowbytes;

   png_read_filter_row_avg_vector(len, 3, row, prev_row);

   PNG_UNUSED(prev_row)
}

void
png_read_filter_row_avg4_vector(png_row_infop row_info, png_bytep row,
    png_const_bytep prev_row)
{
   size_t len = row_info->rowbytes;

   png_read_filter_row_avg_vector(len, 4, row, prev_row);

   PNG_UNUSED(prev_row)
}

#define MIN_CHUNK_LEN 256
#define MAX_CHUNK_LEN 2048

static inline vuint8m1_t
prefix_sum(vuint8m1_t chunk, unsigned char* carry, size_t vl,
    size_t max_chunk_len)
{
   size_t r;

   for (r = 1; r < MIN_CHUNK_LEN; r <<= 1) {
      vbool8_t shift_mask = __riscv_vmsgeu_vx_u8m1_b8(__riscv_vid_v_u8m1(vl), r, vl);
      chunk = __riscv_vadd_vv_u8m1_mu(shift_mask, chunk, chunk, __riscv_vslideup_vx_u8m1(__riscv_vundefined_u8m1(), chunk, r, vl), vl);
   }

   for (r = MIN_CHUNK_LEN; r < MAX_CHUNK_LEN && r < max_chunk_len; r <<= 1) {
      vbool8_t shift_mask = __riscv_vmsgeu_vx_u8m1_b8(__riscv_vid_v_u8m1(vl), r, vl);
      chunk = __riscv_vadd_vv_u8m1_mu(shift_mask, chunk, chunk, __riscv_vslideup_vx_u8m1(__riscv_vundefined_u8m1(), chunk, r, vl), vl);
   }

   chunk = __riscv_vadd_vx_u8m1(chunk, *carry, vl);
   *carry = __riscv_vmv_x_s_u8m1_u8(__riscv_vslidedown_vx_u8m1(chunk, vl - 1, vl));

   return chunk;
}

void
png_read_filter_row_sub3_vector_256(png_row_infop row_info, png_bytep row,
    png_const_bytep prev_row)
{
   size_t len = row_info->rowbytes;
   const size_t max_chunk_len = __riscv_vsetvlmax_e8m1();

   vuint8m1_t r;
   vuint8m1_t g;
   vuint8m1_t b;

   unsigned char r_carry = 0;
   unsigned char g_carry = 0;
   unsigned char b_carry = 0;

   for (size_t vl; len > 0; len -= vl * 3, row += vl * 3) {
      vl = __riscv_vsetvl_e8m1(len / 3);

      __riscv_vlseg3e8_v_u8m1(&r, &g, &b, row, vl);

      r = prefix_sum(r, &r_carry, vl, max_chunk_len);
      g = prefix_sum(g, &g_carry, vl, max_chunk_len);
      b = prefix_sum(b, &b_carry, vl, max_chunk_len);

      __riscv_vsseg3e8_v_u8m1(row, r, g, b, vl);
   }
}

void
png_read_filter_row_sub4_vector_256(png_row_infop row_info, png_bytep row,
    png_const_bytep prev_row)
{
   size_t len = row_info->rowbytes;
   const size_t max_chunk_len = __riscv_vsetvlmax_e8m1();

   vuint8m1_t r;
   vuint8m1_t g;
   vuint8m1_t b;
   vuint8m1_t a;

   unsigned char r_carry = 0;
   unsigned char g_carry = 0;
   unsigned char b_carry = 0;
   unsigned char a_carry = 0;

   for (size_t vl; len > 0; len -= vl * 4, row += vl * 4) {
      vl = __riscv_vsetvl_e8m1(len / 4);

      __riscv_vlseg4e8_v_u8m1(&r, &g, &b, &a, row, vl);

      r = prefix_sum(r, &r_carry, vl, max_chunk_len);
      g = prefix_sum(g, &g_carry, vl, max_chunk_len);
      b = prefix_sum(b, &b_carry, vl, max_chunk_len);
      a = prefix_sum(a, &a_carry, vl, max_chunk_len);

      __riscv_vsseg4e8_v_u8m1(row, r, g, b, a, vl);
   }
}

static inline vint16m1_t
abs_diff(vuint16m1_t a, vuint16m1_t b, size_t vl)
{
   vint16m1_t diff = __riscv_vreinterpret_v_u16m1_i16m1(__riscv_vsub_vv_u16m1(a, b, vl));
   vint16m1_t neg = __riscv_vneg_v_i16m1(diff, vl);

   return __riscv_vmax_vv_i16m1(diff, neg, vl);
}

static inline vint16m1_t
abs_sum(vint16m1_t a, vint16m1_t b, size_t vl)
{
   vint16m1_t sum = __riscv_vadd_vv_i16m1(a, b, vl);
   vint16m1_t neg = __riscv_vneg_v_i16m1(sum, vl);

   return __riscv_vmax_vv_i16m1(sum, neg, vl);
}

static inline void
png_read_filter_row_paeth_vector(size_t len, size_t vl, unsigned char* row,
    const unsigned char* prev)
{
   vuint16m1_t a;
   vuint16m1_t b = __riscv_vmv_v_x_u16m1(0, vl);
   vuint16m1_t c;
   vuint16m1_t d = __riscv_vmv_v_x_u16m1(0, vl);

   for (; len > 0; len -= vl, row += vl, prev += vl) {
      __riscv_vsetvl_e16m1(vl);

      c = b;
      b = __riscv_vzext_vf2_u16m1(__riscv_vle8_v_u8mf2(prev, vl), vl);
      a = d;
      d = __riscv_vzext_vf2_u16m1(__riscv_vle8_v_u8mf2(row, vl), vl);

      vint16m1_t pa = abs_diff(b, c, vl);
      vint16m1_t pb = abs_diff(a, c, vl);
      vint16m1_t pc = abs_sum(pa, pb, vl);

      vint16m1_t smallest = __riscv_vmin_vv_i16m1(pa, __riscv_vmin_vv_i16m1(pb, pc, vl), vl);

      vuint16m1_t nearest = c;
      nearest = __riscv_vmerge_vvm_u16m1(nearest, a, __riscv_vmseq_vv_i16m1_b16(smallest, pa, vl), vl);
      nearest = __riscv_vmerge_vvm_u16m1(nearest, b, __riscv_vmseq_vv_i16m1_b16(smallest, pb, vl), vl);

      d = __riscv_vadd_vv_u16m1(d, nearest, vl);

      __riscv_vse8_v_u8mf2(row, __riscv_vnsrl_wx_u8mf2(d, 0, vl), vl);
   }
}

void
png_read_filter_row_paeth3_vector(png_row_infop row_info, png_bytep row,
    png_const_bytep prev_row)
{
   size_t len = row_info->rowbytes;

   png_read_filter_row_paeth_vector(len, 3, row, prev_row);

   PNG_UNUSED(prev_row)
}

void
png_read_filter_row_paeth4_vector(png_row_infop row_info, png_bytep row,
    png_const_bytep prev_row)
{
   size_t len = row_info->rowbytes;

   png_read_filter_row_paeth_vector(len, 4, row, prev_row);

   PNG_UNUSED(prev_row)
}

#endif /* PNG_RISCV_VECTOR_IMPLEMENTATION */
#endif /* READ */
