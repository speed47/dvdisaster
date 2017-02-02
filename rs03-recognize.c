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
#include "udf.h"

#include "rs03-includes.h"

/*
 * Aux. functions
 */

static int valid_crc_block(unsigned char *buf, guint64 sector, int image_expected)
{  CrcBlock *cb = alloca(2048);
   guint32 recorded_crc, real_crc;

   memcpy(cb, buf, 2048);

   /* See if the magic cookie is there */

   if(   strncmp((char*)cb->cookie, "*dvdisaster*", 12)
      || strncmp((char*)cb->method, "RS03", 4))
     return 0;

   /* Examine the checksum */
   
   recorded_crc = cb->selfCRC;

#ifdef HAVE_BIG_ENDIAN
   cb->selfCRC = 0x47504c00;
#else
   cb->selfCRC = 0x4c5047;
#endif
   real_crc = Crc32((unsigned char*)cb, 2048);

   if(real_crc != recorded_crc)
   {  Verbose(".. invalid CRC block %lld\n", (unsigned long long)sector);
      return 1;
   }

   /* If an ecc file header is found in the image (which might
      rightfully contain ecc files), ignore it */

   if(image_expected && (cb->methodFlags[0] & MFLAG_ECC_FILE))
   {  Verbose(".. Crc block from ecc file in image - IGNORED\n");
      return 1;
   }
    
   return 2;
}

/***
 *** Recognize a RS03 error correction file
 ***/

int RS03RecognizeFile(LargeFile *ecc_file, EccHeader **eh)
{  int crc_block = 0;
   int n;

   Verbose("RS03RecognizeFile(): examining %s\n", ecc_file->path);
   *eh = g_malloc(sizeof(EccHeader));

   /*** First see whether we have a valid ecc header. */
   
   LargeSeek(ecc_file, 0);
   n = LargeRead(ecc_file, *eh, sizeof(EccHeader));

   /* short read -> definitely not an ecc file */
   
   if(n != sizeof(EccHeader))
   {  g_free(*eh);
      *eh=NULL;
      Verbose("RS03RecognizeFile(): short read for ecc header\n");
      return ECCFILE_INVALID;
   }

   /* Validate the header */
   
   if(!strncmp((char*)(*eh)->cookie, "*dvdisaster*", 12))
   {  guint32 recorded_crc,real_crc;
   
      /* Examine the checksum */

     recorded_crc = (*eh)->selfCRC;

#ifdef HAVE_BIG_ENDIAN
      (*eh)->selfCRC = 0x47504c00;
#else
      (*eh)->selfCRC = 0x4c5047;
#endif
      real_crc = Crc32((unsigned char*)(*eh), 4096);

#ifdef HAVE_BIG_ENDIAN
      SwapEccHeaderBytes(*eh);
#endif

      if(real_crc != recorded_crc)
      {  Verbose("RS03RecognizeFile(): checksum error in ecc header\n");
      }
      else
      {  if(!strncmp((char*)(*eh)->method, "RS03", 4))
	 {  Verbose("RS03RecognizeFile(): ecc header found\n");
	    return ECCFILE_PRESENT;
	 }
	 else
	 {  Verbose("RS03RecognizeFile(): wrong codec\n");
	    g_free(*eh);
	    *eh=NULL;
	    return ECCFILE_WRONG_CODEC;
	 }
      }
   }
   else Verbose("RS03RecognizeFile(): no magic cookie in header\n");

   /* No ecc header found; search for CRC blocks.
      The CRC block follow directly after the ecc header,
      so simply continue reading in 2048 chunks until
      the file ends. There is no good criterion for stopping
      the read earlier since the file may be truncated and/or
      contain mangled contents in many unpredictable ways. */

   Verbose("RS03RecognizeFile(): exhaustive search for CRC blocks started\n");
   
   for(;;)
   {  unsigned char buf[2048];
      n = LargeRead(ecc_file, buf, 2048);

      if(n != 2048)
      {  if(n== 0 && LargeEOF(ecc_file))
	      Verbose("RS03RecognizeFile(): end of file reached\n");
	else Verbose("RS03RecognizeFile(): short read for CRC sector %d\n", crc_block);
	 g_free(*eh);
	 *eh=NULL;
	 return ECCFILE_INVALID;
      }

      if(valid_crc_block(buf, crc_block++, FALSE) == 2)
      {  ReconstructRS03Header(*eh, (CrcBlock*)buf);
	 Verbose("** Success: sector %d, rediscovered format with %d roots\n",
		 crc_block+1, (*eh)->eccBytes);

	 /* Rewrite the missing ecc header if possible */
	 
	 if(ecc_file->flags & O_RDWR || ecc_file->flags & O_WRONLY)
	 { int success=0; 
	   EccHeader *le_eh = *eh;
	   char buf[4096];

	   memset(buf, 0, 4096);
#ifdef HAVE_BIG_ENDIAN	   
	   /* eh contains the recovered ecc header in native endian format,
	      which is what we need it in, but it must be written out in
	      little endian. So we have to create an extra copy for writing
	      out when on a big endian machine. */

	   memcpy(buf, *eh, 4096);
	   le_eh = (EccHeader*)buf;
           SwapEccHeaderBytes(le_eh);
   	   le_eh->selfCRC = 0x47504c00;
	   le_eh->selfCRC = Crc32((unsigned char*)buf, 4096);
#endif
	   if(LargeSeek(ecc_file, 0))
	   {  if(LargeWrite(ecc_file, le_eh, 4096))
	        success=1;
	   }

	   if(success) Verbose("** Missing ecc header rewritten\n");
	   else        Verbose("** Note: Could not rewrite ecc header!\n");
	 }
 	 return ECCFILE_PRESENT;
      }
   }	
	
   /* Still nothing found. */

   g_free(*eh);
   *eh=NULL;
   Verbose("RS03RecognizeFile(): no ecc found\n");
   return ECCFILE_INVALID;
}

