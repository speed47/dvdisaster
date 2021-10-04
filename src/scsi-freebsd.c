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

/*** src type: no GUI code ***/

#include "dvdisaster.h"

#include "scsi-layer.h"
#include "udf.h"

#if defined(SYS_FREEBSD) || defined(SYS_KFREEBSD)

/* SCSI wrappers for FreeBSD are still work in progress. */

char* DefaultDevice()
{  DeviceHandle *dh;
   GDir *dir;
   const char* dev;
   int dev_type;

   /* As a convenience, add the simulated drive first. */

   InitSimulatedCD();

   /*** Look for suitable devices */

   dir = g_dir_open("/dev", 0, NULL);

   if(!dir)
   {  PrintLog(_("Can not access /dev for devices\n"
		  "No drives will be pre-selected.\n"));

      return g_strdup("no_drives");
   }

   dh = g_malloc(sizeof(DeviceHandle));

   /* iterate through /dev/pass<n> */

   while((dev = g_dir_read_name(dir)))
   { 
     if(!strncmp(dev,"pass",4))
     { char buf[80];

       sprintf(buf,"/dev/%s", dev); 

       memset(dh, 0, sizeof(DeviceHandle));

       /* see if we can open it */

       dh->camdev = cam_open_pass(buf, O_RDWR, NULL);
       dh->device = buf;

       if(!dh->camdev)   /* device not even present */
	 continue;
       
       dh->ccb = cam_getccb(dh->camdev);
       if(!dh->ccb)
	 continue;

       /* make sure its a CDROM type drive */

       dev_type = InquireDevice(dh, 1);

       cam_freeccb(dh->ccb);
       cam_close_device(dh->camdev);

       if(dev_type != 5)  /* not a CD/DVD ROM */
	 continue;

       /* remember it in our device list */

       g_ptr_array_add(Closure->deviceNodes, g_strdup(buf));
       sprintf(buf, "%s (/dev/%s)", dh->devinfo, dev);
       g_ptr_array_add(Closure->deviceNames, g_strdup(buf));
     }
   }

   g_dir_close(dir);
   g_free(dh);

   if(Closure->deviceNodes->len)
     return g_strdup(g_ptr_array_index(Closure->deviceNodes, 0));
   else
   {  PrintLog(_("No optical drives found in /dev.\n"
		  "No drives will be pre-selected.\n"));

      return g_strdup("no_drives");
   }
}

DeviceHandle* OpenDevice(char *device)
{  DeviceHandle *dh;

   dh = g_malloc0(sizeof(DeviceHandle));

   /* Make sure we do not overrun any memory due
      to differently sized sense structures */

   dh->senseSize = sizeof(Sense);
   if(dh->senseSize > sizeof(struct scsi_sense_data))
      dh->senseSize = sizeof(struct scsi_sense_data);

   if(!strcmp(device, "sim-cd"))
   {  if(!Closure->simulateCD) /* can happen via resource file / last-device */
      {  g_free(dh);
         return NULL;
      }
     
      dh->simImage = LargeOpen(Closure->simulateCD, O_RDONLY, IMG_PERMS);
      if(!dh->simImage)
      {  g_free(dh);

	 Stop(_("Could not open %s: %s"), Closure->simulateCD, strerror(errno));
	 return NULL;
      }
   }
   else
   {  dh->camdev = cam_open_pass(device, O_RDWR, NULL);

      if(!dh->camdev)
      {  g_free(dh);

	 Stop(_("Could not open %s: %s"),device, strerror(errno));
	 return NULL;
      }

      dh->ccb = cam_getccb(dh->camdev);

      if(!dh->ccb)
      {  cam_close_device(dh->camdev);
	 g_free(dh);

	 Stop("Could not allocate ccb for %s", device);
	 return NULL;
      }
   }

   dh->device = g_strdup(device);

   return dh;
}

void CloseDevice(DeviceHandle *dh)
{ 
  if(dh->canReadDefective)
    SetRawMode(dh, MODE_PAGE_UNSET);

  if(dh->rawBuffer)
     FreeRawBuffer(dh->rawBuffer);

  if(dh->ccb)
    cam_freeccb(dh->ccb);
  if(dh->camdev)
    cam_close_device(dh->camdev);
  if(dh->device)
    g_free(dh->device);
  if(dh->typeDescr) 
    g_free(dh->typeDescr);
  if(dh->mediumDescr) 
    g_free(dh->mediumDescr);
  if(dh->defects)
    FreeBitmap(dh->defects);
  g_free(dh);
}

int SendPacket(DeviceHandle *dh, unsigned char *cmd, int cdb_size, unsigned char *buf, int size, Sense *sense, int data_mode)
{  union ccb *ccb = dh->ccb;
   u_int32_t flags = 0;

   if(dh->simImage)
      return SimulateSendPacket(dh, cmd, cdb_size, buf, size, sense, data_mode);

   bzero(&(&ccb->ccb_h)[1],
	 sizeof(struct ccb_scsiio) - sizeof(struct ccb_hdr));

   switch(data_mode)
   {  case DATA_READ:
        flags = CAM_DIR_IN;
	break;
      case DATA_WRITE:
	flags = CAM_DIR_OUT;
	break;
      case DATA_NONE:
	flags = CAM_DIR_NONE;
	break;
      default:
	Stop("illegal data_mode: %d", data_mode);
   }

   cam_fill_csio(&ccb->csio, 1, NULL, flags, CAM_TAG_ACTION_NONE,//MSG_SIMPLE_Q_TAG,
		 buf, size, sizeof(struct scsi_sense_data), cdb_size, 
		 120*1000);  /* 120 secs timeout */

   memcpy(ccb->csio.cdb_io.cdb_bytes, cmd, cdb_size);
		 
   /* Send ccb */

   if(cam_send_ccb(dh->camdev, ccb) < 0) 
   {  printf("cam_send failed\n");
      cam_error_print(dh->camdev, ccb, CAM_ESF_ALL, CAM_EPF_ALL, stdout);
      return -1;
   }

   /* Extract sense data */

   memcpy(sense, &(ccb->csio.sense_data), dh->senseSize);

   if((ccb->ccb_h.status & CAM_STATUS_MASK) == CAM_REQ_CMP)
     return 0;

   /* See what went wrong (not covering all cases) */

   switch(ccb->csio.scsi_status)
   {  case 0x08:  /* BUSY */
        PrintLog("SendPacket: Target busy.\n");
	break;

      case 0x18:  /* Reservation conflict */
        PrintLog("SendPacket: Reservation conflict.\n");
	break;
   } 
   
   return -1;
}

#endif /* defined(SYS_FREEBSD) || defined(SYS_KFREEBSD) */
