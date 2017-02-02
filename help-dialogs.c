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

#include "help-dialogs.h"

/***
 *** Online help system for the preferences
 ***/

/*
 * Create a help window
 */

/* Close button response */

static void close_cb(GtkWidget *widget, gpointer data)
{  LabelWithOnlineHelp *lwoh = (LabelWithOnlineHelp*)data;

   gtk_widget_hide(lwoh->helpWindow);
}

/* Do not destroy the window when closed via the window manager */

static gboolean delete_cb(GtkWidget *widget, GdkEvent *event, gpointer data)
{  LabelWithOnlineHelp *lwoh = (LabelWithOnlineHelp*)data;

   gtk_widget_hide(lwoh->helpWindow);

   return TRUE;
}

/*
 * Get a new integer variable from the lastSizes array 
 */

static int* get_new_int(LabelWithOnlineHelp* lwoh)
{  int *var = g_malloc0(sizeof(int));

   if(!lwoh->lastSizes)
      lwoh->lastSizes = g_ptr_array_new();

   g_ptr_array_add(lwoh->lastSizes, var);

   return var;
}

/*
 * Callback for the help link
 */

static gint help_cb(GtkWidget *widget, GdkEvent *event, gpointer data)
{  GtkWidget *lab = GTK_BIN(widget)->child;
   LabelWithOnlineHelp *lwoh = (LabelWithOnlineHelp*)data;

   switch(event->type)
   {  case GDK_BUTTON_PRESS: 
        if(!lwoh->inside) return FALSE; /* Defect in certain Gtk versions? */
	gtk_widget_show_all(GTK_WIDGET(lwoh->helpWindow));
	break; 

      case GDK_ENTER_NOTIFY: 
	gtk_label_set_markup(GTK_LABEL(lab), lwoh->highlitText);
	lwoh->inside = TRUE;
	break;

      case GDK_LEAVE_NOTIFY: 
	gtk_label_set_markup(GTK_LABEL(lab), lwoh->normalText);
	lwoh->inside = FALSE;
	break;

      default: break;
   }

   return FALSE;
}

/*
 * Create a frame labeled with a link to the help system
 */

LabelWithOnlineHelp* CreateLabelWithOnlineHelp(char *title, char *ascii_text)
{  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
   GtkWidget *vbox, *hbox, *button;
   GtkWidget *ebox  = gtk_event_box_new();
   LabelWithOnlineHelp *lwoh;
 
   /*** Initialize online help context */

   lwoh = g_malloc0(sizeof(LabelWithOnlineHelp));
   lwoh->normalLabel = gtk_label_new(NULL);
   lwoh->linkLabel = gtk_label_new(NULL);
   lwoh->linkBox = ebox;
   lwoh->windowTitle = g_locale_to_utf8(title, -1, NULL, NULL, NULL);
   SetOnlineHelpLinkText(lwoh, ascii_text);

   gtk_label_set_markup(GTK_LABEL(lwoh->normalLabel), lwoh->normalText);

   /*** Create the help window */

   lwoh->helpWindow = window;
   gtk_window_set_title(GTK_WINDOW(window), lwoh->windowTitle);
   gtk_window_set_icon(GTK_WINDOW(window), Closure->windowIcon);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

   lwoh->outerPadding = 12;
   gtk_container_set_border_width(GTK_CONTAINER(window), lwoh->outerPadding);
   lwoh->outerPadding *= 2;

   /* Connect window with the close button from the window manager */

   g_signal_connect(window, "delete_event", G_CALLBACK(delete_cb), lwoh);

   /* Create the main layout of the window */

   lwoh->vbox = vbox = gtk_vbox_new(FALSE, 0);
   gtk_container_add(GTK_CONTAINER(window), vbox);
   
   hbox = gtk_hbox_new(FALSE, 0);
   gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

   button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
   gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, FALSE, 0);
   g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(close_cb), lwoh);

   gtk_box_pack_end(GTK_BOX(vbox), gtk_hseparator_new(), FALSE, FALSE, 6);

   /*** Put link label into an event box */

   gtk_widget_set_events(ebox, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK);
   g_signal_connect(G_OBJECT(ebox), "button_press_event", G_CALLBACK(help_cb), (gpointer)lwoh);
   g_signal_connect(G_OBJECT(ebox), "enter_notify_event", G_CALLBACK(help_cb), (gpointer)lwoh);
   g_signal_connect(G_OBJECT(ebox), "leave_notify_event", G_CALLBACK(help_cb), (gpointer)lwoh);

   gtk_label_set_markup(GTK_LABEL(lwoh->linkLabel), lwoh->normalText);
   gtk_container_add(GTK_CONTAINER(ebox), lwoh->linkLabel);

   return lwoh;
}

