# Spec Delta

## Purpose

Provides a single fuzzer entry point that exercises every C++ algorithm in `src/cpp/` through one `LLVMFuzzerTestOneInput`, selected per input by a leading selector byte, with committed per-selector seed corpora and sanitizer builds.

## ADDED Requirements

### Requirement: One fuzzer binary covers all algorithms
The system SHALL provide one fuzzer target binary that runs every algorithm in `src/cpp/` when fed fuzz input, producing no link errors.

#### Scenario: All algorithms reachable
- **WHEN** `all_targets` is built from `all_targets_fuzz.cpp` against the no-main libFuzzer runtime
- **THEN** one executable exists and each of the nine algorithms is reachable through the selector dispatch

### Requirement: Selector dispatch routes one algorithm per input
The system SHALL route each input to exactly one algorithm using the leading selector byte and SHALL skip inputs with an unknown selector without allocating.

#### Scenario: Well-formed selector
- **WHEN** an input begins with a selector byte in 0..9
- **THEN** only the algorithm for that selector is executed
- **WHEN** an input begins with any other byte
- **THEN** it is skipped with no allocation

### Requirement: Seed corpora are committed per selector
The system SHALL ship a seed corpus under `src/cpp/fuzz_targets/seeds/<selector>/` so each algorithm reproduces from a clean checkout.

#### Scenario: Corpora present
- **WHEN** `git clone` followed by building the harness
- **THEN** one seed directory per selector exists and its stored inputs begin with that selector byte

### Requirement: Existing algorithm behavior is preserved
Fuzzing MUST reach the algorithms through their original entry points and input shapes so that the existing per-target coverage is conserved.

#### Scenario: Known-bug regression
- **WHEN** the harness is run under ASan on the inputs that previously exposed the `find_conditions` divergence and `maxProduct` SEGV
- **THEN** those inputs are reachable through their selectors and reproduce diagnostic coverage

### Requirement: Build produces one fuzzer binary
Building the project SHALL produce exactly one fuzzer binary under `$OUT/` with no undefined symbols after linking against the no-main libFuzzer runtime.

#### Scenario: Single binary
- **WHEN** `.clusterfuzzlite/build.sh` builds the harness
- **THEN** one executable named after `all_targets` is produced under `$OUT/`

### Requirement: Sanitizer matrix is enforced
The build SHALL exercise the unified harness under the address, undefined, and memory sanitizer matrix on the self-hosted runner.

#### Scenario: PR sanitizer job
- **WHEN** a pull request changes a C++ source file
- **THEN** the PR job builds and runs the harness under ASan and reports any crash as a finding
