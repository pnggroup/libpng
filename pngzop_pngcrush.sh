#!/bin/sh

# pngzop_pngcrush.sh

# For use in the pngzop project
# Copyright 2013 by Glenn Randers-Pehrson
# Released under the pngcrush license (equivalent to the libpng license)

# Extracts the concatenated data from the IDAT chunks in a set of PNG files,
# leaving it compressed as found.

# Usage:
# pngzop_pngcrush.sh *.png

# Standard Input: *.png
# Output: *_zop.png

for x in $*
do
  root=`echo $1 | sed -e s/.png$//`
  pngzop_get_ihdr.exe < $1 > ${root}_pngzop.png
  pngzop_pngcrush_zopfli.sh $1 | pngzop_zlib_to_idat.exe >> ${root}_pngzop.png
  pngzop_get_iend.exe < $1 >> ${root}_pngzop.png
  shift
done
