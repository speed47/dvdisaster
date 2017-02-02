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

#include "scsi-layer.h"
#include "rs02-includes.h"

/***
 *** Create an uninitialized CRC buffer
 ***/

CrcBuf *CreateCrcBuf(guint64 sectors)
{  CrcBuf *cb = g_malloc(sizeof(CrcBuf));

   cb->crcbuf = g_malloc(sectors * sizeof(guint32));
   cb->size   = sectors;
   cb->valid  = CreateBitmap0(sectors);

   return cb;
}

/***
 *** Test a 2048 byte block against the checksum in the buffer
 ***/

int CheckAgainstCrcBuffer(CrcBuf *cb, gint64 idx, unsigned char *buf)
{  guint32 crc;

   if(idx < 0 || idx >= cb->size)
     return CRC_OUTSIDE_BOUND;
   
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
