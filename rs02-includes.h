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

#ifndef RS02INCLUDES_H
#define RS02INCLUDES_H

/* Data structs from rs02-window.c */

typedef struct
{
   /*** Widgets for RS02 encoding */

   GtkWidget *encHeadline;
   GtkWidget *encLabel1;
   GtkWidget *encPBar1;
   GtkWidget *encLabel2;
   GtkWidget *encPBar2;
   GtkWidget *encFootline;
   GtkWidget *encFootline2;

   /*** Widgets for RS02 fixing */

   GtkWidget *fixHeadline;
   GtkWidget *fixDrawingArea;
   GtkWidget *fixNotebook;
   GtkWidget *fixFootline;
   GtkWidget *fixFootlineBox;
   GtkWidget *fixCorrected;
   GtkWidget *fixProgress;
   GtkWidget *fixUncorrected;
   Curve  *fixCurve;

   /*** Widgets for RS02 verify action */

   GtkWidget *cmpHeadline;
   GtkWidget *cmpDrawingArea;

   GtkWidget *cmpChkSumErrors;
   GtkWidget *cmpMissingSectors;

   Spiral    *cmpSpiral;
   PangoLayout *cmpLayout;

   GtkWidget *cmpImageSectors;
   GtkWidget *cmpImageMd5Sum;
   GtkWidget *cmpEccHeaders;
   GtkWidget *cmpDataSection;
   GtkWidget *cmpCrcSection;
   GtkWidget *cmpEccSection;
   GtkWidget *cmpImageResult;

   GtkWidget *cmpEccNotebook;
   GtkWidget *cmpEccCreatedBy;
   GtkWidget *cmpEccMethod;
   GtkWidget *cmpEccRequires;
   GtkWidget *cmpEccMediumSectors;
   GtkWidget *cmpEcc1Name;
   GtkWidget *cmpEcc2Name;
   GtkWidget *cmpEcc3Name;
   GtkWidget *cmpEcc1Msg;
   GtkWidget *cmpEcc2Msg;
   GtkWidget *cmpEcc3Msg;
   GtkWidget *cmpEccResult;

   /*** Widgets in the Preferences window */

   GtkWidget *radio1A, *radio1B, *radio2A, *radio2B;
   GtkWidget *cdButtonA, *dvdButton1A, *dvdButton2A, *bdButton1A, *bdButton2A;
   GtkWidget *cdButtonB, *dvdButton1B, *dvdButton2B, *bdButton1B, *bdButton2B;
   GtkWidget *cdUndoButtonA, *dvdUndoButton1A, *dvdUndoButton2A, *bdUndoButton1A, *bdUndoButton2A;
   GtkWidget *cdUndoButtonB, *dvdUndoButton1B, *dvdUndoButton2B, *bdUndoButton1B, *bdUndoButton2B;
   GtkWidget *cdEntryA, *dvdEntry1A, *dvdEntry2A, *bdEntry1A, *bdEntry2A, *otherEntryA;
   GtkWidget *cdEntryB, *dvdEntry1B, *dvdEntry2B, *bdEntry1B, *bdEntry2B, *otherEntryB;
   GtkWidget *cacheScaleA, *cacheScaleB;
   LabelWithOnlineHelp *cacheLwoh;

   /*** Some state vars used during fixing */

   gint64 corrected;
   gint64 uncorrected;
   gint64 nSectors;
   int    eccBytes;
   int    dataBytes;
   int    percent, lastPercent;
} RS02Widgets;

/*
 * local working closure for internal checksums
 */

typedef struct
{  struct _RS02Layout *lay;     /* Codec data layout */
   struct MD5Context md5ctxt;   /* Complete image checksum (currently unused) */
   struct MD5Context dataCtxt;  /* md5sum of original iso image portion */
   struct MD5Context crcCtxt;
   struct MD5Context eccCtxt;
   struct MD5Context metaCtxt;
} RS02CksumClosure;

/* 
 * These are exported via the Method struct 
 */

void CreateRS02EncWindow(Method*, GtkWidget*);
void CreateRS02FixWindow(Method*, GtkWidget*);
void CreateRS02PrefsPage(Method*, GtkWidget*);
void ResetRS02EncWindow(Method*);
void ResetRS02FixWindow(Method*);
void ResetRS02PrefsPage(Method*);
void ReadRS02Preferences(Method*);

void ResetRS02VerifyWindow(Method*);
void CreateRS02VerifyWindow(Method*, GtkWidget*);

/*
 * These are exported (resp. only used) in ecc-rs02.c and rs02*.c
 * and should not be called from somewhere else as we can not
 * rely on the method plug-in being available.
 * If you need similar functions in your own codec,
 * please copy these functions over to the respective plug-in.
 */

/* rs02-common.c */

typedef struct _RS02Layout
{  EccHeader *eh;                /* header for this image/ecc file */
   guint64 dataSectors;          /* number of sectors used for image data */
   guint64 crcSectors;           /* number of sectors needed for CRC32 sector checkums */
   guint64 firstEccHeader;       /* location of first ecc header */
   guint64 headers;              /* number of ecc header ("master block") repeats */
   guint64 headerModulo;         /* Modulo for header repeats */
   guint64 protectedSectors;     /* number of sectors protected by Reed-Solomon data */
   guint64 rsSectors;            /* number of sectors needed for Reed-Solomon data */
   guint64 eccSectors;           /* total number of sectors added to image */
   guint64 sectorsPerLayer;      /* sectors per RS layer (the are ndata layers) */
   guint64 firstCrcLayerIndex;   /* first slice containing a CRC32 data block */
   guint64 mediumCapacity;       /* selected medium capacity */
   int nroots,ndata;             /* RS encoding specification */
   double redundancy;            /* resulting redundancy */
} RS02Layout;

CrcBuf *RS02GetCrcBuf(Image*);
void RS02ResetCksums(Image*);
void RS02UpdateCksums(Image*, gint64, unsigned char*);
int RS02FinalizeCksums(Image*);

void RS02ReadSector(Image*, RS02Layout*, unsigned char*, gint64);
gint64 RS02EccSectorIndex(RS02Layout*, gint64, gint64);
gint64 RS02SectorIndex(RS02Layout*, gint64, gint64);
void RS02SliceIndex(RS02Layout*, gint64, gint64*, gint64*);
RS02Layout *CalcRS02Layout(Image*);
RS02Layout *RS02LayoutFromImage(Image*);
guint64 RS02ExpectedImageSize(Image*);
void WriteRS02Headers(LargeFile*, RS02Layout*, EccHeader*);

/* rs02-create.c */

void RS02Create(void);

/* rs02-fix.c */

void RS02Fix(Image*);

/* rs02-recognize.c */

int  RS02Recognize(Image*);

/* rs02-window.c */

void RS02AddFixValues(RS02Widgets*, int, int);
void RS02SetFixMaxValues(RS02Widgets*, int, int, gint64);
void RS02UpdateFixResults(RS02Widgets*, gint64, gint64);

/* rs02-verify.c */

#define VERIFY_IMAGE_SEGMENTS 1000

void RS02Verify(Image*);

#endif
