"""
RS02 regression tests -- all tests from regtest/rs02.bash.

RS02 is an augmented image codec: ECC data is embedded directly in the image
(no separate .ecc file). Tests are grouped into:
  - TestRS02Strip: 2 strip tests
  - TestRS02Verify: 31 verify tests (incl. modulo_glitch and cross-codec)
  - TestRS02Create: 18 creation tests
  - TestRS02Repair: 25 repair/fix tests
  - TestRS02Scan: 22 scanning tests
  - TestRS02ReadLinear: 28 linear reading tests (read_multipass_ecc_partial_success
    is in test_multipass_read.py)
  - TestRS02ReadAdaptive: 24 adaptive reading tests
"""

import difflib
import os
import shutil

import pytest

from framework import (
    Byteset,
    Erase,
    GoldenTest,
    GoldenTestSuite,
    AppendFile,
    PadSectors,
    SimCD,
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
ISOSIZE = 30000
ECCSIZE = 35000
REAL_ECCSIZE = 34932
SETVERSION = "0.80"
REDUNDANCY = "normal"

# Header positions used by modulo_glitch tests (21 headers)
HMG_HEADERS = [
    274300, 278528, 282624, 286720, 290816, 294912, 299008, 303104,
    307200, 311296, 315392, 319488, 323584, 327680, 331776, 335872,
    339968, 344064, 348160, 352256, 356352,
]

# Sectors to erase in modulo_glitch4 (disambiguating sectors)
HMG_DISAMBIG_SECTORS = [
    276480, 280577, 284672, 288768, 292864, 296960, 301056, 305152,
    309248, 313344, 317440, 321536, 325632, 329728, 333824, 337920,
    342016, 346112, 350208,
]


# ──────────────────────────────────────────────────────────────────
# Helper: Ensure raw (unaugmented) image exists
# ──────────────────────────────────────────────────────────────────

def _ensure_raw_image():
    """Create the raw (unaugmented) 30000-sector image in ISODIR."""
    raw_path = os.path.join(_ISODIR, "rs02-raw.iso")
    if os.path.isfile(raw_path):
        return raw_path
    _run_dvdisaster(
        "--regtest", "--debug",
        "-i{}".format(raw_path),
        "--random-image", str(ISOSIZE),
    )
    return raw_path


def _ensure_master():
    """Create the RS02 augmented master image (raw + RS02 ECC)."""
    master_path = os.path.join(_ISODIR, "rs02-master.iso")
    if os.path.isfile(master_path):
        return master_path
    raw_path = _ensure_raw_image()
    shutil.copy2(raw_path, master_path)
    _run_dvdisaster(
        "--regtest", "--debug", "--set-version", SETVERSION,
        "-i{}".format(master_path),
        "-mRS02", "-n{}".format(ECCSIZE), "-c",
    )
    return master_path


def _ensure_plus137():
    """Create master image with 137 trailing bytes."""
    path = os.path.join(_ISODIR, "rs02-plus137.iso")
    if os.path.isfile(path):
        return path
    raw_path = _ensure_raw_image()
    shutil.copy2(raw_path, path)
    # Append 137 bytes from fixed-random-sequence
    with open(_FIXED_RANDOM_SEQ, "rb") as f:
        data = f.read(137)
    with open(path, "ab") as f:
        f.write(data)
    _run_dvdisaster(
        "--regtest", "--debug", "--set-version", SETVERSION,
        "-i{}".format(path),
        "-mRS02", "-n{}".format(ECCSIZE), "-c",
    )
    return path


def _ensure_hmg_master():
    """Create header-modulo-glitch master image (274300 sectors)."""
    path = os.path.join(_ISODIR, "rs02-hmg-master.iso")
    if os.path.isfile(path):
        return path
    _run_dvdisaster(
        "--debug", "-i", path,
        "--random-image", "274300",
    )
    _run_dvdisaster(
        "--regtest", "--debug", "--set-version", SETVERSION,
        "-i{}".format(path),
        "-mRS02", "-c",
    )
    return path


def _apply_old_style_headers(image_path):
    """Patch all 21 ECC headers to simulate pre-0.79.5 (old style) format.

    This zeroes out the size info bytes and patches the self-checksum in each
    header to simulate headers created by dvdisaster 0.72 or earlier.
    """
    for header in HMG_HEADERS:
        # Patch self-checksum
        for offset, val in [(96, 38), (97, 245), (98, 168), (99, 221)]:
            _run_dvdisaster("--debug", "-i", image_path,
                            "--byteset", "{},{},{}".format(header, offset, val))
        # Zero out size info
        for offset in range(128, 136):
            _run_dvdisaster("--debug", "-i", image_path,
                            "--byteset", "{},{},0".format(header, offset))


# ──────────────────────────────────────────────────────────────────
# Helper: golden file comparison for plain-method tests
# ──────────────────────────────────────────────────────────────────

def _run_golden_compare(test_name, cmd_args, tmp_path, image_path=None,
                        ignore_line=None):
    """Run dvdisaster and compare against a golden file."""
    golden_base = os.path.join(_DATABASE, "RS02_{}".format(test_name))
    golden_path = resolve_golden_path(golden_base)
    if not os.path.isfile(golden_path):
        pytest.skip("Golden file not found: {}".format(golden_path))

    image_md5, ecc_md5, expected_output = parse_golden_file(golden_path)

    _, raw_output = _run_dvdisaster(*cmd_args)

    work_dir = str(tmp_path)
    cleaned = clean_output(
        raw_output,
        tmp_dirs=[work_dir, _TMPDIR, _ISODIR],
        strip_header=True,
        ignore_lines=[ignore_line] if ignore_line else None,
    )

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

    if image_md5 is not None and image_path and os.path.isfile(image_path):
        actual_md5 = _md5_file(image_path)
        assert actual_md5 == image_md5, (
            "Image MD5 mismatch for '{}': expected {}, got {}".format(
                test_name, image_md5, actual_md5
            )
        )

    if ecc_md5 is not None:
        # For RS02, ecc is embedded in the image — check if there's a
        # separate ecc file that should be verified
        ecc_path = os.path.join(_TMPDIR, "rs02-tmp.ecc")
        if os.path.isfile(ecc_path):
            actual_ecc_md5 = _md5_file(ecc_path)
            assert actual_ecc_md5 == ecc_md5, (
                "ECC MD5 mismatch for '{}': expected {}, got {}".format(
                    test_name, ecc_md5, actual_ecc_md5
                )
            )


# ══════════════════════════════════════════════════════════════════
# Strip tests (2)
# ══════════════════════════════════════════════════════════════════

class TestRS02Strip(GoldenTestSuite):
    codec = "RS02"
    codec_prefix = "RS02"
    master = "rs02-master.iso"
    image_size = ISOSIZE
    tests = []

    def _ensure_master(self):
        return _ensure_master()

    def test_strip_ecc(self, tmp_path):
        """Strip ECC from an augmented image."""
        master_path = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(master_path, tmp_iso)
        _run_golden_compare("strip_ecc",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso), "-v", "--strip"],
                            tmp_path, image_path=tmp_iso)

    def test_strip_ecc_not(self, tmp_path):
        """Strip ECC from a non-augmented (already stripped) image."""
        master_path = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(master_path, tmp_iso)
        # First strip the ECC
        _run_dvdisaster("-i{}".format(tmp_iso), "--strip")
        # Then try to strip again
        _run_golden_compare("strip_ecc_not",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso), "-v", "--strip"],
                            tmp_path, image_path=tmp_iso)


# ══════════════════════════════════════════════════════════════════
# Verify tests (31)
# ══════════════════════════════════════════════════════════════════

