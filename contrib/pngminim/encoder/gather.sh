test -n "${PNGSRC}" || PNGSRC=../../..
cp ${PNGSRC}/contrib/pngminus/pnm2png.c pnm2pngm.c
cp "${PNGSRC}"/*.h .
cp "${PNGSRC}"/*.c .
rm example.c pngtest.c pngr*.c pngpread.c
rm -f pnglibconf.h
test -d scripts || mkdir scripts
cp "${PNGSRC}"/scripts/pnglibconf.mak scripts
cp "${PNGSRC}"/scripts/pnglibconf.dfa scripts
cp "${PNGSRC}"/scripts/options.awk scripts
# change the following if zlib is somewhere else
test -n "${ZLIBSRC}" || ZLIBSRC="${PNGSRC}"/../zlib
cp "${ZLIBSRC}"/*.h .
cp "${ZLIBSRC}"/*.c .
rm inf*.[ch]
rm minigzip.c example.c gz*

