#!/usr/bin/env bash
set -eu

# Copyright 2024 Cosmin Truta
# Copyright 2017 Glenn Randers-Pehrson
# Copyright 2016 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
################################################################################

# Disable logging via library build configuration control.
sed -e "s/option STDIO/option STDIO disabled/" \
    -e "s/option WARNING /option WARNING disabled/" \
    -e "s/option WRITE enables WRITE_INT_FUNCTIONS/option WRITE disabled/" \
    scripts/pnglibconf.dfa >scripts/pnglibconf.dfa.tmp
mv -f scripts/pnglibconf.dfa.tmp scripts/pnglibconf.dfa

# Build the libpng library ("libpng16.la"), excluding the auxiliary tools.
autoreconf -f -i
./configure --with-libpng-prefix=OSS_FUZZ_
make -j$(nproc) clean
make -j$(nproc) libpng16.la

# Build libpng_read_fuzzer.
$CXX $CXXFLAGS -std=c++11 -I. \
     $SRC/libpng/contrib/oss-fuzz/libpng_read_fuzzer.cc \
     -o $OUT/libpng_read_fuzzer \
     -lFuzzingEngine .libs/libpng16.a -lz

# Add seed corpus.
find $SRC/libpng -name "*.png" | grep -v crashers | \
     xargs zip $OUT/libpng_read_fuzzer_seed_corpus.zip

cp $SRC/libpng/contrib/oss-fuzz/*.dict \
   $SRC/libpng/contrib/oss-fuzz/*.options \
   $OUT/
