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

#include "read-linear.h"
#include "scsi-layer.h"
#include "udf.h"

/*
 * IO buffer states
 */

enum { BUF_EMPTY, BUF_FULL, BUF_DEAD, BUF_EOF };

/*
 * Send EOF to the worker thread
 */

static void send_eof(read_closure *rc)
{
   g_mutex_lock(rc->mutex);
   while(rc->bufState[rc->readPtr] != BUF_EMPTY)
     g_cond_wait(rc->canRead, rc->mutex);

   rc->bufState[rc->readPtr] = BUF_EOF;
   rc->readPtr++;
   if(rc->readPtr >= READ_BUFFERS)
     rc->readPtr = 0;

   g_cond_signal(rc->canWrite);
   g_mutex_unlock(rc->mutex);
}

/*
 * Cleanup. 
 */

static void cleanup(gpointer data)
{  read_closure *rc = (read_closure*)data;
   int full_read = FALSE;
   int aborted   = rc->earlyTermination;
   int scan_mode = rc->scanMode;
   int i;
   int renderers_left = TRUE;

   /* Make sure that all spiral/curve render idle functions have finished.
      They depend on some structures we are going to free now. 
      If Closure->stopActions == STOP_SHUTDOWN_ALL, the main thread is
      waiting for this thread to terminate and will not invoke any more
      idle functions. Executing the while loop would create a deadlock
      in that case. */

   while(renderers_left && Closure->stopActions != STOP_SHUTDOWN_ALL)
   {  g_mutex_lock(rc->rendererMutex);
      if(rc->activeRenderers<=0)
        renderers_left=FALSE; 
      g_mutex_unlock(rc->rendererMutex);

      g_usleep(G_USEC_PER_SEC/10);
   }
      
   /* In reading passes > 1, Closure->sectorSkip is forced to be one.
      Restore the old value now. */

   Closure->sectorSkip = rc->savedSectorSkip;

   /* Reset temporary ignoring of fatal errors.
      User has to set this in the preferences to make it permanent. */

   if(Closure->ignoreFatalSense == 2)
      Closure->ignoreFatalSense = 0;

   /* This is a failure condition */

   if(g_thread_self() == rc->worker)
   {  g_printf("Reading/Scanning terminated from worker thread - trouble ahead\n");
      return;
   }

   /* Make sure worker thread exits gracefully */

   if(rc->worker && !rc->workerError)
   {  send_eof(rc);
      g_thread_join(rc->worker);
   }

   /* Clean up reader thread */

   if(rc->image)
     full_read = (rc->readOK == rc->image->dh->sectors && !Closure->crcErrors);

   UnregisterCleanup();

   if(Closure->guiMode)
   {  if(rc->unreportedError)
         SwitchAndSetFootline(Closure->readLinearNotebook, 1, Closure->readLinearFootline, 
			      _("<span %s>Aborted by unrecoverable error.</span> %lld sectors read, %lld sectors unreadable/skipped so far."),
			      Closure->redMarkup, rc->readOK, Closure->readErrors); 
   }

   if(rc->readerImage)   
     if(!LargeClose(rc->readerImage))
       Stop(_("Error closing image file:\n%s"), strerror(errno));
   if(rc->writerImage)   
     if(!LargeClose(rc->writerImage))
       Stop(_("Error closing image file:\n%s"), strerror(errno));

   if(rc->image)   CloseImage(rc->image);

   if(rc->mutex)
   {  g_mutex_clear(rc->mutex);
      g_free(rc->mutex);
   }
   if(rc->canRead)
   {  g_cond_clear(rc->canRead);
      g_free(rc->canRead);
   }
   if(rc->canWrite) 
   {  g_cond_clear(rc->canWrite);
      g_free(rc->canWrite);
   }
   if(rc->workerError) g_free(rc->workerError);

   for(i=0; i<READ_BUFFERS; i++)
     if(rc->alignedBuf[i])
       FreeAlignedBuffer(rc->alignedBuf[i]);

   if(rc->msg)     g_free(rc->msg);
   if(rc->speedTimer) g_timer_destroy(rc->speedTimer);
   if(rc->readTimer)  g_timer_destroy(rc->readTimer);
   if(rc->readMap) FreeBitmap(rc->readMap);
   if(rc->crcBuf) FreeCrcBuf(rc->crcBuf);
   if(rc->volumeLabel) g_free(rc->volumeLabel);

   if(rc->rendererMutex)
   {  g_mutex_clear(rc->rendererMutex);
      g_free(rc->rendererMutex);
   }
   
   /* trigger failure if some threads are still accessing this */
   memset(rc, sizeof(read_closure), 0xff); 
   g_free(rc);

   ClearCrcCache(); /* cache handling is currently broken. Discard the cache! */
   
   /* Continue with ecc file creation after read */

   if(Closure->readAndCreate && Closure->guiMode && !scan_mode && !aborted)  /* General prerequisites */
   {  if(   !strncmp(Closure->methodName, "RS01", 4)                         /* codec prerequisites */
	 || (!strncmp(Closure->methodName, "RS03", 4) && Closure->eccTarget == ECC_FILE) ) 
      {	 if(!full_read)
	 {  ModalDialog(GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, NULL,
			_("Automatic error correction file creation\n"
			  "is only possible after a full reading pass.\n"));
	    AllowActions(TRUE);
	 }
	 else ContinueWithAction(ACTION_CREATE_CONT); 
      }
      else AllowActions(TRUE);
   }
   else 
     if(Closure->guiMode)
       AllowActions(TRUE);

   if(!full_read && Closure->crcCache)
     ClearCrcCache();

   if(scan_mode)   /* we haven't created an image, so throw away the crc sums */
     ClearCrcCache();
   
   /* In GUI mode both the reader and worker are spawned sub threads;
      however in CLI mode the reader is the main thread and must not be terminated. */

   if(Closure->guiMode)
      g_thread_exit(0);
}

/***
 *** Helper functions for the reader
 ***/

/*
 * Register with different label texts depending on rc->scanMode
 */

static void register_reader(read_closure *rc)
{
   if(rc->scanMode)  /* Output messages differ in read and scan mode */
   {  RegisterCleanup(_("Scanning aborted"), cleanup, rc);
      if(Closure->guiMode)
	SetLabelText(GTK_LABEL(Closure->readLinearHeadline), 
		     "<big>%s</big>\n<i>%s</i>",
		     _("Scanning medium for read errors."),
		     _("Medium: not yet determined"));
   }
   else
   {  RegisterCleanup(_("Reading aborted"), cleanup, rc);
      if(Closure->guiMode)
       SetLabelText(GTK_LABEL(Closure->readLinearHeadline), 
		    "<big>%s</big>\n<i>%s</i>",
		    _("Preparing for reading the medium image."),
		    _("Medium: not yet determined"));
   }
}

