#!/usr/bin/env bash

. common.bash

ISOSIZE=21000
REDUNDANCY=20

MASTERISO=$ISODIR/rs03f-master.iso
MASTERECC=$ISODIR/rs03f-master.ecc
SIMISO=$ISODIR/rs03f-sim.iso
TMPISO=$ISODIR/rs03f-tmp.iso
TMPECC=$ISODIR/rs03f-tmp.ecc
CODEC_PREFIX=RS03f

# Create master image

if ! file_exists $MASTERISO; then
    $NEWVER --debug -i$MASTERISO --random-image $ISOSIZE >>$LOGFILE 2>&1
    echo -e "$FILE_MSG"
    FILE_MSG=""
fi

if ! file_exists $MASTERECC; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$MASTERECC -mRS03 -n$REDUNDANCY -o file -c >>$LOGFILE 2>&1
    echo -e "$FILE_MSG"
    FILE_MSG=""
fi

# Create image whose length is not a multiple of 2048

ISO_PLUS56=$ISODIR/rs03f-plus56_bytes.iso
ECC_PLUS56=$ISODIR/rs03f-plus56_bytes.ecc
if ! file_exists $ISO_PLUS56; then
    cp $MASTERISO $ISO_PLUS56
    dd if="$RNDSEQ" count=1 bs=56 >>$ISO_PLUS56 2>/dev/null
    echo -e "$FILE_MSG"
    FILE_MSG=""
fi

if ! file_exists $ECC_PLUS56; then
    $NEWVER --debug --set-version $SETVERSION -i$ISO_PLUS56 -e$ECC_PLUS56 -mRS03 -n$REDUNDANCY -o file -c >>$LOGFILE 2>&1
    echo -e "$FILE_MSG"
    FILE_MSG=""
fi

### Verification tests

echo "# Verify tests"

# Test good files

if try "good image" good; then

  run_regtest good "-t" $MASTERISO $MASTERECC
fi

# Test good files, quick test

if try "good image, quick test" good_quick; then

  run_regtest good_quick "-tq" $MASTERISO $MASTERECC
fi

# Test with missing image, ecc file

if try "missing image" no_image; then
  run_regtest no_image "-t" $ISODIR/no.iso  $MASTERECC
fi

# Test with good image and ecc file, both not multiple of 2048

if try "image with 56 extra bytes" plus56_bytes; then

  cp $MASTERISO $TMPISO
  dd if="$RNDSEQ" count=1 bs=56 >>$TMPISO 2>/dev/null

  $NEWVER --debug --set-version $SETVERSION -i$TMPISO -e$TMPECC -mRS03 -n$REDUNDANCY -o file -c >>$LOGFILE 2>&1

  run_regtest plus56_bytes "-t" $TMPISO $TMPECC
fi

# Test with no image, ecc for image not multiple of 2048 

if try "no image; ecc for image with 56 extra bytes" no_image_plus56_bytes; then

  cp $MASTERISO $TMPISO
  dd if="$RNDSEQ" count=1 bs=56 >>$TMPISO 2>/dev/null

  $NEWVER --debug --set-version $SETVERSION -i$TMPISO -e$TMPECC -mRS03 -n$REDUNDANCY -o file -c >>$LOGFILE 2>&1

  run_regtest no_image_plus56_bytes "-t" $ISODIR/no.iso $TMPECC
fi


# Good image and ecc file, encoding needs no data padding as
# file size (20124) is dividable by layer size (86)

if try "image with special padding situation" special_padding; then
  $NEWVER --debug -i$TMPISO --random-image 20124 >>$LOGFILE 2>&1

  $NEWVER --debug --set-version $SETVERSION -i$TMPISO -e$TMPECC -mRS03 -n$REDUNDANCY -o file -c >>$LOGFILE 2>&1

  run_regtest special_padding "-v -t" $TMPISO $TMPECC
fi

# Good image and ecc file, encoding needs no data padding as
# file size (20124) is dividable by layer size (86).
# Last sector is only partially filled.

if try "image with special padding situation plus 56 bytes" special_padding_plus56; then
  $NEWVER --debug -i$TMPISO --random-image 20123 >>$LOGFILE 2>&1
  dd if="$RNDSEQ" count=1 bs=56 >>$TMPISO 2>/dev/null

  $NEWVER --debug --set-version $SETVERSION -i$TMPISO -e$TMPECC -mRS03 -n$REDUNDANCY -o file -c >>$LOGFILE 2>&1

  run_regtest special_padding_plus56 "-v -t" $TMPISO $TMPECC
fi

# Test with normal image, ecc for image not multiple of 2048 

if try "normal image; ecc for image with 56 extra bytes" normal_image_ecc_plus56_bytes; then
  cp $MASTERISO $TMPISO
  dd if="$RNDSEQ" count=1 bs=56 >>$TMPISO 2>/dev/null

  $NEWVER --debug --set-version $SETVERSION -i$TMPISO -e$TMPECC -mRS03 -n$REDUNDANCY -o file -c >>$LOGFILE 2>&1

  run_regtest normal_image_ecc_plus56_bytes "-t" $MASTERISO $TMPECC
fi

# Test with image not multiple of 2048, normal ecc file

if try "image with 56 extra bytes; ecc for normal image" image_plus56_normal_ecc; then
  cp $MASTERISO $TMPISO
  dd if="$RNDSEQ" count=1 bs=56 >>$TMPISO 2>/dev/null

  run_regtest image_plus56_normal_ecc "-t" $TMPISO $MASTERECC
fi

# Test with image a few bytes shorter than ecc (both not multiple of 2048)

if try "image a few bytes shorter as expected; both not multiple of 2048" few_bytes_shorter; then
  LONGISO=$ISODIR/rs03f-plus390-bytes.iso

  cp $MASTERISO $TMPISO
  dd if="$RNDSEQ" count=1 bs=56 >>$TMPISO 2>/dev/null

  cp $MASTERISO $LONGISO
  dd if="$RNDSEQ" count=1 bs=390 >>$LONGISO 2>/dev/null

  $NEWVER --debug --set-version $SETVERSION -i$LONGISO -e$TMPECC -mRS03 -n$REDUNDANCY -o file -c >>$LOGFILE 2>&1

  run_regtest few_bytes_shorter "-t" $TMPISO $TMPECC
  rm -f $LONGISO
fi

# Test with image a few bytes longer than ecc (both not multiple of 2048)

if try "image a few bytes longer as expected; both not multiple of 2048" few_bytes_longer; then
  SHORTISO=$ISODIR/rs03f-plus56-bytes.iso

  cp $MASTERISO $SHORTISO
  dd if="$RNDSEQ" count=1 bs=56 >>$SHORTISO 2>/dev/null

  cp $MASTERISO $TMPISO
  dd if="$RNDSEQ" count=1 bs=390 >>$TMPISO 2>/dev/null

  $NEWVER --debug --set-version $SETVERSION -i$SHORTISO -e$TMPECC -mRS03 -n$REDUNDANCY -o file -c >>$LOGFILE 2>&1

  run_regtest few_bytes_longer "-t" $TMPISO $TMPECC
  rm -f $SHORTISO
