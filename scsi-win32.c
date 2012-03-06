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

/**
 ** Windows wrapper. 
 **
 * Actually we have two wrappers; one for SPTI and one for ASPI.
 * SPTI requires Windows 2000 or XP and root priviledges, but seems
 * to be more compatible.
 * Otoh, ASPI run without special priviledges and even on the older
 * Windows 9x versions. 
 * The ASPI wrapper has only been tested against the WNASPI32.DLL
 * made by Adaptec.
 *
 * SPTI is tried first and then we fall back to ASPI.
 */

#ifdef SYS_MINGW

/*
 * This is independent from later decision between SPTI and ASPI. 
 * The drive letter ordering is remembered to aid the 
 * drive letter / ASPI drive guessing / mapping.
 */

static int drive_letters[26];
static int cd_dvd_drives = 0;

char* DefaultDevice()
{  static char picked[3] = "C:";
   char drive_letter;
   char device_path[16],buf[80];
   DeviceHandle *dh;

   for(drive_letter='C'; drive_letter<='Z'; drive_letter++)
   {  UINT drive_type;

      g_sprintf(device_path, "%c:\\", drive_letter);
      drive_type = GetDriveType(device_path);

      if(drive_type == DRIVE_CDROM)
      {  drive_letters[cd_dvd_drives] = drive_letter;
	 cd_dvd_drives++;

         if(cd_dvd_drives == 1)
	   *picked = drive_letter;

	 /* Collect drives accessible via SPTI */
	
	 sprintf(buf, "%c:", drive_letter);

	 if((dh = open_spti_device(buf)))
	 {  InquireDevice(dh, 1);

	    g_ptr_array_add(Closure->deviceNodes, g_strdup(buf));
	    sprintf(buf, "%c: %s", drive_letter, dh->devinfo);
	    g_ptr_array_add(Closure->deviceNames, g_strdup(buf));
	    CloseDevice(dh);
         } 
      }
   }

   /* Try looking for drives using ASPI.
      Changed behaviour since V0.72: 
      We provide both SPTI and ASPI in the GUI. */

   if(Closure->aspiLib)  
   {  int none_picked = !Closure->deviceNodes->len;
      DeviceHandle *dh = open_aspi_device("A:", 2);
      if(dh) CloseDevice(dh);

      if(none_picked 
	 && Closure->deviceNodes->len) /* pick first aspi drive */
	*picked = ((char*)g_ptr_array_index(Closure->deviceNodes,0))[0];
   }

   if(cd_dvd_drives > 0)
     return g_strdup(picked);

   return g_strdup("No_Drive_found");
}

/*
 * Close the SPTI/ASPI devices.
 */

void CloseDevice(DeviceHandle *dh)
{
  if(dh->canReadDefective)
    SetRawMode(dh, MODE_PAGE_UNSET);

  if(dh->rawBuffer)
     FreeRawBuffer(dh->rawBuffer);

  if(!dh->aspiUsed)             /* SPTI cleanup */
  {  CloseHandle(dh->fd);
  }

  if(dh->rs02Header)
    g_free(dh->rs02Header);
  if(dh->typeDescr) 
    g_free(dh->typeDescr);
  if(dh->mediumDescr) 
     g_free(dh->mediumDescr);
  if(dh->isoInfo)
    FreeIsoInfo(dh->isoInfo);
  if(dh->defects)
    FreeBitmap(dh->defects);

  g_free(dh->device);
  g_free(dh);
}

/**
 ** The ASPI wrapper.
 **/

/* 
 * Not everyone has the development kit with the #includes
 * for sending SCSI packets via ASPI, so we define what we need here.
 */

#define SS_PENDING        0x00
#define SS_COMP           0x01
#define SS_ERR            0x04

#define SRB_DIR_IN        0x08
#define SRB_DIR_OUT       0x10
#define SRB_EVENT_NOTIFY  0x40

#define SC_HA_INQUIRY     0x00 /* Host adapter inquiry */
#define SC_GET_DEV_TYPE   0x01 /* Get device type */
#define SC_EXEC_SCSI_CMD  0x02 /* Execute SCSI command */

#define DTYPE_DASD  0
#define DTYPE_CDROM 5

