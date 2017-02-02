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

/***
 *** Forward declarations
 ***/

static void redraw_curve(RS03Widgets*);
static void update_geometry(RS03Widgets*);

/***
 *** Encoding window
 ***/

void ResetRS03EncWindow(Method *method)
{  RS03Widgets *wl = (RS03Widgets*)method->widgetList;

   SetProgress(wl->encPBar1, 0, 100);
   SetProgress(wl->encPBar2, 0, 100);

   gtk_widget_hide(wl->encLabel2);
   gtk_widget_hide(wl->encPBar2);

   gtk_widget_hide(wl->encLabel3);
   gtk_widget_hide(wl->encLabel4);
   gtk_widget_hide(wl->encLabel5);
   gtk_widget_hide(wl->encThreads);
   gtk_widget_hide(wl->encPerformance);
   gtk_widget_hide(wl->encBottleneck);

   gtk_label_set_text(GTK_LABEL(wl->encFootline), "");
   gtk_label_set_text(GTK_LABEL(wl->encFootline2), "");
}

void CreateRS03EncWindow(Method *method, GtkWidget *parent)
{  GtkWidget *wid,*table,*pbar,*sep;
   RS03Widgets *wl;

   if(!method->widgetList)
   {  wl = g_malloc0(sizeof(RS03Widgets));
      method->widgetList = wl;
   }
   else wl = method->widgetList;

   wl->encHeadline = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(wl->encHeadline), 0.0, 0.0); 
   gtk_misc_set_padding(GTK_MISC(wl->encHeadline), 5, 0);
   gtk_box_pack_start(GTK_BOX(parent), wl->encHeadline, FALSE, FALSE, 3);

   sep = gtk_hseparator_new();
   gtk_box_pack_start(GTK_BOX(parent), sep, FALSE, FALSE, 0);

   sep = gtk_hseparator_new();
   gtk_box_pack_start(GTK_BOX(parent), sep, FALSE, FALSE, 0);

   table = gtk_table_new(2, 5, FALSE);
   gtk_box_pack_start(GTK_BOX(parent), table, FALSE, FALSE, 30);

   wl->encLabel1 = wid = gtk_label_new(NULL);
   gtk_label_set_markup(GTK_LABEL(wid),
			_utf("<b>1. Reserving space:</b>"));
   gtk_misc_set_alignment(GTK_MISC(wid), 0.0, 0.0);
   gtk_table_attach(GTK_TABLE(table), wid, 0, 1, 0, 1, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 10, 20);

   pbar = wl->encPBar1 = gtk_progress_bar_new();
   gtk_table_attach(GTK_TABLE(table), pbar, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND, 10, 20);

   wl->encLabel2 = wid = gtk_label_new(NULL);
   gtk_label_set_markup(GTK_LABEL(wid),
			_utf("<b>2. Creating error correction data:</b>"));
   gtk_misc_set_alignment(GTK_MISC(wid), 0.0, 0.0);
   gtk_table_attach(GTK_TABLE(table), wid, 0, 1, 1, 2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 10, 20);

   pbar = wl->encPBar2 = gtk_progress_bar_new();
   gtk_table_attach(GTK_TABLE(table), pbar, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_EXPAND, 10, 20);


   wl->encLabel3 = wid = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(wid), 1.0, 0.0); 
   gtk_label_set_markup(GTK_LABEL(wid),_utf("<b>Encoder info:</b>"));
   gtk_table_attach(GTK_TABLE(table), wid, 0, 1, 2, 3, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 10, 5);
   
   wl->encThreads = wid = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(wid), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table), wid, 1, 2, 2, 3, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 10,5);

   wl->encLabel4 = wid = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(wid), 1.0, 0.0); 
   gtk_label_set_markup(GTK_LABEL(wid),_utf("<b>Performance:</b>"));
   gtk_table_attach(GTK_TABLE(table), wid, 0, 1, 4, 5, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 10, 5);

   wl->encPerformance = wid = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(wid), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table), wid, 1, 2, 4, 5, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 10, 5);

   wl->encLabel5 = wid = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(wid), 1.0, 0.0); 
   gtk_label_set_markup(GTK_LABEL(wid),_utf("<b>State:</b>"));
   gtk_table_attach(GTK_TABLE(table), wid, 0, 1, 5, 6, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 10, 5);

   wl->encBottleneck = wid = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(wid), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table), wid, 1, 2, 5, 6, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 10, 5);

   wl->encFootline = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(wl->encFootline), 0.0, 0.5); 
   gtk_misc_set_padding(GTK_MISC(wl->encFootline), 20, 0);
   gtk_box_pack_start(GTK_BOX(parent), wl->encFootline, FALSE, FALSE, 3);

   wl->encFootline2 = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(wl->encFootline2), 0.0, 0.5); 
   gtk_misc_set_padding(GTK_MISC(wl->encFootline2), 20, 0);
   gtk_box_pack_start(GTK_BOX(parent), wl->encFootline2, FALSE, FALSE, 3);
}

