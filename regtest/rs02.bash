#!/usr/bin/env bash

. common.bash

ISOSIZE=30000
ECCSIZE=35000
REAL_ECCSIZE=34932

MASTERISO=$ISODIR/rs02-master.iso
TMPISO=$ISODIR/rs02-tmp.iso
SIMISO=$ISODIR/rs02-sim.iso
TMPECC=$ISODIR/rs02-tmp.ecc  # rs02 augmented image wrapped by ecc file

ISO_PLUS137=$ISODIR/rs02-plus137.iso

CODEC_PREFIX=RS02

# Create master image

if ! file_exists $MASTERISO; then
    $NEWVER --debug -i$MASTERISO --random-image $ISOSIZE >>$LOGFILE 2>&1
    echo "$NEWVER --debug --set-version $SETVERSION -i$MASTERISO -mRS02 -n$ECCSIZE -c" >>$LOGFILE
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -mRS02 -n$ECCSIZE -c >>$LOGFILE 2>&1
    echo -e "$FILE_MSG"
    FILE_MSG=""
fi

# Create master image with 137 trailing bytes

if ! file_exists $ISO_PLUS137; then
    echo "$NEWVER --debug -i$ISO_PLUS137 --random-image $ISOSIZE" >>$LOGFILE
    $NEWVER --debug -i$ISO_PLUS137 --random-image $ISOSIZE >>$LOGFILE 2>&1
    echo "dd if="$RNDSEQ" count=1 bs=137 >>$ISO_PLUS137"  >>$LOGFILE
    dd if="$RNDSEQ" count=1 bs=137 >>$ISO_PLUS137 2>/dev/null
    echo "$NEWVER --debug --set-version $SETVERSION -i$ISO_PLUS137 -mRS02 -n$ECCSIZE -c" >>$LOGFILE
    $NEWVER --debug --set-version $SETVERSION -i$ISO_PLUS137 -mRS02 -n$ECCSIZE -c >>$LOGFILE 2>&1

    echo -e "$FILE_MSG"
    FILE_MSG=""
fi

### Verification tests

echo "# Verify tests"

# Test good files

if try "good image" good; then
  run_regtest good "-t -v" $MASTERISO
fi

# Test good files, quick mode

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

# Image contains one header w/o cookie

if try "image with defective header" bad_header; then
   cp $MASTERISO $TMPISO
   $NEWVER -i$TMPISO --debug --byteset 30592,1,1 >>$LOGFILE 2>&1

   run_regtest bad_header "-t" $TMPISO
fi

# Image contains one header with CRC errors and one w/o cookie

if try "image with defective headers" bad_headers; then
   cp $MASTERISO $TMPISO
   $NEWVER -i$TMPISO --debug --byteset 30592,1,1 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --byteset 31488,100,1 >>$LOGFILE 2>&1

  run_regtest bad_headers "-t" $TMPISO
fi

# Image contains 3 missing headers and two with CRC errors

if try "image with defective headers" missing_headers; then
   cp $MASTERISO $TMPISO
   $NEWVER -i$TMPISO --debug --erase 30080 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --erase 31360 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --erase 34816 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --byteset 30592,100,1 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --byteset 31488,100,1 >>$LOGFILE 2>&1

  run_regtest missing_headers "-t" $TMPISO
fi

# Image contains 2 rows of missing sectors and a single one
# in the data portion

if try "image with missing data sectors" missing_data_sectors; then
   cp $MASTERISO $TMPISO
   $NEWVER -i$TMPISO --debug --erase 1000-1049 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --erase 21230 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --erase 22450-22457 >>$LOGFILE 2>&1

  run_regtest missing_data_sectors "-t" $TMPISO
fi

# Image contains 1 row of missing sectors and a single one
# in the crc portion

if try "image with missing crc sectors" missing_crc_sectors; then
   cp $MASTERISO $TMPISO
   $NEWVER -i$TMPISO --debug --erase 30020-30030 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --erase 30034 >>$LOGFILE 2>&1

  run_regtest missing_crc_sectors "-t" $TMPISO
fi

# Image contains 1 row of missing sectors and a single one
# in the ecc portion

if try "image with missing ecc sectors" missing_ecc_sectors; then
   cp $MASTERISO $TMPISO
   $NEWVER -i$TMPISO --debug --erase 32020-32030 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --erase 33034 >>$LOGFILE 2>&1

  run_regtest missing_ecc_sectors "-t" $TMPISO
fi

# Image contains bad byte in the data section

if try "image with bad data byte" data_bad_byte; then
   cp $MASTERISO $TMPISO 
   $NEWVER -i$TMPISO --debug --byteset 1235,50,10 >>$LOGFILE 2>&1

  run_regtest data_bad_byte "-t" $TMPISO
fi

# Image contains bad byte in the CRC section

if try "image with bad crc byte" crc_bad_byte; then
   cp $MASTERISO $TMPISO 
   $NEWVER -i$TMPISO --debug --byteset 30020,50,10 >>$LOGFILE 2>&1

  run_regtest crc_bad_byte "-t" $TMPISO
fi

# Image contains bad byte in the ecc section

if try "image with bad ecc byte" ecc_bad_byte; then
   cp $MASTERISO $TMPISO 
   $NEWVER -i$TMPISO --debug --byteset 33100,50,10 >>$LOGFILE 2>&1

  run_regtest ecc_bad_byte "-t" $TMPISO
fi

# Image is good with ECC header following directly after the user data

if try "good image, no ECC offset" good_0_offset; then
  run_regtest good_0_offset "-v -t" $MASTERISO
fi

# Image is good with ECC header following after the user data with 150 sectors padding

if try "good image, 150 sectors ECC offset" good_150_offset; then
   $NEWVER --debug -i$TMPISO --random-image $ISOSIZE >>$LOGFILE 2>&1
#   dd if=/dev/zero bs=2048 count=150 >>$TMPISO 2>/dev/null
   $NEWVER --debug -i$TMPISO --byteset 16,80,198 >>$LOGFILE 2>&1  # fake 150 more sectors in vss
   $NEWVER --debug -i$TMPISO --byteset 16,87,198 >>$LOGFILE 2>&1
   $NEWVER --debug --set-version $SETVERSION -i$TMPISO -mRS02 -n$ECCSIZE -c >>$LOGFILE 2>&1

   run_regtest good_150_offset "-v -t" $TMPISO
fi

# Image misses the first header.

if try "master header missing" bad_master; then
    cp $MASTERISO $TMPISO 
    $NEWVER --debug -i$TMPISO --erase 30000 >>$LOGFILE 2>&1

    run_regtest bad_master "-v -t" $TMPISO
fi

# Test the pre-0.79.5 header modulo glitch correction
# with available ecc size information (header created by 0.79.5 or later)

if try "header modulo glitch, post 0.79.5 style hdr" modulo_glitch; then
   HMGISO=$ISODIR/rs02-hmg-master.iso

   if ! test -f $HMGISO; then
     $NEWVER --debug -i $HMGISO --random-image 274300  >>$LOGFILE 2>&1
     $NEWVER --debug --set-version $SETVERSION -i$HMGISO -mRS02 -c >>$LOGFILE 2>&1
   fi

   run_regtest modulo_glitch "-v -t" $HMGISO
fi

# Test the pre-0.79.5 header modulo glitch correction
# with no ecc size information (header created by 0.72 or earlier versions)

if try "header modulo glitch, old style, complete img" modulo_glitch2; then
   HMGISO=$ISODIR/rs02-hmg-master.iso

   if ! test -f $HMGISO; then
     $NEWVER --debug -i $HMGISO --random-image 274300  >>$LOGFILE 2>&1
     $NEWVER --debug --set-version $SETVERSION -i$HMGISO -mRS02 -c >>$LOGFILE 2>&1
   fi

   cp $HMGISO $TMPISO  # create the old style image
   for header in 274300 278528 282624 286720 290816 294912 299008 303104 307200 311296 315392 319488 323584 327680 331776 335872 339968 344064 348160 352256 356352; do
       $NEWVER --debug -i $TMPISO --byteset $header,96,38  >>$LOGFILE 2>&1 # self sum
       $NEWVER --debug -i $TMPISO --byteset $header,97,245 >>$LOGFILE 2>&1
       $NEWVER --debug -i $TMPISO --byteset $header,98,168 >>$LOGFILE 2>&1
       $NEWVER --debug -i $TMPISO --byteset $header,99,221 >>$LOGFILE 2>&1

       $NEWVER --debug -i $TMPISO --byteset $header,128,0 >>$LOGFILE 2>&1 # size info
       $NEWVER --debug -i $TMPISO --byteset $header,129,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $TMPISO --byteset $header,130,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $TMPISO --byteset $header,131,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $TMPISO --byteset $header,132,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $TMPISO --byteset $header,133,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $TMPISO --byteset $header,134,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $TMPISO --byteset $header,135,0 >>$LOGFILE 2>&1
   done

   run_regtest modulo_glitch2 "-v -t" $TMPISO
fi

# Test the pre-0.79.5 header modulo glitch correction
# with no ecc size information (header created by 0.72 or earlier versions)
# truncated image so that the simple size comparison heuristics will fail
# and ecc header search is performed.

if try "header modulo glitch, old style, truncated img" modulo_glitch3; then
   HMGISO=$ISODIR/rs02-hmg-master.iso

   if ! test -f $HMGISO; then
     $NEWVER --debug -i $HMGISO --random-image 274300  >>$LOGFILE 2>&1
     $NEWVER --debug --set-version $SETVERSION -i$HMGISO -mRS02 -c >>$LOGFILE 2>&1
   fi

   cp $HMGISO $TMPISO  # create the old style image
   for header in 274300 278528 282624 286720 290816 294912 299008 303104 307200 311296 315392 319488 323584 327680 331776 335872 339968 344064 348160 352256 356352; do
       $NEWVER --debug -i $TMPISO --byteset $header,96,38  >>$LOGFILE 2>&1 # self sum
       $NEWVER --debug -i $TMPISO --byteset $header,97,245 >>$LOGFILE 2>&1
       $NEWVER --debug -i $TMPISO --byteset $header,98,168 >>$LOGFILE 2>&1
       $NEWVER --debug -i $TMPISO --byteset $header,99,221 >>$LOGFILE 2>&1

       $NEWVER --debug -i $TMPISO --byteset $header,128,0 >>$LOGFILE 2>&1 # size info
       $NEWVER --debug -i $TMPISO --byteset $header,129,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $TMPISO --byteset $header,130,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $TMPISO --byteset $header,131,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $TMPISO --byteset $header,132,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $TMPISO --byteset $header,133,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $TMPISO --byteset $header,134,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $TMPISO --byteset $header,135,0 >>$LOGFILE 2>&1
   done
   $NEWVER --debug -i $TMPISO --truncate=357520  >>$LOGFILE 2>&1

   run_regtest modulo_glitch3 "-v -t" $TMPISO
