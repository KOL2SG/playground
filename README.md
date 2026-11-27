# 📦 playground – Vulnerability Scanning + LLM Code Review + Multi-Language Fuzzing

**What this repository is for**

It is a **playground / demo** showing how to combine:

1. **Multi-scanner vulnerability scanning** (Draugr) across C++, Go, Docker, GitHub-Actions, secrets, etc.
2. **LLM-powered code review** (self-hosted Ollama) that reviews Draugr reports, diffs, and fuzz outputs, then posts a concise, actionable comment on a pull request.
3. **Fuzzing** (C++ via ClusterFuzzLite / libFuzzer, and Go via the native `go test -fuzz`).
4. **AI report automation & optimization** — the user is experimenting with AI to automatically produce, condense, and optimize security reports so reviewers get a short, prioritized, fixed-ready summary instead of raw scanner output.

Everything needed for the pipeline lives inside this repository so it can be run locally or fully automated via GitHub Actions.

---

## 📋 Repository contents (layout)

| Path | What it is |
|------|----------|
| `src/cpp/` | C++ algorithm sources, fuzz targets, and the ClusterFuzzLite harness (`src/cpp/fuzz_targets/all_targets_fuzz.cpp` — a single unified selector-byte dispatcher). |
| `src/GO/` | Go code + native fuzz tests (`go test -fuzz`) for `hasDuplicate`, `isAnagram`, `twoSum`, `groupAnagrams`, `checkAnagram`. |
| `GO/` | Older Go demo files (`go.mod`, `duplicate.go`, etc.). |
| `.clusterfuzzlite/` | ClusterFuzzLite configuration: `project.yaml` (`language: c++`), `Dockerfile`, `build.sh`, and `scripts/ai_clusterfuzz_review.py`. |
| `scripts/` | AI review scripts: `ai_review.py`, `ai_clusterfuzz_review.py`, `ai_security_review.py`, `pr_review.py`. |
| `draugr.saga.yaml` | Draugr configuration: enabled controls (SAST, SCA, IaC, secrets), gate `failOn: P1`, output locations, publisher settings. |
| `draugr-out/` (generated) | Draugr scan output: `report.json` (human-readable summary) and `results.sarif` (GitHub Code Scanning SARIF). |
| `.github/workflows/` | GitHub Actions workflows (see below). |
| `docker-compose.yml` | `open-webui` service used to serve the self-hosted LLM (optional local setup). |

---

## 🏗️ Pipeline / workflow overview

The repository wires together three pipelines that run either locally or in GitHub Actions.

```
                    ┌─────────────────────────── GitHub Actions ───────────────────────────┐
                    │  (self-hosted runner: [self-hosted, Linux, X64, qwen3-coder])      │
  Push / PR ───────►│  ┌──────────────┐   ┌──────────────┐   ┌──────────────────────────┐ │
  (opened/sync)───►│  │ clusterfuzzlite │──►│  go-fuzz.yml │──►│  AI review (Ollama)    │ │
  (C++ libFuzzer) │  │ build_fuzzers │   │ (Go native)  │   │  scripts/ai_review.py etc.│ │
  (C++ fuzzers)   │  │ run_fuzzers   │   │  -fuzz targets│   │  + posting comments      │ │
                    │  └──────────────┘   └──────────────┘   └─────────────┬──────────┘ │
                    │                                                     │                  │
                    │  ┌──────────────┐   ┌──────────────┐   ┌───────────▼──────────┐  │
                    │  │  Draugr scan  │──►│  AI review  │──►│  post PR comment /    │ │
                    │  │  (SAST/SCA/   │   │  of reports │   │  SARIF upload to      │ │
                    │  │  IaC/Secrets) │   │  (Ollama)   │   │  code scanning        │ │
                    │  │  + gates      │   │  (Ollama)   │   │                     │  │
                    │  └──────────────┘   └──────────────┘   └────────────────────┘  │
                    └──────────────────────────────────────────────────────────────┘
```

**Pipeline stages**

1. **Build / fuzz** — ClusterFuzzLite builds and runs a unified libFuzzer target on push/PR/cron/batch (C++). Separate `go-fuzz.yml` runs Go's native `go test -fuzz`.
2. **Vulnerability scan** — Draugr scans the code (SAST/SCA/IaC/Secrets), producing `draugr-out/report.json` + `results.sarif`.
3. **AI report automation & optimization** — an LLM (Ollama) reviews the Draugr reports / diffs / fuzz reports and outputs a concise, structured, prioritized review (issue tables, severity, file:line, recommendations). This is the AI report automation & optimization experiment.
4. **Post / upload** — the AI review is posted as a GitHub PR comment; the SARIF is uploaded to GitHub Code Scanning so findings can be browsed in the Security tab.

