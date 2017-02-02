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

#include "rs02-includes.h"

/***
 *** Method registration
 ***/

static void destroy(Method*);


void register_rs02(void)
{  Method *method = g_malloc0(sizeof(Method));

   method->ckSumClosure = g_malloc0(sizeof(RS02CksumClosure));

   /*** Standard infomation and methods */ 

   strncpy(method->name, "RS02", 4);
   method->menuEntry = g_strdup(_("Augmented image (RS02)"));
   method->description = g_strdup(_("Reed-Solomon method with improved tolerance for defective ecc data"));
   method->create  = RS02Create;
   method->fix     = RS02Fix;
   method->verify  = RS02Verify;

   /*** Linkage to rs02-common.c */

   method->recognizeEccImage = RS02Recognize;
   method->getCrcBuf         = RS02GetCrcBuf;
   method->resetCksums       = RS02ResetCksums;
   method->updateCksums      = RS02UpdateCksums;
   method->finalizeCksums    = RS02FinalizeCksums;
   method->expectedImageSize = RS02ExpectedImageSize;

   /*** Linkage to rs02-window.c */

   method->createCreateWindow = CreateRS02EncWindow;
   method->createFixWindow    = CreateRS02FixWindow;

   method->resetCreateWindow = ResetRS02EncWindow;
   method->resetFixWindow    = ResetRS02FixWindow;

   method->createPrefsPage   = CreateRS02PrefsPage;
   method->resetPrefsPage    = ResetRS02PrefsPage;
   method->readPreferences   = ReadRS02Preferences;

   /*** Linkage to rs02-verify.c */

   method->createVerifyWindow = CreateRS02VerifyWindow;
   method->resetVerifyWindow  = ResetRS02VerifyWindow;

   /*** Register ourself */

   method->destroy = destroy;

   RegisterMethod(method);
}

static void destroy(Method *method)
{  RS02Widgets *wl = (RS02Widgets*)method->widgetList;
   RS02CksumClosure *csc = (RS02CksumClosure*)method->ckSumClosure;

   if(csc->lay)
      g_free(csc->lay);
   g_free(method->ckSumClosure);

   if(wl)
   {  if(wl->fixCurve) FreeCurve(wl->fixCurve);

      if(wl->cmpSpiral)
	FreeSpiral(wl->cmpSpiral);

      if(wl->cmpLayout)
	g_object_unref(wl->cmpLayout);

      g_free(wl);
   }
}

