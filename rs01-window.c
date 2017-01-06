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

#include "rs01-includes.h"

/***
 *** Forward declarations
 ***/

static void redraw_curve(RS01Widgets*);
static void update_geometry(RS01Widgets*);

/* Protected widget access */
      
static void activate_toggle_button(GtkToggleButton *toggle, int state)
{  if(toggle) gtk_toggle_button_set_active(toggle, state);
}

static void set_range_value(GtkRange *range, int value)
{  if(range) gtk_range_set_value(range, value);
}

static void set_spin_button_value(GtkSpinButton *spin, int value)
{  if(spin) gtk_spin_button_set_value(spin, value);
}

/***
 *** Encoding window
 ***/

/*
 * Reset the notebook contents for new encoding action
 */

void ResetRS01EncodeWindow(Method *method)
{  RS01Widgets *wl = (RS01Widgets*)method->widgetList;

   SetProgress(wl->encPBar1, 0, 100);
   SetProgress(wl->encPBar2, 0, 100);

   gtk_widget_hide(wl->encLabel2);
   gtk_widget_hide(wl->encPBar2);
   gtk_widget_hide(wl->curveButton);

   gtk_label_set_text(GTK_LABEL(wl->encFootline), "");
   gtk_label_set_text(GTK_LABEL(wl->encFootline2), "");

}

/* 
 * Show the button for switching to the reading curve
 */

static gboolean show_button_idle_func(gpointer data)
{  Method *method = (Method*)data;
   RS01Widgets *wl = (RS01Widgets*)method->widgetList;

   gtk_widget_show(wl->curveButton);

   return FALSE;
}

void RS01ShowCurveButton(Method *method)
{  
   g_idle_add(show_button_idle_func, method);
   
}

/* 
 * Switch back to the reading curve (read and create mode only)
 */

static gboolean curve_button_cb(GtkWidget *wid, gpointer action)
{  gtk_notebook_set_current_page(GTK_NOTEBOOK(Closure->notebook), 1);
  
   return FALSE;
}

/*
 * Create the notebook contents for creating an error correction file
 */

void CreateRS01EWindow(Method *method, GtkWidget *parent)
{  RS01Widgets *wl;
   GtkWidget *sep,*wid,*pbar,*table,*hbox;

   if(!method->widgetList)
   {  wl = g_malloc0(sizeof(RS01Widgets));
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
			_utf("<b>1. Calculating image sector checksums:</b>"));
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

   hbox = gtk_hbox_new(FALSE, 0);
   gtk_box_pack_start(GTK_BOX(parent), hbox, FALSE, FALSE, 0);

   wid = gtk_label_new(NULL);
   gtk_misc_set_padding(GTK_MISC(wid), 10, 0);
   gtk_box_pack_start(GTK_BOX(hbox), wid, FALSE, FALSE, 0);

   wl->curveButton = gtk_button_new_with_label(_utf("Show reading speed curve"));
   g_signal_connect(G_OBJECT(wl->curveButton), "clicked", G_CALLBACK(curve_button_cb), NULL);
   gtk_box_pack_start(GTK_BOX(hbox), wl->curveButton, FALSE, FALSE, 0);
}

/***
 *** "Fix" window
 ***/

/*
 * Set the media size and ecc capacity
 */

static gboolean set_max_idle_func(gpointer data)
{  RS01Widgets *wl = (RS01Widgets*)data;

   redraw_curve(wl);

   return FALSE;
}

void RS01SetFixMaxValues(RS01Widgets *wl, int data_bytes, int ecc_bytes, gint64 sectors)
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
{  RS01Widgets *wl = (RS01Widgets*)data;

   SetLabelText(GTK_LABEL(wl->fixCorrected), _("Repaired: %lld"), wl->corrected); 
   SetLabelText(GTK_LABEL(wl->fixUncorrected), _("Unrepairable: <span %s>%lld</span>"),Closure->redMarkup, wl->uncorrected); 
   SetLabelText(GTK_LABEL(wl->fixProgress), _("Progress: %3d.%1d%%"), wl->percent/10, wl->percent%10);

   return FALSE;
}

void RS01UpdateFixResults(RS01Widgets *wl, gint64 corrected, gint64 uncorrected)
{
   wl->corrected = corrected;
   wl->uncorrected = uncorrected;

   g_idle_add(results_idle_func, wl);
}

/*
 * Update the error curve 
 */

