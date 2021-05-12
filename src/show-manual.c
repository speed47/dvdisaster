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

#include "dvdisaster.h"

#ifdef SYS_MINGW
#include "windows.h"
#include "shellapi.h"
#endif

void ShowURL(char *target)
{  guint64 ignore;
   pid_t pid;
   int hyperlink = 0;
   char *path;
   
   if(target && !strncmp(target, "http", 4))
   {  hyperlink = 1;
      path = g_strdup(target);
   }
     
   /* Process local files */
   
   if(!hyperlink)
   {  if(!Closure->docDir)
      {  
	 CreateMessage(_("Documentation not installed."), GTK_MESSAGE_ERROR);
	 return;
      }

      /* If no target is given, show the manual. */

     if(!target) 
     {   path = g_strdup_printf("%s/manual.pdf",Closure->docDir); 
     }
     else 
        if(*target != '/') path = g_strdup_printf("%s/%s",Closure->docDir, target); 
        else               path = g_strdup(target); 

     if(!LargeStat(path, &ignore))
     {  
        CreateMessage(_("Documentation file\n%s\nnot found.\n"), GTK_MESSAGE_ERROR, path);
        g_free(path);
        return;
     }
   }

#ifdef SYS_MINGW
   /* Okay, Billy wins big time here ;-) */

   ShellExecute(NULL, "open", path, NULL, NULL, SW_SHOWNORMAL);
#else

   /* fork xdg-open */
   
   pid = fork();

   if(pid == -1)
   {  printf("fork failed\n");
      return;
   }

   /* try calling the viewer */

   if(pid == 0)
   {  char *argv[10];
      int argc = 0;

      argv[argc++] = "xdg-open";
      argv[argc++] = path;
      argv[argc++] = NULL;
      execvp(argv[0], argv);

      _exit(110); /* couldn't execute */
   }
#endif
}

