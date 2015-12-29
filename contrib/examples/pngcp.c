/* pngcp.c
 *
 * Copyright (c) 2015 John Cunningham Bowler
 *
 * Last changed in libpng 1.7.0 [(PENDING RELEASE)]
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * This is a minimal example of copying a PNG without changes using the
 * png_read_png and png_write_png interfaces.
 *
 * For a more extensive example that uses the transforms see
 * contrib/libtests/pngimage.c in the libpng distribution.
 */
#define _POSIX_SOURCE 1
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>

#include <unistd.h>
#include <sys/stat.h>

#if defined(HAVE_CONFIG_H) && !defined(PNG_NO_CONFIG_H)
#  include <config.h>
#endif

/* Define the following to use this test against your installed libpng, rather
 * than the one being built here:
 */
#ifdef PNG_FREESTANDING_TESTS
#  include <png.h>
#else
#  include "../../png.h"
#endif

#include <zlib.h>

#ifndef PNG_SETJMP_SUPPORTED
#  include <setjmp.h> /* because png.h did *not* include this */
#endif

#ifdef __GNUC__
   /* Many versions of GCC erroneously report that local variables unmodified
    * within the scope of a setjmp may be clobbered.  This hacks round the
    * problem (sometimes) without harming other compilers.
    */
#  define gv volatile
#else
#  define gv
#endif

#if PNG_LIBPNG_VER < 10700
   /* READ_PNG and WRITE_PNG were not defined, so: */
#  ifdef PNG_INFO_IMAGE_SUPPORTED
#     ifdef PNG_SEQUENTIAL_READ_SUPPORTED
#        define PNG_READ_PNG_SUPPORTED
#     endif /* SEQUENTIAL_READ */
#     ifdef PNG_WRITE_SUPPORTED
#        define PNG_WRITE_PNG_SUPPORTED
#     endif /* WRITE */
#  endif /* INFO_IMAGE */
#endif /* pre 1.7.0 */

#if (defined(PNG_READ_PNG_SUPPORTED)) && (defined(PNG_WRITE_PNG_SUPPORTED))
/* This structure is used to control the test of a single file. */
typedef enum
{
   VERBOSE,        /* switches on all messages */
   INFORMATION,
   WARNINGS,       /* switches on warnings */
   LIBPNG_WARNING,
   APP_WARNING,
   ERRORS,         /* just errors */
   APP_FAIL,       /* continuable error - no need to longjmp */
   LIBPNG_ERROR,   /* this and higher cause a longjmp */
   LIBPNG_BUG,     /* erroneous behavior in libpng */
   APP_ERROR,      /* such as out-of-memory in a callback */
   QUIET,          /* no normal messages */
   USER_ERROR,     /* such as file-not-found */
   INTERNAL_ERROR
} error_level;
#define LEVEL_MASK      0xf   /* where the level is in 'options' */

#define STRICT          0x010 /* Fail on warnings as well as errors */
#define LOG             0x020 /* Log pass/fail to stdout */
#define CONTINUE        0x040 /* Continue on APP_FAIL errors */
#define SIZES           0x080 /* Report input and output sizes */
#define OPTION     0x80000000 /* Used for handling options */
#define LIST       0x80000001 /* Used for handling options */

/* Result masks apply to the result bits in the 'results' field below; these
 * bits are simple 1U<<error_level.  A pass requires either nothing worse than
 * warnings (--relaxes) or nothing worse than information (--strict)
 */
#define RESULT_STRICT(r)   (((r) & ~((1U<<WARNINGS)-1)) == 0)
#define RESULT_RELAXED(r)  (((r) & ~((1U<<ERRORS)-1)) == 0)

/* OPTION DEFINITIONS */
static const char range_lo[] = "min";
static const char range_hi[] = "max";
static const char all[] = "all";
#define RANGE(lo,hi) { range_lo, lo }, { range_hi, hi }
static const struct value_list
{
   const char *name;  /* the command line name of the value */
   const int   value; /* the actual value to use */
}
#if defined(PNG_WRITE_CUSTOMIZE_COMPRESSION_SUPPORTED) ||\
    defined(PNG_WRITE_CUSTOMIZE_ZTXT_COMPRESSION_SUPPORTED)
vl_strategy[] =
{
   /* This controls the order of search and also the default (which is huffman
    * only compression):
    */
   { "fixed", Z_FIXED },
   { "huffman", Z_HUFFMAN_ONLY },
   { "RLE", Z_RLE },
   { "filtered", Z_FILTERED },
   { "default", Z_DEFAULT_STRATEGY },
   { all, 0 }
},
vl_level[] =
{
   { "default", Z_DEFAULT_COMPRESSION /* this is -1 */ },
   { "none", Z_NO_COMPRESSION },
   { "speed", Z_BEST_SPEED },
   { "best", Z_BEST_COMPRESSION },
   RANGE(0, 9),
   { all, 0 }
},
vl_windowBits[] =
{
   { "default", 15 },
   { "small", 9 },
   RANGE(8, 15),
   { all, 0 }
},
vl_memLevel[] =
{
   { "default", 8 },
   { "least", 1 },
   RANGE(1, 9),
   { all, 0 }
},
#endif /* WRITE_CUSTOMIZE_*COMPRESSION */
#ifdef PNG_WRITE_FILTER_SUPPORTED
vl_filter[] =
{
   { all,      PNG_ALL_FILTERS   },
   { "off",    PNG_NO_FILTERS    },
   { "none",   PNG_FILTER_NONE   },
   { "sub",    PNG_FILTER_SUB    },
   { "up",     PNG_FILTER_UP     },
   { "avg",    PNG_FILTER_AVG    },
   { "paeth",  PNG_FILTER_PAETH  }
},
#endif /* WRITE_FILTER */
#define SELECT_HEURISTICALLY 1
#define SELECT_METHODICALLY  2
#if defined(PNG_SELECT_FILTER_HEURISTICALLY_SUPPORTED) ||\
    defined(PNG_SELECT_FILTER_METHODICALLY_SUPPORTED)
