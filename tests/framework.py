"""
Golden-test framework for dvdisaster regression tests.

Provides a declarative DSL for defining regression tests that compare
dvdisaster CLI output against golden reference files from regtest/database/.
"""

import hashlib
import os
import platform
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass, field
from typing import List, Optional, Tuple

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

_PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
_DVDISASTER = os.path.join(_PROJECT_ROOT, "dvdisaster")
_DATABASE = os.path.join(_PROJECT_ROOT, "regtest", "database")
_ISODIR = "/var/tmp/regtest"
_TMPDIR = "/dev/shm" if os.path.isdir("/dev/shm") else "/var/tmp"

# ---------------------------------------------------------------------------
# 1. Damage Operations
# ---------------------------------------------------------------------------


@dataclass
class Erase:
    """Erase sectors: ``--erase <spec>``."""
    spec: str
    fill_unreadable: Optional[int] = None

    def cli_args(self) -> List[str]:
        args = ["--erase", self.spec]
        if self.fill_unreadable is not None:
            args.append("--fill-unreadable={}".format(self.fill_unreadable))
        return args


@dataclass
class Byteset:
    """Set a byte in a sector: ``--byteset sector,offset,value``."""
    sector: int
    offset: int
    value: int

    def cli_args(self) -> List[str]:
        return ["--byteset", "{},{},{}".format(self.sector, self.offset, self.value)]


@dataclass
class Truncate:
    """Truncate image to N sectors: ``--truncate=N``."""
    sectors: int

    def cli_args(self) -> List[str]:
        return ["--truncate={}".format(self.sectors)]


@dataclass
class PadBytes:
    """Append N zero bytes (Python-level, not CLI)."""
    count: int

    @property
    def pad_size(self) -> int:
        return self.count


@dataclass
class PadSectors:
    """Append N * 2048 zero bytes (Python-level, not CLI)."""
    count: int

    @property
    def pad_size(self) -> int:
        return self.count * 2048


@dataclass
class AppendFile:
    """Append contents of a file to the image (Python-level, not CLI)."""
    path: str


# ---------------------------------------------------------------------------
# 2. Golden File Parser
# ---------------------------------------------------------------------------


def parse_golden_file(path):
    # type: (str) -> Tuple[Optional[str], Optional[str], str]
    """Parse a golden reference file.

    Returns (image_md5, ecc_md5, expected_output).
    MD5 values are None when the golden file says "ignore".
    expected_output is everything from line 3 onwards (after the two MD5 lines).
    """
    with open(path, "r") as f:
        lines = f.readlines()

    image_md5 = lines[0].strip() if len(lines) > 0 else None
    ecc_md5 = lines[1].strip() if len(lines) > 1 else None

    if image_md5 == "ignore":
        image_md5 = None
    if ecc_md5 == "ignore":
        ecc_md5 = None

    expected_output = "".join(lines[2:])
    return image_md5, ecc_md5, expected_output


def resolve_golden_path(base_path):
    # type: (str) -> str
    """Resolve platform-specific golden file variant.

    Checks for .darwin or .win suffixes on macOS/Windows respectively,
    falling back to the base path.
    """
    system = platform.system()
    if system == "Darwin":
        variant = base_path + ".darwin"
        if os.path.isfile(variant):
            return variant
    elif system == "Windows" or "MSYSTEM" in os.environ:
        variant = base_path + ".win"
        if os.path.isfile(variant):
            return variant
    return base_path


# ---------------------------------------------------------------------------
# 3. Output Cleaning
# ---------------------------------------------------------------------------

# Pre-compiled patterns
_RE_MEMLEAK = re.compile(r"^dvdisaster: No memory leaks found\.\s*$", re.MULTILINE)
_RE_WIN_PATH = re.compile(r"[A-Z]:/[A-Za-z0-9_/-]+/")
_RE_GH_ACTIONS_TMP = re.compile(r"[-A-Za-z0-9_~]+/AppData/Local/Temp/")


