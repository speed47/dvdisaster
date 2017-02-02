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

#include "rs02-includes.h"
#include "rs03-includes.h"
#include "udf.h"

#include <time.h>

/***
 *** Debugging functions.
 ***/

/*
 * Debugging function to seed the image with random correctable errors
 */

/* RS01-style files */

static void random_error1(Image *image, char *arg)
{  EccHeader *eh;
  gint64 block_idx[255];
   gint64 s,si;
   int block_sel[255];
   int i,percent,last_percent = 0;
   int n_data,n_errors;
   double eras_scale, blk_scale;

   SRandom(Closure->randomSeed);
   eh = image->eccFileHeader;
      
   n_errors = atoi(arg);

   if(n_errors < 1|| n_errors > eh->eccBytes)
     Stop(_("Number of erasures must be > 0 and <= %d\n"), eh->eccBytes);

   n_data = 255-eh->eccBytes;
   eras_scale = (n_errors+1)/((double)MY_RAND_MAX+1.0);
   blk_scale = (double)n_data/((double)MY_RAND_MAX+1.0);


   /*** Setup block pointers */

   s = (image->sectorSize+n_data-1)/n_data;

   for(si=0, i=0; i<n_data; si+=s, i++)
     block_idx[i] = si;

   PrintLog(_("\nGenerating random correctable erasures (%s; for %d roots, max erasures = %d).\n"),
	    "RS01", eh->eccBytes, n_errors);

   /*** Randomly delete the blocks */

   for(si=0; si<s; si++)
   {  int n_erasures = (int)(eras_scale*(double)Random());

      /* Reset the block selector */

      for(i=0; i<n_data; i++)  
	block_sel[i] = 0;

      /* Randomly pick n blocks */

      for(i=0; i<n_erasures; i++)
      {  int idx;
      
         do
	 {  idx = (int)(blk_scale*(double)Random());
	 } while(block_sel[idx]);

	 block_sel[idx] = 1;
      }

      /* Delete the randomly picked blocks */

      for(i=0; i<n_data; i++)
      {  unsigned char missing[2048];
 	int write_size = 2048;
	 
	 if(block_sel[i] && block_idx[i]<image->sectorSize)
	 {  if(!LargeSeek(image->file, (gint64)(2048*block_idx[i])))
	       Stop(_("Failed seeking to sector %lld in image: %s"),block_idx[i],strerror(errno));

	    CreateMissingSector(missing, block_idx[i], image->imageFP, FINGERPRINT_SECTOR, NULL); 

	    if(block_idx[i] == image->sectorSize - 1 && image->inLast < 2048)
	      write_size = image->inLast;

	    if(LargeWrite(image->file, missing, write_size) != write_size)
	       Stop(_("Failed writing to sector %lld in image: %s"),block_idx[i],strerror(errno));
	 }
	 
	 block_idx[i]++;
      }

      percent = (100*si)/s;
      if(last_percent != percent) 
      {  PrintProgress(_("Progress: %3d%%"),percent);
	 last_percent = percent;
      }
   }

   PrintProgress(_("Progress: 100%%\n"
	"Recover the image using the --fix option before doing another --random-errors run.\n"
	"Otherwise you'll accumulate >= %d erasures/ECC block and the image will be lost.\n"), 
	n_errors);
}

/* RS02 ecc images */

