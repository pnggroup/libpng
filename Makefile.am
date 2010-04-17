# Makefile.am:
#   Source file for Makefile.in (and hence Makefile)
#

PNGLIB_BASENAME= libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@

# libpng does not follow GNU file name conventions
AUTOMAKE_OPTIONS = foreign color-tests

# test programs - run on make check, make distcheck
check_PROGRAMS= pngtest
pngtest_SOURCES = pngtest.c
pngtest_LDADD = libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@.la
TESTS = test-pngtest.sh
TESTS_ENVIRONMENT= srcdir=$(srcdir) 

# man pages
dist_man_MANS= libpng.3 libpngpf.3 png.5

# generate the -config scripts if required
binconfigs= libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@-config 
EXTRA_SCRIPTS= libpng-config libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@-config
bin_SCRIPTS= @binconfigs@

# rules to build libpng, only build the old library on request
lib_LTLIBRARIES=libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@.la
# EXTRA_LTLIBRARIES= libpng.la
libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@_la_SOURCES = png.c pngset.c pngget.c pngrutil.c \
	pngtrans.c pngwutil.c \
	pngread.c pngrio.c pngwio.c pngwrite.c pngrtran.c \
	pngwtran.c pngmem.c pngerror.c pngpread.c \
	png.h pngconf.h pnglibconf.h pngpriv.h pngstruct.h pnginfo.h pngdebug.h

nodist_libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@_la_SOURCES = pnglibconf.h

libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@_la_CPPFLAGS = @LIBPNG_DEFINES@

libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@_la_LDFLAGS = -no-undefined -export-dynamic \
	-version-number @PNGLIB_MAJOR@@PNGLIB_MINOR@:@PNGLIB_RELEASE@:0

if HAVE_LD_VERSION_SCRIPT
  # Versioned symbols and restricted exports
  libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@_la_LDFLAGS += -Wl,--version-script=libpng.vers
  libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@_la_DEPENDENCIES = libpng.vers
else
  # Only restricted exports when possible
  libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@_la_LDFLAGS += -export-symbols libpng.sym
  libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@_la_DEPENDENCIES = libpng.sym
endif

#distribute headers in /usr/include/libpng/*
pkgincludedir= $(includedir)/$(PNGLIB_BASENAME)
pkginclude_HEADERS= png.h pngconf.h
nodist_pkginclude_HEADERS= pnglibconf.h

# pkg-config stuff, note that libpng.pc is always required in order
# to get the correct library
pkgconfigdir = @pkgconfigdir@
pkgconfig_DATA = libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@.pc

