/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2004-2017 Carsten Gnoerlich.
 *  Copyright (C) 2019-2021 The dvdisaster development team.
 * 
 *  Email: support@dvdisaster.org
 *
 *  This file is part of dvdisaster.
 *
 *  dvdisaster is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  dvdisaster is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with dvdisaster. If not, see <http://www.gnu.org/licenses/>.
 */

/*** src type: no GUI code ***/

#include "dvdisaster.h"
#include "udf.h"

#include "rs03-includes.h"

/*
 * Aux. functions
 */

static int valid_crc_block(unsigned char *buf, guint64 sector, int image_expected)
{  CrcBlock *cb = alloca(2048);
   guint32 recorded_crc, real_crc;

   memcpy(cb, buf, 2048);

   /* See if the magic cookie is there */

   if(   strncmp((char*)cb->cookie, "*dvdisaster*", 12)
      || strncmp((char*)cb->method, "RS03", 4))
     return 0;

   /* Examine the checksum */
   
   recorded_crc = cb->selfCRC;

#ifdef HAVE_BIG_ENDIAN
   cb->selfCRC = 0x47504c00;
#else
   cb->selfCRC = 0x4c5047;
#endif
   real_crc = Crc32((unsigned char*)cb, 2048);

   if(real_crc != recorded_crc)
   {  Verbose(".. invalid CRC block %lld\n", (unsigned long long)sector);
      return 1;
   }

   /* If an ecc file header is found in the image (which might
      rightfully contain ecc files), ignore it */

   if(image_expected && (cb->methodFlags[0] & MFLAG_ECC_FILE))
   {  Verbose(".. Crc block from ecc file in image - IGNORED\n");
      return 1;
   }
    
   return 2;
}

/***
 *** Recognize a RS03 error correction file
 ***/

int RS03RecognizeFile(LargeFile *ecc_file, EccHeader **eh)
{  int crc_block = 0;
   int n;

   Verbose("RS03RecognizeFile(): examining %s\n", ecc_file->path);
   *eh = g_malloc(sizeof(EccHeader));

   /*** First see whether we have a valid ecc header. */
   
   LargeSeek(ecc_file, 0);
   n = LargeRead(ecc_file, *eh, sizeof(EccHeader));

   /* short read -> definitely not an ecc file */
   
   if(n != sizeof(EccHeader))
   {  g_free(*eh);
      *eh=NULL;
      Verbose("RS03RecognizeFile(): short read for ecc header\n");
      return ECCFILE_INVALID;
   }

   /* Validate the header */
   
   if(!strncmp((char*)(*eh)->cookie, "*dvdisaster*", 12))
   {  guint32 recorded_crc,real_crc;
   
      /* Examine the checksum */

     recorded_crc = (*eh)->selfCRC;

#ifdef HAVE_BIG_ENDIAN
      (*eh)->selfCRC = 0x47504c00;
#else
      (*eh)->selfCRC = 0x4c5047;
#endif
      real_crc = Crc32((unsigned char*)(*eh), 4096);

#ifdef HAVE_BIG_ENDIAN
      SwapEccHeaderBytes(*eh);
#endif

      if(real_crc != recorded_crc)
      {  Verbose("RS03RecognizeFile(): checksum error in ecc header\n");
      }
      else
      {  if(!strncmp((char*)(*eh)->method, "RS03", 4))
	 {  Verbose("RS03RecognizeFile(): ecc header found\n");
	    return ECCFILE_PRESENT;
	 }
	 else
	 {  Verbose("RS03RecognizeFile(): wrong codec\n");
	    g_free(*eh);
	    *eh=NULL;
	    return ECCFILE_WRONG_CODEC;
	 }
      }
   }
   else Verbose("RS03RecognizeFile(): no magic cookie in header\n");

   /* No ecc header found; search for CRC blocks.
      The CRC block follow directly after the ecc header,
      so simply continue reading in 2048 chunks until
      the file ends. There is no good criterion for stopping
      the read earlier since the file may be truncated and/or
      contain mangled contents in many unpredictable ways. */

   Verbose("RS03RecognizeFile(): exhaustive search for CRC blocks started\n");
   
   for(;;)
   {  unsigned char buf[2048];
      n = LargeRead(ecc_file, buf, 2048);

      if(n != 2048)
      {  if(n== 0 && LargeEOF(ecc_file))
	      Verbose("RS03RecognizeFile(): end of file reached\n");
	else Verbose("RS03RecognizeFile(): short read for CRC sector %d\n", crc_block);
	 g_free(*eh);
	 *eh=NULL;
	 return ECCFILE_INVALID;
      }

      if(valid_crc_block(buf, crc_block++, FALSE) == 2)
      {  ReconstructRS03Header(*eh, (CrcBlock*)buf);
	 Verbose("** Success: sector %d, rediscovered format with %d roots\n",
		 crc_block+1, (*eh)->eccBytes);

	 /* Rewrite the missing ecc header if possible */
	 
	 if(ecc_file->flags & O_RDWR || ecc_file->flags & O_WRONLY)
	 { int success=0; 
	   EccHeader *le_eh = *eh;
	   char buf[4096];

	   memset(buf, 0, 4096);
#ifdef HAVE_BIG_ENDIAN	   
	   /* eh contains the recovered ecc header in native endian format,
	      which is what we need it in, but it must be written out in
	      little endian. So we have to create an extra copy for writing
	      out when on a big endian machine. */

	   memcpy(buf, *eh, 4096);
	   le_eh = (EccHeader*)buf;
           SwapEccHeaderBytes(le_eh);
   	   le_eh->selfCRC = 0x47504c00;
	   le_eh->selfCRC = Crc32((unsigned char*)buf, 4096);
#endif
	   if(LargeSeek(ecc_file, 0))
	   {  if(LargeWrite(ecc_file, le_eh, 4096))
	        success=1;
	   }

	   if(success) Verbose("** Missing ecc header rewritten\n");
	   else        Verbose("** Note: Could not rewrite ecc header!\n");
	 }
 	 return ECCFILE_PRESENT;
      }
   }	
	
   /* Still nothing found. */

   g_free(*eh);
   *eh=NULL;
   Verbose("RS03RecognizeFile(): no ecc found\n");
   return ECCFILE_INVALID;
}

