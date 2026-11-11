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

/* AVX2 is selected at runtime; only the encoder function uses AVX2 instructions. */

#ifdef HAVE_AVX2
#include <immintrin.h>

int ProbeAVX2(void)
{  return __builtin_cpu_supports("avx2");
}

__attribute__((target("avx2")))
void encode_next_layer_avx2(ReedSolomonTables *rt, unsigned char *data, unsigned char *parity, guint64 layer_size, int shift)
{  gint32 *gf_index_of  = rt->gfTables->indexOf;
   gint32 *enc_alpha_to = rt->gfTables->encAlphaTo;
   gint32 *rs_gpoly     = rt->gpoly;
   int nroots           = rt->nroots;
   int nroots_aligned   = (nroots+15)&~15;
   int offset           = nroots-shift-1;
   int i,j;

   for(i=0; i<layer_size; i++)
   {  int feedback = gf_index_of[data[i] ^ parity[shift]];

      if(feedback != GF_ALPHA0) /* non-zero feedback term */
      {	 guint8 *e_lut = rt->bLut[feedback]+offset;

	 for(j=0; j+32<=nroots_aligned; j+=32)
	   _mm256_storeu_si256((__m256i*)(parity+j),
			       _mm256_xor_si256(_mm256_loadu_si256((__m256i*)(parity+j)),
						_mm256_loadu_si256((__m256i*)(e_lut+j))));
	 if(j<nroots_aligned)
	   _mm_storeu_si128((__m128i*)(parity+j),
			    _mm_xor_si128(_mm_loadu_si128((__m128i*)(parity+j)),
					  _mm_loadu_si128((__m128i*)(e_lut+j))));

	 parity[shift] = enc_alpha_to[feedback + rs_gpoly[0]];
      }
      else  /* zero feedback term */
	parity[shift] = 0;

      parity += nroots_aligned;
   }
}
#else
/* Stub functions for builds without AVX2 intrinsics. */
int ProbeAVX2(void)
{  return 0;
}

void encode_next_layer_avx2(ReedSolomonTables *rt, unsigned char *data, unsigned char *parity, guint64 layer_size, int shift)
{
   Stop("Mega borkage - EncodeNextLayerAVX2() stub called.\n");
}
#endif
