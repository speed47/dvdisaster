#!/usr/bin/env bash

. common.bash

ISOSIZE=21000
REDUNDANCY="-n normal"

MASTERISO=$ISODIR/rs01-master.iso
MASTERECC=$ISODIR/rs01-master.ecc
TMPISO=$ISODIR/rs01-tmp.iso
TMPECC=$ISODIR/rs01-tmp.ecc
SIMISO=$ISODIR/rs01-sim.iso

CODEC_PREFIX=RS01

# Create master image and ecc file

if ! file_exists $MASTERISO; then
    echo "$NEWVER --debug -i$MASTERISO --random-image $ISOSIZE" >>$LOGFILE
    $NEWVER --debug -i$MASTERISO --random-image $ISOSIZE >>$LOGFILE 2>&1
    echo -e "$FILE_MSG"
    FILE_MSG=""
fi

if ! file_exists $MASTERECC; then
    echo "$NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$MASTERECC -c $REDUNDANCY" >>$LOGFILE
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$MASTERECC -c $REDUNDANCY >>$LOGFILE 2>&1
    echo -e "$FILE_MSG"
    FILE_MSG=""
fi

ISO_PLUS56=$ISODIR/rs01-plus56_bytes.iso
ECC_PLUS56=$ISODIR/rs01-plus56_bytes.ecc
if ! file_exists $ISO_PLUS56; then
    cp $MASTERISO $ISO_PLUS56
    dd if=/dev/zero of=/tmp/padding count=1 bs=56 >>$LOGFILE 2>&1
    cat /tmp/padding >>$ISO_PLUS56
    rm -f /tmp/padding
    echo -e "$FILE_MSG"
    FILE_MSG=""
fi

if ! file_exists $ECC_PLUS56; then
    $NEWVER --debug --set-version $SETVERSION -i$ISO_PLUS56 -e$ECC_PLUS56 -c $REDUNDANCY >>$LOGFILE 2>&1
    echo -e "$FILE_MSG"
    FILE_MSG=""
fi

### Verification tests

echo "# Verify tests"

# Test good files
if try "good image" good; then

  run_regtest good "-t" $MASTERISO $MASTERECC
fi

# Test good files
if try "good image, quick test" good_quick; then
  run_regtest good_quick "-tq" $MASTERISO $MASTERECC
fi

# Test with neither image nor ecc file
if try "missing files" no_files; then
   run_regtest no_files "-t" $ISODIR/no.iso  $ISODIR/no.ecc
fi

# Test with missing image, ecc file
if try "missing image" no_image; then
   run_regtest no_image "-t" $ISODIR/no.iso  $MASTERECC
fi

# Test with good image, no ecc file
if try "missing ecc" no_ecc; then
   run_regtest no_ecc "-t" $MASTERISO $ISODIR/no.ecc
fi

# Test with missing sectors, no ecc file
if try "defective image, no ecc" defective_image_no_ecc; then
   cp $MASTERISO $TMPISO
   $NEWVER -i$TMPISO --debug --erase 1000-1049 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --erase 11230 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --erase 12450-12457 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --byteset 13444,0,154 >>$LOGFILE 2>&1

  run_regtest defective_image_no_ecc "-t" $TMPISO $ISODIR/no.ecc
fi

# Test with good image and ecc file, both not multiple of 2048
if try "image with 56 extra bytes" plus56_bytes; then
  run_regtest plus56_bytes "-t" $ISO_PLUS56 $ECC_PLUS56
fi

# Test with good image not multiple of 2048, no ecc file
if try "image with 56 extra bytes, no ecc" image_plus56_bytes; then
  run_regtest image_plus56_bytes "-t" $ISO_PLUS56 $ISODIR/no.ecc
fi

# Test with no image, ecc for image not multiple of 2048 
if try "only ecc for image with 56 extra bytes" ecc_plus56_bytes; then
  run_regtest ecc_plus56_bytes "-t" $ISODIR/no.iso $ECC_PLUS56
fi

# Test with normal image, ecc for image not multiple of 2048 
if try "normal image, ecc file plus 56 bytes" normal_image_ecc_plus56b; then
  run_regtest normal_image_ecc_plus56b "-t" $MASTERISO $ECC_PLUS56
fi

# Test with image not multiple of 2048, normal ecc file
if try "image not multiple of 2048, normal ecc file" image_plus56b_normal_ecc; then
  run_regtest image_plus56b_normal_ecc "-t" $ISO_PLUS56 $MASTERECC
fi

# Test with image a few bytes shorter than ecc
if try "image a few bytes shorter then ecc" image_few_bytes_shorter; then

  cp $MASTERISO $TMPISO
  dd if=/dev/zero of=/tmp/padding count=1 bs=55 >>$LOGFILE 2>&1
  cat /tmp/padding >>$TMPISO
  rm -f /tmp/padding

  run_regtest image_few_bytes_shorter "-t" $TMPISO $ECC_PLUS56
fi

# Test with image a few bytes longer than ecc
if try "image a few bytes longer then ecc" image_few_bytes_longer; then
  cp $MASTERISO $TMPISO
  dd if=/dev/zero of=/tmp/padding count=1 bs=57 >>$LOGFILE 2>&1
  cat /tmp/padding >>$TMPISO
  rm -f /tmp/padding

  run_regtest image_few_bytes_longer "-t" $TMPISO $ECC_PLUS56
fi

# Image is a few bytes shorter (original multiple of 2048)
if try "image a few bytes truncated" truncated_by_bytes; then
  dd if=$MASTERISO of=$TMPISO count=1 bs=$((2048*ISOSIZE-7)) >>$LOGFILE 2>&1

  run_regtest truncated_by_bytes "-t" $TMPISO $MASTERECC
fi

# Image is truncated by 5 sectors
if try "truncated image" truncated; then
  cp $MASTERISO $TMPISO
  NEWSIZE=$((ISOSIZE-5))
  $NEWVER -i$TMPISO --debug --truncate=$NEWSIZE >>$LOGFILE 2>&1

  run_regtest truncated "-t" $TMPISO $MASTERECC
fi

# Image contains 1 extra sector
if try "image with one extra sector" plus1; then
  cp $MASTERISO $TMPISO
  dd if=/dev/zero of=/tmp/padding count=1 bs=2048 >>$LOGFILE 2>&1
  cat /tmp/padding >>$TMPISO
  rm -f /tmp/padding

  run_regtest plus1 "-t" $TMPISO $MASTERECC
fi

# Image contains 17 extra sectors
if try "image with 17 extra sectors" plus17; then
  cp $MASTERISO $TMPISO
  dd if=/dev/zero of=/tmp/padding count=17 bs=2048 >>$LOGFILE 2>&1
  cat /tmp/padding >>$TMPISO
  rm -f /tmp/padding

  run_regtest plus17 "-t" $TMPISO $MASTERECC
fi

# Image contains 2 rows of missing sectors, a single one
# and a CRC error
if try "image with missing sectors and crc errors" defective_with_ecc; then
  cp $MASTERISO $TMPISO
  $NEWVER -i$TMPISO --debug --erase 1000-1049 >>$LOGFILE 2>&1
  $NEWVER -i$TMPISO --debug --erase 11230 >>$LOGFILE 2>&1
  $NEWVER -i$TMPISO --debug --erase 12450-12457 >>$LOGFILE 2>&1
  $NEWVER -i$TMPISO --debug --byteset 13444,0,154 >>$LOGFILE 2>&1

  run_regtest defective_with_ecc "-t" $TMPISO $MASTERECC
fi

# Image contains just missing blocks
if try "image with missing sectors" missing_sectors_with_ecc; then

  cp $MASTERISO $TMPISO
  $NEWVER -i$TMPISO --debug --erase 1000-1049 >>$LOGFILE 2>&1
  $NEWVER -i$TMPISO --debug --erase 11230 >>$LOGFILE 2>&1
  $NEWVER -i$TMPISO --debug --erase 12450-12457 >>$LOGFILE 2>&1
  
  run_regtest missing_sectors_with_ecc "-t" $TMPISO $MASTERECC
fi

# Image contains just CRC errors
if try "image with crc errors" crc_errors_with_ecc; then
  cp $MASTERISO $TMPISO
  $NEWVER -i$TMPISO --debug --byteset 13444,0,154 >>$LOGFILE 2>&1

  run_regtest crc_errors_with_ecc "-t" $TMPISO $MASTERECC
fi

