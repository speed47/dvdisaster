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

/* Needed version must be consistent between CRC blocks and ECC headers
   since ECC headers may be reconstructed from CRC blocks. */

#define NEEDED_VERSION 7900

//#define VERBOSE 1
#ifdef VERBOSE
  #define verbose(format,args...) printf(format, ## args)
#else
  #define verbose(format,args...)
#endif

#ifdef HAVE_MMAP
  #include <sys/mman.h>

#ifdef SYS_LINUX
  #define MMAP_FLAGS (MAP_SHARED | MAP_POPULATE | MAP_NORESERVE) 
#endif

#ifdef SYS_FREEBSD
  #define MMAP_FLAGS (MAP_SHARED | MAP_PREFAULT_READ) 
#endif

#ifdef SYS_NETBSD
  #define MMAP_FLAGS (MAP_SHARED) 
#endif

#endif

/***
 *** Local data package used during encoding
 ***/

typedef struct
{  Method *self;
   Image *image;
   RS03Widgets *wl;
   RS03Layout *lay;
   EccHeader *eh;              /* ecc header in native byte order */
   EccHeader *eh_le;           /* ecc header in little endian order */
   GaloisTables *gt;           /* common lookup tables for RS encoders */
   ReedSolomonTables *rt;

   guint32 pageSize;           /* needed for memory mapping */
   unsigned char **ioData;     /* shared buffers between IO and RS threads */
   guint32 *ioCrc;             /* only an alias pointer into data! */
   unsigned char **ioMmapBase; /* mmap() works on multiples of page sizes */
   guint64 *ioMmapSize;        /* so the mmap area might differ from sector range */
   unsigned char **encoderData;/* shared buffers between IO and RS threads */
   guint32 *encoderCrc;        /* only an alias pointer into data! */
   unsigned char **encoderMmapBase;
   guint64 *encoderMmapSize;
   unsigned char *paritybase;
   unsigned char *parity;
   unsigned char **slice;
   int slicesFree;          /* flag for sharing it between IO and encoder */
   guint32 *firstCrc;       /* storage for first CRC block */
   guint64 chunkSize;       /* we can process this much layer sectors at a time */
   guint64 chunkBytes;      /* 2048 * above */

   /* The IO and encoder threads are working interleaved.
      Each one keeps track of its state in a separate data set. */
   
   guint64 ioChunk;         /* chunk we are currently working on */
   guint64 encoderChunk; 
   guint64 flushChunk; 
   guint64 ioLayerSectors;  /* last layer maybe smaller than chunkSize */
   guint64 encoderLayerSectors;  
   guint64 flushLayerSectors;  

   GMutex *lock;            /* lock on this struct */
   GCond *ioCond;           /* sync between encoder and IO threads */
   GTimer *avgTimer;        /* total (=average encoding timer) */
   GTimer *contTimer;       /* continuous timing */
   guint64 sectorsToEncode; /* total number of sector to encode */
   int buffersToEncode;     /* number of unprocessed buffers */
   int nextBufferIndex;     /* next buffer which needs to be encoded */
   GThread *thread[MAX_CODEC_THREADS];
   char *msg;
   int earlyTermination;
   int abortImmediately;

   LargeFile *writeHandle;  /* additional image file handle for writing */ 
   int progress;            /* for the status gauge / message */
   int lastProgress;
   int lastPercent;
   int cpuBound,ioBound;
} ecc_closure;

static void ecc_cleanup(gpointer data)
{  ecc_closure *ec = (ecc_closure*)data;
   int i;

   UnregisterCleanup();

   /* Wait for workers to finish if we aborted
      prematurely */

   if(ec->abortImmediately)
   {  
      /* Nudge workers to wake up and abort */

      g_mutex_lock(ec->lock);
      g_cond_broadcast(ec->ioCond);
      g_mutex_unlock(ec->lock);

      /* Wait for all worker to exit */

      for(i=0; i<Closure->codecThreads; i++)
      {  g_thread_join(ec->thread[i]);
	 fflush(stdout);
      }
   }

   if(Closure->guiMode)
   {  if(ec->earlyTermination)
        SetLabelText(GTK_LABEL(ec->wl->encFootline),
		     _("<span %s>Aborted by unrecoverable error.</span>"),
		     Closure->redMarkup); 
      AllowActions(TRUE);
   }

   /*** We must invalidate the CRC cache as it does only cover the
	data portion of the image, not the full RS03 enhanced image. */

   if(Closure->crcCache)
     ClearCrcCache();

   /*** Clean up */

   if(ec->image) CloseImage(ec->image);
   if(ec->lock)
   {  g_mutex_clear(ec->lock);
      g_free(ec->lock);
   }
   if(ec->ioCond) 
   {  g_cond_clear(ec->ioCond);
      g_free(ec->ioCond);
   }
   if(ec->eh) g_free(ec->eh);
   if(ec->eh_le) g_free(ec->eh_le);
   if(ec->rt) FreeReedSolomonTables(ec->rt);
   if(ec->gt) FreeGaloisTables(ec->gt);
   if(ec->paritybase) g_free(ec->paritybase);
   if(ec->msg) g_free(ec->msg);
   if(ec->avgTimer) g_timer_destroy(ec->avgTimer);
   if(ec->contTimer) g_timer_destroy(ec->contTimer);
   if(ec->firstCrc) g_free(ec->firstCrc);

#ifdef HAVE_MMAP
   if(Closure->encodingIOStrategy == IO_STRATEGY_MMAP)
   {  if(ec->lay)
      {  for(i=0; i<ec->lay->ndata-1; i++)
	 {  if(ec->ioMmapBase && ec->ioMmapBase[i])
	    {  if(munmap(ec->ioMmapBase[i], ec->ioMmapSize[i]) == -1)
		  Stop("munmap() failed: %s\n", strerror(errno));
	       ec->ioData[i] = NULL;
	       ec->ioMmapBase[i] = NULL;
	    }

	    if(ec->encoderMmapBase && ec->encoderMmapBase[i])
	    {  if(munmap(ec->encoderMmapBase[i], ec->encoderMmapSize[i]) == -1)
		  Stop("munmap() failed: %s\n", strerror(errno));
	       ec->encoderData[i] = NULL;
	       ec->encoderMmapBase[i] = NULL;
	    }
	 }
      }
      g_free(ec->ioMmapBase);
      g_free(ec->ioMmapSize);
      g_free(ec->encoderMmapBase);
      g_free(ec->encoderMmapSize);
   }
#endif

   if(ec->lay) g_free(ec->lay);

   for(i=0; i<256; i++)
   {  if(ec->slice && ec->slice[i])
         g_free(ec->slice[i]);
      if(ec->ioData && ec->ioData[i])
         g_free(ec->ioData[i]);
      if(ec->encoderData && ec->encoderData[i])
         g_free(ec->encoderData[i]);
   }

   if(ec->slice)  g_free(ec->slice);
   if(ec->ioData) g_free(ec->ioData);
   if(ec->encoderData) g_free(ec->encoderData);
   g_free(ec);

   if(Closure->guiMode)
     g_thread_exit(0);
}

