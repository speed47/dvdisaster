#!/usr/bin/env bash

. common.bash

ISOSIZE=21000
ECCSIZE=25000
REAL_ECCSIZE=24990

MASTERISO=$ISODIR/rs03i-master.iso
TMPISO=$ISODIR/rs03i-tmp.iso
TMPECC=$ISODIR/rs03i-tmp.ecc  # rs03 augmented image wrapped by ecc file
SIMISO=$ISODIR/rs03i-sim.iso

LARGEMASTERISO=$ISODIR/rs03i-large.iso
LMI_HEADER=235219
LMI_LAYER_SIZE=1409
LMI_FIRSTCRC=235303

CODEC_PREFIX=RS03i

# Create master image

if ! file_exists $MASTERISO; then
    $NEWVER --debug -i$MASTERISO --random-image $ISOSIZE >>$LOGFILE 2>&1
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -mRS03 -n$ECCSIZE -c >>$LOGFILE 2>&1
    echo -e "$FILE_MSG"
    FILE_MSG=""
fi

# Create large master image

if ! file_exists $LARGEMASTERISO; then
    $NEWVER --debug -i$LARGEMASTERISO --random-image 235219 >>$LOGFILE 2>&1
    $NEWVER --debug --set-version $SETVERSION -i$LARGEMASTERISO -mRS03 -c >>$LOGFILE 2>&1
    echo -e "$FILE_MSG"
    FILE_MSG=""
fi

### Verification tests

echo "# Verify tests"

# Test good files

if try "good image" good; then
  run_regtest good "-t" $MASTERISO
fi

# Test good files, quick test

if try "good image, quick test" good_quick; then
  run_regtest good_quick "-tq" $MASTERISO
fi

# Test with non-existing image

if try "missing image" no_image; then
  run_regtest no_image "-t" $ISODIR/no.iso
fi

# Image is truncated by 5 sectors

if try "truncated image" truncated; then
  cp $MASTERISO $TMPISO
  NEWSIZE=$((REAL_ECCSIZE-5))
  $NEWVER -i$TMPISO --debug --truncate=$NEWSIZE >>$LOGFILE 2>&1

  run_regtest truncated "-t" $TMPISO
fi

# Image contains 1 extra sector

if try "image with one extra sector" plus1; then
   cp $MASTERISO $TMPISO
   dd if=/dev/zero count=1 bs=2048 >>$TMPISO 2>/dev/null

  run_regtest plus1 "-t" $TMPISO
fi

# Image contains 17 extra sectors

if try "image with 17 extra sectors" plus17; then
   cp $MASTERISO $TMPISO
   dd if=/dev/zero count=17 bs=2048 >>$TMPISO 2>/dev/null

  run_regtest plus17 "-t" $TMPISO
fi

# Image contains 56 extra bytes (which are not recorded in the ISO
# filesystem), then it is augmented and tested.

if try "image with 56 extra bytes" plus_56_bytes; then

  # recreate image to get rid of the ecc portion
  $NEWVER --debug -i$TMPISO --random-image $ISOSIZE >>$LOGFILE 2>&1
  for i in $(seq 56); do echo -n "1" >>$TMPISO; done
  $NEWVER --debug -i$TMPISO --set-version $SETVERSION -mRS03 -n$ECCSIZE -c >>$LOGFILE 2>&1

  IGNORE_LOG_LINE="^Avg performance|^Augmenting image with Method RS03"
  run_regtest plus_56_bytes "--debug -t -v -n$ECCSIZE" $TMPISO
fi

# Image contains one CRC block w/o cookie

if try "CRC block with invalid cookie" bad_crc_cookie; then
   cp $MASTERISO $TMPISO
   $NEWVER -i$TMPISO --debug --byteset 21100,1026,1 >>$LOGFILE 2>&1

   run_regtest bad_crc_cookie "-t" $TMPISO
fi

# Image contains two CRC block CRC errors

if try "CRC blocks with invalid checksum" bad_crc_checksum; then
   cp $MASTERISO $TMPISO
   $NEWVER -i$TMPISO --debug --byteset 21100,900,1 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --byteset 21107,555,1 >>$LOGFILE 2>&1

   run_regtest bad_crc_checksum "-t" $TMPISO
fi

# Image contains several missing CRC sectors

if try "several missing CRC sectors" missing_crc_sectors; then
   cp $MASTERISO $TMPISO
   $NEWVER -i$TMPISO --debug --erase 21100-21108 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --erase 21111 >>$LOGFILE 2>&1

   run_regtest missing_crc_sectors "-t" $TMPISO
fi

# Image contains several missing data sectors

if try "several missing data sectors" missing_data_sectors; then
   cp $MASTERISO $TMPISO
   $NEWVER -i$TMPISO --debug --erase 1500-1673 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --erase 13420-14109 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --erase 17812 >>$LOGFILE 2>&1

   run_regtest missing_data_sectors "-t" $TMPISO
fi

# Image contains several missing ecc sectors

if try "several missing ecc sectors" missing_ecc_sectors; then
   cp $MASTERISO $TMPISO
   $NEWVER -i$TMPISO --debug --erase 21168 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --erase 21900-21950 >>$LOGFILE 2>&1

   run_regtest missing_ecc_sectors "-t" $TMPISO
fi

# Image contains bad byte in data portion

if try "bad byte in data sector" data_bad_byte; then
   cp $MASTERISO $TMPISO
   $NEWVER -i$TMPISO --debug --byteset 4096,100,17 >>$LOGFILE 2>&1

   run_regtest data_bad_byte "-t" $TMPISO
fi

# Image contains bad byte in crc portion
# -> already covered in bad_crc_checksum case

# Image contains bad byte in ecc portion

if try "bad byte in ecc sector" ecc_bad_byte; then
   cp $MASTERISO $TMPISO
   $NEWVER -i$TMPISO --debug --byteset 21878,100,17 >>$LOGFILE 2>&1

   run_regtest ecc_bad_byte "-t" $TMPISO
fi

# Image size is exact multiple of layer size,
# resulting in a padding layer containing just the ecc sector behind the data area.

if try "image is multiple of layer size" layer_multiple; then
  $NEWVER --debug -i $TMPISO --random-image 14508 >>$LOGFILE 2>&1
  $NEWVER --debug --set-version $SETVERSION -mRS03 -n20000 -c -i $TMPISO >>$LOGFILE 2>&1
  
  run_regtest layer_multiple "-t" $TMPISO
  rm -f $TMPISO
fi

# Image size is exact multiple of layer size minus two sectors,
# resulting in no padding behind the data area.

if try "image crafted to have no padding" no_padding; then
  $NEWVER --debug -i $TMPISO --random-image 14506 >>$LOGFILE 2>&1
  $NEWVER --debug --set-version $SETVERSION -mRS03 -n20000 -c -i $TMPISO >>$LOGFILE 2>&1
  
  run_regtest no_padding "-t" $TMPISO
  rm -f $TMPISO
fi

# Augmented image is protected by an outer RS01 error correction file

if try "with RS01 error correction file" with_rs01_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -c -n normal >>$LOGFILE 2>&1

    run_regtest with_rs01_file "-v -t" $MASTERISO $TMPECC
fi

# Augmented image and non-matching RS01 error correction file
# Expected behaviour is to report the non-matching ecc file
# rather than falling back to using the RS03 part since the
# user did probably have some intention specifying the ecc file.

if try "with non-matching RS01 error correction file" with_wrong_rs01_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -c -n normal >>$LOGFILE 2>&1
    $NEWVER --debug -i$TMPECC --byteset 0,24,1 >>$LOGFILE 2>&1

    run_regtest with_wrong_rs01_file "-v -t" $MASTERISO $TMPECC
fi

# Augmented image is protected by an outer RS03 error correction file

if try "with RS03 error correction file" with_rs03_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -mRS03 -c -n 20 -o file >>$LOGFILE 2>&1

    run_regtest with_rs03_file "-v -t" $MASTERISO $TMPECC
fi

# Augmented image and non-matching RS03 error correction file
# TODO: Bad sub blocks come from creating placeholders with wrong checksum.
# Will be fixed when ecc header integrity checks are implemented.

if try "with non-matching RS03 error correction file" with_wrong_rs03_file; then
    # Create image with manipulated fingerprint sector
    cp $MASTERISO $TMPISO
    $NEWVER --debug -i$TMPISO --byteset 16,240,1 >>$LOGFILE 2>&1

    # Create ecc file for "wrong" image
    $NEWVER --debug --set-version $SETVERSION -i$TMPISO -e$TMPECC -mRS03 -c -n 20 -o file >>$LOGFILE 2>&1

    # Now test against original image
    run_regtest with_wrong_rs03_file "-v -t" $MASTERISO $TMPECC
fi

# Augmented image containing several uncorrectable dead sector markers
# within the CRC area. There is a specical code segment in RS03GetCrcBuf()
# for testing this. Trigger the outout for DSM found in the image.
# (test case in other places: DSM in medium, file)

if try "crc section with uncorrectable dead sector markers" crc_section_with_uncorrectable_dsm; then

  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase 21077 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 21077,353,50 >>$LOGFILE 2>&1 // displaced sector from 22077
  $NEWVER --debug -i$TMPISO --erase 21080 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 21080,353,53 >>$LOGFILE 2>&1 // displaced sector from 25080
  $NEWVER --debug -i$TMPISO --erase 21081 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 21081,353,53 >>$LOGFILE 2>&1 // displaced sector from 25081

  run_regtest crc_section_with_uncorrectable_dsm  "-t" $TMPISO
fi

# Augmented image containing several uncorrectable dead sector markers
# (sector displacement) in the data part

if try "image with uncorrectable dead sector markers" uncorrectable_dsm_in_image; then

  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase 3030 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 3030,353,49 >>$LOGFILE 2>&1 // displaced from sector 3130
  $NEWVER --debug -i$TMPISO --erase 4400 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 4400,353,53 >>$LOGFILE 2>&1 // displaced from sector 4500
  $NEWVER --debug -i$TMPISO --erase 4411 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 4411,353,53 >>$LOGFILE 2>&1 // displaced from sector 4511

  run_regtest uncorrectable_dsm_in_image  "-t" $TMPISO
fi

# Augmented image containing several uncorrectable dead sector markers
# (sector displacement) in the data part, with verbose option

