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

/*** src type: some GUI code ***/

#include "dvdisaster.h"

int canprint(char c)
{
   return ((isascii(c) && !iscntrl(c)) ? 1 : 0);
}

#ifdef WITH_GUI_NO
static void log_window_vprintf(char *format, va_list argp) {}
static void log_window_append(char *text) {}
#else

/*
 * limit message output to the log window.
 */

#define MAX_LOG_WIN_SIZE 10240

static void clamp_gstring(GString *string)
{  gchar *ptr;
   int cut;

   if(string->len < MAX_LOG_WIN_SIZE)
      return;

   /* Remove head of the string so that it gets smaller than
      the maximum size, and cut off until the next newline */

   ptr = string->str;
   cut = string->len - MAX_LOG_WIN_SIZE;
   ptr += cut;

   while(*ptr && *ptr != '\n')
   {  ptr++;
      cut++;
   }

   g_string_erase(string, 0, cut);
}

static void log_window_vprintf(char *format, va_list argp)
{  char *tmp,*utf_tmp;

   tmp = g_strdup_vprintf(format, argp);
   utf_tmp = g_locale_to_utf8(tmp, -1, NULL, NULL, NULL);

   g_mutex_lock(Closure->logLock);
   g_string_append(Closure->logString, utf_tmp);
   clamp_gstring(Closure->logString);
   g_mutex_unlock(Closure->logLock);

   GuiUpdateLog();
   
   g_free(tmp);
   g_free(utf_tmp);
}

static void log_window_append(char *text)
{  char *utf_tmp = g_locale_to_utf8(text, -1, NULL, NULL, NULL);

   g_mutex_lock(Closure->logLock);
   g_string_append(Closure->logString, utf_tmp);
   clamp_gstring(Closure->logString);
   g_mutex_unlock(Closure->logLock);

   GuiUpdateLog();
   
   g_free(utf_tmp);
}
#endif /* WITH_GUI_YES */

/***
 *** gettext() convenience
 ***/

/*
 * The usual gettext() wrapper so that we can use strings
 * like "file!quit" in menus.
 */

char *sgettext(char *msgid)
{  char *msgval;

#ifdef WITH_NLS_YES
   msgval = gettext(msgid);
#else
   msgval = msgid;
#endif

   /*** If translating menu labels fails, try to make the default nicer. */
   
   if(msgval == msgid)  
   {  msgval = strrchr(msgid, '|');
      if(msgval) return msgval + 1;
      else       return msgid;
   }

   return msgval;
}

char *sgettext_utf8(char *msgid)
{  static gchar ringbuf[20][1024];
   static int ringptr;
   char *msgval;

#ifdef WITH_NLS_YES
   msgval = gettext(msgid);
#else
   msgval = msgid;
#endif

   /*** If translating menu labels fails, try to make the default nicer. */
   
   if(msgval == msgid)  
   {  msgval = strrchr(msgid, '|');
      if(msgval) return msgval + 1;
      else       return msgid;
   }

   /*** If we are running the GUI, convert to UTF8 for Gtk+ */

   if(Closure->guiMode)
   {  char *msg_utf8 = g_locale_to_utf8(msgval, -1, NULL, NULL, NULL);

      ringptr = (ringptr + 1) % 20;
      g_strlcpy(ringbuf[ringptr], msg_utf8, 1023);
      g_free(msg_utf8);
      
      return ringbuf[ringptr];
   }

   return msgval;
}

/***
 *** Format conversion
 ***/

/*
 * Convert between 8 bytes of unsigned char and gint64.
 * Needed since the EccHeader contains misaligned 64bit values
 * for historical reasons.
 * The uchar version uses a little endian representation.
 */

gint64 uchar_to_gint64(unsigned char *bytes)
{  gint64 out;

   memcpy(&out, bytes, sizeof(gint64));

#ifdef HAVE_BIG_ENDIAN
   return SwapBytes64(out);
#else
   return out;
#endif
}

void gint64_to_uchar(unsigned char *out, gint64 in)
{
#ifdef HAVE_BIG_ENDIAN
  in = SwapBytes64(in);
#endif

   memcpy(out, &in, sizeof(in));
}

