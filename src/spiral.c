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

#ifdef WITH_GUI_YES
#include "dvdisaster.h"

/***
 *** Archimede did not publish his source,
 *** so we have to write our own routines ;-)
 */

/*
 * Allocate and fill in the spiral data structure
 */

Spiral* GuiCreateSpiral(GdkRGBA *fill, int start_radius, int segment_size, int n_segments)
{  Spiral *spiral;
   double a = 0.0;
   double scale_o = start_radius + segment_size;
   double ring_expand;
   int i;

   if(!Closure->guiMode)
     return NULL;
   
   spiral = g_malloc0(sizeof(Spiral));
   spiral->startRadius  = start_radius; 
   spiral->segmentSize  = segment_size;
   spiral->segmentCount = spiral->segmentClipping = n_segments;
   spiral->segmentPos   = g_malloc(n_segments * sizeof(double));
   spiral->segmentColor = g_malloc(n_segments * sizeof(GdkRGBA*));
   spiral->segmentOutline = g_malloc(n_segments * sizeof(GdkRGBA*));
   spiral->cursorPos    = -1;
   spiral->lastRenderedCursorPos = -1;

   for(i=0; i<n_segments; i++)
   { 
     spiral->segmentPos[i] = a; 
     spiral->segmentColor[i] = fill;
     spiral->segmentOutline[i] = 0; /* foreground */

     ring_expand =  ((double)segment_size * a) / (2.0*M_PI);
     a += atan((double)segment_size / scale_o);
     scale_o = (double)start_radius + ring_expand + (double)segment_size;
   }

   spiral->diameter = 2.0 * scale_o;

   return spiral;
}

void GuiSetSpiralWidget(Spiral *spiral, GtkWidget *widget)
{  GtkAllocation a = {0};
   gtk_widget_get_allocation(widget, &a);

   if(!spiral->widget)
   {  spiral->widget       = widget;
      spiral->mx           = a.width/2;
      spiral->my           = a.height/2;
   }
}   

void GuiFreeSpiral(Spiral *spiral)
{  if(!spiral) return;
  
   g_free(spiral->segmentPos);
   g_free(spiral->segmentColor);
   g_free(spiral->segmentOutline);
   g_free(spiral);
}

/*
 * Fill spiral segments with given color
 * Also resets the cursor (this function serves to reset the spiral)
 */

void GuiFillSpiral(Spiral *spiral, GdkRGBA *color)
{  int i;

   if(spiral)
     for(i=0; i<spiral->segmentCount; i++)
       spiral->segmentColor[i] = color;

   spiral->cursorPos    = -1;
}

/*
 * Draw the whole spiral
 */

void GuiDrawSpiral(cairo_t *cr, Spiral *spiral)
{  double a;
   double xi0,yi0,xo0,yo0;
   double scale_i,scale_o;
   int i;

   if(!spiral->widget) return;

   cairo_set_line_width(cr, 1.0);
   cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

   /* Get foreground and grid colors */

   GdkRGBA fg = {0};
   GtkStyleContext *context = gtk_widget_get_style_context(spiral->widget);
   gtk_style_context_get_color(context, gtk_widget_get_state_flags(spiral->widget), &fg);
   GdkRGBA outline_default = fg;
   outline_default.alpha   = 0.25;

   scale_i = spiral->startRadius;
   scale_o = spiral->startRadius + spiral->segmentSize;
   xi0 = spiral->mx + spiral->startRadius;
   yi0 = yo0 = spiral->my;
   xo0 = xi0 + spiral->segmentSize;

   for(a=0.0, i=0; i<spiral->segmentClipping; i++)
   {  double xi1,yi1,xo1,yo1;
      double ring_expand = ((double)spiral->segmentSize * a) / (2.0*M_PI);

      a += atan((double)spiral->segmentSize / scale_o);

      scale_i = (double)spiral->startRadius + ring_expand;
      scale_o = scale_i + spiral->segmentSize;
      xi1 = spiral->mx + scale_i*cos(a);
      yi1 = spiral->my + scale_i*sin(a);
      xo1 = spiral->mx + scale_o*cos(a);
      yo1 = spiral->my + scale_o*sin(a);


      GdkRGBA *outline = spiral->segmentOutline[i] ? spiral->segmentOutline[i] : &outline_default;

      cairo_move_to(cr, xi0, yi0);
      cairo_line_to(cr, xo0, yo0);
      cairo_line_to(cr, xo1, yo1);
      cairo_line_to(cr, xi1, yi1);
      cairo_close_path(cr);
      gdk_cairo_set_source_rgba(cr, spiral->segmentColor[i]);
      cairo_fill_preserve(cr);
      gdk_cairo_set_source_rgba(cr, outline);
      cairo_stroke(cr);

      xi0 = xi1; yi0 = yi1;
      xo0 = xo1; yo0 = yo1;
   }
}

