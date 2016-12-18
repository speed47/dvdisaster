/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2004-2015 Carsten Gnoerlich.
 *
 *  The Reed-Solomon error correction draws a lot of inspiration - and even code -
 *  from Phil Karn's excellent Reed-Solomon library: http://www.ka9q.net/code/fec/
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

#include "galois-inlines.h"

/***
 *** Galois field arithmetic.
 *** 
 * Calculations are done over the extension field GF(2**n).
 * Be careful not to overgeneralize these arithmetics;
 * they only work for the case of GF(p**n) with p being prime.
 */

/* Initialize the Galois field tables */


GaloisTables* CreateGaloisTables(gint32 gf_generator)
{  GaloisTables *gt = g_malloc0(sizeof(GaloisTables));
   gint32 b,log;

   /* Allocate the tables.
      The encoder uses a special version of alpha_to which has the mod_fieldmax()
      folded into the table. */

   gt->gfGenerator = gf_generator;

   gt->indexOf     = g_malloc(GF_FIELDSIZE * sizeof(gint32));
   gt->alphaTo     = g_malloc(GF_FIELDSIZE * sizeof(gint32));
   gt->encAlphaTo  = g_malloc(2*GF_FIELDSIZE * sizeof(gint32));
   
   /* create the log/ilog values */

   for(b=1, log=0; log<GF_FIELDMAX; log++)
   {  gt->indexOf[b]   = log;
      gt->alphaTo[log] = b;
      b = b << 1;
      if(b & GF_FIELDSIZE)
	b = b ^ gf_generator;
   }

   if(b!=1) Stop("Failed to create the Galois field log tables!\n");

   /* we're even closed using infinity (makes things easier) */

   gt->indexOf[0] = GF_ALPHA0;    /* log(0) = inf */
   gt->alphaTo[GF_ALPHA0] = 0;   /* and the other way around */

   for(b=0; b<2*GF_FIELDSIZE; b++)
     gt->encAlphaTo[b] = gt->alphaTo[mod_fieldmax(b)];

   return gt;
}

void FreeGaloisTables(GaloisTables *gt)
{
  if(gt->indexOf)     g_free(gt->indexOf);
  if(gt->alphaTo)     g_free(gt->alphaTo);
  if(gt->encAlphaTo) g_free(gt->encAlphaTo);

  g_free(gt);
}

/***
 *** Create the Reed-Solomon generator polynomial
 *** and some auxiliary data structures.
 */

ReedSolomonTables *CreateReedSolomonTables(GaloisTables *gt,
					   gint32 first_consecutive_root,
					   gint32 prim_elem,
					   int nroots_in)
{  ReedSolomonTables *rt = g_malloc0(sizeof(ReedSolomonTables));
   int lut_size, feedback;
   gint32 i,j,root;
   guint8 *lut;

   rt->gfTables = gt;
   rt->fcr      = first_consecutive_root;
   rt->primElem = prim_elem;
   rt->nroots   = nroots_in;
   rt->ndata    = GF_FIELDMAX - rt->nroots;

   rt->gpoly    = g_malloc((rt->nroots+1) * sizeof(gint32));

   /* Create the RS code generator polynomial */

   rt->gpoly[0] = 1;

   for(i=0, root=first_consecutive_root*prim_elem; i<rt->nroots; i++, root+=prim_elem)
   {  rt->gpoly[i+1] = 1;

     /* Multiply gpoly  by  alpha**(root+x) */

     for(j=i; j>0; j--)
     {
       if(rt->gpoly[j] != 0)
         rt->gpoly[j] = rt->gpoly[j-1] ^ gt->alphaTo[mod_fieldmax(gt->indexOf[rt->gpoly[j]] + root)]; 
       else
	 rt->gpoly[j] = rt->gpoly[j-1];
     }

     rt->gpoly[0] = gt->alphaTo[mod_fieldmax(gt->indexOf[rt->gpoly[0]] + root)];
   }

   /* Store the polynomials index for faster encoding */ 

   for(i=0; i<=rt->nroots; i++)
     rt->gpoly[i] = gt->indexOf[rt->gpoly[i]];

#if 0
   /* for the precalculated unrolled loops only */

   for(i=gt->nroots-1; i>0; i--)
     PrintCLI(
	    "                  par_idx[((++spk)&%d)] ^= enc_alpha_to[feedback + %3d];\n",
	    nroots-1,gt->gpoly[i]);

   PrintCLI("                  par_idx[sp] = enc_alpha_to[feedback + %3d];\n",
	  gt->gpoly[0]);
#endif

  /* 
   * Initialize the shift pointer so that we will come out at shiftPtr==0
   * respectively (ndata+sp) mod nroots = 0 after working in all ndata layers.
   */

   rt->shiftInit = rt->nroots - rt->ndata % rt->nroots;
   if(rt->shiftInit == rt->nroots)
     rt->shiftInit = 0;

   /*
    * Initialize lookup tables for both encoder types.
    * The 32bit portable encoder will shift them to word boundaries,
    * while the SSE2 encoder does direct unaligned reads.
    */

   lut_size = (rt->nroots+15)&~15;
   lut_size += 16;
   for(i=0; i<GF_FIELDSIZE; i++)
      rt->bLut[i] = g_malloc0(2*lut_size);

   for(feedback=0; feedback<256; feedback++)
   {  gint32 *gpoly        = rt->gpoly + rt->nroots;
      gint32 *enc_alpha_to = gt->encAlphaTo;
      int nroots = rt->nroots;

      for(i=0; i<nroots; i++)
      {  guint8 value = (guint8)enc_alpha_to[feedback + *--gpoly];
	 rt->bLut[feedback][i] = rt->bLut[feedback][nroots+i] = value; 
      }
   }

   /*
    * Prepare lookup table for syndrome calculation.
    */

   lut = rt->synLut = g_malloc(rt->nroots * GF_FIELDSIZE * sizeof(int));
   for(i=0; i<rt->nroots; i++)
     for(j=0; j<GF_FIELDSIZE; j++)
       *lut++ = gt->alphaTo[mod_fieldmax(gt->indexOf[j] + (rt->fcr+i)*rt->primElem)];

   return rt;
}

void FreeReedSolomonTables(ReedSolomonTables *rt)
{ int i;

  if(rt->gpoly)        g_free(rt->gpoly);

  for(i=0; i<GF_FIELDSIZE; i++)
  {  g_free(rt->bLut[i]);
  }
  g_free(rt->synLut);

  g_free(rt);
}