fi

# Image is a few bytes shorter (original multiple of 2048)

if try "image few bytes shorter than multiple of 2048" few_bytes_shorter2; then

  MYSIZE=$((2048*ISOSIZE-104))
  dd if=$MASTERISO of=$TMPISO count=1 bs=$MYSIZE >>$LOGFILE 2>&1

  run_regtest few_bytes_shorter2 "-t" $TMPISO $MASTERECC
fi

# Image is truncated by 5 sectors

if try "image truncated by 5 sectors" image_truncated_by5; then

  MYSIZE=$((2048*(ISOSIZE-5)))
  dd if=$MASTERISO of=$TMPISO count=1 bs=$MYSIZE >>$LOGFILE 2>&1

  run_regtest image_truncated_by5 "-t" $TMPISO $MASTERECC
fi

# Image contains 1 extra sector

if try "image with 1 extra sector" one_extra_sector; then

  cp $MASTERISO $TMPISO
  dd if="$RNDSEQ" count=1 bs=2048 >>$TMPISO 2>/dev/null

  run_regtest one_extra_sector "-t" $TMPISO $MASTERECC
fi

# Image contains 17 extra sectors

if try "image with 17 extra sectors" 17_extra_sectors; then

  cp $MASTERISO $TMPISO
  dd if=/dev/zero count=17 bs=2048 >>$TMPISO 2>/dev/null

  run_regtest 17_extra_sectors "-t" $TMPISO $MASTERECC
fi

# Image contains just one row of missing sectors

if try "image with missing sectors" missing_sectors; then
  cp $MASTERISO $TMPISO
  $NEWVER --debug -i $TMPISO --erase 500-524 >>$LOGFILE 2>&1

  run_regtest missing_sectors "-t" $TMPISO $MASTERECC
fi

# Image contains just CRC errors

if try "image with crc errors" crc_errors; then
  cp $MASTERISO $TMPISO
  $NEWVER --debug -i $TMPISO --byteset 670,50,50 >>$LOGFILE 2>&1
  $NEWVER --debug -i $TMPISO --byteset 770,50,50 >>$LOGFILE 2>&1
  $NEWVER --debug -i $TMPISO --byteset 771,50,50 >>$LOGFILE 2>&1
  $NEWVER --debug -i $TMPISO --byteset 772,50,50 >>$LOGFILE 2>&1

  run_regtest crc_errors "-t" $TMPISO $MASTERECC
fi

# Image contains 2 rows of missing sectors, a single one
# and a CRC error

if try "image with mixed errors" mixed_errors; then
  cp $MASTERISO $TMPISO
  $NEWVER --debug -i $TMPISO --erase 500-524 >>$LOGFILE 2>&1
  $NEWVER --debug -i $TMPISO --byteset 670,50,50 >>$LOGFILE 2>&1
  $NEWVER --debug -i $TMPISO --erase 699 >>$LOGFILE 2>&1
  $NEWVER --debug -i $TMPISO --byteset 770,50,50 >>$LOGFILE 2>&1
  $NEWVER --debug -i $TMPISO --byteset 771,50,50 >>$LOGFILE 2>&1
  $NEWVER --debug -i $TMPISO --byteset 772,50,50 >>$LOGFILE 2>&1
  $NEWVER --debug -i $TMPISO --erase 978-1001 >>$LOGFILE 2>&1

  run_regtest mixed_errors  "-t" $TMPISO $MASTERECC
fi

# CRC error in fingerprint sector

if try "crc error in fingerprint sector" crc_error_in_fingerprint; then
  cp $MASTERISO $TMPISO
  $NEWVER --debug -i $TMPISO --byteset 16,450,17 >>$LOGFILE 2>&1

  run_regtest crc_error_in_fingerprint "-t" $TMPISO $MASTERECC
fi

# fingerprint sector unreadable

if try "fingerprint sector unreadable" fingerprint_unreadable; then

  cp $MASTERISO $TMPISO
  $NEWVER --debug -i $TMPISO --erase 16 >>$LOGFILE 2>&1

  run_regtest fingerprint_unreadable "-t" $TMPISO $MASTERECC
fi

###
### Several manipulated ecc files
###

# Ecc header is missing

if try "Ecc header is missing" missing_ecc_header; then

  cp $MASTERECC $TMPECC
  $NEWVER --debug -i $TMPECC --erase 0 >>$LOGFILE 2>&1

  run_regtest missing_ecc_header "-t -v" $MASTERISO $TMPECC
fi

# Ecc header is missing, some CRC blocks are also gone

if try "Ecc header and some CRC blocks are missing" missing_ecc_header_and_crc; then

  cp $MASTERECC $TMPECC
  $NEWVER --debug -i $TMPECC --erase 0-16 >>$LOGFILE 2>&1

  run_regtest missing_ecc_header_and_crc "-t -v" $MASTERISO $TMPECC
fi

# Ecc header is missing, first CRC block is defective also

if try "Ecc header missing, first CRC block defective" missing_ecc_header_and_defective_crc; then

  cp $MASTERECC $TMPECC
  $NEWVER --debug -i $TMPECC --erase 0 >>$LOGFILE 2>&1
  $NEWVER --debug -i $TMPECC --byteset 2,50,107 >>$LOGFILE 2>&1

  run_regtest missing_ecc_header_and_defective_crc "-t -v" $MASTERISO $TMPECC
fi

# Ecc header has checksum error

if try "checksum error in ecc header" ecc_header_crc_error; then

  cp $MASTERECC $TMPECC
  $NEWVER --debug -i $TMPECC --byteset 0,32,107 >>$LOGFILE 2>&1

  run_regtest ecc_header_crc_error "-t -v" $MASTERISO $TMPECC
fi

# Truncated error correction file

if try "truncated ecc file" ecc_file_truncated; then

  dd if=$MASTERECC of=$TMPECC bs=2048 count=1788 >/dev/null 2>&1

  run_regtest ecc_file_truncated "-t" $MASTERISO $TMPECC
fi

# Error correction file with trailing garbage

if try "ecc file with trailing garbage" ecc_file_plus_garbage; then

  cp $MASTERECC $TMPECC
  dd if="$RNDSEQ" count=1 bs=3980 >>$TMPECC 2>>$LOGFILE

  run_regtest ecc_file_plus_garbage "-t" $MASTERISO $TMPECC
fi

# Error correction file with cookie-less CRC sector

if try "ecc file with cookie-less CRC sector" ecc_file_cookieless_crc; then

  cp $MASTERECC $TMPECC
  $NEWVER --debug -i $TMPECC --byteset 2,1024,70 >>$LOGFILE 2>&1

  run_regtest ecc_file_cookieless_crc "-t" $MASTERISO $TMPECC
fi

# Error correction file with two CRC errors in CRC sectors
# NOTE: Both defective cookies and defective checksums are
#       reported as "signature errors"!

if try "ecc file with byte errors in CRC sectors" ecc_file_defective_crc; then

  cp $MASTERECC $TMPECC
  $NEWVER --debug -i $TMPECC --byteset 4,101,70 >>$LOGFILE 2>&1
  $NEWVER --debug -i $TMPECC --byteset 5,908,23 >>$LOGFILE 2>&1

  run_regtest ecc_file_defective_crc "-t" $MASTERISO $TMPECC
