# Proposal — unified C++ fuzz harness

## Why

The C++ algorithm code in `src/cpp/` is protected by ten separate fuzz targets under `src/cpp/fuzz_targets/`, each a near-identical self-contained file (a `LLVMFuzzerTestOneInput`, a `main`, and one engine include). Maintaining ten essentially-duplicate targets makes it hard to add a new algorithm, and the per-target `main` + per-target corpus scheme does not scale. One harness that dispatches to every algorithm with a single input byte, keyed on a selector byte, collapses the matrix into one binary and one corpus convention.

## What Changes

- Introduce a single fuzzer source, `src/cpp/fuzz_targets/all_targets_fuzz.cpp`, whose `LLVMFuzzerTestOneInput` reads a selector byte at `Data[0]` and dispatches to one algorithm per case (0=`find_conditions`, 1=`two_sum`, 2=`two_sum_hash`, 3=`longest_sequence`, 4=`maxProduct`, 5=`join_point`, 6=`variadic`, 7=`sudoko`, 8=`topK`, 9=`longest_substring`).
- Each case reuses the existing engine implementation unchanged; only the entry structure changes.
- Replace the ten per-target `main` definitions so the single harness links cleanly against the prebuilt no-main libFuzzer runtime (`.libfuzz/libclang_rt.fuzzer_no_main-x86_64.a`).
- Consolidate the shared helpers (`fuzz_util.h` vs `fuzz_util.hpp`) to a single header.
- Add a per-selector seed corpus under `src/cpp/fuzz_targets/seeds/` so each algorithm reproduces from a clean checkout.
- Update `.clusterfuzzlite/build.sh` and the `cflite_*` CI workflows to build one binary (`$OUT/all_targets`) with the address/undefined/memory sanitizer matrix, instead of ten separate binaries.

## Capabilities

### New Capabilities
- `cpp-fuzz-matrix`: A single fuzzer harness that exercises all C++ algorithms in `src/cpp/` through one entry point, selected per input by a leading selector byte, with per-selector seed corpora and sanitizer builds.

### Modified Capabilities
- None (existing spec deltas: `cxx-fuzzing`, `go-fuzzing` cover the *separate* per-target fuzzing model and are unaffected).

## Impact

- **New files:** `src/cpp/fuzz_targets/all_targets_fuzz.cpp`, `src/cpp/fuzz_targets/all_solvers.hpp`, `src/cpp/fuzz_targets/seeds/` (10 selector dirs), `.clusterfuzzlite/build.sh` change.
- **Modified files:** the six engine files that define `class Solution` (become include-safe against `all_solvers.hpp`), `fuzz_util.h`/`fuzz_util.hpp` (dedup).
- **CI:** `.clusterfuzzlite/build.sh`, `.github/workflows/cflite_build.yml`, `cflite_pr.yml`, `cflite_batch.yml`, `cflite_cron.yml`.
- **Deliverable:** one fuzzer binary `/out/all_targets` that runs ASan/UBSan/Msan across all algorithms per input.