/***
 *** Some sub tasks to be done during encoding
 ***/

/*
 * Abort encoding
 */

static void abort_encoding(ecc_closure *ec, int truncate)
{  RS03Widgets *wl = ec->wl;

   if(truncate && ec->lay)
   {  if(Closure->eccTarget == ECC_FILE)
	 LargeUnlink(Closure->eccName);
      else if(!LargeTruncate(ec->image->file, (gint64)(2048*ec->lay->dataSectors)))
	Stop(_("Could not truncate %s: %s\n"),Closure->imageName,strerror(errno));

      if(Closure->stopActions == STOP_CURRENT_ACTION) /* suppress memleak warning when closing window */
	 SetLabelText(GTK_LABEL(wl->encFootline), 
		      _("<span %s>Aborted by user request!</span> (partial ecc data removed from image)"),
		      Closure->redMarkup); 
   }
   else
   {  if(Closure->stopActions == STOP_CURRENT_ACTION) /* suppress memleak warning when closing window */
	 SetLabelText(GTK_LABEL(wl->encFootline), 
		      _("<span %s>Aborted by user request!</span>"),
		      Closure->redMarkup); 
   }

   ec->earlyTermination = FALSE;   /* suppress respective error message */

   ecc_cleanup((gpointer)ec);
}


/*
 * Remove already existing RS03 ecc data from the image.
 */

static void remove_old_ecc(ecc_closure *ec)
{  guint64 ignore;

   /* Handle error correction file case first */

   if(Closure->eccTarget == ECC_FILE)
   {  if(LargeStat(Closure->eccName, &ignore))
      {  
	 if(ConfirmEccDeletion(Closure->eccName))
	    LargeUnlink(Closure->eccName);
	 else
	 {  SetLabelText(GTK_LABEL(ec->wl->encFootline),
			 _("<span %s>Aborted to keep existing ecc file.</span>"),
			 Closure->redMarkup); 
	    ec->earlyTermination = FALSE;
	    ecc_cleanup((gpointer)ec);
	 }
      }
      return;
   }

   /* Augmented image case */

   if(ec->image->eccHeader)
   {  gint64 data_sectors = uchar_to_gint64(ec->image->eccHeader->sectors);
      guint64 data_bytes;
      int answer;

      if(Closure->confirmDeletion || !Closure->guiMode)
	answer = ModalWarning(GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL, NULL,
			      _("Image \"%s\" already contains error correction information.\n"
				"Truncating image to data part (%lld sectors).\n"),
			      Closure->imageName, data_sectors);
      else answer = TRUE;

      if(!answer)
	abort_encoding(ec, FALSE);

      if(ec->image->eccHeader->inLast != 2048)
      {
	   data_bytes = (gint64)(2048*(data_sectors-1)+ec->image->eccHeader->inLast);
	   ec->image->sectorSize = data_sectors;
	   ec->image->inLast = ec->image->eccHeader->inLast;
      }
      else 
      {
	 data_bytes = (gint64)(2048*data_sectors);
	 ec->image->sectorSize = data_sectors;
	 ec->image->inLast = 2048;
      }

      g_free(ec->image->eccHeader);  /* get rid of old header! */
      ec->image->eccHeader = NULL;

      if(!LargeTruncate(ec->image->file, data_bytes))
	Stop(_("Could not truncate %s: %s\n"),Closure->imageName,strerror(errno));

      PrintLog(_("Image size is now"));
      if(ec->image->inLast == 2048)
           PrintLog(_(": %lld medium sectors.\n"), ec->image->sectorSize);
      else PrintLog(_(": %lld medium sectors and %d bytes.\n"), 
		   ec->image->sectorSize-1, ec->image->inLast);
   }
}

/*
 * Fill in the necessary values for the EccHeader.
 */

static void prepare_header(ecc_closure *ec)
{  EccHeader *eh;
   RS03Layout *lay = ec->lay;
   Image *image = ec->image;
   
   ec->eh    = g_malloc0(sizeof(EccHeader));
   ec->eh_le = g_malloc0(sizeof(EccHeader));
   eh = lay->eh = ec->eh;

   memcpy(eh->cookie, "*dvdisaster*", 12);
   memcpy(eh->method, "RS03", 4);
   eh->methodFlags[0]  = Closure->eccTarget == ECC_FILE ? MFLAG_ECC_FILE : 0;
   memcpy(eh->mediumFP, image->imageFP, 16);
   memcpy(eh->mediumSum, image->mediumSum, 16);
   gint64_to_uchar(eh->sectors, lay->dataSectors);
   eh->dataBytes       = lay->ndata;
   eh->eccBytes        = lay->nroots;

   eh->creatorVersion  = Closure->version;
   eh->neededVersion   = NEEDED_VERSION;
   eh->fpSector        = FINGERPRINT_SECTOR;
   eh->inLast          = image->inLast;
   eh->sectorsPerLayer = lay->sectorsPerLayer;

   eh->selfCRC = 0x4c5047;

   memcpy(ec->eh_le, eh, sizeof(EccHeader));

#ifdef HAVE_BIG_ENDIAN
   SwapEccHeaderBytes(ec->eh_le);
   ec->eh_le->selfCRC = 0x47504c00;
#endif

   eh->selfCRC = Crc32((unsigned char*)eh, 4096);
   ec->eh_le->selfCRC = Crc32((unsigned char*)ec->eh_le, 4096);
}

/*
 * Expand the image by lay->eccSectors.
 * This avoids horrible file fragmentation under some file systems. 
 */

