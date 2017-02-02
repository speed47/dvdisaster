
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

/*
 * Debugging function
 */

void DumpSector(RawBuffer *rb, char *path)
{  FILE *file;
   char *filename;
   int i;

   if(rb->samplesRead <= 0)
     return;

   filename = g_strdup_printf("%s%lld.h", path, (long long)rb->lba);

   file = portable_fopen(filename, "w");
   
   fprintf(file, 
	   "#define SAMPLES_READ %d\n"
	   "#define SAMPLE_LENGTH %d\n"
	   "#define LBA %lld\n"
	   "unsigned char cd_frame[][%d] = {\n",
	   rb->samplesRead, rb->sampleSize, 
	   (long long)rb->lba, rb->sampleSize);

   for(i=0; i<rb->samplesRead; i++)
   {  int j;

      fprintf(file, "{\n");
      for(j=0; j<rb->sampleSize; j++)
      {  fprintf(file, "%3d, ",rb->rawBuf[i][j]); 
	 if(j%16 == 15) fprintf(file, "\n");
      }
      fprintf(file, "},\n");
   }
   
   fprintf(file, "};\n");

   fclose(file);

   PrintCLI(_("Sector %lld dumped to %s\n"), rb->lba, filename);

   g_free(filename);
}

/***
 *** Create our local working context
 ***/

RawBuffer *CreateRawBuffer(int sample_length)
{  RawBuffer *rb;
   int i,j;

   rb = g_malloc0(sizeof(RawBuffer));
   rb->samplesMax = MAX(1, Closure->maxReadAttempts);
   rb->sampleSize = sample_length;

   rb->gt = CreateGaloisTables(0x11d);
   rb->rt = CreateReedSolomonTables(rb->gt, 0, 1, 10);

   rb->dataOffset = 16;  /* default for mode1 data sectors */

   rb->workBuf   = CreateAlignedBuffer(sample_length*MAX_CLUSTER_SECTORS);
   rb->zeroSector= g_malloc0(sample_length);
   rb->rawBuf    = g_malloc(Closure->maxReadAttempts * sizeof(unsigned char*));
   rb->recovered = g_malloc(sample_length);
   rb->byteState = g_malloc(sample_length);
   rb->byteCount = g_malloc(sample_length);
   rb->reference = g_malloc(sample_length);

   for(i=0; i<Closure->maxReadAttempts; i++)
   {  rb->rawBuf[i] = g_malloc(sample_length);
   }

   for(i=0; i<N_P_VECTORS; i++)
   {  rb->pParity1[i] = g_malloc(Closure->maxReadAttempts);
      rb->pParity2[i] = g_malloc(Closure->maxReadAttempts);
   }

   for(i=0; i<N_Q_VECTORS; i++)
   {  rb->qParity1[i] = g_malloc(Closure->maxReadAttempts);
      rb->qParity2[i] = g_malloc(Closure->maxReadAttempts);
   }

   rb->pLoad = g_malloc0(Closure->maxReadAttempts * sizeof(int));
   rb->qLoad = g_malloc0(Closure->maxReadAttempts * sizeof(int));

   for(i=0; i<N_P_VECTORS; i++)
      rb->pList[i]  = g_malloc(Closure->maxReadAttempts * sizeof(char*));

   for(i=0; i<N_P_VECTORS; i++)
     for(j=0; j<Closure->maxReadAttempts; j++)
       rb->pList[i][j] = g_malloc(P_VECTOR_SIZE);

   for(i=0; i<N_Q_VECTORS; i++)
      rb->qList[i]  = g_malloc(Closure->maxReadAttempts * sizeof(char*));

   for(i=0; i<N_Q_VECTORS; i++)
     for(j=0; j<Closure->maxReadAttempts; j++)
       rb->qList[i][j] = g_malloc(Q_VECTOR_SIZE);

   memset(rb->pn, 0, sizeof(rb->pn));
   memset(rb->qn, 0, sizeof(rb->qn));

   return rb;
}

