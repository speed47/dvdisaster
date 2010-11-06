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

#ifndef RS03INCLUDES_H
#define RS03INCLUDES_H

/* Data structs from rs03-window.c */

typedef struct
{
   /*** Widgets for RS03 encoding */

   GtkWidget *encHeadline;
   GtkWidget *encLabel1;
   GtkWidget *encPBar1;
   GtkWidget *encLabel2;
   GtkWidget *encPBar2;
   GtkWidget *encLabel3;
   GtkWidget *encThreads;
   GtkWidget *encLabel4;
   GtkWidget *encPerformance;
   GtkWidget *encLabel5;
   GtkWidget *encBottleneck;
   GtkWidget *encFootline;
   GtkWidget *encFootline2;

   /*** Widgets for RS03 fixing */

   GtkWidget *fixHeadline;
   GtkWidget *fixDrawingArea;
   GtkWidget *fixNotebook;
   GtkWidget *fixFootline;
   GtkWidget *fixFootlineBox;
   GtkWidget *fixCorrected;
   GtkWidget *fixProgress;
   GtkWidget *fixUncorrected;
   Curve  *fixCurve;

   /*** Widgets for RS03 verify action */

   GtkWidget *cmpHeadline;
   GtkWidget *cmpDrawingArea;

   GtkWidget *cmpChkSumErrors;
   GtkWidget *cmpMissingSectors;

   Spiral    *cmpSpiral;
   PangoLayout *cmpLayout;

   GtkWidget *cmpImageSectors;
   GtkWidget *cmpImageMd5Sum;
   GtkWidget *cmpDataSection;
   GtkWidget *cmpCrcSection;
   GtkWidget *cmpEccSection;
   GtkWidget *cmpImageErasure;
   GtkWidget *cmpImagePrognosis;
   GtkWidget *cmpImageErasureCnt;
   GtkWidget *cmpImagePrognosisMsg;
   GtkWidget *cmpImageResult;

   GtkWidget *cmpEccCreatedBy;
   GtkWidget *cmpEccMethod;
   GtkWidget *cmpEccType;
   GtkWidget *cmpEccRequires;
   GtkWidget *cmpEccDataCrc;
   GtkWidget *cmpEccDataCrcVal;
   GtkWidget *cmpEccResult;
   GtkWidget *cmpEccSynLabel;
   GtkWidget *cmpEccSyndromes;

   /*** Widgets in the Preferences window */

   GtkWidget *eccFileA, *eccFileB;
   GtkWidget *eccImageA, *eccImageB;
   GtkWidget *radio1A,*radio2A,*radio3A,*radio4A;
   GtkWidget *radio1B,*radio2B,*radio3B,*radio4B;
   GtkWidget *radio4LabelA, *radio4LabelB;
   GtkWidget *redundancyNotebook;
   GtkWidget *redundancyScaleA, *redundancyScaleB;
   GtkWidget *redundancySpinA, *redundancySpinB;
   GtkWidget *prefetchScaleA, *prefetchScaleB;
   GtkWidget *threadsScaleA, *threadsScaleB;
   LabelWithOnlineHelp *prefetchLwoh;
   LabelWithOnlineHelp *threadsLwoh;

   /*** Some state vars used during fixing */

   gint64 corrected;
   gint64 uncorrected;
   gint64 nSectors;
   int    eccBytes;
   int    dataBytes;
   int    percent, lastPercent;
} RS03Widgets;

/* 
 * These are exported via the Method struct 
 */

void CreateRS03EncWindow(Method*, GtkWidget*);
void CreateRS03FixWindow(Method*, GtkWidget*);
void CreateRS03PrefsPage(Method*, GtkWidget*);
void ResetRS03EncWindow(Method*);
void ResetRS03FixWindow(Method*);
void ResetRS03PrefsPage(Method*);
void ReadRS03Preferences(Method*);

void ResetRS03VerifyWindow(Method*);
void CreateRS03VerifyWindow(Method*, GtkWidget*);

/*
 * These are exported (resp. only used) in ecc-rs03.c and rs03*.c
 * and should not be called from somewhere else as we can not
 * rely on the method plug-in being available.
 * If you need similar functions in your own codec,
 * please copy these functions over to the respective plug-in.
 */

/* rs03-common.c */

typedef struct _RS03Layout
{  EccHeader *eh;                /* header for this image/ecc file */
   guint64 dataSectors;          /* number of sectors used for image data */
   guint64 dataPadding;          /* padding sectors in last data layer */
   guint64 totalSectors;         /* data+padding+header+crc+ecc */ 
   guint64 sectorsPerLayer;      /* sectors per RS layer (the are ndata layers) */
   guint64 mediumCapacity;       /* selected medium capacity */
   guint64 eccHeaderPos;         /* location of first ecc header */
   guint64 firstCrcPos;          /* location of first crc sector */
   guint64 firstEccPos;          /* location of first ecc sector */
   int nroots,ndata;             /* RS encoding specification */
   int inLast;                   /* contents of last image file sector */
   double redundancy;            /* resulting redundancy */
   int target;                   /* 0: ecc file; 1: augmented image */
} RS03Layout;

#define RS03_READ_NOTHING 0x00
#define RS03_READ_DATA    0x01
#define RS03_READ_CRC     0x02
#define RS03_READ_ECC     0x04

void RS03ReadSectors(LargeFile*, RS03Layout*, unsigned char*, gint64, gint64, gint64, int);

gint64 RS03SectorIndex(RS03Layout*, gint64, gint64);
RS03Layout *CalcRS03Layout(gint64, EccHeader *, int);
void WriteRS03Header(LargeFile*, RS03Layout*, EccHeader*);
void ReconstructRS03Header(EccHeader*, CrcBlock*);

/* rs03-create.c */

void RS03Create(Method*);

/* rs03-fix.c */

void RS03Fix(Method*);

/* rs03-recognize.c */

int  RS03RecognizeFile(Method*, LargeFile*);
EccHeader* FindRS03HeaderInImage(LargeFile*);
int  RS03RecognizeImage(Method*, LargeFile*);

/* rs03-window.c */

void RS03AddFixValues(RS03Widgets*, int, int);
void RS03SetFixMaxValues(RS03Widgets*, int, int, gint64);
void RS03UpdateFixResults(RS03Widgets*, gint64, gint64);

/* rs03-verify.c */

#define VERIFY_IMAGE_SEGMENTS 1000

void RS03Verify(Method*);

/* temporary single threaded versions */

void RS03SCreate(Method*);
void CreateRS03SEncWindow(Method*, GtkWidget*);
void ResetRS03SEncWindow(Method*);

#endif
