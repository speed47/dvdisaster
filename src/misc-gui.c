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

/***
 *** GUI functions which are cli-only safe for convenience.
 ***/

/*
 * Label convenience functions.
 * Sets the label text from another thread.
 */

#ifdef WITH_GUI_YES
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

void GuiSetLabelText(GtkWidget *widget, char *format, ...)
{  label_info *li; 
   va_list argp;

   if(!Closure->guiMode)
     return;
   
   li = g_malloc(sizeof(label_info));
   li->label = GTK_LABEL(widget);

   va_start(argp, format);
   if(format)
   {  char *tmp  = g_strdup_vprintf(format, argp);

      if(!tmp) tmp=g_strdup_printf("GuiSetLabelText(%s) failed",format);
      li->text = g_locale_to_utf8(tmp, -1, NULL, NULL, NULL);
      g_free(tmp);
   }
   else li->text = g_locale_to_utf8("(null)", -1, NULL, NULL, NULL);
   va_end(argp);
   g_idle_add(label_idle_func, li);
}
#endif

/*
 * Progress bar convenience function.
 * Percentage is given as a multiple of 0.1 percent.
 */

#ifdef WITH_GUI_YES

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

void GuiSetProgress(GtkWidget *pbar, int percent, int max)
{  progress_info *pi;

   if(!Closure->guiMode) return;
    
   pi = g_malloc(sizeof(progress_info));

   pi->pbar    = pbar;
   pi->percent = percent;
   pi->max     = max;

   g_idle_add(progress_idle_func, pi);
}
#endif

/*
 * Switch a notebook to another page and set the text in a label.
 * Used in some footlines in the GUI.
 * Does nothing in CLI mode to save us from lots of #ifdef WITH_GUI
 */

#ifdef WITH_GUI_YES
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