fi

# Error correction file with byte error in ecc portion

if try "ecc file with byte error in ECC portion" ecc_file_defective_ecc; then

  cp $MASTERECC $TMPECC
  $NEWVER --debug -i $TMPECC --byteset 1040,101,70 >>$LOGFILE 2>&1

  run_regtest ecc_file_defective_ecc "-t" $MASTERISO $TMPECC
fi

# Error correction file with missing crc sectors

if try "ecc file with missing crc sectors" ecc_file_missing_crc; then

   cp $MASTERECC $TMPECC
   $NEWVER --debug -i $TMPECC --erase 10-19 >>$LOGFILE 2>&1

   run_regtest ecc_file_missing_crc "-t" $MASTERISO $TMPECC
fi

# Error correction file with missing ecc sectors

if try "ecc file with missing ecc sectors" ecc_file_missing_ecc; then

   cp $MASTERECC $TMPECC
   $NEWVER --debug -i $TMPECC --erase 1000-1014 >>$LOGFILE 2>&1

  run_regtest ecc_file_missing_ecc "-t" $MASTERISO $TMPECC
fi

# Error correction file with missing crc sector and real crc error in data sector
# Can not detect CRC error due to missing CRC sum, but Reed-Solomon will find it.

if try "ecc file with missing crc sector and crc error in data" ecc_file_missing_crc2; then

   cp $MASTERISO $TMPISO
   $NEWVER --debug -i $TMPISO --byteset 91,10,10 >>$LOGFILE 2>&1
 
   cp $MASTERECC $TMPECC
   $NEWVER --debug -i $TMPECC --erase 2 >>$LOGFILE 2>&1

   run_regtest ecc_file_missing_crc2 "-t" $TMPISO $TMPECC
fi

# Error correction file with corrupted crc sector and real crc error in data sector
# Can not detect CRC error due to missing CRC sum, but Reed-Solomon will find it.

if try "ecc file with corrupted crc sector and crc error in data" ecc_file_missing_crc3; then
  cp $MASTERISO $TMPISO
  $NEWVER --debug -i $TMPISO --byteset 91,10,10 >>$LOGFILE 2>&1

  cp $MASTERECC $TMPECC
  $NEWVER --debug -i $TMPECC --byteset 2,123,97 >>$LOGFILE 2>&1

  run_regtest ecc_file_missing_crc3 "-t" $TMPISO $TMPECC
fi

# Augmented image containing several uncorrectable dead sector markers
# within the CRC area of the ecc file. There is a specical code segment in RS03GetCrcBuf()
# for testing this. Trigger the output for DSM found in the image.
# (test case in other places: DSM in medium, image)

if try "crc section with uncorrectable dead sector markers" crc_section_with_uncorrectable_dsm; then

  cp $MASTERISO $TMPISO
  cp $MASTERECC $TMPECC
  $NEWVER --debug -i$TMPECC --erase 10 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --erase 15 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --erase 16 >>$LOGFILE 2>&1

  run_regtest crc_section_with_uncorrectable_dsm  "-t" $TMPISO $TMPECC
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

  run_regtest uncorrectable_dsm_in_image  "-t" $TMPISO $MASTERECC
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

  run_regtest uncorrectable_dsm_in_image_verbose  "-t -v" $TMPISO $MASTERECC
fi

# Augmented image containing several uncorrectable dead sector markers
# (sector displacement) in the ecc part of the ecc file

if try "ecc section with uncorrectable dead sector markers" ecc_section_with_uncorrectable_dsm; then

  cp $MASTERISO $TMPISO
  cp $MASTERECC $TMPECC
  $NEWVER --debug -i$TMPECC --erase 200 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --erase 240 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --erase 241 >>$LOGFILE 2>&1

  run_regtest ecc_section_with_uncorrectable_dsm  "-t" $TMPISO $TMPECC
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

  run_regtest uncorrectable_dsm_in_image2 "-t" $TMPISO $MASTERECC
fi

# Augmented image containing several uncorrectable dead sector markers
# (non matching fingerprint) in the data part, verbose outpur

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

  run_regtest uncorrectable_dsm_in_image2_verbose "-t -v" $TMPISO $MASTERECC
fi

### Creation tests

echo "# Creation tests"

# Create ecc file

if try "ecc file creation" ecc_create; then

  IGNORE_LOG_LINE="^Avg performance|^Creating the error correction file with Method RS03"
  replace_config method-name RS03
  replace_config ecc-target 0
  replace_config redundancy $REDUNDANCY
  extra_args="--debug --set-version $SETVERSION"
  run_regtest ecc_create "-mRS03 -n$REDUNDANCY -o file -c" $MASTERISO $TMPECC
fi

# Create with missing image

if try "ecc creating with missing image" ecc_missing_image; then
  NO_FILE=$ISODIR/none.iso

  replace_config method-name RS03
  replace_config ecc-target 0
  replace_config redundancy $REDUNDANCY
  extra_args="--debug --set-version $SETVERSION"
  run_regtest ecc_missing_image "-mRS03 -n $REDUNDANCY -o file -c" $NO_FILE $TMPECC
fi

# Create with already existing ecc file (of diffent redundancy)

if try "ecc creating with existing ecc file" ecc_existing_file; then

    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -mRS03 -n$((REDUNDANCY+10)) -o file -c >>$LOGFILE 2>&1


    IGNORE_LOG_LINE="^Avg performance|^Creating the error correction file with Method RS03"
    replace_config method-name RS03
    replace_config ecc-target 0
    replace_config redundancy $REDUNDANCY
    extra_args="--debug --set-version $SETVERSION"
    run_regtest ecc_existing_file "-mRS03 -n $REDUNDANCY -o file -c" $MASTERISO $TMPECC
fi

# Create with no read permission on image

if try "ecc creating with no read permission" ecc_no_read_perm; then

  cp $MASTERISO $TMPISO
  chmod 000 $TMPISO

  replace_config method-name RS03
  replace_config ecc-target 0
  replace_config redundancy $REDUNDANCY
  extra_args="--debug --set-version $SETVERSION"
  run_regtest ecc_no_read_perm "-mRS03 -n $REDUNDANCY -o file -c" $TMPISO $TMPECC

  rm -f $TMPISO
fi

# Create with no write permission on ecc file
# Should not do any harm at all: Ecc file will
# be recreated with write permissions

if try "ecc creating with no write permission" ecc_no_write_perm; then
  touch $TMPECC
  chmod 000 $TMPECC

  IGNORE_LOG_LINE="^Avg performance|^Creating the error correction file with Method RS03"
  replace_config method-name RS03
  replace_config ecc-target 0
  replace_config redundancy $REDUNDANCY
  extra_args="--debug --set-version $SETVERSION"
  run_regtest ecc_no_write_perm "-mRS03 -n $REDUNDANCY -o file -c" $MASTERISO $TMPECC
fi

