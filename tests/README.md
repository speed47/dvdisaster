# dvdisaster Test Suite

This directory contains the Python/pytest integration test suite for dvdisaster. All regression tests have been migrated from the legacy bash-based framework in `regtest/` to this declarative, maintainable Python framework. The bash tests in `regtest/config.txt` are all disabled; the golden reference files in `regtest/database/` are still used by the Python tests.

## Quick Start

```bash
# Build dvdisaster first
./configure --with-gui=no && make -j$(nproc)

# Run all tests
pip install pytest
python3 -m pytest tests/ -v

# Run a single codec
python3 -m pytest tests/test_rs01.py -v

# Run a specific test class
python3 -m pytest tests/test_rs02.py::TestRS02Verify -v

# Run a specific test
python3 -m pytest "tests/test_rs01.py::TestRS01Verify::test_golden[good]" -v
```

Master images are cached in `/var/tmp/regtest/` (created on first run, reused thereafter). The first run is slower because it creates these images.

## Test Count

| File | Tests | Status |
|------|------:|--------|
| `test_rs01.py` | 85 | Migrated from bash |
| `test_rs02.py` | 76 | Migrated from bash |
| `test_rs03f.py` | 85 | Migrated from bash |
| `test_rs03i.py` | 142 | Migrated from bash |
| `test_multipass_read.py` | 4 | Semantic tests (all codecs) |
| `test_rs03_recognize.py` | 4 | Semantic tests (RS03 recognition) |
| `test_framework.py` | 28 | Unit tests for the framework itself |
| **Total** | **424** | |

## Architecture

### Framework (`framework.py`)

The framework provides a declarative DSL for defining regression tests. Instead of writing imperative bash scripts, tests are declared as data:

```python
class TestRS01Verify(GoldenTestSuite):
    codec = "RS01"
    codec_prefix = "RS01"
    master = "rs01-master.iso"
    master_ecc = "rs01-master.ecc"

    tests = [
        GoldenTest("good", action="-t", use_master=True, ecc="master_ecc"),
        GoldenTest("missing_sectors", action="-t",
                   damage=[Erase("1500-1673"), Erase("13420-14109")],
                   ecc="master_ecc"),
    ]
```

The `GoldenTestSuite` base class converts each `GoldenTest` entry into a parametrized pytest test case at collection time. The execution flow for each test:

1. Create temp work directory (pytest `tmp_path`)
2. Resolve image path (master, custom, or new empty path)
3. Apply damage operations (erase sectors, set bytes, truncate, pad, append files)
4. Handle simulated CD reading (`sim_cd`) if specified
5. Run: `dvdisaster --regtest --no-progress -i<image> [-e<ecc>] <action> [extra_args]`
6. Clean output (strip headers, paths, memory-leak lines, `ignore_lines` patterns)
7. Compare cleaned output against golden reference file
8. Verify MD5 checksums if specified in the golden file

### Damage Operations

Atomic operations applied to images before running dvdisaster:

| Operation | Description | Implementation |
|-----------|-------------|----------------|
| `Erase("15800-16199")` | Zero out sector range | CLI: `--erase` |
| `Erase("100:hardware failure")` | Erase with sim label | CLI: `--erase` |
| `Erase("500", fill_unreadable=0)` | Erase + fill pattern | CLI: `--erase --fill-unreadable` |
| `Byteset(4096, 100, 17)` | Set byte at sector/offset | CLI: `--byteset` |
| `Truncate(20500)` | Truncate image to N sectors | CLI: `--truncate` |
| `PadBytes(55)` | Append N zero bytes | Python-level |
| `PadSectors(17)` | Append N×2048 zero bytes | Python-level |
| `AppendFile(path)` | Append file contents | Python-level |

CLI operations invoke `dvdisaster --debug` to modify the image. Python-level operations modify the file directly with `open()`.

### SimCD (Simulated CD Reading)

For scan and read tests, `SimCD` simulates reading from a damaged disc:

```python
GoldenTest("scan_missing_sectors", action="-s",
           sim_cd=SimCD(source="master",
                        damage=[Erase("1000-1049"), Erase("21230")]))
```

When `sim_cd` is present, the framework:
1. Copies the source image to `sim.iso` in the work directory
2. Applies sim-cd damage operations
3. Adds `--debug --sim-cd=<path> --fixed-speed-values --spinup-delay=0` to the command

### Golden Files

