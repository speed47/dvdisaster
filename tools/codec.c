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
#include <stdio.h>
#include <stdlib.h>

#include <bzlib.h>

#include "pngio.h"
#include "codec.h"
#include "md5.h"
#include "memory.h"

/***
 *** Find most used color 
 ***/

static void find_background(Image *pi)
{  unsigned int *pixel,*count,*color,size,t_used,t_max,i,max_color;

   color = calloc(4, sizeof(unsigned int)); 
   count = calloc(4, sizeof(unsigned int)); 
   t_used = 1;
   t_max  = 4;

   size  = pi->width * pi->height;
   pixel = pi->image;

   color[0] = *pixel;

   while(size--)
   {  for(i=0; i<t_used; i++)
        if(color[i] == *pixel) 
	{  count[i]++;
	   break;
	}

      if(i == t_used)
      {  t_used++;  

         if(t_used == t_max)
	 { t_max *= 2; 
	   color = realloc(color, t_max * sizeof(unsigned int));
	   count = realloc(count, t_max * sizeof(unsigned int));
	   if(!color && !count) Stop("out of memory enlarging color table to %d", t_max);
	 }

	 color[t_used-1] = *pixel;
	 count[t_used-1] = 1;
      }

      pixel++;
   }

   max_color = 0;
   for(i=0; i<t_used; i++)
     if(count[i] > max_color)
     {  max_color += count[i];
        pi->tile_background = color[i];
     }

   fprintf(stdout, "  %d colors, tile background is %x (%d pixels).\n", t_used,pi->tile_background,max_color);

   free(color);
   free(count);
}

/***
 *** Maintain the tile database
 ***/

typedef struct
{  unsigned int image[MAX_TILE_SIZE][MAX_TILE_SIZE];
   unsigned int width, height;
   unsigned int background;
} tile;

typedef struct
{  unsigned int image[2*MAX_TILE_SIZE][MAX_TILE_SIZE];
   unsigned int width, height;
   unsigned int basey;
   unsigned int basex,minx,maxx;
} work_tile;


typedef struct
{  int x,y;          /* Position of tile */
   unsigned int n;   /* Index of tile */
} opcode;

Image **img_list;
unsigned int img_n,img_max;
tile **tile_db;
unsigned int db_n,db_max;
opcode *oc_list;
unsigned int oc_n,oc_max;

void InitTileDatabase()
{
   img_max = db_max = oc_max = 4;
   img_list  = malloc(sizeof(Image*)*img_max);
   tile_db   = malloc(sizeof(tile*)*db_max);
   oc_list   = malloc(sizeof(opcode)*oc_max);
}

void FreeTileDatabase()
{  int i;

   for(i=0; i<img_n; i++)
      free(img_list[i]);
   free(img_list);

   for(i=0; i<db_n; i++)
      free(tile_db[i]);
   free(tile_db);

   free(oc_list);
}

static void add_image(Image *pi)
{
   if(++img_n >= img_max)
   {  img_max *= 2;
      img_list = realloc(img_list, sizeof(Image*)*img_max);
      if(!img_list) Stop("out of memory expanding image list");
   }

   img_list[img_n-1] = pi;
}

static int tilecmp(tile *a, tile *b)
{  unsigned int i,j;

   if(a->width != b->width || a->height != b->height) return 1;

   for(j=0; j<a->height; j++)
     for(i=0; i<a->width; i++)
       if(    a->image[i][j] != b->image[i][j]
	  && (a->image[i][j] != a->background || b->image[i][j] != b->background)
	 )
	 return 1;

   return 0;
}

static int add_tile(tile *new_tile, int x, int y)
{  unsigned int idx;
   opcode *lidx;
   int new_needed = 1;

   for(idx = 0; idx < db_n; idx++)
     if(!tilecmp(new_tile, tile_db[idx]))
     {  new_needed = 0;
        break;
     }

   if(new_needed)
   {  idx = db_n; 
      if(++db_n >= db_max)
      {  db_max *= 2;
         tile_db = realloc(tile_db, sizeof(tile*)*db_max);
         if(!tile_db) Stop("out of memory expanding tile data base");
      }
   
      tile_db[idx] = malloc(sizeof(tile));
      if(!tile_db[idx]) Stop("out of memory adding tile to the data base");
      memcpy(tile_db[idx], new_tile, sizeof(tile));
   }

   if(++oc_n >= oc_max)
   {  oc_max *= 2;
      oc_list = realloc(oc_list, sizeof(opcode)*oc_max);
      if(!oc_list) Stop("out of memory expanding opcode table");
   }

   lidx = &oc_list[oc_n - 1];
   lidx->x = x;
   lidx->y = y;
   lidx->n = idx;

   return new_needed;
}