# Create ecc file for image with 56 additional bytes
if try "image with 56 extra bytes" ecc_create_plus56; then

   cp $MASTERISO $TMPISO
   dd if="$RNDSEQ" count=1 bs=56 >>$TMPISO 2>$LOGFILE

   IGNORE_LOG_LINE="^Avg performance|^Creating the error correction file with Method RS03"
   replace_config method-name RS03
   replace_config ecc-target 0
   replace_config redundancy $REDUNDANCY
   extra_args="--debug --set-version $SETVERSION"
   run_regtest ecc_create_plus56 "-mRS03 -n $REDUNDANCY -o file -c" $TMPISO $TMPECC
fi

# Try to create ecc file from image with missing sectors

if try "creating ecc from image with missing sectors" ecc_missing_sectors; then

  cp $MASTERISO $TMPISO
  $NEWVER --debug -i $TMPISO --erase 500-524 >>$LOGFILE 2>&1

  IGNORE_LOG_LINE="^Avg performance|^Creating the error correction file with Method RS03"
  replace_config method-name RS03
  replace_config ecc-target 0
  replace_config redundancy $REDUNDANCY
  extra_args="--debug --set-version $SETVERSION"
  run_regtest ecc_missing_sectors "-mRS03 -n $REDUNDANCY -o file -c" $TMPISO $TMPECC
fi

# Read image and create ecc in the same program call.
# Tests whether CRC and ECC information is handed over correctly.
# NOTE: cache handling is currently disabled and will be fixed in 0.79.6!

if try "read image and create ecc in one call" ecc_create_after_read; then
  cp $MASTERISO $SIMISO

  rm -f $TMPISO $TMPECC
  IGNORE_LOG_LINE="^Avg performance|^Creating the error correction file with Method RS03"
  replace_config method-name RS03
  replace_config ecc-target 0
  replace_config redundancy $REDUNDANCY
  replace_config read-and-create 1
  extra_args="--debug --set-version $SETVERSION --sim-cd=$SIMISO  --fixed-speed-values"
  run_regtest ecc_create_after_read "-r -c -mRS03 -o file -n$REDUNDANCY" $TMPISO $TMPECC
fi

# Read image with ecc file and create new (other) ecc in the same program call.
# Tests whether CRC and ECC information is handed over correctly.
# NOTE: cache handling is currently disabled and will be fixed in 0.79.6!

if try "read image with ecc (RS01) and create new ecc" ecc_recreate_after_read_rs01; then
  cp $MASTERISO $SIMISO

  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -e$TMPECC -c -n 8 >>$LOGFILE 2>&1

  rm -f $TMPISO
  IGNORE_LOG_LINE="^Avg performance|^Creating the error correction file with Method RS03"
  replace_config method-name RS03
  replace_config ecc-target 0
  replace_config redundancy $REDUNDANCY
  extra_args="--debug --set-version $SETVERSION --sim-cd=$SIMISO  --fixed-speed-values"
  run_regtest ecc_recreate_after_read_rs01 "-r -c -mRS03 -o file -n$REDUNDANCY" $TMPISO $TMPECC
fi

# Read image with ecc file and create new (other) ecc in the same program call.
# Tests whether CRC and ECC information is handed over correctly.
# NOTE: cache handling is currently disabled and will be fixed in 0.79.6!

if try "read image with ecc (RS02) and create new ecc" ecc_recreate_after_read_rs02; then
  cp $MASTERISO $SIMISO

  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -mRS02 -c -n$((ISOSIZE+3000)) >>$LOGFILE 2>&1

  rm -f $TMPISO $TMPECC
  IGNORE_LOG_LINE="^Avg performance|^Creating the error correction file with Method RS03"
  replace_config method-name RS03
  replace_config ecc-target 0
  replace_config redundancy $REDUNDANCY
  extra_args="--debug --set-version $SETVERSION --sim-cd=$SIMISO  --fixed-speed-values"
  run_regtest ecc_recreate_after_read_rs02 "-r -c -mRS03 -o file -n$REDUNDANCY" $TMPISO $TMPECC
fi

# Read image with ecc file and create new (other) ecc in the same program call.
# Tests whether CRC and ECC information is handed over correctly.
# NOTE: cache handling is currently disabled and will be fixed in 0.79.6!

if try "read image with ecc (RS03i) and create new ecc" ecc_recreate_after_read_rs03i; then
  cp $MASTERISO $SIMISO

  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -mRS03 -c -n$((ISOSIZE+3000)) >>$LOGFILE 2>&1

  rm -f $TMPISO $TMPECC
  IGNORE_LOG_LINE="^Avg performance|^Creating the error correction file with Method RS03"
  replace_config method-name RS03
  replace_config ecc-target 0
  replace_config redundancy $REDUNDANCY
  extra_args="--debug --set-version $SETVERSION --sim-cd=$SIMISO  --fixed-speed-values"
  run_regtest ecc_recreate_after_read_rs03i "-r -c -mRS03 -o file -n$REDUNDANCY" $TMPISO $TMPECC
fi

# Read image with ecc file and create new (other) ecc in the same program call.
# Tests whether CRC and ECC information is handed over correctly.
# NOTE: cache handling is currently disabled and will be fixed in 0.79.6!

if try "read image with ecc (RS03f) and create new ecc" ecc_recreate_after_read_rs03f; then
  cp $MASTERISO $SIMISO

  $NEWVER --debug --set-version $SETVERSION -i$SIMISO -e$TMPECC -mRS03 -o file -c -n 9 >>$LOGFILE 2>&1

  rm -f $TMPISO
  IGNORE_LOG_LINE="^Avg performance|^Creating the error correction file with Method RS03"
  replace_config method-name RS03
  replace_config ecc-target 0
  replace_config redundancy $REDUNDANCY
  extra_args="--debug --set-version $SETVERSION --sim-cd=$SIMISO  --fixed-speed-values"
  run_regtest ecc_recreate_after_read_rs03f "-r -c -mRS03 -o file -n$REDUNDANCY" $TMPISO $TMPECC
fi

# Complete image in a reading pass, then create an ecc file for it.
# Cached checksums must be discarded before creating the ecc.
# NOTE: cache handling is currently disabled and will be fixed in 0.79.6!

if try "create ecc after completing partial image" ecc_create_after_partial_read; then
  cp $MASTERISO $TMPISO

  $NEWVER --debug -i$TMPISO --erase 1000-1500 >>$LOGFILE 2>&1

  rm -f $TMPECC
  IGNORE_LOG_LINE="^Avg performance|^Creating the error correction file with Method RS03"
  replace_config method-name RS03
  replace_config ecc-target 0
  replace_config redundancy $REDUNDANCY
  extra_args="--debug --set-version $SETVERSION --sim-cd=$SIMISO  --fixed-speed-values"
  run_regtest ecc_create_after_partial_read "-r -c -mRS03 -o file -n$REDUNDANCY" $TMPISO $TMPECC
fi

### Fixing tests

echo "# Repair tests"

# Fix good image

if try "fixing good image" fix_good; then
  cp $MASTERISO $TMPISO
  cp $MASTERECC $TMPECC

  run_regtest fix_good "-f" $TMPISO $TMPECC
fi

