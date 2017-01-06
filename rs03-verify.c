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

#include "rs03-includes.h"

#define EXIT_CODE_SIZE_MISMATCH 1
#define EXIT_CODE_VERSION_MISMATCH 2

#define EXIT_CODE_UNEXPECTED_EOF 10
#define EXIT_CODE_MISSING_SECTOR 11
#define EXIT_CODE_CHECKSUM_ERROR 12
#define EXIT_CODE_SYNDROME_ERROR 13

/***
 *** Reset the verify output window
 ***/

void ResetRS03VerifyWindow(Method *self)
{  RS03Widgets *wl = (RS03Widgets*)self->widgetList;

   SetLabelText(GTK_LABEL(wl->cmpImageSectors), "");
   SetLabelText(GTK_LABEL(wl->cmpImageMd5Sum), "");
   SetLabelText(GTK_LABEL(wl->cmpDataSection), "");
   SetLabelText(GTK_LABEL(wl->cmpCrcSection), "");
   SetLabelText(GTK_LABEL(wl->cmpEccSection), "");
   SetLabelText(GTK_LABEL(wl->cmpImageErasure), "");
   SetLabelText(GTK_LABEL(wl->cmpImagePrognosis), "");
   SetLabelText(GTK_LABEL(wl->cmpImageErasureCnt), "");
   SetLabelText(GTK_LABEL(wl->cmpImagePrognosisMsg), "");
   SetLabelText(GTK_LABEL(wl->cmpImageResult), "");
   SwitchAndSetFootline(wl->cmpImageNotebook, 1, NULL, NULL);

   SetLabelText(GTK_LABEL(wl->cmpEccCreatedBy), "dvdisaster");
   SetLabelText(GTK_LABEL(wl->cmpEccMethod), "");
   SetLabelText(GTK_LABEL(wl->cmpEccType), "");
   SetLabelText(GTK_LABEL(wl->cmpEccRequires), "");
   SetLabelText(GTK_LABEL(wl->cmpEccDataCrc), _("Data checksum:"));
   SetLabelText(GTK_LABEL(wl->cmpEccDataCrcVal), "");
   SetLabelText(GTK_LABEL(wl->cmpEccFingerprint), _("n/a"));
   SetLabelText(GTK_LABEL(wl->cmpEccResult), "");
   SetLabelText(GTK_LABEL(wl->cmpEccSynLabel), "");
   SetLabelText(GTK_LABEL(wl->cmpEccSyndromes), "");

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

static void add_verify_values(Method *method, int percent, 
			       gint64 newMissing, gint64 newCrcErrors)
{  RS03Widgets *wl = (RS03Widgets*)method->widgetList;
   spiral_idle_info *sii = g_malloc(sizeof(spiral_idle_info));

   if(percent < 0 || percent > VERIFY_IMAGE_SEGMENTS)
     return;

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

static void redraw_spiral(RS03Widgets *wl)
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
{  RS03Widgets *wl = (RS03Widgets*)data;
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

   if(!event->count)      /* Exposure compression */
     redraw_spiral(wl);   /* Redraw the spiral */

   return TRUE;
}

/***
 *** Create the notebook contents for the verify output
 ***/

void CreateRS03VerifyWindow(Method *self, GtkWidget *parent)
{  RS03Widgets *wl = (RS03Widgets*)self->widgetList;
   GtkWidget *sep,*notebook,*ignore,*table,*table2,*lab,*frame,*d_area;
   int y1,y2;

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


   /*** Ecc data info */

   frame = gtk_frame_new(_utf("Error correction properties"));
   gtk_table_attach(GTK_TABLE(table), frame, 0, 1, 0, 1, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 5);

   table2 = gtk_table_new(2, 7, FALSE);
   ignore = gtk_label_new("ecc info");
   gtk_container_set_border_width(GTK_CONTAINER(table2), 5);
   gtk_container_add(GTK_CONTAINER(frame), table2);
   y1=0; y2=1;

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Type:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccType = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Method:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccMethod = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Created by:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccCreatedBy = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Requires:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccRequires = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = wl->cmpEccDataCrc = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Data checksum:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccDataCrcVal = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Fingerprint:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccFingerprint = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = wl->cmpEccResult = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0);
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 2, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 4);

   /*** Image spiral */

   frame = gtk_frame_new(_utf("Image state"));
   gtk_table_attach(GTK_TABLE(table), frame, 1, 2, 0, 2, GTK_SHRINK | GTK_FILL, GTK_EXPAND | GTK_FILL, 5, 5);

   wl->cmpSpiral = CreateSpiral(Closure->grid, Closure->background, 10, 5, VERIFY_IMAGE_SEGMENTS);
   d_area = wl->cmpDrawingArea = gtk_drawing_area_new();
   gtk_widget_set_size_request(d_area, wl->cmpSpiral->diameter+20, -1);
   gtk_container_add(GTK_CONTAINER(frame), d_area);
   g_signal_connect(G_OBJECT(d_area), "expose_event", G_CALLBACK(expose_cb), (gpointer)wl);

   /*** Image info */

   frame = gtk_frame_new(_utf("Data integrity"));
   gtk_table_attach(GTK_TABLE(table), frame, 0, 1, 1, 2, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 5, 5);

   notebook = wl->cmpImageNotebook = gtk_notebook_new();
   gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
   gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);
   gtk_container_add(GTK_CONTAINER(frame), notebook);

   ignore = gtk_label_new("no image");
   lab = gtk_label_new(_utf("No image present."));
   gtk_notebook_append_page(GTK_NOTEBOOK(notebook), lab, ignore);

   table2 = gtk_table_new(2, 9, FALSE);
   ignore = gtk_label_new("image info");
   gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table2, ignore);
   gtk_container_set_border_width(GTK_CONTAINER(table2), 5);
   y1=0; y2=1;

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Medium sectors:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpImageSectors = gtk_label_new("0");
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Data checksum:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpImageMd5Sum = gtk_label_new("0");
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Data section:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpDataSection = gtk_label_new(".");
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Crc section:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpCrcSection = gtk_label_new(".");
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Ecc section:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccSection= gtk_label_new(".");
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = wl->cmpEccSynLabel = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0);
   SetLabelText(GTK_LABEL(lab), _("Ecc block test:")); 
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccSyndromes = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = wl->cmpImageErasure = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpImageErasureCnt = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = wl->cmpImagePrognosis = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpImagePrognosisMsg = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = wl->cmpImageResult = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0);
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 2, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 4);
}

