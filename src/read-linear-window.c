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

#include "read-linear.h"
#include "scsi-layer.h"

#define C2_CLAMP_VALUE 2352

/***
 *** Forward declarations
 ***/

static void redraw_curve(void);
static void update_geometry(void);

/***
 *** Routines for updating the GUI from the action thread.
 ***/

void GuiInitializeCurve(void *rc_ptr, int max_rate, int can_c2)
{  read_closure *rc = (read_closure*)rc_ptr;
   int i;

   if(!Closure->guiMode)
     return;

   Closure->readLinearCurve->maxY = max_rate;
   Closure->readLinearCurve->maxX = rc->image->dh->sectors/512;
   Closure->readLinearCurve->logMaxY = C2_CLAMP_VALUE;

   if(can_c2) Closure->readLinearCurve->enable = DRAW_FCURVE | DRAW_LCURVE;
   else       Closure->readLinearCurve->enable = DRAW_FCURVE;

   rc->lastCopied = (1000*rc->firstSector)/rc->image->dh->sectors;
   rc->lastPlotted = rc->lastSegment = rc->lastCopied;

   if(Closure->readLinearSpiral)
     for(i=rc->lastCopied-1; i>=0; i--)
     {  Closure->readLinearSpiral->segmentColor[i] = Closure->blueSector;
        Closure->readLinearCurve->ivalue[i] = 0;
     }
}

/*
 * Drawing the reading speed curve 
 */

typedef struct
{  read_closure *rc;
   int percent;
} curve_info;

static gboolean curve_idle_func(gpointer data)
{  curve_info *ci = (curve_info*)data;
   read_closure *rc=ci->rc;
   char *utf,buf[80];
   gint i;

   /*** Update the textual output */

   g_snprintf(buf, 80, _("Current Speed: %d.%dx"), 
	      (int)Closure->readLinearCurve->fvalue[ci->percent],
	      (int)(fmod(10*Closure->readLinearCurve->fvalue[ci->percent],10)));
   utf = g_locale_to_utf8(buf, -1, NULL, NULL, NULL);
   gtk_label_set_text(GTK_LABEL(Closure->readLinearSpeed), utf);
   g_free(utf);

   g_snprintf(buf, 80, _("Unreadable / skipped sectors: %" PRId64), Closure->readErrors);

   utf = g_locale_to_utf8(buf, -1, NULL, NULL, NULL);
   gtk_label_set_text(GTK_LABEL(Closure->readLinearErrors), utf);
   g_free(utf);

   /*** Update color of the changed spiral segments */

   for(i=rc->lastSegment; i<ci->percent; i++)
     switch(Closure->readLinearCurve->ivalue[i])
     {  case 0: GuiSetSpiralSegmentColor(Closure->readLinearSpiral, Closure->blueSector, i); break;
        case 1: GuiSetSpiralSegmentColor(Closure->readLinearSpiral, Closure->greenSector, i); break;
        case 2: GuiSetSpiralSegmentColor(Closure->readLinearSpiral, Closure->redSector, i); break;
        case 3: GuiSetSpiralSegmentColor(Closure->readLinearSpiral, Closure->darkSector, i); break;
        case 4: GuiSetSpiralSegmentColor(Closure->readLinearSpiral, Closure->yellowSector, i); break;
     }

   rc->lastSegment = ci->percent;

   /* Don't touch the curve 2nd or higher reading pass, of if there is no new data */

   if(rc->pass || rc->lastPlotted >= ci->percent)
   {  g_free(ci);
      g_mutex_lock(rc->rendererMutex);
      rc->activeRenderers--;
      g_mutex_unlock(rc->rendererMutex);
      return FALSE;
   }

   /*** Resize the Y axes if speed value exceeds current maximum */

   for(i=rc->lastPlotted+1; i<=ci->percent; i++)
     if(Closure->readLinearCurve->fvalue[i] > Closure->readLinearCurve->maxY)
        Closure->readLinearCurve->maxY = Closure->readLinearCurve->fvalue[i];

   /*** Schedule the curve for redrawing */

   rc->lastPlotted = ci->percent;
   gtk_widget_queue_draw(Closure->readLinearCurveArea);

   g_free(ci);
   g_mutex_lock(rc->rendererMutex);
   rc->activeRenderers--;
   g_mutex_unlock(rc->rendererMutex);
   return FALSE;
}

/*
 * Add one new data point
 */