vl_select[] =
{
   { all,   SELECT_HEURISTICALLY|SELECT_METHODICALLY },
   { "off", 0 },
#ifdef PNG_SELECT_FILTER_HEURISTICALLY_SUPPORTED
   { "heuristically", SELECT_HEURISTICALLY },
#endif /* SELECT_FILTER_HEURISTICALLY */
#ifdef PNG_SELECT_FILTER_METHODICALLY_SUPPORTED
   { "methodically", SELECT_METHODICALLY },
#endif /* SELECT_FILTER_METHODICALLY */
   { "both", SELECT_HEURISTICALLY|SELECT_METHODICALLY }
},
#endif /* SELECT_FILTER_HEURISTICALLY || SELECT_FILTER_METHODICALLY */
vl_on_off[] = { { "on", 1 }, { "off", 2 } };

static const struct option
{
   const char              *name;         /* name of the option */
   png_uint_32              opt;          /* an option, or OPTION or LIST */
   png_byte                 value_count;  /* length of the list of values: */
   const struct value_list *values;       /* values for OPTION or LIST */
}  options[] =
{
   /* struct display options, these are set when the command line is read */
#  define S(n,v) { #n, v, 2, vl_on_off },
   S(verbose,  VERBOSE)
   S(warnings, WARNINGS)
   S(errors,   ERRORS)
   S(quiet,    QUIET)
   S(strict,   STRICT)
   S(log,      LOG)
   S(continue, CONTINUE)
   S(sizes,    SIZES)
#  undef S

   /* OPTION settings, these and LIST settings are read on demand */
#  define VL(oname, name, type)\
   { oname, type, (sizeof vl_ ## name)/(sizeof vl_ ## name[0]), vl_ ## name },
#  define VLO(oname, name) VL(oname, name, OPTION)

#  ifdef PNG_WRITE_CUSTOMIZE_COMPRESSION_SUPPORTED
#     define VLCIDAT(name) VLO(#name, name)
#  else
#     define VLCIDAT(name)
#  endif /* WRITE_CUSTOMIZE_COMPRESSION */

#  ifdef PNG_WRITE_CUSTOMIZE_ZTXT_COMPRESSION_SUPPORTED
#     define VLCzTXt(name) VLO("text-" #name, name)
#  else
#     define VLCzTXt(name)
#  endif /* WRITE_CUSTOMIZE_ZTXT_COMPRESSION */

#  define VLC(name) VLCIDAT(name) VLCzTXt(name)

   VLC(strategy)
   VLC(level)
   VLC(windowBits)
   VLC(memLevel)

#  undef VLO

   /* LIST settings */
#  define VLL(name) VL(#name, name, LIST)
#if defined(PNG_SELECT_FILTER_HEURISTICALLY_SUPPORTED) ||\
    defined(PNG_SELECT_FILTER_METHODICALLY_SUPPORTED)
   VLL(select)
#endif /* SELECT_FILTER_HEURISTICALLY || SELECT_FILTER_METHODICALLY */
#ifdef PNG_WRITE_FILTER_SUPPORTED
   VLL(filter)
#endif /* WRITE_FILTER */
#  undef VLL
#  undef VL
};

#define opt_count ((sizeof options)/(sizeof options[0]))

struct display
{
   jmp_buf          error_return;      /* Where to go to on error */
   unsigned int     errset;            /* error_return is set */

   const char      *operation;         /* What is happening */
   const char      *filename;          /* The name of the original file */
   const char      *output_file;       /* The name of the output file */

   /* Base file information */
   png_uint_32      w;
   png_uint_32      h;
   int              bpp;
   png_alloc_size_t size;

   /* Used on both read and write: */
   FILE            *fp;

   /* Used on a read, both the original read and when validating a written
    * image.
    */
   png_alloc_size_t read_size;
   png_structp      read_pp;
   png_infop        ip;

   /* Used to write a new image (the original info_ptr is used) */
   png_alloc_size_t write_size;
   png_alloc_size_t best_size;
   png_structp      write_pp;

   /* Options handling */
   png_uint_32      results;           /* A mask of errors seen */
   png_uint_32      options;           /* See display_log below */
   png_byte         entry[opt_count];  /* The selected entry+1 of an option that
                                        * appears on the command line, or 0 if
                                        * it was not given. */
   int              value[opt_count];  /* Corresponding value */

   /* Compression exhaustive testing */
   /* Temporary variables used only while testing a single collectiono of
    * settings:
    */
   unsigned int     csp;               /* next stack entry to use */
   unsigned int     nsp;               /* highest active entry+1 found so far */

   /* Values used while iterating through all the combinations of settings for a
    * single file:
    */
   unsigned int     tsp;               /* nsp from the last run; this is the
                                        * index+1 of the highest active entry on
                                        * this run; this entry will be advanced.
                                        */
#  define SL 8 /* stack limit */
   struct stack
   {
      png_byte      opt;               /* The option being tested */
      png_byte      entry;             /* The next value entry to be tested */
      png_byte      end;               /* This is the last entry */
      int           opt_string_end;    /* End of the option string in 'curr' */
   }                stack[SL];         /* Stack of entries being tested */
   char             curr[32*SL];       /* current options being tested */
   char             best[32*SL];       /* best options */

   char             namebuf[FILENAME_MAX+1]; /* output file name */
};

static void
display_init(struct display *dp)
   /* Call this only once right at the start to initialize the control
    * structure, the (struct buffer) lists are maintained across calls - the
    * memory is not freed.
    */
{
   memset(dp, 0, sizeof *dp);
   dp->operation = "internal error";
   dp->filename = "command line";
   dp->output_file = "no output file";
   dp->options = WARNINGS; /* default to !verbose, !quiet */
   dp->fp = NULL;
   dp->read_pp = NULL;
   dp->ip = NULL;
   dp->write_pp = NULL;
}

static void
display_clean_read(struct display *dp)
{
   if (dp->read_pp != NULL)
      png_destroy_read_struct(&dp->read_pp, NULL, NULL);

   if (dp->fp != NULL)
   {
      FILE *fp = dp->fp;
      dp->fp = NULL;
      (void)fclose(fp);
   }
}

static void
display_clean_write(struct display *dp)
{
   if (dp->fp != NULL)
   {
      FILE *fp = dp->fp;
      dp->fp = NULL;
      (void)fclose(fp);
   }

   if (dp->write_pp != NULL)
      png_destroy_write_struct(&dp->write_pp, dp->tsp > 0 ? NULL : &dp->ip);
}

static void
display_clean(struct display *dp)
{
   display_clean_read(dp);
   display_clean_write(dp);
   dp->output_file = NULL;

   /* leave the filename for error detection */
   dp->results = 0; /* reset for next time */
}

static void
display_destroy(struct display *dp)
{
   /* Release any memory held in the display. */
   display_clean(dp);
}

static struct display *
get_dp(png_structp pp)
   /* The display pointer is always stored in the png_struct error pointer */
{
   struct display *dp = (struct display*)png_get_error_ptr(pp);

   if (dp == NULL)
   {
      fprintf(stderr, "pngcp: internal error (no display)\n");
      exit(99); /* prevents a crash */
   }

   return dp;
}

/* error handling */
#ifdef __GNUC__
#  define VGATTR __attribute__((__format__ (__printf__,3,4)))
   /* Required to quiet GNUC warnings when the compiler sees a stdarg function
    * that calls one of the stdio v APIs.
    */
#else
#  define VGATTR
#endif
static void VGATTR
display_log(struct display *dp, error_level level, const char *fmt, ...)
   /* 'level' is as above, fmt is a stdio style format string.  This routine
    * does not return if level is above LIBPNG_WARNING
    */
{
   dp->results |= 1U << level;

   if (level > (error_level)(dp->options & LEVEL_MASK))
   {
      const char *lp;
      va_list ap;

      switch (level)
      {
         case INFORMATION:    lp = "information"; break;
         case LIBPNG_WARNING: lp = "warning(libpng)"; break;
         case APP_WARNING:    lp = "warning(pngcp)"; break;
         case APP_FAIL:       lp = "error(continuable)"; break;
         case LIBPNG_ERROR:   lp = "error(libpng)"; break;
         case LIBPNG_BUG:     lp = "bug(libpng)"; break;
         case APP_ERROR:      lp = "error(pngcp)"; break;
         case USER_ERROR:     lp = "error(user)"; break;

         case INTERNAL_ERROR: /* anything unexpected is an internal error: */
         case VERBOSE: case WARNINGS: case ERRORS: case QUIET:
         default:             lp = "bug(pngcp)"; break;
      }

      fprintf(stderr, "%s: %s: %s",
         dp->filename != NULL ? dp->filename : "<stdin>", lp, dp->operation);

      fprintf(stderr, ": ");

      va_start(ap, fmt);
      vfprintf(stderr, fmt, ap);
      va_end(ap);

      fputc('\n', stderr);
   }
   /* else do not output any message */

   /* Errors cause this routine to exit to the fail code */
   if (level > APP_FAIL || (level > ERRORS && !(dp->options & CONTINUE)))
   {
      if (dp->errset)
         longjmp(dp->error_return, level);

      else
         exit(99);
   }
}

/* OPTIONS:
 *
 * The command handles options of the forms:
 *
 *    --option
 *       Turn an option on (Option)
 *    --no-option
 *       Turn an option off (Option)
 *    --option=value
 *       Set an option to a value (Value)
 *    --option=val1,val2,val3
 *       Set an option to a bitmask constructed from the values (List)
 */
static png_byte
optind(struct display *dp, const char *opt, size_t len)
   /* Return the index (in options[]) of the given option, outputs an error if
    * it does not exist.  Takes the name of the option and a length (number of
    * characters in the name).
    */
{
   png_byte j;

   for (j=0; j<opt_count; ++j)
      if (strncmp(options[j].name, opt, len) == 0 && options[j].name[len] == 0)
         return j;

   /* If the setjmp buffer is set the code is asking for an option index; this
    * is bad.  Otherwise this is the command line option parsing.
    */
   display_log(dp, dp->errset ? INTERNAL_ERROR : USER_ERROR,
         "%.*s: unknown option", (int)/*SAFE*/len, opt);
   abort(); /* NOT REACHED */
}

static int
getopt(struct display *dp, const char *opt, int *value)
{
   const png_byte i = optind(dp, opt, strlen(opt));

   if (dp->entry[i]) /* option was set on command line */
   {
      *value = dp->value[i];
      return 1;
   }

   else
      return 0;
}

static void
set_opt_string(struct display *dp, unsigned int sp)
   /* Add the appropriate option string to dp->curr. */
{
   int offset, add;
   png_byte opt = dp->stack[sp].opt;
   const char *entry_name = options[opt].values[dp->stack[sp].entry].name;

   if (sp > 0)
      offset = dp->stack[sp-1].opt_string_end;

   else
      offset = 0;

   if (entry_name == range_lo)
      add = sprintf(dp->curr+offset, " --%s=%d", options[opt].name,
            dp->value[opt]);

   else
      add = sprintf(dp->curr+offset, " --%s=%s", options[opt].name, entry_name);

   if (add < 0)
      display_log(dp, INTERNAL_ERROR, "sprintf failed");

   assert(offset+add < (int)/*SAFE*/sizeof dp->curr);
   dp->stack[sp].opt_string_end = offset+add;
}

static int
opt_list_end(struct display *dp, png_byte opt, png_byte entry)
{
   if (options[opt].values[entry].name == range_lo)
      return entry+1U >= options[opt].value_count /* missing range_hi */ ||
         options[opt].values[entry+1U].name != range_hi /* likewise */ ||
         options[opt].values[entry+1U].value <= dp->value[opt] /* range end */;

   else
      return entry+1U >= options[opt].value_count /* missing 'all' */ ||
         options[opt].values[entry+1U].name == all /* last entry */;
}

static void
push_opt(struct display *dp, unsigned int sp, png_byte opt)
   /* Push a new option onto the stack, initializing the new stack entry
    * appropriately; this does all the work of next_opt (setting end/nsp) for
    * the first entry in the list.
    */
{
   png_byte entry;
   const char *entry_name;

   assert(sp == dp->tsp && sp < SL);

   /* The starting entry is entry 0 unless there is a range in which case it is
    * the entry corresponding to range_lo:
    */
   entry = options[opt].value_count;
   assert(entry > 0U);

   do
   {
      entry_name = options[opt].values[--entry].name;
      if (entry_name == range_lo)
         break;
   }
   while (entry > 0U);

   dp->tsp = sp+1U;
   dp->stack[sp].opt = opt;
   dp->stack[sp].entry = entry;
   dp->value[opt] = options[opt].values[entry].value;

   set_opt_string(dp, sp);

   if (opt_list_end(dp, opt, entry))
   {
      dp->stack[sp].end = 1;
      display_log(dp, APP_WARNING, "%s: only testing one value",
            options[opt].name);
   }

   else
   {
      dp->stack[sp].end = 0;
      dp->nsp = dp->tsp;
   }
}

static void
next_opt(struct display *dp, unsigned int sp)
   /* Return the next value for this option.  When called 'sp' is expected to be
    * the topmost stack entry - only the topmost entry changes each time round -
    * and there must be a valid entry to return.  next_opt will set dp->nsp to
    * sp+1 if more entries are available, otherwise it will not change it and
    * set dp->stack[s].end to true.
    */
{
   png_byte entry, opt;
   const char *entry_name;

   /* dp->stack[sp] must be the top stack entry and it must be active: */
   assert(sp+1U == dp->tsp && !dp->stack[sp].end);

   opt = dp->stack[sp].opt;
   entry = dp->stack[sp].entry;
   assert(entry+1U < options[opt].value_count);
   entry_name = options[opt].values[entry].name;
   assert(entry_name != NULL);

   /* For ranges increment the value but don't change the entry, for all other
    * cases move to the next entry and load its value:
    */
   if (entry_name == range_lo) /* a range */
      dp->value[opt]++;

   else
   {
      /* Increment 'entry' */
      dp->value[opt] = options[opt].values[++entry].value;
      dp->stack[sp].entry = entry;
   }

   set_opt_string(dp, sp);

   if (opt_list_end(dp, opt, entry)) /* end of list */
      dp->stack[sp].end = 1;

   else /* still active after all these tests */
      dp->nsp = dp->tsp;
}

static int
getallopts(struct display *dp, const char *opt_str, int *value)
   /* Like getop but iterate over all the values if the option was set to "all".
    */
{
   const png_byte opt = optind(dp, opt_str, strlen(opt_str));

   if (dp->entry[opt]) /* option was set on command line */
   {
      /* Simple, single value, entries don't have a stack frame and have a fixed
       * value (it doesn't change once set on the command line).  Otherwise the
       * value (entry) selected from the command line is 'all':
       */
      if (options[opt].values[dp->entry[opt]-1].name == all)
      {
         unsigned int sp = dp->csp++; /* my stack entry */

         assert(sp >= dp->nsp); /* nsp starts off zero */

         /* If the entry was active in the previous run dp->stack[sp] is already
          * set up and dp->tsp will be greater than sp, otherwise a new entry
          * needs to be created.
          *
          * dp->nsp is handled this way:
          *
          * 1) When an option is pushed onto the stack dp->nsp and dp->tsp are
          *    both set (by push_opt) to the next stack entry *unless* there is
          *    only one entry in the new list, in which case dp->stack[sp].end
          *    is set.
          *
          * 2) For the top stack entry next_opt is called.  The entry must be
          *    active (dp->stack[sp].end is not set) and either 'nsp' or 'end'
          *    will be updated as appropriate.
          *
          * 3) For lower stack entries nsp is set unless the stack entry is
          *    already at the end.  This means that when all the higher entries
          *    are popped this entry will be too.
          */
         if (sp >= dp->tsp)
            push_opt(dp, sp, opt); /* This sets tsp to sp+1 */

         else if (sp+1U >= dp->tsp)
            next_opt(dp, sp);

         else if (!dp->stack[sp].end) /* Active, not at top of stack */
            dp->nsp = sp+1U;
      }

      *value = dp->value[opt];
      return 1; /* set */
   }

   else
      return 0; /* not set */
}

static int
find_val(struct display *dp, png_byte opt, const char *str, size_t len)
   /* Like optind but sets (index+i) of the entry in options[opt] that matches
    * str[0..len-1] into dp->entry[opt] as well as returning the actual value.
    */
{
   int rlo = INT_MAX, rhi = INT_MIN;
   png_byte j, irange = 0;

   for (j=1U; j<=options[opt].value_count; ++j)
   {
      if (strncmp(options[opt].values[j-1U].name, str, len) == 0 &&
          options[opt].values[j-1U].name[len] == 0)
      {
         dp->entry[opt] = j;
         return options[opt].values[j-1U].value;
      }
      else if (options[opt].values[j-1U].name == range_lo)
         rlo = options[opt].values[j-1U].value, irange = j;
      else if (options[opt].values[j-1U].name == range_hi)
         rhi = options[opt].values[j-1U].value;
   }

   /* No match on the name, but there may be a range. */
   if (irange > 0)
   {
      char *ep = NULL;
      long l = strtol(str, &ep, 0);

      if (ep == str+len && l >= rlo && l <= rhi)
      {
         dp->entry[opt] = irange; /* range_lo */
         return (int)/*SAFE*/l;
      }
   }

   display_log(dp, dp->errset ? INTERNAL_ERROR : USER_ERROR,
         "%s: unknown value setting '%.*s'", options[opt].name,
         (int)/*SAFE*/len, str);
   abort(); /* NOT REACHED */
}

static int
opt_check(struct display *dp, const char *arg)
{
   assert(dp->errset == 0);

   if (arg != NULL && arg[0] == '-' && arg[1] == '-')
   {
      int i = 0, negate = (strncmp(arg+2, "no-", 3) == 0), val;
      png_byte j;

      if (negate)
         arg += 5; /* --no- */

      else
         arg += 2; /* -- */

      /* Find the length (expect arg\0 or arg=) */
      while (arg[i] != 0 && arg[i] != '=') ++i;

      /* So arg[0..i-1] is the argument name, this does not return if this isn't
       * a valid option name.
       */
      j = optind(dp, arg, i);

      /* It matcheth an option; check the remainder. */
      if (arg[i] == 0) /* no specified value, use the default */
      {
         val = options[j].values[negate].value;
         dp->entry[j] = (png_byte)/*SAFE*/(negate + 1U);
      }

      else
      {
         const char *list = arg + (i+1);

         /* Expect a single value here unless this is a list, in which case
          * multiple values are combined.
          */
         if (options[j].opt != LIST)
         {
            /* find_val sets 'dp->entry[j]' to a non-zero value: */
            val = find_val(dp, j, list, strlen(list));

            if (negate)
            {
               if (options[j].opt < OPTION)
                  val = !val;

               else
               {
                  display_log(dp, USER_ERROR,
                        "%.*s: option=arg cannot be negated", i, arg);
                  abort(); /* NOT REACHED */
               }
            }
         }

         else /* multiple options separated by ',' characters */
         {
            /* --no-option negates list values from the default, which should
             * therefore be 'all'.  Notice that if the option list is empty in
             * this case nothing will be removed and therefore --no-option= is
             * the same as --option.
             */
            if (negate)
               val = options[j].values[0].value;

            else
               val = 0;

            while (*list != 0) /* allows option= which sets 0 */
            {
               /* A value is terminated by the end of the list or a ','
                * character.
                */
               int v, iv;

               iv = 0; /* an index into 'list' */
               while (list[++iv] != 0 && list[iv] != ',') {}

               v = find_val(dp, j, list, iv);

               if (negate)
                  val &= ~v;

               else
                  val |= v;

               list += iv;
               if (*list != 0)
                  ++list; /* skip the ',' */
            }
         }
      }

      /* 'val' is the new value, store it for use later and debugging: */
      dp->value[j] = val;

      if (options[j].opt < LEVEL_MASK)
      {
         /* The handling for error levels is to set the level. */
         if (val) /* Set this level */
            dp->options = (dp->options & ~LEVEL_MASK) | options[j].opt;

         else
            display_log(dp, USER_ERROR,
      "%.*s: messages cannot be turned off individually; set a message level",
                  i, arg);
      }

      else if (options[j].opt < OPTION)
      {
         if (val)
            dp->options |= options[j].opt;

         else
            dp->options &= ~options[j].opt;
      }

      return 1; /* this is an option */
   }

   else
      return 0; /* not an option */
}

/* The following is used in main to verify that the final argument is a
 * directory:
 */
static int
checkdir(const char *pathname)
{
   struct stat buf;
   return stat(pathname, &buf) == 0 && S_ISDIR(buf.st_mode);
}

/* Work out whether a path is valid (if not a display_log occurs), a directory
 * (1 is returned) or a file *or* non-existent (0 is returned).
 *
 * Used for a write path.
 */
static int
isdir(struct display *dp, const char *pathname)
{
   if (pathname == NULL)
      return 0; /* stdout */
   
   else if (pathname[0] == 0)
      return 1; /* empty string */

   else
   {
      struct stat buf;
      int ret = stat(pathname, &buf);

      if (ret == 0) /* the entry exists */
      {
         if (S_ISDIR(buf.st_mode))
            return 1;

         /* Else expect an object that exists and can be written: */
         if (access(pathname, W_OK) != 0)
            display_log(dp, USER_ERROR, "%s: cannot be written (%s)", pathname,
                  strerror(errno));

         return 0; /* file (exists, can be written) */
      }

      else /* an error */
      {
         /* Non-existence is fine, other errors are not: */
         if (errno != ENOENT)
            display_log(dp, USER_ERROR, "%s: invalid output name (%s)",
                  pathname, strerror(errno));

         return 0; /* file (does not exist) */
      }
   }
}

static void
makename(struct display *dp, const char *dir, const char *infile)
{
   /* Make a name for an output file (and check it). */
   dp->namebuf[0] = 0;

   if (dir == NULL || infile == NULL)
      display_log(dp, INTERNAL_ERROR, "NULL name to makename");

   else
   {
      size_t dsize = strlen(dir);

      if (dsize < FILENAME_MAX+2)
      {
         size_t isize = strlen(infile);
         size_t istart = isize-1;

         /* This should fail before here: */
         if (infile[istart] == '/')
            display_log(dp, INTERNAL_ERROR, "infile with trailing /");

         memcpy(dp->namebuf, dir, dsize);
         if (dsize > 0 && dp->namebuf[dsize-1] != '/')
            dp->namebuf[dsize++] = '/';

         /* Find the rightmost non-/ character: */
         while (istart > 0 && infile[istart-1] != '/')
            --istart;

         isize -= istart;
         infile += istart;

         if (dsize+isize <= FILENAME_MAX)
         {
            memcpy(dp->namebuf+dsize, infile, isize+1);

            if (isdir(dp, dp->namebuf))
               display_log(dp, USER_ERROR, "%s: output file is a directory",
                     dp->namebuf);
         }

         else
         {
            dp->namebuf[dsize] = 0;
            display_log(dp, USER_ERROR, "%s%s: output file name too long",
                  dp->namebuf, infile);
         }
      }

      else
         display_log(dp, USER_ERROR, "%s: output directory name too long", dir);
   }
}

/* error handler callbacks for libpng */
static void PNGCBAPI
display_warning(png_structp pp, png_const_charp warning)
{
   display_log(get_dp(pp), LIBPNG_WARNING, "%s", warning);
}

static void PNGCBAPI
display_error(png_structp pp, png_const_charp error)
{
   struct display *dp = get_dp(pp);

   display_log(dp, LIBPNG_ERROR, "%s", error);
}

static void
display_start_read(struct display *dp, const char *filename)
{
   if (filename != NULL)
   {
      dp->filename = filename;
      dp->fp = fopen(filename, "rb");
   }

   else
   {
      dp->filename = "<stdin>";
      dp->fp = stdin;
   }

   dp->w = dp->h = 0U;
   dp->bpp = 0U;
   dp->size = 0U;
   dp->read_size = 0U;

   if (dp->fp == NULL)
      display_log(dp, USER_ERROR, "file open failed (%s)", strerror(errno));
}

static void PNGCBAPI
read_function(png_structp pp, png_bytep data, png_size_t size)
{
   struct display *dp = get_dp(pp);

   if (fread(data, size, 1U, dp->fp) == 1U)
      dp->read_size += size;

   else
   {
      if (feof(dp->fp))
         display_log(dp, USER_ERROR, "PNG file truncated");
      else
         display_log(dp, USER_ERROR, "PNG file read failed (%s)",
               strerror(errno));
   }
}

static void
read_png(struct display *dp, const char *filename)
{
   display_clean_read(dp); /* safety */
   display_start_read(dp, filename);

   dp->read_pp = png_create_read_struct(PNG_LIBPNG_VER_STRING, dp,
      display_error, display_warning);
   if (dp->read_pp == NULL)
      display_log(dp, LIBPNG_ERROR, "failed to create read struct");

   /* The png_read_png API requires us to make the info struct, but it does the
    * call to png_read_info.
    */
   dp->ip = png_create_info_struct(dp->read_pp);
   if (dp->ip == NULL)
      display_log(dp, LIBPNG_ERROR, "failed to create info struct");

   /* Set the IO handling */
   png_set_read_fn(dp->read_pp, dp, read_function);

#  ifdef PNG_SET_UNKNOWN_CHUNKS_SUPPORTED
      png_set_keep_unknown_chunks(dp->read_pp, PNG_HANDLE_CHUNK_ALWAYS, NULL,
            0);
#  endif /* SET_UNKNOWN_CHUNKS */

#  ifdef PNG_SET_USER_LIMITS_SUPPORTED
      /* Remove the user limits, if any */
      png_set_user_limits(dp->read_pp, 0x7fffffff, 0x7fffffff);
#  endif /* SET_USER_LIMITS */

   /* Now read the PNG. */
   png_read_png(dp->read_pp, dp->ip, 0U/*transforms*/, NULL/*params*/);
   dp->w = png_get_image_width(dp->read_pp, dp->ip);
   dp->h = png_get_image_height(dp->read_pp, dp->ip);
   dp->bpp = png_get_bit_depth(dp->read_pp, dp->ip) *
             png_get_channels(dp->read_pp, dp->ip);
   dp->size = png_get_rowbytes(dp->read_pp, dp->ip) * dp->h; /* can overflow */
   display_clean_read(dp);
   dp->operation = "none";
}

static void
display_start_write(struct display *dp, const char *filename)
{
   if (filename != NULL)
   {
      dp->output_file = filename;
      dp->fp = fopen(filename, "wb");
   }

   else
   {
      dp->output_file = "<stdout>";
      dp->fp = stdout;
   }

   dp->write_size = 0U;

   if (dp->fp == NULL)
      display_log(dp, USER_ERROR, "%s: file open failed (%s)", dp->output_file,
            strerror(errno));
}

static void PNGCBAPI
write_function(png_structp pp, png_bytep data, png_size_t size)
{
   struct display *dp = get_dp(pp);

   if (fwrite(data, size, 1U, dp->fp) == 1U)
      dp->write_size += size;

   else
      display_log(dp, USER_ERROR, "%s: PNG file write failed (%s)",
            dp->output_file, strerror(errno));
}

/* Compression option, 'method' is never set: there is no choice */
#define SET_COMPRESSION\
   SET(strategy, strategy);\
   SET(level, level);\
   SET(windowBits, window_bits);\
   SET(memLevel, mem_level);

#ifdef PNG_WRITE_CUSTOMIZE_COMPRESSION_SUPPORTED
static void
set_compression(struct display *dp)
{
   int val;

#  define SET(name, func) if (getallopts(dp, #name, &val))\
      png_set_compression_ ## func(dp->write_pp, val);
   SET_COMPRESSION
#  undef SET
}
#else
#  define set_compression(dp, pp) ((void)0)
#endif /* WRITE_CUSTOMIZE_COMPRESSION */

#ifdef PNG_WRITE_CUSTOMIZE_ZTXT_COMPRESSION_SUPPORTED
static void
set_text_compression(struct display *dp)
{
   int val;

#  define SET(name, func) if (getallopts(dp, "text-" #name, &val))\
      png_set_text_compression_ ## func(dp->write_pp, val);
   SET_COMPRESSION
#  undef SET
}
#else
#  define set_text_compression(dp, pp) ((void)0)
#endif /* WRITE_CUSTOMIZE_ZTXT_COMPRESSION */

static void
write_png(struct display *dp, const char *destname)
{
   display_clean_write(dp); /* safety */
   display_start_write(dp, destname);

   dp->write_pp = png_create_write_struct(PNG_LIBPNG_VER_STRING, dp,
      display_error, display_warning);

   if (dp->write_pp == NULL)
      display_log(dp, APP_ERROR, "failed to create write png_struct");

   png_set_write_fn(dp->write_pp, dp, write_function, NULL/*flush*/);

#  ifdef PNG_SET_UNKNOWN_CHUNKS_SUPPORTED
      png_set_keep_unknown_chunks(dp->write_pp, PNG_HANDLE_CHUNK_ALWAYS, NULL,
            0);
#  endif /* SET_UNKNOWN_CHUNKS */

#  ifdef PNG_SET_USER_LIMITS_SUPPORTED
      /* Remove the user limits, if any */
      png_set_user_limits(dp->write_pp, 0x7fffffff, 0x7fffffff);
#  endif

   /* OPTION HANDLING */
   /* compression outputs, IDAT and zTXt/iTXt: */
   dp->tsp = dp->nsp;
   dp->nsp = dp->csp = 0;
   set_compression(dp);
   set_text_compression(dp);

   /* filter handling */
#if defined(PNG_SELECT_FILTER_HEURISTICALLY_SUPPORTED) ||\
    defined(PNG_SELECT_FILTER_METHODICALLY_SUPPORTED)
      {
         int val;

         if (getopt(dp, "select", &val))
         {
#           ifdef PNG_SELECT_FILTER_HEURISTICALLY_SUPPORTED
               png_set_option(dp->write_pp, PNG_SELECT_FILTER_HEURISTICALLY,
                     (val & SELECT_HEURISTICALLY) != 0);
#           endif /* SELECT_FILTER_HEURISTICALLY */
#           ifdef PNG_SELECT_FILTER_METHODICALLY_SUPPORTED
               png_set_option(dp->write_pp, PNG_SELECT_FILTER_METHODICALLY,
                     (val & SELECT_METHODICALLY) != 0);
#           endif /* SELECT_FILTER_METHODICALLY */
         }
      }
#endif /* SELECT_FILTER_HEURISTICALLY || SELECT_FILTER_METHODICALLY */

#  ifdef PNG_WRITE_FILTER_SUPPORTED
      {
         int val;

         if (getopt(dp, "filter", &val))
            png_set_filter(dp->write_pp, PNG_FILTER_TYPE_BASE, val);
      }
#  endif /* WRITE_FILTER */

   /* This just uses the 'read' info_struct directly, it contains the image. */
   png_write_png(dp->write_pp, dp->ip, 0U/*transforms*/, NULL/*params*/);

   /* Make sure the file was written ok: */
   {
      FILE *fp = dp->fp;
      dp->fp = NULL;
      if (fclose(fp))
         display_log(dp, APP_ERROR, "%s: write failed (%s)", destname,
               strerror(errno));
   }

   /* Clean it on the way out - if control returns to the caller then the
    * written_file contains the required data.
    */
   display_clean_write(dp);
   dp->operation = "none";
}

static void
cp_one_file(struct display *dp, const char *filename, const char *destname)
{
   dp->filename = filename;
   dp->operation = "read";

   /* Read it then write it: */
   if (filename != NULL && access(filename, R_OK) != 0)
      display_log(dp, USER_ERROR, "%s: invalid file name (%s)",
            filename, strerror(errno));

   read_png(dp, filename);

   /* But 'destname' may be a directory. */
   dp->operation = "write";

   if (destname != NULL) /* else stdout */
   {
      if (isdir(dp, destname))
      {
         makename(dp, destname, filename);
         destname = dp->namebuf;
      }

      else if (access(destname, W_OK) != 0 && errno != ENOENT)
         display_log(dp, USER_ERROR, "%s: invalid output name (%s)", destname,
               strerror(errno));
   }

   dp->nsp = 0;
   dp->curr[0] = 0;
   dp->best[0] = 0; /* acts as a flag for the caller */
   write_png(dp, destname);

   if (dp->nsp > 0) /* interating over lists */
   {
      char tmpname[(sizeof dp->namebuf) + 4];
      assert(dp->curr[0] == ' ' && dp->tsp > 0);

      /* Loop to find the best option, first initialize the 'best' fields: */
      strcpy(dp->best, dp->curr);
      dp->best_size = dp->write_size;
      strcpy(tmpname, destname);
      strcat(tmpname, ".tmp"); /* space for .tmp allocated above */

      do
      {
         write_png(dp, tmpname);

         /* And compare the sizes (theoretically this could overflow, in which
          * case this program will need to be rewritten perhaps considerably).
          */
         if (dp->write_size < dp->best_size)
         {
            if (rename(tmpname, destname) != 0)
               display_log(dp, APP_ERROR, "rename %s %s failed (%s)", tmpname,
                     destname, strerror(errno));

            strcpy(dp->best, dp->curr);
            dp->best_size = dp->write_size;
         }

         else if (unlink(tmpname) != 0)
            display_log(dp, APP_WARNING, "unlink %s failed (%s)", tmpname,
                  strerror(errno));
      }
      while (dp->nsp > 0);

      /* Do this for the 'sizes' option so that it reports the correct size. */
      dp->write_size = dp->best_size;
   }
}

static int
cppng(struct display *dp, const char *file, const char *gv dest)
   /* Exists solely to isolate the setjmp clobbers which some versions of GCC
    * erroneously generate.
    */
{
   int ret = setjmp(dp->error_return);

   if (ret == 0)
   {
      dp->errset = 1;
      cp_one_file(dp, file, dest);
      dp->errset = 0;
      return 0;
   }

   else
   {
      dp->errset = 0;

      if (ret < ERRORS) /* shouldn't longjmp on warnings */
         display_log(dp, INTERNAL_ERROR, "unexpected return code %d", ret);

      return ret;
   }
}

int
main(const int argc, const char * const * const argv)
{
   /* For each file on the command line test it with a range of transforms */
   int option_end;
   struct display d;

   display_init(&d);

   d.operation = "options";
   for (option_end = 1;
        option_end < argc && opt_check(&d, argv[option_end]);
        ++option_end)
   {
   }

   /* Do a quick check on the directory target case; when there are more than
    * two arguments the last one must be a directory.
    */
   if (option_end+2 < argc && !checkdir(argv[argc-1]))
   {
      fprintf(stderr,
            "pngcp: %s: directory required with more than two arguments\n",
            argv[argc-1]);
      return 99;
   }

   {
      int errors = 0;
      int i = option_end;

      /* Do this at least once; if there are no arguments stdin/stdout are used.
       */
      d.operation = "files";
      do
      {
         const char *infile = NULL;
         const char *outfile = NULL;

         if (i < argc)
         {
            infile = argv[i++];
            if (i < argc)
               outfile = argv[argc-1];
         }

         {
            int ret = cppng(&d, infile, outfile);

            if (ret > QUIET) /* abort on user or internal error */
               return 99;
         }

         if (d.options & SIZES)
            printf("%s [%ld x %ld %d bpp %lu bytes] 0x%lx %lu -> %lu\n",
                  infile, (unsigned long)d.w, (unsigned long)d.h, d.bpp,
                  (unsigned long)d.size, (unsigned long)d.results,
                  (unsigned long)d.read_size, (unsigned long)d.write_size);

         /* This somewhat replicates the above information, but it seems better
          * to do both.
          */
         if (d.best[0] != 0 && (error_level)(d.options & LEVEL_MASK) < QUIET)
            printf("%s [%ld x %ld %d bpp %lu bytes] %lu -> %lu with '%s'\n",
                  infile, (unsigned long)d.w, (unsigned long)d.h, d.bpp,
                  (unsigned long)d.size, (unsigned long)d.read_size,
                  (unsigned long)d.best_size, d.best);

         /* Here on any return, including failures, except user/internal issues
          */
         {
            const int pass = (d.options & STRICT) ?
               RESULT_STRICT(d.results) : RESULT_RELAXED(d.results);

            if (!pass)
               ++errors;

            if (d.options & LOG)
            {
               int j;

               printf("%s: pngcp", pass ? "PASS" : "FAIL");

               for (j=1; j<option_end; ++j)
                  printf(" %s", argv[j]);

               if (infile != NULL)
                  printf(" %s", infile);

               printf("\n");
            }
         }

         display_clean(&d);
      }
      while (i+1 < argc);
         /* I.e. for all cases after the first time through the loop require
          * there to be at least two arguments left and for the last one to be a
          * directory (this was checked above).
          */

      /* Release allocated memory */
      display_destroy(&d);

      return errors != 0;
   }
}
#else /* !READ_PNG || !WRITE_PNG */
int
main(void)
{
   fprintf(stderr, "pngcp: no support for png_read/write_image\n");
   return 77;
}
#endif
