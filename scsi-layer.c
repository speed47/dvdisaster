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

/***
 *** Forward declarations
 ***/

static int query_type(DeviceHandle*, int);
static gint64 query_size(Image*);
static int query_copyright(DeviceHandle*);

static int read_dvd_sector(DeviceHandle*, unsigned char*, int, int);
static int read_cd_sector(DeviceHandle*, unsigned char*, int, int);
static int read_raw_cd_sector(DeviceHandle*, unsigned char*, int, int);

/***
 *** Create a buffer aligned at a 4096 byte boundary.
 *** Some SCSI drivers seem to need this.
 */

AlignedBuffer* CreateAlignedBuffer(int size)
{  AlignedBuffer *ab = g_malloc0(sizeof(AlignedBuffer));

   ab->base = g_malloc(size+4096);
   ab->buf  = ab->base + (4096 - ((unsigned long)ab->base & 4095));

   return ab;
}

void FreeAlignedBuffer(AlignedBuffer *ab)
{  g_free(ab->base);
   g_free(ab);
}

/*
 * Align a length to a multiple of 4. 
 * Some broken chipsets fail on DMA otherways.
 */

static void length_align(unsigned int *length)
{
   if(*length & 3)
   {  Verbose("# Warning: Realigning length from %d to %d\n",
	      *length, *length & ~3);
      *length &= ~3;
   }
}

/***
 *** CD and DVD query routines.
 ***/

/*
 * Send INQUIRY to the device.
 */

int InquireDevice(DeviceHandle *dh, int probe_only)
{  AlignedBuffer *ab = CreateAlignedBuffer(2048);
   Sense sense;
   char *ibuf,*vbuf;
   unsigned char cmd[MAX_CDB_SIZE];
   unsigned char device_type;

   /*** Try to learn something about the device vendor */

   memset(cmd, 0, MAX_CDB_SIZE);
   cmd[0] = 0x12;   /* INQUIRY */
   cmd[4] = 36;     /* allocation length */

   if(SendPacket(dh, cmd, 6, ab->buf, 36, &sense, DATA_READ)<0)
   {  FreeAlignedBuffer(ab);
      if(probe_only) return 0x1f;  /* don't care about failure, just return invalid device */

      strcpy(dh->devinfo, _("unknown"));

#ifdef SYS_LINUX
      PrintCLI("\n");
      Stop(_("Can open %s, but INQUIRY fails.\n"
	     "Chances are that you're using ide-scsi emulation for an ATAPI drive,\n"
	     "and try to access it via /dev/cdrom or /dev/hd?.\n"
	     "Either use /dev/scd? or /dev/sr? instead, or disable ide-scsi emulation.\n"),
	   dh->device);
      return 0;
#else
      PrintCLI("\n");
      Stop(_("INQUIRY failed. Something is wrong with drive %s.\n"),dh->device);
      return 0;
#endif
   }
   else
   {  int i,j,vidx=0; 

      ibuf = dh->devinfo;
      vbuf = dh->vendor;
      for(i=0,j=8; j<36; j++)   
      {  if(j==32) 
	 {  vidx = i;
	    if(i>0 && !isspace((int)ibuf[i-1])) /* separate the version string */
	      ibuf[i++] = ' ';
	 }
         if(   isprint(ab->buf[j])         /* eliminate multiple spaces and unprintables */
	       && (!isspace(ab->buf[j]) || (i>0 && !isspace((int)ibuf[i-1]))))
	 {    vbuf[i] = ab->buf[j];
	      ibuf[i++] = ab->buf[j];
	 }
      }
      ibuf[i] = vbuf[i] = 0;

      if(vidx) vbuf[vidx--] = 0;
      while(vidx >= 0 && vbuf[vidx] == ' ')
	vbuf[vidx--] =0;

      if(ab->buf[0] != 0x05 && !probe_only)
      {  PrintCLI("\n");
	 if(ab->buf[0]) Stop(_("Device %s (%s) is not an optical drive."),dh->device,ibuf);
	 else           Stop(_("Device %s (%s) is a hard disk."),dh->device,ibuf);
      }
   }

   device_type = ab->buf[0] & 0x1f; 
   FreeAlignedBuffer(ab);

   return device_type;  /* return the SCSI peripheral device type */
}

/*
 * Some drives do not know about profiles.
 * In theory only plain CD-ROM type drives have no choices between
 * different profiles, but we must consider buggy firmware also.
 * Do a quick and dirty differentiation between the possible formats.
 */

static int try_fallback_type_check(DeviceHandle *dh)
{  AlignedBuffer *ab;
   Sense *sense = &dh->sense;
   unsigned char cmd[MAX_CDB_SIZE];
   int length;

   Verbose("# *** try_fallback_type_check(%s) ***\n", dh->devinfo);

   ab = CreateAlignedBuffer(2048); 
  
   /*** If the medium is a BD, the following will succeed. */

   /* Query length of returned data */

   memset(cmd, 0, MAX_CDB_SIZE);
   cmd[0] = 0xad;     /* READ DVD STRUCTURE */
   cmd[1] = 0x01;     /* subcommand for BD */
   cmd[6] = 0;        /* First layer */
   cmd[7] = 0;        /* We want DI (disc information) */
   cmd[8] = 0;        /* Allocation length */
   cmd[9] = MIN_TRANSFER_LEN;

   /*** Only a BD should respond positively here */
   
   Verbose("# BD: trying READ DVD with BD subcommand for size\n");
   if(SendPacket(dh, cmd, 12, ab->buf, MIN_TRANSFER_LEN, sense, DATA_READ)<0)
   {  
      Verbose("# failed -> not a BD type medium.\n");
      goto try_dvd;
   }

   /*** Some DVD drives ignore the media type 0x01 and return the dvd structure.
	Since the DVD structure is 2052 bytes while the BD DI is 4100 bytes,
	we can tell from the size whether we have been fooled. */

   length = ab->buf[0]<<8 | ab->buf[1];
   length += 2;
   if(length != 4100) /* not a BD */
   {  Verbose("# allocation length = %d != 4100 -> not a BD type medium.\n", length);
      goto try_dvd;
   }

   Verbose("# -> Looks like a BD type medium.\n");
   dh->profileDescr = "(BD)";
   dh->shortProfile = "(BD)";
   dh->mainType = DVD;
   dh->subType = DVD;
   FreeAlignedBuffer(ab);
   return TRUE;

   /*** If the medium is a DVD, the following query will succeed. */

   /* Query length of returned data */
try_dvd:
   Verbose("# DVD: Trying READ DVD STRUCTURE.\n");

   memset(cmd, 0, MAX_CDB_SIZE);
   cmd[0] = 0xad;     /* READ DVD STRUCTURE */
   cmd[6] = 0;        /* First layer */
   cmd[7] = 0;        /* We want PHYSICAL info */
   cmd[8] = 0;        /* Allocation length */
   cmd[9] = MIN_TRANSFER_LEN;

   /* Different drives react with different error codes on this request;
      especially CDROMs seem to react very indeterministic here 
      (okay, they obviously do not know about DVD cdbs).
      So we do not look for specific error and regard any failure as a sign
      that the medium is not a DVD. */

   if(SendPacket(dh, cmd, 12, ab->buf, MIN_TRANSFER_LEN, sense, DATA_READ)<0)
   {  
      Verbose("# failed -> not a DVD type medium\n");
      goto assume_cd;
   }

   Verbose("# -> Looks like a DVD type medium.\n");
   dh->profileDescr = "(DVD)";
   dh->shortProfile = "(DVD)";
   dh->mainType = DVD;
   dh->subType = DVD;
   FreeAlignedBuffer(ab);
   return TRUE;

   /*** No need to investigate further; if it is not a CD some
	subsequent tests will fail. */

assume_cd:  
   Verbose("# CD: It's either a CD or totally broken.\n");
   dh->profileDescr = "(CD)";
   dh->shortProfile = "(CD)";
   dh->mainType = CD;
   FreeAlignedBuffer(ab);
   return TRUE;
}

/*
 * Ask drive for active profile; this gives us a hint
 * about the kind of medium the drive believes to see
 */

static int get_configuration(DeviceHandle *dh)
{  AlignedBuffer *ab;
   Sense *sense = &dh->sense;
   unsigned char cmd[MAX_CDB_SIZE];
   guint32 data_length;
   int len,ret;

   Verbose("# *** get_configuration(%s) ***\n", dh->devinfo);

   len = 2048;
   ab = CreateAlignedBuffer(len); 

   memset(cmd, 0, MAX_CDB_SIZE);
   cmd[0] = 0x46;      /* GET CONFIGURATION */
   cmd[1] = 1;         /* only current features */
   cmd[7] = len>>8;    /* buffer size */
   cmd[8] = len&0xff;  

   ret = SendPacket(dh, cmd, 10, ab->buf, len, sense, DATA_READ);

   if(ret<0) 
   {  RememberSense(sense->sense_key, sense->asc, sense->ascq);
      Verbose("# -> failure\n");
      FreeAlignedBuffer(ab);
      if(try_fallback_type_check(dh))
	return 0;
      return ret;
   }

   data_length = ab->buf[0]<<24 | ab->buf[1] | ab->buf[2] | ab->buf[3];
   dh->profile = ab->buf[6]<<8 | ab->buf[7];
   FreeAlignedBuffer(ab);

   Verbose("# %d data len, %x current\n", data_length, dh->profile);

   switch(dh->profile)
   {  /* Note: subType handling is different for CD for historical reasons */
      case 0x08: dh->profileDescr = "CD-ROM";
	         dh->shortProfile = "CD-ROM";
                 dh->mainType = CD;
	         break;
      case 0x09: dh->profileDescr = "CD-R";
	         dh->shortProfile = "CD-R";
                 dh->mainType = CD;
	         break;
      case 0x0a: dh->profileDescr = "CD-RW";
	         dh->shortProfile = "CD-RW";
                 dh->mainType = CD;
		 dh->rewriteable = TRUE;
	         break;

      case 0x10: dh->profileDescr = "DVD-ROM";
	         dh->shortProfile = "DVD-ROM";
                 dh->mainType = DVD;
		 dh->subType = DVD;
	         break;
      case 0x11: dh->profileDescr = "DVD-R Sequential recording";
	         dh->shortProfile = "DVD-R";
	         dh->mainType = DVD; dh->isDash = TRUE;
		 dh->subType = DVD_DASH_R;
	         break;
      case 0x12: dh->profileDescr = "DVD-RAM";
	         dh->shortProfile = "DVD-RAM";
                 dh->mainType = DVD;
		 dh->rewriteable = TRUE;
		 dh->subType = DVD_RAM;
	         break;
      case 0x13: dh->profileDescr = "DVD-RW Restricted overwrite";
	         dh->shortProfile = "DVD-RW";
	         dh->mainType = DVD; dh->isDash = TRUE;
		 dh->rewriteable = TRUE;
		 dh->subType = DVD_DASH_RW;
	         break;
      case 0x14: dh->profileDescr = "DVD-RW Sequential overwrite";
	         dh->shortProfile = "DVD-RW";
	         dh->mainType = DVD; dh->isDash = TRUE;
		 dh->rewriteable = TRUE;
		 dh->subType = DVD_DASH_RW;
	         break;
      case 0x15: dh->profileDescr = "DVD-R DL Sequential recording";
	         dh->shortProfile = "DVD-R DL";
	         dh->mainType = DVD; dh->isDash = TRUE;
		 dh->subType = DVD_DASH_R_DL;
	         break;
      case 0x16: dh->profileDescr = "DVD-R DL Layer jump recording";
	         dh->shortProfile = "DVD-R DL";
	         dh->mainType = DVD; dh->isDash = TRUE;
		 dh->subType = DVD_DASH_R_DL;
	         break;
      case 0x17: dh->profileDescr = "DVD-RW DL";
	         dh->shortProfile = "DVD-RW DL";
	         dh->mainType = DVD; dh->isDash = TRUE;
		 dh->rewriteable = TRUE;
		 dh->subType = DVD_DASH_RW_DL;
	         break;

      case 0x1a: dh->profileDescr = "DVD+RW";
	         dh->shortProfile = "DVD+RW";
	         dh->mainType = DVD; dh->isPlus = TRUE;
		 dh->rewriteable = TRUE;
		 dh->subType = DVD_PLUS_RW;
	         break;
      case 0x1b: dh->profileDescr = "DVD+R";
	         dh->shortProfile = "DVD+R";
	         dh->mainType = DVD; dh->isPlus = TRUE;
		 dh->subType = DVD_PLUS_R;
	         break;
      case 0x2a: dh->profileDescr = "DVD+RW DL";
	         dh->shortProfile = "DVD+RW DL";
	         dh->mainType = DVD; dh->isPlus = TRUE;
		 dh->rewriteable = TRUE;
		 dh->subType = DVD_PLUS_RW_DL;
	         break;
      case 0x2b: dh->profileDescr = "DVD+R DL";
	         dh->shortProfile = "DVD+R DL";
	         dh->mainType = DVD; dh->isPlus = TRUE;
		 dh->subType = DVD_PLUS_R_DL;
	         break;

      case 0x40: dh->profileDescr = "BD-ROM";
	         dh->shortProfile = "BD-ROM";
	         dh->mainType = BD;
		 dh->subType = BD;
	         break;
      case 0x41: dh->profileDescr = "BD-R Sequential recording mode";
 	         dh->shortProfile = "BD-R";
	         dh->mainType = BD;
		 dh->subType = BD_R;
	         break;
      case 0x42: dh->profileDescr = "BD-R Random recording mode";
 	         dh->shortProfile = "BD-R";
	         dh->mainType = BD;
		 dh->subType = BD_R;
	         break;
      case 0x43: dh->profileDescr = "BD-RE";
 	         dh->shortProfile = "BD-RE";
	         dh->mainType = BD;
		 dh->rewriteable = TRUE;
		 dh->subType = BD_RE;
	         break;

      default:   dh->profileDescr = "Unknown profile";
	         dh->mainType = UNSUPPORTED;
		 dh->subType = UNSUPPORTED;
	         break;
   }

   Verbose("-> profile %x: %s\n", dh->profile, dh->profileDescr);
   return ret;
}

