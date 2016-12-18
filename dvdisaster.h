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

#ifndef DVDISASTER_H
#define DVDISASTER_H

/* "Dare to be gorgeous and unique. 
 *  But don't ever be cryptic or otherwise unfathomable.
 *  Make it unforgettably great."
 *
 *  From "A Final Note on Style", 
 *  Amiga Intuition Reference Manual, 1986, p. 231
 */

/***
 *** I'm too lazy to mess with #include dependencies.
 *** Everything #includeable is rolled up herein...
 */

#define _GNU_SOURCE

#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#ifdef WITH_NLS_YES
 #include <libintl.h>
 #include <locale.h>
#endif
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "md5.h"

#ifndef G_THREADS_ENABLED
 #error "need multithreading glib2"
#endif

/* Phrase extraction for gettext() 
   Note that these functions are even required when
   WITH_NLS_NO is set! */

#define _(string) sgettext(string)
#define _utf(string) sgettext_utf8(string)

/* File permissions for images */

#define IMG_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

/* Using round() is preferred over rint() on systems which have it */

#ifndef HAVE_ROUND
 #define round(x) rint(x)
#endif

/* Some standard media sizes */

#define CDR_SIZE         (351*1024)
#define DVD_SL_SIZE      2295104  /* DVD+R/RW size used at least common denominator */
#define DVD_DL_SIZE 	 4171712  /* also seen: 4148992 4173824  */
#define BD_SL_SIZE      11826176
#define BD_DL_SIZE	23652352

/* Maximum accepted media sizes (in 2K sectors) */

#define MAX_CDR_SIZE     (100*60*75)     /* CDs can't have >100min w/o severe hacks  */
#define MAX_DVD_SL_SIZE  2500000         /* I have to guess here */
#define MAX_DVD_DL_SIZE  4600000         /* I have to guess here */

/* Maximum number of parallel encoder/decoder threads */

#define MAX_CODEC_THREADS 1024           /* not including IO and GUI */
#define MAX_OLD_CACHE_SIZE  8096         /* old cache for RS01/RS02  */
#define MAX_PREFETCH_CACHE_SIZE (512*1024)   /* upto 0.5TB RS03  */

/* Choices for I/O strategy */

#define IO_STRATEGY_READWRITE 0
#define IO_STRATEGY_MMAP 1

/* SCSI driver selection on Linux */

#define DRIVER_NONE 0
#define DRIVER_CDROM 1
#define DRIVER_SG 3

/* Definitions for Closure->eccTarget */

#define ECC_FILE  0
#define ECC_IMAGE 1

/* Definitions for Closure->stopActions */

#define STOP_CURRENT_ACTION 1
#define STOP_SHUTDOWN_ALL 2

/***
 *** Our global closure (encapsulation of global variables)
 ***/

