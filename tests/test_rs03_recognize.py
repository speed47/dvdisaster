"""
Tests for RS03 recognition robustness.

These tests verify that RS03 ECC data can be found even when the image
sector count doesn't match the expected medium size. This covers:

- BD-RE read-back: image padded with extra sectors (upstream #97)
- NODM without flag: image created with --no-bdr-defect-management
  but recognized without the flag (upstream #69)

The core fix is in src/rs03-recognize.c: RS03RecognizeImage() now tries
all known medium sizes as candidates instead of guessing a single one.
"""

import os

import pytest

from conftest import (
    augment_image_rs03,
    create_random_image,
    erase_sectors,
    run_dvdisaster,
    scan_image,
)

# Small image size for fast tests (in 2K sectors)
IMAGE_SECTORS = 21000
# RS03 augmented size matching the regtest default
ECC_SIZE = 25000


class TestBDREReadBackMismatch:
    """Issue #97: RS03 ECC not recognized after BD-RE read-back.

    Scenario: Image augmented at ECC_SIZE (25000 sectors), but when
    read back from a BD-RE disc, the image has extra trailing sectors
    (the drive returns the full formatted capacity). The recognize path
    must try multiple candidate layer sizes to find the ECC data.
    """

    def test_padded_image_recognized(self, dvdisaster_bin, work_dir):
        """RS03 data should be found in an image padded with extra sectors."""
        image = str(work_dir / "test.iso")
        scan_out = str(work_dir / "scan.iso")

        # Create and augment image
        create_random_image(dvdisaster_bin, image, IMAGE_SECTORS)
        augment_image_rs03(dvdisaster_bin, image, medium_size=ECC_SIZE)

        # Verify augmentation produced a reasonable image
        # (RS03 layout rounding means size may not be exactly ECC_SIZE*2048)
        original_size = os.path.getsize(image)
        assert original_size > IMAGE_SECTORS * 2048

        # Pad with 5000 extra zero sectors (simulates BD-RE read-back)
        padding_sectors = 5000
        with open(image, "ab") as f:
            f.write(b"\x00" * (padding_sectors * 2048))

        padded_size = os.path.getsize(image)
        assert padded_size == original_size + padding_sectors * 2048

        # Scan the padded image — should still find RS03 data
        output = scan_image(
            dvdisaster_bin, scan_out, sim_cd=image
        )

        assert "RS03" in output, (
            f"RS03 not found in padded image scan output:\n{output}"
        )
        # Should not report "no RS03 data found"
        assert "no RS03 data found" not in output.lower(), (
            f"RS03 recognition failed on padded image:\n{output}"
        )

    def test_heavily_padded_image(self, dvdisaster_bin, work_dir):
        """RS03 data should be found even with significant padding."""
        image = str(work_dir / "test.iso")
        scan_out = str(work_dir / "scan.iso")

        create_random_image(dvdisaster_bin, image, IMAGE_SECTORS)
        augment_image_rs03(dvdisaster_bin, image, medium_size=ECC_SIZE)

        # Pad to double size
        with open(image, "ab") as f:
            f.write(b"\x00" * (ECC_SIZE * 2048))

        output = scan_image(
            dvdisaster_bin, scan_out, sim_cd=image
        )

        assert "RS03" in output, (
            f"RS03 not found in heavily padded image:\n{output}"
        )


class TestHeaderlessRecognition:
    """Issue #69/#97: RS03 ECC found via candidate search when header is missing.

    When the ECC header is erased or unreadable, RS03RecognizeImage() must
    fall back to the multi-candidate layer size search. This tests that the
    exhaustive search finds the ECC data even when the header is gone.

    Note: We use small custom -n sizes rather than BDNODM/BD sizes because
    real BD sizes (12M+ sectors) would create ~24GB images, too large for CI.
    The candidate search mechanism is the same regardless of size.
    """

    def test_headerless_image_recognized(self, dvdisaster_bin, work_dir):
        """RS03 data should be found even when the ECC header is erased."""
        image = str(work_dir / "test.iso")
        scan_out = str(work_dir / "scan.iso")

        # Create and augment image at a custom size
        create_random_image(dvdisaster_bin, image, IMAGE_SECTORS)
        augment_image_rs03(dvdisaster_bin, image, medium_size=ECC_SIZE)

        # Erase the ECC header sector to force the candidate search path
        erase_sectors(dvdisaster_bin, image, str(IMAGE_SECTORS))

        # Scan — should find RS03 data via exhaustive candidate search
        output = scan_image(
            dvdisaster_bin,
            scan_out,
            sim_cd=image,
            extra_args=["-v"],
        )

        # The multi-candidate search should rediscover the format
        assert "no RS03 data found" not in output.lower(), (
            f"RS03 recognition failed with erased header:\n{output}"
        )

    def test_headerless_padded_image_recognized(self, dvdisaster_bin, work_dir):
        """RS03 data found when header is erased AND image is padded."""
        image = str(work_dir / "test.iso")
        scan_out = str(work_dir / "scan.iso")

        create_random_image(dvdisaster_bin, image, IMAGE_SECTORS)
        augment_image_rs03(dvdisaster_bin, image, medium_size=ECC_SIZE)

        # Erase the ECC header
        erase_sectors(dvdisaster_bin, image, str(IMAGE_SECTORS))

        # Pad with extra sectors (simulates BD-RE read-back with missing header)
        with open(image, "ab") as f:
            f.write(b"\x00" * (3000 * 2048))

        output = scan_image(
            dvdisaster_bin,
            scan_out,
            sim_cd=image,
            extra_args=["-v"],
        )

        assert "no RS03 data found" not in output.lower(), (
            f"RS03 recognition failed with erased header + padding:\n{output}"
        )