static void expand_image(ecc_closure *ec)
{  RS03Layout *lay = ec->lay;
   Image *image = ec->image;
   int last_percent, percent, n;
   gint64 sectors,ecc_padding;
   LargeFile *ecc_out;
   char *failed_write, *progress_msg;

   /* Output file depends on ecc target */

   if(Closure->eccTarget == ECC_FILE)
   {    ecc_out = image->eccFile;
        failed_write = _("Failed expanding the ecc file: %s\n");
	progress_msg = _("Preparing ecc file: %3d%%");
   }
   else
   {    ecc_out = image->file;
        failed_write = _("Failed expanding the image: %s\n");
	progress_msg = _("Preparing image: %3d%%");
   }

   /* If the image file does not end at a sector boundary,
      fill it up with zeros. */

   if(Closure->eccTarget == ECC_IMAGE && image->inLast != 2048)
   {  int fill = 2048 - image->inLast;
      int n;
      unsigned char zeros[fill];

      memset(zeros, 0, fill);

      if(!LargeSeek(image->file, image->file->size))
	Stop(_("Failed seeking to end of image: %s\n"), strerror(errno));

      n = LargeWrite(image->file, zeros, fill);
      if(n != fill)
	Stop(_(failed_write), strerror(errno));
   }

   /* Seek to end of file if augmenting an image */

   if(Closure->eccTarget == ECC_IMAGE)
      if(!LargeSeek(image->file, 2048*lay->dataSectors))
	 Stop(_("Failed seeking to end of image: %s\n"), strerror(errno));

   /* Space for the ecc header */

   prepare_header(ec);
   n = LargeWrite(ecc_out, ec->eh_le, 4096);
   if(n != 4096)
      Stop(_(failed_write), strerror(errno));
   if(Closure->eccTarget == ECC_IMAGE)
   {  image->file->size += 4096;
      image->sectorSize += 2;
   }

   /* Padding sectors for the data section */

   for(sectors=0; sectors<lay->dataPadding; sectors++)
   {  unsigned char pad_sector[2048];
      int n;

      CreatePaddingSector(pad_sector, lay->dataSectors+sectors+2, image->imageFP, FINGERPRINT_SECTOR);

      n = LargeWrite(ecc_out, pad_sector, 2048);
      if(n != 2048)
	Stop(_(failed_write), strerror(errno));
      if(Closure->eccTarget == ECC_IMAGE)
      {  image->file->size += 2048;
	 image->sectorSize ++;
      }
   }

   /* Padding sectors for the CRC section */

   for(sectors=0; sectors<lay->sectorsPerLayer; sectors++)
   {  unsigned char pad_sector[2048];
      int n;

      CreateMissingSector(pad_sector, lay->firstCrcPos+sectors, image->imageFP, FINGERPRINT_SECTOR, 
		"CRC padding by expand_image()");

      n = LargeWrite(ecc_out, pad_sector, 2048);
      if(n != 2048)
	Stop(_(failed_write), strerror(errno));
   }

   /* Now add the sectors needed for the ecc data */

   last_percent = 0;
   ecc_padding = lay->nroots*lay->sectorsPerLayer;
   for(sectors = 0; sectors < ecc_padding; sectors++)
   {  unsigned char dead_sector[2048];
      int n;

      if(Closure->stopActions) /* User hit the Stop button */
	abort_encoding(ec, TRUE);

      CreateMissingSector(dead_sector, lay->firstEccPos+sectors, image->imageFP, FINGERPRINT_SECTOR, 
		"ECC padding by expand_image()");

      n = LargeWrite(ecc_out, dead_sector, 2048);
      if(n != 2048)
	Stop(_(failed_write), strerror(errno));

      percent = (100*sectors) / ecc_padding;

      if(last_percent != percent)
      {  PrintProgress(_(progress_msg), percent);

	 if(Closure->guiMode)
	   SetProgress(ec->wl->encPBar1, percent, 100);

	 last_percent = percent; 
      }
   }

   PrintProgress(_(progress_msg), 100);
   PrintProgress("\n");

   if(Closure->guiMode)
     SetProgress(ec->wl->encPBar1, 100, 100);
}

/*
 * Fill in the necessary values for the CrcBlock.
 */

static void prepare_crc_block(ecc_closure *ec, CrcBlock *cb)
{  RS03Layout *lay = ec->lay;
   Image *image = ec->image;

   memcpy(cb->cookie, "*dvdisaster*", 12);
   memcpy(cb->method, "RS03", 4);
   cb->methodFlags[0]  = Closure->eccTarget == ECC_FILE ? MFLAG_ECC_FILE : 0; 
   cb->creatorVersion  = Closure->version;
   cb->neededVersion   = NEEDED_VERSION;
   cb->fpSector        = FINGERPRINT_SECTOR;
   memcpy(cb->mediumFP, image->imageFP, 16);
   memcpy(cb->mediumSum, image->mediumSum, 16);
   cb->dataSectors     = lay->dataSectors;   
   cb->inLast          = image->inLast;
   cb->dataBytes       = lay->ndata;
   cb->eccBytes        = lay->nroots;
   cb->sectorsPerLayer = lay->sectorsPerLayer;

   cb->selfCRC = 0x4c5047;

#ifdef HAVE_BIG_ENDIAN
   SwapCrcBlockBytes(cb);
   cb->selfCRC = 0x47504c00;
#endif

   cb->selfCRC = Crc32((unsigned char*)cb, 2048);
}

/*
 * Calculate the Reed-Solomon error correction code
 */

/* The IO thread. Reads the image sectors and dispatches them to the
   Reed-Solomon encoder threads. Does also collect and write out the CRC and
   parity sectors. */

static void flip_buffers(ecc_closure *ec)
{  unsigned char **dtmp;
   guint32 *ctmp;
   guint64 *etmp;

   ctmp = ec->ioCrc;  ec->ioCrc  = ec->encoderCrc;  ec->encoderCrc  = ctmp;
   dtmp = ec->ioData; ec->ioData = ec->encoderData; ec->encoderData = dtmp;
   dtmp = ec->ioMmapBase; ec->ioMmapBase = ec->encoderMmapBase; ec->encoderMmapBase = dtmp;
   etmp = ec->ioMmapSize; ec->ioMmapSize = ec->encoderMmapSize; ec->encoderMmapSize = etmp;
}

