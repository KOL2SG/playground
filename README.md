Below is a ready‑to‑use **README.md** that succinctly explains what your repository does, the tools it uses, and how to run the full security‑scan + LLM‑review pipeline.

```markdown
# 📦 playground – Automated Vulnerability Scanning + LLM‑Powered Code Review

This repository demonstrates a **two‑step security workflow**:

1. **Draugr** scans the entire code‑base (C++, Go, Docker, GitHub Actions, secrets, etc.) and produces a SARIF/JSON report.
2. **Ollama** (a self‑hosted LLM) reads the PR diff, generates a concise code‑review, and posts the review as a comment on the PR.

The goal is to have an **automatic “scan → review → comment” loop** that surfaces high‑severity findings and gives developers actionable remediation guidance directly inside the pull‑request.

---  

## 📑 Table of Contents  

- [What the repo contains](#-what-the-repo-contains)  
- [Prerequisites](#-prerequisites)  
- [Getting started](#-getting-started)  
- [Run a full Draugr scan locally](#-run-a-full-draugr-scan-locally)  
- [Generate a diff for a PR](#-generate-a-diff-for-a-pr)  
- [LLM review with Ollama](#-llm-review-with-ollama)  
- [GitHub Actions CI integration](#-github-actions-ci-integration)  
- [Understanding the Draugr report](#-understanding-the-draugr-report)  
- [Customising the pipeline](#-customising-the-pipeline)  
- [Contributing](#-contributing)  
- [License](#-license)  

---  

## 🧩 What the repo contains  

| Path / File | Language / Purpose | Why it matters |
|--------------|----------------------|----------------|
| `cpp/` | C++ examples (`palindrome_Linklist.cpp`, `ringbuffer.cpp`, …) | Provides source for **SAST** scanning (Semgrep). |
| `GO/duplicate.go` | Go function | Used by **govulncheck** (Go reachability analysis). |
| `.github/workflows/` | GitHub‑Actions YAML files (some intentionally use mutable tags) | Triggers **mutable‑action‑tag** findings – a classic supply‑chain risk. |
| `docker-compose.yml` | Docker service (`open-webui`) | Demonstrates a **writable root filesystem** issue for Trivy. |
| `scripts/ai_review.py` | Python script that calls Ollama and posts a comment | Implements the **LLM review** step. |
| `draugr.saga.yaml` | Full Draugr configuration (controls, gates, publishers) | Drives which scanners run and how the gate is evaluated (`failOn: P1`). |
| `draugr-out/` (generated) | `report.json` + `results.sarif` | Output of the latest Draugr scan. |
| `.zed/settings.json` | Enables inline debug previews (optional) | Helpful for local debugging in Zed. |

---  

## ✅ Prerequisites  

| Tool | Install (Linux/macOS) | Remarks |
|------|----------------------|---------|
| **Docker** | `curl -fsSL https://get.docker.com | sh` | Needed for Ollama. |
| **Ollama** | `curl -L https://ollama.com/install.sh | sh` | Run `ollama serve &`. |
| **Ollama model** (e.g., `qwen3-coder:30b`) | `ollama pull qwen3-coder:30b` | Any model that accepts a `prompt`. |
| **Draugr binary** | ```bash\ncurl -L https://github.com/draugr-dev/draugr/releases/latest/download/draugr-linux-amd64 -o draugr\nchmod +x draugr\n``` | Place in repo root or add to `$PATH`. |
| **Python 3.12+** | `python3 -m venv venv && source venv/bin/activate && pip install -r requirements.txt` | Installs `requests` (used by the script). |
| **Git** | `git --version` | Needed to generate diffs. |
| **GitHub token** (`repo` + `issues` scopes) | Create a personal access token or use `${{ secrets.GITHUB_TOKEN }}` in Actions. | Required for posting PR comments. |

---  

## 🚀 Getting started  

```bash
# Clone the repo
git clone https://github.com/KOL2SG/playground.git
cd playground

# (Optional) set up a clean virtualenv for the review script
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt

# Start Ollama (if not already running)
ollama serve &

# Pull an LLM model
ollama pull qwen3-coder:30b

# Export required environment variables for local testing
export OLLAMA_URL="http://127.0.0.1:11434/api/generate"
export OLLAMA_MODEL="qwen3-coder:30b"
export GITHUB_TOKEN="ghp_XXXXXXXXXXXXXXXXXXXX"
export GITHUB_REPOSITORY="KOL2SG/playground"
export PR_NUMBER="1"   # replace with a real PR number
```

You are now ready to run the scan + review pipeline.

---  

## 🛡️ Run a full Draugr scan locally  

```bash
# From the repository root
./draugr scan draugr.saga.yaml -o draugr-out
```

*Outputs*  

- `draugr-out/report.json` – human‑readable summary (verdict, priorities, actions).  
- `draugr-out/results.sarif` – SARIF file you can upload to **GitHub Code Scanning**.  

The first run intentionally fails the gate (`failOn: P1`) because the repo includes a few high‑severity demo findings (mutable GitHub‑Action tags, Go stdlib CVEs, writable Docker root, …).

---  

## 📄 Generate a diff for a PR  

The LLM review script expects a file named `pr.diff` at the repository root.

```bash
# Example: diff of the current branch vs main
git fetch origin main
git diff origin/main...HEAD > pr.diff
```

Feel free to use any commit range (`git diff <base>..<head>`) – the diff will be embedded in the prompt sent to the LLM.

