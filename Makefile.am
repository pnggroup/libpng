## -*- makefile -*-
##
## Process this file with automake to produce Makefile.in
##
## $Id: Makefile.am,v 1.8 2001/05/16 15:25:29 cmdjb Exp $
##
## Copyright 1998-2000, Dave Beckett, ILRT, University of Bristol
## http://purl.org/net/dajobe/
##

CFLAGS=@CFLAGS@

bin_PROGRAMS = pngmeta

man_MANS = pngmeta.1

AUTOMAKE_OPTIONS=foreign no-dependencies

EXTRA_DIST=test1 test2 test3 test4 \
	   test1.xpt test2.xpt test3.xpt test4.xpt \
	   8859.png ctzn0g04.png ct1n0g04.png \
	   pngmeta.1 Makefile.old pngmeta.spec \
	   autogen.sh

man_MANS=pngmeta.1

TESTS=test1 test2 test3 test4

test-results: pngmeta
	./pngmeta -quiet ct1n0g04.png > test1.xpt
	./pngmeta -soif -quiet ctzn0g04.png > test2.xpt
	./pngmeta -html -quiet 8859.png > test3.xpt
	./pngmeta -xrdf ct1n0g04.png > test4.xpt

MAINTAINERCLEANFILES=test1.xpt test2.xpt test3.xpt test4.xpt
