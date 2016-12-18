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

#include "rs03-includes.h"

/***
 *** Create the preferences page for setting redundancy etc.
 ***/

enum 
{  PREF_NROOTS = 0,
   PREF_PRELOAD = 1,
   PREF_THREADS = 2
};

static int prefetch_size[] = { 32, 64, 96, 128, 192, 256, 384, 512, 768, 1024 };
static int threads_count[] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,20,24,28,31,32 };

static void activate_toggle_button(GtkToggleButton *toggle, int state)
{  if(toggle) gtk_toggle_button_set_active(toggle, state);
}

static void set_range_value(GtkRange *range, int value)
{  if(range) gtk_range_set_value(range, value);
}

static void set_spin_button_value(GtkSpinButton *spin, int value)
{  if(spin) gtk_spin_button_set_value(spin, value);
}

static void set_sensitive(GtkWidget *widget, int value)
{
  if(widget) gtk_widget_set_sensitive(widget, value);
}

/*
 * Ecc storage method selection 
 */

static void eccmethod_cb(GtkWidget *widget, gpointer data)
{  RS03Widgets *wl = (RS03Widgets*)data;
   int state  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

   if(!state)  /* only track changes to activate state */
     return;

   if(widget == wl->eccFileA || widget == wl->eccFileB)
   {  Closure->eccTarget = ECC_FILE;

      activate_toggle_button(GTK_TOGGLE_BUTTON(wl->eccFileA), TRUE); 
      activate_toggle_button(GTK_TOGGLE_BUTTON(wl->eccFileB), TRUE); 

      set_sensitive(wl->radio1A, TRUE);
      set_sensitive(wl->radio1B, TRUE);

      gtk_notebook_set_current_page(GTK_NOTEBOOK(wl->redundancyNotebook), 1);
   }

   if(widget == wl->eccImageA || widget == wl->eccImageB)
   {  Closure->eccTarget = ECC_IMAGE;

      activate_toggle_button(GTK_TOGGLE_BUTTON(wl->eccImageA), TRUE); 
      activate_toggle_button(GTK_TOGGLE_BUTTON(wl->eccImageB), TRUE); 

      set_sensitive(wl->radio1A, FALSE);
      set_sensitive(wl->radio1B, FALSE);

      gtk_notebook_set_current_page(GTK_NOTEBOOK(wl->redundancyNotebook), 0);
   }
}

/*
 * Codec type selection 
 */

static void encoding_alg_cb(GtkWidget *widget, gpointer data)
{  RS03Widgets *wl = (RS03Widgets*)data;
   int state  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

   if(!state)  /* only track changes to activate state */
     return;

   if(widget == wl->eaRadio1A || widget == wl->eaRadio1B)
   {  Closure->encodingAlgorithm = ENCODING_ALG_32BIT;

      activate_toggle_button(GTK_TOGGLE_BUTTON(wl->eaRadio1A), TRUE); 
      activate_toggle_button(GTK_TOGGLE_BUTTON(wl->eaRadio1B), TRUE); 
   }

   if(widget == wl->eaRadio2A || widget == wl->eaRadio2B)
   {  Closure->encodingAlgorithm = ENCODING_ALG_64BIT;

      activate_toggle_button(GTK_TOGGLE_BUTTON(wl->eaRadio2A), TRUE); 
      activate_toggle_button(GTK_TOGGLE_BUTTON(wl->eaRadio2B), TRUE); 
   }

   if(widget == wl->eaRadio3A || widget == wl->eaRadio3B)
   { 
#ifdef HAVE_SSE2 
      Closure->encodingAlgorithm = ENCODING_ALG_SSE2;
#endif
#ifdef HAVE_ALTIVEC 
      Closure->encodingAlgorithm = ENCODING_ALG_ALTIVEC;
#endif

      activate_toggle_button(GTK_TOGGLE_BUTTON(wl->eaRadio3A), TRUE); 
      activate_toggle_button(GTK_TOGGLE_BUTTON(wl->eaRadio3B), TRUE); 
   }

   if(widget == wl->eaRadio4A || widget == wl->eaRadio4B)
   {  Closure->encodingAlgorithm = ENCODING_ALG_DEFAULT;

      activate_toggle_button(GTK_TOGGLE_BUTTON(wl->eaRadio4A), TRUE); 
      activate_toggle_button(GTK_TOGGLE_BUTTON(wl->eaRadio4B), TRUE); 
   }
}

/*
 * I/O strategy selection
 */

static void io_strategy_cb(GtkWidget *widget, gpointer data)
{  RS03Widgets *wl = (RS03Widgets*)data;
   int state  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

   if(!state)  /* only track changes to activate state */
     return;

   if(widget == wl->ioRadio1A || widget == wl->ioRadio1B)
   {  Closure->encodingIOStrategy = IO_STRATEGY_READWRITE;

      activate_toggle_button(GTK_TOGGLE_BUTTON(wl->ioRadio1A), TRUE); 
      activate_toggle_button(GTK_TOGGLE_BUTTON(wl->ioRadio1B), TRUE); 
   }

   if(widget == wl->ioRadio2A || widget == wl->ioRadio2B)
   {  Closure->encodingIOStrategy = IO_STRATEGY_MMAP;

      activate_toggle_button(GTK_TOGGLE_BUTTON(wl->ioRadio2A), TRUE); 
      activate_toggle_button(GTK_TOGGLE_BUTTON(wl->ioRadio2B), TRUE); 
   }
}

