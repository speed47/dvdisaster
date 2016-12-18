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

#include "rs03-includes.h"


/***
 *** Read and buffer CRC information from RS03 file 
 ***/

CrcBuf *RS03GetCrcBuf(Image *image)
{  RS03CksumClosure *csc;
   CrcBuf *cbuf;
   RS03Layout *lay;
   EccHeader *eh;
   AlignedBuffer *ab = CreateAlignedBuffer(2048);
   guint32 *crc_buf = (guint32*)ab->buf;
   gint64 block_idx[256];
   gint64 crc_sector,s;
   int i;
   int crc_valid = 1;

   /* Allocate buffer for ascending sector order CRCs */

   if(image->eccFileHeader)
   {  eh = image->eccFileHeader;
      csc = (RS03CksumClosure*)image->eccFileMethod->ckSumClosure;

      lay = CalcRS03Layout(image, ECC_FILE); 
      cbuf = CreateCrcBuf((lay->ndata-1)*lay->sectorsPerLayer);
   }
   else 
   {  eh = image->eccHeader;
      csc = (RS03CksumClosure*)image->eccMethod->ckSumClosure;
      lay = CalcRS03Layout(image, ECC_IMAGE); 
      cbuf = CreateCrcBuf((lay->ndata-1)*lay->sectorsPerLayer);
   }

   csc->signatureErrors=0;
   if(csc->lay) g_free(csc->lay);
   csc->lay = lay;

   /* First sector containing crc data */

   crc_sector = lay->firstCrcPos;

   /* Initialize ecc block index pointers.
      Note that CRC blocks are shifted by one 
      (each ECC block contains the CRC for the next ECC block) */

   for(s=0, i=0; i<lay->ndata; s+=lay->sectorsPerLayer, i++)
     block_idx[i] = s+1;

   /* Cycle through the ecc blocks.
      Each ecc block contains the CRCs for the following ecc block;
      these are rearranged in ascending sector order. */

   for(s=0; s<lay->sectorsPerLayer; s++)
   {  int err;

      /* Get CRC sector for current ecc block */

      if(image->eccFile && image->eccFileState == ECCFILE_PRESENT)  /* read from separate ecc file */
      {  if(!LargeSeek(image->eccFile, (gint64)(2048*(lay->firstCrcPos+s))))
	    CreateMissingSector(ab->buf, crc_sector, image->imageFP, FINGERPRINT_SECTOR, NULL);
	 else
	   if(LargeRead(image->eccFile, ab->buf, 2048) != 2048)
	     CreateMissingSector(ab->buf, crc_sector, image->imageFP, FINGERPRINT_SECTOR, NULL);
      }
      else /* read from augmented image */
      {  int n = ImageReadSectors(image, ab->buf, crc_sector, 1);
	 if(n!=1)
	    CreateMissingSector(ab->buf, crc_sector, image->imageFP, FINGERPRINT_SECTOR, NULL);
      }

      err = CheckForMissingSector(ab->buf, crc_sector, eh->mediumFP, eh->fpSector);
#if 0
      if(err != SECTOR_PRESENT)
      {  int source_type;

	 if(image->eccFile && image->eccFileState == ECCFILE_PRESENT)
	      source_type = SOURCE_ECCFILE;
	 else source_type = image->type == IMAGE_FILE ? SOURCE_IMAGE : SOURCE_MEDIUM;

	 if(!unrecoverable_sectors && err != SECTOR_MISSING)
	    PrintLog("\n");
         ExplainMissingSector(ab->buf, crc_sector, err, source_type, &unrecoverable_sectors);
      }
#endif
      crc_sector++;
      crc_valid = (err == SECTOR_PRESENT);

      /* Check the CrcBlock data structure */

      if(crc_valid)
      {  CrcBlock *cb = (CrcBlock*)ab->buf;

	 if(  memcmp(cb->cookie, "*dvdisaster*", 12)
	    ||memcmp(cb->method, "RS03", 4))
	 {  crc_valid = FALSE;
	    csc->signatureErrors++;
         }
	 else
         {  guint32 recorded_crc = cb->selfCRC;
            guint32 real_crc;

#ifdef HAVE_BIG_ENDIAN
            cb->selfCRC = 0x47504c00;
#else
            cb->selfCRC = 0x4c5047;
#endif

            real_crc = Crc32((unsigned char*)cb, 2048);

            if(real_crc != recorded_crc)
            {  crc_valid = FALSE;
	       csc->signatureErrors++;
            }
         }
      }

      /* Go through all data sectors of current ecc block;
	 distribute the CRC values */

      for(i=0; i<lay->ndata-1; i++)
      {
	 /* CRC sums for the first ecc block are contained in the last
	    CRC sector. Wrap the block_idx accordingly. */
      
	 if(s == lay->sectorsPerLayer-1)
	    block_idx[i] = i*lay->sectorsPerLayer;

	 /* Sort crc into appropriate place if CRC block is valid */

	 if(crc_valid)
	 {  cbuf->crcbuf[block_idx[i]] = crc_buf[i];
	    SetBit(cbuf->valid,block_idx[i]);
	 }

	 block_idx[i]++;
      }
   }

   FreeAlignedBuffer(ab);
   return cbuf;
}

