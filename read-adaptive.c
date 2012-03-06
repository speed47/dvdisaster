/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2004-2011 Carsten Gnoerlich.
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

#include "scsi-layer.h"
#include "udf.h"

#include "rs02-includes.h"

//#define CHECK_VISITED 1        /* This gives only reasonable output            */
                                 /* when the .ecc file is present while reading! */ 

/***
 *** Local data package used during reading
 ***/

enum { IMAGE_ONLY, ECC_IN_FILE, ECC_IN_IMAGE };

typedef struct
{  DeviceHandle *dh;            /* device we are reading from */
   gint64 sectors;              /* sectors in medium (maybe cooked value from file system) */
   LargeFile *image;            /* image file */
   int readMode;                /* see above enum for choices */
   EccInfo *ei;                 /* ecc info and header struct */
   EccHeader *eh;               /* if ecc information is present */
   int rs01LayerSectors;        /* length of each RS01 ecc layer in sectors */
   RS02Layout *lay;             /* layout of RS02 type ecc data */

   AlignedBuffer *ab;           /* buffer suitable for reading from the drive */
   unsigned char *buf;          /* buffer component from above */
   Bitmap *map;                 /* bitmap for keeping track of read sectors */
   CrcBuf *crcBuf;              /* preloaded CRC info from ecc data */

   unsigned char *fingerprint;  /* needed for missing sector */
   char *volumeLabel;           /* generation */

   gint64 readable;             /* current outcome of reading process */
   gint64 unreadable;
   gint64 correctable;

   gint64 firstSector;          /* user limited reading range */
   gint64 lastSector;

   gint64 *intervals;           /* queue for keeping track of unread intervals */
   gint64 maxIntervals;
   gint64 nIntervals;

   gint64 intervalStart;        /* information about currently processed interval */
   gint64 intervalEnd;
   gint64 intervalSize;
   gint64 highestWrittenSector; /* current size of image file */ 
 
   char progressMsg[256];       /* message output related */
   char progressBs[256];
   char progressSp[256];
   int  progressMsgLen;
   int  lastPercent;            /* used to determine next progress update */
   gint64 lastUnreadable;       /* used to find out whether something changed */
   gint64 lastCorrectable;      /* since last progress output */
   char *subtitle;              /* description of reading mode */
 
   int sectorsPerSegment;       /* number of sectors per spiral segment */
   int *segmentState;           /* tracks whether all sectors within segment are processed */

   int earlyTermination;        /* information about termination cause */

#ifdef CHECK_VISITED
   char *count;
#endif

} read_closure;

static void cleanup(gpointer data)
{  read_closure *rc = (read_closure*)data;

   Closure->cleanupProc = NULL;

   /* Reset temporary ignoring of fatal errors.
      User has to set this in the preferences to make it permanent. */

   if(Closure->ignoreFatalSense == 2)
      Closure->ignoreFatalSense = 0;

   /* Rewrite the header sectors if we were reading an RS02 image;
      otherwise the image will not be recognized later. */

   if(rc->readMode == ECC_IN_IMAGE)   
   {  RS02Layout *lay = rc->lay;
      guint64 hpos;
      guint64 end = lay->eccSectors+lay->dataSectors;

      if(rc->highestWrittenSector+1 < end)  /* image may have been partially read */
	end = rc->highestWrittenSector+1;

      /* Careful: we must not call Stop() here to avoid infinite
	 recursion in error situations. */

      if(lay->firstEccHeader > end)  /* ecc area not reached during read? */
	goto bail_out;

      if(!LargeSeek(rc->image, 2048*lay->firstEccHeader))
	goto bail_out;
   
      if(LargeWrite(rc->image, rc->eh, sizeof(EccHeader)) != sizeof(EccHeader))
	goto bail_out;

      hpos = (lay->protectedSectors + lay->headerModulo - 1) / lay->headerModulo;
      hpos *= lay->headerModulo;

      while(hpos < end)
      {  if(!LargeSeek(rc->image, 2048*hpos))
	  break;

	if(LargeWrite(rc->image, rc->eh, sizeof(EccHeader)) != sizeof(EccHeader))
	  break;

	hpos += lay->headerModulo;
      }
   }

bail_out:
   if(Closure->guiMode)
   {  if(rc->earlyTermination)
        SetAdaptiveReadFootline(_("Aborted by unrecoverable error."), Closure->redText);

      AllowActions(TRUE);
   }

   if(rc->image)   
     if(!LargeClose(rc->image))
       Stop(_("Error closing image file:\n%s"), strerror(errno));
   if(rc->dh)      CloseDevice(rc->dh);
 
   if(rc->ei) FreeEccInfo(rc->ei);

   if(rc->subtitle) g_free(rc->subtitle);
   if(rc->segmentState) g_free(rc->segmentState);

   if(rc->ab) FreeAlignedBuffer(rc->ab);
   
   if(rc->intervals) g_free(rc->intervals);
   if(rc->crcBuf) FreeCrcBuf(rc->crcBuf);

   if(rc->fingerprint) g_free(rc->fingerprint);
   if(rc->volumeLabel) g_free(rc->volumeLabel);

   if(rc->map)
     FreeBitmap(rc->map);

   g_free(rc);

   if(Closure->guiMode)
      g_thread_exit(0);
}

/***
 *** Sorted queue of unread intervals
 ***/

/*
 * Sort new interval into the queue
 */

static void add_interval(read_closure *rc, gint64 start, gint64 size)
{  int i,si;
  
  /* Trivial case: empty interval list */

  if(rc->nIntervals == 0)
  {  rc->intervals[0] = start;
     rc->intervals[1] = size;
     rc->nIntervals++;
     return;
  }

  /* Find insertion place in list */

  for(i=0,si=0; i<rc->nIntervals; i++,si+=2)
    if(size > rc->intervals[si+1])
      break;

  /* Make sure we have enough space in the array */

  rc->nIntervals++;
  if(rc->nIntervals > rc->maxIntervals)
  {  rc->maxIntervals *= 2;
     
     rc->intervals = g_realloc(rc->intervals, rc->maxIntervals*2*sizeof(gint64));
  }


  /* Shift unless we insert at the list tail */

  if(i<rc->nIntervals-1)
    memmove(rc->intervals+si+2, rc->intervals+si, 2*sizeof(gint64)*(int)(rc->nIntervals-i-1));

  /* Add new pair into the list */
  
  rc->intervals[si]   = start;
  rc->intervals[si+1] = size;
}

/*
 * Remove first element from the queue
 */

static void pop_interval(read_closure *rc)
{  
  if(rc->nIntervals > 0)
  {  rc->nIntervals--;
     memmove(rc->intervals, rc->intervals+2, 2*sizeof(gint64)*(int)rc->nIntervals);
  }
}

/*
 * Print the queue (for debugging purposes only)
 */

void print_intervals(read_closure *rc)
{  int i;

   printf("%lld Intervals:\n", (long long int)rc->nIntervals);
   for(i=0; i<rc->nIntervals; i++)
     printf("%7lld [%7lld..%7lld]\n",
	    (long long int)rc->intervals[2*i+1], 
	    (long long int)rc->intervals[2*i], 
	    (long long int)rc->intervals[2*i]+rc->intervals[2*i+1]-1);
}

