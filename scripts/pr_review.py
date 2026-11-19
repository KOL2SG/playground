#!/usr/bin/env python3
"""
Run the local LLM code review on the self-hosted GitHub runner.

Flow:
  1. Read the compile gate status written by the workflow (build/status.txt).
  2. Fetch the PR diff (GitHub API).
  3. Send the diff + build gate to the LOCAL Ollama model
     (OpenAI-compatible endpoint at http://localhost:11434/v1).
  4. Post the review back to the PR as a GitHub PR review (event: COMMENT).

Run inside the GitHub Actions job (or manually with the env vars set).
Python stdlib only -- no external packages required.
"""
import json
import os
import sys
import urllib.request
import urllib.error

OLLAMA_URL = os.environ.get("OLLAMA_URL", "http://127.0.0.1:11434/api/generate")
MODEL_ID = os.environ.get("MODEL_ID", "hf.co/unsloth/Qwen3.8-27B-GGUF:UD-Q4_K_M")
TOKEN = os.environ.get("GITHUB_TOKEN", "")
REPO = os.environ.get("GITHUB_REPOSITORY", "")
EVENT_PATH = os.environ.get("GITHUB_EVENT_PATH", "")
STATUS_FILE = os.path.join(
    os.path.dirname(os.path.abspath(__file__)), "..", "build", "status.txt"
)
EMAIL = os.environ.get("GITHUB_ACTOR", "review-bot")

SYSTEM_PROMPT = """You are a senior engineer reviewing a pull request.
Apply these rules strictly and be concrete:

RULES:
- NO raw SQL; only SQLAlchemy ORM.
- Every FastAPI endpoint must have type annotations and Pydantic models.
- NO `any` types in TypeScript/React. All components must have props interfaces.
- Fetch data with React Query; NEVER use useEffect for API calls.
- No raw owning pointers; use RAII and smart pointers
  (std::unique_ptr, std::shared_ptr, std::weak_ptr).
- No manual new/delete; never double-free or leak.
- Bounds-safe containers/algorithms; no strcpy/sprintf family without bounds checks.
- Error handling: C uses error codes + goto cleanup; C++ uses exceptions consistently.
- Type safety: enum class, nullptr, std::optional, constexpr; no #define constants.
- Compiler warnings as errors: -Wall -Wextra -Wpedantic -Werror.
- Sanitizers (ASan/UBSan/LSan/TSan) in every CI build.
- Avoid undefined behaviour, dangling references, and vector-reallocation bugs.

Return results as Markdown with:
1. A short verdict (solid / ok with minor issues / fixes needed).
2. A findings table: File | Line | Severity (Critical/Warning/Nit) | Fix.
3. A "High-risk" list (memory corruption, UB, crash, data race, leak).
4. Passing checks (compile gate, sanitizers, formatting).

Be concise but give the concrete fix, not just a description."""

USER_PROMPT = """Review the git diff below.

### PR summary
(if this event carries a PR title/description, include it)

### Diff
{file_patches}

### Build gate
{status}

Return your review as Markdown. Be concise but give the concrete fix for each finding."""


# ---------------------------------------------------------------- helpers

def get_pr_number():
    try:
        with open(EVENT_PATH, "r") as fh:
            ev = json.load(fh)
    except Exception:
        return os.environ.get("GITHUB_PR_NUM", "")
    if isinstance(ev, dict):
        return ev.get("number") or ev.get("pull_request", {}).get("number") or ""
    return ""


def gh_url(path):
    return f"https://api.github.com/repos/{REPO}{path}"


def gh_get(path):
    req = urllib.request.Request(
        gh_url(path),
        headers={"Authorization": f"Bearer {TOKEN}",
                 "Accept": "application/vnd.github+json",
                 "X-GitHub-Api-Version": "2022-11-28"},
    )
    try:
        with urllib.request.urlopen(req, timeout=60) as resp:
            return json.loads(resp.read().decode())
    except urllib.error.HTTPError as e:
        body = e.read().decode(errors="replace") if e.fp else ""
        print(f"GitHub API error {e.code} for {path}: {body}", file=sys.stderr)
        raise


