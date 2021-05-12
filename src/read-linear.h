/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2004-2017 Carsten Gnoerlich.
 *  Copyright (C) 2019-2021 The dvdisaster development team.
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

#ifndef READ_LINEAR_H
#define READ_LINEAR_H

/*
 * Local data package used during reading 
 */

#define READ_BUFFERS 128   /* equals 4MB of buffer space */

typedef struct
{  LargeFile *readerImage;  /* we need two file handles to prevent LargeSeek() */
   LargeFile *writerImage;  /* race conditions between the reader and writer */
   Image *image;
   Method *eccMethod;       /* Ecc method selected for this image */
   EccHeader *eccHeader;    /* accompanying Ecc header */
   GThread *worker;
   struct MD5Context md5ctxt;   /* Complete image checksum (RS01) */
   struct MD5Context dataCtxt;  /* Image section checksums (RS02) */
   struct MD5Context crcCtxt;   /* Image section checksums (RS02) */
   struct MD5Context eccCtxt;   /* Ecc layer checksum (RS02) */
   struct MD5Context metaCtxt;  /* Ecc meta checksum (RS02) */
   int doChecksumsFromImage;    /* calculate sector CRC and MD5 from image */
   int doChecksumsFromCodec;    /* let codec compute its own/additional checksums */
   int savedSectorSkip;
   char *volumeLabel;

   /* Data exchange between reader and worker */

   struct _AlignedBuffer *alignedBuf[READ_BUFFERS];
   gint64 bufferedSector[READ_BUFFERS];
   int nSectors[READ_BUFFERS];
   int bufState[READ_BUFFERS];
   GMutex *mutex;
   GCond *canRead, *canWrite;
   int readPtr,writePtr;
   char *workerError;

   /* for usage within the reader */

   gint64 firstSector, lastSector;   /* reading range */

   gint64 readPos;                   /* current sector reading position */
   Bitmap *readMap;                  /* map of already read sectors */

   gint64 readMarker;
   int rereading;                    /* TRUE if working on existing image */
   char *msg;
   GTimer *speedTimer,*readTimer;
   int unreportedError;
   int earlyTermination;
   int scanMode;
   int lastPercent;
   int firstSpeedValue;
   double speed,lastSpeed;
   gint64 readOK, lastReadOK;
   int previousReadErrors;
   int previousCRCErrors;
   gint64 deadWritten;
   gint64 lastErrorsPrinted;
   int pass;
   int maxC2;                       /* max C2 error since last output */
   int crcIncomplete;               /* CRC information was found incomplete (RS03 only) */
  
   /* for drawing the curve and spiral */

   gint lastCopied;
   gint lastSegment;
   gint lastPlotted;
   gint lastPlottedY;
   gint activeRenderers;
   GMutex *rendererMutex;

} read_closure;

#endif /* READ_LINEAR_H */