/***
 *** Read one or more image sectors from the .iso file.
 ***/

void RS03ReadSectors(Image *image, RS03Layout *lay, unsigned char *buf, 
		     gint64 layer, gint64 layer_sector, gint64 how_many, int flags)
{  LargeFile *target_file = NULL;
   gint64 start_sector=0;
   gint64 stop_sector=0;
   gint64 byte_size = how_many * 2048;
   gint64 img_file_sector_size;
   gint64 ecc_file_sector_size=0;
   gint64 target_file_sector_size;
   int in_last;
   gint64 n;

   if(layer < 0 || layer > 255) 
      Stop("RS03ReadSectors: layer %lld out of range 0 .. 255\n", layer);
   if(layer_sector < 0 || layer_sector >= lay->sectorsPerLayer) 
      Stop("RS03ReadSectors: offset %lld out of range 0 .. %lld)\n",
	   layer_sector, lay->sectorsPerLayer-1);

   /* "Image" file size may not be a multiple of 2048 */

   in_last = image->file->size % 2048;
   img_file_sector_size = image->file->size/2048;
   if(in_last) img_file_sector_size++;
   
   /* Ignore trailing garbage in the image file */

   if(lay->target == ECC_FILE)
   {  /* If the image is longer as expected in the ecc file,
	 truncate sizes to the values recorded in the ecc file. */
      if(img_file_sector_size > lay->dataSectors)
      {  img_file_sector_size = lay->dataSectors;
	 in_last = lay->eh->inLast;
      }

      /* If the image has the right sector size, but contains
	 a few bytes more in this sector as expected, truncate
	 the value. However if the last sector contains some
	 bytes less than expected, keep the smaller value
	 to prevent reading past the image later on. */
      if(   img_file_sector_size == lay->dataSectors
	    && in_last > lay->eh->inLast)
      {   in_last = lay->eh->inLast;
      }

      /* Ecc file size is currently considered to be a multiple
	 of the sector size (which is normally the case).
	 If the ecc file is tuncated by a few bytes,
	 the last incomplete sector is ignored. */

      ecc_file_sector_size = image->eccFile->size/2048;
   }

   /* Read out of the data layer */

   if(layer < lay->ndata-1)
   {  if(!(flags & RS03_READ_DATA))
	 Stop("RS03ReadSectors: trying to read data layer, but flag not set\n");
      
      start_sector = layer*lay->sectorsPerLayer + layer_sector;
      stop_sector  = start_sector + how_many - 1;

      if(stop_sector >= (layer+1)*lay->sectorsPerLayer)
	Stop("RS03ReadSectors: range %lld..%lld crosses layer boundary\n",
	     start_sector, stop_sector);
      target_file = image->file;
   }

   /* Read out of the crc layer */

   if(layer == lay->ndata-1)
   {  if(!(flags & RS03_READ_CRC))
	 Stop("RS03ReadSectors: trying to read crc layer, but flag not set\n");

      start_sector = lay->firstCrcPos + layer_sector;
      stop_sector  = start_sector + how_many - 1;

      if(lay->target == ECC_IMAGE)
	   target_file = image->file;
      else target_file = image->eccFile;
   }

   /*** Read out of the ecc layers */

   target_file_sector_size = img_file_sector_size;

   if(layer >= lay->ndata)
   {  if(!(flags & RS03_READ_ECC))
	 Stop("RS03ReadSectors: trying to read ecc layer, but flag not set\n");

      start_sector = lay->firstEccPos + (layer-lay->ndata)*lay->sectorsPerLayer + layer_sector;
      stop_sector  = start_sector + how_many - 1;

      if(lay->target == ECC_IMAGE)
	   target_file = image->file;
      else 
      {    target_file = image->eccFile;
	   target_file_sector_size = ecc_file_sector_size;
	   in_last = 0; /* Ecc file size if always a multiple of 2048 */
      }
   }

   /* Reading beyond the image returns 
      - dead sectors if the image was truncated
      - padding sectors if the real end of the image is exceeded.
      Create them in memory; shorten read range accordingly */

   if(stop_sector >= target_file_sector_size)
   {  unsigned char *bufptr = buf;
      char *volume_label = NULL;
      guint64 expected_sectors;

#if 0  //FIXME
      if(rc->image->isoInfo && rc->image->isoInfo->volumeLabel[0])
      rc->volumeLabel = g_strdup(rc->image->isoInfo->volumeLabel);
#endif

      if(lay->target == ECC_FILE)
	 expected_sectors = lay->dataSectors;
      else 
	 expected_sectors = lay->totalSectors;

      for(n=start_sector; n<=stop_sector; n++)
      {  
	 if(n>=target_file_sector_size)
	 {  guint8 *fp = lay->eh ? lay->eh->mediumFP : NULL;
	    if(n>=expected_sectors)
	    {  CreatePaddingSector(bufptr, n, fp, FINGERPRINT_SECTOR);
	    }
	    else
	    {  CreateMissingSector(bufptr, n, fp, FINGERPRINT_SECTOR, volume_label);
	    }
	    byte_size -= 2048;
	 }
	 bufptr += 2048;
      }
   }

   if(byte_size<=0)
      return;

   /* Image with ecc files may have an incomplete last sector.
      Deal with it appropriately. */

   if(lay->target == ECC_FILE && in_last)
   {  if(start_sector <= target_file_sector_size-1
	 && target_file_sector_size-1 <= stop_sector)
      {  
	 memset(buf, 0, byte_size);
	 byte_size = byte_size - 2048 + in_last;
      }
   }

   /* All sectors are consecutively readable in image case */
   
   if(!LargeSeek(target_file, (gint64)(2048*start_sector)))
      Stop(_("Failed seeking to sector %lld in image: %s"),
	   start_sector, strerror(errno));

   n = LargeRead(target_file, buf, byte_size);
   if(n != byte_size)
      Stop(_("Failed reading sector %lld in image: %s"),
	   start_sector, strerror(errno));
}

