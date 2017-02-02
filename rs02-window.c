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

extern gint64 CurrentMediumSize(int);  /* from scsi-layer.h */

/***
 *** Forward declarations
 ***/

static void redraw_curve(RS02Widgets*);
static void update_geometry(RS02Widgets*);

/***
 *** Encoding window
 ***/

void ResetRS02EncWindow(Method *method)
{  RS02Widgets *wl = (RS02Widgets*)method->widgetList;

   SetProgress(wl->encPBar1, 0, 100);
   SetProgress(wl->encPBar2, 0, 100);

   gtk_widget_hide(wl->encLabel2);
   gtk_widget_hide(wl->encPBar2);

   gtk_label_set_text(GTK_LABEL(wl->encFootline), "");
   gtk_label_set_text(GTK_LABEL(wl->encFootline2), "");
}

void CreateRS02EncWindow(Method *method, GtkWidget *parent)
{  GtkWidget *wid,*table,*pbar,*sep;
   RS02Widgets *wl;

   if(!method->widgetList)
   {  wl = g_malloc0(sizeof(RS02Widgets));
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

   table = gtk_table_new(2, 2, FALSE);
   gtk_box_pack_start(GTK_BOX(parent), table, FALSE, FALSE, 30);

   wl->encLabel1 = wid = gtk_label_new(NULL);
   gtk_label_set_markup(GTK_LABEL(wid),
			_utf("<b>1. Preparing image:</b>"));
   gtk_misc_set_alignment(GTK_MISC(wid), 0.0, 0.0);
   gtk_table_attach(GTK_TABLE(table), wid, 0, 1, 0, 1, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 20, 20);

   pbar = wl->encPBar1 = gtk_progress_bar_new();
   gtk_table_attach(GTK_TABLE(table), pbar, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND, 20, 20);

   wl->encLabel2 = wid = gtk_label_new(NULL);
   gtk_label_set_markup(GTK_LABEL(wid),
			_utf("<b>2. Creating error correction data:</b>"));
   gtk_misc_set_alignment(GTK_MISC(wid), 0.0, 0.0);
   gtk_table_attach(GTK_TABLE(table), wid, 0, 1, 1, 2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 20, 20);

   pbar = wl->encPBar2 = gtk_progress_bar_new();
   gtk_table_attach(GTK_TABLE(table), pbar, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_EXPAND, 20, 20);

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
{  RS02Widgets *wl = (RS02Widgets*)data;

   redraw_curve(wl);

   return FALSE;
}

void RS02SetFixMaxValues(RS02Widgets *wl, int data_bytes, int ecc_bytes, gint64 sectors)
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
{  RS02Widgets *wl = (RS02Widgets*)data;

   SetLabelText(GTK_LABEL(wl->fixCorrected), _("Repaired: %lld"), wl->corrected); 
   SetLabelText(GTK_LABEL(wl->fixUncorrected), _("Unrepairable: <span %s>%lld</span>"),Closure->redMarkup, wl->uncorrected); 
   SetLabelText(GTK_LABEL(wl->fixProgress), _("Progress: %3d.%1d%%"), wl->percent/10, wl->percent%10);

   return FALSE;
}

void RS02UpdateFixResults(RS02Widgets *wl, gint64 corrected, gint64 uncorrected)
{
   wl->corrected = corrected;
   wl->uncorrected = uncorrected;

   g_idle_add(results_idle_func, wl);
}

/*
 * Update the error curve 
 */

static gboolean curve_idle_func(gpointer data)
{  RS02Widgets *wl = (RS02Widgets*)data;
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

void RS02AddFixValues(RS02Widgets *wl, int percent, int ecc_max)
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

static void update_geometry(RS02Widgets *wl)
{  
   /* Curve geometry */ 

   UpdateCurveGeometry(wl->fixCurve, "999", 20);

   /* Label positions in the foot line */

   gtk_box_set_child_packing(GTK_BOX(wl->fixFootlineBox), wl->fixCorrected,
			     TRUE, TRUE, wl->fixCurve->leftX, GTK_PACK_START);
   gtk_box_set_child_packing(GTK_BOX(wl->fixFootlineBox), wl->fixUncorrected, 
			     TRUE, TRUE, wl->fixCurve->leftX, GTK_PACK_START);
}

static void redraw_curve(RS02Widgets *wl)
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
{  RS02Widgets *wl = (RS02Widgets*)data; 

   if(event->count) /* Exposure compression */
     return TRUE;

   update_geometry(wl);
   redraw_curve(wl);

   return TRUE;
}

void ResetRS02FixWindow(Method *method)
{  RS02Widgets *wl = (RS02Widgets*)method->widgetList;

   gtk_notebook_set_current_page(GTK_NOTEBOOK(wl->fixNotebook), 0);

   ZeroCurve(wl->fixCurve);
   RS02UpdateFixResults(wl, 0, 0);

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


void CreateRS02FixWindow(Method *method, GtkWidget *parent)
{  RS02Widgets *wl;
   GtkWidget *sep,*ignore,*d_area,*notebook,*hbox;

   if(!method->widgetList)
   {  wl = g_malloc0(sizeof(RS02Widgets));
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

/***
 *** Create the preferences page for setting redundancy etc.
 ***/

enum 
{  PREF_NROOTS = 0,
   PREF_CACHE = 1,
   PREF_ECC_SIZE = 2
};

#ifdef HAVE_32BIT
static int cache_size[] = { 8, 16, 32, 64, 96, 128, 192, 256, 384, 512, 768, 
			    1024, 1536 };
#else
static int cache_size[] = { 8, 16, 32, 64, 96, 128, 192, 256, 384, 512, 768, 
			    1024, 1536, 2048, 2560, 3072, 4096, 5120, 6144, 7168, 8192 };
#endif

static gchar* format_cb(GtkScale *scale, gdouble value, gpointer data)
{  int nroots = value;
   int ndata  = GF_FIELDMAX - nroots;
   char *label;

   if(GPOINTER_TO_INT(data) == PREF_CACHE)
     label = g_strdup(" ");
   else
     label = g_strdup_printf(_utf("%4.1f%% redundancy (%d roots)"),
			    ((double)nroots*100.0)/(double)ndata,
			    nroots);

   FORGET(label);  /* will be g_free()ed by the scale */
   return label;
}

static void cache_cb(GtkWidget *widget, gpointer data)
{  RS02Widgets *wl = (RS02Widgets*)data;
   LabelWithOnlineHelp *lwoh = wl->cacheLwoh;
   int value;
   char *text, *utf;

   value = gtk_range_get_value(GTK_RANGE(widget));
   Closure->cacheMiB = cache_size[value];
	
   text = g_strdup_printf(_("%d MiB of file cache"), Closure->cacheMiB);
   utf  = g_locale_to_utf8(text, -1, NULL, NULL, NULL);
   gtk_label_set_markup(GTK_LABEL(lwoh->normalLabel), utf);
   gtk_label_set_markup(GTK_LABEL(lwoh->linkLabel), utf);
   SetOnlineHelpLinkText(lwoh, text);
   UpdateMethodPreferences();
   g_free(text);
   g_free(utf);
}

static void toggle_cb(GtkWidget *widget, gpointer data)
{  Method *method = (Method*)data;
   RS02Widgets *wl = (RS02Widgets*)method->widgetList;
   int state  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

   if(state == TRUE)
   {  
      if(widget == wl->radio1A) /* automatic */
      {  
	 gtk_widget_set_sensitive(wl->cdEntryA, TRUE);
	 gtk_widget_set_sensitive(wl->dvdEntry1A, TRUE);
	 gtk_widget_set_sensitive(wl->dvdEntry2A, TRUE);
	 gtk_widget_set_sensitive(wl->bdEntry1A, TRUE);
	 gtk_widget_set_sensitive(wl->bdEntry2A, TRUE);
	 gtk_widget_set_sensitive(wl->cdButtonA, TRUE);
	 gtk_widget_set_sensitive(wl->dvdButton1A, TRUE);
	 gtk_widget_set_sensitive(wl->dvdButton2A, TRUE);
	 gtk_widget_set_sensitive(wl->bdButton1A, TRUE);
	 gtk_widget_set_sensitive(wl->bdButton2A, TRUE);
	 gtk_widget_set_sensitive(wl->cdUndoButtonA, TRUE);
	 gtk_widget_set_sensitive(wl->dvdUndoButton1A, TRUE);
	 gtk_widget_set_sensitive(wl->dvdUndoButton2A, TRUE);
	 gtk_widget_set_sensitive(wl->bdUndoButton1A, TRUE);
	 gtk_widget_set_sensitive(wl->bdUndoButton2A, TRUE);

	 gtk_widget_set_sensitive(wl->otherEntryA, FALSE);

	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio1B), TRUE);

         Closure->mediumSize = 0;
      }

      if(widget == wl->radio1B) /* automatic */
      {  
	 gtk_widget_set_sensitive(wl->cdEntryB, TRUE);
	 gtk_widget_set_sensitive(wl->dvdEntry1B, TRUE);
	 gtk_widget_set_sensitive(wl->dvdEntry2B, TRUE);
	 gtk_widget_set_sensitive(wl->bdEntry1B, TRUE);
	 gtk_widget_set_sensitive(wl->bdEntry2B, TRUE);
	 gtk_widget_set_sensitive(wl->cdButtonB, TRUE);
	 gtk_widget_set_sensitive(wl->dvdButton1B, TRUE);
	 gtk_widget_set_sensitive(wl->dvdButton2B, TRUE);
	 gtk_widget_set_sensitive(wl->bdButton1B, TRUE);
	 gtk_widget_set_sensitive(wl->bdButton2B, TRUE);
	 gtk_widget_set_sensitive(wl->cdUndoButtonB, TRUE);
	 gtk_widget_set_sensitive(wl->dvdUndoButton1B, TRUE);
	 gtk_widget_set_sensitive(wl->dvdUndoButton2B, TRUE);
	 gtk_widget_set_sensitive(wl->bdUndoButton1B, TRUE);
	 gtk_widget_set_sensitive(wl->bdUndoButton2B, TRUE);

	 gtk_widget_set_sensitive(wl->otherEntryB, FALSE);

	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio1A), TRUE);

         Closure->mediumSize = 0;
      }

      if(widget == wl->radio2A) /* user specified value */
      {  const char *value = gtk_entry_get_text(GTK_ENTRY(wl->otherEntryA));

	 gtk_widget_set_sensitive(wl->cdEntryA, FALSE);
	 gtk_widget_set_sensitive(wl->dvdEntry1A, FALSE);
	 gtk_widget_set_sensitive(wl->dvdEntry2A, FALSE);
	 gtk_widget_set_sensitive(wl->bdEntry1A, FALSE);
	 gtk_widget_set_sensitive(wl->bdEntry2A, FALSE);
	 gtk_widget_set_sensitive(wl->cdButtonA, FALSE);
	 gtk_widget_set_sensitive(wl->dvdButton1A, FALSE);
	 gtk_widget_set_sensitive(wl->dvdButton2A, FALSE);
	 gtk_widget_set_sensitive(wl->bdButton1A, FALSE);
	 gtk_widget_set_sensitive(wl->bdButton2A, FALSE);
	 gtk_widget_set_sensitive(wl->cdUndoButtonA, FALSE);
	 gtk_widget_set_sensitive(wl->dvdUndoButton1A, FALSE);
	 gtk_widget_set_sensitive(wl->dvdUndoButton2A, FALSE);
	 gtk_widget_set_sensitive(wl->bdUndoButton1A, FALSE);
	 gtk_widget_set_sensitive(wl->bdUndoButton2A, FALSE);

	 gtk_widget_set_sensitive(wl->otherEntryA, TRUE);

	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio2B), TRUE);

	 Closure->mediumSize = atoll(value);
      }

      if(widget == wl->radio2B) /* user specified value */
      {  const char *value = gtk_entry_get_text(GTK_ENTRY(wl->otherEntryB));

	 gtk_widget_set_sensitive(wl->cdEntryB, FALSE);
	 gtk_widget_set_sensitive(wl->dvdEntry1B, FALSE);
	 gtk_widget_set_sensitive(wl->dvdEntry2B, FALSE);
	 gtk_widget_set_sensitive(wl->bdEntry1B, FALSE);
	 gtk_widget_set_sensitive(wl->bdEntry2B, FALSE);
	 gtk_widget_set_sensitive(wl->cdButtonB, FALSE);
	 gtk_widget_set_sensitive(wl->dvdButton1B, FALSE);
	 gtk_widget_set_sensitive(wl->dvdButton2B, FALSE);
	 gtk_widget_set_sensitive(wl->bdButton1B, FALSE);
	 gtk_widget_set_sensitive(wl->bdButton2B, FALSE);
	 gtk_widget_set_sensitive(wl->cdUndoButtonB, FALSE);
	 gtk_widget_set_sensitive(wl->dvdUndoButton1B, FALSE);
	 gtk_widget_set_sensitive(wl->dvdUndoButton2B, FALSE);
	 gtk_widget_set_sensitive(wl->bdUndoButton1B, FALSE);
	 gtk_widget_set_sensitive(wl->bdUndoButton2B, FALSE);

	 gtk_widget_set_sensitive(wl->otherEntryB, TRUE);

	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio2A), TRUE);

	 Closure->mediumSize = atoll(value);
      }
   }
}

