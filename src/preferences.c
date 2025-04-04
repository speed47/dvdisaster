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

extern gint64 CurrentMediumSize(int);  /* from scsi-layer.h */

/***
 *** debugging workaround
 ***/

#if 0
#define gtk_widget_set_sensitive(widget, state) {  printf("%s, line %d - gtk_widget_set_sensitive()\n", __FILE__, __LINE__); gtk_widget_set_sensitive(widget, state); }
#endif

/***
 *** Protected widget mutators
 ***
 * We might have some race conditions where a callback tries to change
 * a not yet initialized widget.
 */

static void activate_toggle_button(GtkToggleButton *toggle, int state)
{  if(toggle)
      gtk_toggle_button_set_active(toggle, state);
}

static void set_widget_sensitive(GtkWidget *widget, int state)
{  if(widget)
      gtk_widget_set_sensitive(widget, state);
}

static void set_entry_text(GtkEntry *entry, char *text)
{  if(entry)
      gtk_entry_set_text(entry, text);
}

/***
 *** Local data structs
 ***/

/* non linear scale housekeeping */

typedef struct
{  GtkWidget *label;  /* for help system linkage */
   LabelWithOnlineHelp *lwoh;   
   int action;
   int *values;
   char *format;
   struct _prefs_context *pc;
} non_linear_info;

/***
 *** Preferences window housekeeping
 ***/

typedef struct _prefs_context
{  char *formatLinear;
   char *formatAdaptive;

   GPtrArray *helpPages;   /* The online help frame structures */

  /* Widgets for changing preferences settings. The are two copies (A and B) 
     of each; one for the standard dialog and one embedded in the online help. */

   GtkWidget *suffixA, *suffixB;
   GtkWidget *radioLinearA, *radioLinearB;
   GtkWidget *radioAdaptiveA, *radioAdaptiveB;
   GtkWidget *minAttemptsScaleA, *minAttemptsScaleB;
   GtkWidget *maxAttemptsScaleA, *maxAttemptsScaleB;
   GtkWidget *readMediumA, *readMediumB;
   GtkWidget *cacheDefectiveA, *cacheDefectiveB;
   GtkWidget *cacheDefectiveDirA, *cacheDefectiveDirB;
   GtkWidget *cacheDefectiveChooser;
   GtkWidget *cacheDefectivePrefixA, *cacheDefectivePrefixB;
   GtkWidget *rangeToggleA, *rangeToggleB;
   GtkWidget *rangeSpin1A, *rangeSpin1B;
   GtkWidget *rangeSpin2A, *rangeSpin2B;
   GtkWidget *rawButtonA, *rawButtonB;
   GtkWidget *jumpScaleA, *jumpScaleB;
   GtkWidget *daoButtonA, *daoButtonB;
   GtkWidget *ignoreISOSizeA, *ignoreISOSizeB;
   GtkWidget *dsmButtonA, *dsmButtonB;
   GtkWidget *recogRS02A, *recogRS02B;
   GtkWidget *recogRS03A, *recogRS03B;
   GtkWidget *byteEntryA, *byteEntryB;
   GtkWidget *byteCheckA, *byteCheckB;
   GtkWidget *spinUpA, *spinUpB;
   GtkWidget *internalAttemptsA, *internalAttemptsB;
   GtkWidget *radioRawMode20A, *radioRawMode20B;
   GtkWidget *radioRawMode21A, *radioRawMode21B;
   GtkWidget *radioRawModeOtherA, *radioRawModeOtherB;
   GtkWidget *rawModeValueA, *rawModeValueB;
   GtkWidget *fatalSenseA, *fatalSenseB;
   GtkWidget *ejectA, *ejectB;
   GtkWidget *readAndCreateButtonA, *readAndCreateButtonB;
   GtkWidget *unlinkImageButtonA, *unlinkImageButtonB;
   GtkWidget *confirmDeletionA, *confirmDeletionB;
   GtkWidget *mainNotebook;
   GtkWidget *methodChooserA,*methodChooserB;
   GtkWidget *methodNotebook;
   GtkWidget *cancelOKA, *cancelOKB;
   GtkWidget *verboseA, *verboseB;
   GtkWidget *logFileA, *logFileB;
   GtkWidget *logFilePathA, *logFilePathB;
   GtkWidget *logFileChooser;

   GtkWidget *redA, *redB;
   GtkWidget *yellowA, *yellowB;
   GtkWidget *greenA, *greenB;
   GtkWidget *blueA, *blueB;
   GtkWidget *whiteA, *whiteB;
   GtkWidget *darkA, *darkB;
   GtkWidget *redTextA, *redTextB;
   GtkWidget *greenTextA, *greenTextB;
   GtkWidget *barColorA, *barColorB;
   GtkWidget *logColorA, *logColorB;
   GtkWidget *curveColorA, *curveColorB;

   non_linear_info *jumpScaleInfoA, *jumpScaleInfoB;
   LabelWithOnlineHelp *jumpScaleLwoh;

   non_linear_info *minAttemptsScaleInfoA, *minAttemptsScaleInfoB;
   non_linear_info *maxAttemptsScaleInfoA, *maxAttemptsScaleInfoB;
   LabelWithOnlineHelp *minAttemptsScaleLwoh, *maxAttemptsScaleLwoh;
} prefs_context;

void GuiFreePreferences(void *context)
{  prefs_context *pc = (prefs_context*)context;
   int i;

   for(i=0; i<pc->helpPages->len; i++)
   {  LabelWithOnlineHelp *lwoh = g_ptr_array_index(pc->helpPages,i);

      GuiFreeLabelWithOnlineHelp(lwoh);
   }
   g_ptr_array_free(pc->helpPages, FALSE);

   if(pc->formatLinear) g_free(pc->formatLinear);
   if(pc->formatAdaptive) g_free(pc->formatAdaptive);

   if(pc->jumpScaleInfoA) g_free(pc->jumpScaleInfoA);
   if(pc->jumpScaleInfoB) g_free(pc->jumpScaleInfoB);
   if(pc->minAttemptsScaleInfoA->format) g_free(pc->minAttemptsScaleInfoA->format);
   if(pc->minAttemptsScaleInfoB->format) g_free(pc->minAttemptsScaleInfoB->format);
   if(pc->minAttemptsScaleInfoA) g_free(pc->minAttemptsScaleInfoA);
   if(pc->minAttemptsScaleInfoB) g_free(pc->minAttemptsScaleInfoB);
   if(pc->maxAttemptsScaleInfoA->format) g_free(pc->maxAttemptsScaleInfoA->format);
   if(pc->maxAttemptsScaleInfoB->format) g_free(pc->maxAttemptsScaleInfoB->format);
   if(pc->maxAttemptsScaleInfoA) g_free(pc->maxAttemptsScaleInfoA);
   if(pc->maxAttemptsScaleInfoB) g_free(pc->maxAttemptsScaleInfoB);

   g_free(pc);
}

static gboolean delete_cb(GtkWidget *widget, GdkEvent *event, gpointer data)
{
   GuiFreePreferences(Closure->prefsContext);

   Closure->prefsWindow = NULL;
   Closure->prefsContext = NULL;
   return FALSE;
}

void GuiHidePreferences(void)
{  prefs_context *pc = (prefs_context*)Closure->prefsContext;
   Method *method;
   const char *value1, *value2;
   int method_index;
   int i;

   /* Get reading range values */

   if(   gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pc->rangeToggleA))   
      || gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pc->rangeToggleB)))   
   {     
      Closure->readStart = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(pc->rangeSpin1A));
      Closure->readEnd   = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(pc->rangeSpin2A));
   }
   else Closure->readStart = Closure->readEnd = 0;

   /* Get fill byte and recalculate the dead sector marker */

   if(pc->byteCheckA && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pc->byteCheckA)))   
   {  const char *value1 = gtk_entry_get_text(GTK_ENTRY(pc->byteEntryA));
      const char *value2 = gtk_entry_get_text(GTK_ENTRY(pc->byteEntryB));
      int v1 = strtol(value1, NULL, 0);
      int v2 = strtol(value2, NULL, 0);

      /* both fields may contain different values */

      if(Closure->fillUnreadable != v2)
      {  Closure->fillUnreadable = v2;
 	 gtk_entry_set_text(GTK_ENTRY(pc->byteEntryA), value2);
      }
      else
	if(Closure->fillUnreadable != v1)
	{  Closure->fillUnreadable = v1;
	   gtk_entry_set_text(GTK_ENTRY(pc->byteEntryB), value1);
	}

      if(Closure->fillUnreadable < 0)
	Closure->fillUnreadable = 1;

      if(Closure->fillUnreadable > 255)
	Closure->fillUnreadable = 255;
   }

   /* Get raw reading mode */

   if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pc->radioRawModeOtherA)))   
   {  const char *value1 = gtk_entry_get_text(GTK_ENTRY(pc->rawModeValueA));
      const char *value2 = gtk_entry_get_text(GTK_ENTRY(pc->rawModeValueB));
      int v1 = strtol(value1, NULL, 0);
      int v2 = strtol(value2, NULL, 0);

      /* both fields may contain different values */

      if(Closure->rawMode != v2)
      {  Closure->rawMode = v2;
 	 gtk_entry_set_text(GTK_ENTRY(pc->rawModeValueA), value2);
      }
      else
	if(Closure->rawMode != v1)
	{  Closure->rawMode = v1;
	   gtk_entry_set_text(GTK_ENTRY(pc->rawModeValueB), value1);
	}

      if(Closure->rawMode < 0)
	Closure->rawMode = 0;

      if(Closure->rawMode > 255)
	Closure->rawMode = 255;
   }

   /* Get defective sector cache prefix.
      Both entries might contain different input. */

   value1 = gtk_entry_get_text(GTK_ENTRY(pc->cacheDefectivePrefixA));
   value2 = gtk_entry_get_text(GTK_ENTRY(pc->cacheDefectivePrefixB));
   if(strcmp(Closure->dDumpPrefix, value1))
   {  g_free(Closure->dDumpPrefix);
      Closure->dDumpPrefix = g_strdup(value1);
      gtk_entry_set_text(GTK_ENTRY(pc->cacheDefectivePrefixB), value1);
   }
   else if(strcmp(Closure->dDumpPrefix, value2))
   {  g_free(Closure->dDumpPrefix);
      Closure->dDumpPrefix = g_strdup(value2);
      gtk_entry_set_text(GTK_ENTRY(pc->cacheDefectivePrefixA), value2);
   }

   /* Ask currently selected method to update its settings
      from the preferences */

   method_index = gtk_notebook_get_current_page(GTK_NOTEBOOK(pc->methodNotebook));
   method = g_ptr_array_index(Closure->methodList, method_index);
   if(method->readPreferences)
     method->readPreferences(method);

   /* hide preferences and finish */

   gtk_widget_hide(GTK_WIDGET(Closure->prefsWindow));

   for(i=0; i<pc->helpPages->len; i++)
   {  LabelWithOnlineHelp *lwoh = g_ptr_array_index(pc->helpPages,i);
      gtk_widget_hide(lwoh->helpWindow);
   }
}

static void close_cb(GtkWidget *widget, gpointer data)
{  
   GuiHidePreferences();
}

/***
 *** Setting preferences from external functions 
 ***/

void GuiUpdatePrefsExhaustiveSearch(void)
{  prefs_context *pc = (prefs_context*)Closure->prefsContext;

   if(Closure->prefsContext)
   {  activate_toggle_button(GTK_TOGGLE_BUTTON(pc->recogRS02A), Closure->examineRS02); 
      activate_toggle_button(GTK_TOGGLE_BUTTON(pc->recogRS02B), Closure->examineRS02); 
      activate_toggle_button(GTK_TOGGLE_BUTTON(pc->recogRS03A), Closure->examineRS03); 
      activate_toggle_button(GTK_TOGGLE_BUTTON(pc->recogRS03B), Closure->examineRS03); 
   }
}

void GuiUpdatePrefsConfirmDeletion(void)
{  prefs_context *pc = (prefs_context*)Closure->prefsContext;

   if(Closure->prefsContext)
   {  activate_toggle_button(GTK_TOGGLE_BUTTON(pc->confirmDeletionA), Closure->confirmDeletion); 
      activate_toggle_button(GTK_TOGGLE_BUTTON(pc->confirmDeletionB), Closure->confirmDeletion); 
   }
}

/*
 * Register a preferences help window 
 */

void GuiRegisterPreferencesHelpWindow(LabelWithOnlineHelp *lwoh)
{  prefs_context *pc = (prefs_context*)Closure->prefsContext;

   g_ptr_array_add(pc->helpPages, lwoh);
}

/*
 * Actions used in the preferences
 */

enum 
{  TOGGLE_READ_CREATE,
   TOGGLE_UNLINK,
   TOGGLE_SUFFIX,
   TOGGLE_RECOG_RS02,
   TOGGLE_RECOG_RS03,
   TOGGLE_SIZEDRIVE,
   TOGGLE_DAO,
   TOGGLE_DSM,
   TOGGLE_RANGE,
   TOGGLE_RAW,
   TOGGLE_RAW_20H,
   TOGGLE_RAW_21H,
   TOGGLE_RAW_OTHER,
   TOGGLE_CACHE_DEFECTIVE,
   TOGGLE_CANCEL_OK,
   TOGGLE_FATAL_SENSE,
   TOGGLE_EJECT,
   TOGGLE_VERBOSE,
   TOGGLE_LOGFILE,
   TOGGLE_CONFIRM_DELETION,

   SPIN_DELAY,
   SPIN_INTERNAL_ATTEMPTS,
   SPIN_READ_MEDIUM,

   SLIDER_JUMP,
   SLIDER_MIN_READ_ATTEMPTS,
   SLIDER_MAX_READ_ATTEMPTS,

   COLOR_RED,
   COLOR_YELLOW,
   COLOR_GREEN,
   COLOR_BLUE,
   COLOR_WHITE,
   COLOR_DARK,
   COLOR_RED_TEXT,
   COLOR_GREEN_TEXT,
   COLOR_BAR,
   COLOR_LOG,
   COLOR_CURVE
};

/*
 * Create a new notebook page
 */

static GtkWidget *create_page(GtkWidget *notebook, char *label)
{  GtkWidget *vbox,*tab_label;

   tab_label = gtk_label_new(label);
   vbox = gtk_vbox_new(FALSE, 5);
   gtk_container_set_border_width(GTK_CONTAINER(vbox), 12);
   
   gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, tab_label);

   return vbox;
}

/***
 *** Toggle button actions
 ***/

