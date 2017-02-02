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

#define UNDO_SLOTS 100

enum
{  ACTION_BROWSE_LOAD,
     ACTION_FILESEL_DESTROY,
     ACTION_FILESEL_OK,
     ACTION_FILESEL_CANCEL,
   ACTION_BROWSE_SAVE,
   ACTION_BROWSE_PREV,
   ACTION_BROWSE_NEXT,
   ACTION_SORT_BY_P,
   ACTION_SORT_BY_Q,
   ACTION_LOAD_BUFFER,
     ACTION_FILESEL_LOAD_DESTROY,
     ACTION_FILESEL_LOAD_OK,
     ACTION_FILESEL_LOAD_CANCEL,
   ACTION_SAVE_BUFFER,
     ACTION_FILESEL_SAVE_DESTROY,
     ACTION_FILESEL_SAVE_OK,
     ACTION_FILESEL_SAVE_CANCEL,
   ACTION_TAG_DIFFS,
   ACTION_UNTAG,
   ACTION_UNDO,
   ACTION_REDO,
   ACTION_SMART_LEC,
   ON_CLICK_CORRECT_P,
   ON_CLICK_CORRECT_Q,
   ON_CLICK_TAG_ERASURES,
   ON_CLICK_FIND_OTHER_P,
   ON_CLICK_FIND_OTHER_Q
};

typedef struct _rb_info
{  unsigned char *rawSector;
   int sectorIndex;
   int pFailures;  /* uncorrectable P/Q vectors in this sector */
   int qFailures;
} rb_info;

typedef struct _raw_editor_context
{  GtkWindow *window;
   RawBuffer *rb;
   DefectiveSectorHeader *dsh;
   PangoLayout *layout;
   rb_info *rbInfo;
   unsigned char *undoRing[UNDO_SLOTS];
   unsigned char *undoTags[UNDO_SLOTS];
   unsigned char *tags;
   int undoPos;
   int undoBegin;
   int undoEnd;
   int lastPFrame;  /* last frame searched for other P vector */
   int lastQFrame;
   int lastPVector;  /* last vector searched for other P/Q */
   int lastQVector;

   GtkWidget *leftLabel, *rightLabel;
   GtkWidget *drawingArea;
   GtkWidget *fileSel;
   GtkWidget *loadBufSel;
   GtkWidget *saveBufSel;
   GtkWidget *saveButton;
   int daWidth;
   int daHeight;

   int charWidth;
   int charHeight;
   int byteWidth;
   int byteHeight;

   char *filepath;
   int p2,p1,q2,q1;         /* error states of P/Q vectors */
   int currentSample;
   int sectorChanged;
   int onClickAction;       /* What to do on next mouse click */

   void *smartLECHandle;    /* handle for iterative smart-lec */

} raw_editor_context;

static void evaluate_vectors(raw_editor_context*);
static void render_sector(raw_editor_context*);

static raw_editor_context* create_raw_editor_context()
{  raw_editor_context *rec = Closure->rawEditorContext;

   if(!rec) 
   {  int i;

      Closure->rawEditorContext = rec = g_malloc0(sizeof(raw_editor_context));

      for(i=0; i<UNDO_SLOTS; i++)
      {	 rec->undoRing[i] = g_malloc(CD_RAW_DUMP_SIZE);
	 rec->undoTags[i] = g_malloc(CD_RAW_DUMP_SIZE);
      }

      rec->rb  = CreateRawBuffer(CD_RAW_DUMP_SIZE);
      rec->dsh = g_malloc0(sizeof(DefectiveSectorHeader));
      rec->onClickAction = ON_CLICK_CORRECT_P;
      rec->tags = g_malloc0(CD_RAW_DUMP_SIZE);
   }

   return rec;
}

void FreeRawEditorContext(void *rptr)
{  raw_editor_context *rec = rptr;
   int i;

   for(i=0; i<UNDO_SLOTS; i++)
   {  g_free(rec->undoTags[i]);
      g_free(rec->undoRing[i]);
   }
   g_free(rec->tags);

   FreeRawBuffer(rec->rb);
   if(rec->filepath)
      g_free(rec->filepath);
   g_free(rec->dsh);
   if(rec->rbInfo)
      g_free(rec->rbInfo);
   g_free(rec);

   Closure->rawEditorContext = NULL;
}

static gboolean delete_cb(GtkWidget *widget, GdkEvent *event, gpointer data)
{
   FreeRawEditorContext(Closure->rawEditorContext);

   return FALSE;
}

/***
 *** Undo/Redo operations 
 ***/

static void undo_remember(raw_editor_context *rec)
{  
   /* Next free undo position */

   rec->undoPos = (rec->undoPos+1) % UNDO_SLOTS;

   /* Truncate undo ring if necessary */

   if(rec->undoPos == rec->undoBegin)
      rec->undoBegin = (rec->undoBegin+1) % UNDO_SLOTS;

   memcpy(rec->undoRing[rec->undoPos], rec->rb->recovered, CD_RAW_DUMP_SIZE);
   memcpy(rec->undoTags[rec->undoPos], rec->tags, CD_RAW_DUMP_SIZE);
   rec->undoEnd = rec->undoPos;

   rec->sectorChanged = TRUE;
}

