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

#ifdef SYS_FREEBSD

/* SCSI wrappers for FreeBSD are still work in progress. */

char* DefaultDevice()
{  DeviceHandle *dh;
   GDir *dir;
   const char* dev;
   int dev_type;

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
     if((strlen(dev) == 5 && !strncmp(dev,"pass",4)))
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
   {  PrintLog(_("No CD/DVD drives found in /dev.\n"
		  "No drives will be pre-selected.\n"));

      return g_strdup("no_drives");
   }
}

DeviceHandle* OpenDevice(char *device)
{  DeviceHandle *dh;

   dh = g_malloc0(sizeof(DeviceHandle));

   dh->camdev = cam_open_pass(device, O_RDWR, NULL);

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
  g_free(dh);
}

int SendPacket(DeviceHandle *dh, unsigned char *cmd, int cdb_size, unsigned char *buf, int size, Sense *sense, int data_mode)
{  union ccb *ccb = dh->ccb;
   u_int32_t flags = 0;
   u_int8_t status;

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
      cam_error_print(dh->camdev, ccb, CAM_ESF_ALL, CAM_EPF_ALL, stderr);
      return -1;
   }

   /* Extract sense data */

   memcpy(sense, &(ccb->csio.sense_data), sizeof(struct scsi_sense_data));

   if((ccb->ccb_h.status & CAM_STATUS_MASK) == CAM_REQ_CMP)
     return 0;

   /* See what went wrong (has still to be done; see scsi-win32.c) */

   status = ccb->csio.scsi_status;

   return -1;

   
}

#endif /* SYS_FREEBSD */
