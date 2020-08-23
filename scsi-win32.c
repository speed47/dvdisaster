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

#include "dvdisaster.h"

#include "scsi-layer.h"
#include "udf.h"

/**
 ** Windows wrapper. 
 **
 * As of dvdisaster 0.73.1, Windows 2000 SP4 is the minimum
 * system requirement and thus only SPTI is supported.
 */

#ifdef SYS_MINGW

/*
 * The drive letter ordering is remembered for
 * historical reasons (it was once useful for ASPI mapping)
 */

static int drive_letters[26];
static int cd_dvd_drives = 0;

char* DefaultDevice()
{  
   char drive_letter;
   char device_path[16],buf[80];
   DeviceHandle *dh;

   /* As a convenience, add the simulated drive first. */

   InitSimulatedCD();

   /* Now probe the physical drives. */

   for(drive_letter='C'; drive_letter<='Z'; drive_letter++)
   {  UINT drive_type;

      g_sprintf(device_path, "%c:\\", drive_letter);
      drive_type = GetDriveType(device_path);

      if(drive_type == DRIVE_CDROM)
      {  drive_letters[cd_dvd_drives] = drive_letter;
	 cd_dvd_drives++;

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

   if(Closure->deviceNodes->len)
     return g_strdup(g_ptr_array_index(Closure->deviceNodes, 0));
   else
   {  PrintLog(_("No optical drives found in /dev.\n"
		  "No drives will be pre-selected.\n"));


   return g_strdup("No_Drive_found");
   }
}

/*
 * Close the SPTI device.
 */

void CloseDevice(DeviceHandle *dh)
{
  if(dh->simImage)
    LargeClose(dh->simImage);

  if(dh->canReadDefective)
    SetRawMode(dh, MODE_PAGE_UNSET);

  if(dh->rawBuffer)
     FreeRawBuffer(dh->rawBuffer);

  if(dh->fd)             /* SPTI cleanup */
     CloseHandle(dh->fd);

  if(dh->typeDescr) 
    g_free(dh->typeDescr);
  if(dh->mediumDescr) 
     g_free(dh->mediumDescr);
  if(dh->defects)
    FreeBitmap(dh->defects);

  g_free(dh->device);
  g_free(dh);
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

/*
 * Open the device
 */

DeviceHandle* OpenDevice(char *device)
{  DeviceHandle *dh = NULL; 

   if(!strcmp(device, "sim-cd"))
   {  if(!Closure->simulateCD) /* can happen via resource file / last-device */
          return NULL;
       dh = g_malloc0(sizeof(DeviceHandle));
       dh->senseSize = sizeof(Sense);
       dh->simImage = LargeOpen(Closure->simulateCD, O_RDONLY, IMG_PERMS);
       if(!dh->simImage)
       {  g_free(dh);

          Stop(_("Could not open %s: %s"), Closure->simulateCD, strerror(errno));
          return NULL;
       }
   }
   else {
      if(   (*device >= 'c' && *device <= 'z')
         || (*device >= 'C' && *device <= 'Z'))
        dh = open_spti_device(Closure->device);

      if(!dh)
      {  Stop(_("\nCould not open device %s."), device);
         return NULL;
      }
   }

   return dh;
}

/*
 * Translate Scsi wrapper into SPTI call
 */

int SendPacket(DeviceHandle *dh, unsigned char *cmd, int cdb_size, unsigned char *buf, int size, Sense *sense, int data_mode)
{
   if(dh->simImage)
      return SimulateSendPacket(dh, cmd, cdb_size, buf, size, sense, data_mode);

  return send_spti_packet(dh->fd, cmd, cdb_size, (char *)buf, size, sense, data_mode);
}

#endif /* SYS_MINGW */