/***
 *** Recognize RS03 error correction data in the image
 ***/

#if 0
static int read_fingerprint(LargeFile *file, unsigned char *fingerprint, gint64 sector)
{  struct MD5Context md5ctxt;
   unsigned char buf[2048];
   int n;

   if(!LargeSeek(file, 2048LL*sector))
     return FALSE;

   n = LargeRead(file, buf, 2048);

   if(n != 2048) return FALSE;

   if(CheckForMissingSector(buf, sector, NULL, 0) != SECTOR_PRESENT)
     return FALSE;

   MD5Init(&md5ctxt);
   MD5Update(&md5ctxt, buf, 2048);
   MD5Final(fingerprint, &md5ctxt);

   return TRUE;
}
#endif

static EccHeader* valid_header(unsigned char *buf, gint64 hdr_pos, int image_expected)
{  EccHeader *eh = (EccHeader*)buf;
   guint32 recorded_crc, real_crc;
   //   unsigned char fingerprint[16];

   /* Medium read error in ecc header? */

   if(   (CheckForMissingSector(buf, hdr_pos, NULL, 0) != SECTOR_PRESENT)
      || (CheckForMissingSector(buf+2048, hdr_pos+1, NULL, 0) != SECTOR_PRESENT))
     return NULL;

   /* See if the magic cookie is there */

   if(   strncmp((char*)eh->cookie, "*dvdisaster*", 12)
	 || strncmp((char*)eh->method, "RS03", 4))
     return NULL;

   /* Examine the checksum */

   recorded_crc = eh->selfCRC;

#ifdef HAVE_BIG_ENDIAN
   eh->selfCRC = 0x47504c00;
#else
   eh->selfCRC = 0x4c5047;
#endif
   real_crc = Crc32((unsigned char*)eh, 4096);

   if(real_crc != recorded_crc)
     return NULL;

   /* If an ecc file header is found in the image (which might
      rightfully contain ecc files), ignore it */

   if(image_expected && (eh->methodFlags[0] & MFLAG_ECC_FILE))
   {  Verbose(".. Ecc file header in image - IGNORED\n");
      return NULL;
   }
   
   /* Check the fingerprint */

   eh = g_malloc(sizeof(EccHeader));
   memcpy(eh, buf, sizeof(EccHeader));
#ifdef HAVE_BIG_ENDIAN
   SwapEccHeaderBytes(eh);
#endif
   eh->selfCRC = recorded_crc;

#if 0  //FIXME
   status = read_fingerprint(file, fingerprint, eh->fpSector);

   if(!status)  /* be optimistic if fingerprint sector is unreadable */
     return eh;

   if(!memcmp(fingerprint, eh->mediumFP, 16))  /* good fingerprint */
     {  printf("RS03 header found\n");
      return eh;
     }
   g_free(eh);
#endif

   return eh;
}

