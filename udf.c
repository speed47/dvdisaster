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

#include "udf.h"

/***
 *** Rudimentary UDF and ISO filesystem parsing.
 ***
 * Information about UDF and ISO was gathered from ECMA-119,
 * ECMA-167, ECMA-167 and the UDF 2.6 standard from OSTA.org.
 * However, no claims are made to be actually conformant to any
 * of those standards.
 */

/*
 * ECMA sector indices start with 1;
 * this definition make it easier to use the standard for reading strings
 * from the buffer.
 * begin and end are the bp postions of the string start and end position.
 */

/*
 * 8bit values
 */

#define bp_get_byte(buf, idx) (buf[(idx)-1])
#define bp_set_byte(buf, idx, byte) (buf[(idx)-1] = (unsigned char)byte)

/* 
 * 16bit both byte order
 */

#ifdef HAVE_BIG_ENDIAN
  #define bp_get_short_bbo(buf, idx) (*((guint16*)(buf+idx+1)))
#else
  #define bp_get_short_bbo(buf, idx) (*((guint16*)(buf+idx-1)))
#endif

static void bp_set_short_bbo(unsigned char *buf, int idx, guint16 value)
{  unsigned char wx = (value >> 8) & 0xff;
   unsigned char yz = value & 0xff;

   buf = buf + idx - 1;
   *buf++ = yz;
   *buf++ = wx;
   *buf++ = wx;
   *buf++ = yz;
}

static void bp_set_short_lsb(unsigned char *buf, int idx, guint16 value)
{  unsigned char wx = (value >> 8) & 0xff;
   unsigned char yz = value & 0xff;

   buf = buf + idx - 1;
   *buf++ = yz;
   *buf++ = wx;
}

static void bp_set_short_msb(unsigned char *buf, int idx, guint16 value)
{  unsigned char wx = (value >> 8) & 0xff;
   unsigned char yz = value & 0xff;

   buf = buf + idx - 1;
   *buf++ = wx;
   *buf++ = yz;
}

/*
 * 32bit single byte order
 */

#ifdef HAVE_BIG_ENDIAN
  #define bp_get_long_lsb(buf, idx) (((guint32)buf[(idx)+2])<<24|((guint32)buf[(idx)+1])<<16|((guint32)buf[(idx)])<<8|((guint32)buf[(idx)-1]))
#else
  #define bp_get_long_lsb(buf, idx) (*((guint32*)(buf+idx-1)))
#endif
#define bp_get_long_msb(buf, idx) (((guint32)buf[(idx)-1])<<24|((guint32)buf[idx])<<16|((guint32)buf[(idx)+1])<<8|((guint32)buf[(idx)+2]))

static void bp_set_long_lsb(unsigned char *buf, int idx, guint32 value)
{  unsigned char st = (value >> 24) & 0xff;
   unsigned char uv = (value >> 16) & 0xff;
   unsigned char wx = (value >> 8) & 0xff;
   unsigned char yz = value & 0xff;

   buf = buf + idx - 1;
   *buf++ = yz;
   *buf++ = wx;
   *buf++ = uv;
   *buf++ = st;
}

static void bp_set_long_msb(unsigned char *buf, int idx, guint32 value)
{  unsigned char st = (value >> 24) & 0xff;
   unsigned char uv = (value >> 16) & 0xff;
   unsigned char wx = (value >> 8) & 0xff;
   unsigned char yz = value & 0xff;

   buf = buf + idx - 1;
   *buf++ = st;
   *buf++ = uv;
   *buf++ = wx;
   *buf++ = yz;
}

/*
 * 32bit both byte order 
 */

#ifdef HAVE_BIG_ENDIAN
  #define bp_get_long_bbo(buf, idx) (*((guint32*)(buf+idx+3)))
#else
  #define bp_get_long_bbo(buf, idx) (*((guint32*)(buf+idx-1)))
#endif

static void bp_set_long_bbo(unsigned char *buf, int idx, guint32 value)
{  unsigned char st = (value >> 24) & 0xff;
   unsigned char uv = (value >> 16) & 0xff;
   unsigned char wx = (value >> 8) & 0xff;
   unsigned char yz = value & 0xff;

   buf = buf + idx - 1;
   *buf++ = yz;
   *buf++ = wx;
   *buf++ = uv;
   *buf++ = st;
   *buf++ = st;
   *buf++ = uv;
   *buf++ = wx;
   *buf++ = yz;
}