/*
 * Query size from current medium
 */

static void query_cb(GtkWidget *widget, gpointer data)
{  RS02Widgets *wl = (RS02Widgets*)data;
   char value[40];
   gint64 size;
 
   size = CurrentMediumSize(TRUE);
   g_snprintf(value, 40, "%lld", (long long int)size);

   if(widget == wl->cdButtonA || widget == wl->cdButtonB)
   {  gtk_entry_set_text(GTK_ENTRY(wl->cdEntryA), value);
      gtk_entry_set_text(GTK_ENTRY(wl->cdEntryB), value);
   }

   if(widget == wl->cdUndoButtonA || widget == wl->cdUndoButtonB)
   {  g_snprintf(value, 40, "%lld", (long long int)Closure->savedCDSize);
      gtk_entry_set_text(GTK_ENTRY(wl->cdEntryA), value);
      gtk_entry_set_text(GTK_ENTRY(wl->cdEntryB), value);
   }

   if(widget == wl->dvdButton1A || widget == wl->dvdButton1B)
   {  gtk_entry_set_text(GTK_ENTRY(wl->dvdEntry1A), value);
      gtk_entry_set_text(GTK_ENTRY(wl->dvdEntry1B), value);
   }

   if(widget == wl->dvdUndoButton1A || widget == wl->dvdUndoButton1B)
   {  g_snprintf(value, 40, "%lld", (long long int)Closure->savedDVDSize1);
      gtk_entry_set_text(GTK_ENTRY(wl->dvdEntry1A), value);
      gtk_entry_set_text(GTK_ENTRY(wl->dvdEntry1B), value);
   }

   if(widget == wl->dvdButton2A || widget == wl->dvdButton2B)
   {  gtk_entry_set_text(GTK_ENTRY(wl->dvdEntry2A), value);
      gtk_entry_set_text(GTK_ENTRY(wl->dvdEntry2B), value);
   }

   if(widget == wl->dvdUndoButton2A || widget == wl->dvdUndoButton2B )
   {  g_snprintf(value, 40, "%lld", (long long int)Closure->savedDVDSize2);
      gtk_entry_set_text(GTK_ENTRY(wl->dvdEntry2A), value);
      gtk_entry_set_text(GTK_ENTRY(wl->dvdEntry2B), value);
   }

   if(widget == wl->bdButton1A || widget == wl->bdButton1B)
   {  gtk_entry_set_text(GTK_ENTRY(wl->bdEntry1A), value);
      gtk_entry_set_text(GTK_ENTRY(wl->bdEntry1B), value);
   }

   if(widget == wl->bdUndoButton1A || widget == wl->bdUndoButton1B)
   {  g_snprintf(value, 40, "%lld", (long long int)Closure->savedBDSize1);
      gtk_entry_set_text(GTK_ENTRY(wl->bdEntry1A), value);
      gtk_entry_set_text(GTK_ENTRY(wl->bdEntry1B), value);
   }

   if(widget == wl->bdButton2A || widget == wl->bdButton2B)
   {  gtk_entry_set_text(GTK_ENTRY(wl->bdEntry2A), value);
      gtk_entry_set_text(GTK_ENTRY(wl->bdEntry2B), value);
   }

   if(widget == wl->bdUndoButton2A || widget == wl->bdUndoButton2B )
   {  g_snprintf(value, 40, "%lld", (long long int)Closure->savedBDSize2);
      gtk_entry_set_text(GTK_ENTRY(wl->bdEntry2A), value);
      gtk_entry_set_text(GTK_ENTRY(wl->bdEntry2B), value);
   }
}

