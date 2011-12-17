
/* bsconfig.h - Build System CONFIGuration
 *
 * For conditions of distribution and use, see copyright notice in png.h
 * Copyright (c) 1998-2011 Glenn Randers-Pehrson
 * (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
 * (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)
 *
 * Last changed in libpng 1.6.0 [(PENDING RELEASE)]
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */

/* This file contains includes that provide information itself provided by the
 * environment that builds libpng.  This file is included by any source file
 * that needs build-system specific information, including test programs that
 * are not, themselves, installed.
 *
 * It is provided solely to work round shortcomings in existing build systems;
 * do not *define* anything in this file, simply include other files as
 * required.
 */

/* autotools support */
/* autotools misuse -I to support separate build and source directories, to work
 * round this the autotools generated file "config.h" is included by this file.
 *
 * config.h is created by the "configure" script which also sets HAVE_CONFIG_H.
 * It is used to enable optimizations that can only be detected at build time by
 * the configure script.
 */
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