static void read_next_chunk(ecc_closure *ec, guint64 chunk)
{  RS03Layout *lay = ec->lay;
   int layer;

   /* The last chunk may contain fewer sectors. */

   ec->ioChunk = chunk;
   if(ec->ioChunk+ec->chunkSize < lay->sectorsPerLayer)
      ec->ioLayerSectors = ec->chunkSize;
   else 
   {  ec->ioLayerSectors = lay->sectorsPerLayer-ec->ioChunk;
      verbose("NOTE: actual_layer_sectors %d\n", ec->ioLayerSectors);
   }

   memset(ec->ioCrc, 0, ec->chunkBytes);

   /* Read the next layers of the current chunk. */

   for(layer=0; layer<lay->ndata-1; layer++) /* exclude CRC layer */
   {  guint64 first_sec = layer*lay->sectorsPerLayer+ec->ioChunk;
      guint64 error_sec;
      int err=0;
#ifdef HAVE_MMAP
      int shift;
      guint64 page_offset;
#endif

      if(Closure->stopActions) /* User hit the Stop button */
      {  ec->abortImmediately = TRUE;
	 abort_encoding(ec, TRUE);
      }

      /* Read the next data sectors of this layer.
	 Note that the last layer is made from CRC sums. */

#ifdef HAVE_MMAP
      if(Closure->encodingIOStrategy == IO_STRATEGY_MMAP)
      {  if(ec->ioMmapBase[layer])
	 {  if(munmap(ec->ioMmapBase[layer], ec->ioMmapSize[layer]) == -1)
	       Stop("munmap() failed: %s\n", strerror(errno));
	    ec->ioMmapBase[layer] = NULL;
	    ec->ioData[layer] = NULL;
	 }

	 /* There is a padding area between the last data sector and the
	    CRC layer. In the augmented image case, these padding sectors
	    are actually present in the image, but when creating ecc files
	    these sectors do not exist. Therefore we cannot use memory mapping
	    and need to switch to normal IO when reading beyond the image
	    in the ecc file case (RS03ReadSectors will produce the
	    padding sectors in memory). */

	 if(Closure->eccTarget == ECC_FILE
	    && RS03SectorIndex(lay, layer, ec->ioChunk+ec->ioLayerSectors) 
	    >= lay->dataSectors)
	 {  guint64 n_sectors = ec->ioLayerSectors;

	    if(!ec->ioData[layer])
	       ec->ioData[layer] = g_malloc(ec->chunkBytes+2048);

	    if(ec->ioChunk+ec->ioLayerSectors < lay->sectorsPerLayer)
	       n_sectors++;

	    RS03ReadSectors(ec->image, lay, ec->ioData[layer], 
			    layer, ec->ioChunk, n_sectors, RS03_READ_DATA);
	 }
	 else /* can use memory mapping */
	 {
	    page_offset = 2048*RS03SectorIndex(lay, layer, ec->ioChunk);
	    shift = page_offset % ec->pageSize;
	    page_offset -= shift;
	    
	    if(ec->ioLayerSectors == ec->chunkSize)
	         ec->ioMmapSize[layer] = 2048*ec->ioLayerSectors + 2048 + shift;
	    else ec->ioMmapSize[layer] = 2048*ec->ioLayerSectors + shift;

	    ec->ioMmapBase[layer] = mmap(NULL, ec->ioMmapSize[layer],
					 PROT_READ, MMAP_FLAGS,
					 ec->image->file->fileHandle, 
					 page_offset);
	    if(ec->ioMmapBase[layer] == MAP_FAILED)
	       Stop(_("Failed mmap()ing layer %d: %s\n"), layer, strerror(errno));

	    ec->ioData[layer] = ec->ioMmapBase[layer]+shift;
	 }
      }
      else
#endif /* HAVE_MMAP */
      {
	 RS03ReadSectors(ec->image, lay, ec->ioData[layer], 
			 layer, ec->ioChunk, ec->ioLayerSectors, RS03_READ_DATA);
      }

      err = CheckForMissingSectors(ec->ioData[layer], first_sec, 
				   lay->eh->mediumFP, lay->eh->fpSector, 
				   ec->ioLayerSectors, &error_sec);

      if(err != SECTOR_PRESENT)
      {   /* Remove partial ecc data */
	  if(Closure->eccTarget == ECC_FILE)
	  {  LargeClose(ec->image->eccFile);
	     ec->image->eccFile = ec->writeHandle = NULL;
	     LargeUnlink(Closure->eccName);
	  }
	  else
	  {  LargeTruncate(ec->writeHandle, (gint64)(2048*lay->dataSectors));
	  }

	  ec->abortImmediately = TRUE;

	  Stop(_("Incomplete image\n\n"
		 "The image contains missing sectors,\n"
		 "e.g. sector %lld.\n%s"
		 "Error correction data works like a backup; it must\n"
		 "be created when the image is still fully readable.\n"
		 "Exiting and removing partial error correction data."),
	       error_sec,
	       err == SECTOR_MISSING ? "\n" :
	       _("\nThis image was probably mastered from defective source(s).\n"
		 "Perform a \"Verify\" action for more information.\n\n"));
      }

      /* One sector more to chain back the CRC sums
         (unless we are already in the last chunk).
         Additional space is provided in the ec->ioData buffer. */

      if(Closure->encodingIOStrategy == IO_STRATEGY_READWRITE)
      {
	 if(ec->ioChunk+ec->ioLayerSectors < lay->sectorsPerLayer)
	 {  
	    RS03ReadSectors(ec->image, lay, ec->ioData[layer]+ec->chunkBytes, 
			    layer, ec->ioChunk+ec->ioLayerSectors, 1, RS03_READ_DATA);
	 }
      }
   } /* all layers from chunk finished */
}

static void flush_crc(ecc_closure *ec, LargeFile *file_out)
{  RS03Layout *lay = ec->lay;
   gint64 crc_sect;
   gint64 i;

   /* Write out the CRC layer */
      
   verbose("IO: writing CRC layer\n");
   crc_sect = 2048*(ec->encoderChunk+lay->firstCrcPos);
   if(!LargeSeek(file_out, crc_sect))
   {  ec->abortImmediately = TRUE;

      Stop(_("Failed seeking to sector %lld in image: %s"), crc_sect, strerror(errno));
   }
   for(i=0; i<ec->encoderLayerSectors; i++)
      if(LargeWrite(file_out, ec->encoderCrc+512*i, 2048) != 2048)
      {  ec->abortImmediately = TRUE;
	 Stop(_("Failed writing to sector %lld in image: %s"), crc_sect, strerror(errno));
      }
}

