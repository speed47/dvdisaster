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

#include <sys/wait.h>

/***
 *** Ask user to specify his viewer
 ***/

#define SEARCH_BUTTON 1

typedef struct
{  GtkWidget *dialog;
   GtkWidget *entry;
   GtkWidget *search;
   GtkWidget *filesel;
   GtkWidget *fileok;
   GtkWidget *filecancel;
   char *path;
} viewer_dialog_info;

static void response_cb(GtkWidget *widget, int response, gpointer data)
{  viewer_dialog_info *bdi = (viewer_dialog_info*)data; 

   switch(response)
   {  case GTK_RESPONSE_ACCEPT:
	if(Closure->viewer) g_free(Closure->viewer);
	Closure->viewer = g_strdup(gtk_entry_get_text(GTK_ENTRY(bdi->entry)));
	ShowPDF(bdi->path);
	break;

      case GTK_RESPONSE_REJECT:
	if(bdi->path) g_free(bdi->path);
        break;
   }
   gtk_widget_destroy(widget);
   if(bdi->filesel)
     gtk_widget_destroy(bdi->filesel);
   g_free(bdi);
}

static void search_cb(GtkWidget *widget, gpointer data)
{  viewer_dialog_info *bdi = (viewer_dialog_info*)data; 

   if(widget == bdi->search) 
   {  bdi->filesel = gtk_file_selection_new(_utf("windowtitle|Choose a PDF viewer"));
      bdi->fileok = GTK_FILE_SELECTION(bdi->filesel)->ok_button;
      bdi->filecancel = GTK_FILE_SELECTION(bdi->filesel)->cancel_button;
      ReverseCancelOK(GTK_DIALOG(bdi->filesel));
      gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(bdi->filesel));
      g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(bdi->filesel)->ok_button), "clicked", 
		       G_CALLBACK(search_cb), bdi);
    
      g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(bdi->filesel)->cancel_button), "clicked", 
		       G_CALLBACK(search_cb), bdi);
      
      gtk_widget_show(bdi->filesel);
   }

   if(widget == bdi->fileok)
   {
      if(Closure->viewer) g_free(Closure->viewer);
      Closure->viewer = g_strdup(gtk_file_selection_get_filename(GTK_FILE_SELECTION(bdi->filesel)));
      ShowPDF(bdi->path);
      gtk_widget_destroy(bdi->filesel);
      gtk_widget_destroy(bdi->dialog);
      g_free(bdi);
      return;
   }

   if(widget == bdi->filecancel)
   {  gtk_widget_destroy(bdi->filesel);
      bdi->filesel = NULL;
   }
}

static void viewer_dialog(char *path)
{  GtkWidget *dialog, *vbox, *hbox, *label, *entry, *button;
   viewer_dialog_info *bdi = g_malloc0(sizeof(viewer_dialog_info));

   /* Create the dialog */

   dialog = gtk_dialog_new_with_buttons(_utf("windowtitle|PDF viewer required"), 
				       Closure->window, GTK_DIALOG_DESTROY_WITH_PARENT,
				       GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
				       GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);
   bdi->dialog = dialog;
   if(path)
   {  bdi->path = g_strdup(path);
   }

   vbox = gtk_vbox_new(FALSE, 0);
   gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), vbox, FALSE, FALSE, 0);
   gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

   /* Insert the contents */

   label = gtk_label_new(NULL);
   gtk_label_set_markup(GTK_LABEL(label), _utf("<b>Could not find a suitable PDF viewer.</b>\n\n"
                                               "Which PDF viewer would you like to use\n"
                                               "for reading the online documentation?\n\n"
			                       "Please enter its name (e.g. xpdf) or\n"
			                       "use the \"Search\" button for a file dialog.\n")),
			      gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 10);

   hbox = gtk_hbox_new(FALSE, 0);
   gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 10);

   bdi->entry = entry = gtk_entry_new();
   gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 10);

   bdi->search = button = gtk_button_new_with_label(_utf("Search"));
   g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(search_cb), bdi);
   gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 10);

   /* Show it */

   g_signal_connect(dialog, "response", G_CALLBACK(response_cb), bdi);

   gtk_widget_show_all(dialog);
}

/***
 *** Show the manual in an external viewer
 ***/

/*
 * Check the child processes exit status
 * to find whether the viewer could be invoked.
 */