/*
 * Draw just one segment of the spiral
 */

void GuiSetSpiralSegmentColor(Spiral *spiral, GdkRGBA *color, GdkRGBA *outline, int segment)
{
   if(segment<0 || segment>=spiral->segmentClipping)
     return;

   if (spiral->segmentColor[segment] != color || spiral->segmentOutline[segment] != outline)
   {  spiral->segmentColor[segment] = color;
      spiral->segmentOutline[segment] = outline;
      gtk_widget_queue_draw(spiral->widget);
   }
}

/*
 * Draw a label above or below the spiral
 */

void GuiDrawSpiralLabel(cairo_t *cr, Spiral *spiral, PangoLayout *layout,
			char *text, GdkRGBA *color, int x, int line)
{  int w,h,y;

   cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

   /* Get foreground and grid colors */

   GdkRGBA fg = {0};
   GtkStyleContext *context = gtk_widget_get_style_context(spiral->widget);
   gtk_style_context_get_color(context, gtk_widget_get_state_flags(spiral->widget), &fg);
   GdkRGBA outline = fg;
   outline.alpha   = 0.25;

   GuiSetText(layout, text, &w, &h);
   if(line > 0) y = spiral->my + spiral->diameter / 2 + 20 + (line-1) * (10 + h); 
   else         y = spiral->my - spiral->diameter / 2 - 20 - h + (line+1) * (10 + h); 
   cairo_rectangle(cr, x + 0.5, y+(h-6)/2 + 0.5, 6, 6);
   gdk_cairo_set_source_rgba(cr, color);
   cairo_fill_preserve(cr);
   gdk_cairo_set_source_rgba(cr, &outline);
   cairo_set_line_width(cr, 1.0);
   cairo_stroke(cr);

   cairo_move_to(cr, x+10, y);
   gdk_cairo_set_source_rgba(cr, &fg);
   pango_cairo_show_layout(cr, layout);
}

/* 
 * Move the cursor (a highlighted segment) to a new position.
 * Moving to segment -1 means to disable the cursor.
 */

void GuiMoveSpiralCursor(Spiral *spiral, int to_segment)
{
  if(!Closure->guiMode)
    return;
  
  if(to_segment == spiral->cursorPos)
    return;

  if(to_segment > spiral->segmentClipping)
    return;

  spiral->cursorPos = to_segment;

  if(to_segment > spiral->segmentCount-1)
  {  spiral->cursorPos = -1;
     return;
  }

  gtk_widget_queue_draw(spiral->widget);
}

/*
 * Wrapper for moving the spiral cursor from non-GUI thread
 */

static gboolean cursor_idle_func(gpointer data)
{  Spiral *spiral = (Spiral*)data;

   gint cursorPos = g_atomic_int_get(&spiral->cursorPos);
   if (spiral->lastRenderedCursorPos != cursorPos) {
      spiral->lastRenderedCursorPos = cursorPos;
      gtk_widget_queue_draw(spiral->widget);
   }

   return FALSE;
}
void GuiChangeSpiralCursor(Spiral *spiral, int segment)
{
   if(!Closure->guiMode)
     return;

   g_atomic_int_set(&spiral->cursorPos, segment);
   g_idle_add(cursor_idle_func, spiral);
}
#endif /* WITH_GUI_YES */