typedef struct _GlobalClosure
{  int version;         /* Integer number representing current program version */
   char *cookedVersion; /* version string formatted for GUI use */
   char *versionString; /* more detailed version string */
   char *device;        /* currently selected device to read from */
   GPtrArray *deviceNames;  /* List of drive names */
   GPtrArray *deviceNodes;  /* List of device nodes (C: or /dev/foo) */
   char *imageName;     /* complete path of current image file */
   char *eccName;       /* complete path of current ecc file */
   GPtrArray *methodList; /* List of available methods */
   char *methodName;    /* Name of currently selected codec */
   gint64 readStart;    /* Range to read */
   gint64 readEnd;
   gint64 cdSize;       /* Maximum cd size (for RS02 type images) */
   gint64 dvdSize1;     /* Maximum 1-layer dvd size (for augmented images) */
   gint64 dvdSize2;     /* Maximum 2-layer dvd size (for augmented images) */
   gint64 bdSize1;      /* Maximum 1-layer dvd size (for augmented images) */
   gint64 bdSize2;      /* Maximum 2-layer dvd size (for augmented images) */
   gint64 savedCDSize;  /* Undo values for above */
   gint64 savedDVDSize1;
   gint64 savedDVDSize2;
   gint64 savedBDSize1;
   gint64 savedBDSize2;
   gint64 mediumSize;   /* Maximum medium size (for augmented images) */
   int cacheMiB;        /* Cache setting for the parity codec, in megabytes */
   int prefetchSectors; /* Prefetch setting per encoder thread */
   int codecThreads;    /* Number of threads to use for RS encoders */
   int encodingAlgorithm; /* Force a certain codec type for RS03 */
   int encodingIOStrategy; /* Force a IO strategy for RS03 encoding */
   int sectorSkip;      /* Number of sectors to skip after read error occurs */
   char *redundancy;    /* Error correction code redundancy */
   int eccTarget;       /* 0=file; 1=augmented image */
   int readRaw;         /* Read CD sectors raw + verify them */
   int rawMode;         /* mode for mode page */
   int minReadAttempts; /* minimum reading attempts */
   int maxReadAttempts; /* maximal reading attempts */
   int internalAttempts;/* read attempts by the drive itself */
   int adaptiveRead;    /* Use optimized strategy for reading defective images */
   int speedWarning;    /* Print warning if speed changes by more than given percentage */
   int fillUnreadable;  /* Byte value for filling unreadable sectors or -1 */
   int spinupDelay;     /* Seconds to wait for drive to spin up */
   int truncate;        /* confirms truncation of large images */
   int noTruncate;      /* do not truncate image at the end */
   int dsmVersion;      /* 1 means new style dead sector marker */
   int unlinkImage;     /* delete image after ecc file creation */
   int confirmDeletion; /* do not ask whether files should be deleted */
   int driveSpeed;      /* currently unused */
   int debugMode;       /* may activate additional features */
   int debugCDump;      /* dump as #include file instead of hexdump */
   int verbose;         /* may activate additional messages */
   int quickVerify;     /* do only non time-consuming verify actions */
   int screenShotMode;  /* screen shot mode */
   int autoSuffix;      /* automatically extend files with suffices .iso/.ecc */
   int ignoreIsoSize;   /* get size per READ CAPACITY; ignore all ISO/UDF meta data */
   int examineRS02;     /* perform deep search for RS02 structures */
   int examineRS03;     /* perform deep search for RS03 structures */
   int readAndCreate;   /* automatically create .ecc file after reading an image */
   int enableCurveSwitch; /* TRUE in readAndCreateMode after reading is complete */
   int welcomeMessage;  /* just print dvdisaster logo if FALSE */
   int dotFileVersion;  /* version of dotfile */
   int simulateDefects; /* if >0, this is the percentage of simulated media defects */
   char *simulateCD;    /* Simulate CD from given image */
   int defectiveDump;   /* dump non-recoverable sectors into given path */
   char *dDumpDir;      /* directory for above */
   char *dDumpPrefix;   /* file name prefix for above */
   int reverseCancelOK; /* if TRUE the button order is reversed */
   int eject;           /* eject medium on success */
   int readingPasses;   /* try to read medium n times */
   int pauseAfter;      /* pause after given amount of minutes */
   int pauseDuration;   /* duration of pause in minutes */
   int pauseEject;      /* Eject medium during pause */
   int ignoreFatalSense;/* Continue reading after potential fatal sense errors */
   int useSSE2;         /* TRUE means to use SSE2 version of the codec. */
   int useAltiVec;      /* TRUE means to use AltiVec version of the codec. */
   int clSize;          /* Bytesize of cache line */
   int useSCSIDriver;   /* Whether to use generic or sg driver on Linux */
   int fixedSpeedValues;/* output fixed speed reading to make comparing debugging output easier */  
   char *homeDir;       /* path to users home dir */
   char *dotFile;       /* path to .dvdisaster file */
   char *logFile;       /* path to logfile */
   int  logFileEnabled; /* logfile enabled */
   int  logFileStamped; /* time stamp written to log file */
   char *binDir;        /* place where the binary resides */
   char *docDir;        /* place where our documentation resides */
   char *viewer;        /* Name of preferred PDF viewer */

   GMutex progressLock; /* A mutex protected the stuff below */
   char bs[256];        /* A string of 255 backspace characters */
   char sp[256];        /* A string of 255 space characters */
   int progressLength;  /* Length of last progress msg printed */

   GThread *mainThread; /* Thread of the main() routine */
   void (*cleanupProc)(gpointer);  /* Procedure to cleanup running threads after an error condition */
   gpointer cleanupData;
   GThread *subThread;  /* Wait for this thread to terminate after an abort action */
   char *errorTitle;    /* Title to show in error dialogs */
   gint32 randomSeed;   /* for the random number generator */

   guint32 *crcCache;              /* sectorwise CRC32 for last image read */
   char    *crcImageName;          /* file name of cached image */
   unsigned char md5Cache[16];     /* md5sum of last image read */
   

   /*** GUI-related things */

   int guiMode;              /* TRUE if GUI is active */
   int stopActions;          /* crude method to stop ongoing action(s) */
   int noMissingWarnings;    /* suppress warnings about inconsistent missing sectors */

   GtkWidget *logWidget;     /* Dialog for the log display */
   GtkScrolledWindow *logScroll; /* and its scrolled window */
   GtkTextBuffer *logBuffer; /* Text buffer for the log output */
   GString *logString;       /* holds logging output for current action */
   GMutex *logLock;          /* protects the logString */

   /*** Widgets of the main window */

   GtkWindow *window;        /* main window */
   GtkTooltips *tooltips;    /* our global tooltips structure */
   GdkPixbuf *windowIcon;    /* main window icon */

   GtkWidget *fileMenuImage;  /* Select image entry */
   GtkWidget *fileMenuEcc;    /* Select Parity File entry */
   GtkWidget *toolMenuAnchor; /* Anchor of tool menu */

   GtkWidget *driveCombo;    /* combo box for drive selection */

   GtkWidget *imageFileSel;  /* image file selector */
   GtkWidget *imageEntry;    /* image name entry field */
   GtkWidget *eccFileSel;    /* ecc file selector */
   GtkWidget *eccEntry;      /* ecc name entry field */
   
   GtkWidget *notebook;      /* The notebook behind our central output area */
   GtkLabel  *status;        /* The status label */

   GtkWidget *prefsButton;
   GtkWidget *helpButton;

   GtkWidget *readButton;
   GtkWidget *createButton;
   GtkWidget *scanButton;
   GtkWidget *fixButton;
   GtkWidget *testButton;

   /*** The preferences window */

   GtkWindow *prefsWindow;
   void *prefsContext;       /* local data for the preferences window */

   /*** The raw editor window */

   void *rawEditorContext;

   /*** The medium info window */

   GtkWidget *mediumWindow;   /* Dialog for the medium info window */
   GtkWidget *mediumDrive;
   void *mediumInfoContext;   /* private data */

   /*** Common stuff for drawing curves and spirals */

   GdkGC     *drawGC;
   GdkColor  *background,*foreground,*grid;
   GdkColor  *redText;
   char      *redMarkup;
   GdkColor  *greenText;
   char      *greenMarkup;
   GdkColor  *barColor;
   GdkColor  *logColor;
   GdkColor  *curveColor;
   GdkColor  *redSector;
   GdkColor  *yellowSector;
   GdkColor  *greenSector;
   GdkColor  *blueSector;
   GdkColor  *whiteSector;
   GdkColor  *darkSector;
   char      *invisibleDash;

   /*** Widgets for the linear reading/scanning action */

   GtkWidget *readLinearHeadline;
   GtkWidget *readLinearDrawingArea;
   struct _Curve  *readLinearCurve;
   struct _Spiral *readLinearSpiral;
   GtkWidget *readLinearNotebook;
   GtkWidget *readLinearSpeed;
   GtkWidget *readLinearErrors;
   GtkWidget *readLinearFootline;
   GtkWidget *readLinearFootlineBox;
   gint64 crcErrors, readErrors;  /* these are passed between threads and must therefore be global */
   int    crcAvailable;           /* true when CRC data is available while reading/scanning */

   /*** Widgets for the adaptive reading action */

   GtkWidget *readAdaptiveHeadline;
   GtkWidget *readAdaptiveDrawingArea;
   struct _Spiral *readAdaptiveSpiral;
   char *readAdaptiveSubtitle;
   char *readAdaptiveErrorMsg;
   int additionalSpiralColor;

} GlobalClosure;