fi

# Test the pre-0.79.5 header modulo glitch correction
# with no ecc size information (header created by 0.72 or earlier versions)
# truncated image so that the simple size comparison heuristics will fail
# and ecc header search is performed.

if try "header modulo glitch, old style, truncated img, missing ref secs" modulo_glitch4; then
   HMGISO=$ISODIR/rs02-hmg-master.iso

   if ! test -f $HMGISO; then
     $NEWVER --debug -i $HMGISO --random-image 274300  >>$LOGFILE 2>&1
     $NEWVER --debug --set-version $SETVERSION -i$HMGISO -mRS02 -c >>$LOGFILE 2>&1
   fi

   cp $HMGISO $TMPISO  # create the old style image
   for header in 274300 278528 282624 286720 290816 294912 299008 303104 307200 311296 315392 319488 323584 327680 331776 335872 339968 344064 348160 352256 356352; do
       $NEWVER --debug -i $TMPISO --byteset $header,96,38  >>$LOGFILE 2>&1 # self sum
       $NEWVER --debug -i $TMPISO --byteset $header,97,245 >>$LOGFILE 2>&1
       $NEWVER --debug -i $TMPISO --byteset $header,98,168 >>$LOGFILE 2>&1
       $NEWVER --debug -i $TMPISO --byteset $header,99,221 >>$LOGFILE 2>&1

       $NEWVER --debug -i $TMPISO --byteset $header,128,0 >>$LOGFILE 2>&1 # size info
       $NEWVER --debug -i $TMPISO --byteset $header,129,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $TMPISO --byteset $header,130,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $TMPISO --byteset $header,131,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $TMPISO --byteset $header,132,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $TMPISO --byteset $header,133,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $TMPISO --byteset $header,134,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $TMPISO --byteset $header,135,0 >>$LOGFILE 2>&1
   done
   $NEWVER --debug -i $TMPISO --truncate=357520  >>$LOGFILE 2>&1

   # remove some sectors which could disambiguate the layouts
   # 354304 is the last remaining sector.
   for sector in 276480 280577 284672 288768 292864 296960 301056 305152 309248 313344 317440 321536 325632 329728 333824 337920 342016 346112 350208; do
       $NEWVER --debug -i $TMPISO --erase $sector  >>$LOGFILE 2>&1
   done

   run_regtest modulo_glitch4 "-v -t" $TMPISO
fi

# Augmented image is protected by an outer RS01 error correction file

if try "with RS01 error correction file" with_rs01_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -c -n normal >>$LOGFILE 2>&1

    run_regtest with_rs01_file "-v -t" $MASTERISO $TMPECC
fi

# Augmented image and non-matching RS01 error correction file
# Expected behaviour is to report the non-matching ecc file
# rather than falling back to using the RS02 part since the
# user did probably have some intentention specifying the ecc file.

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
# (sector displacement)

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

# Test image containing several uncorrectable dead sector markers, verbose output

if try "image with uncorrectable dead sector markers, verbose output" uncorrectable_dsm_in_image_verbose; then

  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase 3030 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 3030,353,49 >>$LOGFILE 2>&1 // displaced from sector 3130
  $NEWVER --debug -i$TMPISO --erase 4400 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 4400,353,53 >>$LOGFILE 2>&1 // displaced from sector 4500
  $NEWVER --debug -i$TMPISO --erase 4411 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 4411,353,53 >>$LOGFILE 2>&1 // displaced from sector 4511

  run_regtest uncorrectable_dsm_in_image_verbose  "-t -v" $TMPISO
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

  run_regtest uncorrectable_dsm_in_image2 "-t" $TMPISO
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

  run_regtest uncorrectable_dsm_in_image2_verbose "-t -v" $TMPISO
fi

# Testimage containing several uncorrectable dead sector markers in the CRC section
# (non matching fingerprint)
# In rs02-verify.c there is extra code for handling this.

if try "image with uncorrectable dead sector markers (3)" uncorrectable_dsm_in_image3; then

  cp $MASTERISO $TMPISO
  $NEWVER --debug -i$TMPISO --erase 30030 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 30030,416,55 >>$LOGFILE 2>&1 // wrong fingerprint
  $NEWVER --debug -i$TMPISO --byteset 30030,556,32 >>$LOGFILE 2>&1 // changed label
  $NEWVER --debug -i$TMPISO --byteset 30030,557,50 >>$LOGFILE 2>&1 // changed label
  $NEWVER --debug -i$TMPISO --erase 30031 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 30031,416,53 >>$LOGFILE 2>&1 // wrong fingerprint
  $NEWVER --debug -i$TMPISO --byteset 30031,556,32 >>$LOGFILE 2>&1 // changed label
  $NEWVER --debug -i$TMPISO --byteset 30031,557,50 >>$LOGFILE 2>&1 // changed label
  $NEWVER --debug -i$TMPISO --erase 30032 >>$LOGFILE 2>&1
  $NEWVER --debug -i$TMPISO --byteset 30032,416,53 >>$LOGFILE 2>&1 // wrong fingerprint
  $NEWVER --debug -i$TMPISO --byteset 30032,556,32 >>$LOGFILE 2>&1 // changed label
  $NEWVER --debug -i$TMPISO --byteset 30032,557,50 >>$LOGFILE 2>&1 // changed label

  run_regtest uncorrectable_dsm_in_image3 "-t" $TMPISO
fi


### Creation tests

echo "# Creation tests"

# Create test image

if try "augmented image creation" ecc_create; then
  $NEWVER --debug -i$TMPISO --random-image $ISOSIZE >>$LOGFILE 2>&1

  replace_config method-name RS02
  replace_config medium-size 35000
  extra_args="--debug --set-version $SETVERSION"
  run_regtest ecc_create "-mRS02 -n$ECCSIZE -c" $TMPISO
fi

# Create with missing image

if try "ecc creating with missing image" ecc_missing_image; then
  NO_FILE=$ISODIR/none.iso

  replace_config method-name RS02
  replace_config medium-size 35000
  extra_args="--debug --set-version $SETVERSION"
  run_regtest ecc_missing_image "-mRS02 -n$ECCSIZE -c" $NO_FILE
fi

# Create with no read permission on image

if try "ecc creating with no read permission" ecc_no_read_perm; then
   $NEWVER --debug -i$TMPISO --random-image $ISOSIZE >>$LOGFILE 2>&1
   chmod 000 $TMPISO

  replace_config method-name RS02
  replace_config medium-size 35000
  extra_args="--debug --set-version $SETVERSION"
  run_regtest ecc_no_read_perm "-mRS02 -n$ECCSIZE -c" $TMPISO
  rm -f $TMPISO
fi

# Create with no write permission on image

if try "ecc creating with no write permission" ecc_no_write_perm; then
   $NEWVER --debug -i$TMPISO --random-image $ISOSIZE >>$LOGFILE 2>&1
   chmod 400 $TMPISO

  replace_config method-name RS02
  replace_config medium-size 35000
  extra_args="--debug --set-version $SETVERSION"
  run_regtest ecc_no_write_perm "-mRS02 -n$ECCSIZE -c" $TMPISO
  rm -f $TMPISO
fi

# Create with already RS02-augmented image 

if try "ecc creating from RS02-augmented image" ecc_from_rs02; then
   cp $MASTERISO $TMPISO

  replace_config method-name RS02
  replace_config medium-size 35000
  extra_args="--debug --set-version $SETVERSION"
  run_regtest ecc_from_rs02 "-mRS02 -n$ECCSIZE -c" $TMPISO
fi

# Create with already RS03-augmented image 

if try "ecc creating from RS03-augmented image" ecc_from_rs03; then
   $NEWVER --debug -i$TMPISO --random-image $ISOSIZE >>$LOGFILE 2>&1
   $NEWVER --debug --set-version $SETVERSION -i$TMPISO -mRS03 -n$ECCSIZE -c >>$LOGFILE 2>&1

   replace_config method-name RS02
   replace_config medium-size 35000
   extra_args="--debug --set-version $SETVERSION"
   run_regtest ecc_from_rs03 "-mRS02 -n$ECCSIZE -c" $TMPISO
fi

# Create with already RS02-augmented image having a larger redundancy 

if try "ecc creating from RS02-augmented image w/ higher red." ecc_from_larger_rs02; then
   $NEWVER --debug -i$TMPISO --random-image $ISOSIZE >>$LOGFILE 2>&1
   $NEWVER --debug --set-version $SETVERSION -i$TMPISO -mRS02 -n$((ECCSIZE+5000)) -c >>$LOGFILE 2>&1

  replace_config method-name RS02
  replace_config medium-size 35000
  extra_args="--debug --set-version $SETVERSION"
  run_regtest ecc_from_larger_rs02 "-mRS02 -n$ECCSIZE -c" $TMPISO
fi

# Create with already RS02-augmented image of a non-2048 multiple size

if try "ecc creating from RS02-augmented image w/ non-block size" ecc_from_rs02_non_blocksize; then
   $NEWVER --debug -i$TMPISO --random-image $ISOSIZE >>$LOGFILE 2>&1
   for i in $(seq 56); do echo -n "1" >>$TMPISO; done
   $NEWVER --debug --set-version $SETVERSION -i$TMPISO -mRS02 -n$ECCSIZE -c >>$LOGFILE 2>&1

  replace_config method-name RS02
  replace_config medium-size 35000
  replace_config examine-rs02 1
  extra_args="--debug --set-version $SETVERSION"
  run_regtest ecc_from_rs02_non_blocksize "-mRS02 -n$ECCSIZE -c" $TMPISO
