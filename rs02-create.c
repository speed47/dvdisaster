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

/***
 *** Local data package used during encoding
 ***/

typedef struct
{  Image *image;
   Method *self;
   RS02Widgets *wl;
   RS02Layout *lay;
   GaloisTables *gt;
   ReedSolomonTables *rt;
   EccHeader *eh;
   unsigned char *data;
   unsigned char *parity;
   unsigned char *slice[256];
   struct MD5Context md5Ctxt[256];
   guint8 md5Sum[16*256];
   guint8 eccSum[16];
   char *msg;
   int earlyTermination;
   GTimer *timer;
} ecc_closure;

static void ecc_cleanup(gpointer data)
{  ecc_closure *ec = (ecc_closure*)data;
   int i;

   UnregisterCleanup();

   if(Closure->guiMode)
   {  if(ec->earlyTermination && ec->wl)
        SetLabelText(GTK_LABEL(ec->wl->encFootline),
		     _("<span %s>Aborted by unrecoverable error.</span>"),
		     Closure->redMarkup); 
      AllowActions(TRUE);
   }

   /*** We must invalidate the CRC cache as it does only cover the
	data portion of the image, not the full RS02 enhanced image. */

   if(Closure->crcCache)
     ClearCrcCache();

   /*** Clean up */

   if(ec->image) CloseImage(ec->image);
   if(ec->gt) FreeGaloisTables(ec->gt);
   if(ec->rt) FreeReedSolomonTables(ec->rt);
   if(ec->eh) g_free(ec->eh);
   if(ec->lay) g_free(ec->lay);
   if(ec->data) g_free(ec->data);
   if(ec->parity) g_free(ec->parity);
   if(ec->msg) g_free(ec->msg);
   if(ec->timer) g_timer_destroy(ec->timer);

   for(i=0; i<256; i++)
     if(ec->slice[i])
       g_free(ec->slice[i]);

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
{  RS02Widgets *wl = ec->wl;

   if(truncate && ec->lay)
   {  if(!LargeTruncate(ec->image->file, (gint64)(2048*ec->lay->dataSectors)))
	Stop(_("Could not truncate %s: %s\n"),Closure->imageName,strerror(errno));

      if(Closure->stopActions == STOP_CURRENT_ACTION)
	 SetLabelText(GTK_LABEL(wl->encFootline), 
		      _("<span %s>Aborted by user request!</span> (partial ecc data removed from image)"),
		      Closure->redMarkup); 
   }
   else
   {  if(Closure->stopActions == STOP_CURRENT_ACTION)
	 SetLabelText(GTK_LABEL(wl->encFootline), 
		      _("<span %s>Aborted by user request!</span>"),
		      Closure->redMarkup); 
   }

   ec->earlyTermination = FALSE;   /* suppress respective error message */

   ecc_cleanup((gpointer)ec);
}


/*
 * Remove already existing RS02 ecc data from the image.
 */

static void remove_old_ecc(ecc_closure *ec)
{  
   if(ec->image->eccHeader)
   {  gint64 data_sectors = uchar_to_gint64(ec->image->eccHeader->sectors);
      guint64 data_bytes;
      int answer;

      if(Closure->confirmDeletion  || !Closure->guiMode)
	answer = ModalWarning(GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL, NULL,
			      _("Image \"%s\" already contains error correction information.\n"
				"Truncating image to data part (%lld sectors).\n"),
			      Closure->imageName, data_sectors);
      else answer = TRUE;

      if(!answer)
	abort_encoding(ec, FALSE);

      if(ec->image->eccHeader->inLast != 2048)
	   data_bytes = (guint64)(2048*(data_sectors-1)+ec->image->eccHeader->inLast);
      else data_bytes = (guint64)(2048*data_sectors);

      if(!TruncateImage(ec->image, data_bytes))
	Stop(_("Could not truncate %s: %s\n"),Closure->imageName,strerror(errno));

      PrintLog(_("Image size is now"));
      if(ec->image->inLast == 2048)
           PrintLog(_(": %lld medium sectors.\n"), ec->image->sectorSize);
      else PrintLog(_(": %lld medium sectors and %d bytes.\n"), 
		   ec->image->sectorSize-1, ec->image->inLast);
   }
}

/*
 * Check the image for completeness and calculate the CRC sums
 * if the respective data has not already been supplied by ReadLinear() 
*/

static void check_image(ecc_closure *ec)
{  struct MD5Context image_md5;
   RS02Layout *lay = ec->lay;
   Image *image = ec->image;
   gint64 sectors;
   guint32 *crcptr;
   int last_percent, percent;

   /* Discard old CRC cache no matter what it contains.
    * We will create a new one a few lines below.
    * Note that it is very unusual to augment an image with ecc data
    * which was just read from an actual medium, so optimizing 
    * for the cached CRCs is not necessary. 
    */

   if(Closure->crcCache) 
     ClearCrcCache();    

   last_percent = 0;
   MD5Init(&image_md5);
   
   Closure->crcCache = crcptr = g_malloc(sizeof(guint32) * lay->dataSectors);

   if(!LargeSeek(image->file, 0))
     Stop(_("Failed seeking to start of image: %s\n"), strerror(errno));

   for(sectors = 0; sectors < lay->dataSectors; sectors++)
   {  unsigned char buf[2048];
      int expected,n,err;

      if(Closure->stopActions) /* User hit the Stop button */
	abort_encoding(ec, FALSE);

      if(sectors < image->sectorSize-1) expected = 2048;
      else  
      {  memset(buf, 0, 2048);
	 expected = image->inLast;
      }

      n = LargeRead(image->file, buf, expected);
      if(n != expected)
	Stop(_("Failed reading sector %lld in image: %s"),sectors,strerror(errno));

      /* Look for the dead sector marker */

      err = CheckForMissingSector(buf, sectors, image->fpState == FP_PRESENT ? image->imageFP : NULL, FINGERPRINT_SECTOR);
      if(err != SECTOR_PRESENT)
      {    if(err == SECTOR_MISSING)
	    Stop(_("Image contains unread(able) sectors.\n"
		   "Error correction information can only be\n"
		   "appended to complete (undamaged) images.\n"));
	 else
	    Stop(_("Sector %lld in the image is marked unreadable\n"
		   "and seems to come from a different medium.\n\n"
		   "The image was probably mastered from defective content.\n"
		   "For example it might contain one or more files which came\n"
		   "from a damaged medium which was NOT fully recovered.\n" 
		   "This means that some files may have been silently corrupted.\n\n"
		   "Error correction information can only be\n"
		   "appended to complete (undamaged) images.\n"));
      }
      
      /* Update and cache the CRC sums */

      *crcptr++ = Crc32(buf, 2048);
      MD5Update(&image_md5, buf, n);

      percent = (100*sectors)/(lay->eccSectors + lay->dataSectors);

      if(last_percent != percent) 
      {  PrintProgress(_("Preparing image (checksums, adding space): %3d%%") ,percent);

	 if(Closure->guiMode)
	   SetProgress(ec->wl->encPBar1, percent, 100);
	   
	 last_percent = percent;
      }
   }

   MD5Final(image->mediumSum, &image_md5);
}


/*
 * Expand the image by lay->eccSectors.
 * This avoids horrible file fragmentation under some file systems. 
 */

static void expand_image(ecc_closure *ec)
{  RS02Layout *lay = ec->lay;
   Image *image = ec->image;
   int last_percent, percent;
   gint64 sectors;

   /* If the file does not end at a sector boundary,
      fill it up with zeros. */

   if(image->inLast != 2048)
   {  int fill = 2048 - image->inLast;
      int n;
      unsigned char zeros[fill];

      memset(zeros, 0, fill);

      if(!LargeSeek(image->file, image->file->size))
	Stop(_("Failed seeking to end of image: %s\n"), strerror(errno));

      n = LargeWrite(image->file, zeros, fill);
      if(n != fill)
	Stop(_("Failed expanding the image: %s\n"), strerror(errno));
   }

   /* Now add the sectors needed for the ecc data */

   if(!LargeSeek(image->file, 2048*lay->dataSectors))
     Stop(_("Failed seeking to end of image: %s\n"), strerror(errno));

   last_percent = 0;
   for(sectors = 0; sectors < lay->eccSectors; sectors++)
   {  unsigned char buf[2048];
      int n;

      if(Closure->stopActions) /* User hit the Stop button */
	abort_encoding(ec, TRUE);

      CreateMissingSector(buf, lay->dataSectors+sectors, 
			  image->imageFP, FINGERPRINT_SECTOR, 
			  "RS02 generation placeholder");
      n = LargeWrite(image->file, buf, 2048);
      if(n != 2048)
	Stop(_("Failed expanding the image: %s\n"), strerror(errno));

      percent = (100*(sectors+lay->dataSectors)) / (lay->eccSectors + lay->dataSectors);
      if(last_percent != percent)
      {  PrintProgress(_("Preparing image (checksums, adding space): %3d%%"), percent);

	 if(Closure->guiMode)
	   SetProgress(ec->wl->encPBar1, percent, 100);

	 last_percent = percent; 
      }
   }

   PrintProgress(_("Preparing image (checksums, adding space): %3d%%"), 100);
   PrintProgress("\n");

   if(Closure->guiMode)
     SetProgress(ec->wl->encPBar1, 100, 100);
}

/*
 * Write the RS02 CRC32 sums into the image file 
 */

static void write_crc(ecc_closure *ec)
{  RS02Layout *lay = ec->lay;
   Image *image = ec->image;
   EccHeader *eh = ec->eh;
   gint64 crc_sector;
   gint64 layer_sector;
   gint64 layer_offset;
   guint32 crc_buf[512], *crc_boot_ptr;
   struct MD5Context md5ctxt;
   int crc_idx,i;
   int writepos=0;
   layer_offset = lay->firstCrcLayerIndex + 1;
   crc_sector   = lay->dataSectors + 2;
   crc_idx = 0;

   /*** A copy of the CRCs for the lay->firstCrcLayerIndex ecc block
	is copied into the EccHeader starting with byte position 2048. */

   crc_boot_ptr = (guint32*)((char*)eh + 2048);
   MD5Init(&md5ctxt);

   /*** Calculate the CRCs */

   if(!LargeSeek(image->file, 2048*crc_sector))
     Stop(_("Failed seeking to sector %lld in image: %s"), crc_sector, strerror(errno));

   for(layer_sector=0; layer_sector<lay->sectorsPerLayer; layer_sector++)
   {  gint64 layer_index = (layer_sector + layer_offset) % lay->sectorsPerLayer;

      /* Write CRC sums for layer_index'th slice.
         Some ecc blocks contain padding sectors >= lay->dataSectors. 
         CRCs for padding sectors are not written out,
         so we have to keep in mind that there might be <= ndata CRC sums
         per ecc blocks. */

      for(i=0; i<lay->ndata; i++)
      {  
	 if(layer_index < lay->dataSectors)
	 {  crc_buf[crc_idx++] = Closure->crcCache[layer_index];
 
	    if(layer_sector == lay->sectorsPerLayer - 1)
	      *crc_boot_ptr++ = Closure->crcCache[layer_index];

            if(crc_idx >= 512)
	    {  int n = LargeWrite(image->file, crc_buf, 2048);

	       if(n != 2048)
		 Stop(_("Failed writing to sector %lld in image: %s"), crc_sector, strerror(errno));
	       MD5Update(&md5ctxt, (unsigned char*)crc_buf, n);

	       crc_sector++;
	       crc_idx = 0;
	    }
	    writepos++;
	    layer_index += lay->sectorsPerLayer;
	 }
      }
   }

   /* flush last CRC sector */

   if(crc_idx)
   {  int n; 

      for(n=crc_idx; n<512; n++) /* pad unused portion of CRC buffer */
#ifdef HAVE_BIG_ENDIAN
	crc_buf[n] = 0x47504c00;
#else
	crc_buf[n] = 0x4c5047;
#endif
      n = LargeWrite(image->file, crc_buf, 2048);

      if(n != 2048)
	Stop(_("Failed writing to sector %lld in image: %s"), crc_sector, strerror(errno));

      MD5Update(&md5ctxt, (unsigned char*)crc_buf, n);
   }

   /* finish and store the md5sum */

   MD5Final(eh->crcSum, &md5ctxt);
}

/*
 * Fill in the necessary values for the EccHeader.
 * Note that a copy of the CRC sums for ecc block lay->firstCrcLayerIndex + 1
 * has been put at byte pos 2048 into the Eccheader by the previous function.
 */

static void prepare_header(ecc_closure *ec)
{  Image *image = ec->image;
   EccHeader *eh = ec->eh;
   RS02Layout *lay = ec->lay;

   memcpy(eh->cookie, "*dvdisaster*", 12);
   memcpy(eh->method, "RS02", 4);
   eh->methodFlags[0]  = 0;
   memcpy(eh->mediumFP, image->imageFP, 16);
   memcpy(eh->mediumSum, image->mediumSum, 16);
   memcpy(eh->eccSum, ec->eccSum, 16);
   gint64_to_uchar(eh->sectors, image->sectorSize);
   eh->dataBytes       = lay->ndata;
   eh->eccBytes        = lay->nroots;

   eh->creatorVersion  = Closure->version;
   eh->neededVersion   = 6600;
   eh->fpSector        = FINGERPRINT_SECTOR;
   eh->inLast          = image->inLast;
   eh->sectorsAddedByEcc = lay->eccSectors;

   eh->selfCRC = 0x4c5047;

#ifdef HAVE_BIG_ENDIAN
   SwapEccHeaderBytes(eh);
   eh->selfCRC = 0x47504c00;
#endif

   eh->selfCRC = Crc32((unsigned char*)eh, sizeof(EccHeader));
}

/*
 * Calculate the Reed-Solomon error correction code
 */

static void create_reed_solomon(ecc_closure *ec)
{  RS02Layout *lay = ec->lay;
   Image *image = ec->image;
   int nroots = lay->nroots;
   int ndata  = lay->ndata;
   gint64 b_idx, block_idx[256]; 
   guint64 n_parity_blocks,n_layer_sectors;
   guint64 n_parity_bytes,n_layer_bytes;
   guint64 si,chunk;
   int last_percent, percent, max_percent, progress;
   int layer,i,j,k;
   unsigned char *par_ptr;
   int out_of_memory = 0;
static gint32 *gf_index_of;    /* These need to be static globals */
static gint32 *rs_gpoly;       /* for optimization reasons. */
static gint32 *enc_alpha_to;

   /*** Show the second progress bar */

   if(Closure->guiMode)
   {  ShowWidget(ec->wl->encPBar2);
      ShowWidget(ec->wl->encLabel2);
   }

   /*** Adjust image bounds to include the CRC sectors */

   image->sectorSize = lay->protectedSectors;

   /*** Create table for Galois field math */

   ec->gt = CreateGaloisTables(RS_GENERATOR_POLY);
   ec->rt = CreateReedSolomonTables(ec->gt, RS_FIRST_ROOT, RS_PRIM_ELEM, nroots);

   gf_index_of  = ec->gt->indexOf;
   enc_alpha_to = ec->gt->encAlphaTo;
   rs_gpoly     = ec->rt->gpoly;

   /*** Allocate buffers for the parity calculation and image data caching. 

        The algorithm builds the parity file consecutively in chunks of n_parity_blocks.
        We use all the amount of memory allowed by cacheMiB for caching the parity blocks. */

   n_parity_blocks = ((guint64)Closure->cacheMiB<<20) / (guint64)nroots;  /* 1 MiB = 2^20 */
   n_parity_blocks >>= 1;                              /* two buffer sets for scrambling */
   n_parity_blocks &= ~0x7ff;                          /* round down to multiple of 2048 */
   n_parity_bytes  = (guint64)nroots * n_parity_blocks;

   /* Each chunk of parity blocks is built iteratively by processing the data in layers
      (first all bytes at pos 0, then pos 1, until ndata layers have been processed).
      So we need to buffer n_layer_bytes = n_parity_blocks of input data.
      For practical reasons we require that the layer size is a multiple of the
      medium sector size of 2048 bytes. */

   n_layer_bytes   = n_parity_blocks;
   n_layer_sectors = n_parity_blocks/2048;

   if(n_layer_sectors*2048 != n_parity_blocks)
     Stop("Internal error: parity blocks are not a multiple of sector size.\n");

   ec->parity = g_try_malloc(n_parity_bytes);
   ec->data   = g_try_malloc(n_layer_bytes);

   /*** Create buffers for dividing the ecc information into nroots slices */

   for(i=0; i<nroots; i++)
   {  ec->slice[i] = g_try_malloc(n_layer_bytes);
      if(!ec->slice[i])
	 out_of_memory = 1;
   }

   if(out_of_memory || !ec->parity || !ec->data)
   {  LargeTruncate(image->file, (gint64)(2048*ec->lay->dataSectors));
      Stop(_("Failed allocating memory for I/O cache.\n"
	     "Cache size is currently %d MiB.\n"
	     "Try reducing it.\n"),
	   Closure->cacheMiB);
   }

   /*** Setup the block counters for mapping medium sectors to ecc blocks 
        The image is divided into ndata layers;
        with each layer spanning s lay->sectorsPerLayer sectors. */

   for(b_idx=0, i=0; i<ndata; b_idx+=lay->sectorsPerLayer, i++)
     block_idx[i] = b_idx;

   /*** Initialize md5 contexts for checksumming the nroots slices */

   for(i=0; i<nroots; i++)
      MD5Init(&ec->md5Ctxt[i]);

   /*** Create ecc information for the protected sectors portion of the image. */ 

   max_percent = ndata * ((lay->sectorsPerLayer / n_layer_sectors) + 1);
   progress = percent = 0;
   last_percent = -1;
   g_timer_start(ec->timer);

   /* Process the image.
      From each layer a chunk of n_layer_sectors is read in at once.
      So after (lay->sectorsPerLayer/n_layer_sectors)+1 iterations 
      the whole image has been processed. */

   for(chunk=0; chunk<lay->sectorsPerLayer; chunk+=n_layer_sectors) 
   {  guint64 actual_layer_bytes,actual_layer_sectors;
      int sp;

      /* Prepare the parity data for the next chunk. */

      memset(ec->parity, 0, n_parity_bytes);

      /* The last chunk may contain fewer sectors. */

      if(chunk+n_layer_sectors < lay->sectorsPerLayer)
           actual_layer_sectors = n_layer_sectors;
      else actual_layer_sectors = lay->sectorsPerLayer-chunk;

      actual_layer_bytes   = 2048*actual_layer_sectors;

      /* Work each of the ndata data layers 
	 into the parity data of the current chunk. */

      sp = nroots - ndata % nroots;  /* => (ndata + sp) mod nroots = 0 so that parity */
	                             /* is aligned at sp=0 after ndata iterations */
      if(sp==nroots) sp=0;

      for(layer=0; layer<ndata; layer++)
      {  int offset = 0;
         unsigned char *par_idx = ec->parity;

	 if(Closure->stopActions) /* User hit the Stop button */
	   abort_encoding(ec, TRUE);

         /* Read the next data sectors of this layer. */

   	 for(si=0; si<actual_layer_sectors; si++)
	 {  RS02ReadSector(image, lay, ec->data+offset, block_idx[layer]);
	    block_idx[layer]++;
	    offset += 2048;
	 }

	 /* Now process the data bytes of the current layer. */

	 for(si=0; si<actual_layer_bytes; si++)
	 {  register int feedback;

	    feedback = gf_index_of[ec->data[si] ^ par_idx[sp]];

	    if(feedback != GF_ALPHA0) /* non-zero feedback term */
	    {  register int spk = sp+1;
	       register int *gpoly = rs_gpoly + nroots;

	       switch(nroots-spk)  /* unrolled loop part1 */
	       {  
	          case 170: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 169: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 168: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 167: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 166: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 165: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 164: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 163: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 162: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 161: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 160: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 159: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 158: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 157: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 156: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 155: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 154: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 153: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 152: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 151: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 150: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 149: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 148: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 147: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 146: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 145: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 144: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 143: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 142: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 141: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 140: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 139: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 138: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 137: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 136: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 135: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 134: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 133: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 132: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 131: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 130: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 129: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 128: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 127: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 126: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 125: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 124: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 123: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 122: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 121: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 120: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 119: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 118: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 117: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 116: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 115: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 114: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 113: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 112: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 111: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 110: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 109: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 108: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 107: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 106: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 105: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 104: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 103: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 102: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 101: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 100: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 99: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 98: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 97: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 96: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 95: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 94: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 93: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 92: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 91: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 90: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 89: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 88: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 87: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 86: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 85: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 84: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 83: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 82: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 81: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 80: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 79: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 78: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 77: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 76: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 75: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 74: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 73: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 72: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 71: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 70: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 69: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 68: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 67: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 66: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 65: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 64: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 63: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 62: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 61: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 60: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 59: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 58: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 57: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 56: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 55: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 54: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 53: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 52: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 51: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 50: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 49: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 48: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 47: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 46: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 45: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 44: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 43: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 42: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 41: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 40: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 39: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 38: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 37: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 36: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 35: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 34: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 33: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 32: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 31: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 30: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 29: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 28: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 27: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 26: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 25: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 24: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 23: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 22: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 21: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 20: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 19: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 18: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 17: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 16: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 15: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 14: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 13: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 12: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 11: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 10: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case  9: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case  8: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case  7: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case  6: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case  5: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case  4: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case  3: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case  2: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case  1: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	       }

	       spk = 0;
		  
	       switch(sp)  /* unrolled loop part2 */
	       {
	          case 170: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 169: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 168: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 167: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 166: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 165: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 164: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 163: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 162: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 161: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 160: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 159: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 158: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 157: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 156: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 155: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 154: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 153: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 152: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 151: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 150: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 149: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 148: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 147: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 146: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 145: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 144: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 143: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 142: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 141: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 140: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 139: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 138: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 137: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 136: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 135: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 134: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 133: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 132: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 131: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 130: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 129: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 128: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 127: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 126: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 125: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 124: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 123: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 122: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 121: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 120: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 119: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	          case 118: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 117: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 116: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 115: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 114: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 113: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 112: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 111: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
                  case 110: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 109: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 108: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 107: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 106: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 105: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 104: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 103: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 102: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 101: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		  case 100: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 99: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 98: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 97: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 96: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 95: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 94: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 93: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 92: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 91: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 90: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 89: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 88: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 87: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 86: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 85: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 84: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 83: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 82: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 81: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 80: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 79: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 78: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 77: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 76: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 75: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 74: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 73: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 72: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 71: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 70: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 69: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 68: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 67: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 66: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 65: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 64: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 63: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 62: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 61: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 60: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 59: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 58: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 57: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 56: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 55: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 54: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 53: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 52: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 51: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 50: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 49: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 48: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 47: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 46: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 45: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 44: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 43: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 42: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 41: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 40: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 39: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 38: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 37: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 36: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 35: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 34: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 33: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 32: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 31: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 30: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 29: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 28: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 27: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 26: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 25: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 24: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 23: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 22: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 21: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 20: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 19: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 18: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 17: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 16: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 15: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 14: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 13: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 12: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 11: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case 10: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case  9: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case  8: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case  7: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case  6: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case  5: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case  4: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case  3: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case  2: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
		   case  1: par_idx[spk++] ^= enc_alpha_to[feedback + *--gpoly];
	       }

	       par_idx[sp] = enc_alpha_to[feedback + rs_gpoly[0]];
	    }
	    else                   /* zero feedback term */
	      par_idx[sp] = 0;

	    par_idx += nroots;
	 }

	 if(++sp>=nroots) sp=0;   /* shift */

	 /* Report progress */

	 progress++;
	 percent = (1000*progress)/max_percent;
	 if(last_percent != percent) 
	 {
	     if(Closure->guiMode)
	          SetProgress(ec->wl->encPBar2, percent, 1000);
	     else PrintProgress(_("Ecc generation: %3d.%1d%%"), percent/10, percent%10);

	    last_percent = percent;
	 }
      }

      /* The parity bytes have been prepared as sequences of nroots bytes for each 
	 ecc block. Now we split them up into nroots slices and write them out. */

      par_ptr = ec->parity;

      for(si=0; si<actual_layer_sectors; si++)
      {  guint64 idx = 2048*si;

	 for(j=0; j<2048; j++, idx++)
	 {  for(k=0; k<nroots; k++)
	      ec->slice[k][idx] = *par_ptr++;
	 }
      }

      for(k=0; k<nroots; k++)
      {  int idx=0;

	for(si=0; si<actual_layer_sectors; si++, idx+=2048)
	 {  gint64 s = RS02EccSectorIndex(lay, k, chunk + si);

	    if(!LargeSeek(image->file, 2048*s))
	      Stop(_("Failed seeking to sector %lld in image: %s"), s, strerror(errno));

	    if(LargeWrite(image->file, ec->slice[k]+idx, 2048) != 2048)
	      Stop(_("Failed writing to sector %lld in image: %s"), s, strerror(errno));

	    MD5Update(&ec->md5Ctxt[k], ec->slice[k]+idx, 2048);
	}
      }
   }

   /*** We can store only one md5sum in the header,
	so lets produce a meta-checksum from all nroots md5sums */

   for(i=0; i<nroots; i++)
     MD5Final(&ec->md5Sum[i*16], &ec->md5Ctxt[i]);

   MD5Init(&ec->md5Ctxt[0]);
   MD5Update(&ec->md5Ctxt[0], ec->md5Sum, 16*nroots);
   MD5Final(ec->eccSum, &ec->md5Ctxt[0]);

   /*** Restore image bounds to data portion */

   image->sectorSize = lay->dataSectors;
}

/***
 *** Append the parity information to the image
 ***/

void RS02Create(void)
{  Method *self = FindMethod("RS02");
   RS02Widgets *wl = (RS02Widgets*)self->widgetList;
   Image *image = NULL;
   RS02Layout *lay;
   ecc_closure *ec = g_malloc0(sizeof(ecc_closure));

   ec->earlyTermination = TRUE;
   RegisterCleanup(_("Error correction data creation aborted"), ecc_cleanup, ec);

   /*** Open image file */

   PrintLog(_("\nOpening %s"), Closure->imageName);

   image = OpenImageFromFile(Closure->imageName, O_RDWR, IMG_PERMS);
   if(!image)
   {  PrintLog(": %s.\n", strerror(errno));
      Stop(_("Image file %s: %s."),Closure->imageName, strerror(errno));
      return;
   }

   if(image->inLast == 2048)
        PrintLog(_(": %lld medium sectors.\n"), image->sectorSize);
   else PrintLog(_(": %lld medium sectors and %d bytes.\n"), 
		   image->sectorSize-1, image->inLast);

   /*** Register the cleanup procedure for GUI mode */

   ec->image = image;
   ec->self = self;
   ec->wl = wl;
   ec->eh = g_malloc0(sizeof(EccHeader));
   ec->timer   = g_timer_new();

   if(Closure->guiMode)  /* Preliminary fill text for the head line */
     SetLabelText(GTK_LABEL(wl->encHeadline),
		  _("<big>Augmenting the image with error correction data.</big>\n<i>%s</i>"), 
		  _("- checking image -"));

   /*** If the image already contains error correction information, remove it. */

   remove_old_ecc(ec);

   /*** Calculate a suitable redundancy .*/

   lay = ec->lay = CalcRS02Layout(image);

   /*** Announce what we are going to do */

   if(Closure->guiMode)  /* Preliminary fill text for the head line */
   {  ec->msg = g_strdup_printf(_("Encoding with Method RS02: %lld MiB data, %lld MiB ecc (%d roots; %4.1f%% redundancy)."),
				lay->dataSectors/512, lay->eccSectors/512, lay->nroots, lay->redundancy);

      SetLabelText(GTK_LABEL(wl->encHeadline),
		   _("<big>Augmenting the image with error correction data.</big>\n<i>%s</i>"), 
		   ec->msg);
   }
   else
   {  ec->msg = g_strdup_printf(_("Augmenting image with Method RS02:\n %lld MiB data, %lld MiB ecc (%d roots; %4.1f%% redundancy)."),
				 lay->dataSectors/512, lay->eccSectors/512, lay->nroots, lay->redundancy);

      PrintLog("%s\n",ec->msg);
   }

   /*** Warn if there is not enough space for ecc data */

   if(lay->nroots < 8)
     Stop(_("Not enough space on medium left for error correction data.\n"
	    "Data portion of image: %lld sect.; maximum possible size: %lld sect.\n"
	    "If reducing the image size or using a larger medium is\n"
	    "not an option, please create a separate error correction file."),
	  lay->dataSectors, lay->mediumCapacity);

   if(lay->redundancy < 20)
   {  int answer;

      answer = ModalWarning(GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL, NULL,
			    _("Using redundancies below 20%%%% may not give\n"
			      "the expected data loss protection.\n"));

      if(!answer)
	abort_encoding(ec, FALSE);
   }

   /*** Check image for completeness and fetch its CRC sums */

   check_image(ec);

   /*** Expand the image by lay->eccSectors. */

   expand_image(ec);

   /*** Distribute and write the CRC sums */

   write_crc(ec);

   /*** Create the Reed-Solomon parts of the ecc section */

   create_reed_solomon(ec);

   /*** Prepare the Ecc header 
	and write all copies of the header out */

   prepare_header(ec);
   WriteRS02Headers(image->file, ec->lay, ec->eh);

   PrintTimeToLog(ec->timer, "for ECC generation.\n");

   PrintProgress(_("Ecc generation: 100.0%%\n"));
   PrintLog(_("Image has been augmented with error correction data.\n"
	      "New image size is %lld MiB (%lld sectors).\n"),
	    (lay->dataSectors + lay->eccSectors)/512,
	    lay->dataSectors+lay->eccSectors);
   
   if(Closure->guiMode)
   {  SetProgress(wl->encPBar2, 100, 100);

      SetLabelText(GTK_LABEL(wl->encFootline), 
		   _("Image has been augmented with error correction data.\n"
		     "New image size is %lld MiB (%lld sectors).\n"),
		   (lay->dataSectors + lay->eccSectors)/512,
		   lay->dataSectors+lay->eccSectors);
   }

   /*** Clean up */

   ec->earlyTermination = FALSE;
   ecc_cleanup((gpointer)ec);
}

