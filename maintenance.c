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

#if 0
void Maintenance1(char *debug_arg)
{
  printf("\nMaintenance stub called with arg: %s\n\n", debug_arg);

  exit(0);
}
#else

void Maintenance1(char *debug_arg)
{  GaloisTables *gt = CreateGaloisTables(RS_GENERATOR_POLY);
   ReedSolomonTables *rt = CreateReedSolomonTables(gt, RS_FIRST_ROOT, RS_PRIM_ELEM, 32);
   unsigned char data[2048], parity[32*2048];
   int i;
		  
   memset(parity, 0, 32*2048);
		
   for(i=0; i<223; i++)
   {  int shift = (rt->shiftInit + i) % 32;

      memset(data, i, 2048);
      EncodeNextLayer(rt, data, parity, 2048, shift);
   }

   for(i=0; i<32; i++)
     printf("%02x ", parity[i]);
   printf("\n");

}

#endif
