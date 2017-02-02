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
#include <time.h>

/*
 * Create default dotfile path
 */

void DefaultLogFile()
{
   Closure->logFile = g_strdup_printf("%s/.dvdisaster.log", g_getenv("HOME"));
}

/*
 * Print time stamp to log file 
 */

void LogTimeStamp()
{  time_t now;

   if(!Closure->logFileEnabled)
      return;

   time(&now);
   PrintLogFile("*\n* %s\n* logging started at %s*\n",
		Closure->versionString, ctime(&now));
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