/***
 *** Convenience functions for printing the progress message
 ***/

static void print_progress(read_closure *rc, int immediate)
{  int n;
   int total = rc->readable+rc->correctable;
   int percent = (int)((1000LL*(long long)total)/rc->sectors);

   if(Closure->guiMode)
      return;

   if(   rc->lastPercent >= percent 
      && rc->lastCorrectable == rc->correctable
      && rc->lastUnreadable  == rc->unreadable
      && !immediate)
     return;

   rc->lastPercent = percent;
   rc->lastCorrectable = rc->correctable;
   rc->lastUnreadable  = rc->unreadable;

   if(rc->ei)
     n = g_snprintf(rc->progressMsg, 256,
		    _("Repairable: %2d.%1d%% (correctable: %lld; now reading [%lld..%lld], size %lld)"),
		    percent/10, percent%10, rc->correctable, 
		    rc->intervalStart, rc->intervalStart+rc->intervalSize-1, rc->intervalSize);
   else
     n = g_snprintf(rc->progressMsg, 256,
		    _("Repairable: %2d.%1d%% (missing: %lld; now reading [%lld..%lld], size %lld)"),
		    percent/10, percent%10, rc->sectors-rc->readable, 
		    rc->intervalStart, rc->intervalStart+rc->intervalSize-1, rc->intervalSize);

   if(n>255) n = 255;

   /* If the new message is shorter, overwrite old message with spaces */

   if(rc->progressMsgLen > n)
   {  rc->progressSp[rc->progressMsgLen] = 0;
      rc->progressBs[rc->progressMsgLen] = 0;
      PrintCLI("%s%s", rc->progressSp, rc->progressBs);
      rc->progressSp[rc->progressMsgLen] = ' ';
      rc->progressBs[rc->progressMsgLen] = '\b';
   }

   /* Write new message */

   rc->progressBs[n] = 0;
   PrintCLI("%s%s", rc->progressMsg, rc->progressBs);
   rc->progressBs[n] = '\b';



   rc->progressMsgLen = n;
}

static void clear_progress(read_closure *rc)
{
   if(!rc->progressMsgLen || Closure->guiMode)
     return;

   rc->progressSp[rc->progressMsgLen] = 0;
   PrintCLI("%s", rc->progressSp);
   rc->progressSp[rc->progressMsgLen] = ' ';

   rc->progressBs[rc->progressMsgLen] = 0;
   PrintCLI("%s", rc->progressBs);
   rc->progressBs[rc->progressMsgLen] = '\b';

   rc->progressMsgLen = 0;
}

/*
 * Sector markup in the spiral
 */

static void mark_sector(read_closure *rc, gint64 sector, GdkColor *color)
{  int segment;
   int changed = FALSE;

   if(!Closure->guiMode) return;

   segment = sector / rc->sectorsPerSegment;

   if(color)
   {  GdkColor *old = Closure->readAdaptiveSpiral->segmentColor[segment];
      GdkColor *new = old;

      if(color == Closure->redSector && old != Closure->redSector)
	new = color;

      if(   color == Closure->yellowSector
	 && old != Closure->redSector
	 && old != Closure->yellowSector)
	new = color;

      if(color == Closure->greenSector)
      {  rc->segmentState[segment]++;

	 if(rc->segmentState[segment] >= rc->sectorsPerSegment)
	   new = color;

	 /* Last segment represents less sectors */

	 if(   segment == Closure->readAdaptiveSpiral->segmentClipping - 1
	    && rc->segmentState[segment] >= rc->sectors % rc->sectorsPerSegment)
	   new = color;
      }

      if(new != old)
      {  ChangeSegmentColor(color, segment);
	 changed = TRUE;
      }
   }
   else changed = TRUE;

   if(changed)
     UpdateAdaptiveResults(rc->readable, rc->correctable, 
			   rc->sectors-rc->readable-rc->correctable,
			   (int)((1000LL*(rc->readable+rc->correctable))/rc->sectors));
}

/***
 *** Basic device and image handling and sanity checks.
 ***/

/*
 * Open CD/DVD device and .ecc file.
 * Determine reading mode.
 */

static void open_and_determine_mode(read_closure *rc)
{  unsigned char fp[16];
 
   /* open the device */

   rc->dh = OpenAndQueryDevice(Closure->device);
   rc->readMode = IMAGE_ONLY;

   /* save some useful information for the missing sector marker */

   if(GetMediumFingerprint(rc->dh, fp, FINGERPRINT_SECTOR))
   {  rc->fingerprint = g_malloc(16);
      memcpy(rc->fingerprint, fp, 16);
   }

   if(rc->dh->isoInfo && rc->dh->isoInfo->volumeLabel[0])
      rc->volumeLabel = g_strdup(rc->dh->isoInfo->volumeLabel);

   /* See if we have ecc information available. 
      Prefer the error correction file over augmented images if both are available. */

   /* See if the medium contains RS02 type ecc information */

   rc->ei = OpenEccFile(READABLE_ECC | PRINT_MODE);
   if(rc->ei)   /* RS01 type ecc */
   {  rc->readMode = ECC_IN_FILE;
      rc->eh = rc->ei->eh;

      rc->rs01LayerSectors = (rc->ei->sectors+rc->eh->dataBytes-1)/rc->eh->dataBytes;

      SetAdaptiveReadMinimumPercentage((1000*(rc->eh->dataBytes-rc->eh->eccBytes))/rc->eh->dataBytes);
   }
   else if(rc->dh->rs02Header)  /* see if we have RS02 type ecc */
   {  rc->readMode = ECC_IN_IMAGE;
      rc->eh  = rc->dh->rs02Header;
      rc->lay = CalcRS02Layout(uchar_to_gint64(rc->eh->sectors), rc->eh->eccBytes);
 
      SetAdaptiveReadMinimumPercentage((1000*rc->lay->ndata)/255);

      if(Closure->verbose)  /* for testing purposes */
      {  gint64 s,sinv,slice,idx;

	 for(s=0; s<rc->dh->sectors-1; s++)
	 {  RS02SliceIndex(rc->lay, s, &slice, &idx);
	    sinv = RS02SectorIndex(rc->lay, slice, idx);

	    if(slice == -1)
	       Verbose("Header %lld found at sector %lld\n", idx, s);
	    else
	    if(s != sinv) Verbose("Failed for sector %lld / %lld:\n"
				  "slice %lld, idx %lld\n",
				  s, sinv, slice, idx);
	 }
	 Verbose("RS02SliceIndex() verification finished.\n");
      }
   }

   /* Pick suitable message */

   switch(rc->readMode)
   {  case IMAGE_ONLY:
        rc->subtitle = g_strdup_printf(_("Stopping when unreadable intervals < %d."), 
				       Closure->sectorSkip);
	PrintLog(_("Adaptive reading: %s\n"), rc->subtitle); 
	break;

      case ECC_IN_FILE:
      case ECC_IN_IMAGE:
	rc->subtitle = g_strdup(_("Trying to collect enough data for error correction."));
	PrintLog(_("Adaptive reading: %s\n"), rc->subtitle);  
	break;
   }
}

/*
 * Validate image size against length noted in the ecc header
 */