/*
 * Calculate total number of sectors (including the last sector)
 * and the remaining bytes in the last sector for a given medium size.
 */

void CalcSectors(guint64 size, guint64 *sectors, int *in_last)
{
   *sectors = size/2048;
   *in_last = size & 0x7ff; 
   if(*in_last>0) (*sectors)++;
   else *in_last = 2048;
}

/***
 *** Message output
 ***/

/*
 * Output of the greetings is delayed until the first message is printed.
 */

static void print_greetings(FILE *where)
{  static int greetings_shown;
   
   if(greetings_shown) return;

   greetings_shown = 1;
   g_fprintf(where, "%s\n%s\n", Closure->versionString,
	     _("Copyright 2004-2017 Carsten Gnoerlich.\nCopyright 2019-2021 The dvdisaster development team."));
   /* TRANSLATORS: Excluding all kinds of warranty might be harmful under your
      legislature. If in doubt, just translate the following like "This is free
      software; please refer to the conditions of the GNU GENERAL PUBLIC LICENSE
      in the source code." Avoid making any legal statements by your own.*/
   g_fprintf(where, "%s",
	            _("This software comes with  ABSOLUTELY NO WARRANTY.  This\n"
		      "is free software and you are welcome to redistribute it\n"
		      "under the conditions of the GNU GENERAL PUBLIC LICENSE.\n"  
		      "See the file \"COPYING\" for further information.\n"));
}

/*
 * Print to stdout if run from the command line;
 * do nothing in GUI mode unless Closure->verbose is set.
 */

void PrintCLI(char *format, ...)
{  va_list argp;

   if(Closure->logFileEnabled)
   {  va_start(argp, format);
      VPrintLogFile(format, argp);
      va_end(argp);
   }

   if(Closure->guiMode)
   {  if(Closure->verbose)
      {  va_start(argp, format);
	 log_window_vprintf(format, argp);
	 va_end(argp);
      }
      return;
   }

   va_start(argp, format);
   g_vprintf(format, argp);
   va_end(argp);

   fflush(stdout);
}

/*
 * Print progress.
 * Returns cursor to first character in the line
 * if the message contains no newlines. 
 * Does nothing in GUI mode.
 */

void PrintProgress(char *format, ...)
{  char msg[256];
   va_list argp;
   int n;

   if(Closure->guiMode)
     return;
  
   print_greetings(stdout);

   if(Closure->noProgress)
     return;
  
   va_start(argp, format);
   g_vsnprintf(msg, 256, format, argp);
   n = g_utf8_strlen(msg,-1);
   va_end(argp);

   if(n>255) 
   {  n = 255;
      msg[255] = 0;
   }
   Closure->progressLength = n;

   if(strchr(msg, '\n'))
      g_printf("%s", msg);
   else
   {  g_mutex_lock(&Closure->progressLock);
      Closure->bs[n] = 0;
      g_printf("%s%s", msg, Closure->bs);
      Closure->bs[n] = '\b';
      g_mutex_unlock(&Closure->progressLock);
   }

   fflush(stdout);
}

/*
 * Clear last progress string
 */

void ClearProgress(void)
{  int n = Closure->progressLength;

   if(Closure->noProgress)
     return;
  
   g_mutex_lock(&Closure->progressLock);
   Closure->bs[n] = Closure->sp[n] = 0;
   g_printf("%s%s", Closure->sp, Closure->bs);
   Closure->bs[n] = '\b';
   Closure->sp[n] = ' ';
   g_mutex_unlock(&Closure->progressLock);
}

/*
 * Print a message to both stdout and the log window
 */

void PrintLog(char *format, ...)
{  va_list argp;

   if(Closure->logFileEnabled)
   {  va_start(argp, format);
      VPrintLogFile(format, argp);
      va_end(argp);
   }

   va_start(argp, format);

   if(Closure->guiMode)
      log_window_vprintf(format, argp);
   else 
   {
      print_greetings(stdout);
      g_vprintf(format, argp);

      fflush(stdout);
   }

   va_end(argp);
}

