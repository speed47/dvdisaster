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

#include "dvdisaster.h"

#include "scsi-layer.h"
#include "udf.h"

#ifndef WITH_CLI_ONLY_YES
/*
 * Local data
 */

typedef struct _medium_info
{  GtkLabel *profileDescr;
   GtkLabel *physicalType;
   GtkLabel *bookType;
   GtkLabel *manufID;
   GtkLabel *discStatus;
   GtkLabel *usedCapacity1;
   GtkLabel *usedCapacity2;
   GtkLabel *blankCapacity;
   GtkLabel *isoLabel;
   GtkLabel *isoSize;
   GtkLabel *isoTime;
   GtkLabel *eccState;
   GtkLabel *eccSize;
   GtkLabel *eccVersion;
   GtkLabel *exhaustiveSearch;
   GtkWidget *exhaustiveSearchButton;
} medium_info;

/***
 *** Find out about the medium
 ***/

static void print_defaults(medium_info *mi)
{  SetLabelText(mi->physicalType, _("Please wait..."));
   SetLabelText(mi->bookType, "-");
   SetLabelText(mi->manufID, "-");
   SetLabelText(mi->profileDescr, "-");
   SetLabelText(mi->discStatus, "-");
   SetLabelText(mi->usedCapacity1, "-");
   SetLabelText(mi->usedCapacity2, " ");
   SetLabelText(mi->blankCapacity, "-");
   SetLabelText(mi->isoLabel, "-");
   SetLabelText(mi->isoSize, "-");
   SetLabelText(mi->isoTime, "-");
   SetLabelText(mi->eccState, "-");
   SetLabelText(mi->eccSize, "-");
   SetLabelText(mi->eccVersion, "-");
   SetLabelText(mi->exhaustiveSearch, "-");
}
#endif

static void print_tab(char *label, int tab_width)
{  char *translation=_(label);
   int length = tab_width-g_utf8_strlen(translation, -1);
   char pad[tab_width+1];

   if(length < 1) pad[0] = 0;
   else
   {  memset(pad, ' ', length);
      pad[length]=0;
   }

   PrintCLI("%s%s", translation, pad);
}

