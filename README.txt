Pngcrush documentation

This is the copyright notice, disclaimer, and license:

/*
 * COPYRIGHT NOTICE, DISCLAIMER, AND LICENSE:
 *
 * If you have modified this source, you may insert additional notices
 * immediately after this sentence.
 *
 * Copyright (C) 1998, 1999, 2000 Glenn Randers-Pehrson (randeg@alum.rpi.edu)
 *
 * The pngcrush program is supplied "AS IS".  The Author disclaims all
 * warranties, expressed or implied, including, without limitation, the
 * warranties of merchantability and of fitness for any purpose.  The
 * Author assumes no liability for direct, indirect, incidental, special,
 * exemplary, or consequential damages, which may result from the use of
 * the pngcrush program, even if advised of the possibility of such damage.
 *
 * Permission is hereby granted to anyone to use, copy, modify, and distribute
 * this source code, or portions hereof, for any purpose, without fee, subject
 * to the following restrictions:
 *
 * 1. The origin of this source code must not be misrepresented.
 *
 * 2. Altered versions must be plainly marked as such and must not be
 *    misrepresented as being the original source.
 *
 * 3. This Copyright notice, disclaimer, and license may not be removed
 *    or altered from any source or altered source distribution.
 */

This is the output of "pngcrush" and "pngcrush -help":


 | pngcrush 1.4.1, Copyright (C) 1998, 1999, 2000 Glenn Randers-Pehrson
 | This is a free, open-source program.  Permission is
 | granted to everyone to use pngcrush without fee.
 | This program was built with libpng version 1.0.6e,
 |    Copyright (C) 1995, Guy Eric Schalnat, Group 42 Inc.,
 |    Copyright (C) 1996, 1997 Andreas Dilger,
 |    Copyright (C) 1998, 1999, 2000 Glenn Randers-Pehrson,
 | and zlib version 1.1.3, Copyright (C) 1998,
 |    Jean-loup Gailly and Mark Adler.


usage: pngcrush [options] infile.png outfile.png
       pngcrush -e ext [other options] files.png ...
       pngcrush -d dir [other options] files.png ...
options:
        -brute (Use brute-force, try 114 different methods [11-124])
            -c color_type of output file [0, 2, 4, or 6]
            -d directory_name (where output files will go)
 -double_gamma (used for fixing gamma in PhotoShop 5.0/5.02 files)
            -e extension  (used for creating output filename)
            -f user_filter [0-5]
        -force (Write a new output file even if larger than input)
            -g gamma (float or fixed*100000, e.g., 0.45455 or 45455)
         -itxt b[efore_IDAT]|a[fter_IDAT] "keyword" "text"
            -l zlib_compression_level [0-9]
            -m method [0 through 200]
          -max maximum_IDAT_size [1 through 8192]
            -n (no save; does not do compression or write output PNG)
            -plte_len n (truncate PLTE)
            -q (quiet)
          -rem chunkname (or "alla" or "allb")
-replace_gamma gamma (float or fixed*100000) even if gAMA is present.
          -res dpi
         -srgb [0, 1, 2, or 3]
         -text b[efore_IDAT]|a[fter_IDAT] "keyword" "text"
         -trns index red green blue gray
            -v (display more detailed information)
      -version (display the pngcrush version)
            -w compression_window_size [32, 16, 8, 4, 2, 1, 512]
            -h (help)
            -p (pause)