static void random_error2(Image *image, char *arg)
{  EccHeader *eh = image->eccHeader;
   RS02Layout *lay;
   gint64 si;
   guint64 hpos;
   guint64 end;
   guint64 header[42];
   int block_sel[255];
   int i,percent,last_percent = 0;
   int hidx,n_errors,erase_max = 0;
   double eras_scale, blk_scale, hdr_scale;

   SRandom(Closure->randomSeed);
   lay = RS02LayoutFromImage(image);

   n_errors = atoi(arg);

   if(n_errors < 0)
   {  erase_max = 1;
      n_errors = -n_errors;
   }

   if(n_errors <= 0 || n_errors > eh->eccBytes)
     Stop(_("Number of erasures must be > 0 and <= %d\n"), eh->eccBytes);

   eras_scale = (n_errors+1)/((double)MY_RAND_MAX+1.0);
   blk_scale  = (double)255.0/((double)MY_RAND_MAX+1.0);

   PrintLog(_("\nGenerating random correctable erasures (%s; for %d roots, max erasures = %d).\n"), 
	    "RS02", eh->eccBytes, n_errors);

   /*** Randomly delete some ecc headers */

   header[0] = lay->firstEccHeader;
   hidx = 1;

   hpos = (lay->protectedSectors + lay->headerModulo - 1) / lay->headerModulo;
   hpos *= lay->headerModulo;

   end = lay->eccSectors+lay->dataSectors-2;

   while(hpos < end)  /* Calculate positions of all headers */
   { 
      header[hidx++] = hpos;
      hpos += lay->headerModulo;
   }

   /* Pick one header to remain intact.
      Currently this must be one of the repeated headers */

   hdr_scale = (double)(hidx-1)/((double)MY_RAND_MAX+1.0);
   header[(int)(hdr_scale*(double)Random())+1] = 0;

   for(i=0; i<hidx; i++)
   {  gint64 s = header[i];
      if(s>0)
      {  unsigned char missing[2048];
      
	 if(!LargeSeek(image->file, (gint64)(2048*s)))
	    Stop(_("Failed seeking to sector %lld in image: %s"), s, strerror(errno));

	 CreateMissingSector(missing, s, image->imageFP, image->fpSector, NULL); 

         if(LargeWrite(image->file, missing, 2048) != 2048)
	    Stop(_("Failed writing to sector %lld in image: %s"), s, strerror(errno));
      }
   }

   /*** Randomly delete the blocks */

   for(si=0; si<lay->sectorsPerLayer; si++)
   {  int n_erasures = (int)(eras_scale*(double)Random());
      if(erase_max)
	n_erasures = n_errors;

      /* Reset the block selector */

      for(i=0; i<255; i++)  
	block_sel[i] = 0;

      /* Randomly pick n blocks */

      for(i=0; i<n_erasures; i++)
      {  int idx;
      
         do
	 {  idx = (int)(blk_scale*(double)Random());
	 } while(block_sel[idx]);

	 block_sel[idx] = 1;
      }

      /* Delete the randomly picked blocks */

      for(i=0; i<255; i++)
      {  if(block_sel[i])
	 {  unsigned char missing[2048];
	    gint64 s;
	 
	    if(i<eh->dataBytes)
	    {     s = si + i * lay->sectorsPerLayer;
	          if(s >= lay->protectedSectors)  /* exclude the padding area */ 
		    continue;                     /* respective sectors do not exist */
	    }
	    else  s = RS02EccSectorIndex(lay, i-eh->dataBytes, si);

            if(!LargeSeek(image->file, (gint64)(2048*s)))
	       Stop(_("Failed seeking to sector %lld in image: %s"), s, strerror(errno));

	    CreateMissingSector(missing, s, image->imageFP, image->fpSector, NULL); 
	    if(LargeWrite(image->file, missing, 2048) != 2048)
	       Stop(_("Failed writing to sector %lld in image: %s"), s, strerror(errno));
	  }
      }
      
      percent = (100*si)/lay->sectorsPerLayer;
      if(last_percent != percent) 
      {  PrintProgress(_("Progress: %3d%%"),percent);
	 last_percent = percent;
      }
   }

   PrintProgress(_("Progress: 100%%\n"
	"Recover the image using the --fix option before doing another --random-errors run.\n"
	"Otherwise you'll accumulate >= %d erasures/ECC block and the image will be lost.\n"), 
	n_errors);

   g_free(lay);
}

/* RS03 ecc images */

static void random_error3(Image *image, char *arg)
{  EccHeader *eh;
   RS03Layout *lay;
   gint64 si;
   int block_sel[255];
   int i,percent,last_percent = 0;
   int n_errors,erase_max = 0;
   double eras_scale, blk_scale;

   SRandom(Closure->randomSeed);

   /*** Calculate the layout */

   if(image->eccFileState == ECCFILE_PRESENT)
   {  eh = image->eccFileHeader;
      lay = CalcRS03Layout(image, ECC_FILE);
   }
   else
   {  eh = image->eccHeader;
      lay = CalcRS03Layout(image, ECC_IMAGE); 
   }
   n_errors = atoi(arg);

   if(n_errors < 0)
   {  erase_max = 1;
      n_errors = -n_errors;
   }

   if(n_errors <= 0 || n_errors > eh->eccBytes)
     Stop(_("Number of erasures must be > 0 and <= %d\n"), eh->eccBytes);

   eras_scale = (n_errors+1)/((double)MY_RAND_MAX+1.0);
   blk_scale  = (double)255.0/((double)MY_RAND_MAX+1.0);

   if(lay->target == ECC_FILE)
         PrintLog(_("\nRS03 error correction file with %d roots.\n"), eh->eccBytes);
   else  PrintLog(_("\nRS03 augmented image with %d roots.\n"), eh->eccBytes);
   PrintLog(_("Generating at most %d random correctable erasures.\n"), n_errors);

   /*** Randomly delete the blocks */

   for(si=0; si<lay->sectorsPerLayer; si++)
   {  int n_erasures = (int)(eras_scale*(double)Random());
      if(erase_max)
	n_erasures = n_errors;

      /* Reset the block selector */

      for(i=0; i<255; i++)  
	block_sel[i] = 0;

      /* Randomly pick n blocks */

      for(i=0; i<n_erasures; i++)
      {  int idx;
      
         do
	 {  idx = (int)(blk_scale*(double)Random());
	 } while(block_sel[idx]);

	 block_sel[idx] = 1;
      }

      /* Delete the randomly picked blocks */

      for(i=0; i<255; i++)
      {  if(block_sel[i])
	 {  LargeFile *file;
	    unsigned char missing[2048];
	    gint64 s,file_s;

	    s = RS03SectorIndex(lay, i, si);

	    if(s == 16)  /* FIXME: not implemented */
	       continue;

	    if(s == lay->eccHeaderPos || s == lay->eccHeaderPos+1)
	      continue; /* FIXME: not implemented */

	    /* Do not write out the virtual padding sectors
	       in ecc file case */

	    if(lay->target == ECC_FILE
	       && i<=lay->ndata-1
	       && s>=lay->dataSectors)
	      continue;
	    
	    if(lay->target == ECC_FILE && i>=lay->ndata-1)
	    {    file = image->eccFile;
	         if(i == lay->ndata-1)
		      file_s = lay->firstCrcPos + si;
		 else file_s = lay->firstEccPos + (i-lay->ndata)*lay->sectorsPerLayer + si;
	    }
	    else
	    {    file = image->file;
	         file_s = s;
	    }
	      
            if(!LargeSeek(file, (gint64)(2048*file_s)))  // FIXME: wrong for ecc files
	       Stop(_("Failed seeking to sector %lld in image: %s"), s, strerror(errno));

	    CreateMissingSector(missing, s, image->imageFP, image->fpSector, NULL); 

	    if(LargeWrite(file, missing, 2048) != 2048)
	       Stop(_("Failed writing to sector %lld in image: %s"), s, strerror(errno));
	  }
      }

      percent = (100*si)/lay->sectorsPerLayer;
      if(last_percent != percent) 
      {  PrintProgress(_("Progress: %3d%%"),percent);
	 last_percent = percent;
      }
   }

   PrintProgress(_("Progress: 100%%\n"
	"Recover the image using the --fix option before doing another --random-errors run.\n"
	"Otherwise you'll accumulate >= %d erasures/ECC block and the image will be lost.\n"), 
	n_errors);

   g_free(lay);
}

