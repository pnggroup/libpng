#!/bin/sh
#
# Run the simplified API tests
err=0

echo >> pngtest-log.txt
echo "============ pngstest.sh ==============" >> pngtest-log.txt

echo "Running test-pngstest.sh"
for image in ${srcdir}/contrib/pngsuite/*.png
do
   for opts in ""
   do
      if ./pngstest --strict --log "$@" $opts $image >>pngtest-log.txt 2>&1
      then
         echo "  PASS: pngstest $opts $image"
      else
         echo "  FAIL: pngstest $opts $image"
         err=1
      fi
   done
done

exit $err