typedef struct
{   BYTE   Cmd;                /* ASPI command code = SC_HA_INQUIRY */
    BYTE   Status;             /* ASPI command status byte */
    BYTE   HaId;               /* ASPI host adapter number */
    BYTE   Flags;              /* ASPI request flags */
    DWORD  Hdr_Rsvd;           /* Reserved, MUST = 0 */
    BYTE   Count;              /* Number of host adapters present */
    BYTE   SCSI_ID;            /* SCSI ID of host adapter */
    BYTE   HA_ManagerId[16];   /* String describing the manager */
    BYTE   HA_Identifier[16];  /* String describing the host adapter */
    WORD   HA_BufAlignMask;    /* HA_Unique[1-0] */
    BYTE   HA_Flags;           /* HA_Unique[2] */
    BYTE   HA_MaxTargets;      /* HA_Unique[3] */
    DWORD  HA_MaxTransferLength; /* HA_Unique[7-4] */
    DWORD  HA_MaxSGElements;   /* HA_Unique[11-8] */
    BYTE   HA_Rsvd2[4];        /* HA_Unique[15-12] */
    WORD   HA_Rsvd1;           /* Reserved, MUST = 0 */
} PACKED SRB32_HAInquiry;

typedef struct
{   BYTE   Cmd;                /* ASPI command code = SC_GET_DEV_TYPE */
    BYTE   Status;             /* ASPI command status byte */
    BYTE   HaId;               /* ASPI host adapter number */
    BYTE   Flags;              /* Reserved, MUST = 0 */
    DWORD  Hdr_Rsvd;           /* Reserved, MUST = 0 */
    BYTE   Target;             /* Target's SCSI ID */
    BYTE   Lun;                /* Target's LUN number */
    BYTE   DeviceType;         /* Target's peripheral device type */
    BYTE   Rsvd1;              /* Reserved, MUST = 0 */
} PACKED SRB32_GDEVBlock;

typedef struct
{   BYTE   Cmd;                /* ASPI command code = SC_EXEC_SCSI_CMD */
    BYTE   Status;             /* ASPI command status byte */
    BYTE   HaId;               /* ASPI host adapter number */
    BYTE   Flags;              /* ASPI request flags */
    DWORD  Hdr_Rsvd;           /* Reserved */
    BYTE   Target;             /* Target's SCSI ID */
    BYTE   Lun;                /* Target's LUN number */
    WORD   Rsvd1;              /* Reserved for Alignment */
    DWORD  BufLen;             /* Data Allocation Length */
    BYTE   *BufPtr;            /* Data Buffer Pointer */
    BYTE   SenseLen;           /* Sense Allocation Length */
    BYTE   CDBLen;             /* CDB Length */
    BYTE   HaStat;             /* Host Adapter Status */
    BYTE   TargStat;           /* Target Status */
    VOID   *PostProc;          /* Post routine */
    BYTE   Rsvd2[20];          /* Reserved, MUST be 0 */
    BYTE   CDBByte[16];        /* SCSI CDB */
    BYTE   SenseArea[16];      /* Request Sense buffer */
} PACKED SRB32_ExecSCSICmd;

/*
 * Open and close the ASPI library.
 */

void OpenAspi()
{
  /* Try to open the ASPI library */

  Closure->aspiLib = LoadLibrary("WNASPI32.DLL");
  if(!Closure->aspiLib) return;

  Closure->GetASPI32SupportInfo 
     = (DWORD(*)(void))GetProcAddress(Closure->aspiLib, "GetASPI32SupportInfo");
  if(!Closure->GetASPI32SupportInfo)
  {  PrintLog("GetASPI32SupportInfo() not available.");
     FreeLibrary(Closure->aspiLib);
     Closure->aspiLib = NULL;
     return;
  }

  Closure->SendASPI32Command 
     = (DWORD(*)(void*))GetProcAddress(Closure->aspiLib, "SendASPI32Command");
  if(!Closure->SendASPI32Command)
  {  PrintLog("SendASPI32Command() not available.");
     FreeLibrary(Closure->aspiLib);
     Closure->aspiLib = NULL;
     return;
  }

  /* The Adaptec docs seem to imply that this call is needed
     to initialize the ASPI library. */

  Closure->GetASPI32SupportInfo();  
}

void CloseAspi()
{
  if(Closure->aspiLib)
  {  FreeLibrary(Closure->aspiLib);
  }
}

/*
 * Open and prepare the device using ASPI.
 */

#define LIST_PRINT 1
#define LIST_COLLECT 2