#extra source distribution files.
EXTRA_DIST= \
	ANNOUNCE CHANGES INSTALL LICENSE README TODO \
	pngtest.png pngbar.png pngnow.png pngbar.jpg autogen.sh \
	${srcdir}/projects/visualc6/* \
	${srcdir}/projects/visualc71/* \
	${srcdir}/projects/xcode/* \
	${srcdir}/scripts/* \
	${srcdir}/contrib/gregbook/* \
	${srcdir}/contrib/pngminim/* \
	${srcdir}/contrib/pngminus/* \
	${srcdir}/contrib/pngsuite/* \
	${srcdir}/contrib/visupng/* \
	$(TESTS) \
	CMakeLists.txt example.c libpng-@PNGLIB_VERSION@.txt

SCRIPT_CLEANFILES=scripts/*.out scripts/*.chk scripts/pnglibconf.dfn

CLEANFILES= dfn.c dfn?.out pngout.png libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@.pc \
	libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@-config libpng.vers libpng.sym \
	pngwin.def check.new pnglibconf.* symbols.new \
	$(SCRIPT_CLEANFILES)

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess config.h.in \
config.sub configure depcomp install-sh ltmain.sh missing

SUFFIXES = .chk .dfn .out

$(PNGLIB_BASENAME).pc: libpng.pc
	cp libpng.pc $@

$(PNGLIB_BASENAME)-config: libpng-config
	cp libpng-config $@

scripts/sym.out scripts/vers.out: png.h pngconf.h pnglibconf.h
scripts/symbols.out scripts/pngwin.out: png.h pngconf.h scripts/pnglibconf.h

libpng.sym: scripts/sym.out
	rm -f $@
	cp $? $@
libpng.vers: scripts/vers.out
	rm -f $@
	cp $? $@
pngwin.def: scripts/pngwin.out
	rm -f $@
	cp $? $@
pnglibconf.h: pnglibconf.out
	rm -f $@
	cp $? $@
scripts/pnglibconf.h:
	@echo "This is a machine generated file, but if you want to make" >&2
	@echo "a new one simply make 'scripts/pnglibconf.out' and copy that" >&2
	@exit 1

SYMBOL_CFLAGS = -DPNGLIB_LIBNAME='PNG@PNGLIB_MAJOR@@PNGLIB_MINOR@_0'\
		-DPNGLIB_VERSION='@PNGLIB_VERSION@'\
		-DSYMBOL_PREFIX='$(SYMBOL_PREFIX)'

.dfn.out:
	rm -f $@ dfn.c dfn?.out
	test -d scripts || mkdir scripts
	echo '#include "$<"' >dfn.c
	$(CPP) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) @LIBPNG_DEFINES@\
	    $(CPPFLAGS) $(SYMBOL_CFLAGS) dfn.c >dfn1.out
	$(SED) -n -e 's|^.*PNG_DEFN_MAGIC-\(.*\)-PNG_DEFN_END.*$$|\1|p'\
	    dfn1.out >dfn2.out
	$(SED) -e 's| *@@@ *||g' -e 's| *$$||' dfn2.out >dfn3.out
	rm -f dfn.c dfn[12].out
	mv dfn3.out $@

# The .dfn file for pnglibconf.h is machine generated
pnglibconf.dfn: scripts/pnglibconf.dfa scripts/options.awk
	rm -f $@ dfn?.out
	$(AWK) -f ${srcdir}/scripts/options.awk pre=1 out=dfn1.out\
	    ${srcdir}/scripts/pnglibconf.dfa 1>&2
	$(AWK) -f ${srcdir}/scripts/options.awk pre=0 out=dfn2.out\
	    dfn1.out 1>&2
	rm dfn1.out
	mv dfn2.out $@

# Symbol checks (.def and .out files should match)
scripts/pngwin.chk: scripts/checksym.awk scripts/pngwin.def scripts/pngwin.out
scripts/symbols.chk: scripts/checksym.awk scripts/symbols.def scripts/symbols.out
.out.chk:
	rm -f $@ symbols.new
	$(AWK) -f ${srcdir}/scripts/checksym.awk ${srcdir}/scripts/${*F}.def\
	    $< >&2
	mv symbols.new $@

# used on demand to regenerate the standard header, CPPFLAGS should
# be empty - no non-standard defines
scripts/pnglibconf.dfn: scripts/pnglibconf.dfa scripts/options.awk
	rm -f $@ dfn?.out
	test -z "$(CPPFLAGS)" 
	echo "com @PNGLIB_VERSION@ STANDARD API DEFINITION" |\
	$(AWK) -f ${srcdir}/scripts/options.awk pre=1 out=dfn1.out\
	    logunsupported=1 - ${srcdir}/scripts/pnglibconf.dfa 1>&2
	$(AWK) -f ${srcdir}/scripts/options.awk pre=0 out=dfn2.out\
	    logunsupported=1 dfn1.out 1>&2
	rm dfn1.out
	mv dfn2.out $@

$(libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@_la_OBJECTS): png.h pngconf.h \
	pnglibconf.h pngpriv.h pngdebug.h pnginfo.h pngstruct.h

test: check-am

# Extra checks
check: scripts/symbols.chk scripts/pngwin.chk

# Don't distribute the generated script files
dist-hook:
	cd '$(top_distdir)'; rm -f $(SCRIPT_CLEANFILES)

# install the .../include headers as links to the new ones
install-data-hook:
	cd $(DESTDIR)$(includedir); rm -f png.h pngconf.h pnglibconf.h
	cd $(DESTDIR)$(includedir); $(LN_S) $(PNGLIB_BASENAME)/png.h png.h
	cd $(DESTDIR)$(includedir); $(LN_S) $(PNGLIB_BASENAME)/pngconf.h \
		pngconf.h
	cd $(DESTDIR)$(includedir); $(LN_S) $(PNGLIB_BASENAME)/pnglibconf.h \
		pnglibconf.h
	cd $(DESTDIR)$(pkgconfigdir); rm -f libpng.pc
	cd $(DESTDIR)$(pkgconfigdir); $(LN_S) $(PNGLIB_BASENAME).pc libpng.pc

# do evil things to libpng to cause libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@ to be used
install-exec-hook:
	cd $(DESTDIR)$(bindir); rm -f libpng-config
	cd $(DESTDIR)$(bindir); $(LN_S) $(PNGLIB_BASENAME)-config libpng-config
	@set -x;\
	cd $(DESTDIR)$(libdir);\
	for ext in a la so so.@PNGLIB_MAJOR@@PNGLIB_MINOR@.@PNGLIB_RELEASE@ sl dylib; do\
		rm -f libpng.$$ext;\
                if test -f $(PNGLIB_BASENAME).$$ext; then\
                       $(LN_S) $(PNGLIB_BASENAME).$$ext libpng.$$ext;\
                fi;\
	done

uninstall-hook:
	cd $(DESTDIR)$(includedir); rm -f png.h pngconf.h pnglibconf.h
	rm -f $(DESTDIR)$(pkgconfigdir)/libpng.pc
	rm -f $(DESTDIR)$(bindir)/libpng-config
	rm -f $(DESTDIR)$(libdir)/libpng.a
	rm -f $(DESTDIR)$(libdir)/libpng.la