fi

# Create with already RS03-augmented image of a non-2048 multiple size

if try "ecc creating from RS03-augmented image w/ non-block size" ecc_from_rs03_non_blocksize; then
   $NEWVER --debug -i$TMPISO --random-image $ISOSIZE >/dev/null 2>&1
   dd if="$RNDSEQ" count=1 bs=137 >>$TMPISO 2>/dev/null
   $NEWVER --debug --set-version $SETVERSION -i$TMPISO -mRS03 -n$ECCSIZE -c >/dev/null 2>&1
   
   extra_args="--debug --set-version $SETVERSION"
   run_regtest ecc_from_rs03_non_blocksize "-mRS02 -n$ECCSIZE -c -a RS02" $TMPISO
fi

# Create with already RS02-augmented image of a non-2048 multiple size, larger redundancy.

if try "ecc creating from RS02-augmented image w/ non-block size, larger red." ecc_from_larger_rs02_non_blocksize; then
   $NEWVER --debug -i$TMPISO --random-image $ISOSIZE >>$LOGFILE 2>&1
   for i in $(seq 56); do echo -n "1" >>$TMPISO; done
   $NEWVER --debug --set-version $SETVERSION -i$TMPISO -mRS02 -n$((ECCSIZE+5000)) -c >>$LOGFILE 2>&1

  replace_config method-name RS02
  replace_config medium-size 35000
  replace_config examine-rs02 1
  extra_args="--debug --set-version $SETVERSION"
  run_regtest ecc_from_larger_rs02_non_blocksize "-mRS02 -n$ECCSIZE -c" $TMPISO
fi

# Create with size not being a multiple of 2048

if try "ecc creating from non-blocksize image" ecc_non_blocksize; then
   $NEWVER --debug -i$TMPISO --random-image $ISOSIZE >>$LOGFILE 2>&1
   dd if=/dev/zero count=1 bs=137 >>$TMPISO 2>/dev/null

  replace_config method-name RS02
  replace_config medium-size 35000
  extra_args="--debug --set-version $SETVERSION"
  run_regtest ecc_non_blocksize "-mRS02 -n$ECCSIZE -c" $TMPISO
fi

# Create test image with unreadable sectors 

if try "ecc creating with unreadable sectors" ecc_missing_sectors; then
   $NEWVER --debug -i$TMPISO --random-image $ISOSIZE >>$LOGFILE 2>&1
   $NEWVER --debug -i$TMPISO --erase 719 >>$LOGFILE 2>&1

  replace_config method-name RS02
  replace_config medium-size 35000
  extra_args="--debug --set-version $SETVERSION"
  run_regtest ecc_missing_sectors "-mRS02 -n$ECCSIZE -c" $TMPISO
fi

# Read image and augment with RS02 in one pass.
# Make sure that checksums are handed over correctly between reading
# and error correction creation.
# NOTE: cache handling is currently disabled and will be fixed in 0.79.6!

if try "ecc creating after reading image" ecc_create_after_read; then
   $NEWVER --debug -i$SIMISO --random-image $ISOSIZE >>$LOGFILE 2>&1

   rm -f $TMPISO
   replace_config method-name RS02
   replace_config medium-size 35000
   replace_config read-and-create 1
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values --set-version $SETVERSION"
   run_regtest ecc_create_after_read "-r --spinup-delay=0 -mRS02 -n$ECCSIZE -c" $TMPISO
fi

# Complete image and augment with RS02 in one pass.
# In that case cached checksums can not be used.
# NOTE: cache handling is currently disabled and will be fixed in 0.79.6!

if try "ecc creating after completing image" ecc_create_after_partial_read; then
   $NEWVER --debug -i$SIMISO --random-image $ISOSIZE >>$LOGFILE 2>&1

   cp $SIMISO $TMPISO
   $NEWVER --debug -i$TMPISO --erase 3000-3999 >>$LOGFILE 2>&1
   
   replace_config method-name RS02
   replace_config medium-size 35000
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values --set-version $SETVERSION"
   run_regtest ecc_create_after_partial_read "-r --spinup-delay=0 -mRS02 -n$ECCSIZE -c" $TMPISO
fi

# Read image with ecc file and create new (other) ecc in the same program call.
# Tests whether CRC and ECC information is handed over correctly.
# NOTE: cache handling is currently disabled and will be fixed in 0.79.6!

if try "read image with ecc (RS01) and create new ecc" ecc_recreate_after_read_rs01; then
   $NEWVER --debug -i$SIMISO --random-image $ISOSIZE >>$LOGFILE 2>&1
   $NEWVER --debug --set-version $SETVERSION -i$SIMISO -e$TMPECC -c $REDUNDANCY >>$LOGFILE 2>&1

   rm -f $TMPISO
   replace_config method-name RS02
   replace_config medium-size 35000
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values --set-version $SETVERSION"
   run_regtest ecc_recreate_after_read_rs01 "-r --spinup-delay=0 -mRS02 -n$ECCSIZE -c" $TMPISO $TMPECC
fi

# Read image with ecc file and create new (other) ecc in the same program call.
# Tests whether CRC and ECC information is handed over correctly.
# NOTE: cache handling is currently disabled and will be fixed in 0.79.6!

if try "read image with ecc (RS02) and create new ecc" ecc_recreate_after_read_rs02; then
   $NEWVER --debug -i$SIMISO --random-image $ISOSIZE >>$LOGFILE 2>&1
   $NEWVER --debug --set-version $SETVERSION -i$SIMISO -mRS02 -n50000 -c >>$LOGFILE 2>&1

   rm -f $TMPISO
   replace_config method-name RS02
   replace_config medium-size 35000
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values --set-version $SETVERSION"
   run_regtest ecc_recreate_after_read_rs02 "-r --spinup-delay=0 -mRS02 -n$ECCSIZE -c" $TMPISO
fi

# Read image with ecc file and create new (other) ecc in the same program call.
# Tests whether CRC and ECC information is handed over correctly.
# NOTE: cache handling is currently disabled and will be fixed in 0.79.6!

if try "read image with ecc (RS03i) and create new ecc" ecc_recreate_after_read_rs03i; then
   $NEWVER --debug -i$SIMISO --random-image $ISOSIZE >>$LOGFILE 2>&1
   $NEWVER --debug --set-version $SETVERSION -i$SIMISO -mRS03 -n$((ISOSIZE+7000)) -c >>$LOGFILE 2>&1

   rm -f $TMPISO
   replace_config method-name RS02
   replace_config medium-size 35000
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values --set-version $SETVERSION"
   run_regtest ecc_recreate_after_read_rs03i "-r --spinup-delay=0 -mRS02 -n$ECCSIZE -c" $TMPISO
fi

# Read image with ecc file and create new (other) ecc in the same program call.
# Tests whether CRC and ECC information is handed over correctly.
# NOTE: cache handling is currently disabled and will be fixed in 0.79.6!

if try "read image with ecc (RS03f) and create new ecc" ecc_recreate_after_read_rs03f; then
   $NEWVER --debug -i$SIMISO --random-image $ISOSIZE >>$LOGFILE 2>&1
   $NEWVER --debug --set-version $SETVERSION -i$SIMISO -e$TMPECC -c -n 9 -mRS03 -o file >>$LOGFILE 2>&1

   rm -f $TMPISO
   replace_config method-name RS02
   replace_config medium-size 35000
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values --set-version $SETVERSION"
   run_regtest ecc_recreate_after_read_rs03f "-r --spinup-delay=0 -mRS02 -n$ECCSIZE -c" $TMPISO $TMPECC
fi

### Fixing tests

echo "# Fixing tests"

# Fix with no read permission on image

if try "trying fix with no read permission" fix_no_read_perm; then
  $NEWVER --debug -i$TMPISO --random-image $ISOSIZE >>$LOGFILE 2>&1
  chmod 000 $TMPISO

  run_regtest fix_no_read_perm "--debug --set-version $SETVERSION -f" $TMPISO
  rm -f $TMPISO
fi

# Fix with no write permission on image

if try "trying fix with no write permission" fix_no_write_perm; then
  $NEWVER --debug -i$TMPISO --random-image $ISOSIZE >>$LOGFILE 2>&1
  chmod 400 $TMPISO

  run_regtest fix_no_write_perm "--debug --set-version $SETVERSION -f" $TMPISO
  rm -f $TMPISO
fi

# Fix already good image

if try "trying fix with good image" fix_good_image; then
  cp $MASTERISO $TMPISO

  run_regtest fix_good_image "--debug --set-version $SETVERSION -f" $TMPISO
fi

# Fix image containing 137 extra bytes

if try "trying to fix image with 137 extra bytes" fix_image_plus137; then

  cp $ISO_PLUS137 $TMPISO
  $NEWVER -i$TMPISO --debug --erase 17000 >>$LOGFILE 2>&1
  
  run_regtest fix_image_plus137 "-f" $TMPISO
fi

# Fix image containing one error in the 137 extra bytes and another
# error in the zero-padded area following the 137 bytes.
# Both shall be corrected.

if try "trying to fix image with error in 137 extra bytes" fix_image_error_in_plus137; then

  cp $ISO_PLUS137 $TMPISO
  $NEWVER -i$TMPISO --debug --byteset 30000,111,111 >>$LOGFILE 2>&1
  $NEWVER -i$TMPISO --debug --byteset 30000,500,123 >>$LOGFILE 2>&1
  
  run_regtest fix_image_error_in_plus137 "-f" $TMPISO
fi

# Fix a truncated image

if try "trying fix with truncated image" fix_truncated_image; then
   TRUNC_SIZE=$((REAL_ECCSIZE-210));
   cp $MASTERISO $TMPISO
   $NEWVER --debug -i$TMPISO --truncate=$TRUNC_SIZE >>$LOGFILE 2>&1

   run_regtest fix_truncated_image "--debug --set-version $SETVERSION -f" $TMPISO
fi

# Fix an image with a few trailing bytes

if try "trying fix with trailing bytes" fix_trailing_bytes; then
   cp $MASTERISO $TMPISO
   echo "some trailing garbage appended for testing" >>$TMPISO

   run_regtest fix_trailing_bytes "--debug --set-version $SETVERSION -f" $TMPISO