static gboolean curve_idle_func(gpointer data)
{  RS01Widgets *wl = (RS01Widgets*)data;
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

void RS01AddFixValues(RS01Widgets *wl, int percent, int ecc_max)
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

static void update_geometry(RS01Widgets *wl)
{  
   /* Curve geometry */ 

   UpdateCurveGeometry(wl->fixCurve, "999", 20);

   /* Label positions in the foot line */

   gtk_box_set_child_packing(GTK_BOX(wl->fixFootlineBox), wl->fixCorrected,
			     TRUE, TRUE, wl->fixCurve->leftX, GTK_PACK_START);
   gtk_box_set_child_packing(GTK_BOX(wl->fixFootlineBox), wl->fixUncorrected, 
			     TRUE, TRUE, wl->fixCurve->leftX, GTK_PACK_START);
}

static void redraw_curve(RS01Widgets *wl)
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
{  RS01Widgets *wl = (RS01Widgets*)data; 

   if(event->count) /* Exposure compression */
   {  return TRUE;
   }

   update_geometry(wl);
   redraw_curve(wl);

   return TRUE;
}

/*
 * Reset the notebook contents for new fixing action
 */

void ResetRS01FixWindow(Method *method)
{  RS01Widgets *wl = (RS01Widgets*)method->widgetList;

   gtk_notebook_set_current_page(GTK_NOTEBOOK(wl->fixNotebook), 0);

   ZeroCurve(wl->fixCurve);
   RS01UpdateFixResults(wl, 0, 0);

   if(wl->fixCurve && wl->fixCurve->widget)
   {  gdk_window_clear(wl->fixCurve->widget->window);
      redraw_curve(wl);
   }

   wl->percent = 0;
   wl->lastPercent = 0;
}

/*
 * Create the notebook contents for fixing an image
 */

void CreateRS01FWindow(Method *method, GtkWidget *parent)
{  RS01Widgets *wl;
   GtkWidget *sep,*ignore,*d_area,*notebook,*hbox;

   if(!method->widgetList)
   {  wl = g_malloc0(sizeof(RS01Widgets));
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

#define SYMBOLSIZE 8
#define FIELDSIZE (1<<SYMBOLSIZE)
#define FIELDMAX (FIELDSIZE-1)

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
   int ndata  = FIELDMAX - nroots;
   char *label;

   if(GPOINTER_TO_INT(data) == PREF_CACHE)
   {
     label = g_strdup(" ");
   }
   else
     label = g_strdup_printf(_utf("%4.1f%% redundancy (%d roots)"),
			    ((double)nroots*100.0)/(double)ndata,
			    nroots);

   FORGET(label);  /* will be g_free()ed by the scale */
   return label;
}

static void cache_cb(GtkWidget *widget, gpointer data)
{  RS01Widgets *wl = (RS01Widgets*)data;
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

static void nroots_cb(GtkWidget *widget, gpointer data)
{  RS01Widgets *wl = (RS01Widgets*)data;
   int value;

   value = gtk_range_get_value(GTK_RANGE(widget));
   if(Closure->redundancy) g_free(Closure->redundancy);
   Closure->redundancy = g_strdup_printf("%d", value);

   if(widget == wl->redundancyScaleA)
        set_range_value(GTK_RANGE(wl->redundancyScaleB), value);
   else set_range_value(GTK_RANGE(wl->redundancyScaleA), value);

   UpdateMethodPreferences();
}

static void ecc_size_cb(GtkWidget *widget, gpointer data)
{  RS01Widgets *wl = (RS01Widgets*)data;
   int value;

   value = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
   if(Closure->redundancy) g_free(Closure->redundancy);
   Closure->redundancy = g_strdup_printf("%dm", value);

   if(widget == wl->redundancySpinA)
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(wl->redundancySpinB), atoi(Closure->redundancy));
   else gtk_spin_button_set_value(GTK_SPIN_BUTTON(wl->redundancySpinA), atoi(Closure->redundancy));

   UpdateMethodPreferences();
}

static void toggle_cb(GtkWidget *widget, gpointer data)
{  Method *method = (Method*)data;
   RS01Widgets *wl = (RS01Widgets*)method->widgetList;
   int state  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
      
   if(state == TRUE)
   {  if(widget == wl->radio3A || widget == wl->radio3B)
      {  gtk_widget_set_sensitive(wl->redundancyScaleA, TRUE);
	 gtk_widget_set_sensitive(wl->redundancyScaleB, TRUE);
      }
      else
      {  gtk_widget_set_sensitive(wl->redundancyScaleA, FALSE);
	 gtk_widget_set_sensitive(wl->redundancyScaleB, FALSE);
      }

      if(widget == wl->radio4A || widget == wl->radio4B)
      {  gtk_widget_set_sensitive(wl->redundancySpinA, TRUE); 
	 gtk_widget_set_sensitive(wl->redundancySpinB, TRUE); 
	 gtk_widget_set_sensitive(wl->radio4LabelA, TRUE); 
	 gtk_widget_set_sensitive(wl->radio4LabelB, TRUE); 
      }
      else
      {  gtk_widget_set_sensitive(wl->redundancySpinA, FALSE); 
	 gtk_widget_set_sensitive(wl->redundancySpinB, FALSE); 
	 gtk_widget_set_sensitive(wl->radio4LabelA, FALSE); 
	 gtk_widget_set_sensitive(wl->radio4LabelB, FALSE); 
      }

      if(   widget == wl->radio1A  /* Normal */
	 || widget == wl->radio1B)
      {  
         set_range_value(GTK_RANGE(wl->redundancyScaleA), 32);
         set_range_value(GTK_RANGE(wl->redundancyScaleB), 32);

	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio1A), TRUE);
	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio1B), TRUE);

	 if(Closure->redundancy) g_free(Closure->redundancy);
         Closure->redundancy = g_strdup("normal");
      }

      if(   widget == wl->radio2A  /* High */
	 || widget == wl->radio2B)
      {  
         set_range_value(GTK_RANGE(wl->redundancyScaleA), 64);
         set_range_value(GTK_RANGE(wl->redundancyScaleB), 64);

	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio2A), TRUE);
	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio2B), TRUE);

	 if(Closure->redundancy) g_free(Closure->redundancy);
	 Closure->redundancy = g_strdup("high");
      }

      if(   widget == wl->radio3A  /* number of roots */
	 || widget == wl->radio3B)
      {  int nroots = gtk_range_get_value(GTK_RANGE(wl->redundancyScaleA));

	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio3A), TRUE);
	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio3B), TRUE);

	 if(Closure->redundancy) g_free(Closure->redundancy);
	 Closure->redundancy = g_strdup_printf("%d", nroots);
      }

      if(   widget == wl->radio4A  /* relative to space usage */
	 || widget == wl->radio4B)
      {  int space = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(wl->redundancySpinA));

	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio4A), TRUE);
	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio4B), TRUE);

	 if(Closure->redundancy) g_free(Closure->redundancy);
	 Closure->redundancy = g_strdup_printf("%dm", space);
      }

      UpdateMethodPreferences();
   }
}