/*
 * Print a message to both stdout and the log window,
 * prepending each line with an asterisk and space.
 */

void PrintLogWithAsterisks(char *format, ...)
{  va_list argp;
   char *in, *out;
   char *new_format = alloca(4*strlen(format));

   /* annotate with asterisks */

   out=new_format;
   *out++='*';
   *out++=' ';

   for(in=format; *in; in++)
   {  *out++ = *in;

      if(*in == '\n' && *(in+1))
      {  *out++='*';
	 *out++=' ';
      }
   }
   *out=0;
   
   /* same logging as in PrintLog() */
   
   if(Closure->logFileEnabled)
   {  va_start(argp, format);
      VPrintLogFile(new_format, argp);
      va_end(argp);
   }

   va_start(argp, format);

   if(Closure->guiMode)
      log_window_vprintf(new_format, argp);
   else 
   {
      print_greetings(stdout);
      g_vprintf(new_format, argp);

      fflush(stdout);
   }

   va_end(argp);
}


/*
 * Same as PrintLog(), but does nothing unless Closure->verbose is true
 */

void Verbose(char *format, ...)
{  va_list argp;

   if(Closure->logFileEnabled)
   {  va_start(argp, format);
      VPrintLogFile(format, argp);
      va_end(argp);
   }

   if(!Closure->verbose)
    return;

   va_start(argp, format);

   if(Closure->guiMode)
      log_window_vprintf(format, argp);
   else 
   {
      print_greetings(stdout);
      g_vprintf(format, argp);

      fflush(stdout);
   }

   va_end(argp);
}

/*
 * Print timing results to console and log window 
 */

void PrintTimeToLog(GTimer *timer, char *format, ...)
{  va_list argp;
   gulong ignore;
   double elapsed = g_timer_elapsed(timer, &ignore); 
   double seconds = fmod(elapsed,60.0);
   int minutes = (int)fmod(elapsed / 60.0, 60.0);
   int hours = (int)(elapsed / 3600.0);
   char *tmp1,*tmp2;

   if(!Closure->verbose || Closure->fixedSpeedValues)
     return;

   va_start(argp, format);
   tmp1 = g_strdup_vprintf(format, argp);
   tmp2 = g_strdup_printf("%02d:%02d:%04.1f %s", hours, minutes, seconds, tmp1);
   va_end(argp);

   if(Closure->guiMode)
   { 
      log_window_append(tmp2);
   }
   else
   {  g_printf("%s", tmp2);

      fflush(stdout);
   }

   g_free(tmp1);
   g_free(tmp2);
}

/*
 * Print a message either to the console
 * or show it in the given label
 */

void PrintCLIorLabel(GtkWidget *label, char *format, ...)
{  va_list argp;

   if(Closure->logFileEnabled)
   {  va_start(argp, format);
      VPrintLogFile(format, argp);
      va_end(argp);
   }

   va_start(argp, format);

#ifdef WITH_GUI_YES   
   if(Closure->guiMode)
   {  char *c,*tmp;
	
      tmp = g_strdup_vprintf(format, argp);

      if(Closure->verbose)
	 log_window_append(tmp);

      c   = tmp + strlen(tmp) - 1; 
      while(*c == '\n')          /* Remove trailing newlines */
	*c-- = 0;

      GuiSetLabelText(label, "%s", tmp);  /* converts to utf8 by itself */
   
      g_free(tmp);
   }
   else
#endif
   {  g_vprintf(format, argp);

      fflush(stdout);
   }

   va_end(argp);
}

/*
 * Find out longest phrase in a list of translations
 */

int GetLongestTranslation(char *first, ...)
{  va_list argp;
   char *c;
   int length=g_utf8_strlen(first, -1);

   va_start(argp, first);
   while( (c=va_arg(argp, char*)) )
   {  int new_len = g_utf8_strlen(_(c), -1);
      if(new_len > length)
	 length = new_len;
   }

   va_end(argp);
   return length;
}

/*
 * Issue a warning message to stdout and the log
 */

