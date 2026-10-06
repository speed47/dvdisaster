"""
RS01 regression tests -- all tests from regtest/rs01.bash.

Tests are grouped into:
  - TestRS01Verify: 28 verify tests
  - TestRS01Create: 11 creation tests
  - TestRS01Repair: 18 repair/fix tests
  - TestRS01Scan: 22 scanning tests
  - TestRS01ReadLinear: 38 linear reading tests (read_multipass_ecc_partial_success
    is in test_multipass_read.py)
  - TestRS01ReadAdaptive: 25 adaptive reading tests
"""

import difflib
import os
import re
import shutil
import sys

import pytest
from filelock import FileLock

from framework import (
    Byteset,
    CreateECC,
    Erase,
    GoldenTest,
    GoldenTestSuite,
    PadBytes,
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

# Windows MSYS2/MinGW does not have /dev/sdz; the legacy bash tests use V:
# (drive letter) for a guaranteed-missing device. The golden files for
# no_device tests have .win variants that expect "V:".
NON_EXISTENT_DEVICE = "V:" if sys.platform == "win32" else "/dev/sdz"

# POSIX chmod 0o000/0o400 is not enforced on NTFS, so the Windows dvdisaster
# build cannot trigger "permission denied" errors for locally-created files.
_SKIP_CHMOD_WIN = pytest.mark.skipif(
    sys.platform == "win32",
    reason="POSIX chmod semantics not honored on NTFS",
)

_PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
_DATABASE = os.path.join(_PROJECT_ROOT, "regtest", "database")
_FIXED_RANDOM_SEQ = os.path.join(_PROJECT_ROOT, "regtest", "fixed-random-sequence")

# Constants matching the bash variables
ISOSIZE = 21000
SETVERSION = "0.80"
REDUNDANCY = "normal"


# ---------------------------------------------------------------------------
# Session fixture: create plus56 setup images (shared across all tests)
# ---------------------------------------------------------------------------

@pytest.fixture(scope="session")
def plus56_images():
    """Create rs01-plus56_bytes.iso and .ecc in ISODIR if they don't exist.

    These mirror the bash preamble that creates ISO_PLUS56 and ECC_PLUS56.
    Requires the master image and is idempotent.
    """
    os.makedirs(_ISODIR, exist_ok=True)
    master_iso = os.path.join(_ISODIR, "rs01-master.iso")
    iso_plus56 = os.path.join(_ISODIR, "rs01-plus56_bytes.iso")
    ecc_plus56 = os.path.join(_ISODIR, "rs01-plus56_bytes.ecc")

    # Each file is guarded by its own FileLock so parallel pytest-xdist workers
    # don't race: worker A starts writing → worker B sees isfile=True on the
    # partial file → B skips creation → B's tests fail with "No error
    # correction file present".
    with FileLock(master_iso + ".lock"):
        if not os.path.isfile(master_iso):
            _run_dvdisaster(
                "--regtest", "--debug",
                "-i{}".format(master_iso),
                "--random-image", "21000",
                check=True,
            )

    with FileLock(iso_plus56 + ".lock"):
        if not os.path.isfile(iso_plus56):
            shutil.copy2(master_iso, iso_plus56)
            with open(iso_plus56, "ab") as f:
                f.write(b"\x00" * 56)

    master_ecc = os.path.join(_ISODIR, "rs01-master.ecc")
    with FileLock(master_ecc + ".lock"):
        if not os.path.isfile(master_ecc):
            _run_dvdisaster(
                "--regtest", "--debug", "--set-version", "0.80",
                "-i{}".format(master_iso),
                "-e{}".format(master_ecc),
                "-c", "-n", "normal",
                check=True,
            )

    with FileLock(ecc_plus56 + ".lock"):
        if not os.path.isfile(ecc_plus56):
            _run_dvdisaster(
                "--regtest", "--debug", "--set-version", "0.80",
                "-i{}".format(iso_plus56),
                "-e{}".format(ecc_plus56),
                "-c", "-n", "normal",
                check=True,
            )

    return iso_plus56, ecc_plus56


# ---------------------------------------------------------------------------
# Common damage patterns (reused by multiple tests)
# ---------------------------------------------------------------------------

_DAMAGE_DEFECTIVE = [
    Erase("1000-1049"),
    Erase("11230"),
    Erase("12450-12457"),
    Byteset(13444, 0, 154),
]

_DAMAGE_MISSING_SECTORS = [
    Erase("1000-1049"),
    Erase("11230"),
    Erase("12450-12457"),
]

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
        test_name: name matching the golden file (e.g. 'ecc_create')
        cmd_args: list of CLI arguments
        tmp_path: pytest tmp_path for cleaning
        image_path: path to image file for MD5 check (or None)
        ecc_path: path to ecc file for MD5 check (or None)
        ignore_line_re: regex pattern for lines to strip from output
    """
    golden_base = os.path.join(_DATABASE, "RS01_{}".format(test_name))
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
    """Ensure the RS01 master image exists and return its path."""
    os.makedirs(_ISODIR, exist_ok=True)
    path = os.path.join(_ISODIR, "rs01-master.iso")
    with FileLock(path + ".lock"):
        if not os.path.isfile(path):
            _run_dvdisaster(
                "--regtest", "--debug",
                "-i{}".format(path),
                "--random-image", str(ISOSIZE),
                check=True,
            )
    return path


def _ensure_master_ecc():
    """Ensure the RS01 master ECC exists and return its path."""
    master_iso = _ensure_master()
    path = os.path.join(_ISODIR, "rs01-master.ecc")
    with FileLock(path + ".lock"):
        if not os.path.isfile(path):
            _run_dvdisaster(
                "--regtest", "--debug", "--set-version", SETVERSION,
                "-i{}".format(master_iso),
                "-e{}".format(path),
                "-c", "-n", REDUNDANCY,
                check=True,
            )
    return path


def _append_fixed_random_sequence(path, times=1):
    """Append the fixed-random-sequence file content to path, `times` times."""
    with open(_FIXED_RANDOM_SEQ, "rb") as f:
        seq_data = f.read()
    with open(path, "ab") as f:
        for _ in range(times):
            f.write(seq_data)


# ---------------------------------------------------------------------------
# Test Suite: Verify
# ---------------------------------------------------------------------------

class TestRS01Verify(GoldenTestSuite):
    codec = "RS01"
    codec_prefix = "RS01"
    master = "rs01-master.iso"
    master_ecc = "rs01-master.ecc"
    image_size = 21000
    redundancy = "normal"

    # ------------------------------------------------------------------
    # Declarative tests (auto-parametrized via test_golden)
    # ------------------------------------------------------------------
    tests = [
        # 1. good
        GoldenTest("good", action="-t", use_master=True, ecc="master_ecc"),
        # 2. good_quick
        GoldenTest("good_quick", action="-tq", use_master=True, ecc="master_ecc"),
        # 3. no_files
        GoldenTest("no_files", action="-t", image="no.iso", ecc="no.ecc"),
        # 4. no_image
        GoldenTest("no_image", action="-t", image="no.iso", ecc="master_ecc"),
        # 5. no_ecc
        GoldenTest("no_ecc", action="-t", use_master=True, ecc="no.ecc"),
        # 6. defective_image_no_ecc
        GoldenTest("defective_image_no_ecc", action="-t",
                   damage=_DAMAGE_DEFECTIVE, ecc="no.ecc"),
        # 15. truncated (by 5 sectors)
        GoldenTest("truncated", action="-t",
                   damage=[Truncate(20995)], ecc="master_ecc"),
        # 16. plus1 (1 extra sector)
        GoldenTest("plus1", action="-t",
                   damage=[PadSectors(1)], ecc="master_ecc"),
        # 17. plus17 (17 extra sectors)
        GoldenTest("plus17", action="-t",
                   damage=[PadSectors(17)], ecc="master_ecc"),
        # 18. defective_with_ecc
        GoldenTest("defective_with_ecc", action="-t",
                   damage=_DAMAGE_DEFECTIVE, ecc="master_ecc"),
        # 19. missing_sectors_with_ecc
        GoldenTest("missing_sectors_with_ecc", action="-t",
                   damage=_DAMAGE_MISSING_SECTORS, ecc="master_ecc"),
        # 20. crc_errors_with_ecc
        GoldenTest("crc_errors_with_ecc", action="-t",
                   damage=[Byteset(13444, 0, 154)], ecc="master_ecc"),
        # 21. crc_in_fingerprint
        GoldenTest("crc_in_fingerprint", action="-t",
                   damage=[Byteset(16, 201, 55)], ecc="master_ecc"),
        # 22. missing_fingerprint
        GoldenTest("missing_fingerprint", action="-t",
                   damage=[Erase("16")], ecc="master_ecc"),
        # 23. missing_ecc_header
        GoldenTest("missing_ecc_header", action="-t",
                   use_master=True, ecc="master_ecc",
                   ecc_damage=[Erase("0")]),
        # 24. ecc_header_crc_error
        GoldenTest("ecc_header_crc_error", action="-t",
                   use_master=True, ecc="master_ecc",
                   ecc_damage=[Byteset(0, 22, 107)]),
        # 25. uncorrectable_dsm_in_image
        GoldenTest("uncorrectable_dsm_in_image", action="-t",
                   damage=_DAMAGE_DSM1, ecc="master_ecc"),
        # 26. uncorrectable_dsm_in_image_verbose
        GoldenTest("uncorrectable_dsm_in_image_verbose", action="-t -v",
                   damage=_DAMAGE_DSM1, ecc="master_ecc"),
        # 27. uncorrectable_dsm_in_image2
        GoldenTest("uncorrectable_dsm_in_image2", action="-t",
                   damage=_DAMAGE_DSM2, ecc="master_ecc"),
        # 28. uncorrectable_dsm_in_image2_verbose
        GoldenTest("uncorrectable_dsm_in_image2_verbose", action="-t -v",
                   damage=_DAMAGE_DSM2, ecc="master_ecc"),
    ]

    # ------------------------------------------------------------------
    # Plus56 fixture-dependent tests (#7-#13)
    # ------------------------------------------------------------------

    def test_plus56_bytes(self, plus56_images, tmp_path):
        """#7: verify plus56 image with its own ecc."""
        iso_plus56, ecc_plus56 = plus56_images
        test = GoldenTest(
            "plus56_bytes", action="-t",
            image=os.path.basename(iso_plus56),
            ecc=os.path.basename(ecc_plus56),
        )
        self._run_golden_test(test, tmp_path)

    def test_image_plus56_bytes(self, plus56_images, tmp_path):
        """#8: verify plus56 image with no ecc."""
        iso_plus56, _ = plus56_images
        test = GoldenTest(
            "image_plus56_bytes", action="-t",
            image=os.path.basename(iso_plus56),
            ecc="no.ecc",
        )
        self._run_golden_test(test, tmp_path)

    def test_ecc_plus56_bytes(self, plus56_images, tmp_path):
        """#9: verify no image with plus56 ecc."""
        _, ecc_plus56 = plus56_images
        test = GoldenTest(
            "ecc_plus56_bytes", action="-t",
            image="no.iso",
            ecc=os.path.basename(ecc_plus56),
        )
        self._run_golden_test(test, tmp_path)

    def test_normal_image_ecc_plus56b(self, plus56_images, tmp_path):
        """#10: verify normal master image with plus56 ecc."""
        _, ecc_plus56 = plus56_images
        test = GoldenTest(
            "normal_image_ecc_plus56b", action="-t",
            use_master=True,
            ecc=os.path.basename(ecc_plus56),
        )
        self._run_golden_test(test, tmp_path)

    def test_image_plus56b_normal_ecc(self, plus56_images, tmp_path):
        """#11: verify plus56 image with normal master ecc."""
        iso_plus56, _ = plus56_images
        test = GoldenTest(
            "image_plus56b_normal_ecc", action="-t",
            image=os.path.basename(iso_plus56),
            ecc="master_ecc",
        )
        self._run_golden_test(test, tmp_path)

    def test_image_few_bytes_shorter(self, plus56_images, tmp_path):
        """#12: master + 55 bytes (1 byte shorter than plus56 ecc expects)."""
        _, ecc_plus56 = plus56_images
        test = GoldenTest(
            "image_few_bytes_shorter", action="-t",
            damage=[PadBytes(55)],
            ecc=os.path.basename(ecc_plus56),
        )
        self._run_golden_test(test, tmp_path)

    def test_image_few_bytes_longer(self, plus56_images, tmp_path):
        """#13: master + 57 bytes (1 byte longer than plus56 ecc expects)."""
        _, ecc_plus56 = plus56_images
        test = GoldenTest(
            "image_few_bytes_longer", action="-t",
            damage=[PadBytes(57)],
            ecc=os.path.basename(ecc_plus56),
        )
        self._run_golden_test(test, tmp_path)

    # ------------------------------------------------------------------
    # Truncated by bytes test (#14) -- needs dd-like truncation
    # ------------------------------------------------------------------

    def test_truncated_by_bytes(self, tmp_path):
        """#14: master truncated to (2048*21000 - 7) bytes."""
        master = self._ensure_master()
        master_ecc = self._ensure_master_ecc()
        truncated_size = 2048 * 21000 - 7

        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        with open(master, "rb") as src, open(tmp_iso, "wb") as dst:
            dst.write(src.read(truncated_size))

        test = GoldenTest(
            "truncated_by_bytes", action="-t",
            ecc="master_ecc",
        )
        # Manually set up and run since we prepared the image ourselves
        self._run_golden_test_with_prepared_image(test, tmp_iso, tmp_path)

    def _run_golden_test_with_prepared_image(self, test, image_path, tmp_path):
        """Run a golden test with an already-prepared image file."""
        golden_base = os.path.join(
            os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
            "regtest", "database",
            "{}_{}".format(self.codec_prefix, test.name),
        )
        golden_path = resolve_golden_path(golden_base)
        if not os.path.isfile(golden_path):
            pytest.skip("Golden file not found: {}".format(golden_path))

        image_md5, ecc_md5, expected_output = parse_golden_file(golden_path)
        ecc_path = self._resolve_ecc(test)

        cmd_args = ["--regtest", "--no-progress"]
        cmd_args.append("-i{}".format(image_path))
        if ecc_path:
            cmd_args.append("-e{}".format(ecc_path))
        cmd_args.extend(test.action.split())

        _, raw_output = _run_dvdisaster(*cmd_args)

        work_dir = str(tmp_path)
        cleaned = clean_output(
            raw_output,
            tmp_dirs=[work_dir, _TMPDIR, _ISODIR],
            strip_header=True,
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
                "Output mismatch for test '{}':\n{}".format(test.name, diff_text)
            )

        if image_md5 is not None and os.path.isfile(image_path):
            actual_md5 = _md5_file(image_path)
            assert actual_md5 == image_md5, (
                "Image MD5 mismatch for '{}': expected {}, got {}".format(
                    test.name, image_md5, actual_md5
                )
            )

        if ecc_md5 is not None and ecc_path and os.path.isfile(ecc_path):
            actual_ecc_md5 = _md5_file(ecc_path)
            assert actual_ecc_md5 == ecc_md5, (
                "ECC MD5 mismatch for '{}': expected {}, got {}".format(
                    test.name, ecc_md5, actual_ecc_md5
                )
            )


# ---------------------------------------------------------------------------
# Test Suite: Create
# ---------------------------------------------------------------------------

class TestRS01Create(GoldenTestSuite):
    codec = "RS01"
    codec_prefix = "RS01"
    master = "rs01-master.iso"
    master_ecc = "rs01-master.ecc"
    image_size = 21000
    redundancy = "normal"

    tests = []  # All creation tests are plain methods

    def test_ecc_create(self, tmp_path):
        """Create ecc file from master image."""
        master = _ensure_master()
        ecc = os.path.join(str(tmp_path), "rs01-tmp.ecc")
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(master), "-e{}".format(ecc),
            "--debug", "--set-version", SETVERSION,
            "-c", "-n", REDUNDANCY,
        ]
        _run_golden_compare("ecc_create", cmd, tmp_path,
                            image_path=master, ecc_path=ecc)

    def test_ecc_missing_image(self, tmp_path):
        """Create ecc with missing image."""
        no_file = os.path.join(_ISODIR, "none.iso")
        ecc = os.path.join(str(tmp_path), "rs01-tmp.ecc")
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(no_file), "-e{}".format(ecc),
            "-c", "-n", REDUNDANCY,
        ]
        _run_golden_compare("ecc_missing_image", cmd, tmp_path)

    @_SKIP_CHMOD_WIN
    def test_ecc_no_read_perm(self, tmp_path):
        """Create ecc with no read permission on image."""
        master = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        ecc = os.path.join(str(tmp_path), "rs01-tmp.ecc")
        shutil.copy2(master, tmp_iso)
        os.chmod(tmp_iso, 0o000)
        try:
            cmd = [
                "--regtest", "--no-progress",
                "-i{}".format(tmp_iso), "-e{}".format(ecc),
                "-c", "-n", REDUNDANCY,
            ]
            _run_golden_compare("ecc_no_read_perm", cmd, tmp_path)
        finally:
            os.chmod(tmp_iso, 0o644)

    @_SKIP_CHMOD_WIN
    def test_ecc_no_write_perm(self, tmp_path):
        """Create ecc with no write permission on ecc file (should recreate)."""
        master = _ensure_master()
        ecc = os.path.join(str(tmp_path), "rs01-tmp.ecc")
        # Create and make read-only
        with open(ecc, "w"):
            pass
        os.chmod(ecc, 0o400)
        try:
            cmd = [
                "--regtest", "--no-progress",
                "-i{}".format(master), "-e{}".format(ecc),
                "--debug", "--set-version", SETVERSION,
                "-c", "-n", REDUNDANCY,
            ]
            _run_golden_compare("ecc_no_write_perm", cmd, tmp_path,
                                image_path=master, ecc_path=ecc)
        finally:
            try:
                os.chmod(ecc, 0o644)
            except OSError:
                pass

    def test_ecc_create_plus56(self, plus56_images, tmp_path):
        """Create ecc file from plus56 image."""
        iso_plus56, _ = plus56_images
        ecc = os.path.join(str(tmp_path), "rs01-tmp.ecc")
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(iso_plus56), "-e{}".format(ecc),
            "--debug", "--set-version", SETVERSION,
            "-c", "-n", REDUNDANCY,
        ]
        _run_golden_compare("ecc_create_plus56", cmd, tmp_path,
                            image_path=iso_plus56, ecc_path=ecc)

    def test_ecc_missing_sectors(self, tmp_path):
        """Create ecc from image with unreadable sectors."""
        master = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        ecc = os.path.join(str(tmp_path), "rs01-tmp.ecc")
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [
            Erase("1000-1049"),
            Erase("11230"),
            Erase("12450-12457"),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(ecc),
            "-c", "-n", REDUNDANCY,
        ]
        _run_golden_compare("ecc_missing_sectors", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=ecc)

    def test_ecc_create_after_read(self, tmp_path):
        """Read image and create ecc in one call."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        ecc = os.path.join(str(tmp_path), "rs01-tmp.ecc")
        shutil.copy2(master, sim_iso)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(ecc),
            "--debug", "--set-version", SETVERSION,
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "-r", "-c", "-n", REDUNDANCY, "--spinup-delay=0", "-v",
        ]
        _run_golden_compare("ecc_create_after_read", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=ecc)

    def test_ecc_recreate_after_read_rs01(self, tmp_path):
        """Read image with RS01 ecc and create new ecc."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        ecc = os.path.join(str(tmp_path), "rs01-tmp.ecc")
        shutil.copy2(master, sim_iso)
        # Create initial RS01 ecc with 8r redundancy
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(sim_iso), "-e{}".format(ecc),
            "-c", "-n", "8r", check=True,
        )
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(ecc),
            "--debug", "--set-version", SETVERSION,
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "-r", "-c", "-n", REDUNDANCY, "--spinup-delay=0", "-v",
        ]
        _run_golden_compare("ecc_recreate_after_read_rs01", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=ecc)

    def test_ecc_recreate_after_read_rs02(self, tmp_path):
        """Read image with RS02 ecc and create additional ecc file."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        ecc = os.path.join(str(tmp_path), "rs01-tmp.ecc")
        shutil.copy2(master, sim_iso)
        # Augment with RS02
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(sim_iso), "-c", "-mRS02",
            "-n{}".format(ISOSIZE + 6000), check=True,
        )
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(ecc),
            "--debug", "--set-version", SETVERSION,
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "-r", "-c", "-n", REDUNDANCY, "--spinup-delay=0", "-v",
        ]
        _run_golden_compare("ecc_recreate_after_read_rs02", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=ecc)

    def test_ecc_recreate_after_read_rs03i(self, tmp_path):
        """Read image with RS03i ecc and create additional ecc file."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        ecc = os.path.join(str(tmp_path), "rs01-tmp.ecc")
        shutil.copy2(master, sim_iso)
        # Augment with RS03 (image mode)
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(sim_iso), "-c", "-mRS03",
            "-n{}".format(ISOSIZE + 6000), check=True,
        )
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(ecc),
            "--debug", "--set-version", SETVERSION,
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "-r", "-c", "-n", REDUNDANCY, "--spinup-delay=0", "-v",
        ]
        _run_golden_compare("ecc_recreate_after_read_rs03i", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=ecc)

    def test_ecc_recreate_after_read_rs03f(self, tmp_path):
        """Read image with RS03f ecc and create new ecc."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        ecc = os.path.join(str(tmp_path), "rs01-tmp.ecc")
        shutil.copy2(master, sim_iso)
        # Create RS03 file-mode ecc with 8r redundancy
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(sim_iso), "-e{}".format(ecc),
            "-c", "-n", "8r", "-mRS03", "-o", "file", check=True,
        )
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(ecc),
            "--debug", "--set-version", SETVERSION,
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "-r", "-c", "-n", REDUNDANCY, "--spinup-delay=0", "-v",
        ]
        _run_golden_compare("ecc_recreate_after_read_rs03f", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=ecc)

    def test_ecc_create_after_partial_read(self, tmp_path):
        """Create ecc after completing partial image via read."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        ecc = os.path.join(str(tmp_path), "rs01-tmp.ecc")
        shutil.copy2(master, sim_iso)
        shutil.copy2(master, tmp_iso)
        # Erase sectors 1000-1500 in the tmp image
        _apply_damage(tmp_iso, [Erase("1000-1500")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(ecc),
            "--debug", "--set-version", SETVERSION,
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "-r", "-c", "-n", REDUNDANCY, "--spinup-delay=0", "-v",
        ]
        _run_golden_compare("ecc_create_after_partial_read", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=ecc)


# ---------------------------------------------------------------------------
# Test Suite: Repair
# ---------------------------------------------------------------------------

class TestRS01Repair(GoldenTestSuite):
    codec = "RS01"
    codec_prefix = "RS01"
    master = "rs01-master.iso"
    master_ecc = "rs01-master.ecc"
    image_size = 21000
    redundancy = "normal"

    # Simple repair tests using the declarative approach
    tests = [
        # fix_good: fix good image (no damage)
        GoldenTest("fix_good", action="-f", ecc="master_ecc"),
        # fix_no_write_perm: fix image without write permission
        GoldenTest("fix_no_write_perm", action="-f",
                   chmod_image=0o400, ecc="master_ecc"),
        # fix_missing_sectors
        GoldenTest("fix_missing_sectors", action="-f",
                   damage=[
                       Erase("0"), Erase("190"), Erase("192"),
                       Erase("590-649"), Erase("2000-2139"),
                       Erase("2141-2176"), Erase("20999"),
                   ], ecc="master_ecc"),
        # fix_crc_errors
        GoldenTest("fix_crc_errors", action="-f",
                   damage=[
                       Byteset(0, 1, 1), Byteset(190, 200, 143),
                       Byteset(1200, 100, 1), Byteset(1201, 100, 1),
                       Byteset(20999, 500, 91),
                   ], ecc="master_ecc"),
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
                   damage=[Truncate(20731)], ecc="master_ecc"),
    ]

    # ------------------------------------------------------------------
    # Permission tests requiring special handling
    # ------------------------------------------------------------------

    def test_fix_no_read_perm(self, tmp_path):
        """Fix image without read permission."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, tmp_iso)
        os.chmod(tmp_iso, 0o000)
        try:
            cmd = [
                "--regtest", "--no-progress",
                "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
                "-f",
            ]
            _run_golden_compare("fix_no_read_perm", cmd, tmp_path,
                                image_path=tmp_iso, ecc_path=master_ecc)
        finally:
            os.chmod(tmp_iso, 0o644)

    def test_fix_no_read_perm_ecc(self, tmp_path):
        """Fix image without read permission on ecc."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs01-tmp.ecc")
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

    # ------------------------------------------------------------------
    # Plus56 repair tests
    # ------------------------------------------------------------------

    def test_fix_plus56_bytes(self, plus56_images, tmp_path):
        """Fix good image not multiple of 2048."""
        iso_plus56, ecc_plus56 = plus56_images
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(iso_plus56, tmp_iso)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(ecc_plus56),
            "-f",
        ]
        _run_golden_compare("fix_plus56_bytes", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=ecc_plus56)

    def test_fix_plus56(self, plus56_images, tmp_path):
        """Fix image with CRC error in 56 additional bytes."""
        iso_plus56, ecc_plus56 = plus56_images
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(iso_plus56, tmp_iso)
        _apply_damage(tmp_iso, [Byteset(21000, 28, 90)])
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
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(iso_plus56, tmp_iso)
        # Append extra bytes
        with open(tmp_iso, "ab") as f:
            f.write(b"0123456789abcdef\n")
        _apply_damage(tmp_iso, [Byteset(21000, 55, 90)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(ecc_plus56),
            "-f", "--truncate",
        ]
        _run_golden_compare("fix_plus56_plus17", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=ecc_plus56)

    def test_fix_plus56_plus1s(self, plus56_images, tmp_path):
        """Fix image with CRC error in 56 additional bytes + one sector more."""
        iso_plus56, ecc_plus56 = plus56_images
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(iso_plus56, tmp_iso)
        with open(tmp_iso, "ab") as f:
            f.write(b"\x00" * 2048)
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
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(iso_plus56, tmp_iso)
        with open(tmp_iso, "ab") as f:
            f.write(b"\x00" * 4096)
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
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
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
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(iso_plus56, tmp_iso)
        _apply_damage(tmp_iso, [Truncate(20972)])
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
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, tmp_iso)
        # Append 50 zero bytes (less than 56)
        with open(tmp_iso, "ab") as f:
            f.write(b"\x00" * 50)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(ecc_plus56),
            "-f",
        ]
        _run_golden_compare("fix_plus56_little_truncated", cmd, tmp_path,
                            image_path=tmp_iso, ecc_path=ecc_plus56)