void ReallocRawBuffer(RawBuffer *rb, int new_samples_max)
{  int i,j;

   if(new_samples_max <= rb->samplesMax)
      return;

   rb->rawBuf = g_realloc(rb->rawBuf, new_samples_max * sizeof(unsigned char*));

   for(i=rb->samplesMax; i<new_samples_max; i++)
   {  rb->rawBuf[i] = g_malloc(rb->sampleSize);
   }

   for(i=0; i<N_P_VECTORS; i++)
   {  rb->pParity1[i] = g_realloc(rb->pParity1[i], new_samples_max);
      rb->pParity2[i] = g_realloc(rb->pParity2[i], new_samples_max);
   }

   for(i=0; i<N_Q_VECTORS; i++)
   {  rb->qParity1[i] = g_realloc(rb->qParity1[i], new_samples_max);
      rb->qParity2[i] = g_realloc(rb->qParity2[i], new_samples_max);
   }

   rb->pLoad = g_realloc(rb->pLoad, new_samples_max * sizeof(int));
   rb->qLoad = g_realloc(rb->qLoad, new_samples_max * sizeof(int));

   for(i=0; i<N_P_VECTORS; i++)
      rb->pList[i]  = g_realloc(rb->pList[i],  new_samples_max * sizeof(char*));

   for(i=0; i<N_P_VECTORS; i++)
     for(j=rb->samplesMax; j<new_samples_max; j++)
       rb->pList[i][j] = g_malloc(P_VECTOR_SIZE);

   for(i=0; i<N_Q_VECTORS; i++)
      rb->qList[i]  = g_realloc(rb->qList[i],  new_samples_max * sizeof(char*));

   for(i=0; i<N_Q_VECTORS; i++)
     for(j=rb->samplesMax; j<new_samples_max; j++)
       rb->qList[i][j] = g_malloc(Q_VECTOR_SIZE);

   rb->samplesMax = new_samples_max;

   rb->bestP1 = rb->bestP2 = N_P_VECTORS;
   rb->bestQ1 = rb->bestQ2 = N_Q_VECTORS;
}

void ResetRawBuffer(RawBuffer *rb)
{  int i;

   rb->samplesRead = 0;

   for(i=0; i<N_P_VECTORS; i++)
     rb->pParityN[i][0] = rb->pParityN[i][1] = 0;

   for(i=0; i<N_Q_VECTORS; i++)
     rb->qParityN[i][0] = rb->qParityN[i][1] = 0;

   rb->bestFrame = 0;
   rb->bestP1 = rb->bestP2 = N_P_VECTORS;
   rb->bestQ1 = rb->bestQ2 = N_Q_VECTORS;
}

void FreeRawBuffer(RawBuffer *rb)
{  int i,j;

   FreeGaloisTables(rb->gt);
   FreeReedSolomonTables(rb->rt);

   for(i=0; i<rb->samplesMax; i++)
     g_free(rb->rawBuf[i]);

   for(i=0; i<N_P_VECTORS; i++)
   {  g_free(rb->pParity1[i]);
      g_free(rb->pParity2[i]);
   }

   for(i=0; i<N_Q_VECTORS; i++)
   {  g_free(rb->qParity1[i]);
      g_free(rb->qParity2[i]);
   }

   g_free(rb->pLoad);
   g_free(rb->qLoad);

   for(i=0; i<N_P_VECTORS; i++)
   {  for(j=0; j<rb->samplesMax; j++)
	 g_free(rb->pList[i][j]);

      g_free(rb->pList[i]);
   }

   for(i=0; i<N_Q_VECTORS; i++)
   {  for(j=0; j<rb->samplesMax; j++)
	 g_free(rb->qList[i][j]);

      g_free(rb->qList[i]);
   }

   FreeAlignedBuffer(rb->workBuf);
   g_free(rb->zeroSector);
   g_free(rb->rawBuf);
   g_free(rb->recovered);
   g_free(rb->byteState);
   g_free(rb->byteCount);
   g_free(rb->reference);
   g_free(rb);
}

/***
 *** CD MSF address calculations
 ***/

/* Convert LBA sector number into MSF format */

static void lba_to_msf(int lba, unsigned char *minute, unsigned char *second, unsigned char
*frame)
{
  *frame = lba % 75;
  lba /= 75;
  lba += 2;             /* address + 150 frames */
  *second = lba % 60;
  *minute = lba / 60;
}

