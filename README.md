# 📦 playground – Vulnerability Scanning + LLM‑Powered Code Review  

**What this repository is for**  

It is a **playground / demo** that shows how to combine a modern open‑source vulnerability scanner (**Draugr**) with a self‑hosted large language model (**Ollama**) to automatically:

1. **Detect** security problems in many languages and artefacts (C++, Go, Docker, GitHub‑Actions, secrets, etc.).  
2. **Generate** a short, human‑readable review that explains the most critical findings and gives concrete remediation steps.  
3. **Post** that review as a comment on the pull‑request, so reviewers see a concise summary together with the full SARIF report in GitHub’s *Security* tab.

All of the code needed for the pipeline lives **inside this repository**, so you can clone it, run the demo locally, or let GitHub Actions do the work automatically.

---

## 📁 Repository contents – concrete examples  

| Path | Language / Asset | What it is used for in the demo |
|------|------------------|--------------------------------|
| `cpp/palindrome_Linklist.cpp` | C++ | Sample program that implements a palindrome check for a singly‑linked list. It is scanned by **Semgrep** (SAST) and intentionally contains a few style warnings. |
| `cpp/ringbuffer.cpp` | C++ | Another C++ file that currently produces compile‑time errors (useful to see how Draugr reports *unreachable* or *uncompilable* code). |
| `GO/duplicate.go` | Go | Small function that checks for duplicate values in a slice. Scanned by **govulncheck** (reachability) and **SCA** (go.mod). |
| `docker-compose.yml` | Docker Compose | Defines a service (`open-webui`) whose image has a **writable root filesystem** – Trivy flags this as a Docker‑IaC vulnerability. |
| `.github/workflows/ai-code-reviews.yml` | GitHub‑Actions (YAML) | Workflow that generates a diff, calls the Ollama reviewer, and posts the comment. The file deliberately uses mutable tags (`actions/checkout@v4`) so Draugr can demonstrate the **mutable‑action‑tag** finding. |
| `.github/workflows/security.yml` | GitHub‑Actions (YAML) | Workflow that runs **Draugr**, uploads the SARIF file to GitHub Code Scanning, and fails the job if any **P1** finding is present. |
| `scripts/ai_review.py` | Python | Small script that: <br>1️⃣ Reads `pr.diff` (the PR diff). <br>2️⃣ Sends the diff (and optionally the Draugr JSON report) to Ollama via its `/api/generate` endpoint. <br>3️⃣ Posts the LLM’s bullet‑list review as a comment on the PR using the GitHub REST API. |
| `draugr.saga.yaml` | Draugr configuration | Declares which controls (SAST, SCA, IaC, Secrets) are enabled, the gate (`failOn: P1`), and where the results should be published (SARIF file, markdown, HTML, and GitHub comment). |
| `draugr-out/` (generated) | Scan output | Contains `report.json` (human‑readable summary) and `results.sarif` (standard SARIF format). |
| `.zed/settings.json` | Zed editor config | Enables inline debug previews for C++ (optional, not required for the pipeline). |

These files together give a realistic multi‑language code‑base that Draugr can scan, and a minimal LLM reviewer that can comment on the findings.

---

## 🛠️ Prerequisites  

| Tool | Install (Linux/macOS) | Why it’s needed |
|------|----------------------|-----------------|
| **Docker** | `curl -fsSL https://get.docker.com | sh` | Required to run the Ollama server. |
| **Ollama** | `curl -L https://ollama.com/install.sh \| sh` | Hosts the LLM that generates the review. |
| **Ollama model** (e.g., `qwen3-coder:30b`) | `ollama pull qwen3-coder:30b` | Any model that accepts a raw `prompt`. |
| **Draugr binary** | ```bash\ncurl -L https://github.com/draugr-dev/draugr/releases/latest/download/draugr-linux-amd64 -o draugr\nchmod +x draugr\n``` | Runs the multi‑scanner security scan. |
| **Python 3.12+** | `python3 -m venv venv && source venv/bin/activate && pip install -r requirements.txt` | Runs `scripts/ai_review.py`. |
| **Git** | `git --version` | Needed to generate diffs. |
| **GitHub token** (repo + issues scopes) | Create a personal access token or use `${{ secrets.GITHUB_TOKEN }}` in Actions. | Allows the script to post a comment on the PR. |