/*
 * Incomplete/unfinalized media usually cause big trouble
 * as the usual probes (reading the TOC etc. fail).
 * However sometimes they are still readable sectorwise,
 * so see if we can read the ISO root sector.
 * If we can, close eyes and continue. 
 */

static int query_incomplete(DeviceHandle *dh, int probe_only)
{  AlignedBuffer *ab = CreateAlignedBuffer(2048);
   int status;

   Verbose("#QUERY INCOMPLETE: probing ISO root sector\n");

   status = dh->read(dh, ab->buf, 16, 1);
   if(status)
   {  Verbose("# Reading the ISO root sector failed\n");
      FreeAlignedBuffer(ab);
      return FALSE;
   }

   if(!strncmp((char*)(ab->buf+1), "CD001", 5))
   {  Verbose("#QUERY INCOMPLETE: sector 16 looks like an ISO root sector\n");
      dh->typeDescr = g_strdup_printf(_("Incomplete %s"), dh->shortProfile);
      FreeAlignedBuffer(ab);
      return TRUE;
   }
   else Verbose("-> not an ISO root sector\n");

   FreeAlignedBuffer(ab);
   return FALSE;
}

/*
 * CD specific probing
 */

static int query_cd(DeviceHandle *dh, int probe_only)
{  AlignedBuffer *ab = CreateAlignedBuffer(4096);
   unsigned char *buf = ab->buf;
   unsigned char cmd[MAX_CDB_SIZE];
   Sense *sense = &dh->sense;
   unsigned int length;
   int control;

   Verbose("#CD: starting media probe\n");

   /*** First, do a READ TOC with format 0 to fetch the CONTROL field. */
   
   memset(cmd, 0, MAX_CDB_SIZE);
   cmd[0] = 0x43;  /* READ TOC/PMA/ATIP */
   cmd[2] = 0;     /* format; we want the TOC */
   cmd[6] = 1;     /* track/session number */
   cmd[7] = 0;     /* allocation length */
   cmd[8] = MIN_TRANSFER_LEN;

   Verbose("#CD: querying size of READ TOC/PMA/ATIP (for TOC)\n");
   if(SendPacket(dh, cmd, 10, buf, MIN_TRANSFER_LEN, sense, DATA_READ)<0)
   {  
      FreeAlignedBuffer(ab);
      if(!probe_only)
         Stop(_("%s\nCould not query TOC length.\n"),
	      GetSenseString(sense->sense_key, sense->asc, sense->ascq, TRUE));

      /* Blank CDs have no TOC, so they fail here.
	 Give back some meaningful medium description. */
      dh->typeDescr = g_strdup_printf("%s (%s)", dh->shortProfile, _("blank"));
      return FALSE;
   }

   length = buf[0]<<8 | buf[1];
   length += 2  ;  /* MMC3: "Disc information length excludes itself" */
   length_align(&length);
   Verbose("#CD: size returned is %d\n", length);

   if(length>1024) /* don't let the drive hack us using a buffer overflow ;-) */
   {  if(Closure->verbose)
	 HexDump(buf, 1024, 16);

      FreeAlignedBuffer(ab);
      if(!probe_only)
	 Stop(_("TOC info too long (%d), probably multisession.\n"),length);
      return FALSE;
   }

   memset(cmd, 0, MAX_CDB_SIZE);
   cmd[0] = 0x43;  /* READ TOC/PMA/ATIP */
   cmd[2] = 0;     /* format; we want the TOC */
   cmd[6] = 1;     /* track/session number */
   cmd[7] = (length>>8) & 0xff; /* allocation length */
   cmd[8] = length & 0xff;

   Verbose("#CD: querying real READ TOC/PMA/ATIP (for TOC)\n");
   if(SendPacket(dh, cmd, 10, buf, length, sense, DATA_READ)<0)
   {  FreeAlignedBuffer(ab);
      if(!probe_only)
	 Stop(_("%s\nCould not read TOC.\n"),
	      GetSenseString(sense->sense_key, sense->asc, sense->ascq, TRUE));
      return FALSE;
   }
   if(Closure->verbose)
      HexDump(buf, length, 16);

   control = buf[5];
   Verbose("#CD: control is 0x%x\n", control);

   /*** Do the READ TOC again with format 2 to fetch the full toc
        as we want the disc type info also. 
	We do not use the CONTROL data included here as it turned
	out to be invalid on certain CD-ROMs. Bleah. */

   memset(cmd, 0, MAX_CDB_SIZE);
   cmd[0] = 0x43;  /* READ TOC/PMA/ATIP */
   cmd[1] = 0x02;  /* TIME bit required for this format */
   cmd[2] = 2;     /* format; we want the full TOC */
   cmd[6] = 1;     /* track/session number */
   cmd[7] = 0;     /* allocation length */
   cmd[8] = MIN_TRANSFER_LEN;

   Verbose("#CD: querying size of READ TOC/PMA/ATIP (for full TOC)\n");
   if(SendPacket(dh, cmd, 10, buf, MIN_TRANSFER_LEN, sense, DATA_READ)<0)
   {  FreeAlignedBuffer(ab);
      if(!probe_only)
	 Stop(_("%s\nCould not query full TOC length.\n"),
	      GetSenseString(sense->sense_key, sense->asc, sense->ascq, TRUE));
      return FALSE;
   }

   length = buf[0]<<8 | buf[1];
   length += 2;    /* MMC3: "Disc information length excludes itself" */
   length_align(&length);
   Verbose("#CD: size returned is %d\n", length);

   if(length < 15)
   {  FreeAlignedBuffer(ab);
      if(!probe_only)
	 Stop(_("TOC info too short, length %d.\n"),length);
      return FALSE;
   }
   if(length>1024) /* don't let the drive hack us using a buffer overflow ;-) */
   {  FreeAlignedBuffer(ab);
      if(!probe_only)
	 Stop(_("TOC info too long (%d), probably multisession.\n"),length);
      return FALSE;
   }

   memset(cmd, 0, MAX_CDB_SIZE);
   cmd[0] = 0x43;  /* READ TOC/PMA/ATIP */
   cmd[1] = 0x02;  /* TIME bit required for this format */
   cmd[2] = 2;     /* format; we want the full TOC */
   cmd[6] = 1;     /* track/session number */
   cmd[7] = (length>>8) & 0xff; /* allocation length */
   cmd[8] = length & 0xff;

   Verbose("#CD: querying real READ TOC/PMA/ATIP (for full TOC)\n");
   if(SendPacket(dh, cmd, 10, buf, length, sense, DATA_READ)<0)
   {  FreeAlignedBuffer(ab);
      if(!probe_only)
	 Stop(_("%s\nCould not read full TOC.\n"),
	      GetSenseString(sense->sense_key, sense->asc, sense->ascq, TRUE));
      return FALSE;
   }
   if(Closure->verbose)
      HexDump(buf, length, 16);

   if(buf[7] != 0xa0)
   {  int i;
      PrintLog(_("\nUnexpected TOC format (length %d):\n"),length);

      for(i=0; i<(int)length; i++) 
      {  PrintLog("%02x ",buf[i]);
	 if(i==3 || (i-3)%11 == 0) PrintLog("\n");
      }
      FreeAlignedBuffer(ab);
      if(!probe_only)
	 Stop(_("Consider sending a bug report.\n"));
      return FALSE;
   }

   dh->sessions = buf[3];
   Verbose("#CD: %d sessions\n", dh->sessions);

   if(control & 4)
     switch(buf[13])
     {  case 0x00: dh->typeDescr = g_strdup_printf("%s mode 1", dh->profileDescr); dh->subType = DATA1; break;
        case 0x10: dh->typeDescr = g_strdup_printf("%s/CD-I", dh->profileDescr); dh->subType = UNSUPPORTED; break;
        case 0x20: dh->typeDescr = g_strdup_printf("%s XA",dh->profileDescr); dh->subType = XA21; break;
        default:   dh->typeDescr = g_strdup_printf("%s ??",dh->profileDescr); dh->subType = UNSUPPORTED; break;
     }
   else 
   {  dh->typeDescr = g_strdup_printf("%s Audio", dh->profileDescr); 
      dh->subType = UNSUPPORTED;
   }

   FreeAlignedBuffer(ab);
   Verbose("#CD: CD medium detected, type: %s\n", dh->typeDescr);
   return TRUE;
}

/*
 * DVD specific probing
 */