Golden reference files live in `regtest/database/` and have this format:

```
<image_md5 or "ignore">
<ecc_md5 or "ignore">
<expected output...>
```

Platform-specific variants are supported: `RS01_good.darwin` (macOS), `RS01_good.win` (Windows), with fallback to the base file.

### Output Cleaning

Before comparison, output is cleaned to ensure reproducibility:

- Strip version/copyright header (first 3 lines)
- Remove `dvdisaster: No memory leaks found.` lines
- Remove temp directory paths and Windows drive letters
- Remove `regtest/` prefix from paths
- Remove lines matching `ignore_lines` regex patterns (mirrors bash `IGNORE_LOG_LINE`)

## Test Files

### `test_rs01.py` -- RS01 (Separate ECC File)

RS01 creates a separate `.ecc` file alongside the image. Tests use a 21000-sector master image and a pre-built master ECC file.

| Class | Tests | What it covers |
|-------|------:|----------------|
| `TestRS01Verify` | 28 | Image+ECC verification: good, truncated, missing sectors, bad bytes, CRC errors, cross-codec detection |
| `TestRS01Create` | 11 | ECC creation: normal, with existing image/ECC, missing image, permission errors, non-blocksize images |
| `TestRS01Repair` | 18 | Image repair: truncated, missing sectors, bad bytes, permission errors, with wrong ECC fingerprint |
| `TestRS01Scan` | 22 | Simulated CD scanning: good/defective media, range errors, skip sizes, hardware failures, DSM |
| `TestRS01ReadLinear` | 38 | Linear reading: good/defective media, range errors, TAO tail, fingerprint mismatch, CRC errors, multipass |
| `TestRS01ReadAdaptive` | 25 | Adaptive reading: same scenarios using divide-and-conquer algorithm, hardware failures, DSM |

Notable test patterns:
- **Cross-codec detection**: Verifying RS02/RS03 images with RS01 tool correctly identifies the codec mismatch
- **TAO tail**: Trailing garbage bytes from Track-At-Once burning; dvdisaster should detect and handle them
- **Hardware failure simulation**: `Erase("5000:hardware failure")` marks sectors as having drive-level read errors
- **Multipass reading**: `--read-medium=3` tests multiple reading passes with progressively more sectors recovered

### `test_rs02.py` -- RS02 (Augmented Image)

RS02 embeds ECC data directly in the image (no separate `.ecc` file). Tests use a 30000-sector raw image augmented to 35000 sectors.

| Class | Tests | What it covers |
|-------|------:|----------------|
| `TestRS02Strip` | 2 | Stripping ECC data from augmented images |
| `TestRS02Verify` | 31 | Image verification: good, truncated, padded, bad/missing headers, modulo glitch, cross-codec |
| `TestRS02Create` | 18 | ECC creation: normal, from other codecs, after read, partial read, non-blocksize |
| `TestRS02Repair` | 25 | Image repair: truncated, trailing bytes/TAO/garbage, large file, permission errors, cross-codec |
| `TestRS02Scan` | 22 | Simulated CD scanning: good/defective media, TAO tail, modulo glitch, cross-codec |
| `TestRS02ReadLinear` | 28 | Linear reading: good/defective media, TAO tail, modulo glitch, CRC errors, cross-codec |
| `TestRS02ReadAdaptive` | 22 | Adaptive reading: same scenarios using divide-and-conquer |

Notable test patterns:
- **Modulo glitch**: Simulates pre-0.79.5 dvdisaster headers where sector size info was missing. Uses `_apply_old_style_headers()` to patch 21 header positions with corrected checksums
- **Header modulo glitch (HMG) image**: A 274300-sector image specifically sized to trigger the modulo-glitch code path
- **Large file test**: Creates a 223456-sector (~450MB) image to test repair across all three RS02 sections (data, CRC, ECC)
- **Cross-codec creation**: Tests creating RS02 ECC on images that already have RS01/RS03 ECC data

### `test_rs03f.py` -- RS03f (File-Based ECC, RS03 Algorithm)

RS03f creates a separate `.ecc` file (like RS01) but uses the RS03 algorithm. Tests use a 21000-sector master image with 20-root redundancy.

