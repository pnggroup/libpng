#!/bin/sh
#
# Run the unknown API tests
err=0
image="${srcdir}/pngtest.png"
#
# stream 4 is used for the output of the shell, pngtest-log.txt gets all the
# normal program output.
exec 4>&1 1>>pngtest-log.txt 2>&1

echo
echo "============ test-pngunknown.sh =============="

echo "Running test-pngunknown.sh" >&4

for tests in \
 "discard default=discard"\
 "save default=save"\
 "if-safe default=if-safe"\
 "vpAg vpAg=if-safe"\
 "sTER sTER=if-safe"\
 "IDAT default=discard IDAT=save"\
 "sAPI bKGD=save cHRM=save gAMA=save all=discard iCCP=save sBIT=save sRGB=save"
do
   set $tests
   test="$1"
   shift

   if ./pngunknown "$@" "$image" 4>&-
   then
      echo "  PASS: test-pngunknown $test" >&4
   else
      echo "  FAIL: test-pngunknown $test" >&4
      err=1
   fi
done

exit $err
