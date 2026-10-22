import json
import os
import urllib.request

OLLAMA_URL = os.environ["OLLAMA_URL"]  # e.g. http://10.20.0.15:11434/api/generate
MODEL = os.environ.get("OLLAMA_MODEL", "qwen2.5-coder:7b")
GITHUB_TOKEN = os.environ["GITHUB_TOKEN"]
REPO = os.environ["GITHUB_REPOSITORY"]
PR_NUMBER = os.environ["PR_NUMBER"]

REVIEW_PROMPT = """You are a senior code reviewer. Review the following diff.
Respond with a short bullet list covering: likely bugs, missing tests,
and style issues. Be specific and reference file names and line numbers
where possible. Keep the review under 300 words.

DIFF:
{diff}
"""

def get_diff():
    with open("pr.diff", "r", encoding="utf-8") as f:
        return f.read()

def call_ollama(diff_text):
    payload = json.dumps({
        "model": MODEL,
        "prompt": REVIEW_PROMPT.format(diff=diff_text),  # keep under context window
        "stream": False,
    }).encode("utf-8")
    req = urllib.request.Request(OLLAMA_URL, data=payload, headers={"Content-Type": "application/json"})
    with urllib.request.urlopen(req, timeout=120) as resp:
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
