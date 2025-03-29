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

#include <limits.h>

#ifndef PATH_MAX
  #define PATH_MAX 4096
#endif

/***
 *** Forward declarations
 ***/

static void file_select_cb(GtkWidget*, gpointer);

/***
 *** The Menu and Toolbar action dispatcher
 ***/

typedef enum 
{  MENU_FILE_IMAGE,
     MENU_FILE_IMAGE_OK,
     MENU_FILE_IMAGE_CANCEL,
     MENU_FILE_IMAGE_DESTROY,
   MENU_FILE_ECC,
     MENU_FILE_ECC_OK,
     MENU_FILE_ECC_CANCEL,
     MENU_FILE_ECC_DESTROY,
   MENU_FILE_QUIT,

   MENU_TOOLS_MEDIUM_INFO,
   MENU_TOOLS_RAW_EDITOR,

   MENU_PREFERENCES,

   MENU_HELP_MANUAL,

   MENU_HELP_ABOUT,
   MENU_HELP_GPL,
   MENU_HELP_CHANGELOG,
   MENU_HELP_CREDITS,
   MENU_HELP_TODO
} menu_actions;

static void menu_cb(GtkWidget *widget, gpointer data)
{  
   switch(GPOINTER_TO_INT(data))
   {  case MENU_FILE_IMAGE:
      case MENU_FILE_ECC:
        file_select_cb(widget, data);
        break;

      case MENU_FILE_QUIT:
	/* If an action is currently running with spawned threads,
	   give them time to terminate cleanly. */
   
	if(Closure->subThread)
	{  Closure->stopActions = STOP_SHUTDOWN_ALL;

	   g_thread_join(Closure->subThread);
	}

	/* Extract current file selections so that they are saved in the .dvdisaster file */
   
	g_free(Closure->imageName);
	Closure->imageName = g_strdup(gtk_entry_get_text(GTK_ENTRY(Closure->imageEntry)));
	if(!Closure->imageName || !strlen(Closure->imageName))
	{  if(Closure->imageName) g_free(Closure->imageName);
	   Closure->imageName = g_strdup("none");
	}

	g_free(Closure->eccName);
	Closure->eccName = g_strdup(gtk_entry_get_text(GTK_ENTRY(Closure->eccEntry)));
	if(!Closure->eccName || !strlen(Closure->eccName))
	{  if(Closure->eccName) g_free(Closure->eccName);
	   Closure->eccName = g_strdup("none");
	}

	/* and quit */

        gtk_main_quit();
        break;

      case MENU_TOOLS_MEDIUM_INFO:
	 GuiCreateMediumInfoWindow();
	 break;

      case MENU_TOOLS_RAW_EDITOR:
	 GuiCreateRawEditor();
	 break;

      case MENU_PREFERENCES:
	GuiCreatePreferencesWindow();
	break;

      case MENU_HELP_MANUAL:
	GuiShowURL("manual.pdf");
	break;

      case MENU_HELP_ABOUT:
        GuiAboutDialog();
        break;

      case MENU_HELP_GPL:
        GuiShowGPL();
        break;

      case MENU_HELP_CHANGELOG:
        GuiShowTextfile(_("windowtitle|Change log"), 
			_("<big>Change log</big>\n"
			  "<i>Major differences from earlier program versions.</i>"),
			"CHANGELOG", NULL, NULL);
	break;

      case MENU_HELP_CREDITS:
        GuiShowTextfile(_("windowtitle|Credits"), 
			_("<big>Credits</big>\n"
			  "<i>Thanks go out to...</i>"),
			"CREDITS", NULL, NULL);
	break;

      case MENU_HELP_TODO:
	GuiShowTextfile(_("windowtitle|To do list"), 
			_("<big>To do list</big>\n"
			  "<i>A sneak preview of coming features ... perhaps ;-)</i>"),
			"TODO", NULL, NULL);
	break;

      default:
        g_print("Menu/Toolbar action %d\n",GPOINTER_TO_INT(data));
        break;
  }
}

/***
 *** The Menu system
 ***/

/*
 * Helper functions for creating the menu system
 */

static GtkWidget* add_menu_button(GtkWidget *parent, char *title, int action)
{  char *utf_title = g_locale_to_utf8(title, -1, NULL, NULL, NULL);
   GtkWidget *item;

   item = gtk_menu_item_new_with_label(utf_title);
   g_free(utf_title);
   gtk_menu_shell_append(GTK_MENU_SHELL(parent), item);
   g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(menu_cb), GINT_TO_POINTER(action));

   return item;
}

