# Design

## Context

See proposal.md - Why. The repo contains C++ algorithm sources in `src/cpp/` (including a `good/` directory of reference implementations, a `low_latency/` subdirectory, and several existing `fuzz_vector_*` targets), a prebuilt libFuzzer runtime archive in `.libfuzz/`, a `.clusterfuzzlite/` harness (`project.yaml`, `Dockerfile`, `build.sh`), and ClusterFuzzLite workflows in `.github/workflows/`. Go algorithm sources live in `src/GO/`. Current fuzz coverage is fragmented: one complete C++ target (`longest_sequence.cpp`), several C++ targets that will not link (they reference undefined `Process*` functions), and no Go fuzz coverage.

## Goals / Non-Goals

**Goals:**
- Every C++ algorithm source under `src/cpp/` gets a libFuzzer target compiled against the prebuilt runtime.
- Every Go algorithm in `src/GO/` gets a `go test -fuzz` target with a seed corpus.
- C++ fuzzing stays in the existing ClusterFuzzLite CI; Go fuzzing is added as a self-hosted job.
- No production algorithm behaviour is changed.

**Non-Goals:**
- Rewriting the algorithms.
- Migrating C++ away from ClusterFuzzLite.
- Fuzzing the `good/`, `low_latency/`, or `actions-runner/` trees; those are scaffold, reference material, and the runner checkout, not production code.
- Making Go fuzzing depend on the C++ prebuilt runtime or on OSS-Fuzz storage.

## Decisions

### C++ engine: libFuzzer via prebuilt runtime
- **Choice**: link every C++ fuzz target against `.libfuzz/libclang_rt.fuzzer_no_main-x86_64.a` and compile with the sanitizer flags in `src/cpp/compile_flags.txt`.
- **Rationale**: the repo already downloads the runtime, and the ClusterFuzzLite paths expect it; the same archive CI provides lets local and CI builds match.
- **Alternative considered**: building the runtime from source. Rejected because it adds a build dependency and duplicates what CI ships.

### C++ harness layout: one target file per algorithm
- **Choice**: put new targets in `src/cpp/fuzz_targets/`, one file per algorithm, each defining `LLVMFuzzerTestOneInput` and calling the algorithm.
- **Rationale**: keeps each target self-contained and easy to add, and `build.sh` can loop over the directory.
- **Existing targets**: repair the `fuzz_vector_string`, `fuzz_vector_int`, and `fuzz_vector_bytes` targets to call real implementations (for example, wire the int-vector target to `Solution::longestConsecutive` from `longest_sequence.cpp`), or delete them if no implementation is suitable.

### Go fuzzing: native `go test -fuzz`
- **Choice**: add `*_fuzz_test.go` files under `src/GO/` using `fuzz.Fuzz`, with seed corpora in `src/GO/fuzz/<target>/`.
- **Rationale**: Go 1.24 supports fuzzing natively with no external engine; fuzzing ints, strings, and slices directly is idiomatic and needs no fuzzer library.

### CI: keep C++ on ClusterFuzzLite, add Go as a separate self-hosted job
- **Choice**: leave the C++ ClusterFuzzLite workflows unchanged (they build all targets once `build.sh` is wired) and add a Go fuzzing schedule job.
- **Rationale**: reuses the existing C++/self-hosted plumbing for C++; Go fuzzing has no ClusterFuzzLite action, so a plain `go test -fuzz` job is the simplest and most reliable on the self-hosted runner.

## Risks / Trade-offs

- [Risk] C++ fuzz targets that dereference unvalidated input must remain memory-safe under ASan/MSan. → Mitigation: link the sanitizers into the runtime archive and require the sanitizer matrix; any crash in CI blocks the PR.
- [Risk] The `fuzz_vector_*` files currently reference functions that do not exist. → Mitigation: defined in the C++ harness section of the design; repair or remove during implementation, not at proposal time.
- [Risk] Go seed corpora are small and may miss edge cases. → Mitigation: commit multiple diverse seed inputs per target and tune `-fuzzinterval` in CI.

## Migration Plan

None needed: this is an additive change. Production algorithms and existing CI stay untouched until the apply phase, at which point `build.sh` changes remain backward-compatible with the existing ClusterFuzzLite action.

## Open Questions

- Which Go functions to fuzz first given the package is `main`? → define a `src/GO/fuzz_targets.go` covering all five functions; the reader can extend later.
- Is the self-hosted runner's toolchain (clang, go 1.24.6) sufficient for libFuzzer runtime linking? → verify once by building one C++ target during apply; the prebuilt `.libfuzz` archive is x86_64 Linux and matches the runner.