extern GlobalClosure *Closure;  /* these should be the only global variables! */
extern int exitCode;            /* value to use on exit() */

/***
 *** 
 ***/

#define MAX_FILE_SEGMENTS 100

typedef struct _LargeFile
{  int fileHandle;
   guint64 offset;
   char *path;
   guint64 size;
   int flags;
} LargeFile;

/***
 *** Aligned 64bit data types
 ***
 * Needed to prevent 4 byte packing on 32bit systems.
 */

#define aligned_gint64 gint64 __attribute__((aligned(8)))
#define aligned_guint64 guint64 __attribute__((aligned(8)))

/***
 *** The .ecc file header
 ***/

#define FINGERPRINT_SECTOR 16 /* Sector currently used to calculate the fingerprint. */
                              /* This is the ISO filesystem root sector which contains */
                              /* the volume label and creation time stamps. */
                              /* Versions upto 0.64 used sector 257, */
                              /* but that was not a wise choice for CD media.*/

#define MFLAG_DEVEL (1<<0)    /* for methodFlags[3] */
#define MFLAG_RC    (1<<1)                      

#define MFLAG_DATA_MD5 (1<<0)  /* RS03: md5sum for data part available */
#define MFLAG_ECC_FILE (1<<1)  /* RS03: This is a ecc file */

typedef struct _EccHeader
{  gint8 cookie[12];           /* "*dvdisaster*" */
   gint8 method[4];            /* e.g. "RS01" */
   gint8 methodFlags[4];       /* 0-2 for free use by the respective methods; 3 see above */
   guint8 mediumFP[16];        /* fingerprint of FINGERPRINT SECTOR */ 
   guint8 mediumSum[16];       /* complete md5sum of whole medium */
   guint8 eccSum[16];          /* md5sum of ecc code section of .ecc file */
   guint8 sectors[8];          /* number of sectors medium is supposed to have w/o ecc */
   gint32 dataBytes;           /* data bytes per ecc block */
   gint32 eccBytes;            /* ecc bytes per ecc block */
   gint32 creatorVersion;      /* which dvdisaster version created this */
   gint32 neededVersion;       /* oldest version which can decode this file */
   gint32 fpSector;            /* sector used to calculate mediumFP */
   guint32 selfCRC;            /* CRC32 of EccHeader -- since V0.66 --*/
   guint8 crcSum[16];          /* md5sum of crc code section of RS02 .iso file  */
   gint32 inLast;              /* bytes contained in last sector */
   aligned_guint64 sectorsPerLayer;    /* layer size for RS03 */
   aligned_guint64 sectorsAddedByEcc;  /* sectors added by RS02/RS03 */
   gint8 padding[3960];        /* pad to 4096 bytes: room for future expansion */

  /* Note: Bytes 2048 and up are currently used by the RS02/RS03 codec
           for a copy of the first ecc blocks CRC sums. */
} EccHeader;

/***
 *** The CRC block data structure
 ***
 * RS03 uses this data structure in its CRC layer.
 */

typedef struct _CrcBlock
{  guint32 crc[256];           /* Checksum for the data sectors */
   gint8 cookie[12];           /* "*dvdisaster*" */
   gint8 method[4];            /* e.g. "RS03" */
   gint8 methodFlags[4];       /* 0-2 for free use by the respective methods; 3 see above */
   gint32 creatorVersion;      /* which dvdisaster version created this */
   gint32 neededVersion;       /* oldest version which can decode this file */
   gint32 fpSector;            /* sector used to calculate mediumFP */
   guint8 mediumFP[16];        /* fingerprint of FINGERPRINT SECTOR */ 
   guint8 mediumSum[16];       /* complete md5sum of whole medium */
   aligned_guint64 dataSectors;/* number of sectors of the payload (e.g. iso file sys) */
   gint32 inLast;              /* bytes contained in last sector */
   gint32 dataBytes;           /* data bytes per ecc block */
   gint32 eccBytes;            /* ecc bytes per ecc block */
   aligned_guint64 sectorsPerLayer; /* for recalculation of layout */
   guint32 selfCRC;            /* CRC32 of ourself, zero padded to 2048 bytes */
} CrcBlock;

/***
 *** forward declarations
 ***/

struct _RawBuffer *rawbuffer_forward;
struct _DefectiveSectorHeader *dsh_forward;
struct _DeviceHandle *dh_forward;

/***
 *** bitmap.c
 ***/

typedef struct _Bitmap
{  guint32 *bitmap;
   gint32 size;
   gint32 words;
} Bitmap;

Bitmap* CreateBitmap0(int);
#define GetBit(bm,bit) (bm->bitmap[(bit)>>5] & (1<<((bit)&31))) 
#define SetBit(bm,bit) bm->bitmap[(bit)>>5] |= (1<<((bit)&31)) 
#define ClearBit(bm,bit) bm->bitmap[(bit)>>5] &= ~(1<<((bit)&31)) 
int CountBits(Bitmap*);
void FreeBitmap(Bitmap*);

/***
 *** build.h
 ***/

int buildCount;

/***
 *** cacheprobe.h
 ***/

int ProbeCacheLineSize();

/***
 *** closure.c
 ***/

void InitClosure(void);
void LocalizedFileDefaults(void);
void UpdateMarkup(char**, GdkColor*);
void DefaultColors(void);
void ClearCrcCache(void);
void FreeClosure(void);
void ReadDotfile(void);
void WriteSignature(void);
int  VerifySignature(void);

/***
 *** crc32.c
 ***/

guint32 Crc32(unsigned char*, int);
guint32 EDCCrc32(unsigned char*, int);

/***
 *** crcbuf.c
 ***/

typedef struct _CrcBuf
{  guint32 *crcbuf;
   guint64 size;
   Bitmap *valid;
} CrcBuf;