/***
 *** Transform image into tiles
 ***/

unsigned int clr[] = { 0xff0000, 0x00ff00, 0xffff00, 0x00ffff, 0x0000ff };

#define PIXEL(pi,px,py) (pi->image[(px)+(py)*pi->width])

static void create_tile(Image *pi, work_tile *t, unsigned int x, unsigned int y)
{  
   /* Stop recursion if background or maximal tile size is reached */ 

   if(   (x < t->minx || x > t->maxx)
      && t->maxx-t->minx >= MAX_TILE_SIZE-1) 
     return;

   if(y < t->basey) return;
   if(y - t->basey >= MAX_TILE_SIZE) return;

   if(PIXEL(pi,x,y) == pi->tile_background) 
     return;

   /* Add current pixel to the tile */

   t->image[(MAX_TILE_SIZE+x)-t->basex][y-t->basey] = PIXEL(pi,x,y);
   if(x < t->minx) { t->minx=x; t->width = t->maxx - t->minx + 1; }
   if(x > t->maxx) { t->maxx=x; t->width = t->maxx - t->minx + 1; }
   if(y - t->basey + 1 > t->height) t->height = y - t->basey + 1;

   PIXEL(pi,x,y) = pi->tile_background;

   /* Recursively check the neighboring pixels */

   if(x>0)            create_tile(pi, t, x-1, y);
   if(x<pi->width-1)  create_tile(pi, t, x+1, y);
   if(y>0)            create_tile(pi, t, x, y-1);
   if(y<pi->height-1) create_tile(pi, t, x, y+1);
}

static void create_tiles(Image *pi)
{  work_tile *wt = malloc(sizeof(work_tile));
   tile *t = malloc(sizeof(tile));
   unsigned int x,y;
   //   int cidx=0;
   int n_tiles = 0;
   int r_tiles = 0;
   int deltax,deltay;
   int lastx = 0, lasty = 0;

   t->background = pi->tile_background;

   for(y=0; y<pi->height; y++)
     for(x=0; x<pi->width; x++)
     {  int i,j,off;

        if(PIXEL(pi,x,y) == pi->tile_background)
          continue;
     
        /* initialize the tile */ 

        for(i=0; i<2*MAX_TILE_SIZE; i++) 
          for(j=0; j<MAX_TILE_SIZE; j++) 
	    wt->image[i][j] = pi->tile_background;

	wt->width = wt->height = 1;
	wt->basey = y;
	wt->basex = wt->minx  = wt->maxx = x;
        create_tile(pi, wt, x, y); 
 
	off = (MAX_TILE_SIZE + wt->minx) - wt->basex;
        for(i=0; i<MAX_TILE_SIZE; i++) 
          for(j=0; j<MAX_TILE_SIZE; j++) 
	    t->image[i][j] = wt->image[i+off][j];

	t->width  = wt->width;
	t->height = wt->height;

	deltax = x+wt->minx-wt->basex - lastx;
	deltay = y - lasty;
	if(add_tile(t, deltax, deltay)) n_tiles++;
	else                r_tiles++;
	lastx += deltax;
	lasty = y;
     }

   fprintf(stdout, "  %d new tiles, %d reused tiles\n",n_tiles,r_tiles);

#if 0
   lastx = lasty = 0;
   for(x=0; x<oc_n; x++)
   {  tile *t;
      opcode *oc;
      int i,j;

      oc = &oc_list[x];
      t  = tile_db[oc->n];
      lastx += oc->x;
      lasty += oc->y;

      for(i=0; i<t->width; i++) 
	for(j=0; j<t->height; j++)
	  if(t->image[i][j] != pi->tile_background)
#if 1
	    PIXEL(pi, i+lastx, j+lasty) = clr[cidx];
#else
      	    PIXEL(pi, i+lastx, j+lasty) = t->image[i][j]; 
#endif

      cidx = (cidx+1)%5;
   } 
#endif 

   free(wt);
   free(t);
}

