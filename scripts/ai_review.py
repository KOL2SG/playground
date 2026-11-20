import json
import os
import urllib.request

OLLAMA_URL = os.environ["OLLAMA_URL"]  # e.g. http://10.20.0.15:11434/api/generate
MODEL = os.environ.get("OLLAMA_MODEL", "qwen3.8:latest")
# MODEL = os.environ.get("OLLAMA_MODEL", "beam:cloud")
GITHUB_TOKEN = os.environ["GITHUB_TOKEN"]
REPO = os.environ["GITHUB_REPOSITORY"]
PR_NUMBER = os.environ["PR_NUMBER"]

REVIEW_PROMPT = """You are an elite code reviewer. Review the following diff.

Return results as Markdown with:
1. A short verdict (solid / ok with minor issues / fixes needed).
2. A findings table: File | Line | Severity (Critical/Warning/Nit) | Fix.
3. A "High-risk" list (memory corruption, UB, crash, data race, leak).
4. Passing checks (compile gate, sanitizers, formatting).

Be concise but give the concrete fix, not just a description.

DIFF:
{diff}
"""

def get_diff():
    with open("pr.diff", "r", encoding="utf-8") as f:
        return f.read()

def call_ollama(diff_text):
    payload = json.dumps({
        "model": MODEL,
        "prompt": REVIEW_PROMPT.format(diff=diff_text[:12000]),  # keep under context window
        "stream": False,
    }).encode("utf-8")
    req = urllib.request.Request(OLLAMA_URL, data=payload, headers={"Content-Type": "application/json"})
    with urllib.request.urlopen(req, timeout=600) as resp:
        body = json.loads(resp.read())
    return body["response"]

def post_comment(review_text):
    url = f"https://api.github.com/repos/{REPO}/issues/{PR_NUMBER}/comments"
    payload = json.dumps({"body": f"**Automated review (self-hosted Ollama)**\n\n{review_text}"}).encode("utf-8")
    req = urllib.request.Request(url, data=payload, method="POST", headers={
        "Authorization": f"Bearer {GITHUB_TOKEN}",
        "Accept": "application/vnd.github+json",
        "Content-Type": "application/json",
    })
    urllib.request.urlopen(req, timeout=30)

if __name__ == "__main__":
    diff = get_diff()
    review = call_ollama(diff)
    post_comment(review)
    print("Posted review comment to PR #" + PR_NUMBER)
