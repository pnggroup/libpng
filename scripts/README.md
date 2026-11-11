Build files and scripts for libpng
----------------------------------

    autoconf/          =>  Build files for Autoconf (placeholder)
    cmake/             =>  Build files for CMake
    generators/        =>  Generator programs
    pnglibconf/        =>  Automation for library feature configurations

Makefiles for libpng
--------------------

    makefile.amiga     =>  Amiga makefile
    makefile.atari     =>  Atari makefile
    makefile.clang     =>  Generic clang makefile
    makefile.emcc      =>  Emscripten makefile
    makefile.gcc       =>  Generic gcc makefile
    makefile.ibmc      =>  IBM C/C++ version 3.x for Win32 and OS/2
    makefile.intel     =>  Intel C/C++ version 4.0 and later
    makefile.riscos    =>  Acorn RISCOS makefile
    makefile.std       =>  Generic UNIX makefile
    makefile.vcwin32       =>  Microsoft Visual C++ for Windows/x86
    makefile.vcwin-arm64   =>  Microsoft Visual C++ for Windows/ARM64
    makevms.com        =>  VMS build script
    descrip.mms        =>  VMS makefile for MMS or MMK
    smakefile.ppc      =>  AMIGA smakefile for SAS C V6.58/7.00 PPC compiler
                           (Requires SCOPTIONS, copied from SCOPTIONS.ppc)
    SCOPTIONS.ppc      =>  Used with smakefile.ppc

Other supporting files
----------------------

    macro.lst          =>  List of function-like macros renamed by PNG_PREFIX
    symbols.def        =>  List of exported symbols
    README.md          =>  This file