LabelWithOnlineHelp* CloneLabelWithOnlineHelp(LabelWithOnlineHelp *orig, char *ascii_text)
{  LabelWithOnlineHelp *lwoh;
   GtkWidget *ebox  = gtk_event_box_new();
 
   /*** Initialize online help context from given one */

   lwoh = g_malloc0(sizeof(LabelWithOnlineHelp));
   lwoh->helpWindow = orig->helpWindow;

   /*** Only replace the labels */

   lwoh->normalLabel = gtk_label_new(NULL);
   lwoh->linkLabel   = gtk_label_new(NULL);
   lwoh->linkBox     = ebox;
   lwoh->windowTitle = g_strdup("ignore");

   SetOnlineHelpLinkText(lwoh, ascii_text);

   /*** Put link label into an event box */

   gtk_widget_set_events(ebox, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK);
   g_signal_connect(G_OBJECT(ebox), "button_press_event", G_CALLBACK(help_cb), (gpointer)lwoh);
   g_signal_connect(G_OBJECT(ebox), "enter_notify_event", G_CALLBACK(help_cb), (gpointer)lwoh);
   g_signal_connect(G_OBJECT(ebox), "leave_notify_event", G_CALLBACK(help_cb), (gpointer)lwoh);

   gtk_label_set_markup(GTK_LABEL(lwoh->normalLabel), lwoh->normalText);
   gtk_label_set_markup(GTK_LABEL(lwoh->linkLabel), lwoh->normalText);
   gtk_container_add(GTK_CONTAINER(ebox), lwoh->linkLabel);

   return lwoh;
}

void SetOnlineHelpLinkText(LabelWithOnlineHelp *lwoh, char *ascii_text)
{  char text[strlen(ascii_text)+80];

   if(lwoh->normalText) g_free(lwoh->normalText);
   if(lwoh->highlitText) g_free(lwoh->highlitText);

   lwoh->normalText  = g_locale_to_utf8(ascii_text, -1, NULL, NULL, NULL);
   g_sprintf(text, "<span underline=\"single\" color=\"blue\">%s</span>", ascii_text);
   lwoh->highlitText = g_locale_to_utf8(text, -1, NULL, NULL, NULL);
}

void FreeLabelWithOnlineHelp(LabelWithOnlineHelp *lwoh)
{  
   if(lwoh->lastSizes)
   {  int i;

      for(i=0; i<lwoh->lastSizes->len; i++)
      {  int *var = g_ptr_array_index(lwoh->lastSizes, i);

	 g_free(var);
      }
      g_ptr_array_free(lwoh->lastSizes, FALSE);
   }

   g_free(lwoh->windowTitle);
   g_free(lwoh->normalText);
   g_free(lwoh->highlitText);
   g_free(lwoh);
}

/*
 * Add a paragraph of text to the help window
 */

static gboolean wrapper_fix_cb(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{  int *last_width = (int*)data;
   int label_width = widget->allocation.width;
   
   if(*last_width == label_width)  /* short circuit expose events */ 
      return FALSE;                /* without size changes */

   *last_width = label_width;

   /* This is a hack. We feed the label its own allocation to make it redraw.
      Note that we subtract 4 or else the window would never shrink again. */

   if(label_width<0 || label_width>200)
      gtk_widget_set_size_request(widget, label_width-4, -1);

   return FALSE;
}

void AddHelpParagraph(LabelWithOnlineHelp *lwoh, char *format, ...)
{  GtkWidget *label = gtk_label_new(NULL);
   va_list argp;
   char *text,*utf;

   va_start(argp, format);
   text = g_strdup_vprintf(format, argp);
   va_end(argp);

   utf = g_locale_to_utf8(text, -1, NULL, NULL, NULL);
   gtk_label_set_markup(GTK_LABEL(label), utf);
   g_free(utf);
   g_free(text);

   gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.0);
   gtk_box_pack_start(GTK_BOX(lwoh->vbox), label, FALSE, FALSE, 0);

   /* Work around some bugs in the gtk line wrapper code.
      By default lines are wrapped at the length of 
      "This long string gives a good enough length for any line to have."
      which is, well, stupid. */ 

   gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
   g_signal_connect(label, "expose_event", G_CALLBACK(wrapper_fix_cb), get_new_int(lwoh));
}