int MSFtoLBA(unsigned char minute_bcd, unsigned char second_bcd, unsigned char frame_bcd)
{  int minute = (minute_bcd & 0x0f) + 10*((minute_bcd >> 4) & 0x0f);
   int second = (second_bcd & 0x0f) + 10*((second_bcd >> 4) & 0x0f);
   int frame = (frame_bcd & 0x0f) + 10*((frame_bcd >> 4) & 0x0f);

   return (int)frame + 75 * (second - 2 + 60 * minute);
}

/* Convert byte into BCD notation */

static int int_to_bcd(int value)
{
  return ((value / 10) << 4) | (value % 10);
}

/*
 * Validate the MSF field contents. 
 * Returns TRUE if the given lba matches the MSF field.
 * When sloppy==TRUE; only the frm fields must match.
 * This is because drives often return a sector which is a few numbers
 * off due to problems finding the sync header. These must be kept from
 * going into the sample set. However, differences in the min/sec fields
 * are probably just read errors in the respective bytes as the drive
 * will probably not have derailed over such a large distance.
 */

int CheckMSF(unsigned char *frame, int lba, int sloppy)
{  unsigned char min,sec,frm;

   lba_to_msf(lba, &min, &sec, &frm);
   min = int_to_bcd(min);
   sec = int_to_bcd(sec);
   frm = int_to_bcd(frm);

   if(sloppy && frame[14] == frm)
      return TRUE;

   if(   frame[12] != min
      || frame[13] != sec	
      || frame[14] != frm)
     return FALSE;

   return TRUE;
}

/*
 * Initialize Sync, MSF and data mode bytes 
 */

static unsigned char sync_pattern[12] = 
{ 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0 };

void InitializeCDFrame(unsigned char *cd_frame, int sector, 
		       int xa_mode, int reinitialize)
{  unsigned char minute, second, frame;

   if(!reinitialize)
      memset(cd_frame, 0, MAX_RAW_TRANSFER_SIZE);  /* defensive programming */

   /* 12 sync bytes 0x00, 0xff, 0xff, ..., 0xff, 0xff, 0x00 */

   memcpy(cd_frame, sync_pattern, 12);

   /* MSF of sector address, BCD encoded */

   lba_to_msf(sector, &minute, &second, &frame);
   cd_frame[12] = int_to_bcd(minute);
   cd_frame[13] = int_to_bcd(second);
   cd_frame[14] = int_to_bcd(frame);

   /* Data mode */
   
   cd_frame[15] = 0x01;

   if(!xa_mode)
      memset(&(cd_frame[2068]), 0, 8); /* data tracks have always 8 zero bytes here */
}

/*
 * Returns TRUE is at least 75% of the frame sync bytes match the sync pattern.
 */

static int check_for_sync_pattern(unsigned char *new_frame)
{  int matches = 0;
   int i;

   for(i=0; i<12; i++)
     if(new_frame[i] == sync_pattern[i])
       matches++;

   return matches >= 8;
}

/***
 *** CD level CRC calculation
 ***/

/*
 * Test raw sector against its 32bit CRC.
 * Returns TRUE if frame is good.
 */

int CheckEDC(unsigned char *cd_frame, int xa_mode)
{ unsigned int expected_crc, real_crc;

   /* XA: Get CRC from byte position 2072 */

   if(xa_mode)
   {  expected_crc =  cd_frame[2072] << 24
                    | cd_frame[2073] << 16
                    | cd_frame[2074] <<  8
                    | cd_frame[2075];
   }
   else /* Get CRC from byte position 2064 */
   {  expected_crc =  cd_frame[0x810] << 24
                    | cd_frame[0x811] << 16
                    | cd_frame[0x812] <<  8
                    | cd_frame[0x813];
   }

#ifdef HAVE_LITTLE_ENDIAN
   expected_crc = SwapBytes32(expected_crc);  /* CRC on disc is big endian */
#endif

   if(xa_mode) real_crc = EDCCrc32(cd_frame+16, 2056);
   else        real_crc = EDCCrc32(cd_frame, 2064);

   return expected_crc == real_crc;
}

/***
 *** A very simple L-EC error correction.
 ***
 * Perform just one pass over the Q and P vectors to see if everything
 * is okay respectively correct minor errors. This is pretty much the
 * same stuff the drive is supposed to do in the final L-EC stage.
 */