void GuiAddCurveValues(void *rc_ptr, int percent, int color, int c2)
{  read_closure *rc = (read_closure*)rc_ptr;
   curve_info *ci;
   int i;

   if(!Closure->guiMode || percent < 0 || percent > 1000)
     return;

   ci = g_malloc(sizeof(curve_info));
   ci->rc = rc;
   ci->percent = percent;
   
   /*** Mark unused speed values between lastCopied and Percent */

   if(!rc->pass)
   {  int c2_clamped = c2 < C2_CLAMP_VALUE ? c2 : C2_CLAMP_VALUE;

      Closure->readLinearCurve->fvalue[percent] = rc->speed;
      Closure->readLinearCurve->lvalue[percent] = c2_clamped;

      for(i=rc->lastCopied+1; i<percent; i++)
      {	 Closure->readLinearCurve->fvalue[i] = rc->speed > 0.0 ? -1.0 : 0.0;
	 Closure->readLinearCurve->lvalue[i] = c2_clamped;
      }
   }

   /*** Mark the spiral segments between lastCopied and Percent*/

   /* lastCopied+1 ? */

   if(rc->lastCopied <= percent)
   {  for(i=rc->lastCopied; i<=percent; i++)
	 Closure->readLinearCurve->ivalue[i] = color;

      rc->lastCopied = percent;
   }

   g_mutex_lock(rc->rendererMutex);
   rc->activeRenderers++;
   g_mutex_unlock(rc->rendererMutex);
   g_idle_add(curve_idle_func, ci);
}

/*
 * Mark existing sectors with the dark green color.
 */

static gboolean curve_mark_idle_func(gpointer data)
{
   GuiDrawSpiral(Closure->readLinearSpiral);

   return FALSE;
}

void GuiMarkExistingSectors(void)
{  int i;
   int x;

   if(!Closure->guiMode)
     return;

   x = Closure->readLinearCurve->rightX + 20;
   
   Closure->additionalSpiralColor = 3;
   GuiDrawSpiralLabel(Closure->readLinearSpiral, Closure->readLinearCurve->layout,
		      _("Already present"), Closure->darkSector, x, -1);

   for(i=0; i<1000; i++)
      if(Closure->readLinearSpiral->segmentColor[i] == Closure->greenSector)
      {  Closure->readLinearSpiral->segmentColor[i] = Closure->darkSector;
	 Closure->readLinearCurve->ivalue[i] = 3;
      }

   g_idle_add(curve_mark_idle_func, NULL);
}

/*
 * Redraw the whole curve
 */

static void redraw_curve(void)
{
   GuiRedrawAxes(Closure->readLinearCurve);
   GuiRedrawCurve(Closure->readLinearCurve, 1000);
}

/* Calculate the geometry of the curve */

static void update_geometry(void)
{
   GuiUpdateCurveGeometry(Closure->readLinearCurve, "99x", 10);

   if(Closure->crcBuf && Closure->crcBuf->crcCached)
   {  int w,h;

      GuiSetText(Closure->readLinearCurve->layout, _("Sectors with CRC errors"), &w, &h);

      Closure->readLinearSpiral->my -= h;
   }

   /* Label positions in the foot line */

   gtk_box_set_child_packing(GTK_BOX(Closure->readLinearFootlineBox), Closure->readLinearSpeed, 
			     TRUE, TRUE, Closure->readLinearCurve->leftX, GTK_PACK_START);
   gtk_box_set_child_packing(GTK_BOX(Closure->readLinearFootlineBox), Closure->readLinearErrors, 
			     TRUE, TRUE, Closure->readLinearCurve->leftX, GTK_PACK_START);

}

static void redraw_spiral_labels(void)
{  GdkWindow *d = gtk_widget_get_window(Closure->readLinearSpiral->widget);
   int x,w,h;
   int pos = 1;

   /* Draw and label the spiral */

   x = 10;
   gdk_gc_set_rgb_fg_color(Closure->drawGC, Closure->curveColor);
   GuiSetText(Closure->readLinearCurve->layout, _("Medium state"), &w, &h);
   gdk_draw_layout(d, Closure->drawGC, 
		   x,
		   Closure->readLinearCurve->topY - h - 5, 
		   Closure->readLinearCurve->layout);

   if(Closure->additionalSpiralColor == 0)
     GuiDrawSpiralLabel(Closure->readLinearSpiral, Closure->readLinearCurve->layout,
			_("Not touched this time"), Closure->curveColor, x, -1);

   if(Closure->additionalSpiralColor == 3)
     GuiDrawSpiralLabel(Closure->readLinearSpiral, Closure->readLinearCurve->layout,
			_("Already present"), Closure->darkSector, x, -1);

   GuiDrawSpiralLabel(Closure->readLinearSpiral, Closure->readLinearCurve->layout,
		      _("Successfully read"), Closure->greenSector, x, pos++);

   if(Closure->crcBuf && Closure->crcBuf->crcCached)
     GuiDrawSpiralLabel(Closure->readLinearSpiral, Closure->readLinearCurve->layout,
			_("Sectors with CRC errors"), Closure->yellowSector, x, pos++);

   GuiDrawSpiralLabel(Closure->readLinearSpiral, Closure->readLinearCurve->layout,
		      _("Unreadable / skipped"), Closure->redSector, x, pos++);

   GuiDrawSpiral(Closure->readLinearSpiral);
}