void ResetRS01PrefsPage(Method *method)
{  RS01Widgets *wl = (RS01Widgets*)method->widgetList;
   int index;

   /* Redundancy selection */

   if(Closure->redundancy)
   {  
      if(!strcmp(Closure->redundancy, "normal"))
      {  if(wl->radio1A && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wl->radio1A)) == FALSE)
	 {  activate_toggle_button(GTK_TOGGLE_BUTTON(wl->radio1A), TRUE);
	    activate_toggle_button(GTK_TOGGLE_BUTTON(wl->radio1B), TRUE);
	 }
      }
      else if(!strcmp(Closure->redundancy, "high"))
      {  if(wl->radio2A && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wl->radio2A)) == FALSE)
	 {  activate_toggle_button(GTK_TOGGLE_BUTTON(wl->radio2A), TRUE);
	    activate_toggle_button(GTK_TOGGLE_BUTTON(wl->radio2B), TRUE);
	 }
      }
      else
      {  int last = strlen(Closure->redundancy)-1;

         if(Closure->redundancy[last] == 'm')
	 {  if(wl->redundancySpinA)
	    {  int old = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(wl->redundancySpinA));
	       int new;

	       Closure->redundancy[last] = 0;
	       new = atoi(Closure->redundancy);
	       Closure->redundancy[last] = 'm';

	       if(new != old)
	       {  set_spin_button_value(GTK_SPIN_BUTTON(wl->redundancySpinA), new);
		  set_spin_button_value(GTK_SPIN_BUTTON(wl->redundancySpinB), new);
	       }

	       if(wl->radio4A && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wl->radio4A)) == FALSE)
	       {  activate_toggle_button(GTK_TOGGLE_BUTTON(wl->radio4A), TRUE);
		  activate_toggle_button(GTK_TOGGLE_BUTTON(wl->radio4B), TRUE);
	       }
	    }
	 }
	 else
	 {  if(wl->redundancyScaleA)
	    {  int old = gtk_range_get_value(GTK_RANGE(wl->redundancyScaleA));
	       int new = atoi(Closure->redundancy);

	       if(new != old)
	       {  set_range_value(GTK_RANGE(wl->redundancyScaleA), new);
	          set_range_value(GTK_RANGE(wl->redundancyScaleB), new);
	       }

	       if(wl->radio3A && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wl->radio3A)) == FALSE)
	       {  activate_toggle_button(GTK_TOGGLE_BUTTON(wl->radio3A), TRUE);
		  activate_toggle_button(GTK_TOGGLE_BUTTON(wl->radio3B), TRUE);
	       }
	    }
	 }
      }
   }

   /* Memory caching */

   for(index = 0; index < sizeof(cache_size)/sizeof(int); index++)
     if(cache_size[index] > Closure->cacheMiB)
       break;

   set_range_value(GTK_RANGE(wl->cacheScaleA), index > 0 ? index-1 : index);
   set_range_value(GTK_RANGE(wl->cacheScaleB), index > 0 ? index-1 : index);
}

