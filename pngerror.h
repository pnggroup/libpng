/* This linked list implements a stack structure where jmpbuf context
 * is to be saved. It will allow multiple, nested calls of setjmp/longjmp...
 */

#ifndef _PNGERROR_H
#define _PNGERROR_H

#ifndef PNG_ABORT
#   define PNG_ABORT() abort()
#endif

#ifndef PNG_SETJMP_NOT_SUPPORTED
#  define PNG_SETJMP_SUPPORTED
#endif

#ifdef PNG_SETJMP_SUPPORTED

/* New feature in version 1.1.0d.  Don't undefine this; it's here just so
 * applications can test for the new version. */
#define PNG_JMPBUF_SUPPORTED

/* This is an attempt to force a single setjmp behaviour on Linux.  If
 * the X config stuff didn't define _BSD_SOURCE we wouldn't need this.
 */
#ifdef __linux__
#  ifdef _BSD_SOURCE
#    define _PNG_SAVE_BSD_SOURCE
#    undef _BSD_SOURCE
#  endif
#  ifdef _SETJMP_H
    __png.h__ already includes setjmp.h
    __dont__ include it again
#  endif
#endif /* __linux__ */

/* include setjmp.h for error handling */
#include <setjmp.h>

#ifdef __linux__
#  ifdef _PNG_SAVE_BSD_SOURCE
#    define _BSD_SOURCE
#    undef _PNG_SAVE_BSD_SOURCE
#  endif
#endif /* __linux__ */

typedef struct png_jmpbuf_struct
{
   jmp_buf env;
   struct png_jmpbuf_struct FAR * link;
} png_jmpbuf;

typedef png_jmpbuf FAR * png_jmpbufp;
typedef png_jmpbuf FAR * FAR * png_jmpbufpp;

#define png_jmp_env(png_ptr) png_get_jmpbuf(png_ptr)->env  

extern PNG_EXPORT(png_jmpbufp,png_get_jmpbuf)
   PNGARG((png_structp));

#define png_setjmp(png_ptr) setjmp(png_get_jmpbuf(png_ptr)->env)
#define png_longjmp(png_ptr,val) longjmp(png_get_jmpbuf(png_ptr)->env,val)

#endif /* PNG_SETJMP_SUPPORTED */

#endif /* _PNGERROR_H */