void RandomError(char *arg)
{  Image *image;
   Method *method = NULL;
   char buf[5];

   image = OpenImageFromFile(Closure->imageName, O_RDWR, IMG_PERMS);
   image = OpenEccFileForImage(image, Closure->eccName, O_RDWR, IMG_PERMS);
   ReportImageEccInconsistencies(image);

   /* Determine method. Ecc files win over augmented ecc. */

   if(image && image->eccFileMethod) method = image->eccFileMethod;
   else if(image && image->eccMethod) method = image->eccMethod;
   else Stop("Internal error: No ecc method identified.");

   if(!strncmp(method->name, "RS01", 4))
   {  random_error1(image, arg);
      CloseImage(image);
      return;
   }

   if(!strncmp(method->name, "RS02", 4))
   {  random_error2(image, arg);
      CloseImage(image);
      return;
   }

   if(!strncmp(method->name, "RS03", 4))
   {  random_error3(image, arg);
      CloseImage(image);
      return;
   }

   CloseImage(image);
   strncpy(buf, method->name, 4); buf[4] = 0;
   Stop("Don't know how to handle codec %s\n", buf);

}

/*
 * Debugging function to simulate images with single
 * byte errors (except for faulty cabling and/or controllers,
 * this should never happen)
 */

void Byteset(char *arg)
{  Image *image;
   gint64 s;
   int i,byte;
   char *cpos = NULL;
   unsigned char buf[1];

   /*** Open the image file */

   image = OpenImageFromFile(Closure->imageName, O_RDWR, IMG_PERMS);
   if(!image)
     Stop(_("Can't open %s:\n%s"), Closure->imageName, strerror(errno));

   /*** See which byte to set */

   cpos = strchr(arg,',');
   if(!cpos) Stop(_("2nd argument is missing"));
   *cpos = 0;

   s = atoi(arg);
   arg = cpos+1;

   cpos = strchr(arg,',');
   if(!cpos) Stop(_("3rd argument is missing"));
   *cpos = 0;

   i = atoi(arg);
   byte = atoi(cpos+1);

   if(s<0 || s>=image->sectorSize)
     Stop(_("Sector must be in range [0..%lld]\n"),image->sectorSize-1);

   if(i<0 || i>=2048)
     Stop(_("Byte position must be in range [0..2047]"));

   if(byte<0 || byte>=256)
     Stop(_("Byte value must be in range [0..255]"));

   PrintLog(_("Setting byte %d in sector %lld to value %d.\n"), i, s, byte); 

   /*** Set the byte */
   
   s = 2048*s + i;
   
   if(!LargeSeek(image->file, (gint64)s))
     Stop(_("Failed seeking to start of image: %s\n"),strerror(errno));

   buf[0] = byte;
   if(LargeWrite(image->file, buf, 1) != 1)
     Stop(_("Could not write the new byte value"));

   CloseImage(image);
}

/*
 * Debugging function to simulate medium with unreadable sectors
 */

