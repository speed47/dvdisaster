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

/***
 *** Global callbacks
 ***/

static void destroy_cb(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}

static gboolean delete_cb(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    return FALSE;
}

/***
 *** The right-side action buttons
 ***/

static void remove_the_00s(char *filename)
{  char *dot = strrchr(filename, '.');

   if(dot) 
   {  int pos = dot-filename;
     
      if(pos>2 && filename[pos-2] == '0' 
	       && filename[pos-1] == '0') 
	memmove(filename+pos-2, filename+pos, 
		strlen(filename)-pos+1);
   }
}

/*
 * Callback for the action buttons
 */

static void action_cb(GtkWidget *widget, gpointer data)
{  gint action = GPOINTER_TO_INT(data);
   Method *method; 

   if(action != ACTION_STOP)
   {  
      /* Clear the log buffer, request new log file time stamp */
      
      if(action != ACTION_CREATE_CONT)
      {  g_mutex_lock(Closure->logLock);
	 g_string_truncate(Closure->logString, 0);
         g_string_printf(Closure->logString, _("dvdisaster-%s log\n"),VERSION);
	 g_mutex_unlock(Closure->logLock);
	 Closure->logFileStamped = FALSE;
      }

      /* Make sure we're using the current file selections */
   
      g_free(Closure->imageName);
      Closure->imageName = g_strdup(gtk_entry_get_text(GTK_ENTRY(Closure->imageEntry)));
      if(Closure->autoSuffix)
      {  Closure->imageName = ApplyAutoSuffix(Closure->imageName, "iso");
	 gtk_entry_set_text(GTK_ENTRY(Closure->imageEntry), Closure->imageName);
      }

      if(Closure->crcImageName && strcmp(Closure->imageName, Closure->crcImageName))
	ClearCrcCache();

      g_free(Closure->eccName);
      Closure->eccName = g_strdup(gtk_entry_get_text(GTK_ENTRY(Closure->eccEntry)));
      if(Closure->autoSuffix)
      {  Closure->eccName = ApplyAutoSuffix(Closure->eccName, "ecc");
	 gtk_entry_set_text(GTK_ENTRY(Closure->eccEntry), Closure->eccName);
      }

      /* Transform foo00.[iso|ecc] into foo.[iso|ecc] when in filesplit mode */

      if(Closure->splitFiles)
      {  remove_the_00s(Closure->imageName);
	 remove_the_00s(Closure->eccName);
      }

      /* Reset warnings which may be temporarily disabled during an action */

      Closure->noMissingWarnings = FALSE;

      /* Do not queue up stop actions */

      Closure->stopActions = FALSE;
   }

   /* Dispatch action */

   switch(action)
   {  case ACTION_STOP: 
        Closure->stopActions = TRUE;
        break;

      case ACTION_READ:
	ClearCrcCache();
	AllowActions(FALSE);

	if(Closure->adaptiveRead) 
	{    gtk_notebook_set_current_page(GTK_NOTEBOOK(Closure->notebook), 2);
	     ResetAdaptiveReadWindow();
	     CreateGThread((GThreadFunc)ReadMediumAdaptive, (gpointer)0);
	}
	else 
	{    gtk_notebook_set_current_page(GTK_NOTEBOOK(Closure->notebook), 1);

	     Closure->additionalSpiralColor = 1;
	     ResetLinearReadWindow();
	     CreateGThread((GThreadFunc)ReadMediumLinear, (gpointer)0);
	}
        break;

      case ACTION_CREATE:
      case ACTION_CREATE_CONT:
	method = FindMethod(Closure->methodName); 
	if(!method) 
	{  CreateMessage(_("\nMethod %s not available.\n"
			   "Use -m without parameters for a method list.\n"), 
			 GTK_MESSAGE_ERROR, Closure->methodName);
	   break;
	}

	gtk_notebook_set_current_page(GTK_NOTEBOOK(Closure->notebook), method->tabWindowIndex);
	method->resetCreateWindow(method);
	AllowActions(FALSE);
	CreateGThread((GThreadFunc)CreateEcc, (gpointer)NULL);
        break;

      case ACTION_FIX:
	ClearCrcCache();
	if(!(method = EccFileMethod(TRUE)))
	   break;

	gtk_notebook_set_current_page(GTK_NOTEBOOK(Closure->notebook),  method->tabWindowIndex+1);
	method->resetFixWindow(method);
	AllowActions(FALSE);
	CreateGThread((GThreadFunc)method->fix, (gpointer)method);
        break;

      case ACTION_SCAN:
	ClearCrcCache();
	gtk_notebook_set_current_page(GTK_NOTEBOOK(Closure->notebook), 1);
	Closure->additionalSpiralColor = -1;
	ResetLinearReadWindow();
	AllowActions(FALSE);
	CreateGThread((GThreadFunc)ReadMediumLinear, (gpointer)1);
        break;

      case ACTION_VERIFY:  
	/* If something is wrong with the .iso or .ecc files
	   we fall back to the RS01 method for verifying since it is robust
	   against missing files. */
	if(!(method = EccFileMethod(FALSE)))
	  if(!(method = FindMethod("RS01")))
	     break;

	gtk_notebook_set_current_page(GTK_NOTEBOOK(Closure->notebook), method->tabWindowIndex+2);
	method->resetVerifyWindow(method);
	AllowActions(FALSE);
	CreateGThread((GThreadFunc)method->verify, (gpointer)method);
        break;
   }
}