# Fix image with missing data sectors

if try "fixing image with missing data sectors" fix_missing_data_sectors; then
  cp $MASTERISO $TMPISO
  cp $MASTERECC $TMPECC

  $NEWVER --debug -i $TMPISO --erase 900-924 >>$LOGFILE 2>&1
  $NEWVER --debug -i $TMPISO --erase 73 >>$LOGFILE 2>&1

  run_regtest fix_missing_data_sectors "-f" $TMPISO $TMPECC
fi

# Fix ecc file with missing crc sectors

if try "fixing ecc file with missing crc sectors" fix_missing_crc_sectors; then
  cp $MASTERISO $TMPISO
  cp $MASTERECC $TMPECC

  $NEWVER --debug -i $TMPECC --erase 5-9 >>$LOGFILE 2>&1

  run_regtest fix_missing_crc_sectors "-f" $TMPISO $TMPECC
fi

# Fix ecc file with missing ecc sectors

if try "fixing ecc file with missing ecc sectors" fix_missing_ecc_sectors; then
  cp $MASTERISO $TMPISO
  cp $MASTERECC $TMPECC

  $NEWVER --debug -i $TMPECC --erase 115-119 >>$LOGFILE 2>&1

  run_regtest fix_missing_ecc_sectors "-f" $TMPISO $TMPECC
fi

# Fix image with missing sectors in several border locations

if try "trying to fix image with missing sectors in border cases" fix_border_cases_erasures; then
  cp $MASTERISO $TMPISO
  cp $MASTERECC $TMPECC
  $NEWVER --debug -i$TMPISO --erase 0 >>$LOGFILE 2>&1       # first sector
  $NEWVER --debug -i$TMPISO --erase 90 >>$LOGFILE 2>&1      # first sector, second layer
  $NEWVER --debug -i$TMPISO --erase 180 >>$LOGFILE 2>&1     # first sector, third layer
  $NEWVER --debug -i$TMPISO --erase 20970 >>$LOGFILE 2>&1   # first sector, last data layer
  $NEWVER --debug -i$TMPECC --erase 2 >>$LOGFILE 2>&1       # first sector, crc layer
  $NEWVER --debug -i$TMPECC --erase 92 >>$LOGFILE 2>&1      # first sector, first ecc layer
  $NEWVER --debug -i$TMPECC --erase 182 >>$LOGFILE 2>&1     # first sector, second ecc layer
  $NEWVER --debug -i$TMPECC --erase 1802 >>$LOGFILE 2>&1    # first sector, last ecc layer

  $NEWVER --debug -i$TMPISO --erase 89 >>$LOGFILE 2>&1      # first sector
  $NEWVER --debug -i$TMPISO --erase 179 >>$LOGFILE 2>&1     # first sector, second layer
  $NEWVER --debug -i$TMPISO --erase 269 >>$LOGFILE 2>&1     # first sector, third layer
  $NEWVER --debug -i$TMPISO --erase 20999 >>$LOGFILE 2>&1   # first sector, last data layer
  $NEWVER --debug -i$TMPECC --erase 91 >>$LOGFILE 2>&1       # first sector, crc layer
  $NEWVER --debug -i$TMPECC --erase 181 >>$LOGFILE 2>&1      # first sector, first ecc layer
  $NEWVER --debug -i$TMPECC --erase 271 >>$LOGFILE 2>&1     # first sector, second ecc layer
  $NEWVER --debug -i$TMPECC --erase 1891 >>$LOGFILE 2>&1    # first sector, last ecc layer

  run_regtest fix_border_cases_erasures "-f" $TMPISO $TMPECC
fi

# Fix image with crc errors in several border locations

if try "trying to fix image with crc errors in border cases" fix_border_cases_crc_errors; then
  cp $MASTERISO $TMPISO
  cp $MASTERECC $TMPECC
  $NEWVER --debug -i$TMPISO --byteset 0,0,1 >>$LOGFILE 2>&1       # first sector
  $NEWVER --debug -i$TMPISO --byteset 90,0,0 >>$LOGFILE 2>&1      # first sector, second layer
  $NEWVER --debug -i$TMPISO --byteset 180,0,0 >>$LOGFILE 2>&1     # first sector, third layer
  $NEWVER --debug -i$TMPISO --byteset 20970,0,0 >>$LOGFILE 2>&1   # first sector, last data layer
  $NEWVER --debug -i$TMPECC --byteset 2,0,0 >>$LOGFILE 2>&1       # first sector, crc layer
  $NEWVER --debug -i$TMPECC --byteset 92,0,0 >>$LOGFILE 2>&1      # first sector, first ecc layer
  $NEWVER --debug -i$TMPECC --byteset 182,0,0 >>$LOGFILE 2>&1     # first sector, second ecc layer
  $NEWVER --debug -i$TMPECC --byteset 1802,0,0 >>$LOGFILE 2>&1    # first sector, last ecc layer

  $NEWVER --debug -i$TMPISO --byteset 89,0,0 >>$LOGFILE 2>&1      # first sector
  $NEWVER --debug -i$TMPISO --byteset 179,0,0 >>$LOGFILE 2>&1     # first sector, second layer
  $NEWVER --debug -i$TMPISO --byteset 269,0,0 >>$LOGFILE 2>&1     # first sector, third layer
  $NEWVER --debug -i$TMPISO --byteset 20999,0,0 >>$LOGFILE 2>&1   # first sector, last data layer
  $NEWVER --debug -i$TMPECC --byteset 91,0,0 >>$LOGFILE 2>&1       # first sector, crc layer
  $NEWVER --debug -i$TMPECC --byteset 181,0,0 >>$LOGFILE 2>&1      # first sector, first ecc layer
  $NEWVER --debug -i$TMPECC --byteset 271,0,0 >>$LOGFILE 2>&1     # first sector, second ecc layer
  $NEWVER --debug -i$TMPECC --byteset 1891,0,0 >>$LOGFILE 2>&1    # first sector, last ecc layer

  run_regtest fix_border_cases_crc_errors "-f" $TMPISO $TMPECC
fi

# Fix image without read permission on image

if try "fixing image without read permission" fix_no_read_perm; then

  cp $MASTERISO $TMPISO
  chmod 000 $TMPISO

  run_regtest fix_no_read_perm "-f" $TMPISO $MASTERECC
  rm -f $TMPISO
fi

# Fix image without read permission on ecc file

if try "fixing image without read permission on ecc" fix_no_read_perm_ecc; then

  cp $MASTERISO $TMPISO
  cp $MASTERECC $TMPECC
  chmod 000 $TMPECC

  run_regtest fix_no_read_perm_ecc "-f" $TMPISO $TMPECC
  rm -f $TMPECC
fi

# Fix image without write permission

if try "fixing image without write permission" fix_no_write_perm; then

  cp $MASTERISO $TMPISO
  chmod 400 $TMPISO

  run_regtest fix_no_write_perm "-f" $TMPISO $MASTERECC
  rm -f $TMPISO
fi

# Fix image without write permission for ecc
# TODO: The error message should be more specific