# CRC error in fingerprint sector
if try "crc in fingerprint sector" crc_in_fingerprint; then
  cp $MASTERISO $TMPISO
  $NEWVER -i$TMPISO --debug --byteset 16,201,55 >>$LOGFILE 2>&1

  run_regtest crc_in_fingerprint "-t" $TMPISO $MASTERECC
fi

# fingerprint sector unreadable
if try "missing fingerprint sector" missing_fingerprint; then
  cp $MASTERISO $TMPISO
  $NEWVER -i$TMPISO --debug --erase 16 >>$LOGFILE 2>&1

  run_regtest missing_fingerprint "-t" $TMPISO $MASTERECC
fi

# Ecc header is missing
if try "Ecc header is missing" missing_ecc_header; then
  cp $MASTERECC $TMPECC
  $NEWVER --debug -i $TMPECC --erase 0 >>$LOGFILE 2>&1

  run_regtest missing_ecc_header "-t" $MASTERISO $TMPECC
fi

# Ecc header has checksum error

if try "checksum error in Ecc header" ecc_header_crc_error; then
  cp $MASTERECC $TMPECC
  $NEWVER --debug -i $TMPECC --byteset 0,22,107 >>$LOGFILE 2>&1

  run_regtest ecc_header_crc_error "-t" $MASTERISO $TMPECC
fi

# Test image containing several uncorrectable dead sector markers
# (sector displacement)

if try "image with uncorrectable dead sector markers" uncorrectable_dsm_in_image; then

  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase 3030 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 3030,353,49 >>$LOGFILE 2>&1 // displaced from sector 3130
  $NEWVER --debug -i$TMPISO --erase 4400 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 4400,353,53 >>$LOGFILE 2>&1 // displaced from sector 4500
  $NEWVER --debug -i$TMPISO --erase 4411 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 4411,353,53 >>$LOGFILE 2>&1 // displaced from sector 4511

  run_regtest uncorrectable_dsm_in_image  "-t" $TMPISO $MASTERECC
fi

# Test image containing several uncorrectable dead sector markers, verbose output

if try "image with uncorrectable dead sector markers, verbose output" uncorrectable_dsm_in_image_verbose; then

  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase 3030 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 3030,353,49 >>$LOGFILE 2>&1 // displaced from sector 3130
  $NEWVER --debug -i$TMPISO --erase 4400 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 4400,353,53 >>$LOGFILE 2>&1 // displaced from sector 4500
  $NEWVER --debug -i$TMPISO --erase 4411 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 4411,353,53 >>$LOGFILE 2>&1 // displaced from sector 4511

  run_regtest uncorrectable_dsm_in_image_verbose  "-t -v" $TMPISO $MASTERECC
fi

# Testimage containing several uncorrectable dead sector markers
# (non matching fingerprint)

if try "image with uncorrectable dead sector markers (2)" uncorrectable_dsm_in_image2; then

  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase 3030 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 3030,416,55 >>$LOGFILE 2>&1 // wrong fingerprint
  $NEWVER --debug -i$TMPISO --byteset 3030,556,32 >>$LOGFILE 2>&1 // changed label
  $NEWVER --debug -i$TMPISO --byteset 3030,557,50 >>$LOGFILE 2>&1 // changed label
  $NEWVER --debug -i$TMPISO --erase 4400 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 4400,416,53 >>$LOGFILE 2>&1 // wrong fingerprint
  $NEWVER --debug -i$TMPISO --byteset 4400,556,32 >>$LOGFILE 2>&1 // changed label
  $NEWVER --debug -i$TMPISO --byteset 4400,557,50 >>$LOGFILE 2>&1 // changed label
  $NEWVER --debug -i$TMPISO --erase 4411 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 4411,416,53 >>$LOGFILE 2>&1 // wrong fingerprint
  $NEWVER --debug -i$TMPISO --byteset 4411,556,32 >>$LOGFILE 2>&1 // changed label
  $NEWVER --debug -i$TMPISO --byteset 4411,557,50 >>$LOGFILE 2>&1 // changed label

  run_regtest uncorrectable_dsm_in_image2 "-t" $TMPISO $MASTERECC
fi

# Test image containing several uncorrectable dead sector markers, verbose
# (non matching fingerprint)

if try "image with uncorrectable dead sector markers (2), verbose output" uncorrectable_dsm_in_image2_verbose; then

  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase 3030 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 3030,416,55 >>$LOGFILE 2>&1 // wrong fingerprint
  $NEWVER --debug -i$TMPISO --byteset 3030,556,32 >>$LOGFILE 2>&1 // changed label
  $NEWVER --debug -i$TMPISO --byteset 3030,557,50 >>$LOGFILE 2>&1 // changed label
  $NEWVER --debug -i$TMPISO --erase 4400 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 4400,416,53 >>$LOGFILE 2>&1 // wrong fingerprint
  $NEWVER --debug -i$TMPISO --byteset 4400,556,32 >>$LOGFILE 2>&1 // changed label
  $NEWVER --debug -i$TMPISO --byteset 4400,557,50 >>$LOGFILE 2>&1 // changed label
  $NEWVER --debug -i$TMPISO --erase 4411 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 4411,416,53 >>$LOGFILE 2>&1 // wrong fingerprint
  $NEWVER --debug -i$TMPISO --byteset 4411,556,32 >>$LOGFILE 2>&1 // changed label
  $NEWVER --debug -i$TMPISO --byteset 4411,557,50 >>$LOGFILE 2>&1 // changed label

  run_regtest uncorrectable_dsm_in_image2_verbose "-t -v" $TMPISO $MASTERECC
fi

### Creation tests

echo "# Creation tests"

# Create ecc file
if try "ecc file creation" ecc_create; then

  extra_args="--debug --set-version $SETVERSION"
  run_regtest ecc_create "-c $REDUNDANCY" $MASTERISO $TMPECC
fi

# Create with missing image
if try "ecc creating with missing image" ecc_missing_image; then
  NO_FILE=$ISODIR/none.iso

  run_regtest ecc_missing_image "-c $REDUNDANCY" $NO_FILE $TMPECC
fi

# Create with no read permission on image
if try "ecc creating with no read permission" ecc_no_read_perm; then
  cp $MASTERISO $TMPISO
  chmod 000 $TMPISO

  run_regtest ecc_no_read_perm "-c $REDUNDANCY" $TMPISO $TMPECC
  rm -f $TMPISO
fi

# Create with no write permission on ecc file
# Should not do any harm at all: Ecc file will
# be recreated with write permissions
if try "ecc creating with no write permission" ecc_no_write_perm; then
  touch $TMPECC
  chmod 400 $TMPECC

  extra_args="--debug --set-version $SETVERSION"
  run_regtest ecc_no_write_perm "-c $REDUNDANCY" $MASTERISO $TMPECC
fi

# Create with image size not being a multiple of 2048

if try "ecc file creation with image not multiple of 2048" ecc_create_plus56; then

  extra_args="--debug --set-version $SETVERSION"
  run_regtest ecc_create_plus56 "-c $REDUNDANCY" $ISO_PLUS56 $TMPECC
fi

# Create test image with unreadable sectors 
if try "ecc creating with unreadable sectors" ecc_missing_sectors; then
  cp $MASTERISO $TMPISO
  $NEWVER -i$TMPISO --debug --erase 1000-1049 >>$LOGFILE 2>&1
  $NEWVER -i$TMPISO --debug --erase 11230 >>$LOGFILE 2>&1
  $NEWVER -i$TMPISO --debug --erase 12450-12457 >>$LOGFILE 2>&1

  run_regtest ecc_missing_sectors "-c $REDUNDANCY" $TMPISO $TMPECC
fi

# Read image and create ecc in the same program call.
# Tests whether CRC and ECC information is handed over correctly.
# NOTE: cache handling is currently disabled and will be fixed in 0.79.6!

if try "read image and create ecc in one call" ecc_create_after_read; then
  cp $MASTERISO $SIMISO

  rm -f $TMPISO $TMPECC
  replace_config read-and-create 1
  extra_args="--debug --set-version $SETVERSION --sim-cd=$SIMISO  --fixed-speed-values"
  run_regtest ecc_create_after_read "-r -c $REDUNDANCY --spinup-delay=0" $TMPISO $TMPECC
fi

# Read image with ecc file and create new (other) ecc in the same program call.
# Tests whether CRC and ECC information is handed over correctly.
# NOTE: cache handling is currently disabled and will be fixed in 0.79.6!

if try "read image with ecc (RS01) and create new ecc" ecc_recreate_after_read_rs01; then
  cp $MASTERISO $SIMISO

  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -e$TMPECC -c -n 8 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --set-version $SETVERSION --sim-cd=$SIMISO  --fixed-speed-values"
  run_regtest ecc_recreate_after_read_rs01 "-r -c $REDUNDANCY --spinup-delay=0" $TMPISO $TMPECC