static int query_dvd(DeviceHandle *dh, int probe_only)
{  AlignedBuffer *ab = CreateAlignedBuffer(4096);
   unsigned char *buf = ab->buf;
   unsigned char cmd[MAX_CDB_SIZE];
   Sense *sense = &dh->sense;
   unsigned int ua_start,ua_end,ua_end0;
   int phy_info4, phy_info6;
   unsigned int length;
   int i;

   Verbose("#DVD: starting media probe\n");

   /* Query length of returned data for READ DVD STRUCTURE */

   memset(cmd, 0, MAX_CDB_SIZE);
   cmd[0] = 0xad;     /* READ DVD STRUCTURE */
   cmd[6] = 0;        /* First layer */
   cmd[7] = 0;        /* We want PHYSICAL info */
   cmd[8] = 0;        /* Allocation length */
   cmd[9] = MIN_TRANSFER_LEN;

   Verbose("#DVD: trying READ DVD for size of PHYSICAL info\n");
   if(SendPacket(dh, cmd, 12, buf, MIN_TRANSFER_LEN, sense, DATA_READ)<0)
   {  FreeAlignedBuffer(ab);
      if(!probe_only)
	 Stop(_("%s\nCould not query dvd structure length.\n"), 
	      GetSenseString(sense->sense_key, sense->asc, sense->ascq, TRUE));
      return FALSE;
   }

   length = buf[0]<<8 | buf[1];
   length += 2;
   length_align(&length);

   if(length>4096) /* don't let the drive hack us using a buffer overflow ;-) */
   {  FreeAlignedBuffer(ab);
      if(!probe_only)
	 Stop(_("Could not query dvd physical structure - implausible packet length %d\n"),length);
      return FALSE;
   }
   Verbose("#DVD: size returned is %d\n", length);

   /* Do the real query */

   memset(cmd, 0, MAX_CDB_SIZE);
   cmd[0] = 0xad;     /* READ DVD STRUCTURE */
   cmd[6] = 0;        /* First layer */
   cmd[7] = 0;        /* We want PHYSICAL info */
   cmd[8] = (length>>8) & 0xff;  /* Allocation length */
   cmd[9] = length & 0xff;

   Verbose("#DVD: trying READ DVD for real PHYSICAL info\n");
   if(SendPacket(dh, cmd, 12, buf, length, sense, DATA_READ)<0)
   {  FreeAlignedBuffer(ab);
      if(!probe_only)
	 Stop(_("%s\nCould not query physical dvd structure.\n"),
	      GetSenseString(sense->sense_key, sense->asc, sense->ascq, TRUE));
      return FALSE;
   }
   if(Closure->verbose)
      HexDump(buf, length, 16);

   dh->bookType = (buf[4]>>4) & 0x0f;
   Verbose("#DVD: book type %d\n", dh->bookType);

   /* Determine number of layers */

   dh->layers = 1 + ((buf[6] & 0x60) >> 5);
   dh->sessions = 1;
   Verbose("#DVD: %d layers\n", dh->layers);

   /* While we're at it, extract the user area size.
      For +RW media, this is better than the value provided by READ CAPACITY. */

   ua_start = /*buf[ 8]<<24 |*/ buf[ 9]<<16 | buf[10]<<8 | buf[11]; 
   ua_end   = /*buf[12]<<24 |*/ buf[13]<<16 | buf[14]<<8 | buf[15];
   ua_end0  = /*buf[16]<<24 |*/ buf[17]<<16 | buf[18]<<8 | buf[19];
   Verbose("#DVD: ua_start/_end/_end0: %d %d %d\n", ua_start, ua_end, ua_end0);

   if(dh->layers == 1)
   {  
      dh->userAreaSize = (gint64)(ua_end-ua_start);

      if(dh->userAreaSize < 0 || dh->userAreaSize > MAX_DVD_SL_SIZE)
      {  LogWarning(_("READ DVD STRUCTURE: implausible medium size, %lld-%lld=%lld sectors\n"),
		    (gint64)ua_end, (gint64)ua_start, (gint64)dh->userAreaSize);
	 dh->userAreaSize = 0;
      }
   }
   else 
   {  
      dh->userAreaSize = (gint64)(ua_end0-ua_start)*2;

      if(dh->userAreaSize < 0 || dh->userAreaSize > MAX_DVD_DL_SIZE)
      {  LogWarning(_("READ DVD STRUCTURE: implausible medium size, %lld-%lld=%lld sectors\n"),
		    (gint64)ua_end0, (gint64)ua_start, (gint64)dh->userAreaSize);
	 dh->userAreaSize = 0;
      }
   }

   /*** Some drives report lead in information along with physical info. 
        This allows us to collect some info on DVD-ROM drives which 
        can not do READ DVD STRUCTURE with subtype 0x0e. */

   dh->manuID[0] = 0;

   /* Check if some nonzero bytes have been returned */

   for(i=0x200; i<length; i++)
      if(buf[i])
      {  int j;

	 Verbose("#DVD: physical info contains lead-in data\n");
	 for(j=0; j<6; j++)
	    dh->manuID[j] = isprint(buf[0x225+j]) ? buf[0x225+j] : ' ';
	 dh->manuID[6] = ' ';

	 for(j=0; j<6; j++)
	    dh->manuID[j+7] = isprint(buf[0x22d+j]) ? buf[0x22d+j] : ' ';
	 dh->manuID[13] = 0;

	 for(j=11; j>=0; j--)
	    if(dh->manuID[j] == ' ') dh->manuID[j] = 0;
	    else break;

	 Verbose("#DVD: manufacturer id %s\n", dh->manuID);
	 break;
      }

   /*** Find out medium type */

   phy_info4 = buf[4];
   phy_info6 = buf[6];
   Verbose("#DVD: phy_info4/6: 0x%x 0x%x\n", phy_info4, phy_info6);

   /* Try getting ADIP information. 
      This is more reliable than the physical info. */

   memset(cmd, 0, MAX_CDB_SIZE);
   cmd[0] = 0xad;     /* READ DVD STRUCTURE */
   cmd[6] = 0;        /* First layer */
   cmd[7] = 0x11;     /* We want the ADIP */
   cmd[8] = 0;        /* Allocation length */
   cmd[9] = MIN_TRANSFER_LEN;

   Verbose("#DVD: trying READ DVD for size of ADIP\n");
   if(SendPacket(dh, cmd, 12, buf, MIN_TRANSFER_LEN, sense, DATA_READ) == 0)
   {  length = buf[0]<<8 | buf[1];
      length += 2;
      length_align(&length);

      Verbose("#DVD: size returned is %d\n", length);

      if(length < 4096)
      {  memset(cmd, 0, MAX_CDB_SIZE);
	 cmd[0] = 0xad;     /* READ DVD STRUCTURE */
	 cmd[6] = 0;        /* First layer */
	 cmd[7] = 0x11;     /* We want the ADIP */
	 cmd[8] = (length>>8) & 0xff;  /* Allocation length */
	 cmd[9] = length & 0xff;
	    
	 Verbose("#DVD: trying READ DVD for real ADIP\n");
	 if(SendPacket(dh, cmd, 12, buf, length, sense, DATA_READ) == 0)
	 {  int i;

	    if(Closure->verbose)
	      HexDump(buf, length, 16);

	    dh->isPlus = TRUE;
	    phy_info4 = buf[4];
	    phy_info6 = buf[6];
	    Verbose("#DVD: assuming DVD plus; phy_info4/6 now 0x%x 0x%x\n", phy_info4, phy_info6);

	    for(i=0; i<11; i++)
	      dh->manuID[i] = isprint(buf[23+i]) ? buf[23+i] : ' ';
	    dh->manuID[11] = 0;

	    for(i=10; i>=0; i--)
	      if(dh->manuID[i] == ' ') dh->manuID[i] = 0;
	      else break;
	 }
      }
   }
   else Verbose("#DVD: no ADIP\n");

   /* Get pre-recorded info from lead-in (only on -R/-RW media).
      Only used for getting the manufacturer ID. */
   
   memset(cmd, 0, MAX_CDB_SIZE);
   cmd[0] = 0xad;     /* READ DVD STRUCTURE */
   cmd[6] = 0;        /* First layer */
   cmd[7] = 0x0E;     /* We want the lead-in info */
   cmd[8] = 0;        /* Allocation length */
   cmd[9] = MIN_TRANSFER_LEN;

   Verbose("#DVD: trying READ DVD for size of lead-in\n");
   if(SendPacket(dh, cmd, 12, buf, MIN_TRANSFER_LEN, sense, DATA_READ) == 0)
   {  length = buf[0]<<8 | buf[1];
      length += 2;
      length_align(&length);

      Verbose("#DVD: size returned is %d\n", length);
      if(length < 4096)
      {  memset(cmd, 0, MAX_CDB_SIZE);
	 cmd[0] = 0xad;     /* READ DVD STRUCTURE */
	 cmd[6] = 0;        /* First layer */
	 cmd[7] = 0x0E;     /* We want the lead-in info */
	 cmd[8] = (length>>8) & 0xff;  /* Allocation length */
	 cmd[9] = length & 0xff;

	 Verbose("#DVD: trying READ DVD for real lead-in\n");
	 if(SendPacket(dh, cmd, 12, buf, length, sense, DATA_READ) == 0)
	 {  int i;

 	    dh->isDash = TRUE;
	    Verbose("#DVD: assuming DVD dash\n");

	    for(i=0; i<6; i++)
	      dh->manuID[i] = isprint(buf[21+i]) ? buf[21+i] : ' ';
	    dh->manuID[6] = ' ';

	    for(i=0; i<6; i++)
	      dh->manuID[i+7] = isprint(buf[29+i]) ? buf[29+i] : ' ';
	    dh->manuID[13] = 0;

	    for(i=11; i>=0; i--)
	      if(dh->manuID[i] == ' ') dh->manuID[i] = 0;
	      else break;
	 }
      }
   } else Verbose("#DVD: lead-in could not be queried\n");

   /*** Layer type info (may be faked) */

   if(Closure->verbose)
   {  int layer_type = phy_info6 & 0x0f;
      Verbose("#DVD: Layer type(s): ");
      if(layer_type & 0x01) Verbose("embossed ");
      if(layer_type & 0x02) Verbose("recordable ");
      if(layer_type & 0x04) Verbose("rewriteable ");
      Verbose("\n");
   }

   /*** Evaluate book type */

   dh->bookType = (phy_info4>>4) & 0x0f;
   Verbose("#DVD: book type (%d) evaluation...\n", dh->bookType);
   switch(dh->bookType)   /* evaluate the book type */
   {  case  1: dh->bookDescr = "DVD-RAM"; 
               dh->rewriteable = TRUE;
	       dh->subType = DVD_RAM;
               break;
      case  2: dh->bookDescr = dh->layers == 1 ? "DVD-R" : "DVD-R DL"; 
	       dh->subType   = dh->layers == 1 ? DVD_DASH_R : DVD_DASH_R_DL;
	       break;
      case  3: dh->bookDescr = "DVD-RW"; 
	       dh->subType = DVD_DASH_RW;
               dh->rewriteable = TRUE;
	       break;
      case  4: dh->bookDescr = "HD DVD-ROM";
	       dh->subType = UNSUPPORTED;
	       break;
      case  5: dh->bookDescr = "HD DVD-RAM";
	       dh->subType = UNSUPPORTED;
	       break;
      case  6: dh->bookDescr = "HD DVD-R";
	       dh->subType = UNSUPPORTED;
	       break;
      case  9: dh->bookDescr = "DVD+RW"; 
               dh->rewriteable = TRUE;
	       dh->subType = DVD_PLUS_RW;
	       break;
      case 10: dh->bookDescr = "DVD+R"; 
	       dh->subType = DVD_PLUS_R;
	       break;
      case 13: dh->bookDescr = "DVD+RW DL"; 
               dh->rewriteable = TRUE;
	       dh->subType = DVD_PLUS_RW_DL;
	       break;
      case 14: dh->bookDescr = "DVD+R DL"; 
	       dh->subType = DVD_PLUS_R_DL;
	       break;

      case  0: /* tricky case: real or faked DVD-ROM? */
      {  int layer_type = phy_info6 & 0x0f;
	 
	 Verbose("#DVD: fake DVD-ROM detection (%d %d 0x%x)\n", 
		 dh->isDash, dh->isPlus, layer_type); 
	 if(dh->isDash)
	 {  dh->typeDescr = g_strdup(dh->layers == 1 ? "DVD-R/-RW" : "DVD-R DL"); 
	    dh->subType = DVD;
	    break;
	 }

	 if(dh->isPlus)
	 {  dh->typeDescr = g_strdup(dh->layers == 1 ? "DVD+R/+RW" : "DVD+R DL"); 
	    dh->subType = DVD;
	    break;
	 }

	 if(layer_type & 0x06) /* strange thing: (re-)writeable but neither plus nor dash */ 
	 {  dh->typeDescr = g_strdup("DVD-ROM (fake)");
	    dh->subType = DVD;
	    break;
	 }

	 dh->typeDescr = g_strdup("DVD-ROM");
	 dh->subType = UNSUPPORTED;
	 break;
      }

      default: 
	dh->typeDescr = g_strdup_printf("DVD book type 0x%02x",(phy_info4>>4) & 0x0f); 
	dh->subType = UNSUPPORTED;
	break;
   }
   
   if(!dh->typeDescr)
     dh->typeDescr = g_strdup(dh->bookDescr);

   FreeAlignedBuffer(ab);
   Verbose("#DVD: DVD medium detected, type %s\n", dh->typeDescr);
   return TRUE;
}

/*
 * BD specific probing
 */

