
#include <stdio.h>

/* Copyright 2013 Glenn Randers-Pehrson */

/* Usage:            
 * pngidat.exe < file.png > file.ihdr
 *
 * file.ihdr is the image datastream, with the PNG signature bytes
 * and all chunks up to the first IDAT.
 */

main()
{
   unsigned int i;
   unsigned int len[5];
   unsigned int name[5];
   unsigned int c;

   len[4]='\0';
   name[4]='\0';

/* Copy 8-byte signature */
   for (i=8; i; i--)
   {
      c=getchar();
      if (c == EOF)
         break;
      putchar(c);
   }

   for (;;)
   {
      /* read length */
      unsigned int length;
      len[0]=getchar();
      if (len[0] == EOF)
         break;
      len[1]=getchar();
      len[2]=getchar();
      len[3]=getchar();
      if (len[3] == EOF)
         break;
      length=len[0]<<24 | len[1]<<16 | len[2] << 8 | len[3];

      /* read chunkname */
      name[0]=getchar();
      name[1]=getchar();
      name[2]=getchar();
      name[3]=getchar();
      if (name[3] == EOF)
         break;

      if (name[0] == 'I' && name[1] == 'D' && name[2] == 'A' && name[3] == 'T')
          break;
   
      /* copy the length bytes */
      putchar(len[0]);
      putchar(len[1]);
      putchar(len[2]);
      putchar(len[3]);
       
      /* copy the name bytes */
      putchar(name[0]);
      putchar(name[1]);
      putchar(name[2]);
      putchar(name[3]);
   
      /* copy the data bytes */
      for (i=length; i; i--)
      {
         c=getchar();
         if (c == EOF)
            break;
         putchar(c);
      }
      if (c == EOF)
         break;

      /* copy crc bytes */
      for (i=4; i; i--)
      {
         c=getchar();
         if (c == EOF)
            break;
         putchar(c);
      }
      if (c == EOF)
         break;
   
      if (name[0] == 'I' && name[1] == 'E' && name[2] == 'N' && name[3] == 'D')
      {
         /* should not happen */
         break;
      }
   
   }
}