DeviceHandle* open_aspi_device(char *device, int list_mode)
{ DeviceHandle *dh;
  int status,ret,ha,max_ha;
  SRB32_HAInquiry ha_inq;
  int drive_count = 0;
  int drive_wanted = 0;
  char spti_name[3];
  char letter_wanted = toupper(*device);

  dh = g_malloc0(sizeof(DeviceHandle));
  dh->device = g_strdup(device);

  /* Look for our special ASPI drive syntax */

  if(*device >= '1' && *device <= '9')
    drive_wanted = *device - '0';

  /* Bail out if no ASPI available */

  if(!Closure->aspiLib)
  {  g_free(dh);
     return NULL;
  }

  dh->aspiUsed = TRUE;

  /* Get number of host adapters. */

  ret    = Closure->GetASPI32SupportInfo();  
  status = (ret>>8) & 0xff;
  max_ha = ret & 0xff;

  if(status != SS_COMP)
  {  PrintLog("Could not determine number of host adapters\n");
     g_free(dh->device);
     g_free(dh); 
     return NULL;
  }

#if 0
  PrintLog("Status %d, %d host adapters\n",status,max_ha);
#endif

  /* Now see if we can find any CDROM drives. */

  for(ha=0; ha<max_ha; ha++)
  {  int target,n_targets;

     memset(&ha_inq, 0, sizeof(SRB32_HAInquiry));
     ha_inq.Cmd  = SC_HA_INQUIRY;
     ha_inq.HaId = ha;

     Closure->SendASPI32Command(&ha_inq);
     if(ha_inq.Status != SS_COMP)
       PrintLog("ASPI warning: Could not query host adapter %d\n",ha);

#if 0
     PrintLog("HA %d: %16s\n",ha,ha_inq.HA_Identifier);
#endif

     if(ha_inq.HA_MaxTargets == 16)  /* my interpretation of */
          n_targets = 16;            /* Adaptecs documentation */
     else n_targets = 8;

#if 0
     /* Some ASPI drivers return -1 or 0 for actually working
	configurations, so we can`t rely on this information. */

     if(ha_inq.HA_MaxTransferLength == 0)
       continue;
#endif

     /* Missing the following conditions is close to impossible. */

     if(ha_inq.HA_BufAlignMask >= 4096)
     {  Stop("ASPI alignment = %d requested; can't handle that.\n",
	     ha_inq.HA_BufAlignMask);
        g_free(dh);
	return NULL;
     }
#if 0
     /* Some ASPI drivers return 0 or -1 for actually working drives,
	so this information is also useless. */

     if(ha_inq.HA_MaxTransferLength < MAX_CLUSTER_SIZE)
     {  Stop("ASPI max xfer length = %d; can't handle that.\n",
	     ha_inq.HA_MaxTransferLength);
        g_free(dh);
	return NULL;
     }
#endif
     
     /* Iterate over the HA's possible targets */

     for(target=0; target<n_targets; target++)
     {  SRB32_GDEVBlock gdb;

        if(target == ha_inq.SCSI_ID) /* ignore the HA itself */
          continue;

	memset(&gdb, 0, sizeof(SRB32_GDEVBlock));
	gdb.Cmd    = SC_GET_DEV_TYPE;
	gdb.HaId   = ha;
	gdb.Target = target;
	gdb.Lun    = 0;  /* Are there CDROMs with Lun != 0 ? */
 
	Closure->SendASPI32Command(&gdb);

	if(gdb.Status != SS_COMP)
	  continue;  /* device does not exist */

	if(gdb.DeviceType == DTYPE_CDROM)
	{  char guessed_letter = drive_letters[drive_count];

	   drive_count++;

	   dh->ha = ha;
	   dh->target = target;
	   dh->lun = 0;

	   if(list_mode)
	   { InquireDevice(dh, 1);
	     if(drive_count<26 && guessed_letter)
	     {  spti_name[0] = guessed_letter;
	        spti_name[1] = ':';
	     }
	     else spti_name[0] = spti_name[1] = '?';
	     spti_name[2] = 0;

	     if(list_mode == LIST_PRINT)
	       PrintLog(" %d: (%s) %s\n", drive_count, spti_name, dh->devinfo);
	     else
             {	char buf[50];  /* devinfo is 34 */

		sprintf(buf, "%c: %s [ASPI# %d:]", 
	                guessed_letter, dh->devinfo, drive_count);
		g_ptr_array_add(Closure->deviceNames, g_strdup(buf));
		sprintf(buf, "%d:", drive_count);
		g_ptr_array_add(Closure->deviceNodes, g_strdup(buf));
             }
	   }

	   if(drive_wanted && drive_wanted == drive_count)
	     return dh;  /* drive found by our ASPI syntax */

	   if(letter_wanted == guessed_letter)
	     return dh;  /* hopefully found the right drive for letter */
	}
    }
  }

  if(list_mode == LIST_PRINT)
  {  if(!drive_count)
       PrintLog(_("ASPI manager present, but no CD/DVD drives managed.\n"));
     else if(drive_count != cd_dvd_drives)
       LogWarning(_("%d SPTI drives, but %d ASPI drives.\n"
	            "Drive letter mapping for ASPI drives is probably incorrect.\n"),
	          cd_dvd_drives, drive_count);
     return dh;
  }

  g_free(dh->device);
  g_free(dh); 
  return NULL;
}