void PrintMediumInfo(void *mi_ptr)
{  Image *image;
   DeviceHandle *dh;
#ifndef WITH_CLI_ONLY_YES
   medium_info *mi=(medium_info*)mi_ptr;
#endif
   char *disc_status;
   char *sess_status;
   int tab_width=30;

#ifndef WITH_CLI_ONLY_YES
   if(!mi) /* create dummy medium_info in CLI mode so that PrintCLIorLabel() won't crash */
   {  mi=alloca(sizeof(medium_info));
      memset(mi, 0, sizeof(medium_info));
   }
#endif

#ifndef WITH_CLI_ONLY_YES
   if(Closure->guiMode)
      print_defaults(mi);

   /*** Ensure the UI is fully updated before our thread is stuck doing i/o */
   while (gtk_events_pending()) gtk_main_iteration();
#endif

   image = OpenImageFromDevice(Closure->device, 2 /* allow blanks, see comment in OpenImageFromDevice() */);
#ifndef WITH_CLI_ONLY_YES
   /*** in case of !image, say that we didn't find any medium */
   if(Closure->guiMode)
      SetLabelText(mi->physicalType, _("Medium not present"));
#endif
   if(!image) return;
   dh = image->dh;
   QueryBlankCapacity(dh);
   
   /* Medium properties */

   PrintCLI(_("Physical medium info"));
   PrintCLI("\n\n");

   tab_width=GetLongestTranslation("Medium type:",
				   "Book type:",
				   "Manuf.-ID:",
				   "Drive profile:",
				   "Disc status:",
				   "Used sectors:",
				   "Blank capacity:",
				   NULL)+1;

   print_tab("Medium type:",tab_width);
   PrintCLIorLabel(
#ifndef WITH_CLI_ONLY_YES
mi->physicalType,
#else
NULL,
#endif
   "%s\n", dh->typeDescr);
   print_tab("Book type:",tab_width);
   PrintCLIorLabel(
#ifndef WITH_CLI_ONLY_YES
mi->bookType,
#else
NULL,
#endif
   "%s\n", dh->bookDescr);
   print_tab("Manuf.-ID:",tab_width);
   PrintCLIorLabel(
#ifndef WITH_CLI_ONLY_YES
mi->manufID,
#else
NULL,
#endif
   "%s\n", dh->manuID);
   print_tab("Drive profile:",tab_width);
   PrintCLIorLabel(
#ifndef WITH_CLI_ONLY_YES
mi->profileDescr,
#else
NULL,
#endif
   "%s\n", dh->profileDescr);

   switch(dh->discStatus&3)
   {  case 0: disc_status = g_strdup(_("empty")); break;
      case 1: disc_status = g_strdup(_("appendable")); break;
      case 2: disc_status = g_strdup(_("finalized")); break;
      default: disc_status = g_strdup(_("unknown")); break;
   }
   switch((dh->discStatus>>2)&3)
   {  case 0: sess_status = g_strdup(_("empty")); break;
      case 1: sess_status = g_strdup(_("incomplete")); break;
      case 2: sess_status = g_strdup(_("damaged")); break;
      default: sess_status = g_strdup(_("complete")); break;
   }


   print_tab("Disc status:",tab_width);
   PrintCLIorLabel(
#ifndef WITH_CLI_ONLY_YES
mi->discStatus,
#else
NULL,
#endif
   _("%s (%d sessions; last session %s)\n"),
		disc_status, dh->sessions, sess_status);
   g_free(disc_status);
   g_free(sess_status);

   print_tab("Used sectors:",tab_width);
   PrintCLIorLabel(
#ifndef WITH_CLI_ONLY_YES
mi->usedCapacity1,
#else
NULL,
#endif
   _("%" PRId64 " sectors (%" PRId64 " MiB), from READ CAPACITY\n"),
		dh->readCapacity+1, (dh->readCapacity+1)>>9);
   print_tab(" ",tab_width);
   PrintCLIorLabel(
#ifndef WITH_CLI_ONLY_YES
mi->usedCapacity2,
#else
NULL,
#endif
   _("%" PRId64 " sectors (%" PRId64 " MiB), from DVD structure\n"),
		dh->userAreaSize, dh->userAreaSize>>9);

   print_tab("Blank capacity:",tab_width);
   PrintCLIorLabel(
#ifndef WITH_CLI_ONLY_YES
mi->blankCapacity,
#else
NULL,
#endif
   _("%" PRId64 " sectors (%" PRId64 " MiB)\n"),
		dh->blankCapacity, (dh->blankCapacity)>>9);

   /* Filesystem properties */

   if(image->isoInfo)
   {  tab_width=GetLongestTranslation("Medium label:",
				      "File system size:",
				      "Creation time:",
				      NULL)+1;

      PrintCLI("\n\n");
      PrintCLI(_("Filesystem info"));
      PrintCLI("\n\n");

      print_tab("Medium label:",tab_width);
      PrintCLIorLabel(
#ifndef WITH_CLI_ONLY_YES
mi->isoLabel,
#else
NULL,
#endif
      "%s\n", image->isoInfo->volumeLabel);
      print_tab("File system size:",tab_width);
      PrintCLIorLabel(
#ifndef WITH_CLI_ONLY_YES
mi->isoSize,
#else
NULL,
#endif
       _("%d sectors (%" PRId64 " MiB)\n"),
		   image->isoInfo->volumeSize, (gint64)image->isoInfo->volumeSize>>9);
      print_tab("Creation time:",tab_width);
      PrintCLIorLabel(
#ifndef WITH_CLI_ONLY_YES
mi->isoTime,
#else
NULL,
#endif
      "%s\n", image->isoInfo->creationDate);
   }

   /* Augmented image properties
      fixme: verify RS03 correctness */

   if(image->eccHeader)
   {  EccHeader *eh = image->eccHeader;
      int major = eh->creatorVersion/10000; 
      int minor = (eh->creatorVersion%10000)/100;
      int micro = eh->creatorVersion%100;
      char method[5];
 
      tab_width=GetLongestTranslation("Error correction data:",
				      "Augmented image size:",
				      "dvdisaster version:",
				      NULL)+1;
      PrintCLI("\n\n");
      PrintCLI(_("Augmented image info"));
      PrintCLI("\n\n");

      memcpy(method, eh->method, 4);
      method[4] = 0;
      print_tab("Error correction data:",tab_width);
#ifndef WITH_CLI_ONLY_YES
      PrintCLIorLabel(mi->eccState, _("%s, %d roots, %4.1f%% redundancy.\n"), 
#else
      PrintCLIorLabel(NULL, _("%s, %d roots, %4.1f%% redundancy.\n"), 
#endif
		   method, eh->eccBytes,
		    ((double)eh->eccBytes*100.0)/(double)eh->dataBytes);
      print_tab("Augmented image size:",tab_width);
#ifndef WITH_CLI_ONLY_YES
      PrintCLIorLabel(mi->eccSize, _("%" PRIu64 " sectors (%" PRId64 " MiB)\n"),
#else
      PrintCLIorLabel(NULL, _("%" PRId64 " sectors (%" PRId64 " MiB)\n"),
#endif
		   image->expectedSectors, image->expectedSectors>>9);

      print_tab("dvdisaster version:", tab_width);

      if(micro)
#ifndef WITH_CLI_ONLY_YES
	   PrintCLIorLabel(mi->eccVersion, "%d.%d.%d", major, minor, micro);
#else
	   PrintCLIorLabel(NULL, "%d.%d.%d", major, minor, micro);
#endif
      else
#ifndef WITH_CLI_ONLY_YES
           PrintCLIorLabel(mi->eccVersion, "%d.%d", major, minor);
#else
           PrintCLIorLabel(NULL, "%d.%d", major, minor);
#endif
   }

#ifndef WITH_CLI_ONLY_YES
   if (Closure->examineRS02 && Closure->examineRS03)
   {  SetLabelText(mi->exhaustiveSearch, _("yes"));
      /*** Hide exhaustive search button if exhaustive search is already enabled for RS02 / RS03 */
	  gtk_widget_hide(mi->exhaustiveSearchButton);
   } else
   {  SetLabelText(mi->exhaustiveSearch, _("no"));
      gtk_widget_show(mi->exhaustiveSearchButton);
   }
#endif

   /* Clean up */

   CloseImage(image);
}

/***
 *** GUI callbacks 
 ***/
#ifndef WITH_CLI_ONLY_YES

/*
 * Callback for drive selection
 */

static void drive_select_cb(GtkWidget *widget, gpointer data)
{  int n;
   char *dnode;

   if(!Closure->deviceNodes->len)  /* No drives available */
     return;

   n = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));

   if(n<0)
     return;

   dnode = g_ptr_array_index(Closure->deviceNodes, n);
   g_free(Closure->device);
   Closure->device = g_strdup(dnode);

   gtk_combo_box_set_active(GTK_COMBO_BOX(Closure->driveCombo), n);
}

