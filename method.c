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
 *** Collect the available methods
 ***/

/*
 * Invite all methods for registration 
 */

void CollectMethods(void)
{
  BindMethods();
}

/*
 * All methods register by calling this 
 */

void RegisterMethod(Method *method)
{
  g_ptr_array_add(Closure->methodList, method);
}

/*
 * List the available methods
 */

void ListMethods(void)
{  char name[5];
   unsigned int i;

   PrintCLI(_("\nList of available methods:\n\n"));
   name[4] = 0;
 
   for(i=0; i<Closure->methodList->len; i++)
   {  Method *method = g_ptr_array_index(Closure->methodList, i);

      strncpy(name, method->name, 4);
      PrintCLI("%s -- %s\n",name,method->description);
   }
}

/*
 * Call the method destructors
 */

void CallMethodDestructors(void)
{  unsigned int i;

   for(i=0; i<Closure->methodList->len; i++)  
   {  Method *method = g_ptr_array_index(Closure->methodList, i);
 
      method->destroy(method);
      if(method->menuEntry) g_free(method->menuEntry);
      if(method->description) g_free(method->description);
   }
}

/***
 *** Determine methods from name or ecc information
 ***/

/*
 * Find a method by name
 */

Method *FindMethod(char *name)
{  unsigned int i;

   for(i=0; i<Closure->methodList->len; i++) 
   {  Method *method = g_ptr_array_index(Closure->methodList, i);

      if(!strncmp(method->name, name, 4))
        return method;
   }

   return NULL;
}

