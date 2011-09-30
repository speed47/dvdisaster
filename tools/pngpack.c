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

#define VERSION "0.20"

#include <getopt.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "md5.h"
#include "pngio.h"
#include "codec.h"
#include "decimate.h"
#include "memory.h"

/***
 *** main()
 ***/

typedef enum
{  MODE_NONE, 
   MODE_HELP,
   MODE_PACK,
   MODE_UNPACK,

   MODIFIER_THUMBS
} run_mode;

int main(int argc, char *argv[])
{  int mode = MODE_NONE;
   char *arch_file = NULL;
   int thumbnails = 0;
   int thumb_width = 160;
   char *thumb_dir = strdup("../thumbnails");

   /*** Parse the options */

   for(;;)
   {  int option_index,c;

      static struct option long_options[] =
      { {"help", 0, 0, 'h'},
	{"pack", 1, 0, 'p'},
	{"unpack", 1, 0, 'u'},
	{"thumbnails", 2, 0, 't'},
	{0, 0, 0, 0}
      };

      c = getopt_long(argc, argv, 
		      "hp:u:t::",
		      long_options, &option_index);
      if(c == -1) break;

      switch(c)
      {  case 'p': mode = MODE_PACK; 
	           arch_file = strdup(optarg);
		   break;
	 case 'u': mode = MODE_UNPACK;
	           arch_file = strdup(optarg);
		   break;
	 case 't': thumbnails=1;
	           if(optarg)
		   {  char *copy=strdup(optarg);
		      char *comma=strchr(copy, ',');
		      if(comma) 
		      { *comma=0;
			thumb_width=atoi(copy);
			free(thumb_dir);
			thumb_dir=strdup(comma+1);
			free(copy);
		      }
		      else
		      {  int val = strtol(optarg, NULL, 0);
			 if(val > 0 ) thumb_width=val;
			 else         thumb_dir=copy;
		      }
		   }
		   break;
	 case 'h': 
         case '?':
	    mode = MODE_HELP; break;
	    break;
         default: fprintf(stdout, "?? illegal getopt return value %d\n",c); break;
      }
   }

   /*** Perform the action */
   
   if(mode != MODE_NONE && mode != MODE_HELP)
      fprintf(stdout, "pngpack-0.20 *** Copyright 2005-2009 Carsten Gnoerlich.\n"
	      "This software comes with  ABSOLUTELY NO WARRANTY.  This\n"
	      "is free software and you are welcome to redistribute it\n"
	      "under the conditions of the GNU GENERAL PUBLIC LICENSE.\n"  
	      "See the file \"COPYING\" for further information.\n\n");

   switch(mode)
   {  case MODE_PACK:
      {  Image *pi;
	 int i;

	 InitTileDatabase();

	 for(i=optind; i<argc; i++)
	 {
	    pi = LoadPNG(argv[i]);
	    if(!pi) continue;
	    EncodeImage(pi);
	    free(pi->image);
	 }

	 SavePPK(arch_file);
	 FreeTileDatabase();
      }
	 break;

      case MODE_UNPACK:
      {  Image **img_list;
	 int img_n;
	 unsigned int i;

	 InitTileDatabase();
   	 LoadPPK(arch_file, &img_list, &img_n);
	 for(i=0; i<img_n; i++)
	 {  struct stat mystat;
	    Image *pi = img_list[i];
	    char *c;

	    /* Do not overwrite existing files */

	    if(stat(pi->name, &mystat) != -1)
	    {  c = strrchr(pi->name, '.');
	       if(c) 
	       {  char  *n = malloc(strlen(pi->name)+2);
		  *c = 0;
		  
		  sprintf(n, "_%s.png", pi->name);
		  free(pi->name);
		  pi->name = n;
	       }
	    }

	    fprintf(stdout, "rendering %s (opcodes %d - %d)",pi->name,pi->first_opcode,pi->last_opcode);

	    pi->bytesize = sizeof(unsigned int) * pi->width*pi->height;
	    pi->image = malloc(pi->bytesize);
	    if(!pi->image) Stop("out of memory for image");

	    RenderImage(pi);
	    SavePNG(pi, pi->name);
	    if(thumbnails)
	       SaveThumbnail(pi, pi->name, thumb_width, thumb_dir);

	    free(pi->name);
	    free(pi->image);
	 }
	 FreeTileDatabase();
      }
	 break;

      case MODE_NONE:
      case MODE_HELP:
	 fprintf(stdout, "Usage: pngpack [OPTION...] [FILE]...\n"
		"pngpack maintains archives of PNG images. Depending on the similarity between\n"
		"the images, very high compression ratios can be reached. It is especially\n"
		"suitable for distribution of screen shots for online documentation.\n\n"
		"Examples:\n"
		"  pngpack --pack arch.pngpack img1.png img2.png ... - create archive\n"
		"  pngpack --unpack arch.pngpack                     - unpack archive\n"
		"  pngpack --unpack arch.pngpack --thumbnails 160 ../thumbs\n"
		"    unpacks archive and creates thumbnails of width 160\n"
		"    in the directory ../thumbs\n\n"
		"Main operation:\n"
		"-p, --pack=ARCH     - pack png images into archive\n"
		"-u, --unpack=ARCH   - unpack png images from archive\n\n"
		"Modifiers:\n"
		"-t, --thumbnails=WIDTH,DIR\n"
		"   Create thumbnails while unpacking.\n"
		"   Defaults are WIDTH=160 and DIR=../thumbnails\n");
	 exit(EXIT_FAILURE);
   }

   free(arch_file);
   free(thumb_dir);
#ifdef WITH_MEMDEBUG_YES
   CheckMemleaks();
#endif
   return EXIT_SUCCESS;
}
