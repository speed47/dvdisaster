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

#include "scsi-layer.h"
#include "rs02-includes.h"

/***
 *** Create a CRC buffer ready for accumulating CRC and MD5 sums
 ***/

CrcBuf *CreateCrcBuf(Image *image)
{  CrcBuf *cb = g_malloc0(sizeof(CrcBuf));

  /* MD5 sum related data */
  
   cb->md5Ctxt = g_malloc(sizeof(struct MD5Context));
   MD5Init(cb->md5Ctxt);
   cb->md5State = MD5_BUILDING;
   cb->lastSector = 0;
   memset(cb->dataMD5sum, 0, 16);
   memset(cb->imageMD5sum, 0, 16);

   /* image identification */
   
   if(image->type == IMAGE_FILE)
      cb->imageName = g_strdup(image->file->path);

   /* For augmented images we need both the pure data size and the full size
      in order to compute MD5 sums for both. Depending on what the user wants
      to do with the image (create an ecc file or re-augment the image) we'll
      need the first or the last version. 
      There's one catch, though: If the augmented image is accompanied with
      an ecc file, we must treat the image as raw data; e.g. do not set 
      the data size to the portion protected by the augmented ecc.
      Otherwise, the CRC sums from the ecc file which do also protect
      the augmented ecc part will become unreachable. */
   
   if(image->eccHeader && !image->eccFileHeader)  /* augmented image, and no ecc file? */
   {  cb->dataSectors = uchar_to_gint64(image->eccHeader->sectors);
      if(image->type == IMAGE_FILE)
           cb->allSectors = image->sectorSize;
      else cb->allSectors = image->dh->sectors;
   }
   else
   {  if(image->type == IMAGE_FILE)
            cb->dataSectors = cb->allSectors = image->sectorSize;
      else  cb->dataSectors = cb->allSectors = image->dh->sectors;
   }

   /* Note: The following statement is not correct for RS03.
      It does not hurt since RS03 will set the correct value when
      the CrcBuf is created via RS03GetCrcBuf(), and in all other
      cases it does currently not matter. Especially, CRC sums 
      created during an image read are not used when subsequently
      creating new ecc data. */
   
   cb->coveredSectors = cb->dataSectors;

   /* Extract the fingerprint */

   if(image->fpState == FP_PRESENT)
   {  memcpy(cb->mediumFP, image->imageFP, 16);
      cb->fpValid = TRUE;
   }
   cb->fpSector = image->fpSector;

   /* CRC sum array */
   
   cb->crcbuf  = g_malloc(cb->allSectors * sizeof(guint32));
   cb->crcSize = cb->allSectors;
   cb->valid   = CreateBitmap0(cb->allSectors);
   
   image->crcCache = cb;
   return cb;
}

/*
 * Add a 2048 byte block into the checksum buffer
 */

int AddSectorToCrcBuffer(CrcBuf *cb, int mode, guint64 idx, unsigned char *buf, int buf_size)
{  guint32 crc;

   if(idx < 0 || idx >= cb->crcSize)
     return CRC_OUTSIDE_BOUND;

   /* Update the CRC sums */

   if(   (mode & CRCBUF_UPDATE_CRC)
      || ((mode & CRCBUF_UPDATE_CRC_AFTER_DATA) && idx >= cb->coveredSectors))
   {  crc = Crc32(buf, 2048);  /* should be buf_size, but remains at 2048 for backwards compatibility. */
      cb->crcbuf[idx] = crc;   /* does not harm except that the last sector is padded with the contents */
      SetBit(cb->valid, idx);  /* of the previous sector when reading an image file whole size is not */
   }                           /* a multiple of 2048 */
   
   /* Update the MD5 sums */

   if(!(mode & CRCBUF_UPDATE_MD5))
     return CRC_GOOD;
   
   if(cb->lastSector != idx)  /* sector out of order -> md5sum dead */
   {  cb->md5State = MD5_INVALID;
     return CRC_BAD;
   }
   cb->lastSector++;

   if(idx <= cb->allSectors-1)
      MD5Update(cb->md5Ctxt, buf, buf_size);

   if(idx == cb->dataSectors-1)
   {  MD5Context *dataCtxt = alloca(sizeof(MD5Context));

      memcpy(dataCtxt, cb->md5Ctxt, sizeof(MD5Context));
      MD5Final(cb->dataMD5sum, dataCtxt);
      cb->md5State |= MD5_DATA_COMPLETE;
   }
      
   if(idx == cb->allSectors-1)
   {  MD5Final(cb->imageMD5sum, cb->md5Ctxt);
      cb->md5State |= MD5_IMAGE_COMPLETE;
      cb->md5State &= ~MD5_BUILDING;
   }

   return CRC_GOOD;
}
   
/*
 * Test a 2048 byte block against the checksum in the buffer
 */

int CheckAgainstCrcBuffer(CrcBuf *cb, gint64 idx, unsigned char *buf)
{  guint32 crc;

   if(idx < 0 || idx >= cb->crcSize)
     return CRC_OUTSIDE_BOUND;
   
   crc = Crc32(buf, 2048);

   if(!GetBit(cb->valid, idx))
      return CRC_UNKNOWN;
   
   if(crc == cb->crcbuf[idx])
      return CRC_GOOD;

   return CRC_BAD;
}