static void toggle_cb(GtkWidget *widget, gpointer data)
{  int state  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
   int action = GPOINTER_TO_INT(data);
   prefs_context *pc = (prefs_context*)Closure->prefsContext;

   switch(action)
   {  case TOGGLE_READ_CREATE:
	Closure->readAndCreate = state;
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->readAndCreateButtonA), state);
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->readAndCreateButtonB), state);
	if(state && Closure->adaptiveRead)  /* set reading strategy to linear */
	{  prefs_context *pc = Closure->prefsContext;

	   Closure->adaptiveRead = FALSE;
	   pc->jumpScaleInfoA->format = pc->formatLinear;
	   pc->jumpScaleInfoB->format = pc->formatLinear;
	   activate_toggle_button(GTK_TOGGLE_BUTTON(pc->radioLinearA), TRUE);
	   activate_toggle_button(GTK_TOGGLE_BUTTON(pc->radioLinearB), TRUE);

	   GuiShowMessage(Closure->prefsWindow,
			  _("Switched to the linear reading strategy."), 
			  GTK_MESSAGE_INFO);
	}
	   
	break;

      case TOGGLE_UNLINK:
	Closure->unlinkImage = state;
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->unlinkImageButtonA), state);
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->unlinkImageButtonB), state);
	break;

      case TOGGLE_CONFIRM_DELETION:
	Closure->confirmDeletion = state;
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->confirmDeletionA), state);
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->confirmDeletionB), state);
	break;

      case TOGGLE_SUFFIX:
	Closure->autoSuffix = state;
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->suffixA), state);
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->suffixB), state);
	break;

      case TOGGLE_CACHE_DEFECTIVE:
	Closure->defectiveDump = state;
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->cacheDefectiveA), state);
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->cacheDefectiveB), state);
	break;

      case TOGGLE_LOGFILE:
	Closure->logFileEnabled = state;
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->logFileA), state);
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->logFileB), state);
	break;

      case TOGGLE_CANCEL_OK:
	Closure->reverseCancelOK = state;
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->cancelOKA), state);
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->cancelOKB), state);
	break;

      case TOGGLE_SIZEDRIVE:
	Closure->ignoreIsoSize = state;
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->ignoreISOSizeA), state);
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->ignoreISOSizeB), state);
	break;

      case TOGGLE_DAO:
	Closure->noTruncate = state;
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->daoButtonA), state);
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->daoButtonB), state);
	break;

      case TOGGLE_RECOG_RS02:
	Closure->examineRS02 = state;
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->recogRS02A), state);
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->recogRS02B), state);
	break;

      case TOGGLE_RECOG_RS03:
	Closure->examineRS03 = state;
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->recogRS03A), state);
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->recogRS03B), state);
	break;

      case TOGGLE_DSM:
	Closure->dsmVersion = !state;
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->dsmButtonA), state);
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->dsmButtonB), state);
	if(state)
	{  if(pc->byteCheckA)
	      activate_toggle_button(GTK_TOGGLE_BUTTON(pc->byteCheckA), FALSE);
	   if(pc->byteCheckB)
	      activate_toggle_button(GTK_TOGGLE_BUTTON(pc->byteCheckB), FALSE);
	}
	break;

      case TOGGLE_RAW:
	Closure->readRaw = state;
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->rawButtonA), state);
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->rawButtonB), state);
	break;

      case TOGGLE_FATAL_SENSE:
	Closure->ignoreFatalSense = state;
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->fatalSenseA), state);
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->fatalSenseB), state);
	break;

      case TOGGLE_EJECT:
	Closure->eject = state;
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->ejectA), state);
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->ejectB), state);
	break;

      case TOGGLE_RAW_20H:
	if(state)
	{  Closure->rawMode = 0x20;
	   activate_toggle_button(GTK_TOGGLE_BUTTON(pc->radioRawMode20A), state);
	   activate_toggle_button(GTK_TOGGLE_BUTTON(pc->radioRawMode20B), state);
	   set_widget_sensitive(pc->rawModeValueA, FALSE);
      	   set_widget_sensitive(pc->rawModeValueB, FALSE);
	   set_entry_text(GTK_ENTRY(pc->rawModeValueA), "0x20");
	   set_entry_text(GTK_ENTRY(pc->rawModeValueB), "0x20");
	}
	break;

      case TOGGLE_RAW_21H:
	if(state)
	{  Closure->rawMode = 0x21;
	   activate_toggle_button(GTK_TOGGLE_BUTTON(pc->radioRawMode21A), state);
	   activate_toggle_button(GTK_TOGGLE_BUTTON(pc->radioRawMode21B), state);
	   set_widget_sensitive(pc->rawModeValueA, FALSE);
	   set_widget_sensitive(pc->rawModeValueB, FALSE);
	   set_entry_text(GTK_ENTRY(pc->rawModeValueA), "0x21");
	   set_entry_text(GTK_ENTRY(pc->rawModeValueB), "0x21");
	}
	break;

      case TOGGLE_RAW_OTHER:
	if(state)
	{  activate_toggle_button(GTK_TOGGLE_BUTTON(pc->radioRawModeOtherA), state);
	   activate_toggle_button(GTK_TOGGLE_BUTTON(pc->radioRawModeOtherB), state);
	   set_widget_sensitive(pc->rawModeValueA, TRUE);
      	   set_widget_sensitive(pc->rawModeValueB, TRUE);
	}
	break;

      case TOGGLE_VERBOSE:
	Closure->verbose = state;
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->verboseA), state);
	activate_toggle_button(GTK_TOGGLE_BUTTON(pc->verboseB), state);
	break;

      case TOGGLE_RANGE:
      {  int image_size = CurrentMediumSize(FALSE) - 1;

	 set_widget_sensitive(pc->rangeSpin1A, state);
	 set_widget_sensitive(pc->rangeSpin1B, state);
	 set_widget_sensitive(pc->rangeSpin2A, state);
	 set_widget_sensitive(pc->rangeSpin2B, state);

	 gtk_spin_button_set_value(GTK_SPIN_BUTTON(pc->rangeSpin1A), 0.0);
	 gtk_spin_button_set_value(GTK_SPIN_BUTTON(pc->rangeSpin1B), 0.0);
	 
	 if(state) 
	 {  gtk_spin_button_set_range(GTK_SPIN_BUTTON(pc->rangeSpin2A), 0.0, image_size);
	    gtk_spin_button_set_range(GTK_SPIN_BUTTON(pc->rangeSpin2B), 0.0, image_size);
	    gtk_spin_button_set_value(GTK_SPIN_BUTTON(pc->rangeSpin2A), image_size);
	    gtk_spin_button_set_value(GTK_SPIN_BUTTON(pc->rangeSpin2B), image_size);
	 }
	 else
	 {  gtk_spin_button_set_range(GTK_SPIN_BUTTON(pc->rangeSpin2A), 0.0, 1.0);
	    gtk_spin_button_set_range(GTK_SPIN_BUTTON(pc->rangeSpin2B), 0.0, 1.0);
	    gtk_spin_button_set_value(GTK_SPIN_BUTTON(pc->rangeSpin2A), 0.1);
	    gtk_spin_button_set_value(GTK_SPIN_BUTTON(pc->rangeSpin2B), 0.1);
	 }

	 activate_toggle_button(GTK_TOGGLE_BUTTON(pc->rangeToggleA), state);
	 activate_toggle_button(GTK_TOGGLE_BUTTON(pc->rangeToggleB), state);
      }
	break;
   }
}

/***
 *** Spin button actions
 ***/

static void spin_cb(GtkWidget *widget, gpointer data)
{  int which = GPOINTER_TO_INT(data);
   int value = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
   prefs_context *pc = (prefs_context*)Closure->prefsContext;

   switch(which)
   {  case SPIN_DELAY:
	Closure->spinupDelay = value;
	if(widget == pc->spinUpA)
	{  if(pc->spinUpB)
	     gtk_spin_button_set_value(GTK_SPIN_BUTTON(pc->spinUpB), value);
	}
	if(widget == pc->spinUpB)
	{  if(pc->spinUpA)
	     gtk_spin_button_set_value(GTK_SPIN_BUTTON(pc->spinUpA), value);
	}
	break;

      case SPIN_INTERNAL_ATTEMPTS:
	Closure->internalAttempts = value;
	if(widget == pc->internalAttemptsA)
	{  if(pc->internalAttemptsB)
	     gtk_spin_button_set_value(GTK_SPIN_BUTTON(pc->internalAttemptsB), value);
	}
	if(widget == pc->internalAttemptsB)
	{  if(pc->internalAttemptsA)
	     gtk_spin_button_set_value(GTK_SPIN_BUTTON(pc->internalAttemptsA), value);
	}
	break;

      case SPIN_READ_MEDIUM:
	Closure->readingPasses = value;
	if(widget == pc->readMediumA)
	{  if(pc->readMediumB)
	     gtk_spin_button_set_value(GTK_SPIN_BUTTON(pc->readMediumB), value);
	}
	if(widget == pc->readMediumB)
	{  if(pc->readMediumA)
	     gtk_spin_button_set_value(GTK_SPIN_BUTTON(pc->readMediumA), value);
	}
	break;
   }
}

/*
 * Make sure the reading range is a valid interval
 */

static void read_range_cb(GtkWidget *widget, gpointer data)
{  prefs_context *pc = (prefs_context*)data;

   if(pc->rangeSpin1A == widget || pc->rangeSpin2A == widget)
   {  int from = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(pc->rangeSpin1A));
      int to = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(pc->rangeSpin2A));

      if(from > to)
      {  if(widget == pc->rangeSpin1A)
	 {    gtk_spin_button_set_value(GTK_SPIN_BUTTON(pc->rangeSpin1A), to);
	      gtk_spin_button_set_value(GTK_SPIN_BUTTON(pc->rangeSpin1B), to);
	 }
	 else 
	 {    gtk_spin_button_set_value(GTK_SPIN_BUTTON(pc->rangeSpin2A), from);
	      gtk_spin_button_set_value(GTK_SPIN_BUTTON(pc->rangeSpin2B), from);
	 }
      }
      else 
      {    gtk_spin_button_set_value(GTK_SPIN_BUTTON(pc->rangeSpin1B), from);
	   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pc->rangeSpin2B), to);
      }
   }
   else
   {  int from = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(pc->rangeSpin1B));
      int to = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(pc->rangeSpin2B));

      if(from > to)
      {  if(widget == pc->rangeSpin1B)
	 {    gtk_spin_button_set_value(GTK_SPIN_BUTTON(pc->rangeSpin1A), to);
	      gtk_spin_button_set_value(GTK_SPIN_BUTTON(pc->rangeSpin1B), to);
	 }
	 else 
	 {    gtk_spin_button_set_value(GTK_SPIN_BUTTON(pc->rangeSpin2A), from);
	      gtk_spin_button_set_value(GTK_SPIN_BUTTON(pc->rangeSpin2B), from);
	 }
      }
      else 
      {    gtk_spin_button_set_value(GTK_SPIN_BUTTON(pc->rangeSpin1A), from);
	   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pc->rangeSpin2A), to);
      }
   }
}

/***
 *** Color buttons
 ***/

static void update_color_buttons()
{  prefs_context *pc = (prefs_context*)Closure->prefsContext;

   gtk_color_button_set_color(GTK_COLOR_BUTTON(pc->redA), Closure->redSector);
   gtk_color_button_set_color(GTK_COLOR_BUTTON(pc->redB), Closure->redSector);

   gtk_color_button_set_color(GTK_COLOR_BUTTON(pc->yellowA), Closure->yellowSector);
   gtk_color_button_set_color(GTK_COLOR_BUTTON(pc->yellowB), Closure->yellowSector);

   gtk_color_button_set_color(GTK_COLOR_BUTTON(pc->greenA), Closure->greenSector);
   gtk_color_button_set_color(GTK_COLOR_BUTTON(pc->greenB), Closure->greenSector);

   gtk_color_button_set_color(GTK_COLOR_BUTTON(pc->blueA), Closure->blueSector);
   gtk_color_button_set_color(GTK_COLOR_BUTTON(pc->blueB), Closure->blueSector);

   gtk_color_button_set_color(GTK_COLOR_BUTTON(pc->whiteA), Closure->whiteSector);
   gtk_color_button_set_color(GTK_COLOR_BUTTON(pc->whiteB), Closure->whiteSector);

   gtk_color_button_set_color(GTK_COLOR_BUTTON(pc->darkA), Closure->darkSector);
   gtk_color_button_set_color(GTK_COLOR_BUTTON(pc->darkB), Closure->darkSector);

   gtk_color_button_set_color(GTK_COLOR_BUTTON(pc->redTextA), Closure->redText);
   gtk_color_button_set_color(GTK_COLOR_BUTTON(pc->redTextB), Closure->redText);

   gtk_color_button_set_color(GTK_COLOR_BUTTON(pc->greenTextA), Closure->greenText);
   gtk_color_button_set_color(GTK_COLOR_BUTTON(pc->greenTextB), Closure->greenText);

   gtk_color_button_set_color(GTK_COLOR_BUTTON(pc->barColorA), Closure->barColor);
   gtk_color_button_set_color(GTK_COLOR_BUTTON(pc->barColorB), Closure->barColor);

   gtk_color_button_set_color(GTK_COLOR_BUTTON(pc->logColorA), Closure->logColor);
   gtk_color_button_set_color(GTK_COLOR_BUTTON(pc->logColorB), Closure->logColor);

   gtk_color_button_set_color(GTK_COLOR_BUTTON(pc->curveColorA), Closure->curveColor);
   gtk_color_button_set_color(GTK_COLOR_BUTTON(pc->curveColorB), Closure->curveColor);
}

static void color_set_cb(GtkWidget *widget, gpointer data)
{
   switch(GPOINTER_TO_INT(data))
   {  case COLOR_RED:
         gtk_color_button_get_color(GTK_COLOR_BUTTON(widget), Closure->redSector);
         break;

      case COLOR_YELLOW:
         gtk_color_button_get_color(GTK_COLOR_BUTTON(widget), Closure->yellowSector);
         break;

      case COLOR_GREEN:
         gtk_color_button_get_color(GTK_COLOR_BUTTON(widget), Closure->greenSector);
         break;

      case COLOR_BLUE:
         gtk_color_button_get_color(GTK_COLOR_BUTTON(widget), Closure->blueSector);
         break;

      case COLOR_WHITE:
         gtk_color_button_get_color(GTK_COLOR_BUTTON(widget), Closure->whiteSector);
         break;

      case COLOR_DARK:
         gtk_color_button_get_color(GTK_COLOR_BUTTON(widget), Closure->darkSector);
         break;

      case COLOR_RED_TEXT:
         gtk_color_button_get_color(GTK_COLOR_BUTTON(widget), Closure->redText);
         break;

      case COLOR_GREEN_TEXT:
         gtk_color_button_get_color(GTK_COLOR_BUTTON(widget), Closure->greenText);
         break;

      case COLOR_BAR:
         gtk_color_button_get_color(GTK_COLOR_BUTTON(widget), Closure->barColor);
         break;

      case COLOR_LOG:
         gtk_color_button_get_color(GTK_COLOR_BUTTON(widget), Closure->logColor);
         break;

      case COLOR_CURVE:
         gtk_color_button_get_color(GTK_COLOR_BUTTON(widget), Closure->curveColor);
         break;
   }

   update_color_buttons();
}