if try "image with uncorrectable dead sector markers, verbose" uncorrectable_dsm_in_image_verbose; then

  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase 3030 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 3030,353,49 >>$LOGFILE 2>&1 // displaced from sector 3130
  $NEWVER --debug -i$TMPISO --erase 4400 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 4400,353,53 >>$LOGFILE 2>&1 // displaced from sector 4500
  $NEWVER --debug -i$TMPISO --erase 4411 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 4411,353,53 >>$LOGFILE 2>&1 // displaced from sector 4511

  run_regtest uncorrectable_dsm_in_image_verbose  "-t -v" $TMPISO
fi

# Augmented image containing several uncorrectable dead sector markers
# (sector displacement) in the ecc part

if try "ecc section with uncorrectable dead sector markers" ecc_section_with_uncorrectable_dsm; then

  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase 22030 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 22030,353,49 >>$LOGFILE 2>&1 // displaced from sector 21130
  $NEWVER --debug -i$TMPISO --erase 22400 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 22400,353,53 >>$LOGFILE 2>&1 // displaced from sector 25400
  $NEWVER --debug -i$TMPISO --erase 22411 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 22411,353,53 >>$LOGFILE 2>&1 // displaced from sector 25411

  run_regtest ecc_section_with_uncorrectable_dsm  "-t" $TMPISO
fi

# Augmented image containing several uncorrectable dead sector markers
# (non matching fingerprint) in the data part

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

  run_regtest uncorrectable_dsm_in_image2 "-t" $TMPISO
fi

# Augmented image containing several uncorrectable dead sector markers
# (non matching fingerprint) in the data part, verbose output

if try "image with uncorrectable dead sector markers (2), verbose" uncorrectable_dsm_in_image2_verbose; then

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

  run_regtest uncorrectable_dsm_in_image2_verbose "-t -v" $TMPISO
fi

# Normal sized image with missing iso header.

if try "image with missing iso header" missing_iso_header; then

  cp $LARGEMASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase 16 >>$LOGFILE 2>&1

  run_regtest missing_iso_header "-tq -v" $TMPISO
fi

# Normal sized image with missing ecc header.

if try "image with missing ecc header" missing_header; then

  cp $LARGEMASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase $LMI_HEADER >>$LOGFILE 2>&1

  run_regtest missing_header "-tq -v" $TMPISO
fi

# Normal sized image with missing ecc header; with exhaustive search.
# In the first slice, the CRC sector and some other sectors are unreadable.

if try "image with missing ecc header (2)" missing_header2; then

  cp $LARGEMASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase $LMI_HEADER >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --erase $LMI_FIRSTCRC >>$LOGFILE 2>&1
  for i in $(seq $((120*LMI_LAYER_SIZE)) $LMI_LAYER_SIZE $((135*LMI_LAYER_SIZE))); do
      $NEWVER --debug -i$TMPISO --erase $i >>$LOGFILE 2>&1
  done

  run_regtest missing_header2 "-tq -v" $TMPISO
fi

# Normal sized image with missing ecc header; with exhaustive search.
# In the first few slices, the CRC sector and some other sectors are unreadable.

if try "image with missing ecc header (3)" missing_header3; then

  cp $LARGEMASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase $LMI_HEADER >>$LOGFILE 2>&1
  # slice 0
  $NEWVER --debug -i$TMPISO --erase $LMI_FIRSTCRC >>$LOGFILE 2>&1
  for i in $(seq $((100*LMI_LAYER_SIZE)) $LMI_LAYER_SIZE $((235*LMI_LAYER_SIZE))); do
      $NEWVER --debug -i$TMPISO --erase $i >>$LOGFILE 2>&1
  done
  # slice 1
  $NEWVER --debug -i$TMPISO --byteset $((LMI_FIRSTCRC+1)),500,0 >>$LOGFILE 2>&1
  for i in $(seq $((110*LMI_LAYER_SIZE+1)) $LMI_LAYER_SIZE $((140*LMI_LAYER_SIZE+1))); do
      $NEWVER --debug -i$TMPISO --erase $i >>$LOGFILE 2>&1
  done
  # slice 2
  $NEWVER --debug -i$TMPISO --erase $((LMI_FIRSTCRC+2)) >>$LOGFILE 2>&1
  for i in $(seq $((110*LMI_LAYER_SIZE+2)) $LMI_LAYER_SIZE $((140*LMI_LAYER_SIZE+2))); do
      $NEWVER --debug -i$TMPISO --erase $i >>$LOGFILE 2>&1
  done
  # slice 3
  $NEWVER --debug -i$TMPISO --byteset $((LMI_FIRSTCRC+3)),500,0 >>$LOGFILE 2>&1
  for i in $(seq $((110*LMI_LAYER_SIZE+3)) $LMI_LAYER_SIZE $((139*LMI_LAYER_SIZE+3))); do
      $NEWVER --debug -i$TMPISO --erase $i >>$LOGFILE 2>&1
  done
  # slice 4
  $NEWVER --debug -i$TMPISO --erase $((LMI_FIRSTCRC+4)) >>$LOGFILE 2>&1

  run_regtest missing_header3 "-tq -v" $TMPISO
fi

# Normal sized image with missing ecc header; with exhaustive search.
# Every CRC sector except for the last one is unreadable.

if try "image with missing ecc header (4)" missing_header4; then

  cp $LARGEMASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase $LMI_HEADER >>$LOGFILE 2>&1
  # slice 0
  $NEWVER --debug -i$TMPISO --erase $LMI_FIRSTCRC >>$LOGFILE 2>&1
  for i in $(seq $((100*LMI_LAYER_SIZE)) $LMI_LAYER_SIZE $((235*LMI_LAYER_SIZE))); do
      $NEWVER --debug -i$TMPISO --erase $i >>$LOGFILE 2>&1
  done
  # slice 1
  $NEWVER --debug -i$TMPISO --byteset $((LMI_FIRSTCRC+1)),500,0 >>$LOGFILE 2>&1
  for i in $(seq $((110*LMI_LAYER_SIZE+1)) $LMI_LAYER_SIZE $((140*LMI_LAYER_SIZE+1))); do
      $NEWVER --debug -i$TMPISO --erase $i >>$LOGFILE 2>&1
  done
  # slice 2
  $NEWVER --debug -i$TMPISO --erase $((LMI_FIRSTCRC+2)) >>$LOGFILE 2>&1
  for i in $(seq $((110*LMI_LAYER_SIZE+2)) $LMI_LAYER_SIZE $((140*LMI_LAYER_SIZE+2))); do
      $NEWVER --debug -i$TMPISO --erase $i >>$LOGFILE 2>&1
  done
  # slice 3
  $NEWVER --debug -i$TMPISO --byteset $((LMI_FIRSTCRC+3)),500,0 >>$LOGFILE 2>&1
  for i in $(seq $((110*LMI_LAYER_SIZE+3)) $LMI_LAYER_SIZE $((139*LMI_LAYER_SIZE+3))); do
      $NEWVER --debug -i$TMPISO --erase $i >>$LOGFILE 2>&1
  done
  # slices 4-1407
  $NEWVER --debug -i$TMPISO --erase $((LMI_FIRSTCRC+4))-$((LMI_FIRSTCRC+1407)) >>$LOGFILE 2>&1

  run_regtest missing_header4 "-tq -v" $TMPISO
fi

# Normal sized but truncated image with missing ecc header; with exhaustive search.
# In the first slice, the CRC sector and some other sectors are unreadable.

if try "image with missing ecc header, truncated" missing_header_truncated; then

  cp $LARGEMASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase $LMI_HEADER >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --erase $LMI_FIRSTCRC >>$LOGFILE 2>&1
  for i in $(seq $((120*LMI_LAYER_SIZE)) $LMI_LAYER_SIZE $((135*LMI_LAYER_SIZE))); do
      $NEWVER --debug -i$TMPISO --erase $i >>$LOGFILE 2>&1
  done
  $NEWVER --debug -i$TMPISO --truncate=300000 >>$LOGFILE 2>&1

  run_regtest missing_header_truncated "-tq -v" $TMPISO
fi

# Normal sized image with missing ecc header; with exhaustive search.
# All CRC sectors are missing
# TODO: must become correctable after implementation of ecc-based header recovery.

if try "image with missing ecc header and no crc sectors" missing_header_no_crcsec; then

  cp $LARGEMASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase $LMI_HEADER >>$LOGFILE 2>&1

  # delete CRC layer completely
  $NEWVER --debug -i$TMPISO --erase $((LMI_FIRSTCRC))-$((LMI_FIRSTCRC+1408)) >>$LOGFILE 2>&1

  run_regtest missing_header_no_crcsec "-tq -v" $TMPISO
fi

# Completely random image (no ecc)

if try "image with no ecc at all" random_image; then

  $NEWVER --debug -i$TMPISO --random-image 359295 >>$LOGFILE 2>&1

  run_regtest random_image "-tq -v" $TMPISO
fi

# Image with 8 roots (smallest possible case)

if try "image with 8 roots, no ecc header" rediscover_8_roots; then

  $NEWVER --debug -i$TMPISO --random-image $((LMI_LAYER_SIZE*246-2)) >>$LOGFILE 2>&1
  $NEWVER --debug --set-version $SETVERSION -i$TMPISO -mRS03 -c -x 2 >>$LOGFILE 2>&1
  # delete the header
  $NEWVER --debug -i$TMPISO --erase 346612 >>$LOGFILE 2>&1

  run_regtest rediscover_8_roots "-tq -v" $TMPISO
fi

# Image with 8 roots (smallest possible case)

if try "image with 8 roots, no ecc header (2)" rediscover_8_roots2; then

  $NEWVER --debug -i$TMPISO --random-image $((LMI_LAYER_SIZE*246-2)) >>$LOGFILE 2>&1
  $NEWVER --debug --set-version $SETVERSION -i$TMPISO -mRS03 -c -x 2 >>$LOGFILE 2>&1
  # delete the header and some more CRC sectors
  $NEWVER --debug -i$TMPISO --erase 346612-346620 >>$LOGFILE 2>&1

  run_regtest rediscover_8_roots2 "-tq -v" $TMPISO
fi

# Image with 170 roots (biggest possible case with no padding)

if try "image with 170 roots, no ecc header" rediscover_170_roots; then

  $NEWVER --debug -i$TMPISO --random-image $((LMI_LAYER_SIZE*84-2)) >>$LOGFILE 2>&1
  $NEWVER --debug --set-version $SETVERSION -i$TMPISO -mRS03 -c -x 2 >>$LOGFILE 2>&1
  # delete the header
  $NEWVER --debug -i$TMPISO --erase 118354 >>$LOGFILE 2>&1

  run_regtest rediscover_170_roots "-tq -v" $TMPISO
fi

