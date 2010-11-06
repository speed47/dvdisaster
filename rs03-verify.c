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

#include "rs03-includes.h"

#define EXIT_CODE_SIZE_MISMATCH 1
#define EXIT_CODE_VERSION_MISMATCH 2

#define EXIT_CODE_UNEXPECTED_EOF 10
#define EXIT_CODE_MISSING_SECTOR 11
#define EXIT_CODE_CHECKSUM_ERROR 12
#define EXIT_CODE_SYNDROME_ERROR 13

/***
 *** Reset the verify output window
 ***/

void ResetRS03VerifyWindow(Method *self)
{  RS03Widgets *wl = (RS03Widgets*)self->widgetList;

   SetLabelText(GTK_LABEL(wl->cmpImageSectors), "");
   SetLabelText(GTK_LABEL(wl->cmpImageMd5Sum), "");
   SetLabelText(GTK_LABEL(wl->cmpDataSection), "");
   SetLabelText(GTK_LABEL(wl->cmpCrcSection), "");
   SetLabelText(GTK_LABEL(wl->cmpEccSection), "");
   SetLabelText(GTK_LABEL(wl->cmpImageErasure), "");
   SetLabelText(GTK_LABEL(wl->cmpImagePrognosis), "");
   SetLabelText(GTK_LABEL(wl->cmpImageErasureCnt), "");
   SetLabelText(GTK_LABEL(wl->cmpImagePrognosisMsg), "");
   SetLabelText(GTK_LABEL(wl->cmpImageResult), "");

   SetLabelText(GTK_LABEL(wl->cmpEccCreatedBy), "dvdisaster");
   SetLabelText(GTK_LABEL(wl->cmpEccMethod), "");
   SetLabelText(GTK_LABEL(wl->cmpEccType), "");
   SetLabelText(GTK_LABEL(wl->cmpEccRequires), "");
   SetLabelText(GTK_LABEL(wl->cmpEccDataCrc), _("Data checksum:"));
   SetLabelText(GTK_LABEL(wl->cmpEccDataCrcVal), "");
   SetLabelText(GTK_LABEL(wl->cmpEccResult), "");
   SetLabelText(GTK_LABEL(wl->cmpEccSynLabel), "");
   SetLabelText(GTK_LABEL(wl->cmpEccSyndromes), "");

   wl->lastPercent = 0;

   FillSpiral(wl->cmpSpiral, Closure->background);
   DrawSpiral(wl->cmpSpiral);
}

/***
 *** Manage the image spiral
 ***/

/*
 * Update part of the spiral
 */

typedef struct _spiral_idle_info
{  Spiral *cmpSpiral;
   GdkColor *segColor;
   int from, to;
} spiral_idle_info;

static gboolean spiral_idle_func(gpointer data)
{  spiral_idle_info *sii = (spiral_idle_info*)data;
   int i;

   for(i=sii->from; i<=sii->to; i++)
     DrawSpiralSegment(sii->cmpSpiral, sii->segColor, i-1);

   g_free(sii);
   return FALSE;
}

static void add_verify_values(Method *method, int percent, 
			       gint64 newMissing, gint64 newCrcErrors)
{  RS03Widgets *wl = (RS03Widgets*)method->widgetList;
   spiral_idle_info *sii = g_malloc(sizeof(spiral_idle_info));

   if(percent < 0 || percent > VERIFY_IMAGE_SEGMENTS)
     return;

   sii->cmpSpiral = wl->cmpSpiral;

   sii->segColor = Closure->greenSector;
   if(newCrcErrors) sii->segColor = Closure->yellowSector;
   if(newMissing) sii->segColor = Closure->redSector;

   sii->from = wl->lastPercent+1;
   sii->to   = percent;

   wl->lastPercent = percent;

   g_idle_add(spiral_idle_func, sii);
}

/*
 * Redraw whole spiral
 */

static void redraw_spiral(RS03Widgets *wl)
{  int x = wl->cmpSpiral->mx - wl->cmpSpiral->diameter/2 + 10;

   DrawSpiralLabel(wl->cmpSpiral, wl->cmpLayout,
		   _("Good sectors"), Closure->greenSector, x, 1);

   DrawSpiralLabel(wl->cmpSpiral, wl->cmpLayout,
		   _("Sectors with CRC errors"), Closure->yellowSector, x, 2);

   DrawSpiralLabel(wl->cmpSpiral, wl->cmpLayout,
		   _("Missing sectors"), Closure->redSector, x, 3);

   DrawSpiral(wl->cmpSpiral);
}

/*
 * expose event handler for the spiral
 */

