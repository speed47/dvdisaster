/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2004-2010 Carsten Gnoerlich.
 *  Project home page: http://www.dvdisaster.com
 *  Email: carsten@dvdisaster.com  -or-  cgnoerlich@fsfe.org
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

#include "scsi-layer.h"
#include "rs02-includes.h"

/***
 *** Load crc buffer from RS01 error correction file
 ***/

CrcBuf *GetCRCFromRS01(EccInfo *ei)
{  CrcBuf *cb = g_malloc(sizeof(CrcBuf));
   guint32 *buf;
   gint64 crc_sectors,crc_remainder;
   gint64 i,j,sec_idx;

   cb->crcbuf = g_malloc(ei->sectors * sizeof(guint32));
   cb->size   = ei->sectors;
   cb->valid  = CreateBitmap0(ei->sectors);
   buf = cb->crcbuf;

   /* Seek to beginning of CRC sums */

   if(!LargeSeek(ei->file, (gint64)sizeof(EccHeader)))
      Stop(_("Failed skipping the ecc header: %s"),strerror(errno));

   /* Read crc sums. A sector of 2048 bytes contains 512 CRC sums. */

   crc_sectors = ei->sectors / 512;
   sec_idx = 0;

   for(i=0; i<crc_sectors; i++)
   {  if(LargeRead(ei->file, buf, 2048) != 2048)
	 Stop(_("Error reading CRC information: %s"),strerror(errno));
      buf += 512;

      for(j=0; j<512; j++, sec_idx++)
	 SetBit(cb->valid, sec_idx);
   }

   crc_remainder = sizeof(guint32)*(ei->sectors % 512);
   if(crc_remainder)
   {  if(LargeRead(ei->file, buf, crc_remainder) != crc_remainder)
	 Stop(_("Error reading CRC information: %s"),strerror(errno));

      for( ; sec_idx<ei->sectors; sec_idx++)
	 SetBit(cb->valid, sec_idx);
   }

   return cb;
}

/***
 *** Load crc buffer from RS02 error correction data
 ***
 * Lots of casts from (void*) since we're transporting
 * nonpublic structs.
 */

CrcBuf *GetCRCFromRS02(void *layv, void *dhv, LargeFile *image)
{  RS02Layout *lay = (RS02Layout*)layv;
   DeviceHandle *dh = (DeviceHandle*)dhv;
   AlignedBuffer *ab = CreateAlignedBuffer(2048);
   CrcBuf *cb = g_malloc(sizeof(CrcBuf));
   gint64 block_idx[256];
   guint32 *buf;
   gint64 image_sectors,crc_sector;
   gint64 s,i;
   int crc_idx, crc_valid = FALSE;

   image_sectors = lay->eccSectors+lay->dataSectors;
 
   cb->crcbuf = g_malloc(image_sectors * sizeof(guint32));
   cb->size   = image_sectors;
   cb->valid  = CreateBitmap0(image_sectors);
   buf = cb->crcbuf;

   /* Initialize ecc block index pointers.
      The first CRC set (of lay->ndata checksums) relates to
      ecc block lay->firstCrcLayerIndex + 1. */

   for(s=0, i=0; i<lay->ndata; s+=lay->sectorsPerLayer, i++)
     block_idx[i] = s + lay->firstCrcLayerIndex + 1;

   crc_idx = 512;                   /* force crc buffer reload */
   crc_sector = lay->dataSectors+2; /* first crc data sector on medium */

   /* Cycle through the ecc blocks and descramble CRC sums in
      ascending sector numbers. */

   for(s=0; s<lay->sectorsPerLayer; s++)
   {  gint64 si = (s + lay->firstCrcLayerIndex + 1) % lay->sectorsPerLayer;

      /* Wrap the block_idx[] ptrs at si == 0 */

      if(!si)
      {  gint64 bs;

         for(bs=0, i=0; i<lay->ndata; bs+=lay->sectorsPerLayer, i++)
	   block_idx[i] = bs;
      }

      /* Go through all data sectors of current ecc block */

      for(i=0; i<lay->ndata; i++)
      {  gint64 bidx = block_idx[i];

	 if(bidx < lay->dataSectors)  /* only data sectors have CRCs */
	 {  
	    /* Refill crc cache if needed */
	    
	    if(crc_idx >= 512)
	    {   crc_valid = !ReadSectorsFast(dh, ab->buf, crc_sector++, 1);
		crc_idx = 0;
	    }

	    /* Sort crc into appropriate place */

	    if(crc_valid)
	    {  cb->crcbuf[bidx] = ((guint32*)ab->buf)[crc_idx];
	       SetBit(cb->valid, bidx);
	    }
	    crc_idx++;
	    block_idx[i]++;
	 }
      }
   }

   FreeAlignedBuffer(ab);

   return cb;
}

/***
 *** Test a 2048 byte block against the checksum in the buffer
 ***/

int CheckAgainstCrcBuffer(CrcBuf *cb, gint64 idx, unsigned char *buf)
{  guint32 crc;

   if(idx < 0 || idx >= cb->size)
      Stop("CheckAgainstCrcBuffer: illegal index %ldd\n", idx);

   crc = Crc32(buf, 2048);

   if(!GetBit(cb->valid, idx))
      return CRC_UNKNOWN;
   
   if(crc == cb->crcbuf[idx])
      return CRC_GOOD;

   return CRC_BAD;
}


/***
 *** Clean up
 ***/

void FreeCrcBuf(CrcBuf *cb)
{  
   g_free(cb->crcbuf);
   FreeBitmap(cb->valid);
   g_free(cb);
}