static void undo(raw_editor_context *rec)
{  
   if(rec->undoPos == rec->undoBegin)
      return;

   rec->undoPos = (rec->undoPos-1);
   if(rec->undoPos < 0) rec->undoPos = UNDO_SLOTS-1;
   memcpy(rec->rb->recovered, rec->undoRing[rec->undoPos], CD_RAW_DUMP_SIZE);
   memcpy(rec->tags, rec->undoTags[rec->undoPos], CD_RAW_DUMP_SIZE);
}

static void redo(raw_editor_context *rec)
{  
   if(rec->undoPos == rec->undoEnd)
      return;

   rec->undoPos = (rec->undoPos+1) % UNDO_SLOTS;

   memcpy(rec->rb->recovered, rec->undoRing[rec->undoPos], CD_RAW_DUMP_SIZE);
   memcpy(rec->tags, rec->undoTags[rec->undoPos], CD_RAW_DUMP_SIZE);
}

/***
 *** Sorting by error load
 ***/

static int rb_info_cmp_p(const void *c1, const void *c2)
{  rb_info *rbi1 = (rb_info*)c1;
   rb_info *rbi2 = (rb_info*)c2;

   if(rbi1->pFailures < rbi2->pFailures) return -1;
   else if(rbi1->pFailures > rbi2->pFailures) return  1;
   else if(rbi1->qFailures < rbi2->qFailures) return -1;
   else if(rbi1->qFailures > rbi2->qFailures) return  1;
   else return 0;
}

static void sort_by_p(raw_editor_context *rec)
{
   if(!rec->rbInfo) return;

   qsort(rec->rbInfo, rec->rb->samplesRead, sizeof(rb_info), rb_info_cmp_p);
}

static int rb_info_cmp_q(const void *c1, const void *c2)
{  rb_info *rbi1 = (rb_info*)c1;
   rb_info *rbi2 = (rb_info*)c2;

   if(rbi1->qFailures < rbi2->qFailures) return -1;
   else if(rbi1->qFailures > rbi2->qFailures) return  1;
   else if(rbi1->pFailures < rbi2->pFailures) return -1;
   else if(rbi1->pFailures > rbi2->pFailures) return  1;
   else return 0;
}

static void sort_by_q(raw_editor_context *rec)
{
   if(!rec->rbInfo) return;

   qsort(rec->rbInfo, rec->rb->samplesRead, sizeof(rb_info), rb_info_cmp_q);
}

/* 
 * calculate failures for loaded raw sectors
 */

static void calculate_failures(raw_editor_context *rec)
{  int n_samples = rec->rb->samplesRead;
   int s;

   rec->rbInfo = g_realloc(rec->rbInfo, n_samples*sizeof(rb_info));

   for(s=0; s<n_samples; s++)
   {  unsigned char *buf = rec->rb->rawBuf[s];
      unsigned char vector[Q_VECTOR_SIZE];
      int eras[2];
      int p,defective_p = 0;
      int q,defective_q = 0;

      for(p=0; p<N_P_VECTORS; p++)
      {  GetPVector(buf, vector, p);
	 if(DecodePQ(rec->rb->rt, vector, P_PADDING, eras, 0) != 0)
	    defective_p++;
      }

      for(q=0; q<N_Q_VECTORS; q++)
      {  GetQVector(buf, vector, q);
	 if(DecodePQ(rec->rb->rt, vector, Q_PADDING, eras, 0) != 0)
	    defective_q++;
      }

      rec->rbInfo[s].rawSector = rec->rb->rawBuf[s];
      rec->rbInfo[s].sectorIndex = s;
      rec->rbInfo[s].pFailures = defective_p;
      rec->rbInfo[s].qFailures = defective_q;
   }
}

/***
 *** Browsing heuristics
 ***/

/*
 * raw sector file selection
 */

static void file_select_cb(GtkWidget *widget, gpointer data)
{  raw_editor_context *rec = Closure->rawEditorContext;
   int action = GPOINTER_TO_INT(data);

   switch(action)
   {  
      case ACTION_BROWSE_LOAD:  /* open the dialog */
	 if(!rec->fileSel)
	 {  char filename[strlen(Closure->dDumpDir)+10];

	    rec->fileSel = gtk_file_selection_new(_utf("windowtitle|Raw sector dump selection"));
	    ReverseCancelOK(GTK_DIALOG(rec->fileSel));
            g_signal_connect(G_OBJECT(rec->fileSel), "destroy",
	                     G_CALLBACK(file_select_cb), GINT_TO_POINTER(ACTION_FILESEL_DESTROY));
            g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(rec->fileSel)->ok_button),"clicked", 
	                     G_CALLBACK(file_select_cb), GINT_TO_POINTER(ACTION_FILESEL_OK));
            g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(rec->fileSel)->cancel_button),"clicked", 
	                     G_CALLBACK(file_select_cb), GINT_TO_POINTER(ACTION_FILESEL_CANCEL));
	    sprintf(filename, "%s/", Closure->dDumpDir);
	    gtk_file_selection_set_filename(GTK_FILE_SELECTION(rec->fileSel), filename);
         }
	 gtk_widget_show(rec->fileSel);
	 break;

      case ACTION_FILESEL_DESTROY:
	 rec->fileSel = NULL;
	 break;

      case ACTION_FILESEL_OK:
	 if(rec->filepath)
	    g_free(rec->filepath);
	 rec->filepath = g_strdup(gtk_file_selection_get_filename(GTK_FILE_SELECTION(rec->fileSel)));
	 gtk_widget_hide(rec->fileSel);
	 ResetRawBuffer(rec->rb);
	 ReadDefectiveSectorFile(rec->dsh, rec->rb, rec->filepath);
	 PrintPQStats(rec->rb);
	 memcpy(rec->rb->recovered, rec->rb->rawBuf[0], rec->rb->sampleSize);
	 memcpy(rec->undoRing[0], rec->rb->rawBuf[0], rec->rb->sampleSize);
	 calculate_failures(rec);
	 evaluate_vectors(rec);
	 render_sector(rec);
	 SetLabelText(GTK_LABEL(rec->rightLabel), _("%s loaded, LBA %lld, %d samples."),
		      rec->filepath, rec->rb->lba, rec->rb->samplesRead);
	 break;

      case ACTION_FILESEL_CANCEL:
	 gtk_widget_hide(rec->fileSel);
	 break;
   }
}