class TestRS02Verify(GoldenTestSuite):
    codec = "RS02"
    codec_prefix = "RS02"
    master = "rs02-master.iso"
    image_size = ISOSIZE

    tests = [
        # Simple verify tests
        GoldenTest("good", action="-t -v", use_master=True),
        GoldenTest("good_quick", action="-tq", use_master=True),
        GoldenTest("no_image", action="-t", image="no.iso", ecc="none.file"),
        GoldenTest("truncated", action="-t",
                   damage=[Truncate(REAL_ECCSIZE - 5)]),
        GoldenTest("plus1", action="-t",
                   damage=[PadSectors(1)]),
        GoldenTest("plus17", action="-t",
                   damage=[PadSectors(17)]),
        # Defective headers
        GoldenTest("bad_header", action="-t",
                   damage=[Byteset(30592, 1, 1)]),
        GoldenTest("bad_headers", action="-t",
                   damage=[Byteset(30592, 1, 1), Byteset(31488, 100, 1)]),
        GoldenTest("missing_headers", action="-t",
                   damage=[Erase("30080"), Erase("31360"), Erase("34816"),
                           Byteset(30592, 100, 1), Byteset(31488, 100, 1)]),
        # Missing sectors in data/crc/ecc portions
        GoldenTest("missing_data_sectors", action="-t",
                   damage=[Erase("1000-1049"), Erase("21230"),
                           Erase("22450-22457")]),
        GoldenTest("missing_crc_sectors", action="-t",
                   damage=[Erase("30020-30030"), Erase("30034")]),
        GoldenTest("missing_ecc_sectors", action="-t",
                   damage=[Erase("32020-32030"), Erase("33034")]),
        # Bad bytes in data/crc/ecc
        GoldenTest("data_bad_byte", action="-t",
                   damage=[Byteset(1235, 50, 10)]),
        GoldenTest("crc_bad_byte", action="-t",
                   damage=[Byteset(30020, 50, 10)]),
        GoldenTest("ecc_bad_byte", action="-t",
                   damage=[Byteset(33100, 50, 10)]),
        # ECC offset tests
        GoldenTest("good_0_offset", action="-v -t", use_master=True),
        # DSM tests
        GoldenTest("uncorrectable_dsm_in_image", action="-t",
                   damage=[Erase("3030"), Byteset(3030, 353, 49),
                           Erase("4400"), Byteset(4400, 353, 53),
                           Erase("4411"), Byteset(4411, 353, 53)]),
        GoldenTest("uncorrectable_dsm_in_image_verbose", action="-t -v",
                   damage=[Erase("3030"), Byteset(3030, 353, 49),
                           Erase("4400"), Byteset(4400, 353, 53),
                           Erase("4411"), Byteset(4411, 353, 53)]),
        GoldenTest("uncorrectable_dsm_in_image2", action="-t",
                   damage=[Erase("3030"), Byteset(3030, 416, 55),
                           Byteset(3030, 556, 32), Byteset(3030, 557, 50),
                           Erase("4400"), Byteset(4400, 416, 53),
                           Byteset(4400, 556, 32), Byteset(4400, 557, 50),
                           Erase("4411"), Byteset(4411, 416, 53),
                           Byteset(4411, 556, 32), Byteset(4411, 557, 50)]),
        GoldenTest("uncorrectable_dsm_in_image2_verbose", action="-t -v",
                   damage=[Erase("3030"), Byteset(3030, 416, 55),
                           Byteset(3030, 556, 32), Byteset(3030, 557, 50),
                           Erase("4400"), Byteset(4400, 416, 53),
                           Byteset(4400, 556, 32), Byteset(4400, 557, 50),
                           Erase("4411"), Byteset(4411, 416, 53),
                           Byteset(4411, 556, 32), Byteset(4411, 557, 50)]),
        # DSM in CRC section
        GoldenTest("uncorrectable_dsm_in_image3", action="-t",
                   damage=[Erase("30030"), Byteset(30030, 416, 55),
                           Byteset(30030, 556, 32), Byteset(30030, 557, 50),
                           Erase("30031"), Byteset(30031, 416, 53),
                           Byteset(30031, 556, 32), Byteset(30031, 557, 50),
                           Erase("30032"), Byteset(30032, 416, 53),
                           Byteset(30032, 556, 32), Byteset(30032, 557, 50)]),
    ]

    def _ensure_master(self):
        return _ensure_master()

    # -- Tests requiring special setup (plain methods) --

    def test_good_150_offset(self, tmp_path):
        """Good image with 150 sectors ECC offset."""
        raw_path = _ensure_raw_image()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(raw_path, tmp_iso)
        # Fake 150 more sectors in VSS
        _run_dvdisaster("--debug", "-i{}".format(tmp_iso),
                        "--byteset", "16,80,198")
        _run_dvdisaster("--debug", "-i{}".format(tmp_iso),
                        "--byteset", "16,87,198")
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(tmp_iso), "-mRS02",
                        "-n{}".format(ECCSIZE), "-c")
        _run_golden_compare("good_150_offset",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso), "-v", "-t"],
                            tmp_path, image_path=tmp_iso)

    def test_bad_master(self, tmp_path):
        """Image with missing master header."""
        master_path = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(master_path, tmp_iso)
        _run_dvdisaster("--debug", "-i{}".format(tmp_iso), "--erase", "30000")
        _run_golden_compare("bad_master",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso), "-v", "-t"],
                            tmp_path, image_path=tmp_iso)

    def test_modulo_glitch(self, tmp_path):
        """Header modulo glitch, post 0.79.5 style header."""
        hmg_path = _ensure_hmg_master()
        _run_golden_compare("modulo_glitch",
                            ["--regtest", "--no-progress",
                             "-i{}".format(hmg_path), "-v", "-t"],
                            tmp_path)

    def test_modulo_glitch2(self, tmp_path):
        """Header modulo glitch, old style, complete image."""
        hmg_path = _ensure_hmg_master()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(hmg_path, tmp_iso)
        _apply_old_style_headers(tmp_iso)
        _run_golden_compare("modulo_glitch2",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso), "-v", "-t"],
                            tmp_path, image_path=tmp_iso)

    def test_modulo_glitch3(self, tmp_path):
        """Header modulo glitch, old style, truncated image."""
        hmg_path = _ensure_hmg_master()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(hmg_path, tmp_iso)
        _apply_old_style_headers(tmp_iso)
        _run_dvdisaster("--debug", "-i", tmp_iso, "--truncate=357520")
        _run_golden_compare("modulo_glitch3",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso), "-v", "-t"],
                            tmp_path, image_path=tmp_iso)

    def test_modulo_glitch4(self, tmp_path):
        """Header modulo glitch, old style, truncated, missing ref sectors."""
        hmg_path = _ensure_hmg_master()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(hmg_path, tmp_iso)
        _apply_old_style_headers(tmp_iso)
        _run_dvdisaster("--debug", "-i", tmp_iso, "--truncate=357520")
        for sector in HMG_DISAMBIG_SECTORS:
            _run_dvdisaster("--debug", "-i", tmp_iso,
                            "--erase", str(sector))
        _run_golden_compare("modulo_glitch4",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso), "-v", "-t"],
                            tmp_path, image_path=tmp_iso)

    def test_with_rs01_file(self, tmp_path):
        """Augmented image with outer RS01 error correction file."""
        master_path = _ensure_master()
        tmp_ecc = os.path.join(str(tmp_path), "rs02-tmp.ecc")
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(master_path), "-e{}".format(tmp_ecc),
                        "-c", "-n", "normal")
        _run_golden_compare("with_rs01_file",
                            ["--regtest", "--no-progress",
                             "-i{}".format(master_path),
                             "-e{}".format(tmp_ecc), "-v", "-t"],
                            tmp_path)

    def test_with_wrong_rs01_file(self, tmp_path):
        """Augmented image with non-matching RS01 error correction file."""
        master_path = _ensure_master()
        tmp_ecc = os.path.join(str(tmp_path), "rs02-tmp.ecc")
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(master_path), "-e{}".format(tmp_ecc),
                        "-c", "-n", "normal")
        _run_dvdisaster("--debug", "-i{}".format(tmp_ecc),
                        "--byteset", "0,24,1")
        _run_golden_compare("with_wrong_rs01_file",
                            ["--regtest", "--no-progress",
                             "-i{}".format(master_path),
                             "-e{}".format(tmp_ecc), "-v", "-t"],
                            tmp_path)

    def test_with_rs03_file(self, tmp_path):
        """Augmented image with outer RS03 error correction file."""
        master_path = _ensure_master()
        tmp_ecc = os.path.join(str(tmp_path), "rs02-tmp.ecc")
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(master_path), "-e{}".format(tmp_ecc),
                        "-mRS03", "-c", "-n", "20r", "-o", "file")
        _run_golden_compare("with_rs03_file",
                            ["--regtest", "--no-progress",
                             "-i{}".format(master_path),
                             "-e{}".format(tmp_ecc), "-v", "-t"],
                            tmp_path)

    def test_with_wrong_rs03_file(self, tmp_path):
        """Augmented image with non-matching RS03 error correction file."""
        master_path = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs02-tmp.ecc")
        # Create image with manipulated fingerprint
        shutil.copy2(master_path, tmp_iso)
        _run_dvdisaster("--debug", "-i{}".format(tmp_iso),
                        "--byteset", "16,240,1")
        # Create ecc file for "wrong" image
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
                        "-mRS03", "-c", "-n", "20r", "-o", "file")
        # Test against original image
        _run_golden_compare("with_wrong_rs03_file",
                            ["--regtest", "--no-progress",
                             "-i{}".format(master_path),
                             "-e{}".format(tmp_ecc), "-v", "-t"],
                            tmp_path)