/***
 *** Recognize RS03 error correction data in the image
 ***/

#if 0
static int read_fingerprint(LargeFile *file, unsigned char *fingerprint, gint64 sector)
{  struct MD5Context md5ctxt;
   unsigned char buf[2048];
   int n;

   if(!LargeSeek(file, 2048LL*sector))
     return FALSE;

   n = LargeRead(file, buf, 2048);

   if(n != 2048) return FALSE;

   if(CheckForMissingSector(buf, sector, NULL, 0) != SECTOR_PRESENT)
     return FALSE;

   MD5Init(&md5ctxt);
   MD5Update(&md5ctxt, buf, 2048);
   MD5Final(fingerprint, &md5ctxt);

   return TRUE;
}
#endif

static EccHeader* valid_header(unsigned char *buf, gint64 hdr_pos, int image_expected)
{  EccHeader *eh = (EccHeader*)buf;
   guint32 recorded_crc, real_crc;
   //   unsigned char fingerprint[16];

   /* Medium read error in ecc header? */

   if(   (CheckForMissingSector(buf, hdr_pos, NULL, 0) != SECTOR_PRESENT)
      || (CheckForMissingSector(buf+2048, hdr_pos+1, NULL, 0) != SECTOR_PRESENT))
     return NULL;

   /* See if the magic cookie is there */

   if(   strncmp((char*)eh->cookie, "*dvdisaster*", 12)
	 || strncmp((char*)eh->method, "RS03", 4))
     return NULL;

   /* Examine the checksum */

   recorded_crc = eh->selfCRC;

#ifdef HAVE_BIG_ENDIAN
   eh->selfCRC = 0x47504c00;
#else
   eh->selfCRC = 0x4c5047;
#endif
   real_crc = Crc32((unsigned char*)eh, 4096);

   if(real_crc != recorded_crc)
     return NULL;

   /* If an ecc file header is found in the image (which might
      rightfully contain ecc files), ignore it */

   if(image_expected && (eh->methodFlags[0] & MFLAG_ECC_FILE))
   {  Verbose(".. Ecc file header in image - IGNORED\n");
      return NULL;
   }
   
   /* Check the fingerprint */

   eh = g_malloc(sizeof(EccHeader));
   memcpy(eh, buf, sizeof(EccHeader));
#ifdef HAVE_BIG_ENDIAN
   SwapEccHeaderBytes(eh);
#endif
   eh->selfCRC = recorded_crc;

#if 0  //FIXME
   status = read_fingerprint(file, fingerprint, eh->fpSector);

   if(!status)  /* be optimistic if fingerprint sector is unreadable */
     return eh;

   if(!memcmp(fingerprint, eh->mediumFP, 16))  /* good fingerprint */
     {  printf("RS03 header found\n");
      return eh;
     }
   g_free(eh);
#endif

   return eh;
}

