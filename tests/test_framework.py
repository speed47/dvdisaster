"""
Unit tests for the golden-test framework itself.
"""

import os
import tempfile

import pytest

from framework import (
    Byteset,
    CreateECC,
    Erase,
    GoldenTest,
    PadBytes,
    PadSectors,
    SimCD,
    Truncate,
    clean_output,
    parse_golden_file,
    resolve_golden_path,
)


# ---------------------------------------------------------------------------
# Damage operation CLI arg generation
# ---------------------------------------------------------------------------


class TestDamageOps:
    def test_erase_simple(self):
        op = Erase("1000-1049")
        assert op.cli_args() == ["--erase", "1000-1049"]

    def test_erase_single_sector(self):
        op = Erase("42")
        assert op.cli_args() == ["--erase", "42"]

    def test_erase_with_fill_unreadable(self):
        op = Erase("500", fill_unreadable=0)
        assert op.cli_args() == ["--erase", "500", "--fill-unreadable=0"]

    def test_byteset(self):
        op = Byteset(13444, 0, 154)
        assert op.cli_args() == ["--byteset", "13444,0,154"]

    def test_byteset_large_values(self):
        op = Byteset(20999, 255, 255)
        assert op.cli_args() == ["--byteset", "20999,255,255"]

    def test_truncate(self):
        op = Truncate(20000)
        assert op.cli_args() == ["--truncate=20000"]

    def test_pad_bytes(self):
        op = PadBytes(56)
        assert op.pad_size == 56

    def test_pad_sectors(self):
        op = PadSectors(5000)
        assert op.pad_size == 5000 * 2048

    def test_pad_sectors_zero(self):
        op = PadSectors(0)
        assert op.pad_size == 0


# ---------------------------------------------------------------------------
# Golden file parsing
# ---------------------------------------------------------------------------

_GOLDEN_WITH_MD5 = """\
9503f278d4550a9507a317664481adf8
4be4dcc0f6b88965334ccf1050dfa5fa
This software comes with  ABSOLUTELY NO WARRANTY.  This
is free software and you are welcome to redistribute it
under the conditions of the GNU GENERAL PUBLIC LICENSE.
See the file "COPYING" for further information.

rs01-master.iso: present, contains 21000 medium sectors.
- good image       : all sectors present
"""

_GOLDEN_IGNORE = """\
ignore
ignore
This software comes with  ABSOLUTELY NO WARRANTY.  This
is free software and you are welcome to redistribute it
under the conditions of the GNU GENERAL PUBLIC LICENSE.
See the file "COPYING" for further information.

Opening none.iso: No such file or directory.
"""


class TestGoldenFileParsing:
    def test_parse_with_checksums(self, tmp_path):
        p = str(tmp_path / "golden")
        with open(p, "w") as f:
            f.write(_GOLDEN_WITH_MD5)
        img_md5, ecc_md5, output = parse_golden_file(p)
        assert img_md5 == "9503f278d4550a9507a317664481adf8"
        assert ecc_md5 == "4be4dcc0f6b88965334ccf1050dfa5fa"
        assert output.startswith("This software comes with")
        assert "rs01-master.iso" in output

    def test_parse_with_ignore(self, tmp_path):
        p = str(tmp_path / "golden")
        with open(p, "w") as f:
            f.write(_GOLDEN_IGNORE)
        img_md5, ecc_md5, output = parse_golden_file(p)
        assert img_md5 is None
        assert ecc_md5 is None
        assert "Opening none.iso" in output

    def test_parse_expected_output_starts_at_line3(self, tmp_path):
        p = str(tmp_path / "golden")
        with open(p, "w") as f:
            f.write("md5img\nmd5ecc\nline3\nline4\n")
        _, _, output = parse_golden_file(p)
        assert output == "line3\nline4\n"
        # Must NOT contain md5 lines
        assert "md5img" not in output

    def test_resolve_golden_path_base(self, tmp_path):
        base = str(tmp_path / "RS01_test")
        with open(base, "w") as f:
            f.write("test")
        result = resolve_golden_path(base)
        assert result == base

    def test_resolve_golden_path_nonexistent_variant(self, tmp_path):
        """When platform variant doesn't exist, fall back to base."""
        base = str(tmp_path / "RS01_test")
        with open(base, "w") as f:
            f.write("test")
        result = resolve_golden_path(base)
        assert result == base


