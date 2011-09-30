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
 *** Collect the available methods
 ***/

/*
 * Invite all methods for registration 
 */

void CollectMethods(void)
{
  BindMethods();
}

/*
 * All methods register by calling this 
 */

void RegisterMethod(Method *method)
{
  g_ptr_array_add(Closure->methodList, method);
}

/*
 * List the available methods
 */

void ListMethods(void)
{  char name[5];
   unsigned int i;

   PrintCLI(_("\nList of available methods:\n\n"));
   name[4] = 0;
 
   for(i=0; i<Closure->methodList->len; i++)
   {  Method *method = g_ptr_array_index(Closure->methodList, i);

      strncpy(name, method->name, 4);
      PrintCLI("%s -- %s\n",name,method->description);
   }
}

/*
 * Call the method destructors
 */

void CallMethodDestructors(void)
{  unsigned int i;

   for(i=0; i<Closure->methodList->len; i++)  
   {  Method *method = g_ptr_array_index(Closure->methodList, i);
 
      method->destroy(method);
      if(method->menuEntry) g_free(method->menuEntry);
      if(method->description) g_free(method->description);
      if(method->lastEh)
	g_free(method->lastEh);
   }
}

/***
 *** Determine methods from name or ecc information
 ***/

/*
 * Find a method by name
 */

Method *FindMethod(char *name)
{  unsigned int i;

   for(i=0; i<Closure->methodList->len; i++) 
   {  Method *method = g_ptr_array_index(Closure->methodList, i);

      if(!strncmp(method->name, name, 4))
        return method;
   }

   return NULL;
}

/*
 * Search for ecc headers in RS02 style image files.
 * Note that udf.c has a similar function FindHeaderInMedium().
 */

static int read_fingerprint(LargeFile *file, unsigned char *fingerprint, gint64 sector)
{  struct MD5Context md5ctxt;
   unsigned char buf[2048];
   int n;

   if(!LargeSeek(file, 2048LL*sector))
     return FALSE;

   n = LargeRead(file, buf, 2048);

   if(n != 2048) return FALSE;

   if(CheckForMissingSector(buf, sector, NULL, 0) != SECTOR_PRESENT)
     return FALSE;

   MD5Init(&md5ctxt);
   MD5Update(&md5ctxt, buf, 2048);
   MD5Final(fingerprint, &md5ctxt);

   return TRUE;
}

EccHeader* FindHeaderInImage(char *filename)
{  EccHeader *eh = NULL;
   LargeFile *file;
   unsigned char buf[4096];
   gint64 length,sectors,pos;
   gint64 header_modulo;
   gint64 last_fp = -1;
   unsigned char fingerprint[16];

   if(!LargeStat(filename, &length))
     return NULL;

   file = LargeOpen(filename, O_RDONLY, IMG_PERMS);
   if(!file) return NULL;

   header_modulo = (gint64)1<<62;
   sectors = length / 2048;

   /*** Search for the headers */

   while(header_modulo >= 32)
   {  pos = sectors & ~(header_modulo - 1);

//printf("Trying modulo %lld\n", header_modulo);

      while(pos > 0)
      {  if(LargeSeek(file, 2048*pos))
	 {  int n;

//printf(" trying sector %lld\n", pos);
	    n = LargeRead(file, buf, sizeof(EccHeader));

	    if(n != sizeof(EccHeader))
	      goto check_next_header;

	    eh = (EccHeader*)buf;

	    /* Medium read error in ecc header? */

	    if(   (CheckForMissingSector(buf, pos, NULL, 0) != SECTOR_PRESENT)
	       || (CheckForMissingSector(buf+2048, pos+1, NULL, 0) != SECTOR_PRESENT))
	    {  
//printf(" header at %lld: read error\n", (long long int)pos);
	       goto check_next_header;
	    }

	    /* See if the magic cookie is there */

	    if(!strncmp((char*)eh->cookie, "*dvdisaster*", 12))
	    {  guint32 recorded_crc = eh->selfCRC;
	       guint32 real_crc;

//printf(" header at %lld: magic cookie found\n", (long long int)pos);

#ifdef HAVE_BIG_ENDIAN
	       eh->selfCRC = 0x47504c00;
#else
	       eh->selfCRC = 0x4c5047;
#endif
	       real_crc = Crc32((unsigned char*)eh, sizeof(EccHeader));

	       if(real_crc == recorded_crc)
	       {  eh = g_malloc(sizeof(EccHeader));
		  memcpy(eh, buf, sizeof(EccHeader));
#ifdef HAVE_BIG_ENDIAN
		  SwapEccHeaderBytes(eh);
#endif
		  eh->selfCRC = recorded_crc;
//printf(" --> CRC okay, using it\n");

		  if(last_fp != eh->fpSector)
		  {  int status;

		     status = read_fingerprint(file, fingerprint, eh->fpSector);
		     last_fp = eh->fpSector;

		     if(!status)  /* be optimistic if fingerprint sector is unreadable */
		     {  LargeClose(file);
		        return eh;
		     }
		  }

		  if(!memcmp(fingerprint, eh->mediumFP, 16))  /* good fingerprint */
		  {  LargeClose(file);
		     return eh;
		  }

		  /* might be a header from a larger previous session.
		     discard it and continue */

		  g_free(eh);
	       }
//printf(" CRC failed, skipping it\n");
	       goto check_next_header;
	    }
	    else
	    {
//printf(" no cookie, skipping current modulo\n");
	      goto check_next_modulo;
	    }
	 }

      check_next_header:
	pos -= header_modulo;
      }

   check_next_modulo:
      header_modulo >>= 1;
   }

   LargeClose(file);
   return NULL;
}