# ---------------------------------------------------------------------------
# Test Suite: Scan
# ---------------------------------------------------------------------------

class TestRS01Scan(GoldenTestSuite):
    codec = "RS01"
    codec_prefix = "RS01"
    master = "rs01-master.iso"
    master_ecc = "rs01-master.ecc"
    image_size = 21000
    redundancy = "normal"

    # Declarative scan tests
    # Note: sim_cd adds --sim-cd, --fixed-speed-values, --spinup-delay=0
    # so the action should not repeat --spinup-delay=0
    tests = [
        # scan_no_ecc: scan image, no ecc data
        GoldenTest("scan_no_ecc", action="-s",
                   image="no.iso", ecc="no.ecc",
                   sim_cd=SimCD(source="master"),
                   extra_args=["--debug"]),
        # scan_defective_no_ecc: defective media, no ecc
        GoldenTest("scan_defective_no_ecc", action="-s",
                   image="no.iso", ecc="no.ecc",
                   sim_cd=SimCD(source="master", damage=[
                       Erase("100-200"), Erase("766"), Erase("2410"),
                   ]),
                   extra_args=["--debug"]),
        # scan_defective_no_ecc_again: 1 sector skip
        GoldenTest("scan_defective_no_ecc_again", action="-j 1 -s",
                   image="no.iso", ecc="no.ecc",
                   sim_cd=SimCD(source="master", damage=[
                       Erase("100-200"), Erase("766"), Erase("2410"),
                   ]),
                   extra_args=["--debug"]),
        # scan_defective_large_skip: large sector skip of 256
        GoldenTest("scan_defective_large_skip", action="-s -j 256",
                   image="no.iso", ecc="no.ecc",
                   sim_cd=SimCD(source="master", damage=[
                       Erase("1600-1615"), Erase("6400-10000"),
                   ]),
                   extra_args=["--debug"]),
        # scan_new_with_range_no_ecc: partial range
        GoldenTest("scan_new_with_range_no_ecc", action="-s10000-15000",
                   image="no.iso", ecc="no.ecc",
                   sim_cd=SimCD(source="master"),
                   extra_args=["--debug"]),
        # scan_new_with_invalid_range_no_ecc: invalid range
        GoldenTest("scan_new_with_invalid_range_no_ecc",
                   action="-s10000-55000",
                   image="no.iso", ecc="no.ecc",
                   sim_cd=SimCD(source="master"),
                   extra_args=["--debug"]),
        # scan_with_ecc: scan with ecc data
        GoldenTest("scan_with_ecc", action="-s",
                   image="no.iso", ecc="master_ecc",
                   sim_cd=SimCD(source="master"),
                   extra_args=["--debug"]),
        # scan_with_non_existing_ecc
        GoldenTest("scan_with_non_existing_ecc", action="-s",
                   image="no.iso", ecc="no_ecc",
                   sim_cd=SimCD(source="master"),
                   extra_args=["--debug"]),
        # scan_crc_errors_with_ecc
        GoldenTest("scan_crc_errors_with_ecc", action="-s",
                   image="no.iso", ecc="master_ecc",
                   sim_cd=SimCD(source="master", damage=[
                       Byteset(0, 100, 255), Byteset(1, 180, 200),
                       Byteset(7910, 23, 98), Byteset(20999, 55, 123),
                   ]),
                   extra_args=["--debug"]),
        # scan_no_tao_tail_with_ecc: --dao option
        GoldenTest("scan_no_tao_tail_with_ecc", action="--dao -s",
                   image="no.iso", ecc="master_ecc",
                   sim_cd=SimCD(source="master", damage=[
                       Erase("20998-20999"),
                   ]),
                   extra_args=["--debug"]),
        # scan_more_missing_at_end_with_ecc
        GoldenTest("scan_more_missing_at_end_with_ecc", action="-s",
                   image="no.iso", ecc="master_ecc",
                   sim_cd=SimCD(source="master", damage=[
                       Erase("20954-20999"),
                   ]),
                   extra_args=["--debug"]),
        # scan_with_hardware_failure
        GoldenTest("scan_with_hardware_failure", action="-s",
                   image="no.iso", ecc="no.ecc",
                   sim_cd=SimCD(source="master", damage=[
                       Erase("5000:hardware failure"),
                       Erase("6000:hardware failure"),
                   ]),
                   extra_args=["--debug"]),
        # scan_with_ignored_hardware_failure
        GoldenTest("scan_with_ignored_hardware_failure",
                   action="-s --ignore-fatal-sense",
                   image="no.iso", ecc="no.ecc",
                   sim_cd=SimCD(source="master", damage=[
                       Erase("5000:hardware failure"),
                   ]),
                   extra_args=["--debug"]),
        # scan_medium_with_dsm
        GoldenTest("scan_medium_with_dsm", action="-s",
                   image="no.iso", ecc="no.ecc",
                   sim_cd=SimCD(source="master", damage=[
                       Erase("4999:pass as dead sector marker"),
                       Erase("5799:pass as dead sector marker"),
                   ]),
                   extra_args=["--debug"]),
    ]

    # ------------------------------------------------------------------
    # Tests requiring special setup (plain methods)
    # ------------------------------------------------------------------

    def test_scan_no_device(self, tmp_path):
        """Scan image from non-existent device."""
        master = _ensure_master()
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(os.path.join(_ISODIR, "no.iso")),
            "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
            "--debug", "-d", NON_EXISTENT_DEVICE,
            "--sim-cd={}".format(master), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_no_device", cmd, tmp_path)

    @_SKIP_CHMOD_WIN
    def test_scan_no_device_access(self, tmp_path):
        """Scan image from device with insufficient permissions."""
        master = _ensure_master()
        fake_dev = os.path.join(str(tmp_path), "sdz")
        with open(fake_dev, "w"):
            pass
        os.chmod(fake_dev, 0o000)
        try:
            cmd = [
                "--regtest", "--no-progress",
                "-i{}".format(os.path.join(_ISODIR, "no.iso")),
                "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
                "--debug",
                "--sim-cd={}".format(master), "--fixed-speed-values",
                "--spinup-delay=0", "-d", fake_dev, "-s",
            ]
            _run_golden_compare("scan_no_device_access", cmd, tmp_path)
        finally:
            os.chmod(fake_dev, 0o644)

    @_SKIP_CHMOD_WIN
    def test_scan_with_no_permission_for_ecc(self, tmp_path):
        """Scan with no permission to access ecc file."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        tmp_ecc = os.path.join(str(tmp_path), "rs01-tmp.ecc")
        shutil.copy2(master_ecc, tmp_ecc)
        os.chmod(tmp_ecc, 0o000)
        try:
            cmd = [
                "--regtest", "--no-progress",
                "-i{}".format(os.path.join(_ISODIR, "no.iso")),
                "-e{}".format(tmp_ecc),
                "--debug",
                "--sim-cd={}".format(master), "--fixed-speed-values",
                "--spinup-delay=0", "-s",
            ]
            _run_golden_compare("scan_with_no_permission_for_ecc", cmd, tmp_path)
        finally:
            os.chmod(tmp_ecc, 0o644)

    def test_scan_shorter_with_ecc(self, tmp_path):
        """Scan image shorter than expected with ecc."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Truncate(ISOSIZE - 44)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(os.path.join(_ISODIR, "no.iso")),
            "-e{}".format(master_ecc),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--ignore-iso-size", "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_shorter_with_ecc", cmd, tmp_path)

    def test_scan_longer_with_ecc(self, tmp_path):
        """Scan image longer than expected with ecc."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        shutil.copy2(master, sim_iso)
        _append_fixed_random_sequence(sim_iso, 22)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(os.path.join(_ISODIR, "no.iso")),
            "-e{}".format(master_ecc),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--ignore-iso-size", "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_longer_with_ecc", cmd, tmp_path)

    def test_scan_tao_tail_with_ecc(self, tmp_path):
        """Scan image, tao tail case, ecc data."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        shutil.copy2(master, sim_iso)
        _append_fixed_random_sequence(sim_iso, 1)
        _apply_damage(sim_iso, [Erase("21000-21001")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(os.path.join(_ISODIR, "no.iso")),
            "-e{}".format(master_ecc),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--ignore-iso-size", "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_tao_tail_with_ecc", cmd, tmp_path)

    def test_scan_with_double_ecc(self, tmp_path):
        """Scan image with RS02 data and a RS01 ecc file."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs01-tmp.ecc")
        shutil.copy2(master, sim_iso)
        # Augment with RS02
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(sim_iso), "-mRS02",
            "-n{}".format(ISOSIZE + 5000), "-c", check=True,
        )
        # Create RS01 ecc
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(sim_iso), "-e", tmp_ecc,
            "-c", "-n", REDUNDANCY, check=True,
        )
        # Introduce CRC error in RS02 area
        _apply_damage(sim_iso, [Byteset(25910, 100, 200)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(os.path.join(_ISODIR, "no.iso")),
            "-e{}".format(tmp_ecc),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_with_double_ecc", cmd, tmp_path)

    def test_scan_with_incompatible_ecc(self, tmp_path):
        """Scan image with ecc file requiring a newer dvdisaster version."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs01-tmp.ecc")
        shutil.copy2(master, sim_iso)
        # Create ECC
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(sim_iso), "-e", tmp_ecc,
            "-c", "-n", REDUNDANCY, check=True,
        )
        # Modify version bytes in ECC header
        _apply_damage(tmp_ecc, [
            Byteset(0, 88, 220),
            Byteset(0, 89, 65),
            Byteset(0, 90, 15),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(os.path.join(_ISODIR, "no.iso")),
            "-e{}".format(tmp_ecc),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-s",
        ]
        _run_golden_compare("scan_with_incompatible_ecc", cmd, tmp_path,
                            ignore_line_re=r'^\*          $')


# ---------------------------------------------------------------------------
# Test Suite: Linear Reading
# ---------------------------------------------------------------------------

class TestRS01ReadLinear(GoldenTestSuite):
    codec = "RS01"
    codec_prefix = "RS01"
    master = "rs01-master.iso"
    master_ecc = "rs01-master.ecc"
    image_size = 21000
    redundancy = "normal"

    # Declarative linear reading tests
    # Note: sim_cd adds --sim-cd, --fixed-speed-values, --spinup-delay=0
    # new_image=True means: provide path but don't pre-create the file
    tests = [
        # read_no_ecc: read image, no ecc data
        GoldenTest("read_no_ecc", action="-r", new_image=True,
                   ecc="no.ecc",
                   sim_cd=SimCD(source="master"),
                   extra_args=["--debug"]),
        # read_defective_no_ecc: defective media, no ecc
        GoldenTest("read_defective_no_ecc", action="-r", new_image=True,
                   ecc="no.ecc",
                   sim_cd=SimCD(source="master", damage=[
                       Erase("100-200"), Erase("766"), Erase("2410"),
                   ]),
                   extra_args=["--debug"]),
        # read_defective_large_skip
        GoldenTest("read_defective_large_skip", action="-r -j 256", new_image=True,
                   ecc="no.ecc",
                   sim_cd=SimCD(source="master", damage=[
                       Erase("1600-1615"), Erase("6400-10000"),
                   ]),
                   extra_args=["--debug"]),
        # read_new_with_range_no_ecc
        GoldenTest("read_new_with_range_no_ecc", action="-r10000-15000", new_image=True,
                   ecc="no.ecc",
                   sim_cd=SimCD(source="master"),
                   extra_args=["--debug"]),
        # read_new_with_invalid_range_no_ecc
        GoldenTest("read_new_with_invalid_range_no_ecc", new_image=True,
                   action="-r10000-55000",
                   ecc="no.ecc",
                   sim_cd=SimCD(source="master"),
                   extra_args=["--debug"]),
        # read_two_missing_secs_no_ecc
        GoldenTest("read_two_missing_secs_no_ecc", action="-r -j 1", new_image=True,
                   ecc="no.ecc",
                   sim_cd=SimCD(source="master", damage=[
                       Erase("8020"), Erase("20999"),
                   ]),
                   extra_args=["--debug"]),
        # read_with_ecc: read with ecc data
        GoldenTest("read_with_ecc", action="-r", new_image=True,
                   ecc="master_ecc",
                   sim_cd=SimCD(source="master"),
                   extra_args=["--debug"]),
        # read_with_non_existing_ecc
        GoldenTest("read_with_non_existing_ecc", action="-r", new_image=True,
                   ecc="no_ecc",
                   sim_cd=SimCD(source="master"),
                   extra_args=["--debug"]),
        # read_crc_errors_with_ecc
        GoldenTest("read_crc_errors_with_ecc", action="-r", new_image=True,
                   ecc="master_ecc",
                   sim_cd=SimCD(source="master", damage=[
                       Byteset(0, 100, 255), Byteset(1, 180, 200),
                       Byteset(7910, 23, 98), Byteset(20999, 55, 123),
                   ]),
                   extra_args=["--debug"]),
        # read_no_tao_tail_with_ecc: --dao option
        GoldenTest("read_no_tao_tail_with_ecc", action="--dao -r", new_image=True,
                   ecc="master_ecc",
                   sim_cd=SimCD(source="master", damage=[
                       Erase("20998-20999"),
                   ]),
                   extra_args=["--debug"]),
        # read_more_missing_at_end_with_ecc
        GoldenTest("read_more_missing_at_end_with_ecc", action="-r", new_image=True,
                   ecc="master_ecc",
                   sim_cd=SimCD(source="master", damage=[
                       Erase("20954-20999"),
                   ]),
                   extra_args=["--debug"]),
        # read_with_hardware_failure
        GoldenTest("read_with_hardware_failure", action="-r", new_image=True,
                   ecc="no.ecc",
                   sim_cd=SimCD(source="master", damage=[
                       Erase("5000:hardware failure"),
                       Erase("6000:hardware failure"),
                   ]),
                   extra_args=["--debug"]),
        # read_with_ignored_hardware_failure
        GoldenTest("read_with_ignored_hardware_failure", new_image=True,
                   action="-r --ignore-fatal-sense",
                   ecc="no.ecc",
                   sim_cd=SimCD(source="master", damage=[
                       Erase("5000:hardware failure"),
                       Erase("6000:hardware failure"),
                   ]),
                   extra_args=["--debug"]),
        # read_medium_with_dsm
        GoldenTest("read_medium_with_dsm", action="-r", new_image=True,
                   ecc="no.ecc",
                   sim_cd=SimCD(source="master", damage=[
                       Erase("4999:pass as dead sector marker"),
                       Erase("5005:pass as dead sector marker"),
                       Erase("5007:pass as dead sector marker"),
                   ]),
                   extra_args=["--debug"]),
        # read_medium_with_dsm_verbose
        GoldenTest("read_medium_with_dsm_verbose", action="-r -v", new_image=True,
                   ecc="no.ecc",
                   sim_cd=SimCD(source="master", damage=[
                       Erase("4999:pass as dead sector marker"),
                       Erase("5005:pass as dead sector marker"),
                       Erase("5007:pass as dead sector marker"),
                   ]),
                   extra_args=["--debug"]),
        # read_multipass_partial_success: 3 passes, some recovered in pass 3
        GoldenTest("read_multipass_partial_success", new_image=True,
                   action="--read-medium=3 -r",
                   ecc="no.ecc",
                   sim_cd=SimCD(source="master", damage=[
                       Erase("15800-16199"),
                       Erase("15900-16099:readable in pass 3"),
                   ]),
                   extra_args=["--debug"]),
    ]

    # ------------------------------------------------------------------
    # Tests requiring special setup (plain methods)
    # ------------------------------------------------------------------

    def test_read_no_ecc_good_file(self, tmp_path):
        """Read into existing and complete image file."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        shutil.copy2(master, tmp_iso)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_no_ecc_good_file", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_no_device(self, tmp_path):
        """Read image from non-existent device."""
        master = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
            "--debug",
            "--sim-cd={}".format(master), "--fixed-speed-values",
            "--spinup-delay=0", "-d", NON_EXISTENT_DEVICE, "-r",
        ]
        _run_golden_compare("read_no_device", cmd, tmp_path)

    @_SKIP_CHMOD_WIN
    def test_read_no_device_access(self, tmp_path):
        """Read image from device with insufficient permissions."""
        master = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        fake_dev = os.path.join(str(tmp_path), "sdz")
        with open(fake_dev, "w"):
            pass
        os.chmod(fake_dev, 0o000)
        try:
            cmd = [
                "--regtest", "--no-progress",
                "-i{}".format(tmp_iso),
                "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
                "--debug",
                "--sim-cd={}".format(master), "--fixed-speed-values",
                "--spinup-delay=0", "-d", fake_dev, "-r",
            ]
            _run_golden_compare("read_no_device_access", cmd, tmp_path)
        finally:
            os.chmod(fake_dev, 0o644)

    def test_read_defective_no_ecc_again(self, tmp_path):
        """Read defective image again with 1 sector skip and pre-existing partial image."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [
            Erase("100-200"), Erase("766"), Erase("2410"),
        ])
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [
            Erase("96-207"), Erase("752-767"), Erase("2400-2415"),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-j", "1", "-r",
        ]
        _run_golden_compare("read_defective_no_ecc_again", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_truncated_no_ecc(self, tmp_path):
        """Complete truncated image with no ecc data."""
        master = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [Truncate(ISOSIZE - 560)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
            "--debug",
            "--sim-cd={}".format(master), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_truncated_no_ecc", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_truncated_no_ecc_again(self, tmp_path):
        """Complete truncated image from defective media."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Erase("20800-20875")])
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [Truncate(ISOSIZE - 560)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-j", "1", "-r",
        ]
        _run_golden_compare("read_truncated_no_ecc_again", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_multipass_no_ecc_again(self, tmp_path):
        """Complete truncated image from defective media with multiple passes."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [
            Erase("20800-20875"), Erase("3000-3045"),
        ])
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [
            Truncate(ISOSIZE - 560), Erase("2980-3120"),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--read-medium=3", "--spinup-delay=0", "-j", "1", "-r",
        ]
        _run_golden_compare("read_multipass_no_ecc_again", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_with_gap_no_ecc(self, tmp_path):
        """Complete truncated image with reading gap."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [Truncate(10000)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r15000-end",
        ]
        _run_golden_compare("read_with_gap_no_ecc", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_with_ecc_good_file(self, tmp_path):
        """Read with ecc into existing and complete image file."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        shutil.copy2(master, tmp_iso)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_with_ecc_good_file", cmd, tmp_path,
                            image_path=tmp_iso)

    @_SKIP_CHMOD_WIN
    def test_read_with_no_permission_for_ecc(self, tmp_path):
        """Read with no permission to access ecc file."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs01-tmp.ecc")
        shutil.copy2(master_ecc, tmp_ecc)
        os.chmod(tmp_ecc, 0o000)
        try:
            cmd = [
                "--regtest", "--no-progress",
                "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
                "--debug",
                "--sim-cd={}".format(master), "--fixed-speed-values",
                "--spinup-delay=0", "-r",
            ]
            _run_golden_compare("read_with_no_permission_for_ecc", cmd, tmp_path,
                                image_path=tmp_iso)
        finally:
            os.chmod(tmp_ecc, 0o644)

    def test_read_shorter_with_ecc(self, tmp_path):
        """Read image shorter than expected with ecc."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Truncate(ISOSIZE - 44)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--ignore-iso-size", "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_shorter_with_ecc", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_longer_with_ecc(self, tmp_path):
        """Read image longer than expected with ecc."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        _append_fixed_random_sequence(sim_iso, 22)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--ignore-iso-size", "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_longer_with_ecc", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_tao_tail_with_ecc(self, tmp_path):
        """Read image, tao tail case, ecc data."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        _append_fixed_random_sequence(sim_iso, 1)
        _apply_damage(sim_iso, [Erase("21000-21001")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--ignore-iso-size", "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_tao_tail_with_ecc", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_wrong_fp_with_ecc(self, tmp_path):
        """Re-read image with wrong fingerprint, ecc data."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        # Create partial image (800 sectors) with modified fingerprint
        with open(master, "rb") as src, open(tmp_iso, "wb") as dst:
            dst.write(src.read(2048 * 800))
        _apply_damage(tmp_iso, [Byteset(16, 100, 200)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_wrong_fp_with_ecc", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_with_double_ecc(self, tmp_path):
        """Read image with RS02 data and a RS01 ecc file."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs01-tmp.ecc")
        shutil.copy2(master, sim_iso)
        # Augment with RS02
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(sim_iso), "-mRS02",
            "-n{}".format(ISOSIZE + 5000), "-c", check=True,
        )
        # Create RS01 ecc
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(sim_iso), "-e", tmp_ecc,
            "-c", "-n", REDUNDANCY, check=True,
        )
        # Introduce CRC error in RS02 area
        _apply_damage(sim_iso, [Byteset(25910, 100, 200)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_with_double_ecc", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_with_incompatible_ecc(self, tmp_path):
        """Read image with ecc file requiring a newer dvdisaster version."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs01-tmp.ecc")
        shutil.copy2(master, sim_iso)
        # Create ECC
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(sim_iso), "-e", tmp_ecc,
            "-c", "-n", REDUNDANCY, check=True,
        )
        # Modify version bytes
        _apply_damage(tmp_ecc, [
            Byteset(0, 88, 220),
            Byteset(0, 89, 65),
            Byteset(0, 90, 15),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_with_incompatible_ecc", cmd, tmp_path,
                            image_path=tmp_iso,
                            ignore_line_re=r'^\*          $')

    def test_read_second_pass_with_ecc_success(self, tmp_path):
        """Re-read medium with ecc, successful completion."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [Erase("15800-16199")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_second_pass_with_ecc_success", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_second_pass_with_crc_error(self, tmp_path):
        """Re-read medium with CRC error."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Byteset(15830, 8, 3)])
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [Erase("15800-16199")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_second_pass_with_crc_error", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_medium_with_dsm_in_image(self, tmp_path):
        """Complete image with uncorrectable dead sector markers (displacement)."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, _DAMAGE_DSM1)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_medium_with_dsm_in_image", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_medium_with_dsm_in_image_verbose(self, tmp_path):
        """Complete image with uncorrectable dead sector markers, verbose."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, _DAMAGE_DSM1)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "-v",
        ]
        _run_golden_compare("read_medium_with_dsm_in_image_verbose", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_medium_with_dsm_in_image2(self, tmp_path):
        """Complete image with uncorrectable dead sector markers (wrong fingerprint)."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, _DAMAGE_DSM2)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r",
        ]
        _run_golden_compare("read_medium_with_dsm_in_image2", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_read_medium_with_dsm_in_image2_verbose(self, tmp_path):
        """Complete image with uncorrectable dead sector markers (wrong fp), verbose."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, _DAMAGE_DSM2)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "-v",
        ]
        _run_golden_compare("read_medium_with_dsm_in_image2_verbose", cmd, tmp_path,
                            image_path=tmp_iso)


# ---------------------------------------------------------------------------
# Test Suite: Adaptive Reading
# ---------------------------------------------------------------------------

class TestRS01ReadAdaptive(GoldenTestSuite):
    codec = "RS01"
    codec_prefix = "RS01"
    master = "rs01-master.iso"
    master_ecc = "rs01-master.ecc"
    image_size = 21000
    redundancy = "normal"

    tests = []  # All adaptive tests as plain methods due to varied patterns

    def test_adaptive_good(self, tmp_path):
        """Read good image with adaptive reading."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug",
            "--sim-cd={}".format(master), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "--adaptive-read",
        ]
        _run_golden_compare("adaptive_good", cmd, tmp_path, image_path=tmp_iso)

    def test_adaptive_no_ecc(self, tmp_path):
        """Read image without ecc data, adaptive reading."""
        master = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
            "--debug",
            "--sim-cd={}".format(master), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "--adaptive-read",
        ]
        _run_golden_compare("adaptive_no_ecc", cmd, tmp_path)

    def test_adaptive_no_device(self, tmp_path):
        """Read image from non-existent device, adaptive reading."""
        master = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
            "--debug",
            "--sim-cd={}".format(master), "--fixed-speed-values",
            "--spinup-delay=0", "-d", NON_EXISTENT_DEVICE,
            "-r", "--adaptive-read",
        ]
        _run_golden_compare("adaptive_no_device", cmd, tmp_path)

    @_SKIP_CHMOD_WIN
    def test_adaptive_no_device_access(self, tmp_path):
        """Read image from device with insufficient permissions, adaptive reading."""
        master = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        fake_dev = os.path.join(str(tmp_path), "sdz")
        with open(fake_dev, "w"):
            pass
        os.chmod(fake_dev, 0o000)
        try:
            cmd = [
                "--regtest", "--no-progress",
                "-i{}".format(tmp_iso),
                "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
                "--debug",
                "--sim-cd={}".format(master), "--fixed-speed-values",
                "--spinup-delay=0", "-d", fake_dev,
                "-r", "--adaptive-read",
            ]
            _run_golden_compare("adaptive_no_device_access", cmd, tmp_path)
        finally:
            os.chmod(fake_dev, 0o644)

    def test_adaptive_defective_no_ecc(self, tmp_path):
        """Read defective image without ecc, adaptive reading."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [
            Erase("100-200"), Erase("766"), Erase("2410"),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "--adaptive-read", "-v",
        ]
        _run_golden_compare("adaptive_defective_no_ecc", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_adaptive_defective_large_skip(self, tmp_path):
        """Read defective image with large sector skip, adaptive reading."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [
            Erase("1600-1615"), Erase("6400-10000"),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "-j", "256", "--adaptive-read", "-v",
        ]
        _run_golden_compare("adaptive_defective_large_skip", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_adaptive_truncated_no_ecc(self, tmp_path):
        """Complete truncated image with adaptive reading, no ecc."""
        master = _ensure_master()
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [Truncate(ISOSIZE - 560)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
            "--debug",
            "--sim-cd={}".format(master), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "--adaptive-read",
        ]
        _run_golden_compare("adaptive_truncated_no_ecc", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_adaptive_truncated_no_ecc_again(self, tmp_path):
        """Complete truncated image from defective media, adaptive reading."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Erase("20800-20875")])
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [Truncate(ISOSIZE - 560)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "-v", "--adaptive-read",
        ]
        _run_golden_compare("adaptive_truncated_no_ecc_again", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_adaptive_with_gap_no_ecc(self, tmp_path):
        """Complete truncated image with gap, adaptive reading."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [Truncate(10000)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r15000-end", "--adaptive-read",
        ]
        _run_golden_compare("adaptive_with_gap_no_ecc", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_adaptive_with_gap_no_ecc2(self, tmp_path):
        """Complete truncated image with gap, area ends before medium size."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [Truncate(10000)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r15000-19999", "--adaptive-read",
        ]
        _run_golden_compare("adaptive_with_gap_no_ecc2", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_adaptive_with_gap_no_ecc3(self, tmp_path):
        """Complete truncated image with gap, overlapping already read part."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        shutil.copy2(master, tmp_iso)
        _apply_damage(tmp_iso, [Truncate(10000)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r9000-15000", "--adaptive-read",
        ]
        _run_golden_compare("adaptive_with_gap_no_ecc3", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_adaptive_new_with_range_no_ecc(self, tmp_path):
        """Read new image with given range, adaptive reading."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r10000-15000", "--adaptive-read",
        ]
        _run_golden_compare("adaptive_new_with_range_no_ecc", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_adaptive_new_with_invalid_range_no_ecc(self, tmp_path):
        """Read new image with invalid range, adaptive reading."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r10000-55000", "--adaptive-read",
        ]
        _run_golden_compare("adaptive_new_with_invalid_range_no_ecc", cmd, tmp_path)

    @_SKIP_CHMOD_WIN
    def test_adaptive_with_no_permission_for_ecc(self, tmp_path):
        """Read with no permission to access ecc file, adaptive reading."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs01-tmp.ecc")
        shutil.copy2(master_ecc, tmp_ecc)
        os.chmod(tmp_ecc, 0o000)
        try:
            cmd = [
                "--regtest", "--no-progress",
                "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
                "--debug",
                "--sim-cd={}".format(master), "--fixed-speed-values",
                "--spinup-delay=0", "-r", "--adaptive-read",
            ]
            _run_golden_compare("adaptive_with_no_permission_for_ecc", cmd, tmp_path,
                                image_path=tmp_iso)
        finally:
            os.chmod(tmp_ecc, 0o644)

    def test_adaptive_crc_errors_with_ecc(self, tmp_path):
        """Read image with CRC errors, adaptive reading."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [
            Byteset(0, 100, 255), Byteset(1, 180, 200),
            Byteset(7910, 23, 98), Byteset(20999, 55, 123),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "--adaptive-read",
        ]
        _run_golden_compare("adaptive_crc_errors_with_ecc", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_adaptive_shorter_with_ecc(self, tmp_path):
        """Read shorter image with ecc, adaptive reading."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Truncate(ISOSIZE - 44)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug", "--ignore-iso-size",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "--adaptive-read",
        ]
        _run_golden_compare("adaptive_shorter_with_ecc", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_adaptive_longer_with_ecc(self, tmp_path):
        """Read longer image with ecc, adaptive reading."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        _append_fixed_random_sequence(sim_iso, 22)
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug", "--ignore-iso-size",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "--adaptive-read",
        ]
        _run_golden_compare("adaptive_longer_with_ecc", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_adaptive_tao_tail_with_ecc(self, tmp_path):
        """Read image, tao tail case, adaptive reading."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        _append_fixed_random_sequence(sim_iso, 1)
        _apply_damage(sim_iso, [Erase("21000-21001")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug", "--ignore-iso-size",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "--adaptive-read",
        ]
        _run_golden_compare("adaptive_tao_tail_with_ecc", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_adaptive_no_tao_tail_with_ecc(self, tmp_path):
        """Read image with --dao, adaptive reading."""
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Erase("20998-20999")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug", "--dao",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "--adaptive-read",
        ]
        _run_golden_compare("adaptive_no_tao_tail_with_ecc", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_adaptive_wrong_fp_with_ecc(self, tmp_path):
        """Re-read image with wrong fingerprint, adaptive reading.

        Note: the golden file is named 'adapive_wrong_fp_with_ecc' (typo in original).
        """
        master = _ensure_master()
        master_ecc = _ensure_master_ecc()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        # Create partial image (800 sectors) with modified fingerprint
        with open(master, "rb") as src, open(tmp_iso, "wb") as dst:
            dst.write(src.read(2048 * 800))
        _apply_damage(tmp_iso, [Byteset(16, 100, 200)])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(master_ecc),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "--adaptive-read",
        ]
        # Note: golden file has typo "adapive" (missing 't')
        _run_golden_compare("adapive_wrong_fp_with_ecc", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_adaptive_with_double_ecc(self, tmp_path):
        """Read image with RS02 data and RS01 ecc file, adaptive reading."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs01-tmp.ecc")
        shutil.copy2(master, sim_iso)
        # Augment with RS02
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(sim_iso), "-mRS02",
            "-n{}".format(ISOSIZE + 5000), "-c", check=True,
        )
        # Create RS01 ecc
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(sim_iso), "-e", tmp_ecc,
            "-c", "-n", REDUNDANCY, check=True,
        )
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "--adaptive-read",
        ]
        _run_golden_compare("adaptive_with_double_ecc", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_adaptive_with_incompatible_ecc(self, tmp_path):
        """Read image with ecc requiring newer version, adaptive reading."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        tmp_ecc = os.path.join(str(tmp_path), "rs01-tmp.ecc")
        shutil.copy2(master, sim_iso)
        # Create ECC
        _run_dvdisaster(
            "--regtest", "--debug", "--set-version", SETVERSION,
            "-i{}".format(sim_iso), "-e", tmp_ecc,
            "-c", "-n", REDUNDANCY, check=True,
        )
        # Modify version bytes
        _apply_damage(tmp_ecc, [
            Byteset(0, 88, 220),
            Byteset(0, 89, 65),
            Byteset(0, 90, 15),
        ])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(tmp_ecc),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "--adaptive-read",
        ]
        _run_golden_compare("adaptive_with_incompatible_ecc", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_adaptive_with_hardware_failure(self, tmp_path):
        """Read image with simulated hardware failure, adaptive reading."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Erase("5000:hardware failure")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(os.path.join(_ISODIR, "no.iso")),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "--adaptive-read",
        ]
        _run_golden_compare("adaptive_with_hardware_failure", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_adaptive_with_ignored_hardware_failure(self, tmp_path):
        """Read image ignoring hardware failure, adaptive reading."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Erase("5000:hardware failure")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(os.path.join(_ISODIR, "no.iso")),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "--adaptive-read", "--ignore-fatal-sense",
        ]
        _run_golden_compare("adaptive_with_ignored_hardware_failure", cmd, tmp_path,
                            image_path=tmp_iso)

    def test_adaptive_medium_with_dsm(self, tmp_path):
        """Read medium with dead sector markers, adaptive reading."""
        master = _ensure_master()
        sim_iso = os.path.join(str(tmp_path), "sim.iso")
        tmp_iso = os.path.join(str(tmp_path), "rs01-tmp.iso")
        shutil.copy2(master, sim_iso)
        _apply_damage(sim_iso, [Erase("4999:pass as dead sector marker")])
        cmd = [
            "--regtest", "--no-progress",
            "-i{}".format(tmp_iso), "-e{}".format(os.path.join(_ISODIR, "no.ecc")),
            "--debug",
            "--sim-cd={}".format(sim_iso), "--fixed-speed-values",
            "--spinup-delay=0", "-r", "--adaptive-read",
        ]
        _run_golden_compare("adaptive_medium_with_dsm", cmd, tmp_path,
                            image_path=tmp_iso)
