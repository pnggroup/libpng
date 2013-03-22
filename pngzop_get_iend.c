
#include <stdio.h>

/* Copyright 2013 Glenn Randers-Pehrson */

/* Usage:            
 * pngiend.exe < file.png > file.iend
 *
 * file.iend is the image datastream, minus everything through the
 * last IDAT chunk.
 */

main()
{
   unsigned int c;
   unsigned int i;
   unsigned int len[5];
   unsigned int length;
   unsigned int mode;
   unsigned int name[5];

   len[4]='\0';
   name[4]='\0';
   mode=0; /* Looking for IDAT */

/* Skip 8-byte signature */
   for (i=8; i; i--)
   {
      c=getchar();
      if (c == EOF)
         break;
   }

   for (;;)
   {
      /* read length */
      len[0]=getchar();
      if (len[0] == EOF)
         break;
      len[1]=getchar();
      len[2]=getchar();
      len[3]=getchar();
      if (len[3] == EOF)
         break;
      length=len[0]<<24 | len[1]<<16 | len[2]<< 8 | len[3];

      /* read chunkname */
      name[0]=getchar();
      name[1]=getchar();
      name[2]=getchar();
      name[3]=getchar();
      if (name[3] == EOF)
         break;

      if (name[0] == 'I' && name[1] == 'D' && name[2] == 'A' && \
          name[3] == 'T')
         {
            if (mode == 0)
               mode = 1; /* found IDAT */
         }
      else
         {
            if (mode == 1)
               mode = 2; /* found chunk after IDAT */
         }
   
      if (mode == 2)
         {
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
         }
         
      /* skip or copy the data bytes */
      for (i=length; i; i--)
      {
         c=getchar();
         if (c == EOF)
            break;
         if (mode == 2)
            putchar(c);
      }
      if (c == EOF)
         break;

      /* skip or copy crc bytes */
      for (i=4; i; i--)
      {
         c=getchar();
         if (c == EOF)
            break;
         if (mode == 2)
            putchar(c);
      }
      if (c == EOF)
         break;
   
      if (name[0] == 'I' && name[1] == 'E' && name[2] == 'N' && \
          name[3] == 'D')
         {
            break;
         }
   }
}