options:
        -brute (Use brute-force, try 114 different methods [11-124])

               Very time-consuming and generally not worthwhile.
               You can restrict this option to certain filter types,
               compression levels, or strategies by following it with
               "-f filter", "-l level", or "-z strategy".

            -c color_type of output file [0, 2, 4, or 6]

               Color type for the output file.  Future versions
               will also allow color_type 3, if there are 256 or
               fewer colors present in the input file.  Color types
               4 and 6 are padded with an opaque alpha channel if
               the input file does not have alpha information.
               You can use 0 or 4 to convert color to grayscale.
               Use 0 or 2 to delete an unwanted alpha channel.
               Default is to use same color type as the input file.

            -d directory_name (where output files will go)

               If a directory name is given, then the output
               files are placed in it, with the same filenames as
               those of the original files. For example,
               you would type 'pngcrush -directory CRUSHED *.png'
               to get *.png => CRUSHED/*.png

 -double_gamma (used for fixing gamma in PhotoShop 5.0/5.02 files)

               It has been claimed that the PS5 bug is actually
               more complex than that, in some unspecified way.

            -e extension  (used for creating output filename)

               e.g., -ext .new means *.png => *.new
               and -e _C.png means *.png => *_C.png

            -f user_filter [0-5]

               filter to use with the method specified in the
               preceding '-m method' or '-brute_force' argument.
               0: none; 1-4: use specified filter; 5: adaptive.

        -force (Write a new output file even if larger than input)

               Otherwise the input file will be copied to output
               if it is smaller than any generated file and no chunk
               additions, removals, or changes were requested.

            -g gamma (float or fixed*100000, e.g., 0.45455 or 45455)

               Value to insert in gAMA chunk, only if the input
               file has no gAMA chunk.  To replace an existing
               gAMA chunk, use the '-replace_gamma' option.

         -itxt b[efore_IDAT]|a[fter_IDAT] "keyword" "text"

               Uncompressed iTXt chunk to insert (see -text).

            -l zlib_compression_level [0-9]

               zlib compression level to use with method specified
               with the preceding '-m method' or '-brute_force'
               argument.

            -m method [0 through 200]

               pngcrush method to try (0 means try all of 1-10).
               Can be repeated as in '-m 1 -m 4 -m 7'.
               This can be useful if you run out of memory when pngcrush
               tries methods 2, 3, 5, 6, 8, 9, or 10 which use 
               filtering and are memory intensive.  Methods
               1, 4, and 7 use no filtering; methods 11 and up use 
               specified filter, compression level, and strategy.

          -max maximum_IDAT_size [1 through 8192]

            -n (no save; does not do compression or write output PNG)

               Useful in conjunction with -v option to get info.

            -plte_len n (truncate PLTE)

               Truncates the PLTE.  Be sure not to truncate it to

               less than the greatest index present in IDAT.
            -q (quiet)


          -rem chunkname (or "alla" or "allb")

               Name of an ancillary chunk or optional PLTE to be
               removed.  Be careful with this.  Please don't use 
               this feature to remove transparency, gamma, copyright,
               or other valuable information.  To remove several
               different chunks, repeat: -rem tEXt -rem pHYs.
               Known chunks (those in the PNG 1.1 spec or extensions
               document) can be named with all lower-case letters,
               so "-rem bkgd" is equivalent to "-rem bKGD".  But
               note: "-rem text" removes all forms of text chunks;
               Exact case is required to remove unknown chunks.
               To do surgery with a chain-saw, "-rem alla" removes
               all known ancillary chunks except for tRNS, and
               "-rem allb" removes all but tRNS and gAMA.

-replace_gamma gamma (float or fixed*100000) even if gAMA is present.

          -res dpi

               Write a pHYs chunk with the given resolution.

         -srgb [0, 1, 2, or 3]

               Value of 'rendering intent' for sRGB chunk.

         -text b[efore_IDAT]|a[fter_IDAT] "keyword" "text"

               tEXt chunk to insert.  keyword < 80 chars,

               text < 2048 chars. For now, you can only add ten
               tEXt, iTXt, or zTXt chunks per pngcrush run.

         -trns index red green blue gray

               Insert a tRNS chunk, if no tRNS chunk found in file.
               You must give all five parameters regardless of the
               color type, scaled to the output bit depth.

            -v (display more detailed information)

               Repeat the option (use "-v -v") for even more.

      -version (display the pngcrush version)

            -w compression_window_size [32, 16, 8, 4, 2, 1, 512]

               Size of the sliding compression window, in kbytes
               (or bytes, in case of 512).  It's best to
               use the default (32) unless you run out of memory.
               The program will use a smaller window anyway when
               the uncompressed file is smaller than 16k.

            -z zlib_strategy [0, 1, or 2]

               zlib compression strategy to use with the preceding
               '-m method' argument.

        -zitxt b[efore_IDAT]|a[fter_IDAT] "keyword" "text"

               Compressed iTXt chunk to insert (see -text).

         -ztxt b[efore_IDAT]|a[fter_IDAT] "keyword" "text"

               zTXt chunk to insert (see -text).

            -h (help)

               Display this information.

            -p (pause)

               Wait for [enter] key before continuing display.
               e.g., type 'pngcrush -pause -help', if the help
               screen scrolls out of sight.