fi

# Fix an image with trailing garbage (TAO case)

if try "trying fix with trailing garbage (TAO case)" fix_trailing_tao; then
   cp $MASTERISO $TMPISO
   dd if=/dev/zero count=2 bs=2048 >>$TMPISO 2>/dev/null

   run_regtest fix_trailing_tao "--debug --set-version $SETVERSION -f" $TMPISO
fi

# Fix an image with trailing garbage (general case), without doing anything

if try "trying fix with trailing garbage (general case)" fix_trailing_garbage; then
   cp $MASTERISO $TMPISO
   dd if=/dev/zero count=23 bs=2048 >>$TMPISO 2>/dev/null

   run_regtest fix_trailing_garbage "--debug --set-version $SETVERSION -f" $TMPISO
fi

# Fix an image with trailing garbage (general case), with --truncate

if try "trying fix with trailing garbage with --truncate" fix_trailing_garbage2; then
   cp $MASTERISO $TMPISO
   dd if=/dev/zero count=23 bs=2048 >>$TMPISO 2>/dev/null

   run_regtest fix_trailing_garbage2 "--debug --set-version $SETVERSION -f --truncate" $TMPISO
fi

# Image contains bad master header

if try "trying fix with missing master header" fix_bad_master; then
   cp $MASTERISO $TMPISO
   $NEWVER -i$TMPISO --debug --erase 30000 >>$LOGFILE 2>&1

   run_regtest fix_bad_master "--debug --set-version $SETVERSION -f" $TMPISO
fi

# Image contains one header w/o cookie

if try "image with defective header" fix_bad_header; then
   cp $MASTERISO $TMPISO
   $NEWVER -i$TMPISO --debug --byteset 30592,1,1 >>$LOGFILE 2>&1

   run_regtest fix_bad_header "--debug --set-version $SETVERSION -f" $TMPISO
fi

# Image contains one header with CRC errors and one w/o cookie

if try "image with defective headers" fix_bad_headers; then
   cp $MASTERISO $TMPISO
   $NEWVER -i$TMPISO --debug --byteset 30592,1,1 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --byteset 31488,100,1 >>$LOGFILE 2>&1

   run_regtest fix_bad_headers "--debug --set-version $SETVERSION -f" $TMPISO
fi

# Image contains 3 missing headers and two with CRC errors

if try "image with defective headers" fix_missing_headers; then
   cp $MASTERISO $TMPISO
   $NEWVER -i$TMPISO --debug --erase 30080 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --erase 31360 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --erase 34816 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --byteset 30592,100,1 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --byteset 31488,100,1 >>$LOGFILE 2>&1

  run_regtest fix_missing_headers "--debug --set-version $SETVERSION -f" $TMPISO
fi

# Image contains 2 rows of missing sectors and a single one
# in the data portion

if try "image with missing data sectors" fix_missing_data_sectors; then
   cp $MASTERISO $TMPISO
   $NEWVER -i$TMPISO --debug --erase 1000-1049 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --erase 21230 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --erase 22450-22457 >>$LOGFILE 2>&1

  run_regtest fix_missing_data_sectors "--debug --set-version $SETVERSION -f" $TMPISO
fi

# Image contains 1 row of missing sectors and a single one
# in the crc portion

if try "image with missing crc sectors" fix_missing_crc_sectors; then
   cp $MASTERISO $TMPISO
   $NEWVER -i$TMPISO --debug --erase 30020-30030 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --erase 30034 >>$LOGFILE 2>&1

   run_regtest fix_missing_crc_sectors "--debug --set-version $SETVERSION -f" $TMPISO
fi

# Image contains 1 row of missing sectors and a single one
# in the ecc portion

if try "image with missing ecc sectors" fix_missing_ecc_sectors; then
   cp $MASTERISO $TMPISO
   $NEWVER -i$TMPISO --debug --erase 32020-32030 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --erase 33034 >>$LOGFILE 2>&1

  run_regtest fix_missing_ecc_sectors "--debug --set-version $SETVERSION -f" $TMPISO
fi

# Image is large and contains errors in all three sections.
# We need this test to check whether we correction works
# when multiple sectors are cached; the default test file
# is too small for such tests.

if try "large image with missing sectors" fix_large_file; then
   $NEWVER --debug -i$TMPISO --random-image 223456 >>$LOGFILE 2>&1
   $NEWVER --debug --set-version $SETVERSION -i$TMPISO -mRS02 -c >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --erase 50000-50015 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --erase 223600-223605 >>$LOGFILE 2>&1
   $NEWVER -i$TMPISO --debug --erase 279000-279007 >>$LOGFILE 2>&1

  run_regtest fix_large_file "-f" $TMPISO
fi

# Image contains bad byte in the data section

if try "image with bad data byte" fix_data_bad_byte; then
   cp $MASTERISO $TMPISO 
   $NEWVER -i$TMPISO --debug --byteset 1235,50,10 >>$LOGFILE 2>&1

  run_regtest fix_data_bad_byte "--debug --set-version $SETVERSION -f" $TMPISO
fi

# Image contains bad byte in the CRC section

if try "image with bad crc byte" fix_crc_bad_byte; then
   cp $MASTERISO $TMPISO 
   $NEWVER -i$TMPISO --debug --byteset 30020,50,10 >>$LOGFILE 2>&1

  run_regtest fix_crc_bad_byte "--debug --set-version $SETVERSION -f" $TMPISO
fi

# Image contains bad byte in the ecc section

if try "image with bad ecc byte" fix_ecc_bad_byte; then
   cp $MASTERISO $TMPISO 
   $NEWVER -i$TMPISO --debug --byteset 33100,50,10 >>$LOGFILE 2>&1

  run_regtest fix_ecc_bad_byte "--debug --set-version $SETVERSION -f" $TMPISO
fi

# Image is good with ECC header following directly after the user data

if try "good image, no ECC offset" fix_good_0_offset; then
   cp $MASTERISO $TMPISO 

   run_regtest fix_good_0_offset "--debug --set-version $SETVERSION -v -f" $TMPISO
fi

# Image is good with ECC header following after the user data with 150 sectors padding
# Setting the VSS size 150 sectors beyond the actual image size is sufficient since
# the codes doesn not really care about the contents of the padding area.

if try "good image, 150 sectors ECC offset" fix_good_150_offset; then
   $NEWVER --debug -i$TMPISO --random-image $ISOSIZE >>$LOGFILE 2>&1
   $NEWVER --debug -i$TMPISO --byteset 16,80,198 >>$LOGFILE 2>&1  # fake 150 more sectors in vss
   $NEWVER --debug -i$TMPISO --byteset 16,87,198 >>$LOGFILE 2>&1
   $NEWVER --debug --set-version $SETVERSION -i$TMPISO -mRS02 -n$ECCSIZE -c >>$LOGFILE 2>&1

   run_regtest fix_good_150_offset "--debug --set-version $SETVERSION -v -f" $TMPISO
fi

# Augmented image is protected by an outer RS01 error correction file
# Setting the byte creates a CRC error which can only be detected by
# the outer RS01 code, so we use this as an additional probe that the
# correct (outer) ECC is applied.

if try "RS02 image with RS01 ecc file" fix_with_rs01_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -c -n normal >>$LOGFILE 2>&1
    cp $MASTERISO $TMPISO
    $NEWVER --debug -i$TMPISO --byteset 34930,0,1 >>$LOGFILE 2>&1
    
    run_regtest fix_with_rs01_file "--debug --set-version $SETVERSION -f" $TMPISO $TMPECC
fi

# Augmented image is protected by an outer RS03 error correction file

if try "RS02 image with RS03 error correction file" fix_with_rs03_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -mRS03 -c -n 20 -o file >>$LOGFILE 2>&1

    cp $MASTERISO $TMPISO
    $NEWVER --debug -i$TMPISO --byteset 34930,0,1 >>$LOGFILE 2>&1
    
    run_regtest fix_with_rs03_file "--debug --set-version $SETVERSION -f" $TMPISO $TMPECC
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

# Scan image which is shorter than expected

if try "scanning image being shorter than expected" scan_shorter; then
  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --truncate=$((REAL_ECCSIZE-44)) >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_shorter "--spinup-delay=0 -s -v" $TMPISO
fi

# Scan image which is longer than expected

if try "scanning image being longer than expected" scan_longer; then
  cp $MASTERISO $SIMISO
  for i in $(seq 23); do cat fixed-random-sequence >>$SIMISO; done

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_longer "--spinup-delay=0 -s -v" $TMPISO
fi

# Scan image with two multisession link sectors appended.

if try "scanning image, tao tail case" scan_tao_tail; then
  cp $MASTERISO $SIMISO
  cat fixed-random-sequence >>$SIMISO
  $NEWVER --debug -i$SIMISO --erase 34932-34933 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_tao_tail "--spinup-delay=0 -s" $TMPISO
fi

# Scan image with two real sectors missing at the end.
# -dao option prevents them from being clipped off.

if try "scanning image, no tao tail case" scan_no_tao_tail; then
  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase 34930-34931 >>$LOGFILE 2>&1

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
  $NEWVER --debug -i$SIMISO --byteset 30000,84,220 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 30000,85,65 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 30000,86,15 >>$LOGFILE 2>&1
  # Version info 99.99
  $NEWVER --debug -i$SIMISO --byteset 30000,88,220 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 30000,89,65 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 30000,90,15 >>$LOGFILE 2>&1
  # Patched selfcrc
  $NEWVER --debug -i$SIMISO --byteset 30000,96,106 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 30000,97,230 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 30000,98,75 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 30000,99,203 >>$LOGFILE 2>&1
  
  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_incompatible_ecc "--spinup-delay=0 -s" $TMPISO
fi

# Scan an image containing one header with an invalid cookie.
# Does not produce a warning message while scanning
# Change to better behaviour?

if try "scanning image with one defective header" scan_bad_header; then

  cp $MASTERISO $SIMISO
  $NEWVER -i$SIMISO --debug --byteset 30592,1,1 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_bad_header "--spinup-delay=0 -s" $TMPISO
fi

# Scan an image containing one header with an invalid cookie
# and one with CRC errors.
# Does not produce a warning message while scanning
# Change to better behaviour?