/*
 * Find out current which mode we are operating in:
 * 1. Scanning
 * 2. Creating a new image
 * 3. Completing an existing image
 * Output respective messages and prepare the image file.
 */

static void determine_mode(read_closure *rc)
{  guint8 medium_fp[16], image_fp[16];
   guint64 image_size;
   unsigned char *buf = rc->alignedBuf[0]->buf;
   int unknown_fingerprint = FALSE;

   /*** In scan mode we simply need to output some messages. */

   if(rc->scanMode)
   {  
      rc->msg = g_strdup(_("Scanning medium for read errors."));

      PrintLog("%s\n", rc->msg);
      if(Closure->guiMode)
      {  if(rc->eccMethod)
	   SetLabelText(GTK_LABEL(Closure->readLinearHeadline), 
			"<big>%s</big>\n<i>- %s -</i>", rc->msg,
			_("Reading CRC information"));

         else
	   SetLabelText(GTK_LABEL(Closure->readLinearHeadline), 
			"<big>%s</big>\n<i>%s</i>", rc->msg, rc->image->dh->mediumDescr);
      }

      rc->readMarker = 0;

      if(Closure->guiMode)
	 InitializeCurve(rc, rc->image->dh->maxRate, rc->image->dh->canC2Scan);

      return;
   } 

   /*** If no image file exists, open a new one. */

reopen_image:
   if(!LargeStat(Closure->imageName, &image_size))
   {  
      if(rc->msg) g_free(rc->msg);
      rc->msg = g_strdup(_("Reading new medium image."));
      
      if(!(rc->writerImage = LargeOpen(Closure->imageName, O_WRONLY | O_CREAT, IMG_PERMS)))
	 Stop(_("Can't open %s:\n%s"),Closure->imageName,strerror(errno));
      if(!(rc->readerImage = LargeOpen(Closure->imageName, O_RDONLY, IMG_PERMS)))
	 Stop(_("Can't open %s:\n%s"),Closure->imageName,strerror(errno));

      PrintLog(_("Creating new %s image.\n"),Closure->imageName);
      if(Closure->guiMode)
      {  if(rc->eccMethod)
	    SetLabelText(GTK_LABEL(Closure->readLinearHeadline),
			 "<big>%s</big>\n<i>%s</i>", rc->msg,
			 _("Reading CRC information"));
	 else
	    SetLabelText(GTK_LABEL(Closure->readLinearHeadline),
			 "<big>%s</big>\n<i>%s</i>", rc->msg, rc->image->dh->mediumDescr);
      }
      rc->rereading  = FALSE;
      rc->readMarker = 0;

      if(Closure->guiMode)
	 InitializeCurve(rc, rc->image->dh->maxRate, rc->image->dh->canC2Scan);

      return;
   }

   /*** Examine the given image file */

   if(rc->msg) g_free(rc->msg);
   rc->msg = g_strdup(_("Completing existing medium image."));

   /* Use the existing file as a starting point.
      Set the read marker at the end of the file
      so that the reader looks for "dead_sector" markers
      and skips already read blocks. */

   if(!(rc->readerImage = LargeOpen(Closure->imageName, O_RDONLY, IMG_PERMS)))
      Stop(_("Can't open %s:\n%s"),Closure->imageName,strerror(errno));
   if(!(rc->writerImage = LargeOpen(Closure->imageName, O_WRONLY, IMG_PERMS)))
      Stop(_("Can't open %s:\n%s"),Closure->imageName,strerror(errno));

   rc->rereading  = 1;
   rc->readMarker = image_size / 2048;

   /* Try reading the media and image fingerprints. */

   if(!LargeSeek(rc->readerImage, (gint64)(2048*FINGERPRINT_SECTOR)))
      unknown_fingerprint = TRUE;
   else
   {  struct MD5Context md5ctxt;
      int n = LargeRead(rc->readerImage, buf, 2048);
      int fp_read;

      MD5Init(&md5ctxt);
      MD5Update(&md5ctxt, buf, 2048);
      MD5Final(image_fp, &md5ctxt);

      fp_read = GetImageFingerprint(rc->image, medium_fp, FINGERPRINT_SECTOR);
	 
      if(n != 2048 || !fp_read || (CheckForMissingSector(buf, FINGERPRINT_SECTOR, NULL, 0) != SECTOR_PRESENT))
	 unknown_fingerprint = TRUE;
   }

   /* If fingerprints could be read, compare them. */
      
   if(!unknown_fingerprint && memcmp(image_fp, medium_fp, 16))
   {  	  
      if(!Closure->guiMode)
	 Stop(_("Image file does not match the optical disc."));
      else
      {  int answer = ConfirmImageDeletion(Closure->imageName);
	   
	 if(!answer)
	 {  rc->unreportedError = FALSE;
	    SwitchAndSetFootline(Closure->readLinearNotebook, 1, Closure->readLinearFootline, 
				 _("<span %s>Reading aborted.</span> Please select a different image file."),
				 Closure->redMarkup); 
	       cleanup((gpointer)rc);
	 }
	 else  /* Start over with new file */
	 {  LargeClose(rc->readerImage);
	    LargeClose(rc->writerImage);
	    LargeUnlink(Closure->imageName);
	    goto reopen_image;
	 } 
      }
   }

   /*** If the image is not complete yet, first aim to read the
	unvisited sectors before trying to re-read the missing ones.
        Exception: We must start from the beginning if multiple reading passes are requested. */

   if(!Closure->readStart && !Closure->readEnd 
      && rc->readMarker < rc->image->dh->sectors-1 && Closure->readingPasses <= 1)
   {  PrintLog(_("Completing image %s. Continuing with sector %lld.\n"),
	       Closure->imageName, rc->readMarker);
      rc->firstSector = rc->readMarker;
      Closure->additionalSpiralColor = 0;  /* blue */
   }
   else 
   {  PrintLog(_("Completing image %s. Only missing sectors will be read.\n"), Closure->imageName);
      Closure->additionalSpiralColor = 3;  /* dark green*/
   }
      
   if(Closure->guiMode)
      SetLabelText(GTK_LABEL(Closure->readLinearHeadline),
		   "<big>%s</big>\n<i>%s</i>",rc->msg,rc->image->dh->mediumDescr);

   if(Closure->guiMode)
      InitializeCurve(rc, rc->image->dh->maxRate, rc->image->dh->canC2Scan);
}

/*
 * Fill the gap between rc->readMarker and rc->firstSector
 * with dead sector markers.
 */

