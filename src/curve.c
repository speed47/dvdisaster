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
 *** Constructors and destructors
 ***/

/*
 * Initialize the curve
 */

Curve* GuiCreateCurve(GtkWidget *widget, char *left_label, char *left_format, int n_values, int bottom_format)
{  Curve *curve = g_malloc0(sizeof(Curve));

   curve->widget     = widget;
   curve->layout     = gtk_widget_create_pango_layout(widget, NULL);
   curve->leftLabel  = g_strdup(left_label); 
   curve->leftFormat = g_strdup(left_format);
   curve->bottomFormat = bottom_format;   

   curve->fvalue       = g_malloc0(sizeof(gdouble)*(n_values+1));
   curve->ivalue       = g_malloc0(sizeof(gint)*(n_values+1));
   curve->lvalue       = g_malloc0(sizeof(gint)*(n_values+1));
   curve->lastValueIdx = n_values;

   curve->maxX      = 1;
   curve->maxY      = 1;
   curve->logMaxY   = 1;

   if(bottom_format != CURVE_PERCENT)
     curve->margin = 2;

   return curve;
} 

/*
 * Get rid of it
 */

void GuiFreeCurve(Curve *curve)
{  if(!curve) return;
  
   g_object_unref(curve->layout);
   g_free(curve->leftLabel);
   if(curve->leftLogLabel)
      g_free(curve->leftLogLabel);
   g_free(curve->leftFormat);
   g_free(curve->fvalue);
   g_free(curve->ivalue);
   g_free(curve->lvalue);
   g_free(curve);
}

/*
 * Reset the values
 */

void GuiZeroCurve(Curve *curve)
{  int i;

   if(curve)
     for(i=0; i<=curve->lastValueIdx; i++)
     {  curve->fvalue[i] = -1.0;
        curve->ivalue[i] = 0;
        curve->lvalue[i] = 0;
     }
}

/***
 *** Auxiliary functions
 ***/

/*
 * Calculate pixel coords from curve values
 */

int GuiCurveX(Curve *curve, gdouble x)
{  gdouble width = (curve->rightX - curve->leftX - curve->margin);

   return 1 + curve->leftX + ((gdouble)x * width) / 1000.0;
}

int GuiCurveLX(Curve *curve, gdouble x)
{  gdouble width = (curve->rightX - curve->leftX - curve->margin);

   return 1 + curve->leftX + (x * width) / (gdouble)curve->maxX;
}

int GuiCurveY(Curve *curve, gdouble y)
{  gdouble hfact;

   hfact =   (gdouble)(curve->bottomY - curve->topY) 
           / (gdouble)curve->maxY;

   return curve->bottomY - y * hfact;
}

int GuiCurveLogY(Curve *curve, gdouble y) /* not really a log */
{  gdouble hfact;

   if(y<1) return curve->bottomLY;

   hfact = (gdouble)(curve->bottomLY - curve->topLY);
 
   if(y==1) return curve->bottomLY - ((log(2)/log(curve->logMaxY)) * hfact)/2;
   else     return curve->bottomLY - (log(y)/log(curve->logMaxY)) * hfact;
}

/***
 *** Calculate the curve geometry
 ***/

void GuiUpdateCurveGeometry(Curve *curve, char *largest_left_label, int right_padding)
{  GtkAllocation a = {0};
   int w,h; 

   gtk_widget_get_allocation(curve->widget, &a);

   /* Top and bottom margins */

   GuiSetText(curve->layout, curve->leftLabel, &w, &h);
   curve->topY = h + 10;

   GuiSetText(curve->layout, "0123456789", &w, &h);
   curve->bottomY = a.height - h - 10;

   /* Left and right margins */

   GuiSetText(curve->layout, largest_left_label, &w, &h);
   curve->leftX   = 5 + 6 + 3 + w;
   curve->rightX  = a.width - right_padding;

   /* Add space for the lograithmic curve */

   if(curve->enable & DRAW_LCURVE)
   {  int height = curve->bottomY - curve->topY;

      curve->bottomLY = curve->bottomY;
      curve->bottomY -= height/4;
      curve->topLY = curve->bottomY + h + 15; 
   }
}

/***
 *** Redraw the coordinate axes
 ***/

