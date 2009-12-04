The cbuilder5 project has not been updated to libpng-1.4.0.

It needs to depend on pngpriv.h

It needs to *not* depend on pnggccrd.c or pngvcrd.c

It needs to DEFINE PNG_NO_PEDANTIC_WARNING while building
the library, but not while building an application.

If an updated version is not received, this project will
be removed when libpng-1.4.0 is released.
