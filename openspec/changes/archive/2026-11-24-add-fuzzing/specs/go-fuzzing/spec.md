# Spec Delta

## Purpose

Provides reproducible fuzz-based test coverage for the Go algorithm code in `src/GO/` using Go's native fuzzing (`go test -fuzz`), with committed seed corpora so fuzzing reproduces from a clean checkout.

## ADDED Requirements

### Requirement: A fuzz test exists for every Go algorithm in src/GO
The system SHALL provide a fuzz test for each algorithm function in the `src/GO/` package: `hasDuplicate`, `isAnagram`, `twoSum`, `groupAnagrams`, and `checkAnagram`.
#### Scenario: Fuzz targets enumerate the package
- **WHEN** `go test ./...` runs with fuzzing enabled in `src/GO/`
- **THEN** each algorithm listed above has an associated fuzz target that compiles

### Requirement: Fuzz tests use native Go fuzzing
Each fuzz test SHALL use `go test -fuzz` semantics: a fuzz target named `F<Name>` that consumes fuzzed input and invokes the algorithm function with it.
#### Scenario: A fuzz target runs
- **WHEN** `go test -fuzz=Run -fuzztime=5s ./...` runs in `src/GO/`
- **THEN** every fuzz target executes without error and reports no panic

### Requirement: Seed corpora are committed
Each fuzz test SHALL ship a seed corpus under `src/GO/fuzz/<target>/` so fuzzing reproduces from a clean checkout and catches the same bugs in CI.
#### Scenario: Corpora are present
- **WHEN** `git clone` followed by `go test ./...` runs in `src/GO/`
- **THEN** the seed corpora under `src/GO/fuzz/` exist and are included in the repository

### Requirement: Fuzz tests do not change production behaviour
Adding fuzz tests SHALL not alter the algorithm outputs or panic behaviour of the production Go code.
#### Scenario: Production behaviour unchanged
- **WHEN** the production functions are called with normal inputs
- **THEN** they return identical results with or without the new fuzz tests present
