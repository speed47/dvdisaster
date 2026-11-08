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

/*** src type: no GUI code ***/

#include "dvdisaster.h"

/*
 * Create default dotfile path
 */

void DefaultLogFile()
{
#ifndef SYS_MINGW
   Closure->logFile = g_strdup_printf("%s/.dvdisaster.log", g_getenv("HOME"));
#else
   Closure->logFile = g_strdup_printf("%s/dvdisaster.log", Closure->homeDir); /* portable mode */
#endif
}

/*
 * Print time stamp to log file 
 */

void LogTimeStamp()
{  GDateTime *now;
   gchar *timestamp = NULL;

   if(!Closure->logFileEnabled)
      return;

   now = g_date_time_new_now_local();
   if(now)
   {  timestamp = g_date_time_format(now, "%a %b %e %H:%M:%S %Y");
      g_date_time_unref(now);
   }

   PrintLogFile("*\n* %s\n* logging started at %s\n*\n",
		Closure->versionString, timestamp ? timestamp : "unknown time");
   g_free(timestamp);
}


/* 
 * Print a message to the log file.
 * Tries hard to make the log messages survive a system crash.
 */

void VPrintLogFile(char *format, va_list argp)
{  FILE *file;

   if(!Closure->logFileStamped)
   {  Closure->logFileStamped = TRUE;
      LogTimeStamp();
   }

   /* Log output intentionally follows the user's umask. */
   // codeql[cpp/world-writable-file-creation]
   file = g_fopen(Closure->logFile, "a");
   if(!file)
     return;

   g_vfprintf(file, format, argp);
   fflush(file);
   fclose(file);
}

void PrintLogFile(char *format, ...)
{  va_list argp;

   va_start(argp, format);
   VPrintLogFile(format, argp);
   va_end(argp);
}
