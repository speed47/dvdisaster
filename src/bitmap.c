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
 *** A simple bitmap structure
 ***/

/*
 * Allocate the bitmap
 */

Bitmap* CreateBitmap0(int size)
{  Bitmap *bm = g_malloc(sizeof(Bitmap));

   bm->size   = size;
   bm->words  = (size>>5)+1;
   bm->bitmap = g_malloc0(bm->words*sizeof(guint32));

   return bm;
}

/* 
 * Free it
 */

void FreeBitmap(Bitmap *bm)
{  if(bm->bitmap)
     g_free(bm->bitmap);

   g_free(bm);
}

/*
 * Count the '1' bits in the bitmap 
 */

gint32 CountBits(Bitmap *bm)
{ gint32 i;
  gint32 sum = 0;

  for(i=0; i<bm->size; i++)
    if(GetBit(bm, i))
      sum++;

  return sum;
}
