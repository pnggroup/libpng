/* pngzopz2i */

/* Copyright 2013 Glenn Randers-Pehrson
 * Released under the pngcrush license (equivalent to the libpng license)
 */

/* Usage:            
 * pngzop_zlib_to_idat < file.zlib > file_out.idat
 *
 * file.zlib is the zlib datastream from zopli --zlib file_in.idat
 * file_out.idat is the zlib datastream enclosed in PNG IDAT chunks.
 * This is a single file that may contain multiple IDAT chunks.
 *
 * To do: make the maximum IDAT chunk data length an option (currently fixed
 * at 250000 bytes).
 *
 */

#include <stdio.h>

#include <zlib.h> /* for crc32 */

void
put_uLong(uLong val)
{
   putchar(val >> 24);
   putchar(val >> 16);
   putchar(val >>  8);
   putchar(val >>  0);
}

void
put_chunk(const unsigned char *chunk, uInt length)
{
   uLong crc;

   put_uLong(length-4); /* Exclude the tag */
   fwrite(chunk, 1, length, stdout);

   crc = crc32(0, Z_NULL, 0);
   put_uLong(crc32(crc, chunk, length));
}

int
main()
{
   unsigned char buf[250000];
   int c;
   const unsigned char idat[] = { 73, 68, 65, 84 /* "IDAT" */ };
   int n=6;

   /* IDAT */
   buf[0]=idat[0];
   buf[1]=idat[1];
   buf[2]=idat[2];
   buf[3]=idat[3];

   /* CMF */
   c=getchar();
   if (c != EOF)
     buf[4]=(unsigned char) c;

   /* FLAGS */
   if (c != EOF)
   {
     c=getchar();
     if (c != EOF)
       buf[5]=(unsigned char) c;
   }

   if (c != EOF)
   for(;;)
   {
      /* read up to n=250000 bytes */

      c=getchar();

      if (c != EOF)
      {
        buf[n]=(unsigned char) c;
        n++;
      }

      if (c != EOF && n < 250000)
        continue;

      put_chunk(buf, n);

      if (c == EOF)
        break;

      n=4;
   }
}