EccHeader* FindRS03HeaderInImage(Image *image)
{  EccHeader *eh = NULL;
   gint64 hdr_pos;
   IsoInfo *ii; 
   unsigned char buf[4096];

   switch(image->type)
   { case IMAGE_FILE:
       Verbose("FindRS03HeaderInImage: file %s\n", image->file->path);
       break;

     case IMAGE_MEDIUM:
       Verbose("FindRS03HeaderInImage: medium %s\n", image->dh->device);
       break;

     default:
       Verbose("FindRS03HeaderInImage: unknown type %d\n", image->type);
       break;
   }

   /*** Try to find the header behind the ISO image */

   ii = image->isoInfo;
   if(!ii) Verbose(" . NO ISO structures found!\n");

   if(ii)
   {  hdr_pos = ii->volumeSize;

      if(ImageReadSectors(image, buf, hdr_pos, 2) == 2)
	{  eh = valid_header(buf, hdr_pos, TRUE);
	 if(eh) 
	 { Verbose("FindRS03HeaderInImage(): Header found at pos +0\n"); 
	   return eh;
	 }
      }

      hdr_pos = ii->volumeSize - 150;
      if(ImageReadSectors(image, buf, hdr_pos, 2) == 2)
	{  eh = valid_header(buf, hdr_pos, TRUE);
	 if(eh) 
	 { Verbose("FindRS03HeaderInImage(): Header found at pos -150\n"); 
	   return eh;
	 }
      }
   }
      
   return NULL;
}

typedef struct
{  AlignedBuffer *layer[256];
   AlignedBuffer *ab;
   RS03Layout *layout[256];
   int layer_checked[256];
} recognize_context;

static void free_recognize_context(recognize_context *rc)
{  int i;

   if(rc->ab) FreeAlignedBuffer(rc->ab);

   for(i=0; i<255; i++)
   {  if(rc->layer[i])
         FreeAlignedBuffer(rc->layer[i]);
      if(rc->layout[i])
	 g_free(rc->layout[i]);
   }
   g_free(rc);
}

/*
 * Search for valid CRC blocks assuming a given layer_size.
 * Tries all possible ndata values (84-247) for the given layer_size.
 * Returns TRUE and sets image->eccHeader if a valid CRC block is found.
 * maxtries: maximum number of sector reads (-1 = unlimited).
 * trynumber_inout: pointer to the running try counter (shared across calls).
 */