/*
 * Add an item list to the help window.
 * The list may be preceeded by an optional paragraph of text.
 */

void AddHelpListItem(LabelWithOnlineHelp *lwoh, char *format, ...)
{  GtkWidget *label = gtk_label_new(NULL);
   GtkWidget *bullet = gtk_label_new(" - ");
   GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
   va_list argp;
   char *text,*utf;

   gtk_box_pack_start(GTK_BOX(lwoh->vbox), hbox, FALSE, FALSE, 0);

   gtk_misc_set_alignment(GTK_MISC(bullet), 0.0, 0.0);
   gtk_box_pack_start(GTK_BOX(hbox), bullet, FALSE, FALSE, 0);

   va_start(argp, format);
   text = g_strdup_vprintf(format, argp);
   va_end(argp);

   utf = g_locale_to_utf8(text, -1, NULL, NULL, NULL);
   gtk_label_set_markup(GTK_LABEL(label), utf);
   g_free(utf);
   g_free(text);

   gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.0);
   gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);

   /* Work around some bugs in the gtk line wrapper code.
      By default lines are wrapped at the length of 
      "This long string gives a good enough length for any line to have."
      which is, well, stupid. */ 

   gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
   g_signal_connect(label, "expose_event", G_CALLBACK(wrapper_fix_cb), get_new_int(lwoh));
}

/*
 * Add a (fully functional!) widget set to the help window 
 */

void AddHelpWidget(LabelWithOnlineHelp *lwoh, GtkWidget *widget)
{  
   gtk_box_pack_start(GTK_BOX(lwoh->vbox), widget, FALSE, FALSE, 10);
   gtk_box_pack_start(GTK_BOX(lwoh->vbox), gtk_hseparator_new(), FALSE, FALSE, 10);
}

/***
 *** The log viewer
 ***/

static void log_destroy_cb(GtkWidget *widget, gpointer data)
{
   /* Avoid race condition with next function */

   g_mutex_lock(Closure->logLock);
   Closure->logWidget = NULL;
   Closure->logScroll = NULL;
   Closure->logBuffer = NULL;
   g_mutex_unlock(Closure->logLock);
}

static gboolean log_jump_func(gpointer data)
{  GtkAdjustment *a;
   GtkTextIter end;

   /* Locking is needed as user might destroy the window
      while we are updating it */

   g_mutex_lock(Closure->logLock);
   if(!Closure->logWidget)
   {  g_mutex_unlock(Closure->logLock);
      return FALSE;
   }
   gtk_text_buffer_get_end_iter(Closure->logBuffer, &end);
   gtk_text_buffer_place_cursor(Closure->logBuffer, &end);

   a = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(Closure->logScroll));
   gtk_adjustment_set_value(a, a->upper - a->page_size);
   gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(Closure->logScroll), a);
   g_mutex_unlock(Closure->logLock);

   return FALSE;
}

static gboolean log_idle_func(gpointer data)
{
   g_mutex_lock(Closure->logLock);
   if(Closure->logBuffer)
      gtk_text_buffer_set_text(Closure->logBuffer, Closure->logString->str, Closure->logString->len);
   g_mutex_unlock(Closure->logLock);

   g_idle_add(log_jump_func, NULL);
   return FALSE;
}


void UpdateLog()
{  static int unique_addr;

   if(Closure->logWidget)
   {  g_idle_remove_by_data(&unique_addr);
      g_idle_add(log_idle_func, &unique_addr);
   }
}

void ShowLog()
{ GtkWidget *w;

  if(Closure->logWidget) 
  {  gtk_widget_show(Closure->logWidget);
     return;
  }

  w = ShowTextfile(_("windowtitle|Log data"),
		   _("<big>Log data</big>\n"
		     "<i>Protocol of the current or previous action</i>"),
		   "*LOG*", &Closure->logScroll, &Closure->logBuffer);

  g_signal_connect(G_OBJECT(w), "destroy", G_CALLBACK(log_destroy_cb), NULL);

  Closure->logWidget = w;
}


/***
 *** Specific help dialogs 
 ***/

