/* prefix.c - generate an unprefixed symbol list
 *
 * Copyright (c) 2025 Cosmin Truta
 * Copyright (c) 2013-2014 Glenn Randers-Pehrson
 * Originally written by John Bowler, 2013
 *
 * Use, modification and distribution are subject to
 * the same licensing terms and conditions as libpng.
 * Please see the copyright notice in png.h or visit
 * http://libpng.org/pub/png/src/libpng-LICENSE.txt
 *
 * SPDX-License-Identifier: libpng-2.0
 */

#define PNG_EXPORTA(type, name, args, attributes)\
        PNG_DFN "@" name "@"

/* The configuration information *before* the additional of symbol renames,
 * the list is the C name list; no symbol prefix.
 */
#include "pnglibconf.out"

PNG_DFN_START_SORT 1

#include "../../png.h"

PNG_DFN_END_SORT