static void flush_parity(ecc_closure *ec, LargeFile *file_out)
{  RS03Layout *lay = ec->lay;
   gint64 i;
   int k;

   /* Write out the created parity. */

   verbose("IO: writing parity...\n");
   for(k=0; k<lay->nroots; k++)
   {  gint64 idx=0;

      for(i=0; i<ec->flushLayerSectors; i++, idx+=2048)
      {  gint64 s = RS03SectorIndex(lay, k+lay->ndata, ec->flushChunk+i);
	
	 if(!LargeSeek(file_out, 2048*s))
	 {  ec->abortImmediately = TRUE;
	    Stop(_("Failed seeking to sector %lld in image: %s"), s, strerror(errno));
	 }
	 if(LargeWrite(file_out, ec->slice[k]+idx, 2048) != 2048)
	 {  ec->abortImmediately = TRUE;
	    Stop(_("Failed writing to sector %lld in image: %s"), s, strerror(errno));
	 }
      }
   }
   verbose("IO: parity written.\n");
}

static gpointer io_thread(ecc_closure *ec)
{  RS03Layout *lay = ec->lay;
   LargeFile *file_out = ec->writeHandle;
   int nroots = lay->nroots;
   int ndata  = lay->ndata;
   int nroots_aligned = (nroots+15)&~15; /* 128bit alignment */
   guint64 n_parity_bytes  = (guint64)nroots_aligned * ec->chunkBytes;
   guint64 chunk;
   int needs_preload = 1;
   int parity_available = 0;
   int i;

   verbose("Reader thread initializing\n");

   /*** Allocate local parity buffer aligned at 128bit boundary */

   ec->paritybase = g_malloc(n_parity_bytes+16);      /* output buffer */
   ec->parity     = ec->paritybase + (16- ((unsigned long)ec->paritybase & 15));

   /*** Create buffer for the ndata input layers 
        Space is provided for one more sector so that
        we can read the additional sector needed for
        chaining the CRCs. */

   ec->ioData      = g_malloc0(256*sizeof(unsigned char*));
   ec->encoderData = g_malloc0(256*sizeof(unsigned char*));
#ifdef HAVE_MMAP  /* allocate CRC layer only */
   if(Closure->encodingIOStrategy == IO_STRATEGY_MMAP)
   {  ec->ioMmapBase      = g_malloc0(256*sizeof(unsigned char*));
      ec->encoderMmapBase = g_malloc0(256*sizeof(unsigned char*));
      ec->ioMmapSize      = g_malloc0(256*sizeof(guint64));
      ec->encoderMmapSize = g_malloc0(256*sizeof(guint64));
      ec->ioData[ndata-1]      = g_malloc(ec->chunkBytes);
      ec->encoderData[ndata-1] = g_malloc(ec->chunkBytes);
   }
   else
#endif /* HAVE_MMAP*/
   {  for(i=0; i<ndata; i++)
      {  ec->ioData[i] = g_malloc(ec->chunkBytes+2048);
	 ec->encoderData[i] = g_malloc(ec->chunkBytes+2048);
      }
   }

   ec->ioCrc      = (guint32*)ec->ioData[ndata-1]; /* CRC layer */
   ec->encoderCrc = (guint32*)ec->encoderData[ndata-1]; 
   ec->firstCrc   = g_malloc(256*sizeof(guint32));

   /*** Create buffers for dividing the ecc information into nroots slices */

   ec->slice      = g_malloc0(256*sizeof(unsigned char*));
   for(i=0; i<nroots; i++)
     ec->slice[i] = g_malloc(ec->chunkBytes);

   Verbose("Cache allocation: %lldK+%lldK+%lldK=%lldM (data+parity+descrambling)\n",
	   (long long)((2*ec->chunkBytes*ndata)/1024),
	   (long long)((n_parity_bytes)/1024),
	   (long long)((ec->chunkBytes*nroots)/1024),
	   (long long)((2*ec->chunkBytes*ndata+n_parity_bytes+ec->chunkBytes*nroots)/(1024*1024)));

   /*** Create ecc information for the protected sectors portion of the image. */ 

   /* Process the image.
      From each layer a chunk of ec->chunkSize sectors is read in at once.
      So after (lay->sectorsPerLayer/ec->chunkSize)+1 iterations 
      the whole image has been processed. */

   verbose("NOTE: ndata = %d, chunk size = %d\n", ndata, ec->chunkSize);
   verbose("NOTE: sectors per layer = %lld\n", (long long)lay->sectorsPerLayer);

   for(chunk=0; chunk<lay->sectorsPerLayer; chunk+=ec->chunkSize) 
   {  int cpu_bound = 0;

      verbose("Starting IO processing for chunk %d\n", chunk);

      /* preload first chunk */

      if(needs_preload)
      {  read_next_chunk(ec, chunk);
	 //	 flush_crc(ec, file_out);  // FIXME
	 needs_preload = 0;
	 verbose("IO: first chunk loaded\n");
	 continue;
      }

      /* Broadcast read to the worker threads */
      if(parity_available)
         flush_crc(ec, file_out);

      flip_buffers(ec);

      g_mutex_lock(ec->lock);
      ec->buffersToEncode     = ec->ioLayerSectors;
      ec->encoderLayerSectors = ec->ioLayerSectors;
      ec->nextBufferIndex     = 0;
      ec->encoderChunk        = ec->ioChunk;
      ec->slicesFree          = FALSE;
      g_cond_broadcast(ec->ioCond);
      g_mutex_unlock(ec->lock);

      /* Write out parity from last run */

      if(parity_available)
      {  //flush_crc(ec, file_out);
	 flush_parity(ec, file_out);
      }

      g_mutex_lock(ec->lock);
      ec->slicesFree = TRUE;  /* we have saved the slices; go ahead */
      g_cond_broadcast(ec->ioCond);
      g_mutex_unlock(ec->lock);

      /* Read the next chunk while encoders are working */

      read_next_chunk(ec, chunk);
      //      flush_crc(ec, file_out);  // FIXME

      /* Remember the current portion for writing it out */

      ec->flushLayerSectors = ec->encoderLayerSectors;
      ec->flushChunk        = ec->encoderChunk;
      parity_available      = TRUE;

      /* Wait until the encoders have finished */

      g_mutex_lock(ec->lock);
      cpu_bound = ec->buffersToEncode;
      while(ec->buffersToEncode)
      {  verbose("IO: Waiting for encoders to finish\n");
	 g_cond_wait(ec->ioCond, ec->lock);
      }
      g_mutex_unlock(ec->lock);

      /* Report progress */

      verbose("IO: chunk %d finished\n", ec->ioChunk);

      if(Closure->guiMode)
      {  if(cpu_bound) 
	 {  SetLabelText(GTK_LABEL(ec->wl->encBottleneck), _("CPU bound"));
	    ec->cpuBound++;
	 }
         else
	 {  SetLabelText(GTK_LABEL(ec->wl->encBottleneck), _("I/O bound"));
	    ec->ioBound++;
	 }
      }
   } /* chunk finished */

   /* Broadcast read to the worker threads */

   flush_crc(ec, file_out);
   flush_parity(ec, file_out);
   flip_buffers(ec);

   g_mutex_lock(ec->lock);
   ec->buffersToEncode     = ec->ioLayerSectors;
   ec->encoderLayerSectors = ec->ioLayerSectors;
   ec->nextBufferIndex     = 0;
   ec->encoderChunk        = ec->ioChunk;
   ec->slicesFree          = FALSE;
   g_cond_broadcast(ec->ioCond);
   g_mutex_unlock(ec->lock);

   /* Wait for encoders to finish last chunk */

   g_mutex_lock(ec->lock);
   ec->slicesFree = TRUE;  /* we have saved the slices; go ahead */
   g_cond_broadcast(ec->ioCond);
   while(ec->buffersToEncode)
   {  verbose("IO: Waiting for encoders to finish last chunk\n");
      g_cond_wait(ec->ioCond, ec->lock);
   }
   g_mutex_unlock(ec->lock);

   /* Write out CRC and parity */

   ec->flushLayerSectors = ec->encoderLayerSectors;
   ec->flushChunk        = ec->encoderChunk;

   flush_crc(ec, file_out);
   flush_parity(ec, file_out);

   verbose("IO: finished\n"); fflush(stdout);
   return NULL;
}