static int simple_lec(RawBuffer *rb, unsigned char *frame, char *msg)
{  unsigned char byte_state[rb->sampleSize];
   unsigned char p_vector[P_VECTOR_SIZE];
   unsigned char q_vector[Q_VECTOR_SIZE];
   unsigned char p_state[P_VECTOR_SIZE];
   int erasures[Q_VECTOR_SIZE], erasure_count;
   int ignore[2];
   int p_failures, q_failures;
   int p_corrected, q_corrected;
   int p,q;

   /* Setup */

   memset(byte_state, 0, rb->sampleSize);

   p_failures = q_failures = 0;
   p_corrected = q_corrected = 0;

   /* Perform Q-Parity error correction */

   for(q=0; q<N_Q_VECTORS; q++)
   {  int err;

      /* We have no erasure information for Q vectors */

     GetQVector(frame, q_vector, q);
     err = DecodePQ(rb->rt, q_vector, Q_PADDING, ignore, 0);

     /* See what we've got */

     if(err < 0)  /* Uncorrectable. Mark bytes are erasure. */
     {  q_failures++;
        FillQVector(byte_state, 1, q);
     }
     else         /* Correctable */ 
     {  if(err == 1 || err == 2) /* Store back corrected vector */ 
	{  SetQVector(frame, q_vector, q);
	   q_corrected++;
	}
     }
   }

   /* Perform P-Parity error correction */

   for(p=0; p<N_P_VECTORS; p++)
   {  int err,i;

      /* Try error correction without erasure information */

      GetPVector(frame, p_vector, p);
      err = DecodePQ(rb->rt, p_vector, P_PADDING, ignore, 0);

      /* If unsuccessful, try again using erasures.
	 Erasure information is uncertain, so try this last. */

      if(err < 0 || err > 2)
      {  GetPVector(byte_state, p_state, p);
	 erasure_count = 0;

	 for(i=0; i<P_VECTOR_SIZE; i++)
	   if(p_state[i])
	     erasures[erasure_count++] = i;

	 if(erasure_count > 0 && erasure_count <= 2)
	 {  GetPVector(frame, p_vector, p);
	    err = DecodePQ(rb->rt, p_vector, P_PADDING, erasures, erasure_count);
	 }
      }

      /* See what we've got */

      if(err < 0)  /* Uncorrectable. */
      {  p_failures++;
      }
      else         /* Correctable. */ 
      {  if(err == 1 || err == 2) /* Store back corrected vector */ 
	 {  SetPVector(frame, p_vector, p);
	    p_corrected++;
	 }
      }
   }

   /* Sum up */

   if(q_failures || p_failures || q_corrected || p_corrected)
   {
     PrintCLIorLabel(Closure->status, 
		     "Sector %lld  L-EC P/Q results: %d/%d failures, %d/%d corrected (%s).\n",
		     rb->lba, p_failures, q_failures, p_corrected, q_corrected, msg);
     return 1;
   }

   return 0;
}

/***
 *** Validate CD raw sector
 ***/

int ValidateRawSector(RawBuffer *rb, unsigned char *frame, char *msg)
{  int lec_did_sth = FALSE;
   unsigned char saved_msf[4];

   /* See if the buffer was returned unchanged. */

   if(CheckForMissingSector(frame, rb->lba, NULL, 0) != SECTOR_PRESENT)
   {  RememberSense(3, 255, 0);  /* No data returned */
      return FALSE;
   }

   /* A fully zeroed out buffer is suspicious since at least the
      sync byte sequence and address fields should not be zero.  
      This is usually a sign that the atapi/scsi driver is broken; 
      e.g. that it does not pass through data when the drive 
      signalled an error. */

   if(!memcmp(frame, rb->zeroSector, rb->sampleSize))
   {  RememberSense(3, 255, 5); /* zero sector */
      return FALSE;
   }

   /* Some operating systems are even worse - random data is returned.
      If the sync sequence is missing, reject the sector. */

   if(!check_for_sync_pattern(frame))
   {  RememberSense(3, 255, 8); /* random data */
      return FALSE;
   }

   /* Adapt for XA mode */

   if(rb->xaMode)
   {  memcpy(saved_msf, frame+12, 4);
      memset(frame+12, 0, 4);
   }

  /* Do simple L-EC.
     It seems that drives stop their internal L-EC as soon as the
     EDC is okay, so we may see uncorrected errors in the parity bytes.
     Since we are also interested in the user data only and doing the
     L-EC is expensive, we skip our L-EC as well when the EDC is fine. */

  if(!CheckEDC(frame, rb->xaMode))
     lec_did_sth = simple_lec(rb, frame, msg);


  if(rb->xaMode)
    memcpy(frame+12, saved_msf, 4);

  /* Test internal sector checksum again */

  if(!CheckEDC(frame, rb->xaMode))
  {  RememberSense(3, 255, 1);  /* EDC failure in RAW sector */
       return FALSE;
  }

  /* Test internal sector address */

  if(!CheckMSF(frame, rb->lba, STRICT_MSF_CHECK))
  {  RememberSense(3, 255, 2);  /* Wrong MSF in RAW sector */
     return FALSE;
  }

  /* Tell user that L-EC succeeded */

  if(lec_did_sth)
    PrintCLIorLabel(Closure->status, 
		    "Sector %lld: Recovered in raw reader by L-EC.\n",
		    rb->lba);

   return TRUE;
}

