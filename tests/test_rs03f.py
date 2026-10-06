"""
RS03f regression tests -- from regtest/rs03f.bash.

RS03f is the file-based ECC mode: ECC data is stored in a separate .ecc file
(like RS01), but uses the RS03 codec with configurable redundancy roots.

Tests are grouped into:
  - TestRS03fVerify: 39 verify tests
  - TestRS03fCreate: 14 creation tests
  - TestRS03fRepair: 25 repair/fix tests
  - TestRS03fScan: 18 scanning tests
  - TestRS03fReadLinear: 17 linear reading tests
  - TestRS03fReadAdaptive: 1 adaptive reading test
"""

import difflib
import os
import re
import shutil
import sys

import pytest

# POSIX chmod 0o000/0o400 is not enforced on NTFS, so the Windows dvdisaster
# build cannot trigger "permission denied" errors for locally-created files.
_SKIP_CHMOD_WIN = pytest.mark.skipif(
    sys.platform == "win32",
    reason="POSIX chmod semantics not honored on NTFS",
)

from framework import (
    Byteset,
    Erase,
    GoldenTest,
    GoldenTestSuite,
    PadSectors,
    Truncate,
    _ISODIR,
    _TMPDIR,
    _find_binary,
    _md5_file,
    _run_dvdisaster,
    _apply_damage,
    clean_output,
    parse_golden_file,
    resolve_golden_path,
)

_PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
_DATABASE = os.path.join(_PROJECT_ROOT, "regtest", "database")
_FIXED_RANDOM_SEQ = os.path.join(_PROJECT_ROOT, "regtest", "fixed-random-sequence")

# Constants matching the bash variables
ISOSIZE = 21000
SETVERSION = "0.80"
REDUNDANCY_ROOTS = 20
REDUNDANCY = "{}r".format(REDUNDANCY_ROOTS)


# ---------------------------------------------------------------------------
# Session fixture: create plus56 setup images (shared across all tests)
# ---------------------------------------------------------------------------

@pytest.fixture(scope="session")
def plus56_images():
    """Create rs03f-plus56_bytes.iso and .ecc in ISODIR if they don't exist.

    These mirror the bash preamble that creates ISO_PLUS56 and ECC_PLUS56.
    Note: RS03f uses bytes from the fixed-random-sequence file (not zeros).
    """
    os.makedirs(_ISODIR, exist_ok=True)
    master_iso = os.path.join(_ISODIR, "rs03f-master.iso")
    iso_plus56 = os.path.join(_ISODIR, "rs03f-plus56_bytes.iso")
    ecc_plus56 = os.path.join(_ISODIR, "rs03f-plus56_bytes.ecc")

    # Ensure master exists
    if not os.path.isfile(master_iso):
        _run_dvdisaster(
            "--regtest", "--debug",
            "-i{}".format(master_iso),
            "--random-image", str(ISOSIZE),
            check=True,
        )

    # Create plus56 ISO: master + 56 bytes from fixed-random-sequence
    if not os.path.isfile(iso_plus56):
        shutil.copy2(master_iso, iso_plus56)
        with open(_FIXED_RANDOM_SEQ, "rb") as f:
            data = f.read(56)
        with open(iso_plus56, "ab") as f:
            f.write(data)

    # Create plus56 ECC
    if not os.path.isfile(ecc_plus56):
        # Ensure master ECC exists too
        master_ecc = os.path.join(_ISODIR, "rs03f-master.ecc")
        if not os.path.isfile(master_ecc):
            _run_dvdisaster(
                "--regtest", "--debug", "--set-version", SETVERSION,
                "-i{}".format(master_iso),
                "-e{}".format(master_ecc),
                "-mRS03", "-n", REDUNDANCY, "-o", "file", "-c",
                check=True,
            )
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(iso_plus56),
            "-e{}".format(ecc_plus56),
            "-mRS03", "-n", REDUNDANCY, "-o", "file", "-c",
            check=True,
        )

    return iso_plus56, ecc_plus56


# ---------------------------------------------------------------------------
# Common damage patterns (reused by multiple tests)
# ---------------------------------------------------------------------------

_DAMAGE_DSM1 = [
    Erase("3030"),
    Byteset(3030, 353, 49),
    Erase("4400"),
    Byteset(4400, 353, 53),
    Erase("4411"),
    Byteset(4411, 353, 53),
]

_DAMAGE_DSM2 = [
    Erase("3030"),
    Byteset(3030, 416, 55),
    Byteset(3030, 556, 32),
    Byteset(3030, 557, 50),
    Erase("4400"),
    Byteset(4400, 416, 53),
    Byteset(4400, 556, 32),
    Byteset(4400, 557, 50),
    Erase("4411"),
    Byteset(4411, 416, 53),
    Byteset(4411, 556, 32),
    Byteset(4411, 557, 50),
]


# ---------------------------------------------------------------------------
# Helper: run a golden-file test with prepared image/ecc paths
# ---------------------------------------------------------------------------

def _run_golden_compare(test_name, cmd_args, tmp_path,
                        image_path=None, ecc_path=None,
                        ignore_line_re=None):
    """Run dvdisaster, clean output, compare against golden file.

    Args:
        test_name: name matching the golden file (e.g. 'good')
        cmd_args: list of CLI arguments
        tmp_path: pytest tmp_path for cleaning
        image_path: path to image file for MD5 check (or None)
        ecc_path: path to ecc file for MD5 check (or None)
        ignore_line_re: regex pattern for lines to strip from output
    """
    golden_base = os.path.join(_DATABASE, "RS03f_{}".format(test_name))
    golden_path = resolve_golden_path(golden_base)
    if not os.path.isfile(golden_path):
        pytest.skip("Golden file not found: {}".format(golden_path))

    expected_image_md5, expected_ecc_md5, expected_output = parse_golden_file(golden_path)

    _, raw_output = _run_dvdisaster(*cmd_args)

    work_dir = str(tmp_path)
    cleaned = clean_output(
        raw_output,
        tmp_dirs=[work_dir, _TMPDIR, _ISODIR],
        strip_header=True,
    )

    # Filter ignored lines
    if ignore_line_re:
        lines = cleaned.split("\n")
        lines = [l for l in lines if not re.match(ignore_line_re, l)]
        cleaned = "\n".join(lines)

    if cleaned != expected_output:
        diff = difflib.unified_diff(
            expected_output.splitlines(keepends=True),
            cleaned.splitlines(keepends=True),
            fromfile="expected (golden)",
            tofile="actual (cleaned)",
        )
        diff_text = "".join(diff)
        assert cleaned == expected_output, (
            "Output mismatch for test '{}':\n{}".format(test_name, diff_text)
        )

    if expected_image_md5 is not None and image_path and os.path.isfile(image_path):
        actual_md5 = _md5_file(image_path)
        assert actual_md5 == expected_image_md5, (
            "Image MD5 mismatch for '{}': expected {}, got {}".format(
                test_name, expected_image_md5, actual_md5)
        )

    if expected_ecc_md5 is not None and ecc_path and os.path.isfile(ecc_path):
        actual_md5 = _md5_file(ecc_path)
        assert actual_md5 == expected_ecc_md5, (
            "ECC MD5 mismatch for '{}': expected {}, got {}".format(
                test_name, expected_ecc_md5, actual_md5)
        )


def _ensure_master():
    """Ensure the RS03f master image exists and return its path."""
    os.makedirs(_ISODIR, exist_ok=True)
    path = os.path.join(_ISODIR, "rs03f-master.iso")
    if not os.path.isfile(path):
        _run_dvdisaster(
            "--regtest", "--debug",
            "-i{}".format(path),
            "--random-image", str(ISOSIZE),
            check=True,
        )
    return path


def _ensure_master_ecc():
    """Ensure the RS03f master ECC exists and return its path."""
    master_iso = _ensure_master()
    path = os.path.join(_ISODIR, "rs03f-master.ecc")
    if not os.path.isfile(path):
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(master_iso),
            "-e{}".format(path),
            "-mRS03", "-n", REDUNDANCY, "-o", "file", "-c",
            check=True,
        )
    return path


