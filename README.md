# 📦 playground – Vulnerability Scanning + LLM‑Powered Code Review  

This repository demonstrates a **two‑step security workflow**:

1. **Draugr** scans the whole code‑base (including IaC, secrets, SCA, SAST, etc.) and produces a SARIF report.  
2. **Ollama** (a self‑hosted LLM) reads the Draugr findings, runs a short code‑review on the generated diff, and posts an automated comment back to the PR.

The goal is to have a **continuous‑security pipeline** that not only tells you *what* is vulnerable but also gives you a concise, human‑readable review of the changes and remediation suggestions.

---

## 📋 Table of Contents  

| Section | Description |
|---------|-------------|
| [Overview](#-overview) | What the repo does and why |
| [Prerequisites](#-prerequisites) | Tools you need (Docker, Ollama, Draugr, Python) |
| [Repository Setup](#-repository-setup) | Clone, virtual‑env, and secret handling |
| [Running Draugr locally](#-running-draugr-locally) | Scan the repo and inspect the output |
| [Generating a PR diff](#-generating-a-pr-diff) | How the review script obtains the code change |
| [LLM review with Ollama](#-llm-review-with-ollama) | Call the LLM, produce a comment, and post it |
| [GitHub Actions CI integration](#-github-actions-ci-integration) | Full end‑to‑end automated workflow |
| [How to interpret the report](#-how-to-interpret-the-report) | Priorities, gates, and remediation |
| [Customising the workflow](#-customising-the-workflow) | Pinning action tags, adjusting the scan, changing the LLM |
| [Contributing](#-contributing) | How to help improve the pipeline |
| [License](#-license) | Open‑source terms |

---

## 🌟 Overview  

| Step | Tool | What it does |
|------|------|--------------|
| **1️⃣ Scan** | **Draugr** (`draugr-dev/draugr`) | Runs a suite of open‑source scanners (Trivy, Gitleaks, Semgrep, govulncheck, …) against the repository. Produces a SARIF file (`draugr-out/results.sarif`) and a JSON summary (`draugr-out/report.json`). |
| **2️⃣ Diff** | Git | Generates a `pr.diff` file that contains the changes introduced by the current PR (or any commit range). |
| **3️⃣ Review** | **Ollama** (e.g. `qwen3-coder:30b`) + custom Python script | Sends the diff *and* the Draugr findings to the LLM. The LLM returns a short bullet‑point review that highlights the most critical findings and recommended fixes. |
| **4️⃣ Comment** | GitHub REST API | The script posts the LLM’s review as a comment on the PR (`scripts/ai_review.py`).  |

_The pipeline can run **locally** (helpful for debugging) or as a **GitHub Action** that automatically comments on every PR._

---

## ✅ Prerequisites  

| Item | Installation |
|------|--------------|
| **Docker** (for Ollama) | <code>curl -fsSL https://get.docker.com | sh</code> |
| **Ollama server** | <code>curl -L https://ollama.com/install.sh \| sh</code> then <code>ollama serve &</code> |
| **Ollama model** (example) | <code>ollama pull qwen3-coder:30b</code> |
| **Draugr** (binary) | <code>curl -L https://github.com/draugr-dev/draugr/releases/latest/download/draugr-linux-amd64 -o draugr && chmod +x draugr</code> |
| **Python 3.12+** (for the review script) | <code>python -m venv venv && source venv/bin/activate && pip install -r requirements.txt</code> |
| **Git** | Pre‑installed on most dev machines |
| **GitHub token** with `repo` and `issues` scopes (for the comment) | Create a personal access token or let GitHub Actions provide `${{ secrets.GITHUB_TOKEN }}` |

> **Tip:** All tools run on Linux/macOS. Windows users can use WSL2.

---

## 📂 Repository Setup  

```bash
git clone https://github.com/KOL2SG/playground.git
cd playground

# optional – create a clean virtualenv for the Python script
python -m venv venv
source venv/bin/activate
pip install -r requirements.txt   # (currently just `requests`)

# set required environment variables (example for local testing)
export OLLAMA_URL="http://127.0.0.1:11434/api/generate"
export OLLAMA_MODEL="qwen3-coder:30b"
export GITHUB_TOKEN="ghp_XXXXXXXXXXXXXXXXXXXX"
export GITHUB_REPOSITORY="KOL2SG/playground"
export PR_NUMBER="123"   # the PR you want to comment on
```

Now the repo is ready for the two‑step workflow.

---

## 🛡️ Running Draugr locally  

```bash
# From the repo root:
./draugr scan draugr.saga.yaml -o draugr-out
```

*What you get:*

* `draugr-out/report.json` – human‑readable summary (verdict, priorities, actions).  
* `draugr-out/results.sarif` – standard SARIF that can be uploaded to GitHub Code Scanning.  

The JSON shows the **gate** (`failOn: P1`) and the list of **actions** (remediations) that Draugr suggests – e.g. fixing mutable GitHub‑Action tags, upgrading Go stdlib, etc.

---

## 📁 Generating a PR diff  

The review script expects a file called `pr.diff` in the repository root.

```bash
# Example: get the diff of the current branch vs main
git fetch origin main
git diff origin/main...HEAD > pr.diff
```

The diff can be any range you wish to review (branch vs base, commit vs commit, etc.).

---

## 🤖 LLM review with Ollama  

`scripts/ai_review.py` (or `cpp/scripts/ai_review.py` for the experimental version) does the heavy lifting.

```bash
python scripts/ai_review.py
```

What happens:

1. **Loads** `pr.diff`.  
2. **Calls** Ollama’s `/api/generate` endpoint with a prompt that contains the diff (full diff – we removed the 12 k character slice).  
3. **Receives** a concise bullet‑point review (≤ 300 words).  
4. **Posts** the review as a comment on the PR via the GitHub REST API.

If you want to see the raw LLM response before posting, comment out the `post_comment(review)` line and just `print(review)`.

---

## 📦 GitHub Actions CI integration  

The repository already ships two workflows:

| Workflow | Purpose |
|----------|---------|
| `.github/workflows/ai-code-reviews.yml` | Runs on PRs, generates a diff, calls Ollama, comments back. |
| `.github/workflows/security.yml` | Runs `draugr` on every push/PR, uploads SARIF to *GitHub Code Scanning*, fails the check on any **P1** finding. |

### Minimal workflow outline

```yaml
name: Security & LLM Review
on:
  pull_request:
    types: [opened, synchronize, reopened]

jobs:
  draugr:
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
    needs: draugr
    runs-on: self-hosted   # needs a runner with Ollama reachable
    env:
      OLLAMA_URL: http://127.0.0.1:11434/api/generate
      OLLAMA_MODEL: qwen3-coder:30b
    steps:
      - uses: actions/checkout@v6
      - name: Generate diff
        run: |
          git fetch origin ${{ github.base_ref }}
          git diff origin/${{ github.base_ref }}...HEAD > pr.diff
      - name: Run review script
        run: |
          python scripts/ai_review.py
        env:
          GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
          GITHUB_REPOSITORY: ${{ github.repository }}
          PR_NUMBER: ${{ github.event.pull_request.number }}
```

**Important** – the workflow pins all action references (e.g., `actions/checkout@v6`) to a **full commit SHA** to avoid the mutable‑tag issue that Draugr flags.

---

## 🔍 How to interpret the Draugr report  

| Field | Meaning |
|-------|----------|
| `verdict` | Overall outcome (`pass` or `fail`). A `fail` means at least one **P1** finding exists. |
| `gate.failOnPriority` | The priority that triggers a failure (default **P1**). |
| `controls[].verdict` | Result per control (SAST, SCA, IaC, Secrets, …). |
| `actions[]` | Remediation actions Draugr can automatically suggest (e.g., upgrade a Go module, pin an action to a commit SHA). |
| `priorities` | Count of findings per priority (`p1`, `p2`, …). |

**Typical workflow after a scan:**

1. **Address all P1 findings** – these are the most severe (e.g., mutable GitHub‑Action tags, high‑severity CVEs).  
2. **Apply auto‑fixes** when available (`draugr apply --action-id …`).  
3. **Rerun the scan** to confirm the gate passes.  
4. Once the gate passes, the LLM review runs and provides a friendly summary of the changes.

---

## 🛠️ Customising the pipeline  

### Pinning mutable action tags (recommended)  

Search for `uses: actions/checkout@vX` in the workflow files and replace them with a **40‑character SHA**:

```yaml
# Before (mutable)
uses: actions/checkout@v6

# After (pinned)
uses: actions/checkout@8ade135a41bc03ea155e62e844d188df1ea18608   # <-- example SHA
```

You can obtain the SHA from the GitHub UI → “View commit” → copy the full hash.

### Changing the LLM model  

Set `OLLAMA_MODEL` in the environment or in the workflow:

```yaml
env:
  OLLAMA_MODEL: llama3.2:latest
```

Make sure the model is pulled (`ollama pull <model>`).

### Adjusting the Draugr gate  

Edit `draugr.saga.yaml` → `config.gate.failOn` to a different priority (`P2`, `P3`) if you want a more permissive gate.

### Using a larger diff window  

If your PR diff frequently exceeds the LLM’s context window, you can chunk it or let Draugr limit the report to the files that changed. For a quick test, you could keep the slice:

```python
payload = json.dumps({
    "prompt": REVIEW_PROMPT.format(diff=diff_text[:50000]),   # 50 k chars
})
```

---

## 🤝 Contributing  

1. **Fork** the repo and create a new branch.  
2. Make changes (e.g., tighten the prompt, add more markdown formatting, improve the Python script).  
3. Run the full CI locally: `docker compose up -d` (if you use the provided `docker-compose.yml` for Ollama), then `./draugr scan draugr.saga.yaml -o draugr-out && python scripts/ai_review.py`.  
4. Submit a PR – the CI will automatically run Draugr and the LLM review, so reviewers can see the effect immediately.  

**Good first issues:**  

* Add a fallback for when Ollama is unreachable (post a generic message).  
* Replace the hard‑coded prompt with a Jinja2 template and allow custom prompts via an env var.  
* Implement a `--dry-run` flag for the review script that only prints the LLM output.  

---

## 📄 License  

This repository is licensed under the **MIT License** – see the `LICENSE` file for details.

---

### TL;DR – One‑liner to try locally  

```bash
# 1️⃣ Scan
./draugr scan draugr.saga.yaml -o draugr-out

# 2️⃣ Generate diff (example: current branch vs main)
git fetch origin main
git diff origin/main...HEAD > pr.diff

# 3️⃣ Run LLM review and comment on PR #42
export PR_NUMBER=42
python scripts/ai_review.py
```

You’ll see the LLM’s bullet‑point review appear as a comment on the PR, and the SARIF file can be uploaded to GitHub Code Scanning for a detailed vulnerability view. Enjoy a fully automated security‑first development loop!