void GuiRedrawAxes(cairo_t *cr, Curve *curve)
{  int i,w,h,x,y;
   int yg=0;
   int step;
   int bottom_y;

   cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
   cairo_set_line_width(cr, 1);

   /* Draw and label the grid lines for the log curve */

   if(curve->enable & DRAW_LCURVE)
   {  int val;
      char buf[16];

      gdk_cairo_set_source_rgba(cr, Closure->logColor);
      GuiSetText(curve->layout, curve->leftLogLabel, &w, &h);

      x = curve->leftX - w/2;
      if(x < 5) x = 5;
      cairo_move_to(cr, x, curve->topLY - h - 5);
      pango_cairo_show_layout(cr, curve->layout);

      
      for(val=400; val>3; val/=2)
      {  y = GuiCurveLogY(curve, val);
         sprintf(buf,"%d",val);
         GuiSetText(curve->layout, buf, &w, &h);
         gdk_cairo_set_source_rgba(cr, Closure->logColor);
         cairo_move_to(cr, curve->leftX-9-w, y-h/2);
         pango_cairo_show_layout(cr, curve->layout);
         gdk_cairo_set_source_rgba(cr, Closure->foreground);
         cairo_move_to(cr, curve->leftX-6 + 0.5, y + 0.5);
         cairo_line_to(cr, curve->leftX + 0.5, y + 0.5);
         cairo_stroke(cr);
         gdk_cairo_set_source_rgba(cr, Closure->grid);
         cairo_move_to(cr, curve->leftX + 0.5, y + 0.5);
         cairo_line_to(cr, curve->rightX + 0.5, y + 0.5);
         cairo_stroke(cr);

         val /=2;
         y = GuiCurveLogY(curve, val);
         gdk_cairo_set_source_rgba(cr, Closure->foreground);
         cairo_move_to(cr, curve->leftX-3 + 0.5, y + 0.5);
         cairo_line_to(cr, curve->leftX + 0.5, y + 0.5);
         cairo_stroke(cr);

         if(curve->bottomLY-curve->topLY > 8*h)
         {  sprintf(buf,"%d",val);
            GuiSetText(curve->layout, buf, &w, &h);
            gdk_cairo_set_source_rgba(cr, Closure->logColor);
            pango_cairo_show_layout(cr, curve->layout);
         }
      }
   }

   /* Draw and label the grid lines for the normal curve */

   if(curve->maxY > 20) step = 10;
   else if(curve->maxY > 10) step = 4;
   else if(curve->maxY > 4) step = 2;
   else step = 1;

   for(i=0; i<=curve->maxY; i+=step)
   {  char buf[4];
   
      g_snprintf(buf, 4, curve->leftFormat, i);
      GuiSetText(curve->layout, buf, &w, &h);

      y = yg = GuiCurveY(curve, i);
      gdk_cairo_set_source_rgba(cr, Closure->curveColor);
      cairo_move_to(cr, curve->leftX-9-w, y-h/2);
      pango_cairo_show_layout(cr, curve->layout);
      gdk_cairo_set_source_rgba(cr, Closure->foreground);
      cairo_move_to(cr, curve->leftX-6 + 0.5, y + 0.5);
      cairo_line_to(cr, curve->leftX + 0.5, y + 0.5);
      cairo_stroke(cr);

      gdk_cairo_set_source_rgba(cr, Closure->grid);
      cairo_move_to(cr, curve->leftX + 0.5, y + 0.5);
      cairo_line_to(cr, curve->rightX + 0.5, y + 0.5);
      cairo_stroke(cr);

      gdk_cairo_set_source_rgba(cr, Closure->foreground);
      y = GuiCurveY(curve, i+step/2);
      if(y >= curve->topY) {
         cairo_move_to(cr, curve->leftX-3 + 0.5, y + 0.5);
         cairo_line_to(cr, curve->leftX + 0.5, y + 0.5);
         cairo_stroke(cr);
      }
   }


   /* Draw and label the left coordinate axis */
   gdk_cairo_set_source_rgba(cr, Closure->foreground);

   cairo_move_to(cr, curve->leftX + 0.5, curve->topY + 0.5);
   cairo_line_to(cr, curve->leftX + 0.5, curve->bottomY + 0.5);
   cairo_stroke(cr);

   if(curve->enable & DRAW_LCURVE)
   {  cairo_move_to(cr, curve->leftX + 0.5, curve->topLY + 0.5);
      cairo_line_to(cr, curve->leftX + 0.5, curve->bottomLY + 0.5);
      cairo_stroke(cr);
   }

   gdk_cairo_set_source_rgba(cr, Closure->curveColor);
   GuiSetText(curve->layout, curve->leftLabel, &w, &h);
   x = curve->leftX - w/2;
   if(x < 5) x = 5;
   cairo_move_to(cr, x, curve->topY - h - 5);
   pango_cairo_show_layout(cr, curve->layout);

   /* Draw the right coordinate axis */

   gdk_cairo_set_source_rgba(cr, Closure->foreground);

   cairo_move_to(cr, curve->rightX + 0.5, curve->topY + 0.5);
   cairo_line_to(cr, curve->rightX + 0.5, curve->bottomY + 0.5);
   cairo_stroke(cr);

   if(curve->enable & DRAW_LCURVE) {
      cairo_move_to(cr, curve->rightX + 0.5, curve->topLY + 0.5);
      cairo_line_to(cr, curve->rightX + 0.5, curve->bottomLY + 0.5);
      cairo_stroke(cr);
   }

   /* Draw and label the bottom coordinate axis */

   gdk_cairo_set_source_rgba(cr, Closure->foreground);

   cairo_move_to(cr, curve->leftX + 0.5, curve->bottomY + 0.5);
   cairo_line_to(cr, curve->rightX + 0.5, curve->bottomY + 0.5);
   cairo_stroke(cr);

   if(curve->enable & DRAW_LCURVE)
   {  cairo_move_to(cr, curve->leftX + 0.5, curve->bottomLY + 0.5);
      cairo_line_to(cr, curve->rightX + 0.5, curve->bottomLY + 0.5);
      cairo_stroke(cr);
      bottom_y = curve->bottomLY;
   }
   else bottom_y = curve->bottomY;

   if(curve->maxX <= 100) step = 20;           /* <100M */
   else if(curve->maxX < 1000)  step = 100;    /* 100M ... 1000M */
   else if(curve->maxX < 8000)  step = 1024;   /* 1G .. 8G */
   else if(curve->maxX < 15000) step = 2560;   /* 8G .. 15G */
   else if(curve->maxX < 25000) step = 5120;   /* 15G .. 25G */
   else step = 10240;

   for(i=0; i<=curve->maxX; i+=step)
   {  char buf[10];
   
      switch(curve->bottomFormat)
      {  case CURVE_PERCENT:
 	   g_snprintf(buf, 10, "%d%%", i);
	   break;

         case CURVE_MEGABYTES:
	   if(step <= 100)
	        g_snprintf(buf, 10, "%dM",i);
	   else g_snprintf(buf, 10, "%3.1fG",(gdouble)i/1024.0);
	   break;
      }
      GuiSetText(curve->layout, buf, &w, &h);

      x = GuiCurveLX(curve,i)-1;
      cairo_move_to(cr, x + 0.5, bottom_y+6 + 0.5);
      cairo_line_to(cr, x + 0.5, bottom_y + 0.5);
      cairo_stroke(cr);
      cairo_move_to(cr, x-w/2, bottom_y+8);
      pango_cairo_show_layout(cr, curve->layout);

      if(i && x < curve->rightX)
      {  gdk_cairo_set_source_rgba(cr, Closure->grid);
         cairo_move_to(cr, x + 0.5, curve->bottomY-1 + 0.5);
         cairo_line_to(cr, x + 0.5, yg + 0.5);
         cairo_stroke(cr);

         if(curve->enable & DRAW_LCURVE) {
            cairo_move_to(cr, x + 0.5, curve->bottomLY-1 + 0.5);
            cairo_line_to(cr, x + 0.5, curve->topLY + 0.5);
            cairo_stroke(cr);
         }
      }

      gdk_cairo_set_source_rgba(cr, Closure->foreground);
      x = GuiCurveLX(curve,i+step/2)-1;
      if(x < curve->rightX) {
         cairo_move_to(cr, x + 0.5, bottom_y+3 + 0.5);
         cairo_line_to(cr, x + 0.5, bottom_y + 0.5);
         cairo_stroke(cr);
      }
   }
}