# ---------------------------------------------------------------------------
# Test Suite: Verify
# ---------------------------------------------------------------------------

class TestRS03fVerify(GoldenTestSuite):
    codec = "RS03"
    codec_prefix = "RS03f"
    master = "rs03f-master.iso"
    master_ecc = "rs03f-master.ecc"
    image_size = ISOSIZE
    redundancy = REDUNDANCY

    def _ensure_master(self):
        """Override: RS03f master image creation."""
        return _ensure_master()

    def _ensure_master_ecc(self):
        """Override: RS03f needs -mRS03 -o file flags."""
        return _ensure_master_ecc()

    # ------------------------------------------------------------------
    # Declarative tests (auto-parametrized via test_golden)
    # ------------------------------------------------------------------
    tests = [
        # 1. good
        GoldenTest("good", action="-t", use_master=True, ecc="master_ecc"),
        # 2. good_quick
        GoldenTest("good_quick", action="-tq", use_master=True, ecc="master_ecc"),
        # 3. no_image (bash uses $MASTERECC, not no.ecc)
        GoldenTest("no_image", action="-t", image="no.iso", ecc="master_ecc"),
        # 4. image_truncated_by5
        GoldenTest("image_truncated_by5", action="-t",
                   damage=[Truncate(ISOSIZE - 5)], ecc="master_ecc"),
        # 5. 17_extra_sectors (bash uses /dev/zero)
        GoldenTest("17_extra_sectors", action="-t",
                   damage=[PadSectors(17)], ecc="master_ecc"),
        # 6. missing_sectors
        GoldenTest("missing_sectors", action="-t",
                   damage=[Erase("500-524")], ecc="master_ecc"),
        # 7. crc_errors
        GoldenTest("crc_errors", action="-t",
                   damage=[Byteset(670, 50, 50), Byteset(770, 50, 50),
                           Byteset(771, 50, 50), Byteset(772, 50, 50)],
                   ecc="master_ecc"),
        # 8. mixed_errors
        GoldenTest("mixed_errors", action="-t",
                   damage=[Erase("500-524"), Byteset(670, 50, 50),
                           Erase("699"), Byteset(770, 50, 50),
                           Byteset(771, 50, 50), Byteset(772, 50, 50),
                           Erase("978-1001")],
                   ecc="master_ecc"),
        # 9. crc_error_in_fingerprint
        GoldenTest("crc_error_in_fingerprint", action="-t",
                   damage=[Byteset(16, 450, 17)], ecc="master_ecc"),
        # 10. fingerprint_unreadable
        GoldenTest("fingerprint_unreadable", action="-t",
                   damage=[Erase("16")], ecc="master_ecc"),
        # 11. uncorrectable_dsm_in_image
        GoldenTest("uncorrectable_dsm_in_image", action="-t",
                   damage=_DAMAGE_DSM1, ecc="master_ecc"),
        # 12. uncorrectable_dsm_in_image_verbose
        GoldenTest("uncorrectable_dsm_in_image_verbose", action="-t -v",
                   damage=_DAMAGE_DSM1, ecc="master_ecc"),
        # 13. uncorrectable_dsm_in_image2
        GoldenTest("uncorrectable_dsm_in_image2", action="-t",
                   damage=_DAMAGE_DSM2, ecc="master_ecc"),
        # 14. uncorrectable_dsm_in_image2_verbose
        GoldenTest("uncorrectable_dsm_in_image2_verbose", action="-t -v",
                   damage=_DAMAGE_DSM2, ecc="master_ecc"),
    ]

    # ------------------------------------------------------------------
    # Manual tests: plus56 and related (need fresh ecc creation)
    # ------------------------------------------------------------------

    def test_plus56_bytes(self, tmp_path):
        """Verify image with 56 extra random bytes and its own ecc."""
        master = self._ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")

        shutil.copy2(master, tmp_iso)
        with open(_FIXED_RANDOM_SEQ, "rb") as f:
            data = f.read(56)
        with open(tmp_iso, "ab") as f:
            f.write(data)

        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "-mRS03", "-n", REDUNDANCY, "-o", "file", "-c",
            check=True,
        )

        _run_golden_compare(
            "plus56_bytes",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc), "-t"],
            tmp_path, image_path=tmp_iso, ecc_path=tmp_ecc,
        )

    def test_no_image_plus56_bytes(self, tmp_path):
        """No image; ecc for image with 56 extra bytes."""
        master = self._ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")

        shutil.copy2(master, tmp_iso)
        with open(_FIXED_RANDOM_SEQ, "rb") as f:
            data = f.read(56)
        with open(tmp_iso, "ab") as f:
            f.write(data)

        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "-mRS03", "-n", REDUNDANCY, "-o", "file", "-c",
            check=True,
        )

        no_iso = os.path.join(_ISODIR, "no.iso")
        _run_golden_compare(
            "no_image_plus56_bytes",
            ["--regtest", "--no-progress",
             "-i{}".format(no_iso), "-e{}".format(tmp_ecc), "-t"],
            tmp_path, ecc_path=tmp_ecc,
        )

    def test_special_padding(self, tmp_path):
        """Image with special padding situation (20124 sectors, divisible by layer size)."""
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")

        _run_dvdisaster(
            "--debug",
            "-i{}".format(tmp_iso),
            "--random-image", "20124",
            check=True,
        )

        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "-mRS03", "-n", REDUNDANCY, "-o", "file", "-c",
            check=True,
        )

        _run_golden_compare(
            "special_padding",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc), "-v", "-t"],
            tmp_path, image_path=tmp_iso, ecc_path=tmp_ecc,
        )

    def test_special_padding_plus56(self, tmp_path):
        """Image with special padding plus 56 bytes (20123 sectors + 56 bytes)."""
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")

        _run_dvdisaster(
            "--debug",
            "-i{}".format(tmp_iso),
            "--random-image", "20123",
            check=True,
        )

        with open(_FIXED_RANDOM_SEQ, "rb") as f:
            data = f.read(56)
        with open(tmp_iso, "ab") as f:
            f.write(data)

        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "-mRS03", "-n", REDUNDANCY, "-o", "file", "-c",
            check=True,
        )

        _run_golden_compare(
            "special_padding_plus56",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc), "-v", "-t"],
            tmp_path, image_path=tmp_iso, ecc_path=tmp_ecc,
        )

    def test_normal_image_ecc_plus56_bytes(self, tmp_path):
        """Normal master image verified against ecc from plus56 image."""
        master = self._ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")

        shutil.copy2(master, tmp_iso)
        with open(_FIXED_RANDOM_SEQ, "rb") as f:
            data = f.read(56)
        with open(tmp_iso, "ab") as f:
            f.write(data)

        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "-mRS03", "-n", REDUNDANCY, "-o", "file", "-c",
            check=True,
        )

        _run_golden_compare(
            "normal_image_ecc_plus56_bytes",
            ["--regtest", "--no-progress",
             "-i{}".format(master), "-e{}".format(tmp_ecc), "-t"],
            tmp_path, image_path=master, ecc_path=tmp_ecc,
        )

    def test_image_plus56_normal_ecc(self, tmp_path):
        """Image with 56 extra bytes verified against normal master ecc."""
        master = self._ensure_master()
        master_ecc = self._ensure_master_ecc()
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")

        shutil.copy2(master, tmp_iso)
        with open(_FIXED_RANDOM_SEQ, "rb") as f:
            data = f.read(56)
        with open(tmp_iso, "ab") as f:
            f.write(data)

        _run_golden_compare(
            "image_plus56_normal_ecc",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-e{}".format(master_ecc), "-t"],
            tmp_path, image_path=tmp_iso, ecc_path=master_ecc,
        )

    def test_few_bytes_shorter(self, tmp_path):
        """Image a few bytes shorter than expected; both not multiple of 2048."""
        master = self._ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        long_iso = os.path.join(str(tmp_path), "rs03f-plus390-bytes.iso")

        # Create +56 image
        shutil.copy2(master, tmp_iso)
        with open(_FIXED_RANDOM_SEQ, "rb") as f:
            rnd = f.read(390)
        with open(tmp_iso, "ab") as f:
            f.write(rnd[:56])

        # Create +390 image and its ecc
        shutil.copy2(master, long_iso)
        with open(long_iso, "ab") as f:
            f.write(rnd[:390])

        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(long_iso), "-e{}".format(tmp_ecc),
            "-mRS03", "-n", REDUNDANCY, "-o", "file", "-c",
            check=True,
        )

        _run_golden_compare(
            "few_bytes_shorter",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc), "-t"],
            tmp_path, image_path=tmp_iso, ecc_path=tmp_ecc,
        )

    def test_few_bytes_longer(self, tmp_path):
        """Image a few bytes longer than expected; both not multiple of 2048."""
        master = self._ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        short_iso = os.path.join(str(tmp_path), "rs03f-plus56-bytes.iso")

        with open(_FIXED_RANDOM_SEQ, "rb") as f:
            rnd = f.read(390)

        # Create +56 image and its ecc
        shutil.copy2(master, short_iso)
        with open(short_iso, "ab") as f:
            f.write(rnd[:56])

        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(short_iso), "-e{}".format(tmp_ecc),
            "-mRS03", "-n", REDUNDANCY, "-o", "file", "-c",
            check=True,
        )

        # Create +390 image
        shutil.copy2(master, tmp_iso)
        with open(tmp_iso, "ab") as f:
            f.write(rnd[:390])

        _run_golden_compare(
            "few_bytes_longer",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc), "-t"],
            tmp_path, image_path=tmp_iso, ecc_path=tmp_ecc,
        )

    def test_few_bytes_shorter2(self, tmp_path):
        """Image few bytes shorter than multiple of 2048."""
        master = self._ensure_master()
        master_ecc = self._ensure_master_ecc()
        truncated_size = 2048 * ISOSIZE - 104
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")

        with open(master, "rb") as src, open(tmp_iso, "wb") as dst:
            dst.write(src.read(truncated_size))

        _run_golden_compare(
            "few_bytes_shorter2",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-e{}".format(master_ecc), "-t"],
            tmp_path, image_path=tmp_iso, ecc_path=master_ecc,
        )

    def test_one_extra_sector(self, tmp_path):
        """Image with 1 extra sector (random data from fixed-random-sequence)."""
        master = self._ensure_master()
        master_ecc = self._ensure_master_ecc()
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")

        shutil.copy2(master, tmp_iso)
        with open(_FIXED_RANDOM_SEQ, "rb") as f:
            data = f.read(2048)
        with open(tmp_iso, "ab") as f:
            f.write(data)

        _run_golden_compare(
            "one_extra_sector",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-e{}".format(master_ecc), "-t"],
            tmp_path, image_path=tmp_iso, ecc_path=master_ecc,
        )

    # ------------------------------------------------------------------
    # ECC file manipulation tests
    # ------------------------------------------------------------------

    def test_missing_ecc_header(self, tmp_path):
        """Ecc header is missing."""
        master = self._ensure_master()
        master_ecc = self._ensure_master_ecc()
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")

        shutil.copy2(master_ecc, tmp_ecc)
        _apply_damage(tmp_ecc, [Erase("0")])

        _run_golden_compare(
            "missing_ecc_header",
            ["--regtest", "--no-progress",
             "-i{}".format(master), "-e{}".format(tmp_ecc), "-t", "-v"],
            tmp_path, image_path=master, ecc_path=tmp_ecc,
        )

    def test_missing_ecc_header_and_crc(self, tmp_path):
        """Ecc header and some CRC blocks are missing."""
        master = self._ensure_master()
        master_ecc = self._ensure_master_ecc()
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")

        shutil.copy2(master_ecc, tmp_ecc)
        _apply_damage(tmp_ecc, [Erase("0-16")])

        _run_golden_compare(
            "missing_ecc_header_and_crc",
            ["--regtest", "--no-progress",
             "-i{}".format(master), "-e{}".format(tmp_ecc), "-t", "-v"],
            tmp_path, image_path=master, ecc_path=tmp_ecc,
        )

    def test_missing_ecc_header_and_defective_crc(self, tmp_path):
        """Ecc header missing, first CRC block defective."""
        master = self._ensure_master()
        master_ecc = self._ensure_master_ecc()
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")

        shutil.copy2(master_ecc, tmp_ecc)
        _apply_damage(tmp_ecc, [Erase("0"), Byteset(2, 50, 107)])

        _run_golden_compare(
            "missing_ecc_header_and_defective_crc",
            ["--regtest", "--no-progress",
             "-i{}".format(master), "-e{}".format(tmp_ecc), "-t", "-v"],
            tmp_path, image_path=master, ecc_path=tmp_ecc,
        )

    def test_ecc_header_crc_error(self, tmp_path):
        """Checksum error in ecc header."""
        master = self._ensure_master()
        master_ecc = self._ensure_master_ecc()
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")

        shutil.copy2(master_ecc, tmp_ecc)
        _apply_damage(tmp_ecc, [Byteset(0, 32, 107)])

        _run_golden_compare(
            "ecc_header_crc_error",
            ["--regtest", "--no-progress",
             "-i{}".format(master), "-e{}".format(tmp_ecc), "-t", "-v"],
            tmp_path, image_path=master, ecc_path=tmp_ecc,
        )

    def test_ecc_file_truncated(self, tmp_path):
        """Truncated ecc file (1788 sectors)."""
        master = self._ensure_master()
        master_ecc = self._ensure_master_ecc()
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")

        # dd if=$MASTERECC of=$TMPECC bs=2048 count=1788
        with open(master_ecc, "rb") as src, open(tmp_ecc, "wb") as dst:
            dst.write(src.read(2048 * 1788))

        _run_golden_compare(
            "ecc_file_truncated",
            ["--regtest", "--no-progress",
             "-i{}".format(master), "-e{}".format(tmp_ecc), "-t"],
            tmp_path, image_path=master, ecc_path=tmp_ecc,
        )

    def test_ecc_file_plus_garbage(self, tmp_path):
        """Ecc file with trailing garbage (3980 random bytes)."""
        master = self._ensure_master()
        master_ecc = self._ensure_master_ecc()
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")

        shutil.copy2(master_ecc, tmp_ecc)
        with open(_FIXED_RANDOM_SEQ, "rb") as f:
            data = f.read(3980)
        with open(tmp_ecc, "ab") as f:
            f.write(data)

        _run_golden_compare(
            "ecc_file_plus_garbage",
            ["--regtest", "--no-progress",
             "-i{}".format(master), "-e{}".format(tmp_ecc), "-t"],
            tmp_path, image_path=master, ecc_path=tmp_ecc,
        )

    def test_ecc_file_cookieless_crc(self, tmp_path):
        """Ecc file with cookie-less CRC sector."""
        master = self._ensure_master()
        master_ecc = self._ensure_master_ecc()
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")

        shutil.copy2(master_ecc, tmp_ecc)
        _apply_damage(tmp_ecc, [Byteset(2, 1024, 70)])

        _run_golden_compare(
            "ecc_file_cookieless_crc",
            ["--regtest", "--no-progress",
             "-i{}".format(master), "-e{}".format(tmp_ecc), "-t"],
            tmp_path, image_path=master, ecc_path=tmp_ecc,
        )

    def test_ecc_file_defective_crc(self, tmp_path):
        """Ecc file with byte errors in CRC sectors."""
        master = self._ensure_master()
        master_ecc = self._ensure_master_ecc()
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")

        shutil.copy2(master_ecc, tmp_ecc)
        _apply_damage(tmp_ecc, [Byteset(4, 101, 70), Byteset(5, 908, 23)])

        _run_golden_compare(
            "ecc_file_defective_crc",
            ["--regtest", "--no-progress",
             "-i{}".format(master), "-e{}".format(tmp_ecc), "-t"],
            tmp_path, image_path=master, ecc_path=tmp_ecc,
        )

    def test_ecc_file_defective_ecc(self, tmp_path):
        """Ecc file with byte error in ECC portion."""
        master = self._ensure_master()
        master_ecc = self._ensure_master_ecc()
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")

        shutil.copy2(master_ecc, tmp_ecc)
        _apply_damage(tmp_ecc, [Byteset(1040, 101, 70)])

        _run_golden_compare(
            "ecc_file_defective_ecc",
            ["--regtest", "--no-progress",
             "-i{}".format(master), "-e{}".format(tmp_ecc), "-t"],
            tmp_path, image_path=master, ecc_path=tmp_ecc,
        )

    def test_ecc_file_missing_crc(self, tmp_path):
        """Ecc file with missing CRC sectors."""
        master = self._ensure_master()
        master_ecc = self._ensure_master_ecc()
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")

        shutil.copy2(master_ecc, tmp_ecc)
        _apply_damage(tmp_ecc, [Erase("10-19")])

        _run_golden_compare(
            "ecc_file_missing_crc",
            ["--regtest", "--no-progress",
             "-i{}".format(master), "-e{}".format(tmp_ecc), "-t"],
            tmp_path, image_path=master, ecc_path=tmp_ecc,
        )

    def test_ecc_file_missing_ecc(self, tmp_path):
        """Ecc file with missing ECC sectors."""
        master = self._ensure_master()
        master_ecc = self._ensure_master_ecc()
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")

        shutil.copy2(master_ecc, tmp_ecc)
        _apply_damage(tmp_ecc, [Erase("1000-1014")])

        _run_golden_compare(
            "ecc_file_missing_ecc",
            ["--regtest", "--no-progress",
             "-i{}".format(master), "-e{}".format(tmp_ecc), "-t"],
            tmp_path, image_path=master, ecc_path=tmp_ecc,
        )

    def test_ecc_file_missing_crc2(self, tmp_path):
        """Ecc file with missing CRC sector and CRC error in data."""
        master = self._ensure_master()
        master_ecc = self._ensure_master_ecc()
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")

        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [Byteset(91, 10, 10)])

        shutil.copy2(master_ecc, tmp_ecc)
        _apply_damage(tmp_ecc, [Erase("2")])

        _run_golden_compare(
            "ecc_file_missing_crc2",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc), "-t"],
            tmp_path, image_path=tmp_iso, ecc_path=tmp_ecc,
        )

    def test_ecc_file_missing_crc3(self, tmp_path):
        """Ecc file with corrupted CRC sector and CRC error in data."""
        master = self._ensure_master()
        master_ecc = self._ensure_master_ecc()
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")

        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [Byteset(91, 10, 10)])

        shutil.copy2(master_ecc, tmp_ecc)
        _apply_damage(tmp_ecc, [Byteset(2, 123, 97)])

        _run_golden_compare(
            "ecc_file_missing_crc3",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc), "-t"],
            tmp_path, image_path=tmp_iso, ecc_path=tmp_ecc,
        )

    def test_crc_section_with_uncorrectable_dsm(self, tmp_path):
        """CRC section with uncorrectable dead sector markers."""
        master = self._ensure_master()
        master_ecc = self._ensure_master_ecc()
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")

        shutil.copy2(master, tmp_iso)
        shutil.copy2(master_ecc, tmp_ecc)
        _apply_damage(tmp_ecc, [Erase("10"), Erase("15"), Erase("16")])

        _run_golden_compare(
            "crc_section_with_uncorrectable_dsm",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc), "-t"],
            tmp_path, image_path=tmp_iso, ecc_path=tmp_ecc,
        )

    def test_ecc_section_with_uncorrectable_dsm(self, tmp_path):
        """ECC section with uncorrectable dead sector markers."""
        master = self._ensure_master()
        master_ecc = self._ensure_master_ecc()
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")

        shutil.copy2(master, tmp_iso)
        shutil.copy2(master_ecc, tmp_ecc)
        _apply_damage(tmp_ecc, [Erase("200"), Erase("240"), Erase("241")])

        _run_golden_compare(
            "ecc_section_with_uncorrectable_dsm",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc), "-t"],
            tmp_path, image_path=tmp_iso, ecc_path=tmp_ecc,
        )


