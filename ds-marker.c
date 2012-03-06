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

#define DSM_VERSION "1.00"

/***
 *** Create an unique marker for missing sectors
 ***/

void CreateMissingSector(unsigned char *out, gint64 sector, 
			 unsigned char *fingerprint, gint64 fingerprint_sector,
			 char *volume_label)
{  char *buf = (char*)out;
   char *end_marker;
   int end_length; 

   /* Bytefill requested? */

   if(Closure->fillUnreadable >= 0)
   {  memset(out, Closure->fillUnreadable, 2048);
      return;
   }

   /* historic words ... ;-) */

   memset(buf, 0, 2048);

   g_sprintf(buf,
 	     "dvdisaster dead sector marker\n"
	     "This sector could not be read from the image.\n"
	     "Its contents have been substituted by the dvdisaster read routine.\n");

   end_marker = "dvdisaster dead sector end marker\n";
   end_length = strlen(end_marker);
   memcpy(buf+2046-end_length, end_marker, end_length); 

   /* May we use the new marker features? */

   if(!Closure->dsmVersion)
      return;

   /* make dsm marker unique for this sector and medium */

   g_sprintf(buf+0x100,"Dead sector marker version");
   g_sprintf(buf+0x120,"%s",DSM_VERSION);
   g_sprintf(buf+0x140,"Dead sector number");
   g_sprintf(buf+0x160,"%lld", (long long)sector);
   g_sprintf(buf+0x180,"Medium fingerprint");
   if(fingerprint) memcpy(buf+0x1a0, fingerprint, 16);
   else            memcpy(buf+0x1b0, "none", 4);
   g_sprintf(buf+0x1c0,"Medium fingerprint sector");
   g_sprintf(buf+0x1e0,"%lld", (long long)fingerprint_sector);
   g_sprintf(buf+0x200,"Volume label (if any)");
   g_sprintf(buf+0x220,"%s", volume_label ? volume_label : "none");
}

/***
 *** helper function
 ***/

static int get_recorded_number(unsigned char *buf, gint64 *number)
{
   if(!strcmp((char*)buf+0x140, "Dead sector number"))
   {  *number = strtoll((char*)buf+0x160, NULL, 10);
      return TRUE;
   }

   *number = 0;
   return FALSE;   
}

static char *get_volume_label(unsigned char *buf)
{
   if(!strcmp((char*)buf+0x200, "Volume label (if any)"))
   {  if(!strcmp((char*)buf+0x220, "none"))
	   return NULL;
      else return g_strdup((char*)buf+0x220);
   }

   return NULL;
}

/***
 *** Check whether this is a missing sector
 ***/

int CheckForMissingSector(unsigned char *buf, gint64 sector, 
			  unsigned char *fingerprint, gint64 fingerprint_sector)
{  static char pattern[2048];
   static char last_pattern = 0;
   gint64 recorded_number;

   /* Bytefill used as missing sector marker? */
   
   if(Closure->fillUnreadable >= 0)
   {  if(Closure->fillUnreadable != last_pattern)  /* cache the pattern */
	 memset(pattern, Closure->fillUnreadable, 2048);

      if(memcmp(buf, pattern, 2048)) 
	   return SECTOR_PRESENT;
      else return SECTOR_MISSING;
   }

   /* See if it is our dead sector marker */

   if(strcmp((char*)buf, 
	     "dvdisaster dead sector marker\n"
	     "This sector could not be read from the image.\n"
	     "Its contents have been substituted by the dvdisaster read routine.\n")
      || strcmp((char*)buf+2046-34, "dvdisaster dead sector end marker\n"))
      return SECTOR_PRESENT;

   /* New style missing sector marker? */

   if(strcmp((char*)buf+0x100,"Dead sector marker version"))
      return SECTOR_MISSING;

   /*** Evaluate new style sector marker */

   /* Verify sector number */

   if(get_recorded_number(buf, &recorded_number))
      if(recorded_number != sector)
	 return SECTOR_MISSING_DISPLACED;

   /* Verify medium fingerprint. If the dead sector was fingerprinted with 
      a different sector, ignore the test. Retrieving the right fingerprint
      sector is too expensive. */

   if(fingerprint 
      && !strcmp((char*)buf+0x1c0, "Medium fingerprint sector")
      &&  memcmp((char*)buf+0x1b0, "none", 4))
   {  gint64 fps_recorded = strtoll((char*)buf+0x1e0, NULL, 10);

      if(fps_recorded == fingerprint_sector)
      {  if(!strcmp((char*)buf+0x180, "Medium fingerprint"))
	    if(memcmp((char*)buf+0x1a0, (char*)fingerprint, 16))
	       return SECTOR_MISSING_WRONG_FP;
      }
   }

   return SECTOR_MISSING;
}