/*
 * Track changes of entry widgets
 */

static void entry_tracker_cb(GtkWidget *widget, gpointer data)
{  RS02Widgets *wl = (RS02Widgets*)data;

   if(widget == wl->cdEntryA)
   {  const char *text = gtk_entry_get_text(GTK_ENTRY(wl->cdEntryA));
      gtk_entry_set_text(GTK_ENTRY(wl->cdEntryB), text);
   }
   if(widget == wl->cdEntryB)
   {  const char *text = gtk_entry_get_text(GTK_ENTRY(wl->cdEntryB));
      gtk_entry_set_text(GTK_ENTRY(wl->cdEntryA), text);
   }

   if(widget == wl->dvdEntry1A)
   {  const char *text = gtk_entry_get_text(GTK_ENTRY(wl->dvdEntry1A));
      gtk_entry_set_text(GTK_ENTRY(wl->dvdEntry1B), text);
   }
   if(widget == wl->dvdEntry1B)
   {  const char *text = gtk_entry_get_text(GTK_ENTRY(wl->dvdEntry1B));
      gtk_entry_set_text(GTK_ENTRY(wl->dvdEntry1A), text);
   }

   if(widget == wl->dvdEntry2A)
   {  const char *text = gtk_entry_get_text(GTK_ENTRY(wl->dvdEntry2A));
      gtk_entry_set_text(GTK_ENTRY(wl->dvdEntry2B), text);
   }
   if(widget == wl->dvdEntry2B)
   {  const char *text = gtk_entry_get_text(GTK_ENTRY(wl->dvdEntry2B));
      gtk_entry_set_text(GTK_ENTRY(wl->dvdEntry2A), text);
   }

   if(widget == wl->bdEntry1A)
   {  const char *text = gtk_entry_get_text(GTK_ENTRY(wl->bdEntry1A));
      gtk_entry_set_text(GTK_ENTRY(wl->bdEntry1B), text);
   }
   if(widget == wl->bdEntry1B)
   {  const char *text = gtk_entry_get_text(GTK_ENTRY(wl->bdEntry1B));
      gtk_entry_set_text(GTK_ENTRY(wl->bdEntry1A), text);
   }

   if(widget == wl->bdEntry2A)
   {  const char *text = gtk_entry_get_text(GTK_ENTRY(wl->bdEntry2A));
      gtk_entry_set_text(GTK_ENTRY(wl->bdEntry2B), text);
   }
   if(widget == wl->bdEntry2B)
   {  const char *text = gtk_entry_get_text(GTK_ENTRY(wl->bdEntry2B));
      gtk_entry_set_text(GTK_ENTRY(wl->bdEntry2A), text);
   }

   if(widget == wl->otherEntryA)
   {  const char *text = gtk_entry_get_text(GTK_ENTRY(wl->otherEntryA));
      gtk_entry_set_text(GTK_ENTRY(wl->otherEntryB), text);
   }
   if(widget == wl->otherEntryB)
   {  const char *text = gtk_entry_get_text(GTK_ENTRY(wl->otherEntryB));
      gtk_entry_set_text(GTK_ENTRY(wl->otherEntryA), text);
   }
}

