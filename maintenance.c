/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2004-2012 Carsten Gnoerlich.
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

#if 1
void Maintenance1(char *debug_arg)
{
  printf("\nMaintenance stub called with arg: %s\n\n", debug_arg);

  exit(0);
}
#else

void Maintenance1(char *debug_arg)
{  RawBuffer *rb = CreateRawBuffer(MAX_RAW_TRANSFER_SIZE);
   int i;

   for(i=0; i<MAX_RAW_TRANSFER_SIZE; i+=8)
     strncpy(&rb->rawBuf[0][i], "Raw-Buff", 8);
   
   strcpy(rb->rawBuf[0], debug_arg);
   rb->lba = 250;
   rb->samplesRead = 1;

   if(Closure->dDumpDir) g_free(Closure->dDumpDir); Closure->dDumpDir = g_strdup("/tmp");
   if(Closure->dDumpPrefix) g_free(Closure->dDumpPrefix); Closure->dDumpPrefix = g_strdup("raw");

   SaveDefectiveSector(rb, 1);

   exit(0);
}

#endif
