# test
check_PROGRAMS= pngtest
pngtest_SOURCES = pngtest.c
pngtest_LDADD = libpng.la
TESTS = test-pngtest.sh
TESTS_ENVIRONMENT= srcdir=$(srcdir) 
dist_man_MANS= libpng.3 libpngpf.3 png.5
include_HEADERS= png.h pngconf.h 
bin_SCRIPTS=libpng-config libpng12-config 

#rules to build libpng
lib_LTLIBRARIES=libpng12.la libpng.la
libpng12_la_SOURCES = png.c pngset.c pngget.c pngrutil.c pngtrans.c pngwutil.c \
	pngread.c pngrio.c pngwio.c pngwrite.c pngrtran.c \
	pngwtran.c pngmem.c pngerror.c pngpread.c \
	png.h pngconf.h 
libpng_la_SOURCES = $(libpng12_la_SOURCES)
libpng12_la_LDFLAGS = -export-dynamic -version-info $(LIBPNG12_VERSION_INFO)
libpng_la_LDFLAGS = -export-dynamic -version-info $(LIBPNG3_VERSION_INFO)

#distribute headers in /usr/include/libpng/*
pkginclude_HEADERS= png.h pngconf.h

#extra source distribution files.
EXTRA_DIST= ANNOUNCE CHANGES INSTALL KNOWNBUG LICENSE README TODO Y2KINFO \
	pngtest.png pngbar.png pngnow.png pngbar.jpg autogen.sh \
	${srcdir}/projects/cbuilder5/* ${srcdir}/projects/beos/* ${srcdir}/projects/visualc6/* \
	${srcdir}/projects/wince.txt ${srcdir}/projects/netware.txt \
	${srcdir}/scripts/* \
	${srcdir}/contrib/gregbook/* \
	${srcdir}/contrib/pngminus/* \
	${srcdir}/contrib/pngsuite/* \
	${srcdir}/contrib/visupng/* \
	$(TESTS) \
	example.c libpng.txt pngasmrd.h pnggccrd.c pngvcrd.c 

CLEANFILES= pngout.png libpng.pc libpng12.pc libpng-config libpng12-config


libpng.pc:
	cat ${srcdir}/scripts/libpng.pc.in | \
	sed -e 's,[@]PREFIX[@],$(prefix),' -e 's,[@]INCLUDEDIR[@],$(includedir),' > libpng.pc

libpng12.pc: libpng.pc
	cp libpng.pc libpng12.pc

install-data-local: libpng.pc libpng12.pc
	@INSTALL@ -d @PKGCONFIGDIR@
	@INSTALL_DATA@ libpng.pc @PKGCONFIGDIR@
	@INSTALL_DATA@ libpng12.pc @PKGCONFIGDIR@

uninstall-local:
	rm -f @PKGCONFIGDIR@/libpng.pc
	rm -f @PKGCONFIGDIR@/libpng12.pc

libpng-config:
	( cat ${srcdir}/scripts/libpng-config-head.in; \
	echo prefix=\"$(prefix)\"; \
	echo I_opts=\"-I$(includedir)\"; \
	echo L_opts=\"-L$(libdir)\"; \
	echo R_opts=\"-Wl,-rpath,$(libdir)\"; \
	echo libs=\"-lpng12 -lz -lm\"; \
	cat ${srcdir}/scripts/libpng-config-body.in ) > libpng-config

libpng12-config: libpng-config
	cp libpng-config libpng12-config
