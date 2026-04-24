"""
RS03i regression tests -- from regtest/rs03i.bash.

RS03i is the image-embedded ECC mode: ECC data is embedded directly in the image
(like RS02), but uses the RS03 codec with configurable redundancy.

Tests are grouped into:
  - TestRS03iStrip: 2 strip tests
  - TestRS03iVerify: 48 verify tests
  - TestRS03iCreate: 20 creation tests
  - TestRS03iFix: 27 fixing tests
  - TestRS03iScan: 33 scanning tests
  - TestRS03iReadLinear: 29 linear reading tests
"""

import difflib
import os
import re
import shutil

import pytest

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
ECCSIZE = 25000
REAL_ECCSIZE = 24990
SETVERSION = "0.80"
CUSTOM_ECCSIZE = 28000

# Large master image constants
LMI_HEADER = 235219
LMI_LAYER_SIZE = 1409
LMI_FIRSTCRC = 235303


# ---------------------------------------------------------------------------
# Helper: ensure images exist (idempotent)
# ---------------------------------------------------------------------------

def _ensure_raw_image():
    """Create the raw (unaugmented) 21000-sector image in ISODIR."""
    os.makedirs(_ISODIR, exist_ok=True)
    raw_path = os.path.join(_ISODIR, "rs03i-raw.iso")
    if os.path.isfile(raw_path):
        return raw_path
    _run_dvdisaster(
        "--regtest", "--debug",
        "-i{}".format(raw_path),
        "--random-image", str(ISOSIZE),
        check=True,
    )
    return raw_path


def _ensure_raw_lmi246():
    """Create raw image for LMI246 tests (LMI_LAYER_SIZE*246-2 sectors)."""
    os.makedirs(_ISODIR, exist_ok=True)
    path = os.path.join(_ISODIR, "rs03i-raw-lmi246.iso")
    if os.path.isfile(path):
        return path
    sectors = LMI_LAYER_SIZE * 246 - 2  # 346612
    _run_dvdisaster(
        "--debug",
        "-i{}".format(path),
        "--random-image", str(sectors),
        check=True,
    )
    return path


def _ensure_raw_lmi84():
    """Create raw image for LMI84 tests (LMI_LAYER_SIZE*84-2 sectors)."""
    os.makedirs(_ISODIR, exist_ok=True)
    path = os.path.join(_ISODIR, "rs03i-raw-lmi84.iso")
    if os.path.isfile(path):
        return path
    sectors = LMI_LAYER_SIZE * 84 - 2  # 118354
    _run_dvdisaster(
        "--debug",
        "-i{}".format(path),
        "--random-image", str(sectors),
        check=True,
    )
    return path


def _ensure_raw_lmi84s():
    """Create raw image for LMI84S tests (LMI_LAYER_SIZE*84-2-6000 sectors)."""
    os.makedirs(_ISODIR, exist_ok=True)
    path = os.path.join(_ISODIR, "rs03i-raw-lmi84s.iso")
    if os.path.isfile(path):
        return path
    sectors = LMI_LAYER_SIZE * 84 - 2 - 6000  # 112354
    _run_dvdisaster(
        "--debug",
        "-i{}".format(path),
        "--random-image", str(sectors),
        check=True,
    )
    return path


def _ensure_master():
    """Create the RS03i augmented master image (raw + RS03 ECC embedded)."""
    os.makedirs(_ISODIR, exist_ok=True)
    master_path = os.path.join(_ISODIR, "rs03i-master.iso")
    if os.path.isfile(master_path):
        return master_path
    raw_path = _ensure_raw_image()
    shutil.copy2(raw_path, master_path)
    _run_dvdisaster(
        "--regtest", "--debug", "--set-version", SETVERSION,
        "-i{}".format(master_path),
        "-mRS03", "-n{}".format(ECCSIZE), "-c",
        check=True,
    )
    return master_path


def _ensure_large_master():
    """Create the large RS03i master image (235219 sectors + RS03 ECC)."""
    os.makedirs(_ISODIR, exist_ok=True)
    path = os.path.join(_ISODIR, "rs03i-large.iso")
    if os.path.isfile(path):
        return path
    _run_dvdisaster(
        "--regtest", "--debug",
        "-i{}".format(path),
        "--random-image", "235219",
        check=True,
    )
    _run_dvdisaster(
        "--regtest", "--debug", "--set-version", SETVERSION,
        "-i{}".format(path),
        "-mRS03", "-c",
        check=True,
    )
    return path


def _ensure_custom_master():
    """Create the RS03i custom-size master image (raw + RS03 -n28000)."""
    os.makedirs(_ISODIR, exist_ok=True)
    path = os.path.join(_ISODIR, "rs03i-custom-master.iso")
    if os.path.isfile(path):
        return path
    raw_path = _ensure_raw_image()
    shutil.copy2(raw_path, path)
    _run_dvdisaster(
        "--regtest", "--debug", "--set-version", SETVERSION,
        "-i{}".format(path),
        "-mRS03", "-n{}".format(CUSTOM_ECCSIZE), "-c",
        check=True,
    )
    return path


# ---------------------------------------------------------------------------
# Helper: append fixed-random-sequence to a file
# ---------------------------------------------------------------------------

def _append_fixed_random_sequence(path, times=1):
    """Append the fixed-random-sequence file to path the given number of times."""
    with open(_FIXED_RANDOM_SEQ, "rb") as f:
        seq_data = f.read()
    with open(path, "ab") as f:
        for _ in range(times):
            f.write(seq_data)


# ---------------------------------------------------------------------------
# Helper: golden file comparison
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
    golden_base = os.path.join(_DATABASE, "RS03i_{}".format(test_name))
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


# ---------------------------------------------------------------------------
# Common damage patterns
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


# ===========================================================================
# Strip tests (2)
# ===========================================================================

class TestRS03iStrip(GoldenTestSuite):
    codec = "RS03"
    codec_prefix = "RS03i"
    master = "rs03i-master.iso"
    image_size = ISOSIZE
    tests = []

    def _ensure_master(self):
        return _ensure_master()

    def test_strip_ecc(self, tmp_path):
        """Strip ECC from an augmented RS03i image."""
        master_path = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master_path, tmp_iso)
        _run_golden_compare(
            "strip_ecc",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-v", "--strip"],
            tmp_path, image_path=tmp_iso,
        )

    def test_strip_ecc_not(self, tmp_path):
        """Strip ECC from a non-augmented (already stripped) RS03i image."""
        master_path = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master_path, tmp_iso)
        # First strip the ECC
        _run_dvdisaster("-i{}".format(tmp_iso), "--strip")
        # Then try to strip again
        _run_golden_compare(
            "strip_ecc_not",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-v", "--strip"],
            tmp_path, image_path=tmp_iso,
        )


# ===========================================================================
# Verify tests (48)
# ===========================================================================

