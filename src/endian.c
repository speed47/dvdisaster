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
{  PrintCLI("%s", label);

   while(n--)
     PrintCLI("%02x ",*values++);

   PrintCLI("\n");
}

void PrintEccHeader(EccHeader *eh)
{  char buf[16]; 

  PrintCLI(_("\nContents of Ecc Header:\n\n"));

   strncpy(buf, (char*)eh->cookie, 12); buf[12] = 0;
   PrintCLI("cookie           %s\n",buf);
   strncpy(buf, (char*)eh->method, 4);  buf[4] = 0;
   PrintCLI("method           %s\n",buf);
   print_hex("methodFlags      ", (guint8*)eh->methodFlags, 4);
   print_hex("mediumFP         ", eh->mediumFP, 16);
   print_hex("mediumSum        ", eh->mediumSum, 16);
   print_hex("eccSum           ", eh->eccSum, 16);
   print_hex("sectors          ", eh->sectors, 8);
   PrintCLI("sectors (native)  %" PRId64 "\n", uchar_to_gint64(eh->sectors));
   PrintCLI("dataBytes         %8x\n", eh->dataBytes);
   PrintCLI("eccBytes          %8x\n", eh->eccBytes);
   PrintCLI("creatorVersion    %8x\n", eh->creatorVersion);
   PrintCLI("neededVersion     %8x\n", eh->neededVersion);
   PrintCLI("fpSector          %8x\n", eh->fpSector);
   PrintCLI("selfCRC           %8x\n", eh->selfCRC);
   print_hex("crcSum            ", eh->crcSum, 16);
   PrintCLI("inLast            %8x\n", eh->inLast);
   PrintCLI("sectorsPerLayer   %" PRId64 "\n", eh->sectorsPerLayer);
   PrintCLI("sectorsAddedByEcc %" PRId64 "\n", eh->sectorsAddedByEcc);

   PrintCLI("\n");
}

void print_crc_block(CrcBlock *cb)
{  char buf[16]; 

   PrintCLI("\nContents of CrcBlock:\n\n");

   strncpy(buf, (char*)cb->cookie, 12); buf[12] = 0;
   PrintCLI("cookie           %s\n",buf);
   strncpy(buf, (char*)cb->method, 4);  buf[4] = 0;
   PrintCLI("method           %s\n",buf);
   print_hex("methodFlags      ", (guint8*)cb->methodFlags, 4);
   PrintCLI("creatorVersion   %8x\n", cb->creatorVersion);
   PrintCLI("neededVersion    %8x\n", cb->neededVersion);
   PrintCLI("fpSector         %8x\n", cb->fpSector);
   print_hex("mediumFP         ", cb->mediumFP, 16);
   print_hex("mediumSum        ", cb->mediumSum, 16);
   PrintCLI("dataSectors      %16" PRIx64 "\n",cb->dataSectors);
   PrintCLI("inLast           %8x\n", cb->inLast);
   PrintCLI("dataBytes        %8x\n", cb->dataBytes);
   PrintCLI("eccBytes         %8x\n", cb->eccBytes);
   PrintCLI("sectorsPerLayer  %" PRId64 "\n", cb->sectorsPerLayer);
   PrintCLI("selfCRC          %8x\n", cb->selfCRC);

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
  eh->sectorsPerLayer = SwapBytes64(eh->sectorsPerLayer);
  eh->sectorsAddedByEcc = SwapBytes64(eh->sectorsAddedByEcc);
#ifdef VERBOSE
  printf("after swap:\n");
  print_ecc_header(eh);
#endif
}

void SwapCrcBlockBytes(CrcBlock *cb)
{
#ifdef VERBOSE
  printf("before swap:\n");
  print_crc_block(cb);
#endif

  cb->creatorVersion = SwapBytes32(cb->creatorVersion);
  cb->neededVersion = SwapBytes32(cb->neededVersion);
  cb->fpSector = SwapBytes32(cb->fpSector);
  cb->dataSectors = SwapBytes64(cb->dataSectors);
  cb->inLast = SwapBytes32(cb->inLast);
  cb->dataBytes = SwapBytes32(cb->dataBytes);
  cb->eccBytes = SwapBytes32(cb->eccBytes);
  cb->sectorsPerLayer = SwapBytes64(cb->sectorsPerLayer);
#ifdef VERBOSE
  printf("after swap:\n");
  print_crc_block(cb);
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
