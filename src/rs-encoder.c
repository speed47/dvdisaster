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

/***
 *** Reed-Solomon encoding
 ***/

/* Portable (non-SSE2) version.
 * Using 32bit operands seems to be a good choice for the lowest
 * common denominator between the non-SSE2 systems.
 */

#ifdef HAVE_BIG_ENDIAN
  #define SHIFT_LEFT <<
  #define SHIFT_RIGHT >>
#else
  #define SHIFT_LEFT >>
  #define SHIFT_RIGHT <<
#endif /* HAVE_BIG_ENDIAN */

static void encode_next_layer_portable(ReedSolomonTables *rt, unsigned char *data, unsigned char *parity, guint64 layer_size, int shift)
{  gint32 *gf_index_of  = rt->gfTables->indexOf;
   gint32 *enc_alpha_to = rt->gfTables->encAlphaTo;
   gint32 *rs_gpoly     = rt->gpoly;
   int nroots           = rt->nroots;
   int nroots_aligned   = (nroots+15)&~15;
   int nroots_aligned32 = (nroots+3)&~3;
   int nroots_full      = nroots_aligned32>>2;
   int i,j;

   for(i=0; i<layer_size; i++)
   {  int feedback    = gf_index_of[data[i] ^ parity[shift]];
      int offset      = nroots-shift-1;
      int byte_offset = offset&3;

      if(feedback != GF_ALPHA0) /* non-zero feedback term */
      {	 guint32 *par_idx = (guint32*)parity;
	 guint32 *e_lut   = ((guint32*)(rt->bLut[feedback]+(offset&~3)));

	 /* Process lut in 32 bit steps */

	 switch(byte_offset)
	 {  case 0:
	       for(j=nroots_full; j; j--)
		  *par_idx++ ^= *e_lut++;
	       break;

	    case 1:
	    {  for(j=nroots_full; j; j--)
	       {  guint32 span = *e_lut SHIFT_LEFT 8;
		  e_lut++;
		  span |= *e_lut SHIFT_RIGHT 24;
		  *par_idx++ ^= span;
	       }
	    }
	       break;  

	    case 2:
	    {  for(j=nroots_full; j; j--)
	       {  guint32 span = *e_lut SHIFT_LEFT 16;
		  e_lut++;
		  span |= *e_lut SHIFT_RIGHT 16;
		  *par_idx++ ^= span;
	       }
	    }
	       break;  

	    case 3:
	    {  for(j=nroots_full; j; j--)
	       {  guint32 span = *e_lut SHIFT_LEFT 24;
		  e_lut++;
		  span |= *e_lut SHIFT_RIGHT 8;
		  *par_idx++ ^= span;
	       }
	    }
	       break;  
	 }

	 parity[shift] = enc_alpha_to[feedback + rs_gpoly[0]];
       }
       else  /* zero feedback term */
	   parity[shift] = 0;

       parity += nroots_aligned;
   }
}

/* 64bit integer (non-SSE2) version.
 * May perform better on systems which have shared FPU/SSE2 units
 * between several cores.
 */