# ---------------------------------------------------------------------------
# Test Suite: Creation
# ---------------------------------------------------------------------------

# Common ignore pattern for creation tests: filter performance stats and
# method registration lines that vary between runs.
_CREATE_IGNORE_RE = r"^Avg performance|^Creating the error correction file with Method RS03"


class TestRS03fCreate(GoldenTestSuite):
    codec = "RS03"
    codec_prefix = "RS03f"
    master = "rs03f-master.iso"
    master_ecc = "rs03f-master.ecc"
    image_size = ISOSIZE
    redundancy = REDUNDANCY
    tests = []

    def _ensure_master(self):
        return _ensure_master()

    def _ensure_master_ecc(self):
        return _ensure_master_ecc()

    # 1. ecc_create -- basic ecc file creation
    def test_ecc_create(self, tmp_path):
        """Basic ecc file creation."""
        master = self._ensure_master()
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        _run_golden_compare(
            "ecc_create",
            ["--regtest", "--no-progress",
             "--debug", "--set-version", SETVERSION,
             "-i{}".format(master), "-e{}".format(tmp_ecc),
             "-mRS03", "-n{}".format(REDUNDANCY), "-o", "file", "-c"],
            tmp_path, image_path=master, ecc_path=tmp_ecc,
            ignore_line_re=_CREATE_IGNORE_RE,
        )

    # 2. ecc_missing_image -- missing image
    def test_ecc_missing_image(self, tmp_path):
        """ECC creation with missing image."""
        no_file = os.path.join(_ISODIR, "none.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        _run_golden_compare(
            "ecc_missing_image",
            ["--regtest", "--no-progress",
             "--debug", "--set-version", SETVERSION,
             "-i{}".format(no_file), "-e{}".format(tmp_ecc),
             "-mRS03", "-n", REDUNDANCY, "-o", "file", "-c"],
            tmp_path, ecc_path=tmp_ecc,
        )

    # 3. ecc_existing_file -- create over existing ecc with different redundancy
    def test_ecc_existing_file(self, tmp_path):
        """ECC creation with already existing ecc file (different redundancy)."""
        master = self._ensure_master()
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        # First create ecc with higher redundancy (REDUNDANCY_ROOTS + 10 = 30r)
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(master), "-e{}".format(tmp_ecc),
            "-mRS03", "-n{}r".format(REDUNDANCY_ROOTS + 10),
            "-o", "file", "-c",
        )
        # Then create again with original redundancy
        _run_golden_compare(
            "ecc_existing_file",
            ["--regtest", "--no-progress",
             "--debug", "--set-version", SETVERSION,
             "-i{}".format(master), "-e{}".format(tmp_ecc),
             "-mRS03", "-n", REDUNDANCY, "-o", "file", "-c"],
            tmp_path, image_path=master, ecc_path=tmp_ecc,
            ignore_line_re=_CREATE_IGNORE_RE,
        )

    # 4. ecc_no_read_perm -- no read permission on image
    @_SKIP_CHMOD_WIN
    def test_ecc_no_read_perm(self, tmp_path):
        """ECC creation with no read permission on image."""
        master = self._ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, tmp_iso)
        os.chmod(tmp_iso, 0o000)
        try:
            _run_golden_compare(
                "ecc_no_read_perm",
                ["--regtest", "--no-progress",
                 "--debug", "--set-version", SETVERSION,
                 "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
                 "-mRS03", "-n", REDUNDANCY, "-o", "file", "-c"],
                tmp_path, image_path=tmp_iso, ecc_path=tmp_ecc,
            )
        finally:
            os.chmod(tmp_iso, 0o644)

    # 5. ecc_no_write_perm -- no write permission on ecc file
    @_SKIP_CHMOD_WIN
    def test_ecc_no_write_perm(self, tmp_path):
        """ECC creation with no write permission on ecc file."""
        master = self._ensure_master()
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        # Create ecc file with no permissions
        with open(tmp_ecc, "w"):
            pass
        os.chmod(tmp_ecc, 0o000)
        try:
            _run_golden_compare(
                "ecc_no_write_perm",
                ["--regtest", "--no-progress",
                 "--debug", "--set-version", SETVERSION,
                 "-i{}".format(master), "-e{}".format(tmp_ecc),
                 "-mRS03", "-n", REDUNDANCY, "-o", "file", "-c"],
                tmp_path, image_path=master, ecc_path=tmp_ecc,
                ignore_line_re=_CREATE_IGNORE_RE,
            )
        finally:
            if os.path.exists(tmp_ecc):
                os.chmod(tmp_ecc, 0o644)

    # 6. ecc_create_plus56 -- image with 56 extra bytes
    def test_ecc_create_plus56(self, tmp_path):
        """ECC creation for image with 56 additional bytes."""
        master = self._ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, tmp_iso)
        with open(_FIXED_RANDOM_SEQ, "rb") as f:
            data = f.read(56)
        with open(tmp_iso, "ab") as f:
            f.write(data)
        _run_golden_compare(
            "ecc_create_plus56",
            ["--regtest", "--no-progress",
             "--debug", "--set-version", SETVERSION,
             "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
             "-mRS03", "-n", REDUNDANCY, "-o", "file", "-c"],
            tmp_path, image_path=tmp_iso, ecc_path=tmp_ecc,
            ignore_line_re=_CREATE_IGNORE_RE,
        )

    # 7. ecc_missing_sectors -- image with erased sectors 500-524
    def test_ecc_missing_sectors(self, tmp_path):
        """ECC creation from image with missing sectors."""
        master = self._ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, tmp_iso)
        _run_dvdisaster("--debug", "-i", tmp_iso, "--erase", "500-524")
        _run_golden_compare(
            "ecc_missing_sectors",
            ["--regtest", "--no-progress",
             "--debug", "--set-version", SETVERSION,
             "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
             "-mRS03", "-n", REDUNDANCY, "-o", "file", "-c"],
            tmp_path, image_path=tmp_iso, ecc_path=tmp_ecc,
            ignore_line_re=_CREATE_IGNORE_RE,
        )

    # 8. ecc_create_after_read -- read + create in one call
    def test_ecc_create_after_read(self, tmp_path):
        """Read image and create ecc in one call."""
        master = self._ensure_master()
        sim_iso = os.path.join(str(tmp_path), "rs03f-sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, sim_iso)
        _run_golden_compare(
            "ecc_create_after_read",
            ["--regtest", "--no-progress",
             "--debug", "--set-version", SETVERSION,
             "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
             "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
             "-r", "-c", "-mRS03", "-o", "file",
             "-n{}".format(REDUNDANCY), "-v"],
            tmp_path, image_path=tmp_iso, ecc_path=tmp_ecc,
            ignore_line_re=_CREATE_IGNORE_RE,
        )

    # 9. ecc_recreate_after_read_rs01 -- read with RS01 ecc, create RS03f ecc
    def test_ecc_recreate_after_read_rs01(self, tmp_path):
        """Read image with RS01 ECC and create RS03f ECC."""
        master = self._ensure_master()
        sim_iso = os.path.join(str(tmp_path), "rs03f-sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, sim_iso)
        # Create RS01 ecc (8r) for the sim image
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(sim_iso), "-e{}".format(tmp_ecc),
            "-c", "-n", "8r",
        )
        _run_golden_compare(
            "ecc_recreate_after_read_rs01",
            ["--regtest", "--no-progress",
             "--debug", "--set-version", SETVERSION,
             "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
             "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
             "-r", "-c", "-mRS03", "-o", "file",
             "-n{}".format(REDUNDANCY), "-v"],
            tmp_path, image_path=tmp_iso, ecc_path=tmp_ecc,
            ignore_line_re=_CREATE_IGNORE_RE,
        )

    # 10. ecc_recreate_after_read_rs02 -- read RS02-augmented image, create RS03f ecc
    def test_ecc_recreate_after_read_rs02(self, tmp_path):
        """Read image with RS02 ECC and create RS03f ECC."""
        master = self._ensure_master()
        sim_iso = os.path.join(str(tmp_path), "rs03f-sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, sim_iso)
        # Augment with RS02 (n = ISOSIZE + 3000 = 24000)
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(sim_iso), "-mRS02",
            "-c", "-n{}".format(ISOSIZE + 3000),
        )
        _run_golden_compare(
            "ecc_recreate_after_read_rs02",
            ["--regtest", "--no-progress",
             "--debug", "--set-version", SETVERSION,
             "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
             "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
             "-r", "-c", "-mRS03", "-o", "file",
             "-n{}".format(REDUNDANCY), "-v"],
            tmp_path, image_path=tmp_iso, ecc_path=tmp_ecc,
            ignore_line_re=_CREATE_IGNORE_RE,
        )

    # 11. ecc_recreate_after_read_rs03i -- read RS03 image-augmented, create RS03f ecc
    def test_ecc_recreate_after_read_rs03i(self, tmp_path):
        """Read image with RS03i ECC and create RS03f ECC."""
        master = self._ensure_master()
        sim_iso = os.path.join(str(tmp_path), "rs03f-sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, sim_iso)
        # Augment with RS03 image mode (n = ISOSIZE + 3000 = 24000)
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(sim_iso), "-mRS03",
            "-c", "-n{}".format(ISOSIZE + 3000),
        )
        _run_golden_compare(
            "ecc_recreate_after_read_rs03i",
            ["--regtest", "--no-progress",
             "--debug", "--set-version", SETVERSION,
             "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
             "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
             "-r", "-c", "-mRS03", "-o", "file",
             "-n{}".format(REDUNDANCY), "-v"],
            tmp_path, image_path=tmp_iso, ecc_path=tmp_ecc,
            ignore_line_re=_CREATE_IGNORE_RE,
        )

    # 12. ecc_recreate_after_read_rs03f -- read with RS03f ecc (9r), create new (20r)
    def test_ecc_recreate_after_read_rs03f(self, tmp_path):
        """Read image with RS03f ECC and create new RS03f ECC."""
        master = self._ensure_master()
        sim_iso = os.path.join(str(tmp_path), "rs03f-sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, sim_iso)
        # Create RS03f ecc with 9r redundancy
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(sim_iso), "-e{}".format(tmp_ecc),
            "-mRS03", "-o", "file", "-c", "-n", "9r",
        )
        _run_golden_compare(
            "ecc_recreate_after_read_rs03f",
            ["--regtest", "--no-progress",
             "--debug", "--set-version", SETVERSION,
             "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
             "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
             "-r", "-c", "-mRS03", "-o", "file",
             "-n{}".format(REDUNDANCY), "-v"],
            tmp_path, image_path=tmp_iso, ecc_path=tmp_ecc,
            ignore_line_re=_CREATE_IGNORE_RE,
        )

    # 13. ecc_create_after_partial_read -- complete partial image then create ecc
    def test_ecc_create_after_partial_read(self, tmp_path):
        """Create ecc after completing partial image in reading pass."""
        master = self._ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, tmp_iso)
        # Erase sectors 1000-1500
        _run_dvdisaster("--debug", "-i{}".format(tmp_iso),
                        "--erase", "1000-1500")
        _run_golden_compare(
            "ecc_create_after_partial_read",
            ["--regtest", "--no-progress",
             "--debug", "--set-version", SETVERSION,
             "--sim-cd={}".format(master), "--fixed-speed-values",
             "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
             "-r", "-c", "-mRS03", "-o", "file",
             "-n{}".format(REDUNDANCY), "-v"],
            tmp_path, image_path=tmp_iso, ecc_path=tmp_ecc,
            ignore_line_re=_CREATE_IGNORE_RE,
        )