def clean_output(text, tmp_dirs=None, strip_header=False, ignore_lines=None):
    # type: (str, Optional[List[str]], bool, Optional[List[str]]) -> str
    """Clean dvdisaster output to match golden-file comparison.

    Mirrors the filtering done by ``run_regtest`` in ``regtest/common.bash``.

    Args:
        ignore_lines: regex patterns; lines matching any pattern are removed
            (mirrors bash ``IGNORE_LOG_LINE``).
    """
    # Normalize CRLF to LF — the mingw build writes CRLF line endings to
    # stdout, but golden files use LF.
    text = text.replace("\r\n", "\n")

    if strip_header:
        # Remove first 3 lines (version/copyright header) — matches ``tail -n +4``
        lines = text.split("\n", 3)
        if len(lines) > 3:
            text = lines[3]
        else:
            text = ""

    # Remove memory-leak-OK lines entirely (grep -v equivalent)
    lines = text.split("\n")
    lines = [l for l in lines if not _RE_MEMLEAK.match(l)]
    text = "\n".join(lines)

    # Remove Windows drive-letter paths: ``sed -re "s=[A-Z]:/[A-Za-z0-9_/-]+/==g"``
    text = _RE_WIN_PATH.sub("", text)

    # Remove per-test temp dirs (with trailing slashes)
    if tmp_dirs:
        for d in tmp_dirs:
            d = d.rstrip("/").rstrip("\\")
            text = text.replace(d + "/", "")
            text = text.replace(d + "\\", "")

    # Remove ISODIR and TMPDIR paths (``sed "s=$TMPDIR/*==g;s=$ISODIR/*==g"``)
    # The bash ``/*`` means zero or more slashes, so match with or without trailing /
    for d in [_ISODIR, _TMPDIR]:
        d = d.rstrip("/")
        text = text.replace(d + "/", "")
        text = text.replace(d, "")

    # Remove GitHub Actions temp paths
    text = _RE_GH_ACTIONS_TMP.sub("", text)

    # Remove 'regtest/' prefix
    text = text.replace("regtest/", "")

    # Remove lines matching ignore patterns (mirrors bash IGNORE_LOG_LINE)
    if ignore_lines:
        import re as _re
        compiled = [_re.compile(p) for p in ignore_lines]
        lines = text.split("\n")
        lines = [l for l in lines if not any(r.search(l) for r in compiled)]
        text = "\n".join(lines)

    return text


# ---------------------------------------------------------------------------
# 4. GoldenTest, SimCD, CreateECC dataclasses
# ---------------------------------------------------------------------------


@dataclass
class SimCD:
    """Simulate reading from a CD image."""
    source: str = "master"
    damage: Optional[List] = None


@dataclass
class CreateECC:
    """Create ECC data."""
    method: Optional[str] = None
    redundancy: Optional[str] = None
    output: Optional[str] = None
    ecc_size: Optional[int] = None


@dataclass
class GoldenTest:
    """A single golden-file test case."""
    name: str
    action: str
    damage: Optional[List] = None
    use_master: bool = False
    image: Optional[str] = None
    ecc: Optional[str] = None
    sim_cd: Optional[SimCD] = None
    create_ecc: Optional[CreateECC] = None
    extra_args: Optional[List[str]] = None
    ecc_damage: Optional[List] = None
    chmod_image: Optional[int] = None
    chmod_ecc: Optional[int] = None
    new_image: bool = False


# ---------------------------------------------------------------------------
# 5. GoldenTestSuite Base Class
# ---------------------------------------------------------------------------


def _md5_file(path):
    # type: (str) -> str
    """Compute MD5 hex digest of a file."""
    h = hashlib.md5()
    with open(path, "rb") as f:
        while True:
            chunk = f.read(65536)
            if not chunk:
                break
            h.update(chunk)
    return h.hexdigest()


