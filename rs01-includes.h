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

#ifndef RS01INCLUDES_H
#define RS01INCLUDES_H

/* Data structs from rs01-window.c */

typedef struct
{
   /*** Widgets for RS01 encoding */

   GtkWidget *encHeadline;
   GtkWidget *encLabel1;
   GtkWidget *encPBar1;
   GtkWidget *encLabel2;
   GtkWidget *encPBar2;
   GtkWidget *encFootline;
   GtkWidget *encFootline2;
   GtkWidget *curveButton;

   /*** Widgets for RS01 fixing */

   GtkWidget *fixHeadline;
   GtkWidget *fixDrawingArea;
   GtkWidget *fixNotebook;
   GtkWidget *fixFootline;
   GtkWidget *fixFootlineBox;
   GtkWidget *fixCorrected;
   GtkWidget *fixProgress;
   GtkWidget *fixUncorrected;
   Curve  *fixCurve;

   /*** Widgets for RS01 verify action */

   GtkWidget *cmpHeadline;

   GtkWidget *cmpImageNotebook;
   GtkWidget *cmpImageSectors;
   GtkWidget *cmpChkSumErrors;
   GtkWidget *cmpMissingSectors;
   GtkWidget *cmpImageMd5Sum;
   GtkWidget *cmpImageResult;

   GtkWidget *cmpEccNotebook;
   GtkWidget *cmpEccEmptyMsg;
   GtkWidget *cmpEccCreatedBy;
   GtkWidget *cmpEccMethod;
   GtkWidget *cmpEccRequires;
   GtkWidget *cmpEccMediumSectors;
   GtkWidget *cmpEccImgMd5Sum;
   GtkWidget *cmpEccFingerprint;
   GtkWidget *cmpEccBlocks;
   GtkWidget *cmpEccMd5Sum;
   GtkWidget *cmpEccResult;

   GtkWidget *cmpDrawingArea;
   Spiral *cmpSpiral;
   PangoLayout *cmpLayout;

   /*** Widgets in the Preferences window */

   GtkWidget *radio1A,*radio2A,*radio3A,*radio4A;
   GtkWidget *radio1B,*radio2B,*radio3B,*radio4B;
   GtkWidget *radio4LabelA, *radio4LabelB;
   GtkWidget *redundancyScaleA, *redundancyScaleB;
   GtkWidget *redundancySpinA, *redundancySpinB;
   GtkWidget *cacheScaleA, *cacheScaleB;
   LabelWithOnlineHelp *cacheLwoh;

   /*** Some state vars used during fixing */

   gint64 corrected;
   gint64 uncorrected;
   gint64 nSectors;
   int    eccBytes;
   int    dataBytes;
   int    percent, lastPercent;
} RS01Widgets;

/*
 * local working closure for internal checksums
 */

typedef struct
{  struct MD5Context md5ctxt;   /* Complete image checksum */
} RS01CksumClosure;

/* 
 * These are exported via the Method struct 
 */

void CreateRS01EWindow(Method*, GtkWidget*);
void CreateRS01FWindow(Method*, GtkWidget*);
void CreateRS01PrefsPage(Method*, GtkWidget*);
void ResetRS01EncodeWindow(Method*);
void ResetRS01FixWindow(Method*);
void ResetRS01PrefsPage(Method*);

void RS01ShowCurveButton(Method*);

void ResetRS01VerifyWindow(Method*);
void CreateRS01VerifyWindow(Method*, GtkWidget*);

/*
 * These are exported (resp. only used) in ecc-rs01.c and rs01*.c
 * and should not be called from somewhere else as we can not
 * rely on the method plug-in being available.
 * If you need similar functions in your own codec,
 * please copy these functions over to the respective plug-in.
 */

/* rs01-common.c */

#define READABLE_IMAGE    0
#define READABLE_ECC      0
#define WRITEABLE_IMAGE   (1<<0)
#define WRITEABLE_ECC     (1<<1)
#define PRINT_MODE        (1<<4)
#define CREATE_CRC        ((1<<1) | (1<<5))

CrcBuf *RS01GetCrcBuf(Image*);
void RS01ResetCksums(Image*);
void RS01UpdateCksums(Image*, gint64, unsigned char*);
int RS01FinalizeCksums(Image*);
void RS01ReadSector(Image*, unsigned char*, gint64);
void RS01ScanImage(Method*, Image*, struct MD5Context*, int);
int  RS01Recognize(LargeFile*, EccHeader**);
guint64 RS01ExpectedImageSize(Image*);

/* rs01-create.c */

void RS01Create(void);

/* rs01-fix.c */

void RS01Fix(Image*);

/* rs01-window.c */

void RS01AddFixValues(RS01Widgets*, int, int);
void RS01SetFixMaxValues(RS01Widgets*, int, int, gint64);
void RS01UpdateFixResults(RS01Widgets*, gint64, gint64);

/* rs01-verify.c */

#define VERIFY_IMAGE_SEGMENTS 1000

void RS01Verify(Image*);
void RS01AddVerifyValues(Method*, int, gint64, gint64, gint64, gint64);

#endif
