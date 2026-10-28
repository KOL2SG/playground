# 📦 playground – Automated Vulnerability Scanning + LLM‑Powered Code Review  

This repository is a **playground** that shows how you can combine **Draugr** (a multi‑tool security scanner) with a **self‑hosted Ollama LLM** to automatically:

1. **Find** every security problem in the code base (C++, Go, Docker, GitHub Actions, etc.).  
2. **Generate** a concise, human‑readable review of the findings and the PR changes.  
3. **Post** that review as a comment on the PR, so reviewers get a short summary **and** remediation guidance in one place.

> The workflow is deliberately “no‑frills” so you can copy it into any project and start scanning **right now**.

---

## 📑 Table of Contents  

| Section | What you’ll learn |
|---------|-------------------|
| [What’s inside this repo?](#-what-s-inside-this-repo) | The sample code that Draugr will scan. |
| [Prerequisites](#-prerequisites) | Tools you need locally (Docker, Ollama, Draugr, Python). |
| [Getting started](#-getting-started) | Clone, set up a virtual‑env, fetch a model. |
| [Run a full scan locally](#-run-a-full-scan-locally) | `draugr` → SARIF + JSON report. |
| [Create a diff for a PR](#-create-a-diff-for-a-pr) | Produce the `pr.diff` file that the reviewer consumes. |
| [LLM review with Ollama](#-llm-review-with-ollama) | Call the LLM, get a bullet‑list review, post it to GitHub. |
| [GitHub Actions CI integration](#-github-actions-ci-integration) | End‑to‑end automated pipeline (scan → SARIF upload → LLM comment). |
| [Understanding the Draugr report](#-understanding-the-draugr-report) | Priorities, gates, and actionable fixes. |
| [Customising the workflow](#-customising-the-workflow) | Pin mutable tags, change the model, adjust the gate, etc. |
| [Contributing](#-contributing) | How to help improve the pipeline. |
| [License](#-license) | Open‑source terms. |

---

## 🧩 What’s inside this repo?  

| Directory / file | Language / purpose | Why it matters for the demo |
|------------------|-------------------|----------------------------|
| `cpp/` | C++ (`palindrome_Linklist.cpp`, `ringbuffer.cpp`, many other snippets) | Gives Draugr something to run **SAST** on (Semgrep). |
| `GO/duplicate.go` | Small Go function used by the **Go SCA** scanner (`govulncheck`). | Shows how a Go dependency can be flagged (e.g., std‑lib CVEs). |
| `.github/workflows/` | GitHub‑Actions YAML files (some intentionally use mutable tags). | Draugr flags the **mutable‑action‑tag** issue – a classic supply‑chain risk. |
| `docker-compose.yml` | Docker service (`open-webui`) with a writable root filesystem. | Trivy’s IaC scanner finds the **writable filesystem** problem. |
| `scripts/ai_review.py` | Python script that sends a diff to Ollama and posts a comment. | The **LLM review** step – you can read or modify it. |
| `draugr.saga.yaml` | Full Draugr configuration (controls, gates, publishers). | Drives what scanners run and how the gate is evaluated (`failOn: P1`). |
| `draugr-out/` (generated) | SARIF/JSON results from the last run. | Shows the actual findings for this repo. |
| `.zed/settings.json` | Enables inline debugging in Zed (optional). | Not required for the pipeline, but handy for local debugging. |

---

## 🔧 Prerequisites  

| Tool | Install command (Linux/macOS) | Notes |
|------|-------------------------------|-------|
| **Docker** (required for Ollama) | `curl -fsSL https://get.docker.com | sh` | Verify with `docker version`. |
| **Ollama** (LLM server) | `curl -L https://ollama.com/install.sh \| sh` | Starts a daemon via `ollama serve &`. |
| **Ollama model** (example: `qwen3‑coder:30b`) | `ollama pull qwen3-coder:30b` | Any model that accepts a `prompt` works. |
| **Draugr binary** | ```bash\ncurl -L https://github.com/draugr-dev/draugr/releases/latest/download/draugr-linux-amd64 -o draugr\nchmod +x draugr\n``` | Place the binary in the repo root or add it to `$PATH`. |
| **Python 3.12+** (for the review script) | `python3 -m venv venv && source venv/bin/activate && pip install -r requirements.txt` | `requirements.txt` currently only contains `requests` (optional). |
| **Git** | `git --version` (usually pre‑installed) | Needed for diff generation. |
| **GitHub token** with `repo` and `issues` scope | Create a personal access token or use `${{ secrets.GITHUB_TOKEN }}` in Actions. | The token is used by `ai_review.py` to post a comment. |

> **All of the above run on Linux/macOS.** Windows users can use WSL2.

---

## 🚀 Getting started  

```bash
# 1️⃣ Clone the repo
git clone https://github.com/KOL2SG/playground.git
cd playground

# 2️⃣ (Optional) create a clean Python virtualenv for the review script
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt   # installs `requests` (if needed)

# 3️⃣ Start Ollama (if not already running)
ollama serve &   # runs in the background; you’ll see “Ollama listening on :11434”

# 4️⃣ Pull a model (choose any that supports text generation)
ollama pull qwen3-coder:30b   # ~ 5 GB – adjust if you prefer a smaller model

# 5️⃣ Export required environment variables (example for local testing)
export OLLAMA_URL="http://127.0.0.1:11434/api/generate"
export OLLAMA_MODEL="qwen3-coder:30b"
export GITHUB_TOKEN="ghp_XXXXXXXXXXXXXXXXXXXX"
export GITHUB_REPOSITORY="KOL2SG/playground"
export PR_NUMBER="1"   # any PR you want the script to comment on
```

You’re now ready to run the two‑step security pipeline.

---

## 🛡️ Run a full scan locally  

```bash
# From the repository root:
./draugr scan draugr.saga.yaml -o draugr-out
```

What you’ll see:

| File | Meaning |
|------|---------|
| `draugr-out/report.json` | Human‑readable JSON summary – verdict, priorities, actionable fixes. |
| `draugr-out/results.sarif` | SARIF file that can be uploaded to GitHub Code Scanning (`github/codeql-action/upload-sarif`). |
| `draugr-out/report.json → actions[]` | List of **remediation actions** that Draugr can suggest (e.g., “Pin GitHub‑Action tag to a commit SHA”). |

> **Tip:** The first run deliberately fails the gate (`failOn: P1`). That’s expected – the repo contains a few high‑severity demo findings.

---

## 📄 Create a diff for a PR  

The review script expects a file called `pr.diff` in the repository root.

```bash
# Example: generate a diff between the current branch and the main branch
git fetch origin main
git diff origin/main...HEAD > pr.diff
```

You can also generate diffs for a specific commit range:

```bash
git diff <base‑sha>..<head‑sha> > pr.diff
```

The diff will be embedded in the LLM prompt, so the model can refer to the exact lines that introduced the findings.

---

## 🤖 LLM review with Ollama  

```bash
# Run the review script (it reads pr.diff, calls Ollama, posts a comment)
python scripts/ai_review.py
```

**What the script does**

| Step | Action |
|------|--------|
| `get_diff()` | Reads `pr.diff`. |
| `call_ollama(diff)` | Sends the diff (full text – we removed the 12 k‑char slice) to Ollama using the prompt defined in `scripts/ai_review.py`. |
| `post_comment(review)` | Calls `https://api.github.com/repos/<repo>/issues/<pr>/comments` with the LLM’s response. |
| `print(...)` | Logs a friendly message (`Posted review comment to PR #…`). |

If you want to **see the raw LLM output** before it is posted, comment out the `post_comment(review)` line and simply `print(review)`.

### Example output (truncated)

```
**Automated review (self‑hosted Ollama)**

- The workflow `.github/workflows/ai-code-reviews.yml` uses `actions/checkout@v4`.  
  **Fix:** Pin to a full SHA (`actions/checkout@8ade135a41bc03ea155e62e844d188df1ea18608`).  
- Docker service `open-webui` runs with a writable root filesystem.  
  **Fix:** Add `read_only: true` and `security_opt: ["no-new-privileges:true"]`.  
- Go module `stdlib` in `GO/go.mod` is vulnerable (CVE‑2026‑111943 …).  
  **Fix:** Upgrade Go to 1.26.10 (`go.mod` → `go 1.26.10`).  
- Several Semgrep‑detected mutable‑action‑tag findings (P1).  
  **Fix:** Pin all GitHub‑Action `uses:` references to commit SHAs.  

_Overall: addressed the P1 findings → the next Draugr scan should pass the gate._
```

---

## 📦 GitHub Actions CI integration  

Two workflows already live in `.github/workflows/`:

| Workflow file | Trigger | What it does |
|--------------|---------|--------------|
| `ai-code-reviews.yml` | `pull_request` (opened / synchronize / reopened) | • Generates `pr.diff` <br> • Calls Ollama via `scripts/ai_review.py` <br> • Posts the review comment on the PR |
| `security.yml` | `push` to `main`/`test` **and** `pull_request` | • Runs `draugr` using `draugr.saga.yaml` <br> • Uploads `results.sarif` to **GitHub Code Scanning** <br> • Fails the job if any **P1** finding exists (`gate.failOnPriority = P1`) |

### Minimal CI pipeline (full example)

```yaml
name: Security & LLM Review
on:
  pull_request:
    types: [opened, synchronize, reopened]

jobs:
  draugr-scan:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v6
        with:
          fetch-depth: 0      # required for diff‑aware scans
      - name: Run Draugr
        uses: draugr-dev/draugr@v0
        with:
          saga: draugr.saga.yaml
          tools: true        # installs Trivy, Gitleaks, Semgrep, etc.
          fail-on: P1        # gate fails on any P1 finding
          feeds: true        # update vulnerability feeds before scanning
      - name: Upload SARIF to Code Scanning
        uses: github/codeql-action/upload-sarif@v3
        with:
          sarif_file: draugr-out/results.sarif

  llm-review:
    needs: draugr-scan
    runs-on: self-hosted      # must have access to a running Ollama instance
    env:
      OLLAMA_URL: http://127.0.0.1:11434/api/generate
      OLLAMA_MODEL: qwen3-coder:30b
    steps:
      - uses: actions/checkout@v6
      - name: Generate PR diff
        run: |
          git fetch origin ${{ github.base_ref }}
          git diff origin/${{ github.base_ref }}...HEAD > pr.diff
      - name: Run LLM review
        run: |
          python scripts/ai_review.py
        env:
          GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
          GITHUB_REPOSITORY: ${{ github.repository }}
          PR_NUMBER: ${{ github.event.pull_request.number }}
```

**Important security tip** – all `uses:` statements in the workflow must be **pinned** to a concrete commit SHA (or at least to a fixed release tag) so the workflow itself does not trigger the mutable‑action‑tag finding that Draugr reports.

```yaml
# Bad (mutable tag)
uses: actions/checkout@v6

# Good (pinned)
uses: actions/checkout@8ade135a41bc03ea155e62e844d188df1ea18608
```

You can obtain the SHA from the GitHub UI (click “View commit” on the tag page).

---

## 🔎 Understanding the Draugr report  

Open `draugr-out/report.json` (or view the SARIF in the GitHub *Security* tab).  

| Key | Explanation |
|-----|-------------|
| `verdict` | `fail` → at least one **P1** finding exists. |
| `gate.failOnPriority` | The priority level that triggers a failure (`P1` means “critical”). |
| `controls[]` | Results per scanner type (SAST, SCA, IaC, Secrets, etc.). |
| `actions[]` | **Auto‑remediation suggestions** – each entry includes an `id`, `title`, `priority`, and the file / line it affects. |
| `priorities` | Count of findings per priority (`p1`, `p2`, …). |
| `reachability` | For Go, shows how many vulnerabilities are actually reachable from the code. |

**Typical remediation flow**

1. **Fix all `priority: "P1"` actions** (e.g., pin the mutable GitHub‑Action tags, upgrade Go stdlib).  
2. Re‑run Draugr (`./draugr scan …`). The gate should now be `pass`.  
3. The LLM review will automatically pick up the remaining changes and produce a clean comment.  

---

## 🛠️ Customising the workflow  

| What you want to change | How to do it |
|------------------------|--------------|
| **Pin mutable GitHub‑Action tags** | Search for `uses: actions/checkout@vX` (or any other action) and replace with the full commit SHA (e.g., `uses: actions/checkout@8ade135a41bc03ea155e62e844d188df1ea18608`). |
| **Use a different LLM model** | Set `OLLAMA_MODEL` in the environment or in the workflow YAML (`env: OLLAMA_MODEL: llama3.2:latest`). |
| **Raise or lower the gate threshold** | Edit `draugr.saga.yaml` → `config.gate.failOn` (e.g., `P2` for a more permissive gate). |
| **Include only a subset of scanners** | In `draugr.saga.yaml`, under `config.controls.*.enabled`, toggle `true` / `false`. |
| **Increase the diff sent to the LLM** (if your PR is huge) | In `scripts/ai_review.py`, change the slicing: `diff_text[:12000]` → `diff_text[:50000]` or remove the slice entirely. |
| **Change the LLM prompt** | Edit the `REVIEW_PROMPT` constant in `scripts/ai_review.py`. The placeholder must match the keyword used in `format(...)` (`{diff}` or `{data_report}`). |
| **Add more context to the LLM** | Append the Draugr findings to the prompt, e.g.:<br>`prompt = f"{REVIEW_PROMPT.format(diff=diff)}\n\nDraugr findings:\n{json.dumps(draugr_report, indent=2)}"` |

---

## 🤝 Contributing  

We welcome improvements! Here’s how to get involved:

1. **Fork** the repository and create a feature branch.  
2. **Make your change** (e.g., a better prompt, a new scanner, a fallback for offline Ollama).  
3. **Test locally**: run Draugr → generate a diff → execute `python scripts/ai_review.py`.  
4. **Push** to your fork and open a PR. The CI will automatically run the full security pipeline, so reviewers can see the impact instantly.  

### Good first issues  

* Add a `--dry-run` flag to `ai_review.py` that prints the LLM response without posting to GitHub.  
* Replace the hard‑coded `REVIEW_PROMPT` with a Jinja2 template that can be overridden via an environment variable.  
* Implement a fallback that uses the GitHub Copilot API if Ollama is unavailable.  
* Add a GitHub Action that automatically updates the pinned SHA for any mutable actions (run `git rev-parse` on the tag and rewrite the workflow).  

---

## 📄 License  

This repository is licensed under the **MIT License** – see the `LICENSE` file for the full text.

---

## 🎯 TL;DR – One‑liner to try locally  

```bash
# 1️⃣ Scan the repo
./draugr scan draugr.saga.yaml -o draugr-out

# 2️⃣ Generate a diff (example: current branch vs main)
git fetch origin main
git diff origin/main...HEAD > pr.diff

# 3️⃣ Run the LLM review and post it to PR #42
export PR_NUMBER=42   # replace with your actual PR number
python scripts/ai_review.py
```

You’ll get a **single comment** on the PR that summarises all high‑severity findings (mutable GitHub‑Action tags, writable Docker root, Go stdlib CVEs, etc.) and suggests concrete fixes. The SARIF file can be uploaded to GitHub Code Scanning for a full vulnerability view.

Enjoy a fully automated, **security‑first** development loop!
