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

#include "scsi-layer.h"
#include "udf.h"

#include "rs02-includes.h"

//#define CHECK_VISITED 1        /* This gives only reasonable output            */
                                 /* when the .ecc file is present while reading! */ 

/***
 *** Replace and remove structs and functions in this section!
 ***/

#define READABLE_IMAGE    0
#define READABLE_ECC      0
#define WRITEABLE_IMAGE   (1<<0)
#define WRITEABLE_ECC     (1<<1)
#define PRINT_MODE        (1<<4)
#define CREATE_CRC        ((1<<1) | (1<<5))

typedef struct _EccInfo
{  LargeFile *file;             /* file handle for ecc file */
   struct _EccHeader *eh;       /* the header struct below */
   gint64 sectors;              /* gint64 version of eh->sectors */ 
   struct MD5Context md5Ctxt;   /* md5 context of crc portion of file */
} EccInfo;

static void free_ecc_info(EccInfo *ei)
{
   if(ei->file)
     if(!LargeClose(ei->file))
       Stop(_("Error closing error correction file:\n%s"), strerror(errno));

   if(ei->eh) g_free(ei->eh);
   g_free(ei);
}


static EccInfo* open_ecc_file(int mode)
{  EccInfo *ei = NULL; 
   int file_flags;

   /*** Sanity check for ecc file reads */

   if(!(mode & WRITEABLE_ECC))
   {  guint64 ecc_size;

      if(!LargeStat(Closure->eccName, &ecc_size))
      {  if(!(mode & PRINT_MODE))
	   Stop(_("Can't open %s:\n%s"),Closure->eccName,strerror(errno));
	 return NULL;
      }

      if(ecc_size < 4096)
	Stop(_("Invalid or damaged ecc file"));
   }
   
   /*** Open the ecc file  */

   ei = g_malloc0(sizeof(EccInfo));
   ei->eh = g_malloc0(sizeof(EccHeader));

   file_flags = mode & WRITEABLE_ECC ? O_RDWR | O_CREAT : O_RDONLY;

   if(!(ei->file = LargeOpen(Closure->eccName, file_flags, IMG_PERMS)))
   {  free_ecc_info(ei);
      ei = NULL;

      if(!(mode & PRINT_MODE))  /* missing ecc file no problem in print mode */
	Stop(_("Can't open %s:\n%s"),Closure->eccName,strerror(errno));
      return NULL;
   }

   if(!(mode & WRITEABLE_ECC))
   {   int n = LargeRead(ei->file, ei->eh, sizeof(EccHeader));

       if(n != sizeof(EccHeader))
       {  free_ecc_info(ei);
	  Stop(_("Can't read ecc header:\n%s"),strerror(errno));
       }

       /*** Endian annoyance */

#ifdef HAVE_BIG_ENDIAN
       SwapEccHeaderBytes(ei->eh);
#endif

       /*** See if we can use the ecc file */

       if(strncmp((char*)ei->eh->cookie, "*dvdisaster*", 12))
       {  free_ecc_info(ei);
	  Stop(_("Invalid or damaged ecc file"));
       }
       
       if(Closure->version < ei->eh->neededVersion)
	    PrintCLI(_("* Warning: This ecc file requires dvdisaster-%d.%d!\n"
		    "*          Proceeding could trigger incorrect behaviour.\n"
		    "*          Please read the image without using this ecc file\n"
		    "*          or visit http://www.dvdisaster.org for an upgrade.\n\n"), 
		  ei->eh->neededVersion/10000,
		  (ei->eh->neededVersion%10000)/100);

       ei->sectors = uchar_to_gint64(ei->eh->sectors);
       LargeSeek(ei->file, 0);
   }

   return ei;
}


/*
 * Load crc buffer from RS01 error correction file
 */