static int search_crc_blocks_for_layer_size(Image *image, guint64 image_sectors,
					    guint64 layer_size, gint64 maxtries,
					    gint64 *trynumber_inout)
{  recognize_context *rc = g_malloc0(sizeof(recognize_context));
   int untested_layers;
   int layer, layer_sector;
   int i;

   Verbose(".. trying layer size %" PRId64 "\n", (gint64)layer_size);
   Verbose("Scanning layers for signatures.\n");

   /* Prepare layout for all possible cases (8..170 roots) */

   for(i=84; i<=247; i++)  /* allowed range of ndata */
   {  RS03Layout *lay;
      rc->layout[i] = lay = g_malloc0(sizeof(RS03Layout));
      lay->eh = NULL;
      lay->dataSectors = (i-1)*layer_size-2;
      lay->dataPadding = 0;
      lay->totalSectors = GF_FIELDMAX*layer_size;
      lay->sectorsPerLayer = layer_size;
      lay->mediumCapacity = 0;
      lay->eccHeaderPos = lay->dataSectors;
      lay->firstCrcPos = (i-1)*layer_size;
      lay->firstEccPos = i*layer_size;
      lay->nroots = GF_FIELDMAX-i;
      lay->ndata = i;
      lay->inLast = 2048;
      lay->target = ECC_IMAGE;
   }
   untested_layers = 247-84+1;

   rc->ab = CreateAlignedBuffer(2048);

   for(layer_sector = 0; layer_sector < layer_size; layer_sector++)
   {  CrcBlock *cb = (CrcBlock*)rc->ab->buf;

      Verbose("- layer slice %d\n", layer_sector);
      for(layer = 84; layer <= 247; layer++)
      {  if(!rc->layer_checked[layer])
	 {  gint64 sector;
	    int crc_state;

	    sector = RS03SectorIndex(rc->layout[layer], layer, layer_sector);

	    /* reading beyond the image won't yield anything */
	    if(sector >= image_sectors)
	      goto mark_invalid_layer;

            if (++(*trynumber_inout) > maxtries && maxtries > 0) {
                Verbose("RS03: max tries reached, stopping search\n");
                free_recognize_context(rc);
                return FALSE;
            }

            Verbose("RS03: %s = %" PRId64 ", reading sector %" PRId64 "\n",
		    maxtries < 0 ? "try number" : "tries left",
		    maxtries < 0 ? *trynumber_inout : maxtries - *trynumber_inout,
		    sector);

	    switch(image->type)
	    {  case IMAGE_FILE:
		 RS03ReadSectors(image, rc->layout[layer], rc->ab->buf,
				 layer, layer_sector, 1, RS03_READ_ALL);
		 if(CheckForMissingSector(rc->ab->buf, sector, NULL, 0) != SECTOR_PRESENT)
		    continue;  /* unreadble -> can't decide */
		 break;

	       case IMAGE_MEDIUM:
	       {  int n;
		  n = ImageReadSectors(image, rc->ab->buf, sector, 1);
		  if(!n)
		    continue; /* unreadble -> can't decide */
	       }
	    }

	    /* CRC header found? */

	    crc_state = valid_crc_block(rc->ab->buf, sector, TRUE);
	    if(crc_state)
	    {  int nroots=255-layer-1;

	       if(crc_state == 1) /* corrupted crc header, try this layer again later */
		 continue;
	       Verbose("** Success: sector %" PRId64 ", rediscovered format with %d roots\n",
		       sector, nroots);
	       image->eccHeader = g_malloc(sizeof(EccHeader));
	       ReconstructRS03Header(image->eccHeader, cb);
	       free_recognize_context(rc);
	       return TRUE;
	    }

	    /* Sector readable but not a CRC header -> skip this layer */

mark_invalid_layer:
	    if(!rc->layer_checked[layer])
	    {  rc->layer_checked[layer] = 1;
	       untested_layers--;
	    }
	    if(untested_layers <= 0)
	    {  Verbose("** All layers tested -> no RS03 data found\n");
	       free_recognize_context(rc);
	       return FALSE;
	    }
	 }
      }
      Verbose("-> %d untested layers remaining\n", untested_layers);
   }

   Verbose("-- layer size %" PRId64 " exhausted; %d layers remain untested\n",
	   (gint64)layer_size, untested_layers);
   free_recognize_context(rc);
   return FALSE;
}

/*
 * Bruteforce linear scan for valid CRC blocks.
 * Reads every sector from ~33% of the image onwards, looking for the
 * CRC block magic signature. When a candidate is found, it cross-validates
 * by checking that the block's layout parameters are self-consistent and
 * that a second CRC block exists at a predicted position.
 * Returns TRUE and sets image->eccHeader if a valid CRC block is found.
 */