static void add_menu_separator(GtkWidget *parent)
{  GtkWidget *sep;

   sep = gtk_separator_menu_item_new();
   gtk_menu_shell_append(GTK_MENU_SHELL(parent), sep);
}

#if 0
static void append_sub_menu(GtkWidget *parent, GtkWidget *strip, char *name)
{  char *utf_name = g_locale_to_utf8(name, -1, NULL, NULL, NULL);
   GtkWidget *anchor;

   anchor = gtk_menu_item_new_with_label(utf_name);
   g_free(utf_name);
   gtk_menu_item_set_submenu(GTK_MENU_ITEM(anchor), strip);
   gtk_menu_shell_append(GTK_MENU_SHELL(parent), anchor);
}
#endif

/*
 * Assemble the menu system.
 * Using the itemfactory would make things more complicated wrt localization.
 */

GtkWidget *GuiCreateMenuBar(GtkWidget *parent)
{  GtkWidget *menu_bar, *menu_anchor, *menu_strip, *item;

   /* The overall menu bar */

   menu_bar = gtk_menu_bar_new();
   //   gtk_widget_set_name(menu_bar, "menu-bar");

   /* The file menu */

   menu_strip = gtk_menu_new();

   Closure->fileMenuImage = add_menu_button(menu_strip, _("menu|Select Image"), MENU_FILE_IMAGE);
   Closure->fileMenuEcc   = add_menu_button(menu_strip, _("menu|Select Parity File"), MENU_FILE_ECC);
   add_menu_button(menu_strip, _("menu|Quit"), MENU_FILE_QUIT);

   menu_anchor = gtk_menu_item_new_with_label(_utf("menu|File"));
   gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_anchor), menu_strip);
   gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_anchor);

   /* The tools menu */

   menu_strip = gtk_menu_new();
   item = add_menu_button(menu_strip, _("menu|Medium info"), MENU_TOOLS_MEDIUM_INFO);
   if(!Closure->deviceNodes->len)
     gtk_widget_set_sensitive(item, FALSE);

   if(Closure->debugMode && !Closure->screenShotMode)
      add_menu_button(menu_strip, _("menu|Raw sector editor"), MENU_TOOLS_RAW_EDITOR);
   
   Closure->toolMenuAnchor = menu_anchor = gtk_menu_item_new_with_label(_utf("menu|Tools"));
   gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_anchor), menu_strip);
   gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_anchor);

   /* The help menu */

   menu_strip = gtk_menu_new();

   add_menu_button(menu_strip, _("menu|About"), MENU_HELP_ABOUT);
   add_menu_button(menu_strip, _("menu|User manual"), MENU_HELP_MANUAL);

   add_menu_separator(menu_strip);

   add_menu_button(menu_strip, _("menu|Credits"), MENU_HELP_CREDITS);
   add_menu_button(menu_strip, _("menu|Licence (GPL)"), MENU_HELP_GPL);

   add_menu_separator(menu_strip);

   add_menu_button(menu_strip, _("menu|Change log"), MENU_HELP_CHANGELOG);
   add_menu_button(menu_strip, _("menu|To do list"), MENU_HELP_TODO);

   menu_anchor = gtk_menu_item_new_with_label(_utf("menu|Help"));
   gtk_menu_item_right_justify(GTK_MENU_ITEM(menu_anchor));
   gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_anchor), menu_strip);
   gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_anchor);

   return menu_bar;
}

/***
 *** The toolbar
 ***/

void GuiAttachTooltip(GtkWidget *widget, char *short_descr)
{  char *short_copy = g_locale_to_utf8(short_descr, -1, NULL, NULL, NULL);

   gtk_widget_set_tooltip_text(widget, short_copy);

   g_free(short_copy);
}


/*
 * Callback for drive selection
 */

static void drive_select_cb(GtkWidget *widget, gpointer data)
{  int n;
   char *dnode;

   if(!Closure->deviceNodes->len)  /* No drives available */
     return;

   n = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));

   if(n<0)
     return;

   dnode = g_ptr_array_index(Closure->deviceNodes, n);
   g_free(Closure->device);
   Closure->device = g_strdup(dnode);

   if(Closure->mediumDrive) /* propagate to medium info window */
     gtk_combo_box_set_active(GTK_COMBO_BOX(Closure->mediumDrive), n);
}