/*
 * Save corrected sector to image file
 */

static void save_sector(raw_editor_context *rec)
{  RawBuffer *rb = rec->rb;
   LargeFile *image;
   int unknown_fingerprint = FALSE;
   guint8 image_fp[16];
   int n;

   /*** Sanity checks */

   if(!CheckEDC(rb->recovered, rb->xaMode))
   {  Stop(_("EDC checksum does not match - sector still defective!"));
      return;
   }

   if(!CheckMSF(rb->recovered, rb->lba, STRICT_MSF_CHECK))
   {  Stop(_("LBA does not match MSF code in sector!"));
      return;
   }

   /*** Open image, verify fingerprint */

   if(!(image = LargeOpen(Closure->imageName, O_RDWR, IMG_PERMS)))
      Stop(_("Can't open %s:\n%s"),Closure->imageName,strerror(errno));

   if(!LargeSeek(image, (gint64)(2048*FINGERPRINT_SECTOR)))
      unknown_fingerprint = TRUE;
   else
   {  struct MD5Context md5ctxt;
      unsigned char buf[2048];
      int n = LargeRead(image, buf, 2048);

      MD5Init(&md5ctxt);
      MD5Update(&md5ctxt, buf, 2048);
      MD5Final(image_fp, &md5ctxt);
	 
      if(n != 2048 || (CheckForMissingSector(buf, FINGERPRINT_SECTOR, NULL, 0) != SECTOR_PRESENT))
	 unknown_fingerprint = TRUE;
   }

   if(  !unknown_fingerprint && rec->dsh->properties & DSH_HAS_FINGERPRINT
      && memcmp(image_fp, rec->dsh->mediumFP, 16))
   {  LargeClose(image);
      Stop(_("Raw sector does not belong to the selected image!"));
      return;
   }

   if(!LargeSeek(image, (gint64)(2048*rb->lba)))
   {  LargeClose(image);
      Stop(_("Failed seeking to sector %lld in image [%s]: %s"),
	   rb->lba, "raw-editor", strerror(errno));
   }

   n = LargeWrite(image, rb->recovered+rb->dataOffset, 2048);
   if(n != 2048)
   {  LargeClose(image);
      Stop(_("Failed writing to sector %lld in image [%s]: %s"),
	   rb->lba, "raw-editor", strerror(errno));
   }

   LargeClose(image);
}

/***
 *** Raw sector buffer loading/savinf
 ***/