fi

# Read image with ecc file and create new (other) ecc in the same program call.
# Tests whether CRC and ECC information is handed over correctly.
# Note: RS02 information will not be removed from the image. This ist intentional behaviour.
# NOTE: cache handling is currently disabled and will be fixed in 0.79.6!

if try "read image with ecc (RS02) and create additional ecc file" ecc_recreate_after_read_rs02; then
  cp $MASTERISO $SIMISO

  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -c -mRS02 -n$((ISOSIZE+6000)) >>$LOGFILE 2>&1

  rm -f $TMPISO $TMPECC
  extra_args="--debug --set-version $SETVERSION --sim-cd=$SIMISO  --fixed-speed-values"
  run_regtest ecc_recreate_after_read_rs02 "-r -c $REDUNDANCY --spinup-delay=0" $TMPISO $TMPECC
fi

# Read image with ecc file and create new (other) ecc in the same program call.
# Tests whether CRC and ECC information is handed over correctly.
# Note: RS03 information will not be removed from the image. This ist intentional behaviour.
# NOTE: cache handling is currently disabled and will be fixed in 0.79.6!

if try "read image with ecc (RS03i) and create additional ecc file" ecc_recreate_after_read_rs03i; then
  cp $MASTERISO $SIMISO

  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -c -mRS03 -n$((ISOSIZE+6000)) >>$LOGFILE 2>&1

  rm -f $TMPISO $TMPECC
  extra_args="--debug --set-version $SETVERSION --sim-cd=$SIMISO  --fixed-speed-values"
  run_regtest ecc_recreate_after_read_rs03i "-r -c $REDUNDANCY --spinup-delay=0" $TMPISO $TMPECC
fi

# Read image with ecc file and create new (other) ecc in the same program call.
# Tests whether CRC and ECC information is handed over correctly.
# NOTE: cache handling is currently disabled and will be fixed in 0.79.6!

if try "read image with ecc (RS03f) and create new ecc" ecc_recreate_after_read_rs03f; then
  cp $MASTERISO $SIMISO

  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -e$TMPECC -c -n 8 -mRS03 -o file >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --set-version $SETVERSION --sim-cd=$SIMISO  --fixed-speed-values"
  run_regtest ecc_recreate_after_read_rs03f "-r -c $REDUNDANCY --spinup-delay=0" $TMPISO $TMPECC
fi

# Complete image in a reading pass, then create an ecc file for it.
# Cached checksums must be discarded before creating the ecc.

if try "create ecc after completing partial image" ecc_create_after_partial_read; then
   cp $MASTERISO $SIMISO
   cp $MASTERISO $TMPISO

  $NEWVER --debug -i$TMPISO --erase 1000-1500 >>$LOGFILE 2>&1

  rm -f $TMPECC
  extra_args="--debug --set-version $SETVERSION --sim-cd=$SIMISO  --fixed-speed-values"
  run_regtest ecc_create_after_partial_read "-r -c $REDUNDANCY --spinup-delay=0" $TMPISO $TMPECC
fi

### Fixing tests

echo "# Repair tests"

# Fix good image

if try "fixing good image" fix_good; then

  cp $MASTERISO $TMPISO
  run_regtest fix_good "-f" $TMPISO $MASTERECC
fi

# Fix image without read permission

if try "fixing image without read permission" fix_no_read_perm; then

  cp $MASTERISO $TMPISO
  chmod 000 $TMPISO

  run_regtest fix_no_read_perm "-f" $TMPISO $MASTERECC
  rm -f $TMPISO
fi

# Fix image without read permission for ecc

if try "fixing image without read permission for ecc" fix_no_read_perm_ecc; then

  cp $MASTERISO $TMPISO
  cp $MASTERECC $TMPECC
  chmod 000 $TMPECC

  run_regtest fix_no_read_perm_ecc "-f" $TMPISO $TMPECC
  rm -f $TMPECC
fi

# Fix good image not multiple of 2048

if try "fixing good image not multiple of 2048" fix_good_plus56; then
  cp $ISO_PLUS56 $TMPISO
  
  run_regtest fix_plus56_bytes "-f" $TMPISO $ECC_PLUS56
fi

# Fix image without write permission

if try "fixing image without write permission" fix_no_write_perm; then

  cp $MASTERISO $TMPISO
  chmod 400 $TMPISO

  run_regtest fix_no_write_perm "-f" $TMPISO $MASTERECC
  rm -f $TMPISO
fi

# Fix image with missing sectors

if try "fixing image with missing sectors" fix_missing_sectors; then
  cp $MASTERISO $TMPISO
  $NEWVER -i$TMPISO --debug --erase 0 >>$LOGFILE 2>&1
  $NEWVER -i$TMPISO --debug --erase 190 >>$LOGFILE 2>&1
  $NEWVER -i$TMPISO --debug --erase 192 >>$LOGFILE 2>&1
  $NEWVER -i$TMPISO --debug --erase 590-649 >>$LOGFILE 2>&1
  $NEWVER -i$TMPISO --debug --erase 2000-2139 >>$LOGFILE 2>&1
  $NEWVER -i$TMPISO --debug --erase 2141-2176 >>$LOGFILE 2>&1
  $NEWVER -i$TMPISO --debug --erase $((ISOSIZE-1)) >>$LOGFILE 2>&1

  run_regtest fix_missing_sectors "-f" $TMPISO $MASTERECC

fi

# Fix image with CRC errors

if try "fixing image with crc errors" fix_crc_errors; then
  cp $MASTERISO $TMPISO
  $NEWVER -i$TMPISO --debug --byteset 0,1,1 >>$LOGFILE 2>&1
  $NEWVER -i$TMPISO --debug --byteset 190,200,143 >>$LOGFILE 2>&1
  $NEWVER -i$TMPISO --debug --byteset 1200,100,1 >>$LOGFILE 2>&1
  $NEWVER -i$TMPISO --debug --byteset 1201,100,1 >>$LOGFILE 2>&1
  $NEWVER -i$TMPISO --debug --byteset $((ISOSIZE-1)),500,91 >>$LOGFILE 2>&1

  run_regtest fix_crc_errors "-f" $TMPISO $MASTERECC
fi

# Fix image with additional sectors (TAO case)

if try "fixing image with one additional sector" fix_additional_sector; then
  cp $MASTERISO $TMPISO
  dd if=/dev/zero of=/tmp/padding count=1 bs=2048 >>$LOGFILE 2>&1
  cat /tmp/padding >>$TMPISO
  rm -f /tmp/padding

  run_regtest fix_additional_sector "-f" $TMPISO $MASTERECC
fi

# Fix image with additional sectors (general case)

if try "fixing image with 17 additional sectors" fix_plus17; then
  cp $MASTERISO $TMPISO
  dd if=/dev/zero of=/tmp/padding count=17 bs=2048 >>$LOGFILE 2>&1
  cat /tmp/padding >>$TMPISO
  rm -f /tmp/padding

  run_regtest fix_plus17 "-f" $TMPISO $MASTERECC
fi

# Fix image with additional sectors (general case), with --truncate

if try "fixing image with 17 additional sectors with --truncate" fix_plus17_truncate; then
  cp $MASTERISO $TMPISO
  dd if=/dev/zero of=/tmp/padding count=17 bs=2048 >>$LOGFILE 2>&1
  cat /tmp/padding >>$TMPISO
  rm -f /tmp/padding

  run_regtest fix_plus17_truncate "-f --truncate" $TMPISO $MASTERECC
fi

# Fix image+56bytes 

if try "fixing image with CRC error in 56 additional bytes" fix_plus56; then
  cp $ISO_PLUS56 $TMPISO
  $NEWVER -i$TMPISO --debug --byteset 21000,28,90 >>$LOGFILE 2>&1
  run_regtest fix_plus56 "-f" $TMPISO $ECC_PLUS56
fi

# Fix image+56bytes+more bytes

if try "fixing image with CRC error in 56 additional bytes + few bytes more" fix_plus56_plus17; then
  cp $ISO_PLUS56 $TMPISO
  echo "0123456789abcdef" >>$TMPISO
  $NEWVER -i$TMPISO --debug --byteset 21000,55,90 >>$LOGFILE 2>&1

  run_regtest fix_plus56_plus17 "-f --truncate" $TMPISO $ECC_PLUS56
fi

# Fix image+56bytes+1 sector 

