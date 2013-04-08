#!/bin/sh

# pngzop_brute.sh

# For use in the pngzop project
# Copyright 2013 by Glenn Randers-Pehrson
# Released under the pngcrush license (equivalent to the libpng license)

# Extracts the concatenated data from the IDAT chunks in a set of PNG files,
# recompresses them with zopfli, and embeds them in a new set of PNG files
# with suffix ".png" replaced with "_pngzop_brute.png"

# Usage:
# pngzop_brute *.png

# Standard Input: *.png
# Output: *_pngzop_brute.png

# To do: Adjust zlib CMF to reflect actual windowBits required.

for x in $*
do
  root=`echo $1 | sed -e s/.png$//`
  pngzop_get_ihdr.exe < $1 > ${root}_pngzop_brute.png
  pngzop_brute_zopfli.sh $1
  pngzop_zlib_to_idat.exe < ${root}.zlib >> ${root}_pngzop_brute.png
  rm ${root}.zlib
  pngzop_get_iend.exe < $1 >> ${root}_pngzop_brute.png
  shift
done