void Erase(char *arg)
{  Image *image;
   gint64 start,end,s;
   char *dashpos = NULL;
   char *colonpos = NULL;
   char *simulation_hint = NULL;
   
   /*** Open the image file */

   image = OpenImageFromFile(Closure->imageName, O_RDWR, IMG_PERMS);
   if(!image)
     Stop(_("Can't open %s:\n%s"), Closure->imageName, strerror(errno));
   ExamineUDF(image);  /* get the volume label */

   /** See if there is a special debugging option following
       the sector range. This is intentionally an undocumented feature. */

   colonpos = strchr(arg,':');
   if(colonpos)
   {  *colonpos = 0;
      simulation_hint=colonpos+1;
   }
     
   /*** See which sectors to erase */

   dashpos = strchr(arg,'-');
   if(dashpos)
   {  *dashpos = 0;
      start = atoi(arg);
      end  = atoi(dashpos+1);
   }
   else start = end = atoi(arg);

   if(start>end || start < 0 || end >= image->sectorSize)
     Stop(_("Sectors must be in range [0..%lld].\n"),image->sectorSize-1);

   PrintLog(_("Erasing sectors [%lld,%lld]\n"),start,end);

   /*** Erase them. */

   if(!LargeSeek(image->file, (gint64)(2048*start)))
     Stop(_("Failed seeking to start of image: %s\n"),strerror(errno));

   for(s=start; s<=end; s++)
   {  unsigned char missing[2048];
      int m = (end == image->sectorSize-1) ? image->inLast : 2048;
      int n; 

      CreateDebuggingSector(missing, s, image->imageFP, FINGERPRINT_SECTOR, 
			    image->isoInfo ? image->isoInfo->volumeLabel : NULL,
			    simulation_hint); 

      n = LargeWrite(image->file, missing, m);

      if(n != m)
	Stop(_("Failed writing to sector %lld in image: %s"),s,strerror(errno));
   }

   /*** Clean up */

   CloseImage(image);
}

/*
 * Debugging function for truncating images
 */

void TruncateImageFile(char *arg)
{  Image *image;
   gint64 end;

   /*** Open the image file */

   image = OpenImageFromFile(Closure->imageName, O_RDWR, IMG_PERMS);
   if(!image)
     Stop(_("Can't open %s:\n%s"), Closure->imageName, strerror(errno));

   /*** Determine last sector */

   end = atoi(arg);

   if(end >= image->sectorSize)
     Stop(_("New length must be in range [0..%lld].\n"),image->sectorSize-1);

   PrintLog(_("Truncating image to %lld sectors.\n"),end);

   /*** Truncate it. */

   if(!LargeTruncate(image->file, (gint64)(2048*end)))
     Stop(_("Could not truncate %s: %s\n"),Closure->imageName,strerror(errno));

   /*** Clean up */

   CloseImage(image);
}

/*
 * Debugging function to create an ISO image filled with random numbers
 */

void RandomImage(char *image_name, char *n_sectors, int mark)
{  LargeFile *image;
   IsoHeader *ih;
   gint64 sectors;
   gint64 s = 25; /* number of ISO headers */
   int percent, last_percent = 0;
   guint32 size,invert;

   sectors = atoi(n_sectors);
   if(sectors < 64) sectors = 64;

   /*** Open the image file */

   LargeUnlink(image_name);

   if(!(image = LargeOpen(image_name, O_RDWR | O_CREAT, IMG_PERMS)))
     Stop(_("Can't open %s:\n%s"),image_name,strerror(errno));

   /*** Print banner */

   PrintLog(_("\nCreating random image with %lld sectors.\n\n"
	      "There is no need for permanently storing this image;\n" 
              "you can always reproduce it by calling\n"
	      "dvdisaster --debug %s %lld --random-seed %d\n\n"),
	      sectors, 
	      mark ? "--marked-image" : "--random-image",
	      sectors, Closure->randomSeed);

   if(Closure->randomSeed >= 0)
   {  SRandom(Closure->randomSeed);
      invert = 0;
   }
   else
   {  SRandom(-Closure->randomSeed);
      invert = 0xffffffff;
   }

   /*** Create and write the ISO file system.
	Otherwise some writing software will not recognize the image. */

   ih = InitIsoHeader();
   size = sectors-s;
   if(size>=2048*1024)
      size=2048*1024-1;
   AddFile(ih, "random.data", 2048*size);
   WriteIsoHeader(ih, image);
   FreeIsoHeader(ih);

   /*** Create it */

   while(s<sectors)
   {  guint32 buf[512];
      int i=511;
      int n;

#ifdef HAVE_LITTLE_ENDIAN
      do buf[i--] = (Random32() ^ invert); while(i>=0);
#else
      do buf[i--] = SwapBytes32(Random32() ^ invert); while(i>=0);
#endif
      
      if(mark)  /* Mark the sector with its number. */
      {  int i;

	 for(i=0; i<2048; i+=128)
	   sprintf(((char*)buf)+i, "Sector  %8lld", (long long int)s);
      }

      n = LargeWrite(image, buf, 2048);
      s++;

      if(n != 2048)
	Stop(_("Failed writing to sector %lld in image: %s"),s,strerror(errno));

      percent = (100*s)/sectors;
      if(last_percent != percent) 
      {  PrintProgress(_("Progress: %3d%%"),percent);
	 last_percent = percent;
      }
   }

   /*** Clean up */

   if(!LargeClose(image))
     Stop(_("Error closing image file:\n%s"), strerror(errno));
}

/*
 * Replaces the "unreadable sector" marker with zeros.
 */

