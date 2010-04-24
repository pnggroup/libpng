Microsoft Developer Studio Project File, Format Version 7.10 for libpng.

Copyright (C) 2004 Simon-Pierre Cadieux.

This code is released under the libpng license.
For conditions of distribution and use, see copyright notice in png.h

Changes in libpng 15:
* The DEF file is no longer required, the DLLs link by name.  A standard
  DEF file, pngwin.def, is included in the distribution for applications
  that require link by number (ordinal), builds that require link by
  number should use the ordinals from this file (or from png.h).

Assumptions:
* The libpng source files are in ..\..
* The zlib source files are in ..\..\..\zlib
  The source files must be from zlib version 1.2.3 - this project file
  will not work with any other version.
* The zlib project file is in .

You may delete the zlib project from the solution and use the official
zlib build instead - take care to link against either zlib.lib or zdll.lib
as appropriate.  You must do this if you want to use a version of zlib other
than zlib-1.2.3!

To use:

1) On the main menu, select "File | Open Solution".
   Open "libpng.sln".

2) Display the Solution Explorer view (Ctrl+Alt+L)

3) Set one of the project as the StartUp project. If you just want to build the
   binaries set "libpng" as the startup project (Select "libpng" tree view
   item + Project | Set as StartUp project). If you want to build and test the
   binaries set it to "pngtest" (Select "pngtest" tree view item +
   Project | Set as StartUp project)

4) Select "Build | Configuration Manager...".
   Choose the configuration you wish to build.

5) Select "Build | Clean Solution".

6) Select "Build | Build Solution (Ctrl-Shift-B)"

This project builds the libpng binaries as follows:

* Win32_DLL_Release\libpng15.dll      DLL build
* Win32_DLL_Debug\libpng15d.dll       DLL build (debug version)
* Win32_DLL_VB\libpng15vb.dll         DLL build for Visual Basic (no stdio)
* Win32_LIB_Release\libpng.lib        static build
* Win32_LIB_Debug\libpngd.lib         static build (debug version)

Notes:

If you change anything in the source files, or select different compiler
settings, please change the DLL name to something different than any of
the above names. Also, make sure that in your "pngusr.h" you define
PNG_USER_PRIVATEBUILD and PNG_USER_DLLFNAME_POSTFIX according to the
instructions provided in "pngconf.h".

All DLLs built by this project use the Microsoft dynamic C runtime library
MSVCR71.DLL (MSVCR71D.DLL for debug versions).  If you distribute any of the
above mentioned libraries you may have to include this DLL in your package.
For a list of files that are redistributable in Visual Studio see
$(VCINSTALLDIR)\redist.txt.
