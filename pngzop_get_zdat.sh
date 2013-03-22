#!/bin/sh

# pngzop_get_zdat.sh

# For use in the pngzop project
# Copyright 2013 by Glenn Randers-Pehrson
# Released under the pngcrush license (equivalent to the libpng license)

# Extracts the concatenated data from the IDAT chunks in a set of PNG files,
# leaving it compressed as found.

# Usage:
# pngzop_get_zdat.sh file.png

# Output: file.zdat

for x in $*
do
  root=`echo $1 | sed -e s/.png$//`
  pngzop_get_idat.exe < $1 > $root.zdat
  shift
done