static int bruteforce_scan_for_crc_blocks(Image *image, guint64 image_sectors)
{  AlignedBuffer *ab = CreateAlignedBuffer(2048);
   AlignedBuffer *ab2 = CreateAlignedBuffer(2048);
   guint64 start_sector, sector;
   int found = FALSE;

   /* CRC layer starts at (ndata-1)*sectorsPerLayer.
      Since ndata >= 84, the earliest start is at 83/255 ~ 32.5% of the image.
      Start scanning from 30% to have some margin. */

   start_sector = (image_sectors * 30) / 100;
   Verbose("RS03 bruteforce scan: scanning sectors %" PRId64 " to %" PRId64 "\n",
	   (gint64)start_sector, (gint64)image_sectors);

   for(sector = start_sector; sector < image_sectors; sector++)
   {  int crc_state;
      CrcBlock *cb;

      if(sector % 100000 == 0)
	 Verbose("RS03 bruteforce: scanning sector %" PRId64 " (%.1f%%)\n",
		 (gint64)sector, (100.0 * sector) / image_sectors);

      switch(image->type)
      {  case IMAGE_FILE:
	    if(!LargeSeek(image->file, 2048LL * sector))
	       continue;
	    if(LargeRead(image->file, ab->buf, 2048) != 2048)
	       continue;
	    if(CheckForMissingSector(ab->buf, sector, NULL, 0) != SECTOR_PRESENT)
	       continue;
	    break;

	 case IMAGE_MEDIUM:
	    if(!ImageReadSectors(image, ab->buf, sector, 1))
	       continue;
	    break;

	 default:
	    continue;
      }

      crc_state = valid_crc_block(ab->buf, sector, TRUE);
      if(crc_state != 2)
	 continue;

      /* We found a valid CRC block at this sector.
	 Extract layout parameters and cross-validate. */

      cb = (CrcBlock*)ab->buf;

      {  guint64 spl, data_sectors, first_crc_pos;
	 int nroots, ndata;
	 CrcBlock cb_copy;

	 /* Make a local copy before potential byte-swapping */
	 memcpy(&cb_copy, cb, sizeof(CrcBlock));
#ifdef HAVE_BIG_ENDIAN
	 SwapCrcBlockBytes(&cb_copy);
#endif
	 spl = cb_copy.sectorsPerLayer;
	 nroots = cb_copy.eccBytes;
	 data_sectors = cb_copy.dataSectors;

	 /* Basic sanity: nroots must be in valid range */
	 if(nroots < 8 || nroots > 170)
	 {  Verbose("RS03 bruteforce: sector %" PRId64 " has valid CRC block but invalid nroots=%d, skipping\n",
		    (gint64)sector, nroots);
	    continue;
	 }

	 ndata = GF_FIELDMAX - nroots;

	 /* Sanity: sectorsPerLayer must be positive and reasonable */
	 if(spl == 0 || spl > image_sectors)
	 {  Verbose("RS03 bruteforce: sector %" PRId64 " has invalid sectorsPerLayer=%" PRId64 ", skipping\n",
		    (gint64)sector, (gint64)spl);
	    continue;
	 }

	 /* Check that this sector falls within the expected CRC layer */
	 first_crc_pos = (guint64)(ndata - 1) * spl;
	 if(sector < first_crc_pos || sector >= first_crc_pos + spl)
	 {  Verbose("RS03 bruteforce: sector %" PRId64 " not in expected CRC layer [%" PRId64 ", %" PRId64 "), skipping\n",
		    (gint64)sector, (gint64)first_crc_pos, (gint64)(first_crc_pos + spl));
	    continue;
	 }

	 /* Cross-validate: try to read another CRC block at a different position
	    in the same CRC layer */
	 {  guint64 check_sector;
	    int cross_valid = FALSE;

	    /* Pick a sector in the CRC layer that is different from the one we found */
	    check_sector = first_crc_pos + (sector == first_crc_pos ? 1 : 0);
	    if(check_sector < image_sectors)
	    {  int n_ok = 0;

	       switch(image->type)
	       {  case IMAGE_FILE:
		     if(LargeSeek(image->file, 2048LL * check_sector))
			n_ok = (LargeRead(image->file, ab2->buf, 2048) == 2048);
		     break;
		  case IMAGE_MEDIUM:
		     n_ok = ImageReadSectors(image, ab2->buf, check_sector, 1);
		     break;
	       }

	       if(n_ok && valid_crc_block(ab2->buf, check_sector, TRUE) == 2)
	       {  CrcBlock cb2_copy;
		  memcpy(&cb2_copy, ab2->buf, sizeof(CrcBlock));
#ifdef HAVE_BIG_ENDIAN
		  SwapCrcBlockBytes(&cb2_copy);
#endif
		  /* Verify that both CRC blocks agree on layout parameters */
		  if(cb2_copy.sectorsPerLayer == spl
		     && cb2_copy.eccBytes == nroots
		     && cb2_copy.dataSectors == data_sectors)
		     cross_valid = TRUE;
	       }
	    }

	    if(!cross_valid)
	    {  Verbose("RS03 bruteforce: sector %" PRId64 " cross-validation failed, skipping\n",
		       (gint64)sector);
	       continue;
	    }
	 }

	 /* Cross-validation passed. Reconstruct the header. */
	 Verbose("** RS03 bruteforce success: sector %" PRId64 ", format with %d roots, "
		 "sectorsPerLayer=%" PRId64 "\n",
		 (gint64)sector, nroots, (gint64)spl);
	 image->eccHeader = g_malloc(sizeof(EccHeader));
	 ReconstructRS03Header(image->eccHeader, cb);
	 found = TRUE;
	 break;
      }
   }

   FreeAlignedBuffer(ab);
   FreeAlignedBuffer(ab2);
   return found;
}