static gpointer encoder_thread(ecc_closure *ec)
{  GThread *self;
   unsigned char *par_ptr;
   int cl_size;
   int my_number=-1;
   int nroots = ec->lay->nroots;
   int ndata  = ec->lay->ndata;
   int nroots_aligned = (nroots+15)&~15;
   int shift[ndata];
   int enc_size = 1;
   int percent;
   int idx;
   int i,j,k;

   /*** Identify ourself */

   self = g_thread_self();

   i=my_number; /* prevents stupid compiler warning */
   g_mutex_lock(ec->lock);
   for(i=0; i<Closure->codecThreads; i++)
     if(ec->thread[i] == self)
       my_number = i;
   g_mutex_unlock(ec->lock);

   /*** Pre-calculate some values */

   cl_size = Closure->clSize;
   if(2048%cl_size != 0)
     cl_size = 64;

   /*** The encoder is repeatedly called on 2K chunks.
	Pre-calculate the shift register state value at the beginning
	of each chunk. */

   shift[0] = ec->rt->shiftInit;
   for(i=1; i<ndata; i++)
     shift[i] = (shift[0] + i) % nroots;

   verbose("ENC: Encoder thread %d initialized.\n", my_number);

   for(;;)
   {  int layer;
      int layer_offset;

      g_mutex_lock(ec->lock);
      while(   ec->sectorsToEncode 
	    && !ec->abortImmediately
	    && ec->nextBufferIndex >= ec->encoderLayerSectors)
      {  verbose("ENC: encoder %d waiting for work\n", my_number);
 	 g_cond_wait(ec->ioCond, ec->lock);
      }
      layer_offset = ec->nextBufferIndex;

      verbose("ENC: encoder %d got work for buffer index %d\n", 
	      my_number,layer_offset);

      /* Termination criterion */

      if(!ec->sectorsToEncode || ec->abortImmediately)  
      {  g_mutex_unlock(ec->lock);
	 verbose("ENC: encoder %d exiting\n", my_number);
	 return NULL;
      }
      ec->nextBufferIndex +=enc_size;
      g_mutex_unlock(ec->lock);

      /* Now process the data bytes of the given layer section. */

      for(layer=0; layer<ndata; layer++)
      {  unsigned char *data   = ec->encoderData[layer] + 2048*layer_offset;
	 unsigned char *parity = ec->parity + 2048*nroots_aligned*layer_offset;

	 /* Calculate the CRC32 layer (ndata-1) */
#if 1
	 if(layer < ndata-1) 
	 {  /* The first ecc block CRC needs to be cached for wrap-around */

	    if(!ec->encoderChunk && !layer_offset)
	    {  ec->firstCrc[layer] = Crc32(data, 2048);
	    }

	    /* Chain back CRC sums from next sector into current one */

	    if(ec->encoderChunk+layer_offset < ec->lay->sectorsPerLayer-1)
	    {  ec->encoderCrc[512*layer_offset+layer] = Crc32(data+2048, 2048);
	    }
	    else /* wrap-around: fill in CRCs from first ecc block */
	    {  ec->encoderCrc[512*layer_offset+layer] = ec->firstCrc[layer];
	    }
	 }

	 if(layer == ndata-1)
	    prepare_crc_block(ec, (CrcBlock*)&ec->encoderCrc[512*layer_offset]);
#endif

	 /* Reed-Solomon part */       

	 if(!layer) /* clear parity if this is a new run */
	   memset(parity, 0, 2048*enc_size*nroots_aligned);

	 EncodeNextLayer(ec->rt, data, parity, 2048*enc_size, shift[layer]);
      }

      /* After processing the last data layer the parity bytes have been
	 prepared as sequences of nroots bytes for this ecc block. 
	 Now we split them up into nroots slices and cache them in the output
	 buffer. */

      g_mutex_lock(ec->lock);
      while(!ec->slicesFree && !ec->abortImmediately)
      {  g_cond_wait(ec->ioCond, ec->lock);
      }
      g_mutex_unlock(ec->lock);

      if(ec->abortImmediately)
	 return NULL;

      idx = 2048*layer_offset;
      par_ptr = ec->parity + 2048*nroots_aligned*layer_offset;

      /* Step through the encoded data in cl_size chunks.
	 If we have enough L1/L2 cache for nroots*cl_size
	 cache lines, we can buffer all reads and writes
	 in the processor cache and get a nice speedup.
	 Even if we don't have enough cache for reads,
	 aligning the writes to cl_size should do something. */

      for(j=2048*enc_size/cl_size; j>0; j--)
      {  
	 for(k=0; k<nroots; k++)
	 {  unsigned char *par = par_ptr+k;
	    unsigned char *slice = &ec->slice[k][idx];

	    /* Collect sufficient roots for a particular slice
	       so that one cache line is filled as writing less
	       than one cache line is very expensive. */

	    for(i=cl_size; i>0; i--)
	    {  *slice++ = *par;
	       par += nroots_aligned;
	    }
	 }

	 idx+=cl_size;
	 par_ptr += cl_size*nroots_aligned;
      }

      g_mutex_lock(ec->lock);
      ec->progress+=enc_size;
      percent = (1000*ec->progress)/ec->lay->sectorsPerLayer;
      if(ec->lastPercent != percent) 
      {
	ec->lastPercent = percent;
	g_mutex_unlock(ec->lock);
	if(Closure->guiMode)
	{    gdouble elapsed;
	     gulong ignore;

	     elapsed=g_timer_elapsed(ec->contTimer, &ignore);
	     if(elapsed > 1.0)
	     {  gdouble mbs = ((double)ndata*(ec->progress-ec->lastProgress))/(512.0*elapsed);
		SetLabelText(GTK_LABEL(ec->wl->encPerformance), 
			     _("%5.2fMiB/s current"), mbs);
		ec->lastProgress = ec->progress;
		g_timer_reset(ec->contTimer);
	     }
             SetProgress(ec->wl->encPBar2, percent, 1000);
	}
	else PrintProgress(_("Ecc generation: %3d.%1d%%"), percent/10, percent%10);
      }
      else g_mutex_unlock(ec->lock);

      /* finish processing of this buffer */

      verbose("ENC: encoder %d finished slice %d/ chunk %d\n", 
	      my_number, layer_offset, ec->encoderChunk);
      g_mutex_lock(ec->lock);
      ec->sectorsToEncode-=enc_size*ndata;
      ec->buffersToEncode-=enc_size;
      if(!ec->buffersToEncode)
      {  g_cond_broadcast(ec->ioCond);
	 verbose("ENC: processed last buffer; telling IO process.\n");
	 fflush(stdout);
      }
      g_mutex_unlock(ec->lock);
   }
}

