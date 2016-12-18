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
 * Append message to the log window.
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

   UpdateLog();
   
   g_free(tmp);
   g_free(utf_tmp);
}

static void log_window_append(char *text)
{  char *utf_tmp = g_locale_to_utf8(text, -1, NULL, NULL, NULL);

   g_mutex_lock(Closure->logLock);
   g_string_append(Closure->logString, utf_tmp);
   clamp_gstring(Closure->logString);
   g_mutex_unlock(Closure->logLock);

   UpdateLog();
   
   g_free(utf_tmp);
}

/*
 * Output of the greetings is delayed until the first message is printed.
 */

static void print_greetings(FILE *where)
{  static int greetings_shown;
   
   if(greetings_shown) return;

   greetings_shown = 1;
   g_fprintf(where, "%s\n%s.\n", Closure->versionString, _("Copyright 2004-2015 Carsten Gnoerlich"));
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
   g_vfprintf(stdout, format, argp);
   va_end(argp);

   fflush(stdout);
}

/*
 * Print progress to stderr.
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
  
   print_greetings(stderr);

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
      g_fprintf(stderr, "%s", msg);
   else
   {  g_mutex_lock(&Closure->progressLock);
      Closure->bs[n] = 0;
      g_fprintf(stderr, "%s%s", msg, Closure->bs);
      Closure->bs[n] = '\b';
      g_mutex_unlock(&Closure->progressLock);
   }

   fflush(stderr);
}

/*
 * Clear last progress string
 */

void ClearProgress(void)
{  int n = Closure->progressLength;

   g_mutex_lock(&Closure->progressLock);
   Closure->bs[n] = Closure->sp[n] = 0;
   g_fprintf(stderr, "%s%s", Closure->sp, Closure->bs);
   Closure->bs[n] = '\b';
   Closure->sp[n] = ' ';
   g_mutex_unlock(&Closure->progressLock);
}

/*
 * Print a message to both stderr and the log window
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
      print_greetings(stderr);
      g_vfprintf(stderr, format, argp);

      fflush(stderr);
   }

   va_end(argp);
}

/*
 * Print a message to both stderr and the log window,
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
      print_greetings(stderr);
      g_vfprintf(stderr, new_format, argp);

      fflush(stderr);
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
      print_greetings(stderr);
      g_vfprintf(stderr, format, argp);

      fflush(stderr);
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

   if(!Closure->verbose)
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
   {  g_fprintf(stderr, "%s", tmp2);

      fflush(stderr);
   }

   g_free(tmp1);
   g_free(tmp2);
}

/*
 * Print a message either to the console
 * or show it in the given label
 */

void PrintCLIorLabel(GtkLabel *label, char *format, ...)
{  va_list argp;

   if(Closure->logFileEnabled)
   {  va_start(argp, format);
      VPrintLogFile(format, argp);
      va_end(argp);
   }

   va_start(argp, format);
      
   if(Closure->guiMode)
   {  char *c,*tmp;
	
      tmp = g_strdup_vprintf(format, argp);

      if(Closure->verbose)
	 log_window_append(tmp);

      c   = tmp + strlen(tmp) - 1; 
      while(*c == '\n')          /* Remove trailing newlines */
	*c-- = 0;

      SetLabelText(label, tmp);  /* converts to utf8 by itself */
   
      g_free(tmp);
   }
   else
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
 * Issue a warning message to stderr and the log
 */

static void vlog_warning(char *format, va_list argp)
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
   {  print_greetings(stderr);
      g_fprintf(stderr, "%s", str->str);
      fflush(stderr);
   }

   g_string_free(str, TRUE);
   g_free(body);
}

void LogWarning(char *format, ...)
{  va_list argp;

   va_start(argp, format);
   vlog_warning(format, argp);
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
 *     ShowMessage() takes care of this.  
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
   {  g_fprintf(stderr, "%s", _("\n*\n* dvdisaster - can not continue:\n*\n"));
      va_start(argp, format);
      g_vfprintf(stderr, format, argp);
      va_end(argp);
      g_fprintf(stderr, "\n\n");
      fflush(stderr);
   }

   /*** GUI mode */
   
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

      ShowMessage(Closure->window, utf_msg, GTK_MESSAGE_ERROR);
      g_free(msg);
      g_free(utf_msg);
   }

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
 * Spawning of idle functions.
 * Idle functions are required to perform actions (like opening
 * a dialogue) from a sub thread.
 * However idle functions must not be spawned from the main thread
 * as it would block infinitely; in that case we must run the idle
 * function directly.
 */

