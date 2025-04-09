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

/*** src type: only GUI code ***/

#include "dvdisaster.h"

#ifdef WITH_GUI_YES

/***
 *** Spiral drawing and updating
 ***/

static long long int readable, correctable, missing;
static int percent,min_required;
static GdkRGBA *footer_color;

#define REDRAW_TITLE      1<<0
#define REDRAW_SUBTITLE   1<<1
#define REDRAW_PROGRESS   1<<2
#define REDRAW_ERRORMSG   1<<3

static int draw_text(cairo_t *cr, PangoLayout *l, char *text, int x, int y, GdkRGBA *color, int redraw)
{  int w,h,pw;
   int erase_to;

   GuiSetText(l, text, &w, &h);

   if(redraw)
   {  erase_to = Closure->readAdaptiveSpiral->mx - Closure->readAdaptiveSpiral->diameter/2;
      pw = erase_to-x;

      gdk_cairo_set_source_rgba(cr, color);
      cairo_move_to(cr, x, y);
      pango_cairo_show_layout(cr, l);
   }

   return h;
}

static void redraw_labels(cairo_t *cr, GtkWidget *widget, int erase_mask)
{  char buf[256];
   int x,y,w,h;

   /* Get foreground color */

   GdkRGBA fg = {0};
   GtkStyleContext *context = gtk_widget_get_style_context(widget);
   gtk_style_context_get_color(context, gtk_widget_get_state_flags(widget), &fg);

   /* Draw the labels */

   x = 10; 
   gdk_cairo_set_source_rgba(cr, &fg);

   y = Closure->readAdaptiveSpiral->my - Closure->readAdaptiveSpiral->diameter/2;
   h = draw_text(cr, Closure->readLinearCurve->layout,
		 _("Adaptive reading:"), x, y, &fg, erase_mask & REDRAW_TITLE);

   y += h+h/2;
   if(Closure->readAdaptiveSubtitle)
   {  char *c = Closure->readAdaptiveSubtitle + strlen(Closure->readAdaptiveSubtitle)/2;

      while(*c && *c != ' ')  /* find point to split text in middle */
        c++;

      if(*c)                  /* split text into two lines */
      {  *c = 0;
	 h = draw_text(cr, Closure->readLinearCurve->layout,
		       Closure->readAdaptiveSubtitle, x, y, &fg,
		       erase_mask & REDRAW_SUBTITLE); 
	 h = draw_text(cr, Closure->readLinearCurve->layout,
			c+1, x, y+h, &fg,
			erase_mask & REDRAW_SUBTITLE); 
	 *c = ' ';
      }
      else                    /* draw text in one line */
      {  h = draw_text(cr, Closure->readLinearCurve->layout,
		       Closure->readAdaptiveSubtitle, x, y, &fg,
		       erase_mask & REDRAW_SUBTITLE); 
      }
   }

   y += 4*h;
   h = draw_text(cr, Closure->readLinearCurve->layout,
		 _("Sectors processed"), x, y, &fg, erase_mask & REDRAW_TITLE);

   y += h;
   snprintf(buf, 255, "  %s: %lld", _("readable"), readable);
   h = draw_text(cr, Closure->readLinearCurve->layout, buf, x, y, &fg,
                 erase_mask & REDRAW_PROGRESS);

   y += h;
   snprintf(buf, 255, "  %s: %lld", _("correctable"), correctable);
   h = draw_text(cr, Closure->readLinearCurve->layout, buf, x, y, &fg,
                 erase_mask & REDRAW_PROGRESS);

   y += h;
   snprintf(buf, 255, "  %s: %lld", _("missing"), missing);
   h = draw_text(cr, Closure->readLinearCurve->layout, buf, x, y, &fg,
                 erase_mask & REDRAW_PROGRESS);

   if(min_required > 0 && readable > 0)
   {  int percent = round(((1000*readable)/(readable+correctable+missing)));

      if(!missing)                /* Make sure target percentage is reached */
	percent = min_required;   /* in spite of rounding errors            */

      y += h;
      snprintf(buf, 255, _("Readable: %d.%d%% / %d.%d%% required"), 
	       percent/10, percent%10,
	       min_required/10, min_required%10);
      h = draw_text(cr, Closure->readLinearCurve->layout, buf, x, y, &fg,
                    erase_mask & REDRAW_PROGRESS);
   }

   y += h;
   snprintf(buf, 255, _("Total recoverable: %d.%d%%"), percent/10, percent%10);
   h = draw_text(cr, Closure->readLinearCurve->layout, buf, x, y, &fg,
                 erase_mask & REDRAW_PROGRESS);


   if(Closure->readAdaptiveErrorMsg && erase_mask & REDRAW_ERRORMSG)
   {  gdk_cairo_set_source_rgba(cr, footer_color ? footer_color : &fg);
      
      GuiSetText(Closure->readLinearCurve->layout, Closure->readAdaptiveErrorMsg, &w, &h);
      y = Closure->readAdaptiveSpiral->my + Closure->readAdaptiveSpiral->diameter/2 - h;
      cairo_move_to(cr, x, y);
      pango_cairo_show_layout(cr, Closure->readLinearCurve->layout);
   }
}