static int query_bd(DeviceHandle *dh, int probe_only)
{  AlignedBuffer *ab = CreateAlignedBuffer(8196);
   unsigned char *buf = ab->buf;
   unsigned char cmd[MAX_CDB_SIZE];
   Sense *sense = &dh->sense;
   unsigned int length;
   int i,j;

   Verbose("#BD: starting media probe\n");

   /* Query length of returned data.
      Maybe skip this in the future; it must be 4098 (+2) on BD */

   memset(cmd, 0, MAX_CDB_SIZE);
   cmd[0] = 0xad;     /* READ DISC STRUCTURE */
   cmd[1] = 0x01;     /* subcommand for BD */
   cmd[6] = 0;        /* First layer */
   cmd[7] = 0;        /* We want DI (disc information) */
   cmd[8] = 0;        /* Allocation length */
   cmd[9] = MIN_TRANSFER_LEN;

   Verbose("#BD: trying READ DISC STRUCTURE for size\n");
   if(SendPacket(dh, cmd, 12, buf, MIN_TRANSFER_LEN, sense, DATA_READ)<0)
   {  	FreeAlignedBuffer(ab);
        if(!probe_only)
	  Stop(_("%s\nCould not query BD disc structure length.\n"), 
	       GetSenseString(sense->sense_key, sense->asc, sense->ascq, TRUE));
  	return FALSE;
   }

   length = buf[0]<<8 | buf[1];
   length += 2;
   length_align(&length);
   Verbose("#BD: disc structure query succeeded, length %d bytes\n", length);

   /* Do the real query */

   memset(cmd, 0, MAX_CDB_SIZE);
   cmd[0] = 0xad;     /* READ DISC STRUCTURE */
   cmd[1] = 0x01;     /* media type BD */
   cmd[6] = 0;        /* First layer */
   cmd[7] = 0;        /* We want DI (disc information) */
   cmd[8] = (length>>8) & 0xff;  /* Allocation length */
   cmd[9] = length & 0xff;

   Verbose("#BD: trying READ DISC STRUCTURE for real query\n");
   if(SendPacket(dh, cmd, 12, buf, length, sense, DATA_READ)<0)
   {  FreeAlignedBuffer(ab);
      if(!probe_only)
	 Stop(_("%s\nCould not query BD disc structure.\n"),
	      GetSenseString(sense->sense_key, sense->asc, sense->ascq, TRUE));
      return FALSE;
   }

   if(Closure->verbose)
      HexDump(buf, length, 16);

   dh->layers = 0;        /* n.a. ? */
   dh->sessions = 1;
   dh->userAreaSize = 0;  /* n.a. ? */

   /* Assemble manufacturer info */

   for(i=0,j=-1; i<6; i++)  /* Disc Manufacturer ID */
   {  char c = buf[4+100+i];

      if(isprint(c))
      {  dh->manuID[i] = c; j=i;
      }
      else dh->manuID[i] = ' ';
   }
   dh->manuID[++j]=',';

   for(i=0; i<3; i++)       /* Media type ID */
   {  char c = buf[4+106+i];
      dh->manuID[++j] = isprint(c) ? c : ' ';
   }
   if(dh->manuID[j] == ',') 
        dh->manuID[j]=0;
   else dh->manuID[++j] = 0;

   /* Media type recognition */

   if(!strncmp((char*)&buf[4+8], "BDO", 3))
   {  dh->typeDescr = g_strdup("BD-ROM");
      dh->subType = UNSUPPORTED;
   }

   if(!strncmp((char*)&buf[4+8], "BDW", 3))
   {  dh->typeDescr = g_strdup("BD-RE");
      dh->rewriteable = TRUE;
      dh->subType = BD_RE;
   }

   if(!strncmp((char*)&buf[4+8], "BDR", 3))
   {  dh->typeDescr = g_strdup("BD-R");
      dh->subType = BD_R;
   }

   Verbose("#BD: BD medium successfully probed, type %s\n", dh->typeDescr);
   FreeAlignedBuffer(ab);
   return TRUE;
}

/*
 * Find out what type of disc has been inserted.
 */

static int query_type(DeviceHandle *dh, int probe_only)
{  AlignedBuffer *ab;
   unsigned char *buf;
   unsigned char cmd[MAX_CDB_SIZE];
   Sense *sense = &dh->sense;
   unsigned int length;
   int status;

   /*** See which profile the drive selected.
	This should at least give us a hint to decide 
	between CD, DVD and BD. */

   Verbose("# *** query_type(%s, %d) ***\n", dh->devinfo, probe_only);

   status = get_configuration(dh);
   if(status)  /* something went terribly wrong */
     return FALSE;

   /*** Get disc information to learn the disc state (blank, finalized etc.) */

   ab = CreateAlignedBuffer(4096);
   buf = ab->buf;
   memset(cmd, 0, MAX_CDB_SIZE);
   cmd[0] = 0x51;     /* READ DISC INFORMATION */
   cmd[1] = 0;        /* standard disc info */
   cmd[7] = 0;        /* Allocation length */
   cmd[8] = MIN_TRANSFER_LEN;

   Verbose("# trying READ DISC INFORMATION for size\n");
   if(SendPacket(dh, cmd, 10, buf, MIN_TRANSFER_LEN, sense, DATA_READ) == 0)
   {  length = buf[0]<<8 | buf[1];
      length_align(&length);

      Verbose("# size returned is %d\n", length);

      memset(cmd, 0, MAX_CDB_SIZE);
      cmd[0] = 0x51;         /* READ DISC INFORMATION */
      cmd[1] = 0;            /* standard disc info */
      cmd[7] = length>>8;    /* Allocation length */
      cmd[8] = length&0xff;

      Verbose("# trying READ DISC INFORMATION for real info\n");
      if(SendPacket(dh, cmd, 10, buf, length, sense, DATA_READ) == 0)
      {  dh->discStatus = buf[2];
	 status = (dh->discStatus>>2)&3;
	 if(status == 1 || status == 2) /* incomplete or damaged */
	    dh->incomplete = TRUE;
	 if(dh->mainType == CD)
	    switch(buf[8])
	    {  case    0: dh->subType = DATA1; break;
	       case 0x20: dh->subType  = XA21; break; 
	    }

	 if(Closure->verbose)
	   HexDump(buf, length, 16);
 	 Verbose("# status is %x%s disc type %x\n", 
		 dh->discStatus, dh->incomplete ? " (incomplete!),":",", buf[8]);
      }
      else Verbose("# READ DISC INFORMATION failed\n");
   }
   else Verbose("# Getting size of READ DISC INFORMATION failed\n");
   FreeAlignedBuffer(ab);

   /*** Some defaults */

   strcpy(dh->manuID, "-");
   dh->bookDescr = "-";
   
   /*** Main type has been decided now */

   switch(dh->mainType)
   {  case CD:
        dh->read        = read_cd_sector;
	dh->readRaw     = read_raw_cd_sector;
	dh->singleRate  = 150.0;
	dh->maxRate     = 52;
	dh->clusterSize = 16;  /* really 1, but this is faster */ 
	return dh->incomplete ? query_incomplete(dh, probe_only) : query_cd(dh, probe_only);

      case DVD:
	dh->read        = read_dvd_sector;
	dh->singleRate  = 1352.54;
	dh->maxRate     = 17;
	dh->clusterSize = 16;
	if(!dh->incomplete) return query_dvd(dh, probe_only);
	else
	{  if(query_dvd(dh, TRUE)) return TRUE;
	   return query_incomplete(dh, probe_only);
	}
	break;

      case BD:
	dh->read        = read_dvd_sector;
	dh->singleRate  = 36000.0/8.0;  /* 1x = 36 kbit */
	dh->maxRate     = 9;
	dh->clusterSize = 32;
	if(!dh->incomplete) return query_bd(dh, probe_only);
	else
	{  if(query_bd(dh, TRUE)) return TRUE;
	   return query_incomplete(dh, probe_only);
	}
	break;

      default:  /* maybe HD DVD or sth else we do not support */
	return FALSE;
   }

   return FALSE; /* unreachable */
}

/*
 * Find out whether the disc is blank,
 * and the blank disc capacity.
 */

static int query_blank(DeviceHandle *dh)
{  AlignedBuffer *ab = CreateAlignedBuffer(4096);
   unsigned char *buf = ab->buf;
   unsigned char cmd[MAX_CDB_SIZE];
   Sense *sense = &dh->sense;
   unsigned int length;

   Verbose("# *** QueryBlank(%s) ***\n", dh->devinfo);

   /*** Determine blank size for CD media ***/

   if(dh->mainType == CD)
   {  memset(cmd, 0, MAX_CDB_SIZE);
      cmd[0] = 0x43;  /* READ TOC/PMA/ATIP */
      cmd[1] = 0x02;  /* TIME bit required for this format */
      cmd[2] = 4;     /* format; we want the ATIP */
      cmd[6] = 0;     /* track/session number */
      cmd[7] = 0;     /* allocation length */
      cmd[8] = MIN_TRANSFER_LEN;

      Verbose("#CD: querying size of READ TOC/PMA/ATIP (for ATIP)\n");
      if(SendPacket(dh, cmd, 10, buf, MIN_TRANSFER_LEN, sense, DATA_READ)<0)
      {  FreeAlignedBuffer(ab);
	 return FALSE;
      }

      length = buf[0]<<8 | buf[1];
      length += 2;    /* MMC3: "Disc information length excludes itself" */
      length_align(&length);
      Verbose("#CD: size returned is %d\n", length);

      if(length < 15 || length > 1024)  /* implausible */
      {  FreeAlignedBuffer(ab);
	 return FALSE;
      }
      
      memset(cmd, 0, MAX_CDB_SIZE);
      cmd[0] = 0x43;  /* READ TOC/PMA/ATIP */
      cmd[1] = 0x02;  /* TIME bit required for this format */
      cmd[2] = 4;     /* format; we want the full ATIP */
      cmd[6] = 0;     /* track/session number */
      cmd[7] = (length>>8) & 0xff; /* allocation length */
      cmd[8] = length & 0xff;

      Verbose("#CD: querying real READ TOC/PMA/ATIP (for ATIP)\n");
      if(SendPacket(dh, cmd, 10, buf, length, sense, DATA_READ)<0)
      {  FreeAlignedBuffer(ab);
	 return FALSE;
      }

      if(Closure->verbose)
	 HexDump(buf, length, 16);

      Verbose("#CD: Lead-in @ %d:%d.%d; last possible lead-out %d:%d.%d\n",
	      buf[8], buf[9], buf[10], buf[12], buf[13], buf[14]);

      /* frame + 75 * (second - 2 + 60 * minute) */
      dh->blankCapacity = buf[14] + 75 * (buf[13] - 2 + 60 * buf[12]);
   }

   /*** Determine blank size for DVD media ***/

   if(dh->mainType == DVD)
   {  switch(dh->subType)
      {  case DVD_DASH_R:
	 case DVD_PLUS_R:
	 case DVD_DASH_R_DL:
	 case DVD_PLUS_R_DL:
	    memset(cmd, 0, MAX_CDB_SIZE);
	    cmd[0] = 0x52;  /* READ TRACK INFORMATION */
	    cmd[1] = 0x01;  /* TCDB (track number) addressing) */
	    cmd[5] = 1;     /* we want the first track info */
	    cmd[7] = 0;     /* allocation length */
	    cmd[8] = MIN_TRANSFER_LEN;

	    Verbose("#DVD: querying size of READ TRACK INFORMATION\n");
	    if(SendPacket(dh, cmd, 10, buf, MIN_TRANSFER_LEN, sense, DATA_READ)<0)
	    {  FreeAlignedBuffer(ab);
	       return FALSE;
	    }

	    length = buf[0]<<8 | buf[1];
	    length += 2;    /* MMC3: "Disc information length excludes itself" */
	    length_align(&length);
	    Verbose("#DVD: size returned is %d\n", length);

	    memset(cmd, 0, MAX_CDB_SIZE);
	    cmd[0] = 0x52;  /* READ TRACK INFORMATION */
	    cmd[1] = 0x01;  /* TCDB (track number) addressing) */
	    cmd[5] = 1;     /* we want the first track info */
	    cmd[7] = length>>8;  /* allocation length */
	    cmd[8] = length&0xff;

	    Verbose("#DVD: querying size of READ TRACK INFORMATION\n");
	    if(SendPacket(dh, cmd, 10, buf, length, sense, DATA_READ)<0)
	    {  FreeAlignedBuffer(ab);
	       return FALSE;
	    }

	    if(Closure->verbose)
	       HexDump(buf, length, 16);

	    Verbose("#DVD: free: %d\n", buf[16]<<24|buf[17]<<16|buf[18]<<8|buf[19]);
	    Verbose("#DVD: size: %d\n", buf[24]<<24|buf[25]<<16|buf[26]<<8|buf[27]);

	    dh->blankCapacity = buf[16]<<24|buf[17]<<16|buf[18]<<8|buf[19];
	    break;

	 case DVD_DASH_RW:
	 case DVD_PLUS_RW:
	 case DVD_DASH_RW_DL:
	 case DVD_PLUS_RW_DL:
	 case DVD_RAM:
         {  int cmc_size;
	    int n_lists;
	    int i,idx;

	    memset(cmd, 0, MAX_CDB_SIZE);
	    cmd[0] = 0x23;  /* READ FORMAT CAPACITIES */
	    cmd[7] = 0;     /* allocation length */
	    cmd[8] = 4;

	    Verbose("#DVD: querying size of READ FORMAT CAPACITIES\n");
	    if(SendPacket(dh, cmd, 10, buf, 4, sense, DATA_READ)<0)
	    {  FreeAlignedBuffer(ab);
	       return FALSE;
	    }

	    length = 4+buf[3];
	    length_align(&length);
	    Verbose("#DVD: size returned is %d\n", length);

	    memset(cmd, 0, MAX_CDB_SIZE);
	    cmd[0] = 0x23;        /* READ FORMAT CAPACITIES */
	    cmd[7] = length>>8;   /* allocation length */
	    cmd[8] = length&0xff;

	    Verbose("#DVD: real query of READ FORMAT CAPACITIES\n");
	    if(SendPacket(dh, cmd, 10, buf, length, sense, DATA_READ)<0)
	    {  FreeAlignedBuffer(ab);
	       return FALSE;
	    }

	    if(Closure->verbose)
	       HexDump(buf, length, 16);

	    cmc_size = buf[4]<<24 | buf[5]<<16 | buf[6]<<8 | buf[7];

	    n_lists = buf[3] / 8;
	    n_lists--;

	    Verbose("#DVD: Current/Maximum capacity: %d blocks\n", cmc_size);

	    switch(buf[8] & 3)
	    {  case 1: /* unformatted */
		  Verbose("#DVD: unformatted\n");
		  break;
	       case 2: /* formatted */
		  Verbose("#DVD: formatted\n");
		  break;
	       case 3: /* no medium */
		  Verbose("#DVD: %d no medium\n", buf[8]);
		  break;
	       default: /* b0rked */
		  Verbose("#DVD: b0rked\n");
		  break;
	    }

	    if(!n_lists) 
	    {  FreeAlignedBuffer(ab);
	       return FALSE;
	    }

	    /* Now go through all capacity lists */

	    for(i=0, idx=12; i<n_lists; i++, idx+=8)
	    {  gint64 size;
	       
	       size = (gint64)(buf[idx]<<24 | buf[idx+1]<<16 | buf[idx+2]<<8 | buf[idx+3]);
	       Verbose("#DVD: Cap list %d - type %x, size %lld\n", i, buf[idx+4]>>2, size);
	       
	       switch(buf[idx+4]>>2)  /* format type */
	       {  case 0x00:  /* all media */
		  case 0x10:  /* blank CD-RW capacity */
		  case 0x26:  /* blank DVD+RW capacity */
		     FreeAlignedBuffer(ab);
		     dh->blankCapacity = size;
		     return TRUE;
	       }
	    }
	 }
	    break;
      }
   }

   /*** Determine blank size for BD media ***/

   if(dh->mainType == BD)
   {  unsigned int size,sa_size;
      int n_lists, i, idx;

      memset(cmd, 0, MAX_CDB_SIZE);
      cmd[0] = 0x23;  /* READ FORMAT CAPACITIES */
      cmd[7] = 0;     /* allocation length */
      cmd[8] = 4;

      Verbose("#BD: querying size of READ FORMAT CAPACITIES\n");
      if(SendPacket(dh, cmd, 10, buf, 4, sense, DATA_READ)<0)
      {  FreeAlignedBuffer(ab);
	 return FALSE;
      }

      length = 4+buf[3];
      length_align(&length);
      Verbose("#BD: size returned is %d\n", length);

      memset(cmd, 0, MAX_CDB_SIZE);
      cmd[0] = 0x23;        /* READ FORMAT CAPACITIES */
      cmd[7] = length>>8;   /* allocation length */
      cmd[8] = length&0xff;

      Verbose("#BD: real query of READ FORMAT CAPACITIES\n");
      if(SendPacket(dh, cmd, 10, buf, length, sense, DATA_READ)<0)
      {  FreeAlignedBuffer(ab);
	 return FALSE;
      }

      if(Closure->verbose)
	 HexDump(buf, length, 16);

      size = (buf[4]<<24 | buf[5]<<16 | buf[6]<<8 | buf[7]);
      sa_size = buf[9]<<16 | buf[10]<<8 | buf[11];
      Verbose("#BD: Current/Maximum capacity: %d blocks, %d spare area\n", size, sa_size);

      n_lists = buf[3] / 8;
      n_lists--;

      if(!n_lists) 
      {  FreeAlignedBuffer(ab);
	 return FALSE;
      }

      /* Now go through all capacity lists */

      for(i=0, idx=12; i<n_lists; i++, idx+=8)
      {  gint64 size;
	       
	 size = (gint64)(buf[idx]<<24 | buf[idx+1]<<16 | buf[idx+2]<<8 | buf[idx+3]);
	 sa_size = buf[idx+5]<<16 | buf[idx+6]<<8 | buf[idx+7];
	 Verbose("#BD: Cap list %d - type %x, size %lld, spare %d\n", 
		 i, buf[idx+4]>>2, size, sa_size);
	       
	 switch(buf[idx+4]>>2)  /* format type */
	 {  case 0x00:  /* all media */
	       //     FreeAlignedBuffer(ab);
	       dh->blankCapacity = size;
	       //	       return TRUE;
	 }
      }
   }

   FreeAlignedBuffer(ab);
   return TRUE;
}