if try "fixing image with CRC error in 56 additional bytes + one sector more" fix_plus56_plus1s; then
  cp $ISO_PLUS56 $TMPISO
  dd if=/dev/zero of=/tmp/padding count=1 bs=2048 >>$LOGFILE 2>&1
  cat /tmp/padding >>$TMPISO
  $NEWVER -i$TMPISO --debug --byteset 21000,55,90 >>$LOGFILE 2>&1

  run_regtest fix_plus56_plus1s "-f --truncate" $TMPISO $ECC_PLUS56
fi

# Fix image+56bytes+2 sectors 

if try "fixing image with CRC error in 56 additional bytes + two sectors more" fix_plus56_plus2s; then
  cp $ISO_PLUS56 $TMPISO
  dd if=/dev/zero of=/tmp/padding count=1 bs=4096 >>$LOGFILE 2>&1
  cat /tmp/padding >>$TMPISO
  $NEWVER -i$TMPISO --debug --byteset 21000,55,90 >>$LOGFILE 2>&1

  run_regtest fix_plus56_plus2s "-f --truncate" $TMPISO $ECC_PLUS56
fi

# Fix image+56bytes+more sectors 

if try "fixing image with CRC error in 56 additional bytes + more sectors" fix_plus56_plus17500; then
  cp $ISO_PLUS56 $TMPISO
  dd if=/dev/zero of=/tmp/padding count=1 bs=17500 >>$LOGFILE 2>&1
  cat /tmp/padding >>$TMPISO
  $NEWVER -i$TMPISO --debug --byteset 21000,55,90 >>$LOGFILE 2>&1

  run_regtest fix_plus56_plus17500 "-f --truncate" $TMPISO $ECC_PLUS56
fi

# Fix truncated image

if try "fixing truncated image" fix_truncated; then
  cp $MASTERISO $TMPISO
  $NEWVER -i$TMPISO --debug --truncate=20731 >>$LOGFILE 2>&1
  run_regtest fix_truncated "-f" $TMPISO $MASTERECC
fi

# Fix truncated image not a multiple of 2048

if try "fixing truncated image not a multiple of 2048" fix_plus56_truncated; then
  cp $ISO_PLUS56 $TMPISO
  $NEWVER -i$TMPISO --debug --truncate=20972 >>$LOGFILE 2>&1
  run_regtest fix_plus56_truncated "-f" $TMPISO $ECC_PLUS56
fi

# Fix truncated image not a multiple of 2048 and a few bytes shorter

if try "fixing image not a multiple of 2048 missing a few bytes" fix_plus56_little_truncated; then
  cp $MASTERISO $TMPISO
  dd if=/dev/zero of=/tmp/padding count=1 bs=50 >>$LOGFILE 2>&1
  cat /tmp/padding >>$TMPISO
  rm -f /tmp/padding

  run_regtest fix_plus56_little_truncated "-f" $TMPISO $ECC_PLUS56
fi

### Scanning tests

echo "# Scanning tests"

# Scan image without error correction data available

if try "scanning image, no ecc data" scan_no_ecc; then

  extra_args="--debug -d sim-cd --sim-cd=$MASTERISO --fixed-speed-values"
  run_regtest scan_no_ecc "--spinup-delay=0 -s" $ISODIR/no.iso $ISODIR/no.ecc
fi

# Scan image from non-existant device
# not applicable to GUI mode since drives are discovered differently there

if try "scanning image, device not existant" scan_no_device; then

  extra_args="--debug -d /dev/sdz --sim-cd=$MASTERISO --fixed-speed-values"
  run_regtest scan_no_device "--spinup-delay=0 -s" $ISODIR/no.iso  $ISODIR/no.ecc
fi

# Scan image from device with insufficient permissions
# not applicable to GUI mode since drives are discovered differently there

if try "scanning image, device access denied" scan_no_device_access; then

  touch $ISODIR/sdz
  chmod 000 $ISODIR/sdz
    
  run_regtest scan_no_device_access "--debug --sim-cd=$MASTERISO --fixed-speed-values --spinup-delay=0 -d $ISODIR/sdz -s" $ISODIR/no.iso  $ISODIR/no.ecc
  rm -f $ISODIR/sdz
fi

# Scan image from defective media without error correction data available
# Will report more missing sectors than the original due to the 16 sector skip default

if try "scanning image, defective media, no ecc data" scan_defective_no_ecc; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase 100-200 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase 766 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase 2410 >>$LOGFILE 2>&1

  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_defective_no_ecc "--spinup-delay=0 -s" $ISODIR/no.iso  $ISODIR/no.ecc
fi

# Scan image from above test again, this time with a 1 sector skip size
# Will report an exact error count of the (damaged) original

if try "scanning image, defective media, no ecc data, reading w/ 1 sec step" scan_defective_no_ecc_again; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase 100-200 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase 766 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase 2410 >>$LOGFILE 2>&1

  replace_config jump 0
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_defective_no_ecc_again "--spinup-delay=0 -j 1 -s" $ISODIR/no.iso  $ISODIR/no.ecc
fi

# Scan image from defective media without error correction data available
# using a large sector skip of 256

if try "scanning image, defective media, large sector skip" scan_defective_large_skip; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase 1600-1615 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase 6400-10000 >>$LOGFILE 2>&1
  
  replace_config jump 256
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_defective_large_skip "--spinup-delay=0 -s -j 256" $ISODIR/no.iso  $ISODIR/no.ecc
fi

# Scan a new image, but only for a partial range.
# range 10000-15000 must be entered manually in the GUI;
# there is no way for pre-configuring it

if try "scanning new image with given range, no ecc data" scan_new_with_range_no_ecc; then

  cp $MASTERISO $SIMISO

  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_new_with_range_no_ecc "--spinup-delay=0 -s10000-15000" $ISODIR/no.iso  $ISODIR/no.ecc
fi

# Scan a new image, but only for an invalid range.
# Makes no sense in GUI mode (invalid value can not be entered)

if try "scanning new image with invalid range, no ecc data" scan_new_with_invalid_range_no_ecc; then

  cp $MASTERISO $SIMISO

  run_regtest scan_new_with_invalid_range_no_ecc "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -s10000-55000" $ISODIR/no.iso  $ISODIR/no.ecc
fi

# Scan image with error correction data available

if try "scanning image, ecc data" scan_with_ecc; then

  extra_args="--debug --sim-cd=$MASTERISO --fixed-speed-values"
  run_regtest scan_with_ecc "--spinup-delay=0 -s" $ISODIR/no.iso  $MASTERECC
fi

# Scan image with non existing error correction file given
# Please note that this fact will be silently ignored; e.g. the image
# will be scanned as if no ecc file was given at all.

if try "scanning image, ecc file does not exist" scan_with_non_existing_ecc; then

  extra_args="--debug --sim-cd=$MASTERISO --fixed-speed-values"
  run_regtest scan_with_non_existing_ecc "--spinup-delay=0 -s" $ISODIR/no.iso  $ISODIR/no_ecc
fi

# Scan image with non accessible error correction file given
# Please note that this fact will be silently ignored; e.g. the image
# will be scanned as if no ecc file was given at all.

if try "scanning image, no permission to access ecc file" scan_with_no_permission_for_ecc; then
  cp $MASTERECC $TMPECC
  chmod 000 $TMPECC

  extra_args="--debug --sim-cd=$MASTERISO --fixed-speed-values"
  run_regtest scan_with_no_permission_for_ecc "--spinup-delay=0 -s" $ISODIR/no.iso  $TMPECC
  rm -f $TMPECC
fi

# Scan image with error correction data available
# and CRC errors

if try "scanning image, crc errors, ecc data" scan_crc_errors_with_ecc; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --byteset 0,100,255 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 1,180,200 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 7910,23,98 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 20999,55,123 >>$LOGFILE 2>&1
    
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_crc_errors_with_ecc "--spinup-delay=0 -s" $ISODIR/no.iso  $MASTERECC
fi

# Scan image with error correction data available
# which is a few sectors shorter than expected.

if try "scanning image, less sectors than expected, ecc data" scan_shorter_with_ecc; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --truncate=$((ISOSIZE-44)) >>$LOGFILE 2>&1

  replace_config ignore-iso-size 1
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_shorter_with_ecc "--ignore-iso-size --spinup-delay=0 -s" $ISODIR/no.iso  $MASTERECC
fi

# Scan image with error correction data available
# which is a few sectors longer than expected.