static void default_color_cb(GtkWidget *widget, gpointer data)
{
   GuiDefaultColors();
   update_color_buttons();
}

/***
 *** Non-linear scales
 ***/

static int jump_values[] = { 0, 16, 32, 64, 128, 256, 384, 512, 768, 
			     1024, 2048, 4096, 10240, 20480 };
#define JUMP_VALUE_LENGTH 14

static int min_attempts_values[] = { 1, 2, 3, 4, 5, 7, 9, 11, 15, 20, 25, 30, 40, 50}; 
static int max_attempts_values[] = { 1, 2, 3, 4, 5, 7, 9, 11, 15, 20, 25, 30, 40, 50, 60, 70, 80, 90, 100}; 
#define MIN_ATTEMPTS_VALUE_LENGTH 14
#define MAX_ATTEMPTS_VALUE_LENGTH 19

static void non_linear_cb(GtkWidget *widget, gpointer data)
{  non_linear_info *nli = (non_linear_info*)data;
   int index  = gtk_range_get_value(GTK_RANGE(widget));
   char *text,*utf;

   text = g_strdup_printf(nli->format, nli->values[index]);
   utf = g_locale_to_utf8(text, -1, NULL, NULL, NULL);

   switch(nli->action)
   {  case SLIDER_JUMP:
        Closure->sectorSkip = nli->values[index];

	gtk_range_set_value(GTK_RANGE(nli->pc->jumpScaleB), index);
	gtk_label_set_markup(GTK_LABEL(nli->pc->jumpScaleInfoB->label), utf);

	gtk_range_set_value(GTK_RANGE(nli->pc->jumpScaleA), index);
	gtk_label_set_markup(GTK_LABEL(nli->pc->jumpScaleInfoA->label), utf);
	GuiSetOnlineHelpLinkText(nli->pc->jumpScaleLwoh, text);
	break;

      case SLIDER_MIN_READ_ATTEMPTS:
      { int max  = gtk_range_get_value(GTK_RANGE(nli->pc->maxAttemptsScaleA));

        Closure->minReadAttempts = nli->values[index];

	gtk_range_set_value(GTK_RANGE(nli->pc->minAttemptsScaleB), index);
	gtk_label_set_markup(GTK_LABEL(nli->pc->minAttemptsScaleInfoB->label), utf);

	gtk_range_set_value(GTK_RANGE(nli->pc->minAttemptsScaleA), index);
	gtk_label_set_markup(GTK_LABEL(nli->pc->minAttemptsScaleInfoA->label), utf);
	GuiSetOnlineHelpLinkText(nli->pc->minAttemptsScaleLwoh, text);

	if(index > max)
	{
	   gtk_range_set_value(GTK_RANGE(nli->pc->maxAttemptsScaleA), index);
	   gtk_range_set_value(GTK_RANGE(nli->pc->maxAttemptsScaleB), index);
	}
      }
      break;

      case SLIDER_MAX_READ_ATTEMPTS:
      { int min  = gtk_range_get_value(GTK_RANGE(nli->pc->minAttemptsScaleA));

        Closure->maxReadAttempts = nli->values[index];

	gtk_range_set_value(GTK_RANGE(nli->pc->maxAttemptsScaleB), index);
	gtk_label_set_markup(GTK_LABEL(nli->pc->maxAttemptsScaleInfoB->label), utf);

	gtk_range_set_value(GTK_RANGE(nli->pc->maxAttemptsScaleA), index);
	gtk_label_set_markup(GTK_LABEL(nli->pc->maxAttemptsScaleInfoA->label), utf);
	GuiSetOnlineHelpLinkText(nli->pc->maxAttemptsScaleLwoh, text);

	if(index < min)
	{
	   gtk_range_set_value(GTK_RANGE(nli->pc->minAttemptsScaleA), index);
	   gtk_range_set_value(GTK_RANGE(nli->pc->minAttemptsScaleB), index);
	}
      }
      break;
   }

   g_free(utf);
   g_free(text);
}

static gchar* non_linear_format_cb(GtkScale *scale, gdouble value, gpointer data)
{  char *text;

   text = g_strdup(" ");
   FORGET(text);   /* The scale will free the old string by itself. Weird. */

   return text;
}

static GtkWidget* non_linear_scale(GtkWidget **hbox_out, non_linear_info *nli, 
				   GtkWidget *label, prefs_context *pc, 
				   int action, int *values, int n, int value,
				   int addTooltip)
{  GtkWidget *scale;
   GtkWidget *hbox;
   char *text,*utf;
   int index;

   for(index = 0; index < n; index++)
     if(values[index] > value)
       break;

   nli->action = action;
   nli->values = values;
   nli->pc     = pc;

   hbox = gtk_hbox_new(FALSE, 0);
   scale = gtk_hscale_new_with_range(0,n-1,1);
   gtk_scale_set_value_pos(GTK_SCALE(scale), GTK_POS_RIGHT);
   gtk_range_set_increments(GTK_RANGE(scale), 1, 1);
   gtk_range_set_value(GTK_RANGE(scale), index > 0 ? index-1 : index);
   g_signal_connect(scale, "format-value", G_CALLBACK(non_linear_format_cb), nli);
   g_signal_connect(scale, "value-changed", G_CALLBACK(non_linear_cb), nli);

   gtk_box_pack_start(GTK_BOX(hbox), scale, TRUE, TRUE, 0);

   text = g_strdup_printf(nli->format, nli->values[index > 0 ? index-1 : index]);
   utf = g_locale_to_utf8(text, -1, NULL, NULL, NULL);
   nli->label = label;
   gtk_label_set_markup(GTK_LABEL(label), utf);

   if(nli->lwoh)
   {    GuiSetOnlineHelpLinkText(nli->lwoh, text);
        if (addTooltip) gtk_box_pack_start(GTK_BOX(hbox), nli->lwoh->tooltip, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(hbox), nli->lwoh->linkBox, FALSE, FALSE, 0);
   }
   else gtk_box_pack_start(GTK_BOX(hbox), nli->label, FALSE, FALSE, 0);
   g_free(utf);
   g_free(text);

   *hbox_out = hbox;
   return scale;
}

/*
 * Read strategy selection 
 */

static void strategy_cb(GtkWidget *widget, gpointer data)
{  int state  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
   prefs_context *pc = (prefs_context*)data;

   if(pc->jumpScaleA && pc->jumpScaleB && state == TRUE)
   {  double tmp;

      if(pc->radioLinearA == widget || pc->radioLinearB == widget)
      {  Closure->adaptiveRead = FALSE;
         pc->jumpScaleInfoA->format = pc->formatLinear;
         pc->jumpScaleInfoB->format = pc->formatLinear;
	 activate_toggle_button(GTK_TOGGLE_BUTTON(pc->radioLinearA), TRUE);
	 activate_toggle_button(GTK_TOGGLE_BUTTON(pc->radioLinearB), TRUE);
      }

      if(pc->radioAdaptiveA == widget || pc->radioAdaptiveB == widget)
      {  Closure->adaptiveRead = TRUE;
         pc->jumpScaleInfoA->format = pc->formatAdaptive;
         pc->jumpScaleInfoB->format = pc->formatAdaptive;
	 activate_toggle_button(GTK_TOGGLE_BUTTON(pc->radioAdaptiveA), TRUE);
	 activate_toggle_button(GTK_TOGGLE_BUTTON(pc->radioAdaptiveB), TRUE);

	 if(Closure->readAndCreate)
	 {  Closure->readAndCreate = FALSE;

	    activate_toggle_button(GTK_TOGGLE_BUTTON(pc->readAndCreateButtonA), FALSE);
	    activate_toggle_button(GTK_TOGGLE_BUTTON(pc->readAndCreateButtonB), FALSE);
	    GuiShowMessage(Closure->prefsWindow,
			   _("Disabled automatic error correction file generation."), 
			   GTK_MESSAGE_INFO);
	 }
      }

      /* poor hack to make the range widgets redraw */

      tmp = gtk_range_get_value(GTK_RANGE(pc->jumpScaleA));
      gtk_range_set_value(GTK_RANGE(pc->jumpScaleA), tmp+1.0);
      gtk_range_set_value(GTK_RANGE(pc->jumpScaleA), tmp);
      gtk_range_set_value(GTK_RANGE(pc->jumpScaleB), tmp+1.0);
      gtk_range_set_value(GTK_RANGE(pc->jumpScaleB), tmp);
   }
}

/*
 * Select the mode page value for raw reading
 */

static void rawvalue_cb(GtkWidget *widget, gpointer data)
{  prefs_context *pc = (prefs_context*)data;
   const char *value = gtk_entry_get_text(GTK_ENTRY(widget));
   char new_value[11];

   Closure->rawMode = strtol(value, NULL, 0);

   if(Closure->rawMode < 0)
   {  Closure->rawMode = 0;
      set_entry_text(GTK_ENTRY(pc->rawModeValueA), "0");
      set_entry_text(GTK_ENTRY(pc->rawModeValueB), "0");
      return;
   }

   if(Closure->rawMode > 255)
   {  Closure->rawMode = 255;
   }

   g_snprintf(new_value, 10, "0x%02x", Closure->rawMode);
   set_entry_text(GTK_ENTRY(pc->rawModeValueA), new_value);
   set_entry_text(GTK_ENTRY(pc->rawModeValueB), new_value);
}

/*
 * Select the value for filling unreadable sectors
 */

static void bytefill_cb(GtkWidget *widget, gpointer data)
{  prefs_context *pc = (prefs_context*)data;
   const char *value = gtk_entry_get_text(GTK_ENTRY(widget));
   char byte[11];

   Closure->fillUnreadable = strtol(value, NULL, 0);

   if(Closure->fillUnreadable < 0)
   {  Closure->fillUnreadable = 1;
      set_entry_text(GTK_ENTRY(pc->byteEntryA), "1");
      set_entry_text(GTK_ENTRY(pc->byteEntryB), "1");
      return;
   }

   if(Closure->fillUnreadable > 255)
   {  Closure->fillUnreadable = 255;
   }

   g_snprintf(byte, 10, "0x%02x", Closure->fillUnreadable);
   set_entry_text(GTK_ENTRY(pc->byteEntryA), byte);
   set_entry_text(GTK_ENTRY(pc->byteEntryB), byte);
}


static void bytefill_check_cb(GtkWidget *widget, gpointer data)
{  prefs_context *pc = (prefs_context*)data;
   int state  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

   if(pc->byteCheckA)
     activate_toggle_button(GTK_TOGGLE_BUTTON(pc->byteCheckA), state);
   if(pc->byteCheckB)
     activate_toggle_button(GTK_TOGGLE_BUTTON(pc->byteCheckB), state);

   if(state)
   {  char byte[11];

      set_widget_sensitive(pc->byteEntryA, TRUE);
      set_widget_sensitive(pc->byteEntryB, TRUE);

      if(Closure->fillUnreadable < 0 || Closure->fillUnreadable > 255)
	 Closure->fillUnreadable = 0xb0;

      g_snprintf(byte, 10, "0x%02x", Closure->fillUnreadable);

      set_entry_text(GTK_ENTRY(pc->byteEntryA), byte);
      set_entry_text(GTK_ENTRY(pc->byteEntryB), byte);

      activate_toggle_button(GTK_TOGGLE_BUTTON(pc->dsmButtonA), FALSE);
      activate_toggle_button(GTK_TOGGLE_BUTTON(pc->dsmButtonB), FALSE);

   }
   else
   {  Closure->fillUnreadable = -1;
      set_widget_sensitive(pc->byteEntryA, FALSE);
      set_widget_sensitive(pc->byteEntryB, FALSE);
   }
}

/*
 * Enter the prefix for raw sector files
 */

static void defective_prefix_cb(GtkWidget *widget, gpointer data)
{  prefs_context *pc = (prefs_context*)data;
   char *value = (char*)gtk_entry_get_text(GTK_ENTRY(widget));

   if(!value || !*value)
   {  set_entry_text(GTK_ENTRY(pc->cacheDefectivePrefixA), "sector-");
      set_entry_text(GTK_ENTRY(pc->cacheDefectivePrefixB), "sector-");
      return;

   }

   if(widget == pc->cacheDefectivePrefixA)
      set_entry_text(GTK_ENTRY(pc->cacheDefectivePrefixB), value);

   if(widget == pc->cacheDefectivePrefixB)
      set_entry_text(GTK_ENTRY(pc->cacheDefectivePrefixA), value);
}

/*
 * Run the file chooser for the raw sector files directory
 */

static void cache_defective_dir_cb(GtkWidget *widget, gpointer data)
{  prefs_context *pc = (prefs_context*)data;
   GtkWidget *dialog;

   if(!pc->cacheDefectiveChooser)
   {  char filename[strlen(Closure->dDumpDir)+10];

      if (!Closure->reverseCancelOK)
         dialog = gtk_file_chooser_dialog_new("Raw sector caching",
                                              Closure->window,
                                              GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                              GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                              NULL);
      else
         dialog = gtk_file_chooser_dialog_new("Raw sector caching",
                                              Closure->window,
                                              GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                              GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                              NULL);
      sprintf(filename, "%s/", Closure->dDumpDir);
      gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), filename);

      if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
      {  if(Closure->dDumpDir)
            g_free(Closure->dDumpDir);
         Closure->dDumpDir = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
         if(pc->cacheDefectiveDirA)
            gtk_label_set_text(GTK_LABEL(pc->cacheDefectiveDirA), Closure->dDumpDir);
         if(pc->cacheDefectiveDirB)
            gtk_label_set_text(GTK_LABEL(pc->cacheDefectiveDirB), Closure->dDumpDir);
      }
      gtk_widget_destroy (dialog);
   }
}


/*
 * Run the file chooser for the log file
 */

static void logfile_select_cb(GtkWidget *widget, gpointer data)
{  prefs_context *pc = (prefs_context*)Closure->prefsContext;
   GtkWidget *dialog;

   if(!pc->logFileChooser)
   {  if (!Closure->reverseCancelOK)
         dialog = gtk_file_chooser_dialog_new("Log file",
                                              Closure->window,
                                              GTK_FILE_CHOOSER_ACTION_SAVE,
                                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                              GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                              NULL);
      else
         dialog = gtk_file_chooser_dialog_new("Log file",
                                              Closure->window,
                                              GTK_FILE_CHOOSER_ACTION_SAVE,
                                              GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                              NULL);
      gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), Closure->logFile);

      if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
      {  g_free(Closure->logFile);
         Closure->logFile = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
         Closure->logFileStamped = FALSE;
         if(pc->logFilePathA)
            gtk_label_set_text(GTK_LABEL(pc->logFilePathA), Closure->logFile);
         if(pc->logFilePathB)
            gtk_label_set_text(GTK_LABEL(pc->logFilePathB), Closure->logFile);
      }
      gtk_widget_destroy (dialog);
   }
}