/***
 *** Calculate position of n-th sector of the given layer in the image.
 ***/

gint64 RS03SectorIndex(RS03Layout *lay, gint64 layer, gint64 n)
{  
   if(lay->target == ECC_IMAGE)
      return layer*lay->sectorsPerLayer+n;

   /* Image portion in ecc file case */

   if(layer < lay->ndata-1)
      return layer*lay->sectorsPerLayer+n;

   /* Layers located in the ecc file */

   if(layer == lay->ndata-1) /* CRC layer */
      return lay->firstCrcPos + n;

   /* Ecc layers */

   return lay->firstEccPos + (layer-lay->ndata)*lay->sectorsPerLayer + n;
}


/***
 *** Calculation of the image layout
 ***/

static int get_roots(gint64 data_sectors, gint64 medium_capacity)
{  gint64 sectors_per_layer = medium_capacity/GF_FIELDMAX;
   int ndata = (data_sectors + 2 +sectors_per_layer - 1) / sectors_per_layer;

   return GF_FIELDMAX - ndata - 1;
}

static gint64 ecc_file_size(gint64 sectors, int nr)
{  int nd = GF_FIELDMAX - nr;
   gint64 bytesize; 

   bytesize = 4096 + 2048*(nr+1)*((sectors+nd-1)/nd);

   return (bytesize+0xfffff)/0x100000;   /* size in MiB */
}		 


