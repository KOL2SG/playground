Below is a **complete, ready‑to‑use pipeline** that lives inside the repository you already have.  
It ties together the three pieces you already wrote:

1. **Draugr** – run the full security scan and upload a SARIF file (so you get the nice GitHub Code Scanning UI).  
2. **Diff generation** – create `pr.diff` for the PR that triggered the workflow.  
3. **Ollama LLM review** – feed the diff (and optionally the Draugr JSON report) to Ollama and post the review back to the PR.

The pipeline is expressed as a **single GitHub Actions workflow** (`.github/workflows/security‑pipeline.yml`).  
All you need to do is copy the file into the repository and push it.  
After that every PR (and every push to `main`/`test`) will automatically:

* Fail the CI if a **P1** finding exists (gate defined in `draugr.saga.yaml`).  
* Upload the SARIF results to the **Security** tab.  
* Run the Ollama‑powered reviewer and post a comment on the PR with a concise bullet‑list of the high‑severity issues and remediation tips.

---

## 📂 New file – `.github/workflows/security-pipeline.yml`

```yaml
# ──────────────────────────────────────────────────────────────────────
#  security‑pipeline.yml
#  Runs Draugr → uploads SARIF → generates a PR diff → calls Ollama
#  → posts an automated review comment on the PR.
# ──────────────────────────────────────────────────────────────────────
name: Security & LLM Review Pipeline

# Run on pushes to the protected branches *and* on any PR activity.
on:
  push:
    branches: [main, test]
  pull_request:
    types: [opened, synchronize, reopened]

# --------------------------------------------------------------
#  TWO JOBS – scan first, then review (review needs the scan to finish)
# --------------------------------------------------------------
jobs:
  # ------------------------------------------------------------
  #  1️⃣  Draugr scan
  # ------------------------------------------------------------
  draugr-scan:
    # Use a normal GitHub‑hosted runner – no Ollama needed here.
    runs-on: ubuntu-latest
    permissions:
      # Required to upload SARIF (code‑scanning) and comment on PRs.
      contents: read
      security-events: write
      pull-requests: write
    steps:
      # ------------------------------------------------------------------
      #  Checkout the repository (full history – required for diff mode)
      # ------------------------------------------------------------------
      - name: Checkout repository
        uses: actions/checkout@v6
        with:
          fetch-depth: 0   # needed for Draugr “diff” mode

      # ------------------------------------------------------------------
      #  Run Draugr – this will respect the gate defined in draugr.saga.yaml
      # ------------------------------------------------------------------
      - name: Run Draugr scan
        id: draugr
        uses: draugr-dev/draugr@v0            # always pulls latest v0.x release
        with:
          saga: draugr.saga.yaml              # your full descriptor
          tools: true                         # provision Trivy, Semgrep, … 
          fail-on: P1                         # gate – fail CI on any P1 finding
          feeds: true                         # update vulnerability feeds first

      # ------------------------------------------------------------------
      #  Upload the SARIF file so the Findings appear in the Security tab
      # ------------------------------------------------------------------
      - name: Upload SARIF to GitHub Code Scanning
        uses: github/codeql-action/upload-sarif@v3
        with:
          sarif_file: draugr-out/results.sarif

      # ------------------------------------------------------------------
      #  Export the path to the Draugr JSON report for the next job
      # ------------------------------------------------------------------
      - name: Export report location
        run: echo "REPORT_PATH=$(pwd)/draugr-out/report.json" >> $GITHUB_ENV

  # ------------------------------------------------------------
  #  2️⃣  Ollama LLM review (runs only after a successful scan)
  # ------------------------------------------------------------
  llm-review:
    # Needs a runner that can talk to your local Ollama instance.
    # If you already have a self‑hosted runner that runs the Ollama
    # daemon, point the job at that label.
    runs-on: [self-hosted, Linux, X64, qwen3-coder]   # adjust to your runner label
    needs: draugr-scan
    env:
      # ----------------------------------------------------------------
      #  Ollama endpoint – change if you run it on a different host/port
      # ----------------------------------------------------------------
      OLLAMA_URL: http://127.0.0.1:11434/api/generate
      OLLAMA_MODEL: qwen3-coder:30b          # any model that accepts a prompt
      # ----------------------------------------------------------------
      #  GitHub API credentials – automatically provided by GitHub Actions
      # ----------------------------------------------------------------
      GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
      GITHUB_REPOSITORY: ${{ github.repository }}
      PR_NUMBER: ${{ github.event.pull_request.number }}
    steps:
      # ----------------------------------------------------------------
      #  Checkout the same code that was scanned
      # ----------------------------------------------------------------
      - name: Checkout repository
        uses: actions/checkout@v6
        with:
          fetch-depth: 0

      # ----------------------------------------------------------------
      #  Generate a diff for the PR (the LLM reviewer reads this)
      # ----------------------------------------------------------------
      - name: Generate PR diff
        if: ${{ github.event_name == 'pull_request' }}
        run: |
          # Diff the PR against its base branch
          git fetch origin ${{ github.base_ref }}
          git diff origin/${{ github.base_ref }}...HEAD > pr.diff

      # ----------------------------------------------------------------
      #  OPTIONAL – include the Draugr JSON report inside the LLM prompt.
      #  The script will automatically read the env var REPORT_PATH.
      # ----------------------------------------------------------------
      - name: Export Draugr report path for the script
        run: echo "DRAUGR_JSON=${{ env.REPORT_PATH }}" >> $GITHUB_ENV

      # ----------------------------------------------------------------
      #  Run the Python reviewer (scripts/ai_review.py)
      # ----------------------------------------------------------------
      - name: Run Ollama review and comment on PR
        run: |
          # Activate a virtualenv if you keep one in the repo
          if [ -f venv/bin/activate ]; then source venv/bin/activate; fi
          python scripts/ai_review.py
```

