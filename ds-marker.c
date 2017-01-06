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

#define DSM_VERSION "1.00"
#define PSM_VERSION "1.00"

/***
 *** Create an unique marker for missing sectors
 ***/

static void write_missing_sector(unsigned char *out, guint64 sector, 
				 unsigned char *fingerprint, guint64 fingerprint_sector,
				 char *volume_label, char *simulation_hint)
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

   /* Yes, add the missing sector attributes */

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

   if(simulation_hint)
   {  g_sprintf(buf+0x240,"Simulation hint");
      g_sprintf(buf+0x260,"%s", simulation_hint);
   }
}

void CreateDebuggingSector(unsigned char *out, guint64 sector, 
			   unsigned char *fingerprint, guint64 fingerprint_sector,
			   char *volume_label, char *simulation_hint)
{  write_missing_sector(out, sector, fingerprint, fingerprint_sector, volume_label, simulation_hint);
}

void CreateMissingSector(unsigned char *out, guint64 sector, 
			 unsigned char *fingerprint, guint64 fingerprint_sector,
			 char *volume_label)
{  write_missing_sector(out, sector, fingerprint, fingerprint_sector, volume_label, NULL);
}

/***
 *** Create an unique padding sector
 ***/

void CreatePaddingSector(unsigned char *out, guint64 sector, 
			 unsigned char *fingerprint, guint64 fingerprint_sector)
{  char *buf = (char*)out;
   char *end_marker;
   int end_length; 

   memset(buf, 0, 2048);

   g_sprintf(buf,
 	     "dvdisaster padding sector       "
	     "This is a padding sector needed for augmenting the image "
	     "with error correction data.");

   end_marker = "dvdisaster padding sector end marker";
   end_length = strlen(end_marker);
   memcpy(buf+2047-end_length, end_marker, end_length); 

   g_sprintf(buf+0x100,"Padding sector marker version");
   g_sprintf(buf+0x120,"%s",DSM_VERSION);
   g_sprintf(buf+0x140,"Padding sector number");
   g_sprintf(buf+0x160,"%lld", (long long)sector);
   g_sprintf(buf+0x180,"Medium fingerprint");
   if(fingerprint) memcpy(buf+0x1a0, fingerprint, 16);
   else            memcpy(buf+0x1b0, "none", 4);
   g_sprintf(buf+0x1c0,"Medium fingerprint sector");
   g_sprintf(buf+0x1e0,"%lld", (long long)fingerprint_sector);
}

/***
 *** Helper functions
 ***/

static int get_recorded_number(unsigned char *buf, guint64 *number)
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

/*
 * Used for simulating specific errors
 */

char *GetSimulationHint(unsigned char *buf)
{
   if(!strcmp((char*)buf+0x240, "Simulation hint"))
      return g_strdup((char*)buf+0x260);

   return NULL;
}

/***
 *** Check whether this is a missing sector
 ***/