RS03Layout *CalcRS03Layout(Image *image, int target)
{  RS03Layout *lay = g_malloc0(sizeof(RS03Layout));

   /* See if layout has already been cached in the image */

   if(image->cachedLayout)
   {  RS03Layout *ptr = (RS03Layout*)image->cachedLayout; 
      if(strncmp((char*)(ptr->eh->method), "RS03", 4))
      {  Verbose("CalcRS03Layout(): removed cached layout from other codec\n");
	 g_free(image->cachedLayout);
      }
      else 
      {  if(((RS03Layout*)image->cachedLayout)->target != target)
	 {  Verbose("CalcRS03Layout(): removed cached layout from RS03, wrong target\n");
	    g_free(image->cachedLayout);
	 }
	 {  Verbose("CalcRS03Layout(): returning cached layout (%s)\n",
		    ((RS03Layout*)image->cachedLayout)->target == ECC_FILE ? "file" : "augmented");
	    memcpy(lay, image->cachedLayout, sizeof(RS03Layout));
	    return lay;
	 }
      }
   }

   lay->target = target;

   /* We are going to create an error correction file */

   if(target == ECC_FILE)
   {  guint64 filesize;
      int n_roots = 0;
      char last = 0;

      lay->eh = image->eccFileHeader;

      if(lay->eh)  /* Header given; get number of roots from there */
      {  n_roots = lay->eh->eccBytes;
	 lay->dataSectors = uchar_to_gint64(lay->eh->sectors);
	 lay->inLast = lay->eh->inLast;
      }
      else    /* Calculate number of roots */
      {
	 /* Calculate image size in sectors */

	 if(!LargeStat(Closure->imageName, &filesize))
	    Stop(_("Image file %s not present."),Closure->imageName);

	 CalcSectors(filesize, &lay->dataSectors, &lay->inLast);

	 /* Calculate wanted redundancy from Closure->redundancy */

	 if(Closure->redundancy) /* get last char of redundancy parameter */
	 {  int len = strlen(Closure->redundancy);

	    if(len) last = Closure->redundancy[len-1];
	 }

	 switch(last)
	 {  case '%': 
	    {  double p = atof(Closure->redundancy);

	       if(p<3.2 || p>200.0)
		  Stop(_("Redundancy %4.1f%% out of useful range [3.2%%..200%%]"),p);
	       n_roots = (int)round((GF_FIELDMAX*p) / (100.0+p));
	       break;
	    }
	    case 'm': 
	    {  gint64 ecc_size;

	       ecc_size = strtoll(Closure->redundancy, NULL, 10);
	       if(   ecc_size < ecc_file_size(lay->dataSectors, 8) 
		  || ecc_size > ecc_file_size(lay->dataSectors, 170))
		  Stop(_("Ecc file size %lldm out of useful range [%lld .. %lld]"),
		       ecc_size, 
		       ecc_file_size(lay->dataSectors, 8), 
		       ecc_file_size(lay->dataSectors, 170));

	       for(n_roots=170; n_roots>8; n_roots--)
		  if(ecc_size >= ecc_file_size(lay->dataSectors, n_roots))
		     break;
	       break;
	    }

	    default:
	       if(!Closure->redundancy || !strcmp(Closure->redundancy, "normal")) n_roots = 32; 
	       else if(!strcmp(Closure->redundancy, "high")) n_roots = 64;
	       else n_roots = atoi(Closure->redundancy);
	       break;
	 }
      }

      if(n_roots < 8 || n_roots > 170)
	 Stop(_("Redundancy %d out of useful range [8..170]."),n_roots);

      /* Now we have settled for the number of roots,
	 so calculate the layout. */

      lay->dataPadding = 0; /* always zero for ecc files */
      lay->nroots = n_roots;
      lay->ndata = GF_FIELDMAX - n_roots;

      lay->sectorsPerLayer = (lay->dataSectors + lay->ndata - 2)/(lay->ndata-1);
      lay->totalSectors = lay->dataSectors + 2 + (lay->nroots+1)*lay->sectorsPerLayer;

      lay->mediumCapacity   = 0;  /* unused for ecc files */
      lay->eccHeaderPos     = 0;
      lay->firstCrcPos      = 2;
      lay->firstEccPos      = lay->firstCrcPos + lay->sectorsPerLayer;
      lay->redundancy = ((double)lay->nroots*100.0)/(double)lay->ndata;
   }

   /* We are going to augment an image file */

   if(target == ECC_IMAGE)         
   {  gint64 dataSectors;

      /* Determine smallest possible medium format which
	 can hold the image plus at least 8 roots for ecc.
	 Overriding the medium size via --debug is not recommended
	 as it may render the image irrecoverable in the error case. */

      lay->eh = image->eccHeader;

      if(lay->eh)
      {  dataSectors = uchar_to_gint64(lay->eh->sectors);
      }
      else
      {  dataSectors = image->sectorSize;
	 if(Closure->debugMode && Closure->mediumSize)
	 {   if(dataSectors >= Closure->mediumSize)
	       Stop(_("Medium size smaller than image size (%lld < %lld)"), Closure->mediumSize, dataSectors);
	     lay->mediumCapacity = Closure->mediumSize;
         }
	 else
	 {  if(get_roots(dataSectors, CDR_SIZE) >= 8)
	       lay->mediumCapacity = CDR_SIZE;          /* CDR */
	    else if(get_roots(dataSectors, DVD_SL_SIZE) >= 8)
	       lay->mediumCapacity = DVD_SL_SIZE;       /* Single layered DVD */
	    else if(get_roots(dataSectors, DVD_DL_SIZE) >= 8)
	       lay->mediumCapacity = DVD_DL_SIZE;       /* Double layered DVD */
	    else if(get_roots(dataSectors, BD_SL_SIZE) >= 8)
	       lay->mediumCapacity = BD_SL_SIZE;        /* Single layered BD */
	    else  lay->mediumCapacity = BD_DL_SIZE;     /* Double layered BD */
	 }
      }

      /* Calculate the image layout */

      if(lay->eh) lay->sectorsPerLayer = lay->eh->sectorsPerLayer;
      else        lay->sectorsPerLayer = lay->mediumCapacity/GF_FIELDMAX;
      lay->dataSectors      = dataSectors;
      lay->totalSectors     = GF_FIELDMAX*lay->sectorsPerLayer;

      lay->ndata       = (dataSectors + 2 + lay->sectorsPerLayer - 1) / lay->sectorsPerLayer;
      if(lay->ndata < 84) /* we clip redundancy at 170 roots */
      {  Verbose("Redundancy clipped from %d to %d\n", lay->ndata, 84);
	 lay->ndata = 84;
      }	 
      lay->dataPadding = lay->ndata * lay->sectorsPerLayer - lay->dataSectors - 2;
      lay->ndata++;    /* CRC layer is also protected and counted as part of the data portion */
      lay->nroots      = GF_FIELDMAX-lay->ndata; 
      lay->redundancy = ((double)lay->nroots*100.0)/(double)lay->ndata;
      
      lay->eccHeaderPos     = lay->dataSectors;
      lay->firstCrcPos      = (lay->ndata-1)*lay->sectorsPerLayer;
      lay->firstEccPos      = lay->firstCrcPos + lay->sectorsPerLayer;
   }

   /* Debugging output */

   if(target == ECC_FILE)
        Verbose("Calculated layout for RS03 file:\n");
   else Verbose("Calculated layout for RS03 image:\n");
   
   Verbose("data sectors      = %lld\n", lay->dataSectors);
   Verbose("data padding      = %lld\n", lay->dataPadding);
   Verbose("layer size        = %lld\n", lay->sectorsPerLayer);
   Verbose("total sectors     = %lld\n", lay->totalSectors);
   Verbose("medium capacity   = %lld\n", lay->mediumCapacity);
   Verbose("header position   = %lld\n", lay->eccHeaderPos);
   Verbose("first CRC sector  = %lld\n", lay->firstCrcPos);
   Verbose("first ECC sector  = %lld\n", lay->firstEccPos);
   Verbose("ndata             = %d\n", lay->ndata);
   Verbose("nroots            = %d (%4.1f%%)\n", lay->nroots, lay->redundancy);
   Verbose("\n");

   image->cachedLayout = g_malloc(sizeof(RS03Layout));
   memcpy(image->cachedLayout, lay, sizeof(RS03Layout));

   return lay;
}