void ShowGPL()
{
  ShowTextfile(_("windowtitle|GNU General Public License"), 
	       _("<big>GNU General Public License</big>\n"
		 "<i>The license terms of dvdisaster.</i>"),
	       "COPYING", NULL, NULL);
}

/*
 * Dialog for displaying text files
 */

char *find_file(char *file, size_t *size, char *lang)
{  char *path;
   char lang_suffix[3];
   guint64 stat_size;

   lang_suffix[0] = lang_suffix[2] = 0;

   if(lang)
   {  
      lang_suffix[0] = lang[0];
      lang_suffix[1] = lang[1];
   }

   /* Try file in bin dir */

   if(Closure->binDir) 
   {  if(lang) 
           path = g_strdup_printf("%s/%s.%s",Closure->binDir, file, lang_suffix);
      else path = g_strdup_printf("%s/%s",Closure->binDir, file);

      if(LargeStat(path, &stat_size))
      {	 *size = stat_size;
	 return path;
      }

      g_free(path);
   }

   /* Try file in doc dir */

   if(Closure->docDir)
   {  if(lang)
           path = g_strdup_printf("%s/%s.%s",Closure->docDir, file, lang_suffix);
      else path = g_strdup_printf("%s/%s",Closure->docDir, file);

      if(LargeStat(path, &stat_size))
      {	 *size = stat_size;
	 return path;
      }

      g_free(path);
   }

   return NULL;
}

GtkWidget* ShowTextfile(char *title, char *explanation, char *file, 
			GtkScrolledWindow **scroll_out, GtkTextBuffer **buffer_out)
{  GtkWidget *dialog, *scroll_win, *vbox, *lab, *sep, *view;
   GtkTextBuffer *buffer; 
   GtkTextIter start;
   char *path;
   char *utf,*buf;
   size_t size = 0;

   /*** Read the text file */

   if(*file != '*')
   {  
       if(    !(path = find_file(file, &size, NULL))
	   && !(path = find_file(file, &size, (char*)g_getenv("LANG")))
	   && !(path = find_file(file, &size, "en"))
         )
      {  char *trans = _utf("File\n%s\nnot present");
      
	 buf = g_strdup_printf(trans, file);
	 size = strlen(buf);
      }
      else
      {  FILE *fptr = portable_fopen(path, "rb");
	 size_t bytes_read;

	 if(!fptr)
	 {  char *trans = _utf("File\n%s\nnot accessible");

	    buf = g_strdup_printf(trans, file);
	    size = strlen(buf);
	 }
	 else
	 {  buf = g_malloc(size);
	    bytes_read = fread(buf, 1, size, fptr);
	    fclose(fptr);
	    g_free(path);

	    if(bytes_read < size)
	    {  char *trans = _utf("\n<- Error: Text file truncated here");

 	       size = bytes_read + strlen(trans);
	       buf = realloc(buf, size+1);
	       strcpy(&buf[bytes_read], trans);
	    }
	 }
      }
   }
   else 
   {  g_mutex_lock(Closure->logLock);
      buf  = Closure->logString->str;
      size = Closure->logString->len;
      g_mutex_unlock(Closure->logLock);
   }

   /*** Create the dialog */

   utf = g_locale_to_utf8(title, -1, NULL, NULL, NULL);
   dialog = gtk_dialog_new_with_buttons(utf, Closure->window, GTK_DIALOG_DESTROY_WITH_PARENT,
				       GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT, NULL);
   g_free(utf);
   gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 600);
   g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);

   vbox = gtk_vbox_new(FALSE, 0);
   gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), vbox, TRUE, TRUE, 0);
   gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);

   lab = gtk_label_new(NULL);
   utf = g_locale_to_utf8(explanation, -1, NULL, NULL, NULL);
   gtk_label_set_markup(GTK_LABEL(lab), utf);
   g_free(utf);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_box_pack_start(GTK_BOX(vbox), lab, FALSE, FALSE, 0);

   sep = gtk_hseparator_new();
   gtk_box_pack_start(GTK_BOX(vbox), sep, FALSE, FALSE, 0);

   scroll_win = gtk_scrolled_window_new(NULL, NULL);
   gtk_box_pack_start(GTK_BOX(vbox), scroll_win, TRUE, TRUE, 5);
   if(scroll_out) *scroll_out = GTK_SCROLLED_WINDOW(scroll_win);

   view   = gtk_text_view_new();
   buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
   if(buffer_out) *buffer_out = buffer;

   gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
   gtk_text_buffer_set_text(buffer, buf, size);
   gtk_text_buffer_get_start_iter(buffer, &start);
   gtk_text_buffer_place_cursor(buffer, &start);

   gtk_container_add(GTK_CONTAINER(scroll_win), view);

   /* Show it */

   gtk_widget_show_all(dialog);

   if(*file != '*')
     g_free(buf);

   return dialog;
}