EccHeader* FindRS03HeaderInImage(Image *image)
{  EccHeader *eh = NULL;
   gint64 hdr_pos;
   IsoInfo *ii; 
   unsigned char buf[4096];

   switch(image->type)
   { case IMAGE_FILE:
       Verbose("FindRS03HeaderInImage: file %s\n", image->file->path);
       break;

     case IMAGE_MEDIUM:
       Verbose("FindRS03HeaderInImage: medium %s\n", image->dh->device);
       break;

     default:
       Verbose("FindRS03HeaderInImage: unknown type %d\n", image->type);
       break;
   }

   /*** Try to find the header behind the ISO image */

   ii = image->isoInfo;
   if(!ii) Verbose(" . NO ISO structures found!\n");

   if(ii)
   {  hdr_pos = ii->volumeSize;

      if(ImageReadSectors(image, buf, hdr_pos, 2) == 2)
	{  eh = valid_header(buf, hdr_pos, TRUE);
	 if(eh) 
	 { Verbose("FindRS03HeaderInImage(): Header found at pos +0\n"); 
	   return eh;
	 }
      }

      hdr_pos = ii->volumeSize - 150;
      if(ImageReadSectors(image, buf, hdr_pos, 2) == 2)
	{  eh = valid_header(buf, hdr_pos, TRUE);
	 if(eh) 
	 { Verbose("FindRS03HeaderInImage(): Header found at pos -150\n"); 
	   return eh;
	 }
      }
   }
      
   return NULL;
}

typedef struct 
{  AlignedBuffer *layer[256];
   AlignedBuffer *ab;
   RS03Layout *layout[256];
   int layer_checked[256];
} recognize_context;

static void free_recognize_context(recognize_context *rc)
{  int i;

   if(rc->ab) FreeAlignedBuffer(rc->ab);

   for(i=0; i<255; i++)
   {  if(rc->layer[i])
         FreeAlignedBuffer(rc->layer[i]);
      if(rc->layout[i])
	 g_free(rc->layout[i]); 
   }
   g_free(rc);
}
   