# Image with 170 roots (biggest possible case with no padding)

if try "image with 170 roots, no ecc header (2)" rediscover_170_roots2; then

  $NEWVER --debug -i$TMPISO --random-image $((LMI_LAYER_SIZE*84-2)) >>$LOGFILE 2>&1
  $NEWVER --debug --set-version $SETVERSION -i$TMPISO -mRS03 -c -x 2 >>$LOGFILE 2>&1
  # delete the header and some more CRC sectors
  $NEWVER --debug -i$TMPISO --erase 118354-118360 >>$LOGFILE 2>&1

  run_regtest rediscover_170_roots2 "-tq -v" $TMPISO
fi

# Image with 170 roots (biggest possible case with padding)
# Ecc Header present

if try "image with 170 roots, padding" rediscover_170_roots_padding; then

  $NEWVER --debug -i$TMPISO --random-image $((LMI_LAYER_SIZE*84-2-6000)) >>$LOGFILE 2>&1
  $NEWVER --debug --set-version $SETVERSION -i$TMPISO -mRS03 -c -x 2 >>$LOGFILE 2>&1

  run_regtest rediscover_170_roots-padding "-tq -v" $TMPISO
fi

# Image with 170 roots (biggest possible case with padding)

if try "image with 170 roots, no ecc header, padding" rediscover_170_roots_padding2; then

  $NEWVER --debug -i$TMPISO --random-image $((LMI_LAYER_SIZE*84-2-6000)) >>$LOGFILE 2>&1
  $NEWVER --debug --set-version $SETVERSION -i$TMPISO -mRS03 -c -x 2 >>$LOGFILE 2>&1
  # delete the header and some more CRC sectors
  $NEWVER --debug -i$TMPISO --erase 112354 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --erase 118356-118360 >>$LOGFILE 2>&1

  run_regtest rediscover_170_roots-padding2 "-tq -v" $TMPISO
fi

# Image contains Ecc header with the ecc file flag set

if try "image with ecc header from a file" with_ecc_file_header; then
   cp $MASTERISO $TMPISO

   # Ecc file bit
   $NEWVER -i$TMPISO --debug --byteset 21000,16,2 >>$LOGFILE 2>&1

   # self CRC sum
   $NEWVER -i$TMPISO --debug --byteset 21000,96,142 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --byteset 21000,97,43 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --byteset 21000,98,137 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --byteset 21000,99,29 >>$LOGFILE 2>&1
      
   run_regtest with_ecc_file_header "--debug -t -v -n$ECCSIZE" $TMPISO
fi

# Image contains defective Ecc header
# and a crc block from an ecc file

if try "image with crc block from a file" with_ecc_file_crc_block; then
   cp $MASTERISO $TMPISO

   # Delete Ecc Header

   $NEWVER -i$TMPISO --debug --erase 21000 >>$LOGFILE 2>&1

   # Ecc file bit
   $NEWVER -i$TMPISO --debug --byteset 21070,1040,2 >>$LOGFILE 2>&1

   # self CRC sum
   $NEWVER -i$TMPISO --debug --byteset 21070,1120,208 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --byteset 21070,1121,250 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --byteset 21070,1122,142 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --byteset 21070,1123,101 >>$LOGFILE 2>&1
      
   run_regtest with_ecc_file_crc_block "--debug -t -v -n$ECCSIZE" $TMPISO
fi

### Creation tests

echo "# Creation tests"

# Create ecc file

if try "augmented image creation" ecc_create; then
   $NEWVER --debug -i$TMPISO --random-image $ISOSIZE >>$LOGFILE 2>&1

   IGNORE_LOG_LINE="^Avg performance|^Augmenting image with Method RS03"
   replace_config method-name RS03
   replace_config ecc-target 1
   extra_args="--debug --set-version $SETVERSION"
   run_regtest ecc_create "-mRS03 -n$ECCSIZE -c" $TMPISO
fi

# Create with missing image

if try "creating augmented image with missing image" ecc_missing_image; then

   replace_config method-name RS03
   replace_config ecc-target 1
   extra_args="--debug --set-version $SETVERSION"
   run_regtest ecc_missing_image "-mRS03 -n$ECCSIZE -c" $ISODIR/no.iso
fi

# Create with no read permission on image

if try "creating augmented image with no read permission" ecc_no_read_perm; then
   $NEWVER --debug -i$TMPISO --random-image $ISOSIZE >>$LOGFILE 2>&1
   chmod 000 $TMPISO

   replace_config method-name RS03
   replace_config ecc-target 1
   extra_args="--debug --set-version $SETVERSION"
   run_regtest ecc_no_read_perm "-mRS03 -n$ECCSIZE -c" $TMPISO
   rm -f $TMPISO
fi

# Create with no write permission on image

if try "creating augmented image with no write permission" ecc_no_write_perm; then
   $NEWVER --debug -i$TMPISO --random-image $ISOSIZE >>$LOGFILE 2>&1
   chmod 400 $TMPISO

   replace_config method-name RS03
   replace_config ecc-target 1
   extra_args="--debug --set-version $SETVERSION"
   run_regtest ecc_no_write_perm "-mRS03 -n$ECCSIZE -c" $TMPISO
   rm -f $TMPISO
fi

# Create with already RS03-augmented image 

if try "ecc creating from RS03-augmented image" ecc_from_rs03; then
   cp $MASTERISO $TMPISO

   IGNORE_LOG_LINE="^Avg performance|^Augmenting image with Method RS03"
   replace_config method-name RS03
   replace_config ecc-target 1
   extra_args="--debug --set-version $SETVERSION"
   run_regtest ecc_from_rs03 "-mRS03 -n$ECCSIZE -c" $TMPISO
   rm -f $TMPISO
fi

# Create with already RS02-augmented image 

if try "ecc creating from RS02-augmented image" ecc_from_rs02; then
   $NEWVER --debug -i$TMPISO --random-image $ISOSIZE >>$LOGFILE 2>&1
   $NEWVER --debug --set-version $SETVERSION -i$TMPISO -mRS02 -n$((ECCSIZE+5000)) -c >>$LOGFILE 2>&1

   IGNORE_LOG_LINE="^Avg performance|^Augmenting image with Method RS03"
   replace_config method-name RS03
   replace_config ecc-target 1
   extra_args="--debug --set-version $SETVERSION"
   run_regtest ecc_from_rs02 "-mRS03 -n$ECCSIZE -c" $TMPISO
   rm -f $TMPISO
fi

# Create with already RS03-augmented image having a larger redundancy 

if try "ecc creating from RS03-augmented image w/ higher red." ecc_from_larger_rs03; then
   $NEWVER --debug -i$TMPISO --random-image $ISOSIZE >>$LOGFILE 2>&1
   $NEWVER --debug --set-version $SETVERSION -i$TMPISO -mRS03 -n$((ECCSIZE+5000)) -c >>$LOGFILE 2>&1

   IGNORE_LOG_LINE="^Avg performance|^Augmenting image with Method RS03"
   replace_config method-name RS03
   replace_config ecc-target 1
   extra_args="--debug --set-version $SETVERSION"
   run_regtest ecc_from_larger_rs03 "-mRS03 -n$ECCSIZE -c" $TMPISO
   rm -f $TMPISO
fi

# Create with already RS02-augmented image of a non-2048 multiple size

if try "ecc creating from RS02-augmented image w/ non block size." ecc_from_rs02_non_blocksize; then
   $NEWVER --debug -i$TMPISO --random-image $ISOSIZE >>$LOGFILE 2>&1
   for i in $(seq 56); do echo -n "1" >>$TMPISO; done
   $NEWVER --debug --set-version $SETVERSION -i$TMPISO -mRS02 -n$ECCSIZE -c >>$LOGFILE 2>&1

   IGNORE_LOG_LINE="^Avg performance|^Augmenting image with Method RS03"
   replace_config method-name RS03
   replace_config ecc-target 1
   extra_args="--debug --set-version $SETVERSION"
   run_regtest ecc_from_rs02_non_blocksize "-mRS03 -n$ECCSIZE -c -a RS03" $TMPISO
   rm -f $TMPISO
fi

# Create with already RS03-augmented image of a non-2048 multiple size

if try "ecc creating from RS03-augmented image w/ non block size." ecc_from_rs03_non_blocksize; then
   $NEWVER --debug -i$TMPISO --random-image $ISOSIZE >>$LOGFILE 2>&1
   for i in $(seq 56); do echo -n "1" >>$TMPISO; done
   $NEWVER --debug --set-version $SETVERSION -i$TMPISO -mRS03 -n$ECCSIZE -c >>$LOGFILE 2>&1

   IGNORE_LOG_LINE="^Avg performance|^Augmenting image with Method RS03"
   replace_config method-name RS03
   replace_config ecc-target 1
   extra_args="--debug --set-version $SETVERSION"
   run_regtest ecc_from_rs03_non_blocksize "-mRS03 -n$ECCSIZE -c -a RS03" $TMPISO
   rm -f $TMPISO
fi

# Create with already RS03-augmented image of a non-2048 multiple size, larger redundancy.

if try "ecc creating from RS03-augmented image w/ non block size, larger red." ecc_from_larger_rs03_non_blocksize; then
   $NEWVER --debug -i$TMPISO --random-image $((ISOSIZE+1)) >>$LOGFILE 2>&1
   $NEWVER --debug -i$TMPISO --truncate=$ISOSIZE >>$LOGFILE 2>&1
   for i in $(seq 56); do echo -n "1" >>$TMPISO; done
   $NEWVER --debug --set-version $SETVERSION -i$TMPISO -mRS03 -n$((ECCSIZE+5000)) -c >>$LOGFILE 2>&1

   IGNORE_LOG_LINE="^Avg performance|^Augmenting image with Method RS03"
   replace_config method-name RS03
   replace_config ecc-target 1
   extra_args="--debug --set-version $SETVERSION"
   run_regtest ecc_from_larger_rs03_non_blocksize "-mRS03 -n$ECCSIZE -c" $TMPISO
   rm -f $TMPISO
fi

# Create ecc file for image with 56 additional bytes

if try "image with 56 extra bytes" ecc_non_blocksize; then
  # recreate image to get rid of the ecc portion
  $NEWVER --debug -i$TMPISO --random-image $ISOSIZE >>$LOGFILE 2>&1
  for i in $(seq 56); do echo -n "1" >>$TMPISO; done

  IGNORE_LOG_LINE="^Avg performance|^Augmenting image with Method RS03"
  replace_config method-name RS03
  replace_config ecc-target 1
  extra_args="--debug --set-version $SETVERSION"
  run_regtest ecc_non_blocksize "-mRS03 -n$ECCSIZE -c" $TMPISO
  rm -f $TMPISO