/***
 *** Check the consistency of the augmented image
 ***/

/* 
 * housekeeping
 */

typedef struct
{  Image *image;
   EccHeader *eh;
   RS03Layout *lay;
   RS03Widgets *wl;
   CrcBuf *crcBuf;
   Bitmap *map;
   unsigned char crcSum[16];
   unsigned char *eccBlock[256];
   GaloisTables *gt;
   ReedSolomonTables *rt;
} verify_closure;

static void cleanup(gpointer data)
{  verify_closure *vc = (verify_closure*)data;
   int i;

   UnregisterCleanup();

   if(Closure->guiMode)
      AllowActions(TRUE);

   if(vc->image) CloseImage(vc->image);
   if(vc->lay) 
   {  g_free(vc->lay);
   }
   if(vc->map) FreeBitmap(vc->map);
   if(vc->crcBuf) FreeCrcBuf(vc->crcBuf);

   for(i=0; i<255; i++)
      if(vc->eccBlock[i])
	 g_free(vc->eccBlock[i]);

   if(vc->gt) FreeGaloisTables(vc->gt);
   if(vc->rt) FreeReedSolomonTables(vc->rt);

   g_free(vc);

   if(Closure->guiMode)
     g_thread_exit(0);
}

/***
 *** Prognosis for correctability
 ***/

static int prognosis(verify_closure *vc, gint64 missing, gint64 expected)
{  int j,eccblock;
   int worst_ecc = 0;
   gint64 damaged_sectors = 0, damaged_eccsecs = 0;
   gint64 correctable = 0;
   gint64 recoverable;

   for(eccblock=0; eccblock<vc->lay->sectorsPerLayer; eccblock++)
   {  int count = 255;

      /* Note: ecc file sectors are virtually mapped to augmented image sectors */

      for(j=0; j<255; j++)
      {  gint64 sector = j*vc->lay->sectorsPerLayer+eccblock;

	 if(GetBit(vc->map, sector))
	      count--;
	 else damaged_sectors++;
      }

      if(count>0)                damaged_eccsecs++;
      if(count>worst_ecc)        worst_ecc = count; 
      if(count<=vc->lay->nroots) correctable += count;
   }

   recoverable = expected - missing + correctable;

   if(damaged_sectors > 0)
   {  int percentage  = (1000*recoverable) / expected;

      PrintLog(_("- erasure counts   :  avg =  %.1f; worst = %d per ecc block.\n"),
	      (double)damaged_sectors/(double)damaged_eccsecs,worst_ecc);

      PrintLog(_("- prognosis        : %lld of %lld sectors recoverable (%d.%d%%)\n"),
	       recoverable, expected, percentage/10, percentage%10);

      if(Closure->guiMode)
      {  SetLabelText(GTK_LABEL(vc->wl->cmpImageErasure), _("Erasure counts:"));
	 SetLabelText(GTK_LABEL(vc->wl->cmpImagePrognosis), _("Prognosis:"));

	 SetLabelText(GTK_LABEL(vc->wl->cmpImageErasureCnt),
		      _("<span %s>avg =  %.1f; worst = %d per ecc block.</span>"),
		      worst_ecc <= vc->lay->nroots ? Closure->greenMarkup : Closure->redMarkup,
		      (double)damaged_sectors/(double)damaged_eccsecs,worst_ecc);

	 SetLabelText(GTK_LABEL(vc->wl->cmpImagePrognosisMsg),
		     _("<span %s>%lld of %lld sectors recoverable (%d.%d%%)</span>"),
		      recoverable < expected ? Closure->redMarkup : Closure->greenMarkup,
		     recoverable, expected, percentage/10, percentage%10);
      }
   }	      

   if(damaged_sectors && worst_ecc <= vc->lay->nroots && recoverable >= expected)
        return TRUE;
   else return FALSE;
}

/***
 *** Error syndrome check
 ***/

