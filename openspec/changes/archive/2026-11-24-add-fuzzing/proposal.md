# Proposal

## Why

The repository is a collection of algorithm implementations intended for fuzzing, but coverage is incomplete and inconsistent. Only one C++ fuzzer target is wired into the build, several existing `fuzz_vector_*` targets call undefined functions and will not link, and the Go algorithms in `src/GO/` have no fuzz tests at all. ClusterFuzzLite already exists for C++ but does not build every fuzz target. Fuzzing is needed for all the C++ and Go code so that the code is exercised with coverage-guided input before regressions reach CI.

## What Changes

- **C++ fuzz targets**: add a libFuzzer target for each algorithm source under `src/cpp/`, each compiling against the prebuilt libFuzzer runtime in `.libfuzz/`. Repair the existing `fuzz_vector_string`, `fuzz_vector_int`, and `fuzz_vector_bytes` targets so they link to real implementations instead of undefined `Process*` functions.
- **C++ build**: rewrite `.clusterfuzzlite/build.sh` so it builds every fuzz target into `$OUT/` rather than the single `longest_sequence` target.
- **Go fuzz tests**: add fuzz tests (`go test -fuzz`) for the Go algorithms in `src/GO/` (`hasDuplicate`, `isAnagram`, `twoSum`, `groupAnagrams`, `checkAnagram`), with seed corpora under `src/GO/fuzz/`.
- **CI**: keep the existing C++ ClusterFuzzLite workflows on self-hosted runners; add a Go fuzzing job that runs `go test -fuzz` on a schedule for the `src/GO/` package.
- **Scope note**: no algorithm behaviour is changed; all deliverables are new test/fuzz artifacts and build wiring.

## Capabilities

### New Capabilities

- `cxx-fuzzing`: libFuzzer-based fuzzing of the C++ algorithm code in `src/cpp/`, integrated with ClusterFuzzLite and linked against the prebuilt libFuzzer runtime.
- `go-fuzzing`: native Go fuzzing of the Go algorithm code in `src/GO/` via the `go test -fuzz` infrastructure, with committed seed corpora.

### Modified Capabilities

(none - this change introduces new test artefacts only; no existing capability requirement changes)

## Impact

- `src/cpp/`: new fuzz target sources, repaired existing fuzz targets, new build wiring in `.clusterfuzzlite/build.sh`.
- `src/GO/`: new `_fuzz_test.go` files and seed corpora.
- `.clusterfuzzlite/`: updated `build.sh` to build all C++ fuzz targets.
- `.github/workflows/`: C++ cross-sanitizer fuzzing stays as is; a new Go fuzzing schedule job is added.
- `.libfuzz/`: prebuilt libFuzzer runtime reused for all C++ targets.
