# Design — unified C++ fuzz harness

## Scope

One libFuzzer entry point that, for every input, routes it to exactly one C++ algorithm based on a leading selector byte. Keeps every algorithm's original implementation intact; the only new shared surface is the aggregated solver class and the dispatcher.

## Request shape

```text
[selector:1][payload...]
  selector == 0  -> find_conditions      (return_negative)
  selector == 1  -> two_sum              (Solution::twoSum)
  selector == 2  -> two_sum_hash         (Solution::twoSum(vec, target))
  selector == 3  -> longest_sequence     (Solution::longestConsecutive)
  selector == 4  -> maxProduct           (Solution::maxProduct)
  selector == 5  -> join_point           (join_point(n1, n2))
  selector == 6  -> variadic             (getBinaryRep(x, y))
  selector == 7  -> sudoko               (Solution::isValidSudoku)
  selector == 8  -> topK                 (topKFrequentBucketSort(vec, k))
  selector == 9  -> longest_substring    (Solution::lengthOfLongestSubstring)
  other         -> ignore (cheap exit, no allocation)
```

## Dispatcher

Single `extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)` in `all_targets_fuzz.cpp`. A `switch(Data[0])` on the selector; each arm parses the payload using the existing helpers (`ConsumeIntVector`, `ConsumeString`, fixed-size board memcpy) from `fuzz_util.h`, then calls the engine. The switch makes the five-target case explicit and keeps one clear path through the file.

```text
LLVMFuzzerTestOneInput(Data, Size)
  Size == 0 -> return 0           (no-op seeds)
  selector = Data[0]
  switch (selector)
    case 0..9: parse + call engine
    default : return 0
  return 0
```

No `main()` in the harness — it links the prebuilt no-main libFuzzer runtime (`.libfuzz/libclang_rt.fuzzer_no_main-x86_64.a`) which supplies `LLVMFuzzerMain`.

## Shared solver

`all_solvers.hpp` defines one `class Solution` containing all seven methods (`twoSum`, `longestConsecutive`, `longestConsecutive2`, `longestConsecutive3`, `maxProduct`, `isValidSudoku`, `lengthOfLongestSubstring`). Each method body is carried over verbatim from its engine file; no algorithm logic is rewritten. The six engine `.cpp` files remove their duplicate `class Solution` definition (replaced with `#include "all_solvers.hpp"`) so they compile standalone and include-safe. The dispatcher keeps a single `Solution sol;` and drives only the member method cases against it.

## Build

- `all_targets_fuzz.cpp` compiles with `-std=c++23` (matching the rest of the targets), includes `fuzz_util.h` (single helper header), `all_solvers.hpp`, and the four free-function engine files (`find_conditions_engine.cpp`, `join_point_engine.cpp`, `variadic_engine.cpp`, `topK_engine.cpp`) which have no class conflict.
- Link: `$CXX ... -o $OUT/all_targets $LIB_FUZZING_ENGINE` where `$LIB_FUZZING_ENGINE` = `.libfuzz/libclang_rt.fuzzer_no_main-x86_64.a`.

## Seeds

`src/cpp/fuzz_targets/seeds/` holds ten directories, one per selector (`00_find_conditions/` … `09_longest_substring/`). Each directory contains `.rc` files already in that algorithm's native wire format (int vectors, strings, 81-char boards), so `all_targets` reproduces a given algorithm's corpus by running it under whichever selector its file begins with.

## Verification

- Build: `$OUT/all_targets` exists with no undefined-symbol errors under `-fsanitize=address,undefined`.
- Run each case over its seed corpus under ASan/UBSan/MSan and confirm no crash; negative cases (`find_conditions`, `join_point`) exercise the divergent paths found in the existing per-target runs.
- Sanity: `go test -fuzz` equivalent for C++ is `./all_targets -runs=N` per selector (libFuzzer `./all_targets` default harness); each oracle call is a deterministic single-algorithm invocation, so corpus seeds and crashes remain attributable to one algorithm.