static void check_size(read_closure *rc)
{  
   /* Number of sectors depends on ecc data */
 
   switch(rc->readMode)
   {  case IMAGE_ONLY:
        rc->sectors = rc->dh->sectors;
	return;

      case ECC_IN_FILE:
	rc->sectors = rc->ei->sectors;
	break;

      case ECC_IN_IMAGE:
	rc->sectors = rc->lay->eccSectors + rc->lay->dataSectors;
	break;
   }

   /* Compare size with answer from drive */

   if(rc->sectors < rc->dh->sectors)
   {  int answer;

      answer = ModalWarning(GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL, NULL,
			    _("Medium contains %lld sectors more as recorded in the .ecc file\n"
			      "(Medium: %lld sectors; expected from .ecc file: %lld sectors).\n"
			      "Only the first %lld medium sectors will be processed.\n"),
			    rc->dh->sectors-rc->sectors, rc->dh->sectors, rc->sectors,
			    rc->sectors);

      if(!answer)
      {  SetAdaptiveReadFootline(_("Aborted by user request!"), Closure->redText);
	 rc->earlyTermination = FALSE;
	 cleanup((gpointer)rc);
      }
   }

   if(rc->sectors > rc->dh->sectors)
   {  int answer;

      answer =  ModalWarning(GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL, NULL,
			     _("Medium contains %lld sectors less as recorded in the .ecc file\n"
			       "(Medium: %lld sectors; expected from .ecc file: %lld sectors).\n"),
			     rc->sectors-rc->dh->sectors, rc->dh->sectors, rc->sectors,
			     rc->sectors);

      if(!answer)
      {  SetAdaptiveReadFootline(_("Aborted by user request!"), Closure->redText);
	 rc->earlyTermination = FALSE;
	 cleanup((gpointer)rc);
      }

      rc->sectors = rc->dh->sectors;
   }
}

/*
 * Limit reading to user selected range
 */

void GetReadingRange(gint64 sectors, gint64 *firstSector, gint64 *lastSector)
{  gint64 first, last;

   if(Closure->readStart || Closure->readEnd)
   {  if(!Closure->guiMode) /* more range checks are made below */ 
      {  first = Closure->readStart;
         last  = Closure->readEnd < 0 ? sectors-1 : Closure->readEnd;
      }
      else  /* be more permissive in GUI mode */
      {  first = 0;
 	 last  = sectors-1;

	 if(Closure->readStart <= Closure->readEnd)
	 {  first = Closure->readStart < sectors ? Closure->readStart : sectors-1;
	    last  = Closure->readEnd   < sectors ? Closure->readEnd   : sectors-1;
	 }
      }

      if(first > last || first < 0 || last >= sectors)
	Stop(_("Sectors must be in range [0..%lld].\n"), sectors-1);

      PrintLog(_("Limiting sector range to [%lld,%lld].\n"), first, last);
   }
   else 
   {  first = 0; 
      last  = sectors-1;
   }

   *firstSector = first;
   *lastSector = last;
}

/*
 * Compare medium fingerprint against fingerprint stored in error correction file 
 */

static void check_ecc_fingerprint(read_closure *rc)
{  guint8 digest[16];
   int fp_read;

   fp_read = GetMediumFingerprint(rc->dh, digest, rc->eh->fpSector);

   if(!fp_read) /* Not readable. Bad luck. */
   {  int answer;

      answer = ModalWarning(GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL, NULL,
			    _("Sector %d is missing. Can not compare medium and ecc fingerprints.\n"
			      "Double check that the medium and the ecc file belong together.\n"),
			    rc->eh->fpSector);

      if(!answer)
      {  SetAdaptiveReadFootline(_("Aborted by user request!"), Closure->redText);
	 rc->earlyTermination = FALSE;
	 cleanup((gpointer)rc);
      }
   }
   else 
   {  
      if(memcmp(digest, rc->eh->mediumFP, 16))
	Stop(_("Fingerprints of medium and ecc file do not match.\n"
	       "Medium and ecc file do not belong together.\n"));
   }
}

/*
 * Compare image fingerprint with medium.
 */

int check_image_fingerprint(read_closure *rc)
{  struct MD5Context md5ctxt;
   guint8 image_fp[16], medium_fp[16];
   gint32 fingerprint_sector;
   int fp_read,n;
  
   /* Determine fingerprint sector */

   if(rc->eh)  /* If ecc information is present get fp sector number from there */
        fingerprint_sector = rc->eh->fpSector;
   else fingerprint_sector = FINGERPRINT_SECTOR;

   /* Try to read fingerprint sectors from medium and image */

   if(!LargeSeek(rc->image, (gint64)(2048*fingerprint_sector)))
     return 0; /* can't tell, assume okay */

   n = LargeRead(rc->image, rc->buf, 2048);
   MD5Init(&md5ctxt);
   MD5Update(&md5ctxt, rc->buf, 2048);
   MD5Final(image_fp, &md5ctxt);

   fp_read = GetMediumFingerprint(rc->dh, medium_fp, fingerprint_sector);

   if(n != 2048 || !fp_read || (CheckForMissingSector(rc->buf, fingerprint_sector, NULL, 0) != SECTOR_PRESENT))
     return 0; /* can't tell, assume okay */

   /* If both could be read, compare them */

   if(memcmp(image_fp, medium_fp, 16))
   {  	  
     if(!Closure->guiMode)
       Stop(_("Image file does not match the CD/DVD."));
     else
     {  int answer = ModalDialog(GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL, NULL,
				 _("Image file already exists and does not match the CD/DVD.\n"
				   "The existing image file will be deleted."));
	   
        if(!answer)
	{  rc->earlyTermination = FALSE;
	   SetAdaptiveReadFootline(_("Reading aborted. Please select a different image file."), 
				   Closure->redText);
	   cleanup((gpointer)rc);
	}
	else
	{  LargeClose(rc->image);
	   LargeUnlink(Closure->imageName);
	   return TRUE; /* causes reopen of image in caller */
	} 
     }
   }

   return 0;  /* okay */
}

/*
 * Compare image size with medium.
 * TODO: image with byte sizes being not a multiple of 2048.
 */

void check_image_size(read_closure *rc, gint64 image_file_sectors)
{  
   if(image_file_sectors > rc->sectors)
   {  int answer;
	 
      answer = ModalWarning(GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL, NULL,
			    _("Image file is %lld sectors longer than inserted medium\n"
			      "(Image file: %lld sectors; medium: %lld sectors).\n"),
			    image_file_sectors-rc->sectors, image_file_sectors, rc->sectors);

      if(!answer)
      {  SetAdaptiveReadFootline(_("Aborted by user request!"), Closure->redText);
	 rc->earlyTermination = FALSE;
	 cleanup((gpointer)rc);
      }

      rc->highestWrittenSector = rc->sectors-1;
   }
   else rc->highestWrittenSector = image_file_sectors-1;
}

/***
 *** Load the crc buf from RS01/RS02 error correction data.
 ***/

static void load_crc_buf(read_closure *rc)
{
   switch(rc->readMode)
   {  case ECC_IN_FILE:
	 SetAdaptiveReadSubtitle(_utf("Loading CRC data."));
	 rc->crcBuf = GetCRCFromRS01(rc->ei);
	 break;
      case ECC_IN_IMAGE:
	 SetAdaptiveReadSubtitle(_utf("Loading CRC data."));
	 rc->crcBuf = GetCRCFromRS02(rc->lay, rc->dh, rc->image);
	 break;
      default:
	 rc->crcBuf = NULL;
	 break;
   }
}

