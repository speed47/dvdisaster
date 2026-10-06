"""
Pytest framework for dvdisaster integration tests.

These tests invoke the dvdisaster binary and verify its behavior.

Requirements:
  - dvdisaster binary built and available at project root
  - pytest (`pip install pytest`)

Run:
  pytest tests/ -v              # skip slow tests (default)
  pytest tests/ -v --run-slow   # include large-image tests
"""

import os
import shutil
import subprocess
import tempfile

import pytest

from framework import pytest_generate_tests, filter_empty_golden_placeholders  # noqa: F401


def pytest_addoption(parser):
    parser.addoption(
        "--run-slow", action="store_true", default=False,
        help="Run slow tests (large-image creation, 3+ min each)"
    )


def pytest_configure(config):
    config.addinivalue_line(
        "markers", "slow: marks tests as slow (large image I/O, deselected by default)"
    )


def pytest_collection_modifyitems(config, items):
    filter_empty_golden_placeholders(items)
    if config.getoption("--run-slow"):
        return
    skip_slow = pytest.mark.skip(reason="need --run-slow option to run")
    for item in items:
        if "slow" in item.keywords:
            item.add_marker(skip_slow)


# Project root: one level up from tests/
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DVDISASTER = os.path.join(PROJECT_ROOT, "dvdisaster")


def _find_binary():
    """Find the dvdisaster binary, checking common locations."""
    candidates = [
        DVDISASTER,
        DVDISASTER + ".exe",  # Windows/MSYS2
    ]
    for path in candidates:
        if os.path.isfile(path) and os.access(path, os.X_OK):
            return path
    return None


@pytest.fixture(scope="session")
def dvdisaster_bin():
    """Path to the dvdisaster binary. Skips all tests if not found."""
    binary = _find_binary()
    if binary is None:
        pytest.skip(
            f"dvdisaster binary not found at {DVDISASTER}. "
            "Build it first: ./configure --with-gui=no && make -j$(nproc)"
        )
    return binary


@pytest.fixture
def work_dir(tmp_path):
    """Provide a temporary working directory for a single test."""
    return tmp_path


def run_dvdisaster(binary, *args, check=True):
    """Run dvdisaster with given arguments, return CompletedProcess.

    Captures stdout+stderr combined (dvdisaster mixes both).
    Raises subprocess.CalledProcessError if check=True and exit code != 0.
    """
    cmd = [binary, "--regtest", "--debug"] + list(args)
    result = subprocess.run(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        timeout=120,
    )
    # Decode output, tolerant of encoding issues
    result.text = result.stdout.decode("utf-8", errors="replace")
    if check and result.returncode != 0:
        raise subprocess.CalledProcessError(
            result.returncode, cmd, output=result.stdout
        )
    return result


def create_random_image(binary, path, sectors):
    """Create a random test image with the given number of sectors."""
    run_dvdisaster(binary, f"-i{path}", f"--random-image", str(sectors))


def augment_image_rs03(binary, path, medium_size=None, extra_args=None):
    """Augment an image with RS03 ECC data.

    Args:
        binary: path to dvdisaster
        path: path to the image file
        medium_size: target medium size (sector count or name like "BDNODM")
        extra_args: additional CLI args as list
    """
    args = [f"-i{path}", "-mRS03", "--set-version", "0.80", "-c"]
    if medium_size is not None:
        args.extend(["-n", str(medium_size)])
    if extra_args:
        args.extend(extra_args)
    run_dvdisaster(binary, *args)


def scan_image(binary, image_path, sim_cd=None, ecc_path=None, extra_args=None):
    """Scan an image and return the output text.

    Args:
        binary: path to dvdisaster
        image_path: path for the output image (-i)
        sim_cd: if set, simulate reading from this file as a CD
        ecc_path: optional ecc file path (-e)
        extra_args: additional CLI args as list

    Returns:
        The combined stdout+stderr output as a string.
    """
    args = ["--spinup-delay=0", "-s", f"-i{image_path}"]
    if sim_cd:
        args.extend([f"--sim-cd={sim_cd}", "--fixed-speed-values"])
    if ecc_path:
        args.extend([f"-e{ecc_path}"])
    if extra_args:
        args.extend(extra_args)
    result = run_dvdisaster(binary, *args, check=False)
    return result.text


def erase_sectors(binary, image_path, spec):
    """Erase sectors in an image file.

    Args:
        spec: sector spec like "1000" or "1000-1010" or "1000:hardware failure"
    """
    run_dvdisaster(binary, f"-i{image_path}", f"--erase", spec)
