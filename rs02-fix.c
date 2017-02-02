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

#include "rs02-includes.h"
#include "galois-inlines.h"

/***
 *** Internal housekeeping
 ***/

typedef struct
{  Image *image;
   EccHeader *eh;
   RS02Widgets *wl;
   RS02Layout *lay;
   GaloisTables *gt;
   ReedSolomonTables *rt;
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

   if(fc->image) CloseImage(fc->image);
   if(fc->msg) g_free(fc->msg);

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

static void expand_image(fix_closure *fc, gint64 new_size)
{  Image *image = fc->image;
   int last_percent, percent;
   gint64 sectors, new_sectors;

   if(!LargeSeek(image->file, image->file->size))
     Stop(_("Failed seeking to end of image: %s\n"), strerror(errno));

   last_percent = 0;
   new_sectors = new_size - image->sectorSize;
   for(sectors = 0; sectors < new_sectors; sectors++)
   {  unsigned char buf[2048];
      int n;

      CreateMissingSector(buf, image->sectorSize+sectors, 
			  fc->eh->mediumFP, FINGERPRINT_SECTOR, 
			  "RS02 fix placeholder");
      n = LargeWrite(image->file, buf, 2048);
      if(n != 2048)
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

   image->sectorSize = new_size;
   image->file->size = new_size;
}

/***
 *** Test and fix the current image.
 ***/

void RS02Fix(Image *image)
{  Method *self = FindMethod("RS02");
   RS02Widgets *wl = (RS02Widgets*)self->widgetList;
   RS02Layout *lay;
   fix_closure *fc = g_malloc0(sizeof(fix_closure)); 
   EccHeader *eh;
#ifdef HAVE_BIG_ENDIAN
   EccHeader *eh_swapped;
#endif
   gint32 *gf_index_of;
   gint32 *gf_alpha_to;
   gint64 block_idx[255];
   gint64 s;
   guint32 crc_buf[512];
   gint64 crc_sector_byte;
   int nroots,ndata;
   int crc_idx, ecc_idx;
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
   char *t=NULL;

   /*** Register the cleanup procedure for GUI mode */

   fc->image = image;
   fc->wl = wl;
   fc->earlyTermination = TRUE;
   RegisterCleanup(_("Repairing of image aborted"), fix_cleanup, fc);

   /*** Open the image file */

   if(Closure->guiMode)
     SetLabelText(GTK_LABEL(wl->fixHeadline),
		  _("<big>Repairing the image.</big>\n<i>%s</i>"),
		  _("Opening files..."));

   eh  = fc->eh  = image->eccHeader;
   lay = fc->lay = RS02LayoutFromImage(image); 

   ndata  = lay->ndata;
   nroots = lay->nroots;

   /*** Set up the Galois field arithmetic */

   fc->gt      = CreateGaloisTables(RS_GENERATOR_POLY);
   fc->rt      = CreateReedSolomonTables(fc->gt, RS_FIRST_ROOT, RS_PRIM_ELEM, nroots);
   gf_index_of = fc->gt->indexOf;
   gf_alpha_to = fc->gt->alphaTo;

   /*** Expand a truncated image with "dead sector" markers */

   expected_sectors = lay->eccSectors+lay->dataSectors;
   if(image->sectorSize < expected_sectors)
     expand_image(fc, expected_sectors);

   /*** Announce what we going to do */

   if(Closure->guiMode)
   {  char *msg = g_strdup_printf(_("Image contains error correction data: Method RS02, %d roots, %4.1f%% redundancy."),
				  eh->eccBytes, 
				  ((double)eh->eccBytes*100.0)/(double)eh->dataBytes);

      SetLabelText(GTK_LABEL(wl->fixHeadline),
		  _("<big>Repairing the image.</big>\n<i>%s</i>"), msg);
      RS02SetFixMaxValues(wl, eh->dataBytes, eh->eccBytes, expected_sectors);
      g_free(msg);
   }    

   PrintLog(_("\nFix mode(%s): Repairable sectors will be fixed in the image.\n"),
	    "RS02");

   /*** Truncate an image with trailing garbage */

   if(image->sectorSize > expected_sectors)
   { gint64 diff = image->sectorSize - expected_sectors;
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

        if(!TruncateImage(image, (gint64)(2048*expected_sectors)))
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

       if(!TruncateImage(image, (gint64)(2048*expected_sectors)))
	 Stop(_("Could not truncate %s: %s\n"),Closure->imageName,strerror(errno));

       PrintLog(_("Image has been truncated by %lld sectors.\n"), diff);
     }

     if(diff>2 && !Closure->guiMode)
     {  if(!Closure->truncate)
	   Stop(trans, 
		diff,
		_("Add the --truncate option to the program call\n"
		  "to have the superfluous sectors removed."));

	 if(!TruncateImage(image, (gint64)(2048*expected_sectors)))
	   Stop(_("Could not truncate %s: %s\n"),Closure->imageName,strerror(errno));

	 PrintLog(_("Image has been truncated by %lld sectors.\n"), diff);
     }
   }

   /*** Rewrite all headers from the one which was given us as a reference */

#ifdef HAVE_BIG_ENDIAN
   eh_swapped = g_malloc(sizeof(EccHeader));
   memcpy(eh_swapped, eh, sizeof(EccHeader));
   SwapEccHeaderBytes(eh_swapped);
   WriteRS02Headers(image->file, lay, eh_swapped);
   g_free(eh_swapped);
#else
   WriteRS02Headers(image->file, lay, eh);
#endif

   /*** Prepare buffers for ecc code processing.
	The first lay->protecedSectors are protected by ecc information.
	The medium is logically divided into ndata layers and nroots slices.
	Taking one sector from each layer and slice produces on ecc block
	on which the error correction is carried out. 
	There is a total of lay->sectorsPerLayer ecc blocks.
	A portion of cache_size sectors is read ahead from each layer/slice,
	giving a total cache size of 255*cache_size. */

   cache_size = 2*Closure->cacheMiB;  /* ndata+nroots=255 medium sectors are approx. 0.5MiB */

   for(i=0; i<255; i++)
      fc->imgBlock[i] = g_malloc(cache_size*2048);

   /*** Setup the block counters for mapping medium sectors to ecc blocks.
        Error correction begins at lay->CrcLayerIndex so that we have a chance
        of repairing the CRC information before we need it. */

   for(s=0, i=0; i<lay->ndata; s+=lay->sectorsPerLayer, i++)
     block_idx[i] = s + lay->firstCrcLayerIndex;

   ecc_idx = lay->firstCrcLayerIndex;

   cache_sector = cache_size;  /* forces instant reload of imgBlock cache */
   cache_offset = 2048*cache_sector;

   /*** CRCs for the first ecc block are taken from the ecc header.
	Preset pointers accordingly. */

   crc_sector_byte = 2048 *(lay->dataSectors + 2);  /* first sector with CRC info */
   crc_idx = 0;
   memcpy(crc_buf, (char*)eh + 2048, sizeof(guint32) * lay->ndata);

   /*** Test ecc blocks and attempt error correction */

   last_percent = -1;

   for(s=0; s<lay->sectorsPerLayer; s++)
   { gint64 si = (s + lay->firstCrcLayerIndex) % lay->sectorsPerLayer;
     int bi;

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

     /* Make sure to wrap the block_idx[] ptr properly */

     if(!si)
     {  gint64 bs;

        for(bs=0, i=0; i<lay->ndata; bs+=lay->sectorsPerLayer, i++)
	  block_idx[i] = bs;

	ecc_idx = 0;
     }

     if(s == 1) /* force CRC reload */
       crc_idx = 512;

     /* Fill cache with the next batch of cache_size ecc blocks. */

     if(cache_sector >= cache_size)
     {  
        if(lay->sectorsPerLayer-si < cache_size)
           cache_size = lay->sectorsPerLayer-si;

        for(i=0; i<ndata; i++)       /* Read data portion */
        {  int offset = 0;
	   for(j=0; j<cache_size; j++) 
	   {  RS02ReadSector(image, lay, fc->imgBlock[i]+offset, block_idx[i]+j);
	      offset += 2048;
	   }
	}

        for(i=0; i<nroots; i++)      /* and ecc portion */
        {  int offset = 0;
	   for(j=0; j<cache_size; j++) 
	   {  gint64 esi = RS02EccSectorIndex(lay, i, ecc_idx+j);

	      if(!LargeSeek(image->file, 2048*esi))
		Stop(_("Failed seeking to sector %lld in image: %s"), esi, strerror(errno));

	      if(LargeRead(image->file, fc->imgBlock[i+ndata]+offset, 2048) != 2048)
		Stop(_("Failed reading sector %lld in image: %s"), esi, strerror(errno));

	      offset += 2048;
	   }
	}

        cache_sector = cache_offset = 0;
     }

     /* Look for erasures based on the "dead sector" marker and CRC sums */

     erasure_count = error_count = 0;

     for(i=0; i<lay->ndata; i++)  /* Check the data sectors */
     {  
        erasure_map[i] = 0;
        if(block_idx[i] < lay->protectedSectors)  /* ignore the padding sectors! */
	{  int err = CheckForMissingSector(fc->imgBlock[i]+cache_offset, block_idx[i],
					   eh->mediumFP, eh->fpSector);
	   if(err != SECTOR_PRESENT)
	   {  erasure_map[i] = 1;
	      erasure_list[erasure_count++] = i;
	      damaged_sectors++;
	   }

	  if(block_idx[i] < lay->dataSectors)     /* only data sectors have CRCs */
	  {  guint32 crc = Crc32(fc->imgBlock[i]+cache_offset, 2048);
	     int err;

	     if(crc_idx >= 512)
	     {  if(!LargeSeek(image->file, crc_sector_byte))
		  Stop(_("Failed seeking in crc area: %s"), strerror(errno));
	
		if(LargeRead(image->file, crc_buf, 2048) != 2048)
		  Stop(_("problem reading crc data: %s"), strerror(errno));

		err = CheckForMissingSector((unsigned char*)crc_buf, crc_sector_byte/2048,
					    eh->mediumFP, eh->fpSector);

		crc_sector_byte += 2048;
		crc_idx = 0;
		crc_valid = (err == SECTOR_PRESENT);
	     }

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
	  else if(block_idx[i] >= lay->dataSectors + 2) crc_count++;
	}
     }

     for(i=lay->ndata; i<GF_FIELDMAX; i++)  /* Check the ecc sectors */
     {  gint64 ecc_sector = RS02EccSectorIndex(lay, i-ndata, ecc_idx);
	int err = CheckForMissingSector(fc->imgBlock[i]+cache_offset,
					ecc_sector,
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
	{  PrintCLI(_("* Ecc block %lld: %3d unrepairable sectors: "), s, erasure_count);

	   for(i=0; i<erasure_count; i++)
	   {  gint64 loc = erasure_list[i];

	      if(loc < ndata) PrintCLI("%lld ", block_idx[loc]);
	      else            PrintCLI("%lld ", RS02EccSectorIndex(lay, loc-ndata, ecc_idx));

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
	{  PrintLog("Decoder problem (%d != %d) for %d sectors: ", deg_lambda, count, erasure_count);

	   for(i=0; i<erasure_count; i++)
	   {  gint64 loc = erasure_list[i];

	     if(loc < ndata) PrintLog("%lld ", block_idx[loc]);
	     else            PrintLog("%lld ", RS02EccSectorIndex(lay, loc-ndata, ecc_idx));
	   }
	   PrintLog("\n");
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
		 char *msg;
		 gint64 sector;

		 if(erasure_map[location] == 3)  /* erasure came from CRC error */
		 {  msg = _("-> CRC-predicted error in sector %lld at byte %4d (value %02x '%c', expected %02x '%c')\n");
		 }
		 else
		 {  msg = _("-> Non-predicted error in sector %lld at byte %4d (value %02x '%c', expected %02x '%c')\n");
		    if(erasure_map[location] == 0) /* remember error location */
		    {  erasure_map[location] = 7;
		       error_count++;  
		    }
		 }

		 if(location < ndata)
		      sector = block_idx[location];
		 else sector = RS02EccSectorIndex(lay, location-ndata, ecc_idx);

		 PrintCLI(msg,
			  sector, bi, 
			  old, isprint(old) ? old : '.',
			  new, isprint(new) ? new : '.');
	      }

	      fc->imgBlock[location][offset] ^= gf_alpha_to[mod_fieldmax(gf_index_of[num1] + gf_index_of[num2] + GF_FIELDMAX - gf_index_of[den])];
	   }
	}
     }

     /* Write corrected sectors back to disk
        and report them */

     erasure_count += error_count;  /* total errors encountered */

     if(erasure_count)
     {  PrintCLI(_("  %3d repaired sectors: "), erasure_count);

        for(i=0; i<255; i++)
        {  gint64 sec;
           char type='?';
	   int n;
	   
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

	   if(i < ndata) {  data_corr++; sec = block_idx[i]; }
	   else          {  ecc_corr++;  sec = RS02EccSectorIndex(lay, i-ndata, ecc_idx); }

	   corrected++;

	   PrintCLI("%lld%c ", sec, type);

	   /* Write the recovered sector */

	   if(!LargeSeek(image->file, (gint64)(2048*sec)))
	     Stop(_("Failed seeking to sector %lld in image [%s]: %s"),
		  sec, "FW", strerror(errno));

	   /* augmented images can not have sizes not a multiple of 2048,
	      e.g. we need not to examine the ->inLast value. */
	   
	   n = LargeWrite(image->file, cache_offset+fc->imgBlock[i], 2048);
	   if(n != 2048)
	     Stop(_("could not write medium sector %lld:\n%s"), sec, strerror(errno));

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
	   RS02AddFixValues(wl, percent, local_plot_max);
	   local_plot_max = 0;

	   //if(last_corrected != corrected || last_uncorrected != uncorrected) 
	   RS02UpdateFixResults(wl, corrected, uncorrected);
	}
        else PrintProgress(_("Ecc progress: %3d.%1d%%"),percent/10,percent%10);
        last_percent = percent;
     }

     /* Increment the block indices */

     for(i=0; i<lay->ndata; i++)
	block_idx[i]++;

     ecc_idx++;
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
   }
   else
   {  if(!corrected)
      {    t=_("Good! All sectors are already present.");
           PrintLog("%s\n", t);
      }
      else 
      {    t=_("Good! All sectors are repaired.");
	   PrintLog("%s\n", t);
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
   if(data_count != lay->dataSectors)
        g_printf("ONLY %lld of %lld data sectors processed\n", 
		 (long long int)data_count, (long long int)lay->dataSectors);
   else Verbose("all data sectors processed\n");

   if(crc_count != lay->crcSectors)
        g_printf("%lld of %lld crc sectors processed\n", 
		 (long long int)crc_count, (long long int)lay->crcSectors);
   else Verbose("all  crc sectors processed\n");

   if(ecc_count != lay->rsSectors)
        g_printf("%lld of %lld ecc sectors processed\n", 
		 (long long int)ecc_count, (long long int)lay->rsSectors);
   else Verbose("all  ecc sectors processed\n");

   /*** Clean up */

   fc->earlyTermination = FALSE;

terminate:
   fix_cleanup((gpointer)fc);
}