def _find_binary():
    # type: () -> str
    """Find the dvdisaster binary."""
    for candidate in [_DVDISASTER, _DVDISASTER + ".exe"]:
        if os.path.isfile(candidate) and os.access(candidate, os.X_OK):
            return candidate
    raise RuntimeError(
        "dvdisaster binary not found at {}. "
        "Build it first: ./configure --with-gui=no && make".format(_DVDISASTER)
    )


def _run_dvdisaster(*args, **kwargs):
    # type: (*str, **bool) -> Tuple[int, str]
    """Run dvdisaster and return (returncode, combined_output_text).

    Unlike conftest.run_dvdisaster, this does NOT add --debug by default.
    """
    binary = _find_binary()
    check = kwargs.get("check", False)
    cmd = [binary] + list(args)
    result = subprocess.run(
        cmd,
        stdin=subprocess.DEVNULL,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        timeout=300,
    )
    text = result.stdout.decode("utf-8", errors="replace")
    if check and result.returncode != 0:
        raise subprocess.CalledProcessError(
            result.returncode, cmd, output=result.stdout
        )
    return result.returncode, text


def _apply_damage(image_path, damage_ops):
    # type: (str, List) -> None
    """Apply damage operations to an image file."""
    for op in damage_ops:
        if isinstance(op, AppendFile):
            with open(op.path, "rb") as src, open(image_path, "ab") as dst:
                dst.write(src.read())
        elif isinstance(op, (PadBytes, PadSectors)):
            with open(image_path, "ab") as f:
                f.write(b"\x00" * op.pad_size)
        elif isinstance(op, (Erase, Byteset, Truncate)):
            _run_dvdisaster(
                "--regtest", "--debug",
                "-i{}".format(image_path),
                *op.cli_args(),
                check=True
            )
        else:
            raise TypeError("Unknown damage op: {}".format(type(op)))


