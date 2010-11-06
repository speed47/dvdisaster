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
#include "udf.h"

#include "rs03-includes.h"

/***
 *** Recognize a RS03 error correction file
 ***/

int RS03RecognizeFile(Method *self, LargeFile *ecc_file)
{  EccHeader eh;
   int n;

   LargeSeek(ecc_file, 0);
   n = LargeRead(ecc_file, &eh, sizeof(EccHeader));

   if(n != sizeof(EccHeader))
     return FALSE;

   if(strncmp((char*)eh.cookie, "*dvdisaster*", 12))
     return FALSE;

   if(!strncmp((char*)eh.method, "RS03", 4))
   {
      if(self->lastEh) g_free(self->lastEh);
      self->lastEh = g_malloc(sizeof(EccHeader));
      memcpy(self->lastEh, &eh, sizeof(EccHeader));

#ifdef HAVE_BIG_ENDIAN
      SwapEccHeaderBytes(self->lastEh);
#endif
      return TRUE;
   }

   return FALSE;
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

EccHeader* ValidHeader(unsigned char *buf, gint64 hdr_pos)
{  EccHeader *eh = (EccHeader*)buf;
   guint32 recorded_crc, real_crc;
   //   unsigned char fingerprint[16];

   /* Medium read error in ecc header? */

   if(   (CheckForMissingSector(buf, hdr_pos, NULL, 0) != SECTOR_PRESENT)
      || (CheckForMissingSector(buf+2048, hdr_pos+1, NULL, 0) != SECTOR_PRESENT))
     return NULL;

   /* See if the magic cookie is there */

   if(   strncmp((char*)eh->cookie, "*dvdisaster*", 12)
	 || strncmp((char*)eh->method, "RS03", 4))   // FIXME
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

   /* Check the fingerprint */

   eh = g_malloc(sizeof(EccHeader));
   memcpy(eh, buf, sizeof(EccHeader));
#ifdef HAVE_BIG_ENDIAN
   SwapEccHeaderBytes(eh);
#endif
   eh->selfCRC = recorded_crc;

#if 0
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

EccHeader* FindRS03HeaderInImage(LargeFile *file)
{  EccHeader *eh = NULL;
   IsoInfo *ii; 
   gint64 hdr_pos; 
   unsigned char buf[4096];

   Verbose("FindRS03HeaderInImage(%s)\n", file->path);

   /*** Try to find the header behind the ISO image */

   ii = ExamineUDF(NULL, file);
   if(!ii) Verbose(" . NO ISO structures found!\n");

   if(ii)
   {  hdr_pos = ii->volumeSize;
      if(LargeSeek(file, 2048*hdr_pos))
      {  int n = LargeRead(file, buf, sizeof(EccHeader));

         if(n == sizeof(EccHeader))
	 {  eh = ValidHeader(buf, hdr_pos);
	    if(eh) 
	    { Verbose("FindRS03HeaderInImage(): Header found at pos +0\n"); 
	      return eh;
	    }
	 }
      }

      hdr_pos = ii->volumeSize - 150;
      if(LargeSeek(file, 2048*hdr_pos))
      {  int n = LargeRead(file, buf, sizeof(EccHeader));

         if(n == sizeof(EccHeader))
	 {  eh = ValidHeader(buf, hdr_pos);
	    if(eh) 
	    { Verbose("FindRS03HeaderInImage(): Header found at pos -150\n"); 
	      return eh;
	    }
	 }
      }
   }
      
   return NULL;
}

typedef struct 
{  gint64 bidx[256];
   char *layer[256];
} recognize_context;

static void free_recognize_context(recognize_context *rc)
{  int i;

   for(i=0; i<255; i++)
      if(rc->layer[i])
	 g_free(rc->layer[i]);

   g_free(rc);
}
   
int RS03RecognizeImage(Method *self, LargeFile *ecc_file)
{  recognize_context *rc = g_malloc0(sizeof(recognize_context));
   EccHeader *eh;
   gint64 file_size;
   gint64 layer_size;
   int ecc_block,ndata,nroots;
   int i;

   /* Easy shot: Locate the ecc header in the image */

   eh = FindRS03HeaderInImage(ecc_file); 
  
   if(eh)
   {  if(self->lastEh) g_free(self->lastEh);
      self->lastEh = eh;
      return TRUE;
   }

   /* No exhaustive search unless explicitly okayed by user */

   if(!Closure->examineRS03)
     return FALSE;

   /* Ugly case. Experimentally try the RS-Code. */

   Verbose("RS03RecognizeImage(): No EH\n");

   if(!LargeStat(Closure->imageName, &file_size))
      return FALSE;

   file_size /= 2048;

   if(Closure->debugMode && Closure->mediumSize)
      layer_size = Closure->mediumSize/GF_FIELDMAX;
   else
   {  if(file_size < CDR_SIZE)         layer_size = CDR_SIZE/GF_FIELDMAX;
      else if(file_size < DVD_SL_SIZE) layer_size = DVD_SL_SIZE/GF_FIELDMAX; 
      else if(file_size < DVD_DL_SIZE) layer_size = DVD_DL_SIZE/GF_FIELDMAX; 
      else if(file_size < BD_SL_SIZE)  layer_size = BD_SL_SIZE/GF_FIELDMAX; 
      else                             layer_size = BD_DL_SIZE/GF_FIELDMAX; 
   }

   Verbose(".. trying layer size %lld\n", layer_size);

   for(i=0; i<255; i++)
   {  rc->bidx[i] = i*layer_size;
      rc->layer[i] = malloc(2048);
   }

   /* Now try all ecc blocks */

   for(ecc_block=0; ecc_block<layer_size; ecc_block++)
   {  Verbose("Assembling ecc block %d\n", ecc_block); 

      /* Assemble the ecc block */

      for(i=0; i<255; i++)  
      {  gint64 sector = rc->bidx[i]++;
	 int n;

	 if(!LargeSeek(ecc_file, (gint64)(2048*sector)))
	    Stop(_("Failed seeking to sector %lld in image: %s"),
		 sector, strerror(errno));

	 n = LargeRead(ecc_file, rc->layer[i], 2048);
	 if(n != 2048)
	    Stop(_("Failed reading sector %lld in image: %s"),sector,strerror(errno));
      }	 

      /* Experimentally apply the RS code */

      for(ndata=255-8; ndata >=85; ndata--)
      {  CrcBlock *cb = (CrcBlock*)rc->layer[ndata];

	 /* Do the real decode here */


	 /* See if we have decoded a CRC block */

	 if(  !memcmp(cb->cookie, "*dvdisaster*", 12)
	    ||!memcmp(cb->method, "RS03", 4))
	 {  
	    nroots = 255-ndata-1;
	    Verbose(".. Success: rediscovered format with %d roots\n", nroots); 

	    if(self->lastEh) g_free(self->lastEh);
	    self->lastEh = g_malloc(sizeof(EccHeader));
	    ReconstructRS03Header(self->lastEh, cb);
	    //FIXME: endianess okay?
	    free_recognize_context(rc);
	    return TRUE;
	 }
      }
   }

   free_recognize_context(rc);
   return FALSE;
}

