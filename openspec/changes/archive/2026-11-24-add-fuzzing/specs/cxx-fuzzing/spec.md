# Spec Delta

## Purpose

Provides fuzz-target-based test coverage (libFuzzer) for the C++ algorithm code in `src/cpp/`, integrated with ClusterFuzzLite and linked against the prebuilt libFuzzer runtime in `.libfuzz/`.

## ADDED Requirements

### Requirement: A fuzz target exists for each C++ algorithm source
The system SHALL provide one fuzz target binary per algorithm source file under `src/cpp/`, and building the project SHALL produce every target into `$OUT/`.
#### Scenario: All targets build
- **WHEN** `.clusterfuzzlite/build.sh` runs with the libFuzzer runtime and a sanitizer flags file
- **THEN** one executable named after each source file exists under `$OUT/`, with no link errors

### Requirement: Fuzz targets link against the prebuilt libFuzzer runtime
Each fuzz target SHALL be compiled with the prebuilt `libclang_rt.fuzzer_no_main-x86_64.a` from `.libfuzz/` and SHALL have no undefined symbols after linking.
#### Scenario: A target with no missing symbols
- **WHEN** `clang++` links a fuzz target against `.libfuzz/libclang_rt.fuzzer_no_main-x86_64.a`
- **THEN** the resulting binary runs and processes a fuzzer-provided input without symbol-resolution errors

### Requirement: Existing fuzz_vector targets are usable
The `fuzz_vector_string`, `fuzz_vector_int`, and `fuzz_vector_bytes` targets SHALL either link to a real implementation of their target function or be replaced so they compile.
#### Scenario: Existing targets compile
- **WHEN** the build runs for every existing fuzz target
- **THEN** no target reports an undefined-symbol or compile error

### Requirement: ClusterFuzzLite builds and runs all targets across sanitizers on self-hosted runners
The C++ ClusterFuzzLite workflows SHALL build and run the full set of fuzz targets under the address, undefined, and memory sanitizer matrix on the self-hosted `qwen3-coder` runner.
#### Scenario: PR sanitizer job
- **WHEN** a pull request changes a C++ source file
- **THEN** the PR job builds and runs affected targets under at least the address sanitizer and reports any crash as a finding
