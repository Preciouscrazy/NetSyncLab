"""Check UTF-8 text and selected invisible controls; not an authorship detector."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EXTENSIONS = {".cpp", ".hpp", ".md", ".py", ".txt", ".yml", ".yaml", ".cmake", ".csv"}
BAD = {"\u200b", "\u200c", "\u200d", "\u2060", "\ufeff", "\u202e"}
count = 0
for path in ROOT.rglob("*"):
    if not path.is_file() or any(part == ".git" or part == "build" or part.startswith("build-") or part == ".vs" for part in path.relative_to(ROOT).parts):
        continue
    if path.suffix not in EXTENSIONS and path.name not in {"CMakeLists.txt", ".gitignore", ".editorconfig"}:
        continue
    raw = path.read_bytes()
    if raw.startswith((b"\xef\xbb\xbf", b"\xff\xfe", b"\xfe\xff")):
        raise SystemExit(f"Unexpected BOM: {path}")
    contents = raw.decode("utf-8", errors="strict")
    if any(char in BAD for char in contents):
        raise SystemExit(f"Unexpected invisible character: {path}")
    count += 1
print(f"PASS: {count} UTF-8 text files (source-hygiene checks only)")
