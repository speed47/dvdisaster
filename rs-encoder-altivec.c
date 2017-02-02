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

#ifdef HAVE_ALTIVEC
#  include <altivec.h>
#endif

#include <signal.h>
#include <setjmp.h>

/***
 *** Reed-Solomon encoding using AltiVec intrinsics
 ***
 *** Based on rs-encoder-altivec.c
 *** AltiVec version by michael.klein@puffin.lb.shuttle.de
 ***/

/* AltiVec version */

#ifdef HAVE_ALTIVEC
static volatile int AltiVecPresent;
static jmp_buf jmpbuf;

void sig_ill_handler(int sig)
{
    AltiVecPresent = 0;
    siglongjmp(jmpbuf, 0);
}

int ProbeAltiVec(void)
{
    sig_t old_handler;

    AltiVecPresent = 1;

    old_handler = signal(SIGILL, sig_ill_handler);
    if(!sigsetjmp(jmpbuf, 0))
    {
        vector unsigned char v;
        asm volatile("vor %0, %0, %0": "=v"(v));
    }

    signal(SIGILL, old_handler);

    return AltiVecPresent;
}

void encode_next_layer_altivec(ReedSolomonTables *rt, unsigned char *data, unsigned char *parity, guint64 layer_size, int shift)
{  gint32 *gf_index_of  = rt->gfTables->indexOf;
   gint32 *enc_alpha_to = rt->gfTables->encAlphaTo;
   gint32 *rs_gpoly     = rt->gpoly;
   int nroots           = rt->nroots;
   int nroots_aligned   = (nroots+15)&~15;
   int nroots_full      = nroots_aligned>>4;
   int i,j;

   for(i=0; i<layer_size; i++)
   {  int feedback    = gf_index_of[data[i] ^ parity[shift]];
      int offset      = nroots-shift-1;

      if(feedback != GF_ALPHA0) /* non-zero feedback term */
      { guint8 *par_idx = (guint8*)parity;
        guint8 *e_lut = rt->bLut[feedback]+offset;

         vector unsigned char par, lut, out, msq, lsq, mask;

        /* Process lut in 128 bit steps */

         mask = vec_lvsl(0, e_lut);

        for(j=nroots_full; j; j--)
        {  
           par = vec_ld(0, par_idx);
           msq = vec_ld(0, e_lut);    
           lsq = vec_ld(15, e_lut);    
           lut = vec_perm(msq, lsq, mask);
           out = vec_xor(par, lut);
           vec_st(out, 0, par_idx);
           par_idx += 16;
           e_lut += 16;
        }

        parity[shift] = enc_alpha_to[feedback + rs_gpoly[0]];
      }
      else  /* zero feedback term */
       parity[shift] = 0;

      parity += nroots_aligned;
   }
}
#else /* don't have ALTIVEC */
int ProbeAltiVec()
{  return 0;
}

void encode_next_layer_altivec(ReedSolomonTables *rt, unsigned char *data, unsigned char *parity, guint64 layer_size, int shift)
{
   Stop("Mega borkage - EncodeNextLayerAltiVec() stub called.\n");
}
#endif /* HAVE_ALTIVEC */