void ZeroUnreadable(void)
{  Image *image;
   unsigned char buf[2048],zeros[2048];
   gint64 s,cnt=0;
   int percent, last_percent = 0;

   image = OpenImageFromFile(Closure->imageName, O_RDWR, IMG_PERMS);
   if(!image)
     Stop(_("Can't open %s:\n%s"), Closure->imageName, strerror(errno));
   PrintLog(_("Replacing the \"unreadable sector\" markers with zeros.\n"));
   memset(zeros, 0, 2048);   

   if(!LargeSeek(image->file, (gint64)0))
     Stop(_("Failed seeking to start of image: %s\n"),strerror(errno));

   for(s=0; s<image->sectorSize; s++)
   {  int n = LargeRead(image->file, buf, 2048);

      if(n != 2048)
	Stop(_("Could not read image sector %lld:\n%s\n"),s,strerror(errno));

      /* Replace the dead sector marker */

      if(CheckForMissingSector(buf, s, image->imageFP, FINGERPRINT_SECTOR) != SECTOR_PRESENT)
      {
	if(!LargeSeek(image->file, (gint64)(2048*s)))
	  Stop(_("Failed seeking to sector %lld in image: %s"),s,strerror(errno));

      	n = LargeWrite(image->file, zeros, 2048);
	n=2048;
	
	if(n != 2048)
	  Stop(_("Failed writing to sector %lld in image: %s"),s,strerror(errno));

	cnt++;
      }

      percent = (100*s)/image->sectorSize;
      if(last_percent != percent) 
      {  PrintProgress(_("Progress: %3d%%"),percent);
	 last_percent = percent;
      }
   }

   PrintProgress(_("%lld \"unreadable sector\" markers replaced.\n"), cnt);

   CloseImage(image);
}

/**
 ** Debugging functions to show contents of a given sector
 **/

/* 
 * produce a hex dump
 */

void HexDump(unsigned char *buf, int len, int step)
{  int i,j;

   for(i=0; i<len; i+=step)
   {  PrintLog("%04x: ",i);
      for(j=0; j<step; j++)
	if(i+j >= len) PrintLog((j&0x07) == 0x07 ? "    " : "   ");
	else           PrintLog("%02x%s", buf[i+j], (j&0x07) == 0x07 ? "  " : " ");

      for(j=0; j<step; j++)
      { if(i+j >= len) break;
	if((j&0x07) == 0x07)
	      PrintLog("%c ", isprint(buf[i+j]) ? buf[i+j] : '.');
	else  PrintLog("%c", isprint(buf[i+j]) ? buf[i+j] : '.');
      }
    
      PrintLog("\n");
   }
}

/*
 * produce a C #include file
 */

void CDump(unsigned char *buf, int lba, int len, int step)
{  int i;
   
   g_printf("#define SECTOR_LENGTH %d\n"
	    "#define SECTOR_LBA %d\n"
	    "unsigned char sector_frame[%d] = {\n",
	    len, lba, len);

   len--;
   for(i=0; i<=len; i++)
   {  g_printf("%3d%c ", *buf++, i==len ? ' ' : ','); 

      if(i%step == (step-1))
	g_printf("\n");
   }
 
   printf("};\n");
}

/*
 * Show Ecc header from image file
 */

void ShowHeader(char *arg)
{  Image *image;
   gint64 sector;
   int n;
   EccHeader *eh = alloca(4096);
   
   /*** Open the image file */

   image = OpenImageFromFile(Closure->imageName, O_RDONLY, IMG_PERMS);
   if(!image)
     Stop(_("Can't open %s:\n%s"), Closure->imageName, strerror(errno));
   
   /*** Determine sector to show */

   sector =  atoi(arg);

   if(sector < 0 || sector >= image->sectorSize)
     Stop(_("Sector must be in range [0..%lld]\n"),image->sectorSize-1);

   /*** Load it. */

   if(!LargeSeek(image->file, (gint64)(2048*sector)))
     Stop(_("Failed seeking to sector %lld in image: %s"),sector,strerror(errno));

   n = LargeRead(image->file, eh, 2048);
   if(n != 2048)
     Stop(_("Failed reading sector %lld in image: %s"),sector,strerror(errno));

   /*** Clean up */

   CloseImage(image);

   /*** Show it */

   PrintEccHeader(eh);
}

/*
 * Show sector from image file
 */

void ShowSector(char *arg)
{  Image *image;
   gint64 sector;
   int n;
   unsigned char buf[2048];

   /*** Open the image file */

   image = OpenImageFromFile(Closure->imageName, O_RDONLY, IMG_PERMS);
   if(!image)
     Stop(_("Can't open %s:\n%s"), Closure->imageName, strerror(errno));

   /*** Determine sector to show */

   sector =  atoi(arg);

   if(sector < 0 || sector >= image->sectorSize)
     Stop(_("Sector must be in range [0..%lld]\n"),image->sectorSize-1);

   PrintLog(_("Contents of sector %lld:\n\n"),sector);

   /*** Show it. */

   if(!LargeSeek(image->file, (gint64)(2048*sector)))
     Stop(_("Failed seeking to sector %lld in image: %s"),sector,strerror(errno));

   n = LargeRead(image->file, buf, 2048);
   if(n != 2048)
     Stop(_("Failed reading sector %lld in image: %s"),sector,strerror(errno));

   if(Closure->debugCDump)
        CDump(buf, sector, 2048, 16);
   else 
   {  HexDump(buf, 2048, 32);
      g_printf("CRC32 = %04x\n", Crc32(buf, 2048));
   }

   /*** Clean up */

   CloseImage(image);
}

/* 
 * Read sector from drive
 */

