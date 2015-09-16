#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#include "pnglibconf.h.prebuilt"
#include "../pngpriv.h"

static const struct
{
   png_uint_32    name;
}
png_known_chunks[] =
/* See pngchunk.h for how this works: */
#define PNG_CHUNK_END(n, c1, c2, c3, c4, before, after)\
   { png_ ##n }
#define PNG_CHUNK(n, c1, c2, c3, c4, before, after)\
   PNG_CHUNK_END(n, c1, c2, c3, c4, before, after),
#define PNG_CHUNK_BEGIN(n, c1, c2, c3, c4, before, after)\
   PNG_CHUNK_END(n, c1, c2, c3, c4, before, after),
{
#  include "../pngchunk.h"
};

#define C_KNOWN ((sizeof png_known_chunks)/(sizeof png_known_chunks[0]))

static unsigned int
index_of(png_uint_32 name)
{
   unsigned int i;

   if (name == 0)
      return 0;

   for (i=0; i<C_KNOWN; ++i) if (png_known_chunks[i].name == name) return i;

   assert("not reached" && 0);
}

static unsigned int bitorder[32];

#define PNG_CHUNK_HASH(n,shift,s1,s2,s3,s4,s5)\
   (0x3f & (((n += n >> shift),n += n >> s1),n += n >> s2))

inline unsigned int hash(png_uint_32 name, unsigned int shift, unsigned int s1,
   unsigned int s2, unsigned int s3, unsigned int s4, unsigned int s5)
{
   /* Running the search gives (shift,s1,s2) (2,8,16) */
   //return PNG_CHUNK_HASH(name, shift, s1, s2, s3, s4, s5);
   name += name >> shift;
   name += name >> s1;
   name += name >> s2;
   //name += name >> s3;
   return 0x3f & name;
   /*return 0x3f & ((name) + (
            ((name >>  bitorder[s1])    & 0x01) +
            ((name >> (bitorder[s2]-1)) & 0x02) +
            ((name >> (bitorder[s3]-2)) & 0x04) +
            ((name >> (bitorder[s4]-3)) & 0x08) +
            ((name >> (bitorder[s5]-4)) & 0x10)));*/
}

int main(void) {
   unsigned int s1 = 0, s2 = 0, s3 = 0, s4 = 0, s5 = 0;
   unsigned int shift = 0;
   png_uint_32 mask;
   unsigned int bitcount;
   unsigned int mineq;
   png_uint_32 sarray;
   unsigned int shift_save;
   png_uint_32 reverse_index_save[64];

   assert(C_KNOWN <= 64);

   /* Check IDAT: */
   assert(index_of(png_IDAT) == 0);

   /* Build a mask of all the bits that differ in at least one of the known
    * names.
    */
   {
      png_uint_32 set, unset;
      int i;

      for (i=0, set=unset=0; i<C_KNOWN; ++i)
      {
         set |= png_known_chunks[i].name;
         unset |= ~png_known_chunks[i].name;
      }

      mask = set ^ ~unset;
   }

   //printf("C_KNOWN = %lu, 0x%.8x\n", C_KNOWN, mask);

   assert(mask == 0x3f1f1f3f);

   /* Print the bit array */
   {
      unsigned int i;
      unsigned int ones[32];

      memset(ones, 0, sizeof ones);

      for (i=0; i<C_KNOWN; ++i)
      {
         png_uint_32 name = png_known_chunks[i].name;
         int j, k;
         char s[5], b[33];

         PNG_CSTRING_FROM_CHUNK(s, name);
         for (j=k=0; j<32; ++j)
         {
            if ((name >> j) & 1)
               ++ones[j];

            if ((mask >> (31-j)) & 1)
               b[k++] = ((name >> (31-j)) & 1) ? 'o' : ' ';
         }

         b[k] = 0;

         //printf("%s: %s\n", s, b);
      }

      memset(bitorder, 0, sizeof bitorder);
      bitcount = 0;

      for (i=0; i<C_KNOWN; ++i) if (((C_KNOWN-i) & 1) == 0)
      {
         unsigned int lo = (C_KNOWN - i)>>1;
         unsigned int hi = (C_KNOWN + i)>>1;
         int j;

         for (j=0; j<32; ++j) if (ones[j] == lo || ones[j] == hi)
         {
            //printf(" %2d,", j);
            bitorder[bitcount++] = j;
         }
      }

      //printf("\nbitcount=%u, C_KNOWN=%lu\n", bitcount, C_KNOWN);
   }

   /* s? are masks to exclude bits from the hash, one for each byte: */
   mineq = C_KNOWN;
   sarray = 0;
   for (shift=0; shift<32; ++shift)
   for (s1=0; s1<32; ++s1)
   for (s2=s1+1; s2<32; ++s2)
   //for (s3=s2+1; s3<32; ++s3)
   //for (s4=s3+1; s4<bitcount; ++s4)
   //for (s5=s4+1; s5<bitcount; ++s5)
   {
      int i, eq;
      png_uint_32 reverse_index[64];

      memset(reverse_index, 0, sizeof reverse_index);

      for (i=eq=0; i<C_KNOWN; ++i)
      {
         png_uint_32 name = png_known_chunks[i].name;
         unsigned int h = hash(name, shift, s1, s2, s3, s4, s5);

         if (reverse_index[h] == 0)
            reverse_index[h] = name;

         else
            ++eq;
      }

      if (eq == 0)
      {
         /* Print the LUT: */
         printf("static const png_byte png_chunk_lut[64] =\n{   \n   ");
         for (i=0; i<63; ++i)
         {
            printf("%2u, ", index_of(reverse_index[i]));
            if ((i+1 & 0xf) == 0) printf("\n   ");
         }
         printf("%2u\n};\n\n", index_of(reverse_index[63]));

         //printf("hash: %u, %u, %u, %u, %u, %u\n", shift, s1, s2, s3, s4, s5);
         printf("#define PNG_CHUNK_HASH(n)\\\n   png_chunk_lut[0x3f &"
                " (((n += n >> %u),n += n >> %u),n += n >> %u)]\n\n",
                shift, s1, s2);
         printf("static png_byte\n"
                "png_chunk_index(png_uint_32 name)\n"
                "{\n"
                "   name += name >> %u;\n"
                "   name += name >> %u;\n"
                "   name += name >> %u;\n"
                "   return png_chunk_lut[name & 0x3f];\n"
                "}\n", shift, s1, s2);

         return 0;
      }

      if (eq < mineq)
      {
         mineq = eq;
         sarray = s1 + bitcount * (s2 + bitcount * (s3 + bitcount *
                  (s4 + bitcount *s5)));
         memcpy(reverse_index_save, reverse_index, sizeof reverse_index_save);
         shift_save = shift;
      }
   }

   s1 = sarray % bitcount;
   s2 = (sarray / bitcount) % bitcount;
   s3 = (sarray / bitcount / bitcount) % bitcount;
   s4 = (sarray / bitcount / bitcount / bitcount) % bitcount;
   s5 = (sarray / bitcount / bitcount / bitcount / bitcount) % bitcount;

   printf("best: %u clashes with bits: %u, %u, %u, %u, %u, %u\n",
      mineq, shift_save, s1, s2, s3, s4, s5);

   {
      int i;
      png_uint_32 reverse_index[64];

      memset(reverse_index, 0, sizeof reverse_index);

      for (i=0; i<C_KNOWN; ++i)
      {
         png_uint_32 name = png_known_chunks[i].name;
         unsigned int h = hash(name, shift_save, s1, s2, s3, s4, s5);

         if (reverse_index[h] == 0)
            reverse_index[h] = name;

         else
         {
            char n1[5], n2[5];

            PNG_CSTRING_FROM_CHUNK(n1, reverse_index[h]);
            PNG_CSTRING_FROM_CHUNK(n2, name);
            printf("%d <- %s and %s\n", h, n1, n2);
         }
      }

   }

   return 1;
}