static void buffer_io_cb(GtkWidget *widget, gpointer data)
{  raw_editor_context *rec = Closure->rawEditorContext;
   int action = GPOINTER_TO_INT(data);

   switch(action)
   {  
      case ACTION_LOAD_BUFFER:  /* open the dialog */
	 if(!rec->loadBufSel)
	 {  char filename[strlen(Closure->dDumpDir)+10];

	    rec->loadBufSel = gtk_file_selection_new(_utf("windowtitle|Load buffer from file"));
	    ReverseCancelOK(GTK_DIALOG(rec->loadBufSel));
            g_signal_connect(G_OBJECT(rec->loadBufSel), "destroy",
	                     G_CALLBACK(buffer_io_cb), GINT_TO_POINTER(ACTION_FILESEL_LOAD_DESTROY));
            g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(rec->loadBufSel)->ok_button),"clicked", 
	                     G_CALLBACK(buffer_io_cb), GINT_TO_POINTER(ACTION_FILESEL_LOAD_OK));
            g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(rec->loadBufSel)->cancel_button),"clicked", 
	                     G_CALLBACK(buffer_io_cb), GINT_TO_POINTER(ACTION_FILESEL_LOAD_CANCEL));
	    sprintf(filename, "%s/", Closure->dDumpDir);
	    gtk_file_selection_set_filename(GTK_FILE_SELECTION(rec->loadBufSel), filename);
         }
	 gtk_widget_show(rec->loadBufSel);
	 break;

      case ACTION_SAVE_BUFFER:  /* open the dialog */
	 if(!rec->saveBufSel)
	 {  char filename[strlen(Closure->dDumpDir)+10];

	    rec->saveBufSel = gtk_file_selection_new(_utf("windowtitle|Save buffer to file"));
	    ReverseCancelOK(GTK_DIALOG(rec->saveBufSel));
            g_signal_connect(G_OBJECT(rec->saveBufSel), "destroy",
	                     G_CALLBACK(buffer_io_cb), GINT_TO_POINTER(ACTION_FILESEL_SAVE_DESTROY));
            g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(rec->saveBufSel)->ok_button),"clicked", 
	                     G_CALLBACK(buffer_io_cb), GINT_TO_POINTER(ACTION_FILESEL_SAVE_OK));
            g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(rec->saveBufSel)->cancel_button),"clicked", 
	                     G_CALLBACK(buffer_io_cb), GINT_TO_POINTER(ACTION_FILESEL_SAVE_CANCEL));
	    sprintf(filename, "%s/", Closure->dDumpDir);
	    gtk_file_selection_set_filename(GTK_FILE_SELECTION(rec->saveBufSel), filename);
         }
	 gtk_widget_show(rec->saveBufSel);
	 break;

      case ACTION_FILESEL_LOAD_DESTROY:
	 rec->loadBufSel = NULL;
	 break;

      case ACTION_FILESEL_SAVE_DESTROY:
	 rec->saveBufSel = NULL;
	 break;

      case ACTION_FILESEL_LOAD_OK:
      {  LargeFile *file;
	 char *path;

	 path = (char*)gtk_file_selection_get_filename(GTK_FILE_SELECTION(rec->loadBufSel));
	 gtk_widget_hide(rec->loadBufSel);

	 file = LargeOpen(path, O_RDONLY, IMG_PERMS);
	 LargeRead(file, rec->rb->recovered, rec->rb->sampleSize);
	 LargeClose(file);

	 calculate_failures(rec);
	 evaluate_vectors(rec);
	 render_sector(rec);
	 undo_remember(rec);
	 
	 SetLabelText(GTK_LABEL(rec->rightLabel), _("Buffer loaded from %s."), path);
	 break;
      }

      case ACTION_FILESEL_SAVE_OK:
      {  LargeFile *file;
	 char *path;

	 path = (char*)gtk_file_selection_get_filename(GTK_FILE_SELECTION(rec->saveBufSel));
	 gtk_widget_hide(rec->saveBufSel);

	 file = LargeOpen(path, O_RDWR | O_CREAT, IMG_PERMS);
	 LargeWrite(file, rec->rb->recovered, rec->rb->sampleSize);
	 LargeClose(file);

	 SetLabelText(GTK_LABEL(rec->rightLabel), _("Buffer saved to %s."), path);
	 break;
      }

      case ACTION_FILESEL_LOAD_CANCEL:
	 gtk_widget_hide(rec->loadBufSel);
	 break;

      case ACTION_FILESEL_SAVE_CANCEL:
	 gtk_widget_hide(rec->saveBufSel);
	 break;
   }
}

/***
 *** Raw sector rendering
 ***/

/* Geometry calculation */

static void calculate_geometry(raw_editor_context *rec)
{  int w,h;

   SetText(rec->layout, "w", &w, &h);
   rec->charWidth  = w;
   rec->charHeight = h+h/4;

   SetText(rec->layout, "34", &w, &h);
   rec->byteWidth  = w+w/2;
   rec->byteHeight = h+h/4;

   rec->daWidth  = rec->charWidth * N_P_VECTORS;
   rec->daHeight = rec->charHeight * P_VECTOR_SIZE;

   gtk_widget_set_size_request(rec->drawingArea, rec->daWidth, rec->daHeight);
}

/* evaluate error correction load of vectors */

enum
{  P1_ERROR = 1<<0,
   P2_ERROR = 1<<1,
   P1_CPOS  = 1<<2,
   Q1_ERROR = 1<<3,
   Q2_ERROR = 1<<4,
   Q1_CPOS  = 1<<5
};

static void evaluate_vectors(raw_editor_context *rec)
{  RawBuffer *rb = rec->rb;
   unsigned char *buf = rb->recovered;
   unsigned char p_vector[P_VECTOR_SIZE];
   unsigned char q_vector[Q_VECTOR_SIZE];
   int eras[2];
   int p,q,state;

   memset(rb->byteState, 0, 2352);

   rec->p1 = rec->p2 = 0;
   for(p=0; p<N_P_VECTORS; p++)
   {  GetPVector(buf, p_vector, p);
      switch(DecodePQ(rb->rt, p_vector, P_PADDING, eras, 0))
      {  case 0:  state = 0; break;
	 case 1:  state = P1_ERROR; rec->p1++; break;
	 default: state = P2_ERROR; rec->p2++; break;
      }
      FillPVector(rb->byteState, state, p); 
      if(state == P1_ERROR)
	 rb->byteState[PToByteIndex(p, eras[0])] |= P1_CPOS;
   }

   rec->q1 = rec->q2 = 0;
   for(q=0; q<N_Q_VECTORS; q++)
   {  GetQVector(buf, q_vector, q);
      switch(DecodePQ(rb->rt, q_vector, Q_PADDING, eras, 0))
      {  case 0:  state = 0; break;
	 case 1:  state = Q1_ERROR; rec->q1++; break;
	 default: state = Q2_ERROR; rec->q2++; break;
      }
      OrQVector(rb->byteState, state, q); 
      if(state == Q1_ERROR)
	 rb->byteState[QToByteIndex(q, eras[0])] |= Q1_CPOS;
   }

   if(   CheckEDC(rb->recovered, rb->xaMode) 
	 && CheckMSF(rb->recovered, rb->lba, STRICT_MSF_CHECK))
   {  gtk_widget_set_sensitive(rec->saveButton, TRUE);
      SetLabelText(GTK_LABEL(rec->leftLabel), _("*** Well done: Sector has been recovered! ***"));
   }
   else
      SetLabelText(GTK_LABEL(rec->leftLabel), _("Current buffer state: P %d/%d, Q %d/%d"), 
		   rec->p2, rec->p1, rec->q2, rec->q1);
}