### What the workflow does, step‑by‑step

| Job | Step | What happens |
|-----|------|---------------|
| **draugr‑scan** | Checkout | Full repository (depth 0) so Draugr can see the whole tree. |
| | Run Draugr | Executes `draugr scan draugr.saga.yaml -o draugr-out`. The gate fails on any **P1** finding (`fail-on: P1`). |
| | Upload SARIF | Sends `draugr-out/results.sarif` to the **GitHub Security** tab. |
| | Export report path | Stores the absolute path of `draugr-out/report.json` in an environment variable for the next job. |
| **llm‑review** | Checkout | Ensures the second job works on the same commit. |
| | Generate PR diff | Produces `pr.diff` (only on PR events). |
| | Export report path | Makes the JSON report visible to `ai_review.py` via `DRAUGR_JSON`. |
| | Run Ollama review | Calls `scripts/ai_review.py` which: <br>  · reads `pr.diff` <br>  · optionally reads `DRAUGR_JSON` and adds it to the prompt <br>  · sends the prompt to Ollama <br>  · posts the LLM’s bullet‑list review as a comment on the PR. |

---

## 🛠️ Small change to `scripts/ai_review.py` (optional)

If you want the LLM to *also* see the Draugr JSON findings (highly recommended – it lets the model talk about the exact vulnerabilities), replace the `call_ollama` function with the version below. It appends the report JSON to the prompt if the `DRAUGR_JSON` environment variable exists.

