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
#include "galois-inlines.h"

/***
 *** Reed-Solomon decoding (work in progress; incomplete)
 ***/

/*
 * Test and report the error syndrome.
 */

int TestErrorSyndromes(ReedSolomonTables *rt, unsigned char *data)
{  int syndrome[rt->nroots];
   int syn_error;
   int i,j;

   /*** Form the syndromes: Evaluate data(x) at roots of g(x) */

   for(i=0; i<rt->nroots; i++)
     syndrome[i] = data[0];

   for(j=1; j<GF_FIELDMAX; j++)
     for(i=0; i<rt->nroots; i++)
       if(syndrome[i] == 0) 
            syndrome[i] = data[j];

       else syndrome[i] = data[j] ^ rt->synLut[(i<<8) + syndrome[i]];
#if 0
        else syndrome[i] = data[j] ^ gt->alphaTo[mod_fieldmax(gt->indexOf[syndrome[i]] 
							      + (rt->fcr+i)*rt->primElem)];
#endif

   /*** Check for nonzero condition. */

   syn_error = 0;
   for(i=0; i<rt->nroots; i++)
      syn_error |= syndrome[i];

   /*** If the syndrome is zero, everything is fine. */

   return syn_error;
}