static void create_reed_solomon(ecc_closure *ec)
{  int nroots = ec->lay->nroots;
   int ndata = ec->lay->ndata;
   int i;
   char *alg="none";
   char *iostrat="none";

   /*** Show the second progress bar */

   if(Closure->guiMode)
   {  ShowWidget(ec->wl->encPBar2);
      ShowWidget(ec->wl->encLabel2);
      ShowWidget(ec->wl->encLabel3);
      ShowWidget(ec->wl->encLabel4);
      ShowWidget(ec->wl->encLabel5);
      ShowWidget(ec->wl->encThreads);
      ShowWidget(ec->wl->encPerformance);
      ShowWidget(ec->wl->encBottleneck);

      DescribeRSEncoder(&alg, &iostrat);

      SetLabelText(GTK_LABEL(ec->wl->encThreads), 
			_("%d threads with %s encoding and %s I/O"),
		   Closure->codecThreads, alg, iostrat);
      SetLabelText(GTK_LABEL(ec->wl->encPerformance), "");
      SetLabelText(GTK_LABEL(ec->wl->encBottleneck), "");
   }

   /*** Calculate buffer size for the parity calculation and image data caching. 

        The algorithm builds the parity file consecutively in chunks of 
	Closure->prefetchSectors sectors.
        We use all the amount of memory allowed by cacheMiB for caching the output 
	parity blocks, and additionally 1/nroots of that memory for caching input.

	Each chunk of parity blocks is built iteratively by processing the data 
	in layers (first all bytes at pos 0, then pos 1, until ndata layers have 
	been processed).

	So we need to buffer 2048*Closure->prefetchSectors of input data.
	For practical reasons we require that the layer size is a multiple of the
	medium sector size of 2048 bytes. */

   ec->chunkBytes  = 2048*Closure->prefetchSectors;
   ec->chunkSize   = Closure->prefetchSectors;

   ec->pageSize = sysconf(_SC_PAGE_SIZE);

   /*** Allocate stuff shared by all threads */

   ec->lock          = g_malloc(sizeof(GMutex)); g_mutex_init(ec->lock);
   ec->ioCond        = g_malloc(sizeof(GCond)); g_cond_init(ec->ioCond);
   ec->sectorsToEncode = ndata*ec->lay->sectorsPerLayer;
   if(Closure->eccTarget == ECC_FILE)
      ec->writeHandle   = ec->image->eccFile;
   else
      ec->writeHandle   = ec->image->file;
   ec->lastPercent   = -1;
   ec->cpuBound = ec->ioBound = 0;

   /*** Initialize the encoder tables*/

   ec->gt  = CreateGaloisTables(RS_GENERATOR_POLY);
   ec->rt  = CreateReedSolomonTables(ec->gt, RS_FIRST_ROOT, RS_PRIM_ELEM, nroots);

   /*** Spawn the RS encoder threads */

   g_mutex_lock(ec->lock);  /* ec->thread[i] = ... may produce race condition */
   for(i=0; i<Closure->codecThreads; i++) 
   {  GError *err = NULL;

      verbose("SCHED: creating encoder %d\n", i);
      ec->thread[i] =  g_thread_try_new("encoder", (GThreadFunc)encoder_thread, (gpointer)ec, &err);
      if(!ec->thread[i])
      {  g_mutex_unlock(ec->lock);
	 ec->abortImmediately = TRUE;
         Stop("Could not create encoder thread: %s", err->message);
      }
   }
   g_mutex_unlock(ec->lock);
   g_thread_yield(); /* FIXME */

   /*** Now we actually become being the IO thread */
   
   io_thread(ec);

   /*** Wait for workers to finish */

   for(i=0; i<Closure->codecThreads; i++)
   {  g_thread_join(ec->thread[i]);
      verbose("SCHED: joined with worker %d\n", i);
      fflush(stdout);
   }
   verbose("SCHED: scheduler finished.\n");
}

/***
 *** Append the parity information to the image
 ***/

