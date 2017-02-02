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
#include "galois-inlines.h"

/***
 *** Internal housekeeping
 ***/

typedef struct
{  RS03Widgets *wl;
   RS03Layout *lay;
   GaloisTables *gt;
   ReedSolomonTables *rt;
   Image *image;
   int earlyTermination;
   char *msg;
   unsigned char *imgBlock[255];
} fix_closure;

static void fix_cleanup(gpointer data)
{  fix_closure *fc = (fix_closure*)data;
   int i;

   UnregisterCleanup();

   if(Closure->guiMode)
   {  if(fc->earlyTermination)
         SwitchAndSetFootline(fc->wl->fixNotebook, 1,
			      fc->wl->fixFootline,
			      _("<span %s>Aborted by unrecoverable error.</span>"),
			      Closure->redMarkup); 
      AllowActions(TRUE);
   }

   /** Clean up */

   if(fc->msg) g_free(fc->msg);
   if(fc->image) CloseImage(fc->image);

   for(i=0; i<255; i++)
   {  if(fc->imgBlock[i])
	 g_free(fc->imgBlock[i]); 
   }

   if(fc->lay) g_free(fc->lay);
   if(fc->gt) FreeGaloisTables(fc->gt);
   if(fc->rt) FreeReedSolomonTables(fc->rt);

   g_free(fc);

   if(Closure->guiMode)
     g_thread_exit(0);
}

/*
 * Expand a truncated image 
 */

static void expand_image(Image *image, EccHeader *eh, gint64 new_size)
{  int last_percent, percent;
   gint64 sectors, new_sectors;

   if(!LargeSeek(image->file, image->file->size))
     Stop(_("Failed seeking to end of image: %s\n"), strerror(errno));

   last_percent = 0;
   new_sectors = new_size - image->sectorSize;

   for(sectors = 0; sectors < new_sectors; sectors++)
   {  unsigned char buf[2048];
      int length,n;

      CreateMissingSector(buf, image->sectorSize+sectors, 
			  image->imageFP, FINGERPRINT_SECTOR, 
			  "RS03 fix placeholder");

      if(sectors != new_sectors-1) length = 2048;
      else length = eh->inLast;  /* non-image file may be clipped */

      n = LargeWrite(image->file, buf, length);
      if(n != length)
	Stop(_("Failed expanding the image: %s\n"), strerror(errno));

      percent = (100*sectors) / new_sectors;
      if(last_percent != percent)
      {  if(Closure->guiMode)
	  ;
	 else PrintProgress(_("Expanding image: %3d%%"), percent);
	 last_percent = percent; 
      }
   }

   if(Closure->guiMode)
     ;
   else 
   {  PrintProgress(_("Expanding image: %3d%%"), 100);
      PrintProgress("\n");
   }
}

/***
 *** Test and fix the current image.
 ***/