/* Render the sector */

static void render_sector(raw_editor_context *rec)
{  GdkDrawable *d = rec->drawingArea->window;
   unsigned char *buf = rec->rb->recovered;
   int idx=0;
   int i,j,w,h,x,y;

   if(!d) return;

   gdk_gc_set_rgb_fg_color(Closure->drawGC,Closure->background);
   gdk_draw_rectangle(d, Closure->drawGC, TRUE, 0, 0, rec->daWidth, rec->daHeight);

   idx = 12;
   for(j=0,y=0; j<P_VECTOR_SIZE; j++, y+=rec->charHeight)
   {  for(i=0,x=0; i<N_P_VECTORS; i++, x+=rec->charWidth)
      {  char byte[3];

	 if(rec->tags[idx])
	 {  gdk_gc_set_rgb_fg_color(Closure->drawGC,Closure->curveColor);
	    gdk_draw_rectangle(d, Closure->drawGC, TRUE, x, y, 
			       rec->charWidth, rec->charHeight);
	 }
	 else if(rec->rb->byteState[idx])
	 {  if(rec->rb->byteState[idx] & (P1_CPOS | Q1_CPOS))
	    {  gdk_gc_set_rgb_fg_color(Closure->drawGC,Closure->yellowSector);
	       gdk_draw_rectangle(d, Closure->drawGC, TRUE, x, y, 
				  rec->charWidth, rec->charHeight);
	    } 
	    else if(rec->rb->byteState[idx] & (P1_ERROR | Q1_ERROR))
	    {  gdk_gc_set_rgb_fg_color(Closure->drawGC,Closure->greenText);
	       gdk_draw_rectangle(d, Closure->drawGC, TRUE, x, y, 
				  rec->charWidth, rec->charHeight);
	    }
	    else 
	    {  gdk_gc_set_rgb_fg_color(Closure->drawGC,Closure->redText);
	       gdk_draw_rectangle(d, Closure->drawGC, TRUE, x, y,
				  rec->charWidth, rec->charHeight);
	    }
	 }

         gdk_gc_set_rgb_fg_color(Closure->drawGC,Closure->foreground);

	 sprintf(byte, "%c", isprint(buf[idx]) ? buf[idx] : '.');
	 idx++;
	 SetText(rec->layout, byte, &w, &h);
	 gdk_draw_layout(d, Closure->drawGC, x, y, rec->layout);
      }
   }
}

/* Expose event handler */

