test -n "${PNGSRC}" || PNGSRC=../../..
cp "${PNGSRC}"/contrib/gregbook/rpng2-x.c .
cp "${PNGSRC}"/contrib/gregbook/readpng2.[ch] .
cp "${PNGSRC}"/contrib/gregbook/COPYING .
cp "${PNGSRC}"/contrib/gregbook/LICENSE .
cp "${PNGSRC}"/*.h .
cp "${PNGSRC}"/*.c .
rm example.c pngtest.c pngw*.c
rm -f pnglibconf.h
test -d scripts || mkdir scripts
cp "${PNGSRC}"/scripts/pnglibconf.mak scripts
cp "${PNGSRC}"/scripts/pnglibconf.dfa scripts
cp "${PNGSRC}"/scripts/options.awk scripts
# change the following if zlib is somewhere else
test -n "${ZLIBSRC}" || ZLIBSRC="${PNGSRC}"/../zlib
cp "${ZLIBSRC}"/*.h .
cp "${ZLIBSRC}"/*.c .
rm minigzip.c example.c compress.c deflate.c gz*