fi

# Try to create ecc file from image with missing sectors

if try "creating ecc from image with missing sectors" ecc_missing_sectors; then
  $NEWVER --debug -i $TMPISO --random-image $ISOSIZE >>$LOGFILE 2>&1
  $NEWVER --debug -i $TMPISO --erase 500-524 >>$LOGFILE 2>&1
  
  replace_config method-name RS03
  replace_config ecc-target 1
  extra_args="--debug --set-version $SETVERSION"
  run_regtest ecc_missing_sectors "-mRS03 -n$ECCSIZE -c" $TMPISO
  rm -f $TMPISO
fi

# Create ecc file where image size is exact multiple of layer size,
# resulting in a padding layer containing just the ecc sector behind the data area.

if try "creating ecc, image is multiple of layer size" ecc_layer_multiple; then
  $NEWVER --debug -i $TMPISO --random-image 14508 >>$LOGFILE 2>&1
  
  IGNORE_LOG_LINE="^Avg performance|^Augmenting image with Method RS03"
  replace_config method-name RS03
  replace_config ecc-target 1
  replace_config medium-size 20000
  extra_args="--debug --set-version $SETVERSION"
  run_regtest ecc_layer_multiple "-mRS03 -n20000 -c" $TMPISO
  rm -f $TMPISO
fi

# Create ecc file where image size is exact multiple of layer size minus two sectors,
# resulting in no padding behind the data area.

if try "creating ecc crafted to have no padding" ecc_no_padding; then
  $NEWVER --debug -i $TMPISO --random-image 14506 >>$LOGFILE 2>&1
  
  IGNORE_LOG_LINE="^Avg performance|^Augmenting image with Method RS03"
  replace_config method-name RS03
  replace_config ecc-target 1
  replace_config medium-size 20000
  extra_args="--debug --set-version $SETVERSION"
  run_regtest ecc_no_padding "-mRS03 -n20000 -c" $TMPISO
  rm -f $TMPISO
fi

# Read image and create ecc in the same program call.
# Tests whether CRC and ECC information is handed over correctly.
# NOTE: cache handling is currently disabled and will be fixed in 0.79.6!

if try "read image and create ecc in one call" ecc_create_after_read; then
  $NEWVER --debug -i$SIMISO --random-image $ISOSIZE >>$LOGFILE 2>&1

  rm -f $TMPISO $TMPECC
  IGNORE_LOG_LINE="^Avg performance|^Augmenting image with Method RS03"
  replace_config method-name RS03
  replace_config ecc-target 1
  replace_config medium-size 20000
  replace_config read-and-create 1
  extra_args="--debug --set-version $SETVERSION --sim-cd=$SIMISO  --fixed-speed-values"
  run_regtest ecc_create_after_read "-r -mRS03 -c -n$ECCSIZE --spinup-delay=0" $TMPISO $TMPECC
fi

# Complete image in a reading pass, then create an ecc file for it.
# Cached checksums must be discarded before creating the ecc.
# NOTE: cache handling is currently disabled and will be fixed in 0.79.6!

if try "create ecc after completing partial image" ecc_create_after_partial_read; then
  $NEWVER --debug -i$SIMISO --random-image $ISOSIZE >>$LOGFILE 2>&1
  cp $SIMISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase 1000-1500 >>$LOGFILE 2>&1

  rm -f $TMPECC
  IGNORE_LOG_LINE="^Avg performance|^Augmenting image with Method RS03"
  extra_args="--debug --set-version $SETVERSION --sim-cd=$SIMISO  --fixed-speed-values"
  run_regtest ecc_create_after_partial_read "-r -mRS03 -c -n$ECCSIZE --spinup-delay=0" $TMPISO $TMPECC
fi

# Read image with ecc file and create new (other) ecc in the same program call.
# Tests whether CRC and ECC information is handed over correctly.
# NOTE: cache handling is currently disabled and will be fixed in 0.79.6!

if try "read image with ecc (RS01) and create new ecc" ecc_recreate_after_read_rs01; then
  $NEWVER --debug -i$SIMISO --random-image $ISOSIZE >>$LOGFILE 2>&1
  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -e $TMPECC -mRS01 -c -n 10 >>$LOGFILE 2>&1

  rm -f $TMPISO
  IGNORE_LOG_LINE="^Avg performance|^Augmenting image with Method RS03"
  extra_args="--debug --set-version $SETVERSION --sim-cd=$SIMISO  --fixed-speed-values"
  run_regtest ecc_recreate_after_read_rs01 "-r -mRS03 -c -n$ECCSIZE --spinup-delay=0" $TMPISO $TMPECC
fi

# Read image with ecc file and create new (other) ecc in the same program call.
# Tests whether CRC and ECC information is handed over correctly.
# NOTE: cache handling is currently disabled and will be fixed in 0.79.6!

if try "read image with ecc (RS02) and create new ecc" ecc_recreate_after_read_rs02; then
  $NEWVER --debug -i$SIMISO --random-image $ISOSIZE >>$LOGFILE 2>&1
  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -mRS02 -c -n24000 >>$LOGFILE 2>&1

  rm -f $TMPISO $TMPECC
  IGNORE_LOG_LINE="^Avg performance|^Augmenting image with Method RS03"
  extra_args="--debug --set-version $SETVERSION --sim-cd=$SIMISO  --fixed-speed-values"
  run_regtest ecc_recreate_after_read_rs02 "-r -mRS03 -c -n$ECCSIZE --spinup-delay=0" $TMPISO $TMPECC
fi

# Read image with ecc file and create new (other) ecc in the same program call.
# Tests whether CRC and ECC information is handed over correctly.
# NOTE: cache handling is currently disabled and will be fixed in 0.79.6!

if try "read image with ecc (RS03i) and create new ecc" ecc_recreate_after_read_rs03i; then
  $NEWVER --debug -i$SIMISO --random-image $ISOSIZE >>$LOGFILE 2>&1
  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -mRS03 -c -n23000 >>$LOGFILE 2>&1

  rm -f $TMPISO $TMPECC
  IGNORE_LOG_LINE="^Avg performance|^Augmenting image with Method RS03"
  extra_args="--debug --set-version $SETVERSION --sim-cd=$SIMISO  --fixed-speed-values"
  run_regtest ecc_recreate_after_read_rs03i "-r -mRS03 -c -n$ECCSIZE --spinup-delay=0" $TMPISO $TMPECC
fi

# Read image with ecc file and create new (other) ecc in the same program call.
# Tests whether CRC and ECC information is handed over correctly.
# NOTE: cache handling is currently disabled and will be fixed in 0.79.6!

if try "read image with ecc (RS03f) and create new ecc" ecc_recreate_after_read_rs03f; then
  $NEWVER --debug -i$SIMISO --random-image $ISOSIZE >>$LOGFILE 2>&1
  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -e $TMPECC -mRS03 -c -n 10 -o file >>$LOGFILE 2>&1

  rm -f $TMPISO
  IGNORE_LOG_LINE="^Avg performance|^Augmenting image with Method RS03"
  extra_args="--debug --set-version $SETVERSION --sim-cd=$SIMISO  --fixed-speed-values"
  run_regtest ecc_recreate_after_read_rs03f "-r -mRS03 -c -n$ECCSIZE --spinup-delay=0" $TMPISO $TMPECC
fi

### Fixing tests

echo "# Fixing tests"

# Fix with no read permission on image

if try "trying fix with no read permission" fix_no_read_perm; then
  $NEWVER --debug -i$TMPISO --random-image $ISOSIZE >>$LOGFILE 2>&1
  chmod 000 $TMPISO

  run_regtest fix_no_read_perm "-f" $TMPISO
  rm -f $TMPISO
fi

# Fix with no write permission on image

if try "trying fix with no write permission" fix_no_write_perm; then
  $NEWVER --debug -i$TMPISO --random-image $ISOSIZE >>$LOGFILE 2>&1
  chmod 400 $TMPISO

  run_regtest fix_no_write_perm "-f" $TMPISO
  rm -f $TMPISO
fi

# Fix already good image

if try "trying fix with good image" fix_good_image; then
  cp $MASTERISO $TMPISO

  run_regtest fix_good_image "-f" $TMPISO
  rm -f $TMPISO
fi

# Fix a truncated image

if try "trying fix with truncated image" fix_truncated_image; then
   TRUNC_SIZE=$((REAL_ECCSIZE-210));
   cp $MASTERISO $TMPISO
   $NEWVER --debug -i$TMPISO --truncate=$TRUNC_SIZE >>$LOGFILE 2>&1

   run_regtest fix_truncated_image "-f" $TMPISO
   rm -f $TMPISO
fi

# Fix an image with a few trailing bytes

if try "trying fix with trailing bytes" fix_trailing_bytes; then
   cp $MASTERISO $TMPISO
   echo "some trailing garbage appended for testing" >>$TMPISO

   run_regtest fix_trailing_bytes "-f" $TMPISO
   rm -f $TMPISO
fi

# Fix an image with trailing garbage (TAO case)

if try "trying fix with trailing garbage (TAO case)" fix_trailing_tao; then
   cp $MASTERISO $TMPISO
   dd if=/dev/zero count=2 bs=2048 >>$TMPISO 2>>$LOGFILE

   run_regtest fix_trailing_tao "-f" $TMPISO
   rm -f $TMPISO
fi

# Fix an image with trailing garbage (general case), without doing anything

if try "trying fix with trailing garbage (general case)" fix_trailing_garbage; then
   cp $MASTERISO $TMPISO
   dd if=/dev/zero count=23 bs=2048 >>$TMPISO 2>>$LOGFILE

   run_regtest fix_trailing_garbage "-f" $TMPISO
fi

# Fix an image with trailing garbage (general case), with --truncate

if try "trying fix with trailing garbage with --truncate" fix_trailing_garbage2; then
   cp $MASTERISO $TMPISO
   dd if=/dev/zero count=23 bs=2048 >>$TMPISO 2>>$LOGFILE

   run_regtest fix_trailing_garbage2 "-f --truncate" $TMPISO
fi

# Fix image with missing sectors (real damage and everything else being okay)

if try "trying to fix correctable image" fix_correctable; then
  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase 500-524 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --erase 1000 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 2000,0,111 >>$LOGFILE 2>&1

  run_regtest fix_correctable "-f" $TMPISO
  rm -f $TMPISO
fi

# Fix image with missing sectors in several border locations