/*
 * Setting the notebook page does not work at creation time.
 */

static gboolean notebook_idle_func(gpointer data)
{  RS03Widgets *wl = (RS03Widgets*)data;

   switch(Closure->eccTarget)
   {  case ECC_FILE:
       gtk_notebook_set_current_page(GTK_NOTEBOOK(wl->redundancyNotebook), 1);
       break;

     case ECC_IMAGE:
       gtk_notebook_set_current_page(GTK_NOTEBOOK(wl->redundancyNotebook), 0);
       break;
   }

   return FALSE;
}


/*
 * Redundancy selection for error correction files.
 * Cut&Paste from RS01; bad idea; but RS01 will be obsoleted soon.
 */

static void nroots_cb(GtkWidget *widget, gpointer data)
{  RS03Widgets *wl = (RS03Widgets*)data;
   int value;

   value = gtk_range_get_value(GTK_RANGE(widget));
   if(Closure->redundancy) g_free(Closure->redundancy);
   Closure->redundancy = g_strdup_printf("%d", value);

   if(widget == wl->redundancyScaleA)
        set_range_value(GTK_RANGE(wl->redundancyScaleB), value);
   else set_range_value(GTK_RANGE(wl->redundancyScaleA), value);

   UpdateMethodPreferences();
}

static void ecc_size_cb(GtkWidget *widget, gpointer data)
{  RS03Widgets *wl = (RS03Widgets*)data;
   int value;

   value = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
   if(Closure->redundancy) g_free(Closure->redundancy);
   Closure->redundancy = g_strdup_printf("%dm", value);

   if(widget == wl->redundancySpinA)
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(wl->redundancySpinB), atoi(Closure->redundancy));
   else gtk_spin_button_set_value(GTK_SPIN_BUTTON(wl->redundancySpinA), atoi(Closure->redundancy));

   UpdateMethodPreferences();
}

static void toggle_cb(GtkWidget *widget, gpointer data)
{  Method *method = (Method*)data;
   RS03Widgets *wl = (RS03Widgets*)method->widgetList;
   int state  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

   if(state == TRUE)
   {  if(widget == wl->radio3A || widget == wl->radio3B)
      {  gtk_widget_set_sensitive(wl->redundancyScaleA, TRUE);
	 gtk_widget_set_sensitive(wl->redundancyScaleB, TRUE);
      }
      else
      {  gtk_widget_set_sensitive(wl->redundancyScaleA, FALSE);
	 gtk_widget_set_sensitive(wl->redundancyScaleB, FALSE);
      }

      if(widget == wl->radio4A || widget == wl->radio4B)
      {  gtk_widget_set_sensitive(wl->redundancySpinA, TRUE); 
	 gtk_widget_set_sensitive(wl->redundancySpinB, TRUE); 
	 gtk_widget_set_sensitive(wl->radio4LabelA, TRUE); 
	 gtk_widget_set_sensitive(wl->radio4LabelB, TRUE); 
      }
      else
      {  gtk_widget_set_sensitive(wl->redundancySpinA, FALSE); 
	 gtk_widget_set_sensitive(wl->redundancySpinB, FALSE); 
	 gtk_widget_set_sensitive(wl->radio4LabelA, FALSE); 
	 gtk_widget_set_sensitive(wl->radio4LabelB, FALSE); 
      }

      if(   widget == wl->radio1A  /* Normal */
	 || widget == wl->radio1B)
      {  
         set_range_value(GTK_RANGE(wl->redundancyScaleA), 32);
         set_range_value(GTK_RANGE(wl->redundancyScaleB), 32);

	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio1A), TRUE);
	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio1B), TRUE);

	 if(Closure->redundancy) g_free(Closure->redundancy);
         Closure->redundancy = g_strdup("normal");
      }

      if(   widget == wl->radio2A  /* High */
	 || widget == wl->radio2B)
      {  
         set_range_value(GTK_RANGE(wl->redundancyScaleA), 64);
         set_range_value(GTK_RANGE(wl->redundancyScaleB), 64);

	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio2A), TRUE);
	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio2B), TRUE);

	 if(Closure->redundancy) g_free(Closure->redundancy);
	 Closure->redundancy = g_strdup("high");
      }

      if(   widget == wl->radio3A  /* number of roots */
	 || widget == wl->radio3B)
      {  int nroots = gtk_range_get_value(GTK_RANGE(wl->redundancyScaleA));

	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio3A), TRUE);
	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio3B), TRUE);

	 if(Closure->redundancy) g_free(Closure->redundancy);
	 Closure->redundancy = g_strdup_printf("%d", nroots);
      }

      if(   widget == wl->radio4A  /* relative to space usage */
	 || widget == wl->radio4B)
      {  int space = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(wl->redundancySpinA));

	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio4A), TRUE);
	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio4B), TRUE);

	 if(Closure->redundancy) g_free(Closure->redundancy);
	 Closure->redundancy = g_strdup_printf("%dm", space);
      }

      UpdateMethodPreferences();
   }
}

/*
 * Sector prefetch selection
 */

