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

#include "rs03-includes.h"

/***
 *** Method registration
 ***/

static void destroy(Method*);


void register_rs03(void)
{  Method *method = g_malloc0(sizeof(Method));

   /*** Standard infomation and methods */ 

   strncpy(method->name, "RS03", 4);
   method->menuEntry = g_strdup(_("Multithreaded RS codec (RS03)"));
   method->description = g_strdup(_("Multithreaded Reed-Solomon codec for error correction files and augmented images"));
   method->create  = RS03Create;
   method->fix     = RS03Fix;
   method->verify  = RS03Verify;

   /*** Linkage to rs03-recognize.c */

   method->recognizeEccFile = RS03RecognizeFile;
   method->recognizeEccImage = RS03RecognizeImage;

   /*** Linkage to rs03-window.c */

   method->createCreateWindow = CreateRS03EncWindow;
   method->createFixWindow    = CreateRS03FixWindow;

   method->resetCreateWindow = ResetRS03EncWindow;
   method->resetFixWindow    = ResetRS03FixWindow;

   method->createPrefsPage   = CreateRS03PrefsPage;
   method->resetPrefsPage    = ResetRS03PrefsPage;
   method->readPreferences   = ReadRS03Preferences;

   /*** Linkage to rs03-verify.c */

   method->createVerifyWindow = CreateRS03VerifyWindow;
   method->resetVerifyWindow  = ResetRS03VerifyWindow;

   /*** Register ourself */

   method->destroy = destroy;

   RegisterMethod(method);
}

static void destroy(Method *method)
{  RS03Widgets *wl = (RS03Widgets*)method->widgetList;

   if(wl)
   {  if(wl->fixCurve) FreeCurve(wl->fixCurve);

      if(wl->cmpSpiral)
	FreeSpiral(wl->cmpSpiral);

      if(wl->cmpLayout)
	g_object_unref(wl->cmpLayout);

      g_free(wl);
   }
}