int RS03RecognizeImage(Image *image)
{  recognize_context *rc = g_malloc0(sizeof(recognize_context));
   guint64 image_sectors;
   guint64 layer_size;
   int untested_layers;
   int layer, layer_sector;
   int i;

   switch(image->type)
   { case IMAGE_FILE:
       Verbose("RS03RecognizeImage: file %s\n", image->file->path);
       if(image->eccFile)
	 Stop("Internal error: RS03RecognizeImage() called with ecc file\n");
       image_sectors = image->sectorSize;
       break;

     case IMAGE_MEDIUM:
       Verbose("RS03RecognizeImage: medium %s\n", image->dh->device);
       image_sectors = MAX(image->dh->readCapacity, image->dh->userAreaSize);
       break;

     default:
       Verbose("RS03RecognizeImage: unknown type %d\n", image->type);
       free_recognize_context(rc);
       return FALSE;
       break;
   }

   /* Easy shot: Locate the ecc header in the image */

   image->eccHeader = FindRS03HeaderInImage(image); 
  
   if(image->eccHeader) 
   {  free_recognize_context(rc);
      return TRUE;
   }

   /* No exhaustive search on optical media unless explicitly okayed by user */

   if(!Closure->examineRS03 && image->type == IMAGE_MEDIUM)
   {  free_recognize_context(rc);
      Verbose("RS03RecognizeImage: skipping exhaustive RS03 search\n");
      return FALSE;
   }

   /* Determine image size in augmented case. */

   Verbose("RS03RecognizeImage: No EH, entering exhaustive search\n");

   if(Closure->debugMode && Closure->mediumSize)
   {  layer_size = Closure->mediumSize/GF_FIELDMAX;
      Verbose("Warning: image size set to %lld for debugging!\n", Closure->mediumSize);
   }
   else
   {  if(image_sectors < CDR_SIZE)         layer_size = CDR_SIZE/GF_FIELDMAX;
      else if(image_sectors < DVD_SL_SIZE) layer_size = DVD_SL_SIZE/GF_FIELDMAX; 
      else if(image_sectors < DVD_DL_SIZE) layer_size = DVD_DL_SIZE/GF_FIELDMAX; 
      else if(image_sectors < BD_SL_SIZE)  layer_size = BD_SL_SIZE/GF_FIELDMAX; 
      else                                 layer_size = BD_DL_SIZE/GF_FIELDMAX; 
   }

   Verbose(".. trying layer size %lld\n", layer_size);

   /*
    * Try a quick scan for the CRC sectors in order
    * to re-discover the layout.
    */

   Verbose("Scanning layers for signatures.\n");

   /* Prepare layout for all possible cases (8..170 roots) */

   for(i=84; i<=247; i++)  /* allowed range of ndata */
   {  RS03Layout *lay;
      rc->layout[i] = lay = g_malloc0(sizeof(RS03Layout));
      lay->eh = NULL;
      lay->dataSectors = (i-1)*layer_size-2;
      lay->dataPadding = 0;
      lay->totalSectors = GF_FIELDMAX*layer_size;
      lay->sectorsPerLayer = layer_size;
      lay->mediumCapacity = 0;
      lay->eccHeaderPos = lay->dataSectors;
      lay->firstCrcPos = (i-1)*layer_size;
      lay->firstEccPos = i*layer_size;
      lay->nroots = GF_FIELDMAX-i;
      lay->ndata = i;
      lay->inLast = 2048;
      lay->target = ECC_IMAGE;
   }
   untested_layers = 247-84+1;

   rc->ab = CreateAlignedBuffer(2048);

   for(layer_sector = 0; layer_sector < layer_size; layer_sector++)
   {  CrcBlock *cb = (CrcBlock*)rc->ab->buf;

      Verbose("- layer slice %d\n", layer_sector);
      for(layer = 84; layer <= 247; layer++) 
      {  if(!rc->layer_checked[layer])
	 {  gint64 sector;
	    int crc_state;

	    sector = RS03SectorIndex(rc->layout[layer], layer, layer_sector);

	    /* reading beyond the image won't yield anything */
	    if(sector >= image_sectors)
	      goto mark_invalid_layer;

	    switch(image->type)
	    {  case IMAGE_FILE:
		 RS03ReadSectors(image, rc->layout[layer], rc->ab->buf,
				 layer, layer_sector, 1, RS03_READ_ALL);
		 if(CheckForMissingSector(rc->ab->buf, sector, NULL, 0) != SECTOR_PRESENT)
		    continue;  /* unreadble -> can't decide */
		 break;

	       case IMAGE_MEDIUM:
	       {  int n;	  
		  n = ImageReadSectors(image, rc->ab->buf, sector, 1);
		  if(!n)
		    continue; /* unreadble -> can't decide */
	       }
	    }
		  
	    /* CRC header found? */

	    crc_state = valid_crc_block(rc->ab->buf, sector, TRUE);
	    if(crc_state)
	    {  int nroots=255-layer-1;  

	       if(crc_state == 1) /* corrupted crc header, try this layer again later */
		 continue;
	       Verbose("** Success: sector %lld, rediscovered format with %d roots\n",
		       sector, nroots); 
	       image->eccHeader = g_malloc(sizeof(EccHeader));
	       ReconstructRS03Header(image->eccHeader, cb);
	       /* Note: Rewriting the missing ecc header makes no sense here
		  as we do not have access to the image written in the reading
	          functions, and the error correction will restore it anyways.
	          (contrary to the situation with ecc files) */
	       free_recognize_context(rc);
	       return TRUE;
	    }

	    /* Sector readable but not a CRC header -> skip this layer */

mark_invalid_layer:
	    if(!rc->layer_checked[layer])
	    {  rc->layer_checked[layer] = 1;
	       untested_layers--;
	    }
	    if(untested_layers <= 0)
	    {  Verbose("** All layers tested -> no RS03 data found\n");
	       free_recognize_context(rc);
	       return FALSE;
	    }	       
	 }
      }
      Verbose("-> %d untested layers remaining\n", untested_layers);
   }

   Verbose("-- whole medium/image scanned; %d layers remain untested\n", untested_layers);
   Verbose("-- giving now up as ecc-based search is not yet implemented\n");
   free_recognize_context(rc);
   return FALSE;
	    
   /* 
    * TODO: Assemble all ecc blocks and see whether the error corrction
    * succeeds for a certain number of roots
    */

#if 0
   for(i=0; i<255; i++)
      rc->layer[i] = CreateAlignedBuffer(2048);

   for(ecc_block=0; ecc_block<layer_size; ecc_block++)
   {  Verbose("Assembling ecc block %d\n", ecc_block); 

      /* Assemble the ecc block */

      for(i=0; i<255; i++)  
      {  gint64 sector = rc->bidx[i]++;
	 int n;

	 if(!LargeSeek(ecc_file, (gint64)(2048*sector)))
	    Stop(_("Failed seeking to sector %lld in image: %s"),
		 sector, strerror(errno));

	 n = LargeRead(ecc_file, rc->layer[i], 2048);
	 if(n != 2048)
	    Stop(_("Failed reading sector %lld in image: %s"),sector,strerror(errno));
      }	 

      /* Experimentally apply the RS code */

      for(ndata=255-8; ndata >=85; ndata--)
      {  CrcBlock *cb = (CrcBlock*)rc->layer[ndata];

	 /* Do the real decode here */


	 /* See if we have decoded a CRC block */

	 if(  !memcmp(cb->cookie, "*dvdisaster*", 12)
	    ||!memcmp(cb->method, "RS03", 4))
	 {  
	    nroots = 255-ndata-1;
	    Verbose(".. Success: rediscovered format with %d roots\n", nroots); 

	    image->eccHeader = g_malloc(sizeof(EccHeader));
	    ReconstructRS03Header(image->eccHeader, cb);
	    //FIXME: endianess okay?
	    free_recognize_context(rc);
	    return TRUE;
	 }
      }
   }
#endif

   free_recognize_context(rc);
   return FALSE;
}