/*
 * Callback for the image and ecc file selection.
 * Creates and runs the file selection dialogs.
 */

static void file_select_cb(GtkWidget *widget, gpointer data)
{  int action = GPOINTER_TO_INT(data);

   switch(action)
   {  /*** Image file selection */

      case MENU_FILE_IMAGE:
	 if(!Closure->imageFileSel)
         {  Closure->imageFileSel = gtk_file_selection_new(_utf("windowtitle|Image file selection"));
	    GuiReverseCancelOK(GTK_DIALOG(Closure->imageFileSel));
            g_signal_connect(G_OBJECT(Closure->imageFileSel), "destroy",
	                     G_CALLBACK(file_select_cb), GINT_TO_POINTER(MENU_FILE_IMAGE_DESTROY));
            g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(Closure->imageFileSel)->ok_button),"clicked", 
	                     G_CALLBACK(file_select_cb), GINT_TO_POINTER(MENU_FILE_IMAGE_OK));
    
            g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(Closure->imageFileSel)->cancel_button),"clicked", 
	                     G_CALLBACK(file_select_cb), GINT_TO_POINTER(MENU_FILE_IMAGE_CANCEL));
         }
	 gtk_file_selection_set_filename(GTK_FILE_SELECTION(Closure->imageFileSel),
					 gtk_entry_get_text(GTK_ENTRY(Closure->imageEntry)));
	 gtk_widget_show(Closure->imageFileSel);
	 break;

      case MENU_FILE_IMAGE_DESTROY:
	 Closure->imageFileSel = NULL;
	 break;

      case MENU_FILE_IMAGE_OK:
	 g_free(Closure->imageName);
	 Closure->imageName = g_strdup(gtk_file_selection_get_filename(GTK_FILE_SELECTION(Closure->imageFileSel)));
	 if(Closure->autoSuffix)
	   Closure->imageName = ApplyAutoSuffix(Closure->imageName, "iso");
	 gtk_entry_set_text(GTK_ENTRY(Closure->imageEntry), Closure->imageName);
	 gtk_editable_set_position(GTK_EDITABLE(Closure->imageEntry), -1);
	 gtk_widget_hide(Closure->imageFileSel);
	 break;

      case MENU_FILE_IMAGE_CANCEL:
	 gtk_widget_hide(Closure->imageFileSel);
	 break;

      /*** Same stuff again for ecc file selection */

      case MENU_FILE_ECC:
	 if(!Closure->eccFileSel)
         {  Closure->eccFileSel = gtk_file_selection_new(_utf("windowtitle|Error correction file selection"));
	    GuiReverseCancelOK(GTK_DIALOG(Closure->eccFileSel));
            g_signal_connect(G_OBJECT(Closure->eccFileSel), "destroy",
	                     G_CALLBACK(file_select_cb), GINT_TO_POINTER(MENU_FILE_ECC_DESTROY));
            g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(Closure->eccFileSel)->ok_button),"clicked", 
	                     G_CALLBACK(file_select_cb), GINT_TO_POINTER(MENU_FILE_ECC_OK));
    
            g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(Closure->eccFileSel)->cancel_button),"clicked", 
	                     G_CALLBACK(file_select_cb), GINT_TO_POINTER(MENU_FILE_ECC_CANCEL));
         }
	 gtk_file_selection_set_filename(GTK_FILE_SELECTION(Closure->eccFileSel),
					 gtk_entry_get_text(GTK_ENTRY(Closure->eccEntry)));
	 gtk_widget_show(Closure->eccFileSel);
	 break;

      case MENU_FILE_ECC_DESTROY:
	 Closure->eccFileSel = NULL;
	 break;

      case MENU_FILE_ECC_OK:
	 g_free(Closure->eccName);
	 Closure->eccName = g_strdup(gtk_file_selection_get_filename(GTK_FILE_SELECTION(Closure->eccFileSel)));
	 if(Closure->autoSuffix)
	   Closure->eccName = ApplyAutoSuffix(Closure->eccName, "ecc");
	 gtk_entry_set_text(GTK_ENTRY(Closure->eccEntry), Closure->eccName);
	 gtk_editable_set_position(GTK_EDITABLE(Closure->eccEntry), -1);
	 gtk_widget_hide(Closure->eccFileSel);
	 break;

      case MENU_FILE_ECC_CANCEL:
	 gtk_widget_hide(Closure->eccFileSel);
	 break;
   }
}

