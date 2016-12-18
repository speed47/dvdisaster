/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2004-2015 Carsten Gnoerlich.
 *
 *  Email: carsten@dvdisaster.org  -or-  cgnoerlich@fsfe.org
 *  Project homepage: http://www.dvdisaster.org
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

#include "dvdisaster.h"

#include "rs01-includes.h"

/***
 *** Recognize a RS01 error correction file
 ***/

int RS01Recognize(LargeFile *ecc_file, EccHeader **eh)
{  int n;

   *eh = g_malloc(sizeof(EccHeader));

   LargeSeek(ecc_file, 0);
   n = LargeRead(ecc_file, *eh, sizeof(EccHeader));

   if(n != sizeof(EccHeader))
   {  g_free(*eh);
      return ECCFILE_INVALID;
   }

   if(strncmp((char*)(*eh)->cookie, "*dvdisaster*", 12))
   {  g_free(*eh);
      return ECCFILE_DEFECTIVE_HEADER;
   }

   if(!strncmp((char*)(*eh)->method, "RS01", 4))
   {  
#ifdef HAVE_BIG_ENDIAN
      SwapEccHeaderBytes(*eh);
#endif
      return ECCFILE_PRESENT;
   }

   g_free(*eh);
   return ECCFILE_WRONG_CODEC;
}

/***
 *** Read and buffer CRC information from RS01 file 
 ***/

CrcBuf *RS01GetCrcBuf(Image *image)
{  LargeFile *file = image->eccFile;
   CrcBuf *cb;
   guint32 *buf;
   guint64 image_sectors;
   guint64 crc_sectors,crc_remainder;
   guint64 i,j,sec_idx;

   image_sectors = uchar_to_gint64(image->eccFileHeader->sectors);
   cb = CreateCrcBuf(image_sectors);
   buf = cb->crcbuf;

   /* Seek to beginning of CRC sums */

   if(!LargeSeek(file, (gint64)sizeof(EccHeader)))
      Stop(_("Failed skipping the ecc header: %s"),strerror(errno));

   /* Read crc sums. A sector of 2048 bytes contains 512 CRC sums. */

   crc_sectors = image_sectors / 512;
   sec_idx = 0;

   for(i=0; i<crc_sectors; i++)
   {  if(LargeRead(file, buf, 2048) != 2048)
	 Stop(_("Error reading CRC information: %s"),strerror(errno));
      buf += 512;

      for(j=0; j<512; j++, sec_idx++)
	 SetBit(cb->valid, sec_idx);
   }

   crc_remainder = sizeof(guint32)*(image_sectors % 512);
   if(crc_remainder)
   {  if(LargeRead(file, buf, crc_remainder) != crc_remainder)
	 Stop(_("Error reading CRC information: %s"),strerror(errno));

      for( ; sec_idx<image_sectors; sec_idx++)
	 SetBit(cb->valid, sec_idx);
   }

   return cb;
}

/***
 *** Internal checksum handling.
 ***
 * Not overly complicated as we just have a global md5sum.
 */

void RS01ResetCksums(Image *image)
{  RS01CksumClosure *csc = (RS01CksumClosure*)image->eccFileMethod->ckSumClosure;

   MD5Init(&csc->md5ctxt);
}

void RS01UpdateCksums(Image *image, gint64 sector, unsigned char *buf)
{  RS01CksumClosure *csc = (RS01CksumClosure*)image->eccFileMethod->ckSumClosure;

   MD5Update(&csc->md5ctxt, buf, 2048);
}

int RS01FinalizeCksums(Image *image)
{  Method *self = image->eccFileMethod;
   RS01CksumClosure *csc = (RS01CksumClosure*)self->ckSumClosure;
   guint8 image_fp[16];
   int good_fp;

   MD5Final(image_fp, &csc->md5ctxt);

   good_fp = !(memcmp(image_fp, image->eccFileHeader->mediumSum ,16));
   
   if(good_fp)
        return 0;
   else return DATA_MD5_BAD;
}

/***
 *** Read an image sector from the .iso file.
 ***
 * Two special cases here:
 * - Missing sectors (beyond the range recorded in eh->sectors) will be padded with zeros,
 *   since we need a multiple of ndata sectors for the parity generation. 
 * - Missing sectors beyond the range recorded in ii->sectors, but before the real end
 *   as defined above are treated as "dead sectors".
 */

