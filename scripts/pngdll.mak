# Makefile for png32bd.dll
# ------------- Borland C++ 4.5 -------------

# This makefile expects to find zlib.h and zlib32bd.lib in the
# $(ZLIBDIR) directory.

# The object files here are compiled with the "stdcall" calling convention.
# This DLL requires zlib32bd.lib to be compiled in the same way.

# Note that png32bd.dll exports the zlib functions adler32, crc32 and
# the deflate... and inflate... functions. It does not export the
# compress and uncompress functions, nor any of the gz... functions,
# since libpng does not call them.

ZLIBDIR=..\zlib112
ZLIB=zlib32bd.lib
PNGDLL=png32bd.dll

CFLAGS= -ps -O2 -C -K -N- -k- -d -3 -r- -w-par -w-aus -WDE -I$(ZLIBDIR)
CC=f:\bc45\bin\bcc32
LINKFLAGS= -Tpd -aa -c
LINK=f:\bc45\bin\tlink32
LIBDIR=f:\bc45\lib
IMPLIB=f:\bc45\bin\implib

.autodepend
.c.obj:
        $(CC) -c $(CFLAGS) $<
 
OBJ1=png.obj pngerror.obj pngget.obj pngmem.obj pngpread.obj 
OBJ2=pngread.obj pngrio.obj pngrtran.obj pngrutil.obj pngset.obj 
OBJ3=pngtrans.obj pngwio.obj pngwrite.obj pngwtran.obj pngwutil.obj

all: $(PNGDLL)

$(PNGDLL): $(OBJ1) $(OBJ2) $(OBJ3) $(ZLIBDIR)\$(ZLIB)
        $(LINK) @&&|
$(LINKFLAGS) $(LIBDIR)\c0d32 +
$(OBJ1) +
$(OBJ2) +
$(OBJ3)
$@
-x
$(ZLIBDIR)\$(ZLIB) $(LIBDIR)\import32 $(LIBDIR)\cw32
|,&&|
LIBRARY $(@B)
EXETYPE WINDOWS
CODE PRELOAD MOVEABLE DISCARDABLE
DATA PRELOAD MOVEABLE MULTIPLE
|
        $(IMPLIB) -c $(@R).lib $@

# End of makefile for png32bd.dll