void vLogWarning(char *format, va_list argp)
{  char *warn = _("Warning"); 
   int len = strlen(warn)+4;
   char prefix[len+1]; 
   char *c,*line;
   char *body;
   GString *str;

   body = g_strdup_vprintf(format, argp);

   line = body;
   memset(prefix, ' ', len);
   prefix[0] = '*';
   prefix[len] = 0;

   str = g_string_sized_new(256);
   g_string_append_printf(str,"* %s:%c", warn, Closure->guiMode ? '\n' : ' ');
   do
   {  c = strchr(line,'\n');
      if(c) *c=0;
      if(Closure->guiMode) g_string_append_printf(str,"* %s\n",line);
      else
      {  if(line != body)
	       g_string_append_printf(str,"%s%s\n",prefix,line);
         else  g_string_append_printf(str,"%s\n",line);
      }
      if(c) line = c+1;
   } while(c && *line);

   if(Closure->logFileEnabled)
      PrintLogFile("%s", str->str);

   if(Closure->guiMode)
   {  log_window_append(str->str);
   }
   else
   {  print_greetings(stdout);
      g_printf("%s", str->str);
      fflush(stdout);
   }

   g_string_free(str, TRUE);
   g_free(body);
}

void LogWarning(char *format, ...)
{  va_list argp;

   va_start(argp, format);
   vLogWarning(format, argp);
   va_end(argp);
}

/*
 * Tell user that current action was aborted due to a serious error.
 *
 * There are three different circumstances under which this routine
 * may be called:
 * 
 * a) We're running in CLI mode
 *    -> clean up and terminate the program
 * b) We're running in GUI mode 
 *  -> spawn the dialogue
 *     - via idle function when we're called from a sub thread
 *     - directly when we're called from the main thread
 *     GuiShowMessage() takes care of this.  
 *  -> clean up and continue execution
 */

void Stop(char *format, ...)
{  va_list argp;

   /*** Show message depending on commandline / GUI mode  */ 

   if(Closure->logFileEnabled)
   {  va_start(argp, format);
      PrintLogFile(_("\n*\n* dvdisaster - can not continue:\n*\n"));
      VPrintLogFile(format, argp);
      va_end(argp);
   }

   /*** CLI mode */
   
   if(!Closure->guiMode) 
   {  print_greetings(stdout);
      g_printf("%s", _("\n*\n* dvdisaster - can not continue:\n*\n"));
      va_start(argp, format);
      g_vprintf(format, argp);
      va_end(argp);
      g_printf("\n\n");
      fflush(stdout);
   }

   /*** GUI mode */
#ifdef WITH_GUI_YES   
   else
   {  char *titled,*msg,*utf_msg;
      int idx;

      va_start(argp, format);
      msg = g_strdup_vprintf(format, argp);
      va_end(argp);

      if(Closure->errorTitle)
      {
	 titled = g_strdup_printf("<b>%s</b>\n\n%s", Closure->errorTitle,msg);
	 g_free(msg);
	 msg = titled;
      }

      idx = strlen(msg);  /* Remove trailing newline */
      if(msg[idx-1] == '\n')
	msg[idx-1] = 0;

      utf_msg = g_locale_to_utf8(msg, -1, NULL, NULL, NULL);

      GuiShowMessage(Closure->window, utf_msg, GTK_MESSAGE_ERROR);
      g_free(msg);
      g_free(utf_msg);
   }
#endif /* WITH_GUI_YES */
   
   /* The cleanup procedure is supposed to terminate any running
      threads except for the main thread.
      Since we are running from a "side" thread in GUI mode,
      this will usually not return. */

#if 0
   if(Closure->mainThread)
   {  GThread *me = g_thread_self();

      if(me != Closure->mainThread)
	   printf("Stop() called from sub thread\n");
      else printf("Stop() called from main() thread\n");
   }
   else printf("Stop(): Closure->mainThread not set\n");
#endif

   if(Closure->cleanupProc)
     Closure->cleanupProc(Closure->cleanupData);

   /* Safety check; this indicates broken code.
      Concurrent threads should have been terminated by the
      cleanup above. */
   
   if(Closure->mainThread)
   {  GThread *me = g_thread_self();

      if(me != Closure->mainThread)
	printf("*\n* Warning: unterminated sub thread in Stop()\n*\n");
   }

   /* see above: possibly unreachable in GUI mode! */

   if(!Closure->guiMode)
   {    FreeClosure();
        exit(EXIT_FAILURE);
   }
}

