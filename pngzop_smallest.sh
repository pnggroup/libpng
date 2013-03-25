#!/bin/sh

# For use in the pngzop project
# Copyright 2013 by Glenn Randers-Pehrson
# Released under the pngcrush license (equivalent to the libpng license)

# Selects the smaller of file.zdat (extracted from a PNG) or
# file.idat.zlib (same but recompressed with zopfli)

# picksmallest file1 file...

# for use on platforms that do not have "ls -S"

smallest=$1
smallsize=`ls -l $1 | sed -e "
s/[^ 	]*[	 ]*\
[^ 	]*[	 ]*\
[^ 	]*[	 ]*\
[^ 	]*[	 ]*\
//" -e "s/[ 	].*//"`

shift

for x in $*
do
file=$1
shift

size=`ls -l $file | sed -e "
s/[^ 	]*[	 ]*\
[^ 	]*[	 ]*\
[^ 	]*[	 ]*\
[^ 	]*[	 ]*\
//" -e "s/[ 	].*//"`

if [ $smallsize -gt $size 	] ; then
smallest=$file
smallsize=$size
fi
done

echo $smallest