/*
 * Start an action from another thread
 */

static gboolean action_idle_func(gpointer action)
{  
   Closure->enableCurveSwitch = TRUE;
   action_cb(NULL, action);

   return FALSE;
}


void ContinueWithAction(int action)
{
   g_idle_add(action_idle_func, GINT_TO_POINTER(action));
}

/* 
 * Create the action buttons and the associated notebook pages
 */

static GtkWidget *create_button(char *label, char *icon)
{  GtkWidget *button,*box,*image,*lab;
   char *utf_label = g_locale_to_utf8(label, -1, NULL, NULL, NULL);
 

   button = gtk_button_new();
   box    = gtk_vbox_new(FALSE, 0);
   image  = gtk_image_new_from_stock(icon, GTK_ICON_SIZE_LARGE_TOOLBAR);
   lab    = gtk_label_new(utf_label);
   g_free(utf_label);

   gtk_box_pack_start(GTK_BOX(box), image, FALSE, FALSE, 0);
   gtk_box_pack_start(GTK_BOX(box), lab, FALSE, FALSE, 0);

   gtk_container_add(GTK_CONTAINER(button), box);
   //   gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);

   return button;
}

static GtkWidget* create_action_bar(GtkNotebook *notebook)
{  GtkWidget *vbox, *outer_vbox, *wid, *content, *ignore;
   int window_number = FIRST_CREATE_WINDOW; 
   unsigned int i;  

   outer_vbox = gtk_vbox_new(TRUE, 0);
   vbox = gtk_vbox_new(FALSE, 0);   /* needed for vertical spacing */
   gtk_box_pack_start(GTK_BOX(outer_vbox), vbox, TRUE, TRUE, 3);

   /*** Read */

   Closure->readButton = wid = create_button(_("button|Read"), "dvdisaster-read");
   g_signal_connect(G_OBJECT(wid), "clicked", G_CALLBACK(action_cb), (gpointer)ACTION_READ);
   gtk_box_pack_start(GTK_BOX(vbox), wid, FALSE, FALSE, 0);
   AttachTooltip(wid, _("tooltip|Read Image"), _("Reads a CD/DVD image into a file (or tries to complete an existing image file)."));

   content = gtk_vbox_new(FALSE, 0);   /* read linear window */
   ignore = gtk_label_new("read_tab_l");
   gtk_notebook_append_page(notebook, content, ignore);
   CreateLinearReadWindow(content);

   content = gtk_vbox_new(FALSE, 0);   /* read adaptive window */
   ignore = gtk_label_new("read_tab_a");
   gtk_notebook_append_page(notebook, content, ignore);
   CreateAdaptiveReadWindow(content);

   /*** Create */

   Closure->createButton = wid = create_button(_("button|Create"), "dvdisaster-create");
   g_signal_connect(G_OBJECT(wid), "clicked", G_CALLBACK(action_cb), (gpointer)ACTION_CREATE);
   gtk_box_pack_start(GTK_BOX(vbox), wid, FALSE, FALSE, 0);
   AttachTooltip(wid, _("tooltip|Create error correction data"), _("Creates error correction data. Requires an image file."));

   /*** Scan */

   Closure->scanButton = wid = create_button(_("button|Scan"), "dvdisaster-scan");
   g_signal_connect(G_OBJECT(wid), "clicked", G_CALLBACK(action_cb), (gpointer)ACTION_SCAN);
   gtk_box_pack_start(GTK_BOX(vbox), wid, FALSE, FALSE, 0);
   AttachTooltip(wid, _("tooltip|Scan medium"), _("Scans medium for unreadable sectors."));

   /*** Fix */

   Closure->fixButton = wid = create_button(_("button|Fix"), "dvdisaster-fix");
   g_signal_connect(G_OBJECT(wid), "clicked", G_CALLBACK(action_cb), (gpointer)ACTION_FIX);
   gtk_box_pack_start(GTK_BOX(vbox), wid, FALSE, FALSE, 0);
   AttachTooltip(wid, _("tooltip|Repair image"), _("Repairs an image. Requires an image file and error correction data."));

   /*** Verify */

   Closure->testButton = wid = create_button(_("button|Verify"), "dvdisaster-verify");
   g_signal_connect(G_OBJECT(wid), "clicked", G_CALLBACK(action_cb), (gpointer)ACTION_VERIFY);
   gtk_box_pack_start(GTK_BOX(vbox), wid, FALSE, FALSE, 0);
   AttachTooltip(wid, _("tooltip|Consistency check"), _("Tests consistency of error correction data and image file."));

   /*** Stop */

   wid = create_button(_("button|Stop"), "gtk-stop");
   g_signal_connect(G_OBJECT(wid), "clicked", G_CALLBACK(action_cb), (gpointer)ACTION_STOP);
   gtk_box_pack_end(GTK_BOX(vbox), wid, FALSE, FALSE, 0);
   AttachTooltip(wid, _("tooltip|Abort action"), _("Aborts an ongoing action."));

   /*** Block drive related actions if no drives were found */

   if(!Closure->deviceNodes->len)
   {  gtk_widget_set_sensitive(Closure->readButton, FALSE);
      gtk_widget_set_sensitive(Closure->scanButton, FALSE);
   }

   /*** Create notebook windows for the methods */

   for(i=0; i<Closure->methodList->len; i++)
   {  Method *method = g_ptr_array_index(Closure->methodList, i);

      method->tabWindowIndex = window_number;
      window_number += 3;

      /* Create window */

      content = gtk_vbox_new(FALSE, 0);
      ignore = gtk_label_new("create_tab");
      gtk_notebook_append_page(notebook, content, ignore);
      method->createCreateWindow(method, content);

      /* Fix window */

      content = gtk_vbox_new(FALSE, 0);
      ignore = gtk_label_new("fix_tab");
      gtk_notebook_append_page(notebook, content, ignore);
      method->createFixWindow(method, content);

      /* Verify window */

      content = gtk_vbox_new(FALSE, 0);
      ignore = gtk_label_new("verify_tab");
      gtk_notebook_append_page(notebook, content, ignore);
      method->createVerifyWindow(method, content);

   }

   return outer_vbox;
}

