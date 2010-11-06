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

/***
 *** The Linux SCSI wrapper. Uses the uniform CDROM driver. Kernel 2.6.x recommended. 
 ***/

#ifdef SYS_LINUX
#include <linux/param.h>
#include <scsi/sg.h>

char* DefaultDevice()
{  DeviceHandle *dh;
   GDir *dir;
   const char* dev;
   int dev_type;

   dir = g_dir_open("/dev", 0, NULL);

   if(!dir)
   {  PrintLog(_("Can not access /dev for devices\n"
		  "No drives will be pre-selected.\n"));

      return g_strdup("/dev/cdrom");
   }

   dh = g_malloc(sizeof(DeviceHandle));

   while((dev = g_dir_read_name(dir)))
   {  
     if(   (strlen(dev) == 3 && (!strncmp(dev,"hd",2) || !strncmp(dev,"sd",2) || !strncmp(dev,"sr", 2)))
	|| (strlen(dev) == 4 && !strncmp(dev,"scd",3)) )
     { char buf[80];

       sprintf(buf,"/dev/%s", dev); 
       memset(dh, 0, sizeof(DeviceHandle));
       dh->fd = open(buf, O_RDWR | O_NONBLOCK);
       dh->device = buf;

       if(dh->fd < 0)   /* device not even present */
	 continue;

       dev_type = InquireDevice(dh, 1);
       close(dh->fd);

       if(dev_type != 5)  /* not a CD/DVD ROM */
	 continue;

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

      return g_strdup("/dev/cdrom");
   }
}

DeviceHandle* OpenDevice(char *device)
{  DeviceHandle *dh; 

   dh = g_malloc0(sizeof(DeviceHandle));
   dh->fd = open(device, O_RDWR | O_NONBLOCK);

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

//#define ASSERT_CDB_LENGTH

#ifdef ASSERT_CDB_LENGTH
static void assert_cdb_length(unsigned char cdb, int cdb_size, int expected_size)
{
   if(cdb_size != expected_size)
      PrintLog("SendPacket(): Wrong size %d for opcode %0x (expected %d)\n",
	       cdb_size, cdb, expected_size);
}

static void test_cdb(unsigned char *cdb, int cdb_size)
{
   switch(cdb[0])
   {  case 0x00: assert_cdb_length(cdb[0], cdb_size, 6);   /* TEST UNIT READY */
	 break;
      case 0x12: assert_cdb_length(cdb[0], cdb_size, 6);   /* INQUIRY */
         break;
      case 0x1b: assert_cdb_length(cdb[0], cdb_size, 6);   /* START STOP */
	 break;
      case 0x1e: assert_cdb_length(cdb[0], cdb_size, 6);   /* PREVENT ALLOW MEDIUM REMOVAL */
	 break;
      case 0x23: assert_cdb_length(cdb[0], cdb_size, 10);  /* READ FORMAT CAPACITIES */
	 break;
      case 0x25: assert_cdb_length(cdb[0], cdb_size, 10);  /* READ CAPACITY */
	 break;
      case 0x28: assert_cdb_length(cdb[0], cdb_size, 10);  /* READ(10) */
	 break;
      case 0x43: assert_cdb_length(cdb[0], cdb_size, 10);  /* READ TOC/PMA/ATIP */
	 break;
      case 0x46: assert_cdb_length(cdb[0], cdb_size, 10);  /* GET CONFIGURATION */
	 break;
      case 0x51: assert_cdb_length(cdb[0], cdb_size, 10);  /* READ DISC INFORMATION */
	 break;
      case 0x52: assert_cdb_length(cdb[0], cdb_size, 10);  /* READ TRACK INFORMATION */
	 break;
      case 0x55: assert_cdb_length(cdb[0], cdb_size, 10);  /* MODE SELECT */
	 break;
      case 0x5a: assert_cdb_length(cdb[0], cdb_size, 10);  /* MODE SENSE */
	 break;
      case 0xad: assert_cdb_length(cdb[0], cdb_size, 12);  /* READ DVD STRUCTURE */
	 break;
      case 0xbe: assert_cdb_length(cdb[0], cdb_size, 12);  /* READ CD */
	 break;
      default:
	 PrintLog("SendPacket(): Unknown opcode %0x\n", cdb[0]);
   }
}
#endif

/*
 * The CDROM ioctl() interface has been used since the first dvdisaster
 * release - it's the proven way of accessing the drive.
 */

static int send_packet_cdrom(DeviceHandle *dh, unsigned char *cmd, int cdb_size, unsigned char *buf, int size, Sense *sense, int data_mode)
{  struct cdrom_generic_command cgc;

#ifdef ASSERT_CDB_LENGTH
   test_cdb(cmd, cdb_size);
#endif

   memset(&cgc, 0, sizeof(cgc));

   memcpy(cgc.cmd, cmd, MAX_CDB_SIZE);  /* Linux ignores the CDB size */
   cgc.buffer = buf;
   cgc.buflen = size;
   cgc.sense  = (struct request_sense*)sense;
   cgc.timeout = 10*60*HZ;   /* 10 minutes; a timeout hangs newer kernels  */

   switch(data_mode)
   {  case DATA_READ:
        cgc.data_direction = CGC_DATA_READ; 
	break;
      case DATA_WRITE:
        cgc.data_direction = CGC_DATA_WRITE; 
	break;
      default:
	Stop("illegal data_mode: %d", data_mode);
   }

   return ioctl(dh->fd, CDROM_SEND_PACKET, &cgc);
}

/*
 * Access to the drive through the generic SCSI interface
 * has been added in dvdisaster 0.72 - it may have undetected flaws.
 * Only use it if there are problems with the normal CDROM interface
 * (some ancient parallel SCSI adapters/drives seem to fall into this
 *  category).
 */

static int send_packet_generic(DeviceHandle *dh, unsigned char *cmd, int cdb_size, unsigned char *buf, int size, Sense *sense, int data_mode)
{  struct sg_io_hdr sg_io;

   memset(&sg_io, 0, sizeof(sg_io));
   sg_io.interface_id = 'S';

   switch(data_mode)
   {  case DATA_READ:
	sg_io.dxfer_direction = SG_DXFER_FROM_DEV;
	break;
      case DATA_WRITE:
	sg_io.dxfer_direction = SG_DXFER_TO_DEV;
	break;
      default:
	Stop("illegal data_mode: %d", data_mode);
   }

   sg_io.cmd_len      = cdb_size;
   sg_io.mx_sb_len    = sizeof(Sense);
   sg_io.dxfer_len    = size;
   sg_io.dxferp	      = buf;
   sg_io.cmdp	      = cmd;
   sg_io.sbp	      = (unsigned char*)sense;
   sg_io.timeout      = 10*60*1000;
   sg_io.flags	      = SG_FLAG_LUN_INHIBIT|SG_FLAG_DIRECT_IO;


   if(ioctl(dh->fd,SG_IO,&sg_io)) 
   {  dh->sense.sense_key = 3;   /* pseudo error indicating */
      dh->sense.asc       = 255; /* ioctl() failure */
      dh->sense.ascq      = 254;
      return -1;
   }

   if(sg_io.status)
      return -1;

#if 0
   if ((sg_io.info&SG_INFO_OK_MASK) == SG_INFO_OK)
      return 0;
#endif

   return 0;
}

int SendPacket(DeviceHandle *dh, unsigned char *cmd, int cdb_size, unsigned char *buf, int size, Sense *sense, int data_mode)
{
   if(!Closure->useSGioctl)
        return send_packet_cdrom(dh, cmd, cdb_size, buf, size, sense, data_mode);
   else return send_packet_generic(dh, cmd, cdb_size, buf, size, sense, data_mode);
}

#endif /* SYS_LINUX */