/***
 *** Fix window
 ***/

/*
 * Set the media size and ecc capacity
 */

static gboolean set_max_idle_func(gpointer data)
{  RS03Widgets *wl = (RS03Widgets*)data;

   redraw_curve(wl);

   return FALSE;
}

void RS03SetFixMaxValues(RS03Widgets *wl, int data_bytes, int ecc_bytes, gint64 sectors)
{
   wl->dataBytes = data_bytes;
   wl->eccBytes  = ecc_bytes;
   wl->nSectors  = sectors;
   wl->fixCurve->maxX = 100;
   wl->fixCurve->maxY = ecc_bytes - (ecc_bytes % 5) + 5;

   g_idle_add(set_max_idle_func, wl);
}

/*
 * Update the corrected / uncorrected numbers
 */

static gboolean results_idle_func(gpointer data)
{  RS03Widgets *wl = (RS03Widgets*)data;

   SetLabelText(GTK_LABEL(wl->fixCorrected), _("Repaired: %lld"), wl->corrected); 
   SetLabelText(GTK_LABEL(wl->fixUncorrected), _("Unrepairable: <span %s>%lld</span>"),Closure->redMarkup, wl->uncorrected); 
   SetLabelText(GTK_LABEL(wl->fixProgress), _("Progress: %3d.%1d%%"), wl->percent/10, wl->percent%10);

   return FALSE;
}

void RS03UpdateFixResults(RS03Widgets *wl, gint64 corrected, gint64 uncorrected)
{
   wl->corrected = corrected;
   wl->uncorrected = uncorrected;

   g_idle_add(results_idle_func, wl);
}

/*
 * Update the error curve 
 */

static gboolean curve_idle_func(gpointer data)
{  RS03Widgets *wl = (RS03Widgets*)data;
   gint x0 = CurveX(wl->fixCurve, (double)wl->lastPercent);
   gint x1 = CurveX(wl->fixCurve, (double)wl->percent);
   gint y = CurveY(wl->fixCurve, wl->fixCurve->ivalue[wl->percent]);
   gint i;

   /*** Mark unused ecc values */

   for(i=wl->lastPercent+1; i<wl->percent; i++)
      wl->fixCurve->ivalue[i] = wl->fixCurve->ivalue[wl->percent];

   /*** Resize the Y axes if error values exceeds current maximum */

   if(wl->fixCurve->ivalue[wl->percent] > wl->fixCurve->maxY)
   {  wl->fixCurve->maxY = wl->fixCurve->ivalue[wl->percent];
      wl->fixCurve->maxY = wl->fixCurve->maxY - (wl->fixCurve->maxY % 5) + 5;

      update_geometry(wl);
      gdk_window_clear(wl->fixCurve->widget->window);
      redraw_curve(wl);
      wl->lastPercent = wl->percent;

      return FALSE;
   }

   /*** Draw the error value */

   if(wl->fixCurve->ivalue[wl->percent] > 0)
   {  gdk_gc_set_rgb_fg_color(Closure->drawGC, Closure->barColor);
      gdk_draw_rectangle(wl->fixCurve->widget->window,
			 Closure->drawGC, TRUE,
			 x0, y, x0==x1 ? 1 : x1-x0, wl->fixCurve->bottomY-y);
   }
   wl->lastPercent = wl->percent;

   /* Redraw the ecc capacity threshold line */

   y = CurveY(wl->fixCurve, wl->eccBytes);  
   gdk_gc_set_rgb_fg_color(Closure->drawGC, Closure->greenSector);
   gdk_draw_line(wl->fixCurve->widget->window,
		 Closure->drawGC,
		 wl->fixCurve->leftX-6, y, wl->fixCurve->rightX+6, y);
   return FALSE;
}

/* 
 * Add one new data point 
 */

void RS03AddFixValues(RS03Widgets *wl, int percent, int ecc_max)
{
   if(percent < 0 || percent > 1000)
     return;

   wl->fixCurve->ivalue[percent] = ecc_max;
   wl->percent = percent;
   g_idle_add(curve_idle_func, wl);
}
  
/*
 * Redraw the whole curve
 */

/* Calculate the geometry of the curve and spiral */