class GoldenTestSuite:
    """Base class for golden-file test suites.

    Subclasses declare class-level attributes and a ``tests`` list.
    The ``test_golden`` method is parametrized automatically via
    ``pytest_generate_tests``.
    """

    codec = ""          # type: str
    codec_prefix = ""   # type: str
    master = ""         # type: str
    master_ecc = None   # type: Optional[str]
    image_size = 21000  # type: int
    redundancy = None   # type: Optional[str]
    tests = []          # type: List[GoldenTest]


    def __init_subclass__(cls, **kwargs):
        super().__init_subclass__(**kwargs)
        # Store the tests list so pytest_generate_tests can find it
        if "tests" in cls.__dict__:
            cls._golden_tests = list(cls.__dict__["tests"])
        else:
            cls._golden_tests = []

    def _ensure_master(self):
        # type: () -> str
        """Create master image in ISODIR if it doesn't exist. Returns path."""
        os.makedirs(_ISODIR, exist_ok=True)
        path = os.path.join(_ISODIR, self.master)
        if not os.path.isfile(path):
            _run_dvdisaster(
                "--regtest", "--debug",
                "-i{}".format(path),
                "--random-image", str(self.image_size),
                check=True,
            )
        return path

    def _ensure_master_ecc(self):
        # type: () -> Optional[str]
        """Create master ECC file in ISODIR if it doesn't exist. Returns path."""
        if self.master_ecc is None:
            return None
        os.makedirs(_ISODIR, exist_ok=True)
        master_iso = self._ensure_master()
        path = os.path.join(_ISODIR, self.master_ecc)
        if not os.path.isfile(path):
            args = [
                "--regtest", "--debug", "--set-version", "0.80",
                "-i{}".format(master_iso),
                "-e{}".format(path),
                "-c",
            ]
            if self.redundancy:
                args.extend(["-n", self.redundancy])
            _run_dvdisaster(*args, check=True)
        return path

    def _resolve_image(self, test, work_dir):
        # type: (GoldenTest, str) -> Tuple[str, bool]
        """Resolve image path for a test.

        Returns (path, is_temp) where is_temp indicates the file is a
        temporary copy that can be modified/deleted.
        """
        if test.use_master:
            return self._ensure_master(), False
        if test.image is not None:
            return os.path.join(_ISODIR, test.image), False
        if test.new_image:
            # For reading tests: provide path but don't create the file
            tmp_name = self.master.replace("master", "tmp")
            dest = os.path.join(work_dir, tmp_name)
            return dest, True
        # Default: copy master to work_dir
        master = self._ensure_master()
        tmp_name = self.master.replace("master", "tmp")
        dest = os.path.join(work_dir, tmp_name)
        shutil.copy2(master, dest)
        return dest, True

    def _resolve_ecc(self, test):
        # type: (GoldenTest) -> Optional[str]
        """Resolve ECC file path for a test."""
        if test.ecc is None:
            return None
        if test.ecc == "master_ecc":
            return self._ensure_master_ecc()
        return os.path.join(_ISODIR, test.ecc)

    def _run_golden_test(self, test, tmp_path):
        # type: (GoldenTest, str) -> None
        """Execute a golden-file test and assert results."""
        import pytest

        work_dir = str(tmp_path)

        # 1. Resolve golden file
        golden_base = os.path.join(
            _DATABASE,
            "{}_{}".format(self.codec_prefix, test.name),
        )
        golden_path = resolve_golden_path(golden_base)
        if not os.path.isfile(golden_path):
            pytest.skip("Golden file not found: {}".format(golden_path))

        image_md5, ecc_md5, expected_output = parse_golden_file(golden_path)

        # 2. Resolve image and ecc
        image_path, image_is_temp = self._resolve_image(test, work_dir)
        ecc_path = self._resolve_ecc(test)

        chmod_files = []

        try:
            # 3. Apply damage to image (only if temp copy)
            if test.damage and image_is_temp:
                _apply_damage(image_path, test.damage)

            # 4. Apply ecc_damage: copy ecc first, then damage
            ecc_work_path = ecc_path
            if test.ecc_damage and ecc_path:
                # Use "tmp" name to match bash regtest convention ($TMPECC)
                ecc_basename = os.path.basename(ecc_path).replace("master", "tmp")
                ecc_tmp = os.path.join(work_dir, ecc_basename)
                shutil.copy2(ecc_path, ecc_tmp)
                ecc_work_path = ecc_tmp
                _apply_damage(ecc_work_path, test.ecc_damage)

            # 5. Apply chmod
            if test.chmod_image is not None and image_is_temp:
                os.chmod(image_path, test.chmod_image)
                chmod_files.append(image_path)
            if test.chmod_ecc is not None and ecc_work_path:
                os.chmod(ecc_work_path, test.chmod_ecc)
                chmod_files.append(ecc_work_path)

            # 6. Build command args
            cmd_args = ["--regtest", "--no-progress"]
            cmd_args.append("-i{}".format(image_path))
            if ecc_work_path:
                cmd_args.append("-e{}".format(ecc_work_path))

            # 7. Extra args (added early so --debug is set before
            #    --fixed-speed-values which requires debug mode)
            if test.extra_args:
                cmd_args.extend(test.extra_args)

            # 8. Handle sim_cd
            if test.sim_cd is not None:
                if test.sim_cd.source == "master":
                    sim_src = self._ensure_master()
                else:
                    sim_src = os.path.join(_ISODIR, test.sim_cd.source)
                sim_path = os.path.join(work_dir, "sim.iso")
                shutil.copy2(sim_src, sim_path)
                if test.sim_cd.damage:
                    _apply_damage(sim_path, test.sim_cd.damage)
                cmd_args.extend([
                    "--debug",
                    "--sim-cd={}".format(sim_path),
                    "--fixed-speed-values",
                    "--spinup-delay=0",
                ])

            # 9. Handle create_ecc
            if test.create_ecc is not None:
                cmd_args.extend(["--debug", "--set-version", "0.80"])
                if test.create_ecc.method:
                    cmd_args.extend(["-m{}".format(test.create_ecc.method)])
                if test.create_ecc.redundancy:
                    cmd_args.extend(["-n", test.create_ecc.redundancy])
                if test.create_ecc.output:
                    cmd_args.extend(["-o", test.create_ecc.output])
                if test.create_ecc.ecc_size is not None:
                    cmd_args.extend(["-n", str(test.create_ecc.ecc_size)])

            # 10. Add action
            cmd_args.extend(test.action.split())

            # 11. Redundancy (for create actions like -c)
            if self.redundancy and "-c" in test.action and test.create_ecc is None:
                cmd_args.extend(["-n", self.redundancy])

            # 12. Run
            returncode, raw_output = _run_dvdisaster(*cmd_args)

            # 13. Clean output
            tmp_dirs = [work_dir, _TMPDIR, _ISODIR]
            cleaned = clean_output(
                raw_output,
                tmp_dirs=tmp_dirs,
                strip_header=True,
            )

            # 14. Compare output
            if cleaned != expected_output:
                # Show diff for debugging
                import difflib
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

            # 15. Check MD5 sums
            if image_md5 is not None and os.path.isfile(image_path):
                actual_md5 = _md5_file(image_path)
                assert actual_md5 == image_md5, (
                    "Image MD5 mismatch for '{}': expected {}, got {}".format(
                        test.name, image_md5, actual_md5
                    )
                )

            if ecc_md5 is not None and ecc_work_path and os.path.isfile(ecc_work_path):
                actual_md5 = _md5_file(ecc_work_path)
                assert actual_md5 == ecc_md5, (
                    "ECC MD5 mismatch for '{}': expected {}, got {}".format(
                        test.name, ecc_md5, actual_md5
                    )
                )

        finally:
            # 16. Restore chmod'd files so cleanup can delete them
            for f in chmod_files:
                try:
                    os.chmod(f, 0o644)
                except OSError:
                    pass

    def test_golden(self, golden_test, tmp_path):
        """Parametrized test method — called once per GoldenTest."""
        self._run_golden_test(golden_test, tmp_path)