enum
{  CRC_UNKNOWN,
   CRC_BAD,
   CRC_GOOD,
   CRC_OUTSIDE_BOUND
};

CrcBuf *CreateCrcBuf(guint64);
void FreeCrcBuf(CrcBuf*);

int CheckAgainstCrcBuffer(CrcBuf*, gint64, unsigned char*);

/***
 *** curve.c
 ***/

enum 
{  CURVE_PERCENT,
   CURVE_MEGABYTES
}; 

typedef struct _Curve
{  GtkWidget *widget;   /* drawing area which is hosting us */
   PangoLayout *layout;
   gdouble *fvalue;     /* floating point curve values */
   gint    *ivalue;     /* integer bar curve values */
   gint    *lvalue;     /* logarithmic integer curve */
   gint    enable;      /* which of the above should be drawn */
   gint lastValueIdx;   /* end of value array */
   gint leftX,rightX;   /* Position of left and right y axis */
   gint topY,bottomY;   /* Top and bottom end of i/f y axes */
   gint topLY,bottomLY; /* Top and bottom end of l y axes */
   char *leftLabel;     /* Label of left coordinate axis */
   char *leftLogLabel;  /* Label of left log coordinate axis */
   char *leftFormat;    /* Format of left coordinate axis numbering */
   int bottomFormat;    /* what kind of data are we displaying? */
   int margin;
   int maxX,maxY,logMaxY; /* current max value at y axis */
} Curve;

#define DRAW_ICURVE (1<<0)
#define DRAW_FCURVE (1<<1)
#define DRAW_LCURVE (1<<2)

Curve* CreateCurve(GtkWidget*, char*, char*, int, int);
void ZeroCurve(Curve*);
void FreeCurve(Curve*);

void UpdateCurveGeometry(Curve*, char*, int);

int  CurveX(Curve*, gdouble);
int  CurveY(Curve*, gdouble);
int  CurveLogY(Curve*, gdouble);
void RedrawAxes(Curve*);
void RedrawCurve(Curve*, int);

/***
 *** debug.c
 ***/

void HexDump(unsigned char*, int, int);
void LaTeXify(gint32*, int, int);
void AppendToTextFile(char*,char*, ...);
void CopySector(char*);
void Byteset(char*);
void Erase(char*);
void MergeImages(char*, int);
void RandomError(char*);
void RandomImage(char*, char*, int);
void RawSector(char*);
void ReadSector(char*);
void SendCDB(char*);
void ShowHeader(char*);
void ShowSector(char*);
Bitmap* SimulateDefects(gint64);
void TruncateImageFile(char*);
void ZeroUnreadable(void);

/***
 *** ds-marker.c
 ***/

enum
{  SECTOR_PRESENT,
   SECTOR_MISSING,
   SECTOR_MISSING_DISPLACED,
   SECTOR_MISSING_WRONG_FP,
   SECTOR_WITH_SIMULATION_HINT
};

enum
{  SOURCE_MEDIUM,
   SOURCE_IMAGE,
   SOURCE_ECCFILE
};
   
void CreateMissingSector(unsigned char*, guint64, unsigned char*, guint64, char*);
void CreateDebuggingSector(unsigned char*, guint64, unsigned char*, guint64, char*, char*);
int CheckForMissingSector(unsigned char*, guint64, unsigned char*, guint64);
int CheckForMissingSectors(unsigned char*, guint64, unsigned char*, guint64, int, guint64*);
void ExplainMissingSector(unsigned char*, guint64, int, int, int*);

void CreatePaddingSector(unsigned char*, guint64, unsigned char*, guint64);

char *GetSimulationHint(unsigned char*);

/***
 *** endian.c
 ***/

guint32 SwapBytes32(guint32);
guint64 SwapBytes64(guint64);
void    SwapEccHeaderBytes(EccHeader*);
void    SwapDefectiveHeaderBytes(struct _DefectiveSectorHeader*);
void    SwapCrcBlockBytes(CrcBlock*);
void    PrintEccHeader(EccHeader*);

/***
 *** fix-window.c
 ***/

void CreateFixWindow(GtkWidget*);

/***
 *** galois.c
 ***
 * This is currently the hardcoded GF(2**8).
 * gint32 gives abundant space for the GF.
 * Squeezing it down to guint8 won't probably gain much,
 * so we implement this defensively here.
 *
 * Note that some performance critical stuff needs to
 * be #included from galois-inlines.h
 */  

/* Galois field parameters for 8bit symbol Reed-Solomon code */

#define GF_SYMBOLSIZE 8
#define GF_FIELDSIZE (1<<GF_SYMBOLSIZE)
#define GF_FIELDMAX (GF_FIELDSIZE-1)
#define GF_ALPHA0 GF_FIELDMAX

/* RS polynomial parameters for RS01/RS02 codec */

#define RS_GENERATOR_POLY 0x187    /* 1 + X + X**2 + X**7 + X**8 */
#define RS_FIRST_ROOT 112          /* same choices as in CCSDS */                 
#define RS_PRIM_ELEM 11                  
#define RS_PRIMTH_ROOT 116         /* prim-th root of 1 */           

/* Lookup tables for Galois field arithmetic */

typedef struct _GaloisTables
{  gint32 gfGenerator;  /* GF generator polynomial */ 
   gint32 *indexOf;     /* log */
   gint32 *alphaTo;     /* inverse log */
   gint32 *encAlphaTo; /* inverse log optimized for encoder */
} GaloisTables;

/* Lookup and working tables for the ReedSolomon codecs */

typedef struct _ReedSolomonTables
{  GaloisTables *gfTables;/* from above */
   gint32 *gpoly;        /* RS code generator polynomial */
   gint32 fcr;           /* first consecutive root of RS generator polynomial */
   gint32 primElem;      /* primitive field element */
   gint32 nroots;        /* degree of RS generator polynomial */
   gint32 ndata;         /* data bytes per ecc block */
   gint32 shiftInit;     /* starting value for iteratively processing parity */

   guint8 *bLut[GF_FIELDSIZE];   /* 8bit encoder lookup table */
   guint8 *synLut;       /* Syndrome calculation speedup */
} ReedSolomonTables;

