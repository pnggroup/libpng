#!/bin/sh
#
# Run a sequence of tests quietly, without the slow
# gamma tests
err=0

echo >> pngtest-log.txt
echo "============ pngvalid-simple.sh ==============" >> pngtest-log.txt
echo "Running test-pngvalid-simple.sh"
# The options to test are:
#
# --progressive-read, --interlace on the 'transform' images
# --progressive-read on the 'size' images
#
for opts in "" --progressive-read --interlace \
   "--progressive-read --interlace" "--nostandard --size" \
   "--nostandard --size --progressive-read"
do
   if ./pngvalid  --nogamma $opts >> pngtest-log.txt 2>&1
   then
      echo "  PASS:" pngvalid --nogamma $opts
   else
      echo "  FAIL:" pngvalid --nogamma $opts
      err=1
   fi
done

exit $err
