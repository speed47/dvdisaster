/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2004-2010 Carsten Gnoerlich.
 *  Project home page: http://www.dvdisaster.com
 *  Email: carsten@dvdisaster.com  -or-  cgnoerlich@fsfe.org
 *
 *  The Reed-Solomon error correction draws a lot of inspiration - and even code -
 *  from Phil Karn's excellent Reed-Solomon library: http://www.ka9q.net/code/fec/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA,
 *  or direct your browser at http://www.gnu.org.
 */

#include "dvdisaster.h"

#include "rs01-includes.h"

/***
 *** Read an image sector from the .iso file.
 ****
 * Two special cases here:
 * - Missing sectors (beyond the range recorded in eh->sectors) will be padded with zeros,
 *   since we need a multiple of ndata sectors for the parity generation. 
 * - Missing sectors beyond the range recorded in ii->sectors, but before the real end
 *   as defined above are treated as "dead sectors".
 */

void RS01ReadSector(ImageInfo *ii, EccHeader *eh, unsigned char *buf, gint64 s)
{ gint64 eh_sectors = uchar_to_gint64(eh->sectors);

  if(s >= ii->sectors && s < eh_sectors)
  {
     CreateMissingSector(buf, s, NULL, 0, NULL); /* truncated image */
  }
  else if(s >= eh_sectors)      
  {
     memset(buf, 0, 2048);            /* zero padding for reads past the image */
  }
  else                                /* else normal read within the image */
  {  int n,expected;
	
     if(!LargeSeek(ii->file, (gint64)(2048*s)))
       Stop(_("Failed seeking to sector %lld in image: %s"),
	    s, strerror(errno));

     /* Prepare for short reads at the last image sector.
	Doesn't happen for CD and DVD media, but perhaps for future media? */

     if(s < ii->sectors-1) expected = 2048;
     else  
     {  memset(buf, 0, 2048);
        expected = ii->inLast;
     }

     /* Finally, read the sector */

     n = LargeRead(ii->file, buf, expected);
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

void RS01ScanImage(Method *method, ImageInfo *ii, EccInfo *ei, int mode)
{  RS01Widgets *wl = NULL;
   EccHeader eh; 
   unsigned char buf[2048];
   guint32 *crcbuf = NULL;
   int crcidx = 0;
   struct MD5Context image_md5;
   gint64 eh_sectors = 0;
   gint64 s, first_missing, last_missing;
   gint64 prev_missing = 0;
   gint64 prev_crc_errors = 0;
   int last_percent,current_missing;
   int fp_sector = FINGERPRINT_SECTOR;
   char *msg;

   /* Extract widget list from method */

   if(method->widgetList)
     wl = (RS01Widgets*)method->widgetList;

   /* Read the ecc file header */

   if(ei && mode != CREATE_CRC)
   {   LargeSeek(ei->file, 0);
       LargeRead(ei->file, &eh, sizeof(EccHeader));
       eh_sectors = uchar_to_gint64(eh.sectors);
       fp_sector = eh.fpSector;
   }     

   /* Position behind the ecc file header,
      initialize CRC buffer pointers */

   if(ei)
   {  if(!LargeSeek(ei->file, (gint64)sizeof(EccHeader)))
         Stop(_("Failed skipping the ecc header: %s"),strerror(errno));

      crcbuf = g_malloc(sizeof(guint32) * CRCBUFSIZE);
      crcidx = (mode & CREATE_CRC) ? 0 : CRCBUFSIZE;
      MD5Init(&ei->md5Ctxt);            /* md5sum of CRC portion of ecc file */
   }

   /* Prepare for scanning the image and calculating its md5sum */

   MD5Init(&image_md5);              /* md5sum of image file itself */
   LargeSeek(ii->file, 0);            /* rewind image file */   
      
   if(mode & PRINT_MODE)
        msg = _("- testing sectors  : %3d%%");
   else msg = _("Scanning image sectors: %3d%%");

   last_percent = 0;
   ii->sectorsMissing = 0;
   first_missing = last_missing = -1;

   /* Go through all sectors and look for the "dead sector marker" */
   
   for(s=0; s<ii->sectors; s++)
   {  int n,percent,err;

      /* Check for user interruption */

      if(Closure->stopActions)   
      {  ii->sectorsMissing += ii->sectors - s;
	 if(crcbuf) g_free(crcbuf);
         return;
      }

      /* Read the next sector */

      n = LargeRead(ii->file, buf, 2048);
      if(n != 2048)
      {  if(s != ii->sectors - 1 || n != ii->inLast)
         {  if(crcbuf) g_free(crcbuf);
	    Stop(_("premature end in image (only %d bytes): %s\n"),n,strerror(errno));
         }
	 else /* Zero unused sectors for CRC generation */
	    memset(buf+ii->inLast, 0, 2048-ii->inLast);
      }

      /* Look for the dead sector marker */

      err = CheckForMissingSector(buf, s, ii->fpValid ? ii->mediumFP : NULL, FINGERPRINT_SECTOR);
      if(err != SECTOR_PRESENT)
      {    current_missing = TRUE;
	   ExplainMissingSector(buf, s, err, TRUE);
      }
      else current_missing = FALSE;

      if(current_missing)
      {  if(first_missing < 0) first_missing = s;
         last_missing = s;
	 ii->sectorsMissing++;
      }

      /* Report dead sectors. Combine subsequent missing sectors into one report. */

      if(mode & PRINT_MODE)
	if(!current_missing || s==ii->sectors-1)
	{  if(first_missing>=0)
	    {   if(first_missing == last_missing)
		     PrintCLI(_("* missing sector   : %lld\n"), first_missing);
		else PrintCLI(_("* missing sectors  : %lld - %lld\n"), first_missing, last_missing);
	      first_missing = -1;
	   }
	}

      if(ei)   /* Do something with the CRC portion of the .ecc file */
      {
	 /* If creation of the CRC32 is requested, do that. */

	 if(mode & CREATE_CRC)
	 {  crcbuf[crcidx++] = Crc32(buf, 2048);

	    if(crcidx >= CRCBUFSIZE)  /* write out CRC buffer contents */
	    {  size_t size = CRCBUFSIZE*sizeof(guint32);

	       MD5Update(&ei->md5Ctxt, (unsigned char*)crcbuf, size);
	       if(LargeWrite(ei->file, crcbuf, size) != size)
	       { if(crcbuf) g_free(crcbuf);
		 Stop(_("Error writing CRC information: %s"),strerror(errno));
	       }
	       crcidx = 0;
	    }
	 }

	 /* else do the CRC32 check. Missing sectors are skipped in the CRC report. */
	 
	 else if(s < eh_sectors)
	 {  guint32 crc = Crc32(buf, 2048); 

            /* If the CRC buf is exhausted, refill. */

            if(crcidx >= CRCBUFSIZE)
	    {  size_t remain = ii->sectors-s;
	       size_t size;

	       if(remain < CRCBUFSIZE)
		    size = remain*sizeof(guint32);
	       else size = CRCBUFSIZE*sizeof(guint32);

	       if(LargeRead(ei->file, crcbuf, size) != size)
	       { if(crcbuf) g_free(crcbuf);
		 Stop(_("Error reading CRC information: %s"),strerror(errno));
	       }
	       crcidx = 0;
	    }

	    if(crc != crcbuf[crcidx++] && !current_missing)
	    {  PrintCLI(_("* CRC error, sector: %lld\n"), s);
	       ii->crcErrors++;
	    }
	 }
      }

      MD5Update(&image_md5, buf, n);  /* update image md5sum */

      if(Closure->guiMode && mode & PRINT_MODE) 
	   percent = (VERIFY_IMAGE_SEGMENTS*(s+1))/ii->sectors;
      else percent = (100*(s+1))/ii->sectors;
      if(last_percent != percent) 
      {  PrintProgress(msg,percent);

         if(Closure->guiMode && mode & CREATE_CRC)
	   SetProgress(wl->encPBar1, percent, 100);

	 if(Closure->guiMode && mode & PRINT_MODE)
 	 {  RS01AddVerifyValues(method, percent, ii->sectorsMissing, ii->crcErrors,
				ii->sectorsMissing - prev_missing,
				ii->crcErrors - prev_crc_errors);

	    prev_missing = ii->sectorsMissing;
	    prev_crc_errors = ii->crcErrors;
	 }

	 last_percent = percent;
      }
   }

   /*** Flush the rest of the CRC buffer */

   if((mode & CREATE_CRC) && crcidx)
   {  size_t size = crcidx*sizeof(guint32);

      MD5Update(&ei->md5Ctxt, (unsigned char*)crcbuf, size);
      if(LargeWrite(ei->file, crcbuf, size) != size)
      {	if(crcbuf) g_free(crcbuf);
	Stop(_("Error writing CRC information: %s"),strerror(errno));
      }
   }

   /*** The image md5sum can only be calculated if all blocks have been successfully read. */

   MD5Final(ii->mediumSum, &image_md5);

   LargeSeek(ii->file, 0);
   if(crcbuf) g_free(crcbuf);
}