/***
 *** Examine existing image file.
 ***
 * Build an initial interval list from it.
 */

static void build_interval_from_image(read_closure *rc)
{  gint64 s;
   gint64 first_missing, last_missing, current_missing;
   int tail_included = FALSE;
   int last_percent = 0;
   int crc_result;

   /*** Rewind image file */

   LargeSeek(rc->image, 0);
   first_missing = last_missing = -1;

   /*** Go through all sectors in the image file.
	Check them for "dead sector markers" 
	and for checksum failures if ecc data is present. */
   
   if(Closure->guiMode)
     SetAdaptiveReadSubtitle(_("Analysing existing image file"));

   for(s=0; s<=rc->highestWrittenSector; s++)
   {  int n,percent;

      /* Check for user interruption. */

      if(Closure->stopActions)   
      {  SetAdaptiveReadFootline(_("Aborted by user request!"), Closure->redText);
	 rc->earlyTermination = FALSE;
	 cleanup((gpointer)rc);
      }

      /* Read the next sector */

      n = LargeRead(rc->image, rc->buf, 2048);
      if(n != 2048) /* && (s != rc->sectors - 1 || n != ii->inLast)) */
	Stop(_("premature end in image (only %d bytes): %s\n"),n,strerror(errno));

      /* Look for the dead sector marker */

      current_missing = CheckForMissingSector(rc->buf, s, NULL, 0);

      if(current_missing)
      {  mark_sector(rc, s, Closure->redSector);
	 ExplainMissingSector(rc->buf, s, current_missing, TRUE);
      }

      /* Compare checksums if available */

      if(rc->crcBuf)
	   crc_result = CheckAgainstCrcBuffer(rc->crcBuf, s, rc->buf);
      else crc_result = CRC_UNKNOWN;

      switch(crc_result)
      {  case CRC_GOOD:
	    break;
	 case CRC_UNKNOWN:
	    break;
	 case CRC_BAD:
	    /* If its not already missing because of a read error,
	       make it missing due to the CRC failure. */
	    if(!current_missing)
	    {  current_missing = 1;
	       mark_sector(rc, s, Closure->yellowSector);
	    }
      }

      /* Remember sector state */

      if(current_missing)  /* Remember defect sector in current interval */
      {  if(first_missing < 0) first_missing = s;
	    last_missing = s;
      }
      else                 /* Remember good sector in eccStripe bitmap */
      {  rc->readable++;
	 if(rc->map)
	   SetBit(rc->map, s);

	 mark_sector(rc, s, Closure->greenSector);

#ifdef CHECK_VISITED
	 rc->count[s]++;
#endif
      }

      /* Determine end of current interval and write it out */

      if((!current_missing || s>=rc->highestWrittenSector) && first_missing>=0)
      {  if(s>=rc->highestWrittenSector)   /* special case: interval end = image end */
	 {  last_missing = rc->lastSector; /* may happen when image is truncated */
	    tail_included = TRUE;
	 }

	 /* Clip interval by user selected read range */

	 if(first_missing < rc->firstSector)  
	   first_missing = rc->firstSector;
	 if(last_missing > rc->lastSector)
	   last_missing = rc->lastSector;

	 /* add interval to list */

	 if(first_missing <= last_missing)
	   add_interval(rc, first_missing, last_missing-first_missing+1);

	 first_missing = -1;
      }

      /* Visualize the progress */

      percent = (100*s)/(rc->highestWrittenSector+1);
      if(last_percent != percent) 
      {  if(!Closure->guiMode)
	    PrintProgress(_("Analysing existing image file: %2d%%"),percent);

	 last_percent = percent;
      }
   }

   /*** If the image is shorter than the medium and the missing part
	was not already included in the last interval,
	insert another interval for the missing portion. */

   if(s<rc->lastSector && !tail_included)  /* truncated image? */
     add_interval(rc, s, rc->lastSector-s+1);

   /*** Now that all readable sectors are known,
	determine those which can already be corrected. */

   if(Closure->guiMode)
     SetAdaptiveReadSubtitle(_("Determining correctable sectors"));

   /* RS01 type error correction. */

   if(rc->readMode == ECC_IN_FILE)
   {  for(s=0; s<rc->rs01LayerSectors; s++)
      {  gint64 layer_idx;
	 int j,present=0;

	 /* Count available sectors in each layer */
	 
	 layer_idx = s;
	 for(j=0; j<rc->eh->dataBytes; j++)
	 {  if(   layer_idx >= rc->ei->sectors  /* padding sector */
	       || GetBit(rc->map, layer_idx))
	       present++;

	    layer_idx += rc->rs01LayerSectors;
	 }

	 /* See if remaining sectors are correctable */

	 if(rc->eh->dataBytes-present <= rc->eh->eccBytes)
	 {  layer_idx = s;
	    for(j=0; j<rc->eh->dataBytes; j++)   /* mark them as visited */
	    {  if(   layer_idx < rc->ei->sectors  /* skip padding sectors */
	          && !GetBit(rc->map, layer_idx))
	       {  SetBit(rc->map, layer_idx);
		  rc->correctable++;
#ifdef CHECK_VISITED
		  rc->count[layer_idx]++;
#endif
		  mark_sector(rc, layer_idx, Closure->greenSector);
	       }

	       layer_idx += rc->rs01LayerSectors;
	    }
	 }
      }
   }

   /* RS02 type error correction. */

   if(rc->readMode == ECC_IN_IMAGE)
   {  for(s=0; s<rc->lay->sectorsPerLayer; s++)
      {  int j,sector,present=0;

	 /* Count available sectors in each slice */
	 
	 for(j=0; j<255; j++)
	 {  sector = RS02SectorIndex(rc->lay, j, s);
	    if(   sector < 0  /* padding sector */ 
	       || GetBit(rc->map, sector))
	      present++;
	 }

	 /* See if remaining sectors are correctable */

	 if(255-present <= rc->eh->eccBytes)
	 {  for(j=0; j<255; j++)   /* mark them as visited */
	    {  sector = RS02SectorIndex(rc->lay, j, s);
	        if(   sector >= 0 /* don't mark the padding sector */
		   && !GetBit(rc->map, sector))
	       {  SetBit(rc->map, sector);
		  rc->correctable++;
		  mark_sector(rc, sector, Closure->greenSector);
	       }
	    }
	 }
      }
   }

   /*** Tell user results of image file analysis */

   if(rc->readMode == ECC_IN_FILE || rc->readMode == ECC_IN_IMAGE)
        PrintLog(_("Analysing existing image file: %lld readable, %lld correctable, %lld still missing.\n"),
		 rc->readable, rc->correctable, rc->sectors-rc->readable-rc->correctable);
   else PrintLog(_("Analysing existing image file: %lld readable, %lld still missing.\n"),
		 rc->readable, rc->sectors-rc->readable-rc->correctable);

   if(Closure->guiMode)
     UpdateAdaptiveResults(rc->readable, rc->correctable, 
			   rc->sectors-rc->readable-rc->correctable,
			   (int)((1000LL*(rc->readable+rc->correctable))/rc->sectors));

//   print_intervals(rc);
}

   /*** Mark RS02 header sectors as correctable.
        These are not part of any ecc block and have no influence on
	the decision when enough data has been gathered for error correction.
	Since they are needed for recognizing the image we will rewrite all
	them from the copy we got in rc->eh, but this can only be done when
	the image file has been fully created. */

