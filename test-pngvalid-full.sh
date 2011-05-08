#!/bin/sh
#
# Run a sequence of gamma tests quietly
err=0

echo >> pngtest-log.txt
echo "============ pngvalid-full.sh ==============" >> pngtest-log.txt

echo "Running test-pngvalid-full.sh"
for gamma in threshold transform sbit 16-to-8 background alpha-mode
do
   # For the moment the composition calculation is performed with minimal
   # accuracy, do this to work round the problem:
   if test $gamma = background -o $gamma = alpha-mode
   then
      opts=--use-linear-precision
   else
      opts=
   fi

   if ./pngvalid $opts "--gamma-$gamma" >> pngtest-log.txt 2>&1
   then
      echo "  PASS:" pngvalid "--gamma-$gamma"
   else
      echo "  FAIL:" pngvalid "--gamma-$gamma"
      err=1
   fi
done

echo

exit $err