def gh_post(path, payload):
    req = urllib.request.Request(
        gh_url(path),
        data=json.dumps(payload).encode(),
        headers={"Authorization": f"Bearer {TOKEN}",
                 "Accept": "application/vnd.github+json",
                 "Content-Type": "application/json",
                 "X-GitHub-Api-Version": "2022-11-28"},
        method="POST",
    )
    try:
        with urllib.request.urlopen(req, timeout=60) as resp:
            return json.loads(resp.read().decode())
    except urllib.error.HTTPError as e:
        body = e.read().decode(errors="replace") if e.fp else ""
        print(f"GitHub API error {e.code} for {path}: {body}", file=sys.stderr)
        raise


def fetch_pr_files(num):
    """Return concatenated file patches for the PR (bounded in size)."""
    pr = gh_get(f"/pulls/{num}")
    files = pr.get("files", [])
    parts = []
    total = 0
    for f in files:
        patch = f.get("patch") or ""
        parts.append(f"### {f.get('filename')} (+{f.get('additions')} / -{f.get('deletions')})")
        parts.append(patch)
        total += len(patch)
        if total > 120000:  # keep the prompt bounded
            break
    return "\n".join(parts)


def call_ollama(messages):
    url = f"{OLLAMA_URL}"
    payload = {
        "model": MODEL_ID,
        "messages": messages,
        "temperature": 0.2,
        "max_tokens": 2000,
    }
    req = urllib.request.Request(
        url,
        data=json.dumps(payload).encode(),
        headers={"Content-Type": "application/json", "Accept": "application/json"},
    )
    with urllib.request.urlopen(req, timeout=300) as resp:
        return json.loads(resp.read().decode())["message"]["content"]


def read_compile_status():
    try:
        with open(STATUS_FILE, "r") as fh:
            return fh.read().strip() or "UNKNOWN"
    except Exception:
        return "UNKNOWN"


# ---------------------------------------------------------------- main

def main():
    pr_num = get_pr_number()
    if not pr_num:
        print("ERROR: could not determine PR number from GITHUB_EVENT_PATH", file=sys.stderr)
        sys.exit(1)

    status = read_compile_status()
    print(f"PR #{pr_num} | compile gate: {status}", flush=True)

    # Re-fetch the full repo context at the PR head for the diff.
    pr = gh_get(f"/pulls/{pr_num}")  # PR object: title + head.sha
    pr_title = (pr.get("title") or "").strip()
    head_sha = (pr.get("head") or {}).get("sha") or ""

    file_list = gh_get(f"/pulls/{pr_num}/files")  # list of file objects
    diff_parts = []
    for f in file_list:
        diff_parts.append(f"### {f.get('filename')} (+{f.get('additions')} / -{f.get('deletions')})")
        if f.get("patch"):
            diff_parts.append("\n" + f.get("patch"))
    diff = "\n".join(diff_parts)

    # Keep the prompt bounded (Ollama token window varies by model).
    if len(diff) > 150000:
        diff = diff[:150000] + "\n[... diff truncated for brevity ...]"

    prompt = USER_PROMPT.format(file_patches=diff, status=status).encode("utf-8", "ignore")

    messages = [{"role": "system", "content": SYSTEM_PROMPT},
                {"role": "user", "content": prompt.decode("utf-8", "ignore")}]

    print("Sending review request to local model...", flush=True)
    review = call_ollama(messages)

    report = (
        f"# C/C++ PR Review: {pr_title}\n\n"
        f"- **PR**: #{pr_num} ({REPO})\n"
        f"- **Model**: {MODEL_ID}\n"
        f"- **Build gate**: {status}\n\n"
        f"---\n\n{review}\n"
    )

    # Post as a PR review (comment event).
    try:
        gh_post(
            f"/pulls/{pr_num}/reviews",
            {"body": report, "event": "COMMENT", "commit_id": head_sha},
        )
        print("Review posted to PR #%s." % pr_num)
    except Exception as e:
        print("ERROR posting review: %s" % e, file=sys.stderr)
        sys.exit(1)

    # Print the review to the job log too.
    print("=" * 20, "REVIEW", "=" * 20)
    print(report)
    print("=" * 40)

    # Hard gate: red the PR if the sanitizer/compile build failed.
    gate = read_compile_status()
    sys.exit(1 if gate == "FAIL" else 0)


if __name__ == "__main__":
    main()