static void bp_get(unsigned char *dest, unsigned char *src, int begin, int end)
{  int length = end-begin+1;  

  strncpy((char*)dest, (char*)src+begin-1, length);
}

#if 0
static void bp_set(unsigned char *dest, unsigned char *src, int begin, int end)
{  int length = end-begin+1;  

   memcpy((char*)dest+begin-1, (char*)src, length);
}
#endif

/*
 * String operations
 */

static void bp_get_string(unsigned char *dest, unsigned char *src, int begin, int end)
{  int length = end-begin+1;  

   strncpy((char*)dest, (char*)src+begin-1, length);
   dest[length] = 0;
}

static void bp_set_string(unsigned char *dest, char *src, int begin, int end)
{  int length = end-begin+1;  

   strncpy((char*)(dest+begin-1), src, length);
}

/*
 * D1 string conversion currently simply expands the string to 16bit characters.
 */

static void bp_set_d1_string(unsigned char *dest, char *src, int begin, int end)
{ 
   dest = dest + begin - 1;

   while(begin<=end-2 && *src)
   {  *dest++ = 0;
      *dest++ = (unsigned char)*src++;
      begin += 2;
   }
}

/*
 * Date values
 */

static void get_date(unsigned char *date, unsigned char *buf, int idx)
{  
   idx--;
   sprintf((char*)date, "dd-mm-yyyy hh:mm:ss.ss");
   bp_get(date+ 6, buf, idx+ 1, idx+ 4);
   bp_get(date+ 3, buf, idx+ 5, idx+ 6);
   bp_get(date+ 0, buf, idx+ 7, idx+ 8);
   bp_get(date+11, buf, idx+ 9, idx+10);
   bp_get(date+14, buf, idx+11, idx+12);
   bp_get(date+17, buf, idx+13, idx+14);
   bp_get(date+20, buf, idx+15, idx+16);
}

static void set_date(unsigned char *date, int idx,
		     int year, int month, int day, 
		     int hour, int minute, int second, int hsecond, 
		     int gmt_offset)
{  
  sprintf((char*)(date+idx-1), "%04d%02d%02d%02d%02d%02d%02d",
	  year, month, day, hour, minute, second, hsecond);

  *(date+idx+15) = (unsigned char)gmt_offset;
}

static void set_binary_date(unsigned char *dest, int idx, 
			    int year, int month, int day,
			    int hour, int minute, int second,
			    int gmt_offset)
{
  dest = dest + idx - 1;

  *dest++ = (unsigned char)year;
  *dest++ = (unsigned char)month;
  *dest++ = (unsigned char)day;
  *dest++ = (unsigned char)hour;
  *dest++ = (unsigned char)minute;
  *dest++ = (unsigned char)second;
  *dest++ = (unsigned char)gmt_offset;
}

static void beautify_dchar(char *dchar)
{  int idx = strlen(dchar)-1;

  while(idx>=0)
  {  if(dchar[idx] != ' ') break;
     dchar[idx--] = 0;
  }

  while(idx>0)
  {  if(dchar[idx-1] != ' ')
	dchar[idx] = tolower((int)dchar[idx]);
     idx--;
  }

  if(!*dchar) strcpy(dchar, _("Unnamed"));
}

/***
 *** Extract some useful information from the ISO file system
 ***/

void FreeIsoInfo(IsoInfo *ii)
{  
   g_free(ii);
}

