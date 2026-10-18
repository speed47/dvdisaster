/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2004-2017 Carsten Gnoerlich.
 *  Copyright (C) 2019-2021 The dvdisaster development team.
 *
 *  The Reed-Solomon error correction draws a lot of inspiration - and even code -
 *  from Phil Karn's excellent Reed-Solomon library: http://www.ka9q.net/code/fec/
 *
 *  Email: support@dvdisaster.org
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

/*** src type: some GUI code ***/

#include "dvdisaster.h"

#include "rs01-includes.h"

/***
 *** Interpret our redundancy settings
 ***
 * a) "normal" -> nroots=32; "high" -> nroots=64
 * b) "n"      -> nroots = n
 * c) "n%"     -> use closest nroots for given redundancy in percent
 * d) "nm"     -> choose redundancy so that .ecc file does not exceed n megabytes
 */

static guint64 ecc_file_size(guint64 sectors, int nr)
{  int nd = GF_FIELDMAX - nr;
   guint64 bytesize; 

   bytesize = 4096 + 4*sectors + 2048*nr*((sectors+nd-1)/nd);

   return (bytesize+0xfffff)/0x100000;   /* size in MiB */
}		 

static int calculate_redundancy(char *image_name)
{  int nr = 0;
   char last = 0; 
   double p;
   int ignore;
   guint64 fs,sectors,filesize;

   if(Closure->redundancy) /* get last char of redundancy parameter */
   {  int len = strlen(Closure->redundancy);

      if(len) last = Closure->redundancy[len-1];
   }

   switch(last)
   {  case '%' : p = atof(Closure->redundancy);
                 if(p<3.2 || p>64.5) Stop(_("Redundancy %4.1f%% out of useful range [3.2%%..64.5%%]"),p);
		 nr = (int)round((GF_FIELDMAX*p) / (100.0+p));
	         break;

      case 'r' : nr = atoi(Closure->redundancy);
	         break;

      case 'm' : if(!LargeStat(image_name, &filesize))
  	         {  nr = 32;   /* If the image file is not present, simply return 32. */
		    break;     /* Later stages will fail anyways, but can report the error */
	         }             /* in a more meaningful context. */

	         CalcSectors(filesize, &sectors, &ignore);

	         fs = strtoll(Closure->redundancy, NULL, 10);
	         if(fs < ecc_file_size(sectors, 8) || fs > ecc_file_size(sectors, 100))
		   Stop(_("Ecc file size %" PRId64 "m out of useful range [%" PRId64 " .. %" PRId64 "]"),
			fs, ecc_file_size(sectors, 8), ecc_file_size(sectors, 100));
		 for(nr=100; nr>8; nr--)
		   if(fs >= ecc_file_size(sectors, nr))
		     break;
	         break;

      default:
	if(!Closure->redundancy || !strcmp(Closure->redundancy, "normal")) nr = 32;
	else if(!strcmp(Closure->redundancy, "high")) nr = 64;
	else Stop(_("Invalid redundancy specification '%s'.\n"
		    "Valid formats: normal, high, <number>%%, <number>r, <number>m"),
		  Closure->redundancy);
	break;
   }

   if(nr < 8 || nr > 100)
     Stop(_("Redundancy %d out of useful range [8..100]."),nr);

   return nr;
}

/***
 *** Remove the image file 
 ***/

static void unlink_image(GtkWidget *label)
{
   if(!g_unlink(Closure->imageName))
   {  PrintLog(_("\nImage file %s deleted.\n"),Closure->imageName);
      GuiSetLabelText(label, _("\nImage file %s deleted.\n"), Closure->imageName);
   }
   else 
   {  if(!Closure->guiMode)
         PrintLog("\n");
      else
	 ModalWarning(GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, NULL,
		      _("Image file %s not deleted: %s\n"),
		      Closure->imageName, strerror(errno));
   }
}

/***
 *** Create parity information for the medium sectors.
 ***/

/*
 * Local data package used during encoding
 */

typedef struct
{  Method *self;
   RS01Widgets *wl;
   GaloisTables *gt;
   ReedSolomonTables *rt;
   Image *image;
   int earlyTermination;
   unsigned char *data;
   unsigned char *parity;
   char *msg;
   GTimer *timer;
} ecc_closure;