void CallIdleFunc(gboolean (*idle_func)(gpointer), gpointer data)
{ 
   if(Closure->mainThread == g_thread_self())
   {  idle_func(data);
   }
   else
   {  g_idle_add(idle_func, data);
   }
}

/***
 *** Graphical user interface convenience
 ***/

/* 
 * Show the given widget
 */

static gboolean show_idle_func(gpointer data)
{  
   gtk_widget_show(GTK_WIDGET(data));

   return FALSE;
}

void ShowWidget(GtkWidget *widget)
{
  g_idle_add(show_idle_func, (gpointer)widget);
}

/*
 * Activation / Deactivation of the action buttons
 */

static gboolean allow_actions_idle_func(gpointer data)
{  gboolean s = (data != NULL);

   /* Disable/Enable parts of the menu */

   gtk_widget_set_sensitive(Closure->fileMenuImage, s);
   gtk_widget_set_sensitive(Closure->fileMenuEcc, s);
   gtk_widget_set_sensitive(Closure->toolMenuAnchor, s);

   /* Disable/Enable toolbar and sidebar buttons */

   if(Closure->deviceNodes->len) 
   {  gtk_widget_set_sensitive(Closure->readButton, s);
      gtk_widget_set_sensitive(Closure->scanButton, s);
   }
   gtk_widget_set_sensitive(Closure->createButton, s);
   gtk_widget_set_sensitive(Closure->fixButton, s);
   gtk_widget_set_sensitive(Closure->testButton, s);

   gtk_widget_set_sensitive(Closure->prefsButton, s);
   if(!s && Closure->prefsWindow)
     HidePreferences();

   Closure->stopActions = FALSE;

   return FALSE;
}

void AllowActions(gboolean s)
{
  g_idle_add(allow_actions_idle_func, GINT_TO_POINTER(s));
}

/*
 * Dispatch a non-modal message dialog
 */

typedef struct
{  char *msg;
   GtkMessageType type;
   GtkWindow *window;
} message_info;

static gboolean message_idle_func(gpointer data)
{  message_info *mi = (message_info*)data;
   GtkWidget *dialog;

   dialog = gtk_message_dialog_new_with_markup(mi->window,
					       GTK_DIALOG_DESTROY_WITH_PARENT,
					       mi->type,
					       GTK_BUTTONS_CLOSE,
					       mi->msg, NULL);

   gtk_label_set_line_wrap(GTK_LABEL(((struct _GtkMessageDialog*)dialog)->label), FALSE);
   g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);
   gtk_widget_show(dialog);

   g_free(mi->msg);
   g_free(mi);

   return FALSE;
}

void ShowMessage(GtkWindow *parent, char *msg, GtkMessageType type)
{  message_info *mi = g_malloc(sizeof(message_info));

   mi->msg    = g_strdup(msg);
   mi->type   = type;
   mi->window = parent;

   if(Closure->mainThread == g_thread_self())
        message_idle_func(mi);
   else g_idle_add(message_idle_func, mi);
}

/*
 * Creates a message from the main thread
 */

GtkWidget* CreateMessage(char *format, GtkMessageType type, ...)
{  GtkWidget *dialog;
   va_list argp;
   char *text,*utf8; 

   va_start(argp, type);
   text = g_strdup_vprintf(format, argp);
   va_end(argp);
   utf8 = g_locale_to_utf8(text, -1, NULL, NULL, NULL);

   dialog = gtk_message_dialog_new(Closure->window, 
				   GTK_DIALOG_DESTROY_WITH_PARENT,
				   type,
				   GTK_BUTTONS_CLOSE,
				   utf8, NULL);

   gtk_label_set_line_wrap(GTK_LABEL(((struct _GtkMessageDialog*)dialog)->label), FALSE);
   g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);
   gtk_widget_show(dialog);
   g_free(text);
   g_free(utf8);

   return dialog;
}

