#! /bin/sh
# a quick hack script to generate necessary files from 
# auto* tools.

libtoolize -c -f && aclocal && autoheader && automake --foreign -a -c && autoconf