if try "fixing image without write permission for ecc" fix_no_write_perm_ecc; then

  cp $MASTERISO $TMPISO
  cp $MASTERECC $TMPECC
  chmod 400 $TMPECC

  run_regtest fix_no_write_perm_ecc "-f" $TMPISO $TMPECC
  rm -f $TMPECC
fi

# Fix good image not multiple of 2048

if try "fixing good image not multiple of 2048" fix_good_plus56; then
  cp $ISO_PLUS56 $TMPISO
  
  run_regtest fix_good_plus56 "-f" $TMPISO $ECC_PLUS56
fi

# Fix image with additional sectors (TAO case)

if try "fixing image with one additional sector" fix_additional_sector; then
  cp $MASTERISO $TMPISO
  dd if="$RNDSEQ" count=1 bs=2048 >>$TMPISO 2>/dev/null

  run_regtest fix_additional_sector "-f" $TMPISO $MASTERECC
fi

# Fix image with additional sectors (general case)

if try "fixing image with 17 additional sectors" fix_plus17; then
  cp $MASTERISO $TMPISO
  dd if=/dev/zero count=17 bs=2048 >>$TMPISO 2>/dev/null

  run_regtest fix_plus17 "-f" $TMPISO $MASTERECC
fi

# Fix image with additional sectors (general case), with --truncate

if try "fixing image with 17 additional sectors with --truncate" fix_plus17_truncate; then
  cp $MASTERISO $TMPISO
  dd if=/dev/zero count=17 bs=2048 >>$TMPISO 2>/dev/null

  run_regtest fix_plus17_truncate "-f --truncate" $TMPISO $MASTERECC
fi

# Fix image+56bytes 

if try "fixing image with CRC error in 56 additional bytes" fix_plus56; then
  cp $ISO_PLUS56 $TMPISO
  $NEWVER -i$TMPISO --debug --byteset $ISOSIZE,28,90 >>$LOGFILE 2>&1

  run_regtest fix_plus56 "-f" $TMPISO $ECC_PLUS56
fi

# Fix image+56bytes+more bytes

if try "fixing image with CRC error in 56 additional bytes + few bytes more" fix_plus56_plus17; then
  cp $ISO_PLUS56 $TMPISO
  echo "0123456789abcdef" >>$TMPISO
  $NEWVER -i$TMPISO --debug --byteset $ISOSIZE,55,90 >>$LOGFILE 2>&1

  run_regtest fix_plus56_plus17 "-f" $TMPISO $ECC_PLUS56
fi

# Fix image+56bytes+more bytes, truncate option set

if try "fixing image with CRC error in 56 additional bytes + few bytes more w/ truncate" fix_plus56_plus17_truncate; then
  cp $ISO_PLUS56 $TMPISO
  echo "0123456789abcdef" >>$TMPISO
  $NEWVER -i$TMPISO --debug --byteset $ISOSIZE,55,90 >>$LOGFILE 2>&1

  run_regtest fix_plus56_plus17_truncate "-f --truncate" $TMPISO $ECC_PLUS56
fi

# Fix image+56bytes+1 sector 

if try "fixing image with CRC error in 56 additional bytes + one sector more" fix_plus56_plus1s; then
  cp $ISO_PLUS56 $TMPISO
  dd if="$RNDSEQ" count=1 bs=2048 >>$TMPISO 2>/dev/null
  $NEWVER -i$TMPISO --debug --byteset 21000,55,90 >>$LOGFILE 2>&1

  run_regtest fix_plus56_plus1s "-f --truncate" $TMPISO $ECC_PLUS56
fi

# Fix image+56bytes+2 sectors 

if try "fixing image with CRC error in 56 additional bytes + two sectors more" fix_plus56_plus2s; then
  cp $ISO_PLUS56 $TMPISO
  dd if="$RNDSEQ" count=1 bs=4096 >>$TMPISO 2>/dev/null
  $NEWVER -i$TMPISO --debug --byteset 21000,55,90 >>$LOGFILE 2>&1

  run_regtest fix_plus56_plus2s "-f --truncate" $TMPISO $ECC_PLUS56
fi

# Fix image+56bytes+more sectors 

if try "fixing image with CRC error in 56 additional bytes + more sectors" fix_plus56_plus17500; then
  cp $ISO_PLUS56 $TMPISO
  dd if=/dev/zero count=1 bs=17500 >>$TMPISO 2>/dev/null
  $NEWVER -i$TMPISO --debug --byteset 21000,55,90 >>$LOGFILE 2>&1

  run_regtest fix_plus56_plus17500 "-f --truncate" $TMPISO $ECC_PLUS56
fi

# Fix truncated image

if try "fixing truncated image" fix_truncated; then
  cp $MASTERISO $TMPISO
  $NEWVER -i$TMPISO --debug --truncate=$((ISOSIZE-269)) >>$LOGFILE 2>&1

  run_regtest fix_truncated "-f" $TMPISO $MASTERECC
fi

# Fix truncated image not a multiple of 2048

if try "fixing truncated image not a multiple of 2048" fix_plus56_truncated; then
  cp $ISO_PLUS56 $TMPISO
  $NEWVER -i$TMPISO --debug --truncate=$((ISOSIZE-28)) >>$LOGFILE 2>&1

  run_regtest fix_plus56_truncated "-f" $TMPISO $ECC_PLUS56
fi

# Fix truncated image not a multiple of 2048 and a few bytes shorter

if try "fixing image not a multiple of 2048 missing a few bytes" fix_plus56_little_truncated; then
  cp $MASTERISO $TMPISO
  dd if="$RNDSEQ" count=1 bs=50 >>$TMPISO 2>/dev/null

  run_regtest fix_plus56_little_truncated "-f" $TMPISO $ECC_PLUS56
fi

# Fix with Truncated error correction file

if try "fixing truncated ecc file" fix_ecc_file_truncated; then

  dd if=$MASTERECC of=$TMPECC bs=2048 count=1788 >/dev/null 2>&1

  run_regtest fix_ecc_file_truncated "-f" $MASTERISO $TMPECC
fi

# Ecc header is missing

if try "fixing ecc file with missing ecc header" fix_missing_ecc_header; then

  cp $MASTERECC $TMPECC
  $NEWVER --debug -i $TMPECC --erase 0 >>$LOGFILE 2>&1

  run_regtest fix_missing_ecc_header "-f -v" $MASTERISO $TMPECC
fi

### Scanning tests

echo "# Scanning tests"

# Scan complete / optimal image

if try "scanning good image" scan_good; then
  cp $MASTERISO $SIMISO

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_good "--spinup-delay=0 -s" $TMPISO $MASTERECC
fi

# Scan complete / optimal image

if try "scanning good image, verbose output" scan_good_verbose; then
  cp $MASTERISO $SIMISO

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_good_verbose "--spinup-delay=0 -s -v" $TMPISO $MASTERECC
fi

# Scan image which is shorter than expected
# TODO: Currently we are trying to scan past the medium
# and getting media errors. Is that smart? Rethink later.

if try "scanning image being shorter than expected" scan_shorter; then
  cp $MASTERISO $SIMISO

  $NEWVER --debug -i$SIMISO --truncate=$((ISOSIZE-44)) >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_shorter "--spinup-delay=0 -s" $TMPISO $MASTERECC
