/*- pngtopng
 *
 * COPYRIGHT: please read the file "README.txt" from the containing directory.
 * This file has been placed in the public domain by the authors.
 *
 * Read a PNG and write it out in a fixed format
 *
 * This sample code is just the code from the top of 'example.c' with some error
 * handling added.  See example.c for more comments.
 */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Normally use <png.h> here to get the installed libpng, but this is done to
 * ensure the code picks up the local libpng implementation:
 */
#include "../../png.h"

int main(int argc, const char **argv)
{
   int result = 1;

   if (argc == 3)
   {
      png_image image;

      memset(&image, 0, sizeof image);

      if (png_image_begin_read_from_file(&image, argv[1]))
      {
         png_bytep buffer;
         
         /* Change this to try different formats! */
         image.format = PNG_FORMAT_RGBA;

         buffer = malloc(PNG_IMAGE_SIZE(image));

         if (buffer != NULL)
         {
            if (png_image_finish_read(&image, NULL/*background*/, buffer,
               0/*row_stride*/))
            {
               if (png_image_write_to_file(&image, argv[2],
                  0/*convert_to_8bit*/, buffer, 0/*row_stride*/))
                  result = 0;

               else
                  fprintf(stderr, "pngtopng: write %s: %s\n", argv[2],
                     image.message);
            }

            else
               fprintf(stderr, "pngtopng: read %s: %s\n", argv[1], image.message);
         }

         else
            fprintf(stderr, "pngtopng: out of memory: %lu bytes\n",
               (unsigned long)PNG_IMAGE_SIZE(image));
      }

      else
         /* Failed to read the first argument: */
         fprintf(stderr, "pngtopng: %s: %s\n", argv[1], image.message);
   }

   else
      /* Wrong number of arguments */
      fprintf(stderr, "pngtopng: usage: pngtopng input-file output-file\n");

   return result;
}