/***
 *** Try to recover a raw CD frame sample.
 ***/

/*
 * Customized RS decoding.
 *
 * Erasure information is uncertain,
 * so we try to correct with erasure information as well as without.
 * 
 * Returns the number of corrected errors or 3 if correction failed.
 */

static int p_decode(RawBuffer *rb, unsigned char *vector, unsigned char *state)
{ int erasures[P_VECTOR_SIZE];
  int ignore[2];
  unsigned char working_vector[P_VECTOR_SIZE];
  int err, erasure_count;

  /* Try error correction without erasure information */

  memcpy(working_vector, vector, P_VECTOR_SIZE);
  err = DecodePQ(rb->rt, working_vector, P_PADDING, ignore, 0);

  /* If unsuccessful, try again using erasures. */

  if(err < 0 || err > 2)
  {  int i;

     erasure_count = 0;

     for(i=0; i<P_VECTOR_SIZE; i++)
       if(!(state[i]&2))
	 erasures[erasure_count++] = i;

     if(erasure_count > 0 && erasure_count <= 2)
     {  memcpy(working_vector, vector, P_VECTOR_SIZE);
        err = DecodePQ(rb->rt, working_vector, P_PADDING, erasures, erasure_count);
     }
  }

  if(err == 1 || err == 2)
    memcpy(vector, working_vector, P_VECTOR_SIZE);

  return err < 0 ? 3 : err;
}

static int q_decode(RawBuffer *rb, unsigned char *vector, unsigned char *state)
{ int erasures[Q_VECTOR_SIZE];
  int ignore[2];
  unsigned char working_vector[Q_VECTOR_SIZE];
  int err, erasure_count;

  /* Try error correction without erasure information */

  memcpy(working_vector, vector, Q_VECTOR_SIZE);
  err = DecodePQ(rb->rt, working_vector, Q_PADDING, ignore, 0);

  /* If unsuccessful, try again using erasures. */

  if(err < 0 || err > 2)
  {  int i;

     erasure_count = 0;

     for(i=0; i<Q_VECTOR_SIZE-2; i++)
       if(!(state[i]&2))
	 erasures[erasure_count++] = i;

     if(erasure_count > 0 && erasure_count <= 2)
     {  memcpy(working_vector, vector, Q_VECTOR_SIZE);
        err = DecodePQ(rb->rt, working_vector, Q_PADDING, erasures, erasure_count);
     }
  }

  if(err == 1 || err == 2)
    memcpy(vector, working_vector, Q_VECTOR_SIZE);

  return err < 0 ? 3 : err;
}

/*
 * Try to correct remaining bytes in rb->recovered.
 * Iterates over P and Q vectors until no further improvements are made.
 */

//#define DEBUG_ITERATIVE

