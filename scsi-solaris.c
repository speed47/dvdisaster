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

#ifdef SYS_SOLARIS

#include <sys/scsi/impl/uscsi.h>

/***
 *** Warning
 ***
 * Solaris is not officially supported. 
 * This driver mostly exists for testing purposes on big endian archs.
 * 
 * Note:
 *
 * - Only Solaris 10 3/05 or newer are supported.
 * - Both x86 and Sparc are supported.
 * - GNU tools (gcc, gmake etc.) are required. Do not use the Solaris counterparts.
 * - You must run as root in order to access drives.
 *   Do NOT install the program suid root; it does NOT drop root priviledges!
 * - Volume management must be off (/etc/init.d/volmgt stop)
 * - Use UFS for image and ecc files to get optimal I/O speed.
 *   Error correction will cause fragmentation/slowdown on ZFS.
 */

/* Dummy routines so that we can compile on unknown architectures
   for which we don't have SCSI support yet. */

char* DefaultDevice()
{  DeviceHandle *dh;
   GDir *dir;
   const char* dev;
   int dev_type;

   dir = g_dir_open("/dev/rdsk", 0, NULL);

   if(!dir)
   {  PrintLog(_("Can not access /dev for devices\n"
		  "No drives will be pre-selected.\n"));

      return g_strdup("no_drives");
   }

   dh = g_malloc(sizeof(DeviceHandle));

   while((dev = g_dir_read_name(dir)))
   {  int slice;
      char buf[80];

      /* Sort out the uninteresting devices,
         we want only the cntndns2 ones */
   
      if(sscanf(dev,"c%*dt%*dd%*ds%d", &slice) != 1)
	continue;

      if(slice != 2)
	continue;

      /* Try to open the device */

      sprintf(buf,"/dev/rdsk/%s", dev); 

      memset(dh, 0, sizeof(DeviceHandle));
      dh->fd = open(buf, O_RDONLY | O_NDELAY);
      dh->device = buf;

      if(dh->fd < 0)   /* device not even present */
	continue;

      /* Inquire and see what we've got */

      dev_type = InquireDevice(dh, 1);
      close(dh->fd);

      if(dev_type != 5)  /* not a CD/DVD ROM */
	continue;

      g_ptr_array_add(Closure->deviceNodes, g_strdup(buf));
      sprintf(buf, "%s (%s)", dh->devinfo, dev);
      g_ptr_array_add(Closure->deviceNames, g_strdup(buf));
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
   dh->fd = open(device, O_RDONLY | O_NDELAY); 

   if(dh->fd < 0)
   {  g_free(dh);
      Stop(_("Could not open %s: %s"),device, strerror(errno));
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

  if(dh->fd)
    close(dh->fd);
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
{  struct uscsi_cmd ucmd;

   memset(&ucmd, 0, sizeof(struct uscsi_cmd)); 
 
   ucmd.uscsi_flags = USCSI_SILENT | USCSI_DIAGNOSE | USCSI_RQENABLE; 
   switch(data_mode)
   {  case DATA_READ:
	ucmd.uscsi_flags |= USCSI_READ;
	break;
      case DATA_WRITE:
	ucmd.uscsi_flags |= USCSI_WRITE;
	break;
      case DATA_NONE:
	break;
      default:
	Stop("illegal data_mode: %d", data_mode);
   }

   ucmd.uscsi_timeout = 30*60;   /* wait 30min for completion (timeout locks up the OS) */
   ucmd.uscsi_cdb     = cmd;
   ucmd.uscsi_cdblen  = cdb_size;
   ucmd.uscsi_bufaddr = buf;
   ucmd.uscsi_buflen  = size;
   ucmd.uscsi_rqbuf   = (caddr_t)sense;
   ucmd.uscsi_rqlen   = sizeof(Sense);

   return ioctl(dh->fd, USCSICMD, &ucmd);
}

#endif /* SYS_SOLARIS */
