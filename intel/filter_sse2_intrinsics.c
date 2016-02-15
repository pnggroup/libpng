
/* filter_sse2_intrinsics.c - SSE2 optimized filter functions
 *
 * Copyright (c) 2016 Google, Inc.
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */

#include "../pngpriv.h"

#ifdef PNG_READ_SUPPORTED

#if PNG_INTEL_SSE2_OPT > 0

#if PNG_INTEL_SSE2_OPT == 1
#include <emmintrin.h>
#elif PNG_INTEL_SSE2_OPT == 2
#include <tmmintrin.h>
#endif

// Functions in this file look at most 3 pixels (a,b,c) to predict the 4th (d).
// They're positioned like this:
//    prev:  c b
//    row:   a d
// The Sub filter predicts d=a, Avg d=(a+b)/2, and Paeth predicts d to be
// whichever of a, b, or c is closest to p=a+b-c.
// Up also exists, predicting d=b.  But there is not need to optimize Up
// because the compiler will vectorize it for us.

void png_read_filter_row_sub3_sse2(png_row_infop row_info, png_bytep row,
   png_const_bytep prev)
{
   // The Sub filter predicts each pixel as the previous pixel, a.
   // There is no pixel to the left of the first pixel.  It's encoded directly.
   // That works with our main loop if we just say that left pixel was zero.
   __m128i a, d = _mm_setzero_si128();

   int rb = row_info->rowbytes;
   while (rb > 0) {
      a = d; memcpy(&d, row, 3);
      d = _mm_add_epi8(d, a);
      memcpy(row, &d, 3);

      row += 3;
      rb  -= 3;
    }
}

void png_read_filter_row_sub4_sse2(png_row_infop row_info, png_bytep row,
   png_const_bytep prev)
{
   // The Sub filter predicts each pixel as the previous pixel, a.
   // There is no pixel to the left of the first pixel.  It's encoded directly.
   // That works with our main loop if we just say that left pixel was zero.
   __m128i a, d = _mm_setzero_si128();

   int rb = row_info->rowbytes;
   while (rb > 0) {
      a = d; memcpy(&d, row, 4);
      d = _mm_add_epi8(d, a);
      memcpy(row, &d, 4);

      row += 4;
      rb  -= 4;
    }
}

void png_read_filter_row_avg3_sse2(png_row_infop row_info, png_bytep row,
   png_const_bytep prev)
{
   // The Avg filter predicts each pixel as the (truncated) average of a and b.
   // There's no pixel to the left of the first pixel.  Luckily, it's
   // predicted to be half of the pixel above it.  So again, this works
   // perfectly with our loop if we make sure a starts at zero.
   const __m128i zero = _mm_setzero_si128();
   __m128i    b;
   __m128i a, d = zero;

   int rb = row_info->rowbytes;
   while (rb > 0) {
       memcpy(&b, prev, 3);
       a = d; memcpy(&d, row, 3);

       // PNG requires a truncating average here, so sadly we can't just use
       // _mm_avg_epu8...
       __m128i avg = _mm_avg_epu8(a,b);
       // ...but we can fix it up by subtracting off 1 if it rounded up.
       avg = _mm_sub_epi8(avg, _mm_and_si128(_mm_xor_si128(a,b),
          _mm_set1_epi8(1)));

       d = _mm_add_epi8(d, avg);
       memcpy(row, &d, 3);

       prev += 3;
       row  += 3;
       rb   -= 3;
    }
}
void png_read_filter_row_avg4_sse2(png_row_infop row_info, png_bytep row,
   png_const_bytep prev)
{
   // The Avg filter predicts each pixel as the (truncated) average of a and b.
   // There's no pixel to the left of the first pixel.  Luckily, it's
   // predicted to be half of the pixel above it.  So again, this works
   // perfectly with our loop if we make sure a starts at zero.
   const __m128i zero = _mm_setzero_si128();
   __m128i    b;
   __m128i a, d = zero;

   int rb = row_info->rowbytes;
   while (rb > 0) {
       memcpy(&b, prev, 4);
       a = d; memcpy(&d, row, 4);

       // PNG requires a truncating average here, so sadly we can't just use
       // _mm_avg_epu8...
       __m128i avg = _mm_avg_epu8(a,b);
       // ...but we can fix it up by subtracting off 1 if it rounded up.
       avg = _mm_sub_epi8(avg, _mm_and_si128(_mm_xor_si128(a,b),
          _mm_set1_epi8(1)));

       d = _mm_add_epi8(d, avg);
       memcpy(row, &d, 4);

       prev += 4;
       row  += 4;
       rb   -= 4;
    }
}

// Returns |x| for 16-bit lanes.
static __m128i abs_i16(__m128i x) {
#if PNG_INTEL_SSE2_OPT >= 2
   return _mm_abs_epi16(x);
#else
   // Read this all as, return x<0 ? -x : x.
   // To negate two's complement, you flip all the bits then add 1.
   __m128i is_negative = _mm_cmplt_epi16(x, _mm_setzero_si128());
   // Flip negative lanes.
   x = _mm_xor_si128(x, is_negative);
   // +1 to negative lanes, else +0.
   x = _mm_add_epi16(x, _mm_srli_epi16(is_negative, 15));
   return x;
#endif
}