/*
 * Provide some means of cleaning up 
 * when the current threads are terminated during a Stop() call.
 * Does not currently work if there are multiple threads besides the main thread.
 */

void RegisterCleanup(char *error_title, void (*cleanup)(gpointer), gpointer data)
{  
   Closure->cleanupProc = cleanup;
   Closure->cleanupData = data;
   Closure->subThread = g_thread_self();
   if(Closure->errorTitle) g_free(Closure->errorTitle);
   Closure->errorTitle  = g_strdup(error_title);
}

void UnregisterCleanup()
{  Closure->cleanupProc = NULL;
   Closure->subThread = NULL;
}

/***
 *** Thread utilities
 ***/

/*
 * Thread creation.
 * GNU/Linux seems to provide at least 2MiB of stack size per thread,
 * but FreeBSD has insanely small defaults. If this turns out to be too low,
 * we need the currently commented out alternative.
 */

GThread* CreateGThread(GThreadFunc f, gpointer data)
{  GError *err = NULL;
   GThread *t;

   t = g_thread_try_new("generic thread", f, data, &err);

   if(!t) Stop("Could not create thread: %s\n",err->message);

   return t;
}


/*
 * --strip method and associated cleanup func
 */

static void stripecc_cleanup(gpointer data)
{
   Image *image = (Image*)data;

   UnregisterCleanup();

   if (image)
      CloseImage(image);

   if(Closure->guiMode)
      GuiAllowActions(TRUE);

   GuiExitWorkerThread();
}


void StripECCFromImageFile()
{  Image *image = NULL;
   gint64 end;

   RegisterCleanup(_("Strip ECC aborted"), stripecc_cleanup, (gpointer)image);

   /*** Open the image file */
   image = OpenImageFromFile(Closure->imageName, O_RDWR, IMG_PERMS);
   if(!image)
     Stop(_("Can't open %s:\n%s"), Closure->imageName, strerror(errno));

   if (!image->eccHeader)
     Stop(_("Image is not augmented (no dvdisaster signature found)."));

   PrintLog("Image is augmented (expected sectors = %" PRId64 ")\n", image->expectedSectors);

   end = uchar_to_gint64(image->eccHeader->sectors);
   if (end <= 0)
     Stop(_("Invalid end data sector (%" PRId64 "), aborting"), end);

   PrintLog(_("Truncating image to %" PRId64 " sectors.\n"), end);

   /*** Last chance to cancel in GUI mode */

#ifdef WITH_GUI_YES
   if(Closure->guiMode)
   { int answer = GuiModalDialog(GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL, NULL,
			_("We're about to truncate the image from %" PRId64 " sectors (%" PRId64 " MiB)\n"
			 "to %" PRId64 " sectors (%" PRId64 " MiB), removing any dvdisaster-added ECC data.\n"
			 "This will restore the image to its pre-augmented original size."),
			 image->expectedSectors, image->expectedSectors >> 9, end, end >> 9);
			 /* >> 9 is 2048 bytes (1 sector) to MiB */

     if (answer != 1)
        Stop(_("Aborted on user request"));
   }
#endif

   /*** Truncate it. */

   if(!LargeTruncate(image->file, (gint64)(2048*end)))
     Stop(_("Could not truncate %s: %s\n"),Closure->imageName,strerror(errno));

   PrintLog(_("Image successfully truncated back to its original size.\n"));

#ifdef WITH_GUI_YES
   if (Closure->guiMode)
   {  GuiModalDialog(GTK_MESSAGE_INFO, GTK_BUTTONS_OK, NULL, "%s", _("Image successfully truncated"));
   }
#endif

   /*** Clean up */

   stripecc_cleanup((gpointer)image);
}