/*
 * Find out whether the drive can do C2 scans.
 * Done by empirically trying a READ CD;
 * maybe we should examine the drive feature codes instead. 
 */

static void try_c2_scan(DeviceHandle *dh)
{  Sense *sense = &dh->sense;
   unsigned char cdb[MAX_CDB_SIZE];
   RawBuffer *rb;
   int lba = 0;
   int ret;
 
   dh->canC2Scan = FALSE;
  
   if(!(rb=dh->rawBuffer))  /* sanity check: No C2 scans without RAW reading */
      return;
   
   memset(cdb, 0, MAX_CDB_SIZE);
   cdb[0]  = 0xbe;         /* READ CD */

   switch(dh->subType)     /* Expected sector type */
   {  case DATA1:          /* data mode 1 */ 
        cdb[1] = 2<<2; 
	cdb[9] = 0xba;    /* we want Sync + Header + User data + EDC/ECC + C2 */
	break;  

      case XA21:           /* xa mode 2 form 1 */
	cdb[1] = 4<<2; 
	cdb[9] = 0xfc;
	break;  
   }

   cdb[2]  = (lba >> 24) & 0xff;
   cdb[3]  = (lba >> 16) & 0xff;
   cdb[4]  = (lba >>  8) & 0xff;
   cdb[5]  = lba & 0xff;
   cdb[6]  = 0;        /* number of sectors to read (3 bytes) */
   cdb[7]  = 0;  
   cdb[8]  = 1;        /* read 1 sector */

   cdb[10] = 0;        /* reserved stuff */
   cdb[11] = 0;        /* no special wishes for the control byte */

   ret = SendPacket(dh, cdb, 12, rb->workBuf->buf, CD_RAW_C2_SECTOR_SIZE, sense, DATA_READ);

   Verbose("C2 scan probing: %d (%s); Sense: %s\n",
	   ret, !ret ? "good" : "failed",
	   GetSenseString(sense->sense_key, sense->asc, sense->ascq, 0));

   if(!ret || sense->sense_key != 5)  /* good status or error != illegal request means C2 works */
   {  rb->sampleSize = CD_RAW_C2_SECTOR_SIZE;
      dh->canC2Scan = TRUE;
      return;
   }
   else 
   {  rb->sampleSize = CD_RAW_SECTOR_SIZE; 
      dh->canC2Scan = FALSE;
      return;
   }
}

/*
 * Find out whether the drive can return raw sectors with
 * uncorrected read errors. Depending on override,
 * mode is or'ed with existing flags (override = FALSE),
 * or written over existing flags (override = TRUE). 
 */

/*
 * Read mode page 1 
 */

static int read_mode_page(DeviceHandle *dh, AlignedBuffer *ab, int *parameter_list_length, 
			  unsigned char *read_mode, unsigned char *read_retries)
{  Sense sense;
   unsigned char cdb[16];
   unsigned char *buf = ab->buf;
   int ret;

   memset(cdb, 0, MAX_CDB_SIZE);
   cdb[0] = 0x5a;         /* MODE SENSE(10) */
   cdb[2] = 1;            /* Page code */
   cdb[8] = 252;          /* Allocation length */
   ret  = SendPacket(dh, cdb, 10, buf, 252, &sense, DATA_READ);

   if(ret<0) 
   {  Verbose("\nRead mode page 01h failed: %d (%s); Sense: %s\n", 
	      ret, !ret ? "good" : "failed",
	      GetSenseString(sense.sense_key, sense.asc, sense.ascq, 0));
      return FALSE;
   }

   Verbose("\nMode page 01h:\n");
   Verbose("  mode data length = %d\n", buf[0]<<8 | buf[1]);
   Verbose("  block descriptor length = %d\n", buf[6]<<8 | buf[7]);
   Verbose("  page byte 0 = %2x\n", buf[8]);
   Verbose("  page byte 1 = %2x\n", buf[9]);
   Verbose("  page byte 2 = %2x\n", buf[10]);
   Verbose("  page byte 3 = %2x\n", buf[11]);

   *parameter_list_length = buf[1] + 2;  /* mode data length + 2 */
   *read_mode    = buf[10];
   *read_retries = buf[11];

   Verbose("  using mode data length %d\n", *parameter_list_length);
   return TRUE;
}

static int set_mode_page(DeviceHandle *dh, AlignedBuffer *ab, int parameter_list_length, 
			 unsigned char read_mode, unsigned char read_retries)
{  Sense sense;
   unsigned char cdb[16];
   int ret;

   memset(cdb, 0, MAX_CDB_SIZE);
   cdb[0]  = 0x55;         /* MODE SELECT(10) */
   cdb[1]  = 0x10;         /* set page format (PF) bit */
   cdb[7]  = 0;
   cdb[8]  = parameter_list_length;

   ab->buf[10] = read_mode;
   ab->buf[11] = read_retries;

   ret  = SendPacket(dh, cdb, 10, ab->buf, parameter_list_length, &sense, DATA_WRITE);

   if(ret<0) 
   {  Verbose("Setting mode page 01h to 0x%2x failed: %s\n", read_mode, 
	      GetSenseString(sense.sense_key, sense.asc, sense.ascq, 0));
      return FALSE;
   }

   return TRUE;
}

void SetRawMode(DeviceHandle *dh, int action)
{  AlignedBuffer *ab = CreateAlignedBuffer(2048);
   unsigned char new_read_mode, new_read_retries;
   unsigned char drive_read_mode, ignore;
   int pll;

   dh->canReadDefective = FALSE;

   /*** If MODE_PAGE_UNSET, apply the old settings */

   if(action == MODE_PAGE_UNSET)
   {  unsigned char ignore;

      if(!read_mode_page(dh, ab, &pll, &ignore, &ignore))
      {  FreeAlignedBuffer(ab);
	 return;
      }

      if(!set_mode_page(dh, ab, pll, dh->previousReadMode, dh->previousRetries))
      {  FreeAlignedBuffer(ab);
	 return;
      }

      if(!read_mode_page(dh, ab, &pll, &dh->currentReadMode, &ignore))
      {  FreeAlignedBuffer(ab);
	 return;
      }

      FreeAlignedBuffer(ab);
      return;
   }

   /*** Otherwise we have MODE_PAGE_SET. Set new raw reading mode */

   /* Remember current settings */

   if(!read_mode_page(dh, ab, &pll, &dh->previousReadMode, &dh->previousRetries))
   {  FreeAlignedBuffer(ab);
      return;
   }

   /* Try to set the mode page */

   new_read_mode = dh->previousReadMode | Closure->rawMode;
      
   if(Closure->internalAttempts >= 0)
        new_read_retries = Closure->internalAttempts;
   else new_read_retries = dh->previousRetries;

   Verbose("Trying read mode 0x%02x, %d read attempts.\n",
	   new_read_mode, new_read_retries);

   if(!set_mode_page(dh, ab, pll, new_read_mode, new_read_retries))
      goto reset_mode_page;
	 
   /* Check if drive accepted the change */

   if(!read_mode_page(dh, ab, &pll, &drive_read_mode, &ignore))
      goto reset_mode_page;

   if(drive_read_mode != new_read_mode)
   {  Verbose("Setting raw mode failed: %2x instead of %2x\n", 
		 drive_read_mode, new_read_mode);
      goto reset_mode_page;
   }

   dh->rawBuffer->sampleSize = CD_RAW_SECTOR_SIZE;
   dh->currentReadMode       = drive_read_mode;
   dh->canReadDefective      = TRUE;
   FreeAlignedBuffer(ab);
   return;

reset_mode_page:
   Verbose("Resetting mode page 01h.\n");
   //   set_mode_page(dh, ab, pll, dh->previousReadMode, dh->previousRetries);
   /* Using the new read retries wont hurt */
   set_mode_page(dh, ab, pll, dh->previousReadMode, new_read_retries);
   FreeAlignedBuffer(ab);
}

/*
 * Find out whether we are allowed to create an image from the DVD.
 */