fi

# Scan image which is longer than expected
# Will return image in its original length.

if try "scanning image being longer than expected" scan_longer; then
  cp $MASTERISO $SIMISO

  for i in $(seq 23); do cat fixed-random-sequence >>$SIMISO; done

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_longer "--spinup-delay=0 -s -v" $TMPISO $MASTERECC
fi

# Scan image with two multisession link sectors appended.
# Will return image in its original length.

if try "scanning image, tao tail case" scan_tao_tail; then
  cp $MASTERISO $SIMISO

  cat fixed-random-sequence >>$SIMISO
  $NEWVER --debug -i$SIMISO --erase 21000-21001 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_tao_tail "--spinup-delay=0 -s" $TMPISO $MASTERECC
fi

# Scan image with two real sectors missing at the end.
# -dao option prevents them from being clipped off.

if try "scanning image, no tao tail case" scan_no_tao_tail; then
  cp $MASTERISO $SIMISO

  cat fixed-random-sequence >>$SIMISO
  $NEWVER --debug -i$SIMISO --erase 20998-20999 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_no_tao_tail "--spinup-delay=0 -s --dao" $TMPISO $MASTERECC
fi

# Scan an image for which ecc information is available,
# but requiring a newer dvdisaster version.
# NOTE: Only the master header is manipulated, which is
# sufficient for reaching the goal of this test case.

if try "scanning image requiring a newer dvdisaster version" scan_incompatible_ecc; then

  cp $MASTERISO $SIMISO
  cp $MASTERECC $TMPECC
  # Creator version 99.99
  $NEWVER --debug -i$TMPECC --byteset 0,84,220 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --byteset 0,85,65 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --byteset 0,86,15 >>$LOGFILE 2>&1
  # Version info 99.99
  $NEWVER --debug -i$TMPECC --byteset 0,88,220 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --byteset 0,89,65 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --byteset 0,90,15 >>$LOGFILE 2>&1
  # Patched selfcrc
  $NEWVER --debug -i$TMPECC --byteset 0,96,123 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --byteset 0,97,99 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --byteset 0,98,62 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --byteset 0,99,9 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_incompatible_ecc "--spinup-delay=0 -s" $TMPISO $TMPECC
fi

# Scan an image containing a defective ECC header.
# Will be treated like an ECC-less image since --assume is not set.

if try "scanning image with a defective header" scan_bad_header; then

  cp $MASTERISO $SIMISO
  cp $MASTERECC $TMPECC
  $NEWVER -i$TMPECC --debug --byteset 0,1,1 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_bad_header "--spinup-delay=0 -s -v" $TMPISO $TMPECC
fi

# Image contains 2 rows of missing sectors and a single one
# in the data portion

if try "scanning image with missing data sectors" scan_missing_data_sectors; then
   cp $MASTERISO $SIMISO
   $NEWVER -i$SIMISO --debug --erase 1000-1049 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --erase 11230 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --erase 12450-12457 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest scan_missing_data_sectors "--spinup-delay=0 -s " $TMPISO $MASTERECC
fi

# Image contains 1 row of missing sectors and a single one
# in the crc portion
# TODO: Message "This image was probably mastered from defective sources"
#       does not really make sense as the ecc file is affected.

if try "scanning image with missing crc sectors" scan_missing_crc_sectors; then
   cp $MASTERISO $SIMISO
   cp $MASTERECC $TMPECC 
   $NEWVER -i$TMPECC --debug --erase 5 >>$LOGFILE 2>&1
   $NEWVER -i$TMPECC --debug --erase 77-86 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest scan_missing_crc_sectors "--spinup-delay=0 -s " $TMPISO $TMPECC
fi

# Image contains 1 row of missing sectors and a single one
# in the ecc portion

if try "scanning image with missing ecc sectors" scan_missing_ecc_sectors; then
   cp $MASTERISO $SIMISO
   cp $MASTERECC $TMPECC
   $NEWVER -i$TMPECC --debug --erase 120 >>$LOGFILE 2>&1
   $NEWVER -i$TMPECC --debug --erase 134-190 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest scan_missing_ecc_sectors "--spinup-delay=0 -s " $TMPISO $TMPECC
fi

# Image contains bad byte in the data section

if try "scanning image with bad data byte" scan_data_bad_byte; then
   cp $MASTERISO $SIMISO 
   $NEWVER -i$SIMISO --debug --byteset 1235,50,10 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest scan_data_bad_byte "--spinup-delay=0 -s " $TMPISO $MASTERECC
fi

# Image contains bad byte in the crc section

if try "scanning image with bad crc byte" scan_crc_bad_byte; then
   cp $MASTERISO $SIMISO
   cp $MASTERECC $TMPECC
   $NEWVER -i$TMPECC --debug --byteset 77,50,10 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest scan_crc_bad_byte "--spinup-delay=0 -s " $TMPISO $TMPECC
fi

# Image contains bad byte in the ecc section

if try "scanning image with bad ecc byte" scan_ecc_bad_byte; then
   cp $MASTERISO $SIMISO
   cp $MASTERECC $TMPECC
   $NEWVER -i$TMPECC --debug --byteset 200,50,10 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest scan_ecc_bad_byte "--spinup-delay=0 -s " $TMPISO $TMPECC
fi

# Ecc header is missing

if try "scanning image with missing ecc header" scan_missing_ecc_header; then
  cp $MASTERECC $TMPECC
  $NEWVER --debug -i $TMPECC --erase 0 >>$LOGFILE 2>&1

  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_missing_ecc_header "--spinup-delay=0 -s -v" $MASTERISO $TMPECC
fi

# Ecc header is missing, some CRC blocks are also gone

if try "scanning image with missing ecc header and CRC blocks" scan_missing_ecc_header_and_crc; then

  cp $MASTERECC $TMPECC
  $NEWVER --debug -i $TMPECC --erase 0-16 >>$LOGFILE 2>&1

  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_missing_ecc_header_and_crc "--spinup-delay=0 -s -v" $MASTERISO $TMPECC
fi

# Ecc header is missing, first CRC block is defective also

if try "scanning image with ecc header missing, first CRC block defective" scan_missing_ecc_header_and_defective_crc; then

  cp $MASTERECC $TMPECC
  $NEWVER --debug -i $TMPECC --erase 0 >>$LOGFILE 2>&1
  $NEWVER --debug -i $TMPECC --byteset 2,50,107 >>$LOGFILE 2>&1

  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_missing_ecc_header_and_defective_crc "--spinup-delay=0 -s -v" $MASTERISO $TMPECC
fi

# Ecc header has checksum error

if try "checksum error in ecc header" scan_ecc_header_crc_error; then

  cp $MASTERECC $TMPECC
  $NEWVER --debug -i $TMPECC --byteset 0,32,107 >>$LOGFILE 2>&1

  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_ecc_header_crc_error "--spinup-delay=0 -s -v" $MASTERISO $TMPECC
fi

### Reading tests (linear)

echo "# Reading tests (linear)"

# Read complete / optimal image