# ---------------------------------------------------------------------------
# Output cleaning
# ---------------------------------------------------------------------------


class TestCleanOutput:
    def test_strip_header(self):
        text = "line1\nline2\nline3\nactual output\nmore output\n"
        result = clean_output(text, strip_header=True)
        assert result == "actual output\nmore output\n"

    def test_strip_header_short(self):
        text = "line1\nline2\n"
        result = clean_output(text, strip_header=True)
        assert result == ""

    def test_remove_memleak_line(self):
        text = "some output\ndvdisaster: No memory leaks found.\nmore output\n"
        result = clean_output(text)
        assert "No memory leaks found" not in result
        assert "some output" in result
        assert "more output" in result

    def test_remove_windows_paths(self):
        text = "Opening C:/Users/runner/AppData/Local/Temp/test.iso: ok\n"
        result = clean_output(text)
        assert "C:/" not in result

    def test_remove_github_actions_temp(self):
        text = "Opening runner-abc/AppData/Local/Temp/test.iso\n"
        result = clean_output(text)
        assert "AppData" not in result

    def test_remove_tmp_dirs(self):
        text = "file /dev/shm/test/rs01-tmp.iso is present\n"
        result = clean_output(text, tmp_dirs=["/dev/shm/test"])
        assert "/dev/shm/test/" not in result
        assert "rs01-tmp.iso" in result

    def test_remove_regtest_prefix(self):
        text = "regtest/database/RS01_good\n"
        result = clean_output(text)
        assert result == "database/RS01_good\n"

    def test_remove_isodir(self):
        text = "/var/tmp/regtest/rs01-master.iso: present\n"
        result = clean_output(text)
        assert result == "rs01-master.iso: present\n"

    def test_normalize_crlf(self):
        # Windows mingw build writes CRLF; golden files use LF.
        text = "line one\r\nline two\r\n"
        result = clean_output(text)
        assert result == "line one\nline two\n"

    def test_combined_cleaning(self):
        """Test multiple cleaning operations together."""
        text = (
            "dvdisaster version\ncopyright\nmore header\n"
            "dvdisaster: No memory leaks found.\n"
            "/var/tmp/regtest/rs01-master.iso: present\n"
        )
        result = clean_output(text, strip_header=True)
        assert "No memory leaks found" not in result
        assert result == "rs01-master.iso: present\n"


# ---------------------------------------------------------------------------
# GoldenTest construction
# ---------------------------------------------------------------------------


class TestGoldenTestConstruction:
    def test_minimal(self):
        t = GoldenTest(name="test1", action="-t")
        assert t.name == "test1"
        assert t.action == "-t"
        assert t.damage is None
        assert t.use_master is False
        assert t.extra_args is None

    def test_with_damage(self):
        t = GoldenTest(
            name="test2",
            action="-t",
            damage=[Erase("100"), Byteset(200, 0, 255)],
        )
        assert len(t.damage) == 2
        assert isinstance(t.damage[0], Erase)
        assert isinstance(t.damage[1], Byteset)

    def test_with_sim_cd(self):
        t = GoldenTest(
            name="test3",
            action="-s",
            sim_cd=SimCD(
                source="master",
                damage=[Byteset(0, 100, 255)],
            ),
        )
        assert t.sim_cd.source == "master"
        assert len(t.sim_cd.damage) == 1

    def test_with_create_ecc(self):
        t = GoldenTest(
            name="test4",
            action="-c",
            create_ecc=CreateECC(method="RS01", redundancy="normal"),
        )
        assert t.create_ecc.method == "RS01"
        assert t.create_ecc.redundancy == "normal"

    def test_with_all_fields(self):
        t = GoldenTest(
            name="full",
            action="-f",
            damage=[Erase("1")],
            use_master=False,
            image="custom.iso",
            ecc="custom.ecc",
            extra_args=["--verbose"],
            ecc_damage=[Byteset(0, 0, 0)],
            chmod_image=0o000,
            chmod_ecc=0o444,
        )
        assert t.chmod_image == 0
        assert t.chmod_ecc == 0o444
        assert t.extra_args == ["--verbose"]