if try "scanning image, more sectors than expected, ecc data" scan_longer_with_ecc; then

  cp $MASTERISO $SIMISO
  for i in $(seq 22); do cat fixed-random-sequence >>$SIMISO; done
    
  replace_config ignore-iso-size 1
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_longer_with_ecc "--ignore-iso-size --spinup-delay=0 -s" $ISODIR/no.iso  $MASTERECC
fi

# Scan image with error correction data available
# simulating the multisession case with two additional defective sectors trailing the medium

if try "scanning image, tao tail case, ecc data" scan_tao_tail_with_ecc; then

  cp $MASTERISO $SIMISO
  cat fixed-random-sequence >>$SIMISO
  $NEWVER --debug -i$SIMISO --erase 21000-21001 >>$LOGFILE 2>&1
    
  replace_config ignore-iso-size 1
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_tao_tail_with_ecc "--ignore-iso-size --spinup-delay=0 -s" $ISODIR/no.iso  $MASTERECC
fi

# Scan image with error correction data available
# with two defective sectors at the end and the --dao option

if try "scanning image, tao tail case and --dao, ecc data" scan_no_tao_tail_with_ecc; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase 20998-20999 >>$LOGFILE 2>&1
    
  replace_config dao 1
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_no_tao_tail_with_ecc "--dao --spinup-delay=0 -s" $ISODIR/no.iso  $MASTERECC
fi

# Scan image with error correction data available
# and more than two defective sectors at the end

if try "scanning image, more than 2 sectors missing at end, ecc data" scan_more_missing_at_end_with_ecc; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase 20954-20999 >>$LOGFILE 2>&1
    
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_more_missing_at_end_with_ecc "--spinup-delay=0 -s" $ISODIR/no.iso  $MASTERECC
fi

# Scan an augmented image for which an ecc file is also available.
# In that case, the ecc file gets precedence over the embedded ecc.
# To make sure the RS01 data is used we introduce a CRC error in the RS02
# ecc area - this can only be detected by the "outer" RS01 CRC.

if try "scanning image with RS02 data and a RS01 ecc file" scan_with_double_ecc; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -mRS02 -n$((ISOSIZE+5000)) -c >>$LOGFILE 2>&1
  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -e $TMPECC -c $REDUNDANCY >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 25910,100,200 >>$LOGFILE 2>&1

  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_with_double_ecc "--spinup-delay=0 -s" $ISODIR/no.iso  $TMPECC
fi

# Scan an image for which ecc information is available,
# but requiring a newer dvdisaster version.

if try "scanning image ecc file requiring a newer dvdisaster version" scan_with_incompatible_ecc; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -e $TMPECC -c $REDUNDANCY >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --byteset 0,88,220 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --byteset 0,89,65 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --byteset 0,90,15 >>$LOGFILE 2>&1

  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_with_incompatible_ecc "--spinup-delay=0 -s" $ISODIR/no.iso  $TMPECC
fi

# Scan an image with a simulated hardware failure and 
# --ignore-fatal-sense not set.

if try "scanning image with simulated hardware failure" scan_with_hardware_failure; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase "5000:hardware failure" >>$LOGFILE 2>&1

  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_with_hardware_failure "--spinup-delay=0 -s" $ISODIR/no.iso  $ISODIR/no.ecc
fi

# Scan an image with a simulated hardware failure and 
# --ignore-fatal-sense being set.

if try "scanning image, ignoring simulated hardware failure" scan_with_ignored_hardware_failure; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase "5000:hardware failure" >>$LOGFILE 2>&1

  replace_config ignore-fatal-sense 1
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_with_ignored_hardware_failure "--spinup-delay=0 -s --ignore-fatal-sense" $ISODIR/no.iso  $ISODIR/no.ecc
fi

# Scan medium containing dead sector markers

if try "scanning medium containing dead sector markers" scan_medium_with_dsm; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase "4999:pass as dead sector marker" >>$LOGFILE 2>&1

  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_medium_with_dsm "--spinup-delay=0 -s" $ISODIR/no.iso  $ISODIR/no.ecc
fi

### Reading tests (linear)

echo "# Reading tests (linear)"

# Read image without error correction data available

if try "reading image, no ecc data" read_no_ecc; then

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$MASTERISO --fixed-speed-values"
  run_regtest read_no_ecc "--spinup-delay=0 -r" $TMPISO  $ISODIR/no.ecc
fi

# Read into existing and complete image file

if try "reading good image in good file" read_no_ecc_good_file; then
  cp $MASTERISO $SIMISO
  cp $MASTERISO $TMPISO

  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_no_ecc_good_file "--spinup-delay=0 -r" $TMPISO
fi

# Read image from non-existant device
# Makes no sense in GUI mode.

if try "reading image, device not existant" read_no_device; then

  rm -f $TMPISO
  run_regtest read_no_device "--debug --sim-cd=$MASTERISO --fixed-speed-values --spinup-delay=0 -d /dev/sdz -r" $TMPISO  $ISODIR/no.ecc
fi

# Read image from device with insufficient permissions
# not applicable to GUI mode since drives are discovered differently there

if try "reading image, device access denied" read_no_device_access; then

  touch $ISODIR/sdz
  chmod 000 $ISODIR/sdz
    
  rm -f $TMPISO
  run_regtest read_no_device_access "--debug --sim-cd=$MASTERISO --fixed-speed-values --spinup-delay=0 -d $ISODIR/sdz -r" $TMPISO  $ISODIR/no.ecc
  rm -f $ISODIR/sdz
fi

# Read image from defective media without error correction data available
# Will have more missing sectors than the original due to the 16 sector skip default

if try "reading image, defective media, no ecc data" read_defective_no_ecc; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase 100-200 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase 766 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase 2410 >>$LOGFILE 2>&1
  
  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_defective_no_ecc "--spinup-delay=0 -r" $TMPISO  $ISODIR/no.ecc
fi

# Read image from above test again, this time with a 1 sector skip size
# Will provide an exact copy of the (damaged) original

if try "reading image, defective media, no ecc data, completing w/ 1 sec step" read_defective_no_ecc_again; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase 100-200 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase 766 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase 2410 >>$LOGFILE 2>&1

  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase 96-207 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --erase 752-767 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --erase 2400-2415 >>$LOGFILE 2>&1

  replace_config jump 0
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_defective_no_ecc_again "--spinup-delay=0 -j 1 -r" $TMPISO  $ISODIR/no.ecc
fi

# Read image from defective media without error correction data available
# using a large sector skip of 256

if try "reading image, defective media, large sector skip" read_defective_large_skip; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase 1600-1615 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase 6400-10000 >>$LOGFILE 2>&1
  
  rm -f $TMPISO
  replace_config jump 256
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_defective_large_skip "--spinup-delay=0 -r -j 256" $TMPISO  $ISODIR/no.ecc
fi

# Complete a truncated image

if try "completing truncated image with no ecc data available" read_truncated_no_ecc; then

  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --truncate=$((ISOSIZE-560)) >>$LOGFILE 2>&1

  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_truncated_no_ecc "--spinup-delay=0 -r" $TMPISO  $ISODIR/no.ecc
fi

# Complete a truncated image from simulated defective media

if try "completing truncated image, defective media, no ecc data" read_truncated_no_ecc_again; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase 20800-20875 >>$LOGFILE 2>&1

  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --truncate=$((ISOSIZE-560)) >>$LOGFILE 2>&1

  replace_config jump 0
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_truncated_no_ecc_again "--spinup-delay=0 -j 1 -r" $TMPISO  $ISODIR/no.ecc
fi

# Complete a truncated image from simulated defective media

if try "completing truncated image, defective media, multipass, no ecc data" read_multipass_no_ecc_again; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase 20800-20875 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase 3000-3045 >>$LOGFILE 2>&1

  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --truncate=$((ISOSIZE-560)) >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --erase 2980-3120 >>$LOGFILE 2>&1

  replace_config jump 0
  replace_config read-medium 3
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_multipass_no_ecc_again "--read-medium=3 --spinup-delay=0 -j 1 -r" $TMPISO  $ISODIR/no.ecc
fi

# Complete a partially read image, but continue with gap between the last
# read and the next sector.

if try "completing truncated image with reading gap, no ecc data" read_with_gap_no_ecc; then

  cp $MASTERISO $SIMISO
  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --truncate=10000 >>$LOGFILE 2>&1

  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_with_gap_no_ecc "--spinup-delay=0 -r15000-end" $TMPISO  $ISODIR/no.ecc
fi

# Read a new image, but only for a partial range.