static IsoInfo* examine_primary_vd(unsigned char *buf)
{  IsoInfo *ii = g_malloc(sizeof(IsoInfo));
   unsigned char vlabel[33];
   unsigned char str[80];
   guint32 x,vss;
   unsigned char date[32];
   
   bp_get_string(str, buf, 9, 40);
   Verbose("    System identifier         : |%s|\n", str);

   bp_get_string(vlabel, buf, 41, 72);
   Verbose("    Volume identifier         : |%s|\n", vlabel);

   vss = bp_get_long_bbo(buf, 81);
   Verbose("    Volume space size         : %d sectors\n", vss);

   x = bp_get_short_bbo(buf, 121);
   Verbose("    Volume set size           : %d\n", x);

   x = bp_get_short_bbo(buf, 125);
   Verbose("    Volume sequence size      : %d\n", x);

   x = bp_get_short_bbo(buf, 129);
   Verbose("    Logical block size        : %d\n", x);

   x = bp_get_long_bbo(buf, 133);
   Verbose("    Path table size           : %d bytes\n", x);

   x = bp_get_long_lsb(buf, 141);
   Verbose("    L-Path table location     : %d\n", x);

   x = bp_get_long_lsb(buf, 145);
   Verbose("    Opt L-Path table location : %d\n", x);

   x = bp_get_long_msb(buf, 149);
   Verbose("    M-Path table location     : %d\n", x);

   x = bp_get_long_msb(buf, 153);
   Verbose("    Opt M-Path table location : %d\n", x);

   /* 157 .. 190 directory record */

   /* 191 .. 318 Volume set identifier */

   /* 319 .. 446 Publisher Identifier */

   /* 447 .. 574 Data Preparer Identifier */

   /* 575 .. 702 Application Identfier */

   /* 703 .. 739 Copyright File Identifier */

   /* 740 .. 776 Abstract File Identifier */
   
   /* 777 .. 813 Bibliographic File Identifier */

   get_date(date, buf, 814);
   Verbose("    Volume creation date/time : %s\n", date);
   
   get_date(str, buf, 831);
   Verbose("    Volume modification d/t   : %s\n", str);
   
   get_date(str, buf, 848);
   Verbose("    Volume expiration d/t     : %s\n", str);

   get_date(str, buf, 865);
   Verbose("    Volume effective d/t      : %s\n", str);

   x = bp_get_byte(buf,882);
   Verbose("    File structure version    : %d\n", x);

   /* Extract information for IsoInfo */

   ii->volumeSize = vss;

   if(!Closure->screenShotMode)
   {  strcpy(ii->volumeLabel, (char*)vlabel);
      beautify_dchar(ii->volumeLabel);
   }
   else strcpy(ii->volumeLabel, _("Example disc"));
   strcpy(ii->creationDate, (char*)date);
   ii->creationDate[10] = 0;
   return ii;
}

static IsoInfo* examine_iso(Image *image)
{  AlignedBuffer *ab = CreateAlignedBuffer(2048);
   unsigned char *buf = ab->buf;
   IsoInfo *ii = NULL;
   int sector;
   int vdt,vdt_ver;
   unsigned char sid[6];
      
   Verbose(" Examining the ISO file system...\n");

   /*** Iterate over the volume decriptors */
 
   for(sector=16; sector<32; sector++)
   {  if(Closure->stopActions) 
        continue;

      if(ImageReadSectors(image, buf, sector, 1) != 1)
      {  Verbose("  Sector %2d: unreadable\n", sector);
	 continue;
      }

      vdt = bp_get_byte(buf, 1);      /* Volume descriptor type */
      bp_get_string(sid, buf, 2, 6);  /* Standard identifier */
      vdt_ver = bp_get_byte(buf,7);   /* Volume descriptor version */

      Verbose("  Sector %2d:\n"
		 "   Volume descriptor type    = %d\n"
		 "   Volume descriptor version = %d\n"
		 "   Standard identifier       = %s\n",
		 sector, vdt, vdt_ver, sid);

      if(strncmp((char*)sid,"CD001",5))
      {  Verbose("  * Wrong or missing standard identifier.\n");
	 continue;
      }

      switch(vdt)
      {  case 0: Verbose("   -> boot record: *skipped*\n");
	         break;
	 case 1: Verbose("   -> primary volume descriptor:\n");
	         ii = examine_primary_vd(buf);
		 break;
	 case 2: Verbose("   -> supplementary volume descriptor: *skipped*\n");
	         break;		   
	 case 255: Verbose("   -> volume descriptor set terminator;\n"
			      "      end of ISO file system parsing.\n");
	           goto finished;
	           break;		   
	 default : Verbose("   -> unknown volume descriptor: *skipped*\n");
	           break;
      }
   }

finished:
   FreeAlignedBuffer(ab);
   return ii;
}

/***
 *** The main wrapper for visiting the ISO and UDF file system structures
 ***/


