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
      if(method->lastEh)
	g_free(method->lastEh);
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

/*
 * Find method for a given ecc file (like in RS01)
 * or augmented image (like in the RS02 image format).
 * Since locating the header is expensive in the RS02 case,
 * it is cached in the corresponding Method struct.
 */

Method *EccMethod(int process_error)
{  LargeFile *ecc_file = NULL;
   LargeFile *image = NULL;

   /* First see if an ecc file is available */

   if((ecc_file = LargeOpen(Closure->eccName, O_RDONLY, 0)))
   {  int i;

      for(i=0; i<Closure->methodList->len; i++)  
      {  Method *method = g_ptr_array_index(Closure->methodList, i);

         if(   method->recognizeEccFile
	    && method->recognizeEccFile(method, ecc_file))
	 {  LargeClose(ecc_file);
	    return method;
	 }
      }

      LargeClose(ecc_file);
      if(process_error)
      {  if(Closure->guiMode)
	      CreateMessage(_("\nError correction file type unknown.\n"), GTK_MESSAGE_ERROR);
         else Stop(_("\nError correction file type unknown.\n"));
      }

      return NULL;
   }

   /* No ecc file, see if the image contains hidden ecc information */

   if((image = LargeOpen(Closure->imageName, O_RDONLY, 0)))
   {  int i;

      for(i=0; i<Closure->methodList->len; i++)  
      {  Method *method = g_ptr_array_index(Closure->methodList, i);
#if 0
      char buf[5];
      strncpy(buf,method->name,4);
      buf[4]=0;
      printf("trying %s\n", buf);
#endif
         if(   method->recognizeEccImage
	    && method->recognizeEccImage(method, image))
	 {  LargeClose(image);
	    return method;
	 }
      }

      LargeClose(image);
      if(process_error)
      {  if(Closure->guiMode)
	      CreateMessage(_("\nNo error correction data recognized in image.\n"), GTK_MESSAGE_ERROR);
         else Stop(_("\nNo error correction data recognized in image.\n"));
      }

      return NULL;
   }

   /* Neither ecc file nor augmented image */

   if(process_error)
   {  if(Closure->guiMode)
           CreateMessage(_("Image file %s not present.\n"), GTK_MESSAGE_ERROR, Closure->imageName, strerror(errno));
      else Stop(_("Image file %s not present.\n"), Closure->imageName, strerror(errno));
   }
   return NULL;
}