static gboolean expose_cb(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{  RS03Widgets *wl = (RS03Widgets*)data;
   GtkAllocation *a = &widget->allocation;
   int w,h,size;

   /* Finish spiral initialization */

   if(!wl->cmpLayout)
   {  SetSpiralWidget(wl->cmpSpiral, widget);
      wl->cmpLayout = gtk_widget_create_pango_layout(widget, NULL);
   }

   SetText(wl->cmpLayout, _("Missing sectors"), &w, &h);
   size = wl->cmpSpiral->diameter + 20 + 3*(10+h);  /* approx. size of spiral + labels */

   wl->cmpSpiral->mx = a->width / 2;
   wl->cmpSpiral->my = (wl->cmpSpiral->diameter + a->height - size)/2;

   if(!event->count)      /* Exposure compression */
     redraw_spiral(wl);   /* Redraw the spiral */

   return TRUE;
}

/***
 *** Create the notebook contents for the verify output
 ***/

void CreateRS03VerifyWindow(Method *self, GtkWidget *parent)
{  RS03Widgets *wl = (RS03Widgets*)self->widgetList;
   GtkWidget *sep,*ignore,*table,*table2,*lab,*frame,*d_area;
   int y1,y2;

   wl->cmpHeadline = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(wl->cmpHeadline), 0.0, 0.0); 
   gtk_misc_set_padding(GTK_MISC(wl->cmpHeadline), 5, 0);
   gtk_box_pack_start(GTK_BOX(parent), wl->cmpHeadline, FALSE, FALSE, 3);

   sep = gtk_hseparator_new();
   gtk_box_pack_start(GTK_BOX(parent), sep, FALSE, FALSE, 0);

   sep = gtk_hseparator_new();
   gtk_box_pack_start(GTK_BOX(parent), sep, FALSE, FALSE, 0);

   table = gtk_table_new(2, 2, FALSE);
   gtk_container_set_border_width(GTK_CONTAINER(table), 5);
   gtk_box_pack_start(GTK_BOX(parent), table, TRUE, TRUE, 0);


   /*** Ecc data info */

   frame = gtk_frame_new(_utf("Error correction properties"));
   gtk_table_attach(GTK_TABLE(table), frame, 0, 1, 0, 1, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 5);

   table2 = gtk_table_new(2, 8, FALSE);
   ignore = gtk_label_new("ecc info");
   gtk_container_set_border_width(GTK_CONTAINER(table2), 5);
   gtk_container_add(GTK_CONTAINER(frame), table2);
   y1=0; y2=1;

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Type:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccType = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Method:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccMethod = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Created by:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccCreatedBy = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Requires:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccRequires = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = wl->cmpEccDataCrc = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Data checksum:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccDataCrcVal = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = wl->cmpEccResult = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0);
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 2, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 4);

   /*** Image spiral */

   frame = gtk_frame_new(_utf("Image state"));
   gtk_table_attach(GTK_TABLE(table), frame, 1, 2, 0, 2, GTK_SHRINK | GTK_FILL, GTK_EXPAND | GTK_FILL, 5, 5);

   wl->cmpSpiral = CreateSpiral(Closure->grid, Closure->background, 10, 5, VERIFY_IMAGE_SEGMENTS);
   d_area = wl->cmpDrawingArea = gtk_drawing_area_new();
   gtk_widget_set_size_request(d_area, wl->cmpSpiral->diameter+20, -1);
   gtk_container_add(GTK_CONTAINER(frame), d_area);
   g_signal_connect(G_OBJECT(d_area), "expose_event", G_CALLBACK(expose_cb), (gpointer)wl);

   /*** Image info */

   frame = gtk_frame_new(_utf("Data integrity"));
   gtk_table_attach(GTK_TABLE(table), frame, 0, 1, 1, 2, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 5, 5);

   table2 = gtk_table_new(2, 7, FALSE);
   ignore = gtk_label_new("image info");
   gtk_container_set_border_width(GTK_CONTAINER(table2), 5);
   gtk_container_add(GTK_CONTAINER(frame), table2);
   y1=0; y2=1;

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Medium sectors:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpImageSectors = gtk_label_new("0");
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Data checksum:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpImageMd5Sum = gtk_label_new("0");
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Data section:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpDataSection = gtk_label_new(".");
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Crc section:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpCrcSection = gtk_label_new(".");
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   SetLabelText(GTK_LABEL(lab), _("Ecc section:"));
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccSection= gtk_label_new(".");
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = wl->cmpEccSynLabel = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0);
   SetLabelText(GTK_LABEL(lab), _("Ecc block test:")); 
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpEccSyndromes = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = wl->cmpImageErasure = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpImageErasureCnt = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = wl->cmpImagePrognosis = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 1, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 2 );
   lab = wl->cmpImagePrognosisMsg = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0); 
   gtk_table_attach(GTK_TABLE(table2), lab, 1, 2, y1, y2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
   y1++; y2++;

   lab = wl->cmpImageResult = gtk_label_new(NULL);
   gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.0);
   gtk_table_attach(GTK_TABLE(table2), lab, 0, 2, y1, y2, GTK_SHRINK | GTK_FILL, GTK_SHRINK, 5, 4);
}

/***
 *** Check the consistency of the augmented image
 ***/

/* 
 * housekeeping
 */

typedef struct
{  LargeFile *imgFile;
   LargeFile *eccFile;
   EccHeader *eh;
   RS03Layout *lay;
   RS03Widgets *wl;
   Bitmap *map;
   guint32 *crcBuf;
   gint8   *crcValid;
   unsigned char crcSum[16];
   unsigned char *eccBlock[256];
   GaloisTables *gt;
   ReedSolomonTables *rt;
} verify_closure;

static void cleanup(gpointer data)
{  verify_closure *vc = (verify_closure*)data;
   int i;

   Closure->cleanupProc = NULL;

   if(Closure->guiMode)
      AllowActions(TRUE);

   if(vc->imgFile) LargeClose(vc->imgFile);
   if(vc->lay) 
   {  if(vc->lay->target == ECC_FILE && vc->eccFile)
	 LargeClose(vc->eccFile);
      g_free(vc->lay);
   }
   if(vc->map) FreeBitmap(vc->map);
   if(vc->crcBuf) g_free(vc->crcBuf);
   if(vc->crcValid) g_free(vc->crcValid);

   for(i=0; i<255; i++)
      if(vc->eccBlock[i])
	 g_free(vc->eccBlock[i]);

   if(vc->gt) FreeGaloisTables(vc->gt);
   if(vc->rt) FreeReedSolomonTables(vc->rt);

   g_free(vc);

   if(Closure->guiMode)
     g_thread_exit(0);
}

/***
 *** Read the crc layer. Descramble CRC values from ECC block order.
 ***/

