/* png_bounds_safety.h - portability macros for optional -fbounds-safety
 *
 * Copyright (c) 2026 Jeff Bindel
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * When PNG_SUPPORT_FBOUNDS_SAFETY is defined (typically via
 * -DPNG_SUPPORT_FBOUNDS_SAFETY and a Clang toolchain that implements
 * -fbounds-safety), these macros expand to Clang bounds annotations.
 * Otherwise they expand to nothing so default builds are unchanged.
 *
 * Pattern matches the libwebp utils/types bounds-safety adoption:
 * annotations are inert unless explicitly enabled.
 */

#ifndef PNG_BOUNDS_SAFETY_H
#define PNG_BOUNDS_SAFETY_H

#ifdef PNG_SUPPORT_FBOUNDS_SAFETY

#  include <ptrcheck.h>
/* Non-ABI-breaking counted-by annotations for struct pointer members and
 * parameters. Prefer __counted_by_or_null for pointers that may be NULL
 * while the companion size field is zero (libpng read_buffer pattern).
 */
#  define PNG_COUNTED_BY(n) __counted_by(n)
#  define PNG_COUNTED_BY_OR_NULL(n) __counted_by_or_null(n)

#else /* !PNG_SUPPORT_FBOUNDS_SAFETY */

#  define PNG_COUNTED_BY(n)
#  define PNG_COUNTED_BY_OR_NULL(n)

#endif /* PNG_SUPPORT_FBOUNDS_SAFETY */

#endif /* PNG_BOUNDS_SAFETY_H */