static void mark_rs02_headers(read_closure *rc)
{  gint64 hpos, end;

   if(rc->readMode != ECC_IN_IMAGE) return;

   hpos = (rc->lay->protectedSectors + rc->lay->headerModulo - 1) / rc->lay->headerModulo;
   hpos *= rc->lay->headerModulo;
   end  = rc->lay->eccSectors + rc->lay->dataSectors - 2;

   while(hpos < end)
   {  if(!GetBit(rc->map, hpos))
      {  SetBit(rc->map, hpos);
	 mark_sector(rc, hpos, Closure->greenSector);
	 rc->correctable++;
      }
      if(!GetBit(rc->map, hpos+1))
      {  SetBit(rc->map, hpos+1);
         mark_sector(rc, hpos+1, Closure->greenSector);
	 rc->correctable++;
      }

      hpos += rc->lay->headerModulo;
   }
}

/***
 *** Main routine for adaptive reading
 ***/

static void insert_buttons(GtkDialog *dialog)
{  
  gtk_dialog_add_buttons(dialog, 
			 _utf("Ignore once"), 1,
			 _utf("Ignore always"), 2,
			 _utf("Abort"), 0, NULL);
} 

/*
 * Fill the gap between rc->intervalStart and rc->highestWrittenSector
 * with dead sector markers. These are needed in case the user aborts the operation; 
 * otherwise the gap will just be zero-filled and we don't know its contents
 * are unprocessed if we try to re-read the image later.
 * Also, this prevents fragmentation under most filesystems.
 */

void fill_gap(read_closure *rc)
{  char *anim[] = { ": |*.........|\b\b\b\b\b\b\b\b\b\b\b\b\b\b",
		    ": |.*........|\b\b\b\b\b\b\b\b\b\b\b\b\b\b",
		    ": |..*.......|\b\b\b\b\b\b\b\b\b\b\b\b\b\b",
		    ": |...*......|\b\b\b\b\b\b\b\b\b\b\b\b\b\b",
		    ": |....*.....|\b\b\b\b\b\b\b\b\b\b\b\b\b\b",
		    ": |.....*....|\b\b\b\b\b\b\b\b\b\b\b\b\b\b",
		    ": |......*...|\b\b\b\b\b\b\b\b\b\b\b\b\b\b",
		    ": |.......*..|\b\b\b\b\b\b\b\b\b\b\b\b\b\b",
		    ": |........*.|\b\b\b\b\b\b\b\b\b\b\b\b\b\b",
		    ": |.........*|\b\b\b\b\b\b\b\b\b\b\b\b\b\b"};
  char *t;
  gint64 i,j;
  gint64 firstUnwritten;

  /*** Start filling after rc->highestWrittenSector unless we are
       skipping from Sector 0 to the user selected start area. */

  if(rc->firstSector > 0 && rc->highestWrittenSector == 0)
       firstUnwritten = 0;
  else firstUnwritten = rc->highestWrittenSector + 1;

  /*** Tell user what's going on */

  t = g_strdup_printf(_("Filling image area [%lld..%lld]"), 
		      firstUnwritten, rc->intervalStart-1);
  clear_progress(rc);
  if(Closure->guiMode)
  {  SetAdaptiveReadSubtitle(t);
     ChangeSpiralCursor(Closure->readAdaptiveSpiral, -1); 
  }
  PrintCLI(t);
  g_free(t);

  /*** Seek to end of image */

  if(!LargeSeek(rc->image, (gint64)(2048*firstUnwritten)))
    Stop(_("Failed seeking to sector %lld in image [%s]: %s"),
	 firstUnwritten, "fill", strerror(errno));

  /*** Fill image with dead sector markers until rc->intervalStart */

  for(i=firstUnwritten, j=0; i<rc->intervalStart; i++)
  {  unsigned char buf[2048];
     int n;

     /* Write next sector */ 
    
     CreateMissingSector(buf, i, rc->fingerprint, FINGERPRINT_SECTOR, rc->volumeLabel);
     n = LargeWrite(rc->image, buf, 2048);
     if(n != 2048)
       Stop(_("Failed writing to sector %lld in image [%s]: %s"),
	    i, "fill", strerror(errno));

     /* Check whether user hit the Stop button */
	     
     if(Closure->stopActions)
     {  if(Closure->guiMode)
	 SetAdaptiveReadFootline(_("Aborted by user request!"), Closure->redText);

        rc->earlyTermination = FALSE;  /* suppress respective error message */
	cleanup((gpointer)rc);
     }

     /* Cycle the progress animation */

     if(j++ % 2000)
     {  int seq = (j/2000)%10;

	if(!Closure->guiMode)
	{  g_printf("%s", anim[seq]);
	   fflush(stdout);   /* at least needed for Windows */
	}
     }
	
     /* Show progress in the spiral */
    
     if(Closure->guiMode)
     {  int segment = i / rc->sectorsPerSegment;
       
        if(Closure->readAdaptiveSpiral->segmentColor[segment] == Closure->background)
	  ChangeSegmentColor(Closure->whiteSector, segment);
     }
  }

  PrintCLI("               \n");
  rc->highestWrittenSector = rc->intervalStart-1;

  if(Closure->guiMode)  /* remove temporary fill markers */
  {  RemoveFillMarkers();
     SetAdaptiveReadSubtitle(rc->subtitle);
  }
}


/*
 * If a correctable sector <correctable> lies beyond rc->highestWrittenSector,
 * fill the gap with dead sector markers.
 * So when reading resumes there will be no holes in the image.
 */

void fill_correctable_gap(read_closure *rc, gint64 correctable)
{  
   if(correctable > rc->highestWrittenSector)
   {  gint64 ds = rc->highestWrittenSector+1;
      unsigned char buf[2048];

      if(!LargeSeek(rc->image, (gint64)(2048*ds)))
	Stop(_("Failed seeking to sector %lld in image [%s]: %s"),
	     ds, "skip-corr", strerror(errno));

      for(ds=rc->highestWrittenSector+1; ds<=correctable; ds++)
      {  CreateMissingSector(buf, ds, rc->fingerprint, FINGERPRINT_SECTOR, rc->volumeLabel);
	 if(LargeWrite(rc->image, buf, 2048) != 2048)
	  Stop(_("Failed writing to sector %lld in image [%s]: %s"),
	       ds, "skip-corr", strerror(errno));
      }
      rc->highestWrittenSector = correctable;
   }
}

/*
 * The adaptive read strategy 
 */

