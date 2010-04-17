#!/usr/bin/make -f
# pnglibconf.mak - standard make lines for pnglibconf.h
# 
# These lines are copied from Makefile.am, they illustrate
# how to automate the build of pnglibconf.h from scripts/pnglibconf.dfa
# given 'awk' and 'sed'

# Override as appropriate:
AWK = awk
SED = sed
CPP = $(CC) -E
COPY = cp
DELETE = rm -f
ECHO = echo

# CPPFLAGS should contain the options to control the result,
# but DEFS and CFLAGS are also supported here, override
# as appropriate
DFNFLAGS = $(DEFS) $(CPPFLAGS) $(CFLAGS)

# The standard pnglibconf.h exists as scripts/pnglibconf.h,
# copy this if the following doesn't work.
pnglibconf.h: pnglibconf.dfn
	$(DELETE) $@ dfn.c dfn1.out dfn2.out dfn3.out
	$(ECHO) '#include "pnglibconf.dfn"' >dfn.c
	$(CPP) $(DFNFLAGS) dfn.c >dfn1.out
	$(SED) -n -e 's|^.*PNG_DEFN_MAGIC-\(.*\)-PNG_DEFN_END.*$$|\1|p'\
	    dfn1.out >dfn2.out
	$(SED) -e 's| *@@@ *||g' -e 's| *$$||' dfn2.out >dfn3.out
	$(COPY) dfn3.out $@
	$(DELETE) dfn.c dfn1.out dfn2.out dfn3.out

pnglibconf.dfn: scripts/pnglibconf.dfa scripts/options.awk
	$(DELETE) $@ dfn1.out dfn2.out
	$(AWK) -f scripts/options.awk pre=1 out=dfn1.out\
	    scripts/pnglibconf.dfa 1>&2
	$(AWK) -f scripts/options.awk pre=0 out=dfn2.out dfn1.out 1>&2
	$(COPY) dfn2.out $@
	$(DELETE) dfn1.out dfn2.out

clean-pnglibconf:
	$(DELETE) pnglibconf.h pnglibconf.dfn dfn.c dfn1.out dfn2.out dfn3.out

clean: clean-pnglibconf