/*
 * Set file path for a text entry.
 * Completes relative paths. 
 */

void set_path(GtkWidget *entry, char *path)
{
  if(path[0] == '/' || path[0] == '\\' || path[1] == ':' || strlen(path) < 1)
   {  gtk_entry_set_text(GTK_ENTRY(entry), path);
      gtk_editable_set_position(GTK_EDITABLE(entry), -1);
   }
   else
   {  char buf[PATH_MAX + strlen(path) + 2];

      if(!getcwd(buf, PATH_MAX)) return;
      strcat(buf,"/");

      strcat(buf,path);
      gtk_entry_set_text(GTK_ENTRY(entry), buf);
      gtk_editable_set_position(GTK_EDITABLE(entry), -1);
   }
}

/*
 * Callback for adding file suffixes
 */

static void suffix_cb(GtkWidget *widget, gpointer data)
{  int ecc_file = GPOINTER_TO_INT(data);

   if(!Closure->autoSuffix)
     return;

   if(!ecc_file)
   {  Closure->imageName = g_strdup(gtk_entry_get_text(GTK_ENTRY(Closure->imageEntry)));
      Closure->imageName = ApplyAutoSuffix(Closure->imageName, "iso");
      gtk_entry_set_text(GTK_ENTRY(Closure->imageEntry), Closure->imageName);
   }
   else
   {  Closure->eccName = g_strdup(gtk_entry_get_text(GTK_ENTRY(Closure->eccEntry)));
      Closure->eccName = ApplyAutoSuffix(Closure->eccName, "ecc");
      gtk_entry_set_text(GTK_ENTRY(Closure->eccEntry), Closure->eccName);
   }
}

/*
 * Create the toolbar
 */