if try "reading new image with given range, no ecc data" read_new_with_range_no_ecc; then

  cp $MASTERISO $SIMISO
  rm -f $TMPISO

  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_new_with_range_no_ecc "--spinup-delay=0 -r10000-15000" $TMPISO  $ISODIR/no.ecc
fi

# Read a new image, but only for an invalid range.
# not possible in GUI mode

if try "reading new image with invalid range, no ecc data" read_new_with_invalid_range_no_ecc; then

  cp $MASTERISO $SIMISO
  rm -f $TMPISO

  run_regtest read_new_with_invalid_range_no_ecc "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r10000-55000" $TMPISO  $ISODIR/no.ecc
fi

# Read a new image, containing two missing sectors
# but not at the end, so no tao tail case

if try "reading new image with two missing sectors, no ecc data" read_two_missing_secs_no_ecc; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase 8020 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase $((ISOSIZE-1)) >>$LOGFILE 2>&1

  rm -f $TMPISO

  replace_config jump 0
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_two_missing_secs_no_ecc "--spinup-delay=0 -r -j 1" $TMPISO  $ISODIR/no.ecc
fi

# Read image with error correction data available

if try "reading image, ecc data" read_with_ecc; then

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$MASTERISO --fixed-speed-values"
  run_regtest read_with_ecc "--spinup-delay=0 -r" $TMPISO  $MASTERECC
fi

# Read with ecc into existing and complete image file

if try "reading image, ecc data, good file" read_with_ecc_good_file; then
  cp $MASTERISO $SIMISO
  cp $MASTERISO $TMPISO

  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_with_ecc_good_file "--spinup-delay=0 -r" $TMPISO $MASTERECC
fi

# Read image with non existing error correction file given
# Please note that this fact will be silently ignored; e.g. the image
# will be read as if no ecc file was given at all.

if try "reading image, ecc file does not exist" read_with_non_existing_ecc; then

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$MASTERISO --fixed-speed-values"
  run_regtest read_with_non_existing_ecc "--spinup-delay=0 -r" $TMPISO  $ISODIR/no_ecc
fi

# Read image with non accessible error correction file given
# Please note that this fact will be silently ignored; e.g. the image
# will be read as if no ecc file was given at all.

if try "reading image, no permission to access ecc file" read_with_no_permission_for_ecc; then
  cp $MASTERECC $TMPECC
  chmod 000 $TMPECC

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$MASTERISO --fixed-speed-values"
  run_regtest read_with_no_permission_for_ecc "--spinup-delay=0 -r" $TMPISO  $TMPECC
  rm -f $TMPECC
fi

# Read image with error correction data available
# and CRC errors

if try "reading image, crc errors, ecc data" read_crc_errors_with_ecc; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --byteset 0,100,255 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 1,180,200 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 7910,23,98 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 20999,55,123 >>$LOGFILE 2>&1
    
  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_crc_errors_with_ecc "--spinup-delay=0 -r" $TMPISO  $MASTERECC
fi

# Read image with error correction data available
# which is a few sectors shorter than expected.

if try "reading image, less sectors than expected, ecc data" read_shorter_with_ecc; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --truncate=$((ISOSIZE-44)) >>$LOGFILE 2>&1
    
  rm -f $TMPISO
  replace_config ignore-iso-size 1
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_shorter_with_ecc "--ignore-iso-size --spinup-delay=0 -r" $TMPISO  $MASTERECC
fi

# Read image with error correction data available
# from a medium which is a few sectors longer than expected.

if try "reading image, more sectors than expected, ecc data" read_longer_with_ecc; then

  cp $MASTERISO $SIMISO
  for i in $(seq 22); do cat fixed-random-sequence >>$SIMISO; done
    
  rm -f $TMPISO
  replace_config ignore-iso-size 1
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_longer_with_ecc "--ignore-iso-size --spinup-delay=0 -r" $TMPISO  $MASTERECC
fi

# Read image with error correction data available
# simulating the multisession case with two additional defective sectors trailing the medium

if try "reading image, tao tail case, ecc data" read_tao_tail_with_ecc; then

  cp $MASTERISO $SIMISO
  cat fixed-random-sequence >>$SIMISO
  $NEWVER --debug -i$SIMISO --erase 21000-21001 >>$LOGFILE 2>&1
    
  rm -f $TMPISO
  replace_config ignore-iso-size 1
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_tao_tail_with_ecc "--ignore-iso-size --spinup-delay=0 -r" $TMPISO  $MASTERECC
fi

# Read image with error correction data available
# with two defective sectors at the end and the --dao option

if try "reading image, tao tail case and --dao, ecc data" read_no_tao_tail_with_ecc; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase 20998-20999 >>$LOGFILE 2>&1
    
  rm -f $TMPISO
  replace_config dao 1
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_no_tao_tail_with_ecc "--dao --spinup-delay=0 -r" $TMPISO  $MASTERECC
fi

# Read image with error correction data available
# and more than two defective sectors at the end

if try "reading image, more than 2 sectors missing at end, ecc data" read_more_missing_at_end_with_ecc; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase 20954-20999 >>$LOGFILE 2>&1
    
  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_more_missing_at_end_with_ecc "--spinup-delay=0 -r" $TMPISO  $MASTERECC
fi

# Re-read image with error correction data available
# and wrong fingerprint in existing image

if try "re-reading image, wrong fingerprint, ecc data" read_wrong_fp_with_ecc; then

  cp $MASTERISO $SIMISO

  dd if=$MASTERISO of=$TMPISO bs=2048 count=800 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 16,100,200 >>$LOGFILE 2>&1
    
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_wrong_fp_with_ecc "--spinup-delay=0 -r" $TMPISO  $MASTERECC
fi

# Read an augmented image for which an ecc file is also available.
# In that case, the ecc file gets precedence over the embedded ecc.
# To make sure the RS01 data is used we introduce a CRC error in the RS02
# ecc area - this can only be detected by the "outer" RS01 CRC.

if try "reading image with RS02 data and a RS01 ecc file" read_with_double_ecc; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -mRS02 -n$((ISOSIZE+5000)) -c >>$LOGFILE 2>&1
  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -e $TMPECC -c $REDUNDANCY >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 25910,100,200 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_with_double_ecc "--spinup-delay=0 -r" $TMPISO  $TMPECC
fi

# Read an image for which ecc information is available,
# but requiring a newer dvdisaster version.

if try "reading image ecc file requiring a newer dvdisaster version" read_with_incompatible_ecc; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -e $TMPECC -c $REDUNDANCY >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --byteset 0,88,220 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --byteset 0,89,65 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --byteset 0,90,15 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_with_incompatible_ecc "--spinup-delay=0 -r" $TMPISO  $TMPECC
fi

# Read an image with a simulated hardware failure and 
# --ignore-fatal-sense not set.

if try "reading image with simulated hardware failure" read_with_hardware_failure; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase "5000:hardware failure" >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_with_hardware_failure "--spinup-delay=0 -r" $TMPISO  $ISODIR/no.ecc
fi

# Read an image with a simulated hardware failure and 
# --ignore-fatal-sense being set.

if try "reading image, ignoring simulated hardware failure" read_with_ignored_hardware_failure; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase "5000:hardware failure" >>$LOGFILE 2>&1

  rm -f $TMPISO
  replace_config ignore-fatal-sense 1
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_with_ignored_hardware_failure "--spinup-delay=0 -r --ignore-fatal-sense" $TMPISO  $ISODIR/no.ecc
fi

# Read medium in several passes; some sectors become readable in the third pass.

if try "reading medium in 3 passes; 3rd pass recovers some" read_multipass_partial_success; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase 15800-16199 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase "15900-16099:readable in pass 3" >>$LOGFILE 2>&1

  rm -f $TMPISO
  replace_config read-medium 3
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_multipass_partial_success "--read-medium=3 --spinup-delay=0 -r" $TMPISO  $ISODIR/no.ecc
fi

# Do a second sucessful read attempt at an incomplete image;
# see whether correct results are reported when ecc data is present
# since CRC caching is a bit complicated in this case.

if try "re-reading medium with ecc, successfull" read_second_pass_with_ecc_success; then

  cp $MASTERISO $SIMISO
  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase 15800-16199 >>$LOGFILE 2>&1

  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_second_pass_with_ecc_success "--spinup-delay=0 -r" $TMPISO  $MASTERECC
fi

# Do a second read attempt at an incomplete image;
# see whether CRC errors are still discovered since CRC caching is a bit
# complicated in this case.

if try "re-reading medium with CRC error" read_second_pass_with_crc_error; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --byteset 15830,8,3 >>$LOGFILE 2>&1
  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase 15800-16199 >>$LOGFILE 2>&1

  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_second_pass_with_crc_error "--spinup-delay=0 -r" $TMPISO  $MASTERECC
