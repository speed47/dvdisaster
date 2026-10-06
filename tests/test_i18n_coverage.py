"""CI gate: every locale/*.po must be format-clean and ≥90% translated."""
import subprocess
import re
from pathlib import Path
import pytest

LOCALE_DIR = Path(__file__).parent.parent / "locale"
THRESHOLD_PCT = 90.0

def get_po_files():
    return sorted(LOCALE_DIR.glob("*.po"))

def parse_stats(po_path):
    result = subprocess.run(
        ["msgfmt", "--use-fuzzy", "--statistics", str(po_path), "-o", "/dev/null"],
        capture_output=True, text=True
    )
    if result.returncode != 0:
        raise RuntimeError(
            f"msgfmt failed on {po_path}:\n{result.stderr}"
        )
    output = result.stderr + result.stdout
    translated = int(re.search(r'\b(\d+) translated\b', output).group(1)) if 'translated' in output else 0
    untranslated = int(re.search(r'\b(\d+) untranslated\b', output).group(1)) if 'untranslated' in output else 0
    fuzzy = int(re.search(r'\b(\d+) fuzzy\b', output).group(1)) if 'fuzzy' in output else 0
    total = translated + untranslated + fuzzy
    return translated, untranslated, fuzzy, total

@pytest.mark.parametrize("po_path", get_po_files(), ids=lambda p: p.stem)
def test_po_format_clean(po_path):
    """Fail if msgfmt --use-fuzzy -c reports any format errors."""
    result = subprocess.run(
        ["msgfmt", "--use-fuzzy", "-c", str(po_path), "-o", "/dev/null"],
        capture_output=True, text=True
    )
    assert result.returncode == 0, (
        f"{po_path.name}: msgfmt format errors:\n{result.stderr[:500]}"
    )

@pytest.mark.parametrize("po_path", get_po_files(), ids=lambda p: p.stem)
def test_translation_coverage(po_path):
    """Fail if usable (translated+fuzzy) strings fall below threshold."""
    translated, untranslated, fuzzy, total = parse_stats(po_path)
    assert total > 0, f"{po_path.name}: no strings found"
    # fuzzy entries are machine-translated and compiled into .mo via --use-fuzzy
    usable = translated + fuzzy
    pct = usable / total * 100
    assert pct >= THRESHOLD_PCT, (
        f"{po_path.name}: {pct:.1f}% usable ({usable}/{total}), "
        f"need {THRESHOLD_PCT}% (fuzzy={fuzzy} await human review)"
    )