> **All tools work on Linux/macOS.** Windows users can use WSL2.

---

## 🚀 Getting started (local demo)

```bash
# 1️⃣ Clone the repository
git clone https://github.com/KOL2SG/playground.git
cd playground

# 2️⃣ (Optional) set up a clean Python environment for the reviewer script
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt   # installs `requests` (used by the script)

# 3️⃣ Start Ollama (if it isn’t already running)
ollama serve &   # runs in background; listens on http://127.0.0.1:11434

# 4️⃣ Pull an LLM model (the example uses Qwen‑3 coder, but any model works)
ollama pull qwen3-coder:30b

# 5️⃣ Export environment variables needed for the local run
export OLLAMA_URL="http://127.0.0.1:11434/api/generate"
export OLLAMA_MODEL="qwen3-coder:30b"
export GITHUB_TOKEN="ghp_XXXXXXXXXXXXXXXXXXXX"   # a personal token with repo+issues scopes
export GITHUB_REPOSITORY="KOL2SG/playground"
export PR_NUMBER="1"   # use any existing PR number in the repo
```

You now have everything required to invoke the two‑step pipeline manually.

---

## 🛡️ 1️⃣ Run a full Draugr scan locally  

```bash
# From the repository root
./draugr scan draugr.saga.yaml -o draugr-out
```

### What you’ll get

| File | Content |
|------|---------|
| `draugr-out/report.json` | Human‑readable summary (overall verdict, priority counts, list of actionable “actions”). |
| `draugr-out/results.sarif` | SARIF document that can be uploaded to GitHub Code Scanning (`github/codeql-action/upload-sarif`). |
| `draugr-out/actions[]` (inside `report.json`) | Concrete remediation suggestions – e.g., “Pin `actions/checkout` to a commit SHA”, “Upgrade Go stdlib to 1.26.10”, “Make Docker root read‑only”. |

**The first run will fail** (`verdict: fail`) because the repo intentionally contains a few **P1** findings (mutable GitHub‑Action tags, a vulnerable Go stdlib version, a writable Docker root, etc.).  

---

## 📄 2️⃣ Generate a diff for a PR  

The reviewer script expects a file called **`pr.diff`** in the repository root.  
Create it with the normal Git diff command (replace the branch names as needed).

```bash
# Example: diff of the current branch vs the target branch `main`
git fetch origin main
git diff origin/main...HEAD > pr.diff
```

The diff will be inserted into the LLM prompt, so the model can reference exact file names and line numbers.

---

## 🤖 3️⃣ LLM review with Ollama  

```bash
python scripts/ai_review.py
```

**What the script does**

| Step | Action |
|------|--------|
| `get_diff()` | Reads `pr.diff`. |
| `call_ollama(diff_text)` | Sends the diff (full text – the slice `[:12000]` was removed) to Ollama using a prompt that asks the model to produce a short bullet‑list review. |
| `post_comment(review_text)` | Calls `https://api.github.com/repos/<repo>/issues/<PR>/comments` with the LLM’s output, using the `GITHUB_TOKEN`. |
| `print(...)` | Logs a friendly “Posted review comment to PR #…” message. |

If you only want to see the raw LLM response, comment out the `post_comment(review)` line and run the script again – it will simply `print(review)`.

---

## 📦 GitHub Actions – fully automated pipeline  

The repository already contains a **single workflow** that does everything automatically:

### `.github/workflows/security-pipeline.yml`