# ══════════════════════════════════════════════════════════════════
# Creation tests (18)
# ══════════════════════════════════════════════════════════════════

class TestRS02Create(GoldenTestSuite):
    codec = "RS02"
    codec_prefix = "RS02"
    master = "rs02-master.iso"
    image_size = ISOSIZE
    tests = []

    def _ensure_master(self):
        return _ensure_master()

    def _create_test(self, test_name, tmp_path, source="raw", action_args=None,
                     pre_damage=None, pre_cmds=None, image_path=None,
                     ecc_path=None):
        """Generic creation test helper."""
        if image_path is None:
            if source == "raw":
                src = _ensure_raw_image()
            else:
                src = _ensure_master()
            tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
            shutil.copy2(src, tmp_iso)
            image_path = tmp_iso

        if pre_damage:
            for op in pre_damage:
                args = op.cli_args()
                _run_dvdisaster("--debug", "-i{}".format(image_path), *args)

        if pre_cmds:
            for cmd in pre_cmds:
                cmd(image_path)

        cmd_args = ["--regtest", "--no-progress",
                    "-i{}".format(image_path)]
        if ecc_path:
            cmd_args.append("-e{}".format(ecc_path))
        cmd_args.extend(["-mRS02", "-n{}".format(ECCSIZE), "-c"])
        cmd_args.extend(["--debug", "--set-version", SETVERSION])
        if action_args:
            cmd_args.extend(action_args)

        _run_golden_compare(test_name, cmd_args, tmp_path,
                            image_path=image_path)

    def test_ecc_create(self, tmp_path):
        """Augmented image creation."""
        self._create_test("ecc_create", tmp_path)

    def test_ecc_missing_image(self, tmp_path):
        """ECC creation with missing image."""
        no_file = os.path.join(str(tmp_path), "none.file")
        _run_golden_compare("ecc_missing_image",
                            ["--regtest", "--no-progress",
                             "-i{}".format(no_file),
                             "-mRS02", "-n{}".format(ECCSIZE), "-c",
                             "--debug", "--set-version", SETVERSION],
                            tmp_path)

    def test_ecc_no_read_perm(self, tmp_path):
        """ECC creation with no read permission."""
        raw_path = _ensure_raw_image()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(raw_path, tmp_iso)
        os.chmod(tmp_iso, 0o000)
        try:
            _run_golden_compare("ecc_no_read_perm",
                                ["--regtest", "--no-progress",
                                 "-i{}".format(tmp_iso),
                                 "-mRS02", "-n{}".format(ECCSIZE), "-c",
                                 "--debug", "--set-version", SETVERSION],
                                tmp_path, image_path=tmp_iso)
        finally:
            os.chmod(tmp_iso, 0o644)

    def test_ecc_no_write_perm(self, tmp_path):
        """ECC creation with no write permission."""
        raw_path = _ensure_raw_image()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(raw_path, tmp_iso)
        os.chmod(tmp_iso, 0o400)
        try:
            _run_golden_compare("ecc_no_write_perm",
                                ["--regtest", "--no-progress",
                                 "-i{}".format(tmp_iso),
                                 "-mRS02", "-n{}".format(ECCSIZE), "-c",
                                 "--debug", "--set-version", SETVERSION],
                                tmp_path, image_path=tmp_iso)
        finally:
            os.chmod(tmp_iso, 0o644)

    def test_ecc_from_rs02(self, tmp_path):
        """ECC creation from already RS02-augmented image."""
        self._create_test("ecc_from_rs02", tmp_path, source="master")

    def test_ecc_from_rs03(self, tmp_path):
        """ECC creation from RS03-augmented image."""
        raw_path = _ensure_raw_image()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(raw_path, tmp_iso)
        _run_dvdisaster("--debug", "--set-version", SETVERSION,
                        "-i{}".format(tmp_iso), "-mRS03",
                        "-n{}".format(ECCSIZE), "-c")
        _run_golden_compare("ecc_from_rs03",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "-mRS02", "-n{}".format(ECCSIZE), "-c",
                             "--debug", "--set-version", SETVERSION],
                            tmp_path, image_path=tmp_iso)

    def test_ecc_from_larger_rs02(self, tmp_path):
        """ECC creation from RS02-augmented image with larger redundancy."""
        raw_path = _ensure_raw_image()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(raw_path, tmp_iso)
        _run_dvdisaster("--debug", "--set-version", SETVERSION,
                        "-i{}".format(tmp_iso), "-mRS02",
                        "-n{}".format(ECCSIZE + 5000), "-c")
        _run_golden_compare("ecc_from_larger_rs02",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "-mRS02", "-n{}".format(ECCSIZE), "-c",
                             "--debug", "--set-version", SETVERSION],
                            tmp_path, image_path=tmp_iso)

    def test_ecc_from_rs02_non_blocksize(self, tmp_path):
        """ECC creation from RS02-augmented image with non-block size."""
        raw_path = _ensure_raw_image()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(raw_path, tmp_iso)
        # Append 56 bytes of "1"
        with open(tmp_iso, "ab") as f:
            f.write(b"1" * 56)
        _run_dvdisaster("--debug", "--set-version", SETVERSION,
                        "-i{}".format(tmp_iso), "-mRS02",
                        "-n{}".format(ECCSIZE), "-c")
        _run_golden_compare("ecc_from_rs02_non_blocksize",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "-mRS02", "-n{}".format(ECCSIZE), "-c",
                             "--debug", "--set-version", SETVERSION],
                            tmp_path, image_path=tmp_iso)

    def test_ecc_from_rs03_non_blocksize(self, tmp_path):
        """ECC creation from RS03-augmented image with non-block size."""
        raw_path = _ensure_raw_image()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(raw_path, tmp_iso)
        # Append 137 bytes from fixed-random-sequence
        with open(_FIXED_RANDOM_SEQ, "rb") as f:
            data = f.read(137)
        with open(tmp_iso, "ab") as f:
            f.write(data)
        _run_dvdisaster("--debug", "--set-version", SETVERSION,
                        "-i{}".format(tmp_iso), "-mRS03",
                        "-n{}".format(ECCSIZE), "-c")
        _run_golden_compare("ecc_from_rs03_non_blocksize",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "-mRS02", "-n{}".format(ECCSIZE), "-c",
                             "-a", "RS02",
                             "--debug", "--set-version", SETVERSION],
                            tmp_path, image_path=tmp_iso)

    def test_ecc_from_larger_rs02_non_blocksize(self, tmp_path):
        """ECC creation from RS02-augmented image, non-block size, larger redundancy."""
        raw_path = _ensure_raw_image()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(raw_path, tmp_iso)
        with open(tmp_iso, "ab") as f:
            f.write(b"1" * 56)
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(tmp_iso), "-mRS02",
                        "-n{}".format(ECCSIZE + 5000), "-c")
        _run_golden_compare("ecc_from_larger_rs02_non_blocksize",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "-mRS02", "-n{}".format(ECCSIZE), "-c",
                             "--debug", "--set-version", SETVERSION],
                            tmp_path, image_path=tmp_iso)

    def test_ecc_non_blocksize(self, tmp_path):
        """ECC creation from non-blocksize image."""
        raw_path = _ensure_raw_image()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(raw_path, tmp_iso)
        # Append 137 zero bytes
        with open(tmp_iso, "ab") as f:
            f.write(b"\x00" * 137)
        _run_golden_compare("ecc_non_blocksize",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "-mRS02", "-n{}".format(ECCSIZE), "-c",
                             "--debug", "--set-version", SETVERSION],
                            tmp_path, image_path=tmp_iso)

    def test_ecc_missing_sectors(self, tmp_path):
        """ECC creation with unreadable sectors."""
        raw_path = _ensure_raw_image()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(raw_path, tmp_iso)
        _run_dvdisaster("--debug", "-i{}".format(tmp_iso), "--erase", "719")
        _run_golden_compare("ecc_missing_sectors",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "-mRS02", "-n{}".format(ECCSIZE), "-c",
                             "--debug", "--set-version", SETVERSION],
                            tmp_path, image_path=tmp_iso)

    def test_ecc_create_after_read(self, tmp_path):
        """ECC creation after reading image (read + create in one pass)."""
        raw_path = _ensure_raw_image()
        sim_iso = os.path.join(str(tmp_path), "rs02-sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(raw_path, sim_iso)
        _run_golden_compare("ecc_create_after_read",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "-r", "--spinup-delay=0",
                             "-mRS02", "-n{}".format(ECCSIZE), "-c", "-v",
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--set-version", SETVERSION],
                            tmp_path, image_path=tmp_iso)

    def test_ecc_create_after_partial_read(self, tmp_path):
        """ECC creation after completing a partial image."""
        raw_path = _ensure_raw_image()
        sim_iso = os.path.join(str(tmp_path), "rs02-sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        no_ecc = os.path.join(str(tmp_path), "no.ecc")
        shutil.copy2(raw_path, sim_iso)
        shutil.copy2(sim_iso, tmp_iso)
        _run_dvdisaster("--debug", "-i{}".format(tmp_iso),
                        "--erase", "3000-3999")
        _run_golden_compare("ecc_create_after_partial_read",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "-e{}".format(no_ecc),
                             "-r", "--spinup-delay=0",
                             "-mRS02", "-n{}".format(ECCSIZE), "-c", "-v",
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--set-version", SETVERSION],
                            tmp_path, image_path=tmp_iso)

    def test_ecc_recreate_after_read_rs01(self, tmp_path):
        """Read image with RS01 ECC and create RS02 ECC."""
        raw_path = _ensure_raw_image()
        sim_iso = os.path.join(str(tmp_path), "rs02-sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs02-tmp.ecc")
        shutil.copy2(raw_path, sim_iso)
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(sim_iso), "-e{}".format(tmp_ecc),
                        "-c", REDUNDANCY)
        _run_golden_compare("ecc_recreate_after_read_rs01",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "-e{}".format(tmp_ecc),
                             "-r", "--spinup-delay=0",
                             "-mRS02", "-n{}".format(ECCSIZE), "-c", "-v",
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--set-version", SETVERSION],
                            tmp_path, image_path=tmp_iso)

    def test_ecc_recreate_after_read_rs02(self, tmp_path):
        """Read image with RS02 ECC and create new RS02 ECC."""
        raw_path = _ensure_raw_image()
        sim_iso = os.path.join(str(tmp_path), "rs02-sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        no_ecc = os.path.join(str(tmp_path), "no.ecc")
        shutil.copy2(raw_path, sim_iso)
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(sim_iso), "-mRS02", "-n50000", "-c")
        _run_golden_compare("ecc_recreate_after_read_rs02",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "-e{}".format(no_ecc),
                             "-r", "--spinup-delay=0",
                             "-mRS02", "-n{}".format(ECCSIZE), "-c", "-v",
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--set-version", SETVERSION],
                            tmp_path, image_path=tmp_iso)

    def test_ecc_recreate_after_read_rs03i(self, tmp_path):
        """Read image with RS03i ECC and create RS02 ECC."""
        raw_path = _ensure_raw_image()
        sim_iso = os.path.join(str(tmp_path), "rs02-sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(raw_path, sim_iso)
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(sim_iso), "-mRS03",
                        "-n{}".format(ISOSIZE + 7000), "-c")
        _run_golden_compare("ecc_recreate_after_read_rs03i",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "-r", "--spinup-delay=0",
                             "-mRS02", "-n{}".format(ECCSIZE), "-c",
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--set-version", SETVERSION],
                            tmp_path, image_path=tmp_iso)

    def test_ecc_recreate_after_read_rs03f(self, tmp_path):
        """Read image with RS03f ECC and create RS02 ECC."""
        raw_path = _ensure_raw_image()
        sim_iso = os.path.join(str(tmp_path), "rs02-sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs02-tmp.ecc")
        shutil.copy2(raw_path, sim_iso)
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(sim_iso), "-e{}".format(tmp_ecc),
                        "-c", "-n", "9r", "-mRS03", "-o", "file")
        _run_golden_compare("ecc_recreate_after_read_rs03f",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "-e{}".format(tmp_ecc),
                             "-r", "--spinup-delay=0",
                             "-mRS02", "-n{}".format(ECCSIZE), "-c",
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--set-version", SETVERSION],
                            tmp_path, image_path=tmp_iso)