class TestRS03iVerify(GoldenTestSuite):
    codec = "RS03"
    codec_prefix = "RS03i"
    master = "rs03i-master.iso"
    image_size = ISOSIZE

    tests = [
        # 1. good
        GoldenTest("good", action="-t", use_master=True),
        # 2. good_quick
        GoldenTest("good_quick", action="-tq", use_master=True),
        # 3. no_image
        GoldenTest("no_image", action="-t", image="no.iso", ecc="no.ecc"),
        # 4. truncated
        GoldenTest("truncated", action="-t",
                   damage=[Truncate(REAL_ECCSIZE - 5)]),
        # 5. plus1
        GoldenTest("plus1", action="-t",
                   damage=[PadSectors(1)]),
        # 6. plus17
        GoldenTest("plus17", action="-t",
                   damage=[PadSectors(17)]),
        # 7. bad_crc_cookie
        GoldenTest("bad_crc_cookie", action="-t",
                   damage=[Byteset(21100, 1026, 1)]),
        # 8. bad_crc_checksum
        GoldenTest("bad_crc_checksum", action="-t",
                   damage=[Byteset(21100, 900, 1), Byteset(21107, 555, 1)]),
        # 9. missing_crc_sectors
        GoldenTest("missing_crc_sectors", action="-t",
                   damage=[Erase("21100-21108"), Erase("21111")]),
        # 10. missing_data_sectors
        GoldenTest("missing_data_sectors", action="-t",
                   damage=[Erase("1500-1673"), Erase("13420-14109"), Erase("17812")]),
        # 11. missing_ecc_sectors
        GoldenTest("missing_ecc_sectors", action="-t",
                   damage=[Erase("21168"), Erase("21900-21950")]),
        # 12. data_bad_byte
        GoldenTest("data_bad_byte", action="-t",
                   damage=[Byteset(4096, 100, 17)]),
        # 13. ecc_bad_byte
        GoldenTest("ecc_bad_byte", action="-t",
                   damage=[Byteset(21878, 100, 17)]),
        # 14. uncorrectable_dsm_in_image
        GoldenTest("uncorrectable_dsm_in_image", action="-t",
                   damage=_DAMAGE_DSM1),
        # 15. uncorrectable_dsm_in_image_verbose
        GoldenTest("uncorrectable_dsm_in_image_verbose", action="-t -v",
                   damage=_DAMAGE_DSM1),
        # 16. uncorrectable_dsm_in_image2
        GoldenTest("uncorrectable_dsm_in_image2", action="-t",
                   damage=_DAMAGE_DSM2),
        # 17. uncorrectable_dsm_in_image2_verbose
        GoldenTest("uncorrectable_dsm_in_image2_verbose", action="-t -v",
                   damage=_DAMAGE_DSM2),
    ]

    def _ensure_master(self):
        return _ensure_master()

    # -----------------------------------------------------------------------
    # Manual tests: complex setup
    # -----------------------------------------------------------------------

    def test_plus_56_bytes(self, tmp_path):
        """Image with 56 extra bytes (char '1'), augmented with RS03, then verified."""
        raw_path = _ensure_raw_image()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(raw_path, tmp_iso)

        with open(tmp_iso, "ab") as f:
            f.write(b"1" * 56)

        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(tmp_iso), "-mRS03", "-n{}".format(ECCSIZE), "-c",
        )

        _run_golden_compare(
            "plus_56_bytes",
            ["--regtest", "--no-progress", "--debug",
             "-n{}".format(ECCSIZE),
             "-i{}".format(tmp_iso), "-t", "-v"],
            tmp_path, image_path=tmp_iso,
            ignore_line_re=r"^Avg performance|^Augmenting image with Method RS03",
        )

    def test_layer_multiple(self, tmp_path):
        """Image size is exact multiple of layer size."""
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        _run_dvdisaster(
            "--debug", "-i", tmp_iso, "--random-image", "14508",
        )
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-mRS03", "-n20000", "-c", "-i", tmp_iso,
        )
        _run_golden_compare(
            "layer_multiple",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-t"],
            tmp_path, image_path=tmp_iso,
        )

    def test_no_padding(self, tmp_path):
        """Image size crafted to have no padding behind data area."""
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        _run_dvdisaster(
            "--debug", "-i", tmp_iso, "--random-image", "14506",
        )
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-mRS03", "-n20000", "-c", "-i", tmp_iso,
        )
        _run_golden_compare(
            "no_padding",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-t"],
            tmp_path, image_path=tmp_iso,
        )

    def test_with_rs01_file(self, tmp_path):
        """Augmented image protected by an outer RS01 error correction file."""
        master_path = _ensure_master()
        tmp_ecc = os.path.join(str(tmp_path), "rs03i-tmp.ecc")

        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(master_path), "-e{}".format(tmp_ecc),
            "-c", "-n", "normal",
        )

        _run_golden_compare(
            "with_rs01_file",
            ["--regtest", "--no-progress",
             "-i{}".format(master_path), "-e{}".format(tmp_ecc), "-v", "-t"],
            tmp_path, image_path=master_path, ecc_path=tmp_ecc,
        )

    def test_with_wrong_rs01_file(self, tmp_path):
        """Augmented image with non-matching RS01 error correction file."""
        master_path = _ensure_master()
        tmp_ecc = os.path.join(str(tmp_path), "rs03i-tmp.ecc")

        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(master_path), "-e{}".format(tmp_ecc),
            "-c", "-n", "normal",
        )
        _run_dvdisaster(
            "--debug", "-i", tmp_ecc, "--byteset", "0,24,1",
        )

        _run_golden_compare(
            "with_wrong_rs01_file",
            ["--regtest", "--no-progress",
             "-i{}".format(master_path), "-e{}".format(tmp_ecc), "-v", "-t"],
            tmp_path, image_path=master_path, ecc_path=tmp_ecc,
        )

    def test_with_rs03_file(self, tmp_path):
        """Augmented image protected by an outer RS03 error correction file."""
        master_path = _ensure_master()
        tmp_ecc = os.path.join(str(tmp_path), "rs03i-tmp.ecc")

        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(master_path), "-e{}".format(tmp_ecc),
            "-mRS03", "-c", "-n", "20r", "-o", "file",
        )

        _run_golden_compare(
            "with_rs03_file",
            ["--regtest", "--no-progress",
             "-i{}".format(master_path), "-e{}".format(tmp_ecc), "-v", "-t"],
            tmp_path, image_path=master_path, ecc_path=tmp_ecc,
        )

    def test_with_wrong_rs03_file(self, tmp_path):
        """Augmented image with non-matching RS03 error correction file."""
        master_path = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03i-tmp.ecc")

        # Create image with manipulated fingerprint sector
        shutil.copy2(master_path, tmp_iso)
        _run_dvdisaster(
            "--debug", "-i{}".format(tmp_iso), "--byteset", "16,240,1",
        )

        # Create ecc file for "wrong" image
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "-mRS03", "-c", "-n", "20r", "-o", "file",
        )

        # Test against original image
        _run_golden_compare(
            "with_wrong_rs03_file",
            ["--regtest", "--no-progress",
             "-i{}".format(master_path), "-e{}".format(tmp_ecc), "-v", "-t"],
            tmp_path, image_path=master_path, ecc_path=tmp_ecc,
        )

    def test_crc_section_with_uncorrectable_dsm(self, tmp_path):
        """CRC section with uncorrectable dead sector markers."""
        master_path = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master_path, tmp_iso)

        damage = [
            Erase("21077"),
            Byteset(21077, 353, 50),
            Erase("21080"),
            Byteset(21080, 353, 53),
            Erase("21081"),
            Byteset(21081, 353, 53),
        ]
        _apply_damage(tmp_iso, damage)

        _run_golden_compare(
            "crc_section_with_uncorrectable_dsm",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-t"],
            tmp_path, image_path=tmp_iso,
        )

    def test_ecc_section_with_uncorrectable_dsm(self, tmp_path):
        """ECC section with uncorrectable dead sector markers."""
        master_path = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master_path, tmp_iso)

        damage = [
            Erase("22030"),
            Byteset(22030, 353, 49),
            Erase("22400"),
            Byteset(22400, 353, 53),
            Erase("22411"),
            Byteset(22411, 353, 53),
        ]
        _apply_damage(tmp_iso, damage)

        _run_golden_compare(
            "ecc_section_with_uncorrectable_dsm",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-t"],
            tmp_path, image_path=tmp_iso,
        )

    def test_missing_iso_header(self, tmp_path):
        """Large image with missing ISO header (sector 16)."""
        large_path = _ensure_large_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(large_path, tmp_iso)

        _apply_damage(tmp_iso, [Erase("16")])

        _run_golden_compare(
            "missing_iso_header",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-tq", "-v"],
            tmp_path, image_path=tmp_iso,
        )

    def test_missing_header(self, tmp_path):
        """Large image with missing ECC header (first sector)."""
        large_path = _ensure_large_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(large_path, tmp_iso)

        _apply_damage(tmp_iso, [Erase(str(LMI_HEADER))])

        _run_golden_compare(
            "missing_header",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-t", "-v"],
            tmp_path, image_path=tmp_iso,
        )

    def test_missing_header5(self, tmp_path):
        """Large image with missing ECC header (second sector)."""
        large_path = _ensure_large_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(large_path, tmp_iso)

        _apply_damage(tmp_iso, [Erase(str(LMI_HEADER + 1))])

        _run_golden_compare(
            "missing_header5",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-t", "-v"],
            tmp_path, image_path=tmp_iso,
        )

    def test_missing_header6(self, tmp_path):
        """Large image with both ECC header sectors missing."""
        large_path = _ensure_large_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(large_path, tmp_iso)

        _apply_damage(tmp_iso, [Erase(str(LMI_HEADER)), Erase(str(LMI_HEADER + 1))])

        _run_golden_compare(
            "missing_header6",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-t", "-v"],
            tmp_path, image_path=tmp_iso,
        )

    def test_missing_header2(self, tmp_path):
        """Missing ECC header; first CRC sector and some data sectors unreadable."""
        large_path = _ensure_large_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(large_path, tmp_iso)

        damage = [Erase(str(LMI_HEADER)), Erase(str(LMI_FIRSTCRC))]
        for i in range(120 * LMI_LAYER_SIZE, 136 * LMI_LAYER_SIZE, LMI_LAYER_SIZE):
            damage.append(Erase(str(i)))
        _apply_damage(tmp_iso, damage)

        _run_golden_compare(
            "missing_header2",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-t", "-v"],
            tmp_path, image_path=tmp_iso,
        )

    def test_missing_header3(self, tmp_path):
        """Missing ECC header; first few slices have damaged CRC sectors."""
        large_path = _ensure_large_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(large_path, tmp_iso)

        damage = [Erase(str(LMI_HEADER))]
        # slice 0
        damage.append(Erase(str(LMI_FIRSTCRC)))
        for i in range(150 * LMI_LAYER_SIZE, 237 * LMI_LAYER_SIZE, LMI_LAYER_SIZE):
            damage.append(Erase(str(i)))
        # slice 1 (CRC sector present but bad sector checksum)
        damage.append(Byteset(LMI_FIRSTCRC + 1, 500, 0))
        for i in range(110 * LMI_LAYER_SIZE + 1, 141 * LMI_LAYER_SIZE + 1, LMI_LAYER_SIZE):
            damage.append(Erase(str(i)))
        # slice 2
        damage.append(Erase(str(LMI_FIRSTCRC + 2)))
        for i in range(110 * LMI_LAYER_SIZE + 2, 141 * LMI_LAYER_SIZE + 2, LMI_LAYER_SIZE):
            damage.append(Erase(str(i)))
        # slice 3
        damage.append(Byteset(LMI_FIRSTCRC + 3, 500, 0))
        for i in range(110 * LMI_LAYER_SIZE + 3, 140 * LMI_LAYER_SIZE + 3, LMI_LAYER_SIZE):
            damage.append(Erase(str(i)))
        # slice 4
        damage.append(Erase(str(LMI_FIRSTCRC + 4)))

        _apply_damage(tmp_iso, damage)

        _run_golden_compare(
            "missing_header3",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-t", "-v"],
            tmp_path, image_path=tmp_iso,
        )

    def test_missing_header4(self, tmp_path):
        """Missing ECC header; every CRC sector except the last is unreadable."""
        large_path = _ensure_large_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(large_path, tmp_iso)

        damage = [Erase(str(LMI_HEADER))]
        # slice 0
        damage.append(Erase(str(LMI_FIRSTCRC)))
        for i in range(100 * LMI_LAYER_SIZE, 187 * LMI_LAYER_SIZE, LMI_LAYER_SIZE):
            damage.append(Erase(str(i)))
        # slice 1
        damage.append(Byteset(LMI_FIRSTCRC + 1, 500, 0))
        for i in range(110 * LMI_LAYER_SIZE + 1, 141 * LMI_LAYER_SIZE + 1, LMI_LAYER_SIZE):
            damage.append(Erase(str(i)))
        # slice 2
        damage.append(Erase(str(LMI_FIRSTCRC + 2)))
        for i in range(110 * LMI_LAYER_SIZE + 2, 141 * LMI_LAYER_SIZE + 2, LMI_LAYER_SIZE):
            damage.append(Erase(str(i)))
        # slice 3
        damage.append(Byteset(LMI_FIRSTCRC + 3, 500, 0))
        for i in range(110 * LMI_LAYER_SIZE + 3, 140 * LMI_LAYER_SIZE + 3, LMI_LAYER_SIZE):
            damage.append(Erase(str(i)))
        # slices 4-1407
        damage.append(Erase("{}-{}".format(LMI_FIRSTCRC + 4, LMI_FIRSTCRC + 1407)))

        _apply_damage(tmp_iso, damage)

        _run_golden_compare(
            "missing_header4",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-t", "-v"],
            tmp_path, image_path=tmp_iso,
        )

    def test_missing_header_truncated(self, tmp_path):
        """Missing ECC header (like header2) plus truncated image."""
        large_path = _ensure_large_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(large_path, tmp_iso)

        damage = [Erase(str(LMI_HEADER)), Erase(str(LMI_FIRSTCRC))]
        for i in range(120 * LMI_LAYER_SIZE, 136 * LMI_LAYER_SIZE, LMI_LAYER_SIZE):
            damage.append(Erase(str(i)))
        damage.append(Truncate(300000))
        _apply_damage(tmp_iso, damage)

        _run_golden_compare(
            "missing_header_truncated",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-t", "-v"],
            tmp_path, image_path=tmp_iso,
        )

    def test_missing_header_no_crcsec(self, tmp_path):
        """Missing ECC header and entire CRC layer erased."""
        large_path = _ensure_large_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        none_file = os.path.join(_TMPDIR, "none.file")
        shutil.copy2(large_path, tmp_iso)

        damage = [
            Erase(str(LMI_HEADER)),
            Erase("{}-{}".format(LMI_FIRSTCRC, LMI_FIRSTCRC + 1408)),
        ]
        _apply_damage(tmp_iso, damage)

        _run_golden_compare(
            "missing_header_no_crcsec",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-e{}".format(none_file), "-t", "-v"],
            tmp_path, image_path=tmp_iso,
        )

    def test_random_image(self, tmp_path):
        """Completely random image with no ECC."""
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        none_file = os.path.join(_TMPDIR, "none.file")
        _run_dvdisaster(
            "--debug", "-i{}".format(tmp_iso), "--random-image", "359295",
        )
        _run_golden_compare(
            "random_image",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-e{}".format(none_file), "-tq", "-v"],
            tmp_path, image_path=tmp_iso,
        )

    def test_rediscover_8_roots(self, tmp_path):
        """Image with 8 roots, no ECC header (first sector erased)."""
        raw_path = _ensure_raw_lmi246()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(raw_path, tmp_iso)

        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(tmp_iso), "-mRS03", "-c", "-x", "2",
        )
        _apply_damage(tmp_iso, [Erase("346612")])

        _run_golden_compare(
            "rediscover_8_roots",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-t", "-v"],
            tmp_path, image_path=tmp_iso,
        )

    def test_rediscover_8_roots2(self, tmp_path):
        """Image with 8 roots, no ECC header and some CRC sectors erased."""
        raw_path = _ensure_raw_lmi246()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(raw_path, tmp_iso)

        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(tmp_iso), "-mRS03", "-c", "-x", "2",
        )
        _apply_damage(tmp_iso, [Erase("346612-346620")])

        _run_golden_compare(
            "rediscover_8_roots2",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-t", "-v"],
            tmp_path, image_path=tmp_iso,
        )

    def test_rediscover_170_roots(self, tmp_path):
        """Image with 170 roots, no ECC header (first sector erased)."""
        raw_path = _ensure_raw_lmi84()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(raw_path, tmp_iso)

        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(tmp_iso), "-mRS03", "-c", "-x", "2",
        )
        _apply_damage(tmp_iso, [Erase("118354")])

        _run_golden_compare(
            "rediscover_170_roots",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-t", "-v"],
            tmp_path, image_path=tmp_iso,
        )

    def test_rediscover_170_roots2(self, tmp_path):
        """Image with 170 roots, no ECC header and some CRC sectors erased."""
        raw_path = _ensure_raw_lmi84()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(raw_path, tmp_iso)

        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(tmp_iso), "-mRS03", "-c", "-x", "2",
        )
        _apply_damage(tmp_iso, [Erase("118354-118360")])

        _run_golden_compare(
            "rediscover_170_roots2",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-t", "-v"],
            tmp_path, image_path=tmp_iso,
        )

    def test_rediscover_170_roots_padding(self, tmp_path):
        """Image with 170 roots and padding, ECC header present."""
        raw_path = _ensure_raw_lmi84s()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(raw_path, tmp_iso)

        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(tmp_iso), "-mRS03", "-c", "-x", "2",
        )

        # Golden file uses hyphen: rediscover_170_roots-padding
        _run_golden_compare(
            "rediscover_170_roots-padding",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-tq", "-v"],
            tmp_path, image_path=tmp_iso,
        )

    def test_rediscover_170_roots_padding2(self, tmp_path):
        """Image with 170 roots and padding, no ECC header, some CRC sectors erased."""
        raw_path = _ensure_raw_lmi84s()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(raw_path, tmp_iso)

        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(tmp_iso), "-mRS03", "-c", "-x", "2",
        )
        _apply_damage(tmp_iso, [Erase("112354"), Erase("118356-118360")])

        # Golden file uses hyphen: rediscover_170_roots-padding2
        _run_golden_compare(
            "rediscover_170_roots-padding2",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-t", "-v"],
            tmp_path, image_path=tmp_iso,
        )

    def test_with_ecc_file_header(self, tmp_path):
        """Image contains ECC header with the ecc file flag set."""
        master_path = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master_path, tmp_iso)

        damage = [
            Byteset(21000, 16, 2),
            Byteset(21000, 96, 142),
            Byteset(21000, 97, 43),
            Byteset(21000, 98, 137),
            Byteset(21000, 99, 29),
        ]
        _apply_damage(tmp_iso, damage)

        _run_golden_compare(
            "with_ecc_file_header",
            ["--regtest", "--no-progress", "--debug", "-n{}".format(ECCSIZE),
             "-i{}".format(tmp_iso), "-t", "-v"],
            tmp_path, image_path=tmp_iso,
        )

    def test_with_ecc_file_crc_block(self, tmp_path):
        """Image with defective ECC header and a CRC block from an ecc file."""
        master_path = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master_path, tmp_iso)

        damage = [
            Erase("21000"),
            Byteset(21070, 1040, 2),
            Byteset(21070, 1120, 208),
            Byteset(21070, 1121, 250),
            Byteset(21070, 1122, 142),
            Byteset(21070, 1123, 101),
        ]
        _apply_damage(tmp_iso, damage)

        _run_golden_compare(
            "with_ecc_file_crc_block",
            ["--regtest", "--no-progress", "--debug", "-n{}".format(ECCSIZE),
             "-i{}".format(tmp_iso), "-t", "-v"],
            tmp_path, image_path=tmp_iso,
        )

    def test_with_crc_error_in_padding(self, tmp_path):
        """Image contains CRC error in the padding section."""
        master_path = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master_path, tmp_iso)

        _apply_damage(tmp_iso, [Byteset(21020, 400, 255)])

        _run_golden_compare(
            "with_crc_error_in_padding",
            ["--regtest", "--no-progress", "--debug",
             "-t", "-n{}".format(ECCSIZE),
             "-i{}".format(tmp_iso)],
            tmp_path, image_path=tmp_iso,
        )

    def test_verify_custom_n_good(self, tmp_path):
        """Verify custom-n image without specifying -n (ECC header intact)."""
        custom_path = _ensure_custom_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(custom_path, tmp_iso)

        _run_golden_compare(
            "verify_custom_n_good",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-t"],
            tmp_path, image_path=tmp_iso,
        )

    def test_verify_custom_n_bad_header_no_n(self, tmp_path):
        """Verify custom-n image with damaged header and without -n."""
        custom_path = _ensure_custom_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(custom_path, tmp_iso)

        _apply_damage(tmp_iso, [Byteset(ISOSIZE, 1, 1)])

        _run_golden_compare(
            "verify_custom_n_bad_header_no_n",
            ["--regtest", "--no-progress",
             "-i{}".format(tmp_iso), "-t"],
            tmp_path, image_path=tmp_iso,
        )

    def test_verify_custom_n_bad_header_with_n(self, tmp_path):
        """Verify custom-n image with damaged header, specifying -n."""
        custom_path = _ensure_custom_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(custom_path, tmp_iso)

        _apply_damage(tmp_iso, [Byteset(ISOSIZE, 1, 1)])

        _run_golden_compare(
            "verify_custom_n_bad_header_with_n",
            ["--regtest", "--no-progress", "--debug",
             "-n{}".format(CUSTOM_ECCSIZE),
             "-i{}".format(tmp_iso), "-t"],
            tmp_path, image_path=tmp_iso,
        )