/***
 *** .ppk format loading and saving
 ***/

static void bz_write(BZFILE *b, void *buf, int size)
{  int bzerror;

   BZ2_bzWrite(&bzerror, b, buf, size);
   
   if(bzerror != BZ_OK)
   {  BZ2_bzWriteClose(&bzerror, b, 0, NULL, NULL);
      Stop("Write error in bz2 library: %s\n",strerror(errno));
   }
}

static void save_int(BZFILE *file, int value)
{  unsigned char buf[4];
   int bzerror;

   buf[0] = value>>24 & 0xff;
   buf[1] = value>>16 & 0xff;
   buf[2] = value>> 8 & 0xff;
   buf[3] = value     & 0xff;

   BZ2_bzWrite(&bzerror, file, buf, 4);

   if(bzerror != BZ_OK)
   {  BZ2_bzWriteClose(&bzerror, file, 0, NULL, NULL);
      Stop("Write error in bz2 library: %s\n",strerror(errno));
   }
}

static void save_uint(BZFILE *file, unsigned int value)
{  unsigned char buf[4];
   int bzerror;

   buf[0] = value>>24 & 0xff;
   buf[1] = value>>16 & 0xff;
   buf[2] = value>> 8 & 0xff;
   buf[3] = value     & 0xff;

   BZ2_bzWrite(&bzerror, file, buf, 4);

   if(bzerror != BZ_OK)
   {  BZ2_bzWriteClose(&bzerror, file, 0, NULL, NULL);
      Stop("Write error in bz2 library: %s\n",strerror(errno));
   }
}

void SavePPK(char *name)
{  FILE *file;
   BZFILE *bzfile; 
   unsigned int i; 
   int bzerror;

   fprintf(stdout, "Compressing pingpack archive...");
   fflush(stdout);

   file = fopen(name, "wb");

   if(!file)
     Stop("Could not open %s: %s\n",name,strerror(errno));

   bzfile = BZ2_bzWriteOpen(&bzerror, file, 9, 0, 30);

   if(bzerror != BZ_OK)
     Stop("Could not open %s for bz2\n",name);

   /* The header contains the string ".pngpack", four zeros and the FILEFORMAT as a 4 byte value. */

   bz_write(bzfile, ".pngpack", 8);
   save_int(bzfile, 0);
   save_int(bzfile, FILEFORMAT);

   /* The image section contains the string "Images\000\000",
      the number of images as a 4 byte value,
      and the image description itself.
      The image name is preceeded by its length as a 4 byte value. */

   bz_write(bzfile, "Images\000\000", 8);
   save_int(bzfile, img_n);

   for(i=0; i<img_n; i++)
   {  Image *pi = img_list[i];
      unsigned int len = strlen(pi->name);

      save_uint(bzfile, pi->width);
      save_uint(bzfile, pi->height);
      bz_write(bzfile, pi->checksum, 16);
      save_uint(bzfile, pi->tile_background);
      save_uint(bzfile, pi->png_background);
      save_uint(bzfile, pi->channels);
      save_uint(bzfile, pi->first_opcode);
      save_uint(bzfile, pi->last_opcode);
      save_uint(bzfile, len);
      bz_write(bzfile, pi->name, len);
   }

   /* The opcode section contains the string "Opcodes\000",
      the number of opcodes as a 4 byte value,
      and then the opcodes itself. */

   bz_write(bzfile, "Opcodes\000", 8);
   save_uint(bzfile, oc_n);

   for(i=0; i<oc_n; i++)
   {  opcode *oc = &oc_list[i];

      save_int(bzfile, oc->x);
      save_int(bzfile, oc->y);
      save_uint(bzfile, oc->n);
   }

   /* The tile section contains the string "Tiles\000\000\000",
      the number of tiles as a 4 byte value,
      and then the tiles itself. */

   bz_write(bzfile, "Tiles\000\000\000", 8);
   save_uint(bzfile, db_n);

   for(i=0; i<db_n; i++)
   {  tile *t = tile_db[i];
      unsigned int x,y;

      save_uint(bzfile, t->width);
      save_uint(bzfile, t->height);

      for(y=0; y<t->height; y++)
	for(x=0; x<t->width; x++)
	  save_uint(bzfile, t->image[x][y]);
   }

   BZ2_bzWriteClose(&bzerror, bzfile, 0, NULL, NULL);

   if(bzerror != BZ_OK)
     Stop("Failed to close bz2 file handle: %s\n",strerror(errno));

   if(fclose(file))
     Stop("Could not close %s: %s\n",name,strerror(errno));

   fprintf(stdout, "DONE.\n");
}

