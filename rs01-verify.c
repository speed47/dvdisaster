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

#include "rs01-includes.h"

/***
 *** Reset the verify output window
 ***/

void ResetRS01VerifyWindow(Method *self)
{  RS01Widgets *wl = (RS01Widgets*)self->widgetList;

   SetLabelText(GTK_LABEL(wl->cmpChkSumErrors), "-");
   SetLabelText(GTK_LABEL(wl->cmpMissingSectors), "0");
   SetLabelText(GTK_LABEL(wl->cmpImageMd5Sum), "-");
   SetLabelText(GTK_LABEL(wl->cmpImageResult), "");
   SwitchAndSetFootline(wl->cmpImageNotebook, 1, NULL, NULL);

   SetLabelText(GTK_LABEL(wl->cmpEccEmptyMsg), "");
   SetLabelText(GTK_LABEL(wl->cmpEccCreatedBy), "dvdisaster");
   SetLabelText(GTK_LABEL(wl->cmpEccMethod), "");
   SetLabelText(GTK_LABEL(wl->cmpEccRequires), "");
   SetLabelText(GTK_LABEL(wl->cmpEccMediumSectors), "");
   SetLabelText(GTK_LABEL(wl->cmpEccImgMd5Sum), "");
   SetLabelText(GTK_LABEL(wl->cmpEccFingerprint), _("n/a"));
   SetLabelText(GTK_LABEL(wl->cmpEccBlocks), "");
   SetLabelText(GTK_LABEL(wl->cmpEccMd5Sum), "");
   SetLabelText(GTK_LABEL(wl->cmpEccResult), "");
   SwitchAndSetFootline(wl->cmpEccNotebook, 0, NULL, NULL);

   wl->lastPercent = 0;

   FillSpiral(wl->cmpSpiral, Closure->background);
   DrawSpiral(wl->cmpSpiral);
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
     DrawSpiralSegment(sii->cmpSpiral, sii->segColor, i-1);

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
     SetLabelText(GTK_LABEL(wl->cmpMissingSectors), "<span %s>%lld</span>", 
		  Closure->redMarkup, totalMissing);

   if(newCrcErrors) 
     SetLabelText(GTK_LABEL(wl->cmpChkSumErrors), "<span %s>%lld</span>", 
		  Closure->redMarkup, totalCrcErrors);

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

   DrawSpiralLabel(wl->cmpSpiral, wl->cmpLayout,
		   _("Good sectors"), Closure->greenSector, x, 1);

   DrawSpiralLabel(wl->cmpSpiral, wl->cmpLayout,
		   _("Sectors with CRC errors"), Closure->yellowSector, x, 2);

   DrawSpiralLabel(wl->cmpSpiral, wl->cmpLayout,
		   _("Missing sectors"), Closure->redSector, x, 3);

   DrawSpiral(wl->cmpSpiral);
}

/*
 * expose event handler for the spiral
 */