GaloisTables* CreateGaloisTables(gint32);
void FreeGaloisTables(GaloisTables*);

ReedSolomonTables *CreateReedSolomonTables(GaloisTables*, gint32, gint32, int);
void FreeReedSolomonTables(ReedSolomonTables*);

/***
 *** help-dialogs.c
 ***/

/* Creating labels with links to online help */

typedef struct _LabelWithOnlineHelp
{  GtkWidget *helpWindow;
   GtkWidget *normalLabel;
   GtkWidget *linkBox;
   GtkWidget *linkLabel;
   GtkWidget *vbox;
   GPtrArray *lastSizes;  /* for breaking expose loops between the help windows */
  
   char *windowTitle;
   char *normalText;
   char *highlitText;
   int inside;
   int outerPadding;   /* Padding between window and outer vbox */
} LabelWithOnlineHelp;

LabelWithOnlineHelp* CreateLabelWithOnlineHelp(char*, char*);
LabelWithOnlineHelp* CloneLabelWithOnlineHelp(LabelWithOnlineHelp*, char*);
void FreeLabelWithOnlineHelp(LabelWithOnlineHelp*);
void SetOnlineHelpLinkText(LabelWithOnlineHelp*, char*);
void AddHelpListItem(LabelWithOnlineHelp*, char*, ...);
void AddHelpParagraph(LabelWithOnlineHelp*, char*, ...);
void AddHelpWidget(LabelWithOnlineHelp*, GtkWidget*);

/* Specific online help dialogs */

GtkWidget* ShowTextfile(char*, char*, char*, GtkScrolledWindow**, GtkTextBuffer**);
void ShowGPL();
void ShowLog();
void UpdateLog();
void AboutDialog();

void AboutText(GtkWidget*, char*, ...);
void AboutLink(GtkWidget*, char*, char*);
void AboutTextWithLink(GtkWidget*, char*, char*);

/***
 *** heuristic-lec.c
 ***/

void UpdateByteCounts(struct _RawBuffer*);
void CalculatePQLoad(struct _RawBuffer*);
void UpdatePQParityList(struct _RawBuffer*, unsigned char*);

int HeuristicLEC(unsigned char*, struct _RawBuffer*, unsigned char*);
int SearchPlausibleSector(struct _RawBuffer*, int);
int BruteForceSearchPlausibleSector(struct _RawBuffer*);
int AckHeuristic(struct _RawBuffer*);

/***
 *** icon-factory.c
 ***/

void CreateIconFactory();

/***
 *** image.c
 ***/

enum {IMAGE_NONE, IMAGE_FILE, IMAGE_MEDIUM};
enum {FP_UNKNOWN, FP_UNREADABLE, FP_PRESENT};
enum {ECCFILE_PRESENT, ECCFILE_MISSING, ECCFILE_INVALID, ECCFILE_DEFECTIVE_HEADER, ECCFILE_WRONG_CODEC};

typedef struct _Image
{  int type;                   /* file or medium */

   /* physical storage handles */
  
   LargeFile *file;
   struct _DeviceHandle *dh;

   /*
    * info on file system(s) found on medium
    */

   struct _IsoInfo *isoInfo;  /* Information gathered from ISO filesystem */
   struct _Method *eccMethod; /* Method used for augmenting the image */
   EccHeader *eccHeader;      /* copy of ecc header in augmented image */
   guint64 expectedSectors;   /* Image size according to codec (including augmented parts) */

   guint64 sectorSize;        /* size in sectors if this is a file based image */
   int inLast;                /* files may have incomplete last sector */
   guint64 sectorsMissing;    /* number of missing sectors */
   guint64 crcErrors;         /* number of sectors with crc errors */
   guint8 mediumSum[16];      /* complete md5sum of whole medium */

   /*
    * image fingerprint caching
    */

   guint8 imageFP[16];        /* Image fingerprint */
   guint64 fpSector;          /* Sector used for calculating the fingerprint */
   int fpState;               /* 0=unknown; 1=unreadable; 2=present */

  /*
   * layout caching
   */

   void *cachedLayout;        /* Layout of different codecs */

   /*
    * optionally attached ecc file
    */

   LargeFile *eccFile;        /* ecc file */
   int eccFileState;          /* see enum above */
   struct _Method *eccFileMethod;     /* method associated with it */
   EccHeader *eccFileHeader;  /* copy of ecc header in ecc file */
} Image;

Image* OpenImageFromFile(char*, int, mode_t);
Image* OpenImageFromDevice(char*);   /* really in scsi-layer.h */
Image* OpenEccFileForImage(Image*, char*, int, mode_t);
int ReportImageEccInconsistencies(Image*);
int GetImageFingerprint(Image*, guint8*, guint64);
void ExamineECC(Image*);
int ImageReadSectors(Image*, void*, guint64, int);
int TruncateImage(Image*, guint64);
void CloseImage(Image*);

/***
 *** large-io.c
 ***/

LargeFile *LargeOpen(char*, int, mode_t);
int LargeSeek(LargeFile*, off_t);
int LargeEOF(LargeFile*);
ssize_t LargeRead(LargeFile*, void*, size_t);
ssize_t LargeWrite(LargeFile*, void*, size_t);
int LargeClose(LargeFile*);
int LargeTruncate(LargeFile*, off_t);
int LargeStat(char*, guint64*);
int LargeUnlink(char*);

int DirStat(char*);
FILE *portable_fopen(char*, char*);
int portable_mkdir(char*);
char *ApplyAutoSuffix(char*, char*);

/*** 
 *** l-ec.c
 ***/

#define N_P_VECTORS   86      /* 43 16bit p vectors */
#define P_VECTOR_SIZE 26      /* using RS(26,24) ECC */

#define N_Q_VECTORS   52      /* 26 16bit q vectors */
#define Q_VECTOR_SIZE 45      /* using RS(45,43) ECC */

#define P_PADDING 229         /* padding values for */
#define Q_PADDING 210         /* shortened RS code  */