void ReadSector(char *arg)
{  AlignedBuffer *ab = CreateAlignedBuffer(2048);
   Image *image;
   gint64 sector;
   int status;

   /*** Open the device */

   image = OpenImageFromDevice(Closure->device);
   if(!image)
     Stop(_("Can't open %s:\n%s"), Closure->imageName, strerror(errno));

   /*** Determine sector to show */

   sector =  atoi(arg);

   if(sector < 0 || sector >= image->dh->sectors)
   {  CloseImage(image);
      FreeAlignedBuffer(ab);
      Stop(_("Sector must be in range [0..%lld]\n"),image->dh->sectors-1);
   }

   PrintLog(_("Contents of sector %lld:\n\n"),sector);

   /*** Read it. */

   status = ReadSectors(image->dh, ab->buf, sector, 1); 

   /*** Print results */
   
   if(status)
   {  CloseImage(image);
      FreeAlignedBuffer(ab);
      Stop(_("Failed reading sector %lld: %s"),sector,strerror(errno));
   }

   if(Closure->debugCDump)
        CDump(ab->buf, sector, 2048, 16);
   else 
   {  HexDump(ab->buf, 2048, 32);
      g_printf("CRC32 = %04x\n", Crc32(ab->buf, 2048));
   }

   CloseImage(image);
   FreeAlignedBuffer(ab);
}

/***
 *** Read a raw CD sector
 ***/

void RawSector(char *arg)
{  AlignedBuffer *ab = CreateAlignedBuffer(4096);
   Sense *sense;
   unsigned char cdb[MAX_CDB_SIZE];
   Image *image;
   gint64 lba;
   int length=0,status;
   int offset=16;

   /*** Open the device */

   image = OpenImageFromDevice(Closure->device);
   if(!image)
     Stop(_("Can't open %s:\n%s"), Closure->imageName, strerror(errno));
   sense = &image->dh->sense;

   /*** Only CD can be read in raw mode */

   if(image->dh->mainType != CD)
   {  CloseImage(image);
      FreeAlignedBuffer(ab);
      Stop(_("Raw reading only possible on CD media\n"));
   }

   /*** Determine sector to show */

   lba =  atoi(arg);

   if(lba < 0 || lba >= image->dh->sectors)
   {  CloseImage(image);
      FreeAlignedBuffer(ab);
      Stop(_("Sector must be in range [0..%lld]\n"),image->dh->sectors-1);
   }

   PrintLog(_("Contents of sector %lld:\n\n"),lba);

   /*** Try the raw read */

   memset(cdb, 0, MAX_CDB_SIZE);
   cdb[0]  = 0xbe;         /* READ CD */
   switch(image->dh->subType)     /* Expected sector type */
   {  case DATA1:          /* data mode 1 */ 
        cdb[1] = 2<<2; 
#if 1
	cdb[9] = 0xb8;    /* we want Sync + Header + User data + EDC/ECC */
	length=MAX_RAW_TRANSFER_SIZE; 
#else
	cdb[9] = 0xba;    /* we want Sync + Header + User data + EDC/ECC + C2 */
	length=2646; 
#endif
	offset=16;
	break;  

      case XA21:           /* xa mode 2 form 1 */
	cdb[1] = 4<<2; 
	cdb[9] = 0xf8;
	length=MAX_RAW_TRANSFER_SIZE; 
	offset=24;
	break;  
   }

   cdb[2]  = (lba >> 24) & 0xff;
   cdb[3]  = (lba >> 16) & 0xff;
   cdb[4]  = (lba >>  8) & 0xff;
   cdb[5]  = lba & 0xff;
   cdb[6]  = 0;        /* number of sectors to read (3 bytes) */
   cdb[7]  = 0;  
   cdb[8]  = 1;        /* read nsectors */

   cdb[10] = 0;        /* reserved stuff */
   cdb[11] = 0;        /* no special wishes for the control byte */

   CreateMissingSector(ab->buf, lba, NULL, 0, NULL); 
   status = SendPacket(image->dh, cdb, 12, ab->buf, length, sense, DATA_READ);

   if(status<0)  /* Read failed */
   {  RememberSense(sense->sense_key, sense->asc, sense->ascq);
      CloseImage(image);
      FreeAlignedBuffer(ab);
      Stop("Sector read failed: %s\n", GetLastSenseString(FALSE));
   }
   else  
   {  if(Closure->debugCDump)
         CDump(ab->buf, lba, length, 16);
     else 
     {   HexDump(ab->buf, length, 32);
         g_printf("CRC32 = %04x\n", Crc32(ab->buf+offset, 2048));
     }
   }

   FreeAlignedBuffer(ab);
}

/***
 *** Send a CDB to the drive and report what happens
 ***
 * Sending ill-formed cdbs may kill your system
 * and/or damage yout drive permanently.
 *
 * Example command line call for sending an inquiry:
 *
 * ./dvdisaster --debug --send-cdb 12,00,00,00,24,00:24
 *
 * The first six bytes make up the cdb; cdbs with upto 12 bytes are possible.
 * The :24 arg is the allocation length. 
 * Note that the allocation length must match those specified in the cdb;
 * differing values may crash the system.
 */

enum {  SHIFT0, SHIFT4, ALLOC };