static int query_copyright(DeviceHandle *dh)
{  Sense sense;
   AlignedBuffer *ab = CreateAlignedBuffer(2048);
   unsigned char *buf = ab->buf;
   unsigned char cmd[MAX_CDB_SIZE];
   unsigned char result;
   unsigned int length;
   
   /* The following test does neither apply nor is necessary
      for incomplete (damaged) discs. */ 

   if(dh->incomplete) return FALSE;

   /* Query length of returned data */

   memset(cmd, 0, MAX_CDB_SIZE);
   cmd[0] = 0xad;     /* READ DVD STRUCTURE */
   cmd[6] = 0;        /* First layer */
   cmd[7] = 1;        /* We want copyright info */
   cmd[8] = 0;        /* Allocation length */
   cmd[9] = MIN_TRANSFER_LEN;

   if(SendPacket(dh, cmd, 12, buf, MIN_TRANSFER_LEN, &sense, DATA_READ)<0)
   {  FreeAlignedBuffer(ab);
      Stop(_("%s\nCould not query dvd structure length for format code 1.\n"),
	   GetSenseString(sense.sense_key, sense.asc, sense.ascq, TRUE));
      return TRUE;
   }

   length = buf[0]<<8 | buf[1];
   length += 2;
   length_align(&length);

   if(length>4096) /* don't let the drive hack us using a buffer overflow ;-) */
   {  FreeAlignedBuffer(ab);
      Stop(_("Could not query dvd copyright info - implausible packet length %d\n"),length);
      return TRUE;
   }

   /* Do the real query */

   memset(cmd, 0, MAX_CDB_SIZE);
   cmd[0] = 0xad;     /* READ DVD STRUCTURE */
   cmd[6] = 0;        /* First layer */
   cmd[7] = 1;        /* We want copyright info */
   cmd[8] = (length>>8) & 0xff;  /* Allocation length */
   cmd[9] = length & 0xff;

   if(SendPacket(dh, cmd, 12, buf, length, &sense, DATA_READ)<0)
   {  FreeAlignedBuffer(ab);
      Stop(_("%s\nCould not query copyright info.\n"),
	   GetSenseString(sense.sense_key, sense.asc, sense.ascq, TRUE));
      return TRUE;
   }

   result = buf[4] & 0x11;
   FreeAlignedBuffer(ab);

   return result;
}

/*
 * See whether a sector lies within the user area.
 */

static int check_sector(DeviceHandle *dh, GString *msg_out, guint64 sector, int n_sectors)
{  AlignedBuffer *scratch = CreateAlignedBuffer(MAX_CLUSTER_SIZE);
   int status,result;
   char *msg;

   if(sector<2) return 4;

   status = read_dvd_sector(dh, scratch->buf, sector, n_sectors);
   FreeAlignedBuffer(scratch);

   if(!status)   /* Sector was readable */ 
   {  msg = _("readable");
      result = 0;
   }
   else          /* Read error */
   {  msg = GetLastSenseString(FALSE);

      if(   dh->sense.sense_key == 0x05   /* illegal request */
	 || dh->sense.sense_key == 0x00)  /* or None; happens at least on Pioneer drives */
      {  if(   dh->sense.asc  == 0x63     
            && dh->sense.ascq == 0x00)
	      result = 1;                 /* end of user area */ 
         else result = 2;                 /* other illegal request, */
                                          /* usually 0x21 Logical block address out of range*/
      }
      else result = 3;                    /* other read error */ 
   }

   if(n_sectors == 1)
        g_string_append_printf(msg_out, _("Sector %lld: %s\n"), sector, msg);
   else g_string_append_printf(msg_out, _("Sectors %lld-%lld: %s\n"), 
			       sector, sector+n_sectors-1, msg);

   return result;
}

/*
 * Try to find out if the given sectors really lie
 * at the end of the user area.
 */

static void evaluate_results(int res0, int res1, int *result, char **msg)
{
  if(res0 == 3 || res1 == 3) 
  {  *result = FALSE;
     *msg    = _("is undecideable because of read error");
     return;
  }

  if(res0 == 0 && (res1 == 1 || res1 == 2))
  {  *result = TRUE;
     *msg    = _("looks GOOD");
     return;
  }

  if(res0 == 1 && res1 == 2)
  {  *result = FALSE;
     *msg    = _("gives unformatted size (UNUSABLE)");
     return;
  }

  *result = FALSE;
  *msg    = _("is UNUSABLE");
}

/*
 * Do a READ CAPACITY.
 * Needs to be separated as we need this value to invoke the RS02/RS03
 * size heuristics which in turn must be executed before calling query_size().
 */

static void read_capacity(Image *image)
{  DeviceHandle *dh = image->dh;
   Sense sense;
   AlignedBuffer *ab = CreateAlignedBuffer(2048);
   unsigned char *buf = ab->buf; 
   unsigned char cmd[MAX_CDB_SIZE];
   int implausible = FALSE;

   /*** Query size by doing READ CAPACITY */

   Verbose("# *** read_capacity(%s) ***\n", dh->devinfo);

   memset(cmd, 0, MAX_CDB_SIZE);
   cmd[0] = 0x25;  /* READ CAPACITY */

   if(SendPacket(dh, cmd, 10, buf, 8, &sense, DATA_READ)<0)
   {  FreeAlignedBuffer(ab);
      Stop(_("%s\nCould not query medium size.\n"),
	   GetSenseString(sense.sense_key, sense.asc, sense.ascq, TRUE));
      dh->readCapacity = 0;
      return;
   }

   dh->readCapacity = (gint64)(buf[0]<<24 | buf[1]<<16 | buf[2]<<8 | buf[3]);
   Verbose(" -> %lld\n", dh->readCapacity);
   FreeAlignedBuffer(ab);

   /*** Validate capacity */

   if(dh->mainType == CD && dh->readCapacity > MAX_CDR_SIZE)
      implausible = TRUE;

   if(dh->mainType == DVD && dh->layers == 1 && dh->readCapacity > MAX_DVD_SL_SIZE)
      implausible = TRUE;

   if(dh->mainType == DVD && dh->layers == 2 && dh->readCapacity > MAX_DVD_DL_SIZE)
      implausible = TRUE;

   if(implausible && !dh->simImage)
   {  LogWarning(_("READ CAPACITY: implausible medium size, %lld sectors\n"), 
		 (gint64)dh->readCapacity);
      dh->readCapacity = 0;
   }
}

/*
 * Query the medium size.
 */

static gint64 query_size(Image *image)
{  DeviceHandle *dh = image->dh;
  
   Verbose("# *** query_size(%s) ***\n", dh->devinfo);

   /*** If an EccHeader was already found and the codec specified
	the overall image size, use that as an authoritative source 
	for the medium size. */

   if(image->expectedSectors > 0) 
   {  Verbose("Medium size obtained from ECC header: %lld sectors\n", image->expectedSectors);  
      return image->expectedSectors;
   }
   else Verbose("Medium size could NOT be determined from ECC header.\n");

   /*** Try getting the size from the ISO/UDF filesystem. */

   if(image->isoInfo)
   {  if(Closure->ignoreIsoSize)
      {  Verbose("Note: NOT examining ISO/UDF filesystem as requested by user option!\n");
      }
      else
      {  Verbose("Medium size obtained from ISO/UDF file system: %d sectors\n", 
		 image->isoInfo->volumeSize);  
	 return image->isoInfo->volumeSize;
      }
   }
   else Verbose("Medium size could NOT be determined from ISO/UDF filesystem.\n");

   /*** If everything else fails, query the drive. */
   
   /* For CD media, thats all we have to do */

   if(dh->mainType == CD)
   {  Verbose("CD medium - using size from READ CAPACITY: %lld sectors\n", dh->readCapacity+1);
      return dh->readCapacity+1;  /* size is the number of the last sector, starting with 0 */
   }

   /* BD drives have neither dh->userAreaSize nor will the following heuristics
      work as unformatted sectors can be always read. Stick with READ CAPACITY. */

   if(dh->mainType == BD)
   {  Verbose("BD medium - using size from READ CAPACITY: %lld sectors\n", dh->readCapacity+1);
      return dh->readCapacity+1;  /* size is the number of the last sector, starting with 0 */
   }

   /* For DVD media, READ CAPACITY should give the real image size.
      READ DVD STRUCTURE may be same value or the unformatted size.
      But some drives appear to have both functions reversed,
      so we have to be careful here. */

   if(dh->readCapacity == dh->userAreaSize)  /* If they are equal just return one */
   {  Verbose("READ CAPACITY and READ DVD STRUCTURE agree: %lld sectors\n", dh->readCapacity+1);
      return dh->readCapacity+1;  
   }
   else                                   /* Tricky case. Try some heuristics. */
   {  int last_valid, first_out;
      gint test_sector;
      int cap_result,strct_result,decision;
      char *cap_msg, *strct_msg, *decision_msg;
      GString *warning;

      /*** Try to find out the correct value empirically. */

      warning = g_string_sized_new(1024);
      g_string_printf(warning,
		      _("Different media sizes depending on query method:\n"
			"READ CAPACITY:      %lld sectors\n"
			"READ DVD STRUCTURE: %lld sectors\n\n"),
		      dh->readCapacity+1, dh->userAreaSize+1);

      g_string_append(warning, _("Evaluation of returned medium sizes:\n\n"));

      /*** Look at READ CAPACITY results */

      test_sector = dh->readCapacity;
      last_valid = check_sector(dh, warning, test_sector,   1);
      first_out  = check_sector(dh, warning, test_sector+1, 1);

      /*** Some drives can read about 16 or 32 sectors past the image
           into the border/lead out. Drats. */

      if(last_valid == 0 && first_out == 0)  
      {  test_sector &= ~((long long)15);
       	 first_out  = check_sector(dh, warning, test_sector, 16);
       	 if(!first_out)
	   first_out = check_sector(dh, warning, test_sector+16, 16);
       	 if(!first_out)
	   first_out = check_sector(dh, warning, test_sector+32, 16);
       	 if(!first_out)
	   first_out = check_sector(dh, warning, test_sector+48, 16);
      }

      evaluate_results(last_valid, first_out, &cap_result, &cap_msg);
      g_string_append_printf(warning, "-> READ CAPACITY %s\n\n", cap_msg);

      /*** Look at READ DVD STRUCTURE results */

      test_sector = dh->userAreaSize;
      last_valid = check_sector(dh, warning, test_sector,   1);
      first_out  = check_sector(dh, warning, test_sector+1, 1);
      
      /*** Some drives can read about 16 or 32 sectors past the image
           into the border/lead out. Drats. */

      if(last_valid == 0 && first_out == 0)  
      {  test_sector &= ~((long long)15);
       	 first_out  = check_sector(dh, warning, test_sector, 16);
       	 if(!first_out)
	   first_out = check_sector(dh, warning, test_sector+16, 16);
       	 if(!first_out)
	   first_out = check_sector(dh, warning, test_sector+32, 16);
       	 if(!first_out)
	   first_out = check_sector(dh, warning, test_sector+48, 16);
      }

      evaluate_results(last_valid, first_out, &strct_result, &strct_msg);
      g_string_append_printf(warning, "-> READ DVD STRUCTURE %s\n\n", strct_msg);

      /*** Decide what to use */

      decision = 0;
      decision_msg = NULL;

      if(cap_result) 
      {  decision = 1; 
	 decision_msg = _("Using value from READ CAPACITY");
      }
      if(strct_result) 
      {  decision = 2; 
	 decision_msg = _("Using value from READ DVD STRUCTURE");
      }

      if(!decision)
      {  if(!dh->readCapacity) decision = 2;
         if(!dh->userAreaSize) decision = 1;

	 if(dh->readCapacity && dh->userAreaSize)
	   decision = dh->readCapacity < dh->userAreaSize ? 1 : 2;

	 decision_msg = _("FAILED to determine image size.\n"
			  "Using smaller value as this is right on >90%% of all drives,\n"
			  "but CONTINUE AT YOUR OWN RISK (the image may be incomplete/unusable)");
      }

      g_string_append_printf(warning, _("Final decision: %s\n\n"), decision_msg);
      LogWarning(warning->str);

      g_string_free(warning, TRUE);

      switch(decision)
      {  case 1  : return dh->readCapacity+1;
         case 2  : return dh->userAreaSize+1;
         default : Stop(_("Failed to determine image size.\n"
			  "Try using a different drive."));
	           return 0;
      }
   }

   return 0;
}

/*
 * Return size of current drive and medium (if any)
 */

