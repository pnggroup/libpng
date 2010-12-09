#!/bin/sh
#
# Run a sequence of gamma tests quietly
err=0

echo >> pngtest-log.txt
echo "============ pngvalid-full.sh ==============" >> pngtest-log.txt

echo "Running test-pngvalid-full.sh -- it's lengthy, please wait..."
for gamma in threshold transform sbit 16-to-8
do
   opts=
   test "$gamma" = threshold || opts="$opts --nogamma-threshold"
   test "$gamma" = transform || opts="$opts --nogamma-transform"
   test "$gamma" = sbit      || opts="$opts --nogamma-sbit"
   test "$gamma" = 16-to-8   || opts="$opts --nogamma-16-to-8"

   if ./pngvalid --nostandard $opts >> pngtest-log.txt 2>&1
   then
      echo "  PASS:" pngvalid "(gamma-$gamma)"
   else
      echo "  FAIL:" pngvalid "(gamma-$gamma)"
      err=1
   fi
done

echo

exit $err
