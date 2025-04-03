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

/*** src type: some GUI code ***/

#include "dvdisaster.h"

#include "rs01-includes.h"

#ifdef WITH_GUI_YES

/***
 *** Reset the verify output window
 ***/

void ResetRS01VerifyWindow(Method *self)
{  RS01Widgets *wl = (RS01Widgets*)self->widgetList;

   GuiSetLabelText(wl->cmpChkSumErrors, "-");
   GuiSetLabelText(wl->cmpMissingSectors, "0");
   GuiSetLabelText(wl->cmpImageMd5Sum, "-");
   GuiSetLabelText(wl->cmpImageResult, "");
   GuiSwitchAndSetFootline(wl->cmpImageNotebook, 1, NULL, NULL);

   GuiSetLabelText(wl->cmpEccEmptyMsg, "");
   GuiSetLabelText(wl->cmpEccCreatedBy, "dvdisaster");
   GuiSetLabelText(wl->cmpEccMethod, "");
   GuiSetLabelText(wl->cmpEccRequires, "");
   GuiSetLabelText(wl->cmpEccMediumSectors, "");
   GuiSetLabelText(wl->cmpEccImgMd5Sum, "");
   GuiSetLabelText(wl->cmpEccFingerprint, _("n/a"));
   GuiSetLabelText(wl->cmpEccBlocks, "");
   GuiSetLabelText(wl->cmpEccMd5Sum, "");
   GuiSetLabelText(wl->cmpEccResult, "");
   GuiSwitchAndSetFootline(wl->cmpEccNotebook, 0, NULL, NULL);

   wl->lastPercent = 0;

   GuiFillSpiral(wl->cmpSpiral, Closure->background);
   gtk_widget_queue_draw(wl->cmpSpiral->widget);
}

/***
 *** Manage the image spiral
 ***/

/*
 * Update part of the spiral
 */

typedef struct _spiral_idle_info
{  Spiral *cmpSpiral;
   GdkColor *segColor;
   int from, to;
} spiral_idle_info;

static gboolean spiral_idle_func(gpointer data)
{  spiral_idle_info *sii = (spiral_idle_info*)data;
   int i;

   for(i=sii->from; i<=sii->to; i++)
     GuiSetSpiralSegmentColor(sii->cmpSpiral, sii->segColor, i-1);

   g_free(sii);
   return FALSE;
}

void RS01AddVerifyValues(Method *method, int percent, 
			 gint64 totalMissing, gint64 totalCrcErrors,
			 gint64 newMissing, gint64 newCrcErrors)
{  RS01Widgets *wl = (RS01Widgets*)method->widgetList;
   spiral_idle_info *sii = g_malloc(sizeof(spiral_idle_info));

   if(percent < 0 || percent > VERIFY_IMAGE_SEGMENTS)
     return;

   if(newMissing) 
   {  GuiSetLabelText(wl->cmpMissingSectors, "<span %s>%" PRId64 "</span>", 
		      Closure->redMarkup, totalMissing);
   }
   
   if(newCrcErrors) 
   {  GuiSetLabelText(wl->cmpChkSumErrors, "<span %s>%" PRId64 "</span>", 
		      Closure->redMarkup, totalCrcErrors);
   }
   
   sii->cmpSpiral = wl->cmpSpiral;

   sii->segColor = Closure->greenSector;
   if(newCrcErrors) sii->segColor = Closure->yellowSector;
   if(newMissing) sii->segColor = Closure->redSector;

   sii->from = wl->lastPercent+1;
   sii->to   = percent;

   wl->lastPercent = percent;
   g_idle_add(spiral_idle_func, sii);
}

/*
 * Redraw whole spiral
 */

