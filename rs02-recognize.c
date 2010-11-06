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

#include "rs02-includes.h"

/***
 *** Recognize RS02 error correction data in the image
 ***/

/*
 * Search for ecc headers in RS02 style image files.
 * Note that udf.c has a similar function FindHeaderInMedium().
 */

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

EccHeader* FindRS02HeaderInImage(LargeFile *file)
{  EccHeader *eh = NULL;
   unsigned char buf[4096];
   gint64 sectors,pos;
   gint64 header_modulo;
   gint64 last_fp = -1;
   unsigned char fingerprint[16];

   header_modulo = (gint64)1<<62;
   sectors = file->size / 2048;

   /*** Search for the headers */

   while(header_modulo >= 32)
   {  pos = sectors & ~(header_modulo - 1);

//printf("Trying modulo %lld\n", header_modulo);

      while(pos > 0)
      {  if(LargeSeek(file, 2048*pos))
	 {  int n;

//printf(" trying sector %lld\n", pos);
	    n = LargeRead(file, buf, sizeof(EccHeader));

	    if(n != sizeof(EccHeader))
	      goto check_next_header;

	    eh = (EccHeader*)buf;

	    /* Medium read error in ecc header? */

	    if(   (CheckForMissingSector(buf, pos, NULL, 0) != SECTOR_PRESENT)
	       || (CheckForMissingSector(buf+2048, pos+1, NULL, 0) != SECTOR_PRESENT))
	    {  
//printf(" header at %lld: read error\n", (long long int)pos);
	       goto check_next_header;
	    }

	    /* See if the magic cookie is there */

	    if(   !strncmp((char*)eh->cookie, "*dvdisaster*", 12)
	       && !strncmp((char*)eh->method, "RS02", 4))
	    {  guint32 recorded_crc = eh->selfCRC;
	       guint32 real_crc;

//printf(" header at %lld: magic cookie found\n", (long long int)pos);

#ifdef HAVE_BIG_ENDIAN
	       eh->selfCRC = 0x47504c00;
#else
	       eh->selfCRC = 0x4c5047;
#endif
	       real_crc = Crc32((unsigned char*)eh, sizeof(EccHeader));

	       if(real_crc == recorded_crc)
	       {  eh = g_malloc(sizeof(EccHeader));
		  memcpy(eh, buf, sizeof(EccHeader));
#ifdef HAVE_BIG_ENDIAN
		  SwapEccHeaderBytes(eh);
#endif
		  eh->selfCRC = recorded_crc;
//printf(" --> CRC okay, using it\n");

		  if(last_fp != eh->fpSector)
		  {  int status;

		     status = read_fingerprint(file, fingerprint, eh->fpSector);
		     last_fp = eh->fpSector;

		     if(!status)  /* be optimistic if fingerprint sector is unreadable */
		     {  return eh;
		     }
		  }

		  if(!memcmp(fingerprint, eh->mediumFP, 16))  /* good fingerprint */
		  {  return eh;
		  }

		  /* might be a header from a larger previous session.
		     discard it and continue */

		  g_free(eh);
	       }
//printf(" CRC failed, skipping it\n");
	       goto check_next_header;
	    }
	    else
	    {
//printf(" no cookie, skipping current modulo\n");
	      goto check_next_modulo;
	    }
	 }

      check_next_header:
	pos -= header_modulo;
      }

   check_next_modulo:
      header_modulo >>= 1;
   }

   return NULL;
}



int RS02Recognize(Method *self, LargeFile *ecc_file)
{  EccHeader *eh;

   eh = FindRS02HeaderInImage(ecc_file); 
  
   if(!eh) return FALSE;

   if(self->lastEh) g_free(self->lastEh);
   self->lastEh = eh;

   return TRUE;
}