static void logfile_delete_cb(GtkWidget *widget, gpointer data)
{  GtkWidget *dialog = gtk_message_dialog_new(Closure->prefsWindow,
                                              GTK_DIALOG_DESTROY_WITH_PARENT,
                                              GTK_MESSAGE_QUESTION,
                                              GTK_BUTTONS_OK_CANCEL,
                                              "%s", _utf("Delete the log file?"));
   int answer;

   GuiReverseCancelOK(GTK_DIALOG(dialog));
   answer = gtk_dialog_run(GTK_DIALOG(dialog));

   if(answer == GTK_RESPONSE_OK)
      LargeUnlink(Closure->logFile);
   gtk_widget_destroy(dialog);
}

/***
 *** Error correction method selection
 ***/

static void method_select_cb(GtkWidget *widget, gpointer data)
{  Method *method;
   prefs_context *pc = (prefs_context*)data;
   int n;

   n = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));

   if(n<0 || !pc->methodNotebook)
     return;

   method = g_ptr_array_index(Closure->methodList, n);

   /* Switch methods if selection changed */

   if(strncmp(Closure->methodName, method->name, 4))
   {  GtkWidget *other;

      strncpy(Closure->methodName, method->name, 4);
      gtk_notebook_set_current_page(GTK_NOTEBOOK(pc->methodNotebook), n);

      if(pc->methodChooserA == widget)
	   other = pc->methodChooserB;
      else other = pc->methodChooserA;

      gtk_combo_box_set_active(GTK_COMBO_BOX(other), n);
   }
}

/*
 * Setting the notebook page does not work at creation time.
 */

static gboolean notebook_idle_func(gpointer data)
{  prefs_context *pc = (prefs_context*)data;
   int n;

   n = gtk_combo_box_get_active(GTK_COMBO_BOX(pc->methodChooserA));

   if(n>=0)
     gtk_notebook_set_current_page(GTK_NOTEBOOK(pc->methodNotebook), n);

   gtk_notebook_set_current_page(GTK_NOTEBOOK(pc->mainNotebook), 0);

   return FALSE;
}

/***
 *** Assemble and open the preferences window.
 ***/

void GuiUpdateMethodPreferences(void)
{  int i;
   
   for(i=0; i<Closure->methodList->len; i++)
   {  Method *method = g_ptr_array_index(Closure->methodList, i);

      if(method->resetPrefsPage)
	method->resetPrefsPage(method);
   }
}

