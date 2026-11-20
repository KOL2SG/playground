import json
import os
import urllib.request

OLLAMA_URL = os.environ["OLLAMA_URL"]  # e.g. http://10.20.0.15:11434/api/generate
MODEL = os.environ.get("OLLAMA_MODEL", "hf.co/unsloth/Qwen3.8-27B-GGUF:UD-Q4_K_M")
# MODEL = os.environ.get("OLLAMA_MODEL", "beam:cloud")
GITHUB_TOKEN = os.environ["GITHUB_TOKEN"]
REPO = os.environ["GITHUB_REPOSITORY"]
PR_NUMBER = os.environ["PR_NUMBER"]

REVIEW_PROMPT = """You are an elite code reviewer. Review the following dragur report.
Respond starting with a table of found issues (organised in coloumn like Issue, Severity,
Description, File:Line, Recommendation), how it matters, suggestions to fix if any.
Be specific and reference file names and line numbers where possible.

```json
{data_report}
"""

def get_dragur_report():
    workspace = os.environ.get("GITHUB_WORKSPACE", ".")
    report_file = os.path.join(workspace, "draugr-out", "report.json")
    with open(report_file, "r", encoding="utf-8") as f:
        return json.load(f)

def call_ollama(draugr_report):
    data_report = json.dumps(draugr_report, indent=2)
    payload = json.dumps({
        "model": MODEL,
        "prompt": REVIEW_PROMPT.format(data_report=data_report),
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
    report = get_dragur_report()
    review = call_ollama(report)
    post_comment(review)
    print("Posted review comment to PR #" + PR_NUMBER)