static void fill_gap(read_closure *rc)
{  gint64 s;

   if(!rc->scanMode && rc->firstSector > rc->readMarker)
   {  unsigned char buf[2048];

      s = rc->readMarker;

      if(!LargeSeek(rc->writerImage, (gint64)(2048*s)))
	Stop(_("Failed seeking to sector %lld in image [%s]: %s"),
	     s, "fill", strerror(errno));

      while(s < rc->firstSector)
      {  int n;

	 CreateMissingSector(buf, s, rc->image->imageFP, FINGERPRINT_SECTOR, rc->volumeLabel);
	 n = LargeWrite(rc->writerImage, buf, 2048);
	 if(n != 2048)
	   Stop(_("Failed writing to sector %lld in image [%s]: %s"),
		s, "fill", strerror(errno));
	 s++;
      }
   }
}

/*
 * Allocate memory for CRC32 sums and preload the cache.
 */

static void prepare_crc_cache(read_closure *rc)
{
   Closure->crcAvailable = FALSE;

   /*** Memory for the CRC32 sums is needed in two cases
        and comes in two flavors: */

   /* a) A full image read is attempted. 
         The image CRC32 and md5sum are calculated on the fly,
         as they may be used for ecc creation later. 
         In that case we use the old Closure->crcCache storage. */

   if(   !rc->scanMode && !rc->rereading 
      && rc->firstSector == 0 && rc->lastSector == rc->image->dh->sectors-1)
   {  if(Closure->crcCache)
	 g_printf("Internal problem: crcCache not clean\n");
      Closure->crcCache = g_try_malloc(sizeof(guint32) * rc->image->dh->sectors);

      if(Closure->crcCache)
	Closure->crcImageName = g_strdup(Closure->imageName);

      if(rc->eccMethod && rc->eccMethod->updateCksums && rc->eccMethod->finalizeCksums)
	rc->doMD5sums = TRUE;  /* not all codecs support these actions! */
      MD5Init(&rc->md5ctxt);
   }

   /* b) We have suitable ecc data and want to compare CRC32sums
         against it while reading. Note that for augmented images
         the checksums may be incomplete due to unreadable CRC sectors,
         so we keep them in the CrcBuf struct which deals with lost
         sectors internally. */

   if(rc->eccMethod)
   {  char method_name[5];

      strncpy(method_name, rc->eccMethod->name, 4);
      method_name[4] = 0;
      PrintCLI("%s (%s) ... ",_("Reading CRC information from ecc data"),
	                      method_name);
      if(Closure->guiMode)
	 SetLabelText(GTK_LABEL(Closure->readLinearHeadline),
		      "<big>%s</big>\n<i>%s</i>", 
		      _("Reading CRC information from ecc data"),
		      rc->image->dh->mediumDescr);

      if(rc->eccMethod->getCrcBuf)
      {  rc->crcBuf = rc->eccMethod->getCrcBuf(rc->image);
	 Closure->crcAvailable = TRUE;
	 if(Closure->guiMode)
	    RedrawReadLinearWindow();
      }
	
      else rc->crcBuf = NULL;

      if(rc->eccMethod->resetCksums)
      {  rc->doMD5sums = TRUE;
	 rc->eccMethod->resetCksums(rc->image);
      }

      if(Closure->guiMode)
	 SetLabelText(GTK_LABEL(Closure->readLinearHeadline),
		      "<big>%s</big>\n<i>%s</i>", rc->msg, rc->image->dh->mediumDescr);
      PrintCLI(_("done.\n"));
   }
}

/*
 * Wait for the drive to spin up and prepare the timer
 */

static void prepare_timer(read_closure *rc)
{
   if(Closure->guiMode && Closure->spinupDelay)
     SwitchAndSetFootline(Closure->readLinearNotebook, 1, Closure->readLinearFootline,
			  _("Waiting %d seconds for drive to spin up...\n"), Closure->spinupDelay);

   SpinupDevice(rc->image->dh);

   if(Closure->guiMode && Closure->spinupDelay)
     SwitchAndSetFootline(Closure->readLinearNotebook, 0, Closure->readLinearFootline, "ignore");

   if(Closure->spinupDelay)  /* eliminate initial seek time from timing */
     ReadSectors(rc->image->dh, rc->alignedBuf[0]->buf, rc->firstSector, 1); 
   g_timer_start(rc->speedTimer);
   g_timer_start(rc->readTimer);
}

/*
 * Update the various progress indicators
 */