void RS01ReadSector(Image *image, unsigned char *buf, gint64 s)
{ gint64 eh_sectors = uchar_to_gint64(image->eccFileHeader->sectors);

  if(s >= image->sectorSize && s < eh_sectors)
  {
     CreateMissingSector(buf, s, NULL, 0, NULL); /* truncated image */
  }
  else if(s >= eh_sectors)      
  {
     memset(buf, 0, 2048);            /* zero padding for reads past the image */
  }
  else                                /* else normal read within the image */
  {  int n,expected;
	
     if(!LargeSeek(image->file, (gint64)(2048*s)))
       Stop(_("Failed seeking to sector %lld in image: %s"),
	    s, strerror(errno));

     /* Prepare for short reads at the last image sector.
	Doesn't happen for CD and DVD media, but perhaps for future media? */

     if(s < image->sectorSize-1) expected = 2048;
     else  
     {  memset(buf, 0, 2048);
        expected = image->inLast;
     }

     /* Finally, read the sector */

     n = LargeRead(image->file, buf, expected);
     if(n != expected)
       Stop(_("Failed reading sector %lld in image: %s"),s,strerror(errno));
  }
}


/*
 * Scan the image for missing blocks.
 * If the ecc file is present, also compare the CRC sums.
 * If CREATE_CRC is requested, calculate the CRC sums.
 *
 * Actually this should be usable for all RS01 type ecc files.
 * But unless we have more than one codec, we'll label it as 
 * as RS01 specific method. 
 */

#define CRCBUFSIZE (1024*256)