| Class | Tests | What it covers |
|-------|------:|----------------|
| `TestRS03fVerify` | 34 | Image+ECC verification: good, truncated, padded, plus56 bytes, CRC errors, missing sectors, DSM, ecc file manipulation |
| `TestRS03fCreate` | 14 | ECC creation: normal, missing image, permissions, plus56, after read, cross-codec (RS01/RS02/RS03i/RS03f) |
| `TestRS03fRepair` | 26 | Image repair: good, missing sectors, border cases, plus56 variants, extra sectors, truncation, ecc damage |
| `TestRS03fScan` | 18 | Simulated CD scanning: good/defective media, TAO tail, incompatible ecc, header damage, cross-section errors |
| `TestRS03fReadLinear` | 18 | Linear reading: good/defective media, TAO tail, incompatible ecc, CRC errors, DSM, multipass |
| `TestRS03fReadAdaptive` | 1 | Adaptive reading: good media baseline |

### `test_rs03i.py` -- RS03i (Image-Embedded ECC, RS03 Algorithm)

RS03i embeds ECC data directly in the image (like RS02) using the RS03 algorithm. Tests use a 21000-sector raw image augmented to ~25000 sectors. Includes resource-intensive tests with large master images (~460MB) for header recovery and root discovery.

| Class | Tests | What it covers |
|-------|------:|----------------|
| `TestRS03iStrip` | 2 | Stripping ECC data from augmented images |
| `TestRS03iVerify` | 48 | Image verification: good, truncated, padded, plus56, CRC errors, missing sectors, DSM, header recovery, root discovery, cross-codec, custom -n |
| `TestRS03iCreate` | 20 | ECC creation: normal, permissions, from other codecs, non-blocksize, layer multiple, no padding, after read |
| `TestRS03iFix` | 27 | Image repair: good, truncated, trailing bytes/TAO/garbage, border cases, cross-codec, header recovery, custom -n with bruteforce |
| `TestRS03iScan` | 33 | Simulated CD scanning: good/defective, TAO tail, header recovery, root discovery, cross-codec, padding errors |
| `TestRS03iReadLinear` | 29 | Linear reading: good/defective, header recovery (exhaustive), cross-codec, DSM, multipass, padding errors |

Notable RS03i-specific patterns:
- **Large master images**: 235219-sector (~460MB) images for header recovery and root discovery tests
- **Root discovery**: Tests that verify dvdisaster can determine the ECC root count (8 or 170) from a damaged image
- **Custom -n**: Tests with explicit `-n` override for ECC size, including bruteforce header recovery
- **Layer multiple / no padding**: Edge cases where image size aligns exactly with RS03 internal layout

### `test_multipass_read.py` -- Multipass Reading (All Codecs)

Semantic tests (not golden-file) for multipass reading across RS01, RS02, RS03f (file mode), and RS03i (image mode). These replace flaky bash tests where golden-file comparison was unreliable due to timing-dependent output ordering.

Each test:
1. Creates a 21000-sector image with damaged sectors (15900-16099)
2. Creates codec-specific ECC data
3. Prepares a simulated CD with additional damage and "readable in pass 3" sectors
4. Reads with `--read-medium=3` (3 passes)
5. Asserts semantic properties: CRC errors reported, pass transitions occur, correct final sector counts

### `test_rs03_recognize.py` -- RS03 Recognition Robustness

Semantic tests for RS03 ECC recognition edge cases:
- **BD-RE read-back**: Image padded with extra sectors (the drive returns full formatted capacity)
- **Headerless recognition**: RS03 ECC data found even when the primary header is missing
- **NODM without flag**: Images created with `--no-bdr-defect-management` recognized without the flag

### `test_framework.py` -- Framework Unit Tests

Unit tests for the framework itself (28 tests):
- Damage operation CLI argument generation
- Golden file parsing (MD5 extraction, output extraction)
- Output cleaning (header stripping, path removal, memory-leak filtering)
- `GoldenTest` and `SimCD` dataclass construction

## Bash vs Python: Comparison

### Side-by-Side Example

The same test in both frameworks:

**Bash** (`regtest/rs01.bash`):
```bash
if try "scanning defective media, no ecc" scan_defective_no_ecc; then (
  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase 100-200 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase 766 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase 2410 >>$LOGFILE 2>&1

  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_defective_no_ecc "--spinup-delay=0 -s" $ISODIR/no.iso $ISODIR/no.ecc
) & limit_jobs; fi
```

