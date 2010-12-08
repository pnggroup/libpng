#!/bin/sh
#
# Run a sequence of tests quietly, without the slow
# gamma tests
err=0

for opts in "" --progressive-read --interlace \
   "--progressive-read --interlace"
do
   if ./pngvalid -q --nogamma $opts
   then
      echo "PASS:" pngvalid --nogamma $opts
   else
      echo "FAIL:" pngvalid --nogamma $opts
      err=1
   fi
done

exit $err