if try "reading good image" read_good; then
  cp $MASTERISO $SIMISO

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_good "--spinup-delay=0 -r" $TMPISO $MASTERECC
fi

# Read complete / optimal image

if try "reading good image, verbose output" read_good_verbose; then
  cp $MASTERISO $SIMISO

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_good_verbose "--spinup-delay=0 -r -v" $TMPISO $MASTERECC
fi

# Read into existing and complete image file

if try "reading good image in good file" read_good_file; then
  cp $MASTERISO $SIMISO

  cp $MASTERISO $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_good_file "--spinup-delay=0 -r" $TMPISO  $MASTERECC
fi

# Read image which is shorter than expected
# TODO: Currently we are trying to read past the medium
# and getting media errors. Is that smart? Rethink later.

if try "reading image being shorter than expected" read_shorter; then
  cp $MASTERISO $SIMISO

  $NEWVER --debug -i$SIMISO --truncate=$((ISOSIZE-44)) >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_shorter "--spinup-delay=0 -r" $TMPISO $MASTERECC
fi

# Read image which is longer than expected
# Will return image in its original length.

if try "reading image being longer than expected" read_longer; then
  cp $MASTERISO $SIMISO

  for i in $(seq 23); do cat fixed-random-sequence >>$SIMISO; done

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_longer "--spinup-delay=0 -r -v" $TMPISO $MASTERECC
fi

# Read image with two multisession link sectors appended.
# Will return image in its original length.

if try "reading image, tao tail case" read_tao_tail; then
  cp $MASTERISO $SIMISO

  cat fixed-random-sequence >>$SIMISO
  $NEWVER --debug -i$SIMISO --erase 21000-21001 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_tao_tail "--spinup-delay=0 -r" $TMPISO $MASTERECC
fi

# Read image with two real sectors missing at the end.
# -dao option prevents them from being clipped off.

if try "reading image, no tao tail case" read_no_tao_tail; then
  cp $MASTERISO $SIMISO

  cat fixed-random-sequence >>$SIMISO
  $NEWVER --debug -i$SIMISO --erase 20998-20999 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_no_tao_tail "--spinup-delay=0 -r --dao" $TMPISO $MASTERECC
fi

# Read an image for which ecc information is available,
# but requiring a newer dvdisaster version.
# NOTE: Only the master header is manipulated, which is
# sufficient for reaching the goal of this test case.

if try "reading image requiring a newer dvdisaster version" read_incompatible_ecc; then

  cp $MASTERISO $SIMISO
  cp $MASTERECC $TMPECC
  # Creator version 99.99
  $NEWVER --debug -i$TMPECC --byteset 0,84,220 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --byteset 0,85,65 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --byteset 0,86,15 >>$LOGFILE 2>&1
  # Version info 99.99
  $NEWVER --debug -i$TMPECC --byteset 0,88,220 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --byteset 0,89,65 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --byteset 0,90,15 >>$LOGFILE 2>&1
  # Patched selfcrc
  $NEWVER --debug -i$TMPECC --byteset 0,96,123 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --byteset 0,97,99 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --byteset 0,98,62 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --byteset 0,99,9 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_incompatible_ecc "--spinup-delay=0 -r " $TMPISO $TMPECC
fi

# Read an image containing a defective ECC header.
# Note: The ecc file opened read only, so the defective header
# will not be rewritten during the read.

if try "reading image with a defective header" read_bad_header; then

  cp $MASTERISO $SIMISO
  cp $MASTERECC $TMPECC
  $NEWVER -i$TMPECC --debug --byteset 0,1,1 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_bad_header "--spinup-delay=0 -r -v" $TMPISO $TMPECC
fi

# Image contains 2 rows of missing sectors and a single one
# in the data portion

if try "reading image with missing data sectors" read_missing_data_sectors; then
   cp $MASTERISO $SIMISO
   $NEWVER -i$SIMISO --debug --erase 1000-1049 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --erase 11230 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --erase 12450-12457 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest read_missing_data_sectors "--spinup-delay=0 -r " $TMPISO $MASTERECC
fi

# Image contains 1 row of missing sectors and a single one
# in the crc portion
# TODO: Message "This image was probably mastered from defective sources"
#       does not really make sense as the ecc file is affected.

if try "reading image with missing crc sectors" read_missing_crc_sectors; then
   cp $MASTERISO $SIMISO
   cp $MASTERECC $TMPECC 
   $NEWVER -i$TMPECC --debug --erase 5 >>$LOGFILE 2>&1
   $NEWVER -i$TMPECC --debug --erase 77-86 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest read_missing_crc_sectors "--spinup-delay=0 -r " $TMPISO $TMPECC
fi

# Image contains 1 row of missing sectors and a single one
# in the ecc portion

if try "reading image with missing ecc sectors" read_missing_ecc_sectors; then
   cp $MASTERISO $SIMISO
   cp $MASTERECC $TMPECC
   $NEWVER -i$TMPECC --debug --erase 120 >>$LOGFILE 2>&1
   $NEWVER -i$TMPECC --debug --erase 134-190 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest read_missing_ecc_sectors "--spinup-delay=0 -r " $TMPISO $TMPECC
fi

# Image contains bad byte in the data section

if try "reading image with bad data byte" read_data_bad_byte; then
   cp $MASTERISO $SIMISO 
   $NEWVER -i$SIMISO --debug --byteset 0,50,10 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --byteset 1235,50,10 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --byteset 20999,50,10 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest read_data_bad_byte "--spinup-delay=0 -r " $TMPISO $MASTERECC
fi

# Image contains bad byte in the crc section

if try "reading image with bad crc byte" read_crc_bad_byte; then
   cp $MASTERISO $SIMISO
   cp $MASTERECC $TMPECC
   $NEWVER -i$TMPECC --debug --byteset 77,50,10 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest read_crc_bad_byte "--spinup-delay=0 -r " $TMPISO $TMPECC
fi

# Image contains bad byte in the ecc section

if try "reading image with bad ecc byte" read_ecc_bad_byte; then
   cp $MASTERISO $SIMISO
   cp $MASTERECC $TMPECC
   $NEWVER -i$TMPECC --debug --byteset 200,50,10 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest read_ecc_bad_byte "--spinup-delay=0 -r " $TMPISO $TMPECC
fi

# Ecc file contains several uncorrectable dead sector markers within the CRC area.
# There is a specical code segment in RS03GetCrcBuf() for testing this.
# (test case in other places: DSM in image, medium)
# Note: When judging the DSM, the fingerprint from the image is taken.
# Since this does not match the fingerprint used when erasing the sector in the eccfile,
# we are automatically triggering an uncorrectable dead sector marker.

if try "crc section with uncorrectable dead sector markers" read_crc_section_with_uncorrectable_dsm; then

  cp $MASTERISO $SIMISO
  cp $MASTERECC $TMPECC
  $NEWVER --debug -i$TMPECC --erase 10 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --erase 15 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPECC --erase 20 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_crc_section_with_uncorrectable_dsm  "--spinup-delay=0 -r " $TMPISO $TMPECC
fi

### Reading tests (adaptive)

echo "# Reading tests (adaptive)"