static void read_crc(verify_closure *vc, RS03Layout *lay, gint64 *crc_sig_errors)
{  EccHeader *eh = vc->eh;
   LargeFile *file;
   gint64 block_idx[256];
   guint32 crc_buf[512];
   gint64 crc_sector,s;
   int i,crc_idx;
   int crc_valid = 1;

   /* Allocate buffer for ascending sector order CRCs */

   vc->crcBuf   = g_malloc(2048 * lay->sectorsPerLayer);
   vc->crcValid = g_malloc(512 * lay->sectorsPerLayer);

   /* First sector containing crc data */

   file = lay->target == ECC_FILE ? vc->eccFile : vc->imgFile;

   if(!LargeSeek(file, 2048*(lay->firstCrcPos)))
   { if(lay->target == ECC_FILE)
	  Stop(_("Failed seeking to sector %lld in ecc file: %s"), 
	       lay->firstCrcPos, strerror(errno));
     else Stop(_("Failed seeking to sector %lld in image: %s"), 
	       lay->firstCrcPos, strerror(errno));
   }

   crc_sector = lay->firstCrcPos;

   /* Initialize ecc block index pointers.
      Note that CRC blocks are shifted by one 
      (each ECC block contains the CRC for the next ECC block) */

   for(s=0, i=0; i<lay->ndata; s+=lay->sectorsPerLayer, i++)
     block_idx[i] = s+1;

   crc_idx = 512;  /* force crc buffer reload */

   /* Cycle through the ecc blocks.
      Each ecc block contains the CRCs for the following ecc block;
      these are rearranged in ascending sector order. */

   for(s=0; s<lay->sectorsPerLayer; s++)
   {  int err;

      /* Get CRC sector for current ecc block */

      if(LargeRead(file, crc_buf, 2048) != 2048)
	 Stop(_("problem reading crc data: %s"), strerror(errno));

      err = CheckForMissingSector((unsigned char*)crc_buf, crc_sector, eh->mediumFP, eh->fpSector);
      if(err != SECTOR_PRESENT)
	 ExplainMissingSector((unsigned char*)crc_buf, crc_sector, err, TRUE);

      crc_sector++;
      crc_valid = (err == SECTOR_PRESENT);

      /* Check the CrcBlock data structure */

      if(crc_valid)
      {  CrcBlock *cb = (CrcBlock*)crc_buf;
	 if(  memcmp(cb->cookie, "*dvdisaster*", 12)
	    ||memcmp(cb->method, "RS03", 4))
	 {  crc_valid = FALSE;
	    (*crc_sig_errors)++;
         }
	 else
         {  guint32 recorded_crc = cb->selfCRC;
            guint32 real_crc;

#ifdef HAVE_BIG_ENDIAN
            cb->selfCRC = 0x47504c00;
#else
            cb->selfCRC = 0x4c5047;
#endif

            real_crc = Crc32((unsigned char*)cb, 2048);

            if(real_crc != recorded_crc)
            {  crc_valid = FALSE;
	       (*crc_sig_errors)++;
            }
         }
      }

      /* Go through all data sectors of current ecc block;
	 distribute the CRC values */

      for(i=0; i<lay->ndata-1; i++)
      {
	 /* CRC sums for the first ecc block are contained in the last
	    CRC sector. Wrap the block_idx accordingly. */
      
	 if(s == lay->sectorsPerLayer-1)
	    block_idx[i] = i*lay->sectorsPerLayer;

	 /* Sort crc into appropriate place */

	 vc->crcBuf[block_idx[i]]   = crc_buf[i];
	 vc->crcValid[block_idx[i]] = crc_valid;
	 block_idx[i]++;
      }
   }
}

/***
 *** Prognosis for correctability
 ***/

static int prognosis(verify_closure *vc, gint64 missing, gint64 expected)
{  int j,eccblock;
   int worst_ecc = 0;
   gint64 damaged_sectors = 0, damaged_eccsecs = 0;
   gint64 correctable = 0;
   gint64 recoverable;

#if 0  //FIXME: remove
   printf("prognosis(%lld, %lld)\n", missing, expected);
   for(j=0; j<GF_FIELDMAX*vc->lay->sectorsPerLayer; j++)
     if(!GetBit(vc->map, j))
       printf("%6d missing\n", j);
#endif

   for(eccblock=0; eccblock<vc->lay->sectorsPerLayer; eccblock++)
   {  int count = 255;

      /* Note: ecc file sectors are virtually mapped to augmented image sectors */

      for(j=0; j<255; j++)
      {  gint64 sector = j*vc->lay->sectorsPerLayer+eccblock;

	 if(GetBit(vc->map, sector))
	      count--;
	 else damaged_sectors++;
      }

      if(count>0)                damaged_eccsecs++;
      if(count>worst_ecc)        worst_ecc = count; 
      if(count<=vc->lay->nroots) correctable += count;
   }

   recoverable = expected - missing + correctable;

   if(damaged_sectors > 0)
   {  int percentage  = (1000*recoverable) / expected;

      PrintLog(_("- erasure counts   :  avg =  %.1f; worst = %d per ecc block.\n"),
	      (double)damaged_sectors/(double)damaged_eccsecs,worst_ecc);

      PrintLog(_("- prognosis        : %lld of %lld sectors recoverable (%d.%d%%)\n"),
	       recoverable, expected, percentage/10, percentage%10);

      if(Closure->guiMode)
      {  SetLabelText(GTK_LABEL(vc->wl->cmpImageErasure), _("Erasure counts:"));
	 SetLabelText(GTK_LABEL(vc->wl->cmpImagePrognosis), _("Prognosis:"));

	 SetLabelText(GTK_LABEL(vc->wl->cmpImageErasureCnt),
		      _("<span %s>avg =  %.1f; worst = %d per ecc block.</span>"),
		      worst_ecc <= vc->lay->nroots ? Closure->greenMarkup : Closure->redMarkup,
		      (double)damaged_sectors/(double)damaged_eccsecs,worst_ecc);

	 SetLabelText(GTK_LABEL(vc->wl->cmpImagePrognosisMsg),
		     _("<span %s>%lld of %lld sectors recoverable (%d.%d%%)</span>"),
		      recoverable < expected ? Closure->redMarkup : Closure->greenMarkup,
		     recoverable, expected, percentage/10, percentage%10);
      }
   }	      

   if(damaged_sectors && worst_ecc <= vc->lay->nroots && recoverable >= expected)
        return TRUE;
   else return FALSE;
}

/***
 *** Error syndrome check
 ***/

