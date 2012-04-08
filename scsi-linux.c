/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2004-2012 Carsten Gnoerlich.
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
   AlignedBuffer *ab;
   Sense *sense;
   unsigned char cmd[MAX_CDB_SIZE];
   int length;
   int phy_int_std;

   dh = g_malloc0(sizeof(DeviceHandle));
   dh->fd = open(device, O_RDWR | O_NONBLOCK);

   if(dh->fd < 0)
   {  g_free(dh);
      Stop(_("Could not open %s: %s"),device, strerror(errno));
      return NULL;
   }

   dh->device = g_strdup(device);

   /*** Probe for parallel SCSI.
	We can't use the CDROM_SEND_PACKET ioctl with it. */

   Verbose("# *** OpenDevice(%s) - GET CONFIGURATION ***\n", device);

   length = 2048;
   ab = CreateAlignedBuffer(length); 
   sense = &dh->sense;

   /* Query length of returned data */

   memset(cmd, 0, MAX_CDB_SIZE);
   cmd[0] = 0x46;     /* GET CONFIGURATION */
   cmd[1] = 0x02;     /* only specified feature */
   cmd[2] = 0;
   cmd[3] = 1;        /* we want the core feature (0x0001) */
   cmd[7] = length>>8;
   cmd[8] = length&0xff;        /* Allocation length */

   if(SendPacket(dh, cmd, 10, ab->buf, length, sense, DATA_READ)<0)
   {  
      FreeAlignedBuffer(ab);
      Verbose("# failed -> could not get core feature: %s\n",
	      GetSenseString(sense->sense_key, sense->asc, sense->ascq, 0));

      if(Closure->useSCSIDriver == DRIVER_CDROM_FORCED)
	Verbose("# Would like to play it safe, but CDROM_SEND_PACKET ioctl()\n"
		"# forced via command line. Prepare for wreckage.\n");
      else 
      {  Verbose("# Playing it safe. Forcing use of SG_IO ioctl().\n");
	 dh->forceSG_IO = TRUE;
      }
      return dh;
   }

   length = ab->buf[0]<<24 | ab->buf[1] | ab->buf[2] | ab->buf[3];
   if(length < 12)
   {  FreeAlignedBuffer(ab);
      Verbose("# failed -> invalid length for core feature: %d\n", length);
      return dh;
   }

   phy_int_std = ab->buf[12]<<24 | ab->buf[13]<<16 | ab->buf[14]<<8 | ab->buf[15];

   Verbose("# physical interface standard: %d\n", phy_int_std);
   
   switch(phy_int_std)
   { case 2: Verbose("# ATAPI. Hopefully not behind a bridge.\n");
             break;
     case 1: if(Closure->useSCSIDriver == DRIVER_CDROM_FORCED)
	       Verbose("# SCSI, but CDROM_SEND_PACKET ioctl() forced via command line.\n"
		       "#       Prepare for wreckage.\n");
	     else 
	     {  Verbose("# SCSI. Forcing use of SG_IO ioctl().\n");
                dh->forceSG_IO = TRUE;
	     }
	     break;
   }

   FreeAlignedBuffer(ab);
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
 * Starting with dvdisaster 0.72.2 and 0.79.3, the SG_IO interface has become
 * the default now. You can revert back to old behaviour using --driver=cdrom.
 */

static int send_packet_cdrom(DeviceHandle *dh, unsigned char *cmd, int cdb_size, unsigned char *buf, int size, Sense *sense, int data_mode)
{  struct cdrom_generic_command cgc;

#ifdef ASSERT_CDB_LENGTH
  test_cdb(cmd, cdb_size, data_mode);
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
{  int driver = Closure->useSCSIDriver;

   /* Using the CDROM_SEND_PACKET ioctl kills parallel SCSI adapters.
      Redirect the necessary probing commands to the SG_IO driver. */

   if( (cmd[0] == 0x46 || cmd[0] == 0x12) && driver != DRIVER_CDROM_FORCED)
     driver = DRIVER_SG;

   if(dh->forceSG_IO)
     driver = DRIVER_SG;

   /* dispatch to appropriate driver */

   switch(driver)
   {
      case DRIVER_SG:
	return send_packet_generic(dh, cmd, cdb_size, buf, size, sense, data_mode);
	
      case DRIVER_CDROM_DEFAULT:
      case DRIVER_CDROM_FORCED:
	return send_packet_cdrom(dh, cmd, cdb_size, buf, size, sense, data_mode);

      default: 
        Stop("no SCSI driver selected");
	break;
   }

   return -1;
}

#endif /* SYS_LINUX */