/*
 * Redraw the curve
 */

void GuiRedrawCurve(cairo_t *cr, Curve *curve, int last)
{  int i,x0,x1,fy0;

   gdk_cairo_set_source_rgba(cr, Closure->curveColor);
   cairo_set_line_width(cr, 1.0);

   /* Draw integer bar curve */

   if(curve->enable & DRAW_ICURVE)
   {  gdk_cairo_set_source_rgba(cr, Closure->barColor);
      x0 = GuiCurveX(curve, 0);
      for(i=1; i<=last; i++)
      {  x1 = GuiCurveX(curve, i);
         int iy = GuiCurveY(curve, curve->ivalue[i]);
         if(curve->ivalue[i] > 0)
         {  cairo_rectangle(cr, x0, iy, x0==x1 ? 1 : x1-x0, curve->bottomY-iy);
            cairo_fill(cr);
         }
         x0 = x1;
      }
   }

   /* Draw logarithmic integer curve */

   if(curve->enable & DRAW_LCURVE)
   {  x0 = GuiCurveX(curve, 0);
      for(i=1; i<=last; i++)
      {  gdk_cairo_set_source_rgba(cr, Closure->logColor);
         x1 = GuiCurveX(curve, i);
         int iy = GuiCurveLogY(curve, curve->lvalue[i]);

         if(curve->lvalue[i] > 0)
         {  cairo_rectangle(cr, x0, iy, x0==x1 ? 1 : x1-x0, curve->bottomLY-iy);
            cairo_fill(cr);
         }
         x0 = x1;
      }
   }

   /* Draw regular (floating point) curve */

   if(curve->enable & DRAW_FCURVE)
   {  x0 = GuiCurveX(curve, 0);
      fy0 = GuiCurveY(curve, curve->fvalue[0]);
      gdk_cairo_set_source_rgba(cr, Closure->curveColor);
      cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
      cairo_move_to(cr, x0, fy0);
      for(i=1; i<=last; i++)
      {  x1 = GuiCurveX(curve, i);
         if(x0 < x1 && curve->fvalue[i] >= 0)
         {  cairo_line_to(cr, x1, GuiCurveY(curve, curve->fvalue[i]));
            x0 = x1;
         }
      }
      cairo_stroke(cr);
   }
}
#endif /* WITH_GUI_YES */
