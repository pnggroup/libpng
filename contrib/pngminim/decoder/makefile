# Makefile for PngMinus (pngm2pnm)
# Linux / Unix

#CC=cc
CC=gcc
LD=$(CC)

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

# note: dependencies do not work on implicit rule lines
.c$(O):
	$(CC) -c $(CFLAGS) $<

# dependencies

all: pngm2pnm$(E)

pngm2pnm$(E): $(OBJS)
	$(LD) -o pngm2pnm$(E) $(OBJS)
	strip pngm2pnm$(E)

pnglibconf.h: scripts/pnglibconf.mak scripts/pnglibconf.dfa \
	scripts/options.awk pngusr.h

	rm -f pnglibconf.h

	$(MAKE) $(MAKEFLAGS) -f scripts/pnglibconf.mak\
	 CPPFLAGS=-DPNG_USER_CONFIG $@

clean:
	$(MAKE) $(MAKEFLAGS) -f scripts/pnglibconf.mak clean
	$(RM) pngm2pnm$(O)
	$(RM) pngm2pnm$(E)
	$(RM) $(OBJS)

$(OBJS): png.h pngconf.h pnglibconf.h pngpriv.h pngstruct.h\
	 pnginfo.h pngdebug.h zlib.h

# End of makefile for pngm2pnm
