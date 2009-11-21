/*  pngpack: lossless image compression for a series of screen shots
 *  Copyright (C) 2005-2009 Carsten Gnoerlich.
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
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "pngio.h"
#include "md5.h"
#include "memory.h"

/***
 *** aux stuff
 ***/

void FreeImage(Image *i)
{
   if(i->image) free(i->image);
   if(i->file) fclose(i->file);
   if(i->row_pointers) free(i->row_pointers);
   if(i->png_read) png_destroy_read_struct(&i->png_read, &i->png_info, NULL);
   if(i->png_write) png_destroy_write_struct(&i->png_write, &i->png_info);

   free(i);
}


/***
 *** PNG reading
 ***/

Image *LoadPNG(char *name)
{  struct MD5Context md5ctxt;
   struct stat mystat;
   Image *pi; 
   png_byte *pb;
   unsigned char buf[256];
   unsigned int depth,size,i;
   png_color_16p background;
      
   fprintf(stdout,"Loading %s ... ", name);

   /* stat ppm file */

   if(stat(name, &mystat) == -1)
   {  fprintf(stdout, "COULD NOT STAT %s!\n", name);
      fflush(stdout);
      return NULL;
   }

   /* create image struct, really open */

   pi = calloc(1,sizeof(Image));
   if(!pi) Stop("out of memory for image");

   pi->name = name;

   pi->file = fopen(name, "rb");
   if(!pi->file)
   {  fprintf(stdout, "COULD NOT OPEN %s!\n", name);
      fflush(stdout);
      return NULL;
   }

   /* verify that we've got a png file */

   fread(buf, 1, 8, pi->file);
   if(png_sig_cmp(buf, 0, 8))
   {  fclose(pi->file);
      fprintf(stdout, "%s is not a .png file!\n", name);
      fflush(stdout);
      return NULL;
   }

   /* set up png data structs */

   pi->png_read  = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   pi->png_info = png_create_info_struct(pi->png_read);

   if(!pi->png_read || !pi->png_info)
     Stop("failed to initialize png structs");

   if(setjmp(png_jmpbuf(pi->png_read)))
   {  FreeImage(pi);
      fprintf(stdout, "error decoding .png file!\n");
      fflush(stdout);
      return NULL;
   }

   png_init_io(pi->png_read, pi->file);
   png_set_sig_bytes(pi->png_read, 8);

   /* read and evaluate info portion */

   png_read_info(pi->png_read, pi->png_info);

   pi->width  = png_get_image_width(pi->png_read, pi->png_info);
   pi->height = png_get_image_height(pi->png_read, pi->png_info);
   depth      = png_get_bit_depth(pi->png_read, pi->png_info);
   pi->channels = png_get_channels(pi->png_read, pi->png_info);

   fprintf(stdout, "%dx%d %s image",pi->width,pi->height,pi->channels==3?"RBG":"RBGA");

   if(depth != 8)
   {  FreeImage(pi);
      fprintf(stdout, ", ILLEGAL DEPTH: %d\n",depth);
      fflush(stdout);
      return NULL;
   }

   if(pi->channels == 3)
     png_set_filler(pi->png_read, 0, PNG_FILLER_AFTER);

   /* remember the png background color if there is one */

   if(png_get_bKGD(pi->png_read, pi->png_info, &background))
        pi->png_background = (background->red << 16) | (background->green << 8) | background->blue;
   else pi->png_background = 0;

   /* alloc memory for image */

   size = pi->width * pi->height;
   pi->bytesize = sizeof(unsigned int) * size;
   pi->image = malloc(pi->bytesize);
   if(!pi->image) Stop("out of memory for image");

   pi->row_pointers = malloc(sizeof(png_byte*) * pi->height);
   pb = (png_byte*)pi->image;

   for(i=0; i<pi->height; i++)
   {  pi->row_pointers[i] = pb;
      pb += pi->width*sizeof(unsigned int);
   }

   png_read_image(pi->png_read, pi->row_pointers);

   /* Clean up */

   fprintf(stdout,".\n");
   fflush(stdout);

   fclose(pi->file);
   png_destroy_read_struct(&pi->png_read, &pi->png_info, NULL);
   free(pi->row_pointers);

   /* calculate md5sum of image */

   MD5Init(&md5ctxt);
   MD5Update(&md5ctxt, (unsigned char*)pi->image, pi->bytesize);
   MD5Final(pi->checksum, &md5ctxt);

   return pi;
}

/***
 *** PNG writing
 ***/

void SavePNG(Image *pi, char *name)
{  png_byte *pb;
   unsigned int i; 
   png_color_16 background;

   /* open file */ 

   pi->file = fopen(name, "wb");
   if(!pi->file)
     Stop("Could not open %s: %s\n",name,strerror(errno));

   /* set up png data structs */

   pi->png_write  = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   pi->png_info = png_create_info_struct(pi->png_write);

   if(!pi->png_write || !pi->png_info)
     Stop("failed to initialize png structs");

   if(setjmp(png_jmpbuf(pi->png_write)))
   {  png_destroy_write_struct(&pi->png_write, &pi->png_info);
      fclose(pi->file);
      fprintf(stdout, "error creating .png file!\n");
      fflush(stdout);
      return;
   }

   png_init_io(pi->png_write, pi->file);

   /* supply image info to png library */

   png_set_compression_level(pi->png_write, Z_BEST_COMPRESSION);

   png_set_IHDR(pi->png_write, pi->png_info,
		pi->width, pi->height, 8,
		pi->channels == 4 ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);

   background.red   = (pi->png_background >> 16) & 0xff;
   background.green = (pi->png_background >> 8)  & 0xff;
   background.blue  =  pi->png_background        & 0xff;
   png_set_bKGD(pi->png_write, pi->png_info, &background);

   pi->row_pointers = malloc(sizeof(png_byte*) * pi->height);
   pb = (png_byte*)pi->image;

   for(i=0; i<pi->height; i++)
   {  pi->row_pointers[i] = pb;
      pb += pi->width*sizeof(unsigned int);
   }

   /* and write it out */

   png_write_info(pi->png_write, pi->png_info);
   if(pi->channels == 3)
     png_set_filler(pi->png_write, 0, PNG_FILLER_AFTER);
   png_write_image(pi->png_write, pi->row_pointers);
   png_write_end(pi->png_write, NULL);

   /* clean up */

   if(fclose(pi->file))
     Stop("Could not close %s: %s\n",name,strerror(errno));

   free(pi->row_pointers);
   png_destroy_write_struct(&pi->png_write, &pi->png_info);
}