/*
 * About dialog
 */

static void show_modifying(void)
{  ShowTextfile(_("windowtitle|Modifying dvdisaster"), 
	       _("<big>Modifying dvdisaster</big>\n"
		 "<i>Your changes are not ours.</i>"),
	       "README.MODIFYING", NULL, NULL);
}

static gint about_cb(GtkWidget *widget, GdkEvent *event, gpointer data)
{  GtkWidget *lab = GTK_BIN(widget)->child;
   char *label = (char*)data;
   char text[strlen(label)+80];
   char *utf;
   static int inside;

   switch(event->type)
   {  case GDK_BUTTON_PRESS: 
        if(!inside) return FALSE; /* Defect in certain Gtk versions? */
        if(!strcmp(label,"GPL")) ShowGPL(); 
        else if(!strcmp(label,"MODIFYING")) show_modifying(); 
        else ShowPDF(g_strdup(label));
	break; 
      case GDK_ENTER_NOTIFY: 
	g_sprintf(text, "<span underline=\"single\" color=\"blue\">%s</span>", label);
	utf = g_locale_to_utf8(text, -1, NULL, NULL, NULL);
	gtk_label_set_markup(GTK_LABEL(lab), utf);
	g_free(utf);
	inside = TRUE;
	break;
      case GDK_LEAVE_NOTIFY: 
	g_sprintf(text, "<span color=\"blue\">%s</span>", label);
	utf = g_locale_to_utf8(text, -1, NULL, NULL, NULL);
	gtk_label_set_markup(GTK_LABEL(lab), utf); 
	g_free(utf);
	inside = FALSE;
	break;
      default: break;
   }

   return FALSE;
}

void AboutText(GtkWidget *parent, char *format, ...)
{  GtkWidget *lab;
   char *tmp, *utf_text;
   va_list argp;

   va_start(argp, format);

   lab = gtk_label_new(NULL);
   tmp = g_strdup_vprintf(format, argp);
   utf_text = g_locale_to_utf8(tmp, -1, NULL, NULL, NULL);
   gtk_label_set_markup(GTK_LABEL(lab), utf_text);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_box_pack_start(GTK_BOX(parent), lab, FALSE, FALSE, 0);

   g_free(tmp);
   g_free(utf_text);

   va_end(argp);
}

void AboutLink(GtkWidget *parent, char *label, char *action)
{  GtkWidget *ebox,*lab;
   char text[strlen(label)+80];
   char *label_copy = strdup(label);
   char *utf;

   ebox = gtk_event_box_new();
   gtk_widget_set_events(ebox, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK);
   g_signal_connect(G_OBJECT(ebox), "button_press_event", G_CALLBACK(about_cb), (gpointer)action);
   g_signal_connect(G_OBJECT(ebox), "enter_notify_event", G_CALLBACK(about_cb), (gpointer)label_copy);
   g_signal_connect(G_OBJECT(ebox), "leave_notify_event", G_CALLBACK(about_cb), (gpointer)label_copy);

   gtk_box_pack_start(GTK_BOX(parent), ebox, FALSE, FALSE, 0);

   lab  = gtk_label_new(NULL);
   g_sprintf(text, "<span color=\"blue\">%s</span>", label);
   utf = g_locale_to_utf8(text, -1, NULL, NULL, NULL);
   gtk_label_set_markup(GTK_LABEL(lab), utf);
   gtk_container_add(GTK_CONTAINER(ebox), lab);
   g_free(utf);
}