typedef struct
{  pid_t pid;
   char *path;
   GtkWidget *msg;
   int seconds;
} viewer_info;


static void msg_destroy_cb(GtkWidget *widget, gpointer data)
{  viewer_info *bi = (viewer_info*)data;

   bi->msg = NULL; 
}

/* 
 * The following list of viewers 
 * will be tried one at a time until one entry succeeds by:
 * - returning zero
 * - not returning within 60 seconds
 */

static int viewer_index;
static void try_viewer(viewer_info*);

static char *viewers[] = 
{  "evince",
   "xpdf",
   "okular",
   "gv",
   "mupdf",
   "pdfcube",
   "zathura",
   NULL
};

static gboolean viewer_timeout_func(gpointer data)
{  viewer_info *bi = (viewer_info*)data;
   int status;

   waitpid(bi->pid, &status, WNOHANG);

   /* At least mozilla returns random values under FreeBSD on success,
      so we can't rely on the return value exept our own 110 one. */

   if(WIFEXITED(status))
   {
      switch(WEXITSTATUS(status))
      {  case 110: /* viewer did not execute */
	   viewer_index++;
	   if(!viewers[viewer_index]) /* all viewers from the list failed */
	   {  viewer_dialog(bi->path);

	      if(bi->msg) 
		gtk_widget_destroy(bi->msg);
	      if(bi->path) 
		g_free(bi->path);
	      g_free(bi);
	   }
	   else                        /* try next viewer from list */
	   {  bi->seconds = 0;  
	      try_viewer(bi);
	   }
	   return FALSE;

         case 0:  /* viewer assumed to be successful */
         default:
	   if(bi->msg) 
	     gtk_widget_destroy(bi->msg);
	   if(bi->path) 
	     g_free(bi->path);
	   g_free(bi);
	   return FALSE;
      }
   }

   bi->seconds++;
   if(bi->seconds == 10 && bi->msg)
   {  gtk_widget_destroy(bi->msg);
      bi->msg = NULL;
   }

   return bi->seconds > 60 ? FALSE : TRUE;
}

/*
 * Invoke the viewer
 */

static void try_viewer(viewer_info *bi)
{  pid_t pid;

   bi->pid = pid = fork();

   if(pid == -1)
   {  printf("fork failed\n");
      return;
   }

   /* make the parent remember and wait() for the viewer */

   if(pid > 0)  
   {  g_timeout_add(1000, viewer_timeout_func, (gpointer)bi);

      if(viewer_index)
      {  g_free(Closure->viewer);
	 Closure->viewer = g_strdup(viewers[viewer_index]);
      }
   }

   /* try calling the viewer */

   if(pid == 0)
   {  char *argv[10];
      int argc = 0;

      argv[argc++] = viewer_index ? viewers[viewer_index] : Closure->viewer;
      argv[argc++] = bi->path;
      argv[argc++] = NULL;
      execvp(argv[0], argv);

      _exit(110); /* couldn't execute */
   }
}

void ShowPDF(char *target)
{  viewer_info *bi = g_malloc0(sizeof(viewer_info));
   guint64 ignore;

   if(!Closure->docDir)
   {  
      CreateMessage(_("Documentation not installed."), GTK_MESSAGE_ERROR);
      g_free(bi);
      return;
   }

   /* If no target is given, show the manual. */

   if(!target) 
   {    bi->path = g_strdup_printf("%s/manual.pdf",Closure->docDir); 
   }
   else 
      if(*target != '/') bi->path = g_strdup_printf("%s/%s",Closure->docDir, target); 
      else               bi->path = g_strdup(target); 

   if(!LargeStat(bi->path, &ignore))
   {  
      CreateMessage(_("Documentation file\n%s\nnot found.\n"), GTK_MESSAGE_ERROR, bi->path);
      g_free(bi->path);
      g_free(bi);
      return;
   }

   /* Lock the help button and show a message for 10 seconds. */

   TimedInsensitive(Closure->helpButton, 10000);
   bi->msg = CreateMessage(_("Please hang on until the viewer comes up!"), GTK_MESSAGE_INFO);
   g_signal_connect(G_OBJECT(bi->msg), "destroy", G_CALLBACK(msg_destroy_cb), bi);

   /* Try the first viwer */

   viewer_index = 0;
   try_viewer(bi);
}
