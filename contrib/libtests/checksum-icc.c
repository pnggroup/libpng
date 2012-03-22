/* checksum-icc.c
 *
 * Copyright (c) 2012 John Cunningham Bowler
 *
 * Last changed in libpng 1.6.0 [(PENDING RELEASE)]
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * Generate crc32 and adler32 checksums of the given input files, used to
 * generate check-codes for use when matching ICC profiles within libpng.
 */
#include <stdio.h>

#include <zlib.h>

static int
read_one_file(FILE *ip, const char *name)
{
   uLong length = 0;
   uLong a32 = adler32(0, NULL, 0);
   uLong c32 = crc32(0, NULL, 0);

   for (;;)
   {
      int ch = getc(ip);
      Byte b;

      if (ch == EOF) break;

      b = (Byte)ch;

      ++length;
      a32 = adler32(a32, &b, 1);
      c32 = crc32(c32, &b, 1);
   }

   if (ferror(ip))
      return 0;

   /* Success */
   printf("{ 0x%8.8lx, 0x%8.8lx, %lu, \"%s\" },\n", (unsigned long)a32,
      (unsigned long)c32, (unsigned long)length, name);

   return 1;
}

int main(int argc, char **argv)
{
   int err = 0;

   printf("= { /* adler32   crc32   length  name */\n");

   if (argc > 1)
   {
      int i;

      for (i=1; i<argc; ++i)
      {
         FILE *ip = fopen(argv[i], "rb");

         if (ip == NULL || !read_one_file(ip, argv[i]))
         {
            err = 1;
            perror(argv[i]);
            fprintf(stderr, "%s: read error\n", argv[i]);
            printf(" { 0, 0, 0, /* ERROR: */, \"%s\" },\n", argv[i]);
         }

         (void)fclose(ip);
      }
   }

   else
   {
      if (!read_one_file(stdin, "-"))
      {
         err = 1;
         perror("stdin");
         fprintf(stderr, "stdin: read error\n");
         printf(" { 0, 0, 0, /* ERROR: */, \"-\" },\n");
      }
   }

   printf("};\n");

   return err;
}