void GuiCreatePreferencesWindow(void)
{  
   if(!Closure->prefsWindow)  /* No window to reuse? */
   {  GtkWidget *window, *outer_box, *notebook, *space;
      GtkWidget *hbox, *vbox, *vbox2, *vbox3, *button, *frame, *table;
      GtkWidget *lab;
      LabelWithOnlineHelp *lwoh,*lwoh_clone;
      prefs_context *pc = g_malloc0(sizeof(prefs_context));
      int i, method_idx = 0;
 
      pc->helpPages = g_ptr_array_new();
      Closure->prefsContext = pc;
 
      window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
      Closure->prefsWindow = GTK_WINDOW(window);
      gtk_window_set_title(GTK_WINDOW(window), _utf("Preferences"));
      gtk_window_set_default_size(GTK_WINDOW(window), -1, 150);
      gtk_window_set_icon(GTK_WINDOW(window), Closure->windowIcon);
      gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
      gtk_container_set_border_width(GTK_CONTAINER(window), 12);

      /* Connect with the close button from the window manager */

      g_signal_connect(window, "delete_event", G_CALLBACK(delete_cb), NULL);

      /* Create the main layout of the window */

      outer_box = gtk_vbox_new(FALSE, 0);
      gtk_container_add(GTK_CONTAINER(window), outer_box);

      notebook = pc->mainNotebook = gtk_notebook_new();
      gtk_box_pack_start(GTK_BOX(outer_box), notebook, TRUE, TRUE, 0);

      space = gtk_image_new();
      gtk_box_pack_start(GTK_BOX(outer_box), space, FALSE, FALSE, 4);

      hbox = gtk_hbox_new(FALSE, 0);
      gtk_box_pack_start(GTK_BOX(outer_box), hbox, FALSE, FALSE, 0);

      button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
      gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, FALSE, 0);
      g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(close_cb), NULL);

      /*** Image creation page */

      vbox = create_page(notebook, _utf("Image"));

      /** Reading preferences */
      
      frame = gtk_frame_new(_utf("Image creation"));
      gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

      vbox2 = gtk_vbox_new(FALSE, 20);
      gtk_container_set_border_width(GTK_CONTAINER(vbox2), 10);
      gtk_container_add(GTK_CONTAINER(frame), vbox2);

      /* Reading strategy */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Reading strategy"),
					  _("Reading strategy: "));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox = gtk_hbox_new(FALSE, 4);
	 GtkWidget *lab, *radio1, *radio2;

	 gtk_box_pack_start(GTK_BOX(hbox), i ? lwoh->normalLabel : lwoh->linkBox, FALSE, FALSE, 0);
         if(!i) gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);

	 radio1 = gtk_radio_button_new(NULL);
	 if(!i) pc->radioLinearA = radio1;
	 else   pc->radioLinearB = radio1;
	 g_signal_connect(G_OBJECT(radio1), "toggled", G_CALLBACK(strategy_cb), pc);
	 gtk_box_pack_start(GTK_BOX(hbox), radio1, FALSE, FALSE, 0);
	 lab = gtk_label_new(_utf("Linear"));
	 gtk_container_add(GTK_CONTAINER(radio1), lab);

	 radio2 = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(radio1));
	 if(!i) pc->radioAdaptiveA = radio2;
	 else   pc->radioAdaptiveB = radio2;
 	 g_signal_connect(G_OBJECT(radio2), "toggled", G_CALLBACK(strategy_cb), pc);
	 gtk_box_pack_start(GTK_BOX(hbox), radio2, FALSE, FALSE, 0);
	 lab = gtk_label_new(_utf("Adaptive (for defective media)"));
	 gtk_container_add(GTK_CONTAINER(radio2), lab);

	 if(Closure->adaptiveRead)
	      activate_toggle_button(GTK_TOGGLE_BUTTON(radio2), TRUE);
         else activate_toggle_button(GTK_TOGGLE_BUTTON(radio1), TRUE);

	 if(!i) gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
	 else   GuiAddHelpWidget(lwoh, hbox);
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Reading strategy</b>\n\n"
	   "Use the <b>linear strategy</b> for:\n"
	   "- processing undamaged media, or\n"
	   "- reading defective media when no error correction data is available.\n\n"
	   "The <b>adaptive strategy</b> is optimized for\n"
	   "- reading defective media\n"
	   "- if (and only if) error correction data is available.\n\n"
	   "Using the adaptive strategy without error correction data "
	   "is possible but it is recommended to use linear reading in that case."));

      /* Reading range */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Reading range"),
					  _("Read/Scan from sector"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox = gtk_hbox_new(FALSE, 4);
	 GtkWidget *toggle,*spin1, *spin2;

	 //	 toggle = gtk_check_button_new_with_label(_utf("Read/Scan from sector"));
	 toggle = gtk_check_button_new();
	 if(!i) pc->rangeToggleA = toggle;
	 else   pc->rangeToggleB = toggle;
	 g_signal_connect(G_OBJECT(toggle), "toggled", G_CALLBACK(toggle_cb), GINT_TO_POINTER(TOGGLE_RANGE));
	 gtk_box_pack_start(GTK_BOX(hbox), toggle, FALSE, FALSE, 0);

	 gtk_box_pack_start(GTK_BOX(hbox), i ? lwoh->normalLabel : lwoh->linkBox, 
			    FALSE, FALSE, 0);
         if(!i) gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);

	 spin1 = gtk_spin_button_new_with_range(0, 10000000, 1000);
	 if(!i) pc->rangeSpin1A = spin1;	 
	 else   pc->rangeSpin1B = spin1;
	 gtk_entry_set_width_chars(GTK_ENTRY(spin1), 9);
	 set_widget_sensitive(spin1, FALSE);
	 g_signal_connect(spin1, "value-changed", G_CALLBACK(read_range_cb), pc);
	 gtk_box_pack_start(GTK_BOX(hbox), spin1, FALSE, FALSE, 0);

	 lab = gtk_label_new(_utf("to sector"));
	 gtk_box_pack_start(GTK_BOX(hbox), lab, FALSE, FALSE, 0);

	 spin2 = gtk_spin_button_new_with_range(0, 1, 10000);
	 if(!i) pc->rangeSpin2A = spin2;	 
	 else   pc->rangeSpin2B = spin2;
	 gtk_entry_set_width_chars(GTK_ENTRY(spin2), 9);
	 set_widget_sensitive(spin2, FALSE);
	 g_signal_connect(spin2, "value-changed", G_CALLBACK(read_range_cb), pc);
	 gtk_box_pack_start(GTK_BOX(hbox), spin2, FALSE, FALSE, 0);

	 if(!i) gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
	 else   GuiAddHelpWidget(lwoh, hbox);

	 activate_toggle_button(GTK_TOGGLE_BUTTON(toggle), FALSE);
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Reading range</b>\n\n"
	   "Reading can be limited to a part of the medium (in sectors holding 2KB each). "
	   "The values include the borders: 0-100 will read 101 sectors.\n\n"

	   "<b>Note:</b> Limiting the reading range is not recommended for <i>adaptive reading</i> since it might "
	   "prevent sectors from being read which are required for a successful error correction.\n\n"
	   "These settings are only effective for the current session and will not be saved."));

      /*** Image recognization */

      frame = gtk_frame_new(_utf("Error correction data recognization"));
      gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

      vbox2 = gtk_vbox_new(FALSE, 15);
      gtk_container_set_border_width(GTK_CONTAINER(vbox2), 10);
      gtk_container_add(GTK_CONTAINER(frame), vbox2);

      /* RS02 */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Exhaustive RS02 header search"),
					  _("Perform exhaustive search for RS02 headers"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	 GtkWidget *button = gtk_check_button_new();

	 gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
	 gtk_box_pack_start(GTK_BOX(hbox), i ? lwoh->normalLabel : lwoh->linkBox, FALSE, FALSE, 0);
         if(!i) gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);

	 if(!i) pc->recogRS02A = button;
	 else   pc->recogRS02B = button;

	 activate_toggle_button(GTK_TOGGLE_BUTTON(button), Closure->examineRS02);
	 g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(toggle_cb), GINT_TO_POINTER(TOGGLE_RECOG_RS02));
	 if(!i) gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
	 else   GuiAddHelpWidget(lwoh, hbox);
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Exhaustive RS02 header search</b>\n\n"
	   "When this setting is off only a quick check "
	   "for RS02 data is performed. If the medium or "
	   "image is damaged, the quick test may not suffice "
	   "to identify the image as being augmented with RS02.\n\n"
	   "Therefore you should turn this option <b>on</b> "
	   "if a medium/image contains RS02 data, but is not "
	   "being recognized as such. Searching for the RS02 "
	   "information may cause a significant delay at the "
	   "start of reading and scanning processes.\n\n"
	   "Leave this option <b>off</b> when you are "
	   "processing media or images which are not augmented "
	   "with RS02 data. Otherwise you will waste a lot of "
	   "time searching for the RS02 signatures and increase "
	   "wear on the drive."
	   ));

      /* RS03 */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Recover RS03 signatures"),
					  _("Find and recover RS03 signatures"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	 GtkWidget *button = gtk_check_button_new();

	 gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
	 gtk_box_pack_start(GTK_BOX(hbox), i ? lwoh->normalLabel : lwoh->linkBox, FALSE, FALSE, 0);
         if(!i) gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);

	 if(!i) pc->recogRS03A = button;
	 else   pc->recogRS03B = button;

	 activate_toggle_button(GTK_TOGGLE_BUTTON(button), Closure->examineRS03);
	 g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(toggle_cb), GINT_TO_POINTER(TOGGLE_RECOG_RS03));
	 if(!i) gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
	 else   GuiAddHelpWidget(lwoh, hbox);
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Exhaustive RS03 header search</b>\n\n"
	   "When this setting is off only a quick check "
	   "for RS03 data is performed. If the medium or "
	   "image is damaged, the quick test may not suffice "
	   "to identify the image as being augmented with RS03.\n\n"
	   "Therefore you should turn this option <b>on</b> "
	   "if a medium/image contains RS03 data, but is not "
	   "being recognized as such. Searching for the RS03 "
	   "information may cause a significant delay at the "
	   "start of reading and scanning processes.\n\n"
	   "Leave this option <b>off</b> when you are "
	   "processing media or images which are not augmented "
	   "with RS03 data. Otherwise you will waste a lot of "
	   "time searching for the RS03 signatures and increase "
	   "wear on the drive."
	   ));

      /** Image properties */

      frame = gtk_frame_new(_utf("Image properties"));
      gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

      vbox2 = gtk_vbox_new(FALSE, 15);
      gtk_container_set_border_width(GTK_CONTAINER(vbox2), 10);
      gtk_container_add(GTK_CONTAINER(frame), vbox2);

      /* Query size from drive */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Ignore ISO/UDF meta data"),
					  _("Ignore image size recorded in ISO/UDF file system"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	 GtkWidget *button = gtk_check_button_new();

	 gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
	 gtk_box_pack_start(GTK_BOX(hbox), i ? lwoh->normalLabel : lwoh->linkBox, FALSE, FALSE, 0);
         if(!i) gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);

	 if(!i) pc->ignoreISOSizeA = button;
	 else   pc->ignoreISOSizeB = button;

	 activate_toggle_button(GTK_TOGGLE_BUTTON(button), Closure->ignoreIsoSize);
	 g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(toggle_cb), GINT_TO_POINTER(TOGGLE_SIZEDRIVE));
	 if(!i) gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
	 else   GuiAddHelpWidget(lwoh, hbox);
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Ignore image size recorded in ISO/UDF filesystem</b>\n\n"
	   "When reading or scanning optical discs, the overall size of the medium "
	   "needs to be determined. dvdisaster will always use the image size "
	   "recorded in the error correction data if such data is present. "
	   "Otherwise, image size is queried in the following order:\n\n"
	   "1. Image size recorded in the ISO/UDF file system\n"
	   "2. Image size reported by the optical drive.\n\n"
	   "Using this order makes sense as image sizes reported by most drives "
	   "are unreliable in many cases. However in some rare cases "
	   "the image size recorded in the ISO/UDF filesystem is wrong. Some "
	   "GNU/Linux live CDs may have this problem. If you read back the ISO "
	   "image from such CDs and its md5sum does not match the advertised one, "
	   "try re-reading the image with this option turned on.\n"
	   "Do <b>not blindly turn this option on</b> as it will most likely "
	   "create sub optimal or corrupted ISO images, especially if you "
	   "plan to use the image for error correction data generation."));

      /* DAO button */

      lwoh = GuiCreateLabelWithOnlineHelp(_("DAO mode"),
					  _("Assume image to be written in DAO mode (don't truncate)"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	 GtkWidget *button = gtk_check_button_new();

	 gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
	 gtk_box_pack_start(GTK_BOX(hbox), i ? lwoh->normalLabel : lwoh->linkBox, FALSE, FALSE, 0);
         if(!i) gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);

	 if(!i) pc->daoButtonA = button;
	 else   pc->daoButtonB = button;

	 activate_toggle_button(GTK_TOGGLE_BUTTON(button), Closure->noTruncate);
	 g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(toggle_cb), GINT_TO_POINTER(TOGGLE_DAO));
	 if(!i) gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
	 else   GuiAddHelpWidget(lwoh, hbox);
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Assume DAO mode</b>\n\n"
	   "Media written in \"TAO\" (\"track at once\") mode may contain two sectors "
	   "with pseudo read errors at the end. By default these two sectors are ignored.\n\n"

	   "If you are extremely unlucky to have a \"DAO\" (\"disc at once\") medium "
	   "with exactly one or two real read errors at the end, dvdisaster may treat "
	   "this as a \"TAO\" disc and truncate the image by two sectors. In that case "
	   "activate this option to have the last two read errors handled correctly.\n\n"

	   "<b>Tip:</b> To avoid these problems, consider using the \"DAO / Disc at once\" "
	   "(sometimes also called \"SAO / Session at once\") mode for writing single "
	   "session media."));

      /*** Image format */

      frame = gtk_frame_new(_utf("Image format"));
      gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

      vbox2 = gtk_vbox_new(FALSE, 15);
      gtk_container_set_border_width(GTK_CONTAINER(vbox2), 10);
      gtk_container_add(GTK_CONTAINER(frame), vbox2);

      /* new style missing sector marker */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Missing sector tags"),
					  _("Use old style missing sector tags (not recommended)"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	 GtkWidget *button = gtk_check_button_new();

	 gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
	 gtk_box_pack_start(GTK_BOX(hbox), i ? lwoh->normalLabel : lwoh->linkBox, FALSE, FALSE, 0);
         if(!i) gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);

	 if(!i) pc->dsmButtonA = button;
	 else   pc->dsmButtonB = button;

	 activate_toggle_button(GTK_TOGGLE_BUTTON(button), !Closure->dsmVersion);
	 g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(toggle_cb), GINT_TO_POINTER(TOGGLE_DSM));
	 if(!i) gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
	 else   GuiAddHelpWidget(lwoh, hbox);
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Missing sector tagging</b>\n\n"
	   "Missing sectors are tagged with a special code sequence "
	   "in the image. By default, an improved "
	   "code is used which can detect some wilfully damaged "
	   "content. This includes media which have been created "
	   "from partially recovered images, and images containing "
	   "files from such partial media.\n"
	   "However only dvdisaster 0.72 and up will recognize "
	   "the improved tags. Activate this switch to force using the "
	   "older format when this image will be processed with older "
	   "dvdisaster versions. Otherwise the older dvdisaster versions "
	   "will not see any missing sectors in the resulting images.\n"
	   "N.b.: dvdisaster >= 0.72 will automatically recognize "
	   "both tag formats when reading images; setting this value "
	   "only affects the creation of new images."));

      /** byte filling */

      if(Closure->debugMode)
      {
	 lwoh = GuiCreateLabelWithOnlineHelp(_("Filling of unreadable sectors"), 
					     _("Fill unreadable sectors with byte:"));
	 GuiRegisterPreferencesHelpWindow(lwoh);

	 for(i=0; i<2; i++)
	 {  GtkWidget *hbox = gtk_hbox_new(FALSE, 4);
	    GtkWidget *check, *entry;

	    check = gtk_check_button_new();
	    g_signal_connect(check, "toggled", G_CALLBACK(bytefill_check_cb), pc);
	    gtk_box_pack_start(GTK_BOX(hbox), check, FALSE, FALSE, 0);
	    gtk_box_pack_start(GTK_BOX(hbox), i ? lwoh->normalLabel : lwoh->linkBox, FALSE, FALSE, 0);
            if(!i) gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);

	    entry = gtk_entry_new();
	    g_signal_connect(entry, "activate", G_CALLBACK(bytefill_cb), pc);
	    gtk_entry_set_width_chars(GTK_ENTRY(entry), 5);
	    gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 0);

	    if(!i)
	    {  pc->byteCheckA = check;
	       pc->byteEntryA = entry;
	       gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
	    }
	    else
	    {  pc->byteCheckB = check;
	       pc->byteEntryB = entry;
	       GuiAddHelpWidget(lwoh, hbox);
	    }

	    if(Closure->fillUnreadable >= 0)
	    {  char value[11];
	
	       g_snprintf(value, 10, "0x%02x", Closure->fillUnreadable);
	       gtk_entry_set_text(GTK_ENTRY(entry), value);
	       activate_toggle_button(GTK_TOGGLE_BUTTON(check), TRUE);
	    }
	    else set_widget_sensitive(entry, FALSE);
	 }

	 GuiAddHelpParagraph(lwoh, 
	    _("<b>Filling of unreadable sectors</b>\n\n"
			    
	      "dvdisaster marks unreadable sectors with a special filling pattern which "
	      "is very unlikely to occur in undamaged media.\n"
	      "In other data recovery software it is common to fill unreadable sectors "
	      "with a certain byte value. To allow interoperability with such programs, "
	      "you can specify the byte value they are using:\n"));

	 GuiAddHelpListItem(lwoh,
	    _("0xb0 (176 decimal): for compatibility with h2cdimage published by \"c't\", "
	      "a German periodical.\n"));

	 GuiAddHelpParagraph(lwoh,
	    _("<b>Note:</b> Using zero filling (0x00, decimal 0) is highly discouraged. "
	      "Most media contain regular zero filled sectors which can not be told apart "
	      "from unreadable sectors if zero filling is used."));
      }

      /*** Drive parameters page */

      vbox = create_page(notebook, _utf("Drive"));

      /** Drive initialisation */

      frame = gtk_frame_new(_utf("Drive initialisation"));
      gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

      lwoh = GuiCreateLabelWithOnlineHelp(_("Drive initialisation"), 
					  _("Wait"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      lwoh_clone = GuiCloneLabelWithOnlineHelp(lwoh, _("seconds for drive to spin up"));
      GuiRegisterPreferencesHelpWindow(lwoh_clone);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox = gtk_hbox_new(FALSE, 4);
	 GtkWidget *spin;

	 gtk_box_pack_start(GTK_BOX(hbox), i ? lwoh->normalLabel : lwoh->linkBox, FALSE, FALSE, 0);

	 spin = gtk_spin_button_new_with_range(0, 30, 1);
	 gtk_entry_set_width_chars(GTK_ENTRY(spin), 3);
	 gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), Closure->spinupDelay);
	 g_signal_connect(spin, "value-changed", G_CALLBACK(spin_cb), (gpointer)SPIN_DELAY);
	 gtk_box_pack_start(GTK_BOX(hbox), spin, FALSE, FALSE, 0);

	 gtk_box_pack_start(GTK_BOX(hbox), i ? lwoh_clone->normalLabel : lwoh_clone->linkBox, FALSE, FALSE, 0);
         if(!i) gtk_box_pack_start(GTK_BOX(hbox), lwoh_clone->tooltip, FALSE, FALSE, 0);
	 gtk_container_set_border_width(GTK_CONTAINER(hbox), 10);

	 if(!i)
	 {  pc->spinUpA = spin;
	    gtk_container_add(GTK_CONTAINER(frame), hbox);
	 }
	 else
	 {  pc->spinUpB = spin;
	    GuiAddHelpWidget(lwoh, hbox);
	 }
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Drive initialisation</b>\n\n"
	   "Waits the specified amount of seconds for letting the drive spin up. "
	   "This avoids speed jumps at the beginning of the reading curve."));

      /** Drive raw reading parameters */

      frame = gtk_frame_new(_utf("Raw reading parameters"));
      gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

      vbox2 = gtk_vbox_new(FALSE, 15);
      gtk_container_set_border_width(GTK_CONTAINER(vbox2), 10);
      gtk_container_add(GTK_CONTAINER(frame), vbox2);

      /* Raw reading mode */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Raw reading mode"),
					  _("Raw reading mode: "));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox = gtk_hbox_new(FALSE, 4);
	 GtkWidget *lab, *radio1, *radio2, *radio3, *entry;
	 char value[11];

	 gtk_box_pack_start(GTK_BOX(hbox), i ? lwoh->normalLabel : lwoh->linkBox, FALSE, FALSE, 0);
         if(!i) gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);

	 radio1 = gtk_radio_button_new(NULL);
	 if(!i) pc->radioRawMode20A = radio1;
	 else   pc->radioRawMode20B = radio1;
	 g_signal_connect(G_OBJECT(radio1), "toggled", G_CALLBACK(toggle_cb), GINT_TO_POINTER(TOGGLE_RAW_20H));
	 gtk_box_pack_start(GTK_BOX(hbox), radio1, FALSE, FALSE, 0);
	 lab = gtk_label_new("0x20");
	 gtk_container_add(GTK_CONTAINER(radio1), lab);

	 radio2 = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(radio1));
	 if(!i) pc->radioRawMode21A = radio2;
	 else   pc->radioRawMode21B = radio2;
	 g_signal_connect(G_OBJECT(radio2), "toggled", G_CALLBACK(toggle_cb), GINT_TO_POINTER(TOGGLE_RAW_21H));
	 gtk_box_pack_start(GTK_BOX(hbox), radio2, FALSE, FALSE, 0);
	 lab = gtk_label_new("0x21");
	 gtk_container_add(GTK_CONTAINER(radio2), lab);

	 radio3 = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(radio2));
	 if(!i) pc->radioRawModeOtherA = radio3;
	 else   pc->radioRawModeOtherB = radio3;
 	 g_signal_connect(G_OBJECT(radio3), "toggled", G_CALLBACK(toggle_cb), GINT_TO_POINTER(TOGGLE_RAW_OTHER));
	 gtk_box_pack_start(GTK_BOX(hbox), radio3, FALSE, FALSE, 0);
	 lab = gtk_label_new(_utf("other:"));
	 gtk_container_add(GTK_CONTAINER(radio3), lab);

	 entry = gtk_entry_new();
	 g_signal_connect(entry, "activate", G_CALLBACK(rawvalue_cb), pc);
	 gtk_entry_set_width_chars(GTK_ENTRY(entry), 5);
	 gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 0);
	 if(!i) pc->rawModeValueA = entry;
	 else   pc->rawModeValueB = entry;
	
	 g_snprintf(value, 10, "0x%02x", Closure->rawMode);
	 gtk_entry_set_text(GTK_ENTRY(entry), value);

	 switch(Closure->rawMode)
	 {  case 0x20:
	       activate_toggle_button(GTK_TOGGLE_BUTTON(radio1), TRUE);
	       set_widget_sensitive(entry, FALSE);
	       break;

	    case 0x21:
	       activate_toggle_button(GTK_TOGGLE_BUTTON(radio2), TRUE);
	       set_widget_sensitive(entry, FALSE);
	       break;

	    default:
	       activate_toggle_button(GTK_TOGGLE_BUTTON(radio3), TRUE);
	       break;
	 }

	 if(!i) gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
	 else   GuiAddHelpWidget(lwoh, hbox);
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Raw reading mode</b>\n\n"
	   "There are several ways to put the drive into a mode "
	   "which transfers partially read data from defective sectors:\n\n"
	   "<b>0x20</b> This is the <i>recommended</i> mode. "
	   "The drive tries to apply "
	   "the built-in error correction to the best possible extent "
	   "before transferring a defective sector.\n\n"
	   "<b>0x21</b> In this mode the drive skips the last stage "
	   "of its internal error correction and returns the "
	   "uncorrected sector instead. This may result in sectors "
	   "being tagged and processed as defective which would come "
	   "out good in other reading modes, causing unnecessary "
	   "work or even uncorrectable sectors.\n" 
	   "However some drives appear to be unable to transfer data "
	   "in mode 0x20, but can do so in mode 0x21, so this is your "
	   "last resort then. Also, if sectors are not recoverable "
	   "after reading and caching sectors in mode 0x20, then adding "
	   "some mode 0x21 reads to the cache might deliver additional information.\n\n"
	   "<b>0x01</b> Some drives do the right thing when given this value, "
	   "although this makes no sense according to the SCSI specs. Try entering "
	   "this value in the \"other\" field if the other choices do not work. "
	   "See the mode page 01h documentation in chapter 6 of MMC3 or later "
	   "for additional information."));

      /* Firmware rereads */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Internal read attempts"), 
					  _("Reread defective sectors"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      lwoh_clone = GuiCloneLabelWithOnlineHelp(lwoh, _("times"));
      GuiRegisterPreferencesHelpWindow(lwoh_clone);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox = gtk_hbox_new(FALSE, 4);
	 GtkWidget *spin;

	 gtk_box_pack_start(GTK_BOX(hbox), i ? lwoh->normalLabel : lwoh->linkBox, FALSE, FALSE, 0);
         if(!i) gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);

	 spin = gtk_spin_button_new_with_range(-1, 10, 1);
	 gtk_entry_set_width_chars(GTK_ENTRY(spin), 3);
	 gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), Closure->internalAttempts);
	 g_signal_connect(spin, "value-changed", G_CALLBACK(spin_cb), (gpointer)SPIN_INTERNAL_ATTEMPTS);
	 gtk_box_pack_start(GTK_BOX(hbox), spin, FALSE, FALSE, 0);

	 gtk_box_pack_start(GTK_BOX(hbox), i ? lwoh_clone->normalLabel : lwoh_clone->linkBox, FALSE, FALSE, 0);
	 //	 gtk_container_set_border_width(GTK_CONTAINER(hbox), 10);

	 if(!i)
	 {  pc->internalAttemptsA = spin;
	    gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
	    //	    gtk_container_add(GTK_CONTAINER(frame), hbox);
	 }
	 else
	 {  pc->internalAttemptsB = spin;
	    GuiAddHelpWidget(lwoh, hbox);
	 }
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Internal read attempts</b>\n\n"
	   "The drive firmware usually retries unreadable sectors "
	   "a few times before giving up and returning a read error.\n"
	   "But it is usually more efficient to manage the reading "
	   "attempts from the client software, e.g. through the "
	   "settings in the \"Read attempts\" preferences tab.\n"
	   "Lowering this value to 0 or 1 can speed up processing "
	   "of damaged media and reduce the drive wear; however "
	   "most drives will simply ignore what you enter here.\n"
	   "Use the value -1 to leave the drive at its default setting."));

      /* Fatal error handling */

      frame = gtk_frame_new(_utf("Fatal error handling"));
      gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

      lwoh = GuiCreateLabelWithOnlineHelp(_("Fatal error handling"), 
					  _("Ignore fatal errors"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox = gtk_hbox_new(FALSE, 4);
	 GtkWidget *toggle;

	 toggle = gtk_check_button_new();
	 activate_toggle_button(GTK_TOGGLE_BUTTON(toggle), Closure->ignoreFatalSense);
       	 g_signal_connect(G_OBJECT(toggle), "toggled", G_CALLBACK(toggle_cb), GINT_TO_POINTER(TOGGLE_FATAL_SENSE));
	 gtk_box_pack_start(GTK_BOX(hbox), toggle, FALSE, FALSE, 0);

	 if(!i)
	 {  pc->fatalSenseA = toggle;
	    gtk_box_pack_start(GTK_BOX(hbox), lwoh->linkBox, FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);
	    gtk_container_add(GTK_CONTAINER(frame), hbox);
	    gtk_container_set_border_width(GTK_CONTAINER(hbox), 12);
	 }
	 else
	 {  pc->fatalSenseB = toggle;
	    gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);
	    GuiAddHelpWidget(lwoh, hbox);
	 }
      }	 

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Fatal error handling</b>\n\n"
	   "By default dvdisaster stops reading when the drive "
	   "reports a fatal error. This prevents further fruitless "
	   "read attempts and possible damage to the drive.\n"
	   "However some drives produce unfounded fatal messages. "
	   "For such drives ignoring fatal errors may be needed to "
	   "do uninterrupted reading of damaged media."));

      /* Eject medium */

      frame = gtk_frame_new(_utf("Media ejection"));
      gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

      lwoh = GuiCreateLabelWithOnlineHelp(_("Eject medium after successful read"), 
					  _("Eject medium after successful read"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox = gtk_hbox_new(FALSE, 4);
	 GtkWidget *toggle;

	 toggle = gtk_check_button_new();
	 activate_toggle_button(GTK_TOGGLE_BUTTON(toggle), Closure->eject);
       	 g_signal_connect(G_OBJECT(toggle), "toggled", G_CALLBACK(toggle_cb), GINT_TO_POINTER(TOGGLE_EJECT));
	 gtk_box_pack_start(GTK_BOX(hbox), toggle, FALSE, FALSE, 0);

	 if(!i)
	 {  pc->ejectA = toggle;
	    gtk_box_pack_start(GTK_BOX(hbox), lwoh->linkBox, FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);
	    gtk_container_add(GTK_CONTAINER(frame), hbox);
	    gtk_container_set_border_width(GTK_CONTAINER(hbox), 12);
	 }
	 else
	 {  pc->ejectB = toggle;
	    gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);
	    GuiAddHelpWidget(lwoh, hbox);
	 }
      }	 

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Medium ejection</b>\n\n"
	   "Activate this option to have the medium ejected after "
	   "a successful read or scan operation.\n\n"
	   "Note that the desktop environment "
	   "may prevent other applications from ejecting media. "
	   "In that case eject the medium through the desktop "
	   "user interface."));

      /*** "Read attempts" page */

      vbox = create_page(notebook, _utf("Read attempts"));

      /** Reading preferences */
      
      frame = gtk_frame_new(_utf("Sector read errors"));
      gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

      vbox2 = gtk_vbox_new(FALSE, 20);
      gtk_container_set_border_width(GTK_CONTAINER(vbox2), 10);
      gtk_container_add(GTK_CONTAINER(frame), vbox2);

      /* Raw verify */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Raw reading"),
					  _("Read and analyze raw sectors"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);

	 button = gtk_check_button_new();
	 gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
       	 gtk_box_pack_start(GTK_BOX(hbox), i ? lwoh->normalLabel : lwoh->linkBox, FALSE, FALSE, 0);
         if(!i) gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);

 	 if(!i) pc->rawButtonA = button;
	 else   pc->rawButtonB = button;

         activate_toggle_button(GTK_TOGGLE_BUTTON(button), Closure->readRaw);
         g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(toggle_cb), GINT_TO_POINTER(TOGGLE_RAW));

	 if(!i) gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
	 else   GuiAddHelpWidget(lwoh, hbox);
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Raw reading</b> (affects CD media only)\n\n"
	   "Activating this option has several effects:\n\n"
	   "C2 quality scanning will be performed when supported "
	   "by the drive.\n\n"
	   "Media sectors are read in raw mode. The L-EC P/Q "
	   "vectors, EDC checksum and MSF address contained "
	   "in the raw data are checked to make sure that the "
	   "sector was correctly read.\n\n"
	   "Additional data recovery heuristics and raw sector "
	   "caching becomes available if either\n"
	   "- adaptive reading is used, or\n"
	   "- linear reading is configured to skip 0 sectors after a read error.\n"
	   "Raw sector caching also needs checking of the respective option."
	   ));

      /* Minimum reading attempts */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Minimum number of reading attempts"),
					  "ignore");
      GuiRegisterPreferencesHelpWindow(lwoh);
      GuiLockLabelSize(lwoh->normalLabel, _utf("Min. %d reading attempts per sector"), 99);
      GuiLockLabelSize(lwoh->linkLabel, _utf("Min. %d reading attempts per sector"), 99);

      pc->minAttemptsScaleLwoh = lwoh;
      pc->minAttemptsScaleInfoA = g_malloc0(sizeof(non_linear_info));
      pc->minAttemptsScaleInfoB = g_malloc0(sizeof(non_linear_info));
      pc->minAttemptsScaleInfoA->format = g_strdup(_("Min. %d reading attempts per sector"));
      pc->minAttemptsScaleInfoB->format = g_strdup(_("Min. %d reading attempts per sector"));

      pc->minAttemptsScaleInfoA->lwoh = lwoh;

      for(i=0; i<2; i++)
	{  GtkWidget *scale,*scale_box;

	 scale = non_linear_scale(&scale_box, 
				  i ? pc->minAttemptsScaleInfoB : pc->minAttemptsScaleInfoA,
				  i ? lwoh->normalLabel : lwoh->linkLabel, 
				  pc, SLIDER_MIN_READ_ATTEMPTS, min_attempts_values, MIN_ATTEMPTS_VALUE_LENGTH,
				  Closure->minReadAttempts, !i);

	 if(!i) pc->minAttemptsScaleA = scale;
	 else   pc->minAttemptsScaleB = scale;

	 if(!i) gtk_box_pack_start(GTK_BOX(vbox2), scale_box, FALSE, FALSE, 0);
	 else   GuiAddHelpWidget(lwoh, scale_box);
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Minimum number of reading attempts</b>\n\n"
	   "If an unreadable sector is encountered, "
	   "dvdisaster tries to re-read it the given number of times.\n\n" 
	   "Increasing the number of reading attempts may improve data recovery "
	   "on marginal media, but will also increase processing time and "
	   "mechanical wear on the drive."));

      /* Maximum reading attempts */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Maximum number of reading attempts"),
					  "ignore");
      GuiRegisterPreferencesHelpWindow(lwoh);
      GuiLockLabelSize(lwoh->normalLabel, _utf("Max. %d reading attempts per sector"), 100);
      GuiLockLabelSize(lwoh->linkLabel, _utf("Max. %d reading attempts per sector"), 100);

      pc->maxAttemptsScaleLwoh = lwoh;
      pc->maxAttemptsScaleInfoA = g_malloc0(sizeof(non_linear_info));
      pc->maxAttemptsScaleInfoB = g_malloc0(sizeof(non_linear_info));
      pc->maxAttemptsScaleInfoA->format = g_strdup(_("Max. %d reading attempts per sector"));
      pc->maxAttemptsScaleInfoB->format = g_strdup(_("Max. %d reading attempts per sector"));

      pc->maxAttemptsScaleInfoA->lwoh = lwoh;

      for(i=0; i<2; i++)
	{  GtkWidget *scale,*scale_box;

	 scale = non_linear_scale(&scale_box, 
				  i ? pc->maxAttemptsScaleInfoB : pc->maxAttemptsScaleInfoA,
				  i ? lwoh->normalLabel : lwoh->linkLabel, 
				  pc, SLIDER_MAX_READ_ATTEMPTS, max_attempts_values, MAX_ATTEMPTS_VALUE_LENGTH,
				  Closure->maxReadAttempts, !i);

	 if(!i) pc->maxAttemptsScaleA = scale;
	 else   pc->maxAttemptsScaleB = scale;

	 if(!i) gtk_box_pack_start(GTK_BOX(vbox2), scale_box, FALSE, FALSE, 0);
	 else   GuiAddHelpWidget(lwoh, scale_box);
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Maximum number of reading attempts</b>\n\n"
	   "When the minimum number of reading attempts is reached "
	   "without success, dvdisaster might choose to perform additional "
	   "reading attempts up to this number.\n\n"

	   "The decision to do more attempts depends on the quality of "
	   "data gathered so far, which in turn is influenced by the "
	   "capabilities of your optical drive and the operating system. "
	   "So depending on your configuration, you may or "
	   "may not see dvdisaster using the maximum value."
	   ));

      /* Jump selector */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Treatment of unreadable areas"),
					  "ignore");
      GuiRegisterPreferencesHelpWindow(lwoh);
      if(  GuiGetLabelWidth(GTK_LABEL(lwoh->normalLabel), _utf("Skip %d sectors after read error"), 20480)
	 > GuiGetLabelWidth(GTK_LABEL(lwoh->normalLabel), _utf("Stop reading when unreadable intervals &lt; %d"), 20480))
      {    GuiLockLabelSize(lwoh->normalLabel, _utf("Skip %d sectors after read error"), 20480);
	   GuiLockLabelSize(lwoh->linkLabel, _utf("Skip %d sectors after read error"), 20480);
      }
      else 
      {    GuiLockLabelSize(lwoh->normalLabel, _utf("Stop reading when unreadable intervals &lt; %d"), 20480);
	   GuiLockLabelSize(lwoh->linkLabel, _utf("Stop reading when unreadable intervals &lt; %d"), 20480);
      }

      pc->jumpScaleLwoh = lwoh;
      pc->formatLinear = g_strdup(_("Skip %d sectors after read error"));
      pc->formatAdaptive  = g_strdup(_("Stop reading when unreadable intervals &lt; %d"));

      pc->jumpScaleInfoA = g_malloc0(sizeof(non_linear_info));
      pc->jumpScaleInfoB = g_malloc0(sizeof(non_linear_info));
      pc->jumpScaleInfoA->format = Closure->adaptiveRead ? pc->formatAdaptive : pc->formatLinear;
      pc->jumpScaleInfoB->format = Closure->adaptiveRead ? pc->formatAdaptive : pc->formatLinear;

      pc->jumpScaleInfoA->lwoh = lwoh;

      for(i=0; i<2; i++)
      {  GtkWidget *scale, *scale_box;
 
	 scale = non_linear_scale(&scale_box, 
				  i ? pc->jumpScaleInfoB : pc->jumpScaleInfoA, 
				  i ? lwoh->normalLabel : lwoh->linkLabel, 
				  pc, SLIDER_JUMP, jump_values, JUMP_VALUE_LENGTH,
				  Closure->sectorSkip, !i);

	 if(!i) pc->jumpScaleA = scale;
	 else   pc->jumpScaleB = scale;

	 if(!i) gtk_box_pack_start(GTK_BOX(vbox2), scale_box, FALSE, FALSE, 0);
	 else   GuiAddHelpWidget(lwoh, scale_box);
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Treatment of unreadable areas</b>\n\n"
	   "Defective media usually contain numerous read errors in a contigous region. "
	   "Skipping sectors after a read error reduces the processing time and the "
	   "mechanical wear on the drive, but leaves larger gaps in the image file.\n\n"
	   "Effects on the <b>linear reading strategy</b>:"));

      GuiAddHelpListItem(lwoh, 
	 _("Skipping a large number of sectors (e.g. 1024) gives a quick overview of "
	   "damaged areas, but will usually not collect enough data for repairing the image."));
      
      GuiAddHelpListItem(lwoh, 
	 _("Smaller values like 16, 32 or 64 are a good trade-off: The processing time will be"
	   "considerably shortened, but still enough data for repairing the image is collected.\n"));

      GuiAddHelpParagraph(lwoh, 
	 _("The <b>adaptive reading strategy</b> uses this setting only if no error correction data "
	   "is available. In that case the reading process will stop when no unread areas "
	   "larger than the selected size remain. Values smaller than 128 <i>are not recommended</i> "
	   "as they cause the drive to carry out lots of laser head repositioning during the "
	   "final phase of the reading process. If adaptive reading with a setting of 128 is not "
	   "sufficient, try reading the remaining sectors with an additional linear reading pass.\n\n"

	   "On DVD and BD media read errors do usually extend over at least 16 sectors for technical "
	   "reasons. Therefore selecting a value less than 16 is not recommended for DVD and BD."
	   ));

      /** Media re-reads */
      
      frame = gtk_frame_new(_utf("Media read attempts"));
      gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

      lwoh = GuiCreateLabelWithOnlineHelp(_("Media read attempts"), 
					  _("Read the whole medium "));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	 GtkWidget *spin;
	 GtkWidget *label;

      	 gtk_box_pack_start(GTK_BOX(hbox), i ? lwoh->normalLabel : lwoh->linkBox, FALSE, FALSE, 0);
         if(!i) gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);
	 spin = gtk_spin_button_new_with_range(1, 200, 1);
	 gtk_entry_set_width_chars(GTK_ENTRY(spin), 3);
	 gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), 
				   Closure->readingPasses < 2 ? 1 : Closure->readingPasses);
	 g_signal_connect(spin, "value-changed", G_CALLBACK(spin_cb), (gpointer)SPIN_READ_MEDIUM);
	 gtk_box_pack_start(GTK_BOX(hbox), spin, FALSE, FALSE, 0);
	 label = gtk_label_new(_utf(" times"));
	 gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

	 if(!i)
	 {  pc->readMediumA = spin;
	    gtk_container_set_border_width(GTK_CONTAINER(hbox), 10);
	    gtk_container_add(GTK_CONTAINER(frame), hbox);
	 }
	 else
	 {  pc->readMediumB = spin;
	    GuiAddHelpWidget(lwoh, hbox);
	 }
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Media read attempts</b> for the linear reading strategy\n\n"
	   "If unreadable sectors remain after reading the medium from start to end, "
	   "the medium is read again up to he given number of times.\n\n"
	   "Only the missing sectors will be tried in the additional reading passes."));

      /** Defective sector caching */
      
      frame = gtk_frame_new(_utf("Raw sector caching"));
      gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

      vbox2 = gtk_vbox_new(FALSE, 20);
      gtk_container_set_border_width(GTK_CONTAINER(vbox2), 10);
      gtk_container_add(GTK_CONTAINER(frame), vbox2);

      /* Toggle button */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Raw sector caching"), 
					  _("Keep uncorrectable raw sectors in the following directory:"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *table = gtk_table_new(3,2,FALSE);
	 GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	 GtkWidget *tinybox = gtk_hbox_new(FALSE, 0);
	 GtkWidget *label = gtk_label_new(Closure->dDumpDir);
	 GtkWidget *select = gtk_button_new_with_label(_utf("Select"));

	 button = gtk_check_button_new();
	 gtk_box_pack_start(GTK_BOX(tinybox), button, FALSE, FALSE, 0);
	 gtk_box_pack_start(GTK_BOX(tinybox), i ? lwoh->normalLabel : lwoh->linkBox, FALSE, FALSE, 0);
	 if (!i) gtk_box_pack_start(GTK_BOX(tinybox), lwoh->tooltip, FALSE, FALSE, 0);

	 gtk_table_attach(GTK_TABLE(table), tinybox, 0, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

	 gtk_misc_set_alignment(GTK_MISC(lwoh->linkLabel), 0.0, 0.0);
	 gtk_misc_set_alignment(GTK_MISC(lwoh->normalLabel), 0.0, 0.0);

	 hbox = gtk_hbox_new(FALSE, 0);
	 gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	 gtk_table_attach(GTK_TABLE(table), hbox, 
			  1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

	 gtk_table_attach(GTK_TABLE(table), select, 
			  2, 3, 0, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
	 g_signal_connect(G_OBJECT(select), "clicked", G_CALLBACK(cache_defective_dir_cb), pc);


 	 if(!i) 
	 {    pc->cacheDefectiveA = button;
	      pc->cacheDefectiveDirA = label;
	 }
	 else
	 {    pc->cacheDefectiveB = button;
	      pc->cacheDefectiveDirB = label;
	 }

	 if(Closure->readRaw && Closure->defectiveDump)
	      activate_toggle_button(GTK_TOGGLE_BUTTON(button), TRUE);
	 else activate_toggle_button(GTK_TOGGLE_BUTTON(button), FALSE);
         g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(toggle_cb), GINT_TO_POINTER(TOGGLE_CACHE_DEFECTIVE));

	 if(!i) gtk_box_pack_start(GTK_BOX(vbox2), table, FALSE, FALSE, 0);
	 else   GuiAddHelpWidget(lwoh, table);
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Raw sector caching</b>\n\n"
	   "Some drives are capable of delivering partial data from defective "
	   "sectors. While one partial sector is useless by itself, "
	   "20 or more of them might combine into a complete sector.\n\n"
	   "If you activate this button, dvdisaster will collect partial "
	   "sectors in the selected directory. "
	   "When enough parts have been gathered in subsequent reading passes, "
	   "the respective sector is automatically recombined from the "
	   "stored parts.\n\n"
	   "Please note that not all drives and operating systems "
	   "support reading partial data. "
	   "It is not an error of the cache directory remains empty.\n"
	   "dvdisaster will not remove any files from the given directory; "
	   "you need to clean it up manually after a successful medium recovery."
	   ));

      /* Sector cache file prefix */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Raw sector file prefix"), 
					  _("Raw sector file prefix: "));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	 GtkWidget *entry = gtk_entry_new();

       	 gtk_box_pack_start(GTK_BOX(hbox), i ? lwoh->normalLabel : lwoh->linkBox, FALSE, FALSE, 0);
         if(!i) gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);

	 g_signal_connect(entry, "activate", G_CALLBACK(defective_prefix_cb), pc);
	 gtk_entry_set_width_chars(GTK_ENTRY(entry), 20);
	 gtk_entry_set_text(GTK_ENTRY(entry), Closure->dDumpPrefix);
	 gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 0);

 	 if(!i) pc->cacheDefectivePrefixA = entry;
	 else   pc->cacheDefectivePrefixB = entry;

	 if(!i) gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
	 else   GuiAddHelpWidget(lwoh, hbox);
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Raw sector file prefix</b>\n\n"
	   "Use a different prefix for each disc you are trying "
	   "to recover, e.g. \"disc1-\" and so on."));

      /*** "Error correction" page */

      /* Method chooser menu */

      vbox = create_page(notebook, _utf("Error correction"));

      lwoh = GuiCreateLabelWithOnlineHelp(_("Error correction method"), 
					  _("Storage method:"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox = gtk_hbox_new(FALSE, 4);
	 GtkWidget *chooser;
	 int j;

	 gtk_box_pack_start(GTK_BOX(hbox), i ? lwoh->normalLabel : lwoh->linkBox, FALSE, FALSE, 0);
         if(!i) gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);

	 chooser = gtk_combo_box_text_new();

       	 g_signal_connect(G_OBJECT(chooser), "changed", G_CALLBACK(method_select_cb), pc);

	 for(j=0; j<Closure->methodList->len; j++)
	 {  Method *method = g_ptr_array_index(Closure->methodList, j);
	    char *utf;

	    utf  = g_locale_to_utf8(method->menuEntry, -1, NULL, NULL, NULL);
	    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(chooser), utf);
	    g_free(utf);

	    if(!strncmp(Closure->methodName, method->name, 4))
	      method_idx = j;
	 }

	 gtk_combo_box_set_active(GTK_COMBO_BOX(chooser), method_idx);
	 gtk_box_pack_start(GTK_BOX(hbox), chooser, FALSE, FALSE, 0);
	 
	 if(!i)
	 {  pc->methodChooserA = chooser;
	    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	 }
	 else
	 {  pc->methodChooserB = chooser;
	    GuiAddHelpWidget(lwoh, hbox);
	 }
      }

      GuiAddHelpParagraph(lwoh, _("<b>Error correction method</b>\n\n"
	 "dvdisaster creates error correction data which is used to recover "
	 "unreadable sectors if the disc becomes damaged later on. There are "
	 "different codecs and ways available for storing the error correction "
	 "information:\n"));

      GuiAddHelpListItem(lwoh, _("The RS01 codec\n"
	 "RS01 is the recommended codec for storing error correction data in separate files.\n"));

      GuiAddHelpListItem(lwoh, _("The RS02 codec\n"
	 "RS02 is the currently recommended codec for "
	 "augmenting images with error correction data.\n"));

      GuiAddHelpListItem(lwoh, _("The RS03 codec (Warning: experimental)\n"
	 "RS03 can either store error correction data in a separate file "
	 "or augment the image with it. It provides multithreading "
	 "to scale with multicore processors and contains some subtle improvements "
	 "over RS01 and RS02. However it should not be used for productive work "
	 "unless a stable version is released with dvdisaster V0.80."));

      /* sub pages for individual method configuration */
      
      pc->methodNotebook = gtk_notebook_new();
      gtk_notebook_set_show_tabs(GTK_NOTEBOOK(pc->methodNotebook), FALSE);
      gtk_notebook_set_show_border(GTK_NOTEBOOK(pc->methodNotebook), FALSE);
      gtk_box_pack_start(GTK_BOX(vbox), pc->methodNotebook, TRUE, TRUE, 0);

      for(i=0; i<Closure->methodList->len; i++)
      {  Method *method = g_ptr_array_index(Closure->methodList, i);
	 GtkWidget *vbox2 = gtk_vbox_new(FALSE, 0);
	 GtkWidget *ignore = gtk_label_new("method_tab");

	 if(method->createPrefsPage)
	    method->createPrefsPage(method, vbox2);
	 else
	 {  GtkWidget *lab;

	    lab = gtk_label_new("This method has no configuration options.");
	    gtk_box_pack_start(GTK_BOX(vbox2), lab, TRUE, TRUE, 0);
	 }

	 gtk_notebook_append_page(GTK_NOTEBOOK(pc->methodNotebook), vbox2, ignore);
	 if(i==method_idx)
      gtk_notebook_set_current_page(GTK_NOTEBOOK(pc->methodNotebook), method_idx);
      }

      g_idle_add(notebook_idle_func, pc);

      /*** "Files" page */

      vbox = create_page(notebook, _utf("Files"));

      /* file extension */

      frame = gtk_frame_new(_utf("Local files (on hard disk)"));
      gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

      vbox2 = gtk_vbox_new(FALSE, 15);
      gtk_container_set_border_width(GTK_CONTAINER(vbox2), 10);
      gtk_container_add(GTK_CONTAINER(frame), vbox2);

      lwoh = GuiCreateLabelWithOnlineHelp(_("Automatic file suffixes"),
					  _("Automatically add .iso and .ecc file suffixes"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	 GtkWidget *button = gtk_check_button_new();

	 gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
	 gtk_box_pack_start(GTK_BOX(hbox), i ? lwoh->normalLabel : lwoh->linkBox, FALSE, FALSE, 0);
         if(!i) gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);

	 activate_toggle_button(GTK_TOGGLE_BUTTON(button), Closure->autoSuffix);
	 g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(toggle_cb), GINT_TO_POINTER(TOGGLE_SUFFIX));

	 if(!i)
	 {  pc->suffixA = button;
	    gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
	 }
	 else
	 {  pc->suffixB = button;
	    GuiAddHelpWidget(lwoh, hbox);
	 }
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Automatically add file suffixes</b>\n\n"
	   "When this switch is set, files will be automatically appended with \".iso\" "
	   "or \".ecc\" suffixes if no other file name extension is already present."));

      /*** Automatic file creation and deletion */

      frame = gtk_frame_new(_utf("Automatic file creation and deletion"));
      gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

      vbox2 = gtk_vbox_new(FALSE, 15);
      gtk_container_set_border_width(GTK_CONTAINER(vbox2), 10);
      gtk_container_add(GTK_CONTAINER(frame), vbox2);

      /* automatic creation */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Automatic .ecc file creation"),
					  _("Create error correction file after reading image"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	 GtkWidget *button = gtk_check_button_new();

	 gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
      	 gtk_box_pack_start(GTK_BOX(hbox), i ? lwoh->normalLabel : lwoh->linkBox, FALSE, FALSE, 0);
         if(!i) gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);
	 activate_toggle_button(GTK_TOGGLE_BUTTON(button), Closure->readAndCreate);
	 g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(toggle_cb), GINT_TO_POINTER(TOGGLE_READ_CREATE));

	 if(!i)
	 {  pc->readAndCreateButtonA = button;
	    gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
	 }
	 else
	 {  pc->readAndCreateButtonB = button;
	    GuiAddHelpWidget(lwoh, hbox);
	 }
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Automatic error correction file creation</b>\n\n"
	   "Automatically creates an error correction file after reading an image. "
	   "Together with the \"Remove image\" option this will speed up error correction "
	   "file generation for a series of different media."));

      /* automatic deletion */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Automatic image file removal"),
					  _("Remove image after error correction file creation"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	 GtkWidget *button = gtk_check_button_new();

	 gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
      	 gtk_box_pack_start(GTK_BOX(hbox), i ? lwoh->normalLabel : lwoh->linkBox, FALSE, FALSE, 0);
         if(!i) gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);
	 activate_toggle_button(GTK_TOGGLE_BUTTON(button), Closure->unlinkImage);
	 g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(toggle_cb), GINT_TO_POINTER(TOGGLE_UNLINK));

	 if(!i)
	 {  pc->unlinkImageButtonA = button;
	    gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
	 }
	 else
	 {  pc->unlinkImageButtonB = button;
	    GuiAddHelpWidget(lwoh, hbox);
	 }
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Automatic image file removal</b>\n\n"
	   "If this switch is set the image file will be deleted following the successful "
	   "generation of the respective error correction file."));

      /*** Deletion confirmation */

      frame = gtk_frame_new(_utf("Confirm file overwriting"));
      gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

      vbox2 = gtk_vbox_new(FALSE, 15);
      gtk_container_set_border_width(GTK_CONTAINER(vbox2), 10);
      gtk_container_add(GTK_CONTAINER(frame), vbox2);

      /* automatic creation */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Confirm file overwriting"),
					  _("Ask before overwriting image and ecc files"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	 GtkWidget *button = gtk_check_button_new();

	 gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
      	 gtk_box_pack_start(GTK_BOX(hbox), i ? lwoh->normalLabel : lwoh->linkBox, FALSE, FALSE, 0);
         if(!i) gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);
	 activate_toggle_button(GTK_TOGGLE_BUTTON(button), Closure->confirmDeletion);
	 g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(toggle_cb), GINT_TO_POINTER(TOGGLE_CONFIRM_DELETION));

	 if(!i)
	 {  pc->confirmDeletionA = button;
	    gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
	 }
	 else
	 {  pc->confirmDeletionB = button;
	    GuiAddHelpWidget(lwoh, hbox);
	 }
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Ask before overwriting image and ecc files</b>\n\n"
	   "dvdisaster will ask you for confirmation "
	   "when it is going to overwrite an existing image "
	   "or error correction file if this option is checked."));

      /*** GUI page */

      vbox = create_page(notebook, _utf("Appearance"));

      /** Color scheme
          Using a table gives better control over spacing between the frames. */

      table = gtk_table_new(2, 4, FALSE);
      gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 0);

      vbox3 = gtk_vbox_new(FALSE, 0);
      gtk_table_attach(GTK_TABLE(table), vbox3, 
		       0, 1, 0, 3, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 5, 5);

      frame = gtk_frame_new(_utf("Sector coloring"));
      gtk_box_pack_start(GTK_BOX(vbox3), frame, FALSE, FALSE, 0);

      gtk_box_pack_start(GTK_BOX(vbox3), gtk_label_new(NULL), TRUE, TRUE, 0);

      vbox2 = gtk_vbox_new(FALSE, 20);
      gtk_container_set_border_width(GTK_CONTAINER(vbox2), 10);
      gtk_container_add(GTK_CONTAINER(frame), vbox2);
 
      /* Green color */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Good sectors"), _("Good sector"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox  = gtk_hbox_new(FALSE, 4);

         button = gtk_color_button_new_with_color(Closure->greenSector);
         g_signal_connect(G_OBJECT(button), "color-set", G_CALLBACK(color_set_cb), (gpointer)COLOR_GREEN);
         gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	 if(!i)
	 {  gtk_box_pack_start(GTK_BOX(hbox), lwoh->linkBox, FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);
	    gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
            pc->greenA = button;
	 }
	 else
	 {  
	    gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);
	    GuiAddHelpWidget(lwoh, hbox);
            pc->greenB = button;
	 }
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Good sectors</b>\n\n"
	   "This color indicates good sectors."));

      /* Yellow color */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Checksum errors"), _("Checksum error"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox  = gtk_hbox_new(FALSE, 4);

         button = gtk_color_button_new_with_color(Closure->yellowSector);
         g_signal_connect(G_OBJECT(button), "color-set", G_CALLBACK(color_set_cb), (gpointer)COLOR_YELLOW);
         gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	 if(!i)
	 {  gtk_box_pack_start(GTK_BOX(hbox), lwoh->linkBox, FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);
	    gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
            pc->yellowA = button;
	 }
	 else
	 {  
	    gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);
	    GuiAddHelpWidget(lwoh, hbox);
            pc->yellowB = button;
	 }
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Checksum errors</b>\n\n"
	   "This color is used for displaying sectors with wrong check sums."));

      /* Red color */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Unreadable sectors"), _("Unreadable"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox  = gtk_hbox_new(FALSE, 4);

         button = gtk_color_button_new_with_color(Closure->redSector);
         g_signal_connect(G_OBJECT(button), "color-set", G_CALLBACK(color_set_cb), (gpointer)COLOR_RED);
         gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	 if(!i)
	 {  gtk_box_pack_start(GTK_BOX(hbox), lwoh->linkBox, FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);
	    gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
            pc->redA = button;
	 }
	 else
	 {  
	    gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);
	    GuiAddHelpWidget(lwoh, hbox);
            pc->redB = button;
	 }
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Unreadable sectors</b>\n\n"
	   "This color is used for marking unreadable sectors."));

      /* Dark color */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Present sectors"), _("Present sector"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox  = gtk_hbox_new(FALSE, 4);

         button = gtk_color_button_new_with_color(Closure->darkSector);
         g_signal_connect(G_OBJECT(button), "color-set", G_CALLBACK(color_set_cb), (gpointer)COLOR_DARK);
         gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	 if(!i)
	 {  gtk_box_pack_start(GTK_BOX(hbox), lwoh->linkBox, FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);
	    gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
            pc->darkA = button;
	 }
	 else
	 {  
	    gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);
	    GuiAddHelpWidget(lwoh, hbox);
            pc->darkB = button;
	 }
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Present sectors</b>\n\n"
	   "Sectors which are already present are marked with this color."));

      /* Blue color */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Ignored sectors"), _("Ignored sector"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox  = gtk_hbox_new(FALSE, 4);

         button = gtk_color_button_new_with_color(Closure->blueSector);
         g_signal_connect(G_OBJECT(button), "color-set", G_CALLBACK(color_set_cb), (gpointer)COLOR_BLUE);
         gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	 if(!i)
	 {  gtk_box_pack_start(GTK_BOX(hbox), lwoh->linkBox, FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);
	    gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
            pc->blueA = button;
	 }
	 else
	 {  
	    gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);
	    GuiAddHelpWidget(lwoh, hbox);
            pc->blueB = button;
	 }
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Ignored sectors</b>\n\n"
	   "Sectors marked with this color will not be processed "
	   "in the current run."));

      /* White color */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Highlit sectors"), _("Highlit sector"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox  = gtk_hbox_new(FALSE, 4);

         button = gtk_color_button_new_with_color(Closure->whiteSector);
         g_signal_connect(G_OBJECT(button), "color-set", G_CALLBACK(color_set_cb), (gpointer)COLOR_WHITE);
         gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	 if(!i)
	 {  gtk_box_pack_start(GTK_BOX(hbox), lwoh->linkBox, FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);
	    gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
            pc->whiteA = button;
	 }
	 else
	 {  
	    gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);
	    GuiAddHelpWidget(lwoh, hbox);
            pc->whiteB = button;
	 }
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Highlit sectors</b>\n\n"
	   "This color is used for temporarily highlighting "
	   "sectors during adaptive reading."));

      /** Text colors */

      frame = gtk_frame_new(_utf("Text colors"));
      gtk_table_attach(GTK_TABLE(table), frame, 
		       1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 5, 5);

      vbox2 = gtk_vbox_new(FALSE, 20);
      gtk_container_set_border_width(GTK_CONTAINER(vbox2), 10);
      gtk_container_add(GTK_CONTAINER(frame), vbox2);

      /* Positive text */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Positive text"), _("Positive text"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox  = gtk_hbox_new(FALSE, 4);

         button = gtk_color_button_new_with_color(Closure->greenText);
         g_signal_connect(G_OBJECT(button), "color-set", G_CALLBACK(color_set_cb), (gpointer)COLOR_GREEN_TEXT);
         gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	 if(!i)
	 {  gtk_box_pack_start(GTK_BOX(hbox), lwoh->linkBox, FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);
	    gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
            pc->greenTextA = button;
	 }
	 else
	 {  
	    gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);
	    GuiAddHelpWidget(lwoh, hbox);
            pc->greenTextB = button;
	 }
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Positive text</b>\n\n"
	   "Good news are printed in this color."));

      /* Positive text */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Negative text"), _("Negative text"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox  = gtk_hbox_new(FALSE, 4);

         button = gtk_color_button_new_with_color(Closure->redText);
         g_signal_connect(G_OBJECT(button), "color-set", G_CALLBACK(color_set_cb), (gpointer)COLOR_RED_TEXT);
         gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	 if(!i)
	 {  gtk_box_pack_start(GTK_BOX(hbox), lwoh->linkBox, FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);
	    gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
            pc->redTextA = button;
	 }
	 else
	 {  
	    gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);
	    GuiAddHelpWidget(lwoh, hbox);
            pc->redTextB = button;
	 }
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Negative text</b>\n\n"
	   "Bad news are printed in this color."));

      /** Curve colors */

      frame = gtk_frame_new(_utf("Curve colors"));
      gtk_table_attach(GTK_TABLE(table), frame, 
		       1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 5, 5);
      vbox2 = gtk_vbox_new(FALSE, 20);
      gtk_container_set_border_width(GTK_CONTAINER(vbox2), 10);
      gtk_container_add(GTK_CONTAINER(frame), vbox2);

      /* Reading speed curve */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Curve color"), _("Curve color"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox  = gtk_hbox_new(FALSE, 4);

         button = gtk_color_button_new_with_color(Closure->curveColor);
         g_signal_connect(G_OBJECT(button), "color-set", G_CALLBACK(color_set_cb), (gpointer)COLOR_CURVE);
         gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	 if(!i)
	 {  gtk_box_pack_start(GTK_BOX(hbox), lwoh->linkBox, FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);
	    gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
            pc->curveColorA = button;
	 }
	 else
	 {  
	    gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);
	    GuiAddHelpWidget(lwoh, hbox);
            pc->curveColorB = button;
	 }
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Curve color and labels</b>\n\n"
	   "The reading speed curve, its left side and top labels "
	   "are printed in this color."));

      /* Logarithmic scale (C2 errors) */

      lwoh = GuiCreateLabelWithOnlineHelp(_("C2 errors"), _("C2 errors"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox  = gtk_hbox_new(FALSE, 4);

         button = gtk_color_button_new_with_color(Closure->logColor);
         g_signal_connect(G_OBJECT(button), "color-set", G_CALLBACK(color_set_cb), (gpointer)COLOR_LOG);
         gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	 if(!i)
	 {  gtk_box_pack_start(GTK_BOX(hbox), lwoh->linkBox, FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);
	    gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
            pc->logColorA = button;
	 }
	 else
	 {  
	    gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);
	    GuiAddHelpWidget(lwoh, hbox);
            pc->logColorB = button;
	 }
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>C2 error color</b>\n\n"
	   "The logarithmic bar graph showing the C2 errors "
	   "is rendered in this color during the \"read\" "
	   "and \"scan\" operations."));

      /* Error correction load */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Error correction load"), _("Error correction load"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox  = gtk_hbox_new(FALSE, 4);

         button = gtk_color_button_new_with_color(Closure->barColor);
         g_signal_connect(G_OBJECT(button), "color-set", G_CALLBACK(color_set_cb), (gpointer)COLOR_BAR);
         gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	 if(!i)
	 {  gtk_box_pack_start(GTK_BOX(hbox), lwoh->linkBox, FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);
	    gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
            pc->barColorA = button;
	 }
	 else
	 {  
	    gtk_box_pack_start(GTK_BOX(hbox), lwoh->normalLabel, FALSE, FALSE, 0);
	    GuiAddHelpWidget(lwoh, hbox);
            pc->barColorB = button;
	 }
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Error correction load</b>\n\n"
	   "The bar graph showing the error correction load "
	   "is rendered in this color during the \"Fix\" operation."));

      /* Padding space */