void ExamineUDF(Image *image)
{  
   if(!image) return;

   if(image->type == IMAGE_MEDIUM) Verbose("\nExamineUDF(Device: %s)\n", image->dh->devinfo);
   if(image->type == IMAGE_FILE  ) Verbose("\nExamineUDF(File: %s)\n", image->file->path);

   image->isoInfo = examine_iso(image);

   Verbose(" Examining the UDF file system...\n");
   Verbose("  not yet implemented.\n\n");
}

/***
 *** Rudimentary support for creating .iso images.
 *** Currently only used for creating the random image;
 * can only create .iso images containing exactly one file.
 */

/*
 * Directory record writing
 */

static IsoDir* create_iso_dir()
{  IsoDir *id = g_malloc0(sizeof(IsoDir));

  id->dir      = g_malloc0(2048);
  id->nSectors = 1;

  return id;
}

static void free_iso_dir(IsoDir *id)
{  g_free(id->dir);
   g_free(id);
}

static void add_directory_record(IsoDir *iso_dir,
				 int extent, int file_flags, int file_identifier, 
				 int year, int month, int day, 
				 int hour, int minute, int second, int gmt_offset)
{  unsigned char *dir = iso_dir->dir + iso_dir->tail;

   iso_dir->tail += 34;

   bp_set_byte(dir, 1, 34);           /* Length of directory record       */
   bp_set_byte(dir, 2,  0);           /* Extended attribute length        */
   bp_set_long_bbo(dir, 3, extent);   /* Location of Extent               */
   bp_set_long_bbo(dir, 11, 2048);    /* Data length of file section      */
   set_binary_date(dir, 19,           /* Binary date rep of creation time */
		   year,month,day,hour,minute,second,gmt_offset);
   bp_set_byte(dir, 26, file_flags);  /* File flags (we are a directory)  */
   bp_set_byte(dir, 27, 0);           /* File unit size if interleaved    */
   bp_set_byte(dir, 28, 0);           /* Interleave gap size              */
   bp_set_short_bbo(dir, 29, 1);      /* Volume sequence number           */
   bp_set_byte(dir, 33, 1);           /* Length of file identifier        */
   bp_set_byte(dir, 34, file_identifier);  /* File identifier             */
} 

static void add_file_record(IsoDir *iso_dir, char *filename, guint64 size, int extent, 
			    int year, int month, int day, 
			    int hour, int minute, int second, int gmt_offset)
{  unsigned char *dir = iso_dir->dir + iso_dir->tail;
   int sl = strlen(filename);
   int entry_len = 34+sl;

   iso_dir->tail += entry_len;

   bp_set_byte(dir, 1, entry_len);    /* Length of directory record       */
   bp_set_byte(dir, 2,  0);           /* Extended attribute length        */
   bp_set_long_bbo(dir, 3, extent);   /* Location of Extent               */
   bp_set_long_bbo(dir, 11, size);    /* Data length of file section      */
   set_binary_date(dir, 19,           /* Binary date rep of creation time */
		   year,month,day,hour,minute,second,gmt_offset);
   bp_set_byte(dir, 26, 0);           /* File flags (we are a file)       */
   bp_set_byte(dir, 27, 0);           /* File unit size if interleaved    */
   bp_set_byte(dir, 28, 0);           /* Interleave gap size              */
   bp_set_short_bbo(dir, 29, 1);      /* Volume sequence number           */
   bp_set_byte(dir, 33, sl);           /* Length of file identifier       */
   bp_set_string(dir, filename, 34, 34+sl);  /* File name                 */
} 

static void add_d1_file_record(IsoDir *iso_dir, char *filename, guint64 size, int extent, 
			       int year, int month, int day, 
			       int hour, int minute, int second, int gmt_offset)
{  unsigned char *dir = iso_dir->dir + iso_dir->tail;
   int sl = 2*strlen(filename);
   int entry_len = 34+sl;

   iso_dir->tail += entry_len;

   bp_set_byte(dir, 1, entry_len);    /* Length of directory record       */
   bp_set_byte(dir, 2,  0);           /* Extended attribute length        */
   bp_set_long_bbo(dir, 3, extent);   /* Location of Extent               */
   bp_set_long_bbo(dir, 11, size);    /* Data length of file section      */
   set_binary_date(dir, 19,           /* Binary date rep of creation time */
		   year,month,day,hour,minute,second,gmt_offset);
   bp_set_byte(dir, 26, 0);           /* File flags (we are a file)       */
   bp_set_byte(dir, 27, 0);           /* File unit size if interleaved    */
   bp_set_byte(dir, 28, 0);           /* Interleave gap size              */
   bp_set_short_bbo(dir, 29, 1);      /* Volume sequence number           */
   bp_set_byte(dir, 33, sl);           /* Length of file identifier       */
   bp_set_d1_string(dir, filename, 34, 34+sl);  /* File name              */
} 