/*
 * Callback for updating the medium information
 */

static void update_cb(GtkWidget *widget, gpointer data)
{  medium_info *mi=(medium_info*)data;
   PrintMediumInfo(mi);
}

/*
 * Callback for forcing exhaustive search
 */

static void es_cb(GtkWidget *widget, gpointer data)
{   medium_info *mi=(medium_info*)data;
	int oldRS02 = Closure->examineRS02;
	int oldRS03 = Closure->examineRS03;

	gtk_widget_hide(mi->exhaustiveSearchButton);

	Closure->examineRS02 = TRUE;
	Closure->examineRS03 = TRUE;

	PrintMediumInfo(mi);

	Closure->examineRS02 = oldRS02;
	Closure->examineRS03 = oldRS03;
}

/*
 * Close notification
 */

static void mi_destroy_cb(GtkWidget *widget, gpointer data)
{
   Closure->mediumWindow = NULL;
   Closure->mediumDrive = NULL;

   g_free(Closure->mediumInfoContext);
}

/***
 *** Create the medium info window
 ***/

void CreateMediumInfoWindow()
{ GtkWidget *dialog,*vbox,*hbox,*table,*button,*lab,*sep,*frame,*combo_box;
  medium_info *mi;
  int i;
  int dev_idx = 0;

  if(Closure->mediumWindow) 
  {  gtk_widget_show(Closure->mediumWindow);
     return;
  }

  /*** Create the dialog */

  dialog = gtk_dialog_new_with_buttons(_utf("windowtitle|Medium info"), 
				       Closure->window, GTK_DIALOG_DESTROY_WITH_PARENT,
				       GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT, NULL);
  g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);

  Closure->mediumInfoContext = mi = g_malloc0(sizeof(medium_info));

  /*** Inner vbox and title */

  vbox = gtk_vbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), vbox, TRUE, TRUE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

  lab = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(lab), 
		       _utf("<big>Medium info</big>\n"
			    "<i>Properties of the currently inserted medium</i>"));
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_box_pack_start(GTK_BOX(vbox), lab, FALSE, FALSE, 0);

  sep = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(vbox), sep, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(" "), FALSE, FALSE, 0);

  /*** Drive selection */

  frame = gtk_frame_new(_utf("Drive selection"));
  gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

  hbox = gtk_hbox_new(FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(hbox), 10);
  gtk_container_add(GTK_CONTAINER(frame), hbox);

  lab = gtk_label_new(_utf("Drive:"));
  gtk_box_pack_start(GTK_BOX(hbox), lab, FALSE, FALSE, 0);

  lab = gtk_label_new(" ");
  gtk_box_pack_start(GTK_BOX(hbox), lab, FALSE, FALSE, 0);

  combo_box = gtk_combo_box_new_text();
  gtk_box_pack_start(GTK_BOX(hbox), combo_box, FALSE, FALSE, 0);

  g_signal_connect(G_OBJECT(combo_box), "changed", G_CALLBACK(drive_select_cb), NULL);

  for(i=0; i<Closure->deviceNames->len; i++)   
  {
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_box), 
			      g_ptr_array_index(Closure->deviceNames,i));

    if(!strcmp(Closure->device, g_ptr_array_index(Closure->deviceNodes,i)))
      dev_idx = i;
  }

  if(!Closure->deviceNodes->len)
  {  gtk_combo_box_append_text(GTK_COMBO_BOX(combo_box), _utf("No drives found"));
  }

  gtk_combo_box_set_active(GTK_COMBO_BOX(combo_box), dev_idx);

  lab = gtk_label_new(_utf(" "));
  gtk_box_pack_start(GTK_BOX(hbox), lab, FALSE, FALSE, 0);

  button = gtk_button_new_with_label(_utf("Update medium info"));
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(update_cb), mi);
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 10);

  /*** Medium info */

  frame = gtk_frame_new(_utf("Physical medium info"));
  gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

  table = gtk_table_new(2, 8, FALSE);
  gtk_container_set_border_width(GTK_CONTAINER(table), 5);
  gtk_container_add(GTK_CONTAINER(frame), table);

  lab = gtk_label_new(_utf("Medium type:"));
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 0, 1, 0, 1, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
  lab = gtk_label_new(" ");
  mi->physicalType = GTK_LABEL(lab);
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

  lab = gtk_label_new(_utf("Book type:"));
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 0, 1, 1, 2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
  lab = gtk_label_new(" ");
  mi->bookType = GTK_LABEL(lab);
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

  lab = gtk_label_new(_utf("Manuf.-ID:"));
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 0, 1, 2, 3, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
  lab = gtk_label_new(" ");
  mi->manufID = GTK_LABEL(lab);
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 1, 2, 2, 3, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

  lab = gtk_label_new(_utf("Drive profile:"));
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 0, 1, 3, 4, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
  lab = gtk_label_new(" ");
  mi->profileDescr = GTK_LABEL(lab);
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 1, 2, 3, 4, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

  lab = gtk_label_new(_utf("Disc status:"));
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 0, 1, 4, 5, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
  lab = gtk_label_new(" ");
  mi->discStatus = GTK_LABEL(lab);
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 1, 2, 4, 5, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

  lab = gtk_label_new(_utf("Used sectors:"));
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 0, 1, 5, 6, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
  lab = gtk_label_new(" ");
  mi->usedCapacity1 = GTK_LABEL(lab);
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 1, 2, 5, 6, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

  lab = gtk_label_new(" ");
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 0, 1, 6, 7, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
  lab = gtk_label_new(" ");
  mi->usedCapacity2 = GTK_LABEL(lab);
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 1, 2, 6, 7, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

  lab = gtk_label_new(_utf("Blank capacity:"));
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 0, 1, 7, 8, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
  lab = gtk_label_new(" ");
  mi->blankCapacity = GTK_LABEL(lab);
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 1, 2, 7, 8, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

  /*** Filesystem info */

  frame = gtk_frame_new(_utf("Filesystem info"));
  gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

  table = gtk_table_new(2, 3, FALSE);
  gtk_container_set_border_width(GTK_CONTAINER(table), 5);
  gtk_container_add(GTK_CONTAINER(frame), table);

  lab = gtk_label_new(_utf("Medium label:"));
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 0, 1, 0, 1, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
  lab = gtk_label_new(" ");
  mi->isoLabel = GTK_LABEL(lab);
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

  lab = gtk_label_new(_utf("File system size:"));
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 0, 1, 1, 2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
  lab = gtk_label_new(" ");
  mi->isoSize = GTK_LABEL(lab);
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

  lab = gtk_label_new(_utf("Creation time:"));
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 0, 1, 2, 3, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
  lab = gtk_label_new(" ");
  mi->isoTime = GTK_LABEL(lab);
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 1, 2, 2, 3, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

  /*** Error correction info */

  frame = gtk_frame_new(_utf("Augmented image info"));
  gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

  table = gtk_table_new(3, 4, FALSE);
  gtk_container_set_border_width(GTK_CONTAINER(table), 5);
  gtk_container_add(GTK_CONTAINER(frame), table);

  lab = gtk_label_new(_utf("Error correction data:"));
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 0, 1, 0, 1, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
  lab = gtk_label_new(" ");
  mi->eccState = GTK_LABEL(lab);
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 1, 3, 0, 1, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

  lab = gtk_label_new(_utf("Augmented image size:"));
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 0, 1, 1, 2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
  lab = gtk_label_new(" ");
  mi->eccSize = GTK_LABEL(lab);
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 1, 3, 1, 2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

  lab = gtk_label_new(_utf("dvdisaster version:"));
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0);
  gtk_table_attach(GTK_TABLE(table), lab, 0, 1, 2, 3, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
  lab = gtk_label_new(" ");
  mi->eccVersion = GTK_LABEL(lab);
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
  gtk_table_attach(GTK_TABLE(table), lab, 1, 3, 2, 3, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

  lab = gtk_label_new(_utf("Exhaustive search:"));
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0);
  gtk_table_attach(GTK_TABLE(table), lab, 0, 1, 3, 4, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
  lab = gtk_label_new(" ");
  mi->exhaustiveSearch = GTK_LABEL(lab);
  gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0);
  gtk_table_attach(GTK_TABLE(table), lab, 1, 2, 3, 4, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
  mi->exhaustiveSearchButton = gtk_button_new_with_label(_utf("Run exhaustive search"));
  g_signal_connect(G_OBJECT(mi->exhaustiveSearchButton), "clicked", G_CALLBACK(es_cb), mi);
  gtk_table_attach(GTK_TABLE(table), mi->exhaustiveSearchButton, 2, 3, 3, 4, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 0, 0);

  /*** Show it */

  g_signal_connect(G_OBJECT(dialog), "destroy", G_CALLBACK(mi_destroy_cb), NULL);
  Closure->mediumWindow = dialog;
  Closure->mediumDrive = combo_box;
  gtk_widget_show_all(dialog);

  /*** Hide it by default, it'll be unhidden by PrintMediumInfo if needed */
  gtk_widget_hide(mi->exhaustiveSearchButton);

  PrintMediumInfo(mi);
}

#endif