void RS01ScanImage(Method *method, Image* image, struct MD5Context *ecc_ctxt, int mode)
{  RS01Widgets *wl = NULL;
   unsigned char buf[2048];
   guint32 *crcbuf = NULL;
   int unrecoverable_sectors = 0;
   int crcidx = 0;
   struct MD5Context image_md5;
   gint64 s, first_missing, last_missing;
   gint64 prev_missing = 0;
   gint64 prev_crc_errors = 0;
   int last_percent,current_missing;
   char *msg;

   /* Extract widget list from method */

   if(method->widgetList)
     wl = (RS01Widgets*)method->widgetList;

   /* Position behind the ecc file header,
      initialize CRC buffer pointers */

   if(image->eccFile)
   {  if(!LargeSeek(image->eccFile, (gint64)sizeof(EccHeader)))
         Stop(_("Failed skipping the ecc header: %s"),strerror(errno));

      crcbuf = g_malloc(sizeof(guint32) * CRCBUFSIZE);
      crcidx = (mode & CREATE_CRC) ? 0 : CRCBUFSIZE;
      if(mode & CREATE_CRC)
	 MD5Init(ecc_ctxt);            /* md5sum of CRC portion of ecc file */
   }

   /* Prepare for scanning the image and calculating its md5sum */

   MD5Init(&image_md5);              /* md5sum of image file itself */
   LargeSeek(image->file, 0);        /* rewind image file */   
      
   if(mode & PRINT_MODE)
        msg = _("- testing sectors  : %3d%%");
   else msg = _("Scanning image sectors: %3d%%");

   last_percent = 0;
   image->sectorsMissing = 0;
   first_missing = last_missing = -1;

   /* Go through all sectors and look for the "dead sector marker" */
   
   for(s=0; s<image->sectorSize; s++)
   {  int n,percent,err;

      /* Check for user interruption */

      if(Closure->stopActions)   
      {  image->sectorsMissing += image->sectorSize - s;
	 if(crcbuf) g_free(crcbuf);
         return;
      }

      /* Read the next sector */

      n = LargeRead(image->file, buf, 2048);
      if(n != 2048)
      {  if(s != image->sectorSize - 1 || n != image->inLast)
         {  if(crcbuf) g_free(crcbuf);
	    Stop(_("premature end in image (only %d bytes): %s\n"),n,strerror(errno));
         }
	 else /* Zero unused sectors for CRC generation */
	    memset(buf+image->inLast, 0, 2048-image->inLast);
      }

      /* Look for the dead sector marker */

      err = CheckForMissingSector(buf, s, image->fpState == 2 ? image->imageFP : NULL, 
				  FINGERPRINT_SECTOR);
      if(err != SECTOR_PRESENT)
      {    current_missing = TRUE;
	ExplainMissingSector(buf, s, err, SOURCE_IMAGE, &unrecoverable_sectors);
      }
      else current_missing = FALSE;

      if(current_missing)
      {  if(first_missing < 0) first_missing = s;
         last_missing = s;
	 image->sectorsMissing++;
      }

      /* Report dead sectors. Combine subsequent missing sectors into one report. */

      if(mode & PRINT_MODE)
	if(!current_missing || s==image->sectorSize-1)
	{  if(first_missing>=0)
	    {   if(first_missing == last_missing)
		     PrintCLI(_("* missing sector   : %lld\n"), first_missing);
		else PrintCLI(_("* missing sectors  : %lld - %lld\n"), first_missing, last_missing);
	      first_missing = -1;
	   }
	}

      if(image->eccFile) /* Do something with the CRC portion of the .ecc file */
      {
	 /* If creation of the CRC32 is requested, do that. */

	 if(mode & CREATE_CRC)
	 {  crcbuf[crcidx++] = Crc32(buf, 2048);

	    if(crcidx >= CRCBUFSIZE)  /* write out CRC buffer contents */
	    {  size_t size = CRCBUFSIZE*sizeof(guint32);

	       MD5Update(ecc_ctxt, (unsigned char*)crcbuf, size);
	       if(LargeWrite(image->eccFile, crcbuf, size) != size)
	       { if(crcbuf) g_free(crcbuf);
		 Stop(_("Error writing CRC information: %s"),strerror(errno));
	       }
	       crcidx = 0;
	    }
	 }

	 /* else do the CRC32 check. Missing sectors are skipped in the CRC report. */
	 
	 else if(s < image->expectedSectors)
	 {  guint32 crc = Crc32(buf, 2048); 

            /* If the CRC buf is exhausted, refill. */

            if(crcidx >= CRCBUFSIZE)
	    {  size_t remain = image->sectorSize-s;
	       size_t size;

	       if(remain < CRCBUFSIZE)
		    size = remain*sizeof(guint32);
	       else size = CRCBUFSIZE*sizeof(guint32);

	       if(LargeRead(image->eccFile, crcbuf, size) != size)
	       { if(crcbuf) g_free(crcbuf);
		 Stop(_("Error reading CRC information: %s"),strerror(errno));
	       }
	       crcidx = 0;
	    }

	    if(crc != crcbuf[crcidx++] && !current_missing)
	    {  PrintCLI(_("* CRC error, sector: %lld\n"), s);
	       image->crcErrors++;
	    }
	 }
      }

      MD5Update(&image_md5, buf, n);  /* update image md5sum */

      if(Closure->guiMode && mode & PRINT_MODE) 
	   percent = (VERIFY_IMAGE_SEGMENTS*(s+1))/image->sectorSize;
      else percent = (100*(s+1))/image->sectorSize;
      if(last_percent != percent) 
      {  PrintProgress(msg,percent);

         if(Closure->guiMode && mode & CREATE_CRC)
	   SetProgress(wl->encPBar1, percent, 100);

	 if(Closure->guiMode && mode & PRINT_MODE)
 	 {  RS01AddVerifyValues(method, percent, image->sectorsMissing, image->crcErrors,
				image->sectorsMissing - prev_missing,
				image->crcErrors - prev_crc_errors);

	    prev_missing = image->sectorsMissing;
	    prev_crc_errors = image->crcErrors;
	 }

	 last_percent = percent;
      }
   }

   /*** Flush the rest of the CRC buffer */

   if((mode & CREATE_CRC) && crcidx)
   {  size_t size = crcidx*sizeof(guint32);

      MD5Update(ecc_ctxt, (unsigned char*)crcbuf, size);
      if(LargeWrite(image->eccFile, crcbuf, size) != size)
      {	if(crcbuf) g_free(crcbuf);
	Stop(_("Error writing CRC information: %s"),strerror(errno));
      }
   }

   /*** The image md5sum can only be calculated if all blocks have been successfully read. */

   MD5Final(image->mediumSum, &image_md5);

   LargeSeek(image->file, 0);
   if(crcbuf) g_free(crcbuf);
}

/***
 *** Determine expected size of image
 ***/

guint64 RS01ExpectedImageSize(Image *image)
{  EccHeader *eh = image->eccFileHeader;
 
   if(!eh) return 0;

   return uchar_to_gint64(eh->sectors);
}