#if 0
      lab = gtk_label_new("");
      gtk_table_attach(GTK_TABLE(table), lab, 
		       1, 2, 2, 3, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 5, 5);
#endif
      /* Default color scheme */

      button = gtk_button_new_with_label(_utf("Default color scheme"));
      gtk_table_attach(GTK_TABLE(table), button, 
		       1, 2, 2, 3, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 5, 5);

      g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(default_color_cb), NULL);

      /** Reverse OK and Cancel buttons */

      frame = gtk_frame_new(_utf("Dialog boxes"));
      gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

      lwoh = GuiCreateLabelWithOnlineHelp(_("Reverse OK / Cancel buttons"),
					  _("Reverse OK / Cancel buttons"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	 GtkWidget *button = gtk_check_button_new();

	 gtk_container_set_border_width(GTK_CONTAINER(hbox), 10);
	 gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
      	 gtk_box_pack_start(GTK_BOX(hbox), i ? lwoh->normalLabel : lwoh->linkBox, FALSE, FALSE, 0);
         if (!i) gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);
	 activate_toggle_button(GTK_TOGGLE_BUTTON(button), Closure->reverseCancelOK);
	 g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(toggle_cb), GINT_TO_POINTER(TOGGLE_CANCEL_OK));

	 if(!i)
	 {  pc->cancelOKA = button;
	    gtk_container_add(GTK_CONTAINER(frame), hbox);
	 }
	 else
	 {  pc->cancelOKB = button;
	    GuiAddHelpWidget(lwoh, hbox);
	 }
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Reverse OK / Cancel buttons</b>\n\n"
	   "This switch reverses the order of dialog buttons "
	   "(e.g. OK, Cancel).\n\n"
	   "Changes will become active after restarting dvdisaster."));

      /*** "Misc" page */

      vbox = create_page(notebook, _utf("Misc"));

      /** Logging **/

      frame = gtk_frame_new(_utf("Logging"));
      gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

      vbox2 = gtk_vbox_new(FALSE, 15);
      gtk_container_set_border_width(GTK_CONTAINER(vbox2), 10);
      gtk_container_add(GTK_CONTAINER(frame), vbox2);

      lwoh = GuiCreateLabelWithOnlineHelp(_("Verbose logging"),
					  _("Verbose logging"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	 GtkWidget *button = gtk_check_button_new();

	 gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
	 gtk_box_pack_start(GTK_BOX(hbox), i ? lwoh->normalLabel : lwoh->linkBox, FALSE, FALSE, 0);
         if (!i) gtk_box_pack_start(GTK_BOX(hbox), lwoh->tooltip, FALSE, FALSE, 0);

	 activate_toggle_button(GTK_TOGGLE_BUTTON(button), Closure->verbose);
	 g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(toggle_cb), GINT_TO_POINTER(TOGGLE_VERBOSE));

	 if(!i)
	 {  pc->verboseA = button;
	    gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
	 }
	 else
	 {  pc->verboseB = button;
	    GuiAddHelpWidget(lwoh, hbox);
	 }
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Verbose logging</b>\n\n"
	   "More information will be supplied in the Log window "
	   "and/or log file. Useful for debugging, but may lead "
	   "to slower performance."));

      /** Log file */

      lwoh = GuiCreateLabelWithOnlineHelp(_("Logfile:"), 
					  _("Copy log to file:"));
      GuiRegisterPreferencesHelpWindow(lwoh);

      for(i=0; i<2; i++)
      {  GtkWidget *table = gtk_table_new(4,2,FALSE);
	 GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	 GtkWidget *tinybox = gtk_hbox_new(FALSE, 0);
	 GtkWidget *label = gtk_label_new(Closure->logFile);
	 GtkWidget *select = gtk_button_new_with_label(_utf("Select"));
	 GtkWidget *delete = gtk_button_new_with_label(_utf("Delete"));

	 button = gtk_check_button_new();
	 gtk_box_pack_start(GTK_BOX(tinybox), button, FALSE, FALSE, 0);
	 gtk_box_pack_start(GTK_BOX(tinybox), i ? lwoh->normalLabel : lwoh->linkBox, FALSE, FALSE, 0);
	 if (!i) gtk_box_pack_start(GTK_BOX(tinybox), lwoh->tooltip, FALSE, FALSE, 0);

	 gtk_table_attach(GTK_TABLE(table), tinybox, 0, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

	 gtk_misc_set_alignment(GTK_MISC(lwoh->linkLabel), 0.0, 0.0);
	 gtk_misc_set_alignment(GTK_MISC(lwoh->normalLabel), 0.0, 0.0);
	 gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_MIDDLE);

	 hbox = gtk_hbox_new(FALSE, 0);
	 gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
	 gtk_table_attach(GTK_TABLE(table), hbox, 
			  0, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

	 gtk_table_attach(GTK_TABLE(table), select, 
			  2, 3, 0, 2, GTK_SHRINK, GTK_SHRINK, 10, 0);
	 g_signal_connect(G_OBJECT(select), "clicked", G_CALLBACK(logfile_select_cb), 0);

	 gtk_table_attach(GTK_TABLE(table), delete, 
			  3, 4, 0, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
	 g_signal_connect(G_OBJECT(delete), "clicked", G_CALLBACK(logfile_delete_cb), 0);


 	 if(!i) 
	 {    pc->logFileA = button;
	      pc->logFilePathA = label;
	 }
	 else
	 {    pc->logFileB = button;
	      pc->logFilePathB = label;
	 }

	 if(Closure->verbose && Closure->logFileEnabled)
	      activate_toggle_button(GTK_TOGGLE_BUTTON(button), TRUE);
	 else activate_toggle_button(GTK_TOGGLE_BUTTON(button), FALSE);
         g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(toggle_cb), GINT_TO_POINTER(TOGGLE_LOGFILE));

	 if(!i) gtk_box_pack_start(GTK_BOX(vbox2), table, FALSE, FALSE, 0);
	 else   GuiAddHelpWidget(lwoh, table);
      }

      GuiAddHelpParagraph(lwoh, 
	 _("<b>Logfile</b>\n\n"
	   "A copy of the logging information from the log window "
	   "is written to the specified log file. This is useful to "
	   "collect information on program crashes, but affects "
	   "performance negatively."));
   }

   /* Show the created / reused window */

   gtk_widget_show_all(GTK_WIDGET(Closure->prefsWindow));
}
#endif /* WITH_GUI_YES */
