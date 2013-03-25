#!/bin/sh

# png_brute_zopfli.sh

# For use in the pngzop project
# Copyright 2013 by Glenn Randers-Pehrson
# Released under the pngcrush license (equivalent to the libpng license)

# Usage:
# png_brute_zopfli.sh file.png [...]

# Input: *.png, a set of PNG files
# Output: *.zlib, zopfli-compressed IDAT data

for x in $*
do
root=`echo $1 | sed -e "s/.png$//"`

# Generate trial PNGs with filter none, 4 PNG filters, and adaptive filter
for f in 0 1 2 3 4 5
do
  pngcrush -q -m 1 -f $f -force $1 ${root}_f$f.png &
done
wait

# Extract and decompress the zlib datastream from the concatenated IDAT chunks.
for f in 0 1 2 3 4 5
do
  pngzop_get_idat.exe < ${root}_f$f.png | zpipe -d > ${root}_f$f.idat &
done
wait
rm -f ${root}_f?.png

# Recompress the IDAT data using zopfli
for f in 0 1 2 3 4 5
do
  zopfli --i25 --zlib ${root}_f$f.idat &
done
wait
rm -f ${root}_f?.idat

# Copy the smallest result to file.zlib
cat `pngzop_smallest.sh ${root}_f?.idat.zlib | tail -1` > ${root}.zlib
rm -f ${root}_f?.idat.zlib

shift
done