/*
 * Print all CDROM drives accessible over ASPI.
 */

void ListAspiDrives()
{  DeviceHandle *dh;

   PrintCLI(_("\nList of ASPI CD/DVD drives:\n")); 

   dh = open_aspi_device("A:", LIST_PRINT);

   if(dh) 
   {  PrintCLI(_("\nTo force ASPI usage over SPTI, refer to the drive by the\n"
	         "above numbers (use 1:, 2:,... instead of C:, D:,...)\n"));

      CloseDevice(dh);   
   }
   else PrintCLI(_("ASPI manager not available or installed.\n"));
}


/*
 * Send the SCSI command through ASPI.
 */

static int send_aspi_packet(DeviceHandle *dh, unsigned char *cmd, int cdb_size, char *buf, int size, Sense *sense, int data_mode)
{  SRB32_ExecSCSICmd srb;
   DWORD status;
   HANDLE srb_event;

   srb_event = CreateEvent(NULL, 1, 0, NULL);

   /* Prepare the SRB struct */

   memset(&srb, 0, sizeof(SRB32_ExecSCSICmd));
   srb.Cmd     = SC_EXEC_SCSI_CMD;
   srb.HaId    = dh->ha;
   srb.Target  = dh->target;
   srb.Lun     = dh->lun;
   switch(data_mode)
   {  case DATA_WRITE:
	srb.Flags = SRB_DIR_OUT | SRB_EVENT_NOTIFY;
	break;
      case DATA_READ:
	srb.Flags = SRB_DIR_IN | SRB_EVENT_NOTIFY;
	break;
      case DATA_NONE:
	srb.Flags = SRB_EVENT_NOTIFY;
	break;
      default:
	Stop("illegal data_mode for ASPI: %d", data_mode);
	return -1;
   }
   srb.BufPtr  = buf;
   srb.BufLen  = size;
   srb.SenseLen = 16;
   srb.PostProc = srb_event;

   srb.CDBLen  = cdb_size;
   memcpy(&srb.CDBByte, cmd, cdb_size);

   /* Send the SCSI command */

   ResetEvent(srb_event);
   status = Closure->SendASPI32Command(&srb);

   if(status == SS_PENDING)
     WaitForSingleObject(srb_event, INFINITE);

   CloseHandle(srb_event);
   memcpy(sense, &srb.SenseArea, 16);

   /* SS_COMP implies no SCSI error */

   if(srb.Status == SS_COMP) return 0;

   /* Now see what went wrong */

   switch(srb.TargStat)
   {  case 0x00:  /* STATUS_GOOD */
	return -1;

      case 0x02: /* CHECK CONDITION */
	return -1;

      case 0x08: /* BUSY */
	PrintLog("ASPI layer: Target busy.\n");
	return -1;

      case 0x18: /* Reservation conflict */
	PrintLog("ASPI layer: Reservation conflict.\n");
	return -1;
   }

   return -1;
}

/**
 ** The SPTI wrapper.
 **/

/* 
 * The Mingw includes are missing the following structs 
 * for sending SCSI packets via SPTI.
 */

typedef struct 
{ USHORT Length;
  UCHAR  ScsiStatus;
  UCHAR  PathId;
  UCHAR  TargetId;
  UCHAR  Lun;
  UCHAR  CdbLength;
  UCHAR  SenseInfoLength;
  UCHAR  DataIn;
  ULONG  DataTransferLength;
  ULONG  TimeOutValue;
  PVOID  DataBuffer;
  ULONG  SenseInfoOffset;
  UCHAR  Cdb[16];
} SCSI_PASS_THROUGH_DIRECT;

