Pngcrush documentation

This is is a copy of the copyright notice, disclaimer, and license, for
your convenience (the actual notice appears in the file pngcrush.c; in
case of any discrepancy, the copy in pngcrush.c shall prevail):

/*
 * COPYRIGHT NOTICE, DISCLAIMER, AND LICENSE:
 *
 * If you have modified this source, you may insert additional notices
 * immediately after this sentence.
 *
 * Copyright (C) 1998-2002 Glenn Randers-Pehrson (randeg@alum.rpi.edu)
 *
 * The pngcrush computer program is supplied "AS IS".  The Author disclaims all
 * warranties, expressed or implied, including, without limitation, the
 * warranties of merchantability and of fitness for any purpose.  The
 * Author assumes no liability for direct, indirect, incidental, special,
 * exemplary, or consequential damages, which may result from the use of
 * the computer program, even if advised of the possibility of such damage.
 * There is no warranty against interference with your enjoyment of the
 * computer program or against infringement.  There is no warranty that my
 * efforts or the computer program will fulfill any of your particular purposes
 * or needs.  This computer program is provided with all faults, and the entire
 * risk of satisfactory quality, performance, accuracy, and effort is with
 * the user.
 *
 * Permission is hereby irrevocably granted to everyone to use, copy, modify,
 * and distribute this source code, or portions hereof, for any purpose,
 * without payment of any fee, subject to the following restrictions:
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


 | pngcrush 1.5.10, Copyright (C) 1998-2002 Glenn Randers-Pehrson
 | This is a free, open-source program.  Permission is irrevocably
 | granted to everyone to use this version of pngcrush without
 | payment of any fee.
 | Executable name is pngcrush
 | It was built with libpng version 1.2.4, and is
 | running with  libpng version 1.2.4 - July 8, 2002 (header)
 |    Copyright (C) 1998-2002 Glenn Randers-Pehrson,
 |    Copyright (C) 1996, 1997 Andreas Dilger,
 |    Copyright (C) 1995, Guy Eric Schalnat, Group 42 Inc.,
 | and zlib version 1.1.4pc, Copyright (C) 1998,
 |    Jean-loup Gailly and Mark Adler.


usage: pngcrush [options] infile.png outfile.png
       pngcrush -e ext [other options] files.png ...
       pngcrush -d dir [other options] files.png ...
options:
      -already already_crushed_size [e.g., 8192]
    -bit_depth depth (bit_depth to use in output file)
        -brute (Use brute-force, try 114 different methods [11-124])
            -c color_type of output file [0, 2, 4, or 6]
            -d directory_name (where output files will go)
 -double_gamma (used for fixing gamma in PhotoShop 5.0/5.02 files)
            -e extension  (used for creating output filename)
            -f user_filter [0-5]
          -fix (fix otherwise fatal conditions such as bad CRCs)
        -force (Write a new output file even if larger than input)
            -g gamma (float or fixed*100000, e.g., 0.45455 or 45455)
         -iccp length "Profile Name" iccp_file
         -itxt b[efore_IDAT]|a[fter_IDAT] "keyword" "text"
            -l zlib_compression_level [0-9]
         -loco ("loco crush" truecolor PNGs)
            -m method [0 through 200]
          -max maximum_IDAT_size [default 8192]
        -nofilecheck (do not check for infile.png == outfile.png)
            -n (no save; does not do compression or write output PNG)
     -plte_len n (truncate PLTE)
            -q (quiet)
       -reduce (do lossless color type or bit depth reduction)
          -rem chunkname (or "alla" or "allb")
-replace_gamma gamma (float or fixed*100000) even if gAMA is present.
          -res dpi
         -save (keep all copy-unsafe chunks)
         -srgb [0, 1, 2, or 3]
         -text b[efore_IDAT]|a[fter_IDAT] "keyword" "text"
   -trns_array n trns[0] trns[1] .. trns[n-1]
         -trns index red green blue gray
            -v (display more detailed information)
      -version (display the pngcrush version)
            -w compression_window_size [32, 16, 8, 4, 2, 1, 512]
            -z zlib_strategy [0, 1, or 2]
         -zmem zlib_compression_mem_level [1-9, default 9]
        -zitxt b[efore_IDAT]|a[fter_IDAT] "keyword" "text"
         -ztxt b[efore_IDAT]|a[fter_IDAT] "keyword" "text"
            -h (help and legal notices)
            -p (pause)


options (Note: any option can be spelled out for clarity, e.g.,
          "pngcrush -dir New -method 7 -remove bkgd *.png"
          is the same as "pngcrush -d New -m 7 -rem bkgd *.png"):

      -already already_crushed_size [e.g., 8192]

               If file has an IDAT greater than this size, it
               will be considered to be already crushed and will
               not be processed, unless you are making other changes
               or the "-force" option is present.

    -bit_depth depth (bit_depth to use in output file)

               Default output depth is same as input depth.

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

          -fix (fix otherwise fatal conditions such as bad CRCs)

        -force (Write a new output file even if larger than input)

               Otherwise the input file will be copied to output
               if it is smaller than any generated file and no chunk
               additions, removals, or changes were requested.

            -g gamma (float or fixed*100000, e.g., 0.45455 or 45455)

               Value to insert in gAMA chunk, only if the input
               file has no gAMA chunk.  To replace an existing
               gAMA chunk, use the '-replace_gamma' option.

         -iccp length "Profile Name" iccp_file

               file with ICC profile to insert in an iCCP chunk.

         -itxt b[efore_IDAT]|a[fter_IDAT] "keyword" "text"

               Uncompressed iTXt chunk to insert (see -text).

            -l zlib_compression_level [0-9]

               zlib compression level to use with method specified
               with the preceding '-m method' or '-brute_force'
               argument.

         -loco ("loco crush" truecolor PNGs)

               Make the file more compressible by performing a
               lossless reversible color transformation.
               The resulting file is a MNG, not a PNG, and should
               be given the ".mng" file extension.  The
               "loco" option has no effect on grayscale or
               indexed-color PNG files.

            -m method [0 through 200]

               pngcrush method to try (0 means try all of 1-10).
               Can be repeated as in '-m 1 -m 4 -m 7'.
               This can be useful if pngcrush runs out of memory
               when it tries methods 2, 3, 5, 6, 8, 9, or 10 which
               use filtering and are memory intensive.  Methods
               1, 4, and 7 use no filtering; methods 11 and up use 
               specified filter, compression level, and strategy.

          -max maximum_IDAT_size [default 8192]

        -nofilecheck (do not check for infile.png == outfile.png)

               To avoid false hits from MSVC-compiled code.  Note
               that if you use this option, you are responsible for
               ensuring that the input file is not the output file.

            -n (no save; does not do compression or write output PNG)

               Useful in conjunction with -v option to get info.

     -plte_len n (truncate PLTE)

               Truncates the PLTE.  Be sure not to truncate it to
               less than the greatest index present in IDAT.

            -q (quiet)

       -reduce (do lossless color type or bit depth reduction)

          (if possible)

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

         -save (keep all copy-unsafe chunks)

               Save otherwise unknown ancillary chunks that would
               be considered copy-unsafe.  This option makes
               chunks 'known' to pngcrush, so they can be copied.

         -srgb [0, 1, 2, or 3]

               Value of 'rendering intent' for sRGB chunk.

         -text b[efore_IDAT]|a[fter_IDAT] "keyword" "text"

               tEXt chunk to insert.  keyword < 80 chars,

               text < 2048 chars. For now, you can only add ten
               tEXt, iTXt, or zTXt chunks per pngcrush run.

   -trns_array n trns[0] trns[1] .. trns[n-1]

               Insert a tRNS chunk, if no tRNS chunk found in file.
               Values are for the tRNS array in indexed-color PNG.

         -trns index red green blue gray

               Insert a tRNS chunk, if no tRNS chunk found in file.
               You must give all five parameters regardless of the
               color type, scaled to the output bit depth.

            -v (display more detailed information)

               Repeat the option (use "-v -v") for even more.

      -version (display the pngcrush version)

               Look for the most recent version of pngcrush at
               http://pmt.sf.net

            -w compression_window_size [32, 16, 8, 4, 2, 1, 512]

               Size of the sliding compression window, in kbytes
               (or bytes, in case of 512).  It's best to
               use the default (32) unless you run out of memory.
               The program will use a smaller window anyway when
               the uncompressed file is smaller than 16k.

            -z zlib_strategy [0, 1, or 2]

               zlib compression strategy to use with the preceding
               '-m method' argument.

         -zmem zlib_compression_mem_level [1-9, default 9]

        -zitxt b[efore_IDAT]|a[fter_IDAT] "keyword" "text"

               Compressed iTXt chunk to insert (see -text).

         -ztxt b[efore_IDAT]|a[fter_IDAT] "keyword" "text"

               zTXt chunk to insert (see -text).

            -h (help and legal notices)

               Display this information.

            -p (pause)

               Wait for [enter] key before continuing display.
               e.g., type 'pngcrush -pause -help', if the help
               screen scrolls out of sight.