/*
 * Make sure that the current image and ecc file match the
 * cached CRC and md5 informaton
 */

int CrcBufValid(CrcBuf *crcbuf, Image *image, int mode)
{
  if(!crcbuf)
  {  Verbose("CrcBufValid: crcbuf==NULL\n");
     return FALSE;
  }

  if(!image)
  {  Verbose("CrcBufValid: image==NULL\n");
     return FALSE;
  }
  
  /* if still in building state we do not have all CRC sums */

  if(crcbuf->md5State & MD5_BUILDING)
  {  Verbose("CrcBufValid: NO, still building\n");
     return FALSE;
  }

  /* presence of one MD5 sum suffices (data md5sum may not be present
     under some circumstances) */
  
  if((!(crcbuf->md5State & MD5_COMPLETE)))
  {  Verbose("CrcBufValid: NOT complete\n");
     return FALSE;
  }

  /* compare fingerprints of buffer and image */

  if(image->fpState != 2)
  {  Verbose("CrcBufValid: image fingerprint not valid (%d)\n",
	     image->fpState);
     return FALSE;
  }

  if(!crcbuf->fpValid)
  {  Verbose("CrcBufValid: crcbuf fingerprint not valid\n");
     return FALSE;
  }

  if(crcbuf->fpSector != image->fpSector)
  {  Verbose("CrcBufValid: crcbuf/image have different fingerprint sectors (%d/%" PRId64 ")\n",
	     crcbuf->fpSector, image->fpSector);
     return FALSE;
  }
  
  if(memcmp(crcbuf->mediumFP, image->imageFP, 16))
  {  Verbose("CrcBufValid: crcbuf/image have different fingerprints\n");
     return FALSE;
  }

  /* additionally, compare the image sizes */

  switch(mode)
  {  case FULL_IMAGE:
       if(image->sectorSize != crcbuf->allSectors)
       {  Verbose("CrcBufValid(..., FULL_IMAGE): crcbuf/image have different size (%" PRId64 "/%" PRId64 " sectors)\n",
		  image->sectorSize, crcbuf->allSectors);
	  return FALSE;
       }
       break;
     case DATA_SECTORS_ONLY:
       if(image->sectorSize != crcbuf->dataSectors)
       {  Verbose("CrcBufValid(..., DATA_SECTORS_ONLY): crcbuf/image have different size (%" PRId64 "/%" PRId64 " sectors)\n",
		  image->sectorSize, crcbuf->dataSectors);
	  return FALSE;
       }
       break;
  }
  
  Verbose("CrcBufValid: buffer VALID\n");
  return TRUE;
}

/***
 *** Clean up
 ***/

void FreeCrcBuf(CrcBuf *cb)
{  
   if(!cb)
   {  Verbose("FreeCrcBuf - nothing to do\n");
      return;
   }
   
   g_free(cb->crcbuf);
   FreeBitmap(cb->valid);
   if(cb->imageName)
     g_free(cb->imageName);
   if(cb->md5Ctxt)
     g_free(cb->md5Ctxt);
   g_free(cb);

   Verbose("FreeCrcBuf - buffer cleared\n");
}

/***
 *** Debugging output 
 ***/

void PrintCrcBuf(CrcBuf *cb)
{  char digest[33];
  guint64 i,missing=0;
  
   if(!Closure->verbose)
     return;

   PrintLog("CrcBuf contents, image path %s:\n", cb->imageName ? cb->imageName : "none (medium)" );
   PrintLog("  crcSize: %" PRId64 ", dataSectors: %" PRId64 ", coveredSectors: %" PRId64 ", allSectors: %" PRId64 "\n",
	    cb->crcSize, cb->dataSectors, cb->coveredSectors, cb->allSectors);

   PrintLog("  md5State:");
   if(cb->md5State)
   {  if(cb->md5State & MD5_BUILDING) PrintLog(" building");
      if(cb->md5State & MD5_DATA_COMPLETE) PrintLog(" data_complete");
      if(cb->md5State & MD5_IMAGE_COMPLETE) PrintLog(" image_complete");
   }
   else PrintLog(" invalid");
   PrintLog("\n");

   if(cb->md5State & MD5_COMPLETE)
   {  AsciiDigest(digest, cb->dataMD5sum);
      PrintLog("  data: %s\n", digest);
      AsciiDigest(digest, cb->imageMD5sum);
      PrintLog("  full: %s\n", digest);
   }
   AsciiDigest(digest, cb->mediumFP);
   if(cb->fpValid)
        PrintLog("  fp sector: %d; %s\n", cb->fpSector, digest);
   else PrintLog("  fp sector: %d; invalid\n", cb->fpSector);

   for(i=0; i<cb->crcSize; i++)
     if(!GetBit(cb->valid, i))
     {   missing++;
     }
   PrintLog("  missing crcs: %" PRId64 "\n", missing);
}
