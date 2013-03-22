#!/bin/sh

# png_get_idat.sh

# For use in the pngzop project
# Copyright 2013 by Glenn Randers-Pehrson
# Released under the pngcrush license (equivalent to the libpng license)

# Extracts and decompresses the data from the IDAT chunks in a set of
# PNG files.

# Usage:
# png_get_idat.sh file.png

# Output: file.idat

for x in $*
do
     root=`echo $1 | sed -e "s/.png$//"`
     pngzop_get_idat.exe < $1 | zpipe -d  > $root.idat
     shift
done