/*
 * Find method for a given ecc file (like in RS01)
 * or augmented image (like in the RS02 image format).
 * Since locating the header is expensive in the RS02 case,
 * it is cached in the corresponding Method struct.
 */

Method *EccFileMethod(int process_error)
{  LargeFile *ecc_file = NULL;
   Method *method;
   EccHeader *eh;
   char method_name[5];
   gint64 length;

   /* First see if an ecc file is available */

   method_name[0] = 0;

   if((ecc_file = LargeOpen(Closure->eccName, O_RDONLY, 0)))
   {  EccHeader eh;
      int n;

      n = LargeRead(ecc_file, &eh, sizeof(EccHeader));
      LargeClose(ecc_file);

      if(n != sizeof(EccHeader))
	goto no_ecc_file;

      if(strncmp((char*)eh.cookie, "*dvdisaster*", 12))
	goto no_ecc_file;

      memcpy(method_name, eh.method, 4); method_name[4] = 0;

      if((method = FindMethod(method_name)))
	return method;
   }

   /* No ecc file, see if the image contains hidden ecc information */

no_ecc_file:
   if(!LargeStat(Closure->imageName, &length))
   {  if(process_error)
      {  if(Closure->guiMode)
	      CreateMessage(_("Image file %s not present.\n"), GTK_MESSAGE_ERROR, Closure->imageName, strerror(errno));
         else Stop(_("Image file %s not present.\n"), Closure->imageName, strerror(errno));
      }
      return NULL;
   }

   eh = FindHeaderInImage(Closure->imageName);

   if(eh)
   {  memcpy(method_name, eh->method, 4); method_name[4] = 0;

      if((method = FindMethod(method_name)))
      {  if(method->lastEh) g_free(method->lastEh);
	 method->lastEh = eh;

	 return method;
      }
      g_free(eh);
   }

   /* No ecc augmented image */

   if(process_error)
   {  if(Closure->guiMode)
      {  if(method_name[0])
	      CreateMessage(_("\nUnknown method %s.\n"), GTK_MESSAGE_ERROR, method_name);
         else CreateMessage(_("\nNeither ecc file nor ecc data in image found.\n"), GTK_MESSAGE_ERROR);
      }
      else
      {  if(method_name[0])
	      Stop(_("\nUnknown method %s.\n"), method_name);
         else Stop(_("\nNeither ecc file nor ecc data in image found.\n"));
      }
   }

   return NULL;
}

