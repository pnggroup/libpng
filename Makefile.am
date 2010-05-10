## -*- makefile -*-
##
## Process this file with automake to produce Makefile.in
##
## $Source: /home/cur/djb1/develop/pngmeta/pngmeta/RCS/Makefile.am,v $
##
## $Id: Makefile.am,v 1.5 2000/01/31 14:15:53 djb1 Exp $
##
## (C) Copyright 1998, Dave Beckett, University of Kent at Canterbury
##

INCLUDES=-I$(PNG_INCLUDE) -I$(ZLIB_INCLUDE)

bin_PROGRAMS = pngmeta

AUTOMAKE_OPTIONS=foreign no-dependencies

EXTRA_DIST=test1 test2 test3 test4\
	   test1.xpt test2.xpt test3.xpt test4.xpt\
	   8859.png ctzn0g04.png ct1n0g04.png\
	   pngmeta.1 Makefile.old pngmeta.spec

man_MANS=pngmeta.1

TESTS=test1 test2 test3 test4

test-results: pngmeta
	./pngmeta -quiet ct1n0g04.png > test1.xpt
	./pngmeta -soif -quiet ctzn0g04.png > test2.xpt
	./pngmeta -html -quiet 8859.png > test3.xpt
	./pngmeta -xrdf ct1n0g04.png > test4.xpt

MAINTAINERCLEANFILES=test1.xpt test2.xpt test3.xpt test4.xpt
