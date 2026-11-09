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
#include "galois-inlines.h"

/***
 *** Reed-Solomon decoding (work in progress; incomplete)
 ***/

/*
 * Calculate the syndromes of 2048 consecutive codewords.
 * Byte k of codeword k is at layer[j][offset+k] for j=0..254;
 * syndrome i of codeword k is stored in syn[i*2048+k].
 * Working layer-wise lets us process many independent codewords at once.
 * A multiplication by a constant is done with two 16-entry nibble tables,
 * which maps 1:1 onto pshufb (x86) and tbl (ARM).
 */

#if defined(HAVE_SSE2) && defined(__GNUC__)
#include <immintrin.h>

__attribute__((target("avx2")))
static void syndromes_avx2(guint8 *lo, guint8 *hi, unsigned char **layer, int offset, guint8 *syn)
{  __m256i tlo = _mm256_broadcastsi128_si256(_mm_loadu_si128((__m128i*)lo));
   __m256i thi = _mm256_broadcastsi128_si256(_mm_loadu_si128((__m128i*)hi));
   __m256i m   = _mm256_set1_epi8(0x0f);
   int j,k,v;

   for(k=0; k<2048; k+=128)
   {  __m256i s[4];

      for(v=0; v<4; v++)
	 s[v] = _mm256_loadu_si256((__m256i*)(layer[0]+offset+k+32*v));

      for(j=1; j<GF_FIELDMAX; j++)
	 for(v=0; v<4; v++)
	 {  __m256i l = _mm256_shuffle_epi8(tlo, _mm256_and_si256(s[v], m));
	    __m256i h = _mm256_shuffle_epi8(thi, _mm256_and_si256(_mm256_srli_epi16(s[v], 4), m));
	    __m256i d = _mm256_loadu_si256((__m256i*)(layer[j]+offset+k+32*v));
	    s[v] = _mm256_xor_si256(d, _mm256_xor_si256(l, h));
	 }

      for(v=0; v<4; v++)
	 _mm256_storeu_si256((__m256i*)(syn+k+32*v), s[v]);
   }
}
#endif

#ifdef __aarch64__
#include <arm_neon.h>

static void syndromes_neon(guint8 *lo, guint8 *hi, unsigned char **layer, int offset, guint8 *syn)
{  uint8x16_t tlo = vld1q_u8(lo), thi = vld1q_u8(hi), m = vdupq_n_u8(0x0f);
   int j,k,v;

   for(k=0; k<2048; k+=64)
   {  uint8x16_t s[4];

      for(v=0; v<4; v++)
	 s[v] = vld1q_u8(layer[0]+offset+k+16*v);

      for(j=1; j<GF_FIELDMAX; j++)
	 for(v=0; v<4; v++)
	 {  uint8x16_t l = vqtbl1q_u8(tlo, vandq_u8(s[v], m));
	    uint8x16_t h = vqtbl1q_u8(thi, vshrq_n_u8(s[v], 4));
	    s[v] = veorq_u8(vld1q_u8(layer[j]+offset+k+16*v), veorq_u8(l, h));
	 }

      for(v=0; v<4; v++)
	 vst1q_u8(syn+k+16*v, s[v]);
   }
}
#endif

void CalcSyndromes(ReedSolomonTables *rt, unsigned char **layer, int offset, guint8 *syn)
{  int i,j,k;
#if defined(HAVE_SSE2) && defined(__GNUC__)
   static int use_avx2 = -1;

   if(use_avx2 < 0)
     use_avx2 = __builtin_cpu_supports("avx2");
#endif

   for(i=0; i<rt->nroots; i++, syn+=2048)
   {  guint8 *lut = rt->synLut + (i<<8);
      guint8 lo[16], hi[16];

      for(k=0; k<16; k++)
      {  lo[k] = lut[k];
	 hi[k] = lut[k<<4];
      }

#if defined(HAVE_SSE2) && defined(__GNUC__)
      if(use_avx2)
      {  syndromes_avx2(lo, hi, layer, offset, syn);
	 continue;
      }
#endif
#ifdef __aarch64__
      syndromes_neon(lo, hi, layer, offset, syn);
      continue;
#endif
      memcpy(syn, layer[0]+offset, 2048);
      for(j=1; j<GF_FIELDMAX; j++)
      {  unsigned char *d = layer[j]+offset;

	 for(k=0; k<2048; k++)
	    syn[k] = d[k] ^ lo[syn[k] & 15] ^ hi[syn[k] >> 4];
      }
   }
}
