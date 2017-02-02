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

#include "rs02-includes.h"
#include "udf.h"

/***
 *** Recognize RS02 error correction data in the image
 ***/

/*
 * Dialog components for disabling RS02 search
 */

static void no_rs02_cb(GtkWidget *widget, gpointer data)
{  int state  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  
   Closure->examineRS02 = !state;

   UpdatePrefsExhaustiveSearch();
}

static void insert_buttons(GtkDialog *dialog)
{  GtkWidget *check,*align;

   gtk_dialog_add_buttons(dialog, 
			  _utf("Skip RS02 test"), 1,
			  _utf("Continue searching"), 0, NULL);

   align = gtk_alignment_new(0.5, 0.5, 0.0, 0.0);
   gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), align, FALSE, FALSE, 0);

   check = gtk_check_button_new_with_label(_utf("Disable RS02 initialization in the preferences"));
   gtk_container_add(GTK_CONTAINER(align), check);
   gtk_container_set_border_width(GTK_CONTAINER(align), 10);
   g_signal_connect(G_OBJECT(check), "toggled", G_CALLBACK(no_rs02_cb), NULL);

   gtk_widget_show(align);
   gtk_widget_show(check);
} 

/*
 * See whether a given header is valid for RS02
 */

enum { HEADER_FOUND, TRY_NEXT_HEADER, TRY_NEXT_MODULO};

static int try_sector(Image *image, gint64 pos, EccHeader **ehptr, unsigned char *secbuf)
{  EccHeader *eh;
   unsigned char fingerprint[16];
   guint32 recorded_crc;
   guint32 real_crc;
   int fp_read = 0;

   /* Try reading the sector */

   Verbose("try_sector: trying sector %lld\n", pos);

   if(ImageReadSectors(image, secbuf, pos, 2) != 2)
   {  Verbose("try_sector: read error, trying next header\n");
      return TRY_NEXT_HEADER;
   }

   eh = (EccHeader*)secbuf;

   /* See if the magic cookie is there. If not, searching within
      this modulo makes no sense for write-once media.
      However if the medium is rewriteable, there might be trash
      data behind the image. So finding an invalid sector
      does not imply there is not RS02 data present.
      Workaround for mistakenly recognizing RS03 headers added. */

   if(strncmp((char*)eh->cookie, "*dvdisaster*RS02", 16))
   {  if(image->type == IMAGE_MEDIUM && image->dh->rewriteable)
      {   Verbose("try_sector: no cookie but rewriteable medium: skipping header\n");
	  return TRY_NEXT_HEADER;
      }
      else
      {   Verbose("try_sector: no cookie, skipping current modulo\n");
	  return TRY_NEXT_MODULO;
      }
   }
   else Verbose("try_sector: header at %lld: magic cookie found\n", (long long int)pos);

   /* Calculate CRC */

   recorded_crc = eh->selfCRC;

#ifdef HAVE_BIG_ENDIAN
   eh->selfCRC = 0x47504c00;
#else
   eh->selfCRC = 0x4c5047;
#endif
   real_crc = Crc32((unsigned char*)eh, sizeof(EccHeader));

   if(real_crc != recorded_crc)
   {  Verbose("try_sector: CRC failed, skipping header\n");
      return TRY_NEXT_HEADER;
   }

   eh = g_malloc(sizeof(EccHeader));
   memcpy(eh, secbuf, sizeof(EccHeader));
#ifdef HAVE_BIG_ENDIAN
   SwapEccHeaderBytes(eh);
#endif
   eh->selfCRC = recorded_crc;

   Verbose("try_sector: CRC okay\n");

   /* Compare medium fingerprint with that recorded in Ecc header.
      Note that GetImageFingerprint provides internal caching;
      the sector is not read repeatedly */

   fp_read = GetImageFingerprint(image, fingerprint, eh->fpSector);
		  
   if(!fp_read)  /* be optimistic if fingerprint sector is unreadable */
   {  *ehptr = eh;
      Verbose("try_sector: read error in fingerprint sector\n");
      return HEADER_FOUND;
   }

   if(!memcmp(fingerprint, eh->mediumFP, 16))  /* good fingerprint */
   {  *ehptr = eh;
      Verbose("try_sector: fingerprint okay, header good\n");
      return HEADER_FOUND;
   }

   /* This might be a header from a larger previous session.
      Discard it and continue */

   Verbose("try_sector: fingerprint mismatch, skipping sector\n");
   g_free(eh);
   
   return TRY_NEXT_HEADER;
}

/*
 * RS02 header search
 */

