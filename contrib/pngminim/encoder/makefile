
# Makefile for PngMinus (pnm2pngm)
# Linux / Unix

#CC=cc
CC=gcc
LD=$(CC)
AWK=awk
SED=sed
CPP=cpp

RM=rm -f

CFLAGS=-DPNG_USER_CONFIG -DNO_GZCOMPRESS -DNO_GZIP -I. -O1

C=.c
O=.o
L=.a
E=

ZOBJS  = adler32$(O) compress$(O) crc32$(O) deflate$(O) \
	 dummy_inflate$(O) \
	 trees$(O) uncompr$(O) zutil$(O)

OBJS  = pnm2pngm$(O) png$(O) pngerror$(O) pngget$(O) pngmem$(O) \
	pngset$(O) pngtrans$(O) pngwio$(O) pngwrite$(O) \
	pngwtran$(O) pngwutil$(O) $(ZOBJS)

# implicit make rules -------------------------------------------------------

.c$(O): png.h pngconf.h pnglibconf.h pngpriv.h pngstruct.h pnginfo.h pngdebug.h  pngusr.h zlib.h
	$(CC) -c $(CFLAGS) $<

# dependencies

all: pnm2pngm$(E)

# see scripts/pnglibconf.mak for more options
pnglibconf.h: pnglibconf.mak pnglibconf.dfn
	make -f pnglibconf.mak pnglibconf.h

# used on demand to regenerate the standard header, CPPFLAGS should
# be empty - no non-standard defines

.dfn.out:
	rm -f $@ dfn.c dfn?.out
	echo '#include "$<"' >dfn.c
	$(CPP) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) @LIBPNG_DEFINES@\
	    $(CPPFLAGS) $(SYMBOL_CFLAGS) dfn.c >dfn1.out
	$(SED) -n -e 's|^.*PNG_DEFN_MAGIC-\(.*\)-PNG_DEFN_END.*$$|\1|p'\
	    dfn1.out >dfn2.out
	$(SED) -e 's| *@@@ *||g' -e 's| *$$||' dfn2.out >dfn3.out
	rm -f dfn.c dfn[12].out
	mv dfn3.out $@

pnglibconf.dfn: pnglibconf.dfa options.awk
	rm -f $@ dfn?.out
	test -z "$(CPPFLAGS)" 
	echo "com @PNGLIB_VERSION@ STANDARD API DEFINITION" |\
	$(AWK) -f options.awk pre=1 out=dfn1.out\
	    logunsupported=1 - pnglibconf.dfa 1>&2
	$(AWK) -f options.awk pre=0 out=dfn2.out\
	    logunsupported=1 dfn1.out 1>&2
	rm dfn1.out
	mv dfn2.out $@

pnm2pngm$(E): $(OBJS)
	$(LD) -o pnm2pngm$(E) $(OBJS)
	strip pnm2pngm$(E)

clean:
	$(RM) pnm2pngm$(O)
	$(RM) pnm2pngm$(E)
	$(RM) $(OBJS)

# End of makefile for pngm2pnm
