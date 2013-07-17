#!/bin/sh

# pngzop version 1.0.2

# Copyright 2013 by Glenn Randers-Pehrson
# Released under the pngcrush license (which is equivalent to the zlib
# license plus UCITA disclaimers).

# Extracts the concatenated data from the IDAT chunks in a set of PNG files,
# recompresses them with zopfli, and embeds them in a new set of PNG files
# with suffix ".png" replaced (by default) with "_pngzop.png"

# Requires zopfli, pngcrush (version 1.7.66 or later), zpipe (from the
# zlib-1.2.7 or later distribution, in the "examples" directory), pngfix
# (from the libpng-1.6.3 or later distribution), and "mkdir -p",
# along with these programs that should have been installed along with
# this "pngzop" script:
#
#        pngzop_get_ihdr.exe
#        pngzop_get_idat.exe
#        pngzop_get_iend.exe
#        pngzop_zlib_to_idat.exe

# Usage:
# pngzop [-b|--blacken] [-d|--directory dir] [-e|--extension ext] *.png
#        (to overwrite the input, use "-e .png")

# Input: *.png
# Output: *_pngzop.png

# Check pngcrush version
PCMIN=1766
PCVER=`pngcrush 2>&1 | head -2 | tail -1 | sed -e "s/.* //"`
PCTEST=`echo $PCVER | sed -e "s/\.//g"`
if [ $PCTEST -lt $PCMIN ] ; then
     echo Pngcrush version is $PCVER but version 1.7.66 or later is required.
     exit
fi

# Get temporary directory; use TMPDIR if defined, otherwise /tmp
case x${TMPDIR} in
   x)
      temp=/tmp/pngzop-$$
      ;;
   *)
      temp=$TMPDIR/pngzop-$$
      ;;
esac

mkdir -p $temp
      
blacken=
directory=.
extension=

for x in $*
do
case $1 in
  -b|--blacken)
    blacken="-blacken"
    shift;;
  -d|--directory)
    shift
    directory=$1
    shift;;
  -e|--extension)
    shift
    extension=$1
    shift;;
  *)
    break;;
esac
done

# If a directory was defined, don't change filenames (extension == .png)
# unless an extension was also defined.
case x$directory in
    x.)
      ;;
    *)
      case x$extension in
        x)
          extension=.png
          ;;
        *)
          ;;
      esac
      mkdir -p $directory
      ;;
esac

case x$extension in
  x)
    extension=_pngzop.png
    ;;
  *)
    ;;
esac

for x in $*
do
  root=`echo $x | sed -e s/.png$//`

  # prepass to reduce and possibly blacken
  pngcrush -q -m 1 -force -reduce $blacken $x ${temp}/${root}_temp.png

  pngzop_get_ihdr.exe < ${temp}/${root}_temp.png > ${temp}/${root}_pz.png

  # Generate trial PNGs with filter none, 4 PNG filters, and adaptive
  # filter
  for f in 0 1 2 3 4 5
  do
    pngcrush -q -m 1 -f $f -force ${temp}/${root}_temp.png \
              ${temp}/${root}_f$f.png &
  done
  wait

  # Extract and decompress the zlib datastream from the concatenated
  # IDAT chunks.
  for f in 0 1 2 3 4 5
  do
    pngzop_get_idat.exe < ${temp}/${root}_f$f.png | zpipe -d > \
                          ${temp}/${root}_f$f.idat &
  done
  wait
  rm -f ${temp}/${root}_f?.png

  # Recompress the IDAT data using zopfli
  for f in 0 1 2 3 4 5
  do
    zopfli --i25 --zlib ${temp}/${root}_f$f.idat &
  done
  wait
  rm -f ${temp}/${root}_f?.idat

  # Copy the smallest result to file.zlib

  file=${temp}/${root}_f0.idat.zlib
  smallest=$file
  smallsize=`ls -l $file | sed -e "
s/[^ 	]*[	 ]*\
[^ 	]*[	 ]*\
[^ 	]*[	 ]*\
[^ 	]*[	 ]*\
//" -e "s/[ 	].*//"`

  for f in 1 2 3 4 5
  do

  file=${temp}/${root}_f$f.idat.zlib

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

  pngzop_zlib_to_idat.exe < $smallest >> ${temp}/${root}_pz.png
  rm -f ${temp}/${root}_f?.idat.zlib
  pngzop_get_iend.exe < ${temp}/${root}_temp.png >> ${temp}/${root}_pz.png
  rm ${temp}/${root}_temp.png

  # Optimize the CMF bytes and put the result in desired destination.
  pngfix -o --quiet --out=${directory}/${root}${extension} \
         ${temp}/${root}_pz.png
  rm ${temp}/${root}_pz.png

done
rm -rf $temp