/***
 *** Create the main window 
 ***/

/*
 * Show the log data
 */

static void log_cb(GtkWidget *widget, gpointer data)
{  ShowLog();
}

void CreateMainWindow(int *argc, char ***argv)
{   GtkWidget *window,*wid,*outer_box,*middle_box,*status_box,*sep;
    GtkWidget *box, *icon, *button;
    char title[80];
    int sig_okay = TRUE;

    /*** Initialize GTK+ */

    gtk_init(argc, argv);

    /*** Some style tinkering */

    gtk_rc_parse_string("style \"dvdisaster-style\"\n"
                       "{  GtkMenuBar::shadow_type = none\n"
                       "}\n"

                       "class \"GtkMenuBar\" style \"dvdisaster-style\"\n");

    /*** Create our icons */

    CreateIconFactory();

    /*** Open the main window */

    if(Closure->screenShotMode) 
         g_snprintf(title, 80, "dvdisaster-%s", VERSION);
    else g_snprintf(title, 80, "dvdisaster-%s", Closure->cookedVersion);

#ifdef SYS_MINGW
    sig_okay = VerifySignature();
#endif

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), title);
    if(sig_okay)
      gtk_window_set_default_size(GTK_WINDOW(window), -1, 550);
    gtk_window_set_icon(GTK_WINDOW(window), Closure->windowIcon);
    Closure->window = GTK_WINDOW(window);

    /* Connect with the close button from the window manager */

    g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(delete_cb), NULL);

    /* and with destroy events */

    g_signal_connect(window, "destroy", G_CALLBACK(destroy_cb), NULL);

    /*** Validate our binary directory */