void CreateRS01PrefsPage(Method *method, GtkWidget *parent)
{  RS01Widgets *wl = (RS01Widgets*)method->widgetList;
   GtkWidget *frame, *hbox, *vbox, *lab, *scale, *spin;
   GtkWidget *radio; 
   LabelWithOnlineHelp *lwoh;
   unsigned int i, index;
   char *text;

   /*** Redundancy selection */

   frame = gtk_frame_new(_utf("Redundancy for new error correction files"));
   gtk_box_pack_start(GTK_BOX(parent), frame, FALSE, FALSE, 0);

   vbox = gtk_vbox_new(FALSE, 10);
   gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
   gtk_container_add(GTK_CONTAINER(frame), vbox);

   /* Normal redundancy */

   lwoh = CreateLabelWithOnlineHelp(_("Normal redundancy"), _("Normal"));
   RegisterPreferencesHelpWindow(lwoh);

   for(i=0; i<2; i++)
   {  GtkWidget *hbox = gtk_hbox_new(FALSE, 4);

      radio = gtk_radio_button_new(NULL);
      g_signal_connect(G_OBJECT(radio), "toggled", G_CALLBACK(toggle_cb), method);
      gtk_box_pack_start(GTK_BOX(hbox), radio, FALSE, FALSE, 0);

      if(!i)
      {  wl->radio1A = radio;
	 gtk_box_pack_start(GTK_BOX(hbox), lwoh->linkBox, FALSE, FALSE, 0);
	 gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
      }
      else
      {  wl->radio1B = radio;
         gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);
	 AddHelpWidget(lwoh, hbox);
      }
   }

   AddHelpParagraph(lwoh, _("<b>Normal redundancy</b>\n\n"
			    "The preset \"normal\" creates a redundancy of 14.3%%.\n"
			    "It invokes optimized program code to speed up the "
			    "error correction file creation."));

   /* High redundancy */

   lwoh = CreateLabelWithOnlineHelp(_("High redundancy"), _("High"));
   RegisterPreferencesHelpWindow(lwoh);

   for(i=0; i<2; i++)
   {  GtkWidget *hbox = gtk_hbox_new(FALSE, 4);

      radio = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(i?wl->radio1B:wl->radio1A));
      g_signal_connect(G_OBJECT(radio), "toggled", G_CALLBACK(toggle_cb), method);
      gtk_box_pack_start(GTK_BOX(hbox), radio, FALSE, FALSE, 0);

      if(!i)
      {  wl->radio2A = radio;
	 gtk_box_pack_start(GTK_BOX(hbox), lwoh->linkBox, FALSE, FALSE, 0);
	 gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
      }
      else
      {  wl->radio2B = radio;
         gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);
	 AddHelpWidget(lwoh, hbox);
      }
   }

   AddHelpParagraph(lwoh, _("<b>High redundancy</b>\n\n"
			    "The preset \"high\" creates a redundancy of 33.5%%.\n"
			    "It invokes optimized program code to speed up the "
			    "error correction file creation."));


   /* User-selected redundancy */

   lwoh = CreateLabelWithOnlineHelp(_("Other redundancy"), _("Other"));
   RegisterPreferencesHelpWindow(lwoh);

   for(i=0; i<2; i++)
   {  hbox = gtk_hbox_new(FALSE, 4);

      radio = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(i?wl->radio1B:wl->radio1A));
      g_signal_connect(G_OBJECT(radio), "toggled", G_CALLBACK(toggle_cb), method);
      gtk_box_pack_start(GTK_BOX(hbox), radio, FALSE, FALSE, 0);

      if(!i)
      {  wl->radio3A = radio;
	 gtk_box_pack_start(GTK_BOX(hbox), lwoh->linkBox, FALSE, FALSE, 0);
      }
      else
      {  wl->radio3B = radio;
         gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);
      }

      scale = gtk_hscale_new_with_range(8,100,1);
      gtk_scale_set_value_pos(GTK_SCALE(scale), GTK_POS_RIGHT);
      gtk_range_set_increments(GTK_RANGE(scale), 1, 1);
      gtk_range_set_value(GTK_RANGE(scale), 32);
      gtk_widget_set_sensitive(scale, FALSE);
      g_signal_connect(scale, "format-value", G_CALLBACK(format_cb), (gpointer)PREF_NROOTS);
      g_signal_connect(scale, "value-changed", G_CALLBACK(nroots_cb), (gpointer)wl);
      gtk_container_add(GTK_CONTAINER(hbox), scale);

      if(!i)
      {  wl->redundancyScaleA = scale;
	 gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
      }
      else
      {  wl->redundancyScaleB = scale;
	 AddHelpWidget(lwoh, hbox);
      }
   }

   AddHelpParagraph(lwoh, _("<b>Other redundancy</b>\n\n"
			    "Specifies the redundancy by percent.\n"
			    "An error correction file with x%% redundancy "
			    "will be approximately x%% of the size of the "
			    "corresponding image file."));

   /* Space-delimited redundancy */

   lwoh = CreateLabelWithOnlineHelp(_("Space-delimited redundancy"), _("Use at most"));
   RegisterPreferencesHelpWindow(lwoh);

   for(i=0; i<2; i++)
   {  hbox = gtk_hbox_new(FALSE, 4);

      radio = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(i?wl->radio1B:wl->radio1A));
      g_signal_connect(G_OBJECT(radio), "toggled", G_CALLBACK(toggle_cb), method);
      gtk_box_pack_start(GTK_BOX(hbox), radio, FALSE, FALSE, 0);

      if(!i)
      {  wl->radio4A = radio;
	 gtk_box_pack_start(GTK_BOX(hbox), lwoh->linkBox, FALSE, FALSE, 0);
      }
      else
      {  wl->radio4B = radio;
         gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);
      }

      spin = gtk_spin_button_new_with_range(0, 100000, 100);
      g_signal_connect(spin, "value-changed", G_CALLBACK(ecc_size_cb), (gpointer)wl);
      gtk_entry_set_width_chars(GTK_ENTRY(spin), 8);
      gtk_box_pack_start(GTK_BOX(hbox), spin, FALSE, FALSE, 0);

      lab = gtk_label_new(_utf("MiB for error correction data"));
      gtk_box_pack_start(GTK_BOX(hbox), lab, FALSE, FALSE, 0);
      gtk_widget_set_sensitive(spin, FALSE);
      gtk_widget_set_sensitive(lab, FALSE);

      if(!i)
      {  wl->redundancySpinA = spin;
	 wl->radio4LabelA = lab;
         gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
      }
      else
      {  wl->redundancySpinB = spin;
	 wl->radio4LabelB = lab;
	 AddHelpWidget(lwoh, hbox);
      }
   }

   AddHelpParagraph(lwoh, _("<b>Space-delimited redundancy</b>\n\n"
			    "Specifies the maximum size of the error correction file in MiB. "
			    "dvdisaster will choose a suitable redundancy setting so that "
			    "the overall size of the error correction file does not exceed "
			    "the given limit.\n\n"
			    "<b>Advance notice:</b> When using the same size setting for "
			    "images of vastly different size, smaller images receive more "
			    "redundancy than larger ones. This is usually not what you want."));

   /*** Preset redundancy values */

   if(Closure->redundancy)
   {  if(!strcmp(Closure->redundancy, "normal"))
      {  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio1A), TRUE);
         gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio1B), TRUE);
      }
      else if(!strcmp(Closure->redundancy, "high"))
      {  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio2A), TRUE);
         gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio2B), TRUE);
      }
      else
      {  int last = strlen(Closure->redundancy)-1;

         if(Closure->redundancy[last] == 'm')
	 {  Closure->redundancy[last] = 0;
	    gtk_spin_button_set_value(GTK_SPIN_BUTTON(wl->redundancySpinA), atoi(Closure->redundancy));
	    gtk_spin_button_set_value(GTK_SPIN_BUTTON(wl->redundancySpinB), atoi(Closure->redundancy));
	    Closure->redundancy[last] = 'm';
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio4A), TRUE);
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio4B), TRUE);
	 }
	 else
	 {  gtk_range_set_value(GTK_RANGE(wl->redundancyScaleA), atoi(Closure->redundancy));
	    gtk_range_set_value(GTK_RANGE(wl->redundancyScaleB), atoi(Closure->redundancy));
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio3A), TRUE);
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio3B), TRUE);
	 }
      }
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