**Python** (`tests/test_rs01.py`):
```python
GoldenTest("scan_defective_no_ecc", action="-s",
           image="no.iso", ecc="no.ecc",
           sim_cd=SimCD(source="master", damage=[
               Erase("100-200"), Erase("766"), Erase("2410"),
           ]),
           extra_args=["--debug"]),
```

### Comparison Table

| Aspect | Bash (`regtest/`) | Python (`tests/`) |
|--------|-------------------|-------------------|
| **Test count** | 569 (145 RS01 + 150 RS02 + 160 RS03i + 114 RS03f) | 424 (85 RS01 + 76 RS02 + 85 RS03f + 142 RS03i + 8 semantic + 28 framework) |
| **Migration status** | All disabled | All codecs migrated |
| **Test declaration** | Imperative shell scripts (~100-300 LOC each) | Declarative DSL (data + base class) |
| **Lines per test** | 5-15 lines of bash | 2-5 lines of Python (golden tests) |
| **Assertion style** | Golden-file diff only | Hybrid: golden-file + semantic assertions |
| **Timing-sensitive tests** | Flaky (exact output match) | Stable (semantic property checks) |
| **Golden files** | Same `regtest/database/` directory | Same files, reused in place |
| **Master image caching** | `/var/tmp/regtest/` | Same directory, compatible |
| **Parallelism** | `MAX_JOBS` background subshells | pytest-xdist (future), sequential now |
| **Platform variants** | Manual `.darwin`/`.win` suffix files | Same files, auto-detected |
| **Output cleaning** | `grep -v`, `sed`, shell filters | `clean_output()` function |
| **CI integration** | `regtest/runtests.sh` in workflow | `python3 -m pytest tests/` in workflow |
| **Run time (RS01+RS02)** | ~45 min | ~75 min (sequential; parallelism planned) |
| **Error messages** | Binary pass/fail | Unified diff with context |
| **Skip mechanism** | `config.txt` toggles | `pytest.skip()` with reason |
| **Dependencies** | bash, coreutils | Python 3.6+, pytest |
| **Debugging** | Manual `diff` against newlog | `--tb=long`, `-s` for stdout, `--pdb` |

### What Stayed the Same

- Golden reference files in `regtest/database/` are reused as-is (no conversion needed)
- Master images cached in `/var/tmp/regtest/` are shared between both frameworks
- The `--regtest` and `--no-progress` flags are used identically
- Output cleaning logic mirrors the bash `run_regtest` filtering exactly
- Both frameworks run in CI via `.github/workflows/tests.yml`

### What Changed

- **Timing-sensitive tests** (multipass reading) use semantic assertions instead of golden-file comparison, eliminating platform-dependent flakiness
- **Boilerplate elimination**: damage setup, sim-cd wiring, golden file lookup, and output cleaning are handled by the framework, not repeated in each test
- **`--debug` for sim-cd** is now automatically added by the framework (required by `--fixed-speed-values`), reducing a common source of test-writing errors
- **`ignore_lines`** parameter in `clean_output()` replaces bash `IGNORE_LOG_LINE` for filtering timing-dependent output lines
- **`AppendFile`** damage operation replaces `cat file >> image` for TAO tail tests

## Migration Status

All codecs have been fully migrated from bash to Python. All bash tests are disabled in `regtest/config.txt`.

| Codec | Bash Tests (disabled) | Python Tests | Status |
|-------|----------------------|-------------|--------|
| RS01 | 145 | 85 | Migrated |
| RS02 | 150 | 76 | Migrated |
| RS03f | 114 | 85 | Migrated |
| RS03i | 160 | 142 | Migrated |

Python test counts differ from bash because:
- Many bash tests that used multi-step setup map to compact declarative `GoldenTest` entries (fewer methods, same coverage)
- Some duplicate bash entries were consolidated
- `read_multipass_ecc_partial_success` tests live in the shared `test_multipass_read.py`

## Adding New Tests

### Golden test (declarative)

Add to the `tests` list in the appropriate `GoldenTestSuite` subclass:

```python
GoldenTest("my_new_test", action="-t",
           damage=[Erase("100-200"), Byteset(300, 0, 255)],
           ecc="master_ecc"),
```

Then run the test to generate output, and create the golden file from the actual output.

### Semantic test (plain method)

Add a method to the test class:

```python
def test_my_complex_scenario(self, tmp_path):
    """Tests that can't use golden comparison."""
    # Custom setup, assertions, etc.
```

### Framework test

Add to `test_framework.py` for testing framework internals (damage ops, parsing, cleaning).
