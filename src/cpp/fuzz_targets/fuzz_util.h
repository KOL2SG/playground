// fuzz_util.h - shared helpers for libFuzzer targets in src/cpp/fuzz_targets/
// Compile with clang and link .libfuzz/libclang_rt.fuzzer_no_main-x86_64.a.

#ifndef FUZZ_UTIL_H
#define FUZZ_UTIL_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>

// Read a vector<int> straight from the fuzz input (bytes reinterpreted as int).
inline void ConsumeIntVector(const uint8_t *Data, size_t Size, std::vector<int> &Out) {
    if (Size == 0) return;
    size_t n = Size / sizeof(int);
    if (n == 0) return;
    const int *p = reinterpret_cast<const int*>(Data);
    Out.assign(p, p + n);
}

// Read a string from the fuzz input, stopping at the first NUL byte (if any).
inline std::string ConsumeString(const uint8_t *Data, size_t Size) {
    const uint8_t *end = static_cast<const uint8_t*>(std::memchr(Data, 0, Size));
    size_t n = end ? (end - Data) : Size;
    return std::string(reinterpret_cast<const char*>(Data), n);
}

// Cap a parsed vector so fuzzing stays cheap and memory-safe.
inline void EnforceSize(const std::vector<int> &v, size_t cap, std::vector<int> &out) {
    if (v.size() > cap) {
        std::vector<int> tmp;
        tmp.reserve(cap);
        tmp.assign(v.begin(), v.begin() + static_cast<ptrdiff_t>(cap));
        out = std::move(tmp);
    } else {
        out = v;
    }
}

#endif // FUZZ_UTIL_H
