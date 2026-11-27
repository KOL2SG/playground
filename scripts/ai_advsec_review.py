"""Advanced Security (code-scanning) AI review.

Reads GitHub code-scanning alerts for the current PR, feeds them to Ollama,
and posts a concise, optimised, human-readable review as a PR comment.

Only reads - it does not upload SARIF. The PR must have had an analysis
uploaded to GitHub code scanning for the target ref, otherwise this script
reports "no open findings" (see fetch_code_scanning_alerts).

Environment (set by .github/workflows/ai-security-review.yml):
  OLLAMA_URL         -> e.g. http://127.0.0.1:11434/api/generate
  OLLAMA_MODEL       -> e.g. qwen3.8:latest
  GITHUB_TOKEN       -> secrets.GITHUB_TOKEN (needs security-events: read)
  GITHUB_REPOSITORY  -> e.g. KOL2SG/playground
  PR_NUMBER          -> e.g. 42
"""
import json
import os
import sys
import urllib.parse
import urllib.request

OLLAMA_URL = os.environ.get("OLLAMA_URL", "http://127.0.0.1:11434/api/generate")
MODEL = os.environ.get("OLLAMA_MODEL", "qwen3.8:latest")
GITHUB_TOKEN = os.environ.get("GITHUB_TOKEN")
REPO = os.environ.get("GITHUB_REPOSITORY")
PR_NUMBER = os.environ.get("PR_NUMBER")

API_HEADERS = {
    "Authorization": f"Bearer {GITHUB_TOKEN}",
    "Accept": "application/vnd.github+json",
    "X-GitHub-Api-Version": "2022-11-28",
}

REVIEW_PROMPT = """You are an elite code reviewer. You are given GitHub Advanced Security
code-scanning findings for a single pull request, filtered to its HEAD ref.

Reply with a concise, prioritized, fixed-ready markdown review:

1. A short verdict: solid / ok with minor issues / fixes needed.
2. A findings table with columns: File | Line | Severity | Rule | Fix.
3. A "High-risk" list (memory corruption, UB, crash, auth bypass, secret leak,
   data race, resource leak, insecure deps).
4. Concrete remediation for each finding, not just a description.
5. A final "To do" checklist.

Keep it short. Do NOT repeat raw JSON back.

Findings:"""

def fetch_code_scanning_alerts(repo: str, pr_number: str) -> dict:
    """Fetch open code-scanning alerts for one PR's HEAD ref."""
    ref = f"refs/pull/{pr_number}/head"
    ref = f"refs/pull/{pr_number}/head"
    url = (
        f"https://api.github.com/repos/{repo}/code-scanning/alerts"
        f"?ref={ref}&per_page=100"
    )
    req = urllib.request.Request(url, headers=API_HEADERS)
    with urllib.request.urlopen(req, timeout=30) as resp:
        return json.loads(resp.read().decode("utf-8"))


def format_alerts(alerts: dict, pr_number: str) -> str:
    items = alerts.get("alerts", [])
    total = alerts.get("total", 0)
    if not items:
        return (
            f"No open code-scanning findings for this PR (ref: refs/pull/{pr_number}/head)."
            "\n\nTip: upload an analysis (SARIF) so these are populated."
        )
    rows = []
    for a in items:
        rule = a.get("rule") or {}
        inst = a.get("most_recent_instance") or {}
        loc = inst.get("location") or {}
        severity = rule.get("severity") or "unknown"
        rule_id = rule.get("id") or "n/a"
        description = rule.get("description") or "n/a"
        path = loc.get("path") or "n/a"
        line = loc.get("start_line") or inst.get("start_line") or "n/a"
        state = a.get("state") or "open"
        rows.append(
            f"- [{state}][{severity}] `{rule_id}`: {description} (src/{path}:{line})"
        )
    rows_text = "\n".join(rows)
    return f"Open Advanced Security findings: **{total}**.\n\n{rows_text}"


def prompt_ollama(text: str) -> str:
    payload = json.dumps({
        "model": MODEL,
        "prompt": REVIEW_PROMPT + "\n\n```\n" + text + "\n```",
        "stream": False,
    }).encode("utf-8")
    req = urllib.request.Request(
        OLLAMA_URL, data=payload,
        headers={"Content-Type": "application/json"},
    )
    with urllib.request.urlopen(req, timeout=600) as resp:
        body = json.loads(resp.read().decode("utf-8"))
    return body["response"]


def post_comment(pr_number: str, review_text: str) -> None:
    """Post a review comment on the PR."""
    url = f"https://api.github.com/repos/{REPO}/issues/{pr_number}/comments"
    payload = json.dumps({
        "body": f"**Automated Advanced Security review (self-hosted Ollama)**\n\n{review_text}",
    }).encode("utf-8")
    req = urllib.request.Request(
        url, data=payload, method="POST",
        headers={
            "Authorization": f"Bearer {GITHUB_TOKEN}",
            "Accept": "application/vnd.github+json",
            "Content-Type": "application/json",
        },
    )
    urllib.request.urlopen(req, timeout=30)


def main() -> None:
    assert GITHUB_TOKEN and REPO and PR_NUMBER, (
        "missing environment for Advanced Security review"
    )
    pr_number: str = PR_NUMBER
    repo: str = REPO
    alerts = fetch_code_scanning_alerts(repo, pr_number)
    data = format_alerts(alerts, pr_number)
    review = prompt_ollama(data)
    post_comment(pr_number, review)
    print("Posted Advanced Security review for PR #" + pr_number)


if __name__ == "__main__":
    main()
