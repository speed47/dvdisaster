/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2004-2015 Carsten Gnoerlich.
 *
 *  The Reed-Solomon error correction draws a lot of inspiration - and even code -
 *  from Phil Karn's excellent Reed-Solomon library: http://www.ka9q.net/code/fec/
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

#include "rs01-includes.h"
#include "galois-inlines.h"

/*
 * Read crc values from the .ecc file.
 */

static void read_crc(LargeFile *ecc, guint32 *buf, int first_sector, int n_sectors)
{  int n;
  
   if(!LargeSeek(ecc, (gint64)(sizeof(EccHeader) + first_sector*sizeof(guint32))))
     Stop(_("Failed seeking in crc area: %s"), strerror(errno));

   n = LargeRead(ecc, buf, sizeof(guint32)*n_sectors);
	
   if(n != sizeof(guint32)*n_sectors)
     Stop(_("problem reading crc data: %s"),strerror(errno));
}

/***
 *** Fix the medium sectors.
 ***
 */

/*
 * Local data package used during fixing
 */

typedef struct
{  RS01Widgets *wl;
   GaloisTables *gt;
   ReedSolomonTables *rt;
   Image *image;
   int earlyTermination;
   char *msg;
   unsigned char *imgBlock[256];
   guint32 *crcBuf[256];
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

   for(i=0; i<256; i++)
   {  if(fc->imgBlock[i])
         g_free(fc->imgBlock[i]);
      if(fc->crcBuf[i])
	g_free(fc->crcBuf[i]);
   }

   if(fc->gt) FreeGaloisTables(fc->gt);
   if(fc->rt) FreeReedSolomonTables(fc->rt);
 
   g_free(fc);

   if(Closure->guiMode)
      g_thread_exit(0);
}

/*
 * Try to repair the image 
 */