void AboutTextWithLink(GtkWidget *parent, char *text, char *action)
{  char *copy,*head,*end_of_line;
   char *link_start,*link_end; 
   char *utf;

   head = copy = g_strdup(text);

   while(*head)
   {  end_of_line = strchr(head, '\n');
      if(end_of_line && *end_of_line == '\n')
        *end_of_line = 0;

      link_start = strchr(head, '[');
      link_end = strchr(head, ']');

      if(link_start && link_end)
      {  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);

         gtk_box_pack_start(GTK_BOX(parent), hbox, FALSE, FALSE, 0);
	 *link_start++ = *link_end++ = 0;

         if(*head) 
         {  GtkWidget *lab = gtk_label_new(NULL);

	    utf = g_locale_to_utf8(head, -1, NULL, NULL, NULL);
	    gtk_label_set_markup(GTK_LABEL(lab), utf);
	    gtk_box_pack_start(GTK_BOX(hbox), lab, FALSE, FALSE, 0);
	    g_free(utf);
	 }

         AboutLink(hbox, link_start, action);

         if(*link_end) 
         {  GtkWidget *lab = gtk_label_new(NULL);

	    utf = g_locale_to_utf8(link_end, -1, NULL, NULL, NULL);
	    gtk_label_set_markup(GTK_LABEL(lab), utf);
	    gtk_box_pack_start(GTK_BOX(hbox), lab, FALSE, FALSE, 0);
	    g_free(utf);
	 }
      }
      else AboutText(parent, head);

      if(end_of_line) head = end_of_line+1;
      else break;
   }

   g_free(copy);
}

void AboutDialog()
{  GtkWidget *about, *vbox, *sep;
   char *text; 
#ifndef MODIFIED_SOURCE
   const char *lang;
#endif
   /* Create the dialog */

   about = gtk_dialog_new_with_buttons(_utf("windowtitle|About dvdisaster"), 
				       Closure->window, GTK_DIALOG_DESTROY_WITH_PARENT,
				       GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT, NULL);

   g_signal_connect_swapped(about, "response", G_CALLBACK(gtk_widget_destroy), about);

   vbox = gtk_vbox_new(FALSE, 0);
   gtk_box_pack_start(GTK_BOX(GTK_DIALOG(about)->vbox), vbox, FALSE, FALSE, 0);
   gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

   /* Insert the labels */

   text = g_strdup_printf("<span weight=\"bold\" size=\"xx-large\">dvdisaster</span><i> "
			  "Version %s</i>",
			  Closure->cookedVersion);
   AboutText(vbox, text);
   g_free(text);

#ifdef MODIFIED_SOURCE
   AboutTextWithLink(vbox, 
		     _("Modified version Copyright 2015 (please fill in - [directions])\n"
		       "Copyright 2004-2015 Carsten Gnoerlich"),
		     "MODIFYING");
#else
   AboutText(vbox, _("Copyright 2004-2015 Carsten Gnoerlich"));
#endif

   sep = gtk_hseparator_new();
   gtk_box_pack_start(GTK_BOX(vbox), sep, FALSE, FALSE, 10);


   AboutText(vbox, _("dvdisaster provides a margin of safety against data loss\n"
		      "on optical media caused by aging or scratches.\n"
		      "It creates error correction data which is used to recover\n"
		      "unreadable sectors if the disc becomes damaged later on.\n"));

   AboutTextWithLink(vbox, _("This software comes with  <b>absolutely no warranty</b>.\n"
				"This is free software and you are welcome to redistribute it\n"
				"under the conditions of the [GNU General Public License].\n"), 
			"GPL");

#ifdef MODIFIED_SOURCE
   AboutTextWithLink(vbox, _("\nThis program is <b>not the original</b>. It is based on the\n"
			     "source code of dvdisaster, but contains third-party changes.\n\n"
			     "Please do not bother the original authors of dvdisaster\n"
			     "([www.dvdisaster.org]) about issues with this version.\n"),
		             "http://www.dvdisaster.org");

#else
   lang = g_getenv("LANG");
   if(lang && !strncmp(lang, "de", 2))
   {    AboutTextWithLink(vbox, "\n[http://www.dvdisaster.de]", "http://www.dvdisaster.de");
   }
   else 
   {    AboutTextWithLink(vbox, "\n[http://www.dvdisaster.com]", "http://www.dvdisaster.com");
   }

   AboutText(vbox, _("\ne-mail: carsten@dvdisaster.org   -or-   cgnoerlich@fsfe.org")); 
#ifdef SYS_NETBSD
   AboutText(vbox, _("\nNetBSD port: Sergey Svishchev &lt;svs@ropnet.ru&gt;")); 
#endif
#endif
   /* Show it */

   gtk_widget_show_all(about);
}