void ReadMediumAdaptive(gpointer data)
{  read_closure *rc;
   gint64 s;
   gint64 image_file_size;
   int status,i,n;

   /*** Initialize the read closure. */

   rc = g_malloc0(sizeof(read_closure));

   rc->ab = CreateAlignedBuffer(MAX_CLUSTER_SIZE);
   rc->buf = rc->ab->buf;

   memset(rc->progressBs, '\b', 256);
   memset(rc->progressSp, ' ', 256);

   /*** Register the cleanup procedure so that Stop() can abort us properly. */

   rc->earlyTermination = TRUE;

   RegisterCleanup(_("Reading aborted"), cleanup, rc);
   if(Closure->guiMode)
     SetLabelText(GTK_LABEL(Closure->readAdaptiveHeadline), "<big>%s</big>\n<i>%s</i>",
		  _("Preparing for reading the medium image."),
		  _("Medium: not yet determined"));

   /*** Open Device and .ecc file. Determine read mode. */

   open_and_determine_mode(rc);

   /*** Compare image and ecc fingerprints (only if RS01 type .ecc is available) */

   if(rc->readMode == ECC_IN_FILE)
      check_ecc_fingerprint(rc);

   /*** Validate image size against ecc data */
   
   check_size(rc);

   /*** Limit the read range from users choice */

   GetReadingRange(rc->sectors, &rc->firstSector, &rc->lastSector);
   rc->intervalStart = rc->firstSector;
   rc->intervalEnd   = rc->lastSector;
   rc->intervalSize  = rc->intervalEnd - rc->intervalStart + 1;

   /*** Initialize the sector bitmap if ecc information is present.
	This is used to stop when sufficient data for error correction
	becomes available. */

   if(   rc->readMode == ECC_IN_FILE
      || rc->readMode == ECC_IN_IMAGE)
      rc->map = CreateBitmap0(rc->sectors);

#ifdef CHECK_VISITED
   rc->count = g_malloc0((int)rc->sectors+160);
#endif

   /*** Initialize segment state counters (only in GUI mode) */

   if(Closure->guiMode)
   {  //rc->sectorsPerSegment = 1 + (rc->sectors / ADAPTIVE_READ_SPIRAL_SIZE);
      rc->sectorsPerSegment = ((rc->sectors+ADAPTIVE_READ_SPIRAL_SIZE-1) / ADAPTIVE_READ_SPIRAL_SIZE);
      rc->segmentState = g_malloc0(ADAPTIVE_READ_SPIRAL_SIZE * sizeof(int));
      //      ClipReadAdaptiveSpiral(rc->sectors/rc->sectorsPerSegment);
      ClipReadAdaptiveSpiral((rc->sectors+rc->sectorsPerSegment-1)/rc->sectorsPerSegment);
   }

   /*** Initialize the interval list */

   rc->intervals = g_malloc(8*sizeof(gint64));
   rc->maxIntervals = 4; 
   rc->nIntervals = 0; 

   /*** Start with a fresh image file if none is already present. */

reopen_image:
   if(!LargeStat(Closure->imageName, &image_file_size))
   {  if(!(rc->image = LargeOpen(Closure->imageName, O_RDWR | O_CREAT, IMG_PERMS)))
	Stop(_("Can't open %s:\n%s"),Closure->imageName,strerror(errno));

      PrintLog(_("Creating new %s image.\n"),Closure->imageName);
      if(Closure->guiMode)
	SetLabelText(GTK_LABEL(Closure->readAdaptiveHeadline),
		     "<big>%s</big>\n<i>%s</i>",
		     _("Reading new medium image."),
		     rc->dh->mediumDescr);

      /* Mark RS02 header sectors as correctable. */

      mark_rs02_headers(rc);

      /* Preload the CRC buffer */

      load_crc_buf(rc);
   }

   /*** else examine the existing image file ***/

   else 
   {  int reopen;

      if(Closure->guiMode)
	SetLabelText(GTK_LABEL(Closure->readAdaptiveHeadline),
		     "<big>%s</big>\n<i>%s</i>",
		     _("Completing existing medium image."),
		     rc->dh->mediumDescr);

      /* Open the existing image file. */

      if(!(rc->image = LargeOpen(Closure->imageName, O_RDWR, IMG_PERMS)))
	Stop(_("Can't open %s:\n%s"),Closure->imageName,strerror(errno));

      /* See if the image fingerprint matches the medium */

      reopen = check_image_fingerprint(rc);
      if(reopen) 
	goto reopen_image;  /* no match, user wants to erase old image */

      /* Compare length of image and medium. */

      check_image_size(rc, image_file_size / 2048);

      /* Preload the CRC buffer */

      load_crc_buf(rc);

      /* Build the interval list */

      build_interval_from_image(rc);

      /* Mark still missing RS02 header sectors as correctable. */

      mark_rs02_headers(rc);

      /* Already enough information available? */
	 
      if(rc->readable + rc->correctable >= rc->sectors)
      {  char *t = _("\nSufficient data for reconstructing the image is available.\n");
	 PrintLog(t);
	 if(Closure->guiMode)
	   SetAdaptiveReadFootline(t, Closure->greenText);
	 goto finished;
      }

      /* Nope, begin with first interval */

      if(!rc->nIntervals)  /* may happen when reading range is restricted too much */
	goto finished;

      rc->intervalStart = rc->intervals[0];
      rc->intervalSize  = rc->intervals[1];
      rc->intervalEnd   = rc->intervalStart + rc->intervalSize - 1;
      pop_interval(rc);
   }

   /*** Read the medium image. */

   if(Closure->guiMode)
     SetAdaptiveReadSubtitle(rc->subtitle);

   for(;;)
   {  int cluster_mask = rc->dh->clusterSize-1;

      /* If we jumped beyond the highest writtensector, 
	 fill the gap with dead sector markers. */

      if(rc->intervalStart > rc->highestWrittenSector)
	fill_gap(rc);

      /*** Try reading the next interval */

      print_progress(rc, TRUE);

      for(s=rc->intervalStart; s<=rc->intervalEnd; ) /* s is incremented elsewhere */
      {  int nsectors,cnt;
 
	 if(Closure->stopActions)          /* somebody hit the Stop button */
	 {  if(Closure->guiMode)
	       SetAdaptiveReadFootline(_("Aborted by user request!"), Closure->redText);

	    rc->earlyTermination = FALSE;  /* suppress respective error message */
	    goto terminate;
	 }

	 if(Closure->guiMode)
	    ChangeSpiralCursor(Closure->readAdaptiveSpiral, s / rc->sectorsPerSegment);
	    
	 /* Determine number of sectors to read. Read the next dh->clusterSize sectors
	    unless we're at the end of the interval or at a position which is
	    not divideable by the cluster size. */

	 if(s & cluster_mask)
               nsectors = 1;
	 else  nsectors = rc->dh->clusterSize;

	 if(s+nsectors > rc->intervalEnd) nsectors = rc->intervalEnd-s+1;

	 /* Skip sectors which have been marked as correctable by
	    ecc information. */

	 if(rc->map)
	 {  for(i=cnt=0; i<nsectors; i++)  /* sectors already present? */
	    {  int idx = s+i;

	       if(GetBit(rc->map, idx))
		 cnt++;
	    }

	    /* Shift the outer loop down to 1 sector per read.
	       Short circuit the outer loop if the sector is already present. */

	    if(cnt) 
	    {  nsectors = 1;

	       if(GetBit(rc->map, s))
	       {  s++;
		  continue;  /* restart reading loop with next sector */
	       }
	    }
	 }

	 /* Try to actually read the next sector(s) */
reread:
	 status = ReadSectors(rc->dh, rc->buf, s, nsectors);

	 /* Medium Error (3) and Illegal Request (5) may result from 
	    a medium read problem, but other errors are regarded as fatal. */

	 if(status && !Closure->ignoreFatalSense
	    && rc->dh->sense.sense_key 
	    && rc->dh->sense.sense_key != 3 && rc->dh->sense.sense_key != 5)
	 {  int answer;

	    if(!Closure->guiMode)
	      Stop(_("Sector %lld: %s\nCan not recover from above error.\n"
		     "Use the --ignore-fatal-sense option to override."),
		   s, GetLastSenseString(FALSE));

	    answer = ModalDialog(GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, insert_buttons,
				 _("Sector %lld: %s\n\n"
				   "It may not be possible to recover from this error.\n"
				   "Should the reading continue and ignore this error?"),
				 s, GetLastSenseString(FALSE));

	    if(answer == 2)
	      Closure->ignoreFatalSense = 2;

	    if(!answer)
	    {  SetAdaptiveReadFootline(_("Aborted by unrecoverable error."), Closure->redText);

	       rc->earlyTermination = FALSE;  /* suppress respective error message */
	       goto terminate;
	    }
	 }

	 /* When encountering an error during cluster size reads,
	    try again reading each sector one by one.
	    Otherwise we skip cluster size chunks until the unreadable
	    intervals become smaller than the cluster size. */

	 if(status && nsectors > 1)
	 {  nsectors = 1;
	    goto reread;
	 }

	 /* Reading was successful. */

	 if(!status)   
	 {  gint64 b;

	    if(!LargeSeek(rc->image, (gint64)(2048*s)))
	      Stop(_("Failed seeking to sector %lld in image [%s]: %s"),
		   s,"store",strerror(errno));

	    /* Store sector(s) in the image file if they pass the CRC test,
	       otherwise treat them as unprocessed. */

	    for(i=0, b=s; i<nsectors; i++,b++)
	    {  int result;
	       int err;

	       /* Calculate and compare CRC sums.
		  Sectors with bad CRC sums are marked unvisited,
		  but do not terminate the current interval. */

	       if(rc->crcBuf) /* we have crc information */
		    result = CheckAgainstCrcBuffer(rc->crcBuf, b, rc->buf+i*2048);
	       else result = CRC_UNKNOWN;

	       switch(result)
	       {  case CRC_BAD:
		  {  //unsigned char buf[2048];

		     PrintCLI("\n");
		     PrintCLI(_("CRC error in sector %lld\n"),b);
		     print_progress(rc, TRUE);

#if 0 // remark: Do we still need to mark CRC defects as completely missing?
		     CreateMissingSector(buf, b, rc->fingerprint, FINGERPRINT_SECTOR, rc->volumeLabel);
		     n = LargeWrite(rc->image, buf, 2048);
#endif
		     n = LargeWrite(rc->image, rc->buf+i*2048, 2048);
		     if(n != 2048)
			Stop(_("Failed writing to sector %lld in image [%s]: %s"),
			     b, "unv", strerror(errno));

		     mark_sector(rc, b, Closure->yellowSector);
		     
		     if(rc->highestWrittenSector < b)
			rc->highestWrittenSector = b;
		     break;
		  }
		  case CRC_UNKNOWN:
		  case CRC_GOOD:
		     n = LargeWrite(rc->image, rc->buf+i*2048, 2048);
		     if(n != 2048)
			Stop(_("Failed writing to sector %lld in image [%s]: %s"),
			     b, "store", strerror(errno));

		     if(rc->map)
			SetBit(rc->map, b);
		     rc->readable++;

		     mark_sector(rc, b, Closure->greenSector);
		    
		     if(rc->highestWrittenSector < b)
			rc->highestWrittenSector = b;
		     break;
	       }

	       /*** Warn the user if we see dead sector markers on the image.
		    Note: providing the fingerprint is not necessary as any 
		    incoming missing sector marker indicates a huge problem. */

	       err = CheckForMissingSector(rc->buf+i*2048, b, NULL, 0);
	       if(err != SECTOR_PRESENT)
	       {  ExplainMissingSector(rc->buf+i*2048, b, err, FALSE);

		  if(rc->map)  /* Avoids confusion in the ecc stage */
		     ClearBit(rc->map, b);
		  rc->readable--;
	       }
	    }

	    /* See if additional sectors become correctable. */
	    
	    if(rc->readMode == ECC_IN_FILE)  /* RS01 type ecc data */
	    {  for(i=0, b=s; i<nsectors; i++,b++) 
	       {  int j,present=0;
		  gint64 layer_idx;

#ifdef CHECK_VISITED
		  rc->count[b]++;
#endif
		  /* Count available sectors. */

		  layer_idx = b % rc->rs01LayerSectors;
		  for(j=0; j<rc->eh->dataBytes; j++) 
		  {  if(   layer_idx >= rc->ei->sectors /* padding sector */
			|| GetBit(rc->map, layer_idx))
			present++;
		     layer_idx += rc->rs01LayerSectors;
		  }

		  /* If the remaining sectors are correctable,
		     mark them as visited. */

		  if(rc->eh->dataBytes-present <= rc->eh->eccBytes)
		  {  layer_idx = b % rc->rs01LayerSectors;

		     for(j=0; j<rc->eh->dataBytes; j++)
		     {  if(   layer_idx < rc->ei->sectors /* skip padding sector */
			   && !GetBit(rc->map, layer_idx))
			{  SetBit(rc->map, layer_idx);
			   rc->correctable++;
			   mark_sector(rc, layer_idx, Closure->greenSector);

#ifdef CHECK_VISITED
			   rc->count[layer_idx]++;
#endif

			   /* If the correctable sector lies beyond the highest written sector,
			      fill the gap with dead sector markers */

			   fill_correctable_gap(rc, layer_idx);

			}
		        layer_idx += rc->rs01LayerSectors;
		     }
		  }
	       }
	    }

	    if(rc->readMode == ECC_IN_IMAGE)  /* RS02 type ecc data */
	    {  for(i=0, b=s; i<nsectors; i++,b++) 
	       {  int j,present=0;
		  gint64 slice_idx, ignore, sector;

		  /* Count available sectors. */

		  RS02SliceIndex(rc->lay, b, &ignore, &slice_idx);
		  for(j=0; j<255; j++) 
		  {  sector = RS02SectorIndex(rc->lay, j, slice_idx);
		     if(   sector < 0  /* padding sector */ 
			|| GetBit(rc->map, sector))
			present++;
		  }

		  /* If the remaining sectors are correctable,
		     mark them as visited. */

		  if(255-present <= rc->eh->eccBytes)
		  {  for(j=0; j<255; j++)
		     {  sector = RS02SectorIndex(rc->lay, j, slice_idx);
		        if(   sector >= 0 /* don't mark the padding sector */
			   && !GetBit(rc->map, sector))
			{  SetBit(rc->map, sector);
			   rc->correctable++;
			   mark_sector(rc, sector, Closure->greenSector);
			   fill_correctable_gap(rc, sector);
			}
		     }
		  }
	       }
	    }

	    /* Increment sector counters. Adjust max image sector
	       if we added sectors beyond the image size. */

	    s+=nsectors;
#if 0 /* obsoleted since it is carried out right after the LargeWrite() above */
	    if(s>rc->highestWrittenSector) rc->highestWrittenSector=s;
#endif

	    /* Stop reading if enough data for error correction
	       has been gathered */

	    if(rc->readable + rc->correctable >= rc->sectors)
	    {  char *t = _("\nSufficient data for reconstructing the image is available.\n");

	       print_progress(rc, TRUE);
	       PrintLog(t);
	       if(Closure->guiMode && rc->ei)
		  SetAdaptiveReadFootline(t, Closure->foreground);
	       if(Closure->eject)
		  LoadMedium(rc->dh, FALSE);
	       goto finished;
	    }
	 }  /* end of if(!status) (successful reading of sector(s)) */

	 else  /* Process the read error. */
	 {  unsigned char buf[2048];

	    PrintCLI("\n");
	    if(nsectors>1) PrintCLIorLabel(Closure->status,
					   _("Sectors %lld-%lld: %s\n"),
					   s, s+nsectors-1, GetLastSenseString(FALSE));  
	    else	   PrintCLIorLabel(Closure->status,
					   _("Sector %lld: %s\n"),
					   s, GetLastSenseString(FALSE));  

	    rc->unreadable += nsectors;

	    /* Write nsectors of "dead sector" markers */

	    if(!LargeSeek(rc->image, (gint64)(2048*s)))
	      Stop(_("Failed seeking to sector %lld in image [%s]: %s"),
		   s, "nds", strerror(errno));

	    for(i=0; i<nsectors; i++)
	    {  CreateMissingSector(buf, s+i, rc->fingerprint, FINGERPRINT_SECTOR, rc->volumeLabel);

	       n = LargeWrite(rc->image, buf, 2048);
	       if(n != 2048)
		 Stop(_("Failed writing to sector %lld in image [%s]: %s"),
		      s, "nds", strerror(errno));

	       mark_sector(rc, s+i, Closure->redSector);
	    }

	    if(rc->highestWrittenSector < s+nsectors)
	      rc->highestWrittenSector = s+nsectors-1;

	    /* Reading of the interval ends at the first read error.
	       Store the remainder of the current interval in the queue. */

	    if(s+nsectors-1 >= rc->intervalEnd)  /* This was the last sector; interval used up */
	    {  Verbose("... Interval [%lld..%lld] used up\n", rc->intervalStart, rc->intervalEnd);
	    }
	    else  /* Insert remainder of interval into queue */
	    {  rc->intervalStart = s+nsectors;
	       rc->intervalSize  = rc->intervalEnd-rc->intervalStart+1;

	       Verbose("... Interval %lld [%lld..%lld] added\n", 
		       rc->intervalSize, rc->intervalStart, rc->intervalStart+rc->intervalSize-1);

	       add_interval(rc, rc->intervalStart, rc->intervalSize);
	       //print_intervals(rc);
	    }
	    break; /* fall out of reading loop */
	 }

	 print_progress(rc, FALSE);
      }

      /* If we reach this, the current interval has either been read completely
	 or the loop was terminated early by a read error. 
         In both cases, the current interval has already been remove from the queue
         and the queue contains only the still unprocessed intervals. */

      if(s>=rc->intervalEnd) /* we fell out of the reading loop with interval completed */
      {  print_progress(rc, TRUE);
	 PrintCLI("\n");
      }

      /* Pop the next interval from the queue,
         prepare one half from it for processing
         and push the other half back on the queue. */

      if(rc->nIntervals <= 0)
	goto finished;

      rc->intervalStart = rc->intervals[0];
      rc->intervalSize  = rc->intervals[1];
      pop_interval(rc);

      /* Split the new interval */

      if(rc->intervalSize>1)
      {  Verbose("*** Splitting [%lld..%lld]\n",
		 rc->intervalStart,rc->intervalStart+rc->intervalSize-1);

	 add_interval(rc, rc->intervalStart, rc->intervalSize/2);
	 rc->intervalEnd = rc->intervalStart+rc->intervalSize-1;
	 rc->intervalStart = rc->intervalStart+rc->intervalSize/2;
      }
      else /* 1 sector intervals can't be split further */
      {  
	 rc->intervalEnd = rc->intervalStart;
	 Verbose("*** Popped [%lld]\n",rc->intervalStart);
      }

      //print_intervals(rc);

      rc->intervalSize  = rc->intervalEnd-rc->intervalStart+1;

      /* Apply interval size termination criterion */

      if(rc->intervalSize < Closure->sectorSkip)
	 goto finished;
   }

finished:

#ifdef CHECK_VISITED
   {  int i,cnt=0;
      for(i=0; i<(int)rc->sectors; i++)
      {  cnt+=rc->count[i];
         if(rc->count[i] != 1)
           printf("Sector %d: %d\n",i,rc->count[i]);
      }

      printf("\nTotal visited %d (%d)\n",cnt,i);

      for(i=(int)rc->sectors; i<(int)rc->sectors+160; i++)
        if(rc->count[i] != 0)
          printf("SECTOR %d: %d\n",i,rc->count[i]);
   }
#endif

   /* Force output of final results */

   if(Closure->guiMode)
   {  ChangeSpiralCursor(Closure->readAdaptiveSpiral, -1);
      mark_sector(rc, 0, NULL);
   }

   /*** Summarize results. */

   /* We were in ECC_IN_FILE or ECC_IN_IMAGE mode,
      but did not recover sufficient data. */

   if(rc->map && (rc->readable + rc->correctable < rc->sectors))
   {  int total = rc->readable+rc->correctable;
      int percent = (int)((1000LL*(long long)total)/rc->sectors);
      char *t = g_strdup_printf(_("Only %2d.%1d%% of the image are readable or correctable"),
				  percent/10, percent%10); 

      PrintLog(_("\n%s\n"
		  "(%lld readable,  %lld correctable,  %lld still missing).\n"),
		t, rc->readable, rc->correctable, rc->sectors-total);
      if(Closure->guiMode)
	 SetAdaptiveReadFootline(t, Closure->foreground);

      g_free(t);
      exitCode = EXIT_FAILURE;
   }

   /* Results for reading in IMAGE_ONLY mode */

   if(rc->readMode == IMAGE_ONLY)
   {  if(rc->readable == rc->sectors)
      {  char *t = _("\nGood! All sectors have been read.\n"); 
	 PrintLog(t);
	 if(Closure->guiMode)
	   SetAdaptiveReadFootline(t, Closure->foreground);
	 if(Closure->eject)
	    LoadMedium(rc->dh, FALSE);
      }
      else
      {  int percent = (int)((1000LL*rc->readable)/rc->sectors);
	 char *t = g_strdup_printf(_("No unreadable intervals with >= %d sectors left."),
				   Closure->sectorSkip);

	 PrintLog(_("\n%s\n" 
		     "%2d.%1d%% of the image have been read (%lld sectors).\n"),
		   t, percent/10, percent%10, rc->readable);

	 if(Closure->guiMode)
	   SetAdaptiveReadFootline(t, Closure->foreground);
	 g_free(t);
	 exitCode = EXIT_FAILURE;
      }
   }

#if 0
   for(i=0; i<rc->sectors; i++)
     if(!GetBit(rc->map, i))
       printf("Missing: %d\n", i);
#endif

   /*** Close and clean up */

   rc->earlyTermination = FALSE;

terminate:
   cleanup((gpointer)rc);
}