---

## 🤖 1. AI Report Automation & Optimization (experiment)

The user is experimenting with **AI to automate and optimize security reports**. Instead of dumping raw scanner output into a review comment, the AI condenses, prioritises, and formats findings into a structured, actionable review.

**How it works**

- Draugr produces `draugr-out/report.json` (and the SARIF). These raw outputs are large and generic.
- The AI review scripts (`scripts/ai_review.py`, `scripts/ai_clusterfuzz_review.py`, `scripts/ai_security_review.py`, `scripts/ai_advsec_review.py`) send the raw report (or PR diff, fuzz SARIF, or GitHub code-scanning alerts) to a self-hosted Ollama model.
- The model returns a structured Markdown review:
  - a short verdict (`solid` / `ok with minor issues` / `fixes needed`);
  - a findings table (`File | Line | Severity | Fix`);
  - a "High-risk" list (memory corruption, UB, crash, data race, leak);
  - passing checks (compile gate, sanitizers, formatting);
  - concrete fixes, not just descriptions.

**Key scripts / workflows**

| File | Role |
|------|------|
| `scripts/ai_review.py` | Reviews a PR diff; posts comment to PR. |
| `scripts/ai_clusterfuzz_review.py` | Reviews the clusterfuzz SARIF report; posts comment. |
| `scripts/ai_security_review.py` | Reviews the Draugr JSON report; posts comment. |
| `scripts/pr_review.py` | Local/Ollama PR review (uses `http://localhost:11434`, model `hf.co/unsloth/Qwen3.8-27B-GGUF:UD-Q4_K_M`). |
| `.github/workflows/ai-code-reviews.yml` | Runs `ai_review.py` on a PR. |
| `.github/workflows/ai-security-review.yml` | Runs Draugr, then `ai_security_review.py`. |
| `.github/workflows/security_check.yml.bak`, `draugr.yml.bak` | Backup / reference Draugr workflows. |

**Experiment notes**

- Report optimisation: the LLM's output is a compact, prioritised Markdown table rather than the original verbose JSON. This is the "optimize" part of the experiment.
- Report automation: the same pipeline can be reused to produce reports for Draugr, ClusterFuzzLite, and Go fuzzing without manual summarising.
- The model is configurable via the `OLLAMA_MODEL` environment variable (default `qwen3.8:latest`). Any compatible Ollama model can be swapped in.

> **Self-hosted Ollama is required** for the AI review steps (no remote API key is used). A local Ollama is started with `ollama serve &` and a model pulled with `ollama pull qwen3-coder:30b` (or replaced with any model).

---

## 💥 2. C++ fuzzing (ClusterFuzzLite / libFuzzer)

The C++ side uses **ClusterFuzzLite** (libFuzzer) with a single unified harness.

### Files

- `.clusterfuzzlite/Dockerfile` — builds the fuzzer environment (`oss-fuzz-base`), copies the repo, installs `make autoconf automake libtool`.
- `.clusterfuzzlite/project.yaml` — `language: c++`.
- `.clusterfuzzlite/build.sh` — build script run inside the image.
- `src/cpp/fuzz_targets/all_targets_fuzz.cpp` — a single unified harness with a **selector-byte dispatcher**. One binary `$OUT/all_targets` executes each algorithm's fuzzer based on a selector byte.
- `src/cpp/` — algorithm sources + a prebuilt libFuzzer runtime archive `.libfuzz/libclang_rt.fuzzer_no_main-x86_64.a` (no `LLVMFuzzerMain`) plus `.libfuzz/libclang_rt.fuzzer-x86_64.a` (provides `main`).

### CI workflows (C++)

| Workflow | Trigger | What it does |
|------|---------|--------------|
| `cflite_build.yml` | push to `main` | Continuous build: builds the fuzz targets with the address (`address`), undefined (`undefined`), and memory (`memory`) sanitizers. |
| `cflite_pr.yml` | pull_request | PR fuzzing (`mode: code-change`, `fuzz-seconds: 600`, `output-sarif: true`). Includes a `-- Fetch PR merge ref` step to fix `git call failed: ambiguous argument 'refs/remotes/pull/<N>/merge'`. |
| `cflite_cron.yml` | daily cron | Pruning + coverage jobs (sanitizer `coverage`). |
| `cflite_batch.yml` | every 6 hours | Batch fuzzing with the address sanitizer (`fuzz-seconds: 3600`). |
| `clang-code-coverage.yml` | push | Per-file Clang-18 coverage builds (`-fsanitize=fuzzer,address`, `edge,trace-cmp`), `llvm-profdata` merge, and `llvm-cov report/show` HTML. |