static gboolean expose_cb(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{  RS01Widgets *wl = (RS01Widgets*)data;
   GtkAllocation *a = &widget->allocation;
   int w,h,size;

   /* Finish spiral initialization */

   if(!wl->cmpLayout)
   {  SetSpiralWidget(wl->cmpSpiral, widget);
      wl->cmpLayout = gtk_widget_create_pango_layout(widget, NULL);
   }

   SetText(wl->cmpLayout, _("Missing sectors"), &w, &h);
   size = wl->cmpSpiral->diameter + 20 + 3*(10+h);  /* approx. size of spiral + labels */

   wl->cmpSpiral->mx = a->width / 2;
   wl->cmpSpiral->my = (wl->cmpSpiral->diameter + a->height - size)/2;
   //   wl->cmpSpiral->my = wl->cmpSpiral->diameter/2 + 20;

   if(event->count) /* Exposure compression */
     return TRUE;

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
   SetLabelText(GTK_LABEL(lab), _("Medium sectors:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, 0, 1, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpImageSectors = gtk_label_new("0");
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Checksum errors:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, 1, 2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpChkSumErrors = gtk_label_new("0");
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Missing Sectors:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, 2, 3, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpMissingSectors = gtk_label_new("0");
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, 2, 3, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Image checksum:"));
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

   wl->cmpSpiral = CreateSpiral(Closure->grid, Closure->background, 10, 5, VERIFY_IMAGE_SEGMENTS);
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
   SetLabelText(GTK_LABEL(lab), _("Created by:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, 0, 1, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccCreatedBy = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Method:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, 1, 2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccMethod = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Requires:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, 2, 3, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccRequires = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, 2, 3, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Medium sectors:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, 3, 4, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccMediumSectors = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, 3, 4, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Image checksum:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, 4, 5, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccImgMd5Sum = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, 4, 5, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Fingerprint:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, 5, 6, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccFingerprint = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, 5, 6, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Ecc blocks:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, 6, 7, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccBlocks = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, 6, 7, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Ecc checksum:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, 7, 8, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccMd5Sum = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, 7, 8, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

   lab = wl->cmpEccResult = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0);
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 2, 8, 9, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 4);
}

/***
 *** Verify the prefix.* files
 ***/

typedef struct
{  ImageInfo *ii;
   EccInfo   *ei;
} verify_closure;

static void cleanup(gpointer data)
{  verify_closure *cc = (verify_closure*)data;

   Closure->cleanupProc = NULL;

   if(Closure->guiMode)
      AllowActions(TRUE);

   if(cc->ii) FreeImageInfo(cc->ii);
   if(cc->ei) FreeEccInfo(cc->ei);

   g_free(cc);

   if(Closure->guiMode)
     g_thread_exit(0);
}

void RS01Verify(Method *self)
{  RS01Widgets *wl = (RS01Widgets*)self->widgetList;
   verify_closure *cc = g_malloc0(sizeof(verify_closure));
   ImageInfo *ii;
   EccInfo *ei;
   char idigest[33],edigest[33]; 
   gint64 excess_sectors = 0;
   gint64 eh_sectors = 0;
   int n;
   char *ecc_advice = NULL;
   int in_last = 0;

   idigest[0] = 0;

   /*** Prepare for early termination */

   RegisterCleanup(_("Comparison aborted"), cleanup, cc);

   /*** Examine the .iso file */

   if(Closure->guiMode)
     SetLabelText(GTK_LABEL(wl->cmpHeadline), "<big>%s</big>\n<i>%s</i>",
		  _("Comparing image and error correction files."),
		  _("- Checking image file -"));

   PrintLog("\n%s: ", Closure->imageName);
   ei = cc->ei = OpenEccFile(READABLE_ECC | PRINT_MODE);
   ii = cc->ii = OpenImageFile(ei ? ei->eh : NULL, PRINT_MODE);

   if(ei)  
   {  eh_sectors = uchar_to_gint64(ei->eh->sectors);
      if(Closure->guiMode)
         SetLabelText(GTK_LABEL(wl->cmpChkSumErrors), "0");
   }
   else 
     if(Closure->guiMode)
       SetLabelText(GTK_LABEL(wl->cmpChkSumErrors), _("n/a"));

   if(!ii)
   {  PrintLog(_("not present\n"));

      if(Closure->guiMode)
	SwitchAndSetFootline(wl->cmpImageNotebook, 0, NULL, NULL);
   }
   else
   {  if(ii->inLast == 2048)
      {  PrintLog(_("present, contains %lld medium sectors.\n"), ii->sectors);
	 if(Closure->guiMode)
	   SetLabelText(GTK_LABEL(wl->cmpImageSectors), "%lld", ii->sectors);
      }
      else
      {  PrintLog(_("present, contains %lld medium sectors and %d bytes.\n"),
		  ii->sectors-1, ii->inLast);
	 if(Closure->guiMode)
	   SetLabelText(GTK_LABEL(wl->cmpImageSectors), _("%lld sectors + %d bytes"), 
			ii->sectors-1, ii->inLast);
      }

      RS01ScanImage(self, ii, ei, PRINT_MODE);

      if(Closure->stopActions)   
      {  SetLabelText(GTK_LABEL(wl->cmpImageResult), 
		      _("<span %s>Aborted by user request!</span>"),
		      Closure->redMarkup); 
         goto terminate;
      }

      /*** Peek into the ecc file to get expected sector count */

      if(ei)  
      {  gint64 diff = 0;

	 if(ii->sectors < eh_sectors)
	 {  diff = eh_sectors - ii->sectors;

	    PrintLog(_("* truncated image  : %lld sectors too short\n"), diff);
	    if(Closure->guiMode)
	      SetLabelText(GTK_LABEL(wl->cmpImageSectors), 
			   _("<span %s>%lld (%lld sectors too short)</span>"),
			   Closure->redMarkup, ii->sectors, diff);
	    ii->sectorsMissing += diff;
	 }
	 if(ii->sectors > eh_sectors)
	 {  excess_sectors = ii->sectors - eh_sectors;
	 }
      }

      /*** Show summary of image read */

      if(Closure->guiMode)
      {  if(ii->crcErrors)
	    SetLabelText(GTK_LABEL(wl->cmpChkSumErrors), 
			 "<span %s>%lld</span>", Closure->redMarkup, ii->crcErrors);
         if(ii->sectorsMissing)
	    SetLabelText(GTK_LABEL(wl->cmpMissingSectors), 
			 "<span %s>%lld</span>", Closure->redMarkup, ii->sectorsMissing);
      }

      if(excess_sectors)
      {  PrintLog(_("* image too long   : %lld excess sectors\n"), excess_sectors);
         if(Closure->guiMode)
	 {   SetLabelText(GTK_LABEL(wl->cmpImageSectors), 
			  _("<span %s>%lld (%lld excess sectors)</span>"),
			  Closure->redMarkup, ii->sectors, excess_sectors);
	     SetLabelText(GTK_LABEL(wl->cmpImageResult),
			  _("<span %s>Bad image.</span>"),
			  Closure->redMarkup);
	 }
      } 
      else
      {  if(!ii->sectorsMissing)
         {  
	    AsciiDigest(idigest, ii->mediumSum);
      
            if(!ii->crcErrors)
	    {  PrintLog(_("- good image       : all sectors present\n"
		          "- image md5sum     : %s\n"),idigest);
	       if(Closure->guiMode)
	       {  SetLabelText(GTK_LABEL(wl->cmpImageResult),_("<span %s>Good image.</span>"), Closure->greenMarkup);
	          SetLabelText(GTK_LABEL(wl->cmpImageMd5Sum), "%s", idigest);
	       }
	    }
	    else
            {  PrintLog(_("* suspicious image : all sectors present, but %lld CRC errors\n"
		          "- image md5sum     : %s\n"),ii->crcErrors,idigest);

	       if(Closure->guiMode)
	       {  SetLabelText(GTK_LABEL(wl->cmpImageResult), _("<span %s>Image complete, but contains checksum errors!</span>"), Closure->redMarkup);
	          SetLabelText(GTK_LABEL(wl->cmpImageMd5Sum), "%s", idigest);
	       }
	    }
         }
         else
         {  if(!ii->crcErrors)
	         PrintLog(_("* BAD image        : %lld sectors missing\n"), ii->sectorsMissing);
	    else PrintLog(_("* BAD image        : %lld sectors missing, %lld CRC errors\n"), 
		         ii->sectorsMissing, ii->crcErrors);
	    if(Closure->guiMode)
	      SetLabelText(GTK_LABEL(wl->cmpImageResult),
			   _("<span %s>Bad image.</span>"), Closure->redMarkup);
         }
      }
   }

   /*** The .ecc file */

   if(Closure->guiMode)
     SetLabelText(GTK_LABEL(wl->cmpHeadline), "<big>%s</big>\n<i>%s</i>",
		  _("Comparing image and error correction files."),
		  _("- Checking ecc file -"));

   PrintLog("\n%s: ", Closure->eccName);

   if(!ei)
   {  PrintLog(_("not present\n"));
      if(Closure->guiMode)
	SwitchAndSetFootline(wl->cmpEccNotebook, 0, 
			     wl->cmpEccEmptyMsg,_("No error correction file present."));
   }
   else
   {  EccHeader *eh = ei->eh;
      gint8 method[5];
      gint64 ecc_blocks,ecc_expected,count;
      struct MD5Context md5ctxt;
      unsigned char buf[1024];
      unsigned char digest[16];
      int percent,last_percent;
      gint64 ecc_file_size;

      if(!LargeStat(Closure->eccName, &ecc_file_size))
	Stop(_("Could not open %s: %s"), Closure->eccName, strerror(errno));

      //      strncpy(method, eh->method, 4); method[4] = 0;
      memcpy(method, eh->method, 4); method[4] = 0;

      if(!eh->neededVersion)  /* The V0.41.* series did not fill in this field. */
	eh->neededVersion = 4100;

      if(eh->creatorVersion)
      {  int major = eh->creatorVersion/10000; 
	 int minor = (eh->creatorVersion%10000)/100;
         int pl    = eh->creatorVersion%100;

         if(eh->creatorVersion%100)        
	 {  char *format, *color_format = NULL;

	    if(eh->creatorVersion < 6000) format = "%s-%d.%d.%d";
	    else if(eh->creatorVersion <= 6500) format = "%s-%d.%d (pl%d)";
	    else
	    {  if(eh->methodFlags[3] & MFLAG_DEVEL) 
	       {  format = "%s-%d.%d (devel-%d)";
		  color_format = "%s-%d.%d <span %s>(devel-%d)</span>";
	       }
	       else if(eh->methodFlags[3] & MFLAG_RC) 
	       {  format = "%s-%d.%d (rc-%d)";
	          color_format = "%s-%d.%d <span %s>(rc-%d)</span>";
	       }
	       else format = "%s-%d.%d (pl%d)";
	    }
	    PrintLog(format, _("created by dvdisaster"), major, minor, pl);
	    PrintLog("\n");

	    if(Closure->guiMode)
	    {  if(color_format)
		  SwitchAndSetFootline(wl->cmpEccNotebook, 1,
				       wl->cmpEccCreatedBy, 
				       color_format, "dvdisaster",
				       major, minor, Closure->redMarkup, pl);
	       else
		  SwitchAndSetFootline(wl->cmpEccNotebook, 1,
				       wl->cmpEccCreatedBy, 
				       format, "dvdisaster",
				       major, minor, pl);
	    }
	 }
	 else
	 {  PrintLog(_("created by dvdisaster-%d.%d\n"), 
		     major, minor);
	    if(Closure->guiMode)
	      SwitchAndSetFootline(wl->cmpEccNotebook, 1,
				   wl->cmpEccCreatedBy, "dvdisaster-%d.%d",
				   major, minor);
	 }
      }
      else
      {  PrintLog(_("created by dvdisaster-0.41.x.\n"));
         if(Closure->guiMode)
	   SwitchAndSetFootline(wl->cmpEccNotebook, 1,
				wl->cmpEccCreatedBy, "dvdisaster-0.41.x");
      }

      PrintLog(_("- method           : %4s, %d roots, %4.1f%% redundancy.\n"),
	       method, eh->eccBytes, 
	       ((double)eh->eccBytes*100.0)/(double)eh->dataBytes);
      if(Closure->guiMode)
	SetLabelText(GTK_LABEL(wl->cmpEccMethod), _("%4s, %d roots, %4.1f%% redundancy"),
		     method, eh->eccBytes, 
		     ((double)eh->eccBytes*100.0)/(double)eh->dataBytes);

      if(!VerifyVersion(eh, 0))
      {  PrintLog(_("- requires         : dvdisaster-%d.%d (good)\n"),
		  eh->neededVersion/10000,
		  (eh->neededVersion%10000)/100);
         if(Closure->guiMode)
	   SetLabelText(GTK_LABEL(wl->cmpEccRequires), "dvdisaster-%d.%d",
			eh->neededVersion/10000,
			(eh->neededVersion%10000)/100);
      }
      else 
      {  PrintLog(_("* requires         : dvdisaster-%d.%d (BAD)\n"
		    "* Warning          : The following output might be incorrect.\n"
		    "*                  : Please visit http://www.dvdisaster.com for an upgrade.\n"),
		  eh->neededVersion/10000,
		  (eh->neededVersion%10000)/100);

         if(Closure->guiMode)
	 {  SetLabelText(GTK_LABEL(wl->cmpEccRequires), 
			 "<span %s>dvdisaster-%d.%d</span>",
			 Closure->redMarkup,
			 eh->neededVersion/10000,
			 (eh->neededVersion%10000)/100);
	 if(!ecc_advice) 
	    ecc_advice = g_strdup_printf(_("<span %s>Please upgrade your version of dvdisaster!</span>"), Closure->redMarkup);
	 }
      }

      eh_sectors = uchar_to_gint64(eh->sectors);

      if(eh->creatorVersion >= 6600 && eh->inLast != 2048)  /* image file whose length is */
	in_last = eh->inLast;                               /* not a multiple of 2048 */

      if(ii)
      {  if(ii->sectors == eh_sectors && (!in_last || ii->inLast == eh->inLast))
	 {      if(!in_last)
	        {  PrintLog(_("- medium sectors   : %lld (good)\n"),eh_sectors);
	           if(Closure->guiMode)
		     SetLabelText(GTK_LABEL(wl->cmpEccMediumSectors), "%lld", eh_sectors);
		}
	        else
	        {  PrintLog(_("- medium sectors   : %lld sectors + %d bytes (good)\n"),
			    eh_sectors-1, in_last);
	           if(Closure->guiMode)
		     SetLabelText(GTK_LABEL(wl->cmpEccMediumSectors), 
				  _("%lld sectors + %d bytes"), 
				  eh_sectors-1, in_last);
		}
	 }
         else 
	 { if(ii->sectors > eh_sectors && ii->sectors - eh_sectors <= 2)   
	   {    PrintLog(_("* medium sectors   : %lld (BAD, perhaps TAO/DAO mismatch)\n"),eh_sectors);
	        if(Closure->guiMode)
		{  if(!in_last)  
		      SetLabelText(GTK_LABEL(wl->cmpEccMediumSectors), "<span %s>%lld</span>", Closure->redMarkup, eh_sectors);
		  else SetLabelText(GTK_LABEL(wl->cmpEccMediumSectors), "<span %s>%lld sectors + %d bytes</span>", Closure->redMarkup, eh_sectors-1, in_last);
		}
	   }
           else 
	   {    if(!in_last)
	        {  PrintLog(_("* medium sectors   : %lld (BAD)\n"),eh_sectors);
	           if(Closure->guiMode)
		   {  SetLabelText(GTK_LABEL(wl->cmpEccMediumSectors), "<span %s>%lld</span>", Closure->redMarkup, eh_sectors);
		      if(!ecc_advice)
			ecc_advice = g_strdup_printf(_("<span %s>Image size does not match error correction file.</span>"), Closure->redMarkup);
		   }
		}
	        else
	        {  PrintLog(_("* medium sectors   : %lld sectors + %d bytes (BAD)\n"),
			    eh_sectors-1, in_last);
	           if(Closure->guiMode)
		   {  SetLabelText(GTK_LABEL(wl->cmpEccMediumSectors), 
				   _("<span %s>%lld sectors + %d bytes</span>"), 
				   Closure->redMarkup, eh_sectors-1, in_last);
		      if(!ecc_advice)
			 ecc_advice = g_strdup_printf(_("<span %s>Image size does not match error correction file.</span>"), Closure->redMarkup);
		   }
		}
	   }
	 }
      }
      else
      {  if(!in_last)
	 {  PrintLog(_("- medium sectors   : %lld\n"),eh_sectors);
            if(Closure->guiMode)
	      SetLabelText(GTK_LABEL(wl->cmpEccMediumSectors), "%lld", eh_sectors);
	 }
	 else
	 {  PrintLog(_("- medium sectors   : %lld sectors + %d bytes\n"),
		     eh_sectors-1, in_last);
            if(Closure->guiMode)
	      SetLabelText(GTK_LABEL(wl->cmpEccMediumSectors), 
			   _("%lld sectors + %d bytes"), 
			   eh_sectors-1, in_last);
	 }
      }

      /*** Verify md5sums against image and map (if present) */

      AsciiDigest(edigest, eh->mediumSum);
      if(ii && !ii->sectorsMissing && !excess_sectors)
      {  n = !memcmp(eh->mediumSum, ii->mediumSum, 16);
         if(n) PrintLog(_("- image md5sum     : %s (good)\n"),edigest);
	 else  PrintLog(_("* image md5sum     : %s (BAD)\n"),edigest);
	 if(Closure->guiMode)
	 {  if(n) SetLabelText(GTK_LABEL(wl->cmpEccImgMd5Sum), "%s", edigest);
	    else  
	    {  SetLabelText(GTK_LABEL(wl->cmpEccImgMd5Sum), "<span %s>%s</span>", Closure->redMarkup, edigest);
	       SetLabelText(GTK_LABEL(wl->cmpImageMd5Sum), "<span %s>%s</span>", Closure->redMarkup, idigest);
	    }
	 }
      }
      else 
      {  PrintLog(_("- image md5sum     : %s\n"),edigest);
         if(Closure->guiMode)
	   SetLabelText(GTK_LABEL(wl->cmpEccImgMd5Sum), "%s", edigest);
      }

      if(ii)
      {  if(!memcmp(ii->mediumFP, "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000", 16))
	 {    PrintLog(_("* fingerprint match: NOT POSSIBLE - related sector is missing in image!\n"));
	      if(Closure->guiMode)
		 SetLabelText(GTK_LABEL(wl->cmpEccFingerprint), _("<span %s>missing sector prevents calculation</span>"), Closure->redMarkup);
	 }
         else
	 if(memcmp(ii->mediumFP, eh->mediumFP, 16)) 
	 {    PrintLog(_("* fingerprint match: MISMATCH - .iso and .ecc don't belong together!\n"));
	      if(Closure->guiMode)
	      {  SetLabelText(GTK_LABEL(wl->cmpEccFingerprint), 
			      _("<span %s>mismatch</span>"), Closure->redMarkup);

		 if(!ecc_advice)
		    ecc_advice = g_strdup_printf(_("<span %s>Image and error correction files do not belong together!</span>"), Closure->redMarkup);
	      }
	 }
         else 
	 {    PrintLog(_("- fingerprint match: good\n"));
	      if(Closure->guiMode)
		SetLabelText(GTK_LABEL(wl->cmpEccFingerprint), _("good"));
	 }
      }

      ecc_expected = 2048*((eh_sectors+eh->dataBytes-1)/eh->dataBytes);
      ecc_blocks = (ecc_file_size-eh_sectors*sizeof(guint32)-sizeof(EccHeader))/eh->eccBytes;

      if(ecc_expected == ecc_blocks)
      {    PrintLog(_("- ecc blocks       : %lld (good)\n"),ecc_blocks);
	   if(Closure->guiMode)
	     SetLabelText(GTK_LABEL(wl->cmpEccBlocks), "%lld", ecc_blocks);
      }
      else
      {    PrintLog(_("* ecc blocks       : %lld (BAD, expected %lld)\n"),ecc_blocks,ecc_expected);
	   if(Closure->guiMode)
	      SetLabelText(GTK_LABEL(wl->cmpEccBlocks), _("<span %s>%lld (bad, expected %lld)</span>"),Closure->redMarkup,ecc_blocks,ecc_expected);
      }

      /*** Test ecc file against its own md5sum */

      MD5Init(&md5ctxt);

      last_percent = -1;
      count = sizeof(EccHeader);

      if(!LargeSeek(ei->file, (gint64)sizeof(EccHeader)))
	Stop(_("Failed skipping the ecc header: %s"),strerror(errno));

      while(!LargeEOF(ei->file))
      {  n = LargeRead(ei->file, buf, 1024);
         MD5Update(&md5ctxt, buf, n);

	 count += n;
	 percent = (100*count)/ecc_file_size;
	 if(last_percent != percent) 
	 {  if(!Closure->guiMode)
	         PrintProgress(_("- ecc md5sum       : %3d%%"),percent);
 	    else SetLabelText(GTK_LABEL(wl->cmpEccMd5Sum), "%3d%%", percent);
	    last_percent = percent;
	 }

	 if(Closure->stopActions)   
	 {  SetLabelText(GTK_LABEL(wl->cmpEccResult), 
			 _("<span %s>Aborted by user request!</span>"), Closure->redMarkup); 
	   goto terminate;
	 }
      }
      MD5Final(digest, &md5ctxt);
      AsciiDigest(edigest, digest);

      if(memcmp(eh->eccSum, digest, 16))
      {	   PrintLog(_("* ecc md5sum       : BAD, ecc file may be damaged!\n"));
	   if(Closure->guiMode)
	   {  SetLabelText(GTK_LABEL(wl->cmpEccMd5Sum), _("<span %s>bad</span>"), Closure->redMarkup);
	      if(!ecc_advice)
		 ecc_advice = g_strdup_printf(_("<span %s>Error correction file may be damaged!</span>"), Closure->redMarkup);
	   }
      }
      else 
      {    PrintLog(_("- ecc md5sum       : %s (good)\n"),edigest);
	   if(Closure->guiMode)
	     SetLabelText(GTK_LABEL(wl->cmpEccMd5Sum), "%s", edigest);
      }
   }

   PrintLog("\n");

   if(Closure->guiMode)
   {  if(ecc_advice) 
      {  SetLabelText(GTK_LABEL(wl->cmpEccResult), ecc_advice);
         g_free(ecc_advice);
      }
      else SetLabelText(GTK_LABEL(wl->cmpEccResult),
		        _("<span %s>Good error correction file.</span>"), 
			Closure->greenMarkup);
   }

   /*** Close and clean up */

terminate:
   cleanup((gpointer)cc);
}