/*
 * Label convenience functions.
 * Sets the label text from another thread.
 */

typedef struct
{  GtkLabel *label;
   char *text;
} label_info;

static gboolean label_idle_func(gpointer data)
{  label_info *li = (label_info*)data;

   gtk_label_set_markup(li->label, li->text);

   g_free(li->text);
   g_free(li);

   return FALSE;
}

void SetLabelText(GtkLabel *label, char *format, ...)
{  label_info *li = g_malloc(sizeof(label_info));
   va_list argp;

   li->label = label;

   va_start(argp, format);
   if(format)
   {  char *tmp  = g_strdup_vprintf(format, argp);

      if(!tmp) tmp=g_strdup_printf("SetLabelText(%s) failed",format);
      li->text = g_locale_to_utf8(tmp, -1, NULL, NULL, NULL);
      g_free(tmp);
   }
   else li->text = g_locale_to_utf8("(null)", -1, NULL, NULL, NULL);
   va_end(argp);
   g_idle_add(label_idle_func, li);
}

/*
 * Progress bar convenience function.
 * Percentage is given as a multiple of 0.1 percent.
 */

typedef struct
{  GtkWidget *pbar;
   int percent;
   int max;
} progress_info;

static gboolean progress_idle_func(gpointer data)
{  progress_info *pi = (progress_info*)data;
   gdouble val = (gdouble)pi->percent / (gdouble)pi->max;
   char text[20];

   switch(pi->max)
   {  case  100: g_sprintf(text, "%3d%%",pi->percent); break;
      case 1000: g_sprintf(text, "%3d.%1d%%",pi->percent/10,pi->percent%10); break;
   }

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pi->pbar), val);
   gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pi->pbar), text);

   g_free(pi);

   return FALSE;
}

void SetProgress(GtkWidget *pbar, int percent, int max)
{  progress_info *pi = g_malloc(sizeof(progress_info));

   pi->pbar    = pbar;
   pi->percent = percent;
   pi->max     = max;

   g_idle_add(progress_idle_func, pi);
}

/*
 * Perform a modal dialog.
 * Note that the thread running the dialog is different
 * from the one blocking/waiting for the response!
 */

typedef struct
{  GMutex *mutex;
   GCond *cond;
   char *msg;
   int ret;
   GtkMessageType message_type;
   GtkButtonsType button_type;
   void (*button_fn)(GtkDialog*);
} modal_info;

static gboolean modal_idle_func(gpointer data)
{  modal_info *mi = (modal_info*)data;
   GtkWidget *dialog;
   int response;

   dialog = gtk_message_dialog_new(Closure->window,
				   GTK_DIALOG_DESTROY_WITH_PARENT,
				   mi->message_type,
				   mi->button_type,
				   "%s", mi->msg);
   gtk_label_set_line_wrap(GTK_LABEL(((struct _GtkMessageDialog*)dialog)->label), FALSE);

   if(mi->button_fn)
         mi->button_fn(GTK_DIALOG(dialog));
   else  ReverseCancelOK(GTK_DIALOG(dialog));

   response = gtk_dialog_run(GTK_DIALOG(dialog));

   g_mutex_lock(mi->mutex);
   if(mi->button_fn)
     mi->ret = response;
   else switch(response)
        {  case GTK_RESPONSE_OK:
	     mi->ret = 1;
	   break;

           default:
	     mi->ret = 0;
	   break;
	}

   g_cond_signal(mi->cond);
   g_mutex_unlock(mi->mutex);

   gtk_widget_destroy(dialog);

   return FALSE;
}

