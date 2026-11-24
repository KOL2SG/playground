#include <cstdint>
#include <cstddef>
#include <cstring>
#include <vector>
#include <string>
#include "fuzz_util.h"

// Algorithm engine (main and libfuzzer entry stripped for this target).
#include "variadic_engine.cpp"

// libFuzzer entry point.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    int x = 0, y = 0; if (Size >= 8) { const int* p = reinterpret_cast<const int*>(Data); x = p[0]; y = p[1]; }
    getBinaryRep(x, y);
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
