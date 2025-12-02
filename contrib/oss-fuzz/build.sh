#!/bin/bash -eu

# Copyright 2017-2018 Glenn Randers-Pehrson
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
# Revisions by Glenn Randers-Pehrson, 2017:
# 1. Build only the library, not the tools (changed "make -j$(nproc) all" to
#     "make -j$(nproc) libpng16.la").
# 2. Disabled WARNING and WRITE options in pnglibconf.dfa.
# 3. Build zlib alongside libpng
################################################################################

# Disable logging via library build configuration control.
cat scripts/pnglibconf.dfa | \
  sed -e "s/option STDIO/option STDIO disabled/" \
      -e "s/option WARNING /option WARNING disabled/" \
      -e "s/option WRITE enables WRITE_INT_FUNCTIONS/option WRITE disabled/" \
> scripts/pnglibconf.dfa.temp
mv scripts/pnglibconf.dfa.temp scripts/pnglibconf.dfa

# build the libpng library.
autoreconf -f -i
./configure --with-libpng-prefix=OSS_FUZZ_
make -j$(nproc) clean
make -j$(nproc) libpng16.la

# add seed corpus for all fuzz drivers 
find $SRC/libpng -name "*.png" | grep -v crashers | \
     xargs zip $OUT/libpng_read_fuzzer_seed_corpus.zip

find $SRC/libpng -name "*.png" | grep -v crashers | \
     xargs zip $OUT/libpng_dotrans_alpha_seed_corpus.zip

find $SRC/libpng -name "*.png" | grep -v crashers | \
     xargs zip $OUT/libpng_dotrans_rgb_seed_corpus.zip

find $SRC/libpng -name "*.png" | grep -v crashers | \
     xargs zip $OUT/libpng_dotrans_setbackground_seed_corpus.zip

find $SRC/libpng -name "*.png" | grep -v crashers | \
     xargs zip $OUT/libpng_dotrans_filter_seed_corpus.zip

find $SRC/libpng -name "*.png" | grep -v crashers | \
     xargs zip $OUT/libpng_setunknown_seed_corpus.zip

find $SRC/libpng -name "*.png" | grep -v crashers | \
     xargs zip $OUT/libpng_update_twice_seed_corpus.zip

find $SRC/libpng -name "*.png" | grep -v crashers | \
     xargs zip $OUT/libpng_app_error_seed_corpus.zip


# To execute all the fuzz drivers
for fuzzer in $SRC/libpng/contrib/oss-fuzz/*.cc; do
  fuzzer_basename=$(basename -s .cc $fuzzer)

  $CXX $CXXFLAGS \
      -std=c++11 -I. \
      $SRC/libpng/contrib/oss-fuzz/$fuzzer_basename.cc \
      -o $OUT/$fuzzer_basename \
      $LIB_FUZZING_ENGINE \
      .libs/libpng16.a -lz

  cp $SRC/libpng/contrib/oss-fuzz/*.options $OUT/
  cp $SRC/libpng/contrib/oss-fuzz/*.dict $OUT/

done