void RS03Create(void)
{  Method *method = FindMethod("RS03");
   Image *image = NULL;
   RS03Widgets *wl = (RS03Widgets*)method->widgetList;
   RS03Layout *lay;
   ecc_closure *ec = g_malloc0(sizeof(ecc_closure));
   gdouble elapsed,mbs;
   gulong ignore;
   gint64 ecc_sectors;

   /*** Register the cleanup procedure for GUI mode */

   ec->self = method;
   ec->wl = wl;
   ec->earlyTermination = TRUE;

   RegisterCleanup(_("Error correction data creation aborted"), ecc_cleanup, ec);

   if(Closure->guiMode)  /* Preliminary fill text for the head line */
     SetLabelText(GTK_LABEL(wl->encHeadline),
		  _("<big>Augmenting the image with error correction data.</big>\n<i>%s</i>"), 
		  _("- checking image -"));

   /*** Open image file. */

   PrintLog(_("\nOpening %s"), Closure->imageName);

   if(Closure->eccTarget == ECC_IMAGE)   /* augmented image */
      image = OpenImageFromFile(Closure->imageName, O_RDWR, IMG_PERMS);
   else                                  /* error correction file */
      image = OpenImageFromFile(Closure->imageName, O_RDONLY, IMG_PERMS);
 
   if(!image)
   {  PrintLog(": %s.\n", strerror(errno));
      Stop(_("Image file %s: %s."),Closure->imageName, strerror(errno));
   }

   ec->image = image;

   if(image->inLast == 2048)
        PrintLog(_(": %lld medium sectors.\n"), image->sectorSize);
   else PrintLog(_(": %lld medium sectors and %d bytes.\n"), 
		   image->sectorSize-1, image->inLast);

   /*** If the image already contains error correction information, remove it. */

   remove_old_ecc(ec);

   /*** Need to open ecc file too */ 

   if(Closure->eccTarget == ECC_FILE)
   { 
     if(!Closure->eccName || !strlen(Closure->eccName))
       Stop(_("No error correction file specified!\n"));

     image->eccFile = LargeOpen(Closure->eccName, O_RDWR | O_CREAT, IMG_PERMS);
     if(!image->eccFile)
       Stop(_("Can't open %s:\n%s"),Closure->eccName,strerror(errno));
   }

   /*** Calculate the RS03 layout */

   lay = ec->lay = CalcRS03Layout(image, Closure->eccTarget);

   /*** Announce what we are going to do */

   ecc_sectors = lay->nroots*lay->sectorsPerLayer;
   if(Closure->guiMode)  /* Preliminary fill text for the head line */
   {  ec->msg = g_strdup_printf(_("Encoding with Method RS03: %lld MiB data, %lld MiB ecc (%d roots; %4.1f%% redundancy)."),
				lay->dataSectors/512, ecc_sectors/512, lay->nroots, lay->redundancy);

   if(lay->target == ECC_IMAGE)
      SetLabelText(GTK_LABEL(wl->encHeadline),
		   _("<big>Augmenting the image with error correction data.</big>\n<i>%s</i>"), 
		   ec->msg);
   else
      SetLabelText(GTK_LABEL(wl->encHeadline),
		   _("<big>Creating the error correction file.</big>\n<i>%s</i>"), 
		   ec->msg);

   }
   else
   { char *alg, *iostrat;
     DescribeRSEncoder(&alg, &iostrat);
 
     if(Closure->eccTarget == ECC_IMAGE)
	 ec->msg = g_strdup_printf(_("Augmenting image with Method RS03 [%d threads, %s, %s I/O]:\n"
				     "%lld MiB data, %lld MiB ecc (%d roots; %4.1f%% redundancy)."),
				   Closure->codecThreads, alg, iostrat, 
				   lay->dataSectors/512, ecc_sectors/512, lay->nroots, lay->redundancy);
      else
	 ec->msg = g_strdup_printf(_("Creating the error correction file with Method RS03 [%d threads, %s, %s I/O]:\n"
				     "%lld MiB data, %lld MiB ecc (%d roots; %4.1f%% redundancy)."),
				   Closure->codecThreads, alg, iostrat, 
				   lay->dataSectors/512, ecc_sectors/512, lay->nroots, lay->redundancy);

      PrintLog("%s\n",ec->msg);
   }

   /*** Warn if there is not enough space for ecc data */

   if(Closure->eccTarget == ECC_IMAGE && lay->nroots < 8)
     Stop(_("Not enough space on medium left for error correction data.\n"
	    "Data portion of image: %lld sect.; maximum possible size: %lld sect.\n"
	    "If reducing the image size or using a larger medium is not\n"
	    "an option, please create a separate error correction file."),
	  lay->dataSectors, lay->mediumCapacity);

   if(Closure->eccTarget == ECC_IMAGE && lay->redundancy < 20)
   {  int answer;

      answer = ModalWarning(GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL, NULL,
			    _("Using redundancies below 20%%%% may not give\n"
			      "the expected data loss protection.\n"));

      if(!answer)
	abort_encoding(ec, FALSE);
   }

   /*** Expand the image by ecc_sectors. */

   expand_image(ec);

   /*** Create the CRC and Reed-Solomon parts */

   ec->avgTimer = g_timer_new();
   ec->contTimer = g_timer_new();
   create_reed_solomon(ec);
   g_timer_stop(ec->avgTimer);
   g_timer_stop(ec->contTimer);

   /*** Summarize */

   PrintProgress(_("Ecc generation: 100.0%%\n"));
   if(Closure->eccTarget == ECC_IMAGE)
     PrintLog(_("Image has been augmented with error correction data.\n"
		"New image size is %lld MiB (%lld sectors).\n"),
	      (lay->totalSectors)/512,
	      lay->totalSectors);
   else
     PrintLog(_("Error correction file \"%s\" created.\n"
		"Make sure to keep this file on a reliable medium.\n"),
	      Closure->eccName);

   elapsed=g_timer_elapsed(ec->avgTimer, &ignore);
   mbs = ((double)lay->ndata*lay->sectorsPerLayer)/(512.0*elapsed);
   PrintLog(_("Avg performance: %5.2fs (%5.2fMiB/s) total\n"), 
	    elapsed, mbs);
   if(Closure->guiMode)
   {  SetLabelText(GTK_LABEL(wl->encPerformance), _("%5.2fMiB/s average"), mbs);
      SetLabelText(GTK_LABEL(ec->wl->encBottleneck), 
		   _("%d times CPU bound; %d times I/O bound"),
		   ec->cpuBound, ec->ioBound);
   }

   if(Closure->guiMode)
   {  SetProgress(wl->encPBar2, 100, 100);

      if(Closure->eccTarget == ECC_IMAGE)
	SetLabelText(GTK_LABEL(wl->encFootline),
		     _("Image has been augmented with error correction data.\n"
		       "New image size is %lld MiB (%lld sectors).\n"),
		     (lay->totalSectors)/512,
		     lay->totalSectors);
      else
	SetLabelText(GTK_LABEL(wl->encFootline), 
		     _("The error correction file has been successfully created.\n"
		       "Make sure to keep this file on a reliable medium.")); 

   }

   /*** Clean up */

   ec->earlyTermination = FALSE;
   ecc_cleanup((gpointer)ec);
}