static void update_geometry(RS03Widgets *wl)
{  
   /* Curve geometry */ 

   UpdateCurveGeometry(wl->fixCurve, "999", 20);

   /* Label positions in the foot line */

   gtk_box_set_child_packing(GTK_BOX(wl->fixFootlineBox), wl->fixCorrected,
			     TRUE, TRUE, wl->fixCurve->leftX, GTK_PACK_START);
   gtk_box_set_child_packing(GTK_BOX(wl->fixFootlineBox), wl->fixUncorrected, 
			     TRUE, TRUE, wl->fixCurve->leftX, GTK_PACK_START);
}

static void redraw_curve(RS03Widgets *wl)
{  int y;

   /* Redraw the curve */

   RedrawAxes(wl->fixCurve);
   RedrawCurve(wl->fixCurve, wl->percent);

   /* Ecc capacity threshold line */

   y = CurveY(wl->fixCurve, wl->eccBytes);  
   gdk_gc_set_rgb_fg_color(Closure->drawGC, Closure->greenSector);
   gdk_draw_line(wl->fixCurve->widget->window,
		 Closure->drawGC,
		 wl->fixCurve->leftX-6, y, wl->fixCurve->rightX+6, y);
}

/*
 * Expose callback
 */

static gboolean expose_cb(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{  RS03Widgets *wl = (RS03Widgets*)data; 

   if(event->count) /* Exposure compression */
     return TRUE;

   update_geometry(wl);
   redraw_curve(wl);

   return TRUE;
}

void ResetRS03FixWindow(Method *method)
{  RS03Widgets *wl = (RS03Widgets*)method->widgetList;

   gtk_notebook_set_current_page(GTK_NOTEBOOK(wl->fixNotebook), 0);

   ZeroCurve(wl->fixCurve);
   RS03UpdateFixResults(wl, 0, 0);

   if(wl->fixCurve && wl->fixCurve->widget)
   {  gdk_window_clear(wl->fixCurve->widget->window);
      redraw_curve(wl);
   }

   wl->percent = 0;
   wl->lastPercent = 0;
}

/*
 * Create the Fix window contents
 */


void CreateRS03FixWindow(Method *method, GtkWidget *parent)
{  RS03Widgets *wl;
   GtkWidget *sep,*ignore,*d_area,*notebook,*hbox;

   if(!method->widgetList)
   {  wl = g_malloc0(sizeof(RS03Widgets));
      method->widgetList = wl;
   }
   else wl = method->widgetList;

   wl->fixHeadline = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(wl->fixHeadline), 0.0, 0.0); 
   gtk_misc_set_padding(GTK_MISC(wl->fixHeadline), 5, 0);
   gtk_box_pack_start(GTK_BOX(parent), wl->fixHeadline, FALSE, FALSE, 3);

   sep = gtk_hseparator_new();
   gtk_box_pack_start(GTK_BOX(parent), sep, FALSE, FALSE, 0);

   sep = gtk_hseparator_new();
   gtk_box_pack_start(GTK_BOX(parent), sep, FALSE, FALSE, 0);

   d_area = wl->fixDrawingArea = gtk_drawing_area_new();
   gtk_box_pack_start(GTK_BOX(parent), d_area, TRUE, TRUE, 0);
   g_signal_connect(G_OBJECT (d_area), "expose_event", G_CALLBACK(expose_cb), (gpointer)wl);
   
   notebook = wl->fixNotebook = gtk_notebook_new();
   gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
   gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);
   gtk_box_pack_end(GTK_BOX(parent), notebook, FALSE, FALSE, 0);

   hbox = wl->fixFootlineBox = gtk_hbox_new(TRUE, 0);

   wl->fixCorrected = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(wl->fixCorrected), 0.0, 0.0); 
   gtk_box_pack_start(GTK_BOX(hbox), wl->fixCorrected, TRUE, TRUE, 0);

   wl->fixProgress = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(wl->fixProgress), 0.5, 0.0); 
   gtk_box_pack_start(GTK_BOX(hbox), wl->fixProgress, TRUE, TRUE, 0);

   wl->fixUncorrected = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(wl->fixUncorrected), 1.0, 0.0); 
   gtk_box_pack_start(GTK_BOX(hbox), wl->fixUncorrected, TRUE, TRUE, 0);

   ignore = gtk_label_new("progress_tab");
   gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hbox, ignore);

   wl->fixFootline = gtk_label_new("Footline");
   gtk_misc_set_alignment(GTK_MISC(wl->fixFootline), 0.0, 0.5); 
   gtk_misc_set_padding(GTK_MISC(wl->fixFootline), 5, 0);
   ignore = gtk_label_new("footer_tab");
   gtk_notebook_append_page(GTK_NOTEBOOK(notebook), wl->fixFootline, ignore);

   wl->fixCurve  = CreateCurve(d_area, _("Errors/Ecc block"), "%d", 1000, CURVE_PERCENT);
   wl->fixCurve->enable = DRAW_ICURVE;
}

