Microsoft Developer Studio Build File, Format Version 6.00 for
libpng 1.2.0 (September 1, 2001) and zlib

Copyright (C) 2000 Simon-Pierre Cadieux
For conditions of distribution and use, see copyright notice in png.h

Assumes that libpng sources are in ..\..
Assumes that zlib sources have been copied to ..\..\..\zlib

To build:

1) On the main menu Select "Build|Set Active configuration". 
   Among the configurations beginning with "libpng" select the 
   one you wish to build (the corresponding "zlib" configuration
   will be built automatically).

2) Select "Build|Clean"

3) Select "Build|Rebuild All"

4) Look in the appropriate "win32" subdirectories for both "zlib"
   and "libpng" binaries.

This project will build the PNG Development Group's "official" versions of
libpng and zlib libraries:

   libpng3.dll          (default version, currently C code only)
   libpng3.dll         (C + Assembler version)
   libpng3.dll         (C + Assembler debug version)
   libpng3.dll         (C code debug version)
   libpng3[c,e-m].dll   (reserved for official versions) 
   libpng3[n-z].dll     (available for private versions)
   zlib.dll             (default version, currently C code only)
   zlibd.dll            (debug version)

If you change anything in libpng, or select different compiler settings,
please change the library name to an unreserved name, and define
DLLFNAME_POSTFIX and (PRIVATEBUILD or SPECIALBUILD) accordingly. DLLFNAME_POSTFIX
should correspond to a string in the range of "N" to "Z" depending on the letter 
you choose for your private version.

All DLLs built by this project use the Microsoft dynamic C runtime library
MSVCRT.DLL (MSVCRTD.DLL for debug versions). If you distribute any of the
above mentioned libraries you should also include this DLL in your package.
For a list of files that are redistributable in Visual C++ 6.0, see
Common\Redist\Redist.txt on Disc 1 of the Visual C++ 6.0 product CDs. 