---  

## 🤖 LLM review with Ollama  

```bash
python scripts/ai_review.py
```

**What the script does**

1. Reads `pr.diff`.  
2. Calls Ollama’s `/api/generate` endpoint with a prompt that contains the full diff.  
3. Receives a short bullet‑list review (≤ 300 words).  
4. Posts the review as a comment on the PR via the GitHub REST API.  

If you only want to see the raw LLM output, comment out the `post_comment(review)` line and simply `print(review)`.

---  

## 📦 GitHub Actions CI integration  

Two workflows live in `.github/workflows/`:

| Workflow | Trigger | Main steps |
|----------|----------|------------|
| `ai-code-reviews.yml` | `pull_request` (opened / synchronize / reopened) | Generates `pr.diff`, runs `scripts/ai_review.py`, posts the LLM comment. |
| `security.yml` | `push` to `main`/`test` **and** `pull_request` | Runs Draugr (`draugr-dev/draugr@v0`), uploads SARIF (`github/codeql-action/upload-sarif@v3`), fails the job when any **P1** finding is present. |

### Minimal pipeline (full example)

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
          fetch-depth: 0
      - name: Run Draugr
        uses: draugr-dev/draugr@v0
        with:
          saga: draugr.saga.yaml
          tools: true
          fail-on: P1
          feeds: true
      - name: Upload SARIF
        uses: github/codeql-action/upload-sarif@v3
        with:
          sarif_file: draugr-out/results.sarif

  llm-review:
    needs: draugr-scan
    runs-on: self-hosted               # must have access to Ollama
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

> **Security tip:** All `uses:` statements must be **pinned to a full commit SHA** (or a fixed release) to avoid the mutable‑action‑tag finding that Draugr flags.

---  

## 🔎 Understanding the Draugr report  

Open `draugr-out/report.json` (or view the SARIF in the **Security** tab of the repo).

| Field | Meaning |
|-------|----------|
| `verdict` | `fail` indicates at least one **P1** finding exists. |
| `gate.failOnPriority` | Priority that triggers a failure (`P1` by default). |
| `controls[]` | Result per scanner type (SAST, SCA, IaC, Secrets, etc.). |
| `actions[]` | Remediation actions Draugr can suggest (e.g., “Pin mutable action tag to SHA”, “Upgrade Go stdlib”). |
| `priorities` | Count of findings by priority (`p1`, `p2`, `p3`, `p4`). |
| `reachability` | For Go, shows how many vulnerabilities are actually reachable from the code. |

**Typical remediation workflow**

1. Fix all **P1** actions (often a 1‑line change, e.g., pinning an action).  
2. Re‑run Draugr; the gate should now be `pass`.  
3. The LLM review will automatically pick up the remaining changes and provide a clean comment.

---  

## 🛠️ Customising the pipeline  

| What you want to change | How to do it |
|-------------------------|--------------|
| **Pin mutable action tags** | Replace `uses: actions/checkout@vX` with a concrete 40‑character SHA (e.g., `uses: actions/checkout@8ade135a41bc03ea155e62e844d188df1ea18608`). |
| **Swap the LLM model** | Change `OLLAMA_MODEL` in the environment (or in the workflow `env:` block). |
| **Raise/lower the gate** | Edit `draugr.saga.yaml` → `config.gate.failOn` (e.g., `P2` for a more permissive gate). |
| **Run only selected scanners** | In `draugr.saga.yaml`, under `config.controls.<scanner>.enabled`, set `true`/`false`. |
| **Increase diff size sent to Ollama** | Remove the slice (`diff_text[:12000]`) or increase the limit in `scripts/ai_review.py`. |
| **Add the Draugr findings to the LLM prompt** | Append the JSON report to the prompt in `ai_review.py` (helps the model talk directly about the high‑severity items). |
| **Add a `--dry-run` flag** | Modify `ai_review.py` to print the review instead of posting when `--dry-run` is passed. |

---  

## 🤝 Contributing  

We’d love your improvements!  

1. **Fork** the repo and create a branch.  
2. Make changes (e.g., a better prompt, a new scanner, offline fallback).  
3. Test locally: `./draugr scan …` → generate a diff → `python scripts/ai_review.py`.  
4. Push and open a PR – the CI will automatically run the full scan + LLM review, so reviewers can see the effect immediately.

### Good first issues

- Add a `--dry-run` flag to `ai_review.py`.  
- Replace the hard‑coded prompt with a Jinja2 template that can be overridden via an env var.  
- Implement a fallback that uses the GitHub Copilot API when Ollama is unreachable.  
- Write a small action that automatically updates pinned SHA values for mutable actions.

---  

## 📄 License  

This repository is licensed under the **MIT License** – see the `LICENSE` file for details.

---  

## 🎯 TL;DR – One‑liner to try locally  

```bash
# 1️⃣ Scan the repo
./draugr scan draugr.saga.yaml -o draugr-out

# 2️⃣ Generate a diff (example: current branch vs main)
git fetch origin main
git diff origin/main...HEAD > pr.diff

# 3️⃣ Run the LLM review and post it to PR #42
export PR_NUMBER=42   # replace with your PR number
python scripts/ai_review.py
```

You’ll see a concise, bullet‑point comment on the PR that summarises the high‑severity findings and suggests concrete fixes, while the SARIF report is available in GitHub Code Scanning for a complete view. Enjoy a fully automated, **security‑first** development loop!