int RS03RecognizeImage(Image *image)
{  guint64 image_sectors;
   guint64 layer_size;
   gint64 trynumber;
   gint64 maxtries;

   switch(image->type)
   { case IMAGE_FILE:
       Verbose("RS03RecognizeImage: file %s\n", image->file->path);
       if(image->eccFile)
	 Stop("Internal error: RS03RecognizeImage() called with ecc file\n");
       image_sectors = image->sectorSize;
       break;

     case IMAGE_MEDIUM:
       Verbose("RS03RecognizeImage: medium %s\n", image->dh->device);
       image_sectors = MAX(image->dh->readCapacity, image->dh->userAreaSize);
       break;

     default:
       Verbose("RS03RecognizeImage: unknown type %d\n", image->type);
       return FALSE;
       break;
   }

   /* Easy shot: Locate the ecc header in the image */

   if (!Closure->debugMode || !Closure->ignoreRS03header)
   { image->eccHeader = FindRS03HeaderInImage(image);

     if(image->eccHeader)
        return TRUE;
   }

   /* This concludes the non-exhaustive search, where we tried to look for
      an ECC header signature on the sector right after the end of the ISO
      data. This doesn't always work, as some software tend to add some sectors
      after the end of the ISO (ImgBurn does this), or because the medium doesn't
      have any ISO9600 structure at all (some have only UDF for example), in that
      case the above quick search just does nothing.

      By default, we don't launch an exhaustive search unless asked for.
      For example on the medium-info page, we won't do it unless enabled in the options,
      as the inserted medium might not have RS02 nor RS03 at all.
      Of course, when doing verify or repair, as it implies the user knows there is
      some ECC correction available on the medium, our caller will always require
      an exhaustive search. It's also always enabled if we're not reading from a
      drive but from a file on the hard drive, as seeking is very fast.

      However even if not asked for an exhaustive search, and due to what has been
      explained in the first paragraph, we'll always try to read at least 3 sectors
      using the exhaustive search mechanism. On most images having ECC data, we'll
      find the header on the first try, at least on easy cases. This is a tradeoff
      to avoid having to display "no ECC data" on the medium-info page just because
      we didn't bother looking for it too hard, without bringing in the full
      exhaustive search which can take seconds or minutes on an optical drive with
      a medium that, in the end, doesn't have any ECC data.
   */

   if(!Closure->examineRS03 && image->type == IMAGE_MEDIUM)
   {  maxtries = 3; /* no exhaustive search asked and reading from optical drive */
      Verbose("RS03RecognizeImage: quick RS03 search, attempting up to %" PRId64" sector reads max\n", maxtries);
   }
   else
   {  maxtries = -1; /* infinity */
      Verbose("RS03RecognizeImage: No EH, entering exhaustive search\n");
   }

   /* Determine image size in augmented case and try known medium sizes. */

   trynumber = 0;

   if(Closure->mediumSize > 170)
   {  layer_size = Closure->mediumSize/GF_FIELDMAX;
      Verbose("Warning: image size set to %" PRId64 " for debugging!\n", Closure->mediumSize);
   }
   else
   {
      const guint64 bd_sl_sz = (Closure->noBdrDefectManagement ? BD_SL_SIZE_NODM : BD_SL_SIZE);
      const guint64 bd_dl_sz = (Closure->noBdrDefectManagement ? BD_DL_SIZE_NODM : BD_DL_SIZE);
      const guint64 bd_tl_sz = (Closure->noBdrDefectManagement ? BDXL_TL_SIZE_NODM : BDXL_TL_SIZE);
      const guint64 bd_ql_sz = (Closure->noBdrDefectManagement ? BDXL_QL_SIZE_NODM : BDXL_QL_SIZE);
      if(image_sectors < CDR_SIZE)         layer_size = CDR_SIZE/GF_FIELDMAX;
      else if(image_sectors < DVD_SL_SIZE) layer_size = DVD_SL_SIZE/GF_FIELDMAX;
      else if(image_sectors < DVD_DL_SIZE) layer_size = DVD_DL_SIZE/GF_FIELDMAX;
      else if(image_sectors < bd_sl_sz)
         layer_size = bd_sl_sz/GF_FIELDMAX;
      else if(image_sectors < bd_dl_sz)
         layer_size = bd_dl_sz/GF_FIELDMAX;
      else if(image_sectors < bd_tl_sz)
         layer_size = bd_tl_sz/GF_FIELDMAX;
      else layer_size = bd_ql_sz/GF_FIELDMAX;
   }

   if(search_crc_blocks_for_layer_size(image, image_sectors, layer_size, maxtries, &trynumber))
      return TRUE;

   /* Phase 1: If the known medium size didn't work and we're in exhaustive mode,
      try deriving layer_size directly from the image file size.
      For a complete RS03 augmented image, totalSectors = 255 * sectorsPerLayer,
      so sectorsPerLayer = image_sectors / 255. This handles images created with
      a custom -n value. Try the computed value and ±1 for rounding. */

   if(maxtries < 0)  /* only in exhaustive mode */
   {  guint64 derived_layer_size = image_sectors / GF_FIELDMAX;
      int offset;

      Verbose("RS03RecognizeImage: trying image-derived layer sizes\n");

      for(offset = -1; offset <= 1; offset++)
      {  guint64 try_size = derived_layer_size + offset;

	 if(try_size == 0 || try_size == layer_size)
	    continue;  /* skip zero or already-tried size */

	 if(search_crc_blocks_for_layer_size(image, image_sectors, try_size, maxtries, &trynumber))
	    return TRUE;
      }
   }

   /* Phase 2: Bruteforce linear scan for CRC blocks.
      This scans every sector in the image looking for the CRC block
      magic signature. Very slow, but handles any custom -n value even
      when the image is truncated. Only runs when explicitly requested. */

   if(Closure->bruteforceRS03Search && maxtries < 0)
   {  Verbose("RS03RecognizeImage: entering bruteforce linear scan\n");
      if(bruteforce_scan_for_crc_blocks(image, image_sectors))
	 return TRUE;
   }

   Verbose("RS03RecognizeImage: no RS03 data found\n");
   return FALSE;
}

