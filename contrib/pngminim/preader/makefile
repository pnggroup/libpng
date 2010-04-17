# Makefile for PngMinus (rpng2)
# Linux / Unix

#CC=cc
CC=gcc
LD=$(CC)

RM=rm -f

#XINC = -I/usr/include			# old-style, stock X distributions
#XLIB = -L/usr/lib/X11 -lX11		#  (including SGI IRIX)

#XINC = -I/usr/openwin/include		# Sun workstations (OpenWindows)
#XLIB = -L/usr/openwin/lib -lX11

XINC = -I/usr/X11R6/include		# new X distributions (X.org, etc.)
XLIB = -L/usr/X11R6/lib -lX11
#XLIB = -L/usr/X11R6/lib64 -lX11	# e.g., Red Hat on AMD64

#XINC = -I/usr/local/include   		# FreeBSD
#XLIB = -L/usr/local/lib -lX11

#LIBS = $(XLIB)
LIBS = $(XLIB) -lm                      #platforms that need libm

CFLAGS=-DPNG_USER_CONFIG -DNO_GZCOMPRESS -DNO_GZIP \
       -DdeflateParams\(a,b,c\)=Z_OK -I. $(XINC) -O1

C=.c
O=.o
L=.a
E=

ZOBJS  = adler32$(O) crc32$(O) \
	 infback$(O) inffast$(O) inflate$(O) inftrees$(O) \
	 trees$(O) uncompr$(O) zutil$(O)

OBJS  = rpng2-x$(O) readpng2$(O) png$(O) pngerror$(O) pngget$(O) pngmem$(O) \
	pngpread$(O) pngread$(O) pngrio$(O) pngrtran$(O) pngrutil$(O) \
	pngset$(O) pngtrans$(O)  $(ZOBJS)

# implicit make rules -------------------------------------------------------

.c$(O):
	$(CC) -c $(CFLAGS) $<

# dependencies

all: rpng2-x$(E)

rpng2-x$(E): $(OBJS)
	$(LD) -o rpng2-x$(E) $(OBJS) $(LIBS)
	strip rpng2-x$(E)

pnglibconf.h: scripts/pnglibconf.mak scripts/pnglibconf.dfa scripts/options.awk pngusr.h
	rm -f pnglibconf.h
	$(MAKE) $(MAKEFLAGS) -f scripts/pnglibconf.mak CPPFLAGS=-DPNG_USER_CONFIG $@

clean:
	$(MAKE) $(MAKEFLAGS) -f scripts/pnglibconf.mak clean
	$(RM) rpng2-x$(O)
	$(RM) rpng2-x$(E)
	$(RM) $(OBJS)

$(OBJS): png.h pngconf.h pnglibconf.h pngpriv.h pngstruct.h pnginfo.h pngdebug.h  readpng2.h pngusr.h zlib.h

# End of makefile for rpng2-x