int CheckForMissingSector(unsigned char *buf, guint64 sector, 
			  unsigned char *fingerprint, guint64 fingerprint_sector)
{  static char pattern[2048];
   static char last_pattern = 0;
   guint64 recorded_number;
   char *sim_hint;
   
   /* Bytefill used as missing sector marker? */

   if(Closure->fillUnreadable >= 0)
   {  if(Closure->fillUnreadable != last_pattern)  /* cache the pattern */
	 memset(pattern, Closure->fillUnreadable, 2048);

      if(memcmp(buf, pattern, 2048)) 
	   return SECTOR_PRESENT;
      else return SECTOR_MISSING;
   }

   /* See if it is our dead sector marker */

   if(strncmp((char*)buf, 
	     "dvdisaster dead sector marker\n"
	     "This sector could not be read from the image.\n"
	     "Its contents have been substituted by the dvdisaster read routine.\n",
	     143)
      || strncmp((char*)buf+2046-34, "dvdisaster dead sector end marker\n", 34))

      return SECTOR_PRESENT;

   /* New style missing sector marker? */

   if(strcmp((char*)buf+0x100,"Dead sector marker version"))
      return SECTOR_MISSING;

   /*** Evaluate new style sector marker */

   /* Look for hints on simulated images */

   sim_hint = GetSimulationHint(buf);
   if(sim_hint)
   {  g_free(sim_hint);
      return SECTOR_WITH_SIMULATION_HINT;
   }
   
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

int CheckForMissingSectors(unsigned char *buf, guint64 sector, 
			   unsigned char *fingerprint, guint64 fingerprint_sector,
			   int n_sectors, guint64 *first_defect)
{  int i,result;

   for(i=0; i<n_sectors; i++)
   {  result = CheckForMissingSector(buf, sector, fingerprint, fingerprint_sector);

      if(result != SECTOR_PRESENT)
      {  *first_defect = sector;
	 return result;
      }

      buf += 2048;
      sector++;
   }

   return SECTOR_PRESENT;
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

void ExplainMissingSector(unsigned char *buf, guint64 sector, int error, int source_type, int *number)
{  int answer;
   guint64 recorded_number;
   char *vol_label, *label_msg;

   if(Closure->noMissingWarnings)
      return;

   /* Missing sectors should be reported in the following cases:
      - In an image, normal missing sectors are to be expected.
        Only displayced sectors and sectors with wrong fingerprint should be reported.
      - In a medium, all kinds of missing sectors constitute a problem and must be reported.
      - Within an ecc file, no missing sectors should appear although  these are at least
        harmless for RS03-type ecc files. Report them all.
   */
   
   if(source_type == SOURCE_IMAGE && error != SECTOR_MISSING_DISPLACED && error != SECTOR_MISSING_WRONG_FP)
     return;
   
   /* In CLI mode, only report the first unrecoverable sector unless verbose is given. */

   if(!Closure->guiMode && !Closure->verbose && *number > 0)
   {  if(*number == 1)
	 PrintLog(_("* ... more unrecoverable sectors found ...\n"
		    "* further messages are suppressed unless the -v option is given.\n"));
      (*number)++;
      return;
   }
   (*number)++;
   
   /* Get some meta data from the dsm */
   
   get_recorded_number(buf, &recorded_number);

   vol_label = get_volume_label(buf);
   if(vol_label)
   {   if(Closure->guiMode)
	    label_msg = g_strdup_printf(_("\n\nThe label of the original (defective) medium was:\n%s\n\n"), vol_label);
       else label_msg = g_strdup_printf(_("\n* \n* The label of the original (defective) medium was:\n* \n*  %s\n* "), vol_label);
       g_free(vol_label);
   }
   else label_msg = g_strdup("\n");

   /* Error was found in an image */

   if(source_type == SOURCE_IMAGE)
   {  switch(error)
      {  case SECTOR_MISSING_DISPLACED:
	 {  char *msg = _("Unrecoverable sector found!\n\n"
			  "Sector %lld is marked unreadable and annotated to be\n"
			  "in a different location (%lld).\n\n"
			  "The image was probably mastered from defective content.\n"
			  "For example it might contain one or more files which came\n"
			  "from a damaged medium which was NOT fully recovered.\n" 
			  "This means that some files may have been silently corrupted.%s\n"
			  "Since the image was already created defective it can not be\n"
			  "repaired by dvdisaster. Also it will not be possible to create\n"
			  "error correction data for it. Sorry for the bad news.\n");

	    if(!Closure->guiMode)
	        PrintLogWithAsterisks(msg,sector, recorded_number, label_msg);
	    else
	    {  answer = ModalDialog(GTK_MESSAGE_ERROR, GTK_BUTTONS_NONE, insert_buttons, msg,
				    sector, recorded_number, label_msg);
   
	       if(answer) Closure->noMissingWarnings = TRUE;
	    }
	 }
	 break;

	 case SECTOR_MISSING_WRONG_FP:
	 {  char *msg = _("Unrecoverable sector found!\n\n"
			  "Sector %lld is marked unreadable and seems to come\n"
			  "from a different medium.\n\n"
			  "The image was probably mastered from defective content.\n"
			  "For example it might contain one or more files which came\n"
			  "from a damaged medium which was NOT fully recovered.\n" 
			  "This means that some files may have been silently corrupted.%s\n"
			  "Since the image was already created defective it can not be\n"
			  "repaired by dvdisaster. Also it will not be possible to create\n"
			  "error correction data for it. Sorry for the bad news.\n");

	    if(!Closure->guiMode)
	         PrintLogWithAsterisks(msg,sector, label_msg);
	    else
	    {  answer = ModalDialog(GTK_MESSAGE_ERROR, GTK_BUTTONS_NONE, insert_buttons, msg,
				    sector, label_msg);
	       if(answer) Closure->noMissingWarnings = TRUE;
	    }
	 }
	 break;
      }
   }

   /* Error was found while reading a medium */

   if(source_type == SOURCE_MEDIUM)
   {  char *msg = _("Unrecoverable sector found!\n\n"
		    "Sector %lld is marked unreadable on the medium.\n\n"
		    "The medium was probably mastered from defective content.\n"
		    "For example it might contain one or more files which came\n"
		    "from a damaged medium which was NOT fully recovered.\n" 
		    "This means that some files may have been silently corrupted.\n"
		    "Since the medium was already created defective it can not be\n"
		    "repaired by dvdisaster. Also it will not be possible to create\n"
		    "error correction data for it. Sorry for the bad news.\n");
			   
     if(!Closure->guiMode)
          PrintLogWithAsterisks(msg, sector);
     else
     {  answer = ModalDialog(GTK_MESSAGE_ERROR, GTK_BUTTONS_NONE, insert_buttons, msg,
			     sector);
      
        if(answer) Closure->noMissingWarnings = TRUE;
     }
   }

   /* Error was found while reading an ecc file */

   if(source_type == SOURCE_ECCFILE)
   {  char *msg = _("Unrecoverable sector found!\n\n"
		    "Sector %lld is marked unreadable in the ecc file.\n\n"
		    "The ecc file was probably taken from a medium which\n"
		    "was NOT fully recovered. That means that some sectors\n"
		    "in the ecc file are missing and its error correction\n"
		    "capacity will be reduced.\n");
			   
     if(!Closure->guiMode)
          PrintLogWithAsterisks(msg, sector);
     else
     {  answer = ModalDialog(GTK_MESSAGE_ERROR, GTK_BUTTONS_NONE, insert_buttons, msg,
			     sector);
      
        if(answer) Closure->noMissingWarnings = TRUE;
     }
   }

   g_free(label_msg);
}
