/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2004-2015 Carsten Gnoerlich.
 *  CRC32 code based upon public domain code by Ross Williams (see notes below)  
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

/***
 *** Crc32 used in the dvdisaster error correction data
 ***/

/*****************************************************************/
/*                                                               */
/* CRC LOOKUP TABLE                                              */
/* ================                                              */
/* The following CRC lookup table was generated automagically    */
/* by the Rocksoft^tm Model CRC Algorithm Table Generation       */
/* Program V1.0 using the following model parameters:            */
/*                                                               */
/*    Width   : 4 bytes.                                         */
/*    Poly    : 0x04C11DB7L                                      */
/*    Reverse : TRUE.                                            */
/*                                                               */
/* For more information on the Rocksoft^tm Model CRC Algorithm,  */
/* see the document titled "A Painless Guide to CRC Error        */
/* Detection Algorithms" by Ross Williams                        */
/* (ross@guest.adelaide.edu.au.). This document is likely to be  */
/* in the FTP archive "ftp.adelaide.edu.au/pub/rocksoft".        */
/*                                                               */
/*****************************************************************/

static guint32 crctable[256] =
{
 0x00000000L, 0x77073096L, 0xEE0E612CL, 0x990951BAL,
 0x076DC419L, 0x706AF48FL, 0xE963A535L, 0x9E6495A3L,
 0x0EDB8832L, 0x79DCB8A4L, 0xE0D5E91EL, 0x97D2D988L,
 0x09B64C2BL, 0x7EB17CBDL, 0xE7B82D07L, 0x90BF1D91L,
 0x1DB71064L, 0x6AB020F2L, 0xF3B97148L, 0x84BE41DEL,
 0x1ADAD47DL, 0x6DDDE4EBL, 0xF4D4B551L, 0x83D385C7L,
 0x136C9856L, 0x646BA8C0L, 0xFD62F97AL, 0x8A65C9ECL,
 0x14015C4FL, 0x63066CD9L, 0xFA0F3D63L, 0x8D080DF5L,
 0x3B6E20C8L, 0x4C69105EL, 0xD56041E4L, 0xA2677172L,
 0x3C03E4D1L, 0x4B04D447L, 0xD20D85FDL, 0xA50AB56BL,
 0x35B5A8FAL, 0x42B2986CL, 0xDBBBC9D6L, 0xACBCF940L,
 0x32D86CE3L, 0x45DF5C75L, 0xDCD60DCFL, 0xABD13D59L,
 0x26D930ACL, 0x51DE003AL, 0xC8D75180L, 0xBFD06116L,
 0x21B4F4B5L, 0x56B3C423L, 0xCFBA9599L, 0xB8BDA50FL,
 0x2802B89EL, 0x5F058808L, 0xC60CD9B2L, 0xB10BE924L,
 0x2F6F7C87L, 0x58684C11L, 0xC1611DABL, 0xB6662D3DL,
 0x76DC4190L, 0x01DB7106L, 0x98D220BCL, 0xEFD5102AL,
 0x71B18589L, 0x06B6B51FL, 0x9FBFE4A5L, 0xE8B8D433L,
 0x7807C9A2L, 0x0F00F934L, 0x9609A88EL, 0xE10E9818L,
 0x7F6A0DBBL, 0x086D3D2DL, 0x91646C97L, 0xE6635C01L,
 0x6B6B51F4L, 0x1C6C6162L, 0x856530D8L, 0xF262004EL,
 0x6C0695EDL, 0x1B01A57BL, 0x8208F4C1L, 0xF50FC457L,
 0x65B0D9C6L, 0x12B7E950L, 0x8BBEB8EAL, 0xFCB9887CL,
 0x62DD1DDFL, 0x15DA2D49L, 0x8CD37CF3L, 0xFBD44C65L,
 0x4DB26158L, 0x3AB551CEL, 0xA3BC0074L, 0xD4BB30E2L,
 0x4ADFA541L, 0x3DD895D7L, 0xA4D1C46DL, 0xD3D6F4FBL,
 0x4369E96AL, 0x346ED9FCL, 0xAD678846L, 0xDA60B8D0L,
 0x44042D73L, 0x33031DE5L, 0xAA0A4C5FL, 0xDD0D7CC9L,
 0x5005713CL, 0x270241AAL, 0xBE0B1010L, 0xC90C2086L,
 0x5768B525L, 0x206F85B3L, 0xB966D409L, 0xCE61E49FL,
 0x5EDEF90EL, 0x29D9C998L, 0xB0D09822L, 0xC7D7A8B4L,
 0x59B33D17L, 0x2EB40D81L, 0xB7BD5C3BL, 0xC0BA6CADL,
 0xEDB88320L, 0x9ABFB3B6L, 0x03B6E20CL, 0x74B1D29AL,
 0xEAD54739L, 0x9DD277AFL, 0x04DB2615L, 0x73DC1683L,
 0xE3630B12L, 0x94643B84L, 0x0D6D6A3EL, 0x7A6A5AA8L,
 0xE40ECF0BL, 0x9309FF9DL, 0x0A00AE27L, 0x7D079EB1L,
 0xF00F9344L, 0x8708A3D2L, 0x1E01F268L, 0x6906C2FEL,
 0xF762575DL, 0x806567CBL, 0x196C3671L, 0x6E6B06E7L,
 0xFED41B76L, 0x89D32BE0L, 0x10DA7A5AL, 0x67DD4ACCL,
 0xF9B9DF6FL, 0x8EBEEFF9L, 0x17B7BE43L, 0x60B08ED5L,
 0xD6D6A3E8L, 0xA1D1937EL, 0x38D8C2C4L, 0x4FDFF252L,
 0xD1BB67F1L, 0xA6BC5767L, 0x3FB506DDL, 0x48B2364BL,
 0xD80D2BDAL, 0xAF0A1B4CL, 0x36034AF6L, 0x41047A60L,
 0xDF60EFC3L, 0xA867DF55L, 0x316E8EEFL, 0x4669BE79L,
 0xCB61B38CL, 0xBC66831AL, 0x256FD2A0L, 0x5268E236L,
 0xCC0C7795L, 0xBB0B4703L, 0x220216B9L, 0x5505262FL,
 0xC5BA3BBEL, 0xB2BD0B28L, 0x2BB45A92L, 0x5CB36A04L,
 0xC2D7FFA7L, 0xB5D0CF31L, 0x2CD99E8BL, 0x5BDEAE1DL,
 0x9B64C2B0L, 0xEC63F226L, 0x756AA39CL, 0x026D930AL,
 0x9C0906A9L, 0xEB0E363FL, 0x72076785L, 0x05005713L,
 0x95BF4A82L, 0xE2B87A14L, 0x7BB12BAEL, 0x0CB61B38L,
 0x92D28E9BL, 0xE5D5BE0DL, 0x7CDCEFB7L, 0x0BDBDF21L,
 0x86D3D2D4L, 0xF1D4E242L, 0x68DDB3F8L, 0x1FDA836EL,
 0x81BE16CDL, 0xF6B9265BL, 0x6FB077E1L, 0x18B74777L,
 0x88085AE6L, 0xFF0F6A70L, 0x66063BCAL, 0x11010B5CL,
 0x8F659EFFL, 0xF862AE69L, 0x616BFFD3L, 0x166CCF45L,
 0xA00AE278L, 0xD70DD2EEL, 0x4E048354L, 0x3903B3C2L,
 0xA7672661L, 0xD06016F7L, 0x4969474DL, 0x3E6E77DBL,
 0xAED16A4AL, 0xD9D65ADCL, 0x40DF0B66L, 0x37D83BF0L,
 0xA9BCAE53L, 0xDEBB9EC5L, 0x47B2CF7FL, 0x30B5FFE9L,
 0xBDBDF21CL, 0xCABAC28AL, 0x53B39330L, 0x24B4A3A6L,
 0xBAD03605L, 0xCDD70693L, 0x54DE5729L, 0x23D967BFL,
 0xB3667A2EL, 0xC4614AB8L, 0x5D681B02L, 0x2A6F2B94L,
 0xB40BBE37L, 0xC30C8EA1L, 0x5A05DF1BL, 0x2D02EF8DL
};

