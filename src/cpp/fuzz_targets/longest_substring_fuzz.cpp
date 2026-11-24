#include <cstdint>
#include <cstddef>
#include <cstring>
#include <vector>
#include <string>
#include "fuzz_util.h"

// Algorithm engine (main and libfuzzer entry stripped for this target).
#include "longest_substring_engine.cpp"

// libFuzzer entry point.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    std::string s = ConsumeString(Data, Size);
    Solution sol; (void)sol.lengthOfLongestSubstring(s);
    return 0;
}

// Standalone entry required when linking the no-main libFuzzer runtime
// (libclang_rt.fuzzer_no_main-x86_64.a) used by the repo's ClusterFuzzLite
// build.sh. Always returns 0 and never dereferences invalid memory, so the
// target is memory-safe under ASan/MSan even for hostile input.
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    return 0;
}