static void show_progress(read_closure *rc)
{  int percent;

   if(Closure->guiMode && rc->lastErrorsPrinted != Closure->readErrors)
   {  SetLabelText(GTK_LABEL(Closure->readLinearErrors), 
		   _("Unreadable / skipped sectors: %lld"), Closure->readErrors);
      rc->lastErrorsPrinted = Closure->readErrors;
   }

   if(rc->readPos>rc->readMarker) rc->readMarker=rc->readPos;
   percent = (1000*rc->readPos)/rc->image->dh->sectors;
      
   if(rc->lastPercent != percent) 
   {  gulong ignore;
      int color;

      if(Closure->guiMode)
	ChangeSpiralCursor(Closure->readLinearSpiral, percent);

      if(rc->readOK <= rc->lastReadOK)  /* nothing read since last sample? */
      {  rc->speed = 0.0;
	 if(Closure->readErrors - rc->previousReadErrors > 0)
	    color = 2;
	 else if(Closure->crcErrors - rc->previousCRCErrors > 0)
	    color = 4;
	 else color = Closure->additionalSpiralColor;

	 if(Closure->guiMode)
	    AddCurveValues(rc, percent, color, rc->maxC2);
	 rc->lastPercent    = percent;
	 rc->lastSpeed      = rc->speed;
	 rc->previousReadErrors = Closure->readErrors;
	 rc->previousCRCErrors  = Closure->crcErrors;
	 rc->lastReadOK     = rc->readOK;
      }
      else               /* something was read since last sample */
      {  double kb_read = (rc->readOK - rc->lastReadOK) * 2.0;
	 double elapsed = g_timer_elapsed(rc->speedTimer, &ignore);
	 double kb_sec  = kb_read / elapsed;
	 
	 if(Closure->readErrors - rc->previousReadErrors > 0)
	    color = 2;
	 else if(Closure->crcErrors - rc->previousCRCErrors > 0)
	    color = 4;
	 else color = 1;

	 if(rc->firstSpeedValue)
	 {   rc->speed = kb_sec / rc->image->dh->singleRate;

	     if(Closure->guiMode)
	     {  AddCurveValues(rc, rc->lastPercent, color, rc->maxC2);
		AddCurveValues(rc, percent, color, rc->maxC2);
	     }
	     
	     rc->firstSpeedValue    = FALSE;
	     rc->lastPercent        = percent;
	     rc->lastSpeed          = rc->speed;
	     rc->previousReadErrors = Closure->readErrors;
	     rc->previousCRCErrors  = Closure->crcErrors;
	     rc->lastReadOK         = rc->readOK;
	 }
	 else
	 {  static int cut_peaks = 3;

	    /* If reading is interrupted by a requester, following
	       reads might be extremely fast due to read-ahead in
	       the kernel. Try to mask these out. */

	    if(kb_sec / rc->image->dh->singleRate > 100.0 && cut_peaks)
	       cut_peaks--;
	    else
	    {  rc->speed = (rc->speed + kb_sec / rc->image->dh->singleRate) / 2.0;
	       cut_peaks=3;
	    }

	    if(Closure->guiMode)
	       AddCurveValues(rc, percent, color, rc->maxC2);

	    if(Closure->speedWarning && rc->lastSpeed > 0.5)
	    {  double delta = rc->speed - rc->lastSpeed;
	       double abs_delta = fabs(delta);
	       double sp = (100.0*abs_delta) / rc->lastSpeed; 

	       if(sp >= Closure->speedWarning)
	       {  if(delta > 0.0)
		     PrintCLI(_("Sector %lld: Speed increased to %4.1fx\n"), 
			      rc->readPos, fabs(rc->speed));
		  else
		     PrintCLI(_("Sector %lld: Speed dropped to %4.1fx\n"),
			      rc->readPos, fabs(rc->speed));
	       }
	    }

	    if(Closure->fixedSpeedValues)
	         PrintProgress(_("Read position: %3d.%1d%% (nn.nx)"),
			       percent/10,percent%10);
	    else PrintProgress(_("Read position: %3d.%1d%% (%4.1fx)"),
			       percent/10,percent%10,rc->speed);
	    
	    rc->lastPercent    = percent;
	    rc->lastSpeed      = rc->speed;
	    rc->previousReadErrors = Closure->readErrors;
	    rc->previousCRCErrors  = Closure->crcErrors;
	    rc->lastReadOK     = rc->readOK;
	    g_timer_start(rc->speedTimer);
	 }
      }
      rc->maxC2 = 0;
   }
}   

/***
 *** Try reading the medium and create the image and map.
 ***/

/* 
 * The writer / checksum part
 */

static gpointer worker_thread(read_closure *rc)
{  gint64 s;
   int nsectors;
   int i;

   for(;;)
   {  
      /* See if more buffers are available for processing */

      g_mutex_lock(rc->mutex);

      while(rc->bufState[rc->writePtr] == BUF_EMPTY)
      {  g_cond_wait(rc->canWrite, rc->mutex);
      }

      if(rc->bufState[rc->writePtr] == BUF_EOF)
      {  g_mutex_unlock(rc->mutex);
	 return 0;
      }

      s = rc->bufferedSector[rc->writePtr];
      nsectors = rc->nSectors[rc->writePtr];
      g_mutex_unlock(rc->mutex);

      /* Write out buffer, update checksums if not in scan mode */

      if(!rc->scanMode)
      {  int n;

	 if(!LargeSeek(rc->writerImage, (gint64)(2048*s)))
	 {  rc->workerError = g_strdup_printf(_("Failed seeking to sector %lld in image [%s]: %s"),
					      s, "store", strerror(errno));
	    goto update_mutex;
	 }

	 n = LargeWrite(rc->writerImage, rc->alignedBuf[rc->writePtr]->buf, 2048*nsectors);
	 if(n != 2048*nsectors)
	 {  rc->workerError = g_strdup_printf(_("Failed writing to sector %lld in image [%s]: %s"),
	                                      s, "store", strerror(errno));
	    goto update_mutex;
	 }

	 /* On-the-fly CRC calculation */

	 if(Closure->crcCache)
	 {  for(i=0; i<nsectors; i++)
	       Closure->crcCache[s+i] = Crc32(rc->alignedBuf[rc->writePtr]->buf+2048*i, 2048);
	 }
      }

      /* Do on-the-fly CRC / md5sum testing. This is the only action carried out
         in scan mode, but also done while reading. */         

      if(rc->eccMethod && rc->bufState[rc->writePtr] != BUF_DEAD)
      {
	for(i=0; i<nsectors; i++)
	{  unsigned char *buf = rc->alignedBuf[rc->writePtr]->buf+2048*i;
	   gint64 sector = s+i;

	   /* Have the codec update its internal checksums */

	   if(rc->doMD5sums)
	      rc->eccMethod->updateCksums(rc->image, sector, buf);

	   /* Check against CRCs in the ecc data */
	   
	   if(rc->crcBuf && sector < rc->crcBuf->size)
	   {  switch(CheckAgainstCrcBuffer(rc->crcBuf, sector, buf))
	      {  case CRC_BAD:
		   ClearProgress();
		   PrintCLI(_("* CRC error, sector: %lld\n"), (long long int)s+i);
		   Closure->crcErrors++;
		   if(rc->readMap)  /* trigger re-read FIXME*/
		     ClearBit(rc->readMap, sector);
		   break;

	         case CRC_UNKNOWN:  /* CRC data missing or detected as defective */
		   rc->crcIncomplete = TRUE;
		   break;
	      }
	   }
	}
      }

      /* Release this buffer */

update_mutex:
      g_mutex_lock(rc->mutex);
      rc->bufState[rc->writePtr] = BUF_EMPTY;
      rc->writePtr++;
      if(rc->writePtr >= READ_BUFFERS)
	rc->writePtr = 0;
      g_cond_signal(rc->canRead);
      g_mutex_unlock(rc->mutex);

      if(rc->workerError)
	return NULL;
   }

   return NULL;
}

/***
 *** The reader part
 ***/

static void insert_buttons(GtkDialog *dialog)
{  
  gtk_dialog_add_buttons(dialog, 
			 _utf("Ignore once"), 1,
			 _utf("Ignore always"), 2,
			 _utf("Abort"), 0, NULL);
} 