static int vmodal_dialog(GtkMessageType mt, GtkButtonsType bt, 
			 void(*button_fn)(GtkDialog*), char *msg, va_list argp)
{  modal_info *mi = g_malloc(sizeof(modal_info));
   char *tmp;
   int idx,ret;

   mi->message_type = mt;
   mi->button_type = bt;
   mi->button_fn = button_fn;
   mi->mutex = g_malloc(sizeof(GMutex)); g_mutex_init(mi->mutex);
   mi->cond  = g_malloc(sizeof(GCond)); g_cond_init(mi->cond);

   tmp = g_strdup_vprintf(msg, argp);
   idx = strlen(tmp);  /* Remove trailing newline */
   if(tmp[idx-1] == '\n')
     tmp[idx-1] = 0;
   mi->msg   = g_locale_to_utf8(tmp, -1, NULL, NULL, NULL);
   g_free(tmp);

   mi->ret = -1;

   CallIdleFunc(modal_idle_func, mi);

   g_mutex_lock(mi->mutex);
   while(mi->ret == -1)
     g_cond_wait(mi->cond, mi->mutex);

   ret = mi->ret;
   g_mutex_unlock(mi->mutex);
   g_free(mi->msg);
   g_mutex_clear(mi->mutex);
   g_free(mi->mutex);
   g_cond_clear(mi->cond);
   g_free(mi->cond);
   g_free(mi);

   return ret;
}

int ModalDialog(GtkMessageType mt, GtkButtonsType bt, 
		void(*button_fn)(GtkDialog*), char *msg, ...)
{  va_list argp;
   int result;

   va_start(argp, msg);
   result = vmodal_dialog(mt, bt, button_fn, msg, argp);
   va_end(argp);

   return result;
}

/*
 * A wrapper around ModalDialog() to create a logged warning.
 * Note that in CLI mode the answer is always "yes",
 * so warnings will be printed but never abort CLI mode.
 */

int ModalWarning(GtkMessageType mt, GtkButtonsType bt, 
		 void(*button_fn)(GtkDialog*), char *msg, ...)
{  va_list argp;
   int result = 1;  

   va_start(argp, msg);
   vlog_warning(msg, argp);
   va_end(argp);
   
   if(Closure->guiMode)
   { va_start(argp, msg);
     result = vmodal_dialog(mt, bt, button_fn, msg, argp);
     va_end(argp);
   }

   return result;
}

/*
 * Set the text in the pango layout and retrieve its extents.
 */

void SetText(PangoLayout *layout, char *text, int *w, int *h)
{  PangoRectangle rect;
   char *t = g_locale_to_utf8(text, -1, NULL, NULL, NULL);

   pango_layout_set_text(layout, t, -1);
   pango_layout_get_pixel_extents(layout, NULL, &rect);

   g_free(t);

   *w = rect.width;
   *h = rect.height;
}

/*
 * Switch a notebook to another page and set the text in a label.
 * Used in some footlines in the GUI.
 */

typedef struct
{  GtkWidget *notebook;
   int newPage;
   GtkWidget *label;
   char *newText;
} footline_info;

static gboolean footline_idle_func(gpointer data)
{  footline_info *fi = (footline_info*)data;

   if(fi->label) 
     gtk_label_set_markup(GTK_LABEL(fi->label), fi->newText);
   gtk_notebook_set_current_page(GTK_NOTEBOOK(fi->notebook), fi->newPage);

   if(fi->newText)
     g_free(fi->newText);
   g_free(fi);

   return FALSE;
}

void SwitchAndSetFootline(GtkWidget *notebook, int page, GtkWidget *label, char *format, ...)
{  va_list argp;
   char *tmp;
   footline_info *fi = g_malloc0(sizeof(footline_info));
   int len;

   fi->notebook = notebook;
   fi->newPage  = page;
   fi->label    = label;

   if(label)
   {  va_start(argp, format);
      tmp  = g_strdup_vprintf(format, argp);
      len = strlen(tmp);
      if(tmp[len-1] == '\n') tmp[len-1]=0;
      fi->newText = g_locale_to_utf8(tmp, -1, NULL, NULL, NULL);
      g_free(tmp);
      va_end(argp);
   }

   g_idle_add(footline_idle_func, fi);
}

/*
 * Rearrange buttons to OK Cancel order
 * in file dialogs
 * 
 * gtk_dialog_set_alternative_button_order()
 * has been introduced since gtk+2.6,
 * but does not seem to work correctly.
 */

