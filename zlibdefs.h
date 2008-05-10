/* zlibdefs.h -- compile-time definitions for the zlib compression library
 * Copyright (C) 1995-2006 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include <sys/types.h>	/* for off_t */
#include <unistd.h>	/* for SEEK_* and off_t */
#ifdef VMS
#  include <unixio.h>	/* for off_t */
#endif
#ifndef z_off_t
#  define z_off_t off_t
#endif