if try "trying to fix image with missing sectors in border cases" fix_border_cases_erasures; then
  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase 0 >>$LOGFILE 2>&1       # first sector
  $NEWVER --debug -i$TMPISO --erase 98 >>$LOGFILE 2>&1      # first sector, second layer
  $NEWVER --debug -i$TMPISO --erase 196 >>$LOGFILE 2>&1     # first sector, third layer
  $NEWVER --debug -i$TMPISO --erase 20972 >>$LOGFILE 2>&1   # first sector, last data layer
  $NEWVER --debug -i$TMPISO --erase 21070 >>$LOGFILE 2>&1   # first sector, crc layer
  $NEWVER --debug -i$TMPISO --erase 21168 >>$LOGFILE 2>&1   # first sector, first ecc layer
  $NEWVER --debug -i$TMPISO --erase 21266 >>$LOGFILE 2>&1   # first sector, second ecc layer
  $NEWVER --debug -i$TMPISO --erase 24892 >>$LOGFILE 2>&1   # first sector, last ecc layer

  $NEWVER --debug -i$TMPISO --erase 97 >>$LOGFILE 2>&1      # last sector, first layer
  $NEWVER --debug -i$TMPISO --erase 195 >>$LOGFILE 2>&1     # last sector, second layer
  $NEWVER --debug -i$TMPISO --erase 293 >>$LOGFILE 2>&1     # last sector, third layer
  $NEWVER --debug -i$TMPISO --erase 20999 >>$LOGFILE 2>&1   # last sector, last data layer
  $NEWVER --debug -i$TMPISO --erase 21167 >>$LOGFILE 2>&1   # last sector, crc layer
  $NEWVER --debug -i$TMPISO --erase 21265 >>$LOGFILE 2>&1   # last sector, first ecc layer
  $NEWVER --debug -i$TMPISO --erase 21363 >>$LOGFILE 2>&1   # last sector, second ecc layer
  $NEWVER --debug -i$TMPISO --erase 24989 >>$LOGFILE 2>&1   # last sector, last ecc layer

  run_regtest fix_border_cases_erasures "-f" $TMPISO
  rm -f $TMPISO
fi

# Fix image with CRC errors in several border locations

if try "trying to fix image with crc errors in border cases" fix_border_cases_crc_errors; then
  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --byteset 0,0,1 >>$LOGFILE 2>&1       # first sector
  $NEWVER --debug -i$TMPISO --byteset 98,0,0 >>$LOGFILE 2>&1      # first sector, second layer
  $NEWVER --debug -i$TMPISO --byteset 196,0,0 >>$LOGFILE 2>&1     # first sector, third layer
  $NEWVER --debug -i$TMPISO --byteset 20972,0,0 >>$LOGFILE 2>&1   # first sector, last data layer
  $NEWVER --debug -i$TMPISO --byteset 21070,0,0 >>$LOGFILE 2>&1   # first sector, crc layer
  $NEWVER --debug -i$TMPISO --byteset 21168,0,0 >>$LOGFILE 2>&1   # first sector, first ecc layer
  $NEWVER --debug -i$TMPISO --byteset 21266,0,0 >>$LOGFILE 2>&1   # first sector, second ecc layer
  $NEWVER --debug -i$TMPISO --byteset 24892,0,0 >>$LOGFILE 2>&1   # first sector, last ecc layer

  $NEWVER --debug -i$TMPISO --byteset 97,0,0 >>$LOGFILE 2>&1      # last sector, first layer
  $NEWVER --debug -i$TMPISO --byteset 195,0,0 >>$LOGFILE 2>&1     # last sector, second layer
  $NEWVER --debug -i$TMPISO --byteset 293,0,0 >>$LOGFILE 2>&1     # last sector, third layer
  $NEWVER --debug -i$TMPISO --byteset 20999,0,0 >>$LOGFILE 2>&1   # last sector, last data layer
  $NEWVER --debug -i$TMPISO --byteset 21167,0,0 >>$LOGFILE 2>&1   # last sector, crc layer
  $NEWVER --debug -i$TMPISO --byteset 21265,0,0 >>$LOGFILE 2>&1   # last sector, first ecc layer
  $NEWVER --debug -i$TMPISO --byteset 21363,0,0 >>$LOGFILE 2>&1   # last sector, second ecc layer
  $NEWVER --debug -i$TMPISO --byteset 24989,0,0 >>$LOGFILE 2>&1   # last sector, last ecc layer

  run_regtest fix_border_cases_crc_errors "-f" $TMPISO
  rm -f $TMPISO
fi

# Fix ecc file where image size is exact multiple of layer size,
# resulting in a padding layer containing just the ecc sector behind the data area.

if try "fixing ecc, image is multiple of layer size" fix_layer_multiple; then
  $NEWVER --debug -i $TMPISO --random-image 14508 >>$LOGFILE 2>&1
  $NEWVER --debug --set-version $SETVERSION -mRS03 -n20000 -c -i $TMPISO >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --erase 500-524 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --erase 14510-14520 >>$LOGFILE 2>&1
  
  run_regtest fix_layer_multiple "-f" $TMPISO
  rm -f $TMPISO
fi

# Fix ecc file where image size is exact multiple of layer size minus two sectors,
# resulting in no padding behind the data area.

if try "fixing ecc crafted to have no padding" fix_no_padding; then
  $NEWVER --debug -i $TMPISO --random-image 14506 >>$LOGFILE 2>&1
  $NEWVER --debug --set-version $SETVERSION -mRS03 -n20000 -c -i $TMPISO >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --erase 500-524 >>$LOGFILE 2>&1
  
  run_regtest fix_no_padding "-f" $TMPISO
  rm -f $TMPISO
fi

# Augmented image is protected by an outer RS01 error correction file
# Setting the byte creates a CRC error which can only be detected by
# the outer RS01 code, so we use this as an additional probe that the
# correct (outer) ECC is applied.

if try "fixing RS03 with RS01 error correction file" fix_with_rs01_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -c -n normal >>$LOGFILE 2>&1

    cp $MASTERISO $TMPISO
    $NEWVER --debug -i$TMPISO --byteset 24989,0,1 >>$LOGFILE 2>&1
    
    run_regtest fix_with_rs01_file "-f" $TMPISO $TMPECC
fi

# Augmented image is protected by an outer RS03 error correction file

if try "fixing RS03 with RS03 error correction file" fix_with_rs03_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -mRS03 -c -n 20 -o file >>$LOGFILE 2>&1

    cp $MASTERISO $TMPISO
    $NEWVER --debug -i$TMPISO --byteset 24989,0,1 >>$LOGFILE 2>&1
    
    run_regtest fix_with_rs03_file "-f" $TMPISO $TMPECC
fi

# Fix image with missing header after iso portion

if try "trying fix with missing header" fix_with_missing_header; then
   cp $MASTERISO $TMPISO
   $NEWVER --debug -i$TMPISO --erase $ISOSIZE >>$LOGFILE 2>&1

   extra_args="--debug -n $ECCSIZE"
   run_regtest fix_with_missing_header "-f -v" $TMPISO
   rm -f $TMPISO
fi

# Fix image with iso missing header

if try "trying fix with missing iso header" fix_with_missing_iso_header; then
   cp $MASTERISO $TMPISO
   $NEWVER --debug -i$TMPISO --erase 16 >>$LOGFILE 2>&1

   extra_args="--debug -n $ECCSIZE"
   run_regtest fix_with_missing_iso_header "-f -v" $TMPISO
   rm -f $TMPISO
fi

# Image contains Ecc header with the ecc file flag set

if try "image with ecc header from a file" fix_with_ecc_file_header; then
   cp $MASTERISO $TMPISO

   # Ecc file bit
   $NEWVER -i$TMPISO --debug --byteset 21000,16,2 >>$LOGFILE 2>&1

   # self CRC sum
   $NEWVER -i$TMPISO --debug --byteset 21000,96,142 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --byteset 21000,97,43 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --byteset 21000,98,137 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --byteset 21000,99,29 >>$LOGFILE 2>&1
      
   extra_args="--debug -n $ECCSIZE"
   run_regtest fix_with_ecc_file_header "-f -v" $TMPISO
fi

# Image contains defective Ecc header
# and a crc block from an ecc file

if try "image with crc block from a file" fix_with_ecc_file_crc_block; then
   cp $MASTERISO $TMPISO

   # Delete Ecc Header

   $NEWVER -i$TMPISO --debug --erase 21000 >>$LOGFILE 2>&1

   # Ecc file bit
   $NEWVER -i$TMPISO --debug --byteset 21070,1040,2 >>$LOGFILE 2>&1

   # self CRC sum
   $NEWVER -i$TMPISO --debug --byteset 21070,1120,208 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --byteset 21070,1121,250 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --byteset 21070,1122,142 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --byteset 21070,1123,101 >>$LOGFILE 2>&1
      
   extra_args="--debug -n $ECCSIZE"
   run_regtest fix_with_ecc_file_crc_block "-f -v" $TMPISO
fi

### Scanning tests

echo "# Scanning tests"

# Scan complete / optimal image

if try "scanning good image" scan_good; then
  cp $MASTERISO $SIMISO

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_good "--spinup-delay=0 -s" $TMPISO
fi

# Scan complete / optimal image, verbose output

if try "scanning good image, verbose output" scan_good_verbose; then
  cp $MASTERISO $SIMISO

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_good_verbose "--spinup-delay=0 -s -v" $TMPISO
fi

# Scan image which is shorter than expected
# Currently we are trying to read past the medium
# and getting media errors. Is that smart? Rethink later.

if try "scanning image being shorter than expected" scan_shorter; then
  cp $MASTERISO $SIMISO

  $NEWVER --debug -i$SIMISO --truncate=$((REAL_ECCSIZE-44)) >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_shorter "--spinup-delay=0 -s" $TMPISO
fi

# Scan image which is longer than expected
# Will return image in its original length.

if try "scanning image being longer than expected" scan_longer; then
  cp $MASTERISO $SIMISO

  for i in $(seq 23); do cat fixed-random-sequence >>$SIMISO; done

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_longer "--spinup-delay=0 -s -v" $TMPISO
fi

# Scan image with two multisession link sectors appended.
# Will return image in its original length.

if try "scanning image, tao tail case" scan_tao_tail; then
  cp $MASTERISO $SIMISO

  cat fixed-random-sequence >>$SIMISO
  $NEWVER --debug -i$SIMISO --erase 24990-24991 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_tao_tail "--spinup-delay=0 -s" $TMPISO
fi

# Scan image with two real sectors missing at the end.
# -dao option prevents them from being clipped off.