# ══════════════════════════════════════════════════════════════════
# Repair tests (25)
# ══════════════════════════════════════════════════════════════════

class TestRS02Repair(GoldenTestSuite):
    codec = "RS02"
    codec_prefix = "RS02"
    master = "rs02-master.iso"
    image_size = ISOSIZE

    tests = [
        # Simple fix tests using master as source
        GoldenTest("fix_good_image", action="--debug --set-version {} -f".format(SETVERSION)),
        GoldenTest("fix_bad_master", action="--debug --set-version {} -f".format(SETVERSION),
                   damage=[Erase("30000")]),
        GoldenTest("fix_bad_header", action="--debug --set-version {} -f".format(SETVERSION),
                   damage=[Byteset(30592, 1, 1)]),
        GoldenTest("fix_bad_headers", action="--debug --set-version {} -f".format(SETVERSION),
                   damage=[Byteset(30592, 1, 1), Byteset(31488, 100, 1)]),
        GoldenTest("fix_missing_headers", action="--debug --set-version {} -f".format(SETVERSION),
                   damage=[Erase("30080"), Erase("31360"), Erase("34816"),
                           Byteset(30592, 100, 1), Byteset(31488, 100, 1)]),
        GoldenTest("fix_missing_data_sectors",
                   action="--debug --set-version {} -f".format(SETVERSION),
                   damage=[Erase("1000-1049"), Erase("21230"),
                           Erase("22450-22457")]),
        GoldenTest("fix_missing_crc_sectors",
                   action="--debug --set-version {} -f".format(SETVERSION),
                   damage=[Erase("30020-30030"), Erase("30034")]),
        GoldenTest("fix_missing_ecc_sectors",
                   action="--debug --set-version {} -f".format(SETVERSION),
                   damage=[Erase("32020-32030"), Erase("33034")]),
        GoldenTest("fix_data_bad_byte",
                   action="--debug --set-version {} -f".format(SETVERSION),
                   damage=[Byteset(1235, 50, 10)]),
        GoldenTest("fix_crc_bad_byte",
                   action="--debug --set-version {} -f".format(SETVERSION),
                   damage=[Byteset(30020, 50, 10)]),
        GoldenTest("fix_ecc_bad_byte",
                   action="--debug --set-version {} -f".format(SETVERSION),
                   damage=[Byteset(33100, 50, 10)]),
        GoldenTest("fix_good_0_offset",
                   action="--debug --set-version {} -v -f".format(SETVERSION)),
    ]

    def _ensure_master(self):
        return _ensure_master()

    def test_fix_no_read_perm(self, tmp_path):
        """Fix with no read permission (uses raw image, not master)."""
        raw_path = _ensure_raw_image()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(raw_path, tmp_iso)
        os.chmod(tmp_iso, 0o000)
        try:
            _run_golden_compare("fix_no_read_perm",
                                ["--regtest", "--no-progress",
                                 "-i{}".format(tmp_iso),
                                 "--debug", "--set-version", SETVERSION, "-f"],
                                tmp_path, image_path=tmp_iso)
        finally:
            os.chmod(tmp_iso, 0o644)

    def test_fix_no_write_perm(self, tmp_path):
        """Fix with no write permission (uses raw image)."""
        raw_path = _ensure_raw_image()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(raw_path, tmp_iso)
        os.chmod(tmp_iso, 0o400)
        try:
            _run_golden_compare("fix_no_write_perm",
                                ["--regtest", "--no-progress",
                                 "-i{}".format(tmp_iso),
                                 "--debug", "--set-version", SETVERSION, "-f"],
                                tmp_path, image_path=tmp_iso)
        finally:
            os.chmod(tmp_iso, 0o644)

    def test_fix_image_plus137(self, tmp_path):
        """Fix image with 137 extra bytes."""
        plus137_path = _ensure_plus137()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(plus137_path, tmp_iso)
        _run_dvdisaster("-i{}".format(tmp_iso), "--debug", "--erase", "17000")
        _run_golden_compare("fix_image_plus137",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso), "-f"],
                            tmp_path, image_path=tmp_iso)

    def test_fix_image_error_in_plus137(self, tmp_path):
        """Fix image with error in 137 extra bytes."""
        plus137_path = _ensure_plus137()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(plus137_path, tmp_iso)
        _run_dvdisaster("-i{}".format(tmp_iso), "--debug",
                        "--byteset", "30000,111,111")
        _run_dvdisaster("-i{}".format(tmp_iso), "--debug",
                        "--byteset", "30000,500,123")
        _run_golden_compare("fix_image_error_in_plus137",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso), "-f"],
                            tmp_path, image_path=tmp_iso)

    def test_fix_truncated_image(self, tmp_path):
        """Fix a truncated image."""
        master_path = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(master_path, tmp_iso)
        trunc_size = REAL_ECCSIZE - 210
        _run_dvdisaster("--debug", "-i{}".format(tmp_iso),
                        "--truncate={}".format(trunc_size))
        _run_golden_compare("fix_truncated_image",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug", "--set-version", SETVERSION, "-f"],
                            tmp_path, image_path=tmp_iso)

    def test_fix_trailing_bytes(self, tmp_path):
        """Fix image with trailing bytes."""
        master_path = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(master_path, tmp_iso)
        with open(tmp_iso, "ab") as f:
            f.write(b"some trailing garbage appended for testing\n")
        _run_golden_compare("fix_trailing_bytes",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug", "--set-version", SETVERSION, "-f"],
                            tmp_path, image_path=tmp_iso)

    def test_fix_trailing_tao(self, tmp_path):
        """Fix image with trailing TAO garbage."""
        master_path = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(master_path, tmp_iso)
        with open(tmp_iso, "ab") as f:
            f.write(b"\x00" * (2 * 2048))
        _run_golden_compare("fix_trailing_tao",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug", "--set-version", SETVERSION, "-f"],
                            tmp_path, image_path=tmp_iso)

    def test_fix_trailing_garbage(self, tmp_path):
        """Fix image with trailing garbage (general case)."""
        master_path = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(master_path, tmp_iso)
        with open(tmp_iso, "ab") as f:
            f.write(b"\x00" * (23 * 2048))
        _run_golden_compare("fix_trailing_garbage",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug", "--set-version", SETVERSION, "-f"],
                            tmp_path, image_path=tmp_iso)

    def test_fix_trailing_garbage2(self, tmp_path):
        """Fix image with trailing garbage, with --truncate."""
        master_path = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(master_path, tmp_iso)
        with open(tmp_iso, "ab") as f:
            f.write(b"\x00" * (23 * 2048))
        _run_golden_compare("fix_trailing_garbage2",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug", "--set-version", SETVERSION,
                             "-f", "--truncate"],
                            tmp_path, image_path=tmp_iso)

    def test_fix_large_file(self, tmp_path):
        """Large image with missing sectors in all three sections."""
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        _run_dvdisaster("--debug", "-i{}".format(tmp_iso),
                        "--random-image", "223456")
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(tmp_iso), "-mRS02", "-c")
        _run_dvdisaster("-i{}".format(tmp_iso), "--debug",
                        "--erase", "50000-50015")
        _run_dvdisaster("-i{}".format(tmp_iso), "--debug",
                        "--erase", "223600-223605")
        _run_dvdisaster("-i{}".format(tmp_iso), "--debug",
                        "--erase", "279000-279007")
        _run_golden_compare("fix_large_file",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso), "-f"],
                            tmp_path, image_path=tmp_iso)

    def test_fix_good_150_offset(self, tmp_path):
        """Good image with 150 sectors ECC offset."""
        raw_path = _ensure_raw_image()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(raw_path, tmp_iso)
        _run_dvdisaster("--debug", "-i{}".format(tmp_iso),
                        "--byteset", "16,80,198")
        _run_dvdisaster("--debug", "-i{}".format(tmp_iso),
                        "--byteset", "16,87,198")
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(tmp_iso), "-mRS02",
                        "-n{}".format(ECCSIZE), "-c")
        _run_golden_compare("fix_good_150_offset",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug", "--set-version", SETVERSION,
                             "-v", "-f"],
                            tmp_path, image_path=tmp_iso)

    def test_fix_with_rs01_file(self, tmp_path):
        """RS02 image with RS01 ecc file."""
        master_path = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs02-tmp.ecc")
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(master_path), "-e{}".format(tmp_ecc),
                        "-c", "-n", "normal")
        shutil.copy2(master_path, tmp_iso)
        _run_dvdisaster("--debug", "-i{}".format(tmp_iso),
                        "--byteset", "34930,0,1")
        _run_golden_compare("fix_with_rs01_file",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "-e{}".format(tmp_ecc),
                             "--debug", "--set-version", SETVERSION, "-f"],
                            tmp_path, image_path=tmp_iso)

    def test_fix_with_rs03_file(self, tmp_path):
        """RS02 image with RS03 error correction file."""
        master_path = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs02-tmp.ecc")
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(master_path), "-e{}".format(tmp_ecc),
                        "-mRS03", "-c", "-n", "20r", "-o", "file")
        shutil.copy2(master_path, tmp_iso)
        _run_dvdisaster("--debug", "-i{}".format(tmp_iso),
                        "--byteset", "34930,0,1")
        _run_golden_compare("fix_with_rs03_file",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "-e{}".format(tmp_ecc),
                             "--debug", "--set-version", SETVERSION, "-f"],
                            tmp_path, image_path=tmp_iso)


