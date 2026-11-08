"""Run with: python3 .github/codeql/tests/verify.py /path/to/codeql"""
import json
from pathlib import Path
import shlex
import subprocess
import sys
import tempfile


source = Path(__file__).resolve().with_name("check.c")
query = source.parent.parent / "glib" / "NonConstantFormat.ql"
codeql = sys.argv[1] if len(sys.argv) > 1 else "codeql"
expected = {
    line for line, text in enumerate(source.read_text().splitlines(), 1)
    if "/* ALERT" in text
}

with tempfile.TemporaryDirectory(prefix="dvdisaster-glib-models-") as directory:
    temporary = Path(directory)
    database = temporary / "database"
    output = temporary / "results.sarif"
    command = shlex.join([
        "cc", "-std=gnu17", "-Wno-format-security", "-c", str(source),
        "-o", str(temporary / "check.o"),
    ])
    subprocess.run([
        codeql, "database", "create", str(database), "--language=cpp",
        f"--source-root={source.parent}", f"--command={command}",
    ], check=True)
    subprocess.run([
        codeql, "database", "analyze", str(database), str(query),
        "--format=sarif-latest", f"--output={output}", "--threads=2",
        "--ram=2638",
    ], check=True)
    actual = {
        result["locations"][0]["physicalLocation"]["region"]["startLine"]
        for run in json.loads(output.read_text())["runs"]
        for result in run["results"]
    }
    if actual != expected:
        raise SystemExit(f"Expected alert lines {sorted(expected)}, got {sorted(actual)}")
    print("PASS: literal formats pass; untrusted formats and possible %n writes are detected.")
