# Tasks

## 1. C++ fuzz target scaffolding
- [ ] 1.1 Create `src/cpp/fuzz_targets/` and a template fuzz target that links the prebuilt libFuzzer runtime
- [ ] 1.2 Add a fuzz target for `longest_sequence.cpp` and verify `build.sh` produces a runnable binary under `$OUT/`
- [ ] 1.3 Add fuzz targets for the remaining C++ algorithm files (`two_sum`, `find_conditions`, `topK`, etc.) and verify each compiles

## 2. Repair and replace existing C++ fuzz targets
- [ ] 2.1 Inspect `fuzz_vector_string`, `fuzz_vector_int`, and `fuzz_vector_bytes` to locate their missing `Process*` implementations
- [ ] 2.2 Wire each target to a real algorithm or delete it if none exists
- [ ] 2.3 Verify all existing C++ fuzz targets compile under at least one sanitizer

## 3. C++ build wiring
- [ ] 3.1 Rewrite `.clusterfuzzlite/build.sh` to build every fuzz target in `src/cpp/fuzz_targets/` into `$OUT/`
- [ ] 3.2 Verify `build.sh` returns 0 and produces one binary per source file

## 4. Go fuzz tests
- [ ] 4.1 Add `src/GO/hasDuplicate_fuzz_test.go` targeting `hasDuplicate` with a seed corpus
- [ ] 4.2 Add `src/GO/isAnagram_fuzz_test.go` targeting `isAnagram` with a seed corpus
- [ ] 4.3 Add fuzz tests for `twoSum`, `groupAnagrams`, and `checkAnagram`
- [ ] 4.4 Verify `go test -fuzz=Run -fuzztime=5s ./...` passes in `src/GO/`

## 5. Seed corpora for Go
- [ ] 5.1 Add seed corpus directories under `src/GO/fuzz/<target>/` with diverse inputs
- [ ] 5.2 Verify `go test ./...` picks up the corpora without errors

## 6. CI integration
- [ ] 6.1 Add a Go fuzzing schedule job to `.github/workflows/` targeting the self-hosted runner
- [ ] 6.2 Verify the Go fuzzing job executes on the self-hosted runner

## 7. Verification summary
- [ ] 7.1 Build all C++ fuzz targets under the address, undefined, and memory sanitizer matrix
- [ ] 7.2 Run every Go fuzz target with `-fuzztime`
- [ ] 7.3 Confirm no production algorithm output changed
