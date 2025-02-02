/* sym.c - define format of libpng.sym
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

#define PNG_EXPORTA(type, name, args, attributes)\
        PNG_DFN "@" SYMBOL_PREFIX "@@" name "@"

#include "../../png.h"
