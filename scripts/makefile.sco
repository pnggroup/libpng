# makefile for SCO OSr5  ELF and Unixware 7 with Native cc
# Contributed by Mike Hopkirk (hops@sco.com) modified from Makefile.lnx
#   force ELF build dynamic linking, SONAME setting in lib and RPATH in app
# Copyright (C) 1998 Greg Roelofs
# Copyright (C) 1996, 1997 Andreas Dilger
# For conditions of distribution and use, see copyright notice in png.h

CC=cc

# where make install puts libpng.a, libpng.so*, and png.h
prefix=/usr/local

# Where the zlib library and include files are located
#ZLIBLIB=/usr/local/lib
#ZLIBINC=/usr/local/include
ZLIBLIB=../zlib
ZLIBINC=../zlib

CFLAGS= -dy -belf -I$(ZLIBINC) -O3
LDFLAGS=-L. -L$(ZLIBLIB) -lpng -lz -lm

#RANLIB=ranlib
RANLIB=echo

# read libpng.txt or png.h to see why PNGMAJ is 0.  You should not
# have to change it.
PNGMAJ = 0
PNGMIN = 1.2.2
PNGVER = $(PNGMAJ).$(PNGMIN)
LIBNAME = libpng12

INCPATH=$(prefix)/include/libpng
LIBPATH=$(prefix)/lib

OBJS = png.o pngset.o pngget.o pngrutil.o pngtrans.o pngwutil.o \
	pngread.o pngrio.o pngwio.o pngwrite.o pngrtran.o \
	pngwtran.o pngmem.o pngerror.o pngpread.o

OBJSDLL = $(OBJS:.o=.pic.o)

.SUFFIXES:      .c .o .pic.o

.c.pic.o:
	$(CC) -c $(CFLAGS) -KPIC -o $@ $*.c

all: libpng.a $(LIBNAME).so pngtest

libpng.a: $(OBJS)
	ar rc $@ $(OBJS)
	$(RANLIB) $@

$(LIBNAME).so: $(LIBNAME).so.$(PNGMAJ)
	ln -f -s $(LIBNAME).so.$(PNGMAJ) $(LIBNAME).so

$(LIBNAME).so.$(PNGMAJ): $(LIBNAME).so.$(PNGVER)
	ln -f -s $(LIBNAME).so.$(PNGVER) $(LIBNAME).so.$(PNGMAJ)

$(LIBNAME).so.$(PNGVER): $(OBJSDLL)
	$(CC) -G  -Wl,-h,$(LIBNAME).so.$(PNGMAJ) -o $(LIBNAME).so.$(PNGVER) \
	 $(OBJSDLL)

pngtest: pngtest.o $(LIBNAME).so
	LD_RUN_PATH=.:$(ZLIBLIB) $(CC) -o pngtest $(CFLAGS) pngtest.o $(LDFLAGS)

test: pngtest
	./pngtest


install-headers: png.h pngconf.h
	-@if [ ! -d $(INCPATH) ]; then mkdir $(INCPATH); fi
	-@if [ ! -d $(INCPATH)/$(LIBNAME) ]; then mkdir $(INCPATH)/$(LIBNAME); fi
	-@/bin/rm -f $(INCPATH)/png.h
	-@/bin/rm -f $(INCPATH)/pngconf.h
	cp png.h pngconf.h $(INCPATH)/$(LIBNAME)
	chmod 644 $(INCPATH)/$(LIBNAME)/png.h $(INCPATH)/$(LIBNAME)/pngconf.h
	-@/bin/rm -f $(INCPATH)/png.h $(INCPATH)/pngconf.h
	-@/bin/rm -f $(INCPATH)/libpng
	ln -f -s $(INCPATH)/$(LIBNAME) $(INCPATH)/libpng

install-static: install-headers libpng.a
	-@if [ ! -d $(LIBPATH) ]; then mkdir $(LIBPATH); fi
	cp libpng.a $(LIBPATH)/$(LIBNAME).a
	chmod 644 $(LIBPATH)/$(LIBNAME).a
	-@/bin/rm -f $(LIBPATH)/libpng.a
	ln -f -s $(LIBPATH)/$(LIBNAME).a $(LIBPATH)/libpng.a

install-shared: install-headers $(LIBNAME).so.$(PNGVER)
	-@if [ ! -d $(LIBPATH) ]; then mkdir $(LIBPATH); fi
	-@/bin/rm -f $(LIBPATH)/$(LIBNAME).so.$(PNGMAJ)* $(LIBPATH)/$(LIBNAME).so
	-@/bin/rm -f $(LIBPATH)/libpng.so
	-@/bin/rm -f $(LIBPATH)/libpng.so.3
	-@/bin/rm -f $(LIBPATH)/libpng.so.3.*
	cp $(LIBNAME).so.$(PNGVER) $(LIBPATH)
	chmod 755 $(LIBPATH)/$(LIBNAME).so.$(PNGVER)
	(cd $(LIBPATH); \
	ln -f -s $(LIBNAME).so.$(PNGVER) libpng.so; \
	ln -f -s $(LIBNAME).so.$(PNGVER) libpng.so.3; \
	ln -f -s $(LIBNAME).so.$(PNGVER) libpng.so.3.$(PNGMIN); \
	ln -f -s $(LIBNAME).so.$(PNGVER) $(LIBNAME).so.$(PNGMAJ); \
	ln -f -s $(LIBNAME).so.$(PNGMAJ) $(LIBNAME).so)
	-@if [ ! -d $(LIBPATH)/pkgconfig ]; then mkdir $(LIBPATH)/pkgconfig; fi
	cat scripts/libpng.pc.in | sed -e s\!@PREFIX@!$(prefix)! > libpng.pc
	cp libpng.pc $(LIBPATH)/pkgconfig/libpng12.pc
	chmod 644 $(LIBPATH)/pkgconfig/libpng12.pc

install: install-static install-shared

clean:
	/bin/rm -f *.o libpng.a $(LIBNAME).so $(LIBNAME).so.$(PNGMAJ)* pngtest pngout.png

DOCS = ANNOUNCE CHANGES INSTALL KNOWNBUG LICENSE README TODO Y2KINFO
writelock:
	chmod a-w *.[ch35] $(DOCS) scripts/*

# DO NOT DELETE THIS LINE -- make depend depends on it.

png.o png.pic.o: png.h pngconf.h
pngerror.o pngerror.pic.o: png.h pngconf.h
pngrio.o pngrio.pic.o: png.h pngconf.h
pngwio.o pngwio.pic.o: png.h pngconf.h
pngmem.o pngmem.pic.o: png.h pngconf.h
pngset.o pngset.pic.o: png.h pngconf.h
pngget.o pngget.pic.o: png.h pngconf.h
pngread.o pngread.pic.o: png.h pngconf.h
pngrtran.o pngrtran.pic.o: png.h pngconf.h
pngrutil.o pngrutil.pic.o: png.h pngconf.h
pngtrans.o pngtrans.pic.o: png.h pngconf.h
pngwrite.o pngwrite.pic.o: png.h pngconf.h
pngwtran.o pngwtran.pic.o: png.h pngconf.h
pngwutil.o pngwutil.pic.o: png.h pngconf.h
pngpread.o pngpread.pic.o: png.h pngconf.h

pngtest.o: png.h pngconf.h