CrcBuf *GetCRCFromRS01_obsolete(EccInfo *ei)   /* FIXME: obsolete */
{  CrcBuf *cb = g_malloc(sizeof(CrcBuf));
   guint32 *buf;
   gint64 crc_sectors,crc_remainder;
   gint64 i,j,sec_idx;

   cb->crcbuf = g_malloc(ei->sectors * sizeof(guint32));
   cb->size   = ei->sectors;
   cb->valid  = CreateBitmap0(ei->sectors);
   buf = cb->crcbuf;

   /* Seek to beginning of CRC sums */

   if(!LargeSeek(ei->file, (gint64)sizeof(EccHeader)))
      Stop(_("Failed skipping the ecc header: %s"),strerror(errno));

   /* Read crc sums. A sector of 2048 bytes contains 512 CRC sums. */

   crc_sectors = ei->sectors / 512;
   sec_idx = 0;

   for(i=0; i<crc_sectors; i++)
   {  if(LargeRead(ei->file, buf, 2048) != 2048)
	 Stop(_("Error reading CRC information: %s"),strerror(errno));
      buf += 512;

      for(j=0; j<512; j++, sec_idx++)
	 SetBit(cb->valid, sec_idx);
   }

   crc_remainder = sizeof(guint32)*(ei->sectors % 512);
   if(crc_remainder)
   {  if(LargeRead(ei->file, buf, crc_remainder) != crc_remainder)
	 Stop(_("Error reading CRC information: %s"),strerror(errno));

      for( ; sec_idx<ei->sectors; sec_idx++)
	 SetBit(cb->valid, sec_idx);
   }

   return cb;
}

/*
 * Load crc buffer from RS02 error correction data
 *
 * Lots of casts from (void*) since we're transporting
 * nonpublic structs.
 */

CrcBuf *GetCRCFromRS02_obsolete(void *layv, void *dhv, LargeFile *image)
{  RS02Layout *lay = (RS02Layout*)layv;
   DeviceHandle *dh = (DeviceHandle*)dhv;
   AlignedBuffer *ab = CreateAlignedBuffer(2048);
   CrcBuf *cb = g_malloc(sizeof(CrcBuf));
   gint64 block_idx[256];
   gint64 image_sectors,crc_sector;
   gint64 s,i;
   int crc_idx, crc_valid = FALSE;

   image_sectors = lay->eccSectors+lay->dataSectors;
 
   cb->crcbuf = g_malloc(image_sectors * sizeof(guint32));
   cb->size   = image_sectors;
   cb->valid  = CreateBitmap0(image_sectors);

   /* Initialize ecc block index pointers.
      The first CRC set (of lay->ndata checksums) relates to
      ecc block lay->firstCrcLayerIndex + 1. */

   for(s=0, i=0; i<lay->ndata; s+=lay->sectorsPerLayer, i++)
     block_idx[i] = s + lay->firstCrcLayerIndex + 1;

   crc_idx = 512;                   /* force crc buffer reload */
   crc_sector = lay->dataSectors+2; /* first crc data sector on medium */

   /* Cycle through the ecc blocks and descramble CRC sums in
      ascending sector numbers. */

   for(s=0; s<lay->sectorsPerLayer; s++)
   {  gint64 si = (s + lay->firstCrcLayerIndex + 1) % lay->sectorsPerLayer;

      /* Wrap the block_idx[] ptrs at si == 0 */

      if(!si)
      {  gint64 bs;

         for(bs=0, i=0; i<lay->ndata; bs+=lay->sectorsPerLayer, i++)
	   block_idx[i] = bs;
      }

      /* Go through all data sectors of current ecc block */

      for(i=0; i<lay->ndata; i++)
      {  gint64 bidx = block_idx[i];

	 if(bidx < lay->dataSectors)  /* only data sectors have CRCs */
	 {  
	    /* Refill crc cache if needed */
	    
	    if(crc_idx >= 512)
	    {   crc_valid = !ReadSectorsFast(dh, ab->buf, crc_sector++, 1);
		crc_idx = 0;
	    }

	    /* Sort crc into appropriate place */

	    if(crc_valid)
	    {  cb->crcbuf[bidx] = ((guint32*)ab->buf)[crc_idx];
	       SetBit(cb->valid, bidx);
	    }
	    crc_idx++;
	    block_idx[i]++;
	 }
      }
   }

   FreeAlignedBuffer(ab);

   return cb;
}


