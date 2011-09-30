/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2004-2010 Carsten Gnoerlich.
 *  Project home page: http://www.dvdisaster.com
 *  Email: carsten@dvdisaster.com  -or-  cgnoerlich@fsfe.org
 *
 *  The Reed-Solomon error correction draws a lot of inspiration - and even code -
 *  from Phil Karn's excellent Reed-Solomon library: http://www.ka9q.net/code/fec/
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

#include "rs01-includes.h"

/***
 *** Method registration
 ***/

static void destroy(Method*);

void register_rs01(void)
{  Method *method = g_malloc0(sizeof(Method));

   /*** Standard infomation and methods */ 

   strncpy(method->name, "RS01", 4);
   method->menuEntry = g_strdup(_("Error correction file (RS01)"));
   method->description = g_strdup(_("Classic Reed-Solomon method based on polynomial arithmetic"));
   method->create  = RS01Create;
   method->fix     = RS01Fix;
   method->verify  = RS01Verify;

   /*** Linkage to rs01-window.c */

   method->createCreateWindow = CreateRS01EWindow;
   method->createFixWindow    = CreateRS01FWindow;

   method->resetCreateWindow = ResetRS01EncodeWindow;
   method->resetFixWindow    = ResetRS01FixWindow;

   method->createPrefsPage   = CreateRS01PrefsPage;
   method->resetPrefsPage    = ResetRS01PrefsPage;

   /*** Linkage to rs01-verify.c */

   method->createVerifyWindow = CreateRS01VerifyWindow;
   method->resetVerifyWindow  = ResetRS01VerifyWindow;

   /*** Register ourself */

   method->destroy = destroy;

   RegisterMethod(method);
}

static void destroy(Method *method)
{  RS01Widgets *wl = (RS01Widgets*)method->widgetList;

   if(wl)
   {  if(wl->fixCurve) FreeCurve(wl->fixCurve);

      if(wl->cmpSpiral)
	FreeSpiral(wl->cmpSpiral);

      if(wl->cmpLayout)
	g_object_unref(wl->cmpLayout);

      g_free(wl);
   }
}


