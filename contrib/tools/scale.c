/* Given a target range and a source range work out an expression to scale from
 * the source to the target of the form:
 *
 *    (number * mult + add)>>16
 *
 * The command arguments are:
 *
 *    scale target source
 *
 * and the program works out a pair of numbers, mult and add, that evaluate:
 *
 *          number * target
 *   round( --------------- )
 *              source
 *
 * exactly for number in the range 0..source
 */
#define _ISOC99_SOURCE 1

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

static double minerr;
static unsigned long minmult, minadd, minshift;
static long mindelta;

static int
test(unsigned long target, unsigned long source, unsigned long mult,
   long add, unsigned long shift, long delta)
{
   unsigned long i;
   double maxerr = 0;
   double rs = (double)target/source;

   for (i=0; i<=source; ++i)
   {
      unsigned long t = i*mult+add;
      double err = fabs((t >> shift) - i*rs);

      if (err > minerr)
         return 0;

      if (err > maxerr)
         maxerr = err;
   }

   if (maxerr < minerr)
   {
      minerr = maxerr;
      minmult = mult;
      minadd = add;
      minshift = shift;
      mindelta = delta;
   }

   return maxerr < .5;
}

static int
dotest(unsigned long target, unsigned long source, unsigned long mult,
   long add, unsigned long shift, long delta, int print)
{
   if (test(target, source, mult, add, shift, delta))
   {
      if (print & 4)
         printf("      {%11lu,%6ld /* >>%lu */ }, /* %lu/%lu */\n",
            mult, add, shift, target, source);

      else if (print & 2)
         printf("      {%11lu,%6ld,%3lu }, /* %lu/%lu */\n",
            mult, add, shift, target, source);

      else if (print)
         printf("number * %lu/%lu = (number * %lu + %ld) >> %lu [delta %ld]\n",
            target, source, mult, add, shift, delta);

      return 1;
   }

   return 0;
}

static int
find(unsigned long target, unsigned long source, int print, int fixshift)
{
   unsigned long shift = 0;
   unsigned long shiftlim = 0;

   /* In the final math the sum is at most (source*mult+add) >> shift, so:
    *
    *    source*mult+add < 1<<32
    *    mult < (1<<32)/source
    *
    * but:
    *
    *    mult = (target<<shift)/source
    *
    * so:
    *
    *    (target<<shift) < (1<<32)
    */
   if (fixshift < 0)
      while ((target<<shiftlim) < 0x80000000U) ++shiftlim;

   else
      shift = shiftlim = (unsigned long)fixshift;

   minerr = 1E8;

   for (; shift<=shiftlim; ++shift)
   {
      unsigned long mult = ((target<<shift) + (source>>1)) / source;
      long delta;
      long limit = 1; /* seems to be sufficient */
      long add, start, end;

      end = 1<<shift;
      start = -end;

      for (add=start; add<=end; ++add)
         if (dotest(target,source,mult,add,shift,0,print))
            return 1;

      for (delta=1; delta<=limit; ++delta)
      {
#        if 0
            fprintf(stderr, "%lu/%lu: shift %lu, delta %lu\n", target, source,
               shift, delta);
#        endif

         for (add=start; add<=end; ++add)
         {
            if (dotest(target, source, mult-delta, add, shift, -delta, print))
               return 1;

            if (dotest(target, source, mult+delta, add, shift, delta, print))
               return 1;
         }
      }
   }

   if (print & 4)
      printf("      {%11lu,%6ld /* >>%lu */ }, /* %lu/%lu ERROR: .5+%g*/\n",
         minmult, minadd, minshift, target, source, minerr-.5);

   else if (print & 2)
      printf("      {%11lu,%6ld,%3lu }, /* %lu/%lu ERROR: .5+%g*/\n",
         minmult, minadd, minshift, target, source, minerr-.5);

   else if (print)
      printf(
         "number * %lu/%lu ~= (number * %lu + %ld) >> %lu +/-.5+%g [delta %ld]\n",
         target, source, minmult, minadd, minshift, minerr-.5, mindelta);

   return 0;
}

static void
usage(const char *prog)
{
   fprintf(stderr,
      "usage: %s {--denominator|--maxshift|--code} target {source}\n"
      " For each 'source' prints 'mult' and 'add' such that:\n\n"
      "   (number * mult + add) >> 16 = round(number*target/source)\n\n"
      " for all integer values of number in the range 0..source.\n\n"
      " --denominator: swap target and source (specify a single source first\n"
      "                and follow with multiple targets.)\n"
      "    --maxshift: find the lowest shift value that works for all the\n"
      "                repeated 'source' values\n"
      "        --code: output C code for array/structure initialization\n",
      prog);
   exit(1);
}

int
main(int argc, const char **argv)
{
   int i, err = 0, maxshift = 0, firstsrc = 1, code = 0, denominator = 0;
   unsigned long target, shift = 0;

   while (argc > 1)
   {
      if (strcmp(argv[firstsrc], "--maxshift") == 0)
      {
         maxshift = 1;
         ++firstsrc;
      }

      else if (strcmp(argv[firstsrc], "--code") == 0)
      {
         code = 1;
         ++firstsrc;
      }

      else if (strcmp(argv[firstsrc], "--denominator") == 0)
      {
         denominator = 1;
         ++firstsrc;
      }

      else
         break;
   }


   if (argc < 2+firstsrc)
      usage(argv[0]);

   target = strtoul(argv[firstsrc++], 0, 0);
   if (target == 0) usage(argv[0]);

   for (i=firstsrc; i<argc; ++i)
   {
      unsigned long source = strtoul(argv[i], 0, 0);

      if (source == 0) usage(argv[0]);

      if (!find(denominator ? source : target, denominator ? target : source,
         maxshift ? 0 : 1+code, -1))
         err = 1;

      if (minshift > shift) shift = minshift;
   }

   if (maxshift) for (i=firstsrc; i<argc; ++i)
   {
      unsigned long source = strtoul(argv[i], 0, 0);

      if (!find(denominator ? source : target, denominator ? target : source,
         code ? 4 : 1, shift))
         err = 1;
   }

   /* Just an exit code - the printout above lists the problem */
   return err;
}
