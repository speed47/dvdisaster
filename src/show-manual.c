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

static const char *recv_errormsg(int fd)
{  static char buf[256];
   if (read(fd, buf, 256))
      return buf;
   return NULL;
}

/*
 * Clean up bundled environment variables before spawning an external process.
 * Both AppImage (Linux) and .app bundle (macOS) override env vars to point at
 * bundled libraries/resources.  Before exec-ing a host tool (xdg-open / open),
 * we restore the original values so the tool uses the host's own libraries.
 *
 * The launcher scripts save each original value as VARNAME_ORIGINAL before
 * overriding VARNAME.  We restore the original if it exists, otherwise we
 * simply unset VARNAME so the host default applies.
 */
static void cleanup_bundled_env(void)
{
   /* AppImage-specific variables (Linux / GTK3) */
   const char *appimage_vars[] = {
      "GDK_BACKEND",
      "GIO_EXTRA_MODULES",
      "GTK_IM_MODULE",
      "GTK_IM_MODULE_FILE",
      "GTK_PATH",
      "LD_LIBRARY_PATH",
      "NO_AT_BRIDGE",
      NULL,
   };

   /* Variables common to both AppImage and macOS .app bundle */
   const char *common_vars[] = {
      "GDK_PIXBUF_MODULE_FILE",
      "GSETTINGS_SCHEMA_DIR",
      "GTK_MODULES",
      "XDG_DATA_DIRS",
      NULL,
   };

   int is_appimage = g_getenv("DVDISASTER_APPIMAGE") && atoi(g_getenv("DVDISASTER_APPIMAGE"));
   int is_macos_app = g_getenv("DVDISASTER_MACOS_APP") && atoi(g_getenv("DVDISASTER_MACOS_APP"));

   if (!is_appimage && !is_macos_app)
      return;

   /* Restore common vars */
   for (int i = 0; common_vars[i]; i++) {
      gchar *original_name = g_strdup_printf("%s_ORIGINAL", common_vars[i]);
      if (g_getenv(original_name))
         g_setenv(common_vars[i], g_getenv(original_name), 1);
      else
         g_unsetenv(common_vars[i]);
      g_unsetenv(original_name);
      g_free(original_name);
   }

   /* Restore AppImage-only vars */
   if (is_appimage) {
      for (int i = 0; appimage_vars[i]; i++) {
         gchar *original_name = g_strdup_printf("%s_ORIGINAL", appimage_vars[i]);
         if (g_getenv(original_name))
            g_setenv(appimage_vars[i], g_getenv(original_name), 1);
         else
            g_unsetenv(appimage_vars[i]);
         g_unsetenv(original_name);
         g_free(original_name);
      }
   }

   g_unsetenv("DVDISASTER_APPIMAGE");
   g_unsetenv("DVDISASTER_MACOS_APP");
}
#endif

void GuiShowURL(char *target)
{  guint64 ignore;
   int hyperlink = 0;
   char *path;

#ifndef SYS_MINGW
   pid_t pid;
   const char *msg;
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
#else

   /* fork and exec the platform opener (macOS: open, Linux: xdg-open) */

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
#ifdef SYS_DARWIN
      GuiCreateMessage(_("Could not fork to start open"), GTK_MESSAGE_ERROR);
#else
      GuiCreateMessage(_("Could not fork to start xdg-open"), GTK_MESSAGE_ERROR);
#endif
      return;
   }

   /* try calling the viewer */

   if(pid == 0)
   {  char *argv[10];
      int argc = 0;

      /* close reading end of error pipe */
      close(err_pipe[0]);

      /* cleanup bundled env if we're called from AppImage or macOS .app */
      cleanup_bundled_env();

#ifdef SYS_DARWIN
      argv[argc++] = "open";
#else
      argv[argc++] = "xdg-open";
#endif
      argv[argc++] = path;
      argv[argc++] = NULL;
      execvp(argv[0], argv);

      /* if we reach this, telegraph our parent that sth f*cked up */

#ifdef SYS_DARWIN
      send_errormsg(err_pipe[1],
		    _("execvp could not execute \"open\":\n%s\n"),
		    strerror(errno));
#else
      send_errormsg(err_pipe[1],
		    _("execvp could not execute \"xdg-open\":\n%s\nIs xdg-open installed correctly?\n"),
		    strerror(errno));
#endif
      close(err_pipe[1]);
      _exit(110); /* couldn't execute */
   }

   /* Parent process. See if some error condition came down the pipe. */

   close(err_pipe[1]);
   msg = recv_errormsg(err_pipe[0]);
   close(err_pipe[0]);

   if(msg)
   {  GuiCreateMessage("%s", GTK_MESSAGE_ERROR, msg);
   }
#endif
}

#endif
