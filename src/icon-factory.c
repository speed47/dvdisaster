/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2004-2017 Carsten Gnoerlich.
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
// DVDISASTER_GUI_FILE

#include "dvdisaster.h"

#include "inlined-icons.h"

/***
 *** Create our icon factory
 ***/

static GdkPixbuf* create_icon(GtkIconFactory *ifact, char *name, const guint8 *inline_data)
{   GdkPixbuf  *pb;
    GtkIconSet *iset;
    int width, height, rowstride;

    /* gdk_pixbuf_new_from_inline() deprecated; recommended to replace with GResource XML crap.
       One day I'll get rid of GTK+. I swear.

       pb = gdk_pixbuf_new_from_inline(-1, inline_data, FALSE, NULL);
    */    

    rowstride = (inline_data[12] << 24) + (inline_data[13] << 16) + (inline_data[14] << 8) + inline_data[15];
    width     = (inline_data[16] << 24) + (inline_data[17] << 16) + (inline_data[18] << 8) + inline_data[19];
    height    = (inline_data[20] << 24) + (inline_data[21] << 16) + (inline_data[22] << 8) + inline_data[23];
    
    pb = gdk_pixbuf_new_from_data(inline_data+24, GDK_COLORSPACE_RGB, TRUE, 8,
				  width, height, rowstride, NULL, NULL);
    
    iset = gtk_icon_set_new_from_pixbuf(pb);
    
    gtk_icon_factory_add(ifact, name, iset);
    return pb;
}

void CreateIconFactory()
{  GtkIconFactory *ifact;

   /*** Create and register our icon factory */ 

   ifact = gtk_icon_factory_new();
   gtk_icon_factory_add_default(ifact);

   /*** Our action icons */

   create_icon(ifact, "dvdisaster-open-ecc",   dvdisaster_open_ecc);
   create_icon(ifact, "dvdisaster-open-img",   dvdisaster_open_img);
   create_icon(ifact, "dvdisaster-cd",         dvdisaster_cd);

   create_icon(ifact, "dvdisaster-read",   dvdisaster_read);
   Closure->windowIcon = create_icon(ifact, "dvdisaster-create", dvdisaster_create);
   create_icon(ifact, "dvdisaster-scan",   dvdisaster_scan);
   create_icon(ifact, "dvdisaster-fix",    dvdisaster_fix);
   create_icon(ifact, "dvdisaster-verify", dvdisaster_verify);
   create_icon(ifact, "dvdisaster-strip",  dvdisaster_strip);

   /*** Stock GTK icons to defeat theming */

   create_icon(ifact, "dvdisaster-gtk-help", dvdisaster_gtk_help);
   create_icon(ifact, "dvdisaster-gtk-index", dvdisaster_gtk_index);
   create_icon(ifact, "dvdisaster-gtk-preferences", dvdisaster_gtk_preferences);
   create_icon(ifact, "dvdisaster-gtk-quit", dvdisaster_gtk_quit);
   create_icon(ifact, "dvdisaster-gtk-stop", dvdisaster_gtk_stop);
}
