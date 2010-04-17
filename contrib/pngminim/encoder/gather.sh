
# Get the source for the pngminus application
cp ../../pngminus/pnm2png.c pnm2pngm.c

# Get the libpng sources
cp ../../../*.h .
cp ../../../*.c .

# Get the libpng scripts for building pnglibconf.h
cp ../../../scripts/options.awk .
cp ../../../scripts/pnglibconf.dfa .
sed -e "s:scripts/::g" ../../../scripts/pnglibconf.mak > pnglibconf.mak
#14+%
# Remove libpng sources we won't use
rm example.c pngtest.c pngr*.c pngpread.c

# Get the zlib sources
# Change the following 2 lines if zlib is somewhere else
cp ../../../../zlib/*.h .
cp ../../../../zlib/*.c .

# Remove zlib sources we won't use
rm minigzip.c example.c inf*.[ch] gz*
