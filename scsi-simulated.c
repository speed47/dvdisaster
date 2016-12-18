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

/***
 *** Add a menu entry for a simulated CD drive.
 ***/

void InitSimulatedCD()
{  
   if(!Closure->simulateCD)
      return;

   g_ptr_array_add(Closure->deviceNodes, g_strdup("sim-cd"));
   g_ptr_array_add(Closure->deviceNames, g_strdup_printf(_("Simulated CD (%s)"), Closure->simulateCD));
}

/***
 *** Simulate the SCSI device
 ***/

/*
 * While we're at it, check the CDB interface for proper usage.
 */

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

static void write_sense(Sense *sense, int sense_key, int asc, int ascq)
{
  sense->sense_key=sense_key;
  sense->asc=asc;
  sense->ascq=ascq;
}

int SimulateSendPacket(DeviceHandle *dh, unsigned char *cdb, int cdb_size, unsigned char *out_buf, int size, Sense *sense, int direction)
{  unsigned char buf[2048];
   int real_size;
   int alloc_len;

   switch(cdb[0])
   {  

      case 0x00: assert_cdb_length(cdb[0], cdb_size, 6);   /* TEST UNIT READY */
                 assert_cdb_direction(cdb[0], DATA_NONE, direction);

		 return 0;
	 break;

      case 0x12: assert_cdb_length(cdb[0], cdb_size, 6);   /* INQUIRY */
                 assert_cdb_direction(cdb[0], DATA_READ, direction);

		 real_size = (size < 36 ? size : 36);
		 
		 buf[0] = 0x05;          /* PERIPHERAL DEVICE TYPE */
		 buf[4] = real_size-4;   /* ADDITIONAL LENGTH */

		 memcpy(&buf[8], "Simulate", 8);           /* VENDOR ID */
		 memcpy(&buf[16],"d CD drive       ", 16); /* PRODUCT ID */
		 memcpy(&buf[32],"1.00", 4);               /* VERSION */

		 memcpy(out_buf, buf, real_size);
		 return real_size;
         break;
#if 0
      case 0x1b: assert_cdb_length(cdb[0], cdb_size, 6);   /* START STOP */
                 assert_cdb_direction(cdb[0], DATA_NONE, direction);
	 break;
      case 0x1e: assert_cdb_length(cdb[0], cdb_size, 6);   /* PREVENT ALLOW MEDIUM REMOVAL */
                 assert_cdb_direction(cdb[0], DATA_NONE, direction);
	 break;
      case 0x23: assert_cdb_length(cdb[0], cdb_size, 10);  /* READ FORMAT CAPACITIES */
                 assert_cdb_direction(cdb[0], DATA_READ, direction);
	 break;
#endif

      case 0x25: assert_cdb_length(cdb[0], cdb_size, 10);  /* READ CAPACITY */
                 assert_cdb_direction(cdb[0], DATA_READ, direction);
	      {	 guint64 sectors;	 

		 sectors = dh->simImage->size/2048 - 1;
		 memset(buf, 0, 16);
		 buf[0] = (sectors >> 24) & 0xff;		 
		 buf[1] = (sectors >> 16) & 0xff;
		 buf[2] = (sectors >> 8 ) & 0xff;
		 buf[3] = sectors & 0xff;

		 buf[6] = (2048>>8)&0xff;
		 memcpy(out_buf, buf, size);
		 return 0;
	      }
	 break;

#if 0
      case 0x28: assert_cdb_length(cdb[0], cdb_size, 10);  /* READ(10) */
                 assert_cdb_direction(cdb[0], DATA_READ, direction);
	 break;
#endif

      case 0x43: assert_cdb_length(cdb[0], cdb_size, 10);  /* READ TOC/PMA/ATIP */
                 assert_cdb_direction(cdb[0], DATA_READ, direction);
	   
		 alloc_len = cdb[7]<<8 | cdb[8];
		 switch(cdb[2] & 0xf)  /* format field */
		 {  case 0x00:         /* formatted TOC */
		       memset(out_buf, 0, 12);
		       out_buf[0] = 0;
		       out_buf[1] = 10;    /* data length */
		       out_buf[5] = 0x14;  /* CONTROL (data CD) */
		       return 0;

		    case 0x02:         /* full/raw TOC */
		       memset(out_buf, 0, 26);
		       out_buf[0] = 0;
		       out_buf[1] = 24;    /* data length for two track desciptors*/
		       out_buf[3] = 1;     /* 1 session (= last session) */ 
		       out_buf[7] = 0xa0;  /* POINT: first track number */
		       out_buf[13] = 0;    /* disc type: mode 1 */
		       return 0;

		    case 0x04:         /* full ATIP */
		       memset(out_buf, 0, 16);
		       out_buf[0] = 0;
		       out_buf[1] = 16;
		       out_buf[ 8] = 0x61;
		       out_buf[ 9] = 0x1a;
		       out_buf[10] = 0x41;
		       out_buf[11] = 0x00;
		       out_buf[12] = 0x4f;
		       out_buf[13] = 0x3b;
		       out_buf[14] = 0x4a;
		       return 0;

		    default:
		       printf("Simulation of READ TOC failed for format %d\n", cdb[2] & 0xf);
		       return -1;
		 }
		       
	 break;

      case 0x46: assert_cdb_length(cdb[0], cdb_size, 10);  /* GET CONFIGURATION */
                 assert_cdb_direction(cdb[0], DATA_READ, direction);

		 alloc_len = cdb[7]<<8 | cdb[8];

		 memset(out_buf, 0, 8);
		 out_buf[3] = 8;  /* data length */
		 out_buf[7] = 9;  /* CD-R profile */
		 return 0;
	 break;

      case 0x51: assert_cdb_length(cdb[0], cdb_size, 10);  /* READ DISC INFORMATION */
                 assert_cdb_direction(cdb[0], DATA_READ, direction);

		 alloc_len = cdb[7]<<8 | cdb[8];

		 memset(buf, 0, 32);
		 buf[0] = 0;
		 buf[1] = 32;   /* We are a strange CD without OPC tables. */
		 buf[2] = 0x1e; /* finalized and complete */
		 buf[8] = 0;    /* DATA1 format */

		 memcpy(out_buf, buf, alloc_len);
		 return 0;
	 break;

#if 0
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
#endif

      case 0xbe: assert_cdb_length(cdb[0], cdb_size, 12);  /* READ CD */
                 assert_cdb_direction(cdb[0], DATA_READ, direction);
	   {     guint64 lba,last_pos;
	         useconds_t delay;
		 int fact;
		 int i;

		 if(cdb[9] & ~16)  /* We can only deliver user data */
		 {  write_sense(sense, 5, 0xff, 0xe0);
		    return -1;
		 }

		 lba = cdb[2] << 24 | cdb[3] << 16 | cdb[4] << 8 | cdb[5];
		 alloc_len = cdb[6] << 16 | cdb[7] << 8 | cdb[8];

		 last_pos = 2048*lba + 2048*alloc_len - 1;
		 if(last_pos > dh->simImage->size)
		 {  write_sense(sense, 5, 0x21, 0x00);  /* Illegal LBA */
		    return -1;
		 }

		 if(!LargeSeek(dh->simImage, 2048*lba))
		 {  write_sense(sense, 5, 0x02, 0x00); /* no seek complete */
		    return -1;
		 }

		 if(LargeRead(dh->simImage, out_buf, alloc_len*2048) != alloc_len*2048)
		 {  write_sense(sense, 5, 0x11, 0x00); /* unrecovered read error */
		    return -1;
		 }

		 /* Check for dead sector markers and pass them on
		    as read errors */

		 for(i=0; i<alloc_len; i++)
		 {  int err;

		    err = CheckForMissingSector(out_buf+2048*i, lba+i, NULL, 0);

		    if(err != SECTOR_PRESENT)
		    {  if(err == SECTOR_WITH_SIMULATION_HINT)
		       {  char *sim_hint = GetSimulationHint(out_buf+2048*i);

			  /* simulated hardware error */
			  
			  if(!strcmp(sim_hint, "hardware failure"))
			  {  g_free(sim_hint);
			     memset(out_buf, 0, alloc_len*2048);
			     write_sense(sense, 4, 0x09, 0x02); /* hardware error, focus servo failure  */
			     return -1;
			  }

                          /* pass on dead sector marker from this sector */  

			  if(!strcmp(sim_hint, "pass as dead sector marker"))
			  {  g_free(sim_hint);
			     continue;
			  }

			  /* sector becomes readable in pass 3 */
			  
			  if(!strcmp(sim_hint, "readable in pass 3"))
			  {  g_free(sim_hint);
			    if(dh->pass == 2)
			    {  memset(out_buf+i*2048, 64, 2048);
			       continue;
			    }
			    else
			    {  memset(out_buf, 0, alloc_len*2048);
			       write_sense(sense, 3, 0x11, 0x00); /* unrecovered read error */
			       return -1;
			    }
			  }

			  /* unknown simulation code becomes standard read error */
			  
			  g_free(sim_hint);
			  memset(out_buf, 0, alloc_len*2048);
			  write_sense(sense, 3, 0x11, 0x00); /* unrecovered read error */
			  return -1;
		       }
		       else /* standard read error */
		       {  memset(out_buf, 0, alloc_len*2048);
			  write_sense(sense, 3, 0x11, 0x00); /* unrecovered read error */
			  return -1;
		       }
		    }
		 }

		 fact = (int)(200.0*sin(-0.5+(double)lba/6280.0));
		 delay = (500+fact)*alloc_len;
		 usleep(delay);
	   }
	         return 0;
	 break;

      default:
	 printf("SendPacket(): Unknown opcode %0x\n", cdb[0]);
	 PrintLog("SendPacket(): Unknown opcode %0x\n", cdb[0]);
	 return -1;
   }
}