/* 
 * Some values are shared with RS01.
 * If they changed there, update our preferences page.
 */

void ResetRS02PrefsPage(Method *method)
{  RS02Widgets *wl = (RS02Widgets*)method->widgetList;
   int index;

   for(index = 0; index < sizeof(cache_size)/sizeof(int); index++)
     if(cache_size[index] > Closure->cacheMiB)
       break;
   
   if(wl->cacheScaleA)
     gtk_range_set_value(GTK_RANGE(wl->cacheScaleA), index > 0 ? index-1 : index);
   if(wl->cacheScaleB)
     gtk_range_set_value(GTK_RANGE(wl->cacheScaleB), index > 0 ? index-1 : index);
}

/*
 * Read values from our preferences page
 * to make sure that all changed values from text entries
 * are recognized.
 */

void ReadRS02Preferences(Method *method)
{  RS02Widgets *wl = (RS02Widgets*)method->widgetList;
   gint64 value;

   value = atoll(gtk_entry_get_text(GTK_ENTRY(wl->cdEntryA)));
   Closure->cdSize = value > 0 ? value : 0; 
   value = atoll(gtk_entry_get_text(GTK_ENTRY(wl->dvdEntry1A)));
   Closure->dvdSize1 = value > 0 ? value : 0; 
   value = atoll(gtk_entry_get_text(GTK_ENTRY(wl->dvdEntry2A)));
   Closure->dvdSize2 = value > 0 ? value : 0; 
   value = atoll(gtk_entry_get_text(GTK_ENTRY(wl->bdEntry1A)));
   Closure->bdSize1 = value > 0 ? value : 0; 
   value = atoll(gtk_entry_get_text(GTK_ENTRY(wl->bdEntry2A)));
   Closure->bdSize2 = value > 0 ? value : 0; 

   if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wl->radio1A)))   
   {  Closure->mediumSize = 0;
   }

   if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wl->radio2A)))   
   {  value = atoll(gtk_entry_get_text(GTK_ENTRY(wl->otherEntryA)));
   
      Closure->mediumSize = value > 0 ? value : 0; 
   }
}

/*
 * Create our preferences page
 */

