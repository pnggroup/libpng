# Makefile.am:
#   Source file for Makefile.in (and hence Makefile)
#
# Makefile.am need only be changed on a major version number
# change (e.g. libpng12 --> libpng13).  In that case seach
# this file for every instance of the old base name (libpng12)
# and change to the new one (libpng13), then change the
# -version-number settings below so that the new values have
# the correct major part (first field).

PNGLIB_BASENAME= libpng@PNGLIB_MAJOR@

# libpng does not follow GNU file name conventions
AUTOMAKE_OPTIONS = foreign

# test programs - run on make check, make distcheck
check_PROGRAMS= pngtest
pngtest_SOURCES = pngtest.c
pngtest_LDADD = libpng.la
TESTS = test-pngtest.sh
TESTS_ENVIRONMENT= srcdir=$(srcdir) 

# man pages
dist_man_MANS= libpng.3 libpngpf.3 png.5

# generate the -config scripts if required
binconfigs= libpng-config libpng12-config 
EXTRA_SCRIPTS= libpng-config libpng12-config 
bin_SCRIPTS= @binconfigs@

# rules to build libpng, only build the old library on request
lib_LTLIBRARIES=libpng12.la @compatlib@
EXTRA_LTLIBRARIES= libpng.la
libpng12_la_SOURCES = png.c pngset.c pngget.c pngrutil.c pngtrans.c pngwutil.c \
	pngread.c pngrio.c pngwio.c pngwrite.c pngrtran.c \
	pngwtran.c pngmem.c pngerror.c pngpread.c \
	png.h pngconf.h 
libpng_la_SOURCES = $(libpng12_la_SOURCES)
# MAJOR UPGRADE: the version-number settings below must be changed.
libpng12_la_LDFLAGS = -no-undefined -export-dynamic \
	-version-number 0:@PNGLIB_MINOR@:0
# -rpath is needed as automake doesn't know the directory
libpng_la_LDFLAGS = -rpath '$(libdir)' -no-undefined -export-dynamic \
	-version-number 3:@PNGLIB_MINOR@:0

#distribute headers in /usr/include/libpng/*
pkgincludedir= $(includedir)/$(PNGLIB_BASENAME)
pkginclude_HEADERS= png.h pngconf.h

# pkg-config stuff, note that libpng.pc is always required in order
# to get the correct library
pkgconfigdir = @pkgconfigdir@
pkgconfig_DATA = libpng.pc libpng12.pc

#extra source distribution files.
EXTRA_DIST= \
	ANNOUNCE ChangeLog INSTALL KNOWNBUG LICENSE README TODO Y2KINFO \
	pngtest.png pngbar.png pngnow.png pngbar.jpg autogen.sh \
	${srcdir}/projects/cbuilder5/* \
	${srcdir}/projects/beos/* \
	${srcdir}/projects/visualc6/* \
	${srcdir}/projects/visualc71/* \
	${srcdir}/projects/wince.txt \
	${srcdir}/projects/netware.txt \
	${srcdir}/scripts/* \
	${srcdir}/contrib/gregbook/* \
	${srcdir}/contrib/pngminus/* \
	${srcdir}/contrib/pngsuite/* \
	${srcdir}/contrib/visupng/* \
	$(TESTS) \
	example.c libpng.txt pnggccrd.c pngvcrd.c 

CLEANFILES= pngout.png libpng12.pc libpng12-config

$(PNGLIB_BASENAME).pc: libpng.pc
	cp libpng.pc $@

$(PNGLIB_BASENAME)-config: libpng-config
	cp libpng-config $@

# install the .../include headers as links to the new ones
install-data-hook:
	cd $(DESTDIR)$(includedir); rm -f png.h pngconf.h
	cd $(DESTDIR)$(includedir); $(LN_S) $(PNGLIB_BASENAME)/png.h png.h
	cd $(DESTDIR)$(includedir); $(LN_S) $(PNGLIB_BASENAME)/pngconf.h pngconf.h

# do evil things to libpng to cause libpng12 to be used, if
# the compat library is not being built do nothing.
install-exec-hook:
	@if test -n "@compatlib@"; then\
		set -x;\
		cd $(DESTDIR)$(libdir);\
		for ext in a la so; do\
			rm -f libpng.$$ext;\
			$(LN_S) $(PNGLIB_BASENAME).$$ext libpng.$$ext;\
		done;\
	fi