/***
 *** Dialogue for indicating problem with the missing sector
 ***/

static void insert_buttons(GtkDialog *dialog)
{  
   gtk_dialog_add_buttons(dialog, 
			  _utf("Stop reporting these errors"), 1,
			  _utf("Continue reporting"), 0, NULL);
}

void ExplainMissingSector(unsigned char *buf, gint64 sector, int error, int image)
{  int answer;
   gint64 recorded_number;
   char *vol_label, *label_msg;

   if(Closure->noMissingWarnings)
      return;

   if(!Closure->guiMode
      && (   error == SECTOR_MISSING_DISPLACED
	  || error == SECTOR_MISSING_WRONG_FP))
   {  printf("* This image/medium was probably mastered from defective source(s).\n"
	     "* Perform this test in GUI mode for more information.\n");
      return;
   }

   get_recorded_number(buf, &recorded_number);

   vol_label = get_volume_label(buf);
   if(vol_label)
   {    label_msg = g_strdup_printf(_("\n\nThe label of the original (defective) medium was:\n%s\n\n"), vol_label);
	g_free(vol_label);
   }
   else label_msg = g_strdup("\n\n");

   /* Error was found in an image */

   if(image == TRUE)
   {  switch(error)
      {  case SECTOR_MISSING_DISPLACED:
	    answer = ModalDialog(GTK_MESSAGE_ERROR, GTK_BUTTONS_NONE, insert_buttons,
				 _("Unrecoverable sector found!\n\n"
				   "Sector %lld is marked unreadable and annotated to be\n"
				   "in a different location (%lld).\n\n"
				   "The image was probably mastered from defective content.\n"
				   "For example it might contain one or more files which came\n"
				   "from a damaged medium which was NOT fully recovered.\n" 
				   "This means that some files may have been silently corrupted.%s"
				   "Since the image was already created defective it can not be\n"
				   "repaired by dvdisaster. Also it will not be possible to create\n"
				   "error correction data for it. Sorry for the bad news.\n"),
				 sector, recorded_number, label_msg);
   
	    if(answer) Closure->noMissingWarnings = TRUE;
	    break;

	 case SECTOR_MISSING_WRONG_FP:
	    answer = ModalDialog(GTK_MESSAGE_ERROR, GTK_BUTTONS_NONE, insert_buttons,
				 _("Unrecoverable sector found!\n\n"
				   "Sector %lld is marked unreadable and seems to come\n"
				   "from a different medium.\n\n"
				   "The image was probably mastered from defective content.\n"
				   "For example it might contain one or more files which came\n"
				   "from a damaged medium which was NOT fully recovered.\n" 
				   "This means that some files may have been silently corrupted.%s"
				   "Since the image was already created defective it can not be\n"
				   "repaired by dvdisaster. Also it will not be possible to create\n"
				   "error correction data for it. Sorry for the bad news.\n"),
				 sector, label_msg);
   
	    if(answer) Closure->noMissingWarnings = TRUE;
	    break;
      }
   }

   /* Error was found while reading a medium */

   else 
   {  int answer;

      answer = ModalDialog(GTK_MESSAGE_ERROR, GTK_BUTTONS_NONE, insert_buttons,
			   _("Unrecoverable sector found!\n\n"
			     "Sector %lld is marked unreadable on the medium.\n\n"
			     "The medium was probably mastered from defective content.\n"
			     "For example it might contain one or more files which came\n"
			     "from a damaged medium which was NOT fully recovered.\n" 
			     "This means that some files may have been silently corrupted.\n"
			     "Since the medium was already created defective it can not be\n"
			     "repaired by dvdisaster. Also it will not be possible to create\n"
			     "error correction data for it. Sorry for the bad news.\n"),
			   sector);
      
      if(answer) Closure->noMissingWarnings = TRUE;
   }

   g_free(label_msg);
}