static int check_syndromes(verify_closure *vc)
{  RS03Layout *lay = vc->lay;
   Image *image = vc->image;
   gint64 li,ecc_block;
   gint64 cache_idx = Closure->prefetchSectors;
   gint64 ecc_good, ecc_bad, ecc_bad_sub;
   int percent,last_percent = -1;
   int bad_counted;
   int layer,i,j;

   if(Closure->guiMode)
     SetLabelText(GTK_LABEL(vc->wl->cmpHeadline), "<big>%s</big>\n<i>%s</i>",
		  _("Checking the image and error correction files."),
		  _("- Checking ecc blocks (deep verify) -"));

   /* Allocate buffers and initialize layer sector addresses */

   for(i=0, li=0; i<GF_FIELDMAX; i++,li+=lay->sectorsPerLayer)
   {  
      vc->eccBlock[i] = g_try_malloc(2048*Closure->prefetchSectors);
      if(!vc->eccBlock[i])  /* out of memory */
      {  int j;

	 for(j=0; j<i; j++)
	    g_free(vc->eccBlock[j]);

	 if(Closure->guiMode)
	   SetLabelText(GTK_LABEL(vc->wl->cmpEccSyndromes),
			_("<span %s>Out of memory; try reducing sector prefetch!</span>"),
			Closure->redMarkup);
	 PrintLog(_("* Ecc block test   : out of memory; try reducing sector prefetch!\n"));
	 return 0;
      }
   }

   /* Init Reed-Solomon tables */

   vc->gt = CreateGaloisTables(RS_GENERATOR_POLY);
   vc->rt = CreateReedSolomonTables(vc->gt, RS_FIRST_ROOT, RS_PRIM_ELEM, lay->nroots);

   /* Check the error syndromes */

   ecc_good = ecc_bad = ecc_bad_sub = 0;

   for(ecc_block=0; ecc_block<lay->sectorsPerLayer; ecc_block++)
   {  gint64 num_sectors = 0; 
      unsigned char data[GF_FIELDMAX];

      /* Check for user interruption */

      if(Closure->stopActions)   
      {  if(Closure->stopActions == STOP_CURRENT_ACTION) /* suppress memleak warning when closing window */
	    SetLabelText(GTK_LABEL(vc->wl->cmpEccSyndromes), 
			 _("<span %s>Aborted by user request!</span>"),
			 Closure->redMarkup); 
         return 0;
      }

      /* Reload cache? */
      
      if(cache_idx == Closure->prefetchSectors)
      {  
	 cache_idx = 0;
	 num_sectors = Closure->prefetchSectors;
	 if(ecc_block+num_sectors >= lay->sectorsPerLayer)
	    num_sectors = lay->sectorsPerLayer - ecc_block;

	 for(layer=0; layer<GF_FIELDMAX; layer++)
	   if(layer < lay->ndata-1)
	     RS03ReadSectors(image, vc->lay, vc->eccBlock[layer], 
			    layer, ecc_block, num_sectors, RS03_READ_DATA);
	   else
	     RS03ReadSectors(image, vc->lay, vc->eccBlock[layer], 
			    layer, ecc_block, num_sectors, RS03_READ_CRC | RS03_READ_ECC);
      }

      /* Calculate the error syndromes.
	 Note that we are only called when the image does not contain
	 dead sector markers; therefore we can skip this test. */

      bad_counted = FALSE;

      for(i=0; i<2048; i++) 
      {  int result;

	 for(j=0; j<GF_FIELDMAX; j++)
	   data[j] = vc->eccBlock[j][2048*cache_idx+i];

	 result = TestErrorSyndromes(vc->rt, data);

	 if(result)
	 {  ecc_bad_sub++;
	    if(!bad_counted)
	    {  bad_counted++;
	       ecc_bad++;
	    }
	 }
      }
      cache_idx++;

      if(!bad_counted) ecc_good++;

      /* Advance percentage gauge */

      percent = (100*(ecc_block+1))/lay->sectorsPerLayer;
      if(percent != last_percent)
      {  last_percent = percent;

	 if(!ecc_bad)
	 {  if(Closure->guiMode)
	      SetLabelText(GTK_LABEL(vc->wl->cmpEccSyndromes),
			   _("%d%% tested"),
			   percent);
	    PrintProgress(_("- Ecc block test   : %d%% tested"), percent);

	 }
	 else
	 {  if(Closure->guiMode)
	      SetLabelText(GTK_LABEL(vc->wl->cmpEccSyndromes),
			   _("<span %s>%lld good, %lld bad; %d%% tested</span>"),
			   Closure->redMarkup, ecc_good, ecc_bad, percent);
	    PrintProgress(_("* Ecc block test   : %lld good, %lld bad; %d%% tested")
			  , ecc_good, ecc_bad, percent);
	 }
      }
   }

   /* Tell user about our findings */

   if(!ecc_bad)
   {  if(Closure->guiMode)
       SetLabelText(GTK_LABEL(vc->wl->cmpEccSyndromes),_("pass"));
      ClearProgress();
      PrintLog(_("- Ecc block test   : pass\n"));
   }
   else
   {  if(Closure->guiMode)
       SetLabelText(GTK_LABEL(vc->wl->cmpEccSyndromes),
		    _("<span %s>%lld good, %lld bad; %lld bad sub blocks</span>"),
		    Closure->redMarkup, ecc_good, ecc_bad, ecc_bad_sub);
      PrintLog(_("* Ecc block test   : %lld good, %lld bad; %lld bad sub blocks\n"),
	       ecc_good, ecc_bad, ecc_bad_sub);

      exitCode = EXIT_CODE_SYNDROME_ERROR;
   }
   return ecc_bad;
}

/***
 *** The verify action
 ***/