static gchar* format_cb(GtkScale *scale, gdouble value, gpointer data)
{  char *label;

   switch(GPOINTER_TO_INT(data))
   {  case PREF_PRELOAD:
      case PREF_THREADS:
        label = g_strdup(" ");
        break;
      case PREF_NROOTS:
      {  int nroots = value;
	 int ndata  = GF_FIELDMAX - nroots;
   
	 label = g_strdup_printf(_utf("%4.1f%% redundancy (%d roots)"),
				 ((double)nroots*100.0)/(double)ndata,
				nroots);
      }
	break;
      default:
       label = g_strdup(" ");
       break;
   }
#if 0
     label = g_strdup_printf(_utf("%4.1f%% redundancy (%d roots)"),
			    ((double)nroots*100.0)/(double)ndata,
			    nroots);
#endif
   FORGET(label);  /* will be g_free()ed by the scale */
   return label;
}

static void prefetch_cb(GtkWidget *widget, gpointer data)
{  RS03Widgets *wl = (RS03Widgets*)data;
   LabelWithOnlineHelp *lwoh = wl->prefetchLwoh;
   int value;
   char *text, *utf;

   value = gtk_range_get_value(GTK_RANGE(widget));
   Closure->prefetchSectors = prefetch_size[value];

   text = g_strdup_printf(_("%d sectors"), Closure->prefetchSectors);
   utf  = g_locale_to_utf8(text, -1, NULL, NULL, NULL);
   gtk_label_set_markup(GTK_LABEL(lwoh->normalLabel), utf);
   gtk_label_set_markup(GTK_LABEL(lwoh->linkLabel), utf);
   SetOnlineHelpLinkText(lwoh, text);
   UpdateMethodPreferences();
   g_free(text);
   g_free(utf);
}

static void threads_cb(GtkWidget *widget, gpointer data)
{  RS03Widgets *wl = (RS03Widgets*)data;
   LabelWithOnlineHelp *lwoh = wl->threadsLwoh;
   int value;
   char *text, *utf;

   value = gtk_range_get_value(GTK_RANGE(widget));
   Closure->codecThreads = threads_count[value];

   text = g_strdup_printf(_("%d threads"), Closure->codecThreads);
   utf  = g_locale_to_utf8(text, -1, NULL, NULL, NULL);
   gtk_label_set_markup(GTK_LABEL(lwoh->normalLabel), utf);
   gtk_label_set_markup(GTK_LABEL(lwoh->linkLabel), utf);
   SetOnlineHelpLinkText(lwoh, text);
   UpdateMethodPreferences();
   g_free(text);
   g_free(utf);
}

/* 
 * Some values may be shared with other codecs.
 * If they changed there, update our preferences page.
 */

void ResetRS03PrefsPage(Method *method)
{  RS03Widgets *wl = (RS03Widgets*)method->widgetList;
   int index;

   /* Error correction file redundancy */

   if(Closure->redundancy)
   {  
      if(!strcmp(Closure->redundancy, "normal"))
      {  if(wl->radio1A && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wl->radio1A)) == FALSE)
	 {  activate_toggle_button(GTK_TOGGLE_BUTTON(wl->radio1A), TRUE);
            activate_toggle_button(GTK_TOGGLE_BUTTON(wl->radio1B), TRUE);
	 }
      }
      else if(!strcmp(Closure->redundancy, "high"))
      {  if(wl->radio2A && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wl->radio2A)) == FALSE)
	 {  activate_toggle_button(GTK_TOGGLE_BUTTON(wl->radio2A), TRUE);
	    activate_toggle_button(GTK_TOGGLE_BUTTON(wl->radio2B), TRUE);
	 }
      }
      else
      {  int last = strlen(Closure->redundancy)-1;

         if(Closure->redundancy[last] == 'm')
	 {  if(wl->redundancySpinA)
	    {  int old = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(wl->redundancySpinA));
	       int new;

	       Closure->redundancy[last] = 0;
	       new = atoi(Closure->redundancy);
	       Closure->redundancy[last] = 'm';

	       if(new != old)
	       {  set_spin_button_value(GTK_SPIN_BUTTON(wl->redundancySpinA), new);
		  set_spin_button_value(GTK_SPIN_BUTTON(wl->redundancySpinB), new);
	       }

	       if(wl->radio4A && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wl->radio4A)) == FALSE)
	       {  activate_toggle_button(GTK_TOGGLE_BUTTON(wl->radio4A), TRUE);
		  activate_toggle_button(GTK_TOGGLE_BUTTON(wl->radio4B), TRUE);
	       }
	    }
	 }
	 else
	 {  if(wl->redundancyScaleA)
	    {  int old = gtk_range_get_value(GTK_RANGE(wl->redundancyScaleA));
	       int new = atoi(Closure->redundancy);

	       if(new != old)
	       {  set_range_value(GTK_RANGE(wl->redundancyScaleA), new);
	          set_range_value(GTK_RANGE(wl->redundancyScaleB), new);
	       }

	       if(wl->radio3A && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wl->radio3A)) == FALSE)
	       {  activate_toggle_button(GTK_TOGGLE_BUTTON(wl->radio3A), TRUE);
		  activate_toggle_button(GTK_TOGGLE_BUTTON(wl->radio3B), TRUE);
	       }
	    }
	 }
      }
   }

   /* Prefetching */

   for(index = 0; index < sizeof(prefetch_size)/sizeof(int); index++)
     if(prefetch_size[index] > Closure->prefetchSectors)
       break;

   set_range_value(GTK_RANGE(wl->prefetchScaleA), index > 0 ? index-1 : index);
   set_range_value(GTK_RANGE(wl->prefetchScaleB), index > 0 ? index-1 : index);

   /* Number of threads */

   for(index = 0; index < sizeof(threads_count)/sizeof(int); index++)
     if(threads_count[index] > Closure->codecThreads)
       break;

   set_range_value(GTK_RANGE(wl->threadsScaleA), index > 0 ? index-1 : index);
   set_range_value(GTK_RANGE(wl->threadsScaleB), index > 0 ? index-1 : index);
}