/*
 * The table-based CRC32 algorithm
 *
 * Note that endianess does not matter for the internal calculations,
 * but the final CRC sum will be returned in little endian format
 * so that comparing against the sums in the ecc file does not need
 * to be endian-aware.
 */ 

guint32 Crc32(unsigned char *data, int len)
{  guint32 crc = ~0;

   while(len--)
      crc = crctable[(crc ^ *data++) & 0xFF] ^ (crc >> 8);

#ifdef HAVE_BIG_ENDIAN
   crc = SwapBytes32(crc);
#endif

   return crc;
}

/***
 *** EDC checksum used in CDROM sectors
 ***/

/*****************************************************************/
/*                                                               */
/* CRC LOOKUP TABLE                                              */
/* ================                                              */
/* The following CRC lookup table was generated automagically    */
/* by the Rocksoft^tm Model CRC Algorithm Table Generation       */
/* Program V1.0 using the following model parameters:            */
/*                                                               */
/*    Width   : 4 bytes.                                         */
/*    Poly    : 0x8001801BL                                      */
/*    Reverse : TRUE.                                            */
/*                                                               */
/* For more information on the Rocksoft^tm Model CRC Algorithm,  */
/* see the document titled "A Painless Guide to CRC Error        */
/* Detection Algorithms" by Ross Williams                        */
/* (ross@guest.adelaide.edu.au.). This document is likely to be  */
/* in the FTP archive "ftp.adelaide.edu.au/pub/rocksoft".        */
/*                                                               */
/*****************************************************************/

