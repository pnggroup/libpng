#!/bin/sh
#
# Run the simplified API tests
err=0

echo >> pngtest-log.txt
echo "============ pngstest.sh ==============" >> pngtest-log.txt

echo "Running test-pngstest.sh on contrib/pngsuite/*.png"
for opts in "" "--background"
do
   if ./pngstest --log "$@" $opts ${srcdir}/contrib/pngsuite/*.png \
      >>pngtest-log.txt 2>&1
   then
      echo "  PASS: pngstest $opts"
   else
      echo "  FAIL: pngstest $opts"
      err=1
   fi
done
   
exit $err