static gboolean expose_cb(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{  raw_editor_context *rec = Closure->rawEditorContext;

   if(!rec->layout)
   {  rec->layout = gtk_widget_create_pango_layout(widget, NULL);
      calculate_geometry(rec);
   }

   if(event->count) /* Exposure compression */
     return TRUE;

   evaluate_vectors(rec);
   render_sector(rec);

   return TRUE;
}

/* Button press event handler */

static gboolean button_cb(GtkWidget *widget, GdkEventButton *event, gpointer data)
{  raw_editor_context *rec = Closure->rawEditorContext;
   RawBuffer *rb = rec->rb;
   int mouse_x = event->x;
   int mouse_y = event->y;

   switch(rec->onClickAction)
   {  case ON_CLICK_CORRECT_P:
      case ON_CLICK_CORRECT_Q:
      {  unsigned char vector[Q_VECTOR_SIZE];
	 int eras[4], e_scratch, erasures = 0;
	 int i,v,err;
	 int v_size;
	 char type = (rec->onClickAction == ON_CLICK_CORRECT_P) ? 'P' : 'Q';

	 if(type=='P')
	 {  v = mouse_x/rec->charWidth;
	    v_size = P_VECTOR_SIZE;
	 }
	 else
	 {  int bytepos = 12 + mouse_x/rec->charWidth + N_P_VECTORS*(mouse_y/rec->charHeight);
	    int ignore;

	    ByteIndexToQ(bytepos, &v, &ignore);
	    v_size = Q_VECTOR_SIZE;
	 }

	 /* Count erasures */

	 if(type=='P') GetPVector(rec->tags, vector, v);
	 else          GetQVector(rec->tags, vector, v);

	 for(i=0; i<v_size; i++)
	 {  if(!vector[i]) continue;
	    if(erasures>2)
	    {  SetLabelText(GTK_LABEL(rec->rightLabel), _("%c Vector %d has >2 erasures (nothing done)."), type, v);
	       return TRUE;
	    }
	    eras[erasures++] = i;
	 }

	 /* Correct with 0,1,2 erasures (one single erasure is treated as none) */

	 if(erasures == 1) e_scratch = 0;
	 else              e_scratch = erasures;

	 if(type == 'P')
	 {  GetPVector(rb->recovered, vector, v);
	    if(e_scratch == 2)
	    {  vector[eras[0]] ^= 255;
	       vector[eras[1]] ^= 255;
	    }
	    err = DecodePQ(rb->rt, vector, P_PADDING, eras, e_scratch);
	 }
	 else
	 {  GetQVector(rb->recovered, vector, v);
	    if(e_scratch == 2)
	    {  vector[eras[0]] ^= 255;
	       vector[eras[1]] ^= 255;
	    }
	    err = DecodePQ(rb->rt, vector, Q_PADDING, eras, e_scratch);
	 }

	 if(err==0) SetLabelText(GTK_LABEL(rec->rightLabel), 
				 _("%c Vector %d already good."), type, v);
	 else if(err==1 || err==2)
	 {  if(type=='P') SetPVector(rb->recovered, vector, v);
	    else          SetQVector(rb->recovered, vector, v);
	    evaluate_vectors(rec);
	    render_sector(rec);
	    undo_remember(rec);
	    SetLabelText(GTK_LABEL(rec->rightLabel), 
			 _("%c Vector %d corrected (%d erasures)."), type, v, e_scratch);
	 }
	 else SetLabelText(GTK_LABEL(rec->rightLabel), 
			   _("%c Vector %d not correctable (%d erasures)."), type, v, e_scratch);
      }
	 break;

      case ON_CLICK_FIND_OTHER_P:
      {  unsigned char previous[P_VECTOR_SIZE];
	 int i,v;
	 int last;

	 /* Find picked vector, trivially reject if no replacements available */

	 v = mouse_x/rec->charWidth;

	 if(!rb->pn[v])
	 {  SetLabelText(GTK_LABEL(rec->rightLabel), 
			 _("no replacements for P vector %d available"), v);
	    return TRUE;
	 }

	 if(v != rec->lastPVector) 
	      last=-1;
	 else last=rec->lastPFrame;
	 GetPVector(rb->recovered, previous, v);

	 /* Try next variant */

	 i = (last+1)%rb->pn[v];
	 SetPVector(rb->recovered, rb->pList[v][i], v);
	 evaluate_vectors(rec);
	 render_sector(rec);
	 SetLabelText(GTK_LABEL(rec->rightLabel), 
		      _("Exchanged P vector %d with version %d (of %d)."),
		      v, i+1, rb->pn[v]);
	 rec->lastPFrame = i;
	 if(v != rec->lastPVector)
	    undo_remember(rec);
	 rec->lastPVector = v;
      }
	 break;

      case ON_CLICK_FIND_OTHER_Q:
      {  unsigned char previous[Q_VECTOR_SIZE];
	 int bytepos, ignore;
	 int i,v;
	 int last;

	 /* Find picked vector, trivially reject if no replacements available */

	 bytepos = 12 + mouse_x/rec->charWidth + N_P_VECTORS*(mouse_y/rec->charHeight);
	 ByteIndexToQ(bytepos, &v, &ignore);

	 if(!rb->qn[v])
	 {  SetLabelText(GTK_LABEL(rec->rightLabel), 
			 _("no replacements for Q vector %d available"), v);
	    return TRUE;
	 }

	 if(v != rec->lastQVector) 
	      last=-1;
	 else last=rec->lastQFrame;
	 GetQVector(rb->recovered, previous, v);

	 i = (last+1)%rb->qn[v];
	 SetQVector(rb->recovered, rb->qList[v][i], v);
	 evaluate_vectors(rec);
	 render_sector(rec);
	 SetLabelText(GTK_LABEL(rec->rightLabel), 
		      _("Exchanged Q vector %d with version %d (of %d)."),
		      v, i+1, rb->qn[v]);
	 rec->lastQFrame = i;
	 if(v != rec->lastQVector)
	    undo_remember(rec);
	 rec->lastQVector = v;
      }
	 break;

      case ON_CLICK_TAG_ERASURES:
      {  int bytepos = 12 + mouse_x/rec->charWidth + N_P_VECTORS*(mouse_y/rec->charHeight);

	 rec->tags[bytepos] ^= 1;
	 render_sector(rec);
	 undo_remember(rec);
      }
	 break;
   }

   //   undo_remember(rec);

   return TRUE;
}

/* Toggle button handler */

static void toggle_cb(GtkWidget *widget, gpointer data)
{  raw_editor_context *rec = Closure->rawEditorContext;
   int state  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
   int action = GPOINTER_TO_INT(data);

   if(!state) return;

   rec->onClickAction = action;
}


/* Action event handler */

static void action_cb(GtkWidget *widget, gpointer data)
{  raw_editor_context *rec = Closure->rawEditorContext;
   gint action = GPOINTER_TO_INT(data);

   switch(action)
   {  case ACTION_BROWSE_LOAD:
	 file_select_cb(NULL, ACTION_BROWSE_LOAD);
	 break;

      case ACTION_BROWSE_SAVE:
	 save_sector(rec);
	 break;

      case ACTION_BROWSE_PREV:
	 if(rec->currentSample)
	      rec->currentSample--;
	 else rec->currentSample = rec->rb->samplesRead-1;
	 rec->sectorChanged = FALSE;
	 memcpy(rec->rb->recovered, rec->rbInfo[rec->currentSample].rawSector, rec->rb->sampleSize);
	 evaluate_vectors(rec);
	 render_sector(rec);
	 undo_remember(rec);
	 SetLabelText(GTK_LABEL(rec->rightLabel), _("Showing sample %d (of %d)."), 
		      rec->currentSample, rec->rb->samplesRead);
	 break;

      case ACTION_BROWSE_NEXT:
	 if(rec->currentSample<rec->rb->samplesRead-1)
	      rec->currentSample++;
	 else rec->currentSample = 0;
	 rec->sectorChanged = FALSE;
	 memcpy(rec->rb->recovered, rec->rbInfo[rec->currentSample].rawSector, rec->rb->sampleSize);
	 evaluate_vectors(rec);
	 render_sector(rec);
	 undo_remember(rec);
	 SetLabelText(GTK_LABEL(rec->rightLabel), _("Showing sample %d (of %d)."), 
		      rec->currentSample, rec->rb->samplesRead);
	 break;

      case ACTION_UNTAG:
	 memset(rec->tags, 0, 2352);
	 render_sector(rec);
	 undo_remember(rec);
	 break;

      case ACTION_TAG_DIFFS:
      {  int i,j;

	 memset(rec->tags, 0, 2352);
	 for(i=12; i<2352; i++)
	 {  int byte = rec->rb->rawBuf[0][i];

	    for(j=1; j<rec->rb->samplesRead; j++)
	       if(byte != rec->rb->rawBuf[j][i])
		  rec->tags[i] = 1;
	 }
	 render_sector(rec);
	 undo_remember(rec);
	 break;
      }

      case ACTION_UNDO:
	 undo(rec);
	 evaluate_vectors(rec);
	 render_sector(rec);
	 break;

      case ACTION_REDO:
	 redo(rec);
	 evaluate_vectors(rec);
	 render_sector(rec);
	 break;

      case ACTION_SORT_BY_P:
	 sort_by_p(rec);
	 printf("selected %d\n", rec->rbInfo[0].sectorIndex);
	 rec->currentSample = 0;
	 memcpy(rec->rb->recovered, rec->rbInfo[rec->currentSample].rawSector, rec->rb->sampleSize);
	 evaluate_vectors(rec);
	 render_sector(rec);
	 undo_remember(rec);
	 SetLabelText(GTK_LABEL(rec->rightLabel), _("Sector with lowest P failures selected."));
	 break;

      case ACTION_SORT_BY_Q:
	 sort_by_q(rec);
	 rec->currentSample = 0;
	 memcpy(rec->rb->recovered, rec->rbInfo[rec->currentSample].rawSector, rec->rb->sampleSize);
	 evaluate_vectors(rec);
	 render_sector(rec);
	 undo_remember(rec);
	 SetLabelText(GTK_LABEL(rec->rightLabel), _("Sector with lowest Q failures selected."));
	 break;

      case ACTION_SMART_LEC:
      {  char message[SMART_LEC_MESSAGE_SIZE];
	 if(!rec->smartLECHandle)
	    rec->smartLECHandle = PrepareIterativeSmartLEC(rec->rb);
	 SmartLECIteration(rec->smartLECHandle, message);
	 evaluate_vectors(rec);
	 render_sector(rec);
	 undo_remember(rec);
	 SetLabelText(GTK_LABEL(rec->rightLabel), 
		      _("Smart L-EC: %s"), message);
	 break;
      }
   }
}

/***
 *** Raw sector editor widget creation
 ***/

void CreateRawEditor(void)
{  raw_editor_context *rec = NULL;

   rec = create_raw_editor_context();

   if(!rec->window)  /* No window to reuse? */
   {  GtkWidget *window, *outer_box, *hbox, *vbox, *label, *button;
      GtkWidget *hbox2, *vbox1, *vbox2;

      window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
      rec->window = GTK_WINDOW(window);
      gtk_window_set_title(GTK_WINDOW(window), _utf("Raw sector editor"));
      gtk_window_set_default_size(GTK_WINDOW(window), 400, 550);
      gtk_window_set_icon(GTK_WINDOW(window), Closure->windowIcon);
      gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
      gtk_container_set_border_width(GTK_CONTAINER(window), 12);

      /* Connect with the close button from the window manager */

      g_signal_connect(window, "delete_event", G_CALLBACK(delete_cb), NULL);

      /* Create the main layout of the window */

      outer_box = gtk_vbox_new(FALSE, 0);
      gtk_container_add(GTK_CONTAINER(window), outer_box);

      hbox = gtk_hbox_new(FALSE, 0);
      gtk_box_pack_start(GTK_BOX(outer_box), hbox, FALSE, FALSE, 0);

      rec->leftLabel = label = gtk_label_new("Reed-Solomon Sudoku");
      gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.0);
      gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);

      rec->rightLabel = label = gtk_label_new(_("Please load a raw sector file!"));
      gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.0);
      gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);

      hbox = gtk_hbox_new(FALSE, 0);
      gtk_box_pack_start(GTK_BOX(outer_box), hbox, TRUE, TRUE, 0);

      vbox = gtk_vbox_new(FALSE, 0);
      gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);

      /* Actions for browsing the raw samples */
      
      label = gtk_label_new(_utf("Browsing"));
      gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 10);

      hbox2 = gtk_hbox_new(FALSE, 0);
      gtk_box_pack_start(GTK_BOX(vbox), hbox2, FALSE, FALSE, 0);

      vbox1 = gtk_vbox_new(FALSE, 0);
      gtk_box_pack_start(GTK_BOX(hbox2), vbox1, FALSE, FALSE, 0);

      vbox2 = gtk_vbox_new(FALSE, 0);
      gtk_box_pack_start(GTK_BOX(hbox2), vbox2, FALSE, FALSE, 0);

      button = gtk_button_new_with_label(_utf("button|Load"));
      g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(action_cb), 
		       (gpointer)ACTION_BROWSE_LOAD);
      gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);

      rec->saveButton = button = gtk_button_new_with_label(_utf("button|Save"));
      g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(action_cb), 
		       (gpointer)ACTION_BROWSE_SAVE);
      gtk_box_pack_start(GTK_BOX(vbox2), button, FALSE, FALSE, 0);
      gtk_widget_set_sensitive(button, FALSE);

      button = gtk_button_new_with_label(_utf("button|Prev. sector"));
      g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(action_cb), 
		       (gpointer)ACTION_BROWSE_PREV);
      gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);

      button = gtk_button_new_with_label(_utf("button|Next sector"));
      g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(action_cb), 
		       (gpointer)ACTION_BROWSE_NEXT);
      gtk_box_pack_start(GTK_BOX(vbox2), button, FALSE, FALSE, 0);

      button = gtk_button_new_with_label(_utf("button|Sort by P"));
      g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(action_cb), 
		       (gpointer)ACTION_SORT_BY_P);
      gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);

      button = gtk_button_new_with_label(_utf("button|Sort by Q"));
      g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(action_cb), 
		       (gpointer)ACTION_SORT_BY_Q);
      gtk_box_pack_start(GTK_BOX(vbox2), button, FALSE, FALSE, 0);

      /* Actions for editing the recovery buffer */
      
      label = gtk_label_new(_utf("Editing"));
      gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 10);

      hbox2 = gtk_hbox_new(TRUE, 0);
      gtk_box_pack_start(GTK_BOX(vbox), hbox2, FALSE, FALSE, 0);

      vbox1 = gtk_vbox_new(TRUE, 0);
      gtk_box_pack_start(GTK_BOX(hbox2), vbox1, TRUE, TRUE, 0);

      vbox2 = gtk_vbox_new(TRUE, 0);
      gtk_box_pack_start(GTK_BOX(hbox2), vbox2, TRUE, TRUE, 0);

      button = gtk_button_new_with_label(_utf("button|Load Buf"));
      g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(buffer_io_cb), 
		       (gpointer)ACTION_LOAD_BUFFER);
      gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);

      button = gtk_button_new_with_label(_utf("button|Save Buf"));
      g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(buffer_io_cb), 
		       (gpointer)ACTION_SAVE_BUFFER);
      gtk_box_pack_start(GTK_BOX(vbox2), button, FALSE, FALSE, 0);

      button = gtk_button_new_with_label(_utf("button|Tag diffs"));
      g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(action_cb), 
		       (gpointer)ACTION_TAG_DIFFS);
      gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);

      button = gtk_button_new_with_label(_utf("button|Untag all"));
      g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(action_cb), 
		       (gpointer)ACTION_UNTAG);
      gtk_box_pack_start(GTK_BOX(vbox2), button, FALSE, FALSE, 0);

      button = gtk_button_new_with_label(_utf("button|Redo"));
      g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(action_cb), 
		       (gpointer)ACTION_REDO);
      gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);

      button = gtk_button_new_with_label(_utf("button|Undo"));
      g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(action_cb), 
		       (gpointer)ACTION_UNDO);
      gtk_box_pack_start(GTK_BOX(vbox2), button, FALSE, FALSE, 0);

      /* Actions for correcting vectors in the recovery buffer */

      label = gtk_label_new(_utf("Correction"));
      gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 10);

      button = gtk_radio_button_new_with_label(NULL, _utf("button|P vector"));
      g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(toggle_cb), 
		       (gpointer)ON_CLICK_CORRECT_P);
      gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);

      button = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(button), _utf("button|Q vector"));
      g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(toggle_cb), 
		       (gpointer)ON_CLICK_CORRECT_Q);
      gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);

      button = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(button), _utf("button|Find other P"));
      g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(toggle_cb), 
		       (gpointer)ON_CLICK_FIND_OTHER_P);
      gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);

      button = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(button), _utf("button|Find other Q"));
      g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(toggle_cb), 
		       (gpointer)ON_CLICK_FIND_OTHER_Q);
      gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);

      button = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(button), _utf("button|Tag erasures"));
      g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(toggle_cb), 
		       (gpointer)ON_CLICK_TAG_ERASURES);
      gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);

      /* Error correction heuristics */

      label = gtk_label_new(_utf("Heuristics"));
      gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 10);

      button = gtk_button_new_with_label(_utf("button|Smart L-EC"));
      g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(action_cb), 
		       (gpointer)ACTION_SMART_LEC);
      gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);


      /* drawing area */

      rec->drawingArea = gtk_drawing_area_new();
      gtk_widget_add_events(rec->drawingArea, GDK_BUTTON_PRESS_MASK);
      gtk_box_pack_start(GTK_BOX(hbox), rec->drawingArea, TRUE, TRUE, 0);
      g_signal_connect(G_OBJECT(rec->drawingArea), "expose_event", G_CALLBACK(expose_cb), NULL);
      g_signal_connect(G_OBJECT(rec->drawingArea), "button_press_event", G_CALLBACK(button_cb), NULL);
   }

   gtk_widget_show_all(GTK_WIDGET(rec->window));
}