static int check_syndromes(verify_closure *vc)
{  RS03Layout *lay = vc->lay;
   LargeFile *eccfile; 
   gint64 layer_idx[255];
   gint64 li,ecc_block;
   gint64 cache_idx = Closure->prefetchSectors;
   gint64 ecc_good, ecc_bad, ecc_bad_sub;
   int percent,last_percent = -1;
   int bad_counted;
   int layer,i,j;

   if(Closure->guiMode)
     SetLabelText(GTK_LABEL(vc->wl->cmpHeadline), "<big>%s</big>\n<i>%s</i>",
		  _("Checking the image and error correction files."),
		  _("- Checking ecc blocks (deep verify) -"));

   /* Allocate buffers and initialize layer sector addresses */

   for(i=0, li=0; i<GF_FIELDMAX; i++,li+=lay->sectorsPerLayer)
   {  layer_idx[i] = li;
      vc->eccBlock[i] = g_try_malloc(2048*Closure->prefetchSectors);
      if(!vc->eccBlock[i])  /* out of memory */
      {  int j;

	 for(j=0; j<i; j++)
	    g_free(vc->eccBlock[j]);

	 if(Closure->guiMode)
	   SetLabelText(GTK_LABEL(vc->wl->cmpEccSyndromes),
			_("<span %s>Out of memory; try reducing sector prefetch!</span>"),
			Closure->redMarkup);
	 PrintLog(_("* Ecc block test   : out of memory; try reducing sector prefetch!\n"));
	 return 0;
      }
   }

   /* Determine source file for ecc data */

   eccfile = lay->target == ECC_FILE ? vc->eccFile : vc->imgFile;

   /* Init Reed-Solomon tables */

   vc->gt = CreateGaloisTables(RS_GENERATOR_POLY);
   vc->rt = CreateReedSolomonTables(vc->gt, RS_FIRST_ROOT, RS_PRIM_ELEM, lay->nroots);

   /* Check the error syndromes */

   ecc_good = ecc_bad = ecc_bad_sub = 0;

   for(ecc_block=0; ecc_block<lay->sectorsPerLayer; ecc_block++)
   {  gint64 num_sectors = 0; 
      unsigned char data[GF_FIELDMAX];

      /* Check for user interruption */

      if(Closure->stopActions)   
      {  SetLabelText(GTK_LABEL(vc->wl->cmpEccSyndromes), 
		      _("<span %s>Aborted by user request!</span>"),
		      Closure->redMarkup); 
         return 0;
      }

      /* Reload cache? */
      
      if(cache_idx == Closure->prefetchSectors)
      {  
	 cache_idx = 0;
	 num_sectors = Closure->prefetchSectors;
	 if(ecc_block+num_sectors >= lay->sectorsPerLayer)
	    num_sectors = lay->sectorsPerLayer - ecc_block;

	 for(layer=0; layer<GF_FIELDMAX; layer++)
	   if(layer < lay->ndata-1)
	     RS03ReadSectors(vc->imgFile, vc->lay, vc->eccBlock[layer], 
			    layer, ecc_block, num_sectors, RS03_READ_DATA);
	   else
	     RS03ReadSectors(eccfile, vc->lay, vc->eccBlock[layer], 
			    layer, ecc_block, num_sectors, RS03_READ_CRC | RS03_READ_ECC);
      }

      /* Calculate the error syndromes.
	 Note that we are only called when the image does not contain
	 dead sector markers; therefore we can skip this test. */

      bad_counted = FALSE;

      for(i=0; i<2048; i++) 
      {  int result;

	 for(j=0; j<GF_FIELDMAX; j++)
	   data[j] = vc->eccBlock[j][2048*cache_idx+i];

#if 0 //FIXME remove this
	 if((ecc_block==3 || ecc_block==89) && (i==7 || i== 109)) 
	 {  data[129]++;
	    printf("seeded error\n");
	 }
#endif
	 result = TestErrorSyndromes(vc->rt, data);

	 if(result)
	 {  ecc_bad_sub++;
	    if(!bad_counted)
	    {  bad_counted++;
	       ecc_bad++;
	    }
	 }
      }
      cache_idx++;

      if(!bad_counted) ecc_good++;

      /* Advance percentage gauge */

      percent = (100*(ecc_block+1))/lay->sectorsPerLayer;
      if(percent != last_percent)
      {  last_percent = percent;

	 if(!ecc_bad)
	 {  if(Closure->guiMode)
	      SetLabelText(GTK_LABEL(vc->wl->cmpEccSyndromes),
			   _("%d%% tested"),
			   percent);
	    PrintProgress(_("- Ecc block test   : %d%% tested"), percent);

	 }
	 else
	 {  if(Closure->guiMode)
	      SetLabelText(GTK_LABEL(vc->wl->cmpEccSyndromes),
			   _("<span %s>%lld good, %lld bad; %d%% tested</span>"),
			   Closure->redMarkup, ecc_good, ecc_bad, percent);
	    PrintProgress(_("* Ecc block test   : %lld good, %lld bad; %d%% tested")
			  , ecc_good, ecc_bad, percent);
	 }
      }
   }

   /* Tell user about our findings */

   if(!ecc_bad)
   {  if(Closure->guiMode)
       SetLabelText(GTK_LABEL(vc->wl->cmpEccSyndromes),_("pass"));
      ClearProgress();
      PrintLog(_("- Ecc block test   : pass\n"));
   }
   else
   {  if(Closure->guiMode)
       SetLabelText(GTK_LABEL(vc->wl->cmpEccSyndromes),
		    _("<span %s>%lld good, %lld bad; %lld bad sub blocks</span>"),
		    Closure->redMarkup, ecc_good, ecc_bad, ecc_bad_sub);
      PrintLog(_("* Ecc block test   : %lld good, %lld bad; %lld bad sub blocks\n"),
	       ecc_good, ecc_bad, ecc_bad_sub);

      exitCode = EXIT_CODE_SYNDROME_ERROR;
   }
   return ecc_bad;
}

/***
 *** The verify action
 ***/