static void ecc_cleanup(gpointer data)
{  ecc_closure *ec = (ecc_closure*)data;

   UnregisterCleanup();

   if(Closure->guiMode)
   {  if(ec->earlyTermination)
      {  GuiSetLabelText(ec->wl->encFootline,
			 _("<span %s>Aborted by unrecoverable error.</span>"),
			 Closure->redMarkup);
      }
      GuiAllowActions(TRUE);
   }
   
   /** Clean up */

   if(ec->gt) FreeGaloisTables(ec->gt);
   if(ec->rt) FreeReedSolomonTables(ec->rt);
   if(ec->data) g_free(ec->data);
   if(ec->parity) g_free(ec->parity);

   if(ec->image) CloseImage(ec->image);
   if(ec->msg)   g_free(ec->msg);
   if(ec->timer) g_timer_destroy(ec->timer);

#ifdef WITH_GUI_YES
   if(Closure->enableCurveSwitch)
   {  Closure->enableCurveSwitch = FALSE;
      RS01ShowCurveButton(ec->self);
   }
#endif
   
   g_free(ec);

   if(Closure->guiMode)
      g_thread_exit(0);
}

/*
 * Create the parity file.
 */


void RS01Create(void)
{  Method *self = FindMethod("RS01");
   RS01Widgets *wl = (RS01Widgets*)self->widgetList;
   GaloisTables *gt;
   ReedSolomonTables *rt;
   ecc_closure *ec = g_malloc0(sizeof(ecc_closure));
   struct MD5Context md5Ctxt;
   EccHeader *eh;
   Image *image;
   guint64 block_idx[256];  /* must be >= ndata */
   guint64 s,si,n;
   int i;
   int percent = 0,max_percent,progress = 0, last_percent = -1;
   guint64 n_parity_blocks,n_layer_sectors;
   guint64 n_parity_bytes,n_layer_bytes;
   guint64 chunk;
   int layer;
   gint32 nroots;         /* These are copied to increase performance. */
   gint32 ndata;
   gint32 nroots_aligned;

   /*** Register the cleanup procedure for GUI mode */

   ec->self = self;
   ec->wl = wl;
   ec->earlyTermination = TRUE;
   RegisterCleanup(_("Error correction file creation aborted"), ecc_cleanup, ec);

   /*** Set up the Galois field arithmetic */

   /* Calculate number of roots (= max. number of erasures)
      and number of data bytes from redundancy setting */

   i  = calculate_redundancy(Closure->imageName);
   gt = ec->gt = CreateGaloisTables(RS_GENERATOR_POLY);
   rt = ec->rt = CreateReedSolomonTables(gt, RS_FIRST_ROOT, RS_PRIM_ELEM, i);

   nroots       = rt->nroots;
   ndata        = rt->ndata;
   nroots_aligned = (nroots+15)&~15;  /* parity stride used by EncodeNextLayer() */

   /*** Announce what we are going to do */

   ec->msg = g_strdup_printf(_("Encoding with Method RS01: %d roots, %4.1f%% redundancy."),
			     nroots,
			     ((double)nroots*100.0)/(double)ndata);

   GuiSetLabelText(wl->encHeadline,
		   _("<big>Creating the error correction file.</big>\n<i>%s</i>"), ec->msg);

   /*** Test the image file and create the CRC sums */

   /* Get rid of old ecc file (if any exists) */

   if(LargeStat(Closure->eccName, &n))
   {  
      if(GuiConfirmEccDeletion(Closure->eccName))
	 g_unlink(Closure->eccName);
      else
      {  GuiSetLabelText(ec->wl->encFootline,
			 _("<span %s>Aborted to keep existing ecc file.</span>"),
			 Closure->redMarkup); 
	 ec->earlyTermination = FALSE;
	 goto terminate;
      }
   }

   /* Open image and ecc files */

   PrintLog(_("\nOpening %s"), Closure->imageName);

   image = OpenImageFromFile(Closure->imageName, O_RDONLY, IMG_PERMS);
   ec->image = image;
   if(!image)
   {  PrintLog(": %s.\n", strerror(errno));
      Stop(_("Image file %s: %s."),Closure->imageName, strerror(errno));
   }
   if(image->inLast == 2048)
        PrintLog(_(": %" PRId64 " medium sectors.\n"), image->sectorSize);
   else PrintLog(_(": %" PRId64 " medium sectors and %d bytes.\n"), 
		   image->sectorSize-1, image->inLast);

   if(!Closure->eccName || !strlen(Closure->eccName))
     Stop(_("No error correction file specified!\n"));

   image->eccFile = LargeOpen(Closure->eccName, O_RDWR | O_CREAT, IMG_PERMS);
   if(!image->eccFile)
      Stop(_("Can't open %s:\n%s"),Closure->eccName,strerror(errno));

   ec->timer   = g_timer_new();

   /* Try to use CRC values created during last read */

   if(CrcBufValid(Closure->crcBuf, image, FULL_IMAGE))   
   {  guint32 crc_idx;
      int percent, last_percent = 0;
      char *msg = _("Writing sector checksums: %3d%%");

      GuiSetLabelText(wl->encLabel1,
		      _("<b>1. Writing image sector checksums:</b>"));

      memcpy(image->mediumSum, Closure->crcBuf->imageMD5sum, 16);
      MD5Init(&md5Ctxt);    /*  md5sum of CRC portion of ecc file */

      /* Write out the cached CRC sectors */

      if(!LargeSeek(image->eccFile, (gint64)sizeof(EccHeader)))
         Stop(_("Failed skipping the ecc header: %s"),strerror(errno));

      for(crc_idx=0; crc_idx<image->sectorSize; crc_idx+=1024)
      {  int ci,n,size; 
	 guint32 *crcbuf;

	 if(crc_idx + 1024 > image->sectorSize)
	       ci = image->sectorSize - crc_idx;
	 else  ci = 1024;

	 size   = ci*sizeof(guint32);
	 crcbuf = &Closure->crcBuf->crcbuf[crc_idx];

	 n = LargeWrite(image->eccFile, crcbuf, size);
	 MD5Update(&md5Ctxt, (unsigned char*)crcbuf, size);

	 if(size != n)
	   Stop(_("Error writing CRC information: %s"), strerror(errno));

         percent = (100*crc_idx)/image->sectorSize;
         if(last_percent != percent) 
         {  PrintProgress(msg,percent);
	    GuiSetProgress(wl->encPBar1, percent, 100);

	    last_percent = percent;
	 }
      }	    

      PrintProgress(msg, 100);
   }

   /* Cached crc buffer can not be used. 
      Scan image for missing sectors and calculate the checksums.
      Checksums are only computed locally and not provided in the cache. */
   else   
   {  GuiSetLabelText(wl->encLabel1,
		      _("<b>1. Calculating image sector checksums:</b>"));

      FreeCrcBuf(Closure->crcBuf);  /* just a defensive measure */
      Closure->crcBuf = NULL;
     
      RS01ScanImage(self, image, &md5Ctxt, CREATE_CRC);

      if(image->sectorsMissing)
      {  LargeClose(image->eccFile); /* Will be deleted anyways; no need to test for errors */
	 image->eccFile = NULL;

	 g_unlink(Closure->eccName);  /* Do not leave a CRC-only .ecc file behind */

	 if(Closure->stopActions)   
	 {
	    if(Closure->stopActions == STOP_CURRENT_ACTION) /* suppress memleak warning when closing window */
	    {  GuiSetLabelText(wl->encFootline, 
			       _("<span %s>Aborted by user request!</span> (partial error correction file removed)"),
			       Closure->redMarkup);
	    }
	    ec->earlyTermination = FALSE;  /* suppress respective error message */
	   goto terminate;
	 }
	 else 
	 {  GuiSetProgress(wl->encPBar1, 100, 100);

	    Stop(_("%" PRId64 " sectors unread or missing due to errors.\n"), image->sectorsMissing);
	 }
      }
   }

   PrintTimeToLog(ec->timer, "for CRC writing/generation.\n");

   GuiSetProgress(wl->encPBar1, 100, 100);
   GuiShowWidget(wl->encPBar2);
   GuiShowWidget(wl->encLabel2);
   
   if(!Closure->guiMode)
     PrintLog("%s\n",ec->msg);

   /*** Prepare Ecc file header.
        The .eccSum will be filled in after all ecc blocks have been created. */

   image->eccFileHeader = eh = g_malloc0(sizeof(EccHeader));
   memcpy(eh->cookie, "*dvdisaster*", 12);
   memcpy(eh->method, "RS01", 4);
   eh->methodFlags[0] = 1;
   if(!Closure->regtestMode)
     eh->methodFlags[3] = Closure->releaseFlags;
   gint64_to_uchar(eh->sectors, image->sectorSize);
   eh->dataBytes       = ndata;
   eh->eccBytes        = nroots;

   eh->creatorVersion  = Closure->version;
   eh->fpSector        = FINGERPRINT_SECTOR;
   eh->inLast          = image->inLast;

   /* dvdisaster 0.66 brings some extensions which are not compatible with
      prior versions. These are:
      - If the methodFlags contains any other bits set than methodFlags[0] == 1,
        prior versions will incorrectly reject ecc files as being produced by
	version 0.40.7 due to a bug in the version processing code.
	So ecc files tagged with -devel or -rc status will not work with prior
	versions. But they are experimental versions available only through CVS, 
	so this issue is not as big as it appears.
      - Version 0.66 records the inLast value in the ecc file to facilitate
        processing non-image files. Previous versions do not use this field
	and may round up file length to the next multiple of 2048 when doing
	error correction.
   */

   if(image->inLast != 2048)
        eh->neededVersion = 6600;
   else eh->neededVersion = 5500;

   memcpy(eh->mediumFP, image->imageFP, 16);
   memcpy(eh->mediumSum, image->mediumSum, 16);

   if(!LargeSeek(image->eccFile, (gint64)sizeof(EccHeader) + image->sectorSize*sizeof(guint32)))
	Stop(_("Failed skipping ecc+crc header: %s"),strerror(errno));

   /*** Allocate buffers for the parity calculation and image data caching. 

        The algorithm builds the parity file consecutively in chunks of n_parity_blocks.
        We use all the amount of memory allowed by cacheMiB for caching the parity blocks. */

   n_parity_blocks = ((guint64)Closure->cacheMiB<<20) / (guint64)nroots_aligned;
   n_parity_blocks &= ~0x7ff;                   /* round down to multiple of 2048 */
   n_parity_bytes  = (guint64)nroots_aligned * n_parity_blocks;

   /* Each chunk of parity blocks is built iteratively by processing the data in layers
      (first all bytes at pos 0, then pos 1, until ndata layers have been processed).
      So one buffer of n_layer_bytes = n_parity_blocks needs to be buffered.
      For practical reasons we require that the layer size is a multiple of the
      medium sector size of 2048 bytes. */

   n_layer_bytes   = n_parity_blocks;
   n_layer_sectors = n_parity_blocks/2048;

   if(n_layer_sectors*2048 != n_parity_blocks)
     Stop("Internal error: parity blocks are not a multiple of sector size.\n");

   ec->parity = g_try_malloc(n_parity_bytes);
   ec->data   = g_try_malloc(n_layer_bytes);

   if(!ec->parity || !ec->data)
      Stop(_("Failed allocating memory for I/O cache.\n"
	     "Cache size is currently %d MiB.\n"
	     "Try reducing it.\n"),
	   Closure->cacheMiB);

   /*** Setup the block counters for mapping medium sectors to ecc blocks 
        The image is divided into ndata sections;
        with each section spanning s sectors. */

   s = (image->sectorSize+ndata-1)/ndata;

   for(si=0, i=0; i<ndata; si+=s, i++)
     block_idx[i] = si;

   /*** Create ecc information for the medium image. */ 

   max_percent = ndata * ((s / n_layer_sectors) + 1);
   g_timer_start(ec->timer);

   /* Process the image.
      From each section a chunk of n_layer_sectors is read in at once.
      So after (s/n_layer_sectors)+1 iterations the whole image has been processed. */

   for(chunk=0; chunk<s; chunk+=n_layer_sectors) 
   {  guint64 actual_layer_bytes,actual_layer_sectors;

      /* Prepare the parity data for the next chunk. */

      memset(ec->parity, 0, n_parity_bytes);

      /* The last chunk may contain fewer sectors. */

      if(chunk+n_layer_sectors < s)
           actual_layer_sectors = n_layer_sectors;
      else actual_layer_sectors = s-chunk;

      actual_layer_bytes   = 2048*actual_layer_sectors;

      /* Work each of the ndata data layers 
	 into the parity data of the current chunk. */

      {  int sp = rt->shiftInit;  /* => (ndata + sp) mod nroots = 0 */

         for(layer=0; layer<ndata; layer++)
	 {  int offset = 0;

	    if(Closure->stopActions) /* User hit the Stop button */
	    {  if(Closure->stopActions == STOP_CURRENT_ACTION) /* suppress memleak warning when closing window */
	       {  GuiSetLabelText(wl->encFootline, 
				  _("<span %s>Aborted by user request!</span> (partial error correction file removed)"),
				  Closure->redMarkup);
	       }
	       ec->earlyTermination = FALSE;  /* suppress respective error message */
	       LargeClose(image->eccFile);
	       image->eccFile = NULL;
	       g_unlink(Closure->eccName); /* Do not leave partial .ecc file behind */
	       goto terminate;
	    }

	    /* Read the next data sectors of this layer. */

	    for(si=0; si<actual_layer_sectors; si++)
	    {  RS01ReadSector(image, ec->data+offset, block_idx[layer]);
	       block_idx[layer]++;
	       offset += 2048;
	    }

	    /* Now process the data bytes of the current layer. */

	    EncodeNextLayer(rt, ec->data, ec->parity, actual_layer_bytes, sp);

	    if(++sp>=nroots) sp=0;   /* shift */

	    /* Report progress */

	    progress++;
	    percent = (1000*progress)/max_percent;
	    if(last_percent != percent) 
	    {  GuiSetProgress(wl->encPBar2, percent, 1000);
	       PrintProgress(_("Ecc generation: %3d.%1d%%"), percent/10, percent%10);
	       last_percent = percent;
	    }
	 }
	}

      /* EncodeNextLayer() uses a stride of nroots_aligned bytes; compact to nroots */

      if(nroots != nroots_aligned)
	 for(si=1; si<actual_layer_bytes; si++)
	    memmove(ec->parity+si*nroots, ec->parity+si*nroots_aligned, nroots);

      /* Write the nroots bytes of parity information */

      n = LargeWrite(image->eccFile, ec->parity, nroots*actual_layer_bytes);

      if(n != nroots*actual_layer_bytes)
        Stop(_("could not write to ecc file \"%s\":\n%s"),Closure->eccName,strerror(errno));

      MD5Update(&md5Ctxt, ec->parity, nroots*actual_layer_bytes);
   }

   /*** Complete the ecc header and write it out */

   MD5Final(eh->eccSum, &md5Ctxt);

   LargeSeek(image->eccFile, 0);
#ifdef HAVE_BIG_ENDIAN
   SwapEccHeaderBytes(eh);
#endif
   n = LargeWrite(image->eccFile, eh, sizeof(EccHeader));
   if(n != sizeof(EccHeader))
     Stop(_("Can't write ecc header:\n%s"),strerror(errno));

   if(!LargeClose(image->eccFile))
     Stop(_("Error closing error correction file:\n%s"), strerror(errno));
   image->eccFile = NULL;

   PrintTimeToLog(ec->timer, "for ECC generation.\n");

   PrintProgress(_("Ecc generation: 100.0%%\n"));
   PrintLog(_("Error correction file \"%s\" created.\n"
	       "Make sure to keep this file on a reliable medium.\n"),
	     Closure->eccName);
   
   GuiSetProgress(wl->encPBar2, 100, 100);

   GuiSetLabelText(wl->encFootline, 
		   _("The error correction file has been successfully created.\n"
		     "Make sure to keep this file on a reliable medium.")); 

   /*** If the --unlink option or respective GUI switch is set, 
	unlink the image. */

   if(Closure->unlinkImage)
   {  if(ec->image) CloseImage(ec->image);
      ec->image = NULL;
      unlink_image(Closure->guiMode ? wl->encFootline2 : NULL);
   }

   /*** Clean up */

   ec->earlyTermination = FALSE;

terminate:
   ecc_cleanup((gpointer)ec);
}