#ifdef SYS_MINGW
    if(!sig_okay)
    {  GtkWidget *lab, *icon, *button;
       char *msg, *utf, version[80];

       if(Closure->version % 100)
            sprintf(version, "dvdisaster-%s.%d-setup.exe", VERSION, Closure->version%100);
       else sprintf(version, "dvdisaster-%s-setup.exe", VERSION);

       gtk_container_set_border_width(GTK_CONTAINER(window), 10);

       outer_box = gtk_hbox_new(FALSE, 10);
       gtk_container_add(GTK_CONTAINER(window), outer_box);

       middle_box = gtk_vbox_new(FALSE, 0);
       gtk_box_pack_start(GTK_BOX(outer_box), middle_box, FALSE, FALSE, 0);

       icon = gtk_image_new_from_stock(GTK_STOCK_DIALOG_ERROR, GTK_ICON_SIZE_DIALOG);
       gtk_box_pack_start(GTK_BOX(middle_box), icon, FALSE, FALSE, 0);

       middle_box = gtk_vbox_new(FALSE, 0);
       gtk_box_pack_start(GTK_BOX(outer_box), middle_box, FALSE, FALSE, 0);

       lab = gtk_label_new(NULL);
       gtk_box_pack_start(GTK_BOX(middle_box), lab, FALSE, FALSE, 0);
       msg = g_strdup_printf(_("<b>dvdisaster is not properly installed</b>\n\n"
			       "Please execute the installer program (%s) again.\n"), version);
       utf = g_locale_to_utf8(msg, -1, NULL, NULL, NULL);
       gtk_label_set_markup(GTK_LABEL(lab), utf);
       g_free(msg);
       g_free(utf);

       box = gtk_hbox_new(FALSE, 0);
       gtk_box_pack_start(GTK_BOX(middle_box), box, FALSE, FALSE, 0);

       button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
       gtk_box_pack_end(GTK_BOX(box), button, FALSE, FALSE, 0);

       g_signal_connect_swapped(G_OBJECT (button), "clicked",
				G_CALLBACK(gtk_widget_destroy),
				G_OBJECT(window));
       gtk_widget_show_all(window);
       gtk_main();

       return;
    }
#endif

    /*** Initialize the tooltips struct */

    Closure->tooltips = gtk_tooltips_new();

    /*** Create the sub parts of the GUI */

    outer_box = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), outer_box);

    /* Menu and tool bar */

    wid = CreateMenuBar(outer_box);
    gtk_box_pack_start(GTK_BOX(outer_box), wid, FALSE, FALSE, 0);

    sep = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(outer_box), sep, FALSE, FALSE, 0);

    wid = CreateToolBar(outer_box);
    gtk_box_pack_start(GTK_BOX(outer_box), wid, FALSE, FALSE, 3);

    /* Middle part */

    sep = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(outer_box), sep, FALSE, FALSE, 0);

    middle_box = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(outer_box), middle_box, TRUE, TRUE, 0);

    wid = Closure->notebook = gtk_notebook_new();
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(wid), FALSE);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(wid), FALSE);
    gtk_box_pack_start(GTK_BOX(middle_box), wid, TRUE, TRUE, 0);

    CreateWelcomePage(GTK_NOTEBOOK(Closure->notebook));

    wid = create_action_bar((GTK_NOTEBOOK(Closure->notebook)));
    gtk_box_pack_end(GTK_BOX(middle_box), wid, FALSE, FALSE, 3);

    sep = gtk_vseparator_new();
    gtk_box_pack_end(GTK_BOX(middle_box), sep, FALSE, FALSE, 0);

    /* Status bar enclosure */

    status_box = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_end(GTK_BOX(outer_box), status_box, FALSE, FALSE, 0);

    sep = gtk_hseparator_new();
    gtk_box_pack_end(GTK_BOX(outer_box), sep, FALSE, FALSE, 0);

    /* Status bar contents. */

    Closure->status = GTK_LABEL(gtk_label_new(NULL));
    gtk_label_set_ellipsize(GTK_LABEL(Closure->status), PANGO_ELLIPSIZE_END);
    gtk_misc_set_alignment(GTK_MISC(Closure->status), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(status_box), GTK_WIDGET(Closure->status), TRUE, TRUE, 5);

    button = gtk_button_new();
    gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
    gtk_box_pack_end(GTK_BOX(status_box), button, FALSE, FALSE, 5);
    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(log_cb), NULL);
    AttachTooltip(button, 
		  _("tooltip|Protocol for current action"), 
		  _("Displays additional information created during the current or last action."));

    box = gtk_hbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(button), box);

    icon = gtk_image_new_from_stock(GTK_STOCK_INDEX, GTK_ICON_SIZE_SMALL_TOOLBAR);

    gtk_box_pack_start(GTK_BOX(box), icon, FALSE, FALSE, 2);

    wid = gtk_label_new(_utf("View log"));
    gtk_box_pack_start(GTK_BOX(box), wid, FALSE, FALSE, 0);

    /* And enter the main loop */

    gtk_widget_show_all(window);
    gtk_main();
}