unsigned long  edctable[256] =
{
 0x00000000L, 0x90910101L, 0x91210201L, 0x01B00300L,
 0x92410401L, 0x02D00500L, 0x03600600L, 0x93F10701L,
 0x94810801L, 0x04100900L, 0x05A00A00L, 0x95310B01L,
 0x06C00C00L, 0x96510D01L, 0x97E10E01L, 0x07700F00L,
 0x99011001L, 0x09901100L, 0x08201200L, 0x98B11301L,
 0x0B401400L, 0x9BD11501L, 0x9A611601L, 0x0AF01700L,
 0x0D801800L, 0x9D111901L, 0x9CA11A01L, 0x0C301B00L,
 0x9FC11C01L, 0x0F501D00L, 0x0EE01E00L, 0x9E711F01L,
 0x82012001L, 0x12902100L, 0x13202200L, 0x83B12301L,
 0x10402400L, 0x80D12501L, 0x81612601L, 0x11F02700L,
 0x16802800L, 0x86112901L, 0x87A12A01L, 0x17302B00L,
 0x84C12C01L, 0x14502D00L, 0x15E02E00L, 0x85712F01L,
 0x1B003000L, 0x8B913101L, 0x8A213201L, 0x1AB03300L,
 0x89413401L, 0x19D03500L, 0x18603600L, 0x88F13701L,
 0x8F813801L, 0x1F103900L, 0x1EA03A00L, 0x8E313B01L,
 0x1DC03C00L, 0x8D513D01L, 0x8CE13E01L, 0x1C703F00L,
 0xB4014001L, 0x24904100L, 0x25204200L, 0xB5B14301L,
 0x26404400L, 0xB6D14501L, 0xB7614601L, 0x27F04700L,
 0x20804800L, 0xB0114901L, 0xB1A14A01L, 0x21304B00L,
 0xB2C14C01L, 0x22504D00L, 0x23E04E00L, 0xB3714F01L,
 0x2D005000L, 0xBD915101L, 0xBC215201L, 0x2CB05300L,
 0xBF415401L, 0x2FD05500L, 0x2E605600L, 0xBEF15701L,
 0xB9815801L, 0x29105900L, 0x28A05A00L, 0xB8315B01L,
 0x2BC05C00L, 0xBB515D01L, 0xBAE15E01L, 0x2A705F00L,
 0x36006000L, 0xA6916101L, 0xA7216201L, 0x37B06300L,
 0xA4416401L, 0x34D06500L, 0x35606600L, 0xA5F16701L,
 0xA2816801L, 0x32106900L, 0x33A06A00L, 0xA3316B01L,
 0x30C06C00L, 0xA0516D01L, 0xA1E16E01L, 0x31706F00L,
 0xAF017001L, 0x3F907100L, 0x3E207200L, 0xAEB17301L,
 0x3D407400L, 0xADD17501L, 0xAC617601L, 0x3CF07700L,
 0x3B807800L, 0xAB117901L, 0xAAA17A01L, 0x3A307B00L,
 0xA9C17C01L, 0x39507D00L, 0x38E07E00L, 0xA8717F01L,
 0xD8018001L, 0x48908100L, 0x49208200L, 0xD9B18301L,
 0x4A408400L, 0xDAD18501L, 0xDB618601L, 0x4BF08700L,
 0x4C808800L, 0xDC118901L, 0xDDA18A01L, 0x4D308B00L,
 0xDEC18C01L, 0x4E508D00L, 0x4FE08E00L, 0xDF718F01L,
 0x41009000L, 0xD1919101L, 0xD0219201L, 0x40B09300L,
 0xD3419401L, 0x43D09500L, 0x42609600L, 0xD2F19701L,
 0xD5819801L, 0x45109900L, 0x44A09A00L, 0xD4319B01L,
 0x47C09C00L, 0xD7519D01L, 0xD6E19E01L, 0x46709F00L,
 0x5A00A000L, 0xCA91A101L, 0xCB21A201L, 0x5BB0A300L,
 0xC841A401L, 0x58D0A500L, 0x5960A600L, 0xC9F1A701L,
 0xCE81A801L, 0x5E10A900L, 0x5FA0AA00L, 0xCF31AB01L,
 0x5CC0AC00L, 0xCC51AD01L, 0xCDE1AE01L, 0x5D70AF00L,
 0xC301B001L, 0x5390B100L, 0x5220B200L, 0xC2B1B301L,
 0x5140B400L, 0xC1D1B501L, 0xC061B601L, 0x50F0B700L,
 0x5780B800L, 0xC711B901L, 0xC6A1BA01L, 0x5630BB00L,
 0xC5C1BC01L, 0x5550BD00L, 0x54E0BE00L, 0xC471BF01L,
 0x6C00C000L, 0xFC91C101L, 0xFD21C201L, 0x6DB0C300L,
 0xFE41C401L, 0x6ED0C500L, 0x6F60C600L, 0xFFF1C701L,
 0xF881C801L, 0x6810C900L, 0x69A0CA00L, 0xF931CB01L,
 0x6AC0CC00L, 0xFA51CD01L, 0xFBE1CE01L, 0x6B70CF00L,
 0xF501D001L, 0x6590D100L, 0x6420D200L, 0xF4B1D301L,
 0x6740D400L, 0xF7D1D501L, 0xF661D601L, 0x66F0D700L,
 0x6180D800L, 0xF111D901L, 0xF0A1DA01L, 0x6030DB00L,
 0xF3C1DC01L, 0x6350DD00L, 0x62E0DE00L, 0xF271DF01L,
 0xEE01E001L, 0x7E90E100L, 0x7F20E200L, 0xEFB1E301L,
 0x7C40E400L, 0xECD1E501L, 0xED61E601L, 0x7DF0E700L,
 0x7A80E800L, 0xEA11E901L, 0xEBA1EA01L, 0x7B30EB00L,
 0xE8C1EC01L, 0x7850ED00L, 0x79E0EE00L, 0xE971EF01L,
 0x7700F000L, 0xE791F101L, 0xE621F201L, 0x76B0F300L,
 0xE541F401L, 0x75D0F500L, 0x7460F600L, 0xE4F1F701L,
 0xE381F801L, 0x7310F900L, 0x72A0FA00L, 0xE231FB01L,
 0x71C0FC00L, 0xE151FD01L, 0xE0E1FE01L, 0x7070FF00L
};

/*
 * CDROM EDC calculation
 */

guint32 EDCCrc32(unsigned char *data, int len)
{  guint32 crc = 0;

   while(len--)
     crc = edctable[(crc ^ *data++) & 0xFF] ^ (crc >> 8);

#ifdef HAVE_BIG_ENDIAN
   crc = SwapBytes32(crc);
#endif

   return crc;
}
