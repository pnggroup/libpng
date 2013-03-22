#!/bin/sh

# For use in the pngzop project
# Copyright 2013 by Glenn Randers-Pehrson
# Released under the pngcrush license (equivalent to the libpng license)

# Usage:
# pngzop_pngcrush_zopfli.sh file.png > file.zlib

# file.png is a PNG file.
# Standard output is the uncompressed IDAT chunk data, recompressed as a zlib
# datastream.

root=`echo $1 | sed -e "s/.png$//"`
rm -f ${root}_L9.png

# Do pngcrush level 9 with none, 4 filters, and adaptive filtering, and
# select the smallest.
pngcrush -q -m 113 -m 114 -m 115 -m 116 -m 117 -m 118 \
         -force $1 ${root}_L9.png

# Extract and decompress the zlib datastream from the concatenated IDAT chunks.
pngzop_get_idat.exe < ${root}_L9.png | zpipe -d > ${root}_L9.idat
rm ${root}_L9.png

# Recompress with zopfli and write it on standard output.
zopfli --i25 --zlib -c ${root}_L9.idat

rm ${root}_L9.idat