/*
 * Path tables
 */

static IsoPathTable* create_iso_path_table()
{  IsoPathTable *ipt = g_malloc0(sizeof(IsoPathTable));

   ipt->lpath      = g_malloc0(2048);
   ipt->mpath      = g_malloc0(2048);
   ipt->nSectors   = 1;

   return ipt;
}

static void free_iso_path_table(IsoPathTable *ipt)
{  g_free(ipt->lpath);
   g_free(ipt->mpath);
   g_free(ipt);
}

static void add_path(IsoPathTable *ipt, int extent)
{  unsigned char *lpath = ipt->lpath + ipt->tail;
   unsigned char *mpath = ipt->mpath + ipt->tail;

   bp_set_byte(lpath, 1, 1);   /* Length of directory identifier (root=1) */
   bp_set_byte(mpath, 1, 1);

   bp_set_byte(lpath, 2, 0);   /* Extended attribute length */
   bp_set_byte(mpath, 2, 0);   /* Extended attribute length */

   bp_set_long_lsb(lpath, 3, extent); /* Location of extent */
   bp_set_long_msb(mpath, 3, extent);

   bp_set_short_lsb(lpath, 7, 1);     /* Parent directory number */
   bp_set_short_msb(mpath, 7, 1);

   bp_set_byte(lpath, 9, 0);   /* Directory identifier (root=0) */
   bp_set_byte(mpath, 9, 0);
}

/*
 * Initialize the IsoHeader structure
 */

