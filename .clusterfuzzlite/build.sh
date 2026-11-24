#!/bin/bash -eu

# build project
# e.g.
# ./autogen.sh
# ./configure
# make -j$(nproc) all

# build fuzzers
# e.g.
# $CXX $CXXFLAGS -std=c++11 -Iinclude \
#     /path/to/name_of_fuzzer.cc -o $OUT/name_of_fuzzer \
#     $LIB_FUZZING_ENGINE /path/to/library.a

$CXX $CXXFLAGS -std=c++23 -Isrc/cpp/fuzz_targets \
    src/cpp/fuzz_targets/all_targets_fuzz.cpp -o $OUT/all_targets \
    $LIB_FUZZING_ENGINE