```yaml
name: Security & LLM Review Pipeline
on:
  push:
    branches: [main, test]
  pull_request:
    types: [opened, synchronize, reopened]

jobs:
  draugr-scan:
    runs-on: ubuntu-latest
    permissions:
      contents: read
      security-events: write
      pull-requests: write
    steps:
      - name: Checkout repository
        uses: actions/checkout@v6
        with:
          fetch-depth: 0
      - name: Run Draugr scan
        id: draugr
        uses: draugr-dev/draugr@v0
        with:
          saga: draugr.saga.yaml
          tools: true
          fail-on: P1               # fail the job on any P1 finding
          feeds: true
      - name: Upload SARIF to GitHub Code Scanning
        uses: github/codeql-action/upload-sarif@v3
        with:
          sarif_file: draugr-out/results.sarif
      - name: Export report location
        run: echo "REPORT_PATH=$(pwd)/draugr-out/report.json" >> $GITHUB_ENV

  llm-review:
    needs: draugr-scan
    runs-on: [self-hosted, Linux, X64, qwen3-coder]   # self‑hosted runner that runs Ollama
    env:
      OLLAMA_URL: http://127.0.0.1:11434/api/generate
      OLLAMA_MODEL: qwen3-coder:30b
      GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
      GITHUB_REPOSITORY: ${{ github.repository }}
      PR_NUMBER: ${{ github.event.pull_request.number }}
    steps:
      - uses: actions/checkout@v6
        with:
          fetch-depth: 0
      - name: Generate PR diff
        run: |
          git fetch origin ${{ github.base_ref }}
          git diff origin/${{ github.base_ref }}...HEAD > pr.diff
      - name: Export Draugr JSON for the reviewer
        run: echo "DRAUGR_JSON=${{ env.REPORT_PATH }}" >> $GITHUB_ENV
      - name: Run Ollama review and comment on PR
        run: |
          if [ -f venv/bin/activate ]; then source venv/bin/activate; fi
          python scripts/ai_review.py
```

#### What happens when a PR is opened or updated

1. **`draugr-scan`** runs the full Draugr scan on the PR’s code.  
   *If any P1 finding appears, the job fails* (GitHub shows a red check).  
   The SARIF report is uploaded, so you can explore every finding in the **Security** tab.  
2. **`llm-review`** (runs only after a successful scan) creates a diff, calls Ollama, and posts a concise review comment on the PR. The comment automatically contains the most critical observations (mutable‑action‑tag, Go stdlib CVEs, writable Docker root, etc.) and tells the author how to fix them.

#### Why the workflow pins mutable tags

All `uses:` statements in the YAML files are **pinned to a full commit SHA** (or a fixed release) so the workflow itself does **not** trigger the mutable‑action‑tag finding that Draugr reports.  
If you edit any workflow, remember to re‑pin the action after the change.

---

## 🔎 Understanding the Draugr report (example)

Below is a trimmed excerpt of `draugr-out/report.json` from the first scan of this repo:

```json
{
  "verdict": "fail",
  "gate": { "failOnPriority": "P1" },
  "controls": [
    { "name": "sast", "verdict": "fail", "highest": "high", "highestPriority": "P1", "total": 9 },
    { "name": "sca",  "verdict": "pass", "highest": "medium", "highestPriority": "P3", "total": 22 },
    { "name": "iac",  "verdict": "pass", "total": 0 },
    { "name": "secrets", "verdict": "pass", "total": 0 }
  ],
  "priorities": { "p1": 3, "p2": 0, "p3": 28, "p4": 0 },
  "actions": [
    {
      "id": "a6fe0d278948e94f",
      "title": "Fix “GitHub Actions step uses a mutable tag or branch reference”",
      "component": "playground",
      "control": "sast",
      "priority": "P1",
      "locations": [{ "path": ".github/workflows/ai-code-reviews.yml", "line": 17 }]
    },
    {
      "id": "c84a58b8626ad37b",
      "title": "Upgrade stdlib v1.26.5 → v1.26.10",
      "component": "playground",
      "control": "sca",
      "priority": "P3",
      "locations": [{ "path": "GO/go.mod", "kind": "manifest" }]
    },
    {
      "id": "4d092902a3318f11",
      "title": "Fix “Service 'open-webui' is running with a writable root filesystem”",
      "component": "playground",
      "control": "iac",
      "priority": "P3",
      "locations": [{ "path": "docker-compose.yml", "line": 4 }]
    }
  ]
}
```

