# Makefile for PngMinus (pngm2pnm)
# Linux / Unix

#CC=cc
CC=gcc
LD=$(CC)
AWK=awk

RM=rm -f

CFLAGS=-DPNG_USER_CONFIG -DNO_GZCOMPRESS -DNO_GZIP \
       -DdeflateParams\(a,b,c\)=Z_OK -I. -O1

C=.c
O=.o
L=.a
E=

ZOBJS  = adler32$(O) crc32$(O) \
	 infback$(O) inffast$(O) inflate$(O) inftrees$(O) \
	 trees$(O) uncompr$(O) zutil$(O)

OBJS  = pngm2pnm$(O) png$(O) pngerror$(O) pngget$(O) pngmem$(O) \
	pngread$(O) pngrio$(O) pngrtran$(O) pngrutil$(O) \
	pngset$(O) pngtrans$(O)  $(ZOBJS)

# implicit make rules -------------------------------------------------------

.c$(O): png.h pngconf.h pnglibconf.h pngpriv.h pngstruct.h pnginfo.h pngdebug.h  pngusr.h zlib.h
	$(CC) -c $(CFLAGS) $<

# dependencies

all: pngm2pnm$(E)

# see scripts/pnglibconf.mak for more options
pnglibconf.h: pnglibconf.mak pnglibconf.dfn
	make -f pnglibconf.mak pnglibconf.h

# used on demand to regenerate the standard header, CPPFLAGS should
# be empty - no non-standard defines
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


pngm2pnm$(E): $(OBJS)
	$(LD) -o pngm2pnm$(E) $(OBJS)
	strip pngm2pnm$(E)

clean:
	$(RM) pngm2pnm$(O)
	$(RM) pngm2pnm$(E)
	$(RM) $(OBJS)

# End of makefile for pngm2pnm
