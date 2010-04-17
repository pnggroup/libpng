test -n "${PNGSRC}" || PNGSRC=../../..
cp ${PNGSRC}/contrib/pngminus/png2pnm.c pngm2pnm.c
cp "${PNGSRC}"/*.h .
cp "${PNGSRC}"/*.c .
rm example.c pngtest.c pngpread.c pngw*.c
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
