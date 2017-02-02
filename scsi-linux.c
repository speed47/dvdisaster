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
 *** The GNU/Linux SCSI wrapper. Uses the uniform CDROM driver. Kernel 2.6.x recommended. 
 ***/

#ifdef SYS_LINUX
#include <linux/param.h>
#include <scsi/sg.h>

char* DefaultDevice()
{  DeviceHandle *dh;
   GDir *dir;
   const char* dev;
   int dev_type;

   /* As a convenience, add the simulated drive first. */

   InitSimulatedCD();

   /* Now probe the physical drives. */

   dir = g_dir_open("/dev", 0, NULL);

   if(!dir)
   {  PrintLog(_("Can not access /dev for devices\n"
		  "No drives will be pre-selected.\n"));

      return g_strdup("/dev/cdrom");
   }

   dh = g_malloc(sizeof(DeviceHandle));

   while((dev = g_dir_read_name(dir)))
   {  
     if(!strncmp(dev,"scd",3) || !strncmp(dev,"sr", 2))
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
   {  PrintLog(_("No optical drives found in /dev.\n"
		  "No drives will be pre-selected.\n"));

      return g_strdup("/dev/cdrom");
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
  if(dh->simImage)
      LargeClose(dh->simImage);

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

//#define ASSERT_CDB_LENGTH

#ifdef ASSERT_CDB_LENGTH
static void assert_cdb_length(unsigned char cdb, int cdb_size, int expected_size)
{
   if(cdb_size != expected_size)
      PrintLog("SendPacket(): Wrong size %d for opcode %0x (expected %d)\n",
	       cdb_size, cdb, expected_size);
}

static void assert_cdb_direction(unsigned char cdb, int expected, int given)
{
   if(expected != given)
      PrintLog("SendPacket(): Wrong data direction %d for opcode %0x (expected %d)\n",
	       given, cdb, expected);
}

static void test_cdb(unsigned char *cdb, int cdb_size, int direction)
{
   switch(cdb[0])
   {  case 0x00: assert_cdb_length(cdb[0], cdb_size, 6);   /* TEST UNIT READY */
                 assert_cdb_direction(cdb[0], DATA_NONE, direction);
	 break;
      case 0x12: assert_cdb_length(cdb[0], cdb_size, 6);   /* INQUIRY */
                 assert_cdb_direction(cdb[0], DATA_READ, direction);
         break;
      case 0x1b: assert_cdb_length(cdb[0], cdb_size, 6);   /* START STOP */
                 assert_cdb_direction(cdb[0], DATA_NONE, direction);
	 break;
      case 0x1e: assert_cdb_length(cdb[0], cdb_size, 6);   /* PREVENT ALLOW MEDIUM REMOVAL */
                 assert_cdb_direction(cdb[0], DATA_NONE, direction);
	 break;
      case 0x23: assert_cdb_length(cdb[0], cdb_size, 10);  /* READ FORMAT CAPACITIES */
                 assert_cdb_direction(cdb[0], DATA_READ, direction);
	 break;
      case 0x25: assert_cdb_length(cdb[0], cdb_size, 10);  /* READ CAPACITY */
                 assert_cdb_direction(cdb[0], DATA_READ, direction);
	 break;
      case 0x28: assert_cdb_length(cdb[0], cdb_size, 10);  /* READ(10) */
                 assert_cdb_direction(cdb[0], DATA_READ, direction);
	 break;
      case 0x43: assert_cdb_length(cdb[0], cdb_size, 10);  /* READ TOC/PMA/ATIP */
                 assert_cdb_direction(cdb[0], DATA_READ, direction);
	 break;
      case 0x46: assert_cdb_length(cdb[0], cdb_size, 10);  /* GET CONFIGURATION */
                 assert_cdb_direction(cdb[0], DATA_READ, direction);
	 break;
      case 0x51: assert_cdb_length(cdb[0], cdb_size, 10);  /* READ DISC INFORMATION */
                 assert_cdb_direction(cdb[0], DATA_READ, direction);
	 break;
      case 0x52: assert_cdb_length(cdb[0], cdb_size, 10);  /* READ TRACK INFORMATION */
                 assert_cdb_direction(cdb[0], DATA_READ, direction);
	 break;
      case 0x55: assert_cdb_length(cdb[0], cdb_size, 10);  /* MODE SELECT */
                 assert_cdb_direction(cdb[0], DATA_WRITE, direction);
	 break;
      case 0x5a: assert_cdb_length(cdb[0], cdb_size, 10);  /* MODE SENSE */
                 assert_cdb_direction(cdb[0], DATA_READ, direction);
	 break;
      case 0xad: assert_cdb_length(cdb[0], cdb_size, 12);  /* READ DVD STRUCTURE */
                 assert_cdb_direction(cdb[0], DATA_READ, direction);
	 break;
      case 0xbe: assert_cdb_length(cdb[0], cdb_size, 12);  /* READ CD */
                 assert_cdb_direction(cdb[0], DATA_READ, direction);
	 break;
      default:
	 PrintLog("SendPacket(): Unknown opcode %0x\n", cdb[0]);
   }
}
#endif

/*
 * The CDROM ioctl() interface has been used since the first dvdisaster release.
 * However with recent 2.6 kernels it seems to become outdated - several parallel
 * SCSI cards are already exhibiting failures using this interface.
 * Starting with dvdisaster 0.79.3, the SG_IO interface has become
 * the default now. You can revert back to old behaviour using --driver=cdrom.
 */

static int send_packet_cdrom(DeviceHandle *dh, unsigned char *cmd, int cdb_size, unsigned char *buf, int size, Sense *sense, int data_mode)
{  struct cdrom_generic_command cgc;

#ifdef ASSERT_CDB_LENGTH
   test_cdb(cmd, cdb_size, data_mode);
#endif

   memset(&cgc, 0, sizeof(cgc));

   memcpy(cgc.cmd, cmd, MAX_CDB_SIZE);  /* GNU/Linux ignores the CDB size */
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
      case DATA_NONE:
        cgc.data_direction = CGC_DATA_NONE; 
	break;
      default:
	Stop("illegal data_mode: %d", data_mode);
   }

   return ioctl(dh->fd, CDROM_SEND_PACKET, &cgc);
}

/*
 * Access to the drive through the generic SCSI interface
 * has been added in dvdisaster 0.72 - it seems to be better
 * maintained than the older CDROM_SEND_PACKET interface now.
 * Especially parallel SCSI cdroms require this now.
 */

static int send_packet_generic(DeviceHandle *dh, unsigned char *cmd, int cdb_size, unsigned char *buf, int size, Sense *sense, int data_mode)
{  struct sg_io_hdr sg_io;

#ifdef ASSERT_CDB_LENGTH
   test_cdb(cmd, cdb_size, data_mode);
#endif

   memset(&sg_io, 0, sizeof(sg_io));
   sg_io.interface_id = 'S';

   switch(data_mode)
   {  case DATA_READ:
	sg_io.dxfer_direction = SG_DXFER_FROM_DEV;
	break;
      case DATA_WRITE:
	sg_io.dxfer_direction = SG_DXFER_TO_DEV;
	break;
      case DATA_NONE:
	sg_io.dxfer_direction = SG_DXFER_NONE;
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
   if(dh->simImage)
      return SimulateSendPacket(dh, cmd, cdb_size, buf, size, sense, data_mode);

   switch(Closure->useSCSIDriver)
   {
      case DRIVER_SG:
	return send_packet_generic(dh, cmd, cdb_size, buf, size, sense, data_mode);
	
      case DRIVER_CDROM:
	return send_packet_cdrom(dh, cmd, cdb_size, buf, size, sense, data_mode);

      default: 
        Stop("no SCSI driver selected");
	break;
   }

   return -1;
}

#endif /* SYS_LINUX */
