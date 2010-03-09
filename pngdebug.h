/* pngdebug.h
 *
 * Debugging macros, also used in pngtest.c
 */

/* Define PNG_DEBUG at compile time for debugging information.  Higher
 * numbers for PNG_DEBUG mean more debugging information.  This has
 * only been added since version 0.95 so it is not implemented throughout
 * libpng yet, but more support will be added as needed.
 */
#ifdef PNG_DEBUG
#  if (PNG_DEBUG > 0)
#    if !defined(PNG_DEBUG_FILE) && defined(_MSC_VER)
#      include <crtdbg.h>
#      if (PNG_DEBUG > 1)
#        ifndef _DEBUG
#          define _DEBUG
#        endif
#        ifndef png_debug
#          define png_debug(l,m)  _RPT0(_CRT_WARN,m PNG_STRING_NEWLINE)
#        endif
#        ifndef png_debug1
#          define png_debug1(l,m,p1)  _RPT1(_CRT_WARN,m PNG_STRING_NEWLINE,p1)
#        endif
#        ifndef png_debug2
#          define png_debug2(l,m,p1,p2) \
             _RPT2(_CRT_WARN,m PNG_STRING_NEWLINE,p1,p2)
#        endif
#      endif
#    else /* PNG_DEBUG_FILE || !_MSC_VER */
#      ifndef PNG_DEBUG_FILE
#        define PNG_DEBUG_FILE stderr
#      endif /* PNG_DEBUG_FILE */

#      if (PNG_DEBUG > 1)
/* Note: ["%s"m PNG_STRING_NEWLINE] probably does not work on
 * non-ISO compilers
 */
#        ifdef __STDC__
#          ifndef png_debug
#            define png_debug(l,m) \
       { \
       int num_tabs=l; \
       fprintf(PNG_DEBUG_FILE,"%s"m PNG_STRING_NEWLINE,(num_tabs==1 ? "\t" : \
         (num_tabs==2 ? "\t\t":(num_tabs>2 ? "\t\t\t":"")))); \
       }
#          endif
#          ifndef png_debug1
#            define png_debug1(l,m,p1) \
       { \
       int num_tabs=l; \
       fprintf(PNG_DEBUG_FILE,"%s"m PNG_STRING_NEWLINE,(num_tabs==1 ? "\t" : \
         (num_tabs==2 ? "\t\t":(num_tabs>2 ? "\t\t\t":""))),p1); \
       }
#          endif
#          ifndef png_debug2
#            define png_debug2(l,m,p1,p2) \
       { \
       int num_tabs=l; \
       fprintf(PNG_DEBUG_FILE,"%s"m PNG_STRING_NEWLINE,(num_tabs==1 ? "\t" : \
         (num_tabs==2 ? "\t\t":(num_tabs>2 ? "\t\t\t":""))),p1,p2); \
       }
#          endif
#        else /* __STDC __ */
#          ifndef png_debug
#            define png_debug(l,m) \
       { \
       int num_tabs=l; \
       char format[256]; \
       snprintf(format,256,"%s%s%s",(num_tabs==1 ? "\t" : \
         (num_tabs==2 ? "\t\t":(num_tabs>2 ? "\t\t\t":""))), \
         m,PNG_STRING_NEWLINE); \
       fprintf(PNG_DEBUG_FILE,format); \
       }
#          endif
#          ifndef png_debug1
#            define png_debug1(l,m,p1) \
       { \
       int num_tabs=l; \
       char format[256]; \
       snprintf(format,256,"%s%s%s",(num_tabs==1 ? "\t" : \
         (num_tabs==2 ? "\t\t":(num_tabs>2 ? "\t\t\t":""))), \
         m,PNG_STRING_NEWLINE); \
       fprintf(PNG_DEBUG_FILE,format,p1); \
       }
#          endif
#          ifndef png_debug2
#            define png_debug2(l,m,p1,p2) \
       { \
       int num_tabs=l; \
       char format[256]; \
       snprintf(format,256,"%s%s%s",(num_tabs==1 ? "\t" : \
         (num_tabs==2 ? "\t\t":(num_tabs>2 ? "\t\t\t":""))), \
         m,PNG_STRING_NEWLINE); \
       fprintf(PNG_DEBUG_FILE,format,p1,p2); \
       }
#          endif
#        endif /* __STDC __ */
#      endif /* (PNG_DEBUG > 1) */

#    endif /* _MSC_VER */
#  endif /* (PNG_DEBUG > 0) */
#endif /* PNG_DEBUG */
#ifndef png_debug
#  define png_debug(l, m)
#endif
#ifndef png_debug1
#  define png_debug1(l, m, p1)
#endif
#ifndef png_debug2
#  define png_debug2(l, m, p1, p2)
#endif