if try "scanning image, no tao tail case" scan_no_tao_tail; then
  cp $MASTERISO $SIMISO

  $NEWVER --debug -i$SIMISO --erase 24988-24989 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_no_tao_tail "--spinup-delay=0 -s --dao" $TMPISO
fi

# Scan an image for which ecc information is available,
# but requiring a newer dvdisaster version.
# NOTE: Only the master header is manipulated, which is
# sufficient for reaching the goal of this test case.

if try "scanning image requiring a newer dvdisaster version" scan_incompatible_ecc; then

  cp $MASTERISO $SIMISO
  # Creator version 99.99
  $NEWVER --debug -i$SIMISO --byteset 21000,84,220 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 21000,85,65 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 21000,86,15 >>$LOGFILE 2>&1
  # Version info 99.99
  $NEWVER --debug -i$SIMISO --byteset 21000,88,220 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 21000,89,65 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 21000,90,15 >>$LOGFILE 2>&1
  # Patched selfcrc
  $NEWVER --debug -i$SIMISO --byteset 21000,96,208 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 21000,97,125 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 21000,98,164 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 21000,99,44 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_incompatible_ecc "--spinup-delay=0 -s " $TMPISO
fi

# Scan an image containing a defective ECC header.
# Will be treated like an ECC-less image since --assume is not set.

if try "scanning image with a defective header" scan_bad_header; then

  cp $MASTERISO $SIMISO
  $NEWVER -i$SIMISO --debug --byteset 21000,1,1 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_bad_header "--spinup-delay=0 -s" $TMPISO
fi

# Image contains 2 rows of missing sectors and a single one
# in the data portion

if try "scanning image with missing data sectors" scan_missing_data_sectors; then
   cp $MASTERISO $SIMISO
   $NEWVER -i$SIMISO --debug --erase 1000-1049 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --erase 21230 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --erase 22450-22457 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest scan_missing_data_sectors "--spinup-delay=0 -s " $TMPISO
fi

# Image contains 1 row of missing sectors and a single one
# in the crc portion

if try "scanning image with missing crc sectors" scan_missing_crc_sectors; then
   cp $MASTERISO $SIMISO
   $NEWVER -i$SIMISO --debug --erase 21077 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --erase 21100-21120 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest scan_missing_crc_sectors "--spinup-delay=0 -s " $TMPISO
fi

# Image contains 1 row of missing sectors and a single one
# in the ecc portion

if try "scanning image with missing ecc sectors" scan_missing_ecc_sectors; then
   cp $MASTERISO $SIMISO
   $NEWVER -i$SIMISO --debug --erase 21200 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --erase 21340-21365 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest scan_missing_ecc_sectors "--spinup-delay=0 -s " $TMPISO
fi

# Image contains bad byte in the data section

if try "scanning image with bad data byte" scan_data_bad_byte; then
   cp $MASTERISO $SIMISO 
   $NEWVER -i$SIMISO --debug --byteset 1235,50,10 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest scan_data_bad_byte "--spinup-delay=0 -s " $TMPISO
fi

# Image contains bad byte in the crc section

if try "scanning image with bad crc byte" scan_crc_bad_byte; then
   cp $MASTERISO $SIMISO 
   $NEWVER -i$SIMISO --debug --byteset 21077,50,10 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest scan_crc_bad_byte "--spinup-delay=0 -s " $TMPISO
fi

# Image contains bad byte in the ecc section

if try "scanning image with bad ecc byte" scan_ecc_bad_byte; then
   cp $MASTERISO $SIMISO 
   $NEWVER -i$SIMISO --debug --byteset 22000,50,10 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest scan_ecc_bad_byte "--spinup-delay=0 -s " $TMPISO
fi

# Augmented image is protected by an outer RS01 error correction file
# Setting the byte creates a CRC error which can only be detected by
# the outer RS01 code, so we use this as an additional probe that the
# correct (outer) ECC is applied.

if try "scanning with RS01 error correction file" scan_with_rs01_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -c -n normal >>$LOGFILE 2>&1

    cp $MASTERISO $SIMISO
    $NEWVER --debug -i$SIMISO --byteset 24989,0,1 >>$LOGFILE 2>&1
    
    rm -f $TMPISO
    extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
    run_regtest scan_with_rs01_file "--spinup-delay=0 -s " $TMPISO $TMPECC
fi

# Augmented image and non-matching RS01 error correction file
# Currently the mismatch is (generally) not detected
# NOTE: There seems to be an intermittent race condition between
# printing the defective sector number and the reading progress.
# Ignore for now and debug later.
# Should we change this behaviour?
# Expected behaviour for verify is to report the non-matching ecc file
# rather than falling back to using the RS02 part since the
# user did probably have some intentention specifying the ecc file.

if try "scanning with non-matching RS01 error correction file" scan_with_wrong_rs01_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -c -n normal >>$LOGFILE 2>&1
    $NEWVER --debug -i$TMPECC --byteset 0,24,1 >>$LOGFILE 2>&1

    cp $MASTERISO $SIMISO
    $NEWVER --debug -i$SIMISO --byteset 24989,0,1 >>$LOGFILE 2>&1
    
    rm -f $TMPISO
    extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
    run_regtest scan_with_wrong_rs01_file "--spinup-delay=0 -s " $TMPISO $TMPECC
fi

# Augmented image is protected by an outer RS03 error correction file

if try "scanning with RS03 error correction file" scan_with_rs03_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -mRS03 -c -n 20 -o file >>$LOGFILE 2>&1

    cp $MASTERISO $SIMISO
    $NEWVER --debug -i$SIMISO --byteset 24989,0,1 >>$LOGFILE 2>&1
    
    rm -f $TMPISO
    extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
    run_regtest scan_with_rs03_file "--spinup-delay=0 -s " $TMPISO $TMPECC
fi

# Augmented image and non-matching RS03 error correction file

if try "scanning with non-matching RS03 error correction file" scan_with_wrong_rs03_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -mRS03 -c -n 20 -o file >>$LOGFILE 2>&1
    $NEWVER --debug -i$TMPECC --byteset 0,24,1 >>$LOGFILE 2>&1

    cp $MASTERISO $SIMISO
    $NEWVER --debug -i$SIMISO --byteset 24989,0,1 >>$LOGFILE 2>&1
    
    rm -f $TMPISO
    extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
    run_regtest scan_with_wrong_rs03_file "--spinup-delay=0 -s " $TMPISO $TMPECC
fi

# Normal sized image with missing ecc header;
# no exhaustive search activated.

if try "scanning with missing ecc header, no exhaustive search" scan_missing_header_not_exhaustive; then

  cp $LARGEMASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase $LMI_HEADER >>$LOGFILE 2>&1

  rm -f $TMPISO $TMPECC
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_missing_header_not_exhaustive "--spinup-delay=0 -s"  $TMPISO $TMPECC
fi

# Normal sized image with missing ecc header; with exhaustive search.

if try "scanning with missing ecc header" scan_missing_header; then

  cp $LARGEMASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase $LMI_HEADER >>$LOGFILE 2>&1

  replace_config examine-rs03 1
  replace_config medium-size 0
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_missing_header "--spinup-delay=0 -a RS03 -s -v"  $TMPISO $TMPECC
fi

# Normal sized image with missing ecc header; with exhaustive search.
# In the first slice, the CRC sector and some other sectors are unreadable.

if try "scanning with missing ecc header (2)" scan_missing_header2; then

  cp $LARGEMASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase $LMI_HEADER >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase $LMI_FIRSTCRC >>$LOGFILE 2>&1
  for i in $(seq $((120*LMI_LAYER_SIZE)) $LMI_LAYER_SIZE $((135*LMI_LAYER_SIZE))); do
      $NEWVER --debug -i$SIMISO --erase $i >>$LOGFILE 2>&1
  done

  replace_config examine-rs03 1
  replace_config medium-size 0
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_missing_header2 "--spinup-delay=0 -a RS03 -s -v"  $TMPISO $TMPECC
fi

# Normal sized image with missing ecc header; with exhaustive search.
# In the first few slices, the CRC sector and some other sectors are unreadable.

if try "scanning with missing ecc header (3)" scan_missing_header3; then

  cp $LARGEMASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase $LMI_HEADER >>$LOGFILE 2>&1
  # slice 0
  $NEWVER --debug -i$SIMISO --erase $LMI_FIRSTCRC >>$LOGFILE 2>&1
  for i in $(seq $((100*LMI_LAYER_SIZE)) $LMI_LAYER_SIZE $((235*LMI_LAYER_SIZE))); do
      $NEWVER --debug -i$SIMISO --erase $i >>$LOGFILE 2>&1
  done
  # slice 1
  $NEWVER --debug -i$SIMISO --byteset $((LMI_FIRSTCRC+1)),500,0 >>$LOGFILE 2>&1
  for i in $(seq $((110*LMI_LAYER_SIZE+1)) $LMI_LAYER_SIZE $((140*LMI_LAYER_SIZE+1))); do
      $NEWVER --debug -i$SIMISO --erase $i >>$LOGFILE 2>&1
  done
  # slice 2
  $NEWVER --debug -i$SIMISO --erase $((LMI_FIRSTCRC+2)) >>$LOGFILE 2>&1
  for i in $(seq $((110*LMI_LAYER_SIZE+2)) $LMI_LAYER_SIZE $((140*LMI_LAYER_SIZE+2))); do
      $NEWVER --debug -i$SIMISO --erase $i >>$LOGFILE 2>&1
  done
  # slice 3
  $NEWVER --debug -i$SIMISO --byteset $((LMI_FIRSTCRC+3)),500,0 >>$LOGFILE 2>&1
  for i in $(seq $((110*LMI_LAYER_SIZE+3)) $LMI_LAYER_SIZE $((139*LMI_LAYER_SIZE+3))); do
      $NEWVER --debug -i$SIMISO --erase $i >>$LOGFILE 2>&1
  done
  # slice 4
  $NEWVER --debug -i$SIMISO --erase $((LMI_FIRSTCRC+4)) >>$LOGFILE 2>&1

  replace_config examine-rs03 1
  replace_config medium-size 0
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_missing_header3 "--spinup-delay=0 -a RS03 -s -v"  $TMPISO $TMPECC
fi

# Normal sized image with missing ecc header; with exhaustive search.
# Every CRC sector except for the last one is unreadable.