# ---------------------------------------------------------------------------
# Test Suite: Repair
# ---------------------------------------------------------------------------

class TestRS03fRepair(GoldenTestSuite):
    codec = "RS03"
    codec_prefix = "RS03f"
    master = "rs03f-master.iso"
    master_ecc = "rs03f-master.ecc"
    image_size = ISOSIZE
    redundancy = REDUNDANCY

    def _ensure_master(self):
        return _ensure_master()

    def _ensure_master_ecc(self):
        return _ensure_master_ecc()

    # ------------------------------------------------------------------
    # Declarative tests
    # ------------------------------------------------------------------
    tests = [
        # fix_good: fix good image (no damage)
        GoldenTest("fix_good", action="-f", ecc="master_ecc"),
        # fix_missing_data_sectors: erase sectors in image
        GoldenTest("fix_missing_data_sectors", action="-f",
                   damage=[Erase("900-924"), Erase("73")],
                   ecc="master_ecc"),
        # fix_missing_crc_sectors: erase CRC sectors in ecc
        GoldenTest("fix_missing_crc_sectors", action="-f",
                   ecc="master_ecc",
                   ecc_damage=[Erase("5-9")]),
        # fix_missing_ecc_sectors: erase ECC sectors in ecc
        GoldenTest("fix_missing_ecc_sectors", action="-f",
                   ecc="master_ecc",
                   ecc_damage=[Erase("115-119")]),
        # fix_border_cases_erasures: erase sectors across layers in both image and ecc
        GoldenTest("fix_border_cases_erasures", action="-f",
                   damage=[
                       Erase("0"), Erase("90"), Erase("180"), Erase("20970"),
                       Erase("89"), Erase("179"), Erase("269"), Erase("20999"),
                   ],
                   ecc="master_ecc",
                   ecc_damage=[
                       Erase("2"), Erase("92"), Erase("182"), Erase("1802"),
                       Erase("91"), Erase("181"), Erase("271"), Erase("1891"),
                   ]),
        # fix_border_cases_crc_errors: byteset across layers in both image and ecc
        GoldenTest("fix_border_cases_crc_errors", action="-f",
                   damage=[
                       Byteset(0, 0, 1), Byteset(90, 0, 0), Byteset(180, 0, 0),
                       Byteset(20970, 0, 0),
                       Byteset(89, 0, 0), Byteset(179, 0, 0), Byteset(269, 0, 0),
                       Byteset(20999, 0, 0),
                   ],
                   ecc="master_ecc",
                   ecc_damage=[
                       Byteset(2, 0, 0), Byteset(92, 0, 0), Byteset(182, 0, 0),
                       Byteset(1802, 0, 0),
                       Byteset(91, 0, 0), Byteset(181, 0, 0), Byteset(271, 0, 0),
                       Byteset(1891, 0, 0),
                   ]),
        # fix_no_read_perm: no read permission on image
        GoldenTest("fix_no_read_perm", action="-f",
                   chmod_image=0o000, ecc="master_ecc"),
        # fix_no_write_perm: no write permission on image
        GoldenTest("fix_no_write_perm", action="-f",
                   chmod_image=0o400, ecc="master_ecc"),
        # fix_additional_sector: image with 1 extra sector (TAO case)
        GoldenTest("fix_additional_sector", action="-f",
                   damage=[PadSectors(1)], ecc="master_ecc"),
        # fix_plus17: image with 17 additional sectors
        GoldenTest("fix_plus17", action="-f",
                   damage=[PadSectors(17)], ecc="master_ecc"),
        # fix_plus17_truncate: with --truncate
        GoldenTest("fix_plus17_truncate", action="-f --truncate",
                   damage=[PadSectors(17)], ecc="master_ecc"),
        # fix_truncated: truncated image
        GoldenTest("fix_truncated", action="-f",
                   damage=[Truncate(ISOSIZE - 269)], ecc="master_ecc"),
        # fix_ecc_file_truncated: truncated ecc file (uses master image directly)
        GoldenTest("fix_ecc_file_truncated", action="-f",
                   use_master=True,
                   ecc="master_ecc",
                   ecc_damage=[Truncate(1788)]),
        # fix_missing_ecc_header: erase sector 0 of ecc (uses master image directly)
        GoldenTest("fix_missing_ecc_header", action="-f -v",
                   use_master=True,
                   ecc="master_ecc",
                   ecc_damage=[Erase("0")]),
    ]

    # ------------------------------------------------------------------
    # Permission tests on ecc requiring manual copy + chmod
    # ------------------------------------------------------------------

    def test_fix_no_read_perm_ecc(self, tmp_path):
        """Fix image without read permission on ecc."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, tmp_iso)
        shutil.copy2(master_ecc, tmp_ecc)
        os.chmod(tmp_ecc, 0o000)
        try:
            cmd = [
                "--regtest", "--no-progress",
                "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
                "-f",
            ]
            _run_golden_compare("fix_no_read_perm_ecc", cmd, tmp_path,
                                image_path=tmp_iso, ecc_path=tmp_ecc)
        finally:
            os.chmod(tmp_ecc, 0o644)

    def test_fix_no_write_perm_ecc(self, tmp_path):
        """Fix image without write permission for ecc."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, tmp_iso)
        shutil.copy2(master_ecc, tmp_ecc)
        os.chmod(tmp_ecc, 0o400)
        try:
            cmd = [
                "--regtest", "--no-progress",
                "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
                "-f",
            ]
            _run_golden_compare("fix_no_write_perm_ecc", cmd, tmp_path,
                                image_path=tmp_iso, ecc_path=tmp_ecc)
        finally:
            os.chmod(tmp_ecc, 0o644)

    # ------------------------------------------------------------------
    # Plus56 repair tests (require plus56_images fixture)
    # ------------------------------------------------------------------

    def test_fix_good_plus56(self, plus56_images, tmp_path):
        """Fix good image not multiple of 2048."""
        iso_plus56, ecc_plus56 = plus56_images
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(iso_plus56, tmp_iso)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(ecc_plus56),
            "-f",
        ]
        _run_golden_compare("fix_good_plus56", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=ecc_plus56)

    def test_fix_plus56(self, plus56_images, tmp_path):
        """Fix image with CRC error in 56 additional bytes."""
        iso_plus56, ecc_plus56 = plus56_images
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(iso_plus56, tmp_iso)
        _apply_damage(tmp_iso, [Byteset(ISOSIZE, 28, 90)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(ecc_plus56),
            "-f",
        ]
        _run_golden_compare("fix_plus56", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=ecc_plus56)

    def test_fix_plus56_plus17(self, plus56_images, tmp_path):
        """Fix image with CRC error in 56 additional bytes + few bytes more."""
        iso_plus56, ecc_plus56 = plus56_images
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(iso_plus56, tmp_iso)
        with open(tmp_iso, "ab") as f:
            f.write(b"0123456789abcdef\n")
        _apply_damage(tmp_iso, [Byteset(ISOSIZE, 55, 90)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(ecc_plus56),
            "-f",
        ]
        _run_golden_compare("fix_plus56_plus17", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=ecc_plus56)

    def test_fix_plus56_plus17_truncate(self, plus56_images, tmp_path):
        """Fix image with CRC error in 56 additional bytes + few bytes more w/ truncate."""
        iso_plus56, ecc_plus56 = plus56_images
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(iso_plus56, tmp_iso)
        with open(tmp_iso, "ab") as f:
            f.write(b"0123456789abcdef\n")
        _apply_damage(tmp_iso, [Byteset(ISOSIZE, 55, 90)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(ecc_plus56),
            "-f", "--truncate",
        ]
        _run_golden_compare("fix_plus56_plus17_truncate", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=ecc_plus56)

    def test_fix_plus56_plus1s(self, plus56_images, tmp_path):
        """Fix image with CRC error in 56 additional bytes + one sector more."""
        iso_plus56, ecc_plus56 = plus56_images
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(iso_plus56, tmp_iso)
        with open(_FIXED_RANDOM_SEQ, "rb") as src:
            data = src.read(2048)
        with open(tmp_iso, "ab") as f:
            f.write(data)
        _apply_damage(tmp_iso, [Byteset(21000, 55, 90)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(ecc_plus56),
            "-f", "--truncate",
        ]
        _run_golden_compare("fix_plus56_plus1s", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=ecc_plus56)

    def test_fix_plus56_plus2s(self, plus56_images, tmp_path):
        """Fix image with CRC error in 56 additional bytes + two sectors more."""
        iso_plus56, ecc_plus56 = plus56_images
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(iso_plus56, tmp_iso)
        with open(_FIXED_RANDOM_SEQ, "rb") as src:
            data = src.read(4096)
        with open(tmp_iso, "ab") as f:
            f.write(data)
        _apply_damage(tmp_iso, [Byteset(21000, 55, 90)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(ecc_plus56),
            "-f", "--truncate",
        ]
        _run_golden_compare("fix_plus56_plus2s", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=ecc_plus56)

    def test_fix_plus56_plus17500(self, plus56_images, tmp_path):
        """Fix image with CRC error in 56 additional bytes + more sectors."""
        iso_plus56, ecc_plus56 = plus56_images
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(iso_plus56, tmp_iso)
        with open(tmp_iso, "ab") as f:
            f.write(b"\x00" * 17500)
        _apply_damage(tmp_iso, [Byteset(21000, 55, 90)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(ecc_plus56),
            "-f", "--truncate",
        ]
        _run_golden_compare("fix_plus56_plus17500", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=ecc_plus56)

    def test_fix_plus56_truncated(self, plus56_images, tmp_path):
        """Fix truncated image not a multiple of 2048."""
        iso_plus56, ecc_plus56 = plus56_images
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(iso_plus56, tmp_iso)
        _apply_damage(tmp_iso, [Truncate(ISOSIZE - 28)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(ecc_plus56),
            "-f",
        ]
        _run_golden_compare("fix_plus56_truncated", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=ecc_plus56)

    def test_fix_plus56_little_truncated(self, plus56_images, tmp_path):
        """Fix image not a multiple of 2048 missing a few bytes."""
        master = _ensure_master()
        _, ecc_plus56 = plus56_images
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(master, tmp_iso)
        with open(_FIXED_RANDOM_SEQ, "rb") as src:
            data = src.read(50)
        with open(tmp_iso, "ab") as f:
            f.write(data)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(ecc_plus56),
            "-f",
        ]
        _run_golden_compare("fix_plus56_little_truncated", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=ecc_plus56)


# ---------------------------------------------------------------------------
# Helper: append fixed-random-sequence
# ---------------------------------------------------------------------------

def _append_fixed_random_sequence(path, times=1):
    """Append the fixed-random-sequence file content to path, `times` times."""
    with open(_FIXED_RANDOM_SEQ, "rb") as f:
        seq_data = f.read()
    with open(path, "ab") as f:
        for _ in range(times):
            f.write(seq_data)


# ---------------------------------------------------------------------------
# Test Suite: Scanning
# ---------------------------------------------------------------------------

class TestRS03fScan:
    """RS03f scanning tests migrated from regtest/rs03f.bash lines 1011-1253."""

    def test_scan_good(self, tmp_path):
        """Scan complete / optimal image."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(master, sim_iso)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_good", cmd, tmp_path)

    def test_scan_good_verbose(self, tmp_path):
        """Scan complete / optimal image, verbose output."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(master, sim_iso)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s", "-v",
        ]
        _run_golden_compare("scan_good_verbose", cmd, tmp_path)

    def test_scan_shorter(self, tmp_path):
        """Scan image which is shorter than expected."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Truncate(ISOSIZE - 44)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_shorter", cmd, tmp_path)

    def test_scan_longer(self, tmp_path):
        """Scan image which is longer than expected."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(master, sim_iso)
        _append_fixed_random_sequence(sim_iso, 23)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s", "-v",
        ]
        _run_golden_compare("scan_longer", cmd, tmp_path)

    def test_scan_tao_tail(self, tmp_path):
        """Scan image with two multisession link sectors appended."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(master, sim_iso)
        _append_fixed_random_sequence(sim_iso, 1)
        _apply_damage(sim_iso, [Erase("21000-21001")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_tao_tail", cmd, tmp_path)

    def test_scan_no_tao_tail(self, tmp_path):
        """Scan image with two real sectors missing at the end."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(master, sim_iso)
        _append_fixed_random_sequence(sim_iso, 1)
        _apply_damage(sim_iso, [Erase("20998-20999")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s", "--dao",
        ]
        _run_golden_compare("scan_no_tao_tail", cmd, tmp_path)

    def test_scan_incompatible_ecc(self, tmp_path):
        """Scan image requiring a newer dvdisaster version."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, sim_iso)
        shutil.copy2(master_ecc, tmp_ecc)
        # Creator version 99.99 + version info 99.99 + patched selfcrc
        for sector, offset, val in [
            (0, 84, 220), (0, 85, 65), (0, 86, 15),
            (0, 88, 220), (0, 89, 65), (0, 90, 15),
            (0, 96, 123), (0, 97, 99), (0, 98, 62), (0, 99, 9),
        ]:
            _run_dvdisaster("--debug", "-i{}".format(tmp_ecc),
                            "--byteset", "{},{},{}".format(sector, offset, val))
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_incompatible_ecc", cmd, tmp_path,
                            ignore_line_re=r'^\*          $')

    def test_scan_bad_header(self, tmp_path):
        """Scan image with a defective ECC header."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, sim_iso)
        shutil.copy2(_ensure_master_ecc(), tmp_ecc)
        _apply_damage(tmp_ecc, [Byteset(0, 1, 1)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s", "-v",
        ]
        _run_golden_compare("scan_bad_header", cmd, tmp_path)

    def test_scan_missing_data_sectors(self, tmp_path):
        """Scan image with missing data sectors."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [
            Erase("1000-1049"), Erase("11230"), Erase("12450-12457"),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_missing_data_sectors", cmd, tmp_path)

    def test_scan_missing_crc_sectors(self, tmp_path):
        """Scan image with missing CRC sectors in ecc file."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, sim_iso)
        shutil.copy2(_ensure_master_ecc(), tmp_ecc)
        _apply_damage(tmp_ecc, [Erase("5"), Erase("77-86")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_missing_crc_sectors", cmd, tmp_path)

    def test_scan_missing_ecc_sectors(self, tmp_path):
        """Scan image with missing ECC sectors in ecc file."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, sim_iso)
        shutil.copy2(_ensure_master_ecc(), tmp_ecc)
        _apply_damage(tmp_ecc, [Erase("120"), Erase("134-190")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_missing_ecc_sectors", cmd, tmp_path)

    def test_scan_data_bad_byte(self, tmp_path):
        """Scan image with bad byte in the data section."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Byteset(1235, 50, 10)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_data_bad_byte", cmd, tmp_path)

    def test_scan_crc_bad_byte(self, tmp_path):
        """Scan image with bad byte in the CRC section of ecc file."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, sim_iso)
        shutil.copy2(_ensure_master_ecc(), tmp_ecc)
        _apply_damage(tmp_ecc, [Byteset(77, 50, 10)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_crc_bad_byte", cmd, tmp_path)

    def test_scan_ecc_bad_byte(self, tmp_path):
        """Scan image with bad byte in the ECC section of ecc file."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, sim_iso)
        shutil.copy2(_ensure_master_ecc(), tmp_ecc)
        _apply_damage(tmp_ecc, [Byteset(200, 50, 10)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_ecc_bad_byte", cmd, tmp_path)

    def test_scan_missing_ecc_header(self, tmp_path):
        """Scan image with missing ecc header."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, sim_iso)
        shutil.copy2(_ensure_master_ecc(), tmp_ecc)
        _apply_damage(tmp_ecc, [Erase("0")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(master), "-e{}".format(tmp_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s", "-v",
        ]
        _run_golden_compare("scan_missing_ecc_header", cmd, tmp_path)

    def test_scan_missing_ecc_header_and_crc(self, tmp_path):
        """Scan image with missing ecc header and CRC blocks."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, sim_iso)
        shutil.copy2(_ensure_master_ecc(), tmp_ecc)
        _apply_damage(tmp_ecc, [Erase("0-16")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(master), "-e{}".format(tmp_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s", "-v",
        ]
        _run_golden_compare("scan_missing_ecc_header_and_crc", cmd, tmp_path)

    def test_scan_missing_ecc_header_and_defective_crc(self, tmp_path):
        """Scan image with ecc header missing, first CRC block defective."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, sim_iso)
        shutil.copy2(_ensure_master_ecc(), tmp_ecc)
        _apply_damage(tmp_ecc, [Erase("0"), Byteset(2, 50, 107)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(master), "-e{}".format(tmp_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s", "-v",
        ]
        _run_golden_compare("scan_missing_ecc_header_and_defective_crc", cmd, tmp_path)

    def test_scan_ecc_header_crc_error(self, tmp_path):
        """Checksum error in ecc header."""
        master = _ensure_master()
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(_ensure_master_ecc(), tmp_ecc)
        _apply_damage(tmp_ecc, [Byteset(0, 32, 107)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(master), "-e{}".format(tmp_ecc),
            "--debug", "--sim-cd={}".format(master), "--fixed-speed-values",
            "--spinup-delay=0", "-s", "-v",
        ]
        _run_golden_compare("scan_ecc_header_crc_error", cmd, tmp_path)


# ---------------------------------------------------------------------------
# Test Suite: Reading (linear)
# ---------------------------------------------------------------------------

class TestRS03fReadLinear:
    """RS03f linear reading tests migrated from regtest/rs03f.bash lines 1255-1491."""

    def test_read_good(self, tmp_path):
        """Read complete / optimal image."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(master, sim_iso)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_good", cmd, tmp_path, image_path=tmp_iso,
                            ecc_path=master_ecc)

    def test_read_good_verbose(self, tmp_path):
        """Read complete / optimal image, verbose output."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(master, sim_iso)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "-v",
        ]
        _run_golden_compare("read_good_verbose", cmd, tmp_path, image_path=tmp_iso)

    def test_read_good_file(self, tmp_path):
        """Read good image into a pre-existing image file."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(master, sim_iso)
        shutil.copy2(master, tmp_iso)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_good_file", cmd, tmp_path, image_path=tmp_iso)

    def test_read_shorter(self, tmp_path):
        """Read image which is shorter than expected."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Truncate(ISOSIZE - 44)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--ignore-iso-size", "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_shorter", cmd, tmp_path, image_path=tmp_iso)

    def test_read_longer(self, tmp_path):
        """Read image which is longer than expected (returns image in original length)."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(master, sim_iso)
        _append_fixed_random_sequence(sim_iso, 23)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "-v",
        ]
        _run_golden_compare("read_longer", cmd, tmp_path, image_path=tmp_iso)

    def test_read_tao_tail(self, tmp_path):
        """Read image with TAO tail (two link sectors appended, then erased)."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(master, sim_iso)
        _append_fixed_random_sequence(sim_iso, 1)
        _apply_damage(sim_iso, [Erase("21000-21001")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_tao_tail", cmd, tmp_path, image_path=tmp_iso)

    def test_read_no_tao_tail(self, tmp_path):
        """Read image with two real missing sectors at the end (--dao prevents clipping)."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(master, sim_iso)
        _append_fixed_random_sequence(sim_iso, 1)
        _apply_damage(sim_iso, [Erase("20998-20999")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "--dao",
        ]
        _run_golden_compare("read_no_tao_tail", cmd, tmp_path, image_path=tmp_iso)

    def test_read_incompatible_ecc(self, tmp_path):
        """Read image with ECC file requiring a newer dvdisaster version."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, sim_iso)
        shutil.copy2(_ensure_master_ecc(), tmp_ecc)
        # Patch creator version to 99.99 and update selfcrc
        _apply_damage(tmp_ecc, [
            Byteset(0, 84, 220), Byteset(0, 85, 65), Byteset(0, 86, 15),
            Byteset(0, 88, 220), Byteset(0, 89, 65), Byteset(0, 90, 15),
            Byteset(0, 96, 123), Byteset(0, 97, 99), Byteset(0, 98, 62),
            Byteset(0, 99, 9),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_incompatible_ecc", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=tmp_ecc,
                            ignore_line_re=r'^\*          $')

    def test_read_bad_header(self, tmp_path):
        """Read image with a defective ECC header."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, sim_iso)
        shutil.copy2(_ensure_master_ecc(), tmp_ecc)
        _apply_damage(tmp_ecc, [Byteset(0, 1, 1)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "-v",
        ]
        _run_golden_compare("read_bad_header", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=tmp_ecc)

    def test_read_missing_data_sectors(self, tmp_path):
        """Read image with missing data sectors."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [
            Erase("1000-1049"), Erase("11230"), Erase("12450-12457"),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_missing_data_sectors", cmd, tmp_path, image_path=tmp_iso)

    def test_read_missing_crc_sectors(self, tmp_path):
        """Read image with missing CRC sectors in ecc file."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, sim_iso)
        shutil.copy2(_ensure_master_ecc(), tmp_ecc)
        _apply_damage(tmp_ecc, [Erase("5"), Erase("77-86")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_missing_crc_sectors", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=tmp_ecc)

    def test_read_missing_ecc_sectors(self, tmp_path):
        """Read image with missing ECC sectors in ecc file."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, sim_iso)
        shutil.copy2(_ensure_master_ecc(), tmp_ecc)
        _apply_damage(tmp_ecc, [Erase("120"), Erase("134-190")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_missing_ecc_sectors", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=tmp_ecc)

    def test_read_data_bad_byte(self, tmp_path):
        """Read image with bad bytes in the data section."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [
            Byteset(0, 50, 10), Byteset(1235, 50, 10), Byteset(20999, 50, 10),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_data_bad_byte", cmd, tmp_path, image_path=tmp_iso)

    def test_read_crc_bad_byte(self, tmp_path):
        """Read image with a bad byte in the CRC section of ecc file."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, sim_iso)
        shutil.copy2(_ensure_master_ecc(), tmp_ecc)
        _apply_damage(tmp_ecc, [Byteset(77, 50, 10)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_crc_bad_byte", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=tmp_ecc)

    def test_read_ecc_bad_byte(self, tmp_path):
        """Read image with a bad byte in the ECC section of ecc file."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, sim_iso)
        shutil.copy2(_ensure_master_ecc(), tmp_ecc)
        _apply_damage(tmp_ecc, [Byteset(200, 50, 10)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_ecc_bad_byte", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=tmp_ecc)

    def test_read_crc_section_with_uncorrectable_dsm(self, tmp_path):
        """Read image with uncorrectable dead sector markers in CRC area of ecc file."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03f-tmp.ecc")
        shutil.copy2(master, sim_iso)
        shutil.copy2(_ensure_master_ecc(), tmp_ecc)
        _apply_damage(tmp_ecc, [Erase("10"), Erase("15"), Erase("20")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_crc_section_with_uncorrectable_dsm", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=tmp_ecc)

    def test_read_second_pass_with_crc_error(self, tmp_path):
        """Re-read medium with CRC error using a pre-existing partial image."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        # Prepare sim_iso with a bad byte
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Byteset(15830, 8, 3)])
        # Prepare tmp_iso as a partial image (pre-existing, with some sectors erased)
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [Erase("15800-16199")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_second_pass_with_crc_error", cmd, tmp_path,
                            image_path=tmp_iso)


# ---------------------------------------------------------------------------
# Test Suite: Reading (adaptive)
# ---------------------------------------------------------------------------

class TestRS03fReadAdaptive:
    """RS03f adaptive reading tests migrated from regtest/rs03f.bash lines 1516-1531."""

    def test_adaptive_good(self, tmp_path):
        """Adaptive read of a good image with RS03 ecc file."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03f-tmp.iso")
        shutil.copy2(master, sim_iso)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "--adaptive-read",
        ]
        _run_golden_compare("adaptive_good", cmd, tmp_path, image_path=tmp_iso,
                            ecc_path=master_ecc)