int IterativeLEC(RawBuffer *rb)
{  unsigned char p_vector[P_VECTOR_SIZE];
   unsigned char q_vector[Q_VECTOR_SIZE];
   int p_failures, q_failures;
   int p_corrected, q_corrected;
   int p,q;
   int last_p_failures = N_P_VECTORS;
   int last_q_failures = N_Q_VECTORS;
   int iteration=1;

   for(; ;) /* iterate over P- and Q-Parity until failures converge */
   {	
      p_failures = q_failures = 0;
      p_corrected = q_corrected = 0;

      /* Perform Q-Parity error correction */

      for(q=0; q<N_Q_VECTORS; q++)
      {  int err;

	 /* Try error correction */

	 GetQVector(rb->recovered, q_vector, q);
	 err = q_decode(rb, q_vector, rb->byteState);

	 /* See what we've got */

	 if(err > 2)  /* Uncorrectable. Mark bytes as erasure. */
	 {  q_failures++;
	    AndQVector(rb->byteState, ~1, q);
	 }
	 else  /* Correctable. Mark bytes as good; store back results. */ 
	 {  if(err == 1 || err == 2) /* Store back corrected vector */ 
	    {  SetQVector(rb->recovered, q_vector, q);
	       q_corrected++;
	    }
	    OrQVector(rb->byteState, 1, q);
	 }
      }

      /* Perform P-Parity error correction */

      for(p=0; p<N_P_VECTORS; p++)
      {  int err;

	 /* Try error correction */

	 GetPVector(rb->recovered, p_vector, p);
	 err = p_decode(rb, p_vector, rb->byteState);

	 /* See what we've got */

	 if(err > 2)  /* Uncorrectable. */
	 {  p_failures++;
	    AndPVector(rb->byteState, ~2, p);
	 }
	 else  /* Correctable. Mark bytes as good; store back results. */ 
	 {  if(err == 1 || err == 2) /* Store back corrected vector */ 
	    {  SetPVector(rb->recovered, p_vector, p);
	       p_corrected++;
	    }
	    OrPVector(rb->byteState, 2, p);
	 }
      }

      /* See if there was an improvement */

#ifdef DEBUG_ITERATIVE
      printf("L-EC: iteration %d\n", iteration); 
      printf("      Q-failures/corrected: %2d/%2d\n", q_failures, q_corrected);
      printf("      P-failures/corrected: %2d/%2d\n", p_failures, p_corrected);
#endif

      if(CheckEDC(rb->recovered, rb->xaMode) || p_failures + q_failures == 0)
	break;

      if(   last_p_failures > p_failures
	 || last_q_failures > q_failures)
      {  last_p_failures = p_failures;
	 last_q_failures = q_failures;
	 iteration++;
      }
      else break;
   }

   return (p_failures + q_failures == 0);
}

/***
 *** Some frame statistics are updated iteratively,
 *** e.g. whenever a new frame is accumulated.
 */

void UpdateFrameStats(RawBuffer *rb)
{  unsigned char *new_sample = rb->rawBuf[rb->samplesRead-1];
   unsigned char vector[Q_VECTOR_SIZE];
   int p_corr = 0;
   int p_err  = 0;
   int q_corr = 0;
   int q_err  = 0;
   int err,eras[2];
   int p,q;

   /* MAYBE TODO: Try trivial corrections first, e.g.
      correct P/Q with single failure until they damage
      some other vector */

   /* MAYBE TODO: add single byte failures are to the double
      failure count since we want to pick the vector
      with the least number of defective vectors. */

   for(p=0; p<N_P_VECTORS; p++)
   {  GetPVector(new_sample, vector, p);
      err = DecodePQ(rb->rt, vector, P_PADDING, eras, 0);
      switch(err)
      {  case 0: 
	    break;
	 case 1:
	    p_corr++;
	    //	    p_err++;
	    break;
	 default:
	    p_err++;
	    break;
      }
   }

   for(q=0; q<N_Q_VECTORS; q++)
   {  GetQVector(new_sample, vector, q);
      err = DecodePQ(rb->rt, vector, Q_PADDING, eras, 0);
      switch(err)
      {  case 0: 
	    break;
	 case 1:
	    q_corr++;
	    //	    q_err++;
	    break;
	 default:
	    q_err++;
	    break;
      }
   }

   if(p_err > rb->bestP2)
      return;

   if(p_err == rb->bestP2)
   {  if(p_corr > rb->bestP1)
	 return;

      if(p_corr == rb->bestP1)
      {  if(q_err > rb->bestQ2)
	    return;

	 if(q_err == rb->bestQ2 && q_corr >= rb->bestQ1)
	    return;
      }
   }

   rb->bestFrame = rb->samplesRead - 1;
   rb->bestP1 = p_corr;
   rb->bestP2 = p_err;
   rb->bestQ1 = q_corr;
   rb->bestQ2 = q_err;
}