if try "scanning with missing ecc header (4)" scan_missing_header4; then

  cp $LARGEMASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase $LMI_HEADER >>$LOGFILE 2>&1
  # slice 0
  $NEWVER --debug -i$SIMISO --erase $LMI_FIRSTCRC >>$LOGFILE 2>&1
  for i in $(seq $((100*LMI_LAYER_SIZE)) $LMI_LAYER_SIZE $((235*LMI_LAYER_SIZE))); do
      $NEWVER --debug -i$SIMISO --erase $i >>$LOGFILE 2>&1
  done
  # slice 1
  $NEWVER --debug -i$SIMISO --byteset $((LMI_FIRSTCRC+1)),500,0 >>$LOGFILE 2>&1
  for i in $(seq $((110*LMI_LAYER_SIZE+1)) $LMI_LAYER_SIZE $((140*LMI_LAYER_SIZE+1))); do
      $NEWVER --debug -i$SIMISO --erase $i >>$LOGFILE 2>&1
  done
  # slice 2
  $NEWVER --debug -i$SIMISO --erase $((LMI_FIRSTCRC+2)) >>$LOGFILE 2>&1
  for i in $(seq $((110*LMI_LAYER_SIZE+2)) $LMI_LAYER_SIZE $((140*LMI_LAYER_SIZE+2))); do
      $NEWVER --debug -i$SIMISO --erase $i >>$LOGFILE 2>&1
  done
  # slice 3
  $NEWVER --debug -i$SIMISO --byteset $((LMI_FIRSTCRC+3)),500,0 >>$LOGFILE 2>&1
  for i in $(seq $((110*LMI_LAYER_SIZE+3)) $LMI_LAYER_SIZE $((139*LMI_LAYER_SIZE+3))); do
      $NEWVER --debug -i$SIMISO --erase $i >>$LOGFILE 2>&1
  done
  # slices 4-1407
  $NEWVER --debug -i$SIMISO --erase $((LMI_FIRSTCRC+4))-$((LMI_FIRSTCRC+1407)) >>$LOGFILE 2>&1

  replace_config examine-rs03 1
  replace_config medium-size 0
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_missing_header4 "--spinup-delay=0 -a RS03 -s -v"  $TMPISO $TMPECC
fi

# Normal sized but truncated image with missing ecc header; with exhaustive search.
# In the first slice, the CRC sector and some other sectors are unreadable.

if try "scanning with missing ecc header, truncated" scan_missing_header_truncated; then

  cp $LARGEMASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase $LMI_HEADER >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase $LMI_FIRSTCRC >>$LOGFILE 2>&1
  for i in $(seq $((120*LMI_LAYER_SIZE)) $LMI_LAYER_SIZE $((135*LMI_LAYER_SIZE))); do
      $NEWVER --debug -i$SIMISO --erase $i >>$LOGFILE 2>&1
  done
  $NEWVER --debug -i$SIMISO --truncate=300000 >>$LOGFILE 2>&1

  replace_config examine-rs03 1
  replace_config medium-size 0
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_missing_header_truncated "--spinup-delay=0 -a RS03 -s -v"  $TMPISO $TMPECC
fi

# Normal sized image with missing ecc header; with exhaustive search.
# All CRC sectors are missing
# TODO: must become correctable after implementation of ecc-based header recovery.

if try "scanning with missing ecc header and no crc sectors" scan_missing_header_no_crcsec; then

  cp $LARGEMASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase $LMI_HEADER >>$LOGFILE 2>&1

  # delete CRC layer completely
  $NEWVER --debug -i$SIMISO --erase $((LMI_FIRSTCRC))-$((LMI_FIRSTCRC+1408)) >>$LOGFILE 2>&1

  replace_config examine-rs03 1
  replace_config medium-size 0
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_missing_header_no_crcsec "--spinup-delay=0 -a RS03 -s -v"  $TMPISO $TMPECC
fi

# Completely random image (no ecc)

if try "scanning with no ecc at all" scan_random_image; then

  $NEWVER --debug -i$SIMISO --random-image 359295 >>$LOGFILE 2>&1

  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_random_image "--spinup-delay=0 -a RS03 -s -v"  $TMPISO $TMPECC
fi

# Image with 8 roots (smallest possible case)

if try "scanning with 8 roots, no ecc header" scan_rediscover_8_roots; then

  $NEWVER --debug -i$SIMISO --random-image $((LMI_LAYER_SIZE*246-2)) >>$LOGFILE 2>&1
  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -mRS03 -c -x 2 >>$LOGFILE 2>&1
  # delete the header
  $NEWVER --debug -i$SIMISO --erase 346612 >>$LOGFILE 2>&1

  replace_config examine-rs03 1
  replace_config medium-size 0
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_rediscover_8_roots "--spinup-delay=0 -a RS03 -s -v"  $TMPISO $TMPECC
fi

# Image with 8 roots (smallest possible case)

if try "scanning with 8 roots, no ecc header (2)" scan_rediscover_8_roots2; then

  $NEWVER --debug -i$SIMISO --random-image $((LMI_LAYER_SIZE*246-2)) >>$LOGFILE 2>&1
  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -mRS03 -c -x 2 >>$LOGFILE 2>&1
  # delete the header and some more CRC sectors
  $NEWVER --debug -i$SIMISO --erase 346612-346620 >>$LOGFILE 2>&1

  replace_config examine-rs03 1
  replace_config medium-size 0
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_rediscover_8_roots2 "--spinup-delay=0 -a RS03 -s -v"  $TMPISO $TMPECC
fi

# Image with 170 roots (biggest possible case with no padding)

if try "scanning with 170 roots, no ecc header" scan_rediscover_170_roots; then

  $NEWVER --debug -i$SIMISO --random-image $((LMI_LAYER_SIZE*84-2)) >>$LOGFILE 2>&1
  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -mRS03 -c -x 2 >>$LOGFILE 2>&1
  # delete the header
  $NEWVER --debug -i$SIMISO --erase 118354 >>$LOGFILE 2>&1

  replace_config examine-rs03 1
  replace_config medium-size 0
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_rediscover_170_roots "--spinup-delay=0 -a RS03 -s -v"  $TMPISO $TMPECC
fi

# Image with 170 roots (biggest possible case with no padding)

if try "scanning with 170 roots, no ecc header (2)" scan_rediscover_170_roots2; then

  $NEWVER --debug -i$SIMISO --random-image $((LMI_LAYER_SIZE*84-2)) >>$LOGFILE 2>&1
  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -mRS03 -c -x 2 >>$LOGFILE 2>&1
  # delete the header and some more CRC sectors
  $NEWVER --debug -i$SIMISO --erase 118354-118360 >>$LOGFILE 2>&1

  replace_config examine-rs03 1
  replace_config medium-size 0
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_rediscover_170_roots2 "--spinup-delay=0 -a RS03 -s -v"  $TMPISO $TMPECC
fi

# Image with 170 roots (biggest possible case with padding)
# Ecc Header present

if try "scanning with 170 roots, padding" scan_rediscover_170_roots_padding; then

  $NEWVER --debug -i$SIMISO --random-image $((LMI_LAYER_SIZE*84-2-6000)) >>$LOGFILE 2>&1
  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -mRS03 -c -x 2 >>$LOGFILE 2>&1

  replace_config examine-rs03 1
  replace_config medium-size 0
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_rediscover_170_roots-padding "--spinup-delay=0 -a RS03 -s -v"  $TMPISO $TMPECC
fi

# Image with 170 roots (biggest possible case with padding)

if try "scanning with 170 roots, no ecc header, padding" scan_rediscover_170_roots_padding2; then

  $NEWVER --debug -i$SIMISO --random-image $((LMI_LAYER_SIZE*84-2-6000)) >>$LOGFILE 2>&1
  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -mRS03 -c -x 2 >>$LOGFILE 2>&1
  # delete the header and some more CRC sectors
  $NEWVER --debug -i$SIMISO --erase 112354 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase 118356-118360 >>$LOGFILE 2>&1

  replace_config examine-rs03 1
  replace_config medium-size 0
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_rediscover_170_roots-padding2 "--spinup-delay=0 -a RS03 -s -v"  $TMPISO $TMPECC
fi

### Reading tests (linear)

echo "# Reading tests (linear)"

# Read complete / optimal image

if try "reading good image" read_good; then
  cp $MASTERISO $SIMISO

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_good "--spinup-delay=0 -r" $TMPISO
fi

# Read complete / optimal image, verbose output

if try "reading good image, verbose output" read_good_verbose; then
  cp $MASTERISO $SIMISO

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_good_verbose "--spinup-delay=0 -r -v" $TMPISO
fi

# Read into existing and complete image file

if try "reading good image in good file" read_good_file; then
  cp $MASTERISO $SIMISO

  cp $MASTERISO $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_good_file "--spinup-delay=0 -r" $TMPISO
fi

# Read image which is shorter than expected
# Currently we are trying to read past the medium
# and getting media errors. Is that smart? Rethink later.

if try "reading image being shorter than expected" read_shorter; then
  cp $MASTERISO $SIMISO

  $NEWVER --debug -i$SIMISO --truncate=$((REAL_ECCSIZE-44)) >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_shorter "--spinup-delay=0 -r" $TMPISO
fi

# Read image which is longer than expected
# Will return image in its original length.

if try "reading image being longer than expected" read_longer; then
  cp $MASTERISO $SIMISO

  for i in $(seq 23); do cat fixed-random-sequence >>$SIMISO; done

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_longer "--spinup-delay=0 -r -v" $TMPISO
fi

# Read image with two multisession link sectors appended.
# Will return image in its original length.

if try "reading image, tao tail case" read_tao_tail; then
  cp $MASTERISO $SIMISO

  cat fixed-random-sequence >>$SIMISO
  $NEWVER --debug -i$SIMISO --erase 24990-24991 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_tao_tail "--spinup-delay=0 -r" $TMPISO
fi

# Read image with two real sectors missing at the end.
# -dao option prevents them from being clipped off.

if try "reading image, no tao tail case" read_no_tao_tail; then
  cp $MASTERISO $SIMISO

  cat fixed-random-sequence >>$SIMISO
  $NEWVER --debug -i$SIMISO --erase 24988-24989 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_no_tao_tail "--spinup-delay=0 -r --dao" $TMPISO
fi

# Read an image for which ecc information is available,
# but requiring a newer dvdisaster version.
# NOTE: Only the master header is manipulated, which is
# sufficient for reaching the goal of this test case.