if try "scanning image with two defective headers" scan_bad_headers; then

  cp $MASTERISO $SIMISO
  $NEWVER -i$SIMISO --debug --byteset 30592,1,1 >>$LOGFILE 2>&1
  $NEWVER -i$SIMISO --debug --byteset 31488,100,1 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest scan_bad_headers "--spinup-delay=0 -s" $TMPISO
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
   run_regtest scan_missing_data_sectors "--spinup-delay=0 -s" $TMPISO
fi

# Image contains 1 row of missing sectors and a single one
# in the crc portion

if try "scanning image with missing crc sectors" scan_missing_crc_sectors; then
   cp $MASTERISO $SIMISO
   $NEWVER -i$SIMISO --debug --erase 30020-30030 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --erase 30034 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest scan_missing_crc_sectors "--spinup-delay=0 -s" $TMPISO
fi

# Image contains 1 row of missing sectors and a single one
# in the ecc portion

if try "scanning image with missing ecc sectors" scan_missing_ecc_sectors; then
   cp $MASTERISO $SIMISO
   $NEWVER -i$SIMISO --debug --erase 32020-32030 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --erase 33034 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest scan_missing_ecc_sectors "--spinup-delay=0 -s" $TMPISO
fi

# Image contains bad byte in the data section

if try "scanning image with bad data byte" scan_data_bad_byte; then
   cp $MASTERISO $SIMISO 
   $NEWVER -i$SIMISO --debug --byteset 1235,50,10 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest scan_data_bad_byte "--spinup-delay=0 -s" $TMPISO
fi

# Image contains bad byte in the CRC section
# This will create a spurious CRC error elsewhere (sect. 4152)

if try "scanning image with bad crc byte" scan_crc_bad_byte; then
   cp $MASTERISO $SIMISO 
   $NEWVER -i$SIMISO --debug --byteset 30020,50,10 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest scan_crc_bad_byte "--spinup-delay=0 -s" $TMPISO
fi

# Image contains bad byte in the ecc section

if try "scanning image with bad ecc byte" scan_ecc_bad_byte; then
   cp $MASTERISO $SIMISO 
   $NEWVER -i$SIMISO --debug --byteset 33100,50,10 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest scan_ecc_bad_byte "--spinup-delay=0 -s" $TMPISO
fi

# Test the pre-0.79.5 header modulo glitch correction
# with available ecc size information (header created by 0.79.5 or later)

if try "scanning with header modulo glitch, post 0.79.5 style hdr" scan_modulo_glitch; then
   HMGISO=$ISODIR/rs02-hmg-master.iso

   if ! test -f $HMGISO; then
     $NEWVER --debug -i $HMGISO --random-image 274300  >>$LOGFILE 2>&1
     $NEWVER --debug --set-version $SETVERSION -i$HMGISO -mRS02 -c >>$LOGFILE 2>&1
   fi

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$HMGISO --fixed-speed-values"
   run_regtest scan_modulo_glitch "--spinup-delay=0 -s -v" $TMPISO
fi

# Test the pre-0.79.5 header modulo glitch correction
# with no ecc size information (header created by 0.72 or earlier versions)

if try "header modulo glitch, old style, complete img" scan_modulo_glitch2; then
   HMGISO=$ISODIR/rs02-hmg-master.iso

   if ! test -f $HMGISO; then
     $NEWVER --debug -i $HMGISO --random-image 274300  >>$LOGFILE 2>&1
     $NEWVER --debug --set-version $SETVERSION -i$HMGISO -mRS02 -c >>$LOGFILE 2>&1
   fi

   cp $HMGISO $SIMISO  # create the old style image
   for header in 274300 278528 282624 286720 290816 294912 299008 303104 307200 311296 315392 319488 323584 327680 331776 335872 339968 344064 348160 352256 356352; do
       $NEWVER --debug -i $SIMISO --byteset $header,96,38  >>$LOGFILE 2>&1 # self sum
       $NEWVER --debug -i $SIMISO --byteset $header,97,245 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,98,168 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,99,221 >>$LOGFILE 2>&1

       $NEWVER --debug -i $SIMISO --byteset $header,128,0 >>$LOGFILE 2>&1 # size info
       $NEWVER --debug -i $SIMISO --byteset $header,129,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,130,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,131,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,132,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,133,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,134,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,135,0 >>$LOGFILE 2>&1
   done

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest scan_modulo_glitch2 "--spinup-delay=0 -s -v" $TMPISO
fi

# Test the pre-0.79.5 header modulo glitch correction
# with no ecc size information (header created by 0.72 or earlier versions)
# truncated image so that the simple size comparison heuristics will fail
# and ecc header search is performed.

if try "header modulo glitch, old style, truncated img" scan_modulo_glitch3; then
   HMGISO=$ISODIR/rs02-hmg-master.iso

   if ! test -f $HMGISO; then
     $NEWVER --debug -i $HMGISO --sandom-image 274300  >>$LOGFILE 2>&1
     $NEWVER --debug --set-version $SETVERSION -i$HMGISO -mRS02 -c >>$LOGFILE 2>&1
   fi

   cp $HMGISO $SIMISO  # create the old style image
   for header in 274300 278528 282624 286720 290816 294912 299008 303104 307200 311296 315392 319488 323584 327680 331776 335872 339968 344064 348160 352256 356352; do
       $NEWVER --debug -i $SIMISO --byteset $header,96,38  >>$LOGFILE 2>&1 # self sum
       $NEWVER --debug -i $SIMISO --byteset $header,97,245 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,98,168 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,99,221 >>$LOGFILE 2>&1

       $NEWVER --debug -i $SIMISO --byteset $header,128,0 >>$LOGFILE 2>&1 # size info
       $NEWVER --debug -i $SIMISO --byteset $header,129,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,130,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,131,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,132,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,133,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,134,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,135,0 >>$LOGFILE 2>&1
   done
   $NEWVER --debug -i $SIMISO --truncate=357520  >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest scan_modulo_glitch3 "--spinup-delay=0 -s -v" $TMPISO
fi

# Test the pre-0.79.5 header modulo glitch correction
# with no ecc size information (header created by 0.72 or earlier versions)
# truncated image so that the simple size comparison heuristics will fail
# and ecc header search is performed.

if try "header modulo glitch, old style, truncated img, missing ref secs" scan_modulo_glitch4; then
   HMGISO=$ISODIR/rs02-hmg-master.iso

   if ! test -f $HMGISO; then
     $NEWVER --debug -i $HMGISO --sandom-image 274300  >>$LOGFILE 2>&1
     $NEWVER --debug --set-version $SETVERSION -i$HMGISO -mRS02 -c >>$LOGFILE 2>&1
   fi

   cp $HMGISO $SIMISO  # create the old style image
   for header in 274300 278528 282624 286720 290816 294912 299008 303104 307200 311296 315392 319488 323584 327680 331776 335872 339968 344064 348160 352256 356352; do
       $NEWVER --debug -i $SIMISO --byteset $header,96,38  >>$LOGFILE 2>&1 # self sum
       $NEWVER --debug -i $SIMISO --byteset $header,97,245 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,98,168 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,99,221 >>$LOGFILE 2>&1

       $NEWVER --debug -i $SIMISO --byteset $header,128,0 >>$LOGFILE 2>&1 # size info
       $NEWVER --debug -i $SIMISO --byteset $header,129,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,130,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,131,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,132,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,133,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,134,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,135,0 >>$LOGFILE 2>&1
   done
   $NEWVER --debug -i $SIMISO --truncate=357520  >>$LOGFILE 2>&1

   # remove some sectors which could disambiguate the layouts
   # 354304 is the last remaining sector.
   for sector in 276480 280577 284672 288768 292864 296960 301056 305152 309248 313344 317440 321536 325632 329728 333824 337920 342016 346112 350208; do
       $NEWVER --debug -i $SIMISO --erase $sector  >>$LOGFILE 2>&1
   done

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest scan_modulo_glitch4 "--spinup-delay=0 -s -v" $TMPISO
fi

# Augmented image is protected by an outer RS01 error correction file
# Setting the byte creates a CRC error which can only be detected by
# the outer RS01 code, so we use this as an additional probe that the
# correct (outer) ECC is applied.

if try "scanning RS02 image with RS01 ecc file" scan_with_rs01_file; then
    echo
    echo "Test may fail due to a race condition in the output generation."
    echo "Skip with./rs02.bash cont RS02_scan_with_wrong_rs01_file"
    echo -n "Test result - "
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -c -n normal >>$LOGFILE 2>&1
    cp $MASTERISO $SIMISO
    $NEWVER --debug -i$SIMISO --byteset 34930,0,1 >>$LOGFILE 2>&1

    IGNORE_LOG_LINE="^Read position: 100"
    extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
    run_regtest scan_with_rs01_file "--spinup-delay=0 -s" $SIMISO $TMPECC
fi

# Augmented image and non-matching RS01 error correction file
# Currently the mismatch is (generally) not detected
# Should we change this behaviour? Solve later.
# Expected behaviour for verify is to report the non-matching ecc file
# rather than falling back to using the RS02 part since the
# user did probably have some intentention specifying the ecc file.

if try "scanning RS02 image with non-matching RS01 ecc file" scan_with_wrong_rs01_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -c -n normal >>$LOGFILE 2>&1
    $NEWVER --debug -i$TMPECC --byteset 0,24,1 >>$LOGFILE 2>&1  # alter the fingerprint

    cp $MASTERISO $SIMISO
    $NEWVER --debug -i$SIMISO --byteset 34930,0,1 >>$LOGFILE 2>&1

    extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
    run_regtest scan_with_wrong_rs01_file "--spinup-delay=0 -s" $SIMISO $TMPECC
fi

# Augmented image is protected by an outer RS03 error correction file

if try "scanning RS02 image with RS03 ecc file" scan_with_rs03_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -mRS03 -c -n 20 -o file >>$LOGFILE 2>&1

    cp $MASTERISO $SIMISO
    $NEWVER --debug -i$SIMISO --byteset 34930,0,1 >>$LOGFILE 2>&1

    extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
    run_regtest scan_with_rs03_file "--spinup-delay=0 -s" $SIMISO $TMPECC
fi

# Augmented image and non-matching RS03 error correction file