### Build & run locally

```bash
# Build as a standalone fuzzer binary (from the repo root)
clang++ -std=c++23 -I<src> all_targets_fuzz.cpp -o all_targets .libfuzz/libclang_rt.fuzzer-x86_64.a
```

### The unified harness

`src/cpp/fuzz_targets/all_targets_fuzz.cpp` keeps all algorithm engines in one file:

- Shared helpers live in canonical `fuzz_util.hpp`.
- Each algorithm's engine (`hasDuplicate`, `isAnagram`, `twoSum`, `groupAnagrams`, `checkAnagram`) is a `class Solution`; the unified harness dispatches on a selector byte so every algorithm is reachable from one fuzz target.
- No `main()` is provided — it links the no-main libFuzzer runtime (`.libfuzz/libclang_rt.fuzzer-no-main-x86_64.a`).
- Seed corpus `seeds/00`…`09`, each with a selector-prefixed `.rc` file.
- `build.sh` produces `$OUT/all_targets`.

---

## 💥 3. Go fuzzing (native `go test -fuzz`)

Go uses the **native Go fuzzer** (`go test -fuzz`), not libFuzzer. It runs in parallel with the C++ ClusterFuzzLite jobs.

### Files

- `src/GO/go.mod` — Go module (note: the module was renamed from `main` to `github.com/KOL2SG/playground/src/GO`; early Go versions used `package main` as a module name, which broke `go test` importing `"main"`).
- `src/GO/go.sum`
- `src/GO/*.go` — production functions: `hasDuplicate`, `isAnagram`, `twoSum`, `groupAnagrams`, `checkAnagram`, plus `fuzz_common.go` (byte↔int/string marshallers) and `main.go`.
- `src/GO/*_fuzz_test.go` — Go 1.18+ fuzz targets (`FuzzHasDuplicate`, `FuzzIsAnagram`, `FuzzGroupAnagrams`, `FuzzTwoSum`, `FuzzCheckAnagram`), each backed by a committed seed corpus under `src/GO/fuzz/<target>/seed.test`.

### Running locally

```bash
cd src/GO
go test -run=^$ .                 # compile the fuzz tests
go test -fuzz=FuzzHasDuplicate -fuzztime=30s .  # run one target
go test -fuzz=. -fuzztime=60s .   # run all target names (ones listed)
```

### CI workflow

`.github/workflows/go-fuzz.yml` runs on pull requests touching `src/GO/`, loops each fuzz target with `fuzztime=120s` and `timeout=180s` on the self-hosted runner.

---

## 🛡️ 4. Draugr vulnerability scanning

Draugr is the multi-scanner. Its configuration is `draugr.saga.yaml`:

- Controls enabled: **SAST** (Semgrep + gosec), **SCA**, **IaC**, **Secrets**.
- Gate: `failOn: P1` (the scan job fails if any P1 finding appears).
- Output: `draugr-out/report.json` (human-readable) + `draugr-out/results.sarif` (GitHub Code Scanning).

Tools matter. In this repo, `govulncheck` is the Go reachability analyzer, so a Go SDE/SCA gate may report many "unreachable" findings as P3 — the first scan is designed to fail (intentional P1 findings: mutable GitHub-Action tags, a vulnerable Go stdlib, a writable Docker root).

### Draugr report example (trimmed)

```json
{
  "verdict": "fail",
  "controls": [
    { "name": "sast",   "verdict": "fail", "highest": "high", "highestPriority": "P1", "total": 9 },
    { "name": "sca",    "verdict": "pass", "highest": "medium", "highestPriority": "P3", "total": 22 },
    { "name": "iac",    "verdict": "pass", "total": 0 },
    { "name": "secrets","verdict": "pass", "total": 0 }
  ],
  "priorities": { "p1": 3, "p2": 0, "p3": 28, "p4": 0 }
}
```

---

## 🤖 5. AI-powered code review (self-hosted Ollama)

The LLM reviewer reads a PR diff (with optional Draugr JSON), sends it to a local Ollama model, and posts a structured Markdown review as a PR comment. Prompts ask for a verdict, a findings table, a high-risk list, passing checks, and concrete fixes.

### Scripts

| Script | Input | Output |
|------|-------|--------|
| `scripts/ai_review.py` | `pr.diff` + Draugr JSON (optional env `DRAUGR_JSON`) | PR comment (AI review) |
| `scripts/ai_clusterfuzz_review.py` | `cifuzz-sarif/results.sarif` | PR comment (AI review of the fuzz report) |
| `scripts/ai_security_review.py` | `draugr-out/report.json` | PR comment (AI security review) |
| `scripts/pr_review.py` | `pr.diff` + build gate + Draugr `draugr-out` | PR review comment (uses localhost:11434, model `hf.co/unsloth/Qwen3.8-27B-GGUF:UD-Q4_K_M`) |