// Bytewise c ? t : e.
static __m128i if_then_else(__m128i c, __m128i t, __m128i e) {
   return _mm_or_si128(_mm_and_si128(c, t), _mm_andnot_si128(c, e));
}

void png_read_filter_row_paeth3_sse2(png_row_infop row_info, png_bytep row,
   png_const_bytep prev)
{
   // Paeth tries to predict pixel d using the pixel to the left of it, a,
   // and two pixels from the previous row, b and c:
   //   prev: c b
   //   row:  a d
   // The Paeth function predicts d to be whichever of a, b, or c is nearest to
   // p=a+b-c.  The first pixel has no left context, and so uses an Up filter,
   // p = b.  This works naturally with our main loop's p = a+b-c if we force a
   // and c to zero.  Here we zero b and d, which become c and a respectively
   // at the start of the loop.
   const __m128i zero = _mm_setzero_si128();
   __m128i c, b = zero,
           a, d = zero;

   int rb = row_info->rowbytes;
   while (rb > 0) {
      // It's easiest to do this math (particularly, deal with pc) with 16-bit
      // intermediates.
      memcpy(&b, prev, 3);
      memcpy(&d, row, 3);
      c = b; b = _mm_unpacklo_epi8(b, zero);
      a = d; d = _mm_unpacklo_epi8(d, zero);
      __m128i pa = _mm_sub_epi16(b,c),
              // (p-a) == (a+b-c - a) == (b-c)
              pb = _mm_sub_epi16(a,c),
              // (p-b) == (a+b-c - b) == (a-c)
              pc = _mm_add_epi16(pa,pb);
              // (p-c) == (a+b-c - c) == (a+b-c-c) == (b-c)+(a-c)

      pa = abs_i16(pa);  // |p-a|
      pb = abs_i16(pb);  // |p-b|
      pc = abs_i16(pc);  // |p-c|

      __m128i smallest = _mm_min_epi16(pc, _mm_min_epi16(pa, pb));

      // Paeth breaks ties favoring a over b over c.
      __m128i nearest  = if_then_else(_mm_cmpeq_epi16(smallest, pa), a,
                         if_then_else(_mm_cmpeq_epi16(smallest, pb), b,
                                                                     c));


      // Note `_epi8`: we need addition to wrap modulo 255.
      d = _mm_add_epi8(d, nearest);
      __m128i r = _mm_packus_epi16(d,d);
      memcpy(row, &r, 3);
      prev += 3;
      row  += 3;
      rb   -= 3;
   }
}

void png_read_filter_row_paeth4_sse2(png_row_infop row_info, png_bytep row,
   png_const_bytep prev)
{
   // Paeth tries to predict pixel d using the pixel to the left of it, a,
   // and two pixels from the previous row, b and c:
   //   prev: c b
   //   row:  a d
   // The Paeth function predicts d to be whichever of a, b, or c is nearest to
   // p=a+b-c.  The first pixel has no left context, and so uses an Up filter,
   // p = b.  This works naturally with our main loop's p = a+b-c if we force a
   // and c to zero.  Here we zero b and d, which become c and a respectively
   // at the start of the loop.
   const __m128i zero = _mm_setzero_si128();
   __m128i c, b = zero,
           a, d = zero;

   int rb = row_info->rowbytes;
   while (rb > 0) {
      // It's easiest to do this math (particularly, deal with pc) with 16-bit
      // intermediates.
      memcpy(&b, prev, 4);
      memcpy(&d, row, 4);
      c = b; b = _mm_unpacklo_epi8(b, zero);
      a = d; d = _mm_unpacklo_epi8(d, zero);
      __m128i pa = _mm_sub_epi16(b,c),
              // (p-a) == (a+b-c - a) == (b-c)
              pb = _mm_sub_epi16(a,c),
              // (p-b) == (a+b-c - b) == (a-c)
              pc = _mm_add_epi16(pa,pb);
              // (p-c) == (a+b-c - c) == (a+b-c-c) == (b-c)+(a-c)

      pa = abs_i16(pa);  // |p-a|
      pb = abs_i16(pb);  // |p-b|
      pc = abs_i16(pc);  // |p-c|

      __m128i smallest = _mm_min_epi16(pc, _mm_min_epi16(pa, pb));

      // Paeth breaks ties favoring a over b over c.
      __m128i nearest  = if_then_else(_mm_cmpeq_epi16(smallest, pa), a,
                         if_then_else(_mm_cmpeq_epi16(smallest, pb), b,
                                                                     c));


      // Note `_epi8`: we need addition to wrap modulo 255.
      d = _mm_add_epi8(d, nearest);
      __m128i r = _mm_packus_epi16(d,d);
      memcpy(row, &r, 4);
      prev += 4;
      row  += 4;
      rb   -= 4;
   }
}

#endif /* PNG_INTEL_SSE2_OPT > 0 */
#endif /* READ */
