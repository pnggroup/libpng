#include <stdio.h>

/* Copyright 2013 Glenn Randers-Pehrson */

/* Usage:            
 * pngidat.exe < file.png | pigz -d > file.idat
 *
 * file.idat is the image datastream, with the filter bytes.
 */

main()
{
   int i;
   int buf[5];
   int c;

   buf[4]='\0';

/* Skip 8-byte signature */
   for (i=8; i; i--)
      c=getchar();

for (;;)
{
   /* read length */
   int length;
   buf[0]=getchar();
   if (buf[0] == EOF)
      break;
   buf[1]=getchar();
   buf[2]=getchar();
   buf[3]=getchar();
   length=buf[0]<<24 | buf[1]<<16 | buf[2] << 8 | buf[3];
   /* read chunkname */
   buf[0]=getchar();
   buf[1]=getchar();
   buf[2]=getchar();
   buf[3]=getchar();
   if (buf[0] == 'I' && buf[1] == 'D' && buf[2] == 'A' && buf[3] == 'T')
   {
      /* copy the data bytes */
      for (i=length; i; i--)
      {
         c=getchar();
         if (c == EOF)
            break;
         putchar(c);
      }
      /* skip crc byte */
      for (i=4; i; i--)
      {
         c=getchar();
      }
   }

   else if (buf[0] == 'I' && buf[1] == 'E' && buf[2] == 'N' && buf[3] == 'D')
     break;

   else
   {
      for (i=length+4; i; i--)
         c=getchar();
   }
}
}
