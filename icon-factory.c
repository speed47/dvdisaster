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

#include "inlined-icons.h"

/***
 *** Create our icon factory
 ***/

static GdkPixbuf* create_icon(GtkIconFactory *ifact, char *name, const guint8 *inline_data)
{   GdkPixbuf  *pb   = gdk_pixbuf_new_from_inline(-1, inline_data, FALSE, NULL);
    GtkIconSet *iset = gtk_icon_set_new_from_pixbuf(pb);

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

   /*** Stock GTK icons to defeat theming */

   create_icon(ifact, "dvdisaster-gtk-help", dvdisaster_gtk_help);
   create_icon(ifact, "dvdisaster-gtk-index", dvdisaster_gtk_index);
   create_icon(ifact, "dvdisaster-gtk-preferences", dvdisaster_gtk_preferences);
   create_icon(ifact, "dvdisaster-gtk-quit", dvdisaster_gtk_quit);
   create_icon(ifact, "dvdisaster-gtk-stop", dvdisaster_gtk_stop);
}