fi

# Read medium containing several dead sector markers

if try "reading medium containing dead sector markers" read_medium_with_dsm; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase "4999:pass as dead sector marker" >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase "5005:pass as dead sector marker" >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase "5007:pass as dead sector marker" >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_medium_with_dsm "--spinup-delay=0 -r" $TMPISO  $ISODIR/no.ecc
fi

# Read medium containing several dead sector markers, verbose output
# not applicable in GUI mode

if try "reading medium containing dead sector markers, verbose output" read_medium_with_dsm_verbose; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase "4999:pass as dead sector marker" >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase "5005:pass as dead sector marker" >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase "5007:pass as dead sector marker" >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_medium_with_dsm_verbose "--spinup-delay=0 -r -v" $TMPISO  $ISODIR/no.ecc
fi

# Complete medium for image containing several uncorrectable dead sector markers
# (sector displacement)

if try "completing image with uncorrectable dead sector markers" read_medium_with_dsm_in_image; then

  cp $MASTERISO $SIMISO
  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase 3030 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 3030,353,49 >>$LOGFILE 2>&1 // displaced from sector 3130
  $NEWVER --debug -i$TMPISO --erase 4400 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 4400,353,53 >>$LOGFILE 2>&1 // displaced from sector 4500
  $NEWVER --debug -i$TMPISO --erase 4411 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 4411,353,53 >>$LOGFILE 2>&1 // displaced from sector 4511

  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_medium_with_dsm_in_image "--spinup-delay=0 -r" $TMPISO  $ISODIR/no.ecc
fi

# Complete medium for image containing several uncorrectable dead sector markers, verbose output

if try "completing image with uncorrectable dead sector markers, verbose output" read_medium_with_dsm_in_image_verbose; then

  cp $MASTERISO $SIMISO
  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase 3030 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 3030,353,49 >>$LOGFILE 2>&1 // displaced from sector 3130
  $NEWVER --debug -i$TMPISO --erase 4400 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 4400,353,53 >>$LOGFILE 2>&1 // displaced from sector 4500
  $NEWVER --debug -i$TMPISO --erase 4411 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 4411,353,53 >>$LOGFILE 2>&1 // displaced from sector 4511

  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_medium_with_dsm_in_image_verbose "--spinup-delay=0 -r -v" $TMPISO  $ISODIR/no.ecc
fi

# Complete medium for image containing several uncorrectable dead sector markers
# (non matching fingerprint)

if try "completing image with uncorrectable dead sector markers (2)" read_medium_with_dsm_in_image2; then

  cp $MASTERISO $SIMISO
  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase 3030 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 3030,416,55 >>$LOGFILE 2>&1 // wrong fingerprint
  $NEWVER --debug -i$TMPISO --byteset 3030,556,32 >>$LOGFILE 2>&1 // changed label
  $NEWVER --debug -i$TMPISO --byteset 3030,557,50 >>$LOGFILE 2>&1 // changed label
  $NEWVER --debug -i$TMPISO --erase 4400 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 4400,416,53 >>$LOGFILE 2>&1 // wrong fingerprint
  $NEWVER --debug -i$TMPISO --byteset 3030,556,32 >>$LOGFILE 2>&1 // changed label
  $NEWVER --debug -i$TMPISO --byteset 4400,557,50 >>$LOGFILE 2>&1 // changed label
  $NEWVER --debug -i$TMPISO --erase 4411 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 4411,416,53 >>$LOGFILE 2>&1 // wrong fingerprint
  $NEWVER --debug -i$TMPISO --byteset 3030,556,32 >>$LOGFILE 2>&1 // changed label
  $NEWVER --debug -i$TMPISO --byteset 4411,557,50 >>$LOGFILE 2>&1 // changed label

  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_medium_with_dsm_in_image2 "--spinup-delay=0 -r " $TMPISO  $ISODIR/no.ecc
fi

# Complete medium for image containing several uncorrectable dead sector markers, verbose
# (non matching fingerprint)

if try "completing image with uncorrectable dead sector markers (2), verbose output" read_medium_with_dsm_in_image2_verbose; then

  cp $MASTERISO $SIMISO
  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase 3030 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 3030,416,55 >>$LOGFILE 2>&1 // wrong fingerprint
  $NEWVER --debug -i$TMPISO --byteset 3030,556,32 >>$LOGFILE 2>&1 // changed label
  $NEWVER --debug -i$TMPISO --byteset 3030,557,50 >>$LOGFILE 2>&1 // changed label
  $NEWVER --debug -i$TMPISO --erase 4400 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 4400,416,53 >>$LOGFILE 2>&1 // wrong fingerprint
  $NEWVER --debug -i$TMPISO --byteset 3030,556,32 >>$LOGFILE 2>&1 // changed label
  $NEWVER --debug -i$TMPISO --byteset 4400,557,50 >>$LOGFILE 2>&1 // changed label
  $NEWVER --debug -i$TMPISO --erase 4411 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 4411,416,53 >>$LOGFILE 2>&1 // wrong fingerprint
  $NEWVER --debug -i$TMPISO --byteset 3030,556,32 >>$LOGFILE 2>&1 // changed label
  $NEWVER --debug -i$TMPISO --byteset 4411,557,50 >>$LOGFILE 2>&1 // changed label

  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_medium_with_dsm_in_image2_verbose "--spinup-delay=0 -r -v" $TMPISO  $ISODIR/no.ecc
fi

# Mechanismus um C2-Errors zu testen?

### Reading tests (adaptive)

echo "# Reading tests (adaptive)"

echo "Currently not enabled!"
exit 0

# Read good image with error correction data available

if try "reading good image" adaptive_good; then

  rm -f $TMPISO
  run_regtest adaptive_good "--debug --sim-cd=$MASTERISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO  $MASTERECC
fi

# Read image without error correction data available

if try "reading image, no ecc data" adaptive_no_ecc; then

  rm -f $TMPISO
  run_regtest adaptive_no_ecc "--debug --sim-cd=$MASTERISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO  $ISODIR/no.ecc
fi

# Read image from non-existant device

if try "reading image, device not existant" adaptive_no_device; then

  rm -f $TMPISO
  run_regtest adaptive_no_device "--debug --sim-cd=$MASTERISO --fixed-speed-values --spinup-delay=0 -d /dev/sdz -r --adaptive-read" $TMPISO  $ISODIR/no.ecc
fi

# Read image from device with insufficient permissions

if try "reading image, device access denied" adaptive_no_device_access; then

  touch $ISODIR/sdz
  chmod 000 $ISODIR/sdz
    
  rm -f $TMPISO
  run_regtest adaptive_no_device_access "--debug --sim-cd=$MASTERISO --fixed-speed-values --spinup-delay=0 -d $ISODIR/sdz -r --adaptive-read" $TMPISO  $ISODIR/no.ecc
  rm -f $ISODIR/sdz
fi

# Read image from defective media without error correction data available
# Will have more missing sectors than the original due to the divide and conquer 
# cut-off between intervals: e.g the following sectors will be missing:  
# 100- 202; 766-786; 2410-2428
# Please do also note that the termination criterion is somehow misleading
# as it applies to any intervals resulting from a split.
# If we are supposed to terminate when no intervals >= n=16 sectors are
# available, we are actually terminating when the first interval with
# less than 32 sectors is being split (so we are effectively terminating below  2n).
# The example run terminates as soon as the following intervals are left over:
# 20 [    101..    120]
# 20 [    142..    161]
# 20 [    183..    202]
# 20 [    767..    786]
# 19 [    122..    140]
# 19 [    163..    181]
# 18 [   2411..   2428]
#
# Having said that, the results from the following run have been manually
# checked to match what the programmer intended ;-)

if try "reading image, defective media, no ecc data" adaptive_defective_no_ecc; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase 100-200 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase 766 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase 2410 >>$LOGFILE 2>&1
  
  rm -f $TMPISO
  run_regtest adaptive_defective_no_ecc "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read -v" $TMPISO  $ISODIR/no.ecc
fi

# Read image from defective media without error correction data available
# using a large sector skip of 256

if try "reading image, defective media, large sector skip" adaptive_defective_large_skip; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase 1600-1615 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase 6400-10000 >>$LOGFILE 2>&1
  
  rm -f $TMPISO
  run_regtest adaptive_defective_large_skip "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r -j 256 --adaptive-read -v" $TMPISO  $ISODIR/no.ecc