/*** 
 *** The grand wrapper:
 ***
 * Try several strategies to analyse and recover
 * a collection of RAW frame samples.
 */

int TryCDFrameRecovery(RawBuffer *rb, unsigned char *outbuf)
{  unsigned char *new_frame = rb->workBuf->buf;

   /*** Reject unplausible sectors */

   /* See if the buffer was returned unchanged. */

   if(CheckForMissingSector(new_frame, rb->lba, NULL, 0) != SECTOR_PRESENT)
   {  RememberSense(3, 255, 0);  /* No data returned */
      return -1;
   }

   /* A fully zeroed out buffer is suspicious since at least the
      sync byte sequence and address fields should not be zero.  
      This is usually a sign that the atapi/scsi driver is broken; 
      e.g. that it does not pass through data when the drive 
      signalled an error. */

   if(!memcmp(new_frame, rb->zeroSector, rb->sampleSize))
   {  RememberSense(3, 255, 5); /* zero sector */
      return -1;
   }

   /* Some operating systems are even worse - random data is returned.
      If the sync sequence is missing, reject the sector. */

   if(!check_for_sync_pattern(new_frame))
   {  RememberSense(3,255, 8); /* random data */
      return -1;
   }

   /* Compare lba against MSF field. Some drives return sectors
      from wrong places in RAW mode. */

   if(!CheckMSF(new_frame, rb->lba, SLOPPY_MSF_CHECK))
   {  RememberSense(3, 255, 2); /* Wrong MSF in raw sector */
      return -1;
   }

   /* Okay, accept sector as a valid sample for recovery. */

   if(rb->xaMode)
     memset(new_frame+12, 0, 4); 

   memcpy(rb->rawBuf[rb->samplesRead], new_frame, rb->sampleSize);
   rb->samplesRead++;

   UpdateFrameStats(rb);

   /*** Cheap shots: See if we can recover the sector itself
	(without using the more complex heuristics and other
	sectors).
        Note that we ignore the return value of IterativeLEC().
        If e.g. some parity bytes remain uncorrected we don't care
        as long as the EDC tells us that the user data part is okay. */

   memcpy(rb->recovered, new_frame, rb->sampleSize);
   memset(rb->byteState, 0, rb->sampleSize);

   /* If the data section is unaffected by the error,
      do not investigate further. */

   if(CheckEDC(rb->recovered, rb->xaMode)
      && CheckMSF(rb->recovered, rb->lba, STRICT_MSF_CHECK))
   {
       PrintCLIorLabel(Closure->status, 
		       "Sector %lld: Good. Data section passes EDC test.\n",
		       rb->lba);
       memcpy(outbuf, rb->recovered+rb->dataOffset, 2048);
       return 0;
   }

   /* Sometimes we have only errors in the sync pattern and the P/Q vectors,
      but the data section will pass the EDC test. Try it. */

   if(memcmp(rb->recovered, sync_pattern, 12))
   {  memcpy(rb->recovered, sync_pattern, 12);

      if(CheckEDC(rb->recovered, rb->xaMode)
	 && CheckMSF(rb->recovered, rb->lba, STRICT_MSF_CHECK))
      {
	 PrintCLIorLabel(Closure->status, 
			 "Sector %lld: Recovered in raw reader after correcting sync pattern.\n",
			 rb->lba);
	 
	 memcpy(outbuf, rb->recovered+rb->dataOffset, 2048);
	 return 0;
      }
   }


   /* Try the simple iterative L-EC */

   IterativeLEC(rb);

   if(CheckEDC(rb->recovered, rb->xaMode)
      && CheckMSF(rb->recovered, rb->lba, STRICT_MSF_CHECK))
   {
       PrintCLIorLabel(Closure->status, 
		       "Sector %lld: Recovered in raw reader by iterative L-EC.\n",
		       rb->lba);

       memcpy(outbuf, rb->recovered+rb->dataOffset, 2048);
       return 0;
   }

   /*** More sophisticated heuristics */

   /* Incremental update of our data */

   UpdateByteCounts(rb);
   CalculatePQLoad(rb);
   UpdatePQParityList(rb, new_frame);

   /* The actual heuristics */

#if 0
   SmartLEC(rb);

   if(CheckEDC(rb->recovered, rb->xaMode)
      && CheckMSF(rb->recovered, rb->lba, STRICT_MSF_CHECK))
   {  PrintCLIorLabel(Closure->status, 
		      "Sector %lld: Recovered in raw reader by smart L-EC.\n",
		      rb->lba);
      memcpy(outbuf, rb->recovered+rb->dataOffset, 2048);
      return 0; 
   }
#endif

   SearchPlausibleSector(rb, 0);

   if(CheckEDC(rb->recovered, rb->xaMode)
      && CheckMSF(rb->recovered, rb->lba, STRICT_MSF_CHECK))
   {  PrintCLIorLabel(Closure->status, 
		      "Sector %lld: Recovered in raw reader by plausible sector search (0).\n",
		      rb->lba);
      memcpy(outbuf, rb->recovered+rb->dataOffset, 2048);
      return 0; 
   }

   BruteForceSearchPlausibleSector(rb);

   if(CheckEDC(rb->recovered, rb->xaMode)
      && CheckMSF(rb->recovered, rb->lba, STRICT_MSF_CHECK))
   {  PrintCLIorLabel(Closure->status, 
		      "Sector %lld: Recovered in raw reader by brute force plausible sector search (0).\n",
		      rb->lba);
      memcpy(outbuf, rb->recovered+rb->dataOffset, 2048);
      return 0; 
   }

   AckHeuristic(rb);

   if(CheckEDC(rb->recovered, rb->xaMode)
      && CheckMSF(rb->recovered, rb->lba, STRICT_MSF_CHECK))
   {  PrintCLIorLabel(Closure->status, 
		      "Sector %lld: Recovered in raw reader by mutual ack heuristic (0).\n",
		      rb->lba);
      memcpy(outbuf, rb->recovered+rb->dataOffset, 2048);
      return 0; 
   }

   HeuristicLEC(rb->recovered, rb, outbuf);

   if(CheckEDC(rb->recovered, rb->xaMode)
      && CheckMSF(rb->recovered, rb->lba, STRICT_MSF_CHECK))
   {  PrintCLIorLabel(Closure->status, 
		      "Sector %lld: Recovered in raw reader by heuristic L-EC (0).\n",
		      rb->lba);
      memcpy(outbuf, rb->recovered+rb->dataOffset, 2048);
      return 0; 
   }

   SearchPlausibleSector(rb, 1);

   if(CheckEDC(rb->recovered, rb->xaMode)
      && CheckMSF(rb->recovered, rb->lba, STRICT_MSF_CHECK))
   {  PrintCLIorLabel(Closure->status, 
		      "Sector %lld: Recovered in raw reader by plausible sector search (1).\n",
		      rb->lba);
      memcpy(outbuf, rb->recovered+rb->dataOffset, 2048);
      return 0; 
   }

   BruteForceSearchPlausibleSector(rb);

   if(CheckEDC(rb->recovered, rb->xaMode)
      && CheckMSF(rb->recovered, rb->lba, STRICT_MSF_CHECK))
   {  PrintCLIorLabel(Closure->status, 
		      "Sector %lld: Recovered in raw reader by brute force plausible sector search (1).\n",
		      rb->lba);
      memcpy(outbuf, rb->recovered+rb->dataOffset, 2048);
      return 0; 
   }

   AckHeuristic(rb);

   if(CheckEDC(rb->recovered, rb->xaMode)
      && CheckMSF(rb->recovered, rb->lba, STRICT_MSF_CHECK))
   {  PrintCLIorLabel(Closure->status, 
		      "Sector %lld: Recovered in raw reader by mutual ack heuristic (1).\n",
		      rb->lba);
      memcpy(outbuf, rb->recovered+rb->dataOffset, 2048);
      return 0; 
   }

   HeuristicLEC(rb->recovered, rb, outbuf);

   if(CheckEDC(rb->recovered, rb->xaMode)
      && CheckMSF(rb->recovered, rb->lba, STRICT_MSF_CHECK))
   {  PrintCLIorLabel(Closure->status, 
		      "Sector %lld: Recovered in raw reader by heuristic L-EC (1).\n",
		      rb->lba);
      memcpy(outbuf, rb->recovered+rb->dataOffset, 2048);
      return 0; 
   }

   /*** Recovery failed */

   RememberSense(3, 255, 6);  /* Sector accumulated for analysis */
   rb->recommendedAttempts = Closure->maxReadAttempts;
   return -1;
}
