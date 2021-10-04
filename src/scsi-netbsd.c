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

/* NetBSD support by Sergey Svishchev <svs@ropnet.ru>. 
 */

/*** src type: no GUI code ***/

#include "dvdisaster.h"

#include "scsi-layer.h"
#include "udf.h"

#ifdef SYS_NETBSD

#include <sys/device.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/scsiio.h>
#include <util.h>


char* DefaultDevice()
{  DeviceHandle *dh;
   char *disknames, *p, raw;
   int dev_type, sysctl_mib[2];
   size_t sysctl_len;

   /* As a convenience, add the simulated drive first. */

   InitSimulatedCD();

   /* Now probe the physical drives. */

   raw = 'a' + getrawpartition();

   sysctl_mib[0] = CTL_HW;
   sysctl_mib[1] = HW_DISKNAMES;
   if (-1 == sysctl(sysctl_mib, 2, NULL, &sysctl_len, NULL, 0)) {
     PrintLog("Failed to get value of sysctl `hw.disknames'\n");
     return g_strdup("no_drives");
   }
   if (!(disknames = g_malloc(sysctl_len))) {
     PrintLog("Out of memory constructing scan device list\n");
     return g_strdup("no_drives");
   }
   if (-1 == sysctl(sysctl_mib, 2, disknames, &sysctl_len, NULL, 0)) {
     PrintLog("Failed to get value of sysctl `hw.disknames'\n");
     g_free(disknames);
     return g_strdup("no_drives");
   }

   dh = g_malloc(sizeof(DeviceHandle));

   for (p = strtok(disknames, " "); p; p = strtok(NULL, " "))
   {  
     if(!strncmp(p,"cd",2))
     { char buf[80];

       sprintf(buf,"/dev/r%s%c", p, raw);

       memset(dh, 0, sizeof(DeviceHandle));
       dh->fd = open(buf, O_RDONLY | O_NONBLOCK);
       dh->device = buf;

       if(dh->fd < 0)   /* device not even present */
	 continue;

       dev_type = InquireDevice(dh, 1);
       close(dh->fd);

       if(dev_type != 5)  /* not a CD/DVD ROM */
	 continue;

       g_ptr_array_add(Closure->deviceNodes, g_strdup(buf));
       sprintf(buf, "%s (/dev/r%s%c)", dh->devinfo, p, raw);
       g_ptr_array_add(Closure->deviceNames, g_strdup(buf));
     }
   }

   g_free(dh);

   if(Closure->deviceNodes->len)
     return g_strdup(g_ptr_array_index(Closure->deviceNodes, 0));
   else
   {  PrintLog(_("No optical drives found.\n"
		  "No drives will be pre-selected.\n"));

      return g_strdup("no_drives");
   }
}

DeviceHandle* OpenDevice(char *device)
{  DeviceHandle *dh; 

   dh = g_malloc0(sizeof(DeviceHandle));
   dh->senseSize = sizeof(Sense);

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
   {  dh->fd = open(device, O_RDWR | O_NONBLOCK);

      if(dh->fd < 0)
      {  g_free(dh);

	 Stop(_("Could not open %s: %s"),device, strerror(errno));
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

  if(dh->fd)
    close(dh->fd);
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
{  struct scsireq sc;
   int sense_len;
   int rc;

   if(dh->simImage)
      return SimulateSendPacket(dh, cmd, cdb_size, buf, size, sense, data_mode);

   /* prepare the scsi request */

   memset(&sc, 0, sizeof(sc));
   memcpy(sc.cmd, cmd, cdb_size);
   sc.cmdlen = cdb_size;
   sc.databuf = (char *)buf;
   sc.datalen = size;
   sc.senselen = 24;			/* Maybe SENSEBUFLEN would be better? -cg */
   sc.timeout = 60000;			/* linux uses 5000 = 5 * HZ */

   switch(data_mode)
   {  case DATA_READ:
        sc.flags = SCCMD_READ;
	break;
      case DATA_WRITE:
        sc.flags = SCCMD_WRITE;
	break;
      case DATA_NONE:
	sc.flags = 0;
	break;
      default:
	Stop("illegal data_mode: %d", data_mode);
   }

   /* Send the request and save the sense data. */

   rc = ioctl(dh->fd, SCIOCCOMMAND, &sc);

   sense_len = sc.senselen_used;
   if(sense_len > dh->senseSize)
      sense_len = dh->senseSize;
   memcpy(sense, sc.sense, sense_len);

   /* See what we've got back */

   if(rc<0) return rc;    /* does not happen on SCSI errors */

   switch(sc.retsts)
   {  case SCCMD_OK:	  /* everything went fine */  
       	  return 0;   	
      case SCCMD_TIMEOUT: /* we don't know how to handle that yet */
	  PrintLog("SendPacket() - SCSI timeout\n");
	  return -1;
      case SCCMD_BUSY:    /* same here */
	  PrintLog("SendPacket() - target busy\n");
	  return -1;
      case SCCMD_SENSE:   /* SCSI error occurred, sense data available */
	  return -1;
      default:            /* something other went wrong */
	  return -1;
   }

   return -1; /* unreachable */
}
#endif /* SYS_NETBSD */