void RS03Verify(Method *self)
{  verify_closure *vc = g_malloc0(sizeof(verify_closure));
   RS03Widgets *wl = self->widgetList;
   LargeFile *image,*eccfile;
   EccHeader *eh;
   RS03Layout *lay;
   struct MD5Context image_md5;
   unsigned char medium_sum[16];
   char data_digest[33], hdr_digest[33];
   gint64 s, image_sectors, eccfile_sectors, crc_idx;
   int last_percent = 0;
   unsigned char buf[2048];
   gint64 first_missing, last_missing;
   gint64 total_missing,data_missing,crc_missing,ecc_missing;
   gint64 new_missing = 0, new_crc_errors = 0;
   gint64 data_crc_errors,crc_sig_errors;
   gint64 expected_sectors,virtual_expected;
   gint64 expected_image_sectors, expected_eccfile_sectors;
   int major,minor,pl;
   char method[5];
   char *img_advice = NULL;
   char *ecc_advice = NULL;
   char *version;
   int syn_error = 0;
   int try_it;

   /*** Prepare for early termination */

   RegisterCleanup(_("Check aborted"), cleanup, vc);
   vc->wl = wl;
   vc->eh = eh = self->lastEh;  /* will always be present */

   /*** Open the .iso file */

   LargeStat(Closure->imageName, &image_sectors);
   image_sectors /= 2048;
   image = vc->imgFile = LargeOpen(Closure->imageName, O_RDONLY, IMG_PERMS);

   if(!image)  /* Failing here is unlikely since caller could open it */
     Stop("Could not open %s: %s",Closure->imageName, strerror(errno));

   PrintLog(_("\n%s present.\n"), Closure->imageName);

   /*** Optionally open the ecc file, announce what we are going to do */

   LargeStat(Closure->eccName, &eccfile_sectors);
   eccfile_sectors /= 2048;
   if(eh->methodFlags[0] & MFLAG_ECC_FILE)
   {
      eccfile = vc->eccFile = LargeOpen(Closure->eccName, O_RDONLY, IMG_PERMS);

      if(!eccfile)  /* Failing here is unlikely since caller could open it */
	 Stop("Could not open %s: %s",Closure->eccName, strerror(errno));


      if(Closure->guiMode)
	SetLabelText(GTK_LABEL(wl->cmpHeadline), "<big>%s</big>\n<i>%s</i>",
		     _("Checking the image and error correction files."),
		     _("- Checking image file -"));

      PrintLog(_("%s present.\n"), Closure->eccName);
   }
   else 
   {  
     eccfile = image;
     if(Closure->guiMode)
       SetLabelText(GTK_LABEL(wl->cmpHeadline), "<big>%s</big>\n<i>%s</i>",
		    _("Checking the image file."),
		    _("- Checking image file -"));
   }

   /*** Calculate the layout */

   if(eh->methodFlags[0] & MFLAG_ECC_FILE)
        lay = vc->lay = CalcRS03Layout(uchar_to_gint64(eh->sectors), eh, ECC_FILE); 
   else lay = vc->lay = CalcRS03Layout(uchar_to_gint64(eh->sectors), eh, ECC_IMAGE); 


   /*** Print information on the ecc portion */

   PrintLog(_("\nError correction properties:\n"));

   /* Check size of error correction file */

   expected_eccfile_sectors = 2 + (lay->nroots+1)*lay->sectorsPerLayer;
   if(lay->target == ECC_FILE && expected_eccfile_sectors != eccfile_sectors)
   {  char *msg;

      if(expected_eccfile_sectors > eccfile_sectors)
	   msg = g_strdup_printf(_("Ecc file is %lld sectors shorter than expected."),
			         expected_eccfile_sectors - eccfile_sectors);
      else msg = g_strdup_printf(_("Ecc file is %lld sectors longer than expected."), 
				 eccfile_sectors - expected_eccfile_sectors);

      if(Closure->guiMode)
 	 ecc_advice = g_strdup_printf("<span %s>%s</span>", Closure->redMarkup, msg);

      PrintLog(_("* Warning          : %s\n"), msg);
      g_free(msg);
      exitCode = EXIT_CODE_SIZE_MISMATCH;
   }
   
   /* Error correction type */

   if(eh->methodFlags[0] & MFLAG_ECC_FILE)
        PrintLog(_("- type             : Error correction file\n"));
   else PrintLog(_("- type             : Augmented image\n"));

   if(Closure->guiMode)
   {  if(eh->methodFlags[0] & MFLAG_ECC_FILE)
	   SetLabelText(GTK_LABEL(wl->cmpEccType), _("Error correction file"));
      else SetLabelText(GTK_LABEL(wl->cmpEccType), _("Augmented image"));
   }

   /* Error correction method */

   memcpy(method, eh->method, 4); method[4] = 0;

   PrintLog(_("- method           : %4s, %d roots, %4.1f%% redundancy.\n"),
	    method, eh->eccBytes, 
	    ((double)eh->eccBytes*100.0)/(double)eh->dataBytes);

   if(Closure->guiMode)
     SetLabelText(GTK_LABEL(wl->cmpEccMethod), _("%4s, %d roots, %4.1f%% redundancy"),
		  method, eh->eccBytes, 
		  ((double)eh->eccBytes*100.0)/(double)eh->dataBytes);

   /* Creator version */

   major = eh->creatorVersion/10000; 
   minor = (eh->creatorVersion%10000)/100;
   pl    = eh->creatorVersion%100;

   if(eh->creatorVersion%100)        
   {  char *format, *color_format = NULL;

      if(eh->methodFlags[3] & MFLAG_DEVEL) 
      {  format = "%s-%d.%d (devel-%d)";
	 color_format = "%s-%d.%d <span %s>(devel-%d)</span>";
      }
      else if(eh->methodFlags[3] & MFLAG_RC) 
      {  format = "%s-%d.%d (rc-%d)";
	 color_format = "%s-%d.%d <span %s>(rc-%d)</span>";
      }
      else format = "%s-%d.%d (pl%d)";

      PrintLog(format, _("- created by       : dvdisaster"), major, minor, pl);
      PrintLog("\n");

      if(!color_format) color_format = format;
      if(Closure->guiMode)
      {  if(!color_format)
	      SetLabelText(GTK_LABEL(wl->cmpEccCreatedBy), color_format, 
			   "dvdisaster", major, minor, Closure->redMarkup, pl);
	 else SetLabelText(GTK_LABEL(wl->cmpEccCreatedBy), format, 
			   "dvdisaster", major, minor, pl);
      }
   }
   else
   {  PrintLog(_("- created by       : dvdisaster-%d.%d\n"), 
	       major, minor);

      if(Closure->guiMode)
	SetLabelText(GTK_LABEL(wl->cmpEccCreatedBy), "dvdisaster-%d.%d", major, minor);
   }

   /* Required dvdisaster version */

   if(eh->neededVersion%100)        
        version = g_strdup_printf("%d.%d (pl%d)",
				  eh->neededVersion/10000,
				  (eh->neededVersion%10000)/100,
				  eh->neededVersion%100);
   else version = g_strdup_printf("%d.%d",
				  eh->neededVersion/10000,
				  (eh->neededVersion%10000)/100);

   if(!VerifyVersion(eh, 0))
   {  PrintLog(_("- requires         : dvdisaster-%s\n"), version);

      if(Closure->guiMode)
	SetLabelText(GTK_LABEL(wl->cmpEccRequires), "dvdisaster-%s", version);
   }
   else 
   {  PrintLog(_("* requires         : dvdisaster-%s (BAD)\n"
		 "* Warning          : The following output might be incorrect.\n"
		 "*                  : Please visit http://www.dvdisaster.com for an upgrade.\n"),
	       version);

     if(Closure->guiMode)
     {  SetLabelText(GTK_LABEL(wl->cmpEccRequires), 
		     "<span %s>dvdisaster-%s</span>",
		     Closure->redMarkup, version);
        if(!ecc_advice) 
	   ecc_advice = g_strdup_printf(_("<span %s>Please upgrade your version of dvdisaster!</span>"), Closure->redMarkup);
     }

     exitCode = EXIT_CODE_VERSION_MISMATCH;
   }

   g_free(version);

   /* image md5sum as stored in the ecc header */

   if(eh->methodFlags[0] & MFLAG_DATA_MD5)
        AsciiDigest(hdr_digest, eh->mediumSum);
   else strcpy(hdr_digest, _("none available"));

   PrintLog(_("- data md5sum      : %s\n"),hdr_digest);

   if(Closure->guiMode)
     SetLabelText(GTK_LABEL(wl->cmpEccDataCrcVal), "%s", hdr_digest);

   /* print advice collected from above tests */

   if(Closure->guiMode)
   {  if(ecc_advice) 
      {  SetLabelText(GTK_LABEL(wl->cmpEccResult), ecc_advice);
         g_free(ecc_advice);
      }
   }

   /*** Print information on image size */

   PrintLog(_("\nData integrity:\n"));

   /* Provide enough bitmap space for all layers */

   vc->map = CreateBitmap0(GF_FIELDMAX*lay->sectorsPerLayer);

   /* Expected and real sectors */

   if(lay->target == ECC_FILE)
   {  expected_sectors = lay->dataSectors + lay->totalSectors;  /* image + ecc file */
      virtual_expected = GF_FIELDMAX*lay->sectorsPerLayer;      /* for prognosis map */
      expected_image_sectors = lay->dataSectors;                /* just the expected image size */
   }
   else 
   {  virtual_expected = expected_sectors = expected_image_sectors = lay->totalSectors;
      SetBit(vc->map, lay->eccHeaderPos);
      SetBit(vc->map, lay->eccHeaderPos+1);
   }

   if(expected_image_sectors == image_sectors)
   {  if(lay->target == ECC_FILE)
      {  if(Closure->guiMode)
	   SetLabelText(GTK_LABEL(wl->cmpImageSectors), _("%lld in image; %lld in ecc file"), 
			image_sectors, eccfile_sectors);
	 PrintLog(_("- sectors          : %lld in image; %lld in ecc file\n"), 
		  image_sectors, eccfile_sectors);
      }
      else 
      {  if(Closure->guiMode)
	   SetLabelText(GTK_LABEL(wl->cmpImageSectors), _("%lld total / %lld data"), 
			image_sectors, lay->dataSectors);
	 PrintLog(_("- medium sectors   : %lld total / %lld data\n"),
		  image_sectors, lay->dataSectors);
      }
   }
   else
   {  if(Closure->guiMode)
        SetLabelText(GTK_LABEL(wl->cmpImageSectors), _("<span %s>%lld (%lld expected)</span>"), 
		     Closure->redMarkup, image_sectors, expected_image_sectors);
     if(expected_image_sectors > image_sectors)
       img_advice = g_strdup_printf(_("<span %s>Image file is %lld sectors shorter than expected.</span>"), Closure->redMarkup, expected_image_sectors - image_sectors);
     else img_advice = g_strdup_printf(_("<span %s>Image file is %lld sectors longer than expected.</span>"), Closure->redMarkup, image_sectors - expected_image_sectors);
   }
   
   if(Closure->quickVerify)
   {  PrintLog(_("* quick mode        : image NOT scanned\n"));
      goto terminate;
   }

   /*** Read the CRC portion */ 

   crc_sig_errors = 0;
   read_crc(vc, lay, &crc_sig_errors);

   /*** Check the data portion of the image file for the
	"dead sector marker" and CRC errors */
   
   if(!LargeSeek(image, 0))
     Stop(_("Failed seeking to start of image: %s\n"), strerror(errno));

   if(lay->target == ECC_FILE)
     if(!LargeSeek(eccfile, 4096))  /* skip the header */
       Stop(_("Failed seeking to start of ecc file: %s\n"), strerror(errno));

   MD5Init(&image_md5);

   first_missing = last_missing = -1;
   total_missing = data_missing = crc_missing = ecc_missing = 0;
   data_crc_errors = 0;
   crc_idx = 0;

   for(s=0; s<virtual_expected; s++)
   {  int percent,current_missing;
      int defective = 0;

      /* Check for user interruption */

      if(Closure->stopActions)   
      {  SetLabelText(GTK_LABEL(wl->cmpImageResult), 
		      _("<span %s>Aborted by user request!</span>"),
		      Closure->redMarkup); 
         goto terminate;
      }

      /* Read the next sector */

      if(lay->target == ECC_IMAGE || s<lay->dataSectors)
      {  /* Read from image file */
	 if(s < image_sectors)  /* image may be truncated */
	 {  int n = LargeRead(image, buf, 2048);
            if(n != 2048)
	    { exitCode = EXIT_CODE_UNEXPECTED_EOF;
 	      Stop(_("premature end in image (only %d bytes): %s\n"),n,strerror(errno));
	    }
	 }
         else CreateMissingSector(buf, s, eh->mediumFP, eh->fpSector, "padding beyond the image");
      }
      else
      {  /* Simulate the non-existent padding area in ecc files */
 	 if(s >= lay->dataSectors && s<(lay->ndata-1)*lay->sectorsPerLayer)
	 {  memset(buf, 0, 2048);
	 }

	 /* Read from ecc file */
	 else if(s < (lay->ndata-1)*lay->sectorsPerLayer+eccfile_sectors-2)
	 {  int n = LargeRead(eccfile, buf, 2048);
            if(n != 2048)
	    { exitCode = EXIT_CODE_UNEXPECTED_EOF;
	      Stop(_("premature end in ecc file (only %d bytes): %s\n"),n,strerror(errno));
	    }
	 }
         else   /* ecc file is truncated */
	 {  CreateMissingSector(buf, s, eh->mediumFP, eh->fpSector, "padding beyond the image");
	 }
      }

      if(s < lay->dataSectors)
      {  if(s < lay->dataSectors - 1)
	      MD5Update(&image_md5, buf, 2048);
	 else MD5Update(&image_md5, buf, eh->inLast);
      }

      /* Look for the dead sector marker */

      current_missing = CheckForMissingSector(buf, s, eh->mediumFP, eh->fpSector);
      if(current_missing != SECTOR_PRESENT)
	 ExplainMissingSector(buf, s, current_missing, TRUE);

      if(current_missing)
      {  
	 if(first_missing < 0) first_missing = s;
         last_missing = s;
	 total_missing++;
	 new_missing++;

	 if(lay->target == ECC_IMAGE)
	 {  if(s < lay->firstCrcPos) data_missing++;
	    else if(s >= lay->firstCrcPos && s < lay->firstEccPos) crc_missing++;
	    else ecc_missing++;
	 }
	 else  /* ecc file case */
	 {  if(s < lay->dataSectors) data_missing++;
	    else if(s < lay->ndata*lay->sectorsPerLayer) crc_missing++;
	    else ecc_missing++;
	 }
	 defective = TRUE;
	 exitCode = EXIT_CODE_MISSING_SECTOR;
      }

      /* Report dead sectors. Combine subsequent missing sectors into one report. */

      if(!current_missing || s==virtual_expected-1)
      {  if(first_missing>=0)
	{   gint64 first, last;
	    char *ecc_msg;
	
	    if(lay->target == ECC_FILE && last_missing >= (lay->ndata-1)*lay->sectorsPerLayer)
	    {    first = first_missing - (lay->ndata-1)*lay->sectorsPerLayer + 2;
	         last = last_missing - (lay->ndata-1)*lay->sectorsPerLayer + 2;
	         ecc_msg = g_strdup(_(" (in ecc file)"));
	    }
	    else 
	    {    first = first_missing;
	         last = last_missing;
		 ecc_msg = g_strdup(" ");
	    }
	    if(first_missing == last_missing)
	         PrintCLI(_("* missing sector   : %lld%s\n"), first,ecc_msg);
	    else PrintCLI(_("* missing sectors  : %lld - %lld%s\n"), first, last, ecc_msg);
	    first_missing = -1;
	    g_free(ecc_msg);
	 }
      }

      /* If the image sector is from the data portion and it was readable, 
	 test its CRC sum */

      if(   !current_missing
	 && (   (lay->target == ECC_IMAGE && s < lay->firstCrcPos)
	     || (lay->target == ECC_FILE && s < lay->dataSectors)))
      {  guint32 crc = Crc32(buf, 2048);

	 if(vc->crcValid[crc_idx] && crc != vc->crcBuf[crc_idx])
	 {  PrintCLI(_("* CRC error, sector: %lld\n"), s);
	    data_crc_errors++;
	    new_crc_errors++;
	    defective = TRUE;
	    exitCode = EXIT_CODE_CHECKSUM_ERROR;
	 }
      }
      crc_idx++;

      if(!defective)
	SetBit(vc->map, s);

      if(Closure->guiMode) 
      {   /* data part / spiral animation */
	  percent = (VERIFY_IMAGE_SEGMENTS*(s+1))/virtual_expected;

	  /* percentage is reset / output differently for ecc file part */
	  if(lay->target == ECC_FILE && s >= lay->dataSectors) 
	    percent = (100*(s+1-lay->dataSectors)/(virtual_expected-lay->dataSectors));
      }  
      else  percent = (100*(s+1))/virtual_expected;

      if(last_percent != percent) /* Update sector results */
      {  PrintProgress(_("- testing sectors  : %3d%%") ,percent);
	 if(Closure->guiMode)
	 {  if(lay->target == ECC_IMAGE)
	    {  add_verify_values(self, percent, new_missing, new_crc_errors); 
	    }
	    else /* do not include ecc file sectors in the spiral! */
	    {  if(s<lay->dataSectors)
	       {  int image_percent = (VERIFY_IMAGE_SEGMENTS*(s+1))/lay->dataSectors;
	          
	          add_verify_values(self, image_percent, new_missing, new_crc_errors); 
	       }
	       else
	       {  SetLabelText(GTK_LABEL(wl->cmpEccSyndromes),"%d%% tested",percent);
	       }
	    }

	    if(data_missing || data_crc_errors)
	      SetLabelText(GTK_LABEL(wl->cmpDataSection), 
			   _("<span %s>%lld sectors missing; %lld CRC errors</span>"),
			   Closure->redMarkup, data_missing, data_crc_errors);
	    if(crc_missing || crc_sig_errors)
	      SetLabelText(GTK_LABEL(wl->cmpCrcSection), 
			   _("<span %s>%lld sectors missing; %lld signature errors</span>"),
			   Closure->redMarkup, crc_missing, crc_sig_errors);
	    if(ecc_missing)
	      SetLabelText(GTK_LABEL(wl->cmpEccSection), 
			   _("<span %s>%lld sectors missing</span>"),
			   Closure->redMarkup, ecc_missing);
	 }
	 last_percent = percent;
	 new_missing = new_crc_errors = 0;
      }
      
      /* If we have processed the image and are about to switch over
	 to the ecc file, do some bookkeeping. */

      if(lay->target == ECC_FILE && s == lay->dataSectors-1)
      {	 
	if(Closure->guiMode)
	{  /* flush/complete spiral */
	   add_verify_values(self, VERIFY_IMAGE_SEGMENTS, new_missing, new_crc_errors); 

	   SetLabelText(GTK_LABEL(wl->cmpHeadline), "<big>%s</big>\n<i>%s</i>",
			_("Checking the image and error correction files."),
			_("- Checking ecc file -"));

	   SetLabelText(GTK_LABEL(wl->cmpEccSynLabel), _("Error correction file:"));
	   last_percent = 0; /* restart counting for ecc file */
	}
      }
   }

   /* Complete damage summary */

   if(Closure->guiMode)
   {  if(data_missing || data_crc_errors)
        SetLabelText(GTK_LABEL(wl->cmpDataSection), 
		     _("<span %s>%lld sectors missing; %lld CRC errors</span>"),
		     Closure->redMarkup, data_missing, data_crc_errors);
      if(crc_missing || crc_sig_errors)
        SetLabelText(GTK_LABEL(wl->cmpCrcSection), 
		     _("<span %s>%lld sectors missing; %lld signature errors</span>"),
		     Closure->redMarkup, crc_missing, crc_sig_errors);
      if(ecc_missing)
	SetLabelText(GTK_LABEL(wl->cmpEccSection), 
		     _("<span %s>%lld sectors missing</span>"),
		     Closure->redMarkup, ecc_missing);
   }

   /* The image md5sum is only useful if all blocks have been successfully read. */

   MD5Final(medium_sum, &image_md5);
   AsciiDigest(data_digest, medium_sum);

   /* Do a resume of our findings */ 

   if(!total_missing && !data_crc_errors)
      PrintLog(_("- good image/file  : all sectors present\n"
		 "- data md5sum      : %s\n"),data_digest);
   else
   {  if(!data_crc_errors)
         PrintLog(_("* BAD image/file   : %lld sectors missing\n"), total_missing);
      if(!total_missing)
	 PrintLog(_("* suspicious image : all sectors present, but %lld CRC errors\n"), 
		  data_crc_errors);
      if(total_missing && data_crc_errors)
	 PrintLog(_("* BAD image        : %lld sectors missing, %lld CRC errors\n"), 
		  total_missing, data_crc_errors);

      PrintLog(_("  ... data section   : %lld sectors missing; %lld CRC errors\n"), 
	       data_missing, data_crc_errors);
      if(!total_missing && !data_crc_errors && !crc_sig_errors)
	PrintLog(_("  ... data md5sum    : %s\n"), data_digest); 
      PrintLog(_("  ... crc section    : %lld sectors missing\n"), crc_missing);
      PrintLog(_("  ... ecc section    : %lld sectors missing\n"), ecc_missing);
   }

   if(Closure->guiMode)
   {  if(!data_missing && !data_crc_errors) 
                        SetLabelText(GTK_LABEL(wl->cmpDataSection), _("complete"));
      if(!crc_missing && !crc_sig_errors)  
	                SetLabelText(GTK_LABEL(wl->cmpCrcSection), _("complete"));
      if(!ecc_missing)  SetLabelText(GTK_LABEL(wl->cmpEccSection), _("complete"));
     
      SetLabelText(GTK_LABEL(wl->cmpImageMd5Sum), "%s", data_missing ? "-" : data_digest);
   }

   /*** Test error syndromes */

   if(Closure->guiMode)
   {  SetLabelText(GTK_LABEL(wl->cmpEccSynLabel), _("Ecc block test:"));
      SetLabelText(GTK_LABEL(wl->cmpEccSyndromes), "");
   }
   if(0&&total_missing + data_crc_errors != 0)
   { if(Closure->guiMode) 
        SetLabelText(GTK_LABEL(wl->cmpEccSyndromes),
		     _("<span %s>Skipped; not useful on known defective image</span>"),
		     Closure->redMarkup);

     PrintLog(_("* Ecc block test   : skipped; not useful on defective image\n"));
   }
   else syn_error = check_syndromes(vc);

   /*** Print image advice */

   if(Closure->guiMode)
   {
      if(img_advice) 
      {  SetLabelText(GTK_LABEL(wl->cmpImageResult), img_advice);
         g_free(img_advice);
      }
      else 
      {  if(!total_missing && !data_crc_errors && !syn_error)
	    SetLabelText(GTK_LABEL(wl->cmpImageErasure),  /* avoid two blank lines */
			 _("<span %s>Good image.</span>"),
			 Closure->greenMarkup);
	 else
           SetLabelText(GTK_LABEL(wl->cmpImageResult),
			_("<span %s>Damaged image.</span>"),
			Closure->redMarkup);
      }
   }

   /*** Print final results */

   try_it = prognosis(vc, total_missing+data_crc_errors, expected_sectors);

   if(Closure->guiMode)
   {  if(total_missing || data_crc_errors)
      {  if(try_it) SetLabelText(GTK_LABEL(wl->cmpImageResult),
				 _("<span %s>Full data recovery is likely.</span>"),
				 Closure->greenMarkup);
         else       SetLabelText(GTK_LABEL(wl->cmpImageResult),
				 _("<span %s>Full data recovery is NOT possible.</span>"),
				 Closure->redMarkup);
      }
   }

   /*** Close and clean up */

terminate:
   cleanup((gpointer)vc);
}