static void bz_read(BZFILE *b, void *buf, int size)
{  int bzerror,ignore;

   BZ2_bzRead(&bzerror, b, buf, size);
   
   if(bzerror != BZ_OK && bzerror != BZ_STREAM_END)
   {  BZ2_bzReadClose(&ignore, b);
      Stop("Read error in bz2 library: %d,%s\n",bzerror,strerror(errno));
   }
}

static int load_int(BZFILE *file)
{  unsigned char buf[4];
   int bzerror, ignore;

   BZ2_bzRead(&bzerror, file, buf, 4);
   
   if(bzerror != BZ_OK && bzerror != BZ_STREAM_END)
   {  BZ2_bzReadClose(&ignore, file);
      Stop("Read error in bz2 library: %d,%s\n",bzerror,strerror(errno));
   }

   return buf[0]<<24 | buf[1]<<16 | buf[2]<<8 | buf[3];
}

static unsigned int load_uint(BZFILE *file)
{  unsigned char buf[4];
   int bzerror,ignore; 

   BZ2_bzRead(&bzerror, file, buf, 4);
   
   if(bzerror != BZ_OK && bzerror != BZ_STREAM_END)
   {  BZ2_bzReadClose(&ignore, file);
      Stop("Read error in bz2 library: %d,%s\n",bzerror,strerror(errno));
   }

   return buf[0]<<24 | buf[1]<<16 | buf[2]<<8 | buf[3];
}