fi

# Complete a truncated image

if try "completing truncated image with no ecc data available" adaptive_truncated_no_ecc; then

  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --truncate=$((ISOSIZE-560)) >>$LOGFILE 2>&1

  run_regtest adaptive_truncated_no_ecc "--debug --sim-cd=$MASTERISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO  $ISODIR/no.ecc
fi

# Complete a truncated image from simulated defective media
# Leaves 100 unread sectors.

if try "completing truncated image, defective media, no ecc data" adaptive_truncated_no_ecc_again; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase 20800-20875 >>$LOGFILE 2>&1

  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --truncate=$((ISOSIZE-560)) >>$LOGFILE 2>&1

  run_regtest adaptive_truncated_no_ecc_again "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r -v --adaptive-read" $TMPISO  $ISODIR/no.ecc
fi

# Complete a partially read image, but continue with gap between the last
# read and the next sector.
# (not a recommended setup for adaptive reading, but technically allowed)

if try "completing truncated image with reading gap, no ecc data" adaptive_with_gap_no_ecc; then

  cp $MASTERISO $SIMISO
  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --truncate=10000 >>$LOGFILE 2>&1

  run_regtest adaptive_with_gap_no_ecc "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r15000-end --adaptive-read" $TMPISO  $ISODIR/no.ecc
fi

# Complete a partially read image, but continue with gap between the last
# read and the next sector.
# (not a recommended setup for adaptive reading, but technically allowed)
# specified area ends before actual medium size

if try "completing truncated image with reading gap, no ecc data(2)" adaptive_with_gap_no_ecc2; then

  cp $MASTERISO $SIMISO
  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --truncate=10000 >>$LOGFILE 2>&1

  run_regtest adaptive_with_gap_no_ecc2 "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r15000-19999 --adaptive-read" $TMPISO  $ISODIR/no.ecc
fi

# Complete a partially read image, but continue with gap between the last
# read and the next sector.
# (not a recommended setup for adaptive reading, but technically allowed)
# specified area overlaps already read part

if try "completing truncated image with reading gap, no ecc data(3)" adaptive_with_gap_no_ecc3; then

  cp $MASTERISO $SIMISO
  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --truncate=10000 >>$LOGFILE 2>&1

  run_regtest adaptive_with_gap_no_ecc3 "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r9000-15000 --adaptive-read" $TMPISO  $ISODIR/no.ecc
fi

# Read a new image, but only for a partial range.

if try "reading new image with given range, no ecc data" adaptive_new_with_range_no_ecc; then

  cp $MASTERISO $SIMISO
  rm -f $TMPISO

  run_regtest adaptive_new_with_range_no_ecc "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r10000-15000 --adaptive-read" $TMPISO  $ISODIR/no.ecc
fi

# Read a new image, but only for an invalid range.

if try "reading new image with invalid range, no ecc data" adaptive_new_with_invalid_range_no_ecc; then

  cp $MASTERISO $SIMISO
  rm -f $TMPISO

  run_regtest adaptive_new_with_invalid_range_no_ecc "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r10000-55000 --adaptive-read" $TMPISO  $ISODIR/no.ecc
fi

# Read image with non accessible error correction file given
# Please note that this fact will be silently ignored; e.g. the image
# will be read as if no ecc file was given at all.

if try "reading image, no permission to access ecc file" adaptive_with_no_permission_for_ecc; then
  cp $MASTERECC $TMPECC
  chmod 000 $TMPECC

  rm -f $TMPISO
  run_regtest adaptive_with_no_permission_for_ecc "--debug --sim-cd=$MASTERISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO  $TMPECC
  rm -f $TMPECC
fi

# Read image with error correction data available
# and CRC errors
# Adaptive reading will create a new interval on CRC errors,
# but mark them as erasure as expected. The resulting image
# is successfully corrected with 32 erasures/block.

if try "reading image, crc errors, ecc data" adaptive_crc_errors_with_ecc; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --byteset 0,100,255 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 1,180,200 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 7910,23,98 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 20999,55,123 >>$LOGFILE 2>&1
    
  rm -f $TMPISO
  run_regtest adaptive_crc_errors_with_ecc "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO  $MASTERECC
fi

# Read image with error correction data available.
# The image is a few sectors shorter than expected.

if try "reading image, less sectors than expected, ecc data" adaptive_shorter_with_ecc; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --truncate=$((ISOSIZE-44)) >>$LOGFILE 2>&1
    
  rm -f $TMPISO
  run_regtest adaptive_shorter_with_ecc "--debug --ignore-iso-size --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO  $MASTERECC
fi

# Read image with error correction data available
# from a medium which is a few sectors longer than expected.

if try "reading image, more sectors than expected, ecc data" adaptive_longer_with_ecc; then

  cp $MASTERISO $SIMISO
  for i in $(seq 22); do cat fixed-random-sequence >>$SIMISO; done
    
  rm -f $TMPISO
  run_regtest adaptive_longer_with_ecc "--debug --ignore-iso-size --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO  $MASTERECC
fi

# Read image with error correction data available
# simulating the multisession case with two additional defective sectors trailing the medium
# Both this case and the next one do not really make sense for the adaptive
# reading case as the right behaviour is simply caused by using the respective
# values from the ecc data.

if try "reading image, tao tail case, ecc data" adaptive_tao_tail_with_ecc; then

  cp $MASTERISO $SIMISO
  cat fixed-random-sequence >>$SIMISO
  $NEWVER --debug -i$SIMISO --erase 21000-21001 >>$LOGFILE 2>&1
    
  rm -f $TMPISO
  run_regtest adaptive_tao_tail_with_ecc "--debug --ignore-iso-size --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO  $MASTERECC
fi

# Read image with error correction data available
# with two defective sectors at the end and the --dao option

if try "reading image, tao tail case and --dao, ecc data" adaptive_no_tao_tail_with_ecc; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase 20998-20999 >>$LOGFILE 2>&1
    
  rm -f $TMPISO
  run_regtest adaptive_no_tao_tail_with_ecc "--debug --dao --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO  $MASTERECC
fi

# Re-read image with error correction data available
# and wrong fingerprint in existing image

if try "re-reading image, wrong fingerprint, ecc data" adaptive_wrong_fp_with_ecc; then

  cp $MASTERISO $SIMISO

  dd if=$MASTERISO of=$TMPISO bs=2048 count=800 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 16,100,200 >>$LOGFILE 2>&1
    
  run_regtest adapive_wrong_fp_with_ecc "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO  $MASTERECC
fi

# Read an augmented image for which an ecc file is also available.
# In that case, the ecc file gets precedence over the embedded ecc.

if try "reading image with RS02 data and a RS01 ecc file" adaptive_with_double_ecc; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -mRS02 -n$((ISOSIZE+5000)) -c >>$LOGFILE 2>&1
  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -e $TMPECC -c $REDUNDANCY >>$LOGFILE 2>&1

  rm -f $TMPISO
  run_regtest adaptive_with_double_ecc "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO  $TMPECC
fi

# Read an image for which ecc information is available,
# but requiring a newer dvdisaster version.

if try "reading image w/ ecc file requiring a newer dvdisaster version" adaptive_with_incompatible_ecc; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -e $TMPECC -c $REDUNDANCY >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --byteset 0,88,220 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --byteset 0,89,65 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --byteset 0,90,15 >>$LOGFILE 2>&1

  rm -f $TMPISO
  run_regtest adaptive_with_incompatible_ecc "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO  $TMPECC
fi

# Read an image with a simulated hardware failure and 
# --ignore-fatal-sense not set.

if try "reading image with simulated hardware failure" adaptive_with_hardware_failure; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase "5000:hardware failure" >>$LOGFILE 2>&1

  rm -f $TMPISO
  run_regtest adaptive_with_hardware_failure "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO  $ISODIR/no.iso
fi

# Read an image with a simulated hardware failure and 
# --ignore-fatal-sense being set.

if try "reading image, ignoring simulated hardware failure" adaptive_with_ignored_hardware_failure; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase "5000:hardware failure" >>$LOGFILE 2>&1

  rm -f $TMPISO
  run_regtest adaptive_with_ignored_hardware_failure "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read --ignore-fatal-sense" $TMPISO  $ISODIR/no.iso
fi

# Read medium containing several dead sector markers

if try "reading medium containing dead sector markers" adaptive_medium_with_dsm; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase "4999:pass as dead sector marker" >>$LOGFILE 2>&1

  rm -f $TMPISO
  run_regtest adaptive_medium_with_dsm "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO  $ISODIR/no.ecc
fi