```python
import json
import os
import urllib.request

# … (the top of the file stays the same) …

REVIEW_PROMPT = """You are a senior security reviewer.  
Your job is to comment on the diff **and** the security findings that are listed below.

DIFF:
{diff}

---  

SECURITY FINDINGS (JSON):
{findings}

Give a short bullet‑list (≤ 300 words) that:
* Highlights the most critical issues (e.g., any P1 findings).  
* Points to the exact file/line in the diff where the problem appears.  
* Suggests concrete remediation steps (e.g., “pin actions/checkout to a SHA”, “upgrade Go stdlib to 1.26.10”, “set read_only: true in docker‑compose”).  

Keep the tone friendly and actionable."""
 
def load_findings():
    """Read the Draugr JSON report if the env var is set."""
    path = os.getenv("DRAUGR_JSON")
    if path and os.path.isfile(path):
        with open(path, "r", encoding="utf-8") as f:
            return json.dumps(json.load(f), indent=2)
    return "{}"   # empty JSON if no report is supplied

def call_ollama(diff_text):
    payload = json.dumps({
        "model": MODEL,
        "prompt": REVIEW_PROMPT.format(
            diff=diff_text,               # full diff – no slicing
            findings=load_findings()      # embeds the full JSON report
        ),
        "stream": False,
    }).encode("utf-8")
    req = urllib.request.Request(OLLAMA_URL, data=payload,
                                 headers={"Content-Type": "application/json"})
    with urllib.request.urlopen(req, timeout=120) as resp:
        body = json.loads(resp.read())
    return body["response"]
```

> **Why this works:**  
> * The placeholder names (`{diff}` and `{findings}`) now match the `format(...)` call, so no `KeyError` occurs.  
> * By passing the full Draugr JSON, the LLM can reference the exact `id`, `title`, and `location` fields, making the review far more useful.  

If you prefer to keep the original, tiny script (only diff), simply leave it unchanged – the pipeline will still work.

---

## 📦 How to try the pipeline locally (optional)

You can run the same steps on your machine to validate everything before pushing:

```bash
# 1️⃣ Run Draugr (replicates the first job)
./draugr scan draugr.saga.yaml -o draugr-out

# 2️⃣ Generate a diff (replace main with the branch you want to test)
git fetch origin main
git diff origin/main...HEAD > pr.diff

# 3️⃣ Export the path to the report so the script can include it
export DRAUGR_JSON=$(pwd)/draugr-out/report.json

# 4️⃣ Run the LLM reviewer (make sure Ollama is running)
python scripts/ai_review.py
```

You’ll see the LLM’s bullet‑list printed (and, if you set `PR_NUMBER`, it will also be posted to the PR).

---

## 🎯 What you get out of the box

* **Zero‑manual gating** – the job fails automatically on any P1 vulnerability.  
* **SARIF visualisation** – open the *Security* tab on GitHub to browse every finding, filter by priority, and see the exact code locations.  
* **LLM‑generated PR comment** – a short, actionable review that mentions the high‑severity issues and how to fix them, posted directly on the PR for every reviewer to see.  
* **Fully reproducible CI** – the same workflow runs on every push and PR, so you never forget to scan or comment again.

---

## 🚀 Next steps (optional enhancements)

| Idea | How to implement |
|------|-----------------|
| **Auto‑apply fixes** (e.g., pin the action tag automatically) | Use `draugr apply --action-id <id>` in a separate step, then push a new commit. |
| **Chunked diff for huge PRs** | Add a small Python helper that splits `pr.diff` into 5 k‑character chunks and sends them sequentially to Ollama. |
| **Different LLMs** | Change `OLLAMA_MODEL` env var or add a matrix strategy in the workflow to test several models. |
| **Slack / Teams notification** | Add a step after the review to post the LLM summary to a webhook. |
| **Aggregated dashboard** | Export the SARIF data to a custom dashboard (e.g., via `gh api` or a third‑party tool). |

---

### 🎉 You’re all set!

1. **Add the workflow file** (`.github/workflows/security-pipeline.yml`) to the repository.  
2. **Commit & push** – GitHub will automatically run the pipeline on the next PR or push to `main`/`test`.  
3. Open a PR, watch the **Security** tab for findings, and see the **LLM comment** appear once the scan finishes.

That’s the whole “scan → review → comment” loop you asked for – everything lives in the repo, needs no external services (apart from Ollama, which you already host), and gives you immediate, human‑readable feedback on every change. Happy secure coding!