def pytest_generate_tests(metafunc):
    """Parametrize ``test_golden`` from the suite's ``_golden_tests`` list."""
    if "golden_test" in metafunc.fixturenames:
        cls = metafunc.cls
        if cls is not None and hasattr(cls, "_golden_tests"):
            tests = cls._golden_tests
            metafunc.parametrize(
                "golden_test",
                tests,
                ids=[t.name for t in tests],
            )


def filter_empty_golden_placeholders(items):
    """Drop the placeholder pytest emits for ``test_golden`` when a suite's
    ``_golden_tests`` list is empty. Several test classes (e.g. creation and
    strip suites) inherit ``test_golden`` for a uniform base class API but
    implement their checks as standalone methods — they have no golden-table
    entries. Without this filter, each such class contributes one fake
    ``test_golden[...]`` skip that inflates the skip count without
    corresponding to any real test body.

    Pytest signals "empty parametrize" by setting the param value to its
    internal ``NotSetType`` sentinel; detect that by class name so we don't
    import a private API.

    Helper — callers invoke this from their ``pytest_collection_modifyitems``
    hook. Not itself a pytest hook, to avoid collisions with other hooks
    (e.g. the ``--run-slow`` gate in tests/conftest.py).
    """
    def _is_empty_golden_placeholder(item):
        if getattr(item, "originalname", None) != "test_golden":
            return False
        callspec = getattr(item, "callspec", None)
        if callspec is None:
            return False
        val = callspec.params.get("golden_test")
        return val is not None and type(val).__name__ == "NotSetType"

    items[:] = [item for item in items if not _is_empty_golden_placeholder(item)]
