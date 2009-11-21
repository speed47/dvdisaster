/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2004-2009 Carsten Gnoerlich.
 *  Project home page: http://www.dvdisaster.com
 *  Email: carsten@dvdisaster.com  -or-  cgnoerlich@fsfe.org
 *
 *  The Reed-Solomon error correction draws a lot of inspiration - and even code -
 *  from Phil Karn's excellent Reed-Solomon library: http://www.ka9q.net/code/fec/
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

/***
 *** Reed-Solomon encoding
 ***/

/*
 * Optimized encoder for 32 roots
 */

#if 0
void encode_layer_32(ReedSolomonEncoder *rse, unsigned char *data, unsigned char *par_idx, guint64 layer_size)
{  //unsigned char *par_idx = rse->parity;
   gint32 *gf_index_of  = rse->gfTables->indexOf;
   gint32 *enc_alpha_to = rse->gfTables->encAlphaTo;
   int i;

   for(i=0; i<layer_size; i++)
   {  register int feedback = gf_index_of[data[i] ^ par_idx[rse->shiftPtr]];

      if(feedback != GF_ALPHA0) /* non-zero feedback term */
      {  register int spk = rse->shiftPtr;

         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback + 249];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback +  59];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback +  66];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback +   4];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback +  43];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback + 126];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback + 251];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback +  97];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback +  30];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback +   3];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback + 213];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback +  50];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback +  66];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback + 170];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback +   5];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback +  24];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback +   5];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback + 170];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback +  66];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback +  50];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback + 213];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback +   3];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback +  30];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback +  97];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback + 251];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback + 126];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback +  43];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback +   4];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback +  66];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback +  59];
         par_idx[((++spk)&31)] ^= enc_alpha_to[feedback + 249];

	 par_idx[rse->shiftPtr] = enc_alpha_to[feedback];  /* feedback + 0 */
      }
      else  /* zero feedback term */
	 par_idx[rse->shiftPtr] = 0;

      par_idx += 32; /* nroots */
   }

   rse->shiftPtr = (rse->shiftPtr+1) & 31;         /* shift */
}

/*
 * Optimized encoder for 64 roots
 */

void encode_layer_64(ReedSolomonEncoder *rse, unsigned char *data, unsigned char *par_idx, guint64 layer_size)
{  //unsigned char *par_idx = rse->parity;
   gint32 *gf_index_of  = rse->gfTables->indexOf;
   gint32 *enc_alpha_to = rse->gfTables->encAlphaTo;
   int i;

   for(i=0; i<layer_size; i++)
   {  register int feedback = gf_index_of[data[i] ^ par_idx[rse->shiftPtr]];

      if(feedback != GF_ALPHA0) /* non-zero feedback term */
      {  register int spk = rse->shiftPtr;

         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  98];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 247];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 160];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  15];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  96];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  27];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  87];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 175];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  64];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 170];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  53];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  39];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 236];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  39];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  58];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  82];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  44];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  89];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  97];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 182];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  80];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 120];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  40];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 104];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  73];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  73];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  12];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 152];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 205];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  96];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  50];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  21];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 147];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  35];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 241];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  30];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 242];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 145];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 242];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 115];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 148];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  70];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 127];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  71];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  83];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 172];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 224];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 104];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 177];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +   0];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  39];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 194];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  50];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +   9];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +   0];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 208];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 217];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 254];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 165];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 181];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback + 168];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  97];
         par_idx[((++spk)&63)] ^= enc_alpha_to[feedback +  45];

         par_idx[rse->shiftPtr] = enc_alpha_to[feedback +  44];
      }
      else  /* zero feedback term */
	 par_idx[rse->shiftPtr] = 0;

      par_idx += 64; /* nroots */
   }

   rse->shiftPtr = (rse->shiftPtr+1) & 63;         /* shift */
}
#endif

/*
 * Encoder for any number roots
 */