if try "scanning RS02 image with non-matching RS03 ecc file" scan_with_wrong_rs03_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -mRS03 -c -n 20 -o file >>$LOGFILE 2>&1
    $NEWVER --debug -i$TMPECC --byteset 0,24,1 >>$LOGFILE 2>&1

    cp $MASTERISO $SIMISO
    $NEWVER --debug -i$SIMISO --byteset 34930,0,1 >>$LOGFILE 2>&1

    extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
    run_regtest scan_with_wrong_rs03_file "--spinup-delay=0 -s" $SIMISO $TMPECC
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

# Read into existing and complete image file

if try "reading good image in good file" read_good_file; then
  cp $MASTERISO $SIMISO
  cp $MASTERISO $TMPISO

  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_good_file "--spinup-delay=0 -r" $TMPISO
fi

# Read complete / optimal image with verbose debugging output

if try "reading good image with verbose output" read_good_verbose; then
  cp $MASTERISO $SIMISO

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_good_verbose "--spinup-delay=0 -r -v" $TMPISO
fi

# Read image which is shorter than expected
# Currently we are trying to read past the medium
# and getting media errors. Is that smart? Rethink later.

if try "reading image being shorter than expected" read_shorter; then
  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --truncate=$((REAL_ECCSIZE-44)) >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_shorter "--spinup-delay=0 -r -v" $TMPISO
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
  $NEWVER --debug -i$SIMISO --erase 34932-34933 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_tao_tail "--spinup-delay=0 -r" $TMPISO
fi

# Read image with two real sectors missing at the end.
# -dao option prevents them from being clipped off.

if try "reading image, no tao tail case" read_no_tao_tail; then
  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase 34930-34931 >>$LOGFILE 2>&1

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
  $NEWVER --debug -i$SIMISO --byteset 30000,84,220 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 30000,85,65 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 30000,86,15 >>$LOGFILE 2>&1
  # Version info 99.99
  $NEWVER --debug -i$SIMISO --byteset 30000,88,220 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 30000,89,65 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 30000,90,15 >>$LOGFILE 2>&1
  # Patched selfcrc
  $NEWVER --debug -i$SIMISO --byteset 30000,96,106 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 30000,97,230 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 30000,98,75 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 30000,99,203 >>$LOGFILE 2>&1
  
  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_incompatible_ecc "--spinup-delay=0 -r" $TMPISO
fi

# Read an image containing with missing master header.

if try "reading image with missing master header" read_bad_master; then

  cp $MASTERISO $SIMISO
  $NEWVER -i$SIMISO --debug --erase 30000 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_bad_master "--spinup-delay=0 -r -v" $TMPISO
fi

# Read an image containing with missing master header
# and exhaustive RS02 search.

if try "reading image with missing master header, exhaustive" read_bad_master_exhaustive; then

  cp $MASTERISO $SIMISO
  $NEWVER -i$SIMISO --debug --erase 30000 >>$LOGFILE 2>&1

  rm -f $TMPISO
  replace_config examine-rs02 1
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_bad_master_exhaustive "--spinup-delay=0 -r -v -a RS02" $TMPISO
fi

# Read an image containing one header with an invalid cookie.
# Does not produce a warning message while reading
# Change to better behaviour?

if try "reading image with one defective header" read_bad_header; then

  cp $MASTERISO $SIMISO
  $NEWVER -i$SIMISO --debug --byteset 30592,1,1 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_bad_header "--spinup-delay=0 -r" $TMPISO
fi

# Read an image containing one header with an invalid cookie
# and one with CRC errors.
# Does not produce a warning message while reading
# Change to better behaviour?

if try "reading image with two defective headers" read_bad_headers; then

  cp $MASTERISO $SIMISO
  $NEWVER -i$SIMISO --debug --byteset 30592,1,1 >>$LOGFILE 2>&1
  $NEWVER -i$SIMISO --debug --byteset 31488,100,1 >>$LOGFILE 2>&1

  rm -f $TMPISO
  extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
  run_regtest read_bad_headers "--spinup-delay=0 -r" $TMPISO
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
   run_regtest read_missing_data_sectors "--spinup-delay=0 -r" $TMPISO
fi

# Image contains 1 row of missing sectors and a single one
# in the crc portion

if try "reading image with missing crc sectors" read_missing_crc_sectors; then
   cp $MASTERISO $SIMISO
   $NEWVER -i$SIMISO --debug --erase 30020-30030 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --erase 30034 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest read_missing_crc_sectors "--spinup-delay=0 -r" $TMPISO
fi

# Image contains 1 row of missing sectors and a single one
# in the ecc portion

if try "reading image with missing ecc sectors" read_missing_ecc_sectors; then
   cp $MASTERISO $SIMISO
   $NEWVER -i$SIMISO --debug --erase 32020-32030 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --erase 33034 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest read_missing_ecc_sectors "--spinup-delay=0 -r" $TMPISO
fi

# Image contains bad byte in the data section

if try "reading image with bad data byte" read_data_bad_byte; then
   cp $MASTERISO $SIMISO 
   $NEWVER -i$SIMISO --debug --byteset 0,55,12 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --byteset 1235,50,10 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --byteset 29999,128,98 >>$LOGFILE 2>&1
   
   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest read_data_bad_byte "--spinup-delay=0 -r" $TMPISO
fi

# Image contains bad byte in the CRC section
# This will create a spurious CRC error elsewhere (sect. 4152)

if try "reading image with bad crc byte" read_crc_bad_byte; then
   cp $MASTERISO $SIMISO 
   $NEWVER -i$SIMISO --debug --byteset 30020,50,10 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest read_crc_bad_byte "--spinup-delay=0 -r" $TMPISO
fi

# Image contains bad byte in the ecc section

if try "reading image with bad ecc byte" read_ecc_bad_byte; then
   cp $MASTERISO $SIMISO 
   $NEWVER -i$SIMISO --debug --byteset 33100,50,10 >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest read_ecc_bad_byte "--spinup-delay=0 -r" $TMPISO
fi

# Test the pre-0.79.5 header modulo glitch correction
# with available ecc size information (header created by 0.79.5 or later)

if try "reading with header modulo glitch, post 0.79.5 style hdr" read_modulo_glitch; then
   HMGISO=$ISODIR/rs02-hmg-master.iso

   if ! test -f $HMGISO; then
     $NEWVER --debug -i $HMGISO --random-image 274300  >>$LOGFILE 2>&1
     $NEWVER --debug --set-version $SETVERSION -i$HMGISO -mRS02 -c >>$LOGFILE 2>&1
   fi

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$HMGISO --fixed-speed-values"
   run_regtest read_modulo_glitch "--spinup-delay=0 -r -v" $TMPISO
fi

# Test the pre-0.79.5 header modulo glitch correction
# with no ecc size information (header created by 0.72 or earlier versions)

if try "header modulo glitch, old style, complete img" read_modulo_glitch2; then
   HMGISO=$ISODIR/rs02-hmg-master.iso

   if ! test -f $HMGISO; then
     $NEWVER --debug -i $HMGISO --random-image 274300  >>$LOGFILE 2>&1
     $NEWVER --debug --set-version $SETVERSION -i$HMGISO -mRS02 -c >>$LOGFILE 2>&1
   fi

   cp $HMGISO $SIMISO  # create the old style image
   for header in 274300 278528 282624 286720 290816 294912 299008 303104 307200 311296 315392 319488 323584 327680 331776 335872 339968 344064 348160 352256 356352; do
       $NEWVER --debug -i $SIMISO --byteset $header,96,38  >>$LOGFILE 2>&1 # self sum
       $NEWVER --debug -i $SIMISO --byteset $header,97,245 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,98,168 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,99,221 >>$LOGFILE 2>&1

       $NEWVER --debug -i $SIMISO --byteset $header,128,0 >>$LOGFILE 2>&1 # size info
       $NEWVER --debug -i $SIMISO --byteset $header,129,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,130,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,131,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,132,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,133,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,134,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,135,0 >>$LOGFILE 2>&1
   done

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest read_modulo_glitch2 "--spinup-delay=0 -r -v" $TMPISO
fi

# Test the pre-0.79.5 header modulo glitch correction
# with no ecc size information (header created by 0.72 or earlier versions)
# truncated image so that the simple size comparison heuristics will fail
# and ecc header search is performed.

if try "header modulo glitch, old style, truncated img" read_modulo_glitch3; then
   HMGISO=$ISODIR/rs02-hmg-master.iso

   if ! test -f $HMGISO; then
     $NEWVER --debug -i $HMGISO --random-image 274300  >>$LOGFILE 2>&1
     $NEWVER --debug --set-version $SETVERSION -i$HMGISO -mRS02 -c >>$LOGFILE 2>&1
   fi

   cp $HMGISO $SIMISO  # create the old style image
   for header in 274300 278528 282624 286720 290816 294912 299008 303104 307200 311296 315392 319488 323584 327680 331776 335872 339968 344064 348160 352256 356352; do
       $NEWVER --debug -i $SIMISO --byteset $header,96,38  >>$LOGFILE 2>&1 # self sum
       $NEWVER --debug -i $SIMISO --byteset $header,97,245 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,98,168 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,99,221 >>$LOGFILE 2>&1

       $NEWVER --debug -i $SIMISO --byteset $header,128,0 >>$LOGFILE 2>&1 # size info
       $NEWVER --debug -i $SIMISO --byteset $header,129,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,130,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,131,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,132,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,133,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,134,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,135,0 >>$LOGFILE 2>&1
   done
   $NEWVER --debug -i $SIMISO --truncate=357520  >>$LOGFILE 2>&1

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest read_modulo_glitch3 "--spinup-delay=0 -r -v" $TMPISO
fi

# Test the pre-0.79.5 header modulo glitch correction
# with no ecc size information (header created by 0.72 or earlier versions)
# truncated image so that the simple size comparison heuristics will fail
# and ecc header search is performed.