gint64 CurrentMediumSize(int get_blank_size)
{  Image *image;
   gint64 size;

#ifdef SYS_UNKNOWN
   return 0;
#endif

   image = OpenImageFromDevice(Closure->device);
   if(!image) return 0;
   if(InquireDevice(image->dh, 1) != 0x05) 
   {  CloseImage(image);
      return 0;
   }

   /* Avoid size query on unknown media */

   if(!query_type(image->dh, 1) && !get_blank_size)
   {  CloseImage(image);
      return 0;
   }

   /* A size query on unsupported media would derail */

   if((image->dh->subType == UNSUPPORTED) && !get_blank_size)
   {  CloseImage(image);
      return 0;
   }

   /* We can return either the image size or the size of  blank media. */

   if(get_blank_size)
   {  if(!query_blank(image->dh))
      {  CloseImage(image);
	 return 0;
      }

      size = image->dh->blankCapacity;
   }
   else
   {  read_capacity(image);
      size = query_size(image);
   }

   CloseImage(image);

   return size;
}

/***
 *** Optional drive settings
 ***/

/*
 * Spin up drive.
 * Most drives give a *beep* about sending the START STOP CDB,
 * so we simply nudge them with reading request until the spin up
 * time is over. Pathetic ;-)
 */

void SpinupDevice(DeviceHandle *dh)
{  AlignedBuffer *ab;
   GTimer *timer;
   gint64 s;

   if(!Closure->spinupDelay)
      return;

   PrintCLI(_("Waiting %d seconds for drive to spin up...\n"), Closure->spinupDelay);
   
   ab = CreateAlignedBuffer(MAX_CLUSTER_SIZE);

   timer = g_timer_new();
   g_timer_start(timer);

   for(s=0; ;s+=dh->clusterSize)
   {  int status;
      double elapsed;
      gulong ignore;

      if(s>=dh->sectors) return;
 
      status = ReadSectorsFast(dh, ab->buf, s, dh->clusterSize);
      if(status) return;

      elapsed = g_timer_elapsed(timer, &ignore);
      if(elapsed > Closure->spinupDelay)
	break;
   }

   FreeAlignedBuffer(ab);
   g_timer_destroy(timer);
}

/*
 * Load/eject the medium
 */

void LoadMedium(DeviceHandle *dh, int load)
{  Sense sense;
   unsigned char cmd[MAX_CDB_SIZE];

   /* Try to load or eject the medium */

   memset(cmd, 0, MAX_CDB_SIZE);
   cmd[0] = 0x1b;                /* START STOP */
   cmd[4] = load ? 0x03 : 0x02;  /* LOEJ=1; START=load/eject */

   if(SendPacket(dh, cmd, 6, NULL, 0, &sense, DATA_NONE)<0
      && (sense.asc != 0x53 || sense.ascq != 0x02))
   {
      PrintLog(_("%s\nCould not load/unload the medium.\n"), 
		 GetSenseString(sense.sense_key, sense.asc, sense.ascq, TRUE));
      return;
   }

   if(load) return;

   /* Some drives lock the tray when it was closed via START STOP.
      That sucks especially under newer GNU/Linux kernels as we need to
      be root to unlock it. Try anyways; maybe this changes in
      future kernels. */

   memset(cmd, 0, MAX_CDB_SIZE);
   cmd[0] = 0x1e;                /* PREVENT ALLOW MEDIUM REMOVAL */

   if(SendPacket(dh, cmd, 6, NULL, 0, &sense, DATA_NONE)<0)
      PrintLog(_("%s\nCould not unlock the medium.\n"), 
	       GetSenseString(sense.sense_key, sense.asc, sense.ascq, TRUE));

   /* Try ejecting again */

   memset(cmd, 0, MAX_CDB_SIZE);
   cmd[0] = 0x1b;                /* START STOP */
   cmd[4] = 0x02;                /* LOEJ=1; START=eject */

   if(SendPacket(dh, cmd, 6, NULL, 0, &sense, DATA_NONE)<0)
      PrintLog(_("%s\nCould not load/unload the medium.\n"), 
	       GetSenseString(sense.sense_key, sense.asc, sense.ascq, TRUE));
}

/*
 * Wait for the drive to become ready
 */

int TestUnitReady(DeviceHandle *dh)
{  unsigned char cmd[MAX_CDB_SIZE];
   int i;

   /*** Send TEST UNIT READY, return if drive says go */

   memset(cmd, 0, MAX_CDB_SIZE);
   cmd[0] = 0x00;     /* TEST UNIT READY */

   if(SendPacket(dh, cmd, 6, NULL, 0, &dh->sense, DATA_NONE) != -1)
      return TRUE;

   /*** If no medium present, try closing the tray. */

   if(   dh->sense.sense_key == 2  /* Not Ready */
      && dh->sense.asc == 0x3a)    /* Medium not present */
      LoadMedium(dh, TRUE);

   /*** Wait 10 seconds for drives reporting that they are
        becoming ready */

   for(i=0; i<10; i++)
   {  int continue_waiting = FALSE;

      memset(cmd, 0, MAX_CDB_SIZE);
      cmd[0] = 0x00;     /* TEST UNIT READY */

      if(SendPacket(dh, cmd, 6, NULL, 0, &dh->sense, DATA_NONE) != -1)
      {  if(Closure->guiMode)
	    SetLabelText(Closure->status, "");
	 return TRUE;
      }

      if(dh->sense.sense_key == 2)  /* Not Ready */
	 if(   (dh->sense.asc == 0x04 && dh->sense.ascq == 0x01) /* but in process of becoming so */
	     || dh->sense.asc == 0x3a    /* flawed firmware workaround */
	   )
	    continue_waiting = TRUE;

      if(dh->sense.sense_key == 6) /* Unit attention */
	 if(dh->sense.asc == 0x28 && dh->sense.ascq == 0x00)
	    continue_waiting = TRUE;

      if(continue_waiting)
      {  PrintCLIorLabel(Closure->status,
			 _("Waiting 10 seconds for drive: %d\n"),9-i);

	 if(Closure->stopActions)
	    return FALSE;

	 g_usleep(G_USEC_PER_SEC);
	 continue;
      }

      Verbose("fell out waiting for drive %x %x %x\n",dh->sense.sense_key,dh->sense.asc,dh->sense.ascq);
      break;  /* Something is wrong with the drive */
   }

   if(Closure->guiMode)
      SetLabelText(Closure->status, "");

   return FALSE;
}

/*
 * Set the reading speed. Does not work reliably yet. 
 */

/***
 *** Sector reading routines
 ***/

/*
 * Sector reading using the packet interface.
 */

static int read_dvd_sector(DeviceHandle *dh, unsigned char *buf, int lba, int nsectors)
{  Sense *sense = &dh->sense;
   unsigned char cmd[MAX_CDB_SIZE];
   int ret;

   memset(cmd, 0, MAX_CDB_SIZE);
   cmd[0] = 0x28;  /* READ(10) */
   cmd[1] = 0;  /* no special flags */
   cmd[2] = (lba >> 24) & 0xff;
   cmd[3] = (lba >> 16) & 0xff;
   cmd[4] = (lba >>  8) & 0xff;
   cmd[5] = lba & 0xff;
   cmd[6] = 0;         /* reserved */
   cmd[7] = 0;         /* number of sectors */
   cmd[8] = nsectors;  /* read 1 sector */

   ret = SendPacket(dh, cmd, 10, buf, 2048*nsectors, sense, DATA_READ);

   if(ret<0) RememberSense(sense->sense_key, sense->asc, sense->ascq);

   return ret;
}

static int read_cd_sector(DeviceHandle *dh, unsigned char *buf, int lba, int nsectors)
{  Sense *sense = &dh->sense;
   unsigned char cmd[MAX_CDB_SIZE];
   int ret;

   memset(cmd, 0, MAX_CDB_SIZE);
   cmd[0]  = 0xbe;         /* READ CD */
   switch(dh->subType)
   {  case DATA1: cmd[1] = 2<<2; break;  /* data mode 1 */
      case XA21:  cmd[1] = 4<<2; break;  /* xa mode 2 form 1 */
   }
   cmd[2]  = (lba >> 24) & 0xff;
   cmd[3]  = (lba >> 16) & 0xff;
   cmd[4]  = (lba >>  8) & 0xff;
   cmd[5]  = lba & 0xff;
   cmd[6]  = 0;        /* number of sectors to read (3 bytes) */
   cmd[7]  = 0;  
   cmd[8]  = nsectors; /* read nsectors */

   cmd[9]  = 0x10;  /* we want the user data only */
   cmd[10] = 0;    /* reserved stuff */
   cmd[11] = 0;    /* no special wishes for the control byte */

   ret = SendPacket(dh, cmd, 12, buf, 2048*nsectors, sense, DATA_READ);

#if 0
#define BORK 34999
   if(lba<=BORK && BORK<lba+nsectors)  /* fixme */
     {  int offset = 2048*(BORK-lba);
      buf[offset+240]^=255;
  }
#endif

#if 0
#define BORK2 300
   if(lba<=BORK2 && BORK2<lba+nsectors)  /* fixme */
     {  int offset = 2048*(BORK2-lba);
      buf[offset+240]^=255;
  }
#endif

   if(ret<0) RememberSense(sense->sense_key, sense->asc, sense->ascq);
   return ret;
}

static int read_raw_cd_sector(DeviceHandle *dh, unsigned char *outbuf, int lba, int nsectors)
{  Sense *sense = &dh->sense;
   unsigned char cdb[MAX_CDB_SIZE];
   unsigned char c2bit;
   RawBuffer *rb;
   int ret = -1;
   int i,s;
   int offset = 16;

   /* Sanity checks */ 

   if(!dh->rawBuffer)
   {  RememberSense(3, 255, 4);  /* RAW buffer not allocated */
      return -1;
   }

   if(nsectors > dh->clusterSize)
   {  RememberSense(3, 255, 3); /* RAW reading > cluster size not supported */
      return -1;
   }

   rb = dh->rawBuffer;
   rb->lba = lba;

   /*** Perform the raw read */

   c2bit = dh->canC2Scan ? 0x02 : 0;
   memset(cdb, 0, MAX_CDB_SIZE);
   cdb[0]  = 0xbe;         /* READ CD */
   switch(dh->subType)     /* Expected sector type */
   {  case DATA1:          /* data mode 1 */ 
        cdb[1] = 2<<2; 
	cdb[9] = 0xb8 | c2bit; /* we want Sync + Header + User data + EDC/ECC + C2 */
	offset=16;
	break;  

      case XA21:           /* xa mode 2 form 1 */
	cdb[1] = 4<<2; 
	cdb[9] = 0xf8 | c2bit;
	offset=24;
	break;  
   }

   cdb[2]  = (lba >> 24) & 0xff;
   cdb[3]  = (lba >> 16) & 0xff;
   cdb[4]  = (lba >>  8) & 0xff;
   cdb[5]  = lba & 0xff;
   cdb[6]  = 0;        /* number of sectors to read (3 bytes) */
   cdb[7]  = 0;  
   cdb[8]  = nsectors; /* read nsectors */

   cdb[10] = 0;        /* reserved stuff */
   cdb[11] = 0;        /* no special wishes for the control byte */

   /* initialize sectors with our marker */

   for(i=0, s=0; i<nsectors; i++, s+=rb->sampleSize)
      CreateMissingSector(rb->workBuf->buf+s, lba+i, NULL, 0,
			  "read_raw_cd_sector() dummy sector");

   ret = SendPacket(dh, cdb, 12, rb->workBuf->buf, nsectors*rb->sampleSize, sense, DATA_READ);
   RememberSense(sense->sense_key, sense->asc, sense->ascq);

#if 0
   if(lba==16000)  /* fixme */
   {  rb->workBuf->buf[0*rb->sampleSize+230]^=255;
   }
#endif

   /* count c2 error bits */

   if(dh->canC2Scan)
   {  for(i=0, s=0; i<nsectors; i++, s+=rb->sampleSize)
	 dh->c2[i] = CountC2Errors(rb->workBuf->buf+s);
   }

   /* The drive screws up sometimes and returns a damaged sector as good. 
      When nsectors==1, this sector is still interesting for data recovery.
      We flag it as bad so that the next if() will pick it up correctly. */

   if(!ret && nsectors == 1 && !ValidateRawSector(rb, rb->workBuf->buf, "verify incoming"))
   {  ret = -1;
   }

   /*** See if defective sector reading is enabled and applicable. */

   if(ret<0 && dh->canReadDefective)
   {  
     /* Fail if multiple sectors were requested.
	Caller must retry reading single sectors. */

      if(nsectors != 1)
         return ret;

      return TryCDFrameRecovery(rb, outbuf);
   }

   /*** If we reach this, only the simple L-EC check is requested. */

   /* See if drive returned some data at all. */

   for(i=0, s=0; i<nsectors; i++, s+=rb->sampleSize)
      if(CheckForMissingSector(rb->workBuf->buf+s, lba+i, NULL, 0) != SECTOR_PRESENT)
      {  if(dh->canReadDefective)
	    RememberSense(3, 255, 0); /* report that nothing came back */
	 return -1;
      }

   /* Verify data and copy it back */

   ret = 0;
   rb->lba = lba;

   for(i=0, s=0; i<nsectors; i++, s+=rb->sampleSize)
   {  if(!ValidateRawSector(rb, rb->workBuf->buf+s, "verify outgoing"))
        return -1;

      memcpy(outbuf, rb->workBuf->buf+s+offset, 2048);
      outbuf += 2048;
      rb->lba++;
   }

   return ret;
}