IsoHeader* InitIsoHeader(void)
{  IsoHeader *ih = g_malloc0(sizeof(IsoHeader));
   unsigned char *pvd,*svd;
   char blank[128], string[128];

   memset(blank, ' ', 128);

   /*** Initialize primary volume descriptor (sector 16). */

   ih->pvd = pvd = g_malloc0(2048);
   ih->dirSectors = 1; 

   /* 1..8: Standard identifier */

   bp_set_byte(pvd, 1, 1);             /* Volume descriptor type 1    */
   bp_set_string(pvd, "CD001", 2, 6);  /* Standard identifier         */
   bp_set_byte(pvd, 7, 1);             /* Volume descriptor version 1 */

   /* 9..40: System identifier */

   bp_set_string(pvd, blank, 9, 40);

   /* 41..72: Volume identifier */

   memcpy(string, blank, 128);
   memcpy(string, "RANDOM IMAGE", 12);
   bp_set_string(pvd, string, 41, 72);

   /*  73.. 80: Unused, zero bytes */
   /*  81.. 88: Volume space size, filled in at WriteIsoHeader() */
   /*  89..120: Unused, zero bytes */

   /* 121..124: Volume set size */

   bp_set_short_bbo(pvd, 121, 1);

   /* 125..128: Volume sequence number */

   bp_set_short_bbo(pvd, 125, 1);

   /* 129..132: Logical block size */

   bp_set_short_bbo(pvd, 129, 2048);

   /* 133..140: Path table size, filled in at WriteIsoHeader() */
   /* 141..144: L Path table location, filled in at WriteIsoHeader() */
   /* 145..148: Optional L Path table location, filled in at WriteIsoHeader() */
   /* 149..152: M Path table location, filled in at WriteIsoHeader() */
   /* 153..156: Optional M Path table location, filled in at WriteIsoHeader() */

   /* 157..190: Directory record for root dir, filled in at WriteIsoHeader() */

   /* 191..318: Volume set identifier */

   bp_set_string(pvd, blank, 191, 318);

   /* 319..446: Publisher identifier */

   bp_set_string(pvd, blank, 319, 446);

   /* 447..574: Data preparer identifier */

   bp_set_string(pvd, blank, 447, 574);

   /* 575..702 Application Identfier */

   bp_set_string(pvd, blank, 575, 702);

   /* 703..739 Copyright File Identifier */

   bp_set_string(pvd, blank, 703, 739);

   /* 740..776 Abstract File Identifier */

   bp_set_string(pvd, blank, 740, 776);
   
   /* 777..813 Bibliographic File Identifier */

   bp_set_string(pvd, blank, 777, 813);

   /* 814..830 Volume creation time, filled in by WriteIsoHeader() */
   /* 831..847 Volume modification time, filled in by WriteIsoHeader() */
   /* 848..864 Volume expiration time, filled in by WriteIsoHeader() */
   /* 865..881 Volume effective time, filled in by WriteIsoHeader() */

   /* 882      File structure version */

   bp_set_byte(pvd, 882, 1);

   /* 883        Reserved, zero */
   /* 884..1395  for application use */
   /* 1396..2048 Reserved, zero */

   /*** Initialize supplementary volume descriptor (sector 17). */

   ih->svd = svd = g_malloc0(2048);
   ih->dirSectors = 2; 

   /* 1..8: Standard identifier */

   bp_set_byte(svd, 1, 2);             /* Volume descriptor type 1    */
   bp_set_string(svd, "CD001", 2, 6);  /* Standard identifier         */
   bp_set_byte(svd, 7, 1);             /* Volume descriptor version 1 */
   bp_set_byte(svd, 8, 0);             /* Volume flags                */

   /* 9..40: System identifier (zero filled) */

   /* 41..72: Volume identifier */

   bp_set_d1_string(svd, "random image", 41, 72);

   /*  73.. 80: Unused, zero bytes */
   /*  81.. 88: Volume space size, filled in at WriteIsoHeader() */
   /*  89..120: Escape sequences */

   bp_set_byte(svd, 89, 37);
   bp_set_byte(svd, 90, 47);
   bp_set_byte(svd, 91, 64);

   /* 121..124: Volume set size */

   bp_set_short_bbo(svd, 121, 1);

   /* 125..128: Volume sequence number */

   bp_set_short_bbo(svd, 125, 1);

   /* 129..132: Logical block size */

   bp_set_short_bbo(svd, 129, 2048);

   /* 133..140: Path table size, filled in at WriteIsoHeader() */
   /* 141..144: L Path table location, filled in at WriteIsoHeader() */
   /* 145..148: Optional L Path table location, filled in at WriteIsoHeader() */
   /* 149..152: M Path table location, filled in at WriteIsoHeader() */
   /* 153..156: Optional M Path table location, filled in at WriteIsoHeader() */

   /* 157..190: Directory record for root dir, filled in at WriteIsoHeader() */

   /* The standard says to fill the identifier field with blanks,
      but actual writing software seems to use zeroes. */

   /* 191..318: Volume set identifier */

   //bp_set_string(svd, blank, 191, 318);

   /* 319..446: Publisher identifier */

   //bp_set_string(svd, blank, 319, 446);

   /* 447..574: Data preparer identifier */

   //bp_set_string(svd, blank, 447, 574);

   /* 575..702 Application Identfier */

   //bp_set_string(svd, blank, 575, 702);

   /* 703..739 Copyright File Identifier */

   //bp_set_string(svd, blank, 703, 739);

   /* 740..776 Abstract File Identifier */

   //bp_set_string(svd, blank, 740, 776);
   
   /* 777..813 Bibliographic File Identifier */

   //bp_set_string(svd, blank, 777, 813);

   /* 814..830 Volume creation time, filled in by WriteIsoHeader() */
   /* 831..847 Volume modification time, filled in by WriteIsoHeader() */
   /* 848..864 Volume expiration time, filled in by WriteIsoHeader() */
   /* 865..881 Volume effective time, filled in by WriteIsoHeader() */

   /* 882      File structure version */

   bp_set_byte(svd, 882, 1);

   /* 883        Reserved, zero */
   /* 884..1395  for application use */
   /* 1396..2048 Reserved, zero */

   /*** The volume descriptor set terminator (sector 17)
	is trivial and created during WriteIsoHeader() */

   ih->dirSectors = 3; 

   /*** Allocate space for directories and path tables */

   ih->proot = create_iso_dir();
   add_directory_record(ih->proot, 21, 2, 0,
			106, 7, 16, 12, 35, 46, 8);
   add_directory_record(ih->proot, 21, 2, 1,
			106, 7, 16, 12, 35, 46, 8);

   ih->ppath = create_iso_path_table();
   add_path(ih->ppath, 21);

   ih->sroot = create_iso_dir();
   add_directory_record(ih->sroot, 24, 2, 0,
			106, 7, 16, 12, 35, 46, 8);
   add_directory_record(ih->sroot, 24, 2, 1,
			106, 7, 16, 12, 35, 46, 8);

   ih->spath = create_iso_path_table();
   add_path(ih->spath, 24);

   ih->dirSectors += 6;

   return ih;
}