GtkWidget *GuiCreateToolBar(GtkWidget *parent)
{  GtkWidget *box, *button, *ebox, *icon, *prefs, *help, *quit, *sep, *space;
   GtkWidget *combo_box;
   int dev_idx = 0;
   unsigned int i;

   /*** Create the toolbar */

   box = gtk_hbox_new(FALSE, 0);

   /*** Drive selection */

   space = gtk_label_new(NULL);
   gtk_box_pack_start(GTK_BOX(box), space, FALSE, FALSE, 5);

   ebox = gtk_event_box_new();
   gtk_widget_set_events(ebox, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
   gtk_box_pack_start(GTK_BOX(box), ebox, FALSE, FALSE, 0);
   GuiAttachTooltip(ebox, _("tooltip|Drive selection"));
   icon = gtk_image_new_from_stock("dvdisaster-cd", GTK_ICON_SIZE_LARGE_TOOLBAR);
   gtk_container_add(GTK_CONTAINER(ebox), icon);

   Closure->driveCombo = combo_box = gtk_combo_box_text_new();

   g_signal_connect(G_OBJECT(combo_box), "changed", G_CALLBACK(drive_select_cb), NULL);

   for(i=0; i<Closure->deviceNames->len; i++)   
   {
     gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box),
			       g_ptr_array_index(Closure->deviceNames,i));

     if(!strcmp(Closure->device, g_ptr_array_index(Closure->deviceNodes,i)))
       dev_idx = i;
   }

   if(!Closure->deviceNodes->len)
   {   gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box), _utf("No drives found"));
   }

   gtk_combo_box_set_active(GTK_COMBO_BOX(combo_box), dev_idx);
   gtk_widget_set_size_request(combo_box, 200, -1);
   gtk_box_pack_start(GTK_BOX(box), combo_box, FALSE, FALSE, 7);
   GuiAttachTooltip(combo_box, _("tooltip|Drive selection"));

   space = gtk_label_new(NULL);
   gtk_box_pack_start(GTK_BOX(box), space, FALSE, FALSE, 1);

   sep = gtk_vseparator_new();
   gtk_box_pack_start(GTK_BOX(box), sep, FALSE, FALSE, 3);

   /*** Image file selection */

   icon = gtk_image_new_from_stock("dvdisaster-open-img", GTK_ICON_SIZE_LARGE_TOOLBAR);
   button = gtk_button_new();
   gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
   gtk_container_add(GTK_CONTAINER(button), icon);
   g_signal_connect(G_OBJECT(button), "clicked", 
	            G_CALLBACK(file_select_cb), 
	            GINT_TO_POINTER(MENU_FILE_IMAGE));
   gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);

   Closure->imageEntry = gtk_entry_new();
   set_path(Closure->imageEntry, Closure->imageName);
   g_signal_connect(G_OBJECT(Closure->imageEntry), "activate", 
	            G_CALLBACK(suffix_cb), GINT_TO_POINTER(FALSE));
   gtk_box_pack_start(GTK_BOX(box), Closure->imageEntry, TRUE, TRUE, 0);

   space = gtk_label_new(NULL);
   gtk_box_pack_start(GTK_BOX(box), space, FALSE, FALSE, 5);

   sep = gtk_vseparator_new();
   gtk_box_pack_start(GTK_BOX(box), sep, FALSE, FALSE, 3);
   GuiAttachTooltip(button, _("tooltip|Image file selection"));
   GuiAttachTooltip(Closure->imageEntry, _("tooltip|Current image file"));

   /*** Ecc file selection */

   icon = gtk_image_new_from_stock("dvdisaster-open-ecc", GTK_ICON_SIZE_LARGE_TOOLBAR);
   button = gtk_button_new();
   gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
   gtk_container_add(GTK_CONTAINER(button), icon);
   g_signal_connect(G_OBJECT(button), "clicked", 
	            G_CALLBACK(file_select_cb), 
	            GINT_TO_POINTER(MENU_FILE_ECC));
   gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);

   Closure->eccEntry = gtk_entry_new();
   set_path(Closure->eccEntry, Closure->eccName);
   g_signal_connect(G_OBJECT(Closure->eccEntry), "activate", 
	            G_CALLBACK(suffix_cb), GINT_TO_POINTER(TRUE));
   gtk_box_pack_start(GTK_BOX(box), Closure->eccEntry, TRUE, TRUE, 0);

   space = gtk_label_new(NULL);
   gtk_box_pack_start(GTK_BOX(box), space, FALSE, FALSE, 5);

   sep = gtk_vseparator_new();
   gtk_box_pack_start(GTK_BOX(box), sep, FALSE, FALSE, 3);
   GuiAttachTooltip(button, _("tooltip|Error correction file selection"));
   GuiAttachTooltip(Closure->eccEntry, _("tooltip|Current error correction file"));

   /*** Preferences button */

   icon = gtk_image_new_from_stock("dvdisaster-gtk-preferences", GTK_ICON_SIZE_LARGE_TOOLBAR);
   Closure->prefsButton = prefs = gtk_button_new();
   gtk_button_set_relief(GTK_BUTTON(prefs), GTK_RELIEF_NONE);
   gtk_container_add(GTK_CONTAINER(prefs), icon);
   g_signal_connect(G_OBJECT(prefs), "clicked", G_CALLBACK(menu_cb), (gpointer)MENU_PREFERENCES);
   gtk_box_pack_start(GTK_BOX(box), prefs, FALSE, FALSE, 0);
   GuiAttachTooltip(prefs, _("tooltip|Preferences"));

   /*** Help button */

   icon = gtk_image_new_from_stock("dvdisaster-gtk-help", GTK_ICON_SIZE_LARGE_TOOLBAR);
   Closure->helpButton = help = gtk_button_new();
   gtk_button_set_relief(GTK_BUTTON(help), GTK_RELIEF_NONE);
   gtk_container_add(GTK_CONTAINER(help), icon);
   g_signal_connect(G_OBJECT(help), "clicked", G_CALLBACK(menu_cb), (gpointer)MENU_HELP_MANUAL);
   gtk_box_pack_start(GTK_BOX(box), help, FALSE, FALSE, 0);
   GuiAttachTooltip(help, _("tooltip|User manual"));

   /*** Quit button */

   icon = gtk_image_new_from_stock("dvdisaster-gtk-quit", GTK_ICON_SIZE_LARGE_TOOLBAR);
   quit = gtk_button_new();
   gtk_button_set_relief(GTK_BUTTON(quit), GTK_RELIEF_NONE);
   gtk_container_add(GTK_CONTAINER(quit), icon);
   g_signal_connect(G_OBJECT(quit), "clicked", G_CALLBACK(menu_cb), (gpointer)MENU_FILE_QUIT);
   gtk_box_pack_start(GTK_BOX(box), quit, FALSE, FALSE, 0);
   GuiAttachTooltip(quit, _("tooltip|Quit"));

   return box;
}
#endif /* WITH_GUI_YES */
