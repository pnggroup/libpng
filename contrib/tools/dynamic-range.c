/* dynamic-range.c
 *
 * Last changed in libpng 1.7.0
 *
 * COPYRIGHT: Written by John Cunningham Bowler, 2015
 * To the extent possible under law, the author has waived all copyright and
 * related or neighboring rights to this work.  This work is published from:
 * United States.
 *
 * Find the dynamic range of a given gamma encoding given a (linear) precision
 * and a maximum number of encoded values.
 */
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <assert.h>

double range(unsigned int steps, double factor, double gamma)
{
   return pow((steps * (pow(factor, 1/gamma) - 1)), gamma);
}

double max_range_gamma(unsigned int steps, double factor, double *max_range,
      double glo, double rlo, double gmid, double rmid, double ghi, double rhi)
{
   /* Given three values which contain a peak value (so rmid > rlo and rmid >
    * rhi) find the peak by repeated division of the range.  The algorithm is to
    * find the range for two gamma values mid-way between the two pairs
    * (glo,gmid), (ghi,gmid) then find the max; this gives us a new glo/ghi
    * which must be half the distance apart of the previous pair.
    */
   double gammas[5];
   double ranges[5];

   gammas[0] = glo;  ranges[0] = rlo;
   gammas[2] = gmid; ranges[2] = rmid;
   gammas[4] = ghi;  ranges[4] = rhi;

   for (;;)
   {
      int i, m;

      ranges[1] = range(steps, factor, gammas[1] = (gammas[0]+gammas[2])/2);
      ranges[3] = range(steps, factor, gammas[3] = (gammas[2]+gammas[4])/2);

      for (m=1, i=2; i<4; ++i)
         if (ranges[i] >= ranges[m])
            m = i;

      assert(gammas[0] < gammas[m] && gammas[m] < gammas[4]);
      assert(ranges[0] < ranges[m] && ranges[m] > ranges[4]);

      gammas[0] = gammas[m-1]; ranges[0] = ranges[m-1];
      gammas[4] = gammas[m+1]; ranges[4] = ranges[m+1];
      gammas[2] = gammas[m];   ranges[2] = ranges[m];

      if (((gammas[4] - gammas[0])/gammas[2]-1) < 3*DBL_EPSILON ||
          ((ranges[2] - ranges[0])/ranges[2]-1) < 6*DBL_EPSILON)
      {
         *max_range = ranges[2];
         return gammas[2];
      }
   }
}

double best_gamma(unsigned int values, double precision, double *best_range)
{
   /* The 'guess' gamma value is determined by the following formula, which is
    * itself derived from linear regression using values returned by this
    * program:
    */
   double gtry = values * precision / 2.736;
   double rtry;

   /* 'values' needs to be the number of steps after the first, we have to
    * reserve the first value, 0, for 0, so subtract 2 from values.  precision
    * must be adjusted to the step factor.
    */
   values -= 2U;
   precision += 1;
   rtry = range(values, precision, gtry);

   /* Now find two values either side of gtry with a lower range. */
   {
      double glo, ghi, rlo, rhi, gbest, rbest;

      glo = gtry;
      do
      {
         glo *= 0.9;
         rlo = range(values, precision, glo);
      }
      while (rlo >= rtry);

      ghi = gtry;
      do
      {
         ghi *= 1.1;
         rhi = range(values, precision, ghi);
      }
      while (rhi >= rtry);

      gbest = max_range_gamma(values, precision, &rbest,
            glo, rlo, gtry, rtry, ghi, rhi);

      *best_range = rbest / precision;
      return gbest;
   }
}

double linear_regression(double precision, double *bp)
{
   unsigned int values, count = 0;
   double g_sum = 0, g2_sum = 0, v_sum = 0, v2_sum = 0, gv_sum = 0;

   /* Perform simple linear regression to get:
    *
    *    gamma = a + b.values
    */
   for (values = 128; values < 65536; ++values, ++count)
   {
      double range;
      double gamma = best_gamma(values, precision, &range);

      g_sum += gamma;
      g2_sum += gamma * gamma;
      v_sum += values;
      v2_sum += values * (double)values;
      gv_sum += gamma * values;
      /* printf("%u %g %g\n", values, gamma, range); */
   }

   g_sum /= count;
   g2_sum /= count;
   v_sum /= count;
   v2_sum /= count;
   gv_sum /= count;

   {
      double b = (gv_sum - g_sum * v_sum) / (v2_sum - v_sum * v_sum);
      *bp = b;
      return g_sum - b * v_sum;
   }
}

int
main(int argc, const char **argv)
{
   double precision = argc == 2 ? atof(argv[1]) : 0;

   /* Perform a second linear regression here on b:
    *
    *    b = bA + bB * precision
    */
   if (precision == 0)
   {
      double b_sum = 0, b2_sum = 0, p_sum = 0, p2_sum = 0, bp_sum = 0,
             a_sum = 0, count = 0;

      for (precision = .001; precision <= 0.01; precision += .001, count += 1)
      {
         double b;
         double a = linear_regression(precision, &b);

         b_sum += b;
         b2_sum += b * b;
         p_sum += precision;
         p2_sum += precision * precision;
         bp_sum += b * precision;
         a_sum += a;
      }

      b_sum /= count;
      b2_sum /= count;
      p_sum /= count;
      p2_sum /= count;
      bp_sum /= count;
      a_sum /= count;

      {
         double bB = (bp_sum - b_sum * p_sum) / (p2_sum - p_sum * p_sum);
         double bA = b_sum - bB * p_sum;

         printf("a = %g, b = %g + precision/%g\n", a_sum, bA, 1/bB);
      }
   }

   else
   {
      unsigned int bits;
      double b;
      double a = linear_regression(precision, &b);
      printf("precision %g: gamma = %g + values*%g\n", precision, a, b);

      /* For information, given a precision: */
      for (bits=7U; bits <= 16U; ++bits)
      {
         unsigned int values = 1U<<bits;
         double gamma = values*precision/2.736;
         double r = range(values-2U, 1+precision, gamma);

         printf("bits: %u, gamma: %g, range: 1:%g\n", bits, gamma, r);
      }
   }

   return 0;
}