/* Calculate the geometry of the spiral */

static void update_geometry(GtkWidget *widget)
{  GtkAllocation a = {0};
   gtk_widget_get_allocation(widget, &a);

   Closure->readAdaptiveSpiral->mx = a.width - 15 - Closure->readAdaptiveSpiral->diameter / 2;
   Closure->readAdaptiveSpiral->my = a.height / 2;
}

/* Draw event handler */

static gboolean draw_cb(GtkWidget *widget, cairo_t *cr, gpointer data)
{
   GuiSetSpiralWidget(Closure->readAdaptiveSpiral, widget);

   update_geometry(widget);
   redraw_labels(cr, widget, ~0);
   GuiDrawSpiral(cr, Closure->readAdaptiveSpiral);

   return TRUE;
}

/*
 * Clip the spiral.
 */

static gboolean clip_idle_func(gpointer data)
{  Spiral *spiral = Closure->readAdaptiveSpiral;
   int i;

   if(spiral->segmentClipping < spiral->segmentCount)
   {  int clipping = spiral->segmentClipping;

      spiral->segmentClipping = spiral->segmentCount;
   
      for(i=clipping; i < spiral->segmentCount; i++)
	GuiSetSpiralSegmentColor(spiral, &transparent, &transparent, i);

      spiral->segmentClipping = clipping;

      /* Now redraw the last turn */

      for(i=ADAPTIVE_READ_SPIRAL_SIZE-300; i<=clipping; i++)
	GuiSetSpiralSegmentColor(spiral, &transparent, 0, i);
   }   

   return FALSE;
}

void GuiClipReadAdaptiveSpiral(int segments)
{
   Closure->readAdaptiveSpiral->segmentClipping = segments;

   g_idle_add(clip_idle_func, NULL);
}

/*
 * Change the segment color.
 * We can't ask for a redraw when not an the main thread, but can set an idle
 * function to do so.
 */

static gboolean segment_idle_func(gpointer data)
{
   gtk_widget_queue_draw(Closure->readAdaptiveDrawingArea);
   return FALSE;
}

void GuiChangeSegmentColor(GdkRGBA *color, int segment)
{  
   Closure->readAdaptiveSpiral->segmentColor[segment] = color;
   g_idle_add(segment_idle_func, 0);
}

/*
 * Remove the white markers drawn during the fill operation
 */

static gboolean remove_fill_idle_func(gpointer data)
{  Spiral *spiral = Closure->readAdaptiveSpiral;
   int i;

   for(i=0; i<spiral->segmentCount; i++)
      if(spiral->segmentColor[i] == Closure->whiteSector)
         GuiSetSpiralSegmentColor(spiral, &transparent, 0, i);

   return FALSE;
}

void GuiRemoveFillMarkers()
{
   g_idle_add(remove_fill_idle_func, NULL);
}

/***
 *** Redraw the label in our window
 ***/

static gboolean label_redraw_idle_func(gpointer data)
{
   gtk_widget_queue_draw(Closure->readAdaptiveDrawingArea);

   return FALSE;
}