void FreeIsoHeader(IsoHeader *ih)
{ 
   free_iso_dir(ih->proot);
   free_iso_dir(ih->sroot);
   free_iso_path_table(ih->ppath);
   free_iso_path_table(ih->spath);
   g_free(ih->pvd);
   g_free(ih->svd);
   g_free(ih);
}

/*
 * Add file to iso image (works currently only for one file!)
 */

void AddFile(IsoHeader *ih, char *name, guint64 size)
{  static int n;
   char iso[20], joliet[strlen(name)+3];

   n++;
   sprintf(iso,"RAN_%04d.DAT;1", n);
   add_file_record(ih->proot, iso, size, 25,
			106, 7, 16, 12, 28, 10, 8);
   sprintf(joliet,"%s;1", name);
   add_d1_file_record(ih->sroot, joliet, size, 25,
			106, 7, 16, 12, 28, 10, 8);

   ih->volumeSpace += (size+2047)/2048;
}

/*
 * Write out the IsoHeader, return number of sectors written
 */

void WriteIsoHeader(IsoHeader *ih, LargeFile *image)
{  unsigned char zero[2048], sector[2048];
   unsigned char *pvd,*svd;
   int i;

   /* The first 16 sectors are zeroed out */

   memset(zero, 0, 2048);

   for(i=0; i<16; i++)
   {  int n = LargeWrite(image, zero, 2048);

      if(n != 2048)
	Stop(_("Failed writing to sector %lld in image: %s"), (gint64)i, strerror(errno));
   }

   /* Complete the primary volume descriptor */

   pvd = ih->pvd;

   /* 81..88: Volume space size */

   ih->volumeSpace += 16 + ih->dirSectors;
   bp_set_long_bbo(pvd, 81, ih->volumeSpace);

   /* 133..140: Path table size, Fixme: use real values */
   /* 141..144: L Path table location */
   /* 145..148: Optional L Path table location */
   /* 149..152: M Path table location */
   /* 153..156: Optional M Path table location */

   bp_set_long_bbo(pvd, 133, 10);
   bp_set_long_lsb(pvd, 141, 19);
   bp_set_long_lsb(pvd, 145,  0);
   bp_set_long_msb(pvd, 149, 20);
   bp_set_long_msb(pvd, 153,  0);

   /* 157..190: Directory record for root directory, Fixme: use real values */

   bp_set_byte(pvd, 157, 34);           /* Length of directory record       */
   bp_set_byte(pvd, 158,  0);           /* Extended atribute length         */
   bp_set_long_bbo(pvd, 159, 21);       /* Location of Extent               */
   bp_set_long_bbo(pvd, 167, 2048);     /* Data length of file section      */
   set_binary_date(pvd, 175,            /* Binary date rep of creation time */
		   106, 7, 16, 10, 35, 46, 8);
   bp_set_byte(pvd, 182, 2);            /* File flags (we are a directory)  */
   bp_set_byte(pvd, 183, 0);            /* File unit size if interleaved    */
   bp_set_byte(pvd, 184, 0);            /* Interleave gap size              */
   bp_set_short_bbo(pvd, 185, 1);       /* Volume sequence number           */
   bp_set_byte(pvd, 189, 1);            /* Length of file identifier        */
   bp_set_byte(pvd, 190, 0);            /* File identifier                  */

   /* 814..830 Volume creation time */
   
   set_date(pvd, 814, 2006, 7, 16, 10, 35, 46, 23, 8);
   
   /* 831..847 Volume modification time */

   set_date(pvd, 831, 2006, 7, 16, 10, 35, 46, 23, 8);

   /* 848..864 Volume expiration time */

   set_date(pvd, 848, 2106, 7, 16, 10, 35, 46, 23, 8);

   /* 865..881 Volume effective time */

   set_date(pvd, 865, 2006, 7, 16, 10, 35, 46, 23, 8);

   /* Write the pvd */

   if(LargeWrite(image, pvd, 2048) != 2048)
     Stop(_("Failed writing to sector %lld in image: %s"), (gint64)16, strerror(errno));


   /* Create the supplementary volume descriptor */

   svd = ih->svd;

   /* 81..88: Volume space size */

   bp_set_long_bbo(svd, 81, ih->volumeSpace);

   /* 133..140: Path table size, Fixme: use real values */
   /* 141..144: L Path table location */
   /* 145..148: Optional L Path table location */
   /* 149..152: M Path table location */
   /* 153..156: Optional M Path table location */

   bp_set_long_bbo(svd, 133, 10);
   bp_set_long_lsb(svd, 141, 22);
   bp_set_long_lsb(svd, 145,  0);
   bp_set_long_msb(svd, 149, 23);
   bp_set_long_msb(svd, 153,  0);

   /* 157..190: Directory record for root directory, Fixme: use real values */

   bp_set_byte(svd, 157, 34);           /* Length of directory record       */
   bp_set_byte(svd, 158,  0);           /* Extended attribute length        */
   bp_set_long_bbo(svd, 159, 24);       /* Location of Extent               */
   bp_set_long_bbo(svd, 167, 2048);     /* Data length of file section      */
   set_binary_date(svd, 175,            /* Binary date rep of creation time */
		   106, 7, 16, 10, 35, 46, 8);
   bp_set_byte(svd, 182, 2);            /* File flags (we are a directory)  */
   bp_set_byte(svd, 183, 0);            /* File unit size if interleaved    */
   bp_set_byte(svd, 184, 0);            /* Interleave gap size              */
   bp_set_short_bbo(svd, 185, 1);       /* Volume sequence number           */
   bp_set_byte(svd, 189, 1);            /* Length of file identifier        */
   bp_set_byte(svd, 190, 0);            /* File identifier                  */

   /* 814..830 Volume creation time */
   
   set_date(svd, 814, 2006, 7, 16, 10, 35, 46, 23, 8);
   
   /* 831..847 Volume modification time */

   set_date(svd, 831, 2006, 7, 16, 10, 35, 46, 23, 8);

   /* 848..864 Volume expiration time */

   set_date(svd, 848, 2106, 7, 16, 10, 35, 46, 23, 8);

   /* 865..881 Volume effective time */

   set_date(svd, 865, 2006, 7, 16, 10, 35, 46, 23, 8);

   /* Write the svd */

   if(LargeWrite(image, svd, 2048) != 2048)
     Stop(_("Failed writing to sector %lld in image: %s"), (gint64)17, strerror(errno));

   /*** Create and write the volume descriptor set terminator */

   /* 1..8: Standard identifier */

   memset(sector, 0, 2048);

   bp_set_byte(sector, 1, 255);           /* Volume descriptor set terminator */
   bp_set_string(sector, "CD001", 2, 6);  /* Standard identifier              */
   bp_set_byte(sector, 7, 1);             /* Volume descriptor version 1      */

   if(LargeWrite(image, sector, 2048) != 2048)
     Stop(_("Failed writing to sector %lld in image: %s"), (gint64)18, strerror(errno));

   /*** Write the primary and supplementary path tables and root directories */

   if(LargeWrite(image, ih->ppath->lpath, 2048) != 2048)
     Stop(_("Failed writing to sector %lld in image: %s"), (gint64)19, strerror(errno));

   if(LargeWrite(image, ih->ppath->mpath, 2048) != 2048)
     Stop(_("Failed writing to sector %lld in image: %s"), (gint64)20, strerror(errno));
   
   if(LargeWrite(image, ih->proot->dir, 2048) != 2048)
     Stop(_("Failed writing to sector %lld in image: %s"), (gint64)21, strerror(errno));

   if(LargeWrite(image, ih->spath->lpath, 2048) != 2048)
     Stop(_("Failed writing to sector %lld in image: %s"), (gint64)22, strerror(errno));

   if(LargeWrite(image, ih->spath->mpath, 2048) != 2048)
     Stop(_("Failed writing to sector %lld in image: %s"), (gint64)23, strerror(errno));
   
   if(LargeWrite(image, ih->sroot->dir, 2048) != 2048)
     Stop(_("Failed writing to sector %lld in image: %s"), (gint64)24, strerror(errno));

}