void encode_layer(ReedSolomonTables *rt, unsigned char *data, unsigned char *par_idx, guint64 layer_size, int shift)
{  gint32 *gf_index_of  = rt->gfTables->indexOf;
   gint32 *enc_alpha_to = rt->gfTables->encAlphaTo;
   gint32 *rs_gpoly     = rt->gpoly;
   int nroots = rt->nroots;
   int i;

   for(i=0; i<layer_size; i++)
   {  register int feedback = gf_index_of[data[i] ^ par_idx[shift]];

      if(feedback != GF_ALPHA0) /* non-zero feedback term */
      {  register int spk = shift+1;
         register int *gpoly = rs_gpoly + nroots;

	 switch(nroots-spk)
	 {  
	    case 110: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	    case 109: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	    case 108: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	    case 107: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	    case 106: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	    case 105: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	    case 104: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	    case 103: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	    case 102: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	    case 101: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	    case 100: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 99: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 98: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 97: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 96: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 95: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 94: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 93: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 92: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 91: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 90: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 89: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 88: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 87: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 86: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 85: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 84: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 83: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 82: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 81: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 80: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 79: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 78: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 77: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 76: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 75: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 74: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 73: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 72: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 71: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 70: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 69: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 68: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 67: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 66: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 65: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 64: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 63: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 62: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 61: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 60: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 59: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 58: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 57: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 56: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 55: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 54: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 53: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 52: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 51: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 50: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 49: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 48: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 47: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 46: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 45: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 44: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 43: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 42: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 41: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 40: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 39: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 38: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 37: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 36: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 35: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 34: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 33: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 32: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 31: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 30: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 29: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 28: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 27: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 26: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 25: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 24: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 23: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 22: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 21: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 20: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 19: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 18: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 17: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 16: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 15: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 14: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 13: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 12: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 11: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 10: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case  9: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case  8: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case  7: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case  6: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case  5: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case  4: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case  3: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case  2: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case  1: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	 }
         spk = 0;

         switch(shift)
         {
             case 110: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 109: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 108: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 107: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 106: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 105: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 104: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 103: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 102: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 101: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	     case 100: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 99: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 98: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 97: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 96: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 95: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 94: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 93: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 92: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 91: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 90: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 89: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 88: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 87: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 86: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 85: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 84: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 83: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 82: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 81: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 80: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 79: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 78: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 77: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 76: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 75: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 74: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 73: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 72: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 71: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 70: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 69: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 68: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 67: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 66: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 65: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 64: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 63: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 62: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 61: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 60: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 59: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 58: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 57: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 56: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 55: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 54: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 53: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 52: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 51: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 50: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 49: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 48: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 47: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 46: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 45: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 44: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 43: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 42: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 41: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 40: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 39: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 38: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 37: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 36: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 35: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 34: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 33: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 32: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 31: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 30: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 29: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 28: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 27: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 26: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 25: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 24: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 23: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 22: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 21: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 20: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 19: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 18: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 17: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 16: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 15: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 14: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 13: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 12: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 11: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case 10: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case  9: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case  8: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case  7: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case  6: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case  5: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case  4: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case  3: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case  2: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	      case  1: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	   }
 	   par_idx[shift] = enc_alpha_to[feedback + rs_gpoly[0]];
       }
       else  /* zero feedback term */
	   par_idx[shift] = 0;

       par_idx += nroots;
   }
#if 0 /* shift now deliverd from external call */
   if(++(shift)>=nroots) shift=0;   /* shift */
#endif
}

/*
 * Wrapper around the optimized encoder routines
 */

void EncodeNextLayer(ReedSolomonTables *rt, unsigned char *data, unsigned char *parity, guint64 layer_size, int shift)
{
   switch(rt->nroots)
   {  
#if 0
      case 32:
        encode_layer_32(rse, data, parity, layer_size);
        break;

      case 64:
        encode_layer_64(rse, data, parity, layer_size);
        break;
#endif 
      default:
        encode_layer(rt, data, parity, layer_size, shift);
        break;
   }
}


