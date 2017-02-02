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

#ifndef SCSI_LAYER_H
#define SCSI_LAYER_H

#ifdef SYS_LINUX
#include <sys/ioctl.h>
#include <linux/cdrom.h>
#endif

#ifdef SYS_FREEBSD
#include <camlib.h>
#endif

/***
 *** Global settings
 ***/

/* Theretically not needed, but using less causes DMA breakage 
   on some chipsets. */

#define MIN_TRANSFER_LEN 4  

/***
 *** Define the Sense data structure.
 ***/

/* 
 * Linux already has one 
 */

#ifdef SYS_LINUX
#define MAX_CDB_SIZE CDROM_PACKET_SIZE

/* Now globally defined for all OSes here */
//typedef struct request_sense Sense;
#endif

#ifdef SYS_FREEBSD
#define MAX_CDB_SIZE SCSI_MAX_CDBLEN
#endif

#if defined(SYS_UNKNOWN) || defined(SYS_NETBSD)
#define MAX_CDB_SIZE 16   /* longest possible SCSI command */
#endif

/* 
 * The sense struct is named differently on various OSes,
 * Some have subtle differences in the used data types.
 * To avoid typecasting mayhem we simply reproduce the Linux
 * version here and use it on all OS versions.
 */

typedef struct _Sense {
	guint8 error_code		: 7;
	guint8 valid			: 1;
	guint8 segment_number;
	guint8 sense_key		: 4;
	guint8 reserved2		: 1;
	guint8 ili			: 1;
	guint8 reserved1		: 2;
	guint8 information[4];
	guint8 add_sense_len;
	guint8 command_info[4];
	guint8 asc;
	guint8 ascq;
	guint8 fruc;
	guint8 sks[3];
	guint8 asb[46];
} Sense;

/***
 ***  The DeviceHandle is pretty much our device abstraction layer. 
 ***
 * It contains info about the opened device and the inserted medium.
 */

typedef struct _DeviceHandle
{  /*
    * OS-specific data for device access
    */
#if defined(SYS_LINUX) || defined(SYS_NETBSD)
   int fd;                    /* device file descriptor */
#endif
#ifdef SYS_FREEBSD
   struct cam_device *camdev; /* camlib device handle */
   union ccb *ccb;
#endif

  /*
   * Simulated images
   */
  
   LargeFile *simImage;       /* Image for simulation mode */
   int pass;                  /* provided by the reader to simulate failure in specific passes */
  
   /*
    * OS-independent data about the device
    */

   char *device;              /* /dev/foo or whatever the OS uses to name it */
   char devinfo[34];          /* whole device info string from INQUIRY */
   char vendor[34];           /* vendor and product info only */

   Sense sense;               /* sense data from last operation */
   int senseSize;             /* OS may have differently sized struct */

   double singleRate;         /* supposed KB/sec @ single speed */
   int maxRate;               /* guessed maximum transfer rate */
   int clusterSize;           /* number of sectors per cluster */

   /*
    * Raw reading support
    */

   int canReadDefective;      /* TRUE if drive claims to raw read uncorrectable sectors */
   int canC2Scan;             /* TRUE if drive supports C2 error scanning */
   int c2[MAX_CLUSTER_SIZE];  /* C2 errors per sector */
   unsigned char previousReadMode;/* read mode prior to switching to raw reads */
   unsigned char previousRetries; /* retries prior to switching */
   unsigned char currentReadMode; /* current raw read mode */
   RawBuffer *rawBuffer;      /* for performing raw read analysis */
   int (*read)(struct _DeviceHandle*, unsigned char*, int, int);
   int (*readRaw)(struct _DeviceHandle*, unsigned char*, int, int);

   /* 
    * Information about currently inserted medium 
    */

   gint64 sectors;            /* actually used number of sectors */
   int sessions;              /* number of sessions */
   int layers;                /* and layers */
   char manuID[20];           /* Manufacturer info from ADIP/lead-in */
   int mainType;              /* CD or DVD */
   int subType;               /* see enum below */
   char *typeDescr;           /* human readable form of subType */
   int bookType;              /* book type */
   char *bookDescr;           /* human readable of above */
   int profile;               /* profile selected by drive */
   char *profileDescr;        /* human readable form of above */
   char *shortProfile;        /* short version of above */
   int isDash;                /* DVD- */
   int isPlus;                /* DVD+ */
   int incomplete;            /* disc is not finalized or otherwise broken */
   int discStatus;            /* obtained from READ DISC INFORMATION query */
   int rewriteable;
   char *mediumDescr;         /* textual description of medium */

   /*
    * size alternatives from different sources 
    */

   gint64 readCapacity;       /* value returned by READ CAPACITY */
   gint64 userAreaSize;       /* size of user area according to DVD Info struct */
   gint64 blankCapacity;      /* blank capacity (maybe 0 if query failed) */

   /*
    * debugging stuff
    */

   Bitmap *defects;           /* for defect simulation */
} DeviceHandle;

/* 
 * Media types seem not to be standardized anywhere,
 * so we make up our own here.
 */

#define MAIN_TYPE_MASK 0xf0

#define CD             0x10
#define DATA1          0x11
#define XA21           0x12

#define DVD            0x20
#define DVD_RAM        0x21
#define DVD_DASH_R     0x22
#define DVD_DASH_RW    0x23
#define DVD_PLUS_R     0x24
#define DVD_PLUS_RW    0x25
#define DVD_DASH_R_DL  0x26
#define DVD_DASH_RW_DL 0x27
#define DVD_PLUS_R_DL  0x28
#define DVD_PLUS_RW_DL 0x29

#define BD             0x40
#define BD_R           0x41
#define BD_RE          0x42

#define UNSUPPORTED    0x00 

/* transport io modes */

#define DATA_WRITE 0
#define DATA_READ  1
#define DATA_NONE  2

/***
 *** Exported functions
 ***/

/*
 * OS-dependent wrappers from scsi-<os>.c
 */

DeviceHandle* OpenDevice(char*);

int SendPacket(DeviceHandle*, unsigned char*, int, unsigned char*, int, Sense*, int);
int SimulateSendPacket(DeviceHandle*, unsigned char*, int, unsigned char*, int, Sense*, int);

/*** 
 *** scsi-layer.c
 ***
 * The really user-visible stuff
 */

enum 
{  MODE_PAGE_UNSET, 
   MODE_PAGE_SET
};


gint64 CurrentMediumSize(int);
void CloseDevice(DeviceHandle*);

int InquireDevice(DeviceHandle*, int); 
void SetRawMode(DeviceHandle*, int);

void SpinupDevice(DeviceHandle*);
void LoadMedium(struct _DeviceHandle*, int);
int  TestUnitReady(DeviceHandle*);

int ReadSectors(DeviceHandle*, unsigned char*, gint64, int);
int ReadSectorsFast(DeviceHandle*, unsigned char*, gint64, int);

#endif /* SCSI_LAYER_H */