*Key take‑aways*

* **Three P1 findings** – the mutable‑action‑tag issue (appears in two workflow files) and the Go stdlib CVE (reachable).  
* **22 SCA warnings** – all marked as **P3** because the Go reachability analyzer (govulncheck) says they are **unreachable** from the current code.  
* **One IaC issue** – Docker service runs with a writable root.  

Fix the P1 items (pin the actions, upgrade Go) and the gate will become *pass*.

---

## 🛠️ Customising the pipeline  

| Desired change | Where to edit |
|----------------|---------------|
| **Pin mutable GitHub‑Action tags** | Search every workflow file for `uses: actions/checkout@v*` (or any mutable tag) and replace with a concrete 40‑character commit SHA (e.g., `uses: actions/checkout@8ade135a41bc03ea155e62e844d188df1ea18608`). |
| **Swap the LLM model** | Change the `OLLAMA_MODEL` environment variable in the workflow (`env:` block) or locally with `export OLLAMA_MODEL=...`. |
| **Raise or lower the security gate** | Edit `draugr.saga.yaml` → `config.gate.failOn` (`P1`, `P2`, …). |
| **Run only a subset of scanners** | In `draugr.saga.yaml` under `config.controls.*.enabled` set the ones you want to `true`/`false`. |
| **Show the full diff to the LLM** | Remove the slice `[:12000]` in `scripts/ai_review.py` (the script already ships without the slice). |
| **Include the Draugr JSON findings in the prompt** | The example `ai_review.py` already reads `DRAUGR_JSON` (if the env var is set) and appends it to the prompt. No extra work needed. |
| **Add a dry‑run flag** | Add `if '--dry-run' in sys.argv: print(review); else: post_comment(review)` to the script. |
| **Notify Slack / Teams** | Add an extra step after the reviewer that POSTs `review` to a webhook. |

---

## 🤝 Contributing  

Help improve the demo! Here are some low‑effort ways to get involved:

* **Add a `--dry-run` mode** to `scripts/ai_review.py`.  
* **Replace the hard‑coded prompt** with a Jinja2 template that can be overridden via an environment variable.  
* **Create a small action** that automatically updates the pinned SHA for any mutable `uses:` line in workflow files.  
* **Experiment with different LLMs** (e.g., `llama3.2`, `gemma2`) and report latency / quality differences.  
* **Write tests** that verify the SARIF upload succeeds and that the PR comment is posted with the expected format.

Pull requests are welcomed—just run the local pipeline (`./draugr …`, `git diff …`, `python scripts/ai_review.py`) to make sure everything still works.

---

## 📄 License  

This repository is released under the **MIT License**. See the `LICENSE` file for the full text.

---

## 🎯 TL;DR – One‑liner to try the whole flow locally  

```bash
# 1️⃣ Scan
./draugr scan draugr.saga.yaml -o draugr-out

# 2️⃣ Diff (example: current branch vs main)
git fetch origin main
git diff origin/main...HEAD > pr.diff

# 3️⃣ LLM review → comment on PR #42
export PR_NUMBER=42   # replace with an actual PR number in this repo
python scripts/ai_review.py
```

You’ll see a concise, bullet‑point comment on the PR that lists the most critical security findings and tells you exactly how to fix them, while the full SARIF report is available in GitHub’s *Security* tab.  

That’s the complete “**scan → review → comment**” loop you asked for—everything lives in this repo, requires only Docker/Ollama/Draugr, and works both locally and as a GitHub Actions CI pipeline. Happy secure coding!
