/*===
cexcept.h 0.6.0 (2000-Apr-07-Fri)
Adam M. Costello <amc@cs.berkeley.edu>

An interface for exception-handling in ANSI C, developed jointly with
Cosmin Truta <cosmin@cs.toronto.edu>.

    Copyright (c) 2000 Adam M. Costello and Cosmin Truta.  Everyone
    is hereby granted permission to do whatever they like with this
    file, provided that if they modify it they take reasonable steps to
    avoid confusing or misleading people about the authors, version,
    and terms of use of the derived file.  The copyright holders make
    no guarantees about the correctness of this file, and are not
    responsible for any damage resulting from its use.

Comments have been stripped from this copy; if you are interested in cexcept
you should look at the most recent version, which this probably isn't.

For recent versions of cexcept.h with documentation and examples, go to
http://www.cs.berkeley.edu/~amc/cexcept/ or http://cexcept.sourceforge.net/

===*/


#ifndef CEXCEPT_H
#define CEXCEPT_H


#include <setjmp.h>

#define define_exception_type(etype) \
struct exception__state { \
  etype *exception; \
  jmp_buf env; \
}

struct exception_context { \
  struct exception__state *last; \
  int caught; \
};

#define init_exception_context(ec) ((void)((ec)->last = 0))

#define Catch(e) exception__catch(&(e))
#define Catch_anonymous exception__catch(0)

#define Try \
  { \
    struct exception__state *exception__p, exception__s; \
    int exception__i; \
    exception__p = the_exception_context->last; \
    the_exception_context->last = &exception__s; \
    for (exception__i = 0; ; exception__i = 1) \
      if (exception__i) { \
        if (setjmp(exception__s.env) == 0) { \
          if (&exception__s)

#define exception__catch(e_addr) \
          else { } \
          the_exception_context->caught = 0; \
        } \
        else the_exception_context->caught = 1; \
        the_exception_context->last = exception__p; \
        break; \
      } \
      else exception__s.exception = e_addr; \
  } \
  if (!the_exception_context->caught) { } \
  else

#define Throw \
  for (;; longjmp(the_exception_context->last->env, 1)) \
    if (the_exception_context->last->exception) \
      *the_exception_context->last->exception =


#endif /* CEXCEPT_H */