# ---------------------------------------------------------------------------
# Creation tests
# ---------------------------------------------------------------------------

class TestRS03iCreate:
    """RS03i creation tests -- from regtest/rs03i.bash lines 739-1013."""

    _CREATE_IGNORE_RE = r'^Avg performance|^Augmenting image with Method RS03'

    def test_ecc_create(self, tmp_path):
        """Basic augmented image creation."""
        raw = _ensure_raw_image()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(raw, tmp_iso)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--set-version", SETVERSION,
            "-mRS03", "-n{}".format(ECCSIZE), "-c",
        ]
        _run_golden_compare("ecc_create", cmd, tmp_path, image_path=tmp_iso,
                            ignore_line_re=self._CREATE_IGNORE_RE)

    def test_ecc_missing_image(self, tmp_path):
        """Create augmented image with missing image."""
        no_iso = os.path.join(_ISODIR, "no.iso")
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(no_iso),
            "--debug", "--set-version", SETVERSION,
            "-mRS03", "-n{}".format(ECCSIZE), "-c",
        ]
        _run_golden_compare("ecc_missing_image", cmd, tmp_path,
                            ignore_line_re=self._CREATE_IGNORE_RE)

    def test_ecc_no_read_perm(self, tmp_path):
        """Create augmented image with no read permission on image."""
        raw = _ensure_raw_image()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(raw, tmp_iso)
        os.chmod(tmp_iso, 0o000)
        try:
            cmd = [
                "--regtest", "--no-progress",
                "-i{}".format(tmp_iso),
                "--debug", "--set-version", SETVERSION,
                "-mRS03", "-n{}".format(ECCSIZE), "-c",
            ]
            _run_golden_compare("ecc_no_read_perm", cmd, tmp_path,
                                ignore_line_re=self._CREATE_IGNORE_RE)
        finally:
            os.chmod(tmp_iso, 0o644)

    def test_ecc_no_write_perm(self, tmp_path):
        """Create augmented image with no write permission on image."""
        raw = _ensure_raw_image()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(raw, tmp_iso)
        os.chmod(tmp_iso, 0o400)
        try:
            cmd = [
                "--regtest", "--no-progress",
                "-i{}".format(tmp_iso),
                "--debug", "--set-version", SETVERSION,
                "-mRS03", "-n{}".format(ECCSIZE), "-c",
            ]
            _run_golden_compare("ecc_no_write_perm", cmd, tmp_path,
                                ignore_line_re=self._CREATE_IGNORE_RE)
        finally:
            os.chmod(tmp_iso, 0o644)

    def test_ecc_from_rs03(self, tmp_path):
        """Create augmented image from already RS03-augmented image."""
        master = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, tmp_iso)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--set-version", SETVERSION,
            "-mRS03", "-n{}".format(ECCSIZE), "-c",
        ]
        _run_golden_compare("ecc_from_rs03", cmd, tmp_path, image_path=tmp_iso,
                            ignore_line_re=self._CREATE_IGNORE_RE)

    def test_ecc_from_rs02(self, tmp_path):
        """Create augmented image from already RS02-augmented image."""
        raw = _ensure_raw_image()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(raw, tmp_iso)
        # Augment with RS02 first
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(tmp_iso),
            "-mRS02", "-n{}".format(ECCSIZE + 5000), "-c",
        )
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--set-version", SETVERSION,
            "-mRS03", "-n{}".format(ECCSIZE), "-c",
        ]
        _run_golden_compare("ecc_from_rs02", cmd, tmp_path, image_path=tmp_iso,
                            ignore_line_re=self._CREATE_IGNORE_RE)

    def test_ecc_from_larger_rs03(self, tmp_path):
        """Create augmented image from RS03-augmented image with higher redundancy."""
        raw = _ensure_raw_image()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(raw, tmp_iso)
        # Augment with RS03 larger first
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(tmp_iso),
            "-mRS03", "-n{}".format(ECCSIZE + 5000), "-c",
        )
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--set-version", SETVERSION,
            "-mRS03", "-n{}".format(ECCSIZE), "-c",
        ]
        _run_golden_compare("ecc_from_larger_rs03", cmd, tmp_path, image_path=tmp_iso,
                            ignore_line_re=self._CREATE_IGNORE_RE)

    def test_ecc_from_rs02_non_blocksize(self, tmp_path):
        """Create augmented image from RS02-augmented image with non-block size."""
        raw = _ensure_raw_image()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(raw, tmp_iso)
        # Append 56 bytes
        with open(tmp_iso, "ab") as f:
            f.write(b"1" * 56)
        # Augment with RS02
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(tmp_iso),
            "-mRS02", "-n{}".format(ECCSIZE), "-c",
        )
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--set-version", SETVERSION,
            "-mRS03", "-n{}".format(ECCSIZE), "-c", "-a", "RS03",
        ]
        _run_golden_compare("ecc_from_rs02_non_blocksize", cmd, tmp_path, image_path=tmp_iso,
                            ignore_line_re=self._CREATE_IGNORE_RE)

    def test_ecc_from_rs03_non_blocksize(self, tmp_path):
        """Create augmented image from RS03-augmented image with non-block size."""
        raw = _ensure_raw_image()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(raw, tmp_iso)
        # Append 56 bytes
        with open(tmp_iso, "ab") as f:
            f.write(b"1" * 56)
        # Augment with RS03
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(tmp_iso),
            "-mRS03", "-n{}".format(ECCSIZE), "-c",
        )
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--set-version", SETVERSION,
            "-mRS03", "-n{}".format(ECCSIZE), "-c", "-a", "RS03",
        ]
        _run_golden_compare("ecc_from_rs03_non_blocksize", cmd, tmp_path, image_path=tmp_iso,
                            ignore_line_re=self._CREATE_IGNORE_RE)

    def test_ecc_from_larger_rs03_non_blocksize(self, tmp_path):
        """Create augmented image from RS03-augmented image with non-block size, larger redundancy."""
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        # Create fresh random image of ISOSIZE+1 sectors, then truncate
        _run_dvdisaster(
            "--debug",
            "-i{}".format(tmp_iso),
            "--random-image", str(ISOSIZE + 1),
        )
        _run_dvdisaster(
            "--debug",
            "-i{}".format(tmp_iso),
            "--truncate={}".format(ISOSIZE),
        )
        # Append 56 bytes
        with open(tmp_iso, "ab") as f:
            f.write(b"1" * 56)
        # Augment with RS03 larger
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(tmp_iso),
            "-mRS03", "-n{}".format(ECCSIZE + 5000), "-c",
        )
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--set-version", SETVERSION,
            "-mRS03", "-n{}".format(ECCSIZE), "-c",
        ]
        _run_golden_compare("ecc_from_larger_rs03_non_blocksize", cmd, tmp_path, image_path=tmp_iso,
                            ignore_line_re=self._CREATE_IGNORE_RE)

    def test_ecc_non_blocksize(self, tmp_path):
        """Create augmented image from image with 56 extra bytes."""
        raw = _ensure_raw_image()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(raw, tmp_iso)
        # Append 56 bytes
        with open(tmp_iso, "ab") as f:
            f.write(b"1" * 56)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--set-version", SETVERSION,
            "-mRS03", "-n{}".format(ECCSIZE), "-c",
        ]
        _run_golden_compare("ecc_non_blocksize", cmd, tmp_path, image_path=tmp_iso,
                            ignore_line_re=self._CREATE_IGNORE_RE)

    def test_ecc_missing_sectors(self, tmp_path):
        """Create augmented image from image with missing sectors."""
        raw = _ensure_raw_image()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(raw, tmp_iso)
        _apply_damage(tmp_iso, [Erase("500-524")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--set-version", SETVERSION,
            "-mRS03", "-n{}".format(ECCSIZE), "-c",
        ]
        _run_golden_compare("ecc_missing_sectors", cmd, tmp_path, image_path=tmp_iso,
                            ignore_line_re=self._CREATE_IGNORE_RE)

    def test_ecc_layer_multiple(self, tmp_path):
        """Create augmented image where image size is exact multiple of layer size."""
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        _run_dvdisaster(
            "--debug",
            "-i{}".format(tmp_iso),
            "--random-image", "14508",
        )
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--set-version", SETVERSION,
            "-mRS03", "-n20000", "-c",
        ]
        _run_golden_compare("ecc_layer_multiple", cmd, tmp_path, image_path=tmp_iso,
                            ignore_line_re=self._CREATE_IGNORE_RE)

    def test_ecc_no_padding(self, tmp_path):
        """Create augmented image crafted to have no padding."""
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        _run_dvdisaster(
            "--debug",
            "-i{}".format(tmp_iso),
            "--random-image", "14506",
        )
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--set-version", SETVERSION,
            "-mRS03", "-n20000", "-c",
        ]
        _run_golden_compare("ecc_no_padding", cmd, tmp_path, image_path=tmp_iso,
                            ignore_line_re=self._CREATE_IGNORE_RE)

    def test_ecc_create_after_read(self, tmp_path):
        """Read image and create ecc in one call."""
        raw = _ensure_raw_image()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(raw, sim_iso)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--set-version", SETVERSION,
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "-r", "-mRS03", "-c", "-n{}".format(ECCSIZE), "-v", "--spinup-delay=0",
        ]
        _run_golden_compare("ecc_create_after_read", cmd, tmp_path,
                            image_path=tmp_iso,
                            ignore_line_re=self._CREATE_IGNORE_RE)

    def test_ecc_create_after_partial_read(self, tmp_path):
        """Create ecc after completing partial image."""
        raw = _ensure_raw_image()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03i-tmp.ecc")
        shutil.copy2(raw, sim_iso)
        shutil.copy2(sim_iso, tmp_iso)
        _apply_damage(tmp_iso, [Erase("1000-1500")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug", "--set-version", SETVERSION,
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "-r", "-mRS03", "-c", "-n{}".format(ECCSIZE), "-v", "--spinup-delay=0",
        ]
        _run_golden_compare("ecc_create_after_partial_read", cmd, tmp_path,
                            image_path=tmp_iso,
                            ignore_line_re=self._CREATE_IGNORE_RE)

    def test_ecc_recreate_after_read_rs01(self, tmp_path):
        """Read RS01-protected image, create RS03i augmentation."""
        raw = _ensure_raw_image()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03i-tmp.ecc")
        shutil.copy2(raw, sim_iso)
        # Create RS01 ecc for the sim image
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(sim_iso), "-e{}".format(tmp_ecc),
            "-mRS01", "-c", "-n", "10r",
        )
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug", "--set-version", SETVERSION,
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "-r", "-mRS03", "-c", "-n{}".format(ECCSIZE), "-v", "--spinup-delay=0",
        ]
        _run_golden_compare("ecc_recreate_after_read_rs01", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=tmp_ecc,
                            ignore_line_re=self._CREATE_IGNORE_RE)

    def test_ecc_recreate_after_read_rs02(self, tmp_path):
        """Read RS02-protected image, create RS03i augmentation."""
        raw = _ensure_raw_image()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03i-tmp.ecc")
        shutil.copy2(raw, sim_iso)
        # Augment sim image with RS02
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(sim_iso),
            "-mRS02", "-c", "-n24000",
        )
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug", "--set-version", SETVERSION,
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "-r", "-mRS03", "-c", "-n{}".format(ECCSIZE), "-v", "--spinup-delay=0",
        ]
        _run_golden_compare("ecc_recreate_after_read_rs02", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=tmp_ecc,
                            ignore_line_re=self._CREATE_IGNORE_RE)

    def test_ecc_recreate_after_read_rs03i(self, tmp_path):
        """Read RS03i-protected image, create new RS03i augmentation."""
        raw = _ensure_raw_image()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03i-tmp.ecc")
        shutil.copy2(raw, sim_iso)
        # Augment sim image with RS03i
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(sim_iso),
            "-mRS03", "-c", "-n23000",
        )
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug", "--set-version", SETVERSION,
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "-r", "-mRS03", "-c", "-n{}".format(ECCSIZE), "-v", "--spinup-delay=0",
        ]
        _run_golden_compare("ecc_recreate_after_read_rs03i", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=tmp_ecc,
                            ignore_line_re=self._CREATE_IGNORE_RE)

    def test_ecc_recreate_after_read_rs03f(self, tmp_path):
        """Read RS03f-protected image, create RS03i augmentation."""
        raw = _ensure_raw_image()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03i-tmp.ecc")
        shutil.copy2(raw, sim_iso)
        # Create RS03f ecc file for the sim image
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(sim_iso), "-e{}".format(tmp_ecc),
            "-mRS03", "-c", "-n", "10r", "-o", "file",
        )
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug", "--set-version", SETVERSION,
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "-r", "-mRS03", "-c", "-n{}".format(ECCSIZE), "-v", "--spinup-delay=0",
        ]
        _run_golden_compare("ecc_recreate_after_read_rs03f", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=tmp_ecc,
                            ignore_line_re=self._CREATE_IGNORE_RE)


