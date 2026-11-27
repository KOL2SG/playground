# Tasks

## 1. Consolidation

- [x] 1.1 Deduplicate the shared helpers: the canonical helper is `fuzz_util.hpp` (the `.h` is deleted); all target and harness includes resolve to it.
- [x] 1.2 In `find_conditions_engine.cpp`, `join_point_engine.cpp`, `variadic_engine.cpp`, and `topK_engine.cpp` (free-function engines), the files compile when `#include`d by the harness (they are pulled into the unified binary).
- [x] 1.3 In the six `class Solution` engine files, the duplicate `class Solution` definition is replaced with `#include "all_solvers.hpp"`; each still compiles standalone.

## 2. Aggregated solver

- [x] 2.1 Create `all_solvers.hpp` with one solver class per algorithm (TwoSum, LongestSequence, MaxProduct, Sudoku, TwoSumHash, LongestSubstring); no class Solution collides and each method keeps its original signature/body.
- [x] 2.2 Verify the method bodies match the originals; no algorithm logic is changed.

## 3. Unified fuzzer harness

- [x] 3.1 Create `all_targets_fuzz.cpp` with one `extern "C" int LLVMFuzzerTestOneInput(...)` selector dispatcher over the ten cases (0..9).
- [x] 3.2 Ensure no `main()` is defined; the harness links the prebuilt libFuzzer runtime.
- [x] 3.3 Compile `all_targets_fuzz.cpp` with `-std=c++23` and link the full libFuzzer runtime (`.libfuzz/libclang_rt.fuzzer-x86_64.a`); produces `$OUT/all_targets` with no undefined symbols.

## 4. Seed corpora

- [x] 4.1 Create `src/cpp/fuzz_targets/seeds/` with ten directories (`00_find_conditions/` … `09_longest_substring/`).
- [x] 4.2 Place representative `.rc` inputs in each selector directory; each stored input begins with that selector byte.

## 5. Build and CI

- [x] 5.1 Update `.clusterfuzzlite/build.sh` to build one binary (`$OUT/all_targets`).
- [x] 5.2 Update `cflite_build.yml`, `cflite_pr.yml`, `cflite_batch.yml`, and `cflite_cron.yml` to build/run the single harness (`$OUT/all_targets`); confirm findings on crash.

## 6. Verification

- [x] 6.1 Build the harness and confirm it runs under ASan/UBSan (MSan skipped: engine functions are not memory-instrumented).
- [x] 6.2 Confirm every selector case reaches its algorithm; no link/symbol errors (ASan/UBSan runs covered all ten selectors).
- [x] 6.3 Confirm the known-bug inputs are reachable through their selectors under ASan/UBSan: `maxProduct` empty-vector SEGV (selector 4) and `join_point` stack overflow (selector 5).
