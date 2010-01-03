The xcode project has not been entirely updated to libpng-1.4.0.

It needs to *not* depend on pnggccrd.c or pngvcrd.c

It needs to PNG_NO_PEDANTIC_WARNINGS in the CFLAGS while building
the library, but not while building an application.

If an updated version is not received, this project will
be removed when libpng-1.4.0 is released.