# ---------------------------------------------------------------------------
# Fix tests
# ---------------------------------------------------------------------------

class TestRS03iFix:
    """RS03i fix (repair) tests -- from regtest/rs03i.bash lines 1015-1369."""

    def test_fix_no_read_perm(self, tmp_path):
        """Fix fails when image is not readable."""
        raw = _ensure_raw_image()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(raw, tmp_iso)
        os.chmod(tmp_iso, 0o000)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "-f",
        ]
        try:
            _run_golden_compare("fix_no_read_perm", cmd, tmp_path,
                                image_path=tmp_iso)
        finally:
            os.chmod(tmp_iso, 0o644)

    def test_fix_no_write_perm(self, tmp_path):
        """Fix fails when image is read-only."""
        raw = _ensure_raw_image()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(raw, tmp_iso)
        os.chmod(tmp_iso, 0o400)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "-f",
        ]
        try:
            _run_golden_compare("fix_no_write_perm", cmd, tmp_path,
                                image_path=tmp_iso)
        finally:
            os.chmod(tmp_iso, 0o644)

    def test_fix_good_image(self, tmp_path):
        """Fix already good image."""
        master = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, tmp_iso)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "-f",
        ]
        _run_golden_compare("fix_good_image", cmd, tmp_path, image_path=tmp_iso)

    def test_fix_truncated_image(self, tmp_path):
        """Fix truncated image (REAL_ECCSIZE-210 = 24780 sectors)."""
        master = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [Truncate(REAL_ECCSIZE - 210)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "-f",
        ]
        _run_golden_compare("fix_truncated_image", cmd, tmp_path, image_path=tmp_iso)

    def test_fix_trailing_bytes(self, tmp_path):
        """Fix image with trailing garbage bytes appended."""
        master = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, tmp_iso)
        with open(tmp_iso, "ab") as f:
            f.write(b"some trailing garbage appended for testing\n")
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "-f",
        ]
        _run_golden_compare("fix_trailing_bytes", cmd, tmp_path, image_path=tmp_iso)

    def test_fix_trailing_tao(self, tmp_path):
        """Fix image with 2 trailing zero sectors (TAO)."""
        master = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, tmp_iso)
        with open(tmp_iso, "ab") as f:
            f.write(b'\x00' * (2 * 2048))
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "-f",
        ]
        _run_golden_compare("fix_trailing_tao", cmd, tmp_path, image_path=tmp_iso)

    def test_fix_trailing_garbage(self, tmp_path):
        """Fix image with 23 trailing zero sectors."""
        master = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, tmp_iso)
        with open(tmp_iso, "ab") as f:
            f.write(b'\x00' * (23 * 2048))
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "-f",
        ]
        _run_golden_compare("fix_trailing_garbage", cmd, tmp_path, image_path=tmp_iso)

    def test_fix_trailing_garbage2(self, tmp_path):
        """Fix image with 23 trailing zero sectors using --truncate."""
        master = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, tmp_iso)
        with open(tmp_iso, "ab") as f:
            f.write(b'\x00' * (23 * 2048))
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "-f", "--truncate",
        ]
        _run_golden_compare("fix_trailing_garbage2", cmd, tmp_path, image_path=tmp_iso)

    def test_fix_correctable(self, tmp_path):
        """Fix image with correctable errors."""
        master = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [
            Erase("500-524"),
            Erase("1000"),
            Byteset(2000, 0, 111),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "-f",
        ]
        _run_golden_compare("fix_correctable", cmd, tmp_path, image_path=tmp_iso)

    def test_fix_border_cases_erasures(self, tmp_path):
        """Fix image with erasures at border sectors."""
        master = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [
            Erase("0"), Erase("98"), Erase("196"), Erase("20972"),
            Erase("21070"), Erase("21168"), Erase("21266"), Erase("24892"),
            Erase("97"), Erase("195"), Erase("293"), Erase("20999"),
            Erase("21167"), Erase("21265"), Erase("21363"), Erase("24989"),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "-f",
        ]
        _run_golden_compare("fix_border_cases_erasures", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_fix_border_cases_crc_errors(self, tmp_path):
        """Fix image with CRC errors at border sectors."""
        master = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [
            Byteset(0, 0, 1), Byteset(98, 0, 0), Byteset(196, 0, 0),
            Byteset(20972, 0, 0), Byteset(21070, 0, 0), Byteset(21168, 0, 0),
            Byteset(21266, 0, 0), Byteset(24892, 0, 0),
            Byteset(97, 0, 0), Byteset(195, 0, 0), Byteset(293, 0, 0),
            Byteset(20999, 0, 0), Byteset(21167, 0, 0), Byteset(21265, 0, 0),
            Byteset(21363, 0, 0), Byteset(24989, 0, 0),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "-f",
        ]
        _run_golden_compare("fix_border_cases_crc_errors", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_fix_layer_multiple(self, tmp_path):
        """Fix image where ISO size is a layer multiple."""
        raw_path = os.path.join(str(tmp_path), "rs03i-lm-raw.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        _run_dvdisaster(
            "--regtest", "--debug",
            "-i{}".format(raw_path),
            "--random-image", "14508",
            check=True,
        )
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(raw_path),
            "-mRS03", "-c", "-n20000",
            check=True,
        )
        shutil.copy2(raw_path, tmp_iso)
        _apply_damage(tmp_iso, [
            Erase("500-524"),
            Erase("14510-14520"),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "-f",
        ]
        _run_golden_compare("fix_layer_multiple", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_fix_no_padding(self, tmp_path):
        """Fix image without padding (ISO size not a layer multiple)."""
        raw_path = os.path.join(str(tmp_path), "rs03i-np-raw.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        _run_dvdisaster(
            "--regtest", "--debug",
            "-i{}".format(raw_path),
            "--random-image", "14506",
            check=True,
        )
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(raw_path),
            "-mRS03", "-c", "-n20000",
            check=True,
        )
        shutil.copy2(raw_path, tmp_iso)
        _apply_damage(tmp_iso, [Erase("500-524")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "-f",
        ]
        _run_golden_compare("fix_no_padding", cmd, tmp_path, image_path=tmp_iso)

    def test_fix_with_rs01_file(self, tmp_path):
        """Fix RS03i image with outer RS01 ecc file."""
        master = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03i-tmp.ecc")
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(master), "-e{}".format(tmp_ecc),
            "-c", "-n", "normal",
        )
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [Byteset(24989, 0, 1)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "-f",
        ]
        _run_golden_compare("fix_with_rs01_file", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=tmp_ecc)

    def test_fix_with_rs03_file(self, tmp_path):
        """Fix RS03i image with outer RS03f ecc file."""
        master = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03i-tmp.ecc")
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(master), "-e{}".format(tmp_ecc),
            "-mRS03", "-c", "-n", "20r", "-o", "file",
        )
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [Byteset(24989, 0, 1)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "-f",
        ]
        _run_golden_compare("fix_with_rs03_file", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=tmp_ecc)

    def test_fix_with_missing_header(self, tmp_path):
        """Fix image with missing RS03 header sector."""
        master = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [Erase(str(ISOSIZE))])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "-n{}".format(ECCSIZE),
            "-f", "-v",
        ]
        _run_golden_compare("fix_with_missing_header", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_fix_with_missing_iso_header(self, tmp_path):
        """Fix image with missing ISO header (sector 16)."""
        master = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [Erase("16")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "-n{}".format(ECCSIZE),
            "-f", "-v",
        ]
        _run_golden_compare("fix_with_missing_iso_header", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_fix_with_ecc_file_header(self, tmp_path):
        """Fix image where header has ecc-file bit set."""
        master = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [
            Byteset(ISOSIZE, 16, 2),
            Byteset(ISOSIZE, 96, 142),
            Byteset(ISOSIZE, 97, 43),
            Byteset(ISOSIZE, 98, 137),
            Byteset(ISOSIZE, 99, 29),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "-n{}".format(ECCSIZE),
            "-f", "-v",
        ]
        _run_golden_compare("fix_with_ecc_file_header", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_fix_with_ecc_file_crc_block(self, tmp_path):
        """Fix image where CRC block has ecc-file bit set."""
        master = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [
            Erase(str(ISOSIZE)),
            Byteset(21070, 1040, 2),
            Byteset(21070, 1120, 208),
            Byteset(21070, 1121, 250),
            Byteset(21070, 1122, 142),
            Byteset(21070, 1123, 101),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "-n{}".format(ECCSIZE),
            "-f", "-v",
        ]
        _run_golden_compare("fix_with_ecc_file_crc_block", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_fix_with_crc_error_in_padding(self, tmp_path):
        """Fix image with CRC error in padding area."""
        master = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [Byteset(21020, 400, 255)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "-n{}".format(ECCSIZE),
            "-f",
        ]
        _run_golden_compare("fix_with_crc_error_in_padding", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_fix_custom_n_good(self, tmp_path):
        """Fix already good custom-size image."""
        master = _ensure_custom_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, tmp_iso)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "-f",
        ]
        _run_golden_compare("fix_custom_n_good", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_fix_custom_n_bad_header_with_n(self, tmp_path):
        """Fix custom-size image with bad header, providing -n."""
        master = _ensure_custom_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [
            Byteset(ISOSIZE, 1, 1),
            Erase("500-524"),
            Erase("1000"),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "-n{}".format(CUSTOM_ECCSIZE),
            "-f",
        ]
        _run_golden_compare("fix_custom_n_bad_header_with_n", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_fix_custom_n_bad_header_no_n(self, tmp_path):
        """Fix custom-size image with bad header, no -n provided."""
        master = _ensure_custom_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [
            Byteset(ISOSIZE, 1, 1),
            Erase("500-524"),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "-f",
        ]
        _run_golden_compare("fix_custom_n_bad_header_no_n", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_fix_custom_n_bad_header_bruteforce(self, tmp_path):
        """Fix custom-size image with bad header using bruteforce search."""
        master = _ensure_custom_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [
            Byteset(ISOSIZE, 1, 1),
            Erase("500-524"),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--bruteforce-rs03-search", "-f",
        ]
        _run_golden_compare("fix_custom_n_bad_header_bruteforce", cmd,
                            tmp_path, image_path=tmp_iso)

    def test_fix_custom_n_truncated_with_n(self, tmp_path):
        """Fix truncated custom-size image with bad header, providing -n."""
        master = _ensure_custom_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [
            Byteset(ISOSIZE, 1, 1),
            Erase("500-524"),
            Truncate(21500),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "-n{}".format(CUSTOM_ECCSIZE),
            "-f",
        ]
        _run_golden_compare("fix_custom_n_truncated_with_n", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_fix_custom_n_truncated_no_bruteforce(self, tmp_path):
        """Fix truncated custom-size image with bad header, no -n, no bruteforce (expected to fail)."""
        master = _ensure_custom_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [
            Byteset(ISOSIZE, 1, 1),
            Erase("500-524"),
            Truncate(21500),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "-f",
        ]
        _run_golden_compare("fix_custom_n_truncated_no_bruteforce", cmd,
                            tmp_path, image_path=tmp_iso)

    def test_fix_custom_n_truncated_bruteforce(self, tmp_path):
        """Fix truncated custom-size image with bad header using bruteforce."""
        master = _ensure_custom_master()
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [
            Byteset(ISOSIZE, 1, 1),
            Erase("500-524"),
            Truncate(21500),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--bruteforce-rs03-search", "-f",
        ]
        _run_golden_compare("fix_custom_n_truncated_bruteforce", cmd,
                            tmp_path, image_path=tmp_iso)


# ===========================================================================
# Scanning tests (33)
# ===========================================================================

class TestRS03iScan:

    def test_scan_good(self, tmp_path):
        """Scan complete / optimal image."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_good", cmd, tmp_path)

    def test_scan_good_verbose(self, tmp_path):
        """Scan complete / optimal image, verbose output."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s", "-v",
        ]
        _run_golden_compare("scan_good_verbose", cmd, tmp_path)

    def test_scan_shorter(self, tmp_path):
        """Scan image shorter than expected."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Truncate(REAL_ECCSIZE - 44)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_shorter", cmd, tmp_path)

    def test_scan_longer(self, tmp_path):
        """Scan image longer than expected."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _append_fixed_random_sequence(sim_iso, times=23)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s", "-v",
        ]
        _run_golden_compare("scan_longer", cmd, tmp_path)

    def test_scan_tao_tail(self, tmp_path):
        """Scan image with two multisession link sectors appended (TAO tail)."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _append_fixed_random_sequence(sim_iso, times=1)
        _apply_damage(sim_iso, [Erase("24990-24991")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_tao_tail", cmd, tmp_path)

    def test_scan_no_tao_tail(self, tmp_path):
        """Scan image with two real sectors missing at end; --dao prevents clipping."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Erase("24988-24989")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s", "--dao",
        ]
        _run_golden_compare("scan_no_tao_tail", cmd, tmp_path)

    def test_scan_incompatible_ecc(self, tmp_path):
        """Scan image requiring a newer dvdisaster version."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [
            Byteset(21000, 84, 220),
            Byteset(21000, 85, 65),
            Byteset(21000, 86, 15),
            Byteset(21000, 88, 220),
            Byteset(21000, 89, 65),
            Byteset(21000, 90, 15),
            Byteset(21000, 96, 208),
            Byteset(21000, 97, 125),
            Byteset(21000, 98, 164),
            Byteset(21000, 99, 44),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_incompatible_ecc", cmd, tmp_path,
                            ignore_line_re=r'^\*          $')

    def test_scan_bad_header(self, tmp_path):
        """Scan image with a defective ECC header."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Byteset(21000, 1, 1)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_bad_header", cmd, tmp_path)

    def test_scan_missing_data_sectors(self, tmp_path):
        """Scan image with missing sectors in data portion."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [
            Erase("1000-1049"),
            Erase("21230"),
            Erase("22450-22457"),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_missing_data_sectors", cmd, tmp_path)

    def test_scan_missing_crc_sectors(self, tmp_path):
        """Scan image with missing sectors in CRC portion."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [
            Erase("21077"),
            Erase("21100-21120"),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_missing_crc_sectors", cmd, tmp_path)

    def test_scan_missing_ecc_sectors(self, tmp_path):
        """Scan image with missing sectors in ECC portion."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [
            Erase("21200"),
            Erase("21340-21365"),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_missing_ecc_sectors", cmd, tmp_path)

    def test_scan_data_bad_byte(self, tmp_path):
        """Scan image with bad byte in data section."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Byteset(1235, 50, 10)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_data_bad_byte", cmd, tmp_path)

    def test_scan_crc_bad_byte(self, tmp_path):
        """Scan image with bad byte in CRC section."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Byteset(21077, 50, 10)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_crc_bad_byte", cmd, tmp_path)

    def test_scan_ecc_bad_byte(self, tmp_path):
        """Scan image with bad byte in ECC section."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Byteset(22000, 50, 10)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_ecc_bad_byte", cmd, tmp_path)

    def test_scan_with_rs01_file(self, tmp_path):
        """Scan augmented image also protected by outer RS01 ecc file."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03i-tmp.ecc")
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(master), "-e{}".format(tmp_ecc),
                        "-c", "-n", "normal")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Byteset(24989, 0, 1)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_with_rs01_file", cmd, tmp_path,
                            ecc_path=tmp_ecc)

    def test_scan_with_wrong_rs01_file(self, tmp_path):
        """Scan augmented image with non-matching RS01 ecc file."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03i-tmp.ecc")
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(master), "-e{}".format(tmp_ecc),
                        "-c", "-n", "normal")
        _apply_damage(tmp_ecc, [Byteset(0, 24, 1)])
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Byteset(24989, 0, 1)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_with_wrong_rs01_file", cmd, tmp_path,
                            ecc_path=tmp_ecc)

    def test_scan_with_rs03_file(self, tmp_path):
        """Scan augmented image also protected by outer RS03 ecc file."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03i-tmp.ecc")
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(master), "-e{}".format(tmp_ecc),
                        "-mRS03", "-c", "-n", "20r", "-o", "file")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Byteset(24989, 0, 1)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_with_rs03_file", cmd, tmp_path,
                            ecc_path=tmp_ecc)

    def test_scan_with_wrong_rs03_file(self, tmp_path):
        """Scan augmented image with non-matching RS03 ecc file."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03i-tmp.ecc")
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(master), "-e{}".format(tmp_ecc),
                        "-mRS03", "-c", "-n", "20r", "-o", "file")
        _apply_damage(tmp_ecc, [Byteset(0, 24, 1)])
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Byteset(24989, 0, 1)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_with_wrong_rs03_file", cmd, tmp_path,
                            ecc_path=tmp_ecc)

    def test_scan_missing_header_not_exhaustive(self, tmp_path):
        """Scan large image with missing ECC header; no exhaustive search."""
        large = _ensure_large_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(large, sim_iso)
        _apply_damage(sim_iso, [Erase(str(LMI_HEADER))])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_missing_header_not_exhaustive", cmd, tmp_path)

    def test_scan_missing_header(self, tmp_path):
        """Scan large image with missing ECC header; exhaustive search."""
        large = _ensure_large_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(large, sim_iso)
        _apply_damage(sim_iso, [Erase(str(LMI_HEADER))])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-a", "RS03", "-s", "-v",
        ]
        _run_golden_compare("scan_missing_header", cmd, tmp_path)

    def test_scan_missing_header2(self, tmp_path):
        """Scan large image with missing header; first slice has damaged CRC+sectors."""
        large = _ensure_large_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(large, sim_iso)

        damage = [Erase(str(LMI_HEADER)), Erase(str(LMI_FIRSTCRC))]
        for i in range(120 * LMI_LAYER_SIZE, 136 * LMI_LAYER_SIZE, LMI_LAYER_SIZE):
            damage.append(Erase(str(i)))
        _apply_damage(sim_iso, damage)

        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-a", "RS03", "-s", "-v",
        ]
        _run_golden_compare("scan_missing_header2", cmd, tmp_path)

    def test_scan_missing_header3(self, tmp_path):
        """Scan large image with missing header; first few slices have damaged CRC sectors."""
        large = _ensure_large_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(large, sim_iso)

        damage = [Erase(str(LMI_HEADER))]
        # slice 0
        damage.append(Erase(str(LMI_FIRSTCRC)))
        for i in range(100 * LMI_LAYER_SIZE, 236 * LMI_LAYER_SIZE, LMI_LAYER_SIZE):
            damage.append(Erase(str(i)))
        # slice 1
        damage.append(Byteset(LMI_FIRSTCRC + 1, 500, 0))
        for i in range(110 * LMI_LAYER_SIZE + 1, 141 * LMI_LAYER_SIZE + 1, LMI_LAYER_SIZE):
            damage.append(Erase(str(i)))
        # slice 2
        damage.append(Erase(str(LMI_FIRSTCRC + 2)))
        for i in range(110 * LMI_LAYER_SIZE + 2, 141 * LMI_LAYER_SIZE + 2, LMI_LAYER_SIZE):
            damage.append(Erase(str(i)))
        # slice 3
        damage.append(Byteset(LMI_FIRSTCRC + 3, 500, 0))
        for i in range(110 * LMI_LAYER_SIZE + 3, 140 * LMI_LAYER_SIZE + 3, LMI_LAYER_SIZE):
            damage.append(Erase(str(i)))
        # slice 4
        damage.append(Erase(str(LMI_FIRSTCRC + 4)))
        _apply_damage(sim_iso, damage)

        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-a", "RS03", "-s", "-v",
        ]
        _run_golden_compare("scan_missing_header3", cmd, tmp_path)

    def test_scan_missing_header4(self, tmp_path):
        """Scan large image with missing header; every CRC sector except last is unreadable."""
        large = _ensure_large_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(large, sim_iso)

        damage = [Erase(str(LMI_HEADER))]
        # slice 0
        damage.append(Erase(str(LMI_FIRSTCRC)))
        for i in range(100 * LMI_LAYER_SIZE, 236 * LMI_LAYER_SIZE, LMI_LAYER_SIZE):
            damage.append(Erase(str(i)))
        # slice 1
        damage.append(Byteset(LMI_FIRSTCRC + 1, 500, 0))
        for i in range(110 * LMI_LAYER_SIZE + 1, 141 * LMI_LAYER_SIZE + 1, LMI_LAYER_SIZE):
            damage.append(Erase(str(i)))
        # slice 2
        damage.append(Erase(str(LMI_FIRSTCRC + 2)))
        for i in range(110 * LMI_LAYER_SIZE + 2, 141 * LMI_LAYER_SIZE + 2, LMI_LAYER_SIZE):
            damage.append(Erase(str(i)))
        # slice 3
        damage.append(Byteset(LMI_FIRSTCRC + 3, 500, 0))
        for i in range(110 * LMI_LAYER_SIZE + 3, 140 * LMI_LAYER_SIZE + 3, LMI_LAYER_SIZE):
            damage.append(Erase(str(i)))
        # slices 4-1407
        damage.append(Erase("{}-{}".format(LMI_FIRSTCRC + 4, LMI_FIRSTCRC + 1407)))
        _apply_damage(sim_iso, damage)

        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-a", "RS03", "-s", "-v",
        ]
        _run_golden_compare("scan_missing_header4", cmd, tmp_path)

    def test_scan_missing_header_truncated(self, tmp_path):
        """Scan truncated large image with missing header (like header2 but truncated)."""
        large = _ensure_large_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(large, sim_iso)

        damage = [Erase(str(LMI_HEADER)), Erase(str(LMI_FIRSTCRC))]
        for i in range(120 * LMI_LAYER_SIZE, 136 * LMI_LAYER_SIZE, LMI_LAYER_SIZE):
            damage.append(Erase(str(i)))
        damage.append(Truncate(300000))
        _apply_damage(sim_iso, damage)

        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-a", "RS03", "-s", "-v",
        ]
        _run_golden_compare("scan_missing_header_truncated", cmd, tmp_path)

    def test_scan_missing_header_no_crcsec(self, tmp_path):
        """Scan large image with missing header and all CRC sectors deleted."""
        large = _ensure_large_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(large, sim_iso)
        _apply_damage(sim_iso, [
            Erase(str(LMI_HEADER)),
            Erase("{}-{}".format(LMI_FIRSTCRC, LMI_FIRSTCRC + 1408)),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-a", "RS03", "-s", "-v",
        ]
        _run_golden_compare("scan_missing_header_no_crcsec", cmd, tmp_path)

    def test_scan_random_image(self, tmp_path):
        """Scan completely random image (no ECC)."""
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        _run_dvdisaster("--debug", "-i{}".format(sim_iso),
                        "--random-image", "359295")
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-a", "RS03", "-s", "-v",
        ]
        _run_golden_compare("scan_random_image", cmd, tmp_path)

    def test_scan_rediscover_8_roots(self, tmp_path):
        """Scan 8-root image with no ECC header (single missing sector)."""
        raw = _ensure_raw_lmi246()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(raw, sim_iso)
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(sim_iso), "-mRS03", "-c", "-x", "2")
        _apply_damage(sim_iso, [Erase("346612")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-a", "RS03", "-s", "-v",
        ]
        _run_golden_compare("scan_rediscover_8_roots", cmd, tmp_path)

    def test_scan_rediscover_8_roots2(self, tmp_path):
        """Scan 8-root image with no ECC header (header and some CRC sectors missing)."""
        raw = _ensure_raw_lmi246()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(raw, sim_iso)
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(sim_iso), "-mRS03", "-c", "-x", "2")
        _apply_damage(sim_iso, [Erase("346612-346620")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-a", "RS03", "-s", "-v",
        ]
        _run_golden_compare("scan_rediscover_8_roots2", cmd, tmp_path)

    def test_scan_rediscover_170_roots(self, tmp_path):
        """Scan 170-root image with no ECC header (single missing sector)."""
        raw = _ensure_raw_lmi84()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(raw, sim_iso)
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(sim_iso), "-mRS03", "-c", "-x", "2")
        _apply_damage(sim_iso, [Erase("118354")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-a", "RS03", "-s", "-v",
        ]
        _run_golden_compare("scan_rediscover_170_roots", cmd, tmp_path)

    def test_scan_rediscover_170_roots2(self, tmp_path):
        """Scan 170-root image with no ECC header (header and some CRC sectors missing)."""
        raw = _ensure_raw_lmi84()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(raw, sim_iso)
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(sim_iso), "-mRS03", "-c", "-x", "2")
        _apply_damage(sim_iso, [Erase("118354-118360")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-a", "RS03", "-s", "-v",
        ]
        _run_golden_compare("scan_rediscover_170_roots2", cmd, tmp_path)

    def test_scan_rediscover_170_roots_padding(self, tmp_path):
        """Scan 170-root padded image with ECC header present."""
        raw = _ensure_raw_lmi84s()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(raw, sim_iso)
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(sim_iso), "-mRS03", "-c", "-x", "2")
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-a", "RS03", "-s", "-v",
        ]
        _run_golden_compare("scan_rediscover_170_roots-padding", cmd, tmp_path)

    def test_scan_rediscover_170_roots_padding2(self, tmp_path):
        """Scan 170-root padded image with no ECC header and some CRC sectors missing."""
        raw = _ensure_raw_lmi84s()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(raw, sim_iso)
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(sim_iso), "-mRS03", "-c", "-x", "2")
        _apply_damage(sim_iso, [Erase("112354"), Erase("118356-118360")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-a", "RS03", "-s", "-v",
        ]
        _run_golden_compare("scan_rediscover_170_roots-padding2", cmd, tmp_path)

    def test_scan_with_crc_error_in_padding(self, tmp_path):
        """Scan image with CRC error in padding area."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Byteset(21020, 400, 255)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-a", "RS03", "-s",
        ]
        _run_golden_compare("scan_with_crc_error_in_padding", cmd, tmp_path)


# ===========================================================================
# Reading tests (linear) (29)
# ===========================================================================

class TestRS03iReadLinear:

    def test_read_good(self, tmp_path):
        """Read complete / optimal image."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_good", cmd, tmp_path, image_path=tmp_iso)

    def test_read_good_verbose(self, tmp_path):
        """Read complete / optimal image, verbose output."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "-v",
        ]
        _run_golden_compare("read_good_verbose", cmd, tmp_path, image_path=tmp_iso)

    def test_read_good_file(self, tmp_path):
        """Read into existing and complete image file."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        shutil.copy2(master, tmp_iso)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_good_file", cmd, tmp_path, image_path=tmp_iso)

    def test_read_shorter(self, tmp_path):
        """Read image shorter than expected."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Truncate(REAL_ECCSIZE - 44)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_shorter", cmd, tmp_path, image_path=tmp_iso)

    def test_read_longer(self, tmp_path):
        """Read image longer than expected; returns image in original length."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _append_fixed_random_sequence(sim_iso, times=23)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "-v",
        ]
        _run_golden_compare("read_longer", cmd, tmp_path, image_path=tmp_iso)

    def test_read_tao_tail(self, tmp_path):
        """Read image with two multisession link sectors appended (TAO tail)."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _append_fixed_random_sequence(sim_iso, times=1)
        _apply_damage(sim_iso, [Erase("24990-24991")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_tao_tail", cmd, tmp_path, image_path=tmp_iso)

    def test_read_no_tao_tail(self, tmp_path):
        """Read image with two real sectors missing at end; --dao prevents clipping."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _append_fixed_random_sequence(sim_iso, times=1)
        _apply_damage(sim_iso, [Erase("24988-24989")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "--dao",
        ]
        _run_golden_compare("read_no_tao_tail", cmd, tmp_path, image_path=tmp_iso)

    def test_read_incompatible_ecc(self, tmp_path):
        """Read image requiring a newer dvdisaster version."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [
            Byteset(21000, 84, 220),
            Byteset(21000, 85, 65),
            Byteset(21000, 86, 15),
            Byteset(21000, 88, 220),
            Byteset(21000, 89, 65),
            Byteset(21000, 90, 15),
            Byteset(21000, 96, 208),
            Byteset(21000, 97, 125),
            Byteset(21000, 98, 164),
            Byteset(21000, 99, 44),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_incompatible_ecc", cmd, tmp_path,
                            image_path=tmp_iso, ignore_line_re=r'^\*          $')

    def test_read_bad_header(self, tmp_path):
        """Read image with a defective ECC header; treated as ECC-less."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Byteset(21000, 1, 1)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_bad_header", cmd, tmp_path, image_path=tmp_iso)

    def test_read_bad_header_exhaustive(self, tmp_path):
        """Read image with a defective ECC header; exhaustive search enabled."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Byteset(21000, 1, 1)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--debug", "--spinup-delay=0", "-r", "-v", "-aRS03",
            "-n{}".format(ECCSIZE),
        ]
        _run_golden_compare("read_bad_header_exhaustive", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_missing_data_sectors(self, tmp_path):
        """Read image with missing sectors in data portion."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [
            Erase("1000-1049"),
            Erase("21230"),
            Erase("22450-22457"),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_missing_data_sectors", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_missing_crc_sectors(self, tmp_path):
        """Read image with missing sectors in CRC portion."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [
            Erase("21077"),
            Erase("21100-21120"),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_missing_crc_sectors", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_missing_ecc_sectors(self, tmp_path):
        """Read image with missing sectors in ECC portion."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [
            Erase("21200"),
            Erase("21340-21365"),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_missing_ecc_sectors", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_missing_iso_header(self, tmp_path):
        """Read image with missing ISO header; exhaustive search, verbose."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Erase("16")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "-v", "-aRS03", "-n{}".format(ECCSIZE),
        ]
        _run_golden_compare("read_missing_iso_header", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_data_bad_byte(self, tmp_path):
        """Read image with bad bytes in data section."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [
            Byteset(0, 50, 10),
            Byteset(1235, 50, 10),
            Byteset(20999, 50, 10),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_data_bad_byte", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_crc_bad_byte(self, tmp_path):
        """Read image with bad byte in CRC section."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Byteset(21077, 50, 10)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_crc_bad_byte", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_ecc_bad_byte(self, tmp_path):
        """Read image with bad byte in ECC section."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Byteset(22000, 50, 10)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_ecc_bad_byte", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_with_rs01_file(self, tmp_path):
        """Read augmented image protected by outer RS01 ecc file."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03i-tmp.ecc")
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(master), "-e{}".format(tmp_ecc),
                        "-c", "-n", "normal")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Byteset(24989, 0, 1)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_with_rs01_file", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_with_wrong_rs01_file(self, tmp_path):
        """Read augmented image with non-matching RS01 ecc file."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03i-tmp.ecc")
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(master), "-e{}".format(tmp_ecc),
                        "-c", "-n", "normal")
        _apply_damage(tmp_ecc, [Byteset(0, 24, 1)])
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Byteset(24989, 0, 1)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_with_wrong_rs01_file", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_with_rs03_file(self, tmp_path):
        """Read augmented image protected by outer RS03 ecc file."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03i-tmp.ecc")
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(master), "-e{}".format(tmp_ecc),
                        "-mRS03", "-c", "-n", "20r", "-o", "file")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Byteset(24989, 0, 1)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_with_rs03_file", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_with_wrong_rs03_file(self, tmp_path):
        """Read augmented image with non-matching RS03 ecc file."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs03i-tmp.ecc")
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(master), "-e{}".format(tmp_ecc),
                        "-mRS03", "-c", "-n", "20r", "-o", "file")
        _apply_damage(tmp_ecc, [Byteset(0, 24, 1)])
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Byteset(24989, 0, 1)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_with_wrong_rs03_file", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_crc_section_with_uncorrectable_dsm(self, tmp_path):
        """Read augmented image with uncorrectable dead sector markers in CRC area."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [
            Erase("21077:pass as dead sector marker"),
            Erase("21081:pass as dead sector marker"),
            Erase("21082:pass as dead sector marker"),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_crc_section_with_uncorrectable_dsm", cmd,
                            tmp_path, image_path=tmp_iso)

    def test_read_with_missing_header(self, tmp_path):
        """Read large image with missing ECC header; no exhaustive search."""
        large = _ensure_large_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(large, sim_iso)
        damage = [Erase(str(LMI_HEADER)), Erase(str(LMI_FIRSTCRC))]
        for i in range(120 * LMI_LAYER_SIZE, 136 * LMI_LAYER_SIZE, LMI_LAYER_SIZE):
            damage.append(Erase(str(i)))
        _apply_damage(sim_iso, damage)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_with_missing_header", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_with_missing_header_exhaustive(self, tmp_path):
        """Read large image with missing ECC header; exhaustive search."""
        large = _ensure_large_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(large, sim_iso)
        damage = [Erase(str(LMI_HEADER)), Erase(str(LMI_FIRSTCRC))]
        for i in range(120 * LMI_LAYER_SIZE, 136 * LMI_LAYER_SIZE, LMI_LAYER_SIZE):
            damage.append(Erase(str(i)))
        _apply_damage(sim_iso, damage)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "-v", "-a", "RS03",
        ]
        _run_golden_compare("read_with_missing_header_exhaustive", cmd,
                            tmp_path, image_path=tmp_iso)

    def test_read_with_missing_iso_header_exhaustive(self, tmp_path):
        """Read large image with missing ISO header; exhaustive search."""
        large = _ensure_large_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(large, sim_iso)
        _apply_damage(sim_iso, [Erase("16")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "-v", "-a", "RS03",
        ]
        _run_golden_compare("read_with_missing_iso_header_exhaustive", cmd,
                            tmp_path, image_path=tmp_iso)

    def test_read_with_ecc_file_header(self, tmp_path):
        """Read image with ECC header from an ecc file (ecc file bit set)."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [
            Byteset(21000, 16, 2),
            Byteset(21000, 96, 142),
            Byteset(21000, 97, 43),
            Byteset(21000, 98, 137),
            Byteset(21000, 99, 29),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "-v", "-a", "RS03", "-n", str(ECCSIZE),
        ]
        _run_golden_compare("read_with_ecc_file_header", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_with_ecc_file_crc_block(self, tmp_path):
        """Read image with defective ECC header and CRC block from an ecc file."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [
            Erase("21000"),
            Byteset(21070, 1040, 2),
            Byteset(21070, 1120, 208),
            Byteset(21070, 1121, 250),
            Byteset(21070, 1122, 142),
            Byteset(21070, 1123, 101),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "-v", "-a", "RS03", "-n", str(ECCSIZE),
        ]
        _run_golden_compare("read_with_ecc_file_crc_block", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_second_pass_with_crc_error(self, tmp_path):
        """Re-read incomplete image to verify CRC errors are still detected."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Byteset(15830, 8, 3)])
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [Erase("15800-16199")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_second_pass_with_crc_error", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_with_crc_error_in_padding(self, tmp_path):
        """Read image with CRC error in padding area."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs03i-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Byteset(21020, 400, 255)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso),
            "--debug", "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_with_crc_error_in_padding", cmd, tmp_path,
                            image_path=tmp_iso)