static void encode_next_layer_64bit(ReedSolomonTables *rt, unsigned char *data, unsigned char *parity, guint64 layer_size, int shift)
{  gint32 *gf_index_of  = rt->gfTables->indexOf;
   gint32 *enc_alpha_to = rt->gfTables->encAlphaTo;
   gint32 *rs_gpoly     = rt->gpoly;
   int nroots           = rt->nroots;
   int nroots_aligned   = (nroots+15)&~15;
   int nroots_aligned64 = (nroots+7)&~7;
   int nroots_full      = nroots_aligned64>>3;
   int i,j;

   for(i=0; i<layer_size; i++)
   {  int feedback    = gf_index_of[data[i] ^ parity[shift]];
      int offset      = nroots-shift-1;
      int byte_offset = offset&7;

      if(feedback != GF_ALPHA0) /* non-zero feedback term */
      {	 guint64 *par_idx = (guint64*)parity;
	 guint64 *e_lut   = ((guint64*)(rt->bLut[feedback]+(offset&~7)));

	 /* Process lut in 64 bit steps */

	 switch(byte_offset)
	 {  case 0:
	       for(j=nroots_full; j; j--)
		  *par_idx++ ^= *e_lut++;
	       break;

	    case 1:
	    {  for(j=nroots_full; j; j--)
	       {  guint64 span = *e_lut SHIFT_LEFT 8;
		  e_lut++;
		  span |= *e_lut SHIFT_RIGHT 56;
		  *par_idx++ ^= span;
	       }
	    }
	       break;  

	    case 2:
	    {  for(j=nroots_full; j; j--)
	       {  guint64 span = *e_lut SHIFT_LEFT 16;
		  e_lut++;
		  span |= *e_lut SHIFT_RIGHT 48;
		  *par_idx++ ^= span;
	       }
	    }
	       break;  

	    case 3:
	    {  for(j=nroots_full; j; j--)
	       {  guint64 span = *e_lut SHIFT_LEFT 24;
		  e_lut++;
		  span |= *e_lut SHIFT_RIGHT 40;
		  *par_idx++ ^= span;
	       }
	    }
	       break;  

	    case 4:
	    {  for(j=nroots_full; j; j--)
	       {  guint64 span = *e_lut SHIFT_LEFT 32;
		  e_lut++;
		  span |= *e_lut SHIFT_RIGHT 32;
		  *par_idx++ ^= span;
	       }
	    }
	       break;  

	    case 5:
	    {  for(j=nroots_full; j; j--)
	       {  guint64 span = *e_lut SHIFT_LEFT 40;
		  e_lut++;
		  span |= *e_lut SHIFT_RIGHT 24;
		  *par_idx++ ^= span;
	       }
	    }
	       break;  

	    case 6:
	    {  for(j=nroots_full; j; j--)
	       {  guint64 span = *e_lut SHIFT_LEFT 48;
		  e_lut++;
		  span |= *e_lut SHIFT_RIGHT 16;
		  *par_idx++ ^= span;
	       }
	    }
	       break;  

	    case 7:
	    {  for(j=nroots_full; j; j--)
	       {  guint64 span = *e_lut SHIFT_LEFT 56;
		  e_lut++;
		  span |= *e_lut SHIFT_RIGHT 8;
		  *par_idx++ ^= span;
	       }
	    }
	       break;  
	 }

	 parity[shift] = enc_alpha_to[feedback + rs_gpoly[0]];
       }
       else  /* zero feedback term */
	   parity[shift] = 0;

       parity += nroots_aligned;
   }
}

/*
 * Dispatch upon availability of SSE2 intrinsics
 */

void encode_next_layer_sse2(ReedSolomonTables*, unsigned char*, unsigned char*, guint64, int);
void encode_next_layer_altivec(ReedSolomonTables*, unsigned char*, unsigned char*, guint64, int);

void EncodeNextLayer(ReedSolomonTables *rt, unsigned char *data, unsigned char *parity, guint64 layer_size, int shift)
{   
    switch(Closure->encodingAlgorithm)
    {  case ENCODING_ALG_32BIT:
          encode_next_layer_portable(rt, data, parity, layer_size, shift);
	  break;
       case ENCODING_ALG_64BIT:
          encode_next_layer_64bit(rt, data, parity, layer_size, shift);
	  break;
       case ENCODING_ALG_SSE2:
	  encode_next_layer_sse2(rt, data, parity, layer_size, shift);
	  break;
       case ENCODING_ALG_ALTIVEC:
	  encode_next_layer_altivec(rt, data, parity, layer_size, shift);
	  break;
       case ENCODING_ALG_DEFAULT:
	 if(Closure->useSSE2)
	   encode_next_layer_sse2(rt, data, parity, layer_size, shift);
	 else if(Closure->useAltiVec)
	   encode_next_layer_altivec(rt, data, parity, layer_size, shift);
	 else
	   encode_next_layer_portable(rt, data, parity, layer_size, shift);
	 break;
    }
}

/*
 * Provide textual description for current encoder parameters
 */

void DescribeRSEncoder(char **algorithm, char **iostrategy)
{
  switch(Closure->encodingAlgorithm)
  {  case ENCODING_ALG_32BIT:
        *algorithm="32bit";
	break;
     case ENCODING_ALG_64BIT:
        *algorithm="64bit";
	break;
     case ENCODING_ALG_SSE2:
        *algorithm="SSE2";
	break;
     case ENCODING_ALG_ALTIVEC:
        *algorithm="AltiVec";
	break;
     case ENCODING_ALG_DEFAULT:
        if(Closure->useSSE2)
	  *algorithm="SSE2";
	else if(Closure->useAltiVec)
	  *algorithm="AltiVec";
	else
	  *algorithm="64bit";
	break;
  }

  if(Closure->encodingIOStrategy == IO_STRATEGY_MMAP)
       *iostrategy="mmap";
  else *iostrategy="read/write";
}