/*
 * Determine expected size of image.
 * In case of ecc files, only the iso image size is reported.
 */

guint64 RS03ExpectedImageSize(Image *image)
{  EccHeader *eh=image->eccHeader;
   guint64 size = 0;

   if(!eh && image->eccFileHeader)
     eh=image->eccFileHeader;

   if(!eh) return 0;

   if(eh->methodFlags[0] & MFLAG_ECC_FILE)
     size = uchar_to_gint64(eh->sectors); /* ecc file */
   else
     size = 255*eh->sectorsPerLayer;      /* augmented image */

   return size;
}


/***
 *** Write the RS03 header into the image.
 ***/

void WriteRS03Header(LargeFile *file, RS03Layout *lay, EccHeader *eh)
{  int n;

   if(!LargeSeek(file, 2048*lay->eccHeaderPos))
     Stop(_("Failed seeking to ecc header at %lld: %s\n"), lay->eccHeaderPos, strerror(errno));
   
   n = LargeWrite(file, eh, sizeof(EccHeader));
   if(n != sizeof(EccHeader))
     Stop(_("Failed writing ecc header at %lld: %s\n"), lay->eccHeaderPos, strerror(errno));
}

/***
 *** Reconstruct the RS03 header from a CRC block 
 ***/

void ReconstructRS03Header(EccHeader *eh, CrcBlock *cb)
{  int i;

#ifdef HAVE_BIG_ENDIAN
   SwapCrcBlockBytes(cb);
#endif

   memset(eh, 0, sizeof(EccHeader));

   memcpy(eh->cookie, "*dvdisaster*", 12);
   memcpy(eh->method, "RS03", 4);
   for(i=0; i<4; i++)
      eh->methodFlags[i] = cb->methodFlags[i];
   memcpy(eh->mediumFP, cb->mediumFP, 16);
   memcpy(eh->mediumSum, cb->mediumSum, 16);
   gint64_to_uchar(eh->sectors, cb->dataSectors);
   eh->dataBytes = cb->dataBytes;
   eh->eccBytes = cb->eccBytes;
   eh->creatorVersion = cb->creatorVersion;
   eh->neededVersion = cb->neededVersion;
   eh->fpSector = cb->fpSector;
   eh->inLast = cb->inLast;
   eh->sectorsPerLayer = cb->sectorsPerLayer;

   eh->selfCRC = 0x4c5047;

#ifdef HAVE_BIG_ENDIAN
   eh->selfCRC = 0x47504c00;
#endif

   eh->selfCRC = Crc32((unsigned char*)eh, 4096);
}