int RS02Recognize(Image *image)
{  AlignedBuffer *ab = CreateAlignedBuffer(4096);
   Bitmap *try_next_header, *try_next_modulo;
   gint64 pos;
   gint64 header_modulo;
   int read_count = 0;
   int answered_continue = FALSE;
   gint64 max_sectors = 0;

   switch(image->type)
   { case IMAGE_FILE:
       Verbose("RS02Recognize: file %s\n", image->file->path);
       break;

     case IMAGE_MEDIUM:
       Verbose("RS02Recognize: medium %s\n", image->dh->device);
       break;

     default:
       Verbose("RS02Recognize: unknown type %d\n", image->type);
       break;
   }

   /*** Quick search at fixed offsets relative to ISO filesystem */

   if(image->isoInfo)
   {  gint64 iso_size = image->isoInfo->volumeSize; 

      /* Iso size is correct; look for root sector at +2 */

      if(try_sector(image, iso_size, &image->eccHeader, ab->buf) == HEADER_FOUND)
      {  Verbose("Root sector search at +0 successful\n");
	 FreeAlignedBuffer(ab);
	 return TRUE;
      }

      /* Strange stuff. Sometimes the iso size is increased by 150
	 sectors by the burning software. */

      if(try_sector(image, iso_size-150, &image->eccHeader, ab->buf) == HEADER_FOUND)
      {  Verbose("Root sector search at -150 successful\n");
	 FreeAlignedBuffer(ab);
	 return TRUE;
      }
   }

   /*** No exhaustive search on optical media unless explicitly requested. */

   if(!Closure->examineRS02 && image->type == IMAGE_MEDIUM)
   {  Verbose("RS02Recognize: skipping exhaustive RS02 search\n");
      FreeAlignedBuffer(ab);
      return FALSE;
   }

   /*** Normal exhaustive search */

   header_modulo = (gint64)1<<62;

   switch(image->type)
   {  case IMAGE_FILE:
	 max_sectors = image->file->size/2048;
	 break;
      case IMAGE_MEDIUM:
	 max_sectors = MAX(image->dh->readCapacity, image->dh->userAreaSize);
	 break;
   }
   if(max_sectors == 0)
      Stop("max_sectors uninitialized");

   try_next_header = CreateBitmap0(max_sectors);
   try_next_modulo = CreateBitmap0(max_sectors);

   if(image->type == IMAGE_MEDIUM)
      Verbose("Medium rewriteable: %s\n", image->dh->rewriteable ? "TRUE" : "FALSE");

   /*** Search for the headers */

   if(image->type == IMAGE_FILE)  /* Seeking on hard disc is cheap */
      answered_continue = TRUE;

   while(header_modulo >= 32)
   {  pos = max_sectors & ~(header_modulo - 1);

      Verbose("FindHeaderInMedium: Trying modulo %lld\n", header_modulo);

      while(pos > 0)
      {  int result;
	
	 if(Closure->stopActions)
	   goto bail_out;

	 if(GetBit(try_next_header, pos))
	 {  Verbose("Sector %lld cached; skipping\n", pos);
	    goto check_next_header;
	 }

	 if(GetBit(try_next_modulo, pos))
	 {  Verbose("Sector %lld cached; skipping modulo\n", pos);
	     goto check_next_modulo;
	 }

	 result = try_sector(image, pos, &image->eccHeader, ab->buf);

	 switch(result)
	 {  case TRY_NEXT_HEADER:
	       SetBit(try_next_header, pos);
	       read_count++;
	       if(!answered_continue && read_count > 5)
	       {  if(Closure->guiMode)
    		  {  int answer = ModalDialog(GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, insert_buttons,
					      _("Faster medium initialization\n\n"
						"Searching this medium for error correction data may take a long time.\n"
						"Press \"Skip RS02 test\" if you are certain that this medium was\n"
						"not augmented with RS02 error correction data."));
		 
		    if(answer) goto bail_out;
		    answered_continue = TRUE;
		  }
	       }
	       goto check_next_header;
	    case TRY_NEXT_MODULO:
	       SetBit(try_next_modulo, pos);
	       goto check_next_modulo;
	    case HEADER_FOUND:
	       FreeBitmap(try_next_header);
	       FreeBitmap(try_next_modulo);
	       FreeAlignedBuffer(ab);
	       return TRUE;
	 }

      check_next_header:
	pos -= header_modulo;
      }

   check_next_modulo:
      header_modulo >>= 1;
   }

bail_out:
   FreeBitmap(try_next_header);
   FreeBitmap(try_next_modulo);
   FreeAlignedBuffer(ab);
   return FALSE;
}

