Copyright (c) 2017 Glenn Randers-Pehrson

This code is released under the libpng license.
For conditions of distribution and use, see the disclaimer
and license in png.h

Files in this directory are used by the oss-fuzz project
(https://github.com/google/oss-fuzz/tree/master/projects/libpng).
for "fuzzing" libpng.

They were licensed by Google Inc, using the BSD-like Chromium license,
which may be found at https://cs.chromium.org/chromium/src/LICENSE, or, if
noted in the source, under the Apache-2.0 license, which may
be found at http://www.apache.org/licenses/LICENSE-2.0 .
If they have been modified, the derivatives are copyright Glenn Randers-Pehrson
and are released under the same licenses as the originals.  Several of
the original files (libpng_read_fuzzer.options, png.dict, project.yaml)
had no licensing information; we assumed that these were under the Chromium
license. Any new files are released under the libpng license (see png.h).

The files are
                            Original
 Filename                   or derived   Copyright          License
 =========================  ==========   ================   ==========
 Dockerfile*                derived      2017, Glenn R-P    Apache 2.0
 build.sh                   derived      2017, Glenn R-P    Apache 2.0
 libpng_read_fuzzer.cc      derived      2017, Glenn R-P    Chromium
 libpng_read_fuzzer.options original     2015, Chrome Devs  Chromium
 png.dict                   original     2015, Chrome Devs  Chromium
 README.txt (this file)     original     2017, Glenn R-P    libpng

 * Dockerfile is a copy of the file used by oss-fuzz. build.sh,
   png.dict and libpng_read_fuzzer.* are the actual files used by oss-fuzz,
   which retrieves them from the libpng repository at Github.

To do: exercise the png encoder.

The progressive reader fuzzer feeds each input as one block and as one-byte
blocks. It checks completion without reported errors or warnings; for such
completed decodes it compares dimensions, row stride, row callback count and
reconstructed pixels. Reported warning/error decodes are excluded from the
completed-image comparison:
reaching IEND after a damaged compressed stream does not establish valid pixels.
Warnings can only be observed when libpng is built with warning support;
the current OSS-Fuzz build disables that support. Errors are still observed.
The target also checks row callback bounds and combines Adam7 rows through
png_progressive_combine_row. Input, dimension and allocation limits bound cost.

The build script instruments and statically links zlib when its source checkout
is available at $SRC/zlib, as provided by the OSS-Fuzz libpng Dockerfile. This is
needed for meaningful MemorySanitizer results. Other integrations retain the
system-zlib fallback and must supply an instrumented dependency for MSan.

The standalone libpng_progressive_fuzzer_test.cc driver checks that the existing
badadler.png fixture is excluded from the completed-image oracle and that a
clean PNG completes. In an OSS-Fuzz build environment it can be built separately
from the fuzz targets (keep its executable outside $OUT):

    $CXX $CXXFLAGS -std=c++11 -I. \
        contrib/oss-fuzz/libpng_progressive_fuzzer_test.cc \
        -lFuzzingEngine .libs/libpng16.a $SRC/zlib/libz.a \
        -o $WORK/libpng_progressive_test
    $WORK/libpng_progressive_test \
        contrib/testpngs/crashers/badadler.png contrib/pngsuite/basn0g08.png

Additional PNG paths are accepted after those two fixtures. The --invalid-row
and --invalid-pass modes deliberately invoke invalid callbacks and must abort
with PROGRESSIVE_CALLBACK_INVALID; they test that callback contract failures
are not recovered as ordinary decode errors.
