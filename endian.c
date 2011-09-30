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

/***
 *** Conversion between little and big endian words.
 *** Only suitable for non performance critical code.
 */

/* 
 * Swap bytes in a 32 bit word to convert between little and big endian.
 */

guint32 SwapBytes32(guint32 in)
{
  return
        ((in & 0xff000000) >> 24) 
      | ((in & 0x00ff0000) >>  8) 
      | ((in & 0x0000ff00) <<  8) 
      | ((in & 0x000000ff) << 24);
}

guint64 SwapBytes64(guint64 in)
{
  return
        ((in & 0xff00000000000000ull) >> 56)
      | ((in & 0x00ff000000000000ull) >> 40)
      | ((in & 0x0000ff0000000000ull) >> 24)
      | ((in & 0x000000ff00000000ull) >>  8)
      | ((in & 0x00000000ff000000ull) <<  8)
      | ((in & 0x0000000000ff0000ull) << 24)
      | ((in & 0x000000000000ff00ull) << 40)
      | ((in & 0x00000000000000ffull) << 56);
}

/***
 *** Convert the EccHeader structure between different endians.
 ***/

/*
 * A debugging function for printing the Ecc header.
 */

void print_hex(char *label, guint8 *values, int n)
{  PrintCLI(label);

   while(n--)
     PrintCLI("%02x ",*values++);

   PrintCLI("\n");
}

void print_ecc_header(EccHeader *eh)
{  char buf[16]; 

   PrintCLI("\nContents of EccHeader:\n\n");

   strncpy(buf, (char*)eh->cookie, 12); buf[12] = 0;
   PrintCLI("cookie           %s\n",buf);
   strncpy(buf, (char*)eh->method, 4);  buf[4] = 0;
   PrintCLI("method           %s\n",buf);
   print_hex("methodFlags      ", (guint8*)eh->methodFlags, 4);
   print_hex("mediumFP         ", eh->mediumFP, 16);
   print_hex("mediumSum        ", eh->mediumSum, 16);
   print_hex("eccSum           ", eh->eccSum, 16);
   print_hex("sectors          ", eh->sectors, 8);
   PrintCLI("sectors (native) %lld\n", uchar_to_gint64(eh->sectors));
   PrintCLI("dataBytes        %8x\n", eh->dataBytes);
   PrintCLI("eccBytes         %8x\n", eh->eccBytes);
   PrintCLI("creatorVersion   %8x\n", eh->creatorVersion);
   PrintCLI("neededVersion    %8x\n", eh->neededVersion);
   PrintCLI("fpSector         %8x\n", eh->fpSector);
   PrintCLI("selfCRC          %8x\n", eh->selfCRC);
   print_hex("crcSum           ", eh->crcSum, 16);
   PrintCLI("inLast           %8x\n", eh->inLast);

   PrintCLI("\n");
}

/*
 * This is the most annoying part of the endian conversions.
 */

//#define VERBOSE

void SwapEccHeaderBytes(EccHeader *eh)
{
#ifdef VERBOSE
  printf("before swap:\n");
  print_ecc_header(eh);
#endif

  eh->dataBytes = SwapBytes32(eh->dataBytes);
  eh->eccBytes = SwapBytes32(eh->eccBytes);
  eh->creatorVersion = SwapBytes32(eh->creatorVersion);
  eh->neededVersion = SwapBytes32(eh->neededVersion);
  eh->fpSector = SwapBytes32(eh->fpSector);
  eh->inLast = SwapBytes32(eh->inLast);
#ifdef VERBOSE
  printf("after swap:\n");
  print_ecc_header(eh);
#endif
}

void SwapDefectiveHeaderBytes(DefectiveSectorHeader *dsh)
{  
  dsh->lba        = SwapBytes64(dsh->lba);
  dsh->sectorSize = SwapBytes32(dsh->sectorSize);
  dsh->properties = SwapBytes32(dsh->properties);
  dsh->dshFormat  = SwapBytes32(dsh->dshFormat);
  dsh->nSectors   = SwapBytes32(dsh->nSectors);
}