if try "header modulo glitch, old style, truncated img, missing ref secs" read_modulo_glitch4; then
   HMGISO=$ISODIR/rs02-hmg-master.iso

   if ! test -f $HMGISO; then
     $NEWVER --debug -i $HMGISO --random-image 274300  >>$LOGFILE 2>&1
     $NEWVER --debug --set-version $SETVERSION -i$HMGISO -mRS02 -c >>$LOGFILE 2>&1
   fi

   cp $HMGISO $SIMISO  # create the old style image
   for header in 274300 278528 282624 286720 290816 294912 299008 303104 307200 311296 315392 319488 323584 327680 331776 335872 339968 344064 348160 352256 356352; do
       $NEWVER --debug -i $SIMISO --byteset $header,96,38  >>$LOGFILE 2>&1 # self sum
       $NEWVER --debug -i $SIMISO --byteset $header,97,245 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,98,168 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,99,221 >>$LOGFILE 2>&1

       $NEWVER --debug -i $SIMISO --byteset $header,128,0 >>$LOGFILE 2>&1 # size info
       $NEWVER --debug -i $SIMISO --byteset $header,129,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,130,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,131,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,132,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,133,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,134,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,135,0 >>$LOGFILE 2>&1
   done
   $NEWVER --debug -i $SIMISO --truncate=357520  >>$LOGFILE 2>&1

   # remove some sectors which could disambiguate the layouts
   # 354304 is the last remaining sector.
   for sector in 276480 280577 284672 288768 292864 296960 301056 305152 309248 313344 317440 321536 325632 329728 333824 337920 342016 346112 350208; do
       $NEWVER --debug -i $SIMISO --erase $sector  >>$LOGFILE 2>&1
   done

   rm -f $TMPISO
   extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
   run_regtest read_modulo_glitch4 "--spinup-delay=0 -r -v" $TMPISO
fi

# Augmented image is protected by an outer RS01 error correction file
# Setting the byte creates a CRC error which can only be detected by
# the outer RS01 code, so we use this as an additional probe that the
# correct (outer) ECC is applied.

if try "reading RS02 image with RS01 ecc file" read_with_rs01_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -c -n normal >>$LOGFILE 2>&1
    cp $MASTERISO $SIMISO
    $NEWVER --debug -i$SIMISO --byteset 34930,0,1 >>$LOGFILE 2>&1
    
    rm -f $TMPISO
    extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
    run_regtest read_with_rs01_file "--spinup-delay=0 -r" $TMPISO $TMPECC
fi

# Augmented image and non-matching RS01 error correction file
# Currently the mismatch is (generally) not detected
# NOTE: There seems to be an intermittent race condition between
# printing the defective sector number and the reading progress.
# Ignore for now and debug later.
# Should we change this behaviour? Rethink later.
# Expected behaviour for verify is to report the non-matching ecc file
# rather than falling back to using the RS02 part since the
# user did probably have some intentention specifying the ecc file.

if try "reading RS02 image with non-matching RS01 ecc file" read_with_wrong_rs01_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -c -n normal >>$LOGFILE 2>&1
    $NEWVER --debug -i$TMPECC --byteset 0,24,1 >>$LOGFILE 2>&1  # alter the fingerprint

    cp $MASTERISO $SIMISO
    $NEWVER --debug -i$SIMISO --byteset 34930,0,1 >>$LOGFILE 2>&1

    rm -f $TMPISO
    extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
    run_regtest read_with_wrong_rs01_file "--spinup-delay=0 -r" $TMPISO $TMPECC
fi

# Augmented image is protected by an outer RS03 error correction file

if try "reading RS02 image with RS03 ecc file" read_with_rs03_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -mRS03 -c -n 20 -o file >>$LOGFILE 2>&1

    cp $MASTERISO $SIMISO
    $NEWVER --debug -i$SIMISO --byteset 34930,0,1 >>$LOGFILE 2>&1

    rm -f $TMPISO
    extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
    run_regtest read_with_rs03_file "--spinup-delay=0 -r" $TMPISO $TMPECC
fi

# Augmented image and non-matching RS03 error correction file

if try "reading RS02 image with non-matching RS03 ecc file" read_with_wrong_rs03_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -mRS03 -c -n 20 -o file >>$LOGFILE 2>&1
    $NEWVER --debug -i$TMPECC --byteset 0,24,1 >>$LOGFILE 2>&1

    cp $MASTERISO $SIMISO
    $NEWVER --debug -i$SIMISO --byteset 34930,0,1 >>$LOGFILE 2>&1

    rm -f $TMPISO
    extra_args="--debug --sim-cd=$SIMISO --fixed-speed-values"
    run_regtest read_with_wrong_rs03_file "--spinup-delay=0 -r" $TMPISO $TMPECC
fi

### Reading tests (adaptive)

echo "# Reading tests (adaptive)"

echo "Currently not enabled!"
exit 0

# Read complete / optimal image

if try "reading good image" adaptive_good; then
  cp $MASTERISO $SIMISO

  rm -f $TMPISO
  run_regtest adaptive_good "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO
fi

# Read into existing and complete image file

if try "reading good image in good file" adaptive_good_file; then
  cp $MASTERISO $SIMISO
  cp $MASTERISO $TMPISO

  run_regtest adaptive_good_file "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO
fi

# Read complete / optimal image with verbose debugging output

if try "reading good image with verbose output" adaptive_good_verbose; then
  cp $MASTERISO $SIMISO

  rm -f $TMPISO
  run_regtest adaptive_good_verbose "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r -v --adaptive-read" $TMPISO
fi

# Read image which is shorter than expected
# TODO: Currently the media size is taken from the
# ecc data and not the medium so we could be trying to 
# read past the medium and get media errors under some
# circumstances. Is that smart? Rethink later.

if try "reading image being shorter than expected" adaptive_shorter; then
  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --truncate=$((REAL_ECCSIZE-44)) >>$LOGFILE 2>&1

  rm -f $TMPISO
  run_regtest adaptive_shorter "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r -v --adaptive-read" $TMPISO
fi

# Read image which is longer than expected
# Will return image in its original length.
# TODO: no warning is given about the wrong medium length
# as the right size is automatically pulled from the
# ecc header.

if try "reading image being longer than expected" adaptive_longer; then
  cp $MASTERISO $SIMISO
  for i in $(seq 23); do cat fixed-random-sequence >>$SIMISO; done

  rm -f $TMPISO
  run_regtest adaptive_longer "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r -v --adaptive-read" $TMPISO
fi

# Read image with two multisession link sectors appended.
# Will return image in its original length.
# TODO: See comments above. This test is masked off by
# the behaviour decribed in the previous test case.

if try "reading image, tao tail case" adaptive_tao_tail; then
  cp $MASTERISO $SIMISO
  cat fixed-random-sequence >>$SIMISO
  $NEWVER --debug -i$SIMISO --erase 34932-34933 >>$LOGFILE 2>&1

  rm -f $TMPISO
  run_regtest adaptive_tao_tail "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO
fi

# Read image with two real sectors missing at the end.
# -dao option prevents them from being clipped off.
# TODO: See comments above. This test is masked off by
# the behaviour decribed in the previous test case.

if try "reading image, no tao tail case" adaptive_no_tao_tail; then
  cp $MASTERISO $SIMISO
  $NEWVER --debug -i$SIMISO --erase 34930-34931 >>$LOGFILE 2>&1

  rm -f $TMPISO
  run_regtest adaptive_no_tao_tail "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r --dao --adaptive-read" $TMPISO
fi

# Read an image for which ecc information is available,
# but requiring a newer dvdisaster version.
# NOTE: Only the master header is manipulated, which is
# sufficient for reaching the goal of this test case.

if try "reading image requiring a newer dvdisaster version" adaptive_incompatible_ecc; then

  cp $MASTERISO $SIMISO
  # Creator version 99.99
  $NEWVER --debug -i$SIMISO --byteset 30000,84,220 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 30000,85,65 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 30000,86,15 >>$LOGFILE 2>&1
  # Version info 99.99
  $NEWVER --debug -i$SIMISO --byteset 30000,88,220 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 30000,89,65 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 30000,90,15 >>$LOGFILE 2>&1
  # Patched selfcrc
  $NEWVER --debug -i$SIMISO --byteset 30000,96,106 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 30000,97,230 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 30000,98,75 >>$LOGFILE 2>&1
  $NEWVER --debug -i$SIMISO --byteset 30000,99,203 >>$LOGFILE 2>&1
  
  rm -f $TMPISO
  run_regtest adaptive_incompatible_ecc "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO
fi

# Read an image containing one header with an invalid cookie.
# (This is not the header used to identify the RS02 codec!)
# Does not produce a warning message while reading
# TODO: Change to better behaviour?

if try "reading image with one defective header" adaptive_bad_header; then

  cp $MASTERISO $SIMISO
  $NEWVER -i$SIMISO --debug --byteset 30592,1,1 >>$LOGFILE 2>&1

  rm -f $TMPISO
  run_regtest adaptive_bad_header "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r -v --adaptive-read" $TMPISO
fi

# Read an image containing one header with an invalid cookie
# and one with CRC errors.
# Does not produce a warning message while reading
# TODO: Change to better behaviour?

if try "reading image with two defective headers" adaptive_bad_headers; then

  cp $MASTERISO $SIMISO
  $NEWVER -i$SIMISO --debug --byteset 30592,1,1 >>$LOGFILE 2>&1
  $NEWVER -i$SIMISO --debug --byteset 31488,100,1 >>$LOGFILE 2>&1

  rm -f $TMPISO
  run_regtest adaptive_bad_headers "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO
fi

# Image contains 2 rows of missing sectors and a single one
# in the data portion

if try "reading image with missing data sectors" adaptive_missing_data_sectors; then
   cp $MASTERISO $SIMISO
   $NEWVER -i$SIMISO --debug --erase 1000-1049 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --erase 21230 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --erase 22450-22457 >>$LOGFILE 2>&1

   rm -f $TMPISO
   run_regtest adaptive_missing_data_sectors "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO
fi

# Image contains 1 row of missing sectors and a single one
# in the crc portion

if try "reading image with missing crc sectors" adaptive_missing_crc_sectors; then
   cp $MASTERISO $SIMISO
   $NEWVER -i$SIMISO --debug --erase 30020-30030 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --erase 30034 >>$LOGFILE 2>&1

   rm -f $TMPISO
   run_regtest adaptive_missing_crc_sectors "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO
fi

