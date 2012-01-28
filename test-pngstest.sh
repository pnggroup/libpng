#!/bin/sh
#
# Run the simplified API tests
err=0

echo >> pngtest-log.txt
echo "============ pngstest.sh ==============" >> pngtest-log.txt

echo "Running test-pngstest.sh on contrib/pngsuite/*.png"
for opts in "" "--background"
do
   if ./pngstest --log "$@" $opts ${srcdir}/contrib/pngsuite/bas*.png \
      >>pngtest-log.txt 2>&1
   then
      echo "  PASS: pngstest $opts (basic images)"
   else
      echo "  FAIL: pngstest $opts (basic images)"
      err=1
   fi
   if ./pngstest --log "$@" $opts ${srcdir}/contrib/pngsuite/ft*.png \
      >>pngtest-log.txt 2>&1
   then
      echo "  PASS: pngstest $opts (transparent images)"
   else
      echo "  FAIL: pngstest $opts (transparent images)"
      err=1
   fi
done
   
exit $err
