# Small on-disk fixtures for recognition tests at real BD-R capacities.
# Only one CRC block survives; the header and the other sectors are zero.
# These are recognition fixtures, not images suitable for full verification.

function rs03_fixture_le()
{
    local value=$1 width=$2 i octal
    for ((i=0; i<width; i++)); do
        printf -v octal '\\%03o' "$((value & 255))"
        printf '%b' "$octal"
        value=$((value >> 8))
    done
}

function make_bdr_recognition_fixture()
(
    set -e -o pipefail
    local image=$1 spl=$2
    local block="${image}.crc" byte bit crc=4294967295

    # Serialize the CrcBlock format explicitly in little-endian order.
    # The zero-filled prefix is the unused table of data-sector checksums.
    dd if=/dev/zero of="$block" bs=2048 count=1
    {
        printf '*dvdisaster*RS03'
        rs03_fixture_le 0 4       # methodFlags: augmented image
        rs03_fixture_le 8000 4    # creatorVersion
        rs03_fixture_le 7900 4    # neededVersion
        rs03_fixture_le 16 4      # fpSector
        rs03_fixture_le 0 8      # mediumFP and mediumSum (unused)
        rs03_fixture_le 0 8
        rs03_fixture_le 0 8
        rs03_fixture_le 0 8
        rs03_fixture_le "$((200*spl-2))" 8  # dataSectors
        rs03_fixture_le 2048 4   # inLast
        rs03_fixture_le 201 4    # dataBytes, including the CRC layer
        rs03_fixture_le 54 4     # eccBytes
        rs03_fixture_le 0 4      # alignment padding
        rs03_fixture_le "$spl" 8 # sectorsPerLayer
        rs03_fixture_le 0x4c5047 4 # selfCRC placeholder
    } | dd of="$block" bs=1 seek=1024 conv=notrunc

    # Compute the format's CRC32 (no final complement), independently of
    # the application and the host byte order. All padding is covered.
    for byte in $(od -An -v -tu1 "$block"); do
        crc=$((crc ^ byte))
        for ((bit=0; bit<8; bit++)); do
            crc=$(((crc >> 1) ^ ((crc & 1) * 0xedb88320)))
        done
    done
    rs03_fixture_le "$crc" 4 | dd of="$block" bs=1 seek=1120 conv=notrunc

    # Truncate the logical image from 255 to 250 layers. The image-derived
    # guesses consequently miss the sole CRC block at sector 200*spl.
    # Mark NTFS files sparse explicitly before extending them, too.
    : > "$image"
    if [[ $OS =~ Windows ]]; then
        MSYS2_ARG_CONV_EXCL='*' fsutil.exe sparse setflag "$(cygpath -w "$image")"
    fi
    dd if=/dev/zero of="$image" bs=2048 count=1 seek="$((250*spl-1))" conv=notrunc
    dd if="$block" of="$image" bs=2048 count=1 seek="$((200*spl))" conv=notrunc
)