/***
 *** Local data package used during reading
 ***/

enum { IMAGE_ONLY, ECC_IN_FILE, ECC_IN_IMAGE };

typedef struct
{  Image *medium;               /* Medium (disc) we are reading from */
   DeviceHandle *dh;            /* device we are reading from */
   gint64 sectors;              /* sectors in medium (maybe cooked value from file system) */
   gint64 expectedSectors;      /* sectors expected from information in ecc data */
   LargeFile *image;            /* image file for saving the sectors */
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

   if(rc->medium) CloseImage(rc->medium);
 
   if(rc->ei) free_ecc_info(rc->ei);

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
   int percent = (int)((1000LL*(long long)total)/rc->expectedSectors);

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
		    percent/10, percent%10, rc->expectedSectors-rc->readable, 
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
	    && rc->segmentState[segment] >= rc->expectedSectors % rc->sectorsPerSegment)
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
			   rc->expectedSectors-rc->readable-rc->correctable,
			   (int)((1000LL*(rc->readable+rc->correctable))/rc->expectedSectors));
}

/***
 *** Basic device and image handling and sanity checks.
 ***/

/*
 * Open optical drive and .ecc file.
 * Determine reading mode.
 */

static void open_and_determine_mode(read_closure *rc)
{  unsigned char fp[16];
 
   /* open the device */

   rc->medium = OpenImageFromDevice(Closure->device);
   rc->dh = rc->medium->dh;
   rc->readMode = IMAGE_ONLY;

   /* save some useful information for the missing sector marker */

   if(GetImageFingerprint(rc->medium, fp, FINGERPRINT_SECTOR))
   {  rc->fingerprint = g_malloc(16);
      memcpy(rc->fingerprint, fp, 16);
   }

   if(rc->medium->isoInfo && rc->medium->isoInfo->volumeLabel[0])
      rc->volumeLabel = g_strdup(rc->medium->isoInfo->volumeLabel);

   /* See if we have ecc information available. 
      Prefer the error correction file over augmented images if both are available. */

   /* See if the medium contains RS02 type ecc information */

   rc->ei = open_ecc_file(READABLE_ECC | PRINT_MODE);
   if(rc->ei)   /* RS01 type ecc */
   {  rc->readMode = ECC_IN_FILE;
      rc->eh = rc->ei->eh;

      rc->rs01LayerSectors = (rc->ei->sectors+rc->eh->dataBytes-1)/rc->eh->dataBytes;

      SetAdaptiveReadMinimumPercentage((1000*(rc->eh->dataBytes-rc->eh->eccBytes))/rc->eh->dataBytes);
   }
   else   /* see if we have RS02 type ecc */
   if(rc->medium->eccHeader && !strncmp((char*)rc->medium->eccHeader->method,"RS02",4))
   {  rc->readMode = ECC_IN_IMAGE;
      rc->eh  = rc->medium->eccHeader;
      rc->lay = RS02LayoutFromImage(rc->medium);
 
      SetAdaptiveReadMinimumPercentage((1000*rc->lay->ndata)/255);

      if(Closure->version < rc->eh->neededVersion)
	 PrintCLI(_("* Warning: This image requires dvdisaster-%d.%d!\n"
		    "*          Proceeding could trigger incorrect behaviour.\n"
		    "*          Please visit http://www.dvdisaster.org for an upgrade.\n\n"), 
		  rc->eh->neededVersion/10000,
		  (rc->eh->neededVersion%10000)/100);


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
        rc->expectedSectors = rc->sectors = rc->dh->sectors;
	return;

      case ECC_IN_FILE:
	rc->sectors = rc->ei->sectors;
	break;

      case ECC_IN_IMAGE:
	rc->sectors = rc->lay->eccSectors + rc->lay->dataSectors;
	break;
   }
   rc->expectedSectors = rc->sectors; /* keep value from ecc */

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

   fp_read = GetImageFingerprint(rc->medium, digest, rc->eh->fpSector);

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

   fp_read = GetImageFingerprint(rc->medium, medium_fp, fingerprint_sector);

   if(n != 2048 || !fp_read || (CheckForMissingSector(rc->buf, fingerprint_sector, NULL, 0) != SECTOR_PRESENT))
     return 0; /* can't tell, assume okay */

   /* If both could be read, compare them */

   if(memcmp(image_fp, medium_fp, 16))
   {  	  
     if(!Closure->guiMode)
       Stop(_("Image file does not match the optical disc."));
     else
     {  int answer = ConfirmImageDeletion(Closure->imageName);

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
   if(image_file_sectors > rc->expectedSectors)
   {  int answer;
	 
      answer = ModalWarning(GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL, NULL,
			    _("Image file is %lld sectors longer than inserted medium\n"
			      "(Image file: %lld sectors; medium: %lld sectors).\n"),
			    image_file_sectors-rc->expectedSectors, 
			    image_file_sectors, rc->expectedSectors);

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
	 rc->crcBuf = GetCRCFromRS01_obsolete(rc->ei);
	 break;
      case ECC_IN_IMAGE:
	 SetAdaptiveReadSubtitle(_utf("Loading CRC data."));
	 rc->crcBuf = GetCRCFromRS02_obsolete(rc->lay, rc->dh, rc->image);
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
      {  int fixme=0;
	 mark_sector(rc, s, Closure->redSector);
	 ExplainMissingSector(rc->buf, s, current_missing, SOURCE_IMAGE, &fixme);
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
   {  gint first, length;

      /* Make sure the remainder lies with the specified reading range */

      if(s<rc->firstSector)
	   first=rc->firstSector;
      else first=s;
      length = rc->lastSector-first+1;

      if(length > 0)
	add_interval(rc, first, length);
   }

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
		 rc->readable, rc->correctable, rc->expectedSectors-rc->readable-rc->correctable);
   else PrintLog(_("Analysing existing image file: %lld readable, %lld still missing.\n"),
		 rc->readable, rc->expectedSectors-rc->readable-rc->correctable);

   if(Closure->guiMode)
     UpdateAdaptiveResults(rc->readable, rc->correctable, 
			   rc->expectedSectors-rc->readable-rc->correctable,
			   (int)((1000LL*(rc->readable+rc->correctable))/rc->expectedSectors));

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
	   fflush(stdout);
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
   guint64 s;
   guint64 image_file_size;
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

   /* Please note: Commenting the follwing Stop() out will provide
      adaptive reading for RS01 and RS02, but behaviour with RS03
      is unpredictable und undefind. Therefore feel free to re-enable
      adaptive reading for your own use, but do not distibute such
      binaries to unsuspecting users.
      Adaptive reading will be re-introduced for all codecs
      in one of the next versions. */

#if 1
   Stop(_("Adaptive reading is unavailable in this version.\n"
	  "It will be re-introduced in one of the next versions."));
#endif

   /*** Open Device and .ecc file. Determine read mode. */

   open_and_determine_mode(rc);

   if(rc->readMode == IMAGE_ONLY)
   {  PrintCLI(_("* Warning: Using adaptive reading without error correction data\n"
		 "*          has little advantage over linear reading, but may\n"
		 "*          cause significant wear on the drive due to excessive\n"
		 "*          seek operations.\n"
		 "*          Please consider using linear reading instead.\n"));
   }
   
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
      rc->map = CreateBitmap0(rc->expectedSectors);

#ifdef CHECK_VISITED
   rc->count = g_malloc0((int)rc->expectedSectors+160);
#endif

   /*** Initialize segment state counters (only in GUI mode) */

   if(Closure->guiMode)
   {  //rc->sectorsPerSegment = 1 + (rc->sectors / ADAPTIVE_READ_SPIRAL_SIZE);
      rc->sectorsPerSegment = ((rc->expectedSectors+ADAPTIVE_READ_SPIRAL_SIZE-1) / ADAPTIVE_READ_SPIRAL_SIZE);
      rc->segmentState = g_malloc0(ADAPTIVE_READ_SPIRAL_SIZE * sizeof(int));
      //      ClipReadAdaptiveSpiral(rc->sectors/rc->sectorsPerSegment);
      ClipReadAdaptiveSpiral((rc->expectedSectors+rc->sectorsPerSegment-1)/rc->sectorsPerSegment);
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
	 
      if(rc->readable + rc->correctable >= rc->expectedSectors)
      {  char *t = _("\nSufficient data for reconstructing the image is available.\n");

	 if(rc->readMode != IMAGE_ONLY)
	 {  PrintLog(t);
	    if(Closure->guiMode)
	       SetAdaptiveReadFootline(t, Closure->greenText);
	 }
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

      Verbose("... Processing Interval [%lld..%lld], size %d\n",
	      rc->intervalStart, rc->intervalStart+rc->intervalSize-1, rc->intervalSize);
     
      /* If we jumped beyond the highest writtensector, 
	 fill the gap with dead sector markers. 
         Note that we do not have to fill anything if
         rc->intervalStart == rc->highestWrittenSector+1
         as then next sector after rc->highestWrittenSector will
         be written anyways. */

      if(rc->intervalStart > rc->highestWrittenSector+1)
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
	       {  int fixme;
		  ExplainMissingSector(rc->buf+i*2048, b, err, SOURCE_MEDIUM, &fixme);

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

	    if(rc->readable + rc->correctable >= rc->expectedSectors)
	    {  char *t = _("\nSufficient data for reconstructing the image is available.\n");

	       print_progress(rc, TRUE);
	       if(rc->readMode != IMAGE_ONLY)
	       {  PrintLog(t);
		  if(Closure->guiMode && rc->ei)
		    SetAdaptiveReadFootline(t, Closure->foreground);
	       }
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
         In both cases, the current interval has already been removed from the queue
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
      for(i=0; i<(int)rc->expectedSectors; i++)
      {  cnt+=rc->count[i];
         if(rc->count[i] != 1)
           printf("Sector %d: %d\n",i,rc->count[i]);
      }

      printf("\nTotal visited %d (%d)\n",cnt,i);

      for(i=(int)rc->expectedSectors; i<(int)rc->expectedSectors+160; i++)
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

   if(rc->map && (rc->readable + rc->correctable < rc->expectedSectors))
   {  int total = rc->readable+rc->correctable;
      int percent = (int)((1000LL*(long long)total)/rc->expectedSectors);
      char *t = g_strdup_printf(_("Only %2d.%1d%% of the image are readable or correctable"),
				  percent/10, percent%10); 

      PrintLog(_("\n%s\n"
		  "(%lld readable,  %lld correctable,  %lld still missing).\n"),
		t, rc->readable, rc->correctable, rc->expectedSectors-total);
      if(Closure->guiMode)
	 SetAdaptiveReadFootline(t, Closure->foreground);

      g_free(t);
      exitCode = EXIT_FAILURE;
   }

   /* Results for reading in IMAGE_ONLY mode */

   if(rc->readMode == IMAGE_ONLY)
   {  if(rc->readable == rc->expectedSectors)
      {  char *t = _("\nGood! All sectors have been read.\n"); 
	 PrintLog(t);
	 if(Closure->guiMode)
	   SetAdaptiveReadFootline(t, Closure->foreground);
	 if(Closure->eject)
	    LoadMedium(rc->dh, FALSE);
      }
      else
      {  int percent = (int)((1000LL*rc->readable)/rc->expectedSectors);
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
   for(i=0; i<rc->expectedSectors; i++)
     if(!GetBit(rc->map, i))
       printf("Missing: %d\n", i);
#endif

   //print_intervals(rc);

   /*** Close and clean up */

   rc->earlyTermination = FALSE;

terminate:
   cleanup((gpointer)rc);
}

