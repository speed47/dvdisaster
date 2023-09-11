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

#ifdef SYS_MINGW
#include "windows.h"
#include "shellapi.h"
#endif

#ifndef SYS_MINGW
static void send_errormsg(int fd, char *format, ...)
{  va_list argp;
   char *msg;
   int n;
   
   va_start(argp, format);
   msg = g_strdup_vprintf(format, argp);
   va_end(argp);

   n = strlen(msg);
   n = write(fd, msg, n);
   free(msg);
}

static int recv_errormsg(int fd, char **msg)
{  char buf[256];
   int n;
  
   n = read(fd, buf, 256);
   if(!n) return n;

   *msg = g_strdup(buf);
   
   return n;
}
#endif

void GuiShowURL(char *target)
{  guint64 ignore;
   int hyperlink = 0;
   char *path;

#ifndef SYS_MINGW
   pid_t pid;
   char *msg;
   int err_pipe[2]; /* child may send down err msgs to us here */
   int result;
#endif
   
   if(target && !strncmp(target, "http", 4))
   {  hyperlink = 1;
      path = g_strdup(target);
   }
     
   /* Process local files */
   
   if(!hyperlink)
   {  if(!Closure->docDir)
      {  
	 GuiCreateMessage(_("Documentation not installed."), GTK_MESSAGE_ERROR);
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
        GuiCreateMessage(_("Documentation file\n%s\nnot found.\n"), GTK_MESSAGE_ERROR, path);
        g_free(path);
        return;
     }
   }

#ifdef SYS_MINGW
   /* Okay, Billy wins big time here ;-) */

   ShellExecute(NULL, "open", path, NULL, NULL, SW_SHOWNORMAL);
#elif defined(SYS_DARWIN) 
    char command[256];
    snprintf(command, sizeof(command), "open \"%s\"", path);
    system(command);
#else

   /* fork xdg-open */

   result = pipe(err_pipe);
   if(result == -1)
   {  GuiCreateMessage(_("Could not create pipe before fork"), GTK_MESSAGE_ERROR);
      return;
   }
   result = fcntl(err_pipe[0], F_SETFL, O_CLOEXEC);
   if(result == -1)
   {  GuiCreateMessage(_("Could not set pipe flags before fork"), GTK_MESSAGE_ERROR);
      return;
   }
   result = fcntl(err_pipe[1], F_SETFL, O_CLOEXEC);
   if(result == -1)
   {  GuiCreateMessage(_("Could not set pipe flags before fork"), GTK_MESSAGE_ERROR);
      return;
   }
   pid = fork();

   if(pid == -1)
   {  close(err_pipe[0]);
      close(err_pipe[1]);
      GuiCreateMessage(_("Could not fork to start xdg-open"), GTK_MESSAGE_ERROR);
      return;
   }

   /* try calling the viewer */

   if(pid == 0)
   {  char *argv[10];
      int argc = 0;

      /* close reading end of error pipe */
      close(err_pipe[0]);

      /* prepare args and try to exec xdg-open */
      
      argv[argc++] = "xdg-open";
      argv[argc++] = path;
      argv[argc++] = NULL;
      execvp(argv[0], argv);

      /* if we reach this, telegraph our parent that sth f*cked up */

      send_errormsg(err_pipe[1],
		    _("execvp could not execute \"xdg-open\":\n%s\nIs xdg-open installed correctly?\n"),
		    strerror(errno));
      close(err_pipe[1]);
      _exit(110); /* couldn't execute */
   }

   /* Parent process. See if some error condition came down the pipe. */

   close(err_pipe[1]);
   result = recv_errormsg(err_pipe[0], &msg);
   close(err_pipe[0]);

   if(result)
   {  GuiCreateMessage("%s", GTK_MESSAGE_ERROR, msg);
   }
#endif
}

#endif
