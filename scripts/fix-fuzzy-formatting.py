#!/usr/bin/env python3
"""Fix formatting issues in machine-translated fuzzy PO entries.

Problems introduced by GoogleTranslator:
  1. Newline mismatches: msgid starts/ends with \\n but msgstr doesn't
  2. %S instead of %s (translator capitalized format specifier)
  3. %% -> % (translator unescaped double-percent literal)
  4. Residual format spec mismatches: detected via msgfmt and cleared

Strategy:
  - Fix newlines, then fix known format corruptions
  - Run msgfmt --use-fuzzy -c on the whole file to find remaining errors
  - Clear the msgstr on any entries that still report errors (English fallback)
  - Repeat until msgfmt is clean

Usage:
    python3 scripts/fix-fuzzy-formatting.py locale/de.po
    python3 scripts/fix-fuzzy-formatting.py locale/*.po
"""
import re
import subprocess
import sys
import polib


def fix_newlines(msgid: str, msgstr: str) -> str:
    if not msgstr:
        return msgstr
    if msgid.startswith("\n") and not msgstr.startswith("\n"):
        msgstr = "\n" + msgstr
    elif not msgid.startswith("\n") and msgstr.startswith("\n"):
        msgstr = msgstr.lstrip("\n")
    if msgid.endswith("\n") and not msgstr.endswith("\n"):
        msgstr = msgstr + "\n"
    elif not msgid.endswith("\n") and msgstr.endswith("\n"):
        msgstr = msgstr.rstrip("\n")
    return msgstr


def fix_format_specifiers(msgid: str, msgstr: str) -> str:
    if not msgstr:
        return msgstr
    # %S -> %s (translator capitalized the specifier)
    msgstr = re.sub(r'%S\b', '%s', msgstr)
    # Restore missing %% where msgid has more %% occurrences than msgstr
    id_count = len(re.findall(r'%%', msgid))
    str_count = len(re.findall(r'%%', msgstr))
    if id_count > str_count:
        fixes_left = id_count - str_count
        result = []
        i = 0
        while i < len(msgstr) and fixes_left > 0:
            if msgstr[i] == '%':
                if i + 1 < len(msgstr) and msgstr[i + 1] == '%':
                    result.append('%%')
                    i += 2
                else:
                    result.append('%%')
                    i += 1
                    fixes_left -= 1
            else:
                result.append(msgstr[i])
                i += 1
        result.append(msgstr[i:])
        msgstr = ''.join(result)
    return msgstr


def get_error_lines(po_path: str) -> set:
    """Return set of line numbers reported as errors by msgfmt --use-fuzzy -c."""
    result = subprocess.run(
        ['msgfmt', '--use-fuzzy', '-c', po_path, '-o', '/dev/null'],
        capture_output=True, text=True
    )
    lines = set()
    for line in result.stderr.splitlines():
        m = re.match(rf'^{re.escape(po_path)}:(\d+):', line)
        if m:
            lines.add(int(m.group(1)))
    return lines


def linenum_to_entry(po, error_lines: set):
    """Find PO entries that contain the given error line numbers.

    polib reports entry.linenum at the first #: comment line; msgfmt
    reports errors at the msgstr line — typically 5-15 lines later.
    We find the fuzzy entry whose linenum is closest to (but <= ) each
    error line within a 30-line window.
    """
    fuzzy = sorted(po.fuzzy_entries(), key=lambda e: e.linenum)
    bad = set()
    for err_line in error_lines:
        # Walk backwards through sorted entries to find the one that starts
        # before err_line but is within 30 lines
        for entry in reversed(fuzzy):
            if entry.linenum <= err_line <= entry.linenum + 30:
                bad.add(id(entry))
                break
    return [e for e in fuzzy if id(e) in bad]


def fix_po(po_path: str) -> None:
    try:
        po = polib.pofile(po_path)
    except Exception as e:
        print(f"ERROR: cannot read {po_path}: {e}", file=sys.stderr)
        sys.exit(1)

    fuzzy = po.fuzzy_entries()
    print(f"{po_path}: {len(fuzzy)} fuzzy entries")

    fixed_newlines = 0
    fixed_format = 0
    cleared = 0

    # Pass 1: structural fixes
    for entry in fuzzy:
        if not entry.msgid.strip() or not entry.msgstr:
            continue

        fixed = fix_newlines(entry.msgid, entry.msgstr)
        if fixed != entry.msgstr:
            entry.msgstr = fixed
            fixed_newlines += 1

        if entry.msgstr_plural:
            ref = entry.msgid_plural or entry.msgid
            for k, v in entry.msgstr_plural.items():
                fv = fix_newlines(ref, v)
                if fv != v:
                    entry.msgstr_plural[k] = fv

        if 'c-format' in entry.flags:
            fixed = fix_format_specifiers(entry.msgid, entry.msgstr)
            if fixed != entry.msgstr:
                entry.msgstr = fixed
                fixed_format += 1

    po.save(po_path)

    # Pass 2: validate whole file, clear any entries still failing
    error_lines = get_error_lines(po_path)
    if error_lines:
        bad_entries = linenum_to_entry(po, error_lines)
        for entry in bad_entries:
            entry.msgstr = ''
            if entry.msgstr_plural:
                entry.msgstr_plural = {k: '' for k in entry.msgstr_plural}
            cleared += 1
        po.save(po_path)

        # Final check
        remaining = get_error_lines(po_path)
        if remaining:
            print(f"  WARNING: {len(remaining)} error lines still remain after clearing")

    print(f"  fixed newlines: {fixed_newlines}")
    print(f"  fixed format specifiers: {fixed_format}")
    print(f"  cleared still-broken entries: {cleared}")


def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <file.po> [file.po ...]")
        sys.exit(1)
    for path in sys.argv[1:]:
        fix_po(path)


if __name__ == "__main__":
    main()