static gboolean expose_curve_cb(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
   update_geometry();
   redraw_curve();

   return TRUE;
}

static gboolean expose_spiral_cb(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{  GtkAllocation a = {0};
   gtk_widget_get_allocation(widget, &a);

   GuiSetSpiralWidget(Closure->readLinearSpiral, widget);

   /* Override spiral center */
   Closure->readLinearSpiral->mx = a.width - 15 - Closure->readLinearSpiral->diameter / 2;

   redraw_spiral_labels();

   return TRUE;
}

/***
 *** Reset the notebook contents for new scan/read action
 ***/

void GuiResetLinearReadWindow()
{  
   gtk_notebook_set_current_page(GTK_NOTEBOOK(Closure->readLinearNotebook), 0);

   GuiZeroCurve(Closure->readLinearCurve);
   GuiFillSpiral(Closure->readLinearSpiral, Closure->background);
   GuiDrawSpiral(Closure->readLinearSpiral);
}

/*
 * Re-layout and redraw the read window while it is in use.
 * Required to add the information that CRC data is available,
 * since this happens when the the initial rendering of the window
 * contents have already been carried out.
 */

void GuiRedrawReadLinearWindow(void)
{  if(Closure->guiMode)
      gtk_widget_queue_draw(Closure->readLinearCurveArea);
}

/***
 *** Create the notebook contents for the reading and scanning action
 ***/

void GuiCreateLinearReadWindow(GtkWidget *parent)
{  GtkWidget *sep,*ignore,*curve,*spiral,*notebook,*hbox;

   Closure->readLinearHeadline = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(Closure->readLinearHeadline), 0.0, 0.0); 
   gtk_misc_set_padding(GTK_MISC(Closure->readLinearHeadline), 5, 0);
   gtk_label_set_ellipsize(GTK_LABEL(Closure->readLinearHeadline), PANGO_ELLIPSIZE_END);
   gtk_box_pack_start(GTK_BOX(parent), Closure->readLinearHeadline, FALSE, FALSE, 3);

   sep = gtk_hseparator_new();
   gtk_box_pack_start(GTK_BOX(parent), sep, FALSE, FALSE, 0);

   sep = gtk_hseparator_new();
   gtk_box_pack_start(GTK_BOX(parent), sep, FALSE, FALSE, 0);

   hbox = gtk_hbox_new(FALSE, 0);
   gtk_box_pack_start(GTK_BOX(parent), hbox, TRUE, TRUE, 0);

   curve = Closure->readLinearCurveArea = gtk_drawing_area_new();
   gtk_box_pack_start(GTK_BOX(hbox), curve, TRUE, TRUE, 0);
   g_signal_connect(G_OBJECT(curve), "expose_event", G_CALLBACK(expose_curve_cb), NULL);

   Closure->readLinearSpiral = GuiCreateSpiral(Closure->grid, Closure->background, 10, 5, 1000);
   spiral = gtk_drawing_area_new();
   gtk_widget_set_size_request(spiral, Closure->readLinearSpiral->diameter + 20, -1);
   gtk_box_pack_start(GTK_BOX(hbox), spiral, FALSE, FALSE, 0);
   g_signal_connect(G_OBJECT(spiral), "expose_event", G_CALLBACK(expose_spiral_cb), NULL);

   notebook = Closure->readLinearNotebook = gtk_notebook_new();
   gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
   gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);
   gtk_box_pack_end(GTK_BOX(parent), notebook, FALSE, FALSE, 0);

   hbox = Closure->readLinearFootlineBox = gtk_hbox_new(FALSE, 0);
   Closure->readLinearSpeed = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(Closure->readLinearSpeed), 0.0, 0.0); 
   gtk_box_pack_start(GTK_BOX(hbox), Closure->readLinearSpeed, FALSE, FALSE, 0);

   Closure->readLinearErrors = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(Closure->readLinearErrors), 1.0, 0.0); 
   gtk_box_pack_start(GTK_BOX(hbox), Closure->readLinearErrors, TRUE, TRUE, 0);

   ignore = gtk_label_new("progress_tab");
   gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hbox, ignore);

   Closure->readLinearFootline = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(Closure->readLinearFootline), 0.0, 0.5); 
   gtk_misc_set_padding(GTK_MISC(Closure->readLinearFootline), 5, 0);
   ignore = gtk_label_new("footer_tab");
   gtk_notebook_append_page(GTK_NOTEBOOK(notebook), Closure->readLinearFootline, ignore);

   Closure->readLinearCurve  = GuiCreateCurve(curve, _("Speed"), "%dx", 1000, CURVE_MEGABYTES);
   Closure->readLinearCurve->leftLogLabel = g_strdup(_("C2 errors"));
}
#endif /* WITH_GUI_YES */
