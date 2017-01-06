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

/***
 *** Archimede did not publish his source,
 *** so we have to write our own routines ;-)
 */

/*
 * Allocate and fill in the spiral data structure
 */

Spiral* CreateSpiral(GdkColor *outline, GdkColor *fill, 
		     int start_radius, int segment_size, int n_segments)
{  Spiral *spiral = g_malloc0(sizeof(Spiral));
   double a = 0.0;
   double scale_o = start_radius + segment_size;
   double ring_expand;
   int i;

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

void SetSpiralWidget(Spiral *spiral, GtkWidget *widget)
{  GtkAllocation *al = &widget->allocation;

   if(!spiral->drawable)
   {  spiral->drawable     = widget->window;
      spiral->mx           = al->width/2;
      spiral->my           = al->height/2;
   }
}   

void FreeSpiral(Spiral *spiral)
{  g_free(spiral->segmentPos);
   g_free(spiral->segmentColor);
   g_free(spiral);
}

/*
 * Fill spiral segments with given color
 */

void FillSpiral(Spiral *spiral, GdkColor *color)
{  int i;

   if(spiral)
     for(i=0; i<spiral->segmentCount; i++)
       spiral->segmentColor[i] = color;
}

/*
 * Draw the whole spiral
 */

void DrawSpiral(Spiral *spiral)
{  double a;
   int xi0,yi0,xo0,yo0;
   double scale_i,scale_o;
   int i;
   GdkPoint points[4];

   if(!spiral->drawable) return;

   scale_i = spiral->startRadius;
   scale_o = spiral->startRadius + spiral->segmentSize;
   xi0 = spiral->mx + spiral->startRadius;
   yi0 = yo0 = spiral->my;
   xo0 = xi0 + spiral->segmentSize;

   for(a=0.0, i=0; i<spiral->segmentClipping; i++)
   {  int xi1,yi1,xo1,yo1;
      double ring_expand = ((double)spiral->segmentSize * a) / (2.0*M_PI);

      a += atan((double)spiral->segmentSize / scale_o);

      scale_i = (double)spiral->startRadius + ring_expand;
      scale_o = scale_i + spiral->segmentSize;
      xi1 = spiral->mx + scale_i*cos(a);
      yi1 = spiral->my + scale_i*sin(a);
      xo1 = spiral->mx + scale_o*cos(a);
      yo1 = spiral->my + scale_o*sin(a);

      points[0].x = xi0; points[0].y = yi0;
      points[1].x = xo0; points[1].y = yo0;
      points[2].x = xo1; points[2].y = yo1;
      points[3].x = xi1; points[3].y = yi1;

      gdk_gc_set_rgb_fg_color(Closure->drawGC, spiral->segmentColor[i]);
      gdk_draw_polygon(spiral->drawable, Closure->drawGC, TRUE, points, 4);
      gdk_gc_set_rgb_fg_color(Closure->drawGC, spiral->outline);
      gdk_draw_polygon(spiral->drawable, Closure->drawGC, FALSE, points, 4);

      xi0 = xi1; yi0 = yi1;
      xo0 = xo1; yo0 = yo1;
   }
}

/*
 * Draw just one segment of the spiral
 */

void DrawSpiralSegment(Spiral *spiral, GdkColor *color, int segment)
{  double a;
   double scale_i,scale_o,ring_expand;
   GdkPoint points[4];

   if(segment<0 || segment>=spiral->segmentClipping)
     return;

   a = spiral->segmentPos[segment];

   ring_expand = ((double)spiral->segmentSize * a) / (2.0*M_PI);

   scale_i = (double)spiral->startRadius + ring_expand;
   scale_o = scale_i + spiral->segmentSize;
   points[0].x = spiral->mx + scale_i*cos(a);
   points[0].y = spiral->my + scale_i*sin(a);
   points[1].x = spiral->mx + scale_o*cos(a);
   points[1].y = spiral->my + scale_o*sin(a);

   a += atan((double)spiral->segmentSize / scale_o);

   ring_expand = ((double)spiral->segmentSize * a) / (2.0*M_PI);

   scale_i = (double)spiral->startRadius + ring_expand;
   scale_o = scale_i + spiral->segmentSize;
   points[3].x = spiral->mx + scale_i*cos(a);
   points[3].y = spiral->my + scale_i*sin(a);
   points[2].x = spiral->mx + scale_o*cos(a);
   points[2].y = spiral->my + scale_o*sin(a);

   spiral->segmentColor[segment] = color;
   gdk_gc_set_rgb_fg_color(Closure->drawGC, color);
   gdk_draw_polygon(spiral->drawable, Closure->drawGC, TRUE, points, 4);
   gdk_gc_set_rgb_fg_color(Closure->drawGC, spiral->outline);
   gdk_draw_polygon(spiral->drawable, Closure->drawGC, FALSE, points, 4);
}

/*
 * Draw a label above or below the spiral
 */

void DrawSpiralLabel(Spiral *spiral, PangoLayout *layout,
		     char *text, GdkColor *color, int x, int line)
{  GdkDrawable *d = spiral->drawable;
   int w,h,y;

   SetText(layout, text, &w, &h);
   if(line > 0) y = spiral->my + spiral->diameter / 2 + 20 + (line-1) * (10 + h); 
   else         y = spiral->my - spiral->diameter / 2 - 20 - h + (line+1) * (10 + h); 
   gdk_gc_set_rgb_fg_color(Closure->drawGC, color);
   gdk_draw_rectangle(d, Closure->drawGC, TRUE, x, y+(h-6)/2, 6, 6);
   gdk_gc_set_rgb_fg_color(Closure->drawGC, Closure->grid);
   gdk_draw_rectangle(d, Closure->drawGC, FALSE, x, y+(h-6)/2, 6, 6);
   gdk_gc_set_rgb_fg_color(Closure->drawGC, Closure->foreground);
   gdk_draw_layout(d, Closure->drawGC, x+10, y, layout);
}

/* 
 * Move the cursor (a highlighted segment) to a new position.
 * Moving to segment -1 means to disable the cursor.
 */

void MoveSpiralCursor(Spiral *spiral, int to_segment)
{
  if(to_segment == spiral->cursorPos)
    return;

  if(to_segment > spiral->segmentClipping)
    return;

  /* Erase old cursor */

  if(spiral->cursorPos >= 0)
    DrawSpiralSegment(spiral, spiral->colorUnderCursor, spiral->cursorPos);

  /* Moving to -1 means cursor off */

  spiral->cursorPos = to_segment;

  if(to_segment < 0)
    return;

  if(to_segment > spiral->segmentCount-1)
  {  spiral->cursorPos = -1;
     return;
  }

  /* Draw cursor at new place */

  spiral->colorUnderCursor = spiral->segmentColor[to_segment];
  DrawSpiralSegment(spiral, Closure->blueSector, to_segment);
}

/*
 * Wrapper for moving the spiral cursor from non-GUI thread
 */

typedef struct _cursor_info
{  Spiral *spiral;
   int segment;
} cursor_info;

static gboolean cursor_idle_func(gpointer data)
{  cursor_info *ci = (cursor_info*)data;

   MoveSpiralCursor(ci->spiral, ci->segment);
   g_free(ci);

   return FALSE;
}

void ChangeSpiralCursor(Spiral *spiral, int segment)
{  
   if(segment != spiral->cursorPos)
   {  cursor_info *ci = g_malloc(sizeof(cursor_info));

      ci->spiral  = spiral;
      ci->segment = segment;
      g_idle_add(cursor_idle_func, ci);
   }
}
