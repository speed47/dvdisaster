/*  pngpack: lossless image compression for a series of screen shots
 *  Copyright (C) 2005-2010 Carsten Gnoerlich.
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

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "pngio.h"
#include "memory.h"

/*
 * Thumbnail generation
 */

void SaveThumbnail(Image *pi, char *name, int thumb_size, char *thumb_dir)
{  Image *thumb;
   char tname[strlen(thumb_dir)+strlen(name)+2];
   double ratio, inv_ratio;
   unsigned int size, *pixel;
   int i,j;

   /* sanity check */

   if(pi->width < thumb_size && pi->height < thumb_size)
   {  fprintf(stdout, "... NOT creating thumbnail because of image size (%dx%d)\n",
	      pi->width, pi->height);
      return;
   }

   /* Create thumbnail copy from image */

   thumb=malloc(sizeof(Image));
   memcpy(thumb, pi, sizeof(Image));

   if(thumb->width > thumb->height) 
        inv_ratio = (double)thumb->width/(double)thumb_size;
   else inv_ratio = (double)thumb->height/(double)thumb_size;

   ratio = 1.0/inv_ratio;

   thumb->width  = (int)((double)thumb->width*ratio+0.5);
   thumb->height = (int)((double)thumb->height*ratio+0.5);

   sprintf(tname, "%s/%s", thumb_dir, name);

   fprintf(stdout, "... and %dx%d thumbnail %s\n", thumb->width, thumb->height, tname);

   size = thumb->width * thumb->height;
   thumb->bytesize = sizeof(unsigned int) * size;
   thumb->image = malloc(thumb->bytesize);
   if(!thumb->image) Stop("out of memory for image");

   /*
    * Decimate the image 
    */

   pixel = thumb->image;
   for(i=0; i<thumb->height; i++)
      for(j=0; j<thumb->width; j++)
      {  double first_x = j*inv_ratio;
	 double first_y = i*inv_ratio;
	 double last_x  = (j+1)*inv_ratio;
	 double last_y  = (i+1)*inv_ratio;
	 double area = 0.0;
	 int x0 = floor(first_x);
	 int x1 = ceil(last_x);
	 int y0 = floor(first_y);
	 int y1 = ceil(last_y);
	 int x,y;
	 double c1,c2,c3,c4;
	 unsigned int ci1,ci2,ci3,ci4;
#if 0
	 printf("%d %d -> (%d %d) - (%d %d)\n", j,i, x0,y0, x1, y1);
	 printf("%f %f %f %f\n", first_x, first_y, last_x, last_y);
	 printf("%f %f\n", ratio, inv_ratio);
#endif
	 if(x1>=pi->width) x1=pi->width-1;
	 if(y1>=pi->height) y1=pi->height-1;

	 c1 = c2 = c3 = c4 = 0.0;
	 for(y=y0; y<=y1; y++)
	 {  double ysize = 1.0;
	    double size;

	    if(y<y0) ysize = 1.0-(y0-y);
	    if(y>y1) ysize = 1.0-(y-y1);

	    for(x=x0; x<=x1; x++)
	    {  int idx = x+y*pi->width;
	       size = ysize;
	       if(x<x0) size *= (1.0-(x0-x));
	       if(x>x1) size *= (1-0-(x-x1));

	       c1 += size * ((pi->image[idx]>>24)&0xff);
	       c2 += size * ((pi->image[idx]>>16)&0xff);
	       c3 += size * ((pi->image[idx]>> 8)&0xff);
	       c4 += size * ((pi->image[idx]    )&0xff);
	       area += size;
	    }
	 }

	 ci1 = c1/area; ci2 = c2/area; ci3 = c3/area; ci4 = c4/area;
	 *pixel++ = (ci1<<24) | (ci2<<16) | (ci3<<8) | ci4;
      }

   SavePNG(thumb, tname);

   free(thumb->image);
   free(thumb);
}
