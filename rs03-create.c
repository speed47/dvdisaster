/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2004-2010 Carsten Gnoerlich.
 *  Project home page: http://www.dvdisaster.com
 *  Email: carsten@dvdisaster.com  -or-  cgnoerlich@fsfe.org
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

#include "rs03-includes.h"

//#define VERBOSE 1
#ifdef VERBOSE
  #define verbose(format,args...) printf(format, ## args)
#else
  #define verbose(format,args...)
#endif

/***
 *** Local data package used during encoding
 ***/

typedef struct
{  Method *self;
   RS03Widgets *wl;
   RS03Layout *lay;
   ImageInfo *ii;
   EccInfo *ei;
   EccHeader *eh;
   GaloisTables *gt;           /* common lookup tables for RS encoders */
   ReedSolomonTables *rt;

   unsigned char **ioData;     /* shared buffers between IO and RS threads */
   guint32 *ioCrc;             /* only an alias pointer into data! */
   unsigned char **encoderData;/* shared buffers between IO and RS threads */
   guint32 *encoderCrc;        /* only an alias pointer into data! */
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

   Closure->cleanupProc = NULL;

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

   if(ec->lock) g_mutex_free(ec->lock);
   if(ec->ioCond) g_cond_free(ec->ioCond);
   if(ec->ii) FreeImageInfo(ec->ii);
   if(ec->ei) FreeEccInfo(ec->ei);
   if(ec->eh) g_free(ec->eh);
   if(ec->rt) FreeReedSolomonTables(ec->rt);
   if(ec->gt) FreeGaloisTables(ec->gt);
   if(ec->writeHandle) LargeClose(ec->writeHandle);
   if(ec->lay) g_free(ec->lay);
   if(ec->paritybase) g_free(ec->paritybase);
   if(ec->msg) g_free(ec->msg);
   if(ec->avgTimer) g_timer_destroy(ec->avgTimer);
   if(ec->contTimer) g_timer_destroy(ec->contTimer);
   if(ec->firstCrc) g_free(ec->firstCrc);

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
      else if(!LargeTruncate(ec->ii->file, (gint64)(2048*ec->lay->dataSectors)))
	Stop(_("Could not truncate %s: %s\n"),Closure->imageName,strerror(errno));

      SetLabelText(GTK_LABEL(wl->encFootline), 
		   _("<span %s>Aborted by user request!</span> (partial ecc data removed from image)"),
		   Closure->redMarkup); 
   }
   else
   {  SetLabelText(GTK_LABEL(wl->encFootline), 
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
{  EccHeader *old_eh;
   LargeFile *tmp;
   gint64 ignore;

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

   tmp = LargeOpen(Closure->imageName, O_RDWR, IMG_PERMS);
   if(!tmp)  
     return; /* no image file at all */

   old_eh = FindRS03HeaderInImage(tmp);

   if(old_eh)
   {  gint64 data_sectors = uchar_to_gint64(old_eh->sectors);
      int answer;

      g_free(old_eh);

      answer = ModalWarning(GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL, NULL,
			    _("Image \"%s\" already contains error correction information.\n"
			      "Truncating image to data part (%lld sectors).\n"),
			    Closure->imageName, data_sectors);

      if(!answer)
	abort_encoding(ec, FALSE);

      if(!tmp || !LargeTruncate(tmp, (gint64)(2048*data_sectors)))
	Stop(_("Could not truncate %s: %s\n"),Closure->imageName,strerror(errno));
   }

   LargeClose(tmp);
}

/*
 * Fill in the necessary values for the EccHeader.
 */

static void prepare_header(ecc_closure *ec)
{  ImageInfo *ii = ec->ii;
   EccHeader *eh = ec->eh;
   RS03Layout *lay = ec->lay;

   memcpy(eh->cookie, "*dvdisaster*", 12);
   memcpy(eh->method, "RS03", 4);
   eh->methodFlags[0]  = Closure->eccTarget == ECC_FILE ? MFLAG_ECC_FILE : 0;
   eh->methodFlags[3]  = Closure->releaseFlags;
   memcpy(eh->mediumFP, ii->mediumFP, 16);
   memcpy(eh->mediumSum, ii->mediumSum, 16);
   gint64_to_uchar(eh->sectors, ii->sectors);
   eh->dataBytes       = lay->ndata;
   eh->eccBytes        = lay->nroots;

   eh->creatorVersion  = Closure->version;
   eh->neededVersion   = 7900;
   eh->fpSector        = FINGERPRINT_SECTOR;
   eh->inLast          = ii->inLast;
   eh->sectorsPerLayer = lay->sectorsPerLayer;

   eh->selfCRC = 0x4c5047;

#ifdef HAVE_BIG_ENDIAN
   SwapEccHeaderBytes(eh);
   eh->selfCRC = 0x47504c00;
#endif

   eh->selfCRC = Crc32((unsigned char*)eh, 4096);
}

/*
 * Expand the image by lay->eccSectors.
 * This avoids horrible file fragmentation under some file systems. 
 */

static void expand_image(ecc_closure *ec)
{  RS03Layout *lay = ec->lay;
   ImageInfo *ii = ec->ii;
   EccInfo *ei = ec->ei;
   int last_percent, percent, n;
   gint64 sectors,ecc_padding;
   LargeFile *ecc_out;
   char *failed_write, *progress_msg;

   /* Output file depends on ecc target */

   if(Closure->eccTarget == ECC_FILE)
   {    ecc_out = ei->file;
        failed_write = _("Failed expanding the ecc file: %s\n");
	progress_msg = _("Preparing ecc file: %3d%%");
   }
   else
   {    ecc_out = ii->file;
        failed_write = _("Failed expanding the image: %s\n");
	progress_msg = _("Preparing image: %3d%%");
   }

   /* If the image file does not end at a sector boundary,
      fill it up with zeros. */

   if(Closure->eccTarget == ECC_IMAGE && ii->inLast != 2048)
   {  int fill = 2048 - ii->inLast;
      int n;
      unsigned char zeros[fill];

      memset(zeros, 0, fill);

      if(!LargeSeek(ii->file, ii->size))
	Stop(_("Failed seeking to end of image: %s\n"), strerror(errno));

      n = LargeWrite(ii->file, zeros, fill);
      if(n != fill)
	Stop(_(failed_write), strerror(errno));
   }

   /* Seek to end of file if augmenting an image */

   if(Closure->eccTarget == ECC_IMAGE)
      if(!LargeSeek(ii->file, 2048*lay->dataSectors))
	 Stop(_("Failed seeking to end of image: %s\n"), strerror(errno));

   /* Space for the ecc header */

   prepare_header(ec);
   n = LargeWrite(ecc_out, ec->eh, 4096);
   if(n != 4096)
      Stop(_(failed_write), strerror(errno));

   /* Padding sectors for the data section */

   for(sectors=0; sectors<lay->dataPadding; sectors++)
   {  unsigned char pad_sector[2048];
      int n;

      CreatePaddingSector(pad_sector, lay->dataSectors+sectors+2, ii->mediumFP, FINGERPRINT_SECTOR);

      n = LargeWrite(ecc_out, pad_sector, 2048);
      if(n != 2048)
	Stop(_(failed_write), strerror(errno));
   }

   /* Padding sectors for the CRC section */

   for(sectors=0; sectors<lay->sectorsPerLayer; sectors++)
   {  unsigned char pad_sector[2048];
      int n;

      CreateMissingSector(pad_sector, lay->firstCrcPos+sectors, ii->mediumFP, FINGERPRINT_SECTOR, 
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

      CreateMissingSector(dead_sector, lay->firstEccPos+sectors, ii->mediumFP, FINGERPRINT_SECTOR, 
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
{  ImageInfo *ii = ec->ii;
   RS03Layout *lay = ec->lay;
   
   memcpy(cb->cookie, "*dvdisaster*", 12);
   memcpy(cb->method, "RS03", 4);
   cb->methodFlags[0]  = Closure->eccTarget == ECC_FILE ? MFLAG_ECC_FILE : 0; 
   cb->methodFlags[3]  = Closure->releaseFlags;
   cb->creatorVersion  = Closure->version;
   cb->neededVersion   = 7300;
   cb->fpSector        = FINGERPRINT_SECTOR;
   memcpy(cb->mediumFP, ii->mediumFP, 16);
   memcpy(cb->mediumSum, ii->mediumSum, 16);
   cb->dataSectors     = ii->sectors;   
   cb->inLast          = ii->inLast;
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

   ctmp = ec->ioCrc;  ec->ioCrc  = ec->encoderCrc;  ec->encoderCrc  = ctmp;
   dtmp = ec->ioData; ec->ioData = ec->encoderData; ec->encoderData = dtmp;
}

static void read_next_chunk(ecc_closure *ec, guint64 chunk)
{  RS03Layout *lay = ec->lay;
   gint64 s;
   int layer;

   /* The last chunk may contain fewer sectors. */

   ec->ioChunk = chunk;
   if(ec->ioChunk+ec->chunkSize < lay->sectorsPerLayer)
      ec->ioLayerSectors = ec->chunkSize;
   else {ec->ioLayerSectors = lay->sectorsPerLayer-ec->ioChunk;
      verbose("NOTE: actual_layer_sectors %d\n", ec->ioLayerSectors);
   }

   memset(ec->ioCrc, 0, ec->chunkBytes);

   /* Read the next layers of the current chunk. */

   for(layer=0; layer<lay->ndata-1; layer++) /* exclude CRC layer */
   {  gint64 offset = 0;
      gint64 first_sec = layer*lay->sectorsPerLayer+ec->ioChunk;
      gint64 error_sec;
      int err;

      if(Closure->stopActions) /* User hit the Stop button */
      {  ec->abortImmediately = TRUE;
	 abort_encoding(ec, TRUE);
      }
      /* Read the next data sectors of this layer.
	 Note that the last layer is made from CRC sums. */

      RS03ReadSectors(ec->ii->file, lay, ec->ioData[layer], 
		      layer, ec->ioChunk, ec->ioLayerSectors, RS03_READ_DATA);

      err = CheckForMissingSectors(ec->ioData[layer], first_sec, 
				   lay->eh->mediumFP, lay->eh->fpSector, 
				   ec->ioLayerSectors, &error_sec);
      if(err != SECTOR_PRESENT)
      {   /* Remove partial ecc data */
	  if(Closure->eccTarget == ECC_FILE)
	  {  LargeClose(ec->writeHandle);
	     ec->writeHandle = NULL;
	     LargeUnlink(Closure->eccName);
	  }
	  else
	  {  LargeTruncate(ec->writeHandle, (gint64)(2048*ec->ii->sectors));
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

      for(s=0; s<ec->ioLayerSectors; s++)
      {  
	 /* Read the next sector */

	 offset=s*2048;
	 //	 RS03ReadSector(ec->ii, lay, ec->ioData[layer]+offset, layer, ec->ioChunk+s, RS03_READ_DATA);


	 /* CRC32 part */
#if 1
	 if(ec->ioChunk || s)
	 {  if(s) /* fixme: prove correctness */
	       ec->ioCrc[512*(s-1)+layer] = Crc32(ec->ioData[layer]+offset, 2048);
	 }
	 else /* store CRC for the first ecc block in ecc header */
	 {  ec->firstCrc[layer] = Crc32(ec->ioData[layer]+offset, 2048);
	 }

	 /* The first CRC is wrapped to the last layer:
	    At ecc block 0, CRC sums are stored in first_crc
	    rather than being written to ec->crc.
	    For subsequent ecc blocks, their CRC32 sums are
	    written to the previous ec->crc position.
	    This leaves the last slot in ec->crc blank,
	    which is filled in here from the cached results
	    in first_ecc[]. */ 

	 if(ec->ioChunk+s == lay->sectorsPerLayer-1)
	 {  ec->ioCrc[512*s+layer] = ec->firstCrc[layer];
	 }
#endif
      }

      /* One sector more to chain back the CRC sums
         (unless we are already in the last chunk) */

      if(ec->ioChunk+ec->ioLayerSectors < lay->sectorsPerLayer)
      {  unsigned char buf[2048];

	 RS03ReadSectors(ec->ii->file, lay, buf, layer, ec->ioChunk+ec->ioLayerSectors, 1, RS03_READ_DATA);
	 ec->ioCrc[(ec->ioLayerSectors-1)*512+layer] = Crc32(buf, 2048);
      }
   } /* all layers from chunk finished */

   /* Add and prepare the CrcBlock structure */

#if 1
   for(s=0; s<ec->ioLayerSectors; s++)
      prepare_crc_block(ec, (CrcBlock*)&ec->ioCrc[512*s]);  
#endif
}

static void flush_crc(ecc_closure *ec, LargeFile *file_out)
{  RS03Layout *lay = ec->lay;
   gint64 crc_sect;
   gint64 i;

   /* Write out the CRC layer */
      
   verbose("IO: writing CRC layer\n");
   crc_sect = 2048*(ec->ioChunk+lay->firstCrcPos);
   if(!LargeSeek(file_out, crc_sect))
   {  ec->abortImmediately = TRUE;

      Stop(_("Failed seeking to sector %lld in image: %s"), crc_sect, strerror(errno));
   }
   for(i=0; i<ec->ioLayerSectors; i++)
      if(LargeWrite(file_out, ec->ioCrc+512*i, 2048) != 2048)
      {  ec->abortImmediately = TRUE;
	 Stop(_("Failed writing to sector %lld in image: %s"), crc_sect, strerror(errno));
      }
}

static void flush_parity(ecc_closure *ec, LargeFile *file_out)
{  RS03Layout *lay = ec->lay;
   gint64 i;
   int k;

   /* Write out the created parity.
      Note: ecc sectors are interleaved with headers and thus can
            not be written out using a streaming write. */

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

   /*** Create buffer for the ndata input layers */

   ec->ioData      = g_malloc0(256*sizeof(unsigned char*));
   ec->encoderData = g_malloc0(256*sizeof(unsigned char*));
   for(i=0; i<ndata; i++)
   {  ec->ioData[i] = g_malloc(ec->chunkBytes);
      ec->encoderData[i] = g_malloc(ec->chunkBytes);
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

      verbose("Starting IO processing for chunk %d\n", ec->chunk);

      /* preload first chunk */

      if(needs_preload)
      {  read_next_chunk(ec, chunk);
	 flush_crc(ec, file_out);
	 needs_preload = 0;
	 verbose("IO: first chunk loaded\n");
	 continue;
      }

      /* Broadcast read to the worker threads */

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
	 flush_parity(ec, file_out);

      g_mutex_lock(ec->lock);
      ec->slicesFree = TRUE;  /* we have saved the slices; go ahead */
      g_cond_broadcast(ec->ioCond);
      g_mutex_unlock(ec->lock);

      /* Read the next chunk while encoders are working */

      read_next_chunk(ec, chunk);
      flush_crc(ec, file_out);

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

   flush_parity(ec, file_out);

   verbose("IO: finished\n"); fflush(stdout);
   return NULL;
}


static gpointer encoder_thread(ecc_closure *ec)
{  GThread *self;
   unsigned char *par_ptr;
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

   g_mutex_lock(ec->lock);
   for(i=0; i<Closure->codecThreads; i++)
     if(ec->thread[i] == self)
       my_number = i;
   g_mutex_unlock(ec->lock);

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
      int layer_index;

      g_mutex_lock(ec->lock);
      while(   ec->sectorsToEncode 
	    && !ec->abortImmediately
	    && ec->nextBufferIndex >= ec->encoderLayerSectors)
      {  verbose("ENC: encoder %d waiting for work\n", my_number);
 	 g_cond_wait(ec->ioCond, ec->lock);
      }
      layer_offset = ec->nextBufferIndex;
      layer_index  = ec->encoderChunk + layer_offset;

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

	 /* CRC32 part:
	    layer ndata-2 has already been prepared by the IO thread,
	    layer ndata-1 is the CRC layer itself */
#if 0
	 if(layer < ndata-2) 
	 {  if(ec->encoderChunk || layer_offset)
	    {  if(layer_offset) /* fixme: prove correctness */
		ec->crc[512*layer_offset+layer] = Crc32(data+2048, 2048);
	    }
	    else /* store CRC for the first ecc block in ecc header */
	      ec->crcInHeader[layer] = Crc32(data, 2048);
	 }
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

      for(j=2048*enc_size; j>0; j--, idx++)
      {  unsigned char *par = par_ptr;
	 for(k=0; k<nroots; k++)
	   ec->slice[k][idx] = *par++;
	 par_ptr += nroots_aligned;
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
			     _("%5.2fMB/s current"), mbs);
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
      if(Closure->useSSE2)
	   SetLabelText(GTK_LABEL(ec->wl->encThreads), 
			_("%d threads with 128bit intrinsics"),
			Closure->codecThreads);
      else SetLabelText(GTK_LABEL(ec->wl->encThreads), 
			_("%d threads"),
			Closure->codecThreads);
      SetLabelText(GTK_LABEL(ec->wl->encPerformance), "");
      SetLabelText(GTK_LABEL(ec->wl->encBottleneck), "");
   }

   /*** Calculate buffer size for the parity calculation and image data caching. 

        The algorithm builds the parity file consecutively in chunks of 
	Closure->prefetchSectors sectors.
        We use all the amount of memory allowed by cacheMB for caching the output 
	parity blocks, and additionally 1/nroots of that memory for caching input.

	Each chunk of parity blocks is built iteratively by processing the data 
	in layers (first all bytes at pos 0, then pos 1, until ndata layers have 
	been processed).

	So we need to buffer 2048*Closure->prefetchSectors of input data.
	For practical reasons we require that the layer size is a multiple of the
	medium sector size of 2048 bytes. */

   ec->chunkBytes  = 2048*Closure->prefetchSectors;
   ec->chunkSize   = Closure->prefetchSectors;

   /*** Allocate stuff shared by all threads */

   ec->lock          = g_mutex_new();
   ec->ioCond        = g_cond_new();
   ec->sectorsToEncode = ndata*ec->lay->sectorsPerLayer;
   if(Closure->eccTarget == ECC_FILE)
      ec->writeHandle   = LargeOpen(Closure->eccName, O_RDWR, IMG_PERMS);
   else
      ec->writeHandle   = LargeOpen(Closure->imageName, O_RDWR, IMG_PERMS);
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
      ec->thread[i] =  g_thread_create((GThreadFunc)encoder_thread, (gpointer)ec, TRUE, &err);
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

void RS03Create(Method *method)
{  RS03Widgets *wl = (RS03Widgets*)method->widgetList;
   RS03Layout *lay;
   ecc_closure *ec = g_malloc0(sizeof(ecc_closure));
   ImageInfo *ii;
   EccInfo *ei;
   gdouble elapsed,mbs;
   gulong ignore;
   gint64 ecc_sectors;

   /*** Register the cleanup procedure for GUI mode */

   ec->self = method;
   ec->wl = wl;
   ec->eh = g_malloc0(sizeof(EccHeader));
   ec->earlyTermination = TRUE;

   RegisterCleanup(_("Error correction data creation aborted"), ecc_cleanup, ec);

   if(Closure->guiMode)  /* Preliminary fill text for the head line */
     SetLabelText(GTK_LABEL(wl->encHeadline),
		  _("<big>Augmenting the image with error correction data.</big>\n<i>%s</i>"), 
		  _("- checking image -"));

   /*** If the image already contains error correction information, remove it. */

   remove_old_ecc(ec);

   /*** Open image file and calculate a suitable redundancy .*/

   if(Closure->eccTarget == ECC_IMAGE)   /* augmented image */
   {  ii = ec->ii = OpenImageFile(NULL, WRITEABLE_IMAGE);
   }
   else                     /* error correction file */
   {  ii = ec->ii = OpenImageFile(NULL, READABLE_IMAGE);
      ei = ec->ei = OpenEccFile(WRITEABLE_ECC);
   }

   lay = ec->lay = CalcRS03Layout(ii->sectors, 0, Closure->eccTarget);
   lay->eh = ec->eh;

   /*** Announce what we are going to do */

   ecc_sectors = lay->nroots*lay->sectorsPerLayer;
   if(Closure->guiMode)  /* Preliminary fill text for the head line */
   {  ec->msg = g_strdup_printf(_("Encoding with Method RS03: %lld MB data, %lld MB ecc (%d roots; %4.1f%% redundancy)."),
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
   {  if(Closure->eccTarget == ECC_IMAGE)
	 ec->msg = g_strdup_printf(_("Augmenting image with Method RS03 [%d threads]:\n"
				     "%lld MB data, %lld MB ecc (%d roots; %4.1f%% redundancy)."),
				   Closure->codecThreads, lay->dataSectors/512, 
				   ecc_sectors/512, lay->nroots, lay->redundancy);
      else
	 ec->msg = g_strdup_printf(_("Creating the error correction file with Method RS03 [%d threads]:\n"
				     "%lld MB data, %lld MB ecc (%d roots; %4.1f%% redundancy)."),
				   Closure->codecThreads, lay->dataSectors/512, 
				   ecc_sectors/512, lay->nroots, lay->redundancy);

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
		"New image size is %lld MB (%lld sectors).\n"),
	      (lay->dataSectors+lay->dataPadding+ecc_sectors)/512,
	      lay->dataSectors+lay->dataPadding+ecc_sectors);
   else
     PrintLog(_("Error correction file \"%s\" created.\n"
		"Make sure to keep this file on a reliable medium.\n"),
	      Closure->eccName);

   elapsed=g_timer_elapsed(ec->avgTimer, &ignore);
   mbs = ((double)lay->ndata*lay->sectorsPerLayer)/(512.0*elapsed);
   PrintLog(_("Avg performance: %5.2fs (%5.2fMB/s) total\n"), 
	    elapsed, mbs);
   if(Closure->guiMode)
   {  SetLabelText(GTK_LABEL(wl->encPerformance), _("%5.2fMB/s average"), mbs);
      SetLabelText(GTK_LABEL(ec->wl->encBottleneck), 
		   _("%d times CPU bound; %d times I/O bound"),
		   ec->cpuBound, ec->ioBound);
   }

   if(Closure->guiMode)
   {  SetProgress(wl->encPBar2, 100, 100);

      if(Closure->eccTarget == ECC_IMAGE)
	SetLabelText(GTK_LABEL(wl->encFootline),
		     _("Image has been augmented with error correction data.\n"
		       "New image size is %lld MB (%lld sectors).\n"),
		     (lay->dataSectors + ecc_sectors)/512,
		     lay->dataSectors+ecc_sectors);
      else
	SetLabelText(GTK_LABEL(wl->encFootline), 
		     _("The error correction file has been successfully created.\n"
		       "Make sure to keep this file on a reliable medium.")); 

   }

   /*** Clean up */

   ec->earlyTermination = FALSE;
   ecc_cleanup((gpointer)ec);
}