/*
 * Sector reading through the device handle.
 * dh->read dispatches to one the routines above.
 */

int ReadSectors(DeviceHandle *dh, unsigned char *buf, gint64 s, int nsectors)
{  int retry,status = -1;
   int recommended_attempts = Closure->minReadAttempts;

   /* See if we are in a simulated defective area */ 

   if(dh->defects)
   {  gint64 i,idx;

     for(idx=s,i=0; i<nsectors; idx++,i++)
       if(GetBit(dh->defects, idx) && !((Random() & 15)))
       {  dh->sense.sense_key = 3;
	  dh->sense.asc       = 255;
	  dh->sense.ascq      = 255;
	  RememberSense(dh->sense.sense_key, dh->sense.asc, dh->sense.ascq);
	  return TRUE;
       }
   }

#if 0
   if(   (s == 331600 && nsectors > 16)
	 || s >331605)
   {  dh->sense.sense_key = 3;
      dh->sense.asc       = 255;
      dh->sense.ascq      = 255;
      RememberSense(dh->sense.sense_key, dh->sense.asc, dh->sense.ascq);
	  
      return TRUE;
   }
#endif

   /* Reset raw reading buffer (if there is one) */

   if(Closure->readRaw && dh->rawBuffer)
   {  ResetRawBuffer(dh->rawBuffer);
      dh->rawBuffer->recommendedAttempts = Closure->minReadAttempts;
   }

   /* Try normal read */

   for(retry=1; retry<=recommended_attempts; retry++)
   {  
      /* Dispatch between normal reader and raw reader */

      if(Closure->readRaw && dh->readRaw)
	   status = dh->readRaw(dh, buf, s, nsectors);
      else status = dh->read(dh, buf, s, nsectors);

      if(Closure->readRaw && dh->rawBuffer)
	recommended_attempts = dh->rawBuffer->recommendedAttempts;

      if(status)  /* current try was unsucessful */
      {  int last_key, last_asc, last_ascq;

	 if(Closure->stopActions)  /* user break */
	    return status;

	 /* Do not attempt multiple re-reads if nsectors > 1 and sectorSkip == 0
	    as these will be re-read with nsectors==1 anyways. */

//       Why only apply this shortcut to raw reading?
//	 if(dh->canReadDefective && nsectors > 1 && Closure->sectorSkip == 0)
	 if(nsectors > 1 && Closure->sectorSkip == 0)
	 {  PrintCLIorLabel(Closure->status,
			    _("Sectors %lld - %lld: %s\n"),
			    s, s+nsectors-1, GetLastSenseString(FALSE));
	    return status;
	 }

	 /* Print results of current attempt.
	    If the error was a wrong MSF in the sector, 
	    print info about the sector which was returned. */

	 GetLastSense(&last_key, &last_asc, &last_ascq);

	 if(last_key == 3 && last_asc == 255 && last_ascq == 2 && dh->rawBuffer)
	 {  unsigned char *frame = dh->rawBuffer->workBuf->buf;
	    PrintCLIorLabel(Closure->status,
			    _("Sector %lld, try %d: %s Sector returned: %d.\n"),
			    s, retry, GetLastSenseString(FALSE),
			    MSFtoLBA(frame[12], frame[13], frame[14]));
	 }
	 else
	    PrintCLIorLabel(Closure->status,
			    _("Sector %lld, try %d: %s\n"),
			    s, retry, GetLastSenseString(FALSE));

	 /* Last attempt; create failure notice */

	 if(dh->canReadDefective && retry >= recommended_attempts)
	    RememberSense(3, 255, 7);  /* Recovery failed */
      }
      else /* good return status */
      {  if(recommended_attempts > 1 && retry > 1)
	  PrintCLIorLabel(Closure->status,
			  _("Sector %lld, try %d: success\n"), s, retry);

         break;
      }
   }

   /* Update / use the defective sector cache */

   if(dh->canReadDefective && status && Closure->defectiveDump)
   {   
      if(SaveDefectiveSector(dh->rawBuffer,dh->canC2Scan))  /* any new sectors? */
      {  status = TryDefectiveSectorCache(dh->rawBuffer, buf);
	 if(status)
	    RememberSense(3, 255, 7);  /* Recovery failed */
      }
   }
   return status;
}

/*
 * Sector reading through the device handle.
 * dh->read dispatches to one the routines above.
 * No reading retries and/or raw reading are used.
 */

int ReadSectorsFast(DeviceHandle *dh, unsigned char *buf, gint64 s, int nsectors)
{  int status = -1;

   /* See if we are in a simulated defective area */ 

   if(dh->defects)
   {  gint64 i,idx;

     for(idx=s,i=0; i<nsectors; idx++,i++)
       if(GetBit(dh->defects, idx))
       {  dh->sense.sense_key = 3;
	  dh->sense.asc       = 255;
	  dh->sense.ascq      = 255;
	  RememberSense(dh->sense.sense_key, dh->sense.asc, dh->sense.ascq);
	  return TRUE;
       }
   }

   /* Try normal read */

   status = dh->read(dh, buf, s, nsectors);

   return status;
}

/*** 
 *** Open the device and query some of its properties.
 ***/

Image* OpenImageFromDevice(char *device)
{  Image *image = NULL;
   DeviceHandle *dh = NULL;

   /* Open the device. */

   Verbose("# *** OpenImageFromDevice(%s) ***\n", device);
   dh = OpenDevice(device);
   if(!dh) return NULL;

   InquireDevice(dh, 0);
   Verbose("# InquireDevice returned: %s\n", dh->devinfo);

   if(!TestUnitReady(dh))
   {  if(   dh->sense.sense_key == 2  /* Not Ready */
	 && dh->sense.asc == 0x3a)    /* Medium not present */
      {   CloseDevice(dh);
	  Stop(_("Device %s: no medium present\n"), device);
	  return NULL;
      }
      else 
      {  CloseDevice(dh);
	 Stop(_("Device %s does not become ready:\n%s\n\n"), device,
	      GetSenseString(dh->sense.sense_key, dh->sense.asc, dh->sense.ascq, FALSE));
	 return NULL;
      }
   }

   PrintLog(_("\nDevice: %s, %s\n"),device, dh->devinfo);

   /* Query the type and fail immediately if incompatible medium is found
      so that the later tests are not derailed by the wrong medium type */

   if(!query_type(dh, 0))
   {  CloseDevice(dh);
      Stop(_("Drive failed to report media type."));
      return NULL;
   }
     
   Verbose("# query_type() returned.\n");

   if(dh->subType == UNSUPPORTED)
   {  char *td = alloca(strlen(dh->typeDescr)+1);

      strcpy(td, dh->typeDescr);
      CloseDevice(dh);
      Stop(_("This software does not support \"%s\" type media."), td);
      return NULL;
   }

   if(dh->sessions>1)
   {  int sessions = dh->sessions;

      CloseDevice(dh);
      Stop(_("This software does not support multisession (%d sessions) media."), sessions);
      return NULL;
   }

   /* Create the Image structure.
      From here we need it to store additional information about the image. */

   image = g_malloc0(sizeof(Image));
   image->type = IMAGE_MEDIUM;
   image->fpSector = -1;
   image->dh   = dh;

   /* Activate raw reading features if possible,
      output selected reading mode */

   Verbose("# deciding reading strategy...\n");
   switch(dh->mainType)
   {  case CD:
	if(Closure->readRaw)
	{  dh->rawBuffer = CreateRawBuffer(MAX_RAW_TRANSFER_SIZE);

	   dh->rawBuffer->validFP = GetImageFingerprint(image, dh->rawBuffer->mediumFP, FINGERPRINT_SECTOR);
	 
	   if(dh->subType == XA21)
	   {  dh->rawBuffer->dataOffset = 24;
	      dh->rawBuffer->xaMode = TRUE;
	   }

	   SetRawMode(dh, MODE_PAGE_SET);
	   try_c2_scan(dh);

	   PrintLog(_("Using READ CD"));
	   PrintLog(_(", RAW reading"));

	   if(dh->canReadDefective)
	      PrintLog(_(", Mode page 1 ERP = %02xh"), dh->currentReadMode);

	   if(dh->canC2Scan)
	      PrintLog(_(", C2 scanning"));

	} else PrintLog(_("Using READ CD"));
	PrintLog(".\n");
        break;

      default:
	PrintLog(_("Using READ(10).\n"));
	break;
   }

   /* Examine medium type */

   GetImageFingerprint(image, NULL, FINGERPRINT_SECTOR);
   ExamineUDF(image);

   read_capacity(image);  /* Needed for ExamineECC() ! */
   ExamineECC(image);

   Verbose("# Calling query_size()\n");
   dh->sectors = query_size(image);
   Verbose("# returned: %lld sectors\n", dh->sectors); 

   switch(dh->subType & MAIN_TYPE_MASK)
   {  case BD:
      case DVD:
      case CD:
      {  char *tmp;
         if(!image->isoInfo) // || dh->rs02Size > 0)
	    tmp = g_strdup_printf(_("Medium: %s, %lld sectors%s"),
				  dh->typeDescr, dh->sectors,
				  image->expectedSectors ? ", Ecc" : ""); //fixme: validate
	 else
	    tmp = g_strdup_printf(_("Medium \"%s\": %s, %lld sectors%s created %s"),
				  image->isoInfo->volumeLabel,
				  dh->typeDescr, dh->sectors,
				  image->expectedSectors ? ", Ecc," : ",",  //fixme: validate
				  image->isoInfo->creationDate);

	 if(dh->manuID[0] && dh->manuID[0] != '-') 
	      dh->mediumDescr = g_strdup_printf("%s, %s %s.", tmp, _("Manuf.-ID:"), dh->manuID);
	 else dh->mediumDescr = g_strdup_printf("%s.", tmp);
	 g_free(tmp);
	 PrintLog("%s\n\n", dh->mediumDescr);
	 break;
      }

      default:
      {  char *td = alloca(strlen(dh->typeDescr)+1);

 	 strcpy(td, dh->typeDescr);
	 CloseImage(image);
       	 Stop(_("This software does not support \"%s\" type media."), td);
       	 return NULL;
      }
   }

   if(dh->mainType == DVD && query_copyright(dh))
   {  CloseImage(image);
      Stop(_("This software does not support encrypted media.\n"));
      return NULL;
   }

   /* Create the bitmap of simulated defects */

   if(Closure->simulateDefects)
     dh->defects = SimulateDefects(dh->sectors);

   return image;
}

/***
 *** Debugging function for sending a cdb to the drive
 ***/

int SendReadCDB(char *device, unsigned char *buf, unsigned char *cdb, int cdb_len, int alloc_len)
{  DeviceHandle *dh = NULL;
   Sense sense;
   int i,status;

   dh = OpenDevice(device);
   if(!dh) return 0;

   InquireDevice(dh, 0);

   PrintLog("Sending cdb to device: %s, %s\n", device, dh->devinfo);

   PrintLog("cdb:");
   for(i=0; i<cdb_len; i++)
     PrintLog(" %02x",cdb[i]);
   PrintLog(", length %d, allocation length %d\n", cdb_len, alloc_len);

   status = SendPacket(dh, cdb, cdb_len, buf, alloc_len, &sense, DATA_READ);

   CloseDevice(dh);

   if(status < 0)
   {  PrintLog("\nOperation failed with status = %d\n", status);
      PrintLog("Sense key: %02x, asc/ascq: %02x/%02x\n", sense.sense_key, sense.asc, sense.ascq);
      PrintLog("%s\n", GetSenseString(sense.sense_key, sense.asc, sense.ascq, FALSE));
   }

   return status;
}
