Microsoft Developer Studio Build File, Format Version 6.00 for
libpng 1.0.7beta15 (May 29, 2000) and zlib

Copyright (C) 2000 Simon-Pierre Cadieux
For conditions of distribution and use, see copyright notice in png.h

Assumes that libpng sources are in ..
Assumes that zlib sources have been copied to ..\..\zlib

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

   libpng1.dll          (default version, currently C code only)
   libpng1d.dll         (C code debug version)
   libpng1[a-c,e-m].dll (reserved for official versions)
   libpng1[n-z].dll     (available for private versions)
   zlib.dll             (default version)
   zlibd.dll            (debug version)

If you change anything in libpng, or select different compiler settings,
please change the library name to an unreserved name, and define
PRIVATEBUILD or SPECIALBUILD accordingly.


