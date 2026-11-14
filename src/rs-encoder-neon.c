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

/* AArch64 userspace supports Advanced SIMD as a baseline. The configure
 * probe checks the intrinsics without enabling any optional ISA extensions. */

#ifdef HAVE_NEON
#include <arm_neon.h>

int ProbeNEON(void)
{  return 1;
}

void encode_next_layer_neon(ReedSolomonTables *rt, unsigned char *data, unsigned char *parity, guint64 layer_size, int shift)
{  gint32 *gf_index_of  = rt->gfTables->indexOf;
   gint32 *enc_alpha_to = rt->gfTables->encAlphaTo;
   int nroots_aligned   = (rt->nroots+15)&~15;
   int offset           = rt->nroots-shift-1;
   int generator        = rt->gpoly[0];

   for(guint64 i=0; i<layer_size; i++)
   {  int feedback = gf_index_of[data[i] ^ parity[shift]];

      if(feedback != GF_ALPHA0)
      {  const guint8 *lut = rt->bLut[feedback]+offset;

         /* The table offset need not be aligned. Both buffers include the
            padding up to nroots_aligned, as in the SSE2 encoder. */
         for(int j=0; j<nroots_aligned; j+=16)
            vst1q_u8(parity+j, veorq_u8(vld1q_u8(parity+j), vld1q_u8(lut+j)));

         parity[shift] = enc_alpha_to[feedback + generator];
      }
      else
         parity[shift] = 0;

      parity += nroots_aligned;
   }
}
#else
int ProbeNEON(void)
{  return 0;
}

void encode_next_layer_neon(ReedSolomonTables *rt, unsigned char *data, unsigned char *parity, guint64 layer_size, int shift)
{
   Stop("Mega borkage - EncodeNextLayerNEON() stub called.\n");
}
#endif