void GuiSwitchAndSetFootline(GtkWidget *notebook, int page, GtkWidget *label, char *format, ...)
{  va_list argp;
   char *tmp;
   footline_info *fi;
   int len;

   if(!Closure->guiMode) return;
   
   fi = g_malloc0(sizeof(footline_info));
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
 * CLI mode and GUI mode behave differently wrt. to the worker thread.
 * In CLI mode, the worker thread is the main thread and must not be terminated
 * when the worker task is finished. However in GUI mode the worker is a separate
 * thread which must exit after the assigned work is done.
 */

void GuiExitWorkerThread()
{
   if(Closure->guiMode)
      g_thread_exit(0);
}

#endif

/*
 * A wrapper around GuiModalDialog() to create a logged warning.
 * Note that in CLI mode the answer is always "yes",
 * so warnings will be printed but never abort CLI mode.
 */

#ifdef WITH_GUI_YES  
static int vmodal_dialog(GtkMessageType, GtkButtonsType, 
			 void(*)(GtkDialog*), char*, va_list);
#endif

int ModalWarning(GtkMessageType mt, GtkButtonsType bt, 
		 void(*button_fn)(GtkDialog*), char *msg, ...)
{  va_list argp;
   int result = 1;  

   va_start(argp, msg);
   vLogWarning(msg, argp);
   va_end(argp);

#ifdef WITH_GUI_YES   
   if(Closure->guiMode)
   { va_start(argp, msg);
     result = vmodal_dialog(mt, bt, button_fn, msg, argp);
     va_end(argp);
   }
#endif
   
   return result;
}

/*
 * Safety requesters before deleting something.
 */

#ifdef WITH_GUI_YES  
static void insert_button(GtkDialog*);

int GuiConfirmEccDeletion(char *file)
{  int answer;
  
   if(!Closure->guiMode)  /* Always delete it in command line mode */
      return TRUE;

   if(!Closure->confirmDeletion) /* I told you so... */
      return TRUE;

   answer = GuiModalDialog(GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL,
			   insert_button,
			   _("The error correction file is already present:\n\n"
			     "%s\n\n"
			     "Overwrite it?"),
			   file);

   return answer == GTK_RESPONSE_OK;
}
#endif

/*** remaining GUI functions */

#ifdef WITH_GUI_YES
/*
 * Spawning of idle functions.
 * Idle functions are required to perform actions (like opening
 * a dialogue) from a sub thread.
 * However idle functions must not be spawned from the main thread
 * as it would block infinitely; in that case we must run the idle
 * function directly.
 */

static void call_idle_func(gboolean (*idle_func)(gpointer), gpointer data)
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

void GuiShowWidget(GtkWidget *widget)
{
  if(Closure->guiMode)
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
   {  GuiHidePreferences();
   }
     
   Closure->stopActions = FALSE;

   return FALSE;
}

void GuiAllowActions(gboolean s)
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

void GuiShowMessage(GtkWindow *parent, char *msg, GtkMessageType type)
{  message_info *mi;

   if(!Closure->guiMode) return;
  
   mi = g_malloc(sizeof(message_info));
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

GtkWidget* GuiCreateMessage(char *format, GtkMessageType type, ...)
{  GtkWidget *dialog;
   va_list argp;
   char *text,*utf8; 

   if(!Closure->guiMode)
     return NULL;
   
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
   else  GuiReverseCancelOK(GTK_DIALOG(dialog));

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

   call_idle_func(modal_idle_func, mi);

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

int GuiModalDialog(GtkMessageType mt, GtkButtonsType bt, 
		   void(*button_fn)(GtkDialog*), char *msg, ...)
{  va_list argp;
   int result;

   if(!Closure->guiMode)
     Stop("GuiModalDialog() called with Closure->guiMode == False");
   
   va_start(argp, msg);
   result = vmodal_dialog(mt, bt, button_fn, msg, argp);
   va_end(argp);

   return result;
}

/*
 * Set the text in the pango layout and retrieve its extents.
 */

void GuiSetText(PangoLayout *layout, char *text, int *w, int *h)
{  PangoRectangle rect;
   char *t;

   if(!Closure->guiMode)
     return;   
   
   t = g_locale_to_utf8(text, -1, NULL, NULL, NULL);

   pango_layout_set_text(layout, t, -1);
   pango_layout_get_pixel_extents(layout, NULL, &rect);

   g_free(t);

   *w = rect.width;
   *h = rect.height;
}

/*
 * Rearrange buttons to OK Cancel order
 * in file dialogs
 * 
 * gtk_dialog_set_alternative_button_order()
 * has been introduced since gtk+2.6,
 * but does not seem to work correctly.
 */

void GuiReverseCancelOK(GtkDialog *dialog)
{  GtkWidget *box, *button ;

   if(!Closure->guiMode || !Closure->reverseCancelOK)
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
 * Get the width of a label text
 */

int GuiGetLabelWidth(GtkLabel *label, char *format, ...)
{  PangoLayout *layout;
   PangoRectangle rect;
   va_list argp;
   char *text;

   if(!Closure->guiMode)
     return 0;
   
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

void GuiLockLabelSize(GtkWidget *wid, char *format, ...)
{  PangoLayout *layout;
   PangoRectangle rect;
   va_list argp;
   char *text;

   if(!Closure->guiMode)
     return;
   
   va_start(argp, format);
   text = g_strdup_vprintf(format, argp);
   va_end(argp);

   layout = gtk_label_get_layout(GTK_LABEL(wid));
   pango_layout_set_text(layout, text, -1);
   pango_layout_get_pixel_extents(layout, NULL, &rect);

   gtk_widget_set_size_request(wid, rect.width, rect.height);
   gtk_misc_set_alignment(GTK_MISC(wid), 0.0, 0.0);

   g_free(text);
}

/***
 *** Safety requesters before overwriting stuff
 ***/

static void dont_ask_again_cb(GtkWidget *widget, gpointer data)
{  int state  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  
   Closure->confirmDeletion = !state;

   GuiUpdatePrefsConfirmDeletion();
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
   GuiReverseCancelOK(GTK_DIALOG(dialog));
} 

int GuiConfirmImageDeletion(char *file)
{  int answer;

   if(!Closure->guiMode)  /* Always delete it in command line mode */
      return TRUE;

   if(!Closure->confirmDeletion) /* I told you so... */
      return TRUE;

   answer = GuiModalDialog(GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL,
			   insert_button,
			   _("Image file already exists and does not match the medium:\n\n"
			     "%s\n\n"
			     "The existing image file will be deleted."),
			   file);

   return answer == GTK_RESPONSE_OK;
}
#endif
