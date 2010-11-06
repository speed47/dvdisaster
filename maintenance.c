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
