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

Spiral* GuiCreateSpiral(GdkColor *outline, GdkColor *fill, 
			int start_radius, int segment_size, int n_segments)
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
   spiral->segmentColor = g_malloc(n_segments * sizeof(GdkColor*));
   spiral->outline      = outline;
   spiral->cursorPos    = -1;

   for(i=0; i<n_segments; i++)
   { 
     spiral->segmentPos[i] = a; 
     spiral->segmentColor[i] = fill;

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
   g_free(spiral);
}

/*
 * Fill spiral segments with given color
 */

void GuiFillSpiral(Spiral *spiral, GdkColor *color)
{  int i;

   if(spiral)
     for(i=0; i<spiral->segmentCount; i++)
       spiral->segmentColor[i] = color;
}

/*
 * Draw the whole spiral
 */

void GuiDrawSpiral(Spiral *spiral)
{  cairo_t *cr;
   double a;
   double xi0,yi0,xo0,yo0;
   double scale_i,scale_o;
   int i;

   if(!spiral->widget) return;
   cr = gdk_cairo_create(gtk_widget_get_window(spiral->widget));
   cairo_set_line_width(cr, 1.0);

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

      cairo_move_to(cr, xi0, yi0);
      cairo_line_to(cr, xo0, yo0);
      cairo_line_to(cr, xo1, yo1);
      cairo_line_to(cr, xi1, yi1);
      cairo_close_path(cr);
      gdk_cairo_set_source_color(cr, spiral->segmentColor[i]);
      cairo_fill_preserve(cr);
      gdk_cairo_set_source_color(cr, spiral->outline);
      cairo_stroke(cr);

      xi0 = xi1; yi0 = yi1;
      xo0 = xo1; yo0 = yo1;
   }
}

/*
 * Draw just one segment of the spiral
 */

void GuiSetSpiralSegmentColor(Spiral *spiral, GdkColor *color, int segment)
{
   if (spiral->segmentColor[segment] != color)
   {  spiral->segmentColor[segment] = color;
      gtk_widget_queue_draw(spiral->widget);
   }
}

/*
 * Draw a label above or below the spiral
 */

void GuiDrawSpiralLabel(Spiral *spiral, PangoLayout *layout,
			char *text, GdkColor *color, int x, int line)
{  cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(spiral->widget));
   int w,h,y;

   GuiSetText(layout, text, &w, &h);
   if(line > 0) y = spiral->my + spiral->diameter / 2 + 20 + (line-1) * (10 + h); 
   else         y = spiral->my - spiral->diameter / 2 - 20 - h + (line+1) * (10 + h); 
   cairo_rectangle(cr, x + 0.5, y+(h-6)/2 + 0.5, 6, 6);
   gdk_cairo_set_source_color(cr, color);
   cairo_fill_preserve(cr);
   gdk_cairo_set_source_color(cr, Closure->grid);
   cairo_set_line_width(cr, 1.0);
   cairo_stroke(cr);

   cairo_move_to(cr, x+10, y);
   gdk_cairo_set_source_color(cr, Closure->foreground);
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
 * Change the spiral cursor
 */

void GuiChangeSpiralCursor(Spiral *spiral, int segment)
{
   if(!Closure->guiMode)
     return;
  
   if(segment != spiral->cursorPos)
      gtk_widget_queue_draw(spiral->widget);
}
#endif /* WITH_GUI_YES */