void RS03Verify(Image *image)
{  Method *self = FindMethod("RS03");
   verify_closure *vc = g_malloc0(sizeof(verify_closure));
   RS03Widgets *wl = self->widgetList;
   EccHeader *eh = NULL;
   RS03Layout *lay;
   RS03CksumClosure *csc;
   struct MD5Context image_md5;
   unsigned char medium_sum[16];
   char data_digest[33], hdr_digest[33];
   gint64 s, crc_idx;
   int last_percent = 0;
   unsigned char buf[2048];
   gint64 first_missing, last_missing;
   gint64 total_missing,data_missing,crc_missing,ecc_missing;
   gint64 new_missing = 0, new_crc_errors = 0;
   gint64 data_crc_errors;
   gint64 virtual_expected;
   gint64 expected_image_sectors;
   gint64 eccfile_sectors = 0,expected_eccfile_sectors = 0;
   int major,minor,pl;
   char method[5];
   char *img_advice = NULL;
   char *ecc_advice = NULL;
   char *version;
   int syn_error = 0;
   int try_it;
   int missing_sector_explained = 0;
   int matching_byte_size = TRUE;

   /*** Prepare for early termination */

   RegisterCleanup(_("Check aborted"), cleanup, vc);
   vc->image = image;
   vc->wl = wl;

   if(image->eccFileHeader && !strncmp((char*)(image->eccFileHeader->method), "RS03", 4)) 
   {      eh = image->eccFileHeader;
   }
   else if(image->eccHeader && !strncmp((char*)(image->eccHeader->method), "RS03", 4)) 
   {      eh = image->eccHeader;
   }
   else Stop("Internal error: RS03Verify() called without suitable image and ecc file");
   
   vc->eh = eh; 

   /*** Announce type of error correction and what we are going to do */

   if(image->file)
   {
      PrintLog("\n%s present.\n", Closure->imageName);
   }
   else  /* may only happen when ecc file is present */
   {  PrintLog("\n%s not present.\n", Closure->imageName);

      if(Closure->guiMode)
	SwitchAndSetFootline(wl->cmpImageNotebook, 0, NULL, NULL);
   }

   if(eh->methodFlags[0] & MFLAG_ECC_FILE)
   {
      if(Closure->guiMode)
	SetLabelText(GTK_LABEL(wl->cmpHeadline), "<big>%s</big>\n<i>%s</i>",
		     _("Checking the image and error correction files."),
		     _("- Checking image file -"));

      PrintLog(_("%s present.\n"), Closure->eccName);
   }
   else 
   {  
     if(Closure->guiMode)
       SetLabelText(GTK_LABEL(wl->cmpHeadline), "<big>%s</big>\n<i>%s</i>",
		    _("Checking the image file."),
		    _("- Checking image file -"));
   }

   /*** Calculate the layout */

   if(eh->methodFlags[0] & MFLAG_ECC_FILE)
        lay = vc->lay = CalcRS03Layout(image, ECC_FILE); 
   else lay = vc->lay = CalcRS03Layout(image, ECC_IMAGE); 

   /*** Print information on the ecc portion */

   PrintLog(_("\nError correction properties:\n"));

   /* Check size of error correction file */

   if(lay->target == ECC_FILE)
   {  eccfile_sectors = image->eccFile->size / 2048;
      expected_eccfile_sectors = 2 + (lay->nroots+1)*lay->sectorsPerLayer;

      if(expected_eccfile_sectors != eccfile_sectors)
      {  char *msg;

	 if(expected_eccfile_sectors > eccfile_sectors)
	      msg = g_strdup_printf(_("Ecc file is %lld sectors shorter than expected."),
				  expected_eccfile_sectors - eccfile_sectors);
	 else msg = g_strdup_printf(_("Ecc file is %lld sectors longer than expected."), 
				    eccfile_sectors - expected_eccfile_sectors);

	 if(Closure->guiMode)
	    ecc_advice = g_strdup_printf("<span %s>%s</span>", Closure->redMarkup, msg);

	 PrintLog(_("* Warning          : %s\n"), msg);
	 g_free(msg);
	 exitCode = EXIT_CODE_SIZE_MISMATCH;
      }
   }
   
   /* Error correction type */

   if(eh->methodFlags[0] & MFLAG_ECC_FILE)
        PrintLog(_("- type             : Error correction file\n"));
   else PrintLog(_("- type             : Augmented image\n"));

   if(Closure->guiMode)
   {  if(eh->methodFlags[0] & MFLAG_ECC_FILE)
	   SetLabelText(GTK_LABEL(wl->cmpEccType), _("Error correction file"));
      else SetLabelText(GTK_LABEL(wl->cmpEccType), _("Augmented image"));
   }

   /* Error correction method */

   memcpy(method, eh->method, 4); method[4] = 0;

   PrintLog(_("- method           : %4s, %d roots, %4.1f%% redundancy.\n"),
	    method, eh->eccBytes, 
	    ((double)eh->eccBytes*100.0)/(double)eh->dataBytes);

   if(Closure->guiMode)
     SetLabelText(GTK_LABEL(wl->cmpEccMethod), _("%4s, %d roots, %4.1f%% redundancy"),
		  method, eh->eccBytes, 
		  ((double)eh->eccBytes*100.0)/(double)eh->dataBytes);

   /* Creator version */

   major = eh->creatorVersion/10000; 
   minor = (eh->creatorVersion%10000)/100;
   pl    = eh->creatorVersion%100;

   if(eh->creatorVersion%100)        
   {  char *format, *color_format = NULL;

      if(eh->methodFlags[3] & MFLAG_DEVEL) 
      {  format = "%s-%d.%d (devel-%d)";
	 color_format = "%s-%d.%d <span %s>(devel-%d)</span>";
      }
      else if(eh->methodFlags[3] & MFLAG_RC) 
      {  format = "%s-%d.%d (rc-%d)";
	 color_format = "%s-%d.%d <span %s>(rc-%d)</span>";
      }
      else format = "%s-%d.%d (pl%d)";

      PrintLog(format, _("- created by       : dvdisaster"), major, minor, pl);
      PrintLog("\n");

      if(!color_format) color_format = format;
      if(Closure->guiMode)
      {  if(!color_format)
	      SetLabelText(GTK_LABEL(wl->cmpEccCreatedBy), color_format, 
			   "dvdisaster", major, minor, Closure->redMarkup, pl);
	 else SetLabelText(GTK_LABEL(wl->cmpEccCreatedBy), format, 
			   "dvdisaster", major, minor, pl);
      }
   }
   else
   {  PrintLog(_("- created by       : dvdisaster-%d.%d\n"), 
	       major, minor);

      if(Closure->guiMode)
	SetLabelText(GTK_LABEL(wl->cmpEccCreatedBy), "dvdisaster-%d.%d", major, minor);
   }

   /* Required dvdisaster version */

   if(eh->neededVersion%100)        
        version = g_strdup_printf("%d.%d (pl%d)",
				  eh->neededVersion/10000,
				  (eh->neededVersion%10000)/100,
				  eh->neededVersion%100);
   else version = g_strdup_printf("%d.%d",
				  eh->neededVersion/10000,
				  (eh->neededVersion%10000)/100);

   if(Closure->version >= eh->neededVersion)
   {  PrintLog(_("- requires         : dvdisaster-%s\n"), version);

      if(Closure->guiMode)
	SetLabelText(GTK_LABEL(wl->cmpEccRequires), "dvdisaster-%s", version);
   }
   else 
   {  PrintLog(_("* requires         : dvdisaster-%s (BAD)\n"
		 "* Warning          : The following output might be incorrect.\n"
		 "*                  : Please visit http://www.dvdisaster.org for an upgrade.\n"),
	       version);

     if(Closure->guiMode)
     {  SetLabelText(GTK_LABEL(wl->cmpEccRequires), 
		     "<span %s>dvdisaster-%s</span>",
		     Closure->redMarkup, version);
        if(!ecc_advice) 
	   ecc_advice = g_strdup_printf(_("<span %s>Please upgrade your version of dvdisaster!</span>"), Closure->redMarkup);
     }

     exitCode = EXIT_CODE_VERSION_MISMATCH;
   }

   g_free(version);

   /* image md5sum as stored in the ecc header */

   if(eh->methodFlags[0] & MFLAG_DATA_MD5)
        AsciiDigest(hdr_digest, eh->mediumSum);
   else strcpy(hdr_digest, _("none available"));

   PrintLog(_("- data md5sum      : %s\n"),hdr_digest);

   if(Closure->guiMode)
     SetLabelText(GTK_LABEL(wl->cmpEccDataCrcVal), "%s", hdr_digest);

   /* compare images in ecc file case */

   if(eh->methodFlags[0] & MFLAG_ECC_FILE)
   {  if(image && image->file)
      {  if(image->fpState != FP_PRESENT)
         {  PrintLog(_("* fingerprint match: NOT POSSIBLE - related sector is missing in image!\n"));

	    if(Closure->guiMode)
	      SetLabelText(GTK_LABEL(wl->cmpEccFingerprint), _("<span %s>missing sector prevents calculation</span>"), Closure->redMarkup);
	}
	else
	{ 
	   if(memcmp(image->imageFP, eh->mediumFP, 16)) 
	   {  PrintLog(_("* fingerprint match: MISMATCH - .iso and .ecc don't belong together!\n"));

	      if(Closure->guiMode)
	      {  SetLabelText(GTK_LABEL(wl->cmpEccFingerprint), 
			      _("<span %s>mismatch</span>"), Closure->redMarkup);

		if(!ecc_advice)
		  ecc_advice = g_strdup_printf(_("<span %s>Image and error correction files do not belong together!</span>"), Closure->redMarkup);
	      }
	   }
	   else 
	   {  PrintLog(_("- fingerprint match: good\n"));
	      if(Closure->guiMode)
		SetLabelText(GTK_LABEL(wl->cmpEccFingerprint), _("good"));
	   }
	}
      }
   }
   
   /* print advice collected from above tests */

   if(Closure->guiMode)
   {  if(ecc_advice) 
      {  SetLabelText(GTK_LABEL(wl->cmpEccResult), ecc_advice);
         g_free(ecc_advice);
      }
      else SetLabelText(GTK_LABEL(wl->cmpEccResult),
			_("<span %s>Good error correction data.</span>"),
			Closure->greenMarkup);
   }

   if(!image->file)  /* Ecc file but no image */
      goto terminate;

   /*** Print information on image size */

   PrintLog(_("\nData integrity:\n"));

   /* Provide enough bitmap space for all layers */

   vc->map = CreateBitmap0(GF_FIELDMAX*lay->sectorsPerLayer);

   /* Expected and real sectors */

   if(lay->target == ECC_FILE)
   {  //expected_sectors = lay->dataSectors + lay->totalSectors;  /* image + ecc file */
      virtual_expected = GF_FIELDMAX*lay->sectorsPerLayer;      /* for prognosis map */
      expected_image_sectors = lay->dataSectors;                /* just the expected image size */
      if(eh->inLast != image->inLast)
	 matching_byte_size = FALSE;
   }
   else 
   {  virtual_expected = expected_image_sectors = lay->totalSectors;
      SetBit(vc->map, lay->eccHeaderPos);
      SetBit(vc->map, lay->eccHeaderPos+1);
   }

   /* Image size and expected size in ecc file match */
   
   if(expected_image_sectors == image->sectorSize && matching_byte_size)
   {  if(lay->target == ECC_FILE)
      {  if(Closure->guiMode)
	 {  if(image->inLast == 2048)
	      SetLabelText(GTK_LABEL(wl->cmpImageSectors), _("%lld in image; %lld in ecc file"), 
			   image->sectorSize, eccfile_sectors);
	    else
	      SetLabelText(GTK_LABEL(wl->cmpImageSectors), _("%lld sectors + %d bytes in image; %lld in ecc file"), 
			   image->sectorSize-1, image->inLast, eccfile_sectors);
	 }

	 if(image->inLast == 2048)
	      PrintLog(_("- sectors          : %lld in image; "), image->sectorSize);
	 else PrintLog(_("- sectors          : %lld sectors + %d bytes in image; "), image->sectorSize-1, image->inLast);

	 PrintLog(_("%lld in ecc file\n"), eccfile_sectors);
      }
      else 
      {  if(Closure->guiMode)
	   SetLabelText(GTK_LABEL(wl->cmpImageSectors), _("%lld total / %lld data"), 
			image->sectorSize, lay->dataSectors);
	 PrintLog(_("- medium sectors   : %lld total / %lld data\n"),
		  image->sectorSize, lay->dataSectors);
      }
   }
   else  /* Mismatch between image size and expected size in ecc file */
   {  char *image_size, *expected_size;

      if(image->inLast == 2048)
	image_size = g_strdup_printf("%lld", (long long int)image->sectorSize);
      else image_size = g_strdup_printf("%lld sectors + %d bytes", (long long int)image->sectorSize-1, image->inLast);

      if(eh->inLast == 2048)
           expected_size = g_strdup_printf("%lld", (long long int)expected_image_sectors);
      else expected_size = g_strdup_printf("%lld sectors + %d bytes", (long long int)expected_image_sectors-1, eh->inLast);

      if(Closure->guiMode)
      {  SetLabelText(GTK_LABEL(wl->cmpImageSectors), _("<span %s>%s (%s expected)</span>"), 
		      Closure->redMarkup, image_size, expected_size);

	 if(image->sectorSize == expected_image_sectors)
	 {  if(image->inLast < eh->inLast)
	         img_advice = g_strdup_printf(_("<span %s>Image file is %d bytes shorter than expected.</span>"),
					      Closure->redMarkup, eh->inLast-image->inLast);
	    else img_advice = g_strdup_printf(_("<span %s>Image file is %d bytes longer than expected.</span>"),
					      Closure->redMarkup, image->inLast-eh->inLast);
	 }

	 if(expected_image_sectors > image->sectorSize)
	    img_advice = g_strdup_printf(_("<span %s>Image file is %lld sectors shorter than expected.</span>"),
					 Closure->redMarkup, expected_image_sectors - image->sectorSize);
	 if(expected_image_sectors < image->sectorSize)
	    img_advice = g_strdup_printf(_("<span %s>Image file is %lld sectors longer than expected.</span>"),
					 Closure->redMarkup, image->sectorSize - expected_image_sectors);
      }

      if(lay->target == ECC_FILE)
	PrintLog(_("* sectors          : %s (%s expected); %lld sectors in ecc file\n"),
		 image_size, expected_size, eccfile_sectors);
      else
	PrintLog(_("* medium sectors   : %s (%s expected)\n"),
		 image_size, expected_size);
      g_free(image_size);
      g_free(expected_size);
   }
   
   if(Closure->quickVerify)
   {  PrintLog(_("* quick mode        : image NOT scanned\n"));
      goto terminate;
   }

   /*** Read the CRC portion */ 

   vc->crcBuf = self->getCrcBuf(vc->image);
   csc = (RS03CksumClosure*)self->ckSumClosure;

   /*** Check the data portion of the image file for the
	"dead sector marker" and CRC errors */
   
   if(!LargeSeek(image->file, 0))
     Stop(_("Failed seeking to start of image: %s\n"), strerror(errno));

   if(lay->target == ECC_FILE)
     if(!LargeSeek(image->eccFile, 4096))  /* skip the header */
       Stop(_("Failed seeking to start of ecc file: %s\n"), strerror(errno));

   MD5Init(&image_md5);

   first_missing = last_missing = -1;
   total_missing = data_missing = crc_missing = ecc_missing = 0;
   data_crc_errors = 0;
   crc_idx = 0;

   for(s=0; s<virtual_expected; s++)
   {  int percent,current_missing;
      int defective = 0;

      /* Check for user interruption */

      if(Closure->stopActions)   
      {  if(Closure->stopActions == STOP_CURRENT_ACTION) /* suppress memleak warning when closing window */
	    SetLabelText(GTK_LABEL(wl->cmpImageResult), 
			 _("<span %s>Aborted by user request!</span>"),
			 Closure->redMarkup); 
         goto terminate;
      }

      /* Read the next sector */

      RS03ReadSectors(image, vc->lay, buf,
		      s/vc->lay->sectorsPerLayer,
		      s%vc->lay->sectorsPerLayer,
		      1,
		      RS03_READ_DATA|RS03_READ_CRC|RS03_READ_ECC);

      /* update the MD5 sum */

      if(s < lay->dataSectors)
      {  if(s < lay->dataSectors - 1)
	      MD5Update(&image_md5, buf, 2048);
	 else MD5Update(&image_md5, buf, eh->inLast);
      }

      /* Look for the dead sector marker */

      current_missing = CheckForMissingSector(buf, s, eh->mediumFP, eh->fpSector);

      /* Truncated images and ecc files may create "legal" dead sectors. */

      if(current_missing != SECTOR_PRESENT)
      {  int dead_sector_from_truncation = 0;
	 guint64 real_sector = s;
	
	 if(lay->target == ECC_FILE)
	 {   if(s>=lay->dataSectors)
	     {  real_sector = s - (lay->ndata-1)*lay->sectorsPerLayer + 2;
	        if(real_sector*2048 >= image->eccFile->size)
		  dead_sector_from_truncation = 1;
	     }
	 }

	 if(!dead_sector_from_truncation)
	 {  int source_type = SOURCE_IMAGE;

	    if(lay->target == ECC_FILE && s>=lay->dataSectors)
	      source_type = SOURCE_ECCFILE;
	 
	    ExplainMissingSector(buf, real_sector, current_missing, source_type, &missing_sector_explained);
	 }
      }

      if(current_missing)
      {  
	 if(first_missing < 0) first_missing = s;
         last_missing = s;
	 total_missing++;
	 new_missing++;

	 if(lay->target == ECC_IMAGE)
	 {  if(s < lay->firstCrcPos) data_missing++;
	    else if(s >= lay->firstCrcPos && s < lay->firstEccPos) crc_missing++;
	    else ecc_missing++;
	 }
	 else  /* ecc file case */
	 {  if(s < lay->dataSectors) data_missing++;
	    else if(s < lay->ndata*lay->sectorsPerLayer) crc_missing++;
	    else ecc_missing++;
	 }
	 defective = TRUE;
	 exitCode = EXIT_CODE_MISSING_SECTOR;
      }

      /* Report dead sectors. Combine subsequent missing sectors into one report. */

      if(!current_missing || s==virtual_expected-1)
      {  if(first_missing>=0)
	{   gint64 first, last;
	    char *ecc_msg;
	
	    if(lay->target == ECC_FILE && last_missing >= (lay->ndata-1)*lay->sectorsPerLayer)
	    {    first = first_missing - (lay->ndata-1)*lay->sectorsPerLayer + 2;
	         last = last_missing - (lay->ndata-1)*lay->sectorsPerLayer + 2;
	         ecc_msg = g_strdup(_(" (in ecc file)"));
	    }
	    else 
	    {    first = first_missing;
	         last = last_missing;
		 ecc_msg = g_strdup(" ");
	    }
	    if(first_missing == last_missing)
	         PrintCLI(_("* missing sector   : %lld%s\n"), first,ecc_msg);
	    else PrintCLI(_("* missing sectors  : %lld - %lld%s\n"), first, last, ecc_msg);
	    first_missing = -1;
	    g_free(ecc_msg);
	 }
      }

      /* If the image sector is from the data portion and it was readable, 
	 test its CRC sum */

      if(   !current_missing
	 && (   (lay->target == ECC_IMAGE && s < lay->firstCrcPos)
	     || (lay->target == ECC_FILE && s < lay->dataSectors)))
      {  guint32 crc = Crc32(buf, 2048);

	 if(GetBit(vc->crcBuf->valid,crc_idx)
	    && crc != vc->crcBuf->crcbuf[crc_idx])
	 {  PrintCLI(_("* CRC error, sector: %lld\n"), s);
	    data_crc_errors++;
	    new_crc_errors++;
	    defective = TRUE;
	    exitCode = EXIT_CODE_CHECKSUM_ERROR;
	 }
      }
      crc_idx++;

      if(!defective)
	SetBit(vc->map, s);

      if(Closure->guiMode) 
      {   /* data part / spiral animation */
	  percent = (VERIFY_IMAGE_SEGMENTS*(s+1))/virtual_expected;

	  /* percentage is reset / output differently for ecc file part */
	  if(lay->target == ECC_FILE && s >= lay->dataSectors) 
	    percent = (100*(s+1-lay->dataSectors)/(virtual_expected-lay->dataSectors));
      }  
      else  percent = (100*(s+1))/virtual_expected;

      if(last_percent != percent) /* Update sector results */
      {  PrintProgress(_("- testing sectors  : %3d%%") ,percent);
	 if(Closure->guiMode)
	 {  if(lay->target == ECC_IMAGE)
	    {  add_verify_values(self, percent, new_missing, new_crc_errors); 
	    }
	    else /* do not include ecc file sectors in the spiral! */
	    {  if(s<lay->dataSectors)
	       {  int image_percent = (VERIFY_IMAGE_SEGMENTS*(s+1))/lay->dataSectors;
	          
	          add_verify_values(self, image_percent, new_missing, new_crc_errors); 
	       }
	       else
	       {  SetLabelText(GTK_LABEL(wl->cmpEccSyndromes),"%d%% tested",percent);
	       }
	    }

	    if(data_missing || data_crc_errors)
	      SetLabelText(GTK_LABEL(wl->cmpDataSection), 
			   _("<span %s>%lld sectors missing; %lld CRC errors</span>"),
			   Closure->redMarkup, data_missing, data_crc_errors);
	    if(crc_missing || csc->signatureErrors)
	      SetLabelText(GTK_LABEL(wl->cmpCrcSection), 
			   _("<span %s>%lld sectors missing; %lld signature errors</span>"),
			   Closure->redMarkup, crc_missing, csc->signatureErrors);
	    if(ecc_missing)
	      SetLabelText(GTK_LABEL(wl->cmpEccSection), 
			   _("<span %s>%lld sectors missing</span>"),
			   Closure->redMarkup, ecc_missing);
	 }
	 last_percent = percent;
	 new_missing = new_crc_errors = 0;
      }
      
      /* If we have processed the image and are about to switch over
	 to the ecc file, do some bookkeeping. */

      if(lay->target == ECC_FILE && s == lay->dataSectors-1)
      {	 
	if(Closure->guiMode)
	{  /* flush/complete spiral */
	   add_verify_values(self, VERIFY_IMAGE_SEGMENTS, new_missing, new_crc_errors); 

	   SetLabelText(GTK_LABEL(wl->cmpHeadline), "<big>%s</big>\n<i>%s</i>",
			_("Checking the image and error correction files."),
			_("- Checking ecc file -"));

	   SetLabelText(GTK_LABEL(wl->cmpEccSynLabel), _("Error correction file:"));
	   last_percent = 0; /* restart counting for ecc file */
	}
      }
   }

   /* Complete damage summary */

   if(Closure->guiMode)
   {  if(data_missing || data_crc_errors)
        SetLabelText(GTK_LABEL(wl->cmpDataSection), 
		     _("<span %s>%lld sectors missing; %lld CRC errors</span>"),
		     Closure->redMarkup, data_missing, data_crc_errors);
      if(crc_missing || csc->signatureErrors)
        SetLabelText(GTK_LABEL(wl->cmpCrcSection), 
		     _("<span %s>%lld sectors missing; %lld signature errors</span>"),
		     Closure->redMarkup, crc_missing, csc->signatureErrors);
      if(ecc_missing)
	SetLabelText(GTK_LABEL(wl->cmpEccSection), 
		     _("<span %s>%lld sectors missing</span>"),
		     Closure->redMarkup, ecc_missing);
   }

   /* The image md5sum is only useful if all blocks have been successfully read. */

   MD5Final(medium_sum, &image_md5);
   AsciiDigest(data_digest, medium_sum);

   /* Do a resume of our findings */ 

   if(!total_missing && !data_crc_errors && !csc->signatureErrors)
      PrintLog(_("- good image/file  : all sectors present\n"
		 "- data md5sum      : %s\n"),data_digest);
   else
   {  if(!data_crc_errors && !csc->signatureErrors)
         PrintLog(_("* BAD image/file   : %lld sectors missing\n"), total_missing);
      if(!total_missing)
	 PrintLog(_("* suspicious image : all sectors present, but %lld CRC errors\n"), 
		  data_crc_errors);
      if(total_missing && data_crc_errors)
	 PrintLog(_("* BAD image        : %lld sectors missing, %lld CRC errors\n"), 
		  total_missing, data_crc_errors);

      PrintLog(_("  ... data section   : %lld sectors missing; %lld CRC errors\n"), 
	       data_missing, data_crc_errors);
      if(!total_missing && !data_crc_errors && !csc->signatureErrors)
	PrintLog(_("  ... data md5sum    : %s\n"), data_digest); 

      if(csc->signatureErrors)
	 PrintLog(_("  ... crc section    : %lld sectors missing; %lld signature errors\n"), 
		  crc_missing, csc->signatureErrors);
      else
	 PrintLog(_("  ... crc section    : %lld sectors missing\n"), crc_missing);

      PrintLog(_("  ... ecc section    : %lld sectors missing\n"), ecc_missing);
   }

   if(Closure->guiMode)
   {  if(!data_missing && !data_crc_errors) 
                        SetLabelText(GTK_LABEL(wl->cmpDataSection), _("complete"));
      if(!crc_missing && !csc->signatureErrors)  
	                SetLabelText(GTK_LABEL(wl->cmpCrcSection), _("complete"));
      if(!ecc_missing)  SetLabelText(GTK_LABEL(wl->cmpEccSection), _("complete"));
     
      SetLabelText(GTK_LABEL(wl->cmpImageMd5Sum), "%s", data_missing ? "-" : data_digest);
   }

   /*** Test error syndromes */

   if(Closure->guiMode)
   {  SetLabelText(GTK_LABEL(wl->cmpEccSynLabel), _("Ecc block test:"));
      SetLabelText(GTK_LABEL(wl->cmpEccSyndromes), "");
   }
   if(0&&total_missing + data_crc_errors != 0)
   { if(Closure->guiMode) 
        SetLabelText(GTK_LABEL(wl->cmpEccSyndromes),
		     _("<span %s>Skipped; not useful on known defective image</span>"),
		     Closure->redMarkup);

     PrintLog(_("* Ecc block test   : skipped; not useful on defective image\n"));
   }
   else syn_error = check_syndromes(vc);

   /*** Print image advice */

   if(Closure->guiMode)
   {
      if(img_advice) 
      {  SetLabelText(GTK_LABEL(wl->cmpImageResult), img_advice);
         g_free(img_advice);
      }
      else 
      {  if(!total_missing && !data_crc_errors && !syn_error)
	    SetLabelText(GTK_LABEL(wl->cmpImageErasure),  /* avoid two blank lines */
			 _("<span %s>Good image.</span>"),
			 Closure->greenMarkup);
	 else
           SetLabelText(GTK_LABEL(wl->cmpImageResult),
			_("<span %s>Damaged image.</span>"),
			Closure->redMarkup);
      }
   }

   /*** Print final results */

   try_it = prognosis(vc, total_missing+data_crc_errors, lay->totalSectors); 

   if(Closure->guiMode)
   {  if(total_missing || data_crc_errors)
      {  if(try_it) SetLabelText(GTK_LABEL(wl->cmpImageResult),
				 _("<span %s>Full data recovery is likely.</span>"),
				 Closure->greenMarkup);
         else       SetLabelText(GTK_LABEL(wl->cmpImageResult),
				 _("<span %s>Full data recovery is NOT possible.</span>"),
				 Closure->redMarkup);
      }
   }

   /*** Close and clean up */

terminate:
   cleanup((gpointer)vc);
}
