#!/bin/sh
#
# Make a set of test PNG files, MAKEPNG is the name of the makepng executable
# built from contrib/libtests/makepng.c

# Copyright (c) 2015 John Cunningham Bowler

# Last changed in libpng 1.7.0 [(PENDING RELEASE)]

# This code is released under the libpng license.
# For conditions of distribution and use, see the disclaimer
# and license in png.h

# The arguments say whether to build all the files or whether just to build the
# ones that extend the code-coverage of libpng from the existing test files in
# contrib/pngsuite.
test -n "$MAKEPNG" || MAKEPNG=./makepng
if test "$1" = "-v"
then
   verbose=1
   shift
else
   verbose=
fi
what="$1"
shift
cmdline="$@"
opts=

mp(){
   test -n "$verbose" &&
      echo ${MAKEPNG} $opts $cmdline $1 "$3" "$4" "$3-$4$2.png"
   ${MAKEPNG} $opts $cmdline $1 "$3" "$4" "$3-$4$2.png"
}

mpg(){
   if test "$1" = "none"
   then
      mp "" "" "$2" "$3"
   else
      mp "--$1" "-$1" "$2" "$3"
   fi
}

mptrans(){
   if test "$1" = "none"
   then
      mp "--tRNS" "-tRNS" "$2" "$3"
   else
      mp "--tRNS --$1" "-$1-tRNS" "$2" "$3"
   fi
}

case "$what" in
   --small)
      opts="--small";;&

   --all|--small)
      for g in none sRGB linear 1.8
      do
         for c in gray palette
         do
            for b in 1 2 4
            do
               mpg "$g" "$c" "$b"
               mptrans "$g" "$c" "$b"
            done
         done

         mpg "$g" palette 8
         mptrans "$g" palette 8

         for b in 8 16
         do
            for c in gray gray-alpha rgb rgb-alpha
            do
               mpg "$g" "$c" "$b"
            done
            for c in gray rgb
            do
               mptrans "$g" "$c" "$b"
            done
         done
      done;;

   --coverage)
      # Extra images made to improve code coverage:
      ${MAKEPNG} --insert sBIT 1 --tRNS gray 2 gray-2-sBIT-tRNS.png
      :;;

   *)
      echo "$0 $1: unknown argument, usage:" >&2
      echo "  $0 [--all|--coverage|--small]" >&2
      exit 1
esac
