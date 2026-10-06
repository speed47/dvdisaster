#!/usr/bin/env python3
"""Fill untranslated PO entries using machine translation.

Usage:
    python3 scripts/fill-translations.py locale/de.po de
    python3 scripts/fill-translations.py locale/it.po it --dry-run

Requires: pip install deep-translator polib
"""
import argparse
import sys
import polib
from deep_translator import GoogleTranslator


LANG_MAP = {
    "pt_BR": "pt",
    "cs": "cs",
    "de": "de",
    "it": "it",
    "ru": "ru",
    "sv": "sv",
    "be": "be",
    "ka": "ka",
    "pl": "pl",
    "tr": "tr",
    "uk": "uk",
}


def translate_text(text: str, target_lang: str) -> str:
    if not text.strip():
        return text
    try:
        return GoogleTranslator(source="en", target=target_lang).translate(text)
    except Exception as e:
        print(f"  WARNING: translation failed for {text[:40]!r}: {e}", file=sys.stderr)
        return text


def fill_po(po_path: str, locale: str, dry_run: bool = False) -> None:
    lang_code = LANG_MAP.get(locale, locale)
    try:
        po = polib.pofile(po_path)
    except Exception as e:
        print(f"ERROR: cannot read {po_path}: {e}", file=sys.stderr)
        sys.exit(1)

    untranslated = po.untranslated_entries()
    print(f"{po_path}: {len(untranslated)} untranslated entries, target lang={lang_code}")

    for i, entry in enumerate(untranslated, 1):
        if not entry.msgid.strip():
            continue
        print(f"  [{i}/{len(untranslated)}] {entry.msgid[:60]!r}", end="", flush=True)
        translated = translate_text(entry.msgid, lang_code)
        print(f" -> {translated[:60]!r}")

        if not dry_run:
            entry.msgstr = translated
            if "fuzzy" not in entry.flags:
                entry.flags.append("fuzzy")

        if entry.msgid_plural:
            plural_translated = translate_text(entry.msgid_plural, lang_code)
            if not dry_run:
                entry.msgstr_plural = {0: translated, 1: plural_translated}

    if not dry_run:
        po.save(po_path)
        print(f"Saved {po_path}")
    else:
        print("Dry run — no changes written.")


def main():
    parser = argparse.ArgumentParser(description="Fill untranslated PO entries")
    parser.add_argument("po_file", help="Path to .po file")
    parser.add_argument("locale", help="Locale code (e.g. de, it, pt_BR)")
    parser.add_argument("--dry-run", action="store_true", help="Print without saving")
    args = parser.parse_args()
    fill_po(args.po_file, args.locale, dry_run=args.dry_run)


if __name__ == "__main__":
    main()