void ReadMediumLinear(gpointer data)
{  read_closure *rc = g_malloc0(sizeof(read_closure));
   int md5_failure = 0;
   int unrecoverable_sectors = 0;
   GError *err = NULL;
   int nsectors; 
   char *t = NULL;
   int status,n;
   int tao_tail;
   int i;

   /*** This value might be temporarily changed later. */

   rc->savedSectorSkip = Closure->sectorSkip;

   /*** Register the cleanup procedure so that Stop() can abort us properly. */

   rc->unreportedError  = TRUE;
   rc->earlyTermination = TRUE;
   rc->scanMode = GPOINTER_TO_INT(data);

   /* Register with different labels depending on rc->scanMode */

   register_reader(rc);

   /*** Timer setup */

   rc->speedTimer = g_timer_new();
   rc->readTimer  = g_timer_new();

   /*** Mutex for spiral renderer synchronization */

   rc->rendererMutex = g_malloc(sizeof(GMutex));
   g_mutex_init(rc->rendererMutex);

     /*** Create the aligned buffers. */

   for(i=0; i<READ_BUFFERS; i++)
     rc->alignedBuf[i] = CreateAlignedBuffer(MAX_CLUSTER_SIZE);

   /*** Open Device and query medium properties:
        rc->image will point to the optical medium, 
        and possibly the repsective ecc file.
        The on disk image is maintained in rc->reader|writerImage. */

   rc->image = OpenImageFromDevice(Closure->device);
   Closure->readErrors = Closure->crcErrors = rc->readOK = 0;

   /*** Save some useful information for the missing sector marker */

   if(rc->image->isoInfo && rc->image->isoInfo->volumeLabel[0])
      rc->volumeLabel = g_strdup(rc->image->isoInfo->volumeLabel);

   /*** See if we have an ecc file which belongs to the medium,
        and attach it with the Image struct. */

   OpenEccFileForImage(rc->image, Closure->eccName, O_RDONLY, IMG_PERMS);

   /*** If ecc information is available in both the image and the
	ecc file, the ecc file gets precedence. */

   if(rc->image->eccFileMethod)
   {  rc->eccMethod = rc->image->eccFileMethod;
      rc->eccHeader = rc->image->eccFileHeader;
   }
   else if(rc->image->eccMethod)
   {  rc->eccMethod = rc->image->eccMethod;
      rc->eccHeader = rc->image->eccHeader; 
   }

   /*** Some very old versions set eh->inLast to 0 when the last sector
	is completely filled. Set it to 2048 to avoid confusion. */

   if(rc->eccHeader && rc->eccHeader->inLast == 0)
     rc->eccHeader->inLast = 2048;
   
   /*** Make sure we are compatible with the ecc data */

   if(rc->eccHeader && (Closure->version < rc->eccHeader->neededVersion))
   {  if(Closure->guiMode)
      {  int answer;

	 if(rc->image->eccFileState == ECCFILE_PRESENT)
	   answer = ModalWarning(GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL, NULL,
				 _("This ecc file requires dvdisaster-%d.%d!\n\n"
				   "Proceeding could trigger incorrect behaviour.\n"
				   "Please read the image without using this ecc file\n"
				   "or visit http://www.dvdisaster.org for an upgrade.\n\n"), 
				 rc->eccHeader->neededVersion/10000,
				 (rc->eccHeader->neededVersion%10000)/100);
	 else
	   answer = ModalWarning(GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL, NULL,
				 _("This image requires dvdisaster-%d.%d!\n\n"
				   "Proceeding could trigger incorrect behaviour.\n"
				   "Please visit http://www.dvdisaster.org for an upgrade.\n\n"), 
				 rc->eccHeader->neededVersion/10000,
				 (rc->eccHeader->neededVersion%10000)/100);
	 
	 if(!answer)
	 {  SwitchAndSetFootline(Closure->readLinearNotebook, 1, Closure->readLinearFootline, 
				 _("<span %s>Aborted by user request!</span>"),
				 Closure->redMarkup, rc->readOK,Closure->readErrors); 
	    rc->unreportedError = FALSE;  /* suppress respective error message */
	    goto terminate;
	 }
      }
      else
      {  if(rc->image->eccFileState == ECCFILE_PRESENT)
	    PrintCLI(_("* Warning: This ecc file requires dvdisaster-%d.%d!\n"
		    "*          Proceeding could trigger incorrect behaviour.\n"
		    "*          Please read the image without using this ecc file\n"
		    "*          or visit http://www.dvdisaster.org for an upgrade.\n\n"), 
		  rc->eccHeader->neededVersion/10000,
		  (rc->eccHeader->neededVersion%10000)/100);
	else
	    PrintCLI(_("* Warning: This image requires dvdisaster-%d.%d!\n"
		    "*          Proceeding could trigger incorrect behaviour.\n"
		    "*          Please visit http://www.dvdisaster.org for an upgrade.\n\n"), 
		  rc->eccHeader->neededVersion/10000,
		  (rc->eccHeader->neededVersion%10000)/100);
      }
   }

   /*** See if user wants to limit the read range. */

   GetReadingRange(rc->image->dh->sectors, &rc->firstSector, &rc->lastSector);
   if(rc->firstSector > 0)                 /* Mark skipped sectors */
      Closure->additionalSpiralColor = 0;  /* blue */

   /*** Determine the reading mode. There are three possibilities:
	1. scanning (rc->scanMode == TRUE)
	2. reading into a new image file.
	3. completing an existing image file.
	After this function, files are prepared 
	and respective UI messages have been output. */

   determine_mode(rc);

   /*** If rc->firstSector > read_marker, fill the gap with dead sector markers. */

   fill_gap(rc);

   /*** Allocate and preload the CRC sum cache if necessary */

   prepare_crc_cache(rc);

   /*** Allocate a bitmap of read sectors to speed up multiple reading passes */

   if(Closure->readingPasses > 1)
      rc->readMap = CreateBitmap0(rc->image->dh->sectors);

   /*** Start the worker thread. We concentrate on reading from the drive here;
	writing the image file and calculating the checksums is done in a
	concurrent thread. */

   rc->mutex = g_malloc(sizeof(GMutex));
     g_mutex_init(rc->mutex);
   rc->canRead = g_malloc(sizeof(GCond));
     g_cond_init(rc->canRead);  
   rc->canWrite = g_malloc(sizeof(GCond));
     g_cond_init(rc->canWrite);  

   rc->worker = g_thread_try_new("readlinear_worker", (GThreadFunc)worker_thread, (gpointer)rc, &err);
   if(!rc->worker)
     Stop("Could not create worker thread: %s", err->message);

   /*** Prepare the speed timing */

   prepare_timer(rc);

   /*** Reset for the next reading pass */

   rc->lastReadOK = 0;  /* keep between passes */
   rc->lastPercent = (1000*rc->readPos)/rc->image->dh->sectors;

next_reading_pass:
   if(rc->pass > 0)
   {  Closure->readErrors = Closure->crcErrors = 0;
      switch(rc->image->dh->mainType)
      {  case BD:
	    if(Closure->sectorSkip > 32)
	       Closure->sectorSkip = 32;
	    break;
	 case DVD:
	    if(Closure->sectorSkip > 16)
	       Closure->sectorSkip = 16;
	    break;
	 default:
	    Closure->sectorSkip = 0;
	    break;
      }
      Closure->sectorSkip = 0;
      if(Closure->guiMode)
	 MarkExistingSectors();
      rc->lastCopied = 0;  /* Start rendering the spiral from the beginning */
   }

   /*** Read the medium image. */

   rc->readPos = rc->firstSector;
   rc->lastErrorsPrinted = 0;
   rc->previousReadErrors = rc->previousCRCErrors = 0;
   rc->speed = 0;
   rc->lastSpeed = -1.0;
   rc->firstSpeedValue = TRUE;
   tao_tail = 0;

   while(rc->readPos<=rc->lastSector)
   {  int cluster_mask = rc->image->dh->clusterSize-1;

      if(Closure->stopActions)   /* somebody hit the Stop button */
      {
	 if(Closure->stopActions == STOP_CURRENT_ACTION) /* suppress memleak warning when closing window */
	 {   SwitchAndSetFootline(Closure->readLinearNotebook, 1, Closure->readLinearFootline, 
				  _("<span %s>Aborted by user request!</span> %lld sectors read, %lld sectors unreadable/skipped so far."),
				  Closure->redMarkup, rc->readOK,Closure->readErrors); 
	 }
	 rc->unreportedError = FALSE;  /* suppress respective error message */
	 goto terminate;
      }

      /*** Decide between reading in fast mode (dh->clusterSize sectors at once)
	   or reading one sector at a time.
	   Fast mode gains some reading speed due to transfering fewer
	   but larger data blocks from the device.
	   Switching to fast mode is done only on cluster boundaries
	   -- this matches the internal structure of DVD and later media better. 
           In order to treat the 2 read errors at the end of TAO discs correctly,
           we switch back to per sector reading at the end of the medium. */

      if(   rc->readPos & cluster_mask 
	 || rc->readPos >= ((rc->image->dh->sectors - 2) & ~cluster_mask) )
            nsectors = 1;
      else  nsectors = rc->image->dh->clusterSize;

      if(rc->readPos+nsectors > rc->lastSector)  /* don't read past the (CD) media end */
	nsectors = rc->lastSector-rc->readPos+1;

      /*** If rc->readPos is lower than the read marker,
	   check if the sector has already been read
	   in a previous session. */

reread:
      if(!rc->scanMode && rc->readPos < rc->readMarker)
      {  int i,ok = 0;
	 int num_compare = nsectors;

	 /* Get image state from bitmap created at earlier reading pass */

	 if(rc->pass && rc->readMap)
	 {  for(i=0; i<num_compare; i++)
	       if(GetBit(rc->readMap, rc->readPos+i))
		  ok++;
	 }
	 
	 /* else query dead sectors from image */
	 
	 else
	 {  if(!LargeSeek(rc->readerImage, (gint64)(2048*rc->readPos)))
	       Stop(_("Failed seeking to sector %lld in image [%s]: %s"),
		    rc->readPos, "reread", strerror(errno));

	    if(rc->readPos+nsectors > rc->readMarker)
	       num_compare = rc->readMarker-rc->readPos;

	    for(i=0; i<num_compare; i++)
	    {  unsigned char sector_buf[2048];
	       int err;

	       n = LargeRead(rc->readerImage, sector_buf, 2048);
	       if(n != 2048)
		  Stop(_("unexpected read error in image for sector %lld"),rc->readPos);
	       err = CheckForMissingSector(sector_buf, rc->readPos+i,
					   rc->image->fpState == 2 ? rc->image->imageFP : NULL,
					   rc->image->fpSector);
	       if(err != SECTOR_PRESENT)
		 ExplainMissingSector(sector_buf, rc->readPos+i, err, SOURCE_IMAGE, &unrecoverable_sectors);
	       else
	       {  if(!rc->crcBuf
		     || CheckAgainstCrcBuffer(rc->crcBuf, rc->readPos+i, sector_buf) != CRC_BAD)
		  {  ok++;  /* CRC unavailable or good */
		     if(rc->readMap)
		       SetBit(rc->readMap, rc->readPos+i);
		  }
	       }
	    }
	 }

	 if(ok == nsectors)  /* All sectors already present. */
	 {
	   goto step_counter;
	 }
	 else                /* Some sectors still missing */
	 {
           if(nsectors > 1 && ok > 0)
	   {  nsectors = 1;
	      goto reread;
	   }
	 }
      }

      /*** Try reading the next <nsectors> sector(s). */

      g_mutex_lock(rc->mutex);
      if(rc->workerError)       /* something went wrong in the worker thread */
      {	g_mutex_unlock(rc->mutex);
	Stop(rc->workerError);
      }
      while(rc->bufState[rc->readPtr] != BUF_EMPTY)
      {  g_cond_wait(rc->canRead, rc->mutex);
      }
      g_mutex_unlock(rc->mutex);

      status = ReadSectors(rc->image->dh, rc->alignedBuf[rc->readPtr]->buf, rc->readPos, nsectors);

      /*** Medium Error (3) and Illegal Request (5) may result from 
	   a medium read problem, but other errors are regarded as fatal. */

      if(status && !Closure->ignoreFatalSense 
	 && rc->image->dh->sense.sense_key && rc->image->dh->sense.sense_key != 3 && rc->image->dh->sense.sense_key != 5)
      {  int answer;

	 if(!Closure->guiMode)
	    Stop(_("Sector %lld: %s\nCan not recover from above error.\n"
		   "Use the --ignore-fatal-sense option to override."),
		 rc->readPos, GetLastSenseString(FALSE));

	 answer = ModalDialog(GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, insert_buttons,
			      _("Sector %lld: %s\n\n"
				"It may not be possible to recover from this error.\n"
				"Should the reading continue and ignore this error?"),
			      rc->readPos, GetLastSenseString(FALSE));

	 if(answer == 2)
	   Closure->ignoreFatalSense = 2;

	 if(!answer)
	 {  SwitchAndSetFootline(Closure->readLinearNotebook, 1, Closure->readLinearFootline, 
				_("<span %s>Aborted by user request!</span> %lld sectors read, %lld sectors unreadable/skipped so far."),
				 Closure->redMarkup, rc->readOK,Closure->readErrors); 
	    rc->unreportedError = FALSE;  /* suppress respective error message */
	    goto terminate;
	 }
      }

      /*** Evaluate C2 scan results */

      if(rc->image->dh->canC2Scan)
      {  int i;
	 
	 for(i=0; i<nsectors; i++)
	 {  if(rc->image->dh->c2[i])
	    {  if(!status)  /* Do not print C2 and error messages together */
		  PrintCLI(_("Sector %lld: %3d C2 errors.%s\n"), 
			   rc->readPos+i, rc->image->dh->c2[i], "              ");

	       if(rc->image->dh->c2[i] > rc->maxC2)  /* remember highest value */
		  rc->maxC2 = rc->image->dh->c2[i];
	    }
	 }
      }

      /*** Warn the user if we see dead sector markers on the image. */

      if(!status)
      {  for(i=0; i<nsectors; i++)
	 {  unsigned char *sector_buf = rc->alignedBuf[rc->readPtr]->buf;
	    int err;

	    /* Note: providing the fingerprint is not necessary as any 
	       incoming missing sector marker indicates a huge problem. */

	    err = CheckForMissingSector(sector_buf+i*2048, rc->readPos+i,
					rc->image->fpState == 2 ? rc->image->imageFP : NULL,
					rc->image->fpSector);
	    if(err != SECTOR_PRESENT)
	      ExplainMissingSector(sector_buf+i*2048, rc->readPos+i, err, SOURCE_MEDIUM, &unrecoverable_sectors);
	 }
      }

      /*** Pass sector(s) to the worker thread (if reading succeeded) */

      if(!status)
      {  gint64 sidx;

	 /* Mark the sectors as read (preliminary).
	    The worker thread may later reset the bit if it finds
	    a CRC error. Careful: Do this here before the worker
	    is invoked; else we get a nice race condition setting/
	    unsetting the bit on CRC errors */

	 if(rc->readMap)  
	    for(sidx=rc->readPos, i=0; i<nsectors; sidx++,i++)
	       SetBit(rc->readMap, sidx);

	 /* Kick off the worker thread */

	 g_mutex_lock(rc->mutex);
	 rc->bufferedSector[rc->readPtr] = rc->readPos;
	 rc->nSectors[rc->readPtr] = nsectors;
	 rc->bufState[rc->readPtr] = BUF_FULL;
	 rc->readPtr++;
	 if(rc->readPtr >= READ_BUFFERS)
	    rc->readPtr = 0;
	 g_cond_signal(rc->canWrite);
	 g_mutex_unlock(rc->mutex);
	 
	 rc->readOK += nsectors;

      }

      /*** Process the read error if reading failed. */

      if(status)
      {  int nfill;

	 /* Disable on the fly checksum calculation.
	    Do NOT free the CRC cache here to avoid race condition
	    with the worker thread! */

	 if(Closure->crcCache)
	    rc->doMD5sums = FALSE;

	 /* Determine number of sectors to skip forward. 
	    Make sure not to skip past the media end
	    and to land at a multiple of dh->clusterSize. */

	 if(nsectors>=Closure->sectorSkip) nfill = nsectors;
	 else
	 {  int skip = rc->image->dh->clusterSize > Closure->sectorSkip ? rc->image->dh->clusterSize : Closure->sectorSkip;
	    if(rc->readPos+skip > rc->lastSector) nfill = rc->lastSector-rc->readPos+1;
	    else nfill = skip - ((rc->readPos + skip) & cluster_mask);
	 }

	 /* If we are reading past the read marker we must take care 
            to fill up any holes with dead sector markers before skipping forward. 
	    When sectorSkip is 0 and nsectors > 1, we will re-read all these sectors
	    again one by one, so we catch this case in order not to write the markers twice.  */

	 if(!rc->scanMode && rc->readPos+nfill > rc->readMarker
	    && (Closure->sectorSkip || nsectors == 1))
	 {  int i;

	    /* Write nfill of "dead sector" markers so that
	       they are tried again in the following iterations / sessions. */

	    for(i=0; i<nfill; i++)
	    {
	       g_mutex_lock(rc->mutex);
	       if(rc->workerError)       /* something went wrong in the worker thread */
	       {  g_mutex_unlock(rc->mutex);
		  Stop(rc->workerError);
	       }
	       
	       while(rc->bufState[rc->readPtr] != BUF_EMPTY)
	       {  g_cond_wait(rc->canRead, rc->mutex);
	       }

	       CreateMissingSector(rc->alignedBuf[rc->readPtr]->buf, rc->readPos+i, 
				   rc->image->imageFP, FINGERPRINT_SECTOR, rc->volumeLabel);

	       rc->bufferedSector[rc->readPtr] = rc->readPos+i;
	       rc->nSectors[rc->readPtr] = 1;
	       rc->bufState[rc->readPtr] = BUF_DEAD;
	       rc->readPtr++;
	       if(rc->readPtr >= READ_BUFFERS)
		 rc->readPtr = 0;
	       g_cond_signal(rc->canWrite);
	       g_mutex_unlock(rc->mutex);
	    }
	 }

	 /* If sectorSkip is set, perform the skip.
	    nfill has been calculated so that the skip lands
	    at a multiple of dh->clusterSize. Therefore nsectors can remain
	    at its former value as skipping forward will not 
	    destroy cluster size aligned reads. 
	    The nsectors>1 case can only happen when processing the tao_tail. */

	 if(Closure->sectorSkip && nsectors > 1)
	 {  int i;

	    PrintCLIorLabel(Closure->status,
			    _("Sector %lld: %s Skipping %d sectors.\n"),
			    rc->readPos, GetLastSenseString(FALSE), nfill-1);  
	    for(i=0; i<nfill; i++)         /* workaround: large values for nfill */
	    {  Closure->readErrors++;      /* would exceed sampling of green/red */
	       rc->readPos++;              /* in the spiral. Internal NOTE01 */
	       show_progress(rc);
	    }
	    rc->readPos-=nsectors;         /* nsectors will be added again after the goto */
#if 0
	    Closure->readErrors+=nfill;    /* pre-workaround code */
	    rc->readPos+=nfill-nsectors;   /* nsectors will be added again after the goto */
#endif
	    goto step_counter;
	 }

	 /* However, if no sector skipping is requested
	    and we are currently in fast read mode (nsectors > 1),
	    slow down to reading 1 sectors at once
	    and try to re-read the first sector. */

	 else 
	 {
	    if(nsectors > 1)
	    {  nsectors = 1;
	       goto reread;
	    }
	    else 
	    {  PrintCLIorLabel(Closure->status,
			       _("Sector %lld: %s\n"),
			       rc->readPos, GetLastSenseString(FALSE));  
	       if(rc->readPos >= rc->image->dh->sectors - 2) tao_tail++;
	       Closure->readErrors++;
	    }
	 }
      }

      /*** Step the progress counter */

step_counter:
      rc->readPos += nsectors;   /* advance the reading position */

      show_progress(rc);
   }

   /*** If multiple reading passes are allowed, see if we need another pass.
        Note: Checksum errors do not trigger another pass as only sectors
        marked dead are tried on a re-read. This does not hurt as being able
        to checksum means we have ecc data - we can fix the image using ecc
        rather than by re-reading it. */

   if(Closure->guiMode)
     ChangeSpiralCursor(Closure->readLinearSpiral, -1); /* switch cursor off */

   rc->pass++;
   rc->image->dh->pass = rc->pass;
   
   if(   !rc->scanMode
      && (Closure->readErrors || Closure->crcErrors)
      && rc->pass < Closure->readingPasses)
   {  int renderers_left = TRUE;
       
      if(Closure->guiMode)
	   SetLabelText(GTK_LABEL(Closure->readLinearHeadline), 
			_("<big>Trying to complete image, reading pass %d of %d.</big>\n%s"),
			rc->pass+1, Closure->readingPasses, rc->image->dh->mediumDescr);
      else PrintCLI(_("\nTrying to complete image, reading pass %d of %d.\n"),
		    rc->pass+1, Closure->readingPasses);


     /* Make sure that all spiral renderers have finished before resetting
	and starting the next iteration. */

    while(renderers_left)
    {  g_mutex_lock(rc->rendererMutex);
       if(rc->activeRenderers<=0)
	 renderers_left=FALSE; 
       g_mutex_unlock(rc->rendererMutex);
       
       g_usleep(G_USEC_PER_SEC/10);
    }
     
    goto next_reading_pass;
   }

   /*** Signal EOF to writer thread; wait for it to finish */

   send_eof(rc);
   g_thread_join(rc->worker);
   rc->worker = NULL;

   /*** Finalize on-the-fly checksum calculation */

   if(rc->doMD5sums && rc->eccMethod)
        md5_failure = rc->eccMethod->finalizeCksums(rc->image);
   else ClearCrcCache();  /* deferred until here to avoid race condition */

   Verbose("CRC %s.\n", Closure->crcCache ? "cached" : "NOT created");

   /*** Print summary */

   t = NULL;

   /* We were re-reading an incomplete image */

   if(rc->rereading)
   {  if(!Closure->readErrors) t = g_strdup_printf(_("%lld sectors read.     "),rc->readOK);
      else                     t = g_strdup_printf(_("%lld sectors read; %lld unreadable sectors."),rc->readOK,Closure->readErrors);
   }

   /* We were reading the image for the first time */

   else  /* not rereading */
   {  /* Image was fully readable and had no CRC errors (if CRC was available at all!).
         So we had no faulty sectors, but ... */
      if(!Closure->readErrors && !Closure->crcErrors) 
      {
	 if(rc->image->eccFile)  /* ...maybe wrong image size? */
	 {  if(rc->image->dh->sectors != rc->image->expectedSectors)
	       t = g_strdup_printf(_("All sectors successfully read, but wrong image length (%lld sectors difference)"), rc->image->dh->sectors - rc->image->expectedSectors);
	 }
	                  /* ...or bad ecc md5 sum (theoretically impossible by now)? */
	 if(   rc->readOK == rc->image->dh->sectors  /* no user limited range */  
	    && rc->pass == 1              /* md5sum invalid after first reading pass */
	    && md5_failure && !t)         /* and we're signalled a mismatch */
	 {  t = g_strdup(_("All sectors successfully read, but wrong ecc md5sum."));
	 }

	 if(!t) 
	 {  if(rc->eccMethod) /* we had CRC sums to compare against */
	     {  if(rc->crcIncomplete)
		      t = g_strdup(_("All sectors successfully read, but incomplete or damaged checksums."));
	        else  t = g_strdup(_("All sectors successfully read. Checksums match."));
	     }
	     else t = g_strdup(_("All sectors successfully read."));
	 }
      }
      else /* we have unreadable or damaged sectors */
      {  if(Closure->readErrors && !Closure->crcErrors)
	      t = g_strdup_printf(_("%lld unreadable sectors."),Closure->readErrors);
         else if(!Closure->readErrors && Closure->crcErrors)
	 {  if(md5_failure & CRC_MD5_BAD)
	       t = g_strdup_printf(_("%lld CRC errors and a md5sum mismatch in the CRC section."),Closure->crcErrors);
	    else
	       t = g_strdup_printf(_("%lld CRC errors."),Closure->crcErrors);
	 }
	 else t = g_strdup_printf(_("%lld CRC errors, %lld unreadable sectors."),
				  Closure->crcErrors, Closure->readErrors);
      }
   }

   PrintLog("\n%s\n",t);
   if(Closure->guiMode)
   {  if(rc->scanMode) SwitchAndSetFootline(Closure->readLinearNotebook, 1, Closure->readLinearFootline, 
					 "%s%s",_("Scanning finished: "),t);
      else             SwitchAndSetFootline(Closure->readLinearNotebook, 1, Closure->readLinearFootline, 
					 "%s%s",_("Reading finished: "),t);
   }
   if(t) g_free(t);

   if(!Closure->fixedSpeedValues)
     PrintTimeToLog(rc->readTimer, "for reading/scanning.\n");

   if(rc->image->dh->mainType == CD && tao_tail && tao_tail == Closure->readErrors && !Closure->noTruncate)
   {  int answer;
   
      if(Closure->guiMode)
        answer = ModalWarning(GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL, NULL,
			      _("%d sectors missing at the end of the disc.\n"
				"This is okay if the CD was written in TAO (track at once) mode.\n"
				"The Image will be truncated accordingly. See the manual for details.\n"),
			      tao_tail);
      else 
        answer = ModalWarning(GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL, NULL,
			      _("%d sectors missing at the end of the disc.\n"
				"This is okay if the CD was written in TAO (track at once) mode.\n"
				"The Image will be truncated accordingly. See the manual for details.\n"
				"Use the --dao option to disable image truncating.\n"),
			      tao_tail);
     
      if(!rc->scanMode && answer)
        if(!LargeTruncate(rc->writerImage, (gint64)(2048*(rc->image->dh->sectors-tao_tail))))
	  Stop(_("Could not truncate %s: %s\n"),Closure->imageName,strerror(errno));
   }
   else if(Closure->readErrors) exitCode = EXIT_FAILURE;

   /*** Eject medium */

   if(Closure->eject && !Closure->readErrors)
      LoadMedium(rc->image->dh, FALSE);

   /*** Close and clean up */

   rc->unreportedError = FALSE;
   rc->earlyTermination = FALSE;

terminate:
   cleanup((gpointer)rc);
}
