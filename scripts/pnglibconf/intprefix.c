/* intprefix.c - generate an unprefixed internal symbol list
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

#define PNG_INTERNAL_DATA(type, name, array)\
        PNG_DFN "@" name "@"

#define PNG_INTERNAL_FUNCTION(type, name, args, attributes)\
        PNG_DFN "@" name "@"

#define PNG_INTERNAL_CALLBACK(type, name, args, attributes)\
        PNG_DFN "@" name "@"

#define PNGPREFIX_H /* self generation */
#include "../../pngpriv.h"