void SendCDB(char *cdb_raw)
{  AlignedBuffer *ab = CreateAlignedBuffer(MAX_CLUSTER_SIZE);
   int cdb_len = 0;
   int alloc_len = 0;
   unsigned char cdb[16];
   int mode = SHIFT4;
   int nibble=0;
   int status;
   char *c = cdb_raw;

   while(*c && cdb_len<16)
   {  if(*c == ',' || *c== ':')
      {  if(*c == ':')
	   mode = ALLOC;
	 c++; continue; 
      }

      if(*c >= '0' && *c <= '9')
	nibble = *c - '0';
      else if(*c >= 'a' && *c <= 'f')
	nibble = *c - 'a' + 10;
      else if(*c >= 'A' && *c <= 'F')
	nibble = *c - 'A' + 10;
      else Stop("illegal char '%c' in cdb \"%s\"\n",*c,cdb_raw);

      switch(mode)
      {  case SHIFT0:
	   cdb[cdb_len] |= nibble;
	   mode = SHIFT4;
	   cdb_len++;
	   break;

         case SHIFT4:
	   cdb[cdb_len] = nibble << 4;
	   mode = SHIFT0;
	   break;

         case ALLOC:
	   alloc_len = (alloc_len << 4) | nibble;
	   break;
      }

      c++;
   }

   PrintLog("\n");
   status = SendReadCDB(Closure->device, ab->buf, cdb, cdb_len, alloc_len);

   if(!status)
   {  g_printf("\nDrive returned:\n\n");
      HexDump(ab->buf, alloc_len, 16);
   }

   FreeAlignedBuffer(ab);
}

/***
 *** Create a bitmap of simulated defects
 ***/

Bitmap* SimulateDefects(gint64 size)
{  Bitmap *bm = CreateBitmap0(size);
   gint64 defects = (size*(gint64)Closure->simulateDefects)/(gint64)100;
   
   SRandom(Closure->randomSeed);

   /* Create sequences of n sectors until the number of defects is reached. */ 

   while(defects)
   {  double scale, size_scale;
      int n, bit;

      scale = (double)defects/((double)MY_RAND_MAX+1.0);
      if(defects > 32)
	    n = (int)(scale*(double)Random());
      else  n = defects;

      size_scale = (double)(size-n)/((double)MY_RAND_MAX+1.0);
      bit = (int)(size_scale*(double)Random());

      while(n--)
      {	if(!GetBit(bm, bit))
	{  SetBit(bm, bit);
	   defects--;
	}
	bit++;
      }
   }

   return bm;
}

/***
 *** Copy a sector between two image files.
 ***/

void CopySector(char *arg)
{  LargeFile *from, *to;
   char *from_path, *to_path;
   guint64 from_sector, to_sector, sectors;
   unsigned char buf[2048];
   char *cpos = NULL;

   /*** Evaluate arguments */

   cpos = strchr(arg,',');
   if(!cpos) Stop(_("2nd argument is missing"));
   *cpos = 0;
   from_path = arg;
   arg = cpos+1;

   cpos = strchr(arg,',');
   if(!cpos) Stop(_("3rd argument is missing"));
   *cpos = 0;
   from_sector = atoll(arg);
   arg = cpos+1;

   cpos = strchr(arg,',');
   if(!cpos) Stop(_("4th argument is missing"));
   *cpos = 0;
   to_path = arg;

   to_sector = atoll(cpos+1);

   /*** Check the given files */

   if(!(from = LargeOpen(from_path, O_RDONLY, IMG_PERMS)))
     Stop(_("Can't open %s:\n%s"), from_path, strerror(errno));

   LargeStat(from_path, &sectors); sectors /= 2048;
   if(from_sector<0 || from_sector>sectors-1)
     Stop(_("Source sector must be in range [0..%lld]\n"), sectors-1);


   if(!(to = LargeOpen(to_path, O_WRONLY, IMG_PERMS)))
     Stop(_("Can't open %s:\n%s"), to_path, strerror(errno));

   LargeStat(to_path, &sectors); sectors /= 2048;
   if(to_sector<0 || to_sector>sectors-1)
     Stop(_("Destination sector must be in range [0..%lld]\n"), sectors-1);

   /*** Copy the sector */

   PrintLog(_("Copying sector %lld from %s to sector %lld in %s.\n"), 
	    from_sector, from_path, to_sector, to_path); 

   if(!LargeSeek(from, (gint64)(2048*from_sector)))
      Stop(_("Failed seeking to sector %lld in image: %s"),
	   from_sector, strerror(errno));

   if(LargeRead(from, buf, 2048) != 2048)
      Stop(_("Failed reading sector %lld in image: %s"),
	   from_sector, strerror(errno));

   if(!LargeSeek(to, (gint64)(2048*to_sector)))
      Stop(_("Failed seeking to sector %lld in image: %s"),
	   to_sector, strerror(errno));

   if(LargeWrite(to, buf, 2048) != 2048)
      Stop(_("Failed writing to sector %lld in image: %s"),
	   to_sector, strerror(errno));

   /*** Clean up */

   LargeClose(from);
   LargeClose(to);
}

/***
 *** Compare or merge images
 ***/