if try "reading image requiring a newer dvdisaster version" read_incompatible_ecc; then

  cp $MASTERISO $SIMISO
  # Creator version 99.99
  $NEWVER --debug -i$SIMISO --byteset 21000,84,220 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 21000,85,65 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 21000,86,15 >>$LOGFILE 2>&1
  # Version info 99.99
  $NEWVER --debug -i$SIMISO --byteset 21000,88,220 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 21000,89,65 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 21000,90,15 >>$LOGFILE 2>&1
  # Patched selfcrc
  $NEWVER --debug -i$SIMISO --byteset 21000,96,208 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 21000,97,125 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 21000,98,164 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 21000,99,44 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_incompatible_ecc "--spinup-delay=0 -r " $TMPISO
fi

# Read an image containing a defective ECC header.
# Will be treated like an ECC-less image since --assume is not set.

if try "reading image with a defective header" read_bad_header; then

  cp $MASTERISO $SIMISO
  $NEWVER -i$SIMISO --debug --byteset 21000,1,1 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_bad_header "--spinup-delay=0 -r" $TMPISO
fi

# Read an image containing a defective ECC header.
# Exhaustive search enabled.

if try "reading image with a defective header, exhaustive" read_bad_header_exhaustive; then

  cp $MASTERISO $SIMISO
  $NEWVER -i$SIMISO --debug --byteset 21000,1,1 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_bad_header_exhaustive "--debug --spinup-delay=0 -r -v -aRS03 -n$ECCSIZE" $TMPISO
fi

# Image contains 2 rows of missing sectors and a single one
# in the data portion

if try "reading image with missing data sectors" read_missing_data_sectors; then
   cp $MASTERISO $SIMISO
   $NEWVER -i$SIMISO --debug --erase 1000-1049 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --erase 21230 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --erase 22450-22457 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest read_missing_data_sectors "--spinup-delay=0 -r " $TMPISO
fi

# Image contains 1 row of missing sectors and a single one
# in the crc portion

if try "reading image with missing crc sectors" read_missing_crc_sectors; then
   cp $MASTERISO $SIMISO
   $NEWVER -i$SIMISO --debug --erase 21077 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --erase 21100-21120 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest read_missing_crc_sectors "--spinup-delay=0 -r " $TMPISO
fi

# Image contains 1 row of missing sectors and a single one
# in the ecc portion

if try "reading image with missing ecc sectors" read_missing_ecc_sectors; then
   cp $MASTERISO $SIMISO
   $NEWVER -i$SIMISO --debug --erase 21200 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --erase 21340-21365 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest read_missing_ecc_sectors "--spinup-delay=0 -r " $TMPISO
fi

# Image contains bad byte in the data section

if try "reading image with bad data byte" read_data_bad_byte; then
   cp $MASTERISO $SIMISO 
   $NEWVER -i$SIMISO --debug --byteset 0,50,10 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --byteset 1235,50,10 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --byteset 20999,50,10 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest read_data_bad_byte "--spinup-delay=0 -r " $TMPISO
fi

# Image contains bad byte in the crc section

if try "reading image with bad crc byte" read_crc_bad_byte; then
   cp $MASTERISO $SIMISO 
   $NEWVER -i$SIMISO --debug --byteset 21077,50,10 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest read_crc_bad_byte "--spinup-delay=0 -r " $TMPISO
fi

# Image contains bad byte in the ecc section

if try "reading image with bad ecc byte" read_ecc_bad_byte; then
   cp $MASTERISO $SIMISO 
   $NEWVER -i$SIMISO --debug --byteset 22000,50,10 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest read_ecc_bad_byte "--spinup-delay=0 -r " $TMPISO
fi

# Augmented image is protected by an outer RS01 error correction file
# Setting the byte creates a CRC error which can only be detected by
# the outer RS01 code, so we use this as an additional probe that the
# correct (outer) ECC is applied.

if try "reading with RS01 error correction file" read_with_rs01_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -c -n normal >>$LOGFILE 2>&1

    cp $MASTERISO $SIMISO
    $NEWVER --debug -i$SIMISO --byteset 24989,0,1 >>$LOGFILE 2>&1
    
    rm -f $TMPISO
    extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
    run_regtest read_with_rs01_file "--spinup-delay=0 -r " $TMPISO $TMPECC
fi

# Augmented image and non-matching RS01 error correction file
# Currently the mismatch is (generally) not detected
# NOTE: There seems to be an intermittent race condition between
# printing the defective sector number and the reading progress.
# Ignore for now and debug later.
# Should we change this behaviour?
# Expected behaviour for verify is to report the non-matching ecc file
# rather than falling back to using the RS02 part since the
# user did probably have some intentention specifying the ecc file.

if try "reading with non-matching RS01 error correction file" read_with_wrong_rs01_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -c -n normal >>$LOGFILE 2>&1
    $NEWVER --debug -i$TMPECC --byteset 0,24,1 >>$LOGFILE 2>&1

    cp $MASTERISO $SIMISO
    $NEWVER --debug -i$SIMISO --byteset 24989,0,1 >>$LOGFILE 2>&1
    
    rm -f $TMPISO
    extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
    run_regtest read_with_wrong_rs01_file "--spinup-delay=0 -r " $TMPISO $TMPECC
fi

# Augmented image is protected by an outer RS03 error correction file

if try "reading with RS03 error correction file" read_with_rs03_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -mRS03 -c -n 20 -o file >>$LOGFILE 2>&1

    cp $MASTERISO $SIMISO
    $NEWVER --debug -i$SIMISO --byteset 24989,0,1 >>$LOGFILE 2>&1
    
    rm -f $TMPISO
    extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
    run_regtest read_with_rs03_file "--spinup-delay=0 -r " $TMPISO $TMPECC
fi

# Augmented image and non-matching RS03 error correction file

if try "reading with non-matching RS03 error correction file" read_with_wrong_rs03_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -mRS03 -c -n 20 -o file >>$LOGFILE 2>&1
    $NEWVER --debug -i$TMPECC --byteset 0,24,1 >>$LOGFILE 2>&1

    cp $MASTERISO $SIMISO
    $NEWVER --debug -i$SIMISO --byteset 24989,0,1 >>$LOGFILE 2>&1
    
    rm -f $TMPISO
    extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
    run_regtest read_with_wrong_rs03_file "--spinup-delay=0 -r " $TMPISO $TMPECC
fi

# Augmented image containing several uncorrectable dead sector markers
# within the CRC area. There is a specical code segment in RS03GetCrcBuf()
# for testing this. Trigger the outout for DSM found in the medium.
# (test case in other places: DSM in image, file)

if try "crc section with uncorrectable dead sector markers" read_crc_section_with_uncorrectable_dsm; then

  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase "21077:pass as dead sector marker" >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase "21081:pass as dead sector marker" >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase "21082:pass as dead sector marker" >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_crc_section_with_uncorrectable_dsm  "--spinup-delay=0 -r " $TMPISO
fi

# Normal sized image with missing ecc header; without exhaustive search.
# In the first slice, the CRC sector and some other sectors are unreadable.

if try "reading with missing ecc header, not exhaustive" read_with_missing_header; then

  cp $LARGEMASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase $LMI_HEADER >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase $LMI_FIRSTCRC >>$LOGFILE 2>&1
  for i in $(seq $((120*LMI_LAYER_SIZE)) $LMI_LAYER_SIZE $((135*LMI_LAYER_SIZE))); do
      $NEWVER --debug -i$SIMISO --erase $i >>$LOGFILE 2>&1
  done

  rm -f $TMPISO $TMPECC
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_with_missing_header "--spinup-delay=0 -r" $TMPISO $TMPECC
fi

# Normal sized image with missing ecc header; with exhaustive search.
# In the first slice, the CRC sector and some other sectors are unreadable.

if try "reading with missing ecc header, exhaustive" read_with_missing_header_exhaustive; then

  cp $LARGEMASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase $LMI_HEADER >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --erase $LMI_FIRSTCRC >>$LOGFILE 2>&1
  for i in $(seq $((120*LMI_LAYER_SIZE)) $LMI_LAYER_SIZE $((135*LMI_LAYER_SIZE))); do
      $NEWVER --debug -i$SIMISO --erase $i >>$LOGFILE 2>&1
  done

  rm -f $TMPISO $TMPECC
  replace_config examine-rs03 1
  replace_config medium-size 0
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_with_missing_header_exhaustive "--spinup-delay=0 -r -v -a RS03" $TMPISO $TMPECC
fi

# Normal sized image with missing ecc header; with exhaustive search.
# In the first slice, the CRC sector and some other sectors are unreadable.

if try "reading with missing iso header, exhaustive" read_with_missing_iso_header_exhaustive; then

  cp $LARGEMASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase 16 >>$LOGFILE 2>&1

  rm -f $TMPISO $TMPECC
  replace_config examine-rs03 1
  replace_config medium-size 0
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_with_missing_iso_header_exhaustive "--spinup-delay=0 -r -v -a RS03" $TMPISO $TMPECC
fi

# Image contains Ecc header with the ecc file flag set

if try "image with ecc header from a file" read_with_ecc_file_header; then
   cp $MASTERISO $SIMISO

   # Ecc file bit
   $NEWVER -i$SIMISO --debug --byteset 21000,16,2 >>$LOGFILE 2>&1

   # self CRC sum
   $NEWVER -i$SIMISO --debug --byteset 21000,96,142 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --byteset 21000,97,43 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --byteset 21000,98,137 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --byteset 21000,99,29 >>$LOGFILE 2>&1
      
   rm -f $TMPISO
   replace_config examine-rs03 1
   replace_config medium-size 0
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values -n $ECCSIZE"
   run_regtest read_with_ecc_file_header "--spinup-delay=0 -r -v -a RS03" $TMPISO
fi

# Image contains defective Ecc header
# and a crc block from an ecc file

if try "image with crc block from a file" read_with_ecc_file_crc_block; then
   cp $MASTERISO $SIMISO

   # Delete Ecc Header

   $NEWVER -i$SIMISO --debug --erase 21000 >>$LOGFILE 2>&1

   # Ecc file bit
   $NEWVER -i$SIMISO --debug --byteset 21070,1040,2 >>$LOGFILE 2>&1

   # self CRC sum
   $NEWVER -i$SIMISO --debug --byteset 21070,1120,208 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --byteset 21070,1121,250 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --byteset 21070,1122,142 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --byteset 21070,1123,101 >>$LOGFILE 2>&1
      
   rm -f $TMPISO
   replace_config examine-rs03 1
   replace_config medium-size 0
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values -n $ECCSIZE"
   run_regtest read_with_ecc_file_crc_block "--spinup-delay=0 -r -v -a RS03" $TMPISO
fi

### Reading tests (adaptive)

echo "# Reading tests (adaptive)"