# Image contains 1 row of missing sectors and a single one
# in the ecc portion

if try "reading image with missing ecc sectors" adaptive_missing_ecc_sectors; then
   cp $MASTERISO $SIMISO
   $NEWVER -i$SIMISO --debug --erase 32020-32030 >>$LOGFILE 2>&1
   $NEWVER -i$SIMISO --debug --erase 33034 >>$LOGFILE 2>&1

   rm -f $TMPISO
   run_regtest adaptive_missing_ecc_sectors "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO
fi

# Image contains bad byte in the data section

if try "reading image with bad data byte" adaptive_data_bad_byte; then
   cp $MASTERISO $SIMISO 
   $NEWVER -i$SIMISO --debug --byteset 1235,50,10 >>$LOGFILE 2>&1

   rm -f $TMPISO
   run_regtest adaptive_data_bad_byte "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO
fi

# Image contains bad byte in the CRC section
# This will create a spurious CRC error elsewhere (sect. 4152)
# TODO: The defective CRC sector is not repaired. 

if try "reading image with bad crc byte" adaptive_crc_bad_byte; then
   cp $MASTERISO $SIMISO 
   $NEWVER -i$SIMISO --debug --byteset 30020,50,10 >>$LOGFILE 2>&1

   rm -f $TMPISO
   run_regtest adaptive_crc_bad_byte "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO
fi

# Image contains bad byte in the ecc section

if try "reading image with bad ecc byte" adaptive_ecc_bad_byte; then
   cp $MASTERISO $SIMISO 
   $NEWVER -i$SIMISO --debug --byteset 33100,50,10 >>$LOGFILE 2>&1

   rm -f $TMPISO
   run_regtest adaptive_ecc_bad_byte "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO
fi

# Test the pre-0.79.5 header modulo glitch correction
# with available ecc size information (header created by 0.79.5 or later)

if try "reading with header modulo glitch, post 0.79.5 style hdr" adaptive_modulo_glitch; then
   HMGISO=$ISODIR/rs02-hmg-master.iso

   if ! test -f $HMGISO; then
     $NEWVER --debug -i $HMGISO --random-image 274300  >>$LOGFILE 2>&1
     $NEWVER --debug --set-version $SETVERSION -i$HMGISO -mRS02 -c >>$LOGFILE 2>&1
   fi

   rm -f $TMPISO
   run_regtest adaptive_modulo_glitch "--debug --sim-cd=$HMGISO --fixed-speed-values --spinup-delay=0 -r -v --adaptive-read" $TMPISO
fi

# Test the pre-0.79.5 header modulo glitch correction
# with no ecc size information (header created by 0.72 or earlier versions)

if try "header modulo glitch, old style, complete img" adaptive_modulo_glitch2; then
   HMGISO=$ISODIR/rs02-hmg-master.iso

   if ! test -f $HMGISO; then
     $NEWVER --debug -i $HMGISO --random-image 274300  >>$LOGFILE 2>&1
     $NEWVER --debug --set-version $SETVERSION -i$HMGISO -mRS02 -c >>$LOGFILE 2>&1
   fi

   cp $HMGISO $SIMISO  # create the old style image
   for header in 274300 278528 282624 286720 290816 294912 299008 303104 307200 311296 315392 319488 323584 327680 331776 335872 339968 344064 348160 352256 356352; do
       $NEWVER --debug -i $SIMISO --byteset $header,96,38  >>$LOGFILE 2>&1 # self sum
       $NEWVER --debug -i $SIMISO --byteset $header,97,245 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,98,168 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,99,221 >>$LOGFILE 2>&1

       $NEWVER --debug -i $SIMISO --byteset $header,128,0 >>$LOGFILE 2>&1 # size info
       $NEWVER --debug -i $SIMISO --byteset $header,129,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,130,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,131,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,132,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,133,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,134,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,135,0 >>$LOGFILE 2>&1
   done

   rm -f $TMPISO
   run_regtest adaptive_modulo_glitch2 "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r -v --adaptive-read" $TMPISO
fi

# Test the pre-0.79.5 header modulo glitch correction
# with no ecc size information (header created by 0.72 or earlier versions)
# truncated image so that the simple size comparison heuristics will fail
# and ecc header search is performed.

if try "header modulo glitch, old style, truncated img" adaptive_modulo_glitch3; then
   HMGISO=$ISODIR/rs02-hmg-master.iso

   if ! test -f $HMGISO; then
     $NEWVER --debug -i $HMGISO --random-image 274300  >>$LOGFILE 2>&1
     $NEWVER --debug --set-version $SETVERSION -i$HMGISO -mRS02 -c >>$LOGFILE 2>&1
   fi

   cp $HMGISO $SIMISO  # create the old style image
   for header in 274300 278528 282624 286720 290816 294912 299008 303104 307200 311296 315392 319488 323584 327680 331776 335872 339968 344064 348160 352256 356352; do
       $NEWVER --debug -i $SIMISO --byteset $header,96,38  >>$LOGFILE 2>&1 # self sum
       $NEWVER --debug -i $SIMISO --byteset $header,97,245 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,98,168 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,99,221 >>$LOGFILE 2>&1

       $NEWVER --debug -i $SIMISO --byteset $header,128,0 >>$LOGFILE 2>&1 # size info
       $NEWVER --debug -i $SIMISO --byteset $header,129,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,130,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,131,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,132,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,133,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,134,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,135,0 >>$LOGFILE 2>&1
   done
   $NEWVER --debug -i $SIMISO --truncate=357520  >>$LOGFILE 2>&1

   rm -f $TMPISO
   run_regtest adaptive_modulo_glitch3 "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r -v --adaptive-read" $TMPISO
fi

# Test the pre-0.79.5 header modulo glitch correction
# with no ecc size information (header created by 0.72 or earlier versions)
# truncated image so that the simple size comparison heuristics will fail
# and ecc header search is performed.

if try "header modulo glitch, old style, truncated img, missing ref secs" adaptive_modulo_glitch4; then
   HMGISO=$ISODIR/rs02-hmg-master.iso

   if ! test -f $HMGISO; then
     $NEWVER --debug -i $HMGISO --random-image 274300  >>$LOGFILE 2>&1
     $NEWVER --debug --set-version $SETVERSION -i$HMGISO -mRS02 -c >>$LOGFILE 2>&1
   fi

   cp $HMGISO $SIMISO  # create the old style image
   for header in 274300 278528 282624 286720 290816 294912 299008 303104 307200 311296 315392 319488 323584 327680 331776 335872 339968 344064 348160 352256 356352; do
       $NEWVER --debug -i $SIMISO --byteset $header,96,38  >>$LOGFILE 2>&1 # self sum
       $NEWVER --debug -i $SIMISO --byteset $header,97,245 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,98,168 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,99,221 >>$LOGFILE 2>&1

       $NEWVER --debug -i $SIMISO --byteset $header,128,0 >>$LOGFILE 2>&1 # size info
       $NEWVER --debug -i $SIMISO --byteset $header,129,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,130,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,131,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,132,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,133,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,134,0 >>$LOGFILE 2>&1
       $NEWVER --debug -i $SIMISO --byteset $header,135,0 >>$LOGFILE 2>&1
   done
   $NEWVER --debug -i $SIMISO --truncate=357520  >>$LOGFILE 2>&1

   # remove some sectors which could disambiguate the layouts
   # 354304 is the last remaining sector.
   for sector in 276480 280577 284672 288768 292864 296960 301056 305152 309248 313344 317440 321536 325632 329728 333824 337920 342016 346112 350208; do
       $NEWVER --debug -i $SIMISO --erase $sector  >>$LOGFILE 2>&1
   done

   rm -f $TMPISO
   run_regtest adaptive_modulo_glitch4 "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r -v --adaptive-read" $TMPISO
fi

# Augmented image is protected by an outer RS01 error correction file

if try "reading RS02 image with RS01 ecc file" adaptive_with_rs01_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -c -n normal >>$LOGFILE 2>&1
    cp $MASTERISO $SIMISO
    
    rm -f $TMPISO
    run_regtest adaptive_with_rs01_file "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO $TMPECC
fi

# Augmented image and non-matching RS01 error correction file
# Currently the mismatch is (generally) not detected
# NOTE: There seems to be an intermittent race condition between
# printing the defective sector number and the reading progress.
# Ignore for now and debug later.
# TODO: Should we change this behaviour?
# Expected behaviour for verify is to report the non-matching ecc file
# rather than falling back to using the RS02 part since the
# user did probably have some intentention specifying the ecc file.

if try "reading RS02 image with non-matching RS01 ecc file" adaptive_with_wrong_rs01_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -c -n normal >>$LOGFILE 2>&1
    $NEWVER --debug -i$TMPECC --byteset 0,24,1 >>$LOGFILE 2>&1  # alter the fingerprint

    cp $MASTERISO $SIMISO

    rm -f $TMPISO
    run_regtest adaptive_with_wrong_rs01_file "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r --adaptive-read" $TMPISO $TMPECC
fi

# Augmented image is protected by an outer RS03 error correction file

if try "reading RS02 image with RS03 ecc file" read_with_rs03_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -mRS03 -c -n 20 -o file >>$LOGFILE 2>&1

    cp $MASTERISO $SIMISO
    $NEWVER --debug -i$SIMISO --byteset 34930,0,1 >>$LOGFILE 2>&1

    rm -f $TMPISO
    run_regtest read_with_rs03_file "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r" $TMPISO $TMPECC
fi

# Augmented image and non-matching RS03 error correction file

if try "reading RS02 image with non-matching RS03 ecc file" read_with_wrong_rs03_file; then
    $NEWVER --debug --set-version $SETVERSION -i$MASTERISO -e$TMPECC -mRS03 -c -n 20 -o file >>$LOGFILE 2>&1
    $NEWVER --debug -i$TMPECC --byteset 0,24,1 >>$LOGFILE 2>&1

    cp $MASTERISO $SIMISO
    $NEWVER --debug -i$SIMISO --byteset 34930,0,1 >>$LOGFILE 2>&1

    rm -f $TMPISO
    run_regtest read_with_wrong_rs03_file "--debug --sim-cd=$SIMISO --fixed-speed-values --spinup-delay=0 -r" $TMPISO $TMPECC
fi
