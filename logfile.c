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
#include <time.h>

/*
 * Create default dotfile path
 */

void DefaultLogFile()
{

#ifndef SYS_MINGW
   Closure->logFile = g_strdup_printf("%s/.dvdisaster.log", g_getenv("HOME"));
#else
   if(Closure->appData)
        Closure->logFile = g_strdup_printf("%s\\logfile.txt", Closure->appData);
   else Closure->logFile = g_strdup_printf("%s\\logfile.txt", Closure->binDir);
#endif
}

/*
 * Print time stamp to log file 
 */

void LogTimeStamp()
{  time_t now;

   if(!Closure->logFileEnabled)
      return;

   time(&now);
   PrintLogFile("*\n* dvdisaster-%s logging started at %s*\n",
		Closure->cookedVersion, ctime(&now));
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

   file = fopen(Closure->logFile, "a");
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