# ══════════════════════════════════════════════════════════════════
# Scanning tests (22)
# ══════════════════════════════════════════════════════════════════

class TestRS02Scan(GoldenTestSuite):
    codec = "RS02"
    codec_prefix = "RS02"
    master = "rs02-master.iso"
    image_size = ISOSIZE

    tests = [
        GoldenTest("scan_good", action="-s",
                   sim_cd=SimCD(source="master"),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("scan_shorter", action="-s -v",
                   sim_cd=SimCD(source="master",
                                damage=[Truncate(REAL_ECCSIZE - 44)]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("scan_tao_tail", action="-s",
                   sim_cd=SimCD(source="master",
                                damage=[AppendFile(_FIXED_RANDOM_SEQ),
                                        Erase("34932-34933")]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("scan_no_tao_tail", action="-s --dao",
                   sim_cd=SimCD(source="master",
                                damage=[Erase("34930-34931")]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("scan_bad_header", action="-s",
                   sim_cd=SimCD(source="master",
                                damage=[Byteset(30592, 1, 1)]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("scan_bad_headers", action="-s",
                   sim_cd=SimCD(source="master",
                                damage=[Byteset(30592, 1, 1),
                                        Byteset(31488, 100, 1)]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("scan_missing_data_sectors", action="-s",
                   sim_cd=SimCD(source="master",
                                damage=[Erase("1000-1049"), Erase("21230"),
                                        Erase("22450-22457")]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("scan_missing_crc_sectors", action="-s",
                   sim_cd=SimCD(source="master",
                                damage=[Erase("30020-30030"),
                                        Erase("30034")]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("scan_missing_ecc_sectors", action="-s",
                   sim_cd=SimCD(source="master",
                                damage=[Erase("32020-32030"),
                                        Erase("33034")]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("scan_data_bad_byte", action="-s",
                   sim_cd=SimCD(source="master",
                                damage=[Byteset(1235, 50, 10)]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("scan_crc_bad_byte", action="-s",
                   sim_cd=SimCD(source="master",
                                damage=[Byteset(30020, 50, 10)]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("scan_ecc_bad_byte", action="-s",
                   sim_cd=SimCD(source="master",
                                damage=[Byteset(33100, 50, 10)]),
                   extra_args=["--spinup-delay=0"]),
    ]

    def _ensure_master(self):
        return _ensure_master()

    def test_scan_longer(self, tmp_path):
        """Scan image longer than expected."""
        master_path = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        shutil.copy2(master_path, sim_iso)
        with open(_FIXED_RANDOM_SEQ, "rb") as f:
            pad_data = f.read()
        with open(sim_iso, "ab") as f:
            for _ in range(23):
                f.write(pad_data)
        _run_golden_compare("scan_longer",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-s", "-v"],
                            tmp_path, image_path=tmp_iso)

    def test_scan_incompatible_ecc(self, tmp_path):
        """Scan image requiring a newer dvdisaster version."""
        master_path = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(master_path, sim_iso)
        # Fake version 99.99
        for sector, offset, val in [
            (30000, 84, 220), (30000, 85, 65), (30000, 86, 15),
            (30000, 88, 220), (30000, 89, 65), (30000, 90, 15),
            (30000, 96, 106), (30000, 97, 230), (30000, 98, 75),
            (30000, 99, 203),
        ]:
            _run_dvdisaster("--debug", "-i{}".format(sim_iso),
                            "--byteset", "{},{},{}".format(sector, offset, val))
        _run_golden_compare("scan_incompatible_ecc",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-s"],
                            tmp_path, image_path=tmp_iso,
                            ignore_line=r'^\*          $')

    def test_scan_modulo_glitch(self, tmp_path):
        """Scan with header modulo glitch, post 0.79.5 style."""
        hmg_path = _ensure_hmg_master()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        _run_golden_compare("scan_modulo_glitch",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug",
                             "--sim-cd={}".format(hmg_path),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-s", "-v"],
                            tmp_path, image_path=tmp_iso)

    def test_scan_modulo_glitch2(self, tmp_path):
        """Scan with header modulo glitch, old style, complete image."""
        hmg_path = _ensure_hmg_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(hmg_path, sim_iso)
        _apply_old_style_headers(sim_iso)
        _run_golden_compare("scan_modulo_glitch2",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-s", "-v"],
                            tmp_path, image_path=tmp_iso)

    def test_scan_modulo_glitch3(self, tmp_path):
        """Scan with header modulo glitch, old style, truncated."""
        hmg_path = _ensure_hmg_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(hmg_path, sim_iso)
        _apply_old_style_headers(sim_iso)
        _run_dvdisaster("--debug", "-i", sim_iso, "--truncate=357520")
        _run_golden_compare("scan_modulo_glitch3",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-s", "-v"],
                            tmp_path, image_path=tmp_iso)

    def test_scan_modulo_glitch4(self, tmp_path):
        """Scan with header modulo glitch, old style, truncated, missing ref sectors."""
        hmg_path = _ensure_hmg_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(hmg_path, sim_iso)
        _apply_old_style_headers(sim_iso)
        _run_dvdisaster("--debug", "-i", sim_iso, "--truncate=357520")
        for sector in HMG_DISAMBIG_SECTORS:
            _run_dvdisaster("--debug", "-i", sim_iso,
                            "--erase", str(sector))
        _run_golden_compare("scan_modulo_glitch4",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-s", "-v"],
                            tmp_path, image_path=tmp_iso)

    def test_scan_with_rs01_file(self, tmp_path):
        """Scan RS02 image with RS01 ecc file."""
        master_path = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs02-tmp.ecc")
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(master_path), "-e{}".format(tmp_ecc),
                        "-c", "-n", "normal")
        shutil.copy2(master_path, sim_iso)
        _run_dvdisaster("--debug", "-i{}".format(sim_iso),
                        "--byteset", "34930,0,1")
        _run_golden_compare("scan_with_rs01_file",
                            ["--regtest", "--no-progress",
                             "-i{}".format(sim_iso),
                             "-e{}".format(tmp_ecc),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-s"],
                            tmp_path, image_path=sim_iso,
                            ignore_line=r'^Read position: 100')

    def test_scan_with_wrong_rs01_file(self, tmp_path):
        """Scan RS02 image with non-matching RS01 ecc file."""
        master_path = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs02-tmp.ecc")
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(master_path), "-e{}".format(tmp_ecc),
                        "-c", "-n", "normal")
        _run_dvdisaster("--debug", "-i{}".format(tmp_ecc),
                        "--byteset", "0,24,1")
        shutil.copy2(master_path, sim_iso)
        _run_dvdisaster("--debug", "-i{}".format(sim_iso),
                        "--byteset", "34930,0,1")
        _run_golden_compare("scan_with_wrong_rs01_file",
                            ["--regtest", "--no-progress",
                             "-i{}".format(sim_iso),
                             "-e{}".format(tmp_ecc),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-s"],
                            tmp_path, image_path=sim_iso)

    def test_scan_with_rs03_file(self, tmp_path):
        """Scan RS02 image with RS03 ecc file."""
        master_path = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs02-tmp.ecc")
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(master_path), "-e{}".format(tmp_ecc),
                        "-mRS03", "-c", "-n", "20r", "-o", "file")
        shutil.copy2(master_path, sim_iso)
        _run_dvdisaster("--debug", "-i{}".format(sim_iso),
                        "--byteset", "34930,0,1")
        _run_golden_compare("scan_with_rs03_file",
                            ["--regtest", "--no-progress",
                             "-i{}".format(sim_iso),
                             "-e{}".format(tmp_ecc),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-s"],
                            tmp_path, image_path=sim_iso)

    def test_scan_with_wrong_rs03_file(self, tmp_path):
        """Scan RS02 image with non-matching RS03 ecc file."""
        master_path = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs02-tmp.ecc")
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(master_path), "-e{}".format(tmp_ecc),
                        "-mRS03", "-c", "-n", "20r", "-o", "file")
        _run_dvdisaster("--debug", "-i{}".format(tmp_ecc),
                        "--byteset", "0,24,1")
        shutil.copy2(master_path, sim_iso)
        _run_dvdisaster("--debug", "-i{}".format(sim_iso),
                        "--byteset", "34930,0,1")
        _run_golden_compare("scan_with_wrong_rs03_file",
                            ["--regtest", "--no-progress",
                             "-i{}".format(sim_iso),
                             "-e{}".format(tmp_ecc),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-s"],
                            tmp_path, image_path=sim_iso)


# ══════════════════════════════════════════════════════════════════
# Reading tests - linear (28)
# ══════════════════════════════════════════════════════════════════

class TestRS02ReadLinear(GoldenTestSuite):
    codec = "RS02"
    codec_prefix = "RS02"
    master = "rs02-master.iso"
    image_size = ISOSIZE

    tests = [
        GoldenTest("read_good", action="-r", new_image=True,
                   sim_cd=SimCD(source="master"),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("read_good_verbose", action="-r -v", new_image=True,
                   sim_cd=SimCD(source="master"),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("read_shorter", action="-r -v", new_image=True,
                   sim_cd=SimCD(source="master",
                                damage=[Truncate(REAL_ECCSIZE - 44)]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("read_tao_tail", action="-r", new_image=True,
                   sim_cd=SimCD(source="master",
                                damage=[AppendFile(_FIXED_RANDOM_SEQ),
                                        Erase("34932-34933")]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("read_no_tao_tail", action="-r --dao", new_image=True,
                   sim_cd=SimCD(source="master",
                                damage=[Erase("34930-34931")]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("read_bad_header", action="-r", new_image=True,
                   sim_cd=SimCD(source="master",
                                damage=[Byteset(30592, 1, 1)]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("read_bad_headers", action="-r", new_image=True,
                   sim_cd=SimCD(source="master",
                                damage=[Byteset(30592, 1, 1),
                                        Byteset(31488, 100, 1)]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("read_missing_data_sectors", action="-r", new_image=True,
                   sim_cd=SimCD(source="master",
                                damage=[Erase("1000-1049"), Erase("21230"),
                                        Erase("22450-22457")]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("read_missing_crc_sectors", action="-r", new_image=True,
                   sim_cd=SimCD(source="master",
                                damage=[Erase("30020-30030"),
                                        Erase("30034")]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("read_missing_ecc_sectors", action="-r", new_image=True,
                   sim_cd=SimCD(source="master",
                                damage=[Erase("32020-32030"),
                                        Erase("33034")]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("read_data_bad_bytes", action="-r", new_image=True,
                   sim_cd=SimCD(source="master",
                                damage=[Byteset(0, 55, 12),
                                        Byteset(1235, 50, 10),
                                        Byteset(29999, 128, 98)]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("read_crc_bad_byte", action="-r", new_image=True,
                   sim_cd=SimCD(source="master",
                                damage=[Byteset(30020, 50, 10)]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("read_ecc_bad_byte", action="-r", new_image=True,
                   sim_cd=SimCD(source="master",
                                damage=[Byteset(33100, 50, 10)]),
                   extra_args=["--spinup-delay=0"]),
    ]

    def _ensure_master(self):
        return _ensure_master()

    def test_read_good_file(self, tmp_path):
        """Read into existing and complete image file."""
        master_path = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(master_path, sim_iso)
        shutil.copy2(master_path, tmp_iso)
        _run_golden_compare("read_good_file",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-r"],
                            tmp_path, image_path=tmp_iso)

    def test_read_longer(self, tmp_path):
        """Read image longer than expected."""
        master_path = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(master_path, sim_iso)
        with open(_FIXED_RANDOM_SEQ, "rb") as f:
            pad_data = f.read()
        with open(sim_iso, "ab") as f:
            for _ in range(23):
                f.write(pad_data)
        _run_golden_compare("read_longer",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-r", "-v"],
                            tmp_path, image_path=tmp_iso)

    def test_read_incompatible_ecc(self, tmp_path):
        """Read image requiring a newer dvdisaster version."""
        master_path = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(master_path, sim_iso)
        for sector, offset, val in [
            (30000, 84, 220), (30000, 85, 65), (30000, 86, 15),
            (30000, 88, 220), (30000, 89, 65), (30000, 90, 15),
            (30000, 96, 106), (30000, 97, 230), (30000, 98, 75),
            (30000, 99, 203),
        ]:
            _run_dvdisaster("--debug", "-i{}".format(sim_iso),
                            "--byteset", "{},{},{}".format(sector, offset, val))
        _run_golden_compare("read_incompatible_ecc",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-r"],
                            tmp_path, image_path=tmp_iso,
                            ignore_line=r'^\*          $')

    def test_read_bad_master(self, tmp_path):
        """Read image with missing master header."""
        master_path = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(master_path, sim_iso)
        _run_dvdisaster("-i{}".format(sim_iso), "--debug", "--erase", "30000")
        _run_dvdisaster("-i{}".format(sim_iso), "--debug", "--erase", "32768")
        _run_golden_compare("read_bad_master",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-r", "-v"],
                            tmp_path, image_path=tmp_iso)

    def test_read_bad_master_exhaustive(self, tmp_path):
        """Read image with missing master header, exhaustive search."""
        master_path = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(master_path, sim_iso)
        _run_dvdisaster("-i{}".format(sim_iso), "--debug", "--erase", "30000")
        _run_golden_compare("read_bad_master_exhaustive",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-r", "-v",
                             "-a", "RS02"],
                            tmp_path, image_path=tmp_iso)

    def test_read_modulo_glitch(self, tmp_path):
        """Read with header modulo glitch, post 0.79.5 style."""
        hmg_path = _ensure_hmg_master()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        _run_golden_compare("read_modulo_glitch",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug",
                             "--sim-cd={}".format(hmg_path),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-r", "-v"],
                            tmp_path, image_path=tmp_iso)

    def test_read_modulo_glitch2(self, tmp_path):
        """Read with header modulo glitch, old style, complete image."""
        hmg_path = _ensure_hmg_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(hmg_path, sim_iso)
        _apply_old_style_headers(sim_iso)
        _run_golden_compare("read_modulo_glitch2",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-r", "-v"],
                            tmp_path, image_path=tmp_iso)

    def test_read_modulo_glitch3(self, tmp_path):
        """Read with header modulo glitch, old style, truncated."""
        hmg_path = _ensure_hmg_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(hmg_path, sim_iso)
        _apply_old_style_headers(sim_iso)
        _run_dvdisaster("--debug", "-i", sim_iso, "--truncate=357520")
        _run_golden_compare("read_modulo_glitch3",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-r", "-v"],
                            tmp_path, image_path=tmp_iso)

    def test_read_modulo_glitch4(self, tmp_path):
        """Read with header modulo glitch, old style, truncated, missing ref sectors."""
        hmg_path = _ensure_hmg_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(hmg_path, sim_iso)
        _apply_old_style_headers(sim_iso)
        _run_dvdisaster("--debug", "-i", sim_iso, "--truncate=357520")
        for sector in HMG_DISAMBIG_SECTORS:
            _run_dvdisaster("--debug", "-i", sim_iso,
                            "--erase", str(sector))
        _run_golden_compare("read_modulo_glitch4",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-r", "-v"],
                            tmp_path, image_path=tmp_iso)

    def test_read_with_rs01_file(self, tmp_path):
        """Read RS02 image with RS01 ecc file."""
        master_path = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs02-tmp.ecc")
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(master_path), "-e{}".format(tmp_ecc),
                        "-c", "-n", "normal")
        shutil.copy2(master_path, sim_iso)
        _run_dvdisaster("--debug", "-i{}".format(sim_iso),
                        "--byteset", "34930,0,1")
        _run_golden_compare("read_with_rs01_file",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "-e{}".format(tmp_ecc),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-r"],
                            tmp_path, image_path=tmp_iso)

    def test_read_with_wrong_rs01_file(self, tmp_path):
        """Read RS02 image with non-matching RS01 ecc file."""
        master_path = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs02-tmp.ecc")
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(master_path), "-e{}".format(tmp_ecc),
                        "-c", "-n", "normal")
        _run_dvdisaster("--debug", "-i{}".format(tmp_ecc),
                        "--byteset", "0,24,1")
        shutil.copy2(master_path, sim_iso)
        _run_dvdisaster("--debug", "-i{}".format(sim_iso),
                        "--byteset", "34930,0,1")
        _run_golden_compare("read_with_wrong_rs01_file",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "-e{}".format(tmp_ecc),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-r"],
                            tmp_path, image_path=tmp_iso)

    def test_read_with_rs03_file(self, tmp_path):
        """Read RS02 image with RS03 ecc file."""
        master_path = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs02-tmp.ecc")
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(master_path), "-e{}".format(tmp_ecc),
                        "-mRS03", "-c", "-n", "20r", "-o", "file")
        shutil.copy2(master_path, sim_iso)
        _run_dvdisaster("--debug", "-i{}".format(sim_iso),
                        "--byteset", "34930,0,1")
        _run_golden_compare("read_with_rs03_file",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "-e{}".format(tmp_ecc),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-r"],
                            tmp_path, image_path=tmp_iso)

    def test_read_with_wrong_rs03_file(self, tmp_path):
        """Read RS02 image with non-matching RS03 ecc file."""
        master_path = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs02-tmp.ecc")
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(master_path), "-e{}".format(tmp_ecc),
                        "-mRS03", "-c", "-n", "20r", "-o", "file")
        _run_dvdisaster("--debug", "-i{}".format(tmp_ecc),
                        "--byteset", "0,24,1")
        shutil.copy2(master_path, sim_iso)
        _run_dvdisaster("--debug", "-i{}".format(sim_iso),
                        "--byteset", "34930,0,1")
        _run_golden_compare("read_with_wrong_rs03_file",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "-e{}".format(tmp_ecc),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-r"],
                            tmp_path, image_path=tmp_iso)

    def test_read_second_pass_with_crc_error(self, tmp_path):
        """Re-reading medium with CRC error on second pass."""
        master_path = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(master_path, sim_iso)
        _run_dvdisaster("--debug", "-i{}".format(sim_iso),
                        "--byteset", "15830,8,3")
        shutil.copy2(master_path, tmp_iso)
        _run_dvdisaster("--debug", "-i{}".format(tmp_iso),
                        "--erase", "15800-16199")
        _run_golden_compare("read_second_pass_with_crc_error",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-r"],
                            tmp_path, image_path=tmp_iso)

    # read_multipass_ecc_partial_success is in test_multipass_read.py


# ══════════════════════════════════════════════════════════════════
# Reading tests - adaptive (24)
# ══════════════════════════════════════════════════════════════════

class TestRS02ReadAdaptive(GoldenTestSuite):
    codec = "RS02"
    codec_prefix = "RS02"
    master = "rs02-master.iso"
    image_size = ISOSIZE

    tests = [
        GoldenTest("adaptive_good", action="-r --adaptive-read",
                   new_image=True, sim_cd=SimCD(source="master"),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("adaptive_good_verbose", action="-r -v --adaptive-read",
                   new_image=True, sim_cd=SimCD(source="master"),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("adaptive_shorter", action="-r -v --adaptive-read",
                   new_image=True, sim_cd=SimCD(source="master",
                                damage=[Truncate(REAL_ECCSIZE - 44)]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("adaptive_tao_tail", action="-r --adaptive-read",
                   new_image=True, sim_cd=SimCD(source="master",
                                damage=[AppendFile(_FIXED_RANDOM_SEQ),
                                        Erase("34932-34933")]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("adaptive_no_tao_tail",
                   action="-r --dao --adaptive-read",
                   new_image=True, sim_cd=SimCD(source="master",
                                damage=[Erase("34930-34931")]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("adaptive_bad_header", action="-r -v --adaptive-read",
                   new_image=True, sim_cd=SimCD(source="master",
                                damage=[Byteset(30592, 1, 1)]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("adaptive_bad_headers", action="-r --adaptive-read",
                   new_image=True, sim_cd=SimCD(source="master",
                                damage=[Byteset(30592, 1, 1),
                                        Byteset(31488, 100, 1)]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("adaptive_missing_data_sectors",
                   action="-r --adaptive-read",
                   new_image=True, sim_cd=SimCD(source="master",
                                damage=[Erase("1000-1049"), Erase("21230"),
                                        Erase("22450-22457")]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("adaptive_missing_crc_sectors",
                   action="-r --adaptive-read",
                   new_image=True, sim_cd=SimCD(source="master",
                                damage=[Erase("30020-30030"),
                                        Erase("30034")]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("adaptive_missing_ecc_sectors",
                   action="-r --adaptive-read",
                   new_image=True, sim_cd=SimCD(source="master",
                                damage=[Erase("32020-32030"),
                                        Erase("33034")]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("adaptive_data_bad_byte",
                   action="-r --adaptive-read",
                   new_image=True, sim_cd=SimCD(source="master",
                                damage=[Byteset(1235, 50, 10)]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("adaptive_crc_bad_byte",
                   action="-r --adaptive-read",
                   new_image=True, sim_cd=SimCD(source="master",
                                damage=[Byteset(30020, 50, 10)]),
                   extra_args=["--spinup-delay=0"]),
        GoldenTest("adaptive_ecc_bad_byte",
                   action="-r --adaptive-read",
                   new_image=True, sim_cd=SimCD(source="master",
                                damage=[Byteset(33100, 50, 10)]),
                   extra_args=["--spinup-delay=0"]),
    ]

    def _ensure_master(self):
        return _ensure_master()

    def test_adaptive_good_file(self, tmp_path):
        """Read into existing and complete image file (adaptive)."""
        master_path = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(master_path, sim_iso)
        shutil.copy2(master_path, tmp_iso)
        _run_golden_compare("adaptive_good_file",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-r", "--adaptive-read"],
                            tmp_path, image_path=tmp_iso)

    def test_adaptive_longer(self, tmp_path):
        """Read image longer than expected (adaptive)."""
        master_path = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(master_path, sim_iso)
        with open(_FIXED_RANDOM_SEQ, "rb") as f:
            pad_data = f.read()
        with open(sim_iso, "ab") as f:
            for _ in range(23):
                f.write(pad_data)
        _run_golden_compare("adaptive_longer",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-r", "-v",
                             "--adaptive-read"],
                            tmp_path, image_path=tmp_iso)

    def test_adaptive_incompatible_ecc(self, tmp_path):
        """Read image requiring newer dvdisaster (adaptive)."""
        master_path = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(master_path, sim_iso)
        for sector, offset, val in [
            (30000, 84, 220), (30000, 85, 65), (30000, 86, 15),
            (30000, 88, 220), (30000, 89, 65), (30000, 90, 15),
            (30000, 96, 106), (30000, 97, 230), (30000, 98, 75),
            (30000, 99, 203),
        ]:
            _run_dvdisaster("--debug", "-i{}".format(sim_iso),
                            "--byteset", "{},{},{}".format(sector, offset, val))
        _run_golden_compare("adaptive_incompatible_ecc",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-r", "--adaptive-read"],
                            tmp_path, image_path=tmp_iso)

    def test_adaptive_modulo_glitch(self, tmp_path):
        """Read with header modulo glitch, post 0.79.5 (adaptive)."""
        hmg_path = _ensure_hmg_master()
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        _run_golden_compare("adaptive_modulo_glitch",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug",
                             "--sim-cd={}".format(hmg_path),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-r", "-v",
                             "--adaptive-read"],
                            tmp_path, image_path=tmp_iso)

    def test_adaptive_modulo_glitch2(self, tmp_path):
        """Read with header modulo glitch, old style, complete (adaptive)."""
        hmg_path = _ensure_hmg_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(hmg_path, sim_iso)
        _apply_old_style_headers(sim_iso)
        _run_golden_compare("adaptive_modulo_glitch2",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-r", "-v",
                             "--adaptive-read"],
                            tmp_path, image_path=tmp_iso)

    def test_adaptive_modulo_glitch3(self, tmp_path):
        """Read with header modulo glitch, old style, truncated (adaptive)."""
        hmg_path = _ensure_hmg_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(hmg_path, sim_iso)
        _apply_old_style_headers(sim_iso)
        _run_dvdisaster("--debug", "-i", sim_iso, "--truncate=357520")
        _run_golden_compare("adaptive_modulo_glitch3",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-r", "-v",
                             "--adaptive-read"],
                            tmp_path, image_path=tmp_iso)

    def test_adaptive_modulo_glitch4(self, tmp_path):
        """Read with header modulo glitch, old style, truncated, missing ref (adaptive)."""
        hmg_path = _ensure_hmg_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        shutil.copy2(hmg_path, sim_iso)
        _apply_old_style_headers(sim_iso)
        _run_dvdisaster("--debug", "-i", sim_iso, "--truncate=357520")
        for sector in HMG_DISAMBIG_SECTORS:
            _run_dvdisaster("--debug", "-i", sim_iso,
                            "--erase", str(sector))
        _run_golden_compare("adaptive_modulo_glitch4",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-r", "-v",
                             "--adaptive-read"],
                            tmp_path, image_path=tmp_iso)

    def test_adaptive_with_rs01_file(self, tmp_path):
        """Read RS02 image with RS01 ecc file (adaptive)."""
        master_path = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs02-tmp.ecc")
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(master_path), "-e{}".format(tmp_ecc),
                        "-c", "-n", "normal")
        shutil.copy2(master_path, sim_iso)
        _run_golden_compare("adaptive_with_rs01_file",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "-e{}".format(tmp_ecc),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-r", "--adaptive-read"],
                            tmp_path, image_path=tmp_iso)

    def test_adaptive_with_wrong_rs01_file(self, tmp_path):
        """Read RS02 image with non-matching RS01 ecc file (adaptive)."""
        master_path = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs02-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs02-tmp.ecc")
        _run_dvdisaster("--regtest", "--debug", "--set-version", SETVERSION,
                        "-i{}".format(master_path), "-e{}".format(tmp_ecc),
                        "-c", "-n", "normal")
        _run_dvdisaster("--debug", "-i{}".format(tmp_ecc),
                        "--byteset", "0,24,1")
        shutil.copy2(master_path, sim_iso)
        _run_golden_compare("adaptive_with_wrong_rs01_file",
                            ["--regtest", "--no-progress",
                             "-i{}".format(tmp_iso),
                             "-e{}".format(tmp_ecc),
                             "--debug",
                             "--sim-cd={}".format(sim_iso),
                             "--fixed-speed-values",
                             "--spinup-delay=0", "-r", "--adaptive-read"],
                            tmp_path, image_path=tmp_iso)

    # Note: bash tests read_with_rs03_file and read_with_wrong_rs03_file
    # at lines 2134-2153 of rs02.bash are linear reads (-r without
    # --adaptive-read), already covered by TestRS02ReadLinear.