void MergeImages(char *arg, int mode)
{  LargeFile *left, *right;
   char *left_path, *right_path;
   guint64 left_sectors, right_sectors,min_sectors,s;
   int percent,last_percent = 0;
   char *cpos = NULL;

   /*** Evaluate arguments */

   cpos = strchr(arg,',');
   if(!cpos) Stop(_("2nd argument is missing"));
   *cpos = 0;

   left_path = arg;
   right_path = cpos+1;

   /*** Check the given files */

   if(!(left = LargeOpen(left_path, mode ? O_RDWR : O_RDONLY, IMG_PERMS)))
     Stop(_("Can't open %s:\n%s"), left_path, strerror(errno));

   LargeStat(left_path, &left_sectors); left_sectors /= 2048;

   if(!(right = LargeOpen(right_path, O_RDONLY, IMG_PERMS)))
     Stop(_("Can't open %s:\n%s"), right_path, strerror(errno));

   LargeStat(right_path, &right_sectors); right_sectors /= 2048;

   /*** Compare/merge the images */

   if(!mode) PrintLog("Comparing %s (%lld sectors) with %s (%lld sectors).\n", 
		      left_path, left_sectors, right_path, right_sectors);
   else      PrintLog("Merging %s (%lld sectors) with %s (%lld sectors).\n",
		      left_path, left_sectors, right_path, right_sectors);

   /*** Compare them */

   if(left_sectors < right_sectors) 
        min_sectors = left_sectors;
   else min_sectors = right_sectors;


   for(s=0; s<min_sectors; s++)
   {  unsigned char left_buf[2048], right_buf[2048];

      if(LargeRead(left, left_buf, 2048) != 2048)
	 Stop(_("Failed reading sector %lld in image: %s"),
	      s, strerror(errno));

      if(LargeRead(right, right_buf, 2048) != 2048)
	 Stop(_("Failed reading sector %lld in image: %s"),
	      s, strerror(errno));

      if(memcmp(left_buf, right_buf, 2048))
      {
	 if(CheckForMissingSector(left_buf, s, NULL, 0) != SECTOR_PRESENT)
	 {  if(!mode) PrintLog("< Sector %lld missing\n", s);
	    else
	    {  PrintLog("< Sector %lld missing; copied from %s.\n", s, right_path);
	       if(!LargeSeek(left, (2048*s)))
		  Stop(_("Failed seeking to sector %lld in image: %s"),
		       s, strerror(errno));

	       if(LargeWrite(left, right_buf, 2048) != 2048)
		  Stop(_("Failed writing to sector %lld in image: %s"),
		       s, strerror(errno));
	    }
	 }
	 else if(CheckForMissingSector(right_buf, s, NULL, 0) != SECTOR_PRESENT)
	 {  PrintLog("> Sector %lld missing\n", s);
	 }
	 else
	 {  PrintLog("! Sector %lld differs in images\n", s);
	 }
      }

      percent = (100*s)/left_sectors;
      if(last_percent != percent) 
      {  PrintProgress(_("Progress: %3d%%"),percent);
	 last_percent = percent;
      }
   }

   if(left_sectors > right_sectors)
   {  PrintLog("%lld sectors missing at the end of %s\n", 
		  left_sectors-right_sectors, right_path);
   }

   if(left_sectors < right_sectors)
   {  if(!mode)
	 PrintLog("%lld sectors missing at the end of %s\n", 
		  right_sectors-left_sectors, left_path);
      else
      {  unsigned char buf[2048];

	 PrintLog("Transferring %lld sectors from the end of %s to %s.\n", 
		  right_sectors-left_sectors, right_path, left_path);
	 
	 for(s=left_sectors; s<right_sectors; s++)
	 {  if(LargeRead(right, buf, 2048) != 2048)
	       Stop(_("Failed reading sector %lld in image: %s"),
		    s, strerror(errno));

	    if(LargeWrite(left, buf, 2048) != 2048)
	       Stop(_("Failed writing to sector %lld in image: %s"),
		    s, strerror(errno));
	 }
      }
   }

   /*** Clean up */

   LargeClose(left);
   LargeClose(right);
}

/*
 * Print LaTeX'ed table of Galois fields and other matrices
 */

void LaTeXify(gint32 *table , int rows, int columns)
{  int x,y;

   printf("\\begin{tabular}{|l||");
   for(x=0; x<columns; x++)
      printf("c|");
   printf("}\n\\hline\n");

   printf("&");
   for(x=0; x<columns; x++)
      printf("%c %02x ", x==0?' ':'&', x);
   printf("\\\\\n\\hline\n\\hline\n");

   for(y=0; y<rows; y++)
   {  printf("%02x &",16*y);
      for(x=0; x<columns; x++)
	 printf("%c %02x ", x==0?' ':'&', *table++);
      printf("\\\\\n\\hline\n");
   }
   
   printf("\\end{tabular}\n");
}

/*
 * Append a text to a file in printf() manner,
 * not keeping it open.
 */

void AppendToTextFile(char* filename, char *format, ...)
{  va_list argp;
   FILE *file;

   file = fopen(filename, "a");
   if(!file)
     Stop("Could not open %s: %s\n", filename, strerror(errno));

   va_start(argp, format);
   g_vfprintf(file, format, argp);
   va_end(argp);

   if(fclose(file))
     Stop("Could not close %s: %s\n", filename, strerror(errno));
}