void CreateRS02PrefsPage(Method *method, GtkWidget *parent)
{  RS02Widgets *wl = (RS02Widgets*)method->widgetList;
   GtkWidget *frame, *hbox, *vbox, *vbox2, *lab, *scale, *table;
   GtkWidget *radio, *icon; 
   LabelWithOnlineHelp *lwoh;
   unsigned int index;
   char *text,value[40];
   int i;

   /*** Redundancy selection */

   frame = gtk_frame_new(_utf("Maximum image size"));
   gtk_box_pack_start(GTK_BOX(parent), frame, FALSE, FALSE, 0);

   vbox = gtk_vbox_new(FALSE, 10);
   gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
   gtk_container_add(GTK_CONTAINER(frame), vbox);

   hbox = gtk_hbox_new(FALSE, 0);
   gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

   /* Drive capacity table header */

   lwoh = CreateLabelWithOnlineHelp(_("Using the smallest possible size from table"), _("Use smallest possible size from following table (in sectors):"));
   RegisterPreferencesHelpWindow(lwoh);

   vbox2 = gtk_vbox_new(FALSE, 0);
   gtk_box_pack_start(GTK_BOX(hbox), vbox2, FALSE, FALSE, 0);

   for(i=0; i<2; i++)
   {  
      radio = gtk_radio_button_new(NULL);
      g_signal_connect(G_OBJECT(radio), "toggled", G_CALLBACK(toggle_cb), method);

      if(!i)
      {  wl->radio1A = radio;
	 gtk_box_pack_start(GTK_BOX(vbox2), radio, FALSE, FALSE, 0);
      }
      else
      {  GtkWidget *hbox = gtk_hbox_new(FALSE, 4);
         wl->radio1B = radio;
	 gtk_box_pack_start(GTK_BOX(hbox), radio, FALSE, FALSE, 0);
	 gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);
	 AddHelpWidget(lwoh, hbox);
      }
   }

   AddHelpParagraph(lwoh, _("<b>Determine augmented image size from table</b>\n\n"
			    "Augmented images fill up unused medium space "
			    "with error correction information. Activate this option "
			    "if you want the augmented image to fit on the smallest "
			    "possible medium.\n\n"

			    "In order to pick a suitable medium the available media "
			    "capacities must be known. Default sizes for CD and "
			    "one/two layered DVD and BD are given in the table. You can edit "
			    "these sizes according to your needs."));

   table = gtk_table_new(5, 6, FALSE);
   gtk_box_pack_start(GTK_BOX(hbox), table, FALSE, FALSE, 0);

   gtk_misc_set_alignment(GTK_MISC(lwoh->linkLabel), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table), lwoh->linkBox, 0, 5, 0, 1, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 5);

   /* CD capacity table row */

   lwoh = CreateLabelWithOnlineHelp(_("CD-R / CD-RW:"), _("CD-R / CD-RW:"));
   RegisterPreferencesHelpWindow(lwoh);

     gtk_misc_set_alignment(GTK_MISC(lwoh->linkLabel), 0.0, 0.0); 
     gtk_table_attach(GTK_TABLE(table), lwoh->linkBox, 0, 1, 1, 2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 5);

     wl->cdEntryA = gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(wl->cdEntryA), 9);
     g_signal_connect(G_OBJECT(wl->cdEntryA), "activate", G_CALLBACK(entry_tracker_cb), wl);
     gtk_table_attach(GTK_TABLE(table), wl->cdEntryA, 1, 2, 1, 2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 5);

     wl->cdButtonA = gtk_button_new_with_label(_utf("query medium"));
     g_signal_connect(G_OBJECT(wl->cdButtonA), "clicked", G_CALLBACK(query_cb), wl);
     gtk_table_attach(GTK_TABLE(table), wl->cdButtonA, 2, 3, 1, 2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 5);

     icon = gtk_image_new_from_stock(GTK_STOCK_UNDO, GTK_ICON_SIZE_SMALL_TOOLBAR);
     wl->cdUndoButtonA = gtk_button_new();
     gtk_container_add(GTK_CONTAINER(wl->cdUndoButtonA), icon);
     g_signal_connect(G_OBJECT(wl->cdUndoButtonA), "clicked", G_CALLBACK(query_cb), wl);
     gtk_table_attach(GTK_TABLE(table), wl->cdUndoButtonA, 3, 4, 1, 2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 5);

   hbox = gtk_hbox_new(FALSE, 4);

   gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);

   wl->cdEntryB = gtk_entry_new();
   gtk_entry_set_width_chars(GTK_ENTRY(wl->cdEntryB), 9);
   g_signal_connect(G_OBJECT(wl->cdEntryB), "activate", G_CALLBACK(entry_tracker_cb), wl);
   gtk_box_pack_start(GTK_BOX(hbox), wl->cdEntryB, FALSE, FALSE, 0);

   wl->cdButtonB = gtk_button_new_with_label(_utf("query medium"));
   g_signal_connect(G_OBJECT(wl->cdButtonB), "clicked", G_CALLBACK(query_cb), wl);
   gtk_box_pack_start(GTK_BOX(hbox), wl->cdButtonB, FALSE, FALSE, 0);

   icon = gtk_image_new_from_stock(GTK_STOCK_UNDO, GTK_ICON_SIZE_SMALL_TOOLBAR);
   wl->cdUndoButtonB = gtk_button_new();
   gtk_container_add(GTK_CONTAINER(wl->cdUndoButtonB), icon);
   g_signal_connect(G_OBJECT(wl->cdUndoButtonB), "clicked", G_CALLBACK(query_cb), wl);
   gtk_box_pack_start(GTK_BOX(hbox), wl->cdUndoButtonB, FALSE, FALSE, 0);

   AddHelpWidget(lwoh, hbox);

   AddHelpParagraph(lwoh, _("<b>CD medium size</b>\n\n"
			    "This is the maximum capacity assumed for 80min CD media. "
			    "Images smaller than this value will be "
			    "augmented with error correction information "
			    "so that they will fit on the specified CD.\n\n"
			    "You can enter the medium size in sectors of 2K each, "
			    "or press the \"query medium\" button to use the size "
			    "of a medium currently inserted in the selected "
			    "drive. Sometimes this value is incorrect, though.\n"
			    "Use the arrow button to revert to the last saved value.\n\n"
			    "Please note that augmented images will at most triple "
			    "in size as the highest possible redundancy is 200%%.\n"
			    "Even if this limit is not reached the augmented image "
			    "may be a few sectors smaller than specified for "
			    "technical reasons."));

   /* DVD capacity table row */

   lwoh = CreateLabelWithOnlineHelp(_("DVD 1 layer:"), _("DVD 1 layer:"));
   RegisterPreferencesHelpWindow(lwoh);

     gtk_misc_set_alignment(GTK_MISC(lwoh->linkLabel), 0.0, 0.0); 
     gtk_table_attach(GTK_TABLE(table), lwoh->linkBox, 0, 1, 2, 3, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 5);

     wl->dvdEntry1A = gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(wl->dvdEntry1A), 9);
     g_signal_connect(G_OBJECT(wl->dvdEntry1A), "activate", G_CALLBACK(entry_tracker_cb), wl);
     gtk_table_attach(GTK_TABLE(table), wl->dvdEntry1A, 1, 2, 2, 3, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 5);

     wl->dvdButton1A = gtk_button_new_with_label(_utf("query medium"));
     g_signal_connect(G_OBJECT(wl->dvdButton1A), "clicked", G_CALLBACK(query_cb), wl);
     gtk_table_attach(GTK_TABLE(table), wl->dvdButton1A, 2, 3, 2, 3, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 5);

     icon = gtk_image_new_from_stock(GTK_STOCK_UNDO, GTK_ICON_SIZE_SMALL_TOOLBAR);
     wl->dvdUndoButton1A = gtk_button_new();
     gtk_container_add(GTK_CONTAINER(wl->dvdUndoButton1A), icon);
     g_signal_connect(G_OBJECT(wl->dvdUndoButton1A), "clicked", G_CALLBACK(query_cb), wl);
     gtk_table_attach(GTK_TABLE(table), wl->dvdUndoButton1A, 3, 4, 2, 3, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 5);

   hbox = gtk_hbox_new(FALSE, 4);

   gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);

   wl->dvdEntry1B = gtk_entry_new();
   gtk_entry_set_width_chars(GTK_ENTRY(wl->dvdEntry1B), 9);
   g_signal_connect(G_OBJECT(wl->dvdEntry1B), "activate", G_CALLBACK(entry_tracker_cb), wl);
   gtk_box_pack_start(GTK_BOX(hbox), wl->dvdEntry1B, FALSE, FALSE, 0);

   wl->dvdButton1B = gtk_button_new_with_label(_utf("query medium"));
   g_signal_connect(G_OBJECT(wl->dvdButton1B), "clicked", G_CALLBACK(query_cb), wl);
   gtk_box_pack_start(GTK_BOX(hbox), wl->dvdButton1B, FALSE, FALSE, 0);

   icon = gtk_image_new_from_stock(GTK_STOCK_UNDO, GTK_ICON_SIZE_SMALL_TOOLBAR);
   wl->dvdUndoButton1B = gtk_button_new();
   gtk_container_add(GTK_CONTAINER(wl->dvdUndoButton1B), icon);
   g_signal_connect(G_OBJECT(wl->dvdUndoButton1B), "clicked", G_CALLBACK(query_cb), wl);
   gtk_box_pack_start(GTK_BOX(hbox), wl->dvdUndoButton1B, FALSE, FALSE, 0);

   AddHelpWidget(lwoh, hbox);

   AddHelpParagraph(lwoh, _("<b>Single layer DVD medium size</b>\n\n"
			    "This is the maximum capacity assumed for single layer DVD media. "
			    "Images exceeding the smaller media sizes but smaller "
			    "than this value will be augmented with error correction information "
			    "so that they will fit on the specified DVD.\n\n"
			    "You can enter the medium size in sectors of 2K each, "
			    "or press the \"query medium\" button to use the size "
			    "of a medium currently inserted in the selected "
			    "drive. Sometimes this value is incorrect, though.\n"
			    "Use the arrow button to revert to the last saved value.\n\n"
			    "Please note that augmented images will at most triple "
			    "in size as the highest possible redundancy is 200%%.\n"
			    "Even if this limit is not reached the augmented image "
			    "may be a few sectors smaller than specified for "
			    "technical reasons."));

   /* DVD two layer capacity table row */

   lwoh = CreateLabelWithOnlineHelp(_("DVD 2 layers:"), _("DVD 2 layers:"));
   RegisterPreferencesHelpWindow(lwoh);

     gtk_misc_set_alignment(GTK_MISC(lwoh->linkLabel), 0.0, 0.0); 
     gtk_table_attach(GTK_TABLE(table), lwoh->linkBox, 0, 1, 3, 4, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 5);

     wl->dvdEntry2A = gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(wl->dvdEntry2A), 9);
     g_signal_connect(G_OBJECT(wl->dvdEntry2A), "activate", G_CALLBACK(entry_tracker_cb), wl);
     gtk_table_attach(GTK_TABLE(table), wl->dvdEntry2A, 1, 2, 3, 4, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 5);

     wl->dvdButton2A = gtk_button_new_with_label(_utf("query medium"));
     g_signal_connect(G_OBJECT(wl->dvdButton2A), "clicked", G_CALLBACK(query_cb), wl);
     gtk_table_attach(GTK_TABLE(table), wl->dvdButton2A, 2, 3, 3, 4, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 5);

     icon = gtk_image_new_from_stock(GTK_STOCK_UNDO, GTK_ICON_SIZE_SMALL_TOOLBAR);
     wl->dvdUndoButton2A = gtk_button_new();
     gtk_container_add(GTK_CONTAINER(wl->dvdUndoButton2A), icon);
     g_signal_connect(G_OBJECT(wl->dvdUndoButton2A), "clicked", G_CALLBACK(query_cb), wl);
     gtk_table_attach(GTK_TABLE(table), wl->dvdUndoButton2A, 3, 4, 3, 4, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 5);

   hbox = gtk_hbox_new(FALSE, 4);

   gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);

   wl->dvdEntry2B = gtk_entry_new();
   gtk_entry_set_width_chars(GTK_ENTRY(wl->dvdEntry2B), 9);
   g_signal_connect(G_OBJECT(wl->dvdEntry2B), "activate", G_CALLBACK(entry_tracker_cb), wl);
   gtk_box_pack_start(GTK_BOX(hbox), wl->dvdEntry2B, FALSE, FALSE, 0);

   wl->dvdButton2B = gtk_button_new_with_label(_utf("query medium"));
   g_signal_connect(G_OBJECT(wl->dvdButton2B), "clicked", G_CALLBACK(query_cb), wl);
   gtk_box_pack_start(GTK_BOX(hbox), wl->dvdButton2B, FALSE, FALSE, 0);

   icon = gtk_image_new_from_stock(GTK_STOCK_UNDO, GTK_ICON_SIZE_SMALL_TOOLBAR);
   wl->dvdUndoButton2B = gtk_button_new();
   gtk_container_add(GTK_CONTAINER(wl->dvdUndoButton2B), icon);
   g_signal_connect(G_OBJECT(wl->dvdUndoButton2B), "clicked", G_CALLBACK(query_cb), wl);
   gtk_box_pack_start(GTK_BOX(hbox), wl->dvdUndoButton2B, FALSE, FALSE, 0);

   AddHelpWidget(lwoh, hbox);

   AddHelpParagraph(lwoh, _("<b>Two layered DVD medium size</b>\n\n"
			    "This is the maximum capacity assumed for two layered DVD media. "
			    "Images exceeding the smaller media sizes but smaller "
			    "than this value will be augmented with error correction information "
			    "so that they will fit on the specified DVD.\n\n"
			    "You can enter the medium size in sectors of 2K each, "
			    "or press the \"query medium\" button to use the size "
			    "of a medium currently inserted in the selected "
			    "drive. Sometimes this value is incorrect, though.\n"
			    "Use the arrow button to revert to the last saved value.\n\n"
			    "Please note that augmented images will at most triple "
			    "in size as the highest possible redundancy is 200%%.\n"
			    "Even if this limit is not reached the augmented image "
			    "may be a few sectors smaller than specified for "
			    "technical reasons."));

   /* BD capacity table row */

   lwoh = CreateLabelWithOnlineHelp(_("BD 1 layer:"), _("BD 1 layer:"));
   RegisterPreferencesHelpWindow(lwoh);

     gtk_misc_set_alignment(GTK_MISC(lwoh->linkLabel), 0.0, 0.0); 
     gtk_table_attach(GTK_TABLE(table), lwoh->linkBox, 0, 1, 4, 5, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 5);

     wl->bdEntry1A = gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(wl->bdEntry1A), 9);
     g_signal_connect(G_OBJECT(wl->bdEntry1A), "activate", G_CALLBACK(entry_tracker_cb), wl);
     gtk_table_attach(GTK_TABLE(table), wl->bdEntry1A, 1, 2, 4, 5, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 5);

     wl->bdButton1A = gtk_button_new_with_label(_utf("query medium"));
     g_signal_connect(G_OBJECT(wl->bdButton1A), "clicked", G_CALLBACK(query_cb), wl);
     gtk_table_attach(GTK_TABLE(table), wl->bdButton1A, 2, 3, 4, 5, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 5);

     icon = gtk_image_new_from_stock(GTK_STOCK_UNDO, GTK_ICON_SIZE_SMALL_TOOLBAR);
     wl->bdUndoButton1A = gtk_button_new();
     gtk_container_add(GTK_CONTAINER(wl->bdUndoButton1A), icon);
     g_signal_connect(G_OBJECT(wl->bdUndoButton1A), "clicked", G_CALLBACK(query_cb), wl);
     gtk_table_attach(GTK_TABLE(table), wl->bdUndoButton1A, 3, 4, 4, 5, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 5);

   hbox = gtk_hbox_new(FALSE, 4);

   gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);

   wl->bdEntry1B = gtk_entry_new();
   gtk_entry_set_width_chars(GTK_ENTRY(wl->bdEntry1B), 9);
   g_signal_connect(G_OBJECT(wl->bdEntry1B), "activate", G_CALLBACK(entry_tracker_cb), wl);
   gtk_box_pack_start(GTK_BOX(hbox), wl->bdEntry1B, FALSE, FALSE, 0);

   wl->bdButton1B = gtk_button_new_with_label(_utf("query medium"));
   g_signal_connect(G_OBJECT(wl->bdButton1B), "clicked", G_CALLBACK(query_cb), wl);
   gtk_box_pack_start(GTK_BOX(hbox), wl->bdButton1B, FALSE, FALSE, 0);

   icon = gtk_image_new_from_stock(GTK_STOCK_UNDO, GTK_ICON_SIZE_SMALL_TOOLBAR);
   wl->bdUndoButton1B = gtk_button_new();
   gtk_container_add(GTK_CONTAINER(wl->bdUndoButton1B), icon);
   g_signal_connect(G_OBJECT(wl->bdUndoButton1B), "clicked", G_CALLBACK(query_cb), wl);
   gtk_box_pack_start(GTK_BOX(hbox), wl->bdUndoButton1B, FALSE, FALSE, 0);

   AddHelpWidget(lwoh, hbox);

   AddHelpParagraph(lwoh, _("<b>Single layer BD medium size</b>\n\n"
			    "This is the maximum capacity assumed for single layer BD media. "
			    "Images exceeding the smaller media sizes but smaller "
			    "than this value will be augmented with error correction information "
			    "so that they will fit on the specified BD.\n\n"
			    "You can enter the medium size in sectors of 2K each, "
			    "or press the \"query medium\" button to use the size "
			    "of a blank medium currently inserted in the selected drive.\n"
			    "Use the arrow button to revert to the last saved value.\n\n"
			    "Please note that augmented images will at most triple "
			    "in size as the highest possible redundancy is 200%%.\n"
			    "Even if this limit is not reached the augmented image "
			    "may be a few sectors smaller than specified for "
			    "technical reasons."));

   /* BD two layer capacity table row */

   lwoh = CreateLabelWithOnlineHelp(_("BD 2 layers:"), _("BD 2 layers:"));
   RegisterPreferencesHelpWindow(lwoh);

     gtk_misc_set_alignment(GTK_MISC(lwoh->linkLabel), 0.0, 0.0); 
     gtk_table_attach(GTK_TABLE(table), lwoh->linkBox, 0, 1, 5, 6, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 5);

     wl->bdEntry2A = gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(wl->bdEntry2A), 9);
     g_signal_connect(G_OBJECT(wl->bdEntry2A), "activate", G_CALLBACK(entry_tracker_cb), wl);
     gtk_table_attach(GTK_TABLE(table), wl->bdEntry2A, 1, 2, 5, 6, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 5);

     wl->bdButton2A = gtk_button_new_with_label(_utf("query medium"));
     g_signal_connect(G_OBJECT(wl->bdButton2A), "clicked", G_CALLBACK(query_cb), wl);
     gtk_table_attach(GTK_TABLE(table), wl->bdButton2A, 2, 3, 5, 6, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 5);

     icon = gtk_image_new_from_stock(GTK_STOCK_UNDO, GTK_ICON_SIZE_SMALL_TOOLBAR);
     wl->bdUndoButton2A = gtk_button_new();
     gtk_container_add(GTK_CONTAINER(wl->bdUndoButton2A), icon);
     g_signal_connect(G_OBJECT(wl->bdUndoButton2A), "clicked", G_CALLBACK(query_cb), wl);
     gtk_table_attach(GTK_TABLE(table), wl->bdUndoButton2A, 3, 4, 5, 6, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 5);

   hbox = gtk_hbox_new(FALSE, 4);

   gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);

   wl->bdEntry2B = gtk_entry_new();
   gtk_entry_set_width_chars(GTK_ENTRY(wl->bdEntry2B), 9);
   g_signal_connect(G_OBJECT(wl->bdEntry2B), "activate", G_CALLBACK(entry_tracker_cb), wl);
   gtk_box_pack_start(GTK_BOX(hbox), wl->bdEntry2B, FALSE, FALSE, 0);

   wl->bdButton2B = gtk_button_new_with_label(_utf("query medium"));
   g_signal_connect(G_OBJECT(wl->bdButton2B), "clicked", G_CALLBACK(query_cb), wl);
   gtk_box_pack_start(GTK_BOX(hbox), wl->bdButton2B, FALSE, FALSE, 0);

   icon = gtk_image_new_from_stock(GTK_STOCK_UNDO, GTK_ICON_SIZE_SMALL_TOOLBAR);
   wl->bdUndoButton2B = gtk_button_new();
   gtk_container_add(GTK_CONTAINER(wl->bdUndoButton2B), icon);
   g_signal_connect(G_OBJECT(wl->bdUndoButton2B), "clicked", G_CALLBACK(query_cb), wl);
   gtk_box_pack_start(GTK_BOX(hbox), wl->bdUndoButton2B, FALSE, FALSE, 0);

   AddHelpWidget(lwoh, hbox);

   AddHelpParagraph(lwoh, _("<b>Two layered BD medium size</b>\n\n"
			    "This is the maximum capacity assumed for two layered BD media. "
			    "Images exceeding the smaller media sizes but smaller "
			    "than this value will be augmented with error correction information "
			    "so that they will fit on the specified BD.\n\n"
			    "You can enter the medium size in sectors of 2K each, "
			    "or press the \"query medium\" button to use the size "
			    "of a blank medium currently inserted in the selected drive.\n"
			    "Use the arrow button to revert to the last saved value.\n\n"
			    "Please note that augmented images will at most triple "
			    "in size as the highest possible redundancy is 200%%.\n"
			    "Even if this limit is not reached the augmented image "
			    "may be a few sectors smaller than specified for "
			    "technical reasons."));

   /* Fill in values from the closure */

   g_snprintf(value, 40, "%lld", (long long int)Closure->cdSize);
   gtk_entry_set_text(GTK_ENTRY(wl->cdEntryB), value);
   gtk_entry_set_text(GTK_ENTRY(wl->cdEntryA), value);
   g_snprintf(value, 40, "%lld", (long long int)Closure->dvdSize1);
   gtk_entry_set_text(GTK_ENTRY(wl->dvdEntry1A), value);
   gtk_entry_set_text(GTK_ENTRY(wl->dvdEntry1B), value);
   g_snprintf(value, 40, "%lld", (long long int)Closure->dvdSize2);
   gtk_entry_set_text(GTK_ENTRY(wl->dvdEntry2A), value);
   gtk_entry_set_text(GTK_ENTRY(wl->dvdEntry2B), value);
   g_snprintf(value, 40, "%lld", (long long int)Closure->bdSize1);
   gtk_entry_set_text(GTK_ENTRY(wl->bdEntry1A), value);
   gtk_entry_set_text(GTK_ENTRY(wl->bdEntry1B), value);
   g_snprintf(value, 40, "%lld", (long long int)Closure->bdSize2);
   gtk_entry_set_text(GTK_ENTRY(wl->bdEntry2A), value);
   gtk_entry_set_text(GTK_ENTRY(wl->bdEntry2B), value);

   /* custom value selection */

   lwoh = CreateLabelWithOnlineHelp(_("Use at most"), _("Use at most"));
   RegisterPreferencesHelpWindow(lwoh);

   for(i=0; i<2; i++)
   {  GtkWidget *hbox = gtk_hbox_new(FALSE, 4);
      GtkWidget *entry;

      if(!i) wl->radio2A = radio = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(wl->radio1A));
      else   wl->radio2B = radio = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(wl->radio1B));

      g_signal_connect(G_OBJECT(radio), "toggled", G_CALLBACK(toggle_cb), method);
      gtk_box_pack_start(GTK_BOX(hbox), radio, FALSE, FALSE, 0);

      gtk_box_pack_start(GTK_BOX(hbox), !i ? lwoh->linkBox : lwoh->normalLabel, FALSE, FALSE, 0);
 
      entry = gtk_entry_new();
      gtk_entry_set_width_chars(GTK_ENTRY(entry), 9);
      g_signal_connect(G_OBJECT(entry), "activate", G_CALLBACK(entry_tracker_cb), wl);
      gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 0);

      lab = gtk_label_new(_utf("sectors."));
      gtk_box_pack_start(GTK_BOX(hbox), lab, FALSE, FALSE, 0);

      if(!i)
      {  wl->otherEntryA = entry;
	 gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
      }
      else
      {  wl->otherEntryB = entry;
	 AddHelpWidget(lwoh, hbox);
      }
   }

   AddHelpParagraph(lwoh, _("<b>Use at most ... sectors</b>\n\n"
			    "Use this option to override the table settings; "
			    "the augmented image will be expanded to the size "
			    "given in this field.\n"
			    "This allows for the creation of DVD-sized augmented images "
			    "which would normally be fitted to CD size, and to use "
			    "non standard media.\n\n"
			    "Please note that augmented images will at most triple "
			    "in size as the highest possible redundancy is 200%%.\n"
			    "Even if this limit is not reached the augmented image "
			    "may be a few sectors smaller than specified for "
			    "technical reasons."));

   g_snprintf(value, 40, "%lld", (long long int)Closure->mediumSize);
   gtk_entry_set_text(GTK_ENTRY(wl->otherEntryA), value);
   gtk_entry_set_text(GTK_ENTRY(wl->otherEntryB), value);

   /* Initialize radio buttons */

   if(Closure->mediumSize)
   {    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio2A), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio2B), TRUE);
   }
   else 
   {    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio1A), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio1B), TRUE);
        gtk_widget_set_sensitive(wl->otherEntryA, FALSE);
        gtk_widget_set_sensitive(wl->otherEntryB, FALSE);
   }

   /* Memory utilization */

   frame = gtk_frame_new(_utf("Memory utilization"));
   gtk_box_pack_start(GTK_BOX(parent), frame, FALSE, FALSE, 0);

   text = g_strdup_printf(_("%d MiB of file cache"), Closure->cacheMiB);
   lwoh = CreateLabelWithOnlineHelp(_("File cache"), text);
   RegisterPreferencesHelpWindow(lwoh);
   g_free(text);

   wl->cacheLwoh = lwoh;
   LockLabelSize(GTK_LABEL(lwoh->normalLabel), _utf("%d MiB of file cache"), 2222);
   LockLabelSize(GTK_LABEL(lwoh->linkLabel), _utf("%d MiB of file cache"), 2222);

   for(i=0; i<2; i++)
   {  GtkWidget *hbox = gtk_hbox_new(FALSE, 4);
      int n_entries = sizeof(cache_size)/sizeof(int);

      lab = gtk_label_new(_utf("Use"));
      gtk_box_pack_start(GTK_BOX(hbox), lab, FALSE, FALSE, 0);

      for(index = 0; index < n_entries; index++)
	if(cache_size[index] > Closure->cacheMiB)
	  break;

      scale = gtk_hscale_new_with_range(0,n_entries-1,1);
      gtk_scale_set_value_pos(GTK_SCALE(scale), GTK_POS_RIGHT);
      gtk_range_set_increments(GTK_RANGE(scale), 1, 1);
      gtk_range_set_value(GTK_RANGE(scale), index > 0 ? index-1 : index);
      g_signal_connect(scale, "format-value", G_CALLBACK(format_cb), (gpointer)PREF_CACHE);
      g_signal_connect(scale, "value-changed", G_CALLBACK(cache_cb), (gpointer)wl);
      gtk_box_pack_start(GTK_BOX(hbox), scale, TRUE, TRUE, 0);

      if(!i)
      {  wl->cacheScaleA = scale; 
	 gtk_container_set_border_width(GTK_CONTAINER(hbox), 10);
	 gtk_box_pack_start(GTK_BOX(hbox), lwoh->linkBox, FALSE, FALSE, 0);
	 gtk_container_add(GTK_CONTAINER(frame), hbox);
      }
      else
      {  wl->cacheScaleB = scale; 
	 gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);
	 AddHelpWidget(lwoh, hbox);
      }
   }

   AddHelpParagraph(lwoh, _("<b>File cache</b>\n\n"
			    "dvdisaster optimizes access to the image and error correction "
			    "files by maintaining its own cache. "
			    "The preset of 32MiB is suitable for most systems."));
}