void RS03Fix(Image *image)
{  Method *self = FindMethod("RS03");
   RS03Widgets *wl = (RS03Widgets*)self->widgetList;
   RS03Layout *lay;
   fix_closure *fc = g_malloc0(sizeof(fix_closure)); 
   EccHeader *eh;
   gint32 *gf_index_of;
   gint32 *gf_alpha_to;
   gint64 block_idx[255];
   gint64 s;
   guint32 *crc_buf, last_crc_sector1[512], last_crc_sector2[512];
   int nroots,ndata;
   int crc_idx;
   int crc_valid = TRUE;
   int cache_size, cache_sector, cache_offset;
   int erasure_count,erasure_list[255],erasure_map[255];
   int error_count;
   int percent, last_percent;
   int worst_ecc = 0, local_plot_max = 0;
   int i,j;
   gint64 crc_errors=0;
   gint64 data_count=0;
   gint64 ecc_count=0;
   gint64 crc_count=0;
   gint64 data_corr=0;
   gint64 ecc_corr=0;
   gint64 corrected=0;
   gint64 uncorrected=0;
   gint64 damaged_sectors=0;
   gint64 damaged_eccblocks=0;
   gint64 damaged_eccsecs=0;
   gint64 expected_sectors;
   char *t=NULL,*msg;

   /*** Register the cleanup procedure for GUI mode */

   fc->image = image;
   fc->wl = wl;
   fc->earlyTermination = TRUE;
   RegisterCleanup(_("Repairing of image aborted"), fix_cleanup, fc);

   if(image->eccFileHeader)
        eh = image->eccFileHeader;
   else eh = image->eccHeader;

   /*** Open the image file */

   if(Closure->guiMode)
     SetLabelText(GTK_LABEL(wl->fixHeadline),
		  _("<big>Repairing the image.</big>\n<i>%s</i>"),
		  _("Opening files..."));

   /* Calculate the layout and optinally open thee ecc file */

   if(eh->methodFlags[0] & MFLAG_ECC_FILE)
   {  lay = fc->lay = CalcRS03Layout(image, ECC_FILE); 
   }
   else 
   {  lay = fc->lay = CalcRS03Layout(image, ECC_IMAGE); 
   }

   ndata  = lay->ndata;
   nroots = lay->nroots;

   /*** Set up the Galois field arithmetic */

   fc->gt      = CreateGaloisTables(RS_GENERATOR_POLY);
   fc->rt      = CreateReedSolomonTables(fc->gt, RS_FIRST_ROOT, RS_PRIM_ELEM, nroots);
   gf_index_of = fc->gt->indexOf;
   gf_alpha_to = fc->gt->alphaTo;

   /*** Expand a truncated image with "dead sector" markers.
        If the images have the same number of sectors but a 
        different number of bytes in the last sector, 
        expand_image() is not called here as the missing bytes
        are honoured in the RS03ReadSectors() functions and
        the error correction will expand the image by writing
        out the correct number of bytes. */

   if(eh->methodFlags[0] & MFLAG_ECC_FILE)
        expected_sectors = lay->dataSectors;
   else expected_sectors = lay->totalSectors;

   if(image->sectorSize < expected_sectors)
     expand_image(image, eh, expected_sectors);

   /*** Announce what we are going to do */

   if(Closure->guiMode)
   {  if(eh->methodFlags[0] & MFLAG_ECC_FILE)
        msg = g_strdup_printf(_("Error correction file using Method RS03, %d roots, %4.1f%% redundancy."),
			      eh->eccBytes, 
			      ((double)eh->eccBytes*100.0)/(double)eh->dataBytes);
      else
	msg = g_strdup_printf(_("Image contains error correction data: Method RS03, %d roots, %4.1f%% redundancy."),
			      eh->eccBytes, 
			      ((double)eh->eccBytes*100.0)/(double)eh->dataBytes);

      SetLabelText(GTK_LABEL(wl->fixHeadline),
		  _("<big>Repairing the image.</big>\n<i>%s</i>"), msg);
      RS03SetFixMaxValues(wl, eh->dataBytes, eh->eccBytes, expected_sectors);
      g_free(msg);
   }    

   PrintLog(_("\nFix mode(%s): Repairable sectors will be fixed in the image.\n"),
	    eh->methodFlags[0] & MFLAG_ECC_FILE ? "RS03f" : "RS03i");

   /*** Image is a few bytes too long */

   if(image->sectorSize == expected_sectors && image->inLast > eh->inLast)
   {  int difference = image->inLast - eh->inLast;
      guint64 expected_image_size = 2048*(expected_sectors-1)+eh->inLast;

      if(Closure->guiMode)
      {  int answer = ModalDialog(GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL, NULL,
				  _("The image file is %d bytes longer than noted\n"
				    "in the ecc file. Shall the superfluous bytes\n"
				    "be removed from the image file?\n"),
				    difference);

	 if(!answer)
	 {  SwitchAndSetFootline(fc->wl->fixNotebook, 1,
				 fc->wl->fixFootline,
				 _("<span %s>Aborted by user request!</span>"),
				 Closure->redMarkup); 
	    fc->earlyTermination = FALSE;  /* suppress respective error message */
	    goto terminate;
	 }
      }

      if(!Closure->guiMode && !Closure->truncate)
        Stop(_("The image file is %d bytes longer than noted\n"
	       "in the ecc file.\n"
               "Add the --truncate option to the program call\n"
	       "to have the superfluous sectors removed."),
	     difference);
      
      if(!LargeTruncate(image->file, expected_image_size))
	Stop(_("Could not truncate %s: %s\n"),Closure->imageName,strerror(errno));

      PrintLog(_("Image has been truncated by %d bytes.\n"), difference);
      image->inLast = eh->inLast;
   }

   /*** Truncate an image with trailing garbage (more than one sector) */

   if(image->sectorSize > expected_sectors)
   { gint64 diff = image->sectorSize - expected_sectors;
     guint64 expected_image_size = 2048*(expected_sectors-1)+eh->inLast;
     char *trans = _("The image file is %lld sectors longer as noted in the\n"
		     "ecc data. This might simply be zero padding, but could\n"
		     "also mean that the image was manipulated after appending\n"
		     "the error correction information.\n\n%s");

     if(diff>0 && diff<=2)
     {  int answer = ModalWarning(GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL, NULL,
				  _("Image file is %lld sectors longer than expected.\n"
				    "Assuming this is a TAO mode medium.\n"
				    "%lld sectors will be removed from the image end.\n"),
				  diff, diff);

        if(!answer)
        {  SwitchAndSetFootline(fc->wl->fixNotebook, 1,
				fc->wl->fixFootline,
				_("<span %s>Aborted by user request!</span>"),
				Closure->redMarkup); 
	   fc->earlyTermination = FALSE;  /* suppress respective error message */
	   goto terminate;
	}

        image->sectorSize -= diff;

        if(!LargeTruncate(image->file, (gint64)expected_image_size))
	  Stop(_("Could not truncate %s: %s\n"),Closure->imageName,strerror(errno));
     }
     
     if(diff>2 && Closure->guiMode)
     {  int answer = ModalDialog(GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL, NULL,
				 trans,
				 diff, 
				 _("Is it okay to remove the superfluous sectors?"));

       if(!answer)
       {  SwitchAndSetFootline(fc->wl->fixNotebook, 1,
			       fc->wl->fixFootline,
			       _("<span %s>Aborted by user request!</span>"),
			       Closure->redMarkup); 
	  fc->earlyTermination = FALSE;  /* suppress respective error message */
	  goto terminate;
       }

       image->sectorSize -= diff;

       if(!LargeTruncate(image->file, (gint64)expected_image_size))
	 Stop(_("Could not truncate %s: %s\n"),Closure->imageName,strerror(errno));

       PrintLog(_("Image has been truncated by %lld sectors.\n"), diff);
     }

     if(diff>2 && !Closure->guiMode)
     {  if(!Closure->truncate)
	   Stop(trans, 
		diff,
		_("Add the --truncate option to the program call\n"
		  "to have the superfluous sectors removed."));

         image->sectorSize -= diff;

	 if(!LargeTruncate(image->file, (gint64)expected_image_size))
	   Stop(_("Could not truncate %s: %s\n"),Closure->imageName,strerror(errno));

	 PrintLog(_("Image has been truncated by %lld sectors.\n"), diff);
     }
   }

   /*** Prepare buffers for ecc code processing.
	The first lay->dataSectors+lay->crcSectors are protected by ecc information.
	The medium is logically divided into ndata layers and nroots slices.
	Taking one sector from each layer and slice produces on ecc block
	on which the error correction is carried out. 
	There is a total of lay->sectorsPerLayer ecc blocks.
	A portion of cache_size sectors is read ahead from each layer,
	giving a total cache size of 255*cache_size. */

   cache_size = 2*Closure->cacheMiB;  /* ndata+nroots=255 medium sectors are approx. 0.5MiB */

   for(i=0; i<255; i++)
      fc->imgBlock[i] = g_malloc(cache_size*2048);

   /*** Setup the block counters for mapping medium sectors to ecc blocks.
	We begin at the first ecc block (0) */

   for(s=0, i=0; i<lay->ndata; s+=lay->sectorsPerLayer, i++)
     block_idx[i] = s;

   cache_sector = cache_size;  /* forces instant reload of imgBlock cache */
   cache_offset = 2048*cache_sector;

   /*** CRC sums for the first ecc block are stored in the last CRC sector.
	Error handling is done later when this sector is actually used. */

   RS03ReadSectors(image, lay, 
		   (unsigned char*)last_crc_sector2, 
		   lay->ndata-1, lay->sectorsPerLayer-1, 1, RS03_READ_CRC);

   /*** Test ecc blocks and attempt error correction */

   last_percent = -1;

   for(s=0; s<lay->sectorsPerLayer; s++)
   { int bi;

     /* See if user hit the Stop button */

     if(Closure->stopActions) 
     {   if(Closure->stopActions == STOP_CURRENT_ACTION) /* suppress memleak warning when closing window */
	   SwitchAndSetFootline(fc->wl->fixNotebook, 1,
				fc->wl->fixFootline,
				_("<span %s>Aborted by user request!</span>"),
				Closure->redMarkup); 
         fc->earlyTermination = FALSE;  /* suppress respective error message */
	 goto terminate;
     }

     /* Fill cache with the next batch of cache_size ecc blocks. */

     if(cache_sector >= cache_size)
     {  
        if(lay->sectorsPerLayer-s < cache_size)
           cache_size = lay->sectorsPerLayer-s;

	/* Read the data portion */

        for(i=0; i<ndata-1; i++)
        {  
	   RS03ReadSectors(image, lay, fc->imgBlock[i], i, s, 
			   cache_size, RS03_READ_DATA);
	}

	/* Read from the CRC layer */

	RS03ReadSectors(image, lay, fc->imgBlock[ndata-1], ndata-1, s,
			cache_size, RS03_READ_CRC);

	/* Keep a copy of the last CRC sector for the next pass */
	memcpy(last_crc_sector1, last_crc_sector2, 2048);
	memcpy(last_crc_sector2, fc->imgBlock[ndata-1]+2048*(cache_size-1), 2048);

	/* and finally the ecc portion */

        for(i=0; i<nroots; i++)
        {  
	   RS03ReadSectors(image, lay, fc->imgBlock[i+ndata], i+ndata, s,
			   cache_size, RS03_READ_ECC);
	}

        cache_sector = cache_offset = 0;
     }

     /* Set crc ptr to beginning of CRC sector. The first ECC block has no
	CRC sector; the checksums are taken from the Ecc header instead. */

     if(cache_sector==0) 
     {  int err;

        crc_buf = last_crc_sector1;
	err = CheckForMissingSector((unsigned char*)crc_buf, 
				    lay->firstCrcPos,
				    eh->mediumFP, eh->fpSector);
	crc_valid = (err == SECTOR_PRESENT);
     }
     else
     {  int err;

	crc_buf   = (guint32*)(fc->imgBlock[ndata-1]+cache_offset-2048);

	err = CheckForMissingSector((unsigned char*)crc_buf, 
				    block_idx[ndata-1],
				    eh->mediumFP, eh->fpSector);
	crc_valid = (err == SECTOR_PRESENT);
     }
     crc_idx = 0;

     /*** Look for erasures based on the "dead sector" marker and CRC sums */

     erasure_count = error_count = 0;

     /* Check the data sectors */

     for(i=0; i<lay->ndata; i++)  
     {  int err = CheckForMissingSector(fc->imgBlock[i]+cache_offset, block_idx[i],
					eh->mediumFP, eh->fpSector);
       /* FIXME: sector number is wrong for CRC layer in ecc files */
       /* FIXME: Auto-replace the padding sectors */

        if(err == SECTOR_PRESENT)
	{  erasure_map[i] = 0;
	}
	else
	{  erasure_map[i] = 1;
	   erasure_list[erasure_count++] = i;
	   damaged_sectors++;
	}

	if(i < ndata-1)     /* only data sectors have CRCs */
	{  guint32 crc = Crc32(fc->imgBlock[i]+cache_offset, 2048);

	   if(crc_valid && !erasure_map[i] && crc != crc_buf[crc_idx])
	   {  erasure_map[i] = 3;
	      erasure_list[erasure_count++] = i;
	      PrintCLI(_("CRC error in sector %lld\n"),block_idx[i]);
	      damaged_sectors++;
	      crc_errors++;
	   }

	   data_count++;
	   crc_idx++;
	}
	else crc_count++;
     }

     /* Check the ecc sectors */

     for(i=lay->ndata; i<GF_FIELDMAX; i++)
     {  int err = CheckForMissingSector(fc->imgBlock[i]+cache_offset,
					RS03SectorIndex(lay, i, s),
					eh->mediumFP, eh->fpSector);

	if(err)
	{  erasure_map[i] = 1;
	   erasure_list[erasure_count++] = i;
	   damaged_sectors++;
	}
        else erasure_map[i] = 0;

	ecc_count++;
     }

     /* Trivially reject uncorrectable ecc block */

     if(erasure_count>lay->nroots)   /* uncorrectable */
     {  if(!Closure->guiMode)
	{  int sep_printed = 0;

           PrintCLI(_("* Ecc block %lld: %3d unrepairable sectors: "), s, erasure_count);

	   for(i=0; i<erasure_count; i++)
	   {  /* sector counting wraps to 0 for ecc files after the data layer */
              if(eh->methodFlags[0] & MFLAG_ECC_FILE && erasure_list[i] >= ndata-1 && ! sep_printed)
	      {  PrintCLI("; ecc file: ");
                 sep_printed = 1;
              }
              PrintCLI("%lld ", RS03SectorIndex(lay, erasure_list[i], s));
	   }
	   PrintCLI("\n");
     }

	uncorrected += erasure_count;
	goto skip;
     }

     /* Build ecc block and attempt to correct it */

     for(bi=0; bi<2048; bi++)  /* Run through each ecc block byte */
     {  int offset = cache_offset+bi;
        int r, deg_lambda, el, deg_omega;
	int u,q,tmp,num1,num2,den,discr_r;
	int lambda[nroots+1], syn[nroots]; /* Err+Eras Locator poly * and syndrome poly */
	int b[nroots+1], t[nroots+1], omega[nroots+1];
	int root[nroots], reg[nroots+1], loc[nroots];
	int syn_error, count;
	int k;

	/* Form the syndromes; i.e., evaluate data(x) at roots of g(x) */

	for(i=0; i<nroots; i++)
	  syn[i] = fc->imgBlock[0][offset];

	for(j=1; j<GF_FIELDMAX; j++)
	{  int data = fc->imgBlock[j][offset];

	   for(i=0;i<nroots;i++)
	   {  if(syn[i] == 0) syn[i] = data;
	      else syn[i] = data ^ gf_alpha_to[mod_fieldmax(gf_index_of[syn[i]] + (RS_FIRST_ROOT+i)*RS_PRIM_ELEM)];
	   }
	}

	/* Convert syndromes to index form, check for nonzero condition */

	syn_error = 0;
	for(i=0; i<nroots; i++)
	{  syn_error |= syn[i];
	   syn[i] = gf_index_of[syn[i]];
	}

	/* If it is already correct by coincidence, we have nothing to do any further */

	if(syn_error) damaged_eccblocks++; 
	else continue;

	//printf("Syndrome error for ecc block %lld, byte %d\n",s,bi);

	/* If we have found any erasures, 
	   initialize lambda to be the erasure locator polynomial */

	memset(lambda+1, 0, nroots*sizeof(lambda[0]));
	lambda[0] = 1;

	if(erasure_count > 0)
	{  lambda[1] = gf_alpha_to[mod_fieldmax(RS_PRIM_ELEM*(GF_FIELDMAX-1-erasure_list[0]))];
	   for(i=1; i<erasure_count; i++) 
	   {  u = mod_fieldmax(RS_PRIM_ELEM*(GF_FIELDMAX-1-erasure_list[i]));
	      for(j=i+1; j>0; j--) 
	      {  tmp = gf_index_of[lambda[j-1]];
	         if(tmp != GF_ALPHA0)
		   lambda[j] ^= gf_alpha_to[mod_fieldmax(u + tmp)];
	      }
	   }
	}	

	for(i=0; i<nroots+1; i++)
	  b[i] = gf_index_of[lambda[i]];
  
	/* Begin Berlekamp-Massey algorithm to determine error+erasure locator polynomial */

	r = erasure_count;   /* r is the step number */
	el = erasure_count;
	while(++r <= nroots) /* Compute discrepancy at the r-th step in poly-form */
	{  
	  discr_r = 0;
	  for(i=0; i<r; i++)
	    if((lambda[i] != 0) && (syn[r-i-1] != GF_ALPHA0))
	      discr_r ^= gf_alpha_to[mod_fieldmax(gf_index_of[lambda[i]] + syn[r-i-1])];

	  discr_r = gf_index_of[discr_r];	/* Index form */

	  if(discr_r == GF_ALPHA0) 
	  {  /* B(x) = x*B(x) */
	    memmove(b+1, b, nroots*sizeof(b[0]));
	    b[0] = GF_ALPHA0;
	  } 
	  else 
	  {  /* T(x) = lambda(x) - discr_r*x*b(x) */
	     t[0] = lambda[0];
	     for(i=0; i<nroots; i++) 
	     {  if(b[i] != GF_ALPHA0)
		     t[i+1] = lambda[i+1] ^ gf_alpha_to[mod_fieldmax(discr_r + b[i])];
	        else t[i+1] = lambda[i+1];
	     }

	     if(2*el <= r+erasure_count-1) 
	     {  el = r + erasure_count - el;

	        /* B(x) <-- inv(discr_r) * lambda(x) */
	        for(i=0; i<=nroots; i++)
		  b[i] = (lambda[i] == 0) ? GF_ALPHA0 : mod_fieldmax(gf_index_of[lambda[i]] - discr_r + GF_FIELDMAX);
	     } 
	     else 
	     {  /* 2 lines below: B(x) <-- x*B(x) */
	        memmove(b+1, b, nroots*sizeof(b[0]));
		b[0] = GF_ALPHA0;
	     }

	     memcpy(lambda,t,(nroots+1)*sizeof(t[0]));
	  }
	}

	/* Convert lambda to index form and compute deg(lambda(x)) */
	deg_lambda = 0;
	for(i=0; i<nroots+1; i++)
	{  lambda[i] = gf_index_of[lambda[i]];
	   if(lambda[i] != GF_ALPHA0)
	     deg_lambda = i;
	}

	/* Find roots of the error+erasure locator polynomial by Chien search */
	memcpy(reg+1, lambda+1, nroots*sizeof(reg[0]));
	count = 0;		/* Number of roots of lambda(x) */

	for(i=1, k=RS_PRIMTH_ROOT-1; i<=GF_FIELDMAX; i++, k=mod_fieldmax(k+RS_PRIMTH_ROOT))
	{  q=1; /* lambda[0] is always 0 */

	   for(j=deg_lambda; j>0; j--)
	   {  if(reg[j] != GF_ALPHA0) 
	      {  reg[j] = mod_fieldmax(reg[j] + j);
		 q ^= gf_alpha_to[reg[j]];
	      }
	   }

	   if(q != 0) continue; /* Not a root */

	   /* store root (index-form) and error location number */

	   root[count] = i;
	   loc[count] = k;

	   /* If we've already found max possible roots, abort the search to save time */

	   if(++count == deg_lambda) break;
	}

	/* deg(lambda) unequal to number of roots => uncorrectable error detected */

	if(deg_lambda != count)
	{  int sep_printed = 0;
           PrintLog("Decoder problem (%d != %d) for %d sectors: ", deg_lambda, count, erasure_count);

	   for(i=0; i<erasure_count; i++)
	   {  /* sector counting wraps to 0 for ecc files after the data layer */
              if(eh->methodFlags[0] & MFLAG_ECC_FILE && erasure_list[i] >= ndata-1 && ! sep_printed)
	      {  PrintCLI(_("; ecc file: "));
                 sep_printed = 1;
              }
              PrintCLI("%lld ", RS03SectorIndex(lay, erasure_list[i], s));
	   }
	   PrintCLI("\n");
	   uncorrected += erasure_count;
	   goto skip;
	}

	/* Compute err+eras evaluator poly omega(x) = syn(x)*lambda(x) 
	   (modulo x**nroots). in index form. Also find deg(omega). */

	deg_omega = deg_lambda-1;

	for(i=0; i<=deg_omega; i++)
	{  tmp = 0;
	   for(j=i; j>=0; j--)
	   {  if((syn[i - j] != GF_ALPHA0) && (lambda[j] != GF_ALPHA0))
	        tmp ^= gf_alpha_to[mod_fieldmax(syn[i - j] + lambda[j])];
	   }

	   omega[i] = gf_index_of[tmp];
	}

	/* Compute error values in poly-form. 
	   num1 = omega(inv(X(l))), 
	   num2 = inv(X(l))**(FIRST_ROOT-1) and 
	   den  = lambda_pr(inv(X(l))) all in poly-form. */

	for(j=count-1; j>=0; j--)
	{  num1 = 0;

	   for(i=deg_omega; i>=0; i--) 
	   {  if(omega[i] != GF_ALPHA0)
	         num1 ^= gf_alpha_to[mod_fieldmax(omega[i] + i * root[j])];
	   }

	   num2 = gf_alpha_to[mod_fieldmax(root[j] * (RS_FIRST_ROOT - 1) + GF_FIELDMAX)];
	   den = 0;
    
	   /* lambda[i+1] for i even is the formal derivative lambda_pr of lambda[i] */

	   for(i=MIN(deg_lambda, nroots-1) & ~1; i>=0; i-=2) 
	   {  if(lambda[i+1] != GF_ALPHA0)
	        den ^= gf_alpha_to[mod_fieldmax(lambda[i+1] + i * root[j])];
	   }

	   /* Apply error to data */

	   if(num1 != 0)
	   {  int location = loc[j];

	      if(erasure_map[location] != 1)  /* erasure came from CRC error */
	      {  int old = fc->imgBlock[location][offset];
		 int new = old ^ gf_alpha_to[mod_fieldmax(gf_index_of[num1] + gf_index_of[num2] + GF_FIELDMAX - gf_index_of[den])];
		 char *msg, *type;
		 gint64 sector;

		 if(erasure_map[location] == 3)  /* erasure came from CRC error */
		 {  msg = _("-> CRC-predicted error in sector %lld%s at byte %4d (value %02x '%c', expected %02x '%c')\n");
		 }
		 else
		 {  msg = _("-> Non-predicted error in sector %lld%s at byte %4d (value %02x '%c', expected %02x '%c')\n");
		    if(erasure_map[location] == 0) /* remember error location */
		    {  erasure_map[location] = 7;
		       error_count++;  
		    }
		 }

		 sector = RS03SectorIndex(lay, location, s);
		 if(eh->methodFlags[0] & MFLAG_ECC_FILE && location >= ndata-1)
		   type="(ecc)";
		 else
		   type="";
		 
		 PrintCLI(msg,
	                  sector, type, bi, 
			  old, isprint(old) ? old : '.',
			  new, isprint(new) ? new : '.');
	      }

	      fc->imgBlock[location][offset] ^= gf_alpha_to[mod_fieldmax(gf_index_of[num1] + gf_index_of[num2] + GF_FIELDMAX - gf_index_of[den])];
	   }
	}
     }

     /* Write corrected sectors back to disc
        and report them */

     erasure_count += error_count;  /* total errors encountered */

     if(erasure_count)
     {  int sep_printed = 0;
        PrintCLI(_("  %3d repaired sectors: "), erasure_count);

        for(i=0; i<255; i++)
        {  gint64 sec;
           char type='?';
	   int length,n;
	   
	   if(!erasure_map[i]) continue;

	   switch(erasure_map[i])
	   {  case 1:  /* dead sector */
	        type = 'd';
	        break;

	      case 3:  /* crc error */
	        type = 'c';
	        break;

	      case 7:  /* other (new) error */
		type = 'n';
		damaged_sectors++;
	        break;
	   }

	   sec = RS03SectorIndex(lay, i, s);
	   if(i < ndata) {  data_corr++;  }
	   else          {  ecc_corr++;   }
	   corrected++;

	   if(eh->methodFlags[0] & MFLAG_ECC_FILE && i >= ndata-1 && ! sep_printed)
	   {  PrintCLI(_("; ecc file: "));
              sep_printed = 1;
           }
	   PrintCLI("%lld%c ", sec, type);

	   /* Write the recovered sector */

	   if(sec != lay->dataSectors-1) length = 2048;
	   else length = eh->inLast;  /* non-image file may be clipped */

	   /* Write back into the image */

	   if(   lay->target == ECC_IMAGE 
	      || i < ndata-1)
	   {
	      if(!LargeSeek(image->file, (gint64)(2048*sec)))
		 Stop(_("Failed seeking to sector %lld in image [%s]: %s"),
		      sec, "FW", strerror(errno));

	      n = LargeWrite(image->file, cache_offset+fc->imgBlock[i], length);
	      if(n != length)
		 Stop(_("could not write medium sector %lld:\n%s"), sec, strerror(errno));
	   }

	   /* Write back into the error correction file
	      (for the CRC and ECC portion of the ecc block).
	      Note that "sec" contains the virtual adresses as
	      if we were processing an augmented image. */

	   if(lay->target == ECC_FILE && i >= ndata-1)
	   {  
              if(!LargeSeek(image->eccFile, (gint64)(2048*sec)))
		 Stop(_("Failed seeking to sector %lld in ecc file [%s]: %s"),
		      sec, "FW", strerror(errno));

		 n = LargeWrite(image->eccFile, cache_offset+fc->imgBlock[i], 2048);
		 if(n != 2048)
		    Stop(_("could not write ecc file sector %lld:\n%s"),
			 sec, strerror(errno));
	   }
	}
	PrintCLI("\n");
     }

skip:
     /* Collect some damage statistics */
     
     if(erasure_count)
       damaged_eccsecs++;

     if(erasure_count>worst_ecc)
       worst_ecc = erasure_count;

     if(erasure_count>local_plot_max)
       local_plot_max = erasure_count;

     /* Advance the cache pointers */

     cache_sector++;
     cache_offset += 2048;

     /* Report progress */

     percent = (1000*s)/lay->sectorsPerLayer;

     if(last_percent != percent) 
     {  if(Closure->guiMode)
	{  
	   RS03AddFixValues(wl, percent, local_plot_max);
	   local_plot_max = 0;

	   //if(last_corrected != corrected || last_uncorrected != uncorrected) 
	   RS03UpdateFixResults(wl, corrected, uncorrected);
	}
        else PrintProgress(_("Ecc progress: %3d.%1d%%"),percent/10,percent%10);
        last_percent = percent;
     }

     /* Increment the block indices */

     for(i=0; i<lay->ndata; i++)
	block_idx[i]++;
   }

   /*** Print results */

   PrintProgress(_("Ecc progress: 100.0%%\n"));

   if(corrected > 0) PrintLog(_("Repaired sectors: %lld (%lld data, %lld ecc)\n"),
			      corrected, data_corr, ecc_corr);
   if(uncorrected > 0) 
   {  PrintLog(_("Unrepaired sectors: %lld\n"), uncorrected);      
      if(Closure->guiMode)
        SwitchAndSetFootline(wl->fixNotebook, 1, wl->fixFootline,
			     _("Image sectors could not be fully restored "
			       "(%lld repaired; <span %s>%lld unrepaired</span>)"),
			     corrected, Closure->redMarkup, uncorrected);
      exitCode = 2;
   }
   else
   {  if(!corrected)
      {    t=_("Good! All sectors are already present.");
           PrintLog("%s\n", t);
	   exitCode = 0;
      }
      else 
      {    t=_("Good! All sectors are repaired.");
	   PrintLog("%s\n", t);
	   exitCode = 1;
      }
   }
   if(corrected > 0 || uncorrected > 0)
     PrintLog(_("Erasure counts per ecc block:  avg =  %.1f; worst = %d.\n"),
	     (double)damaged_sectors/(double)damaged_eccsecs,worst_ecc);

   if(Closure->guiMode && t)
     SwitchAndSetFootline(wl->fixNotebook, 1, wl->fixFootline,
			  "%s %s", _("Repair results:"), t);

   Verbose("\nSummary of processed sectors:\n");
   Verbose("%lld damaged sectors\n", damaged_sectors);
   Verbose("%lld CRC errors\n", crc_errors);
   Verbose("%lld of %lld ecc blocks damaged (%lld / %lld sectors)\n",
	   damaged_eccblocks, 2048*lay->sectorsPerLayer,
	   damaged_eccsecs, lay->sectorsPerLayer);
   if(data_count != (ndata-1)*lay->sectorsPerLayer)
        g_printf("ONLY %lld of %lld data sectors processed\n", 
		 (long long int)data_count, (long long int)(ndata-1)*lay->sectorsPerLayer);
   else Verbose("all data sectors processed\n");

   if(crc_count != lay->sectorsPerLayer)
        g_printf("%lld of %lld crc sectors processed\n", 
		 (long long int)crc_count, (long long int)lay->sectorsPerLayer);
   else Verbose("all  crc sectors processed\n");

   if(ecc_count != nroots*lay->sectorsPerLayer)
        g_printf("%lld of %lld ecc sectors processed\n", 
		 (long long int)ecc_count, (long long int)nroots*lay->sectorsPerLayer);
   else Verbose("all  ecc sectors processed\n");

   /*** Clean up */

   fc->earlyTermination = FALSE;

terminate:
   fix_cleanup((gpointer)fc);
}