### Env used by the AI scripts

```bash
OLLAMA_URL=http://127.0.0.1:11434/api/generate
OLLAMA_MODEL=qwen3.8:latest
GITHUB_TOKEN=<repo+issues token>
GITHUB_REPOSITORY=KOL2SG/playground
PR_NUMBER=<pr-number>
```

### Workflows

- `.github/workflows/ai-code-reviews.yml` — AI code review on PR.
- `.github/workflows/ai-security-review.yml` — Draugr scan + AI security review on PR (`continue-on-error: true` keeps the pipeline running).
- `.github/workflows/draugr.yml.bak` — reference Draugr workflow.

---

## 📂 Current GitHub Actions workflows

| Workflow | Trigger | Purpose |
|------|---------|---------|
| `cflite_build.yml` | push → `main` | Continuous C++ build (sanitizers: address/undefined/memory). |
| `cflite_pr.yml` | pull_request | PR C++ fuzzing + SARIF upload. |
| `cflite_cron.yml` | daily cron | Prune + coverage. |
| `cflite_batch.yml` | every 6 hours | Batch fuzzing (address sanitizer). |
| `clang-code-coverage.yml` | push `main`/`test` | Per-file Clang-18 coverage (dynamic, edge coverage). |
| `ai-code-reviews.yml` | PR | Ollama PR diff review → PR comment. |
| `ai-security-review.yml` | PR | Draugr scan + Ollama security review → PR comment. |
| `go-fuzz.yml` (new) | PR → `src/GO/**` | Go native fuzzing (`go test -fuzz`). |
| `security_check.yml.bak`, `draugr.yml.bak` | — | Backup / reference Draugr static-analysis workflows. |

---

## 🚀 Getting started (local demo)

```bash
git clone <repo>
cd playground

# 1) Draugr scan (self-hosted)
./draugr scan draugr.saga.yaml -o draugr-out

# 2) AI review pipeline (needs Ollama + a model)
ollama serve &
ollama pull qwen3-coder:30b
export OLLAMA_URL=http://127.0.0.1:11434/api/generate
export OLLAMA_MODEL=qwen3-coder:30b
export GITHUB_TOKEN=<token>
export GITHUB_REPOSITORY=KOL2SG/playground
export PR_NUMBER=1

python scripts/ai_review.py
# or
python scripts/ai_security_review.py
# or
python scripts/ai_clusterfuzz_review.py
```

---

## ⚙️ Customising the pipeline

| Change | Where |
|------|-------|
| Pin mutable GitHub-Action tags | Replace `uses: actions/checkout@v*` with a full commit SHA. |
| Swap the LLM model | `OLLAMA_MODEL` env var (workflow or local `export`). |
| Change the security gate | `draugr.saga.yaml` → `config.gate.failOn`. |
| Enable/disable scanners | `draugr.saga.yaml` → `config.controls.*.enabled`. |
| Show full diff to LLM | Edit the slice in the relevant `scripts/*.py` (remove `[:12000]`). |
| Include Draugr JSON in the prompt | The `ai_review.py` already reads `DRAUGR_JSON` env var. |
| Dry-run the reviewer | Add `if '--dry-run' in sys.argv: print(review)` and skip `post_comment`. |

---

## 🤝 Contributing

Ideas: add a `--dry-run` flag to the reviewer scripts; replace the hardcoded prompt with a Jinja2 template; auto-update pinned action SHAs; experiment with different Ollama models and report on latency/quality; write SARIF-upload and PR-comment tests. Run the local pipeline (`./draugr …`, `git diff …`, `python scripts/ai_review.py`) before opening a PR.

---

## 🎯 TL;DR — try the whole flow locally

```bash
# 1️⃣ Scan
./draugr scan draugr.saga.yaml -o draugr-out

# 2️⃣ Diff (example: current branch vs main)
git fetch origin main
git diff origin/main...HEAD > pr.diff

# 3️⃣ LLM review → comment on PR #42
export PR_NUMBER=42
python scripts/ai_review.py
```

You'll get a concise bullet-point comment listing the most critical findings with concrete fixes, and the full SARIF is available in GitHub's *Security* tab. That's the complete **scan → review → comment** loop, with the added experiment of AI-driven report automation & optimisation — everything lives in this repo and works both locally and as a GitHub Actions CI pipeline.

---

## 📄 License

MIT (see `LICENSE`).