static void redraw_spiral(RS01Widgets *wl)
{  int x = wl->cmpSpiral->mx - wl->cmpSpiral->diameter/2 + 10;

   GuiDrawSpiralLabel(wl->cmpSpiral, wl->cmpLayout,
		      _("Good sectors"), Closure->greenSector, x, 1);

   GuiDrawSpiralLabel(wl->cmpSpiral, wl->cmpLayout,
		      _("Sectors with CRC errors"), Closure->yellowSector, x, 2);

   GuiDrawSpiralLabel(wl->cmpSpiral, wl->cmpLayout,
		      _("Missing sectors"), Closure->redSector, x, 3);

   GuiDrawSpiral(wl->cmpSpiral);
}

/*
 * expose event handler for the spiral
 */

static gboolean expose_cb(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{  RS01Widgets *wl = (RS01Widgets*)data;
   GtkAllocation a = {0};
   gtk_widget_get_allocation(widget, &a);
   int w,h,size;

   /* Finish spiral initialization */

   if(!wl->cmpLayout)
   {  GuiSetSpiralWidget(wl->cmpSpiral, widget);
      wl->cmpLayout = gtk_widget_create_pango_layout(widget, NULL);
   }

   GuiSetText(wl->cmpLayout, _("Missing sectors"), &w, &h);
   size = wl->cmpSpiral->diameter + 20 + 3*(10+h);  /* approx. size of spiral + labels */

   wl->cmpSpiral->mx = a.width / 2;
   wl->cmpSpiral->my = (wl->cmpSpiral->diameter + a.height - size)/2;

   if(event->count) /* Exposure compression */
   {  return TRUE;
   }

   /* Redraw the spiral */

   redraw_spiral(wl);

   return TRUE;
}


/***
 *** Create the notebook contents for the verify output
 ***/

void CreateRS01VerifyWindow(Method *self, GtkWidget *parent)
{  RS01Widgets *wl = (RS01Widgets*)self->widgetList;
   GtkWidget *sep,*notebook,*table,*table2,*ignore,*lab,*frame,*d_area;

   wl->cmpHeadline = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(wl->cmpHeadline), 0.0, 0.0); 
   gtk_misc_set_padding(GTK_MISC(wl->cmpHeadline), 5, 0);
   gtk_box_pack_start(GTK_BOX(parent), wl->cmpHeadline, FALSE, FALSE, 3);

   sep = gtk_hseparator_new();
   gtk_box_pack_start(GTK_BOX(parent), sep, FALSE, FALSE, 0);

   sep = gtk_hseparator_new();
   gtk_box_pack_start(GTK_BOX(parent), sep, FALSE, FALSE, 0);

   table = gtk_table_new(2, 2, FALSE);
   gtk_container_set_border_width(GTK_CONTAINER(table), 5);
   gtk_box_pack_start(GTK_BOX(parent), table, TRUE, TRUE, 0);

   /*** Image info */

   frame = gtk_frame_new(_utf("Image file summary"));
   gtk_table_attach(GTK_TABLE(table), frame, 0, 1, 0, 1, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 5);

   notebook = wl->cmpImageNotebook = gtk_notebook_new();
   gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
   gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);
   gtk_container_add(GTK_CONTAINER(frame), notebook);

   ignore = gtk_label_new("no image");
   lab = gtk_label_new(_utf("No image present."));
   gtk_notebook_append_page(GTK_NOTEBOOK(notebook), lab, ignore);

   table2 = gtk_table_new(2, 5, FALSE);
   ignore = gtk_label_new("image info");
   gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table2, ignore);
   gtk_container_set_border_width(GTK_CONTAINER(table2), 5);

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   GuiSetLabelText(lab, _("Medium sectors:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, 0, 1, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpImageSectors = gtk_label_new("0");
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   GuiSetLabelText(lab, _("Checksum errors:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, 1, 2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpChkSumErrors = gtk_label_new("0");
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   GuiSetLabelText(lab, _("Missing Sectors:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, 2, 3, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpMissingSectors = gtk_label_new("0");
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, 2, 3, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   GuiSetLabelText(lab, _("Image checksum:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, 3, 4, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpImageMd5Sum = gtk_label_new("0");
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, 3, 4, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

   lab = wl->cmpImageResult = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0);
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 2, 4, 5, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 4);

   /*** Image spiral */

   frame = gtk_frame_new(_utf("Image state"));
   gtk_table_attach(GTK_TABLE(table), frame, 1, 2, 0, 2, GTK_SHRINK | GTK_FILL, GTK_EXPAND | GTK_FILL, 5, 5);

   wl->cmpSpiral = GuiCreateSpiral(Closure->grid, Closure->background, 10, 5, VERIFY_IMAGE_SEGMENTS);
   d_area = wl->cmpDrawingArea = gtk_drawing_area_new();
   gtk_widget_set_size_request(d_area, wl->cmpSpiral->diameter+20, -1);
   gtk_container_add(GTK_CONTAINER(frame), d_area);
   g_signal_connect(G_OBJECT(d_area), "expose_event", G_CALLBACK(expose_cb), (gpointer)wl);

   /*** Ecc info */

   frame = gtk_frame_new(_utf("Error correction file summary"));
   gtk_table_attach(GTK_TABLE(table), frame, 0, 1, 1, 2, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 5, 5);

   notebook = wl->cmpEccNotebook = gtk_notebook_new();
   gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
   gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);
   gtk_container_add(GTK_CONTAINER(frame), notebook);

   ignore = gtk_label_new("no ecc file");
   lab = wl->cmpEccEmptyMsg = gtk_label_new("");
   gtk_notebook_append_page(GTK_NOTEBOOK(notebook), lab, ignore);

   table2 = gtk_table_new(2, 9, FALSE);
   ignore = gtk_label_new("ecc info");
   gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table2, ignore);
   gtk_container_set_border_width(GTK_CONTAINER(table2), 5);

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   GuiSetLabelText(lab, _("Created by:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, 0, 1, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccCreatedBy = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   GuiSetLabelText(lab, _("Method:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, 1, 2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccMethod = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   GuiSetLabelText(lab, _("Requires:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, 2, 3, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccRequires = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, 2, 3, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   GuiSetLabelText(lab, _("Medium sectors:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, 3, 4, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccMediumSectors = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, 3, 4, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   GuiSetLabelText(lab, _("Image checksum:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, 4, 5, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccImgMd5Sum = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, 4, 5, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   GuiSetLabelText(lab, _("Fingerprint:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, 5, 6, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccFingerprint = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, 5, 6, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   GuiSetLabelText(lab, _("Ecc blocks:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, 6, 7, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccBlocks = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, 6, 7, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   GuiSetLabelText(lab, _("Ecc checksum:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, 7, 8, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccMd5Sum = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, 7, 8, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

   lab = wl->cmpEccResult = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0);
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 2, 8, 9, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 4);
}
#endif /* WITH_GUI_YES */

/***
 *** Verify the prefix.* files
 ***/

typedef struct
{  Image *image;
} verify_closure;

static void cleanup(gpointer data)
{  verify_closure *vc = (verify_closure*)data;

   UnregisterCleanup();

   if(vc->image) CloseImage(vc->image);
   g_free(vc);

   GuiAllowActions(TRUE);
   GuiExitWorkerThread();
}

/***
 *** Verify entry point fpr both CLI and GUI mode
 ***/

void RS01Verify(Image *image)
{  verify_closure *vc = g_malloc0(sizeof(verify_closure));
   Method *self = FindMethod("RS01");
#ifdef WITH_GUI_YES
   RS01Widgets *wl = (RS01Widgets*)self->widgetList;
#endif
   char idigest[33],edigest[33]; 
   gint64 excess_sectors = 0;
   char *ecc_advice = NULL;

   EccHeader *eh;
   gint8 method[5];
   int ecc_in_last = 0;
   gint64 ecc_blocks,ecc_expected,count;
   struct MD5Context md5ctxt;
   int percent,last_percent;
   unsigned char digest[16];
   unsigned char buf[1024];

   idigest[0] = 0;

   /*** Prepare for early termination */

   RegisterCleanup(_("Comparison aborted"), cleanup, vc);

   /*** Examine the .iso file */

   GuiSetLabelText(wl->cmpHeadline, "<big>%s</big>\n<i>%s</i>",
		   _("Comparing image and error correction files."),
		   _("- Checking image file -"));

   vc->image = image;
   if(image && image->eccFile)  
   {    GuiSetLabelText(wl->cmpChkSumErrors, "0");
   }
   else
   {    GuiSetLabelText(wl->cmpChkSumErrors, _("n/a"));
   }

   /* Report basic image properties */

   PrintLog("\n%s: ", Closure->imageName);
   if(!image || !image->file)
   {  PrintLog(_("not present\n"));

      GuiSwitchAndSetFootline(wl->cmpImageNotebook, 0, NULL, NULL);
      goto process_ecc;
   }

   if(image->inLast == 2048)
   {  PrintLog(_("present, contains %" PRId64 " medium sectors.\n"), image->sectorSize);
      GuiSetLabelText(wl->cmpImageSectors, "%" PRId64, image->sectorSize);
   }
   else
   {  PrintLog(_("present, contains %" PRId64 " medium sectors and %d bytes.\n"),
	       image->sectorSize-1, image->inLast);
      GuiSetLabelText(wl->cmpImageSectors, _("%" PRId64 " sectors + %d bytes"), 
		      image->sectorSize-1, image->inLast);
   }

   if(!Closure->quickVerify)
      RS01ScanImage(self, image, NULL, PRINT_MODE);

   if(Closure->stopActions)   
   {   if(Closure->stopActions == STOP_CURRENT_ACTION) /* suppress memleak warning when closing window */
       {  GuiSetLabelText(wl->cmpImageResult, 
			  _("<span %s>Aborted by user request!</span>"),
			  Closure->redMarkup);
       }
       goto terminate;
   }

   /*** Peek into the ecc file to get expected sector count */

   if(image->eccFile)  
   {  guint64 diff = 0;

      if(image->sectorSize < image->expectedSectors)
      {  diff = image->expectedSectors - image->sectorSize;

	 PrintLog(_("* truncated image  : %" PRId64 " sectors too short\n"), diff);
	 GuiSetLabelText(wl->cmpImageSectors, 
			 _("<span %s>%" PRId64 " (%" PRId64 " sectors too short)</span>"),
			 Closure->redMarkup, image->sectorSize, diff);
	 image->sectorsMissing += diff;
      }
      if(image->sectorSize > image->expectedSectors)
      {  excess_sectors = image->sectorSize - image->expectedSectors;
      }
   }

   /*** Show summary of image read */

   if(image->crcErrors)
   {  GuiSetLabelText(wl->cmpChkSumErrors, 
		      "<span %s>%" PRId64 "</span>",
		      Closure->redMarkup, image->crcErrors);
   }
   if(image->sectorsMissing)
   {  GuiSetLabelText(wl->cmpMissingSectors, 
		      "<span %s>%" PRId64 "</span>",
		      Closure->redMarkup, image->sectorsMissing);
   }
   if(excess_sectors)
   {  PrintLog(_("* image too long   : %" PRId64 " excess sectors\n"), excess_sectors);
      GuiSetLabelText(wl->cmpImageSectors, 
		      _("<span %s>%" PRId64 " (%" PRId64 " excess sectors)</span>"),
		      Closure->redMarkup, image->sectorSize, excess_sectors);
      GuiSetLabelText(wl->cmpImageResult,
		      _("<span %s>Bad image.</span>"),
		      Closure->redMarkup);
   } 
   else if(Closure->quickVerify)
   {  PrintLog(_("* quick mode        : image NOT scanned\n"));
   }
   else
   {  if(!image->sectorsMissing)
      {  
	 AsciiDigest(idigest, image->mediumSum);
      
	 if(!image->crcErrors)
	 {  PrintLog(_("- good image       : all sectors present\n"
		       "- image md5sum     : %s\n"),idigest);
	    GuiSetLabelText(wl->cmpImageResult,_("<span %s>Good image.</span>"), Closure->greenMarkup);
	    GuiSetLabelText(wl->cmpImageMd5Sum, "%s", idigest);
	 }
	 else
	 {  PrintLog(_("* suspicious image : all sectors present, but %" PRId64 " CRC errors\n"
		       "- image md5sum     : %s\n"),image->crcErrors,idigest);

	    GuiSetLabelText(wl->cmpImageResult, _("<span %s>Image complete, but contains checksum errors!</span>"), Closure->redMarkup);
	    GuiSetLabelText(wl->cmpImageMd5Sum, "%s", idigest);
	 }
      }
      else /* sectors are missing */
      {  if(!image->crcErrors)
	      PrintLog(_("* BAD image        : %" PRId64 " sectors missing\n"), image->sectorsMissing);
	 else PrintLog(_("* BAD image        : %" PRId64 " sectors missing, %" PRId64 " CRC errors\n"), 
		         image->sectorsMissing, image->crcErrors);

	 GuiSetLabelText(wl->cmpImageResult,
			 _("<span %s>Bad image.</span>"), Closure->redMarkup);
      }
   }

   /*** The .ecc file */

process_ecc:
   GuiSetLabelText(wl->cmpHeadline, "<big>%s</big>\n<i>%s</i>",
		   _("Comparing image and error correction files."),
		   _("- Checking ecc file -"));

   PrintLog("\n%s: ", Closure->eccName);

   if(!image)
   {  PrintLog(_("not present\n"));
      GuiSwitchAndSetFootline(wl->cmpEccNotebook, 0, 
			      wl->cmpEccEmptyMsg,_("No error correction file present."));
      goto skip_ecc;
   }

   if(image && !image->eccFile)
   {  
      switch(image->eccFileState)
      {  case ECCFILE_NOPERM:
	    PrintLog(_("permission denied\n"));
	    break;
	 case ECCFILE_MISSING:
	    PrintLog(_("not present\n"));
	    break;
	 case ECCFILE_INVALID:
	    PrintLog(_("invalid\n"));
	    break;
	 case ECCFILE_DEFECTIVE_HEADER:
	    PrintLog(_("defective header (unusable)\n"));
	    break;
	 case ECCFILE_WRONG_CODEC:
	    PrintLog(_("unknown codec (unusable)\n"));
	    break;
	 default:
	    PrintLog(_("unusable\n"));
	    break;
      }

      GuiSwitchAndSetFootline(wl->cmpEccNotebook, 0, 
			      wl->cmpEccEmptyMsg,_("No error correction file present."));
      goto skip_ecc;
   }

   eh = image->eccFileHeader;  /* simply an alias */

   /* Version number of dvdisaster used for creating the ecc file */ 

   if(!eh->neededVersion)  /* The V0.41.* series did not fill in this field. */
     eh->neededVersion = 4100;

   if(eh->creatorVersion)
   {  int major = eh->creatorVersion/10000; 
      int minor = (eh->creatorVersion%10000)/100;
      int micro = eh->creatorVersion%100;
      char *unstable="";

      /* Suppress (unstable) output in debug mode to facilitate regression tests */
      if((eh->methodFlags[3] & MFLAG_DEVEL) && !Closure->regtestMode)
	unstable=" (unstable)";
      
      if(micro)  /* version format x.xx.x */
      {  char *format = "%s-%d.%d.%d%s";
	 PrintLog(format, _("created by dvdisaster"), major, minor, micro, unstable);
	 PrintLog("\n");

	 GuiSwitchAndSetFootline(wl->cmpEccNotebook, 1,
				 wl->cmpEccCreatedBy, 
				 format, "dvdisaster",
				 major, minor, micro, unstable);
      }
      else  /* version format x.xx */
      {  char *format = "%s-%d.%d%s";
	 PrintLog(format, _("created by dvdisaster"), 
		 major, minor, unstable);
	 PrintLog("\n");
	 GuiSwitchAndSetFootline(wl->cmpEccNotebook, 1,
				 wl->cmpEccCreatedBy, format, "dvdisaster",
				 major, minor, unstable);
      }
   }
   else
   {  PrintLog(_("created by dvdisaster-0.41.x.\n"));
      GuiSwitchAndSetFootline(wl->cmpEccNotebook, 1,
			      wl->cmpEccCreatedBy, "dvdisaster-0.41.x");
   }
   
   /* Information on RS01 properties */

   memcpy(method, eh->method, 4); method[4] = 0;

   PrintLog(_("- method           : %4s, %d roots, %4.1f%% redundancy.\n"),
	    method, eh->eccBytes, 
	    ((double)eh->eccBytes*100.0)/(double)eh->dataBytes);

   GuiSetLabelText(wl->cmpEccMethod, _("%4s, %d roots, %4.1f%% redundancy"),
		   method, eh->eccBytes, 
		   ((double)eh->eccBytes*100.0)/(double)eh->dataBytes);

   /* Show and verify needed version */

   if(Closure->version >= eh->neededVersion)
   {  PrintLog(_("- requires         : dvdisaster-%d.%d (good)\n"),
	       eh->neededVersion/10000,
	       (eh->neededVersion%10000)/100);
       GuiSetLabelText(wl->cmpEccRequires, "dvdisaster-%d.%d",
		       eh->neededVersion/10000,
		       (eh->neededVersion%10000)/100);
   }
   else 
   {  PrintLog(_("* requires         : dvdisaster-%d.%d (BAD)\n"
		 "* Warning          : The following output might be incorrect.\n"
		 "*                  : Please upgrade dvdisaster.\n"),
	       eh->neededVersion/10000,
	       (eh->neededVersion%10000)/100);

       GuiSetLabelText(wl->cmpEccRequires, 
		       "<span %s>dvdisaster-%d.%d</span>",
		       Closure->redMarkup,
		       eh->neededVersion/10000,
		       (eh->neededVersion%10000)/100);
       if(!ecc_advice) 
	 ecc_advice = g_strdup_printf(_("<span %s>Please upgrade your version of dvdisaster!</span>"), Closure->redMarkup);
   }


   /* image size comparison */

   if(eh->creatorVersion >= 6600 && eh->inLast != 2048)  /* image file whose length is */
     ecc_in_last = eh->inLast;                           /* not a multiple of 2048 */

   if(!image->file)
   {  if(!ecc_in_last)
      {  PrintLog(_("- medium sectors   : %" PRId64 "\n"), image->expectedSectors);
	 GuiSetLabelText(wl->cmpEccMediumSectors, "%" PRId64 "", image->expectedSectors);
      }
      else
      {  PrintLog(_("- medium sectors   : %" PRId64 " sectors + %d bytes\n"),
		  image->expectedSectors-1, ecc_in_last);
	 GuiSetLabelText(wl->cmpEccMediumSectors, 
			 _("%" PRId64 " sectors + %d bytes"), 
			 image->expectedSectors-1, ecc_in_last);
      }
   }

   if(image->file)
   {  
      if(image->sectorSize == image->expectedSectors 
	 && (!ecc_in_last || image->inLast == eh->inLast))
      {  if(!ecc_in_last)
	 {  PrintLog(_("- medium sectors   : %" PRId64 " (good)\n"), image->expectedSectors);
	    GuiSetLabelText(wl->cmpEccMediumSectors, "%" PRId64, image->expectedSectors);
	 }
	 else
	 {  PrintLog(_("- medium sectors   : %" PRId64 " sectors + %d bytes (good)\n"),
		     image->expectedSectors-1, ecc_in_last);
	    GuiSetLabelText(wl->cmpEccMediumSectors, 
			    _("%" PRId64 " sectors + %d bytes"), 
			    image->expectedSectors-1, ecc_in_last);
	 }
      }

      else /* sector sizes differ */
      { /* TAO case (1 or 2 sectors more than expected) */
	if(image->sectorSize > image->expectedSectors && image->sectorSize - image->expectedSectors <= 2)   
	{  PrintLog(_("* medium sectors   : %" PRId64 " (BAD, perhaps TAO/DAO mismatch)\n"), image->expectedSectors);
	   if(!ecc_in_last)  
	   {  GuiSetLabelText(wl->cmpEccMediumSectors, "<span %s>%" PRId64 "</span>", 
			      Closure->redMarkup, image->expectedSectors);
	   }
	   else
	   {  GuiSetLabelText(wl->cmpEccMediumSectors, "<span %s>%" PRId64 " sectors + %d bytes</span>", 
			      Closure->redMarkup, image->expectedSectors-1, ecc_in_last);
	   }
	}
	else  /* more than 2 Sectors difference */ 
	{  if(!ecc_in_last)
	   {  PrintLog(_("* medium sectors   : %" PRId64 " (BAD)\n"), image->expectedSectors);
	      GuiSetLabelText(wl->cmpEccMediumSectors, "<span %s>%" PRId64 "</span>", 
			      Closure->redMarkup, image->expectedSectors);
	      if(!ecc_advice)
		ecc_advice = g_strdup_printf(_("<span %s>Image size does not match error correction file.</span>"), Closure->redMarkup);
	   }
	   else /* byte size difference */
	   {  PrintLog(_("* medium sectors   : %" PRId64 " sectors + %d bytes (BAD)\n"),
		       image->expectedSectors-1, ecc_in_last);
	      GuiSetLabelText(wl->cmpEccMediumSectors, 
			      _("<span %s>%" PRId64 " sectors + %d bytes</span>"), 
			      Closure->redMarkup, image->expectedSectors-1, ecc_in_last);
	      if(!ecc_advice)
		ecc_advice = g_strdup_printf(_("<span %s>Image size does not match error correction file.</span>"), Closure->redMarkup);
	   }
	}
      }
   }
   
   /*** Show and verify image md5sum */

   if(!Closure->quickVerify)
   {  AsciiDigest(edigest, eh->mediumSum);
      if(image && image->file && !image->sectorsMissing && !excess_sectors)
      {  int n = !memcmp(eh->mediumSum, image->mediumSum, 16);
	 if(n)
	 {  PrintLog(_("- image md5sum     : %s (good)\n"),edigest);
	    GuiSetLabelText(wl->cmpEccImgMd5Sum, "%s", edigest);
	 }
	 else
	 {  PrintLog(_("* image md5sum     : %s (BAD)\n"),edigest);
	    GuiSetLabelText(wl->cmpEccImgMd5Sum, "<span %s>%s</span>",
			    Closure->redMarkup, edigest);
	    GuiSetLabelText(wl->cmpImageMd5Sum, "<span %s>%s</span>",
			    Closure->redMarkup, idigest);
	 }
      }
      else 
      {  PrintLog(_("- image md5sum     : %s\n"),edigest);
	 GuiSetLabelText(wl->cmpEccImgMd5Sum, "%s", edigest);
      }
   }

   if(image && image->file)
   {  if(image->fpState != FP_PRESENT)
      {  PrintLog(_("* fingerprint match: NOT POSSIBLE - related sector is missing in image!\n"));
	 GuiSetLabelText(wl->cmpEccFingerprint,
			 _("<span %s>missing sector prevents calculation</span>"),
			 Closure->redMarkup);
      }
      else
      { 
	if(memcmp(image->imageFP, eh->mediumFP, 16)) 
	{  PrintLog(_("* fingerprint match: MISMATCH - .iso and .ecc don't belong together!\n"));

	   GuiSetLabelText(wl->cmpEccFingerprint, 
			   _("<span %s>mismatch</span>"), Closure->redMarkup);

	   if(!ecc_advice)
	     ecc_advice = g_strdup_printf(_("<span %s>Image and error correction files do not belong together!</span>"), Closure->redMarkup);
	}
	else 
	{  PrintLog(_("- fingerprint match: good\n"));
	   GuiSetLabelText(wl->cmpEccFingerprint, _("good"));
	}
      }
   }

   /* Show and verify the number of ecc blocks */

   if(Closure->quickVerify)  /* terminate early */
   {  PrintLog(_("* quick mode        : ecc file NOT scanned\n"));
      goto terminate;
   }

   ecc_expected = 2048*((image->expectedSectors+eh->dataBytes-1)/eh->dataBytes);
   ecc_blocks = (image->eccFile->size-image->expectedSectors*sizeof(guint32)-sizeof(EccHeader))/eh->eccBytes;

   if(ecc_expected == ecc_blocks)
   {  PrintLog(_("- ecc blocks       : %" PRId64 " (good)\n"),ecc_blocks);
      GuiSetLabelText(wl->cmpEccBlocks, "%" PRId64, ecc_blocks);
   }
   else
   {  PrintLog(_("* ecc blocks       : %" PRId64 " (BAD, expected %" PRId64 ")\n"),
	       ecc_blocks,ecc_expected);
      GuiSetLabelText(wl->cmpEccBlocks,
		      _("<span %s>%" PRId64 " (bad, expected %" PRId64 ")</span>"),
		      Closure->redMarkup,ecc_blocks,ecc_expected);
   }

   /*** Test ecc file against its own md5sum */
   
   MD5Init(&md5ctxt);

   last_percent = -1;
   count = sizeof(EccHeader);

   if(!LargeSeek(image->eccFile, (gint64)sizeof(EccHeader)))
     Stop(_("Failed skipping the ecc header: %s"),strerror(errno));

   while(!LargeEOF(image->eccFile))
   {  int n = LargeRead(image->eccFile, buf, 1024);

      MD5Update(&md5ctxt, buf, n);

      count += n;
      percent = (100*count)/image->eccFile->size;
      if(last_percent != percent) 
      {  PrintProgress(_("- ecc md5sum       : %3d%%"),percent);
	 GuiSetLabelText(wl->cmpEccMd5Sum, "%3d%%", percent);
	 last_percent = percent;
      }

      if(Closure->stopActions)   
      {  if(Closure->stopActions == STOP_CURRENT_ACTION) /* suppress memleak warning when closing window */
	 {  GuiSetLabelText(wl->cmpEccResult, 
			    _("<span %s>Aborted by user request!</span>"), Closure->redMarkup);
	 }
	 goto terminate;
      }
   }

   MD5Final(digest, &md5ctxt);
   AsciiDigest(edigest, digest);

   if(memcmp(eh->eccSum, digest, 16))
   {  PrintLog(_("* ecc md5sum       : BAD, ecc file may be damaged!\n"));
      GuiSetLabelText(wl->cmpEccMd5Sum, _("<span %s>bad</span>"), Closure->redMarkup);
      if(!ecc_advice)
	ecc_advice = g_strdup_printf(_("<span %s>Error correction file may be damaged!</span>"), Closure->redMarkup);
   }
   else 
   {  PrintLog(_("- ecc md5sum       : %s (good)\n"),edigest);
      GuiSetLabelText(wl->cmpEccMd5Sum, "%s", edigest);
   }

skip_ecc:
   PrintLog("\n");

   if(ecc_advice) 
   {  GuiSetLabelText(wl->cmpEccResult, "%s", ecc_advice);
      g_free(ecc_advice);
   }
   else
   {  GuiSetLabelText(wl->cmpEccResult,
		      _("<span %s>Good error correction file.</span>"), 
		      Closure->greenMarkup);
   }

   /*** Close and clean up */

terminate:
   cleanup(vc);
}