void RS01Fix(Image *image)
{  Method *method = FindMethod("RS01");
   RS01Widgets *wl = (RS01Widgets*)method->widgetList;
   GaloisTables *gt;
   ReedSolomonTables *rt;
   fix_closure *fc = g_malloc0(sizeof(fix_closure)); 
   EccHeader *eh = NULL;
   unsigned char parity[256];
   int erasure_count,erasure_list[256],erasure_map[256];
   gint64 block_idx[256];
   gint64 s,si;
   int i,j,k,n;
   gint64 corrected, uncorrected;
   gint64 parity_block = 0;
   guint64 expected_image_size;
   int worst_ecc,damaged_ecc,damaged_sec,percent,last_percent = -1;
   int cache_size,cache_sector,cache_offset = 0;
   int local_plot_max;
   char *t = NULL;
   gint32 nroots;         /* These are copied to increase performance. */
   gint32 ndata;
   gint32 *gf_index_of;
   gint32 *gf_alpha_to;

   /*** Register the cleanup procedure for GUI mode */

   fc->image = image;
   fc->wl = wl;
   fc->earlyTermination = TRUE;
   RegisterCleanup(_("Repairing of image aborted"), fix_cleanup, fc);

   eh = image->eccFileHeader;

   /*** Announce what we are going to do. */

   fc->msg = g_strdup_printf(_("Error correction file using Method RS01, %d roots, %4.1f%% redundancy."),
			     eh->eccBytes, 
			     ((double)eh->eccBytes*100.0)/(double)eh->dataBytes);

   if(Closure->guiMode)
   {  SetLabelText(GTK_LABEL(wl->fixHeadline),
		  _("<big>Repairing the image.</big>\n<i>%s</i>"),fc->msg);
      RS01SetFixMaxValues(wl, eh->dataBytes, eh->eccBytes, image->sectorSize);
   }    

   PrintLog(_("\nFix mode(%s): Repairable sectors will be fixed in the image.\n"),
	    "RS01");

   /*** Do some trivial comparisons between the .ecc file and the image file */

   if(!eh->inLast)       /* field is unused/zero in versions prior to 0.66 */
     eh->inLast = 2048;

   expected_image_size = 2048*(image->expectedSectors-1)+eh->inLast;

   /* Special case: If the iso file is a few bytes too short
      or too long, and the last bytes are zeroes, the
      codec won't discover the mismatch from the CRC sum.
      Fill up the missing bytes with zeroes here; this
      will either be correct or picked up by the CRC 
      compare later. */
   
   if(image->sectorSize == image->expectedSectors
      && image->inLast < eh->inLast)
   {  int padding = eh->inLast - image->inLast;
      unsigned char buf[padding];
      int n;

      memset(buf, 0, padding);
      LargeSeek(image->file, image->file->size);
      n = LargeWrite(image->file, buf, padding);
      image->file->size += n;
      image->inLast += n;
      if(n != padding)
	 Stop(_("Failed writing to sector %lld in image [%s]: %s"),
	      image->sectorSize, "SC", strerror(errno));
   }

   if(image->file->size > expected_image_size)
   { gint64 diff = image->sectorSize - image->expectedSectors;
     char *trans = _("The image file is %lld sectors longer as noted in the\n"
		     "ecc file. This might simply be zero padding, especially\n"
		     "on dual layer DVD media, but could also mean that\n"
		     "the image and ecc files do not belong together.\n\n%s");

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
	image->inLast = eh->inLast;

        if(!LargeTruncate(image->file, expected_image_size))
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
       image->inLast = eh->inLast;

       if(!LargeTruncate(image->file, expected_image_size))
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
	 image->inLast = eh->inLast;

	 if(!LargeTruncate(image->file, expected_image_size))
	   Stop(_("Could not truncate %s: %s\n"),Closure->imageName,strerror(errno));

	 PrintLog(_("Image has been truncated by %lld sectors.\n"), diff);
     }
   }

   if(image->sectorSize == image->expectedSectors && image->inLast > eh->inLast)
   {  int difference = image->inLast - eh->inLast;

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

   if(image->sectorSize < image->expectedSectors)
   {  int answer;

      answer = ModalWarning(GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL, NULL,
			    _("Image file appears to be truncated.\n"
			      "Consider completing it with another reading pass before going on.\n"));
      if(!answer)
      {  SwitchAndSetFootline(fc->wl->fixNotebook, 1,
			      fc->wl->fixFootline,
			      _("<span %s>Aborted by user request!</span>"),
			      Closure->redMarkup); 
	 fc->earlyTermination = FALSE;  /* suppress respective error message */
	 goto terminate;
      }
   }

   if(image->fpState != FP_PRESENT)
   {  int answer;

      answer = ModalWarning(GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL, NULL,
			    _("Sector %d is missing. Can not compare image and ecc fingerprints.\n"
			      "Double check that image and ecc file belong together.\n"),
			    eh->fpSector);
      if(!answer)
      {  SwitchAndSetFootline(fc->wl->fixNotebook, 1,
			      fc->wl->fixFootline,
			      _("<span %s>Aborted by user request!</span>"),
			      Closure->redMarkup); 
	 fc->earlyTermination = FALSE;  /* suppress respective error message */
	 goto terminate;
      }
   }
   else if(memcmp(image->imageFP, eh->mediumFP, 16))
          Stop(_("Fingerprints of image and ecc file do not match.\n"
	         "Image and ecc file do not belong together.\n"));

   /*** Set up the Galois field arithmetic */

   gt = fc->gt = CreateGaloisTables(RS_GENERATOR_POLY);
   rt = fc->rt = CreateReedSolomonTables(gt, RS_FIRST_ROOT, RS_PRIM_ELEM, eh->eccBytes);

   gf_index_of = gt->indexOf;
   gf_alpha_to = gt->alphaTo;

   nroots      = rt->nroots;
   ndata       = rt->ndata;

   /*** Prepare buffers for ecc code processing.
	Our ecc blocks are built from ndata medium sectors spread over the full medium size.
        We read cache_size * ndata medium sectors ahead. */

   cache_size = 2*Closure->cacheMiB;  /* ndata medium sectors are approx. 0.5MiB */

   for(i=0; i<ndata; i++)
   {  fc->imgBlock[i] = g_malloc(cache_size*2048);
      fc->crcBuf[i]   = g_malloc(sizeof(int) * cache_size);
   }

   /*** Setup the block counters for mapping medium sectors to
	ecc blocks */

   s = (image->expectedSectors+ndata-1)/ndata;

   for(si=0, i=0; i<ndata; si+=s, i++)
     block_idx[i] = si;

   cache_sector = cache_size;  /* forces instant reload of cache */

   /*** Verify ecc information for the medium image. */ 

   corrected = uncorrected = 0;
   worst_ecc = damaged_ecc = damaged_sec = local_plot_max = 0;

   for(si=0; si<s; si++)
   { 
     if(Closure->stopActions) /* User hit the Stop button */
     {   if(Closure->stopActions == STOP_CURRENT_ACTION) /* suppress memleak warning when closing window */
	   SwitchAndSetFootline(fc->wl->fixNotebook, 1,
				fc->wl->fixFootline,
				_("<span %s>Aborted by user request!</span>"),
				Closure->redMarkup); 
         fc->earlyTermination = FALSE;  /* suppress respective error message */
	 goto terminate;
     }

     /* Read the next batch of (cache_size * ndata) medium sectors
        if the cache ran empty. */

     if(cache_sector >= cache_size)
     {  
        if(s-si < cache_size)
           cache_size = s-si;
        for(i=0; i<ndata; i++)
        {  int offset = 0;
	   for(j=0; j<cache_size; j++) 
	   {  RS01ReadSector(image, fc->imgBlock[i]+offset, block_idx[i]+j);
	      offset += 2048;
	   }
	   read_crc(image->eccFile, fc->crcBuf[i], block_idx[i], cache_size);
	}
        cache_sector = cache_offset = 0;
     }

     /* Determine erasures based on the "dead sector" marker */

     erasure_count = 0;

     for(i=0; i<ndata; i++)
     {  guint32 crc = Crc32(fc->imgBlock[i]+cache_offset, 2048);

        erasure_map[i] = 0;

        if(block_idx[i] < image->expectedSectors)  /* ignore the padding sectors! */
	{  int err=CheckForMissingSector(fc->imgBlock[i]+cache_offset, block_idx[i], NULL, 0);

	   if(err != SECTOR_PRESENT)
	   {  erasure_map[i] = 1;
	      erasure_list[erasure_count++] = i;
	   }
	   else if(crc != fc->crcBuf[i][cache_sector])
	   {  erasure_map[i] = 3;
	      erasure_list[erasure_count++] = i;
	      PrintCLI(_("CRC error in sector %lld\n"),block_idx[i]);
	   }
	}
     }

     if(!erasure_count)  /* Skip completely read blocks */
     {  parity_block+=2048;
        goto skip;
     }
     else
     {  damaged_ecc++;
        damaged_sec+=erasure_count;
     }

     if(erasure_count>worst_ecc)
       worst_ecc = erasure_count;

     if(erasure_count>local_plot_max)
       local_plot_max = erasure_count;

     /* Turn the ndata medium sectors into 2048 ecc blocks
        and try to correct them. */

     if(erasure_count>nroots)   /* uncorrectable */
     {  if(!Closure->guiMode)
	{  PrintCLI(_("* %3d unrepairable sectors: "), erasure_count);

	   for(i=0; i<erasure_count; i++)
	     PrintCLI("%lld ", block_idx[erasure_list[i]]);

	   PrintCLI("\n");
	}

	uncorrected += erasure_count;
	parity_block+=2048;

	/* For truncated images, make sure we leave no "zero holes" in the image
	   by writing the sector(s) with our "dead sector" markers. */

	for(i=0; i<erasure_count; i++)
	{  gint64 idx = block_idx[erasure_list[i]];
	   unsigned char buf[2048];

	   if(idx < image->sectorSize)
	     continue;  /* It's (already) dead, Jim ;-) */

	   if(!LargeSeek(image->file, (gint64)(2048*idx)))
	     Stop(_("Failed seeking to sector %lld in image [%s]: %s"),
		  idx, "FD", strerror(errno));

	   CreateMissingSector(buf, idx, eh->mediumFP, eh->fpSector, NULL);

	   n = LargeWrite(image->file, buf, 2048);
	   if(n != 2048)
	     Stop(_("Failed writing to sector %lld in image [%s]: %s"),
		  idx, "WD", strerror(errno));
	}
     }
     else  /* try to correct them */
     {  int bi;

        for(bi=0; bi<2048; bi++)
        {  int offset = cache_offset+bi;
	   int r, deg_lambda, el, deg_omega;
	   int u,q,tmp,num1,num2,den,discr_r;
	   int lambda[nroots+1], s[nroots]; /* Err+Eras Locator poly * and syndrome poly */
	   int b[nroots+1], t[nroots+1], omega[nroots+1];
	   int root[nroots], reg[nroots+1], loc[nroots];
	   int syn_error, count;

	   /* Read the parity bytes */

	   if(!LargeSeek(image->eccFile, (gint64)(sizeof(EccHeader) + image->expectedSectors*sizeof(guint32) + nroots*parity_block)))
	     Stop(_("Failed seeking in ecc area: %s"), strerror(errno));

	   n = LargeRead(image->eccFile, parity, nroots);
	   if(n != nroots)
	     Stop(_("Can't read ecc file:\n%s"),strerror(errno));
	   parity_block++;

	   /* Form the syndromes; i.e., evaluate data(x) at roots of g(x) */

	   for(i=0; i<nroots; i++)
	     s[i] = fc->imgBlock[0][offset];

	   for(j=1; j<GF_FIELDMAX; j++)
	   {  int data = j>=ndata ? parity[j-ndata] : fc->imgBlock[j][offset];

	      for(i=0;i<nroots;i++)
              {  if(s[i] == 0) s[i] = data;
	         else s[i] = data ^ gf_alpha_to[mod_fieldmax(gf_index_of[s[i]] + (RS_FIRST_ROOT+i)*RS_PRIM_ELEM)];
	      }
	   }

	   /* Convert syndromes to index form, check for nonzero condition */

	   syn_error = 0;
	   for(i=0; i<nroots; i++)
           {  syn_error |= s[i];
	      s[i] = gf_index_of[s[i]];
	   }

	   /* If it is already correct by coincidence,
	      we have nothing to do any further */

	   if(!syn_error) continue;

	   /* NOTE: Since we already know all our erasure positions, 
	      we could do away simpler than by using the Berlekamp and Chien
	      algorithms.I've left them in this release to have a reference
	      implementation Phil's library code which can be compared
	      against later optimized versions. */

	   /* Init lambda to be the erasure locator polynomial */

	   memset(lambda+1, 0, nroots*sizeof(lambda[0]));
	   lambda[0] = 1;

	   lambda[1] = gf_alpha_to[mod_fieldmax(RS_PRIM_ELEM*(GF_FIELDMAX-1-erasure_list[0]))];
	   for(i=1; i<erasure_count; i++) 
	   {  u = mod_fieldmax(RS_PRIM_ELEM*(GF_FIELDMAX-1-erasure_list[i]));
	      for(j=i+1; j>0; j--) 
	      {  tmp = gf_index_of[lambda[j-1]];
	         if(tmp != GF_ALPHA0)
		   lambda[j] ^= gf_alpha_to[mod_fieldmax(u + tmp)];
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
	       if((lambda[i] != 0) && (s[r-i-1] != GF_ALPHA0))
		 discr_r ^= gf_alpha_to[mod_fieldmax(gf_index_of[lambda[i]] + s[r-i-1])];

	     discr_r = gf_index_of[discr_r];	/* Index form */

	     if(discr_r == GF_ALPHA0) 
	     {
	        /* B(x) = x*B(x) */
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
	      //if(++count >= deg_lambda) break;
	   }

	   /* deg(lambda) unequal to number of roots => uncorrectable error detected */

	   if(deg_lambda != count)
	   {  PrintLog("Decoder problem (%d != %d) for %d sectors: ", deg_lambda, count, erasure_count);

	      for(i=0; i<erasure_count; i++)
	      {  gint64 idx = block_idx[erasure_list[i]];
	       
                 PrintLog("%lld ", idx);
	      }
	      PrintLog("\n");
	      break;
	   }

	   /* Compute err+eras evaluator poly omega(x) = s(x)*lambda(x) 
	      (modulo x**nroots). in index form. Also find deg(omega). */

	   deg_omega = deg_lambda-1;

	   for(i=0; i<=deg_omega; i++)
	   {  tmp = 0;
	      for(j=i; j>=0; j--)
	      {  if((s[i - j] != GF_ALPHA0) && (lambda[j] != GF_ALPHA0))
		   tmp ^= gf_alpha_to[mod_fieldmax(s[i - j] + lambda[j])];
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
		
		 if(location >= 0 && location < ndata)
		 {  if(erasure_map[location] == 3)
		    {  int old = fc->imgBlock[location][offset];
		       int new = old ^ gf_alpha_to[mod_fieldmax(gf_index_of[num1] + gf_index_of[num2] + GF_FIELDMAX - gf_index_of[den])];

		       PrintCLI(_("-> Error located in sector %lld at byte %4d (value %02x '%c', expected %02x '%c')\n"),
				block_idx[location], bi, 
				old, isprint(old) ? old : '.',
				new, isprint(new) ? new : '.');
		    }

		    if(!erasure_map[location])
		      PrintLog(_("Unexpected byte error in sector %lld, byte %d\n"),
			       block_idx[location], bi);

		    fc->imgBlock[location][offset] ^= gf_alpha_to[mod_fieldmax(gf_index_of[num1] + gf_index_of[num2] + GF_FIELDMAX - gf_index_of[den])];
		 }
		 else
		   PrintLog(_("Bad error location %d; corrupted .ecc file?\n"), location);
	      }
	   }
	}
     }

     /*** Report if any sectors could be recovered.
	  Write the recovered sectors to the image file .*/

     if(erasure_count && erasure_count<=nroots)
     {  PrintCLI(_("  %3d repaired sectors: "), erasure_count);

        for(i=0; i<erasure_count; i++)
	{  gint64 idx = block_idx[erasure_list[i]];
	   int length;

	   PrintCLI("%lld ", idx);

	   /* Write the recovered sector */

	   if(!LargeSeek(image->file, (gint64)(2048*idx)))
	     Stop(_("Failed seeking to sector %lld in image [%s]: %s"),
		  idx, "FW", strerror(errno));

	   if(idx < image->expectedSectors-1) length = 2048;
	   else length = eh->inLast;

	   n = LargeWrite(image->file, cache_offset+fc->imgBlock[erasure_list[i]], length);
	   if(n != length)
	     Stop(_("could not write medium sector %lld:\n%s"),idx,strerror(errno));
	}

	PrintCLI("\n");
	corrected += erasure_count;
     }

skip:
     /* Advance the cache pointers */

     cache_sector++;
     cache_offset += 2048;

     /* Report progress */

     percent = (1000*(si+1))/s;

     if(last_percent != percent) 
     {  if(Closure->guiMode)
	{  
	   RS01AddFixValues(wl, percent, local_plot_max);
	   local_plot_max = 0;

	   RS01UpdateFixResults(wl, corrected, uncorrected);
	}
        else PrintProgress(_("Ecc progress: %3d.%1d%%"),percent/10,percent%10);
        last_percent = percent;
     }

     /* Increment the block indices */

     for(i=0; i<ndata; i++)
	block_idx[i]++;
   }

   /*** Print results */

   PrintProgress(_("Ecc progress: 100.0%%\n"));
   if(corrected > 0) PrintLog(_("Repaired sectors: %lld     \n"),corrected);
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
	     (double)damaged_sec/(double)damaged_ecc,worst_ecc);

   if(Closure->guiMode && t)
     SwitchAndSetFootline(wl->fixNotebook, 1, wl->fixFootline,
			  "%s %s", _("Repair results:"), t);


   /*** Clean up */

   fc->earlyTermination = FALSE;

terminate:
   fix_cleanup((gpointer)fc);
}
