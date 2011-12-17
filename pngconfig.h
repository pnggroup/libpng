/* pngconf.h
 *
 * libpng version 1.6.0beta02 - December 17, 2011
 *
 * Copyright (c) 2011 Glenn Randers-Pehrson
 * Written by John Bowler.
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * config.h is created by and PNG_CONFIGURE_LIBPNG is set by the "configure"
 * script.  We may need it here to get the correct configuration on things
 * like limits.
 */

#ifndef PNGCONFIG_H
#define PNGCONFIG_H

#ifdef PNG_CONFIGURE_LIBPNG
#  ifdef HAVE_CONFIG_H
#    include "config.h"
#  endif
#endif

#endif /* PNGCONFIG_H */