int PToByteIndex(int, int);
int QToByteIndex(int, int);
void ByteIndexToP(int, int*, int*);
void ByteIndexToQ(int, int*, int*);

void PrintVector(unsigned char*, int, int);

void GetPVector(unsigned char*, unsigned char*, int);
void SetPVector(unsigned char*, unsigned char*, int);
void FillPVector(unsigned char*, unsigned char, int);
void AndPVector(unsigned char*, unsigned char, int);
void OrPVector(unsigned char*, unsigned char, int);

void GetQVector(unsigned char*, unsigned char*, int);
void SetQVector(unsigned char*, unsigned char*, int);
void FillQVector(unsigned char*, unsigned char, int);
void AndQVector(unsigned char*, unsigned char, int);
void OrQVector(unsigned char*, unsigned char, int);

int DecodePQ(ReedSolomonTables*, unsigned char*, int, int*, int);

int CountC2Errors(unsigned char*);

/***
 *** logfile.c
 ***/

void DefaultLogFile();
void VPrintLogFile(char*, va_list);
void PrintLogFile(char*, ...);

/***
 *** maintenance.c
 ***
 *
 * Provides a context for running testing functions
 * within the usual program context.
 * Only for debugging / development purposes.
 */

void Maintenance1(char*);

/***
 *** main-window.c
 ***/

#define FIRST_CREATE_WINDOW 3

typedef enum 
{  ACTION_WELCOME,   /* Tab 0; not really an action   */
   ACTION_STOP,      /* ----   does not have a window */
   ACTION_READ,      /* Tab 1 (linear); Tab 2 (adaptive)*/
   ACTION_SCAN,      /* Tab 1 (linear); Tab 2 (adaptive)*/
   ACTION_VERIFY,    /* VERIFY, CREATE and FIX have separate windows assigned */
   ACTION_CREATE,    /* for each method. */
   ACTION_CREATE_CONT,
   ACTION_FIX        
} MajorActions;

void CreateMainWindow(int*, char***);
void ContinueWithAction(int);

/***
 *** medium-info.c
 ***/

void CreateMediumInfoWindow(void);
void PrintMediumInfo(void*);

/***
 *** memtrack.c
 ***/

/*
 * Macro replacements for the glib functions.
 */

#ifdef WITH_MEMDEBUG_YES
#define g_malloc(size) malloc_ext(size,__FILE__,__LINE__)
#define g_malloc0(size) malloc_ext(size,__FILE__,__LINE__)
#define g_realloc(ptr,size) realloc_ext(ptr,size,__FILE__,__LINE__)
#define g_strdup(str) strdup_ext(str,__FILE__,__LINE__)

#define g_try_malloc(size) try_malloc_ext(size,__FILE__,__LINE__)