void LoadPPK(char *name, Image ***list_out, int *n_out)
{  FILE *file;
   BZFILE *bzfile; 
   char header[9]; 
   unsigned int i,file_format;
   int bzerror;
   int j;

   file = fopen(name, "rb");
   if(!file)
     Stop("Could not open %s: %s\n",name,strerror(errno));

   bzfile = BZ2_bzReadOpen(&bzerror, file, 0, 0, NULL, 0);
   if(bzerror != BZ_OK)
     Stop("Could not open %s for bz2\n",name);

   /* evaluate the header */

   bz_read(bzfile, header, 8);
   if(strncmp(header, ".pngpack", 8))
     Stop("%s is not a pngpack file",name);

   file_format = load_int(bzfile);  /* always zero */
   file_format = load_int(bzfile);

   /* read the image list */

   bz_read(bzfile, header, 8);
   if(strncmp(header, "Images\000\000", 8))
     Stop("%s: missing images chunk",name);
   
   img_n    = load_uint(bzfile);
   img_list = realloc(img_list, sizeof(Image*)*img_n);
   if(!img_list) Stop("out of memory allocating the image table");

   fprintf(stdout, "%s contains %d images:\n",name,img_n);

   for(i=0; i<img_n; i++)
   {  int len;
      Image *pi;

      pi = img_list[i] = malloc(sizeof(Image));
      if(!pi) Stop("out of memory allocating image structure");

      pi->width  = load_uint(bzfile);
      pi->height = load_uint(bzfile);
      bz_read(bzfile, pi->checksum, 16);
      pi->tile_background = load_uint(bzfile);
      pi->png_background = load_uint(bzfile);
      pi->channels = load_uint(bzfile);
      pi->first_opcode = load_uint(bzfile);
      pi->last_opcode = load_uint(bzfile);
      len = load_uint(bzfile);

      pi->name = malloc(len+1);
      if(!pi->name) Stop("out of memory allocating image name");
      bz_read(bzfile, pi->name, len);
      pi->name[len] = 0;

      fprintf(stdout, "%4d x %4d: %s\n",pi->width,pi->height,pi->name);
   }

   /* read the opcode list */

   bz_read(bzfile, header, 8);
   if(strncmp(header, "Opcodes\000", 8))
     Stop("%s: missing opcodes chunk",name);
   
   oc_n    = load_uint(bzfile);
   oc_list = realloc(oc_list,sizeof(opcode)*oc_n);
   if(!oc_list) Stop("out of memory allocating the opcode table");

   for(i=0; i<oc_n; i++)
   {  opcode *oc = &oc_list[i];

      oc->x = load_int(bzfile);
      oc->y = load_int(bzfile);
      oc->n = load_uint(bzfile);
   }

   /* read the tile list */

   bz_read(bzfile, header, 8);
   if(strncmp(header, "Tiles\000\000\000", 8))
     Stop("%s: missing tiles chunk",name);

   db_n = load_uint(bzfile);
   tile_db = realloc(tile_db,sizeof(tile*)*db_n);
   if(!tile_db) Stop("out of memory allocating the tile data base");

   for(i=0; i<db_n; i++)
   {  tile *t = malloc(sizeof(tile));
      unsigned int x,y;

      if(!t) Stop("out of memory allocating more tiles");
      tile_db[i] = t;

      t->width = load_uint(bzfile);
      t->height = load_uint(bzfile);

      for(y=0; y<t->height; y++)
	for(x=0; x<t->width; x++)
	  t->image[x][y] = load_uint(bzfile);
   }

   fprintf(stdout, "%d tiles, %d opcodes\n\n", db_n, oc_n);

   BZ2_bzReadClose(&bzerror, bzfile);

   if(bzerror != BZ_OK)
     Stop("Failed to close bz2 file handle: %s\n",strerror(errno));

   fclose(file);

   /* restore original background value of each tile */

   for(j=img_n-1; j>=0; j--)
   {  Image *pi = img_list[j];

      for(i=pi->first_opcode; i<=pi->last_opcode; i++)
      {  opcode *oc = &oc_list[i];
         tile *t = tile_db[oc->n];

	 t->background = pi->tile_background;
      }
   }

   *n_out = img_n;
   *list_out = img_list;
}

/***
 *** Encode image (add it to the tile data base)
 ***/

void EncodeImage(Image *pi)
{
   pi->first_opcode = oc_n;
   find_background(pi);
   add_image(pi);
   create_tiles(pi);
   pi->last_opcode = oc_n-1;
}

/***
 *** Render image from the ppk
 ***/

void RenderImage(Image *pi)
{  struct MD5Context md5ctxt;
   unsigned char checksum[16]; 
   unsigned int oidx,i,*p;
   unsigned int background;
   int x=0, y=0;

   /* Clear the image */ 

   i = pi->width * pi->height; 
   p = pi->image;

#ifdef HAVE_LITTLE_ENDIAN
   background = pi->tile_background;
   while(i--)
     *p++ = background;
#else
   background = SwapBytes32(pi->tile_background);
   while(i--)
     *p++ = background;
#endif

   /* Render it */

   for(oidx=pi->first_opcode; oidx<=pi->last_opcode; oidx++)
   {  tile *t;
      opcode *oc;
      unsigned int i,j;

      oc = &oc_list[oidx];
      t  = tile_db[oc->n];

      x += oc->x;
      y += oc->y;

      for(i=0; i<t->width; i++) 
	for(j=0; j<t->height; j++)
	  if(t->image[i][j] != t->background)
#ifdef HAVE_LITTLE_ENDIAN
	    PIXEL(pi, i+x, j+y) = t->image[i][j]; 
#else
            PIXEL(pi, i+x, j+y) = SwapBytes32(t->image[i][j]); 
#endif
   }  

   /* verify md5sum */

   MD5Init(&md5ctxt);
   MD5Update(&md5ctxt, (unsigned char*)pi->image, pi->bytesize);
   MD5Final(checksum, &md5ctxt);

   if(!memcmp(pi->checksum, checksum, 16))
         fprintf(stdout, "\n");
   else  fprintf(stdout, " - DECODING FAILURE (checksum error).\n");
}