void GuiSetAdaptiveReadSubtitle(char *title)
{
   if(!Closure->guiMode)
     return;
  
   if(Closure->readAdaptiveSubtitle)
     g_free(Closure->readAdaptiveSubtitle);

   Closure->readAdaptiveSubtitle = g_strdup(title);

   g_idle_add(label_redraw_idle_func, GINT_TO_POINTER(REDRAW_SUBTITLE));
}

void GuiSetAdaptiveReadFootline(char *msg, GdkRGBA *color)
{
   if(!Closure->guiMode)
     return;

   if(Closure->readAdaptiveErrorMsg)
     g_free(Closure->readAdaptiveErrorMsg);

   Closure->readAdaptiveErrorMsg = g_strdup(msg);
   footer_color = color;

   g_idle_add(label_redraw_idle_func, GINT_TO_POINTER(REDRAW_ERRORMSG));
}

void GuiUpdateAdaptiveResults(gint64 r, gint64 c, gint64 m, int p)
{  readable = r;
   correctable = c;
   missing = m;
   percent = p;

   if(!Closure->guiMode)
     return;
   
   g_idle_remove_by_data(GINT_TO_POINTER(REDRAW_PROGRESS));
   g_idle_add(label_redraw_idle_func, GINT_TO_POINTER(REDRAW_PROGRESS));
}   

/***
 *** Reset the notebook contents for new read action
 ***/

void GuiResetAdaptiveReadWindow()
{  GuiFillSpiral(Closure->readAdaptiveSpiral, &transparent);
   //   DrawSpiral(Closure->readAdaptiveSpiral);

   if(Closure->readAdaptiveSubtitle)
     g_free(Closure->readAdaptiveSubtitle);

   if(Closure->readAdaptiveErrorMsg)
     g_free(Closure->readAdaptiveErrorMsg);

   Closure->readAdaptiveSubtitle = NULL;
   Closure->readAdaptiveErrorMsg = NULL;

   readable = correctable = missing = 0;
   percent = min_required = 0;

   if(gtk_widget_get_window(Closure->readAdaptiveDrawingArea))
   {  static GdkRectangle rect;
      GtkAllocation a = {0};
      gtk_widget_get_allocation(Closure->readAdaptiveDrawingArea, &a);

      rect.x = rect.y = 0;
      rect.width  = a.width;
      rect.height = a.height;

      gtk_widget_queue_draw(Closure->readAdaptiveDrawingArea);
   }
}

/*
 * Set the minimum required data recovery value
 */

void GuiSetAdaptiveReadMinimumPercentage(int value)
{  min_required = value;
}

/***
 *** Create the notebook contents for the reading and scanning action
 ***/

void GuiCreateAdaptiveReadWindow(GtkWidget *parent)
{  GtkWidget *sep,*d_area;

   Closure->readAdaptiveHeadline = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(Closure->readAdaptiveHeadline), 0.0, 0.0); 
   gtk_misc_set_padding(GTK_MISC(Closure->readAdaptiveHeadline), 5, 0);
   gtk_label_set_ellipsize(GTK_LABEL(Closure->readAdaptiveHeadline), PANGO_ELLIPSIZE_END);
   gtk_box_pack_start(GTK_BOX(parent), Closure->readAdaptiveHeadline, FALSE, FALSE, 3);

   sep = gtk_hseparator_new();
   gtk_box_pack_start(GTK_BOX(parent), sep, FALSE, FALSE, 0);

   sep = gtk_hseparator_new();
   gtk_box_pack_start(GTK_BOX(parent), sep, FALSE, FALSE, 0);

   d_area = Closure->readAdaptiveDrawingArea = gtk_drawing_area_new();
   gtk_box_pack_start(GTK_BOX(parent), d_area, TRUE, TRUE, 0);
   g_signal_connect(G_OBJECT(d_area), "draw", G_CALLBACK(draw_cb), NULL);

   Closure->readAdaptiveSpiral
     = GuiCreateSpiral(&transparent, 10, 5, ADAPTIVE_READ_SPIRAL_SIZE);

   gtk_widget_set_size_request(d_area, -1, Closure->readAdaptiveSpiral->diameter);
}
#endif /* WITH_GUI_YES */