#define g_strdup_printf(format,args...) \
        strdup_printf_ext(format,__FILE__,__LINE__, ## args)
#define g_strdup_vprintf(format,argp) \
        strdup_vprintf_ext(format,argp,__FILE__,__LINE__)
#define g_locale_to_utf8(str,size,in,out,gerr) \
        g_locale_to_utf8_ext(str,size,in,out,gerr,__FILE__,__LINE__)
#define g_free(size) free_ext(size,__FILE__,__LINE__)

#define REMEMBER(ptr) remember(ptr, 0, __FILE__, __LINE__)
#define FORGET(ptr) forget(ptr)
#else
#define REMEMBER(ptr)
#define FORGET(ptr)
#endif

/* 
 * Protos for the replacement functions.
 */

void*	malloc_ext(int,char*,int);
void*	realloc_ext(void*, int, char*, int);
void*	try_malloc_ext(int,char*,int);
char*	strdup_ext(const char*,char*,int);
char*	strdup_printf_ext(char*, char*, int, ...);
char*	strdup_vprintf_ext(char*, va_list, char*, int);
gchar*  g_locale_to_utf8_ext(const gchar*, gssize, gsize*, gsize*, GError**, char*, int);
void	free_ext(void*,char*,int);

void    remember(void*, int, char*, int);
int     forget(void*);

void    check_memleaks(void);

/***
 *** menubar.c
 ***/

void AttachTooltip(GtkWidget*, char*, char*);
GtkWidget* CreateMenuBar(GtkWidget*);
GtkWidget* CreateToolBar(GtkWidget*);

/***
 *** method.c / method-link.c
 ***
 * method-link.c is automatically created by the configure script.
 */

#define DATA_MD5_BAD (1<<0)          /* for result of finalizeCksums */ 
#define CRC_MD5_BAD (1<<1)
#define ECC_MD5_BAD (1<<2)

typedef struct _Method
{  char name[4];                     /* Method name tag */
   guint32 properties;               /* see definition above */
   char *description;                /* Fulltext description */
   char *menuEntry;                  /* Text for use in preferences menu */
   void (*create)(void);             /* Creates an error correction file */
   void (*fix)(Image*);              /* Fixes a damaged image */
   void (*verify)(Image*);           /* Verifies image with ecc data */
   int  (*recognizeEccFile)(LargeFile*, EccHeader**); /* checks whether we can handle this ecc file */
   int  (*recognizeEccImage)(Image*); /* checks whether we can handle this augmented image */
   guint64 (*expectedImageSize)(Image*);/* calculates expected image size (incl. augmented data) */
   CrcBuf* (*getCrcBuf)(Image*);      /* read and cache CRC info from ecc data */
   void (*resetCksums)(Image*);       /* Methods for building internal */
   void (*updateCksums)(Image*, gint64, unsigned char*);/* checksum while reading an image */
   int  (*finalizeCksums)(Image*);
   void *ckSumClosure;                                   /* working closure for above */
   void (*createVerifyWindow)(struct _Method*, GtkWidget*);
   void (*createCreateWindow)(struct _Method*, GtkWidget*);
   void (*createFixWindow)(struct _Method*, GtkWidget*);
   void (*createPrefsPage)(struct _Method*, GtkWidget*);
   void (*resetVerifyWindow)(struct _Method*);
   void (*resetCreateWindow)(struct _Method*);
   void (*resetFixWindow)(struct _Method*);
   void (*resetPrefsPage)(struct _Method*);
   void (*readPreferences)(struct _Method*);
   void (*destroy)(struct _Method*);
   int  tabWindowIndex;              /* our position in the (invisible) notebook */
   void *widgetList;                 /* linkage to window system */
} Method;

void BindMethods(void);        /* created by configure in method-link.c */

void CollectMethods(void);
void RegisterMethod(Method*);
void ListMethods(void);
Method* FindMethod(char*);
void CallMethodDestructors(void);

/***
 *** misc.c 
 ***/

char* sgettext(char*);
char* sgettext_utf8(char*);

gint64 uchar_to_gint64(unsigned char*);
void gint64_to_uchar(unsigned char*, gint64);

void CalcSectors(guint64, guint64*, int*);

void PrintCLI(char*, ...);
void PrintLog(char*, ...);
void PrintLogWithAsterisks(char*, ...);
void Verbose(char*, ...);
void PrintTimeToLog(GTimer*, char*, ...);
void PrintProgress(char*, ...);
void ClearProgress(void);
void PrintCLIorLabel(GtkLabel*, char*, ...);
int GetLongestTranslation(char*, ...);

void LogWarning(char*, ...);
void Stop(char*, ...);
void RegisterCleanup(char*, void (*)(gpointer), gpointer);
void UnregisterCleanup(void);

GThread* CreateGThread(GThreadFunc, gpointer);

void ShowWidget(GtkWidget*);
void AllowActions(gboolean);

void ShowMessage(GtkWindow*, char*, GtkMessageType);
GtkWidget* CreateMessage(char*, GtkMessageType, ...);
void SetLabelText(GtkLabel*, char*, ...);
void SetProgress(GtkWidget*, int, int);

int ModalDialog(GtkMessageType, GtkButtonsType, void (*)(GtkDialog*), char*, ...);
int ModalWarning(GtkMessageType, GtkButtonsType, void (*)(GtkDialog*), char*, ...);

void SetText(PangoLayout*, char*, int*, int*);
void SwitchAndSetFootline(GtkWidget*, int, GtkWidget*, char*, ...);

void ReverseCancelOK(GtkDialog*);
void TimedInsensitive(GtkWidget*, int);

int GetLabelWidth(GtkLabel*, char*, ...);
void LockLabelSize(GtkLabel*, char*, ...);

int ConfirmImageDeletion(char *);
int ConfirmEccDeletion(char *);

/***
 *** preferences.c
 ***/

void CreatePreferencesWindow(void);
void UpdateMethodPreferences(void);
void HidePreferences(void);
void FreePreferences(void*);

void UpdatePrefsExhaustiveSearch(void);
void UpdatePrefsConfirmDeletion(void);
void RegisterPreferencesHelpWindow(LabelWithOnlineHelp*);

/***
 *** print-sense.c
 ***/

void RememberSense(int, int, int);
char *GetSenseString(int, int, int, int);
char* GetLastSenseString(int);
void GetLastSense(int*, int*, int*);

/***
 *** random.c
 ***/

#define	MY_RAND_MAX	2147483647

gint32  Random(void);
void    SRandom(gint32);
guint32 Random32(void);

/***
 *** raw-editor.c
 ***/

void CreateRawEditor(void);
void FreeRawEditorContext(void*);


/***
 *** raw-sector-cache.c
 ***/

typedef struct _DefectiveSectorHeader
{  unsigned char mediumFP[16];       /* Medium fingerprint */
   gint64 lba;                       /* LBA of this sector */
   gint32 sectorSize;                /* Sector size in bytes */
   gint32 properties;                /* Flags for future use */
   gint32 dshFormat;                 /* Format of this file */
   gint32 nSectors;                  /* Number of sectors in this file */
} DefectiveSectorHeader;

enum                                 /* for ->properties above */
{  DSH_HAS_FINGERPRINT = (1<<0),
   DSH_XA_MODE         = (1<<1)
};

int SaveDefectiveSector(struct _RawBuffer*, int);
int TryDefectiveSectorCache(struct _RawBuffer*, unsigned char*);
void ReadDefectiveSectorFile(DefectiveSectorHeader *, struct _RawBuffer*, char*);

/*** 
 *** read-linear.c
 ***/

void ReadMediumLinear(gpointer);

/***
 *** read-linear-window.c
 ***/

void ResetLinearReadWindow();
void CreateLinearReadWindow(GtkWidget*);

void InitializeCurve(void*, int, int);
void AddCurveValues(void*, int, int, int);
void MarkExistingSectors(void);
void RedrawReadLinearWindow(void);

/*** 
 *** read-adaptive.c
 ***/

void GetReadingRange(gint64, gint64*, gint64*);

void ReadMediumAdaptive(gpointer);

/***
 *** read-adaptive-window.c
 ***/

#define ADAPTIVE_READ_SPIRAL_SIZE 4800

void ResetAdaptiveReadWindow();
void SetAdaptiveReadMinimumPercentage(int);
void CreateAdaptiveReadWindow(GtkWidget*);

void ClipReadAdaptiveSpiral(int);
void SetAdaptiveReadSubtitle(char*);
void SetAdaptiveReadFootline(char*, GdkColor*);
void UpdateAdaptiveResults(gint64, gint64, gint64, int);
void ChangeSegmentColor(GdkColor*, int);
void RemoveFillMarkers();

/***
 *** recover-raw.c
 ***/

#define MAX_RAW_TRANSFER_SIZE (2352+296)  /* main channel plus C2 vector and mask */
#define CD_RAW_DUMP_SIZE MAX_RAW_TRANSFER_SIZE
#define CD_RAW_SECTOR_SIZE 2352  
#define CD_RAW_C2_SECTOR_SIZE (2352+294)  /* main channel plus C2 vector */

typedef struct _RawBuffer
{  GaloisTables *gt;          /* for L-EC Reed-Solomon */
   ReedSolomonTables *rt;
   struct _AlignedBuffer *workBuf; /* working buffer for READ CD */
   unsigned char *zeroSector; /* a raw sector containing just zeros. */
   unsigned char **rawBuf;    /* buffer for raw read attempts */
   int samplesRead;           /* number of samples read */
   int samplesMax;            /* maximum number of samples we can store */
   int sampleSize;            /* size of samples */
   int dataOffset;            /* offset to user data in frame */
   int xaMode;                /* frame is in XA21 mode */
   int recommendedAttempts;   /* number of retries recommended by reading heuristics */

   unsigned char *recovered;  /* working buffer for cd frame recovery */
   unsigned char *byteState;  /* state of error correction */
   unsigned char *reference;  /* NULL or the correct sector (for debugging purposes) */
   unsigned char *byteCount;  /* stores how many different bytes were read for each position in a sector */   
   gint64 lba;                /* sector number were currently working on */

   guint8 mediumFP[16];       /* medium fingerprint for raw sector cache validation */
   int validFP;               /* indicates valid fingerprint */

   unsigned char *pParity1[N_P_VECTORS];
   unsigned char *pParity2[N_P_VECTORS];
   int pParityN[N_P_VECTORS][2];

   unsigned char *qParity1[N_Q_VECTORS];
   unsigned char *qParity2[N_Q_VECTORS];
   int qParityN[N_Q_VECTORS][2];

   int *pLoad,*qLoad;

   /* data structures for the smart_lec */

   unsigned char **pList[N_P_VECTORS];  /* List of accepting P vectors */
   int pn[N_P_VECTORS];

   unsigned char **qList[N_Q_VECTORS];  /* List of accepting Q vectors */
   int qn[N_Q_VECTORS];

   int bestFrame;                      /* Frame with lowest failures */
   int bestP1, bestP2, bestQ1, bestQ2;

} RawBuffer;

enum                          /* values for byteState */
{  FRAME_BYTE_UNKNOWN,        /* state of byte is unknown */
   FRAME_BYTE_ERROR,          /* byte is wrong (= erasure for ecc) */
   FRAME_BYTE_GOOD            /* byte is correct */
};

RawBuffer* CreateRawBuffer(int);
void ReallocRawBuffer(RawBuffer*, int);
void ResetRawBuffer(RawBuffer*);
void FreeRawBuffer(RawBuffer*);

void DumpSector(RawBuffer*, char*);

#define STRICT_MSF_CHECK FALSE
#define SLOPPY_MSF_CHECK TRUE

int MSFtoLBA(unsigned char, unsigned char, unsigned char);
int CheckEDC(unsigned char*, int);
int CheckMSF(unsigned char*, int, int);
void InitializeCDFrame(unsigned char*, int, int, int);

void UpdateFrameStats(RawBuffer*);
int ValidateRawSector(RawBuffer*, unsigned char*, char*);
int IterativeLEC(RawBuffer*);
int TryCDFrameRecovery(RawBuffer*, unsigned char*);

/*** 
 *** scsi-layer.c
 ***
 * Note that there is also a scsi-layer.h with more
 * scsi wrapper dependent declarations.
 */

/* Maximum number of sectors per request */

#define MAX_CLUSTER_SIZE (32*2048)
#define MAX_CLUSTER_SECTORS 32

typedef struct _AlignedBuffer
{  unsigned char *base;
   unsigned char *buf;
} AlignedBuffer;

AlignedBuffer *CreateAlignedBuffer(int);
void FreeAlignedBuffer(AlignedBuffer*);

char* DefaultDevice(void);
gint64 CurrentImageSize(void);
gint64 CurrentImageCapacity(void);

int SendReadCDB(char*, unsigned char*, unsigned char*, int, int);

/*** 
 *** scsi-simulated.c
 ***/

void InitSimulatedCD(void);

/***
 *** rs-decoder.c
 ***/

int TestErrorSyndromes(ReedSolomonTables*, unsigned char*);

/***
 *** rs-encoder.c and friends
 ***/

typedef enum
{  ENCODING_ALG_DEFAULT,
   ENCODING_ALG_INVALID,
   ENCODING_ALG_32BIT,
   ENCODING_ALG_64BIT,
   ENCODING_ALG_SSE2,   
   ENCODING_ALG_ALTIVEC
} CODEC_TYPE;

void EncodeNextLayer(ReedSolomonTables*, unsigned char*, unsigned char*, guint64, int);
void DescribeRSEncoder(char**, char**);
int ProbeSSE2(void);
int ProbeAltiVec(void);

/***
 *** show-manual.c
 ***/

void ShowPDF(char*);

/***
 *** smart-lec.c
 ***/

#define SMART_LEC_MESSAGE_SIZE 256

void CollectGoodVectors(RawBuffer*);
void PrintPQStats(RawBuffer*);
int SmartLEC(RawBuffer*);

void *PrepareIterativeSmartLEC(RawBuffer*);
void SmartLECIteration(void*, char*);
void EndIterativeSmartLEC(void*);

/***
 *** spiral.c
 ***/

typedef struct _Spiral
{  GdkDrawable *drawable;
   int mx, my;
   int startRadius;
   int segmentSize;
   int segmentCount;
   double *segmentPos;
   GdkColor **segmentColor;
   GdkColor *outline;
   int diameter;
   int segmentClipping;
   int cursorPos;
   GdkColor *colorUnderCursor;
} Spiral;

Spiral* CreateSpiral(GdkColor*, GdkColor*, int, int, int);
void SetSpiralWidget(Spiral*, GtkWidget*);
void FillSpiral(Spiral*, GdkColor*);
void FreeSpiral(Spiral*);
void DrawSpiral(Spiral*);
void DrawSpiralSegment(Spiral*, GdkColor*, int);
void DrawSpiralLabel(Spiral*, PangoLayout*, char*, GdkColor*, int, int);

void ChangeSpiralCursor(Spiral*, int);
void MoveSpiralCursor(Spiral*, int);

/***
 *** welcome-window.c
 ***/

void CreateWelcomePage(GtkNotebook*);

#endif				/* DVDISASTER_H */