/*
 * Read values from our preferences page
 * to make sure that all changed values from text entries
 * are recognized.
 */

void ReadRS03Preferences(Method *method)
{
#if 0
   RS03Widgets *wl = (RS03Widgets*)method->widgetList;
#endif
}

/*
 * Create our preferences page
 */

void CreateRS03PrefsPage(Method *method, GtkWidget *parent)
{  RS03Widgets *wl = (RS03Widgets*)method->widgetList;
   GtkWidget *frame, *hbox, *vbox, *lab, *scale, *spin, *radio;
   LabelWithOnlineHelp *lwoh;
   unsigned int index;
   char *text;
   int i;

   /*** Target for error correction data */

   frame = gtk_frame_new(_utf("Error correction data storage"));
   gtk_box_pack_start(GTK_BOX(parent), frame, FALSE, FALSE, 0);

   vbox = gtk_vbox_new(FALSE, 10);
   gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
   gtk_container_add(GTK_CONTAINER(frame), vbox);

   lwoh = CreateLabelWithOnlineHelp(_("Error correction data storage"), 
				    _("Store ECC data in: "));
   RegisterPreferencesHelpWindow(lwoh);

   for(i=0; i<2; i++)
   {  GtkWidget *hbox = gtk_hbox_new(FALSE, 4);
      GtkWidget *radio1, *radio2;

      gtk_box_pack_start(GTK_BOX(hbox), i ? lwoh->normalLabel : lwoh->linkBox, FALSE, FALSE, 0);

      radio1 = gtk_radio_button_new(NULL);
      g_signal_connect(G_OBJECT(radio1), "toggled", G_CALLBACK(eccmethod_cb), (gpointer)wl);
      gtk_box_pack_start(GTK_BOX(hbox), radio1, FALSE, FALSE, 0);
      lab = gtk_label_new(_utf("File"));
      gtk_container_add(GTK_CONTAINER(radio1), lab);

      radio2 = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(radio1));
      g_signal_connect(G_OBJECT(radio2), "toggled", G_CALLBACK(eccmethod_cb), (gpointer)wl);
      gtk_box_pack_start(GTK_BOX(hbox), radio2, FALSE, FALSE, 0);
      lab = gtk_label_new(_utf("Image"));
      gtk_container_add(GTK_CONTAINER(radio2), lab);

      switch(Closure->eccTarget)
      {  case ECC_FILE: activate_toggle_button(GTK_TOGGLE_BUTTON(radio1), TRUE); break;
         case ECC_IMAGE: activate_toggle_button(GTK_TOGGLE_BUTTON(radio2), TRUE); break;
      }

      if(!i)
      {  wl->eccFileA  = radio1;
	 wl->eccImageA = radio2;
	 gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
      }
      else  
      {  wl->eccFileB  = radio1;
	 wl->eccImageB = radio2;
	 AddHelpWidget(lwoh, hbox);
      }
   }

   AddHelpParagraph(lwoh, 
		    _("<b>Error correction data storage</b>\n\n"
		      "Select between two ways of storing the "
		      "error correction information:\n"));

   
    AddHelpListItem(lwoh, _("Augmented image (recommended)\n"
			   "The error correction data will be stored along with the user data on the "
			   "same medium. This requires the creation of an image file prior to writing the "
			   "medium. The error correction data will be appended to that image "
			   "and fill up the remaining space.\n"
			   "Damaged sectors in the error correction "
			   "information reduce the data recovery capacity, but do not make recovery "
			   "impossible - a second medium for keeping or protecting the error correction "
			   "information is not required.\n"));

  AddHelpListItem(lwoh, _("Error correction file\n"
			   "Error correction files are the only way of protecting existing media "
			   "as they can be stored somewhere else. They are kept on a separate "
			   "medium which must also be protected by dvdisaster. This prevents from losing the "
			   "error correction files in case of a medium defect.\n"));

      /*** Redundancy selection */

   frame = gtk_frame_new(_utf("Redundancy for new error correction files"));
   gtk_box_pack_start(GTK_BOX(parent), frame, FALSE, FALSE, 0);

   /* Notebook for disabling redundancy selection for embedded images */
      
   wl->redundancyNotebook = gtk_notebook_new();
   gtk_notebook_set_show_tabs(GTK_NOTEBOOK(wl->redundancyNotebook), FALSE);
   gtk_notebook_set_show_border(GTK_NOTEBOOK(wl->redundancyNotebook), FALSE);
   gtk_container_add(GTK_CONTAINER(frame), wl->redundancyNotebook);

   /* dummy page for augmented images */

   lab = gtk_label_new(_utf("no settings for augmented images"));
   gtk_notebook_append_page(GTK_NOTEBOOK(wl->redundancyNotebook), lab, 
			    gtk_label_new(""));

   g_idle_add(notebook_idle_func, wl); /* defer notebook page activation */

   /* real entry for error correction files */

   vbox = gtk_vbox_new(FALSE, 10);
   gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
   gtk_notebook_append_page(GTK_NOTEBOOK(wl->redundancyNotebook), vbox, 
			    gtk_label_new(""));

   /* Normal redundancy */

   lwoh = CreateLabelWithOnlineHelp(_("Normal redundancy"), _("Normal"));
   RegisterPreferencesHelpWindow(lwoh);

   for(i=0; i<2; i++)
   {  GtkWidget *hbox = gtk_hbox_new(FALSE, 4);

      radio = gtk_radio_button_new(NULL);
      g_signal_connect(G_OBJECT(radio), "toggled", G_CALLBACK(toggle_cb), method);
      gtk_box_pack_start(GTK_BOX(hbox), radio, FALSE, FALSE, 0);

      if(!i)
      {  wl->radio1A = radio;
	 gtk_box_pack_start(GTK_BOX(hbox), lwoh->linkBox, FALSE, FALSE, 0);
	 gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
      }
      else
      {  wl->radio1B = radio;
         gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);
	 AddHelpWidget(lwoh, hbox);
      }
   }

   AddHelpParagraph(lwoh, _("<b>Normal redundancy</b>\n\n"
			    "The preset \"normal\" creates a redundancy of 14.3%%.\n"
			    "It invokes optimized program code to speed up the "
			    "error correction file creation."));

   /* High redundancy */

   lwoh = CreateLabelWithOnlineHelp(_("High redundancy"), _("High"));
   RegisterPreferencesHelpWindow(lwoh);

   for(i=0; i<2; i++)
   {  GtkWidget *hbox = gtk_hbox_new(FALSE, 4);

      radio = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(i?wl->radio1B:wl->radio1A));
      g_signal_connect(G_OBJECT(radio), "toggled", G_CALLBACK(toggle_cb), method);
      gtk_box_pack_start(GTK_BOX(hbox), radio, FALSE, FALSE, 0);

      if(!i)
      {  wl->radio2A = radio;
	 gtk_box_pack_start(GTK_BOX(hbox), lwoh->linkBox, FALSE, FALSE, 0);
	 gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
      }
      else
      {  wl->radio2B = radio;
         gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);
	 AddHelpWidget(lwoh, hbox);
      }
   }

   AddHelpParagraph(lwoh, _("<b>High redundancy</b>\n\n"
			    "The preset \"high\" creates a redundancy of 33.5%%.\n"
			    "It invokes optimized program code to speed up the "
			    "error correction file creation."));


   /* User-selected redundancy */

   lwoh = CreateLabelWithOnlineHelp(_("Other redundancy"), _("Other"));
   RegisterPreferencesHelpWindow(lwoh);

   for(i=0; i<2; i++)
   {  hbox = gtk_hbox_new(FALSE, 4);

      radio = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(i?wl->radio1B:wl->radio1A));
      g_signal_connect(G_OBJECT(radio), "toggled", G_CALLBACK(toggle_cb), method);
      gtk_box_pack_start(GTK_BOX(hbox), radio, FALSE, FALSE, 0);

      if(!i)
      {  wl->radio3A = radio;
	 gtk_box_pack_start(GTK_BOX(hbox), lwoh->linkBox, FALSE, FALSE, 0);
      }
      else
      {  wl->radio3B = radio;
         gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);
      }

      scale = gtk_hscale_new_with_range(8,170,1);
      gtk_scale_set_value_pos(GTK_SCALE(scale), GTK_POS_RIGHT);
      gtk_range_set_increments(GTK_RANGE(scale), 1, 1);
      gtk_range_set_value(GTK_RANGE(scale), 32);
      gtk_widget_set_sensitive(scale, FALSE);
      g_signal_connect(scale, "format-value", G_CALLBACK(format_cb), (gpointer)PREF_NROOTS);
      g_signal_connect(scale, "value-changed", G_CALLBACK(nroots_cb), (gpointer)wl);
      gtk_container_add(GTK_CONTAINER(hbox), scale);

      if(!i)
      {  wl->redundancyScaleA = scale;
	 gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
      }
      else
      {  wl->redundancyScaleB = scale;
	 AddHelpWidget(lwoh, hbox);
      }
   }

   AddHelpParagraph(lwoh, _("<b>Other redundancy</b>\n\n"
			    "Specifies the redundancy by percent.\n"
			    "An error correction file with x%% redundancy "
			    "will be approximately x%% of the size of the "
			    "corresponding image file."));

   /* Space-delimited redundancy */

   lwoh = CreateLabelWithOnlineHelp(_("Space-delimited redundancy"), _("Use at most"));
   RegisterPreferencesHelpWindow(lwoh);

   for(i=0; i<2; i++)
   {  hbox = gtk_hbox_new(FALSE, 4);

      radio = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(i?wl->radio1B:wl->radio1A));
      g_signal_connect(G_OBJECT(radio), "toggled", G_CALLBACK(toggle_cb), method);
      gtk_box_pack_start(GTK_BOX(hbox), radio, FALSE, FALSE, 0);

      if(!i)
      {  wl->radio4A = radio;
	 gtk_box_pack_start(GTK_BOX(hbox), lwoh->linkBox, FALSE, FALSE, 0);
      }
      else
      {  wl->radio4B = radio;
         gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);
      }

      spin = gtk_spin_button_new_with_range(0, 100000, 100);
      g_signal_connect(spin, "value-changed", G_CALLBACK(ecc_size_cb), (gpointer)wl);
      gtk_entry_set_width_chars(GTK_ENTRY(spin), 8);
      gtk_box_pack_start(GTK_BOX(hbox), spin, FALSE, FALSE, 0);

      lab = gtk_label_new(_utf("MiB for error correction data"));
      gtk_box_pack_start(GTK_BOX(hbox), lab, FALSE, FALSE, 0);
      gtk_widget_set_sensitive(spin, FALSE);
      gtk_widget_set_sensitive(lab, FALSE);

      if(!i)
      {  wl->redundancySpinA = spin;
	 wl->radio4LabelA = lab;
         gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
      }
      else
      {  wl->redundancySpinB = spin;
	 wl->radio4LabelB = lab;
	 AddHelpWidget(lwoh, hbox);
      }
   }

   AddHelpParagraph(lwoh, _("<b>Space-delimited redundancy</b>\n\n"
			    "Specifies the maximum size of the error correction file in MiB. "
			    "dvdisaster will choose a suitable redundancy setting so that "
			    "the overall size of the error correction file does not exceed "
			    "the given limit.\n\n"
			    "<b>Advance notice:</b> When using the same size setting for "
			    "images of vastly different size, smaller images receive more "
			    "redundancy than larger ones. This is usually not what you want."));

   /* Preset redundancy values
      FIXME: replace by ResetRS03Prefs()? */

   if(Closure->redundancy)
   {  if(!strcmp(Closure->redundancy, "normal"))
      {  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio1A), TRUE);
         gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio1B), TRUE);
      }
      else if(!strcmp(Closure->redundancy, "high"))
      {  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio2A), TRUE);
         gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio2B), TRUE);
      }
      else
      {  int last = strlen(Closure->redundancy)-1;

         if(Closure->redundancy[last] == 'm')
	 {  Closure->redundancy[last] = 0;
	    gtk_spin_button_set_value(GTK_SPIN_BUTTON(wl->redundancySpinA), atoi(Closure->redundancy));
	    gtk_spin_button_set_value(GTK_SPIN_BUTTON(wl->redundancySpinB), atoi(Closure->redundancy));
	    Closure->redundancy[last] = 'm';
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio4A), TRUE);
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio4B), TRUE);
	 }
	 else
	 {  gtk_range_set_value(GTK_RANGE(wl->redundancyScaleA), atoi(Closure->redundancy));
	    gtk_range_set_value(GTK_RANGE(wl->redundancyScaleB), atoi(Closure->redundancy));
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio3A), TRUE);
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wl->radio3B), TRUE);
	 }
      }
   }

   /*** IO parameters */

   /* Prefetch sectors */

   frame = gtk_frame_new(_utf("I/O parameters"));
   gtk_box_pack_start(GTK_BOX(parent), frame, FALSE, FALSE, 0);

   vbox = gtk_vbox_new(FALSE, 10);
   gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
   gtk_container_add(GTK_CONTAINER(frame), vbox);

   text = g_strdup_printf(_("%d sectors"), Closure->prefetchSectors);
   lwoh = CreateLabelWithOnlineHelp(_("Sector preloading"), text);
   RegisterPreferencesHelpWindow(lwoh);
   g_free(text);

   wl->prefetchLwoh = lwoh;
   LockLabelSize(GTK_LABEL(lwoh->normalLabel), _utf("%d sectors"), 2222);
   LockLabelSize(GTK_LABEL(lwoh->linkLabel), _utf("%d sectors"), 2222);

   for(i=0; i<2; i++)
   {  GtkWidget *hbox = gtk_hbox_new(FALSE, 4);
      int n_entries = sizeof(prefetch_size)/sizeof(int);

      lab = gtk_label_new(_utf("Preload"));
      gtk_box_pack_start(GTK_BOX(hbox), lab, FALSE, FALSE, 0);

      for(index = 0; index < n_entries; index++)
	if(prefetch_size[index] > Closure->prefetchSectors)
	  break;

      scale = gtk_hscale_new_with_range(0,n_entries-1,1);
      gtk_scale_set_value_pos(GTK_SCALE(scale), GTK_POS_RIGHT);
      gtk_range_set_increments(GTK_RANGE(scale), 1, 1);
      gtk_range_set_value(GTK_RANGE(scale), index > 0 ? index-1 : index);
      g_signal_connect(scale, "format-value", G_CALLBACK(format_cb), (gpointer)PREF_PRELOAD);
      g_signal_connect(scale, "value-changed", G_CALLBACK(prefetch_cb), (gpointer)wl);
      gtk_box_pack_start(GTK_BOX(hbox), scale, TRUE, TRUE, 0);

      if(!i)
      {  wl->prefetchScaleA = scale; 
	 gtk_box_pack_start(GTK_BOX(hbox), lwoh->linkBox, FALSE, FALSE, 0);
	 gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
      }
      else
      {  wl->prefetchScaleB = scale; 
	 gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);
	 AddHelpWidget(lwoh, hbox);
      }
   }

   AddHelpParagraph(lwoh, _("<b>Sector preloading</b>\n\n"
			    "dvdisaster optimizes access to the image and error correction "
			    "data by preloading and caching parts of them.\n\n"
			    "The optimal preload value depends on the storage system "
			    "used for the image and error correction files.\n"
			    "Use small preload values for systems with low latency "
			    "and seek time, e.g. SSDs. For magnetic hard disks "
			    "performance may be better using larger preload values.\n\n"
			    "A preload value of n will used approx. n MiB of RAM."));

   /*** IO strategy */

   lwoh = CreateLabelWithOnlineHelp(_("I/O strategy"), 
				    _("I/O strategy: "));
   RegisterPreferencesHelpWindow(lwoh);

   for(i=0; i<2; i++)
   {  GtkWidget *hbox = gtk_hbox_new(FALSE, 4);
      GtkWidget *radio1, *radio2;

      gtk_box_pack_start(GTK_BOX(hbox), i ? lwoh->normalLabel : lwoh->linkBox, FALSE, FALSE, 0);

      radio1 = gtk_radio_button_new(NULL);
      g_signal_connect(G_OBJECT(radio1), "toggled", G_CALLBACK(io_strategy_cb), (gpointer)wl);
      gtk_box_pack_start(GTK_BOX(hbox), radio1, FALSE, FALSE, 0);
      lab = gtk_label_new(_utf("read/write"));
      gtk_container_add(GTK_CONTAINER(radio1), lab);

      radio2 = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(radio1));
      g_signal_connect(G_OBJECT(radio2), "toggled", G_CALLBACK(io_strategy_cb), (gpointer)wl);
      gtk_box_pack_start(GTK_BOX(hbox), radio2, FALSE, FALSE, 0);
      lab = gtk_label_new(_utf("memory mapped"));
      gtk_container_add(GTK_CONTAINER(radio2), lab);

      switch(Closure->encodingIOStrategy)
      {  case IO_STRATEGY_READWRITE: activate_toggle_button(GTK_TOGGLE_BUTTON(radio1), TRUE); break;
         case IO_STRATEGY_MMAP:      activate_toggle_button(GTK_TOGGLE_BUTTON(radio2), TRUE); break;
      }

      if(!i)
      {  wl->ioRadio1A = radio1;
	 wl->ioRadio2A = radio2;
	 gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
      }
      else  
      {  wl->ioRadio1B = radio1;
	 wl->ioRadio2B = radio2;
	 AddHelpWidget(lwoh, hbox);
      }
   }

   AddHelpParagraph(lwoh, _("<b>I/O strategy</b>\n\n"
     "This option controls how dvdisaster performs its disk I/O while creating error "
     "correction data. Try both options and see which performs best on your hardware "
     "setting.\n\n" 
     "The <b>read/write</b> option activates dvdisaster's own I/O scheduler "
     "which reads and writes image data using normal file I/O. The advantage of this "
     "scheme is that dvdisaster knows exactly which data needs to be cached and preloaded; "
     "the disadvantage is that all data needs to be copied between the kernel and "
     "dvdisaster's own buffers. Usually, this I/O scheme works best on slow storage "
     "with high latency and seek times; e.g. on all storage involving spinning platters.\n\n"
     "The <b>memory mapped</b> option uses the kernel's memory mapping scheme for direct access "
     "to the image file. This has the advantage of minimal overhead, but may be adversely "
     "affected by poor caching and preloading decisions made by the kernel (since the kernel does not "
     "know what dvdisaster is going to do with the data). This scheme "
     "performs well when encoding in a RAM-based file system (such as /dev/shm on GNU/Linux) "
     "and on very fast media with low latency such as SSDs."
 			    ));

   /*** Number of threads */

   frame = gtk_frame_new(_utf("Multithreading"));
   gtk_box_pack_start(GTK_BOX(parent), frame, FALSE, FALSE, 0);

   text = g_strdup_printf(_("%d threads"), Closure->codecThreads);
   lwoh = CreateLabelWithOnlineHelp(_("Multithreading"), text);
   RegisterPreferencesHelpWindow(lwoh);
   g_free(text);

   wl->threadsLwoh = lwoh;
   LockLabelSize(GTK_LABEL(lwoh->normalLabel), _utf("%d threads"), 22);
   LockLabelSize(GTK_LABEL(lwoh->linkLabel), _utf("%d threads"), 22);

   for(i=0; i<2; i++)
   {  GtkWidget *hbox = gtk_hbox_new(FALSE, 4);
      int n_entries = sizeof(threads_count)/sizeof(int);

      lab = gtk_label_new(_utf("Use"));
      gtk_box_pack_start(GTK_BOX(hbox), lab, FALSE, FALSE, 0);

      for(index = 0; index < n_entries; index++)
	if(threads_count[index] > Closure->codecThreads)
	  break;

      scale = gtk_hscale_new_with_range(0,n_entries-1,1);
      gtk_scale_set_value_pos(GTK_SCALE(scale), GTK_POS_RIGHT);
      gtk_range_set_increments(GTK_RANGE(scale), 1, 1);
      gtk_range_set_value(GTK_RANGE(scale), index > 0 ? index-1 : index);
      g_signal_connect(scale, "format-value", G_CALLBACK(format_cb), (gpointer)PREF_THREADS);
      g_signal_connect(scale, "value-changed", G_CALLBACK(threads_cb), (gpointer)wl);
      gtk_box_pack_start(GTK_BOX(hbox), scale, TRUE, TRUE, 0);

      if(!i)
      {  wl->threadsScaleA = scale; 
	 gtk_container_set_border_width(GTK_CONTAINER(hbox), 10);
	 gtk_box_pack_start(GTK_BOX(hbox), lwoh->linkBox, FALSE, FALSE, 0);
	 gtk_container_add(GTK_CONTAINER(frame), hbox);
      }
      else
      {  wl->threadsScaleB = scale; 
	 gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);
	 AddHelpWidget(lwoh, hbox);
      }
   }

   AddHelpParagraph(lwoh, _("<b>Multithreading</b>\n\n"
			    "RS03 can use multiple threads (and therefore CPU cores)"
			    "for encoding.\n"
			    "For systems with 4 cores or less, set the number of "
			    "threads to the number of cores. If you have more cores, "
			    "leave one unused for doing I/O and graphics updates.\n"
			    "E.g. use 7 threads on an 8 core system.\n\n"
			    "Performance will not scale linearly "
			    "with the number of CPU cores. Hard disk performance "
			    "is more limiting than raw CPU power. When using "
			    "4 cores or more, memory bandwidth may also affect "
			    "performance."));

   /*** Codec type */

   frame = gtk_frame_new(_utf("Encoding algorithm"));
   gtk_box_pack_start(GTK_BOX(parent), frame, FALSE, FALSE, 0);

   vbox = gtk_vbox_new(FALSE, 10);
   gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
   gtk_container_add(GTK_CONTAINER(frame), vbox);

   lwoh = CreateLabelWithOnlineHelp(_("Encoding algorithm"), 
				    _("Use: "));
   RegisterPreferencesHelpWindow(lwoh);

   for(i=0; i<2; i++)
   {  GtkWidget *hbox = gtk_hbox_new(FALSE, 4);
      GtkWidget *radio1, *radio2, *radio3=NULL, *radio4;

      gtk_box_pack_start(GTK_BOX(hbox), i ? lwoh->normalLabel : lwoh->linkBox, FALSE, FALSE, 0);

      radio1 = gtk_radio_button_new(NULL);
      g_signal_connect(G_OBJECT(radio1), "toggled", G_CALLBACK(encoding_alg_cb), (gpointer)wl);
      gtk_box_pack_start(GTK_BOX(hbox), radio1, FALSE, FALSE, 0);
      lab = gtk_label_new(_utf("32bit"));
      gtk_container_add(GTK_CONTAINER(radio1), lab);

      radio2 = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(radio1));
      g_signal_connect(G_OBJECT(radio2), "toggled", G_CALLBACK(encoding_alg_cb), (gpointer)wl);
      gtk_box_pack_start(GTK_BOX(hbox), radio2, FALSE, FALSE, 0);
      lab = gtk_label_new(_utf("64bit"));
      gtk_container_add(GTK_CONTAINER(radio2), lab);

      if(Closure->useSSE2)
      {  radio3 = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(radio2));
	 g_signal_connect(G_OBJECT(radio3), "toggled", G_CALLBACK(encoding_alg_cb), (gpointer)wl);
	 gtk_box_pack_start(GTK_BOX(hbox), radio3, FALSE, FALSE, 0);
	 lab = gtk_label_new(_utf("SSE2"));
	 gtk_container_add(GTK_CONTAINER(radio3), lab);
      }
      if(Closure->useAltiVec)
      {  radio3 = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(radio2));
	 g_signal_connect(G_OBJECT(radio3), "toggled", G_CALLBACK(encoding_alg_cb), (gpointer)wl);
	 gtk_box_pack_start(GTK_BOX(hbox), radio3, FALSE, FALSE, 0);
	 lab = gtk_label_new(_utf("AltiVec"));
	 gtk_container_add(GTK_CONTAINER(radio3), lab);
      }

      radio4 = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(radio2));
      g_signal_connect(G_OBJECT(radio4), "toggled", G_CALLBACK(encoding_alg_cb), (gpointer)wl);
      gtk_box_pack_start(GTK_BOX(hbox), radio4, FALSE, FALSE, 0);
      lab = gtk_label_new(_utf("auto"));
      gtk_container_add(GTK_CONTAINER(radio4), lab);

      switch(Closure->encodingAlgorithm)
      {  case ENCODING_ALG_DEFAULT: activate_toggle_button(GTK_TOGGLE_BUTTON(radio4), TRUE); break;
         case ENCODING_ALG_32BIT:   activate_toggle_button(GTK_TOGGLE_BUTTON(radio1), TRUE); break;
         case ENCODING_ALG_64BIT:   activate_toggle_button(GTK_TOGGLE_BUTTON(radio2), TRUE); break;
         case ENCODING_ALG_SSE2:    
         case ENCODING_ALG_ALTIVEC: activate_toggle_button(GTK_TOGGLE_BUTTON(radio3), TRUE); break;
      }

      if(!i)
      {  wl->eaRadio1A = radio1;
	 wl->eaRadio2A = radio2;
	 wl->eaRadio3A = radio3;
	 wl->eaRadio4A = radio4;
	 gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
      }
      else  
      {  wl->eaRadio1B = radio1;
	 wl->eaRadio2B = radio2;
	 wl->eaRadio3B = radio3;
	 wl->eaRadio4B = radio4;
	 AddHelpWidget(lwoh, hbox);
      }
   }

   AddHelpParagraph(lwoh, _("<b>Encoding algorithm</b>\n\n"
     "This option affects the speed of generating RS03 error correction data.\n"
     "dvdisaster can either use a generic encoding algorithm using 32bit or 64bit "
     "wide operations running on the integer unit of the processor, or use "
     "processor specific extensions.\n\n"
     "Available extensions are SSE2 for x86 based processors and AltiVec "
     "on PowerPC processors. These extensions encode with 128bit wide operations "
     "and will usually provide the fastest encoding variant. If \"auto\" is selected, the "
     "SSE2/AltiVec algorithms will be selected if the processor supports them; "
     "otherwise the 64bit algorithm will be used."
			    ));
}