void ReverseCancelOK(GtkDialog *dialog)
{  GtkWidget *box, *button ;

   if(!Closure->reverseCancelOK)
      return;

   box = dialog->action_area; 
   button = ((GtkBoxChild*)(g_list_first(GTK_BOX(box)->children)->data))->widget;

   gtk_box_reorder_child(GTK_BOX(box), button, 1);

#if 0
   gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog),
					   GTK_RESPONSE_OK,
					   GTK_RESPONSE_CANCEL,
					   -1);
#endif
}

/*
 * Make widget insensitive for a given amount of time.
 */

static gboolean insensitive_timeout_func(gpointer data)
{  
   gtk_widget_set_sensitive((GtkWidget*)data, TRUE);

   return FALSE;
}


void TimedInsensitive(GtkWidget *widget, int delay)
{

  gtk_widget_set_sensitive(widget, FALSE);

  g_timeout_add(delay, insensitive_timeout_func, (gpointer)widget);
}

/*
 * Get the width of a label text
 */

int GetLabelWidth(GtkLabel *label, char *format, ...)
{  PangoLayout *layout;
   PangoRectangle rect;
   va_list argp;
   char *text;

   va_start(argp, format);
   text = g_strdup_vprintf(format, argp);
   va_end(argp);

   layout = gtk_label_get_layout(label);
   pango_layout_set_text(layout, text, -1);
   pango_layout_get_pixel_extents(layout, NULL, &rect);

   g_free(text);

   return rect.width;
}
 

/*
 * Lock the size of a label to that of the given sample text.
 */

void LockLabelSize(GtkLabel *label, char *format, ...)
{  PangoLayout *layout;
   PangoRectangle rect;
   va_list argp;
   char *text;

   va_start(argp, format);
   text = g_strdup_vprintf(format, argp);
   va_end(argp);

   layout = gtk_label_get_layout(label);
   pango_layout_set_text(layout, text, -1);
   pango_layout_get_pixel_extents(layout, NULL, &rect);

   gtk_widget_set_size_request(GTK_WIDGET(label), rect.width, rect.height);
   gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.0);

   g_free(text);
}

/***
 *** Safety requesters before overwriting stuff
 ***/

static void dont_ask_again_cb(GtkWidget *widget, gpointer data)
{  int state  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  
   Closure->confirmDeletion = !state;

   UpdatePrefsConfirmDeletion();
}

static void insert_button(GtkDialog *dialog)
{  GtkWidget *check,*align;

   align = gtk_alignment_new(0.5, 0.5, 0.0, 0.0);
   gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), align, FALSE, FALSE, 0);

   check = gtk_check_button_new_with_label(_utf("Do not ask again"));
   gtk_container_add(GTK_CONTAINER(align), check);
   gtk_container_set_border_width(GTK_CONTAINER(align), 10);
   g_signal_connect(G_OBJECT(check), "toggled", G_CALLBACK(dont_ask_again_cb), NULL);

   gtk_widget_show(align);
   gtk_widget_show(check);
   ReverseCancelOK(GTK_DIALOG(dialog));
} 

int ConfirmImageDeletion(char *file)
{  int answer;

   if(!Closure->guiMode)  /* Always delete it in command line mode */
      return TRUE;

   if(!Closure->confirmDeletion) /* I told you so... */
      return TRUE;

   answer = ModalDialog(GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL,
			insert_button,
			_("Image file already exists and does not match the medium:\n\n"
			  "%s\n\n"
			  "The existing image file will be deleted."),
			file);

   return answer == GTK_RESPONSE_OK;
}

int ConfirmEccDeletion(char *file)
{  int answer;

   if(!Closure->guiMode)  /* Always delete it in command line mode */
      return TRUE;

   if(!Closure->confirmDeletion) /* I told you so... */
      return TRUE;

   answer = ModalDialog(GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL,
			insert_button,
			_("The error correction file is already present:\n\n"
			  "%s\n\n"
			  "Overwrite it?"),
			file);

   return answer == GTK_RESPONSE_OK;
}

