/* symbols.c - find all exported symbols
 *
 * Copyright (c) 2025 Cosmin Truta
 * Copyright (c) 2011-2014 Glenn Randers-Pehrson
 * Originally written by John Bowler, 2011
 *
 * Use, modification and distribution are subject to
 * the same licensing terms and conditions as libpng.
 * Please see the copyright notice in png.h or visit
 * http://libpng.org/pub/png/src/libpng-LICENSE.txt
 *
 * SPDX-License-Identifier: libpng-2.0
 */

/* NOTE:
 * Making 'symbols.chk' checks both that the exported symbols in the library
 * don't change, and (implicitly) that pnglibconf.h.prebuilt is as expected.
 * If pnglibconf.h.prebuilt is remade using scripts/pnglibconf/pnglibconf.dfa,
 * then this checks the .dfa file too.
 */

#define PNG_EXPORTA(type, name, args, attributes)\
        PNG_DFN "@" name "@"
#define PNG_REMOVED(type, name, args, attributes)\
        PNG_DFN "; @" name "@"

/* Read the defaults, but use pnglibconf.h.prebuilt, i.e.,
 * the 'standard' header file.
 */
#include "../../pnglibconf.h.prebuilt"
#include "../../png.h"

/* Some things are turned off by default.  Turn these things
 * on here (by hand) to get the APIs they expose and validate
 * that no harm is done.  This list is the set of options
 * defaulted to 'off' in scripts/pnglibconf/pnglibconf.dfa
 *
 * MAINTAINER NOTE:
 * If scripts/pnglibconf/pnglibconf.dfa options are changed
 * from/to 'disabled', this needs updating.
 */
#define PNG_BENIGN_ERRORS_SUPPORTED
#define PNG_ERROR_NUMBERS_SUPPORTED
#define PNG_READ_BIG_ENDIAN_SUPPORTED  /* should do nothing! */
#define PNG_INCH_CONVERSIONS_SUPPORTED
#define PNG_READ_16_TO_8_ACCURATE_SCALE_SUPPORTED
#define PNG_SET_OPTION_SUPPORTED

#undef PNG_H
#include "../../png.h"

/* Finally there are a couple of places where option support
 * actually changes the APIs revealed using a #if/#else/#endif
 * test in png.h, test these here.
 */
#undef  PNG_FLOATING_POINT_SUPPORTED /* Exposes 'fixed' APIs */
#undef  PNG_ERROR_TEXT_SUPPORTED     /* Exposes unsupported APIs */

#undef PNG_H
#include "../../png.h"
