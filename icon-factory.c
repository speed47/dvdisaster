/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2004-2010 Carsten Gnoerlich.
 *  Project home page: http://www.dvdisaster.com
 *  Email: carsten@dvdisaster.com  -or-  cgnoerlich@fsfe.org
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA,
 *  or direct your browser at http://www.gnu.org.
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

   create_icon(ifact, "dvdisaster-read",   dvdisaster_read);
   Closure->windowIcon = create_icon(ifact, "dvdisaster-create", dvdisaster_create);
   create_icon(ifact, "dvdisaster-scan",   dvdisaster_scan);
   create_icon(ifact, "dvdisaster-fix",    dvdisaster_fix);
   create_icon(ifact, "dvdisaster-verify", dvdisaster_verify);
}