typedef struct 
{ SCSI_PASS_THROUGH_DIRECT spt;
  ULONG  Filler;
  UCHAR  SenseBuf[32];
} SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;

#define IOCTL_SCSI_BASE 0x00000004
#define IOCTL_SCSI_PASS_THROUGH_DIRECT  CTL_CODE( IOCTL_SCSI_BASE, 0x0405, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS )

/*
 * Open the device using SPTI.
 */

DeviceHandle* open_spti_device(char *device)
{  DeviceHandle *dh; 
   char device_path[16];

   if(   strlen(device) != 2
      || toupper(*device) < 'C'
      || toupper(*device) > 'Z'
      || device[1] != ':')
   {
      Stop(_("\nIllegal device name \"%s\" (use devices \"C:\" ... \"Z:\")"),device);
      return NULL;
   }
      
   dh = g_malloc0(sizeof(DeviceHandle));
   dh->device = g_strdup(device);

   g_sprintf(device_path, "\\\\.\\%c:", toupper(*device));

   dh->fd = CreateFile(device_path,                  
	               GENERIC_READ | GENERIC_WRITE,
	               FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL, OPEN_EXISTING, 0, NULL );

   if(dh->fd == INVALID_HANDLE_VALUE)  /* Might be Win9x or missing priviledges */
   {  g_free(dh->device);
      g_free(dh);
      return NULL;
   }

   return dh;
}

/*
 * Send the SCSI command through SPTI.
 * At least W2K seems to have problems with SCSI commands timing out;
 * it even manages to do a spontaneous reboot because of that occasionally. 
 * So we set the time out value to a conservative 120 secs.
 */ 

static int send_spti_packet(HANDLE fd, unsigned char *cmd, int cdb_size, char *buf, int size, Sense *sense, int data_mode)
{  SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER ss;
   BOOL success;
   ULONG returned;

   memset(&ss, 0, sizeof(ss));

   ss.spt.Length          	= sizeof(SCSI_PASS_THROUGH_DIRECT);
   ss.spt.CdbLength       	= cdb_size;
   ss.spt.SenseInfoLength 	= 24;
   ss.spt.TargetId              = 1;
   switch(data_mode)
   {  case DATA_WRITE:
	ss.spt.DataIn          	= 0;  /* SCSI_IOCTL_DATA_OUT */
	break;
      case DATA_READ:
	ss.spt.DataIn          	= 1;  /* SCSI_IOCTL_DATA_IN */
	break;
      case DATA_NONE:
	ss.spt.DataIn          	= 2;  /* SCSI_IOCTL_DATA_UNSPECIFIED */
	break;
      default:
	Stop("illegal data_mode: %d",data_mode);
	return -1;
   }
   ss.spt.DataTransferLength 	= size;
   ss.spt.TimeOutValue    	= 120;  /* see comment above */
   ss.spt.DataBuffer      	= buf;
   ss.spt.SenseInfoOffset 	= offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, SenseBuf);
   memcpy(ss.spt.Cdb, cmd, cdb_size);

   success = DeviceIoControl(fd, IOCTL_SCSI_PASS_THROUGH_DIRECT,
			    &ss, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER),
			    &ss, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER),
			    &returned, FALSE);

   memcpy(sense, ss.SenseBuf, 24);  /* Hmm. What a kludge. */

   if(!success || ss.spt.ScsiStatus) return -1;
   return 0;
}

/***
 *** ASPI/SPTI wrapping
 ***/

/*
 * Open the device
 */

DeviceHandle* OpenDevice(char *device)
{  DeviceHandle *dh = NULL; 

   if(   (*device >= 'c' && *device <= 'z')
      || (*device >= 'C' && *device <= 'Z'))
     dh = open_spti_device(Closure->device);

   if(!dh) dh = open_aspi_device(Closure->device, 0);

   if(!dh)
   {  Stop(_("\nNeither SPTI nor ASPI worked opening %s."), device);
      return NULL;
   }

   return dh;
}

/*
 * Dispatch between SPTI and ASPI for packet sending.
 */

int SendPacket(DeviceHandle *dh, unsigned char *cmd, int cdb_size, unsigned char *buf, int size, Sense *sense, int data_mode)
{
  if(dh->aspiUsed) 
       return send_aspi_packet(dh, cmd, cdb_size, buf, size, sense, data_mode);
  else return send_spti_packet(dh->fd, cmd, cdb_size, buf, size, sense, data_mode);
}

#endif /* SYS_MINGW */
