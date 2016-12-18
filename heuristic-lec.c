/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2004-2015 Carsten Gnoerlich.
 *  Copyright (C) 2006 Andrei Grecu
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

/***
 *** Auxiliary functions for collecting some stuff incrementally
 ***/

/*
 * Update count of different bytes read for each position
 */

void UpdateByteCounts(RawBuffer *rb)
{
   int i, j;

   /* The first time there is not much to do */
   if(rb->samplesRead == 1)
   {
      memset(rb->byteCount, 1, rb->sampleSize);
      return;
   }

   /* Update byte counts */
   for(i = 0; i < rb->sampleSize; i++)
   {
      char found = 0;
      for(j = 0; j < rb->samplesRead - 1; j++)
      {
         if(rb->rawBuf[rb->samplesRead - 1][i] == rb->rawBuf[j][i]) 
         {
            found = 1;
            break;
         }
      }
      
      if(!found) rb->byteCount[i]++;
   }
}

/*
 * Determine work load of P/Q vectors in the given frame
 */

void CalculatePQLoad(RawBuffer *rb)
{  unsigned char p_vector[P_VECTOR_SIZE];
   unsigned char q_vector[Q_VECTOR_SIZE];
   int frame_idx = rb->samplesRead - 1;
   unsigned char *new_frame = rb->rawBuf[frame_idx];
   int ignore[2];
   int q, p;
   int err;
        
   for(q = 0; q < N_Q_VECTORS; q++)
   {
     GetQVector(new_frame, q_vector, q);
     err = DecodePQ(rb->rt, q_vector, Q_PADDING, ignore, 0);
     if(err <  0) rb->qLoad[frame_idx] += 2;
     if(err == 1) rb->qLoad[frame_idx]++; /* We assume without any erasures specified there can't be more than 1 errors corrected. */
   }      

   for(p = 0; p < N_P_VECTORS; p++)
   {
     GetPVector(new_frame, p_vector, p);
     err = DecodePQ(rb->rt, p_vector, P_PADDING, ignore, 0);
     if(err <  0) rb->pLoad[frame_idx] += 2;
     if(err == 1) rb->pLoad[frame_idx]++; /* We assume without any erasures specified there can't be more than 1 errors corrected. */
   }      
}   

/*
 * Collect a list of all seen P/Q parity bytes.
 */

void UpdatePQParityList(RawBuffer *rb, unsigned char *new_frame)
{  unsigned char p_vector[P_VECTOR_SIZE];
   unsigned char q_vector[Q_VECTOR_SIZE];
   int i,p,q;

   /*** See if new frame has any Q parity bytes different from the existing ones. */

   for(q=0; q<N_Q_VECTORS; q++)
   {  int qfound = FALSE;

      GetQVector(new_frame, q_vector, q);

      for(i=0; i<rb->qParityN[q][0]; i++)
        if(rb->qParity1[q][i] == q_vector[43])
        {  qfound = TRUE;
           break;
        }

      if(!qfound)
        rb->qParity1[q][rb->qParityN[q][0]++] = q_vector[43];

      qfound = FALSE;

      for(i=0; i<rb->qParityN[q][1]; i++)
        if(rb->qParity2[q][i] == q_vector[44])
        {  qfound = TRUE;
           break;
        }

      if(!qfound)
        rb->qParity2[q][rb->qParityN[q][1]++] = q_vector[44];
   }

   /*** See if new frame has any P parity bytes different from the existing ones. */

   for(p=0; p<N_P_VECTORS; p++)
   {  int pfound = FALSE;

      GetPVector(new_frame, p_vector, p);

      for(i=0; i<rb->pParityN[p][0]; i++)
        if(rb->pParity1[p][i] == p_vector[24])
        {  pfound = TRUE;
           break;
        }

      if(!pfound)
        rb->pParity1[p][rb->pParityN[p][0]++] = p_vector[24];

      pfound = FALSE;

      for(i=0; i<rb->pParityN[p][1]; i++)
        if(rb->pParity2[p][i] == p_vector[25])
        {  pfound = TRUE;
           break;
        }

      if(!pfound)
        rb->pParity2[p][rb->pParityN[p][1]++] = p_vector[25];
   }
}

/***
 *** Heuristic L-EC attempt
 *** Andrei Grecu, 2006
 */

static int eval_q_candidate(RawBuffer *rb, unsigned char *q_vector, int q, 
                            int *p_failures_out, int *p_errors_out)
{
   unsigned char     p_vector[P_VECTOR_SIZE];
   unsigned char old_q_vector[Q_VECTOR_SIZE];
   int ignore[2];
   int p, p_errors = 0;
   int p_failures = 0;
   int err;
   
   GetQVector(rb->recovered, old_q_vector, q);
   SetQVector(rb->recovered,     q_vector, q);
   
   /* Count P failures after setting our Q vector. */

   for(p = 0; p < N_P_VECTORS; p++)
   {
      GetPVector(rb->recovered, p_vector, p);
      err = DecodePQ(rb->rt, p_vector, P_PADDING, ignore, 0);
      if(err <  0) p_failures++;
      else if(err == 1) p_errors++;
   }            

   SetQVector(rb->recovered, old_q_vector, q);

   *p_failures_out = p_failures;
   *p_errors_out   = p_errors;

   return TRUE;
}

static void eval_p_candidate(RawBuffer *rb, unsigned char *p_vector, int p, 
                            int *q_failures_out, int *q_errors_out)
{
   unsigned char     q_vector[Q_VECTOR_SIZE];
   unsigned char old_p_vector[P_VECTOR_SIZE];
   int ignore[2];
   int q, q_errors = 0;
   int q_failures = 0;
   int err;
   
   GetPVector(rb->recovered, old_p_vector, p);
   SetPVector(rb->recovered,     p_vector, p);
   
   /* Count Q failures after setting our P vector. */

   for(q = 0; q < N_Q_VECTORS; q++)
   {
      GetQVector(rb->recovered, q_vector, q);
      err = DecodePQ(rb->rt, q_vector, Q_PADDING, ignore, 0);
      if(err <  0) q_failures++;
      else if(err == 1) q_errors++;
   }            

   SetPVector(rb->recovered, old_p_vector, p);

   *q_failures_out = q_failures;
   *q_errors_out = q_errors;
}


/*
 * An enhanced L-EC loop
 */   

//#define DEBUG_HEURISTIC_LEC

int HeuristicLEC(unsigned char *cd_frame, RawBuffer *rb, unsigned char *out)
{
   unsigned char p_vector[P_VECTOR_SIZE];
   unsigned char q_vector[Q_VECTOR_SIZE];
   unsigned char p_state[P_VECTOR_SIZE];
   unsigned char q_state[Q_VECTOR_SIZE];
   int erasures[Q_VECTOR_SIZE], decimated_erasures[2], erasure_count;
   int ignore[2];
   int p_failures, q_failures;
   int p_corrected, q_corrected;
   int i,p,q;
   int iteration=1;
   int p_err, q_err;
   int p_decimated, q_decimated;
   int last_p_err = N_P_VECTORS;
   int last_q_err = N_Q_VECTORS;
   int last_p_failures = N_P_VECTORS;
   int last_q_failures = N_Q_VECTORS;
   int max_p_failures = 0;
   int max_p_errors = 0;
   int max_q_failures = 0;
   int max_q_errors = 0;
   int err;

   memset(rb->byteState, FRAME_BYTE_UNKNOWN, rb->sampleSize);
   
   /* Count initial P failures */

   for(p = 0; p < N_P_VECTORS; p++)
   {
      GetPVector(rb->recovered, p_vector, p);
      err = DecodePQ(rb->rt, p_vector, P_PADDING, ignore, 0);
      if(err < 0) max_p_failures++;
      if(err == 1) max_p_errors++;
   }

   /* Count initial Q failures */

   for(q = 0; q < N_Q_VECTORS; q++)
   {
      GetQVector(rb->recovered, q_vector, q);
      err = DecodePQ(rb->rt, q_vector, Q_PADDING, ignore, 0);
      if(err < 0) max_q_failures++;
      if(err == 1) max_q_errors++;
   }   
   
#ifdef DEBUG_HEURISTIC_LEC
   Verbose("      P-failures/corrected/errors + decimated: %2d/%2d/ 0 + 0\n", max_p_failures, max_p_errors);
   Verbose("      Q-failures/corrected/errors + decimated: %2d/%2d/ 0 + 0\n", max_q_failures, max_q_errors);
#endif

   for(; ;) /* iterate over P- and Q-Parity until failures converge */
   {   
      p_failures  = q_failures = 0;
      p_corrected = q_corrected = 0;
      p_err = q_err = 0;
      p_decimated = q_decimated = 0;

      /* Perform P-Parity error correction */

      for(p=0; p<N_P_VECTORS; p++)
      {  
         /* Determine number of erasures */

         GetPVector(rb->byteState, p_state, p);
         erasure_count = 0;

         for(i=0; i<P_VECTOR_SIZE; i++)
            if(p_state[i] == FRAME_BYTE_ERROR) erasures[erasure_count++] = i;

         /* First try to see whether P is correctable without erasure markings. */

         GetPVector(rb->recovered, p_vector, p);
         err = DecodePQ(rb->rt, p_vector, P_PADDING, ignore, 0);
         
         if(err == 1) /* Store back corrected vector */ 
         {  
            SetPVector(rb->recovered, p_vector, p);
            FillPVector(rb->byteState, FRAME_BYTE_GOOD, p);
            p_corrected++;
            p_err += err;
         }
            
         /* Erasure markings are overly pessimistic as each failing vector will mark
            a full row as an erasure. We assume that there are only 2 real erasures
            and try all combinations.
            The case for 2 erasure is also included here. */

         if(err < 0 && erasure_count > 1) 
         {  unsigned char best_p[P_VECTOR_SIZE];
            int best_q_failures = N_Q_VECTORS;
            int best_q_errors = N_Q_VECTORS;
            int candidate = FALSE;
            int a, b;

            /* Try error correction with decimated erasures */
            
            for(a = 0; a < erasure_count - 1; a++)
            {
               for(b = a + 1; b < erasure_count; b++)
               {
                  decimated_erasures[0] = erasures[a];
                  decimated_erasures[1] = erasures[b];
                  GetPVector(rb->recovered, p_vector, p);
                  err = DecodePQ(rb->rt, p_vector, P_PADDING, decimated_erasures, 2);
                  if(err == 2) 
                  {  int q_err, q_fail;

                     candidate = TRUE;
                     eval_p_candidate(rb, p_vector, p, &q_fail, &q_err);

                     if(q_fail <= best_q_failures && q_err <= best_q_errors)
                     {  best_q_failures = q_fail;
                        best_q_errors   = q_err;
                        memcpy(best_p, p_vector, P_VECTOR_SIZE); 
                     }
                  }
               }
            }

            if(!candidate) err = -1; /* If P failed */
            else
            {  FillPVector(rb->byteState, FRAME_BYTE_GOOD, p);
               SetPVector(rb->recovered, best_p, p);
               
               err = 0;
               p_err += 2;
               p_corrected++;
               p_decimated++;
            }
         }

         if(err == 0) FillPVector(rb->byteState, FRAME_BYTE_GOOD, p);
         if(err < 0)  /* Uncorrectable. */
         {  
            p_failures++;
            FillPVector(rb->byteState, FRAME_BYTE_ERROR, p);
         }
      }

      if(CheckEDC(rb->recovered, rb->xaMode)) break;

      /* Perform Q-Parity error correction */

      for(q=0; q<N_Q_VECTORS; q++)
      {  
         /* Determine number of erasures */

         GetQVector(rb->byteState, q_state, q);
         erasure_count = 0;

         for(i=0; i<Q_VECTOR_SIZE-2; i++)
            if(q_state[i] == FRAME_BYTE_ERROR) erasures[erasure_count++] = i;

         /* First try to see whether Q is correctable without erasure markings. */

         GetQVector(rb->recovered, q_vector, q);
         err = DecodePQ(rb->rt, q_vector, Q_PADDING, ignore, 0);

         if(err == 1) /* Store back corrected vector */ 
         {  
            SetQVector(rb->recovered, q_vector, q);
            FillQVector(rb->byteState, FRAME_BYTE_GOOD, q);
            q_corrected++;
            q_err++;
         }
         
         /* If there are more than 2 erasures we have have to decimate them to 2. */
         /* The case for 2 erasure is also included here. */

         if(err < 0 && erasure_count > 1) 
         {  unsigned char best_q[Q_VECTOR_SIZE];
            int best_p_failures = N_P_VECTORS;
            int best_p_errors = N_P_VECTORS;
            int candidate = FALSE;
            int a, b;      

            /* Try error correction with decimated erasures */
            
            for(a = 0; a < erasure_count - 1; a++)
            {
               for(b = a + 1; b < erasure_count; b++)
               {
                  decimated_erasures[0] = erasures[a];
                  decimated_erasures[1] = erasures[b];
                  GetQVector(rb->recovered, q_vector, q);
                  err = DecodePQ(rb->rt, q_vector, Q_PADDING, decimated_erasures, 2);
                  if(err == 2) 
                  {  int p_err, p_fail;

                     candidate = TRUE;
                     eval_q_candidate(rb, q_vector, q, &p_fail, &p_err);

                     if(p_fail <= best_p_failures && p_err <= best_p_errors)
                     {  best_p_failures = p_fail;
                        best_p_errors   = p_err;
                        memcpy(best_q, q_vector, Q_VECTOR_SIZE); 
                     }
                  }
               }
            }

            if(!candidate) err = -1; /* If Q failed */
            else
            {  FillQVector(rb->byteState, FRAME_BYTE_GOOD, q);
               SetQVector(rb->recovered, best_q, q);
               
               err = 0;
               q_err += 2;
               q_corrected++;
               q_decimated++;
            }
         }
         
         if(err == 0) FillQVector(rb->byteState, FRAME_BYTE_GOOD, q);
         if(err < 0)  /* Uncorrectable. Mark bytes are erasure. */
         {  
            q_failures++;
            FillQVector(rb->byteState, FRAME_BYTE_ERROR, q);
         }
      }

      /* See if there was an improvement */

#ifdef DEBUG_HEURISTIC_LEC
      Verbose("L-EC: iteration %d\n", iteration); 
      Verbose("      P-failures/corrected/errors + decimated: %2d/%2d/%2d + %2d\n", p_failures, p_corrected, p_err, p_decimated);
      Verbose("      Q-failures/corrected/errors + decimated: %2d/%2d/%2d + %2d\n", q_failures, q_corrected, q_err, q_decimated);
#endif
      
      if(p_failures + p_err + q_failures + q_err == 0) break;
      if(last_p_err <= p_err && last_q_err <= q_err && last_p_failures <= p_failures && last_q_failures <= q_failures) break;
                
      if(iteration > N_P_VECTORS + N_Q_VECTORS) break;
      
      if(CheckEDC(rb->recovered, rb->xaMode)) break;

      last_p_err = p_err;
      last_q_err = q_err;
      last_p_failures = p_failures;
      last_q_failures = q_failures;
      iteration++;
   }
   
   return TRUE;
}

/***
 *** Heuristic search for best sector
 *** Andrei Grecu, 2006
 */

//#define DEBUG_SEARCHPLAUSIBLESECTOR

static int check_q_plausibility(RawBuffer *rb, unsigned char *target_q_vector, 
                                int q, int pos_a, int pos_b)
{  unsigned char q_vector[Q_VECTOR_SIZE];
   int plausible = FALSE;
   int q_index;
   int i;
   
   /* If no position was given, then find it out. */

   if(pos_a == -1)
   {
     GetQVector(rb->recovered, q_vector, q);
     for(i = 0; i < Q_VECTOR_SIZE; i++)
       if(target_q_vector[i] != q_vector[i]) 
       {
         pos_a = i;
         break;
       }

     /* pos_a == -1 implies that only 1 byte was corrected
        -> don't care about pos_b */

     pos_b = -1;  
   }
        
   /* Check plausibility of pos_a. */

   q_index = QToByteIndex(q, pos_a);

   for(i = 0; i < rb->samplesRead; i++)
     if(rb->rawBuf[i][q_index] == target_q_vector[pos_a]) 
     { 
       plausible = TRUE; 
       break; 
     }
        
   if(!plausible) return FALSE;

   /* pos_b only != -1 when Q corrected in erasure mode */

   plausible = FALSE;
        
   if(pos_b != -1)
   {
     /* Check plausibility of pos_b. */

     q_index = QToByteIndex(q, pos_b);

     for(i = 0; i < rb->samplesRead; i++)
       if(rb->rawBuf[i][q_index] == target_q_vector[pos_b]) 
       { 
         plausible = TRUE; 
         break; 
       }
     
     if(!plausible) return FALSE;
   }

   return TRUE;
}

static int check_p_plausibility(RawBuffer *rb, unsigned char *target_p_vector, 
                                int p, int pos_a, int pos_b)
{  unsigned char p_vector[P_VECTOR_SIZE];
   int plausible = FALSE;
   int p_index;
   int i;
   
   /* If no position was given, then find it out. */

   if(pos_a == -1)
   {
     GetPVector(rb->recovered, p_vector, p);
     for(i = 0; i < P_VECTOR_SIZE; i++)
     {
       if(target_p_vector[i] != p_vector[i]) 
       {
         pos_a = i;
         break;
       }
     }

     /* pos_a == -1 implies that only 1 byte was corrected
        -> don't care about pos_b */

     pos_b = -1;
   }
        
   /* Check plausibility of pos_a. */

   p_index = PToByteIndex(p, pos_a);

   for(i = 0; i < rb->samplesRead; i++)
     if(rb->rawBuf[i][p_index] == target_p_vector[pos_a]) 
     { 
       plausible = TRUE; 
       break; 
     }

   if(!plausible) return FALSE;
   
   /* pos_b only != -1 when P corrected in erasure mode */

   plausible = FALSE;

   if(pos_b != -1)
   {
     /* Check plausibility of pos_b. */

     p_index = PToByteIndex(p, pos_b);

     for(i = 0; i < rb->samplesRead; i++)
       if(rb->rawBuf[i][p_index] == target_p_vector[pos_b]) 
       { 
         plausible = TRUE; 
         break; 
       }
                
     if(!plausible) return FALSE;
   }
   
   return TRUE;
}

static int find_better_p(RawBuffer *rb, int p, int refError)
{  unsigned char p_vector[P_VECTOR_SIZE];
   int np1, np2;
   int ignore[2];
   int err;
  
   /* Try all possible Ps and see whether we get a better load. */
 
   for(np1 = 0; np1 < rb->pParityN[p][0]; np1++)
   {
     for(np2 = 0; np2 < rb->pParityN[p][1]; np2++)
     {
       GetPVector(rb->recovered, p_vector, p);
       p_vector[24] = rb->pParity1[p][np1];
       p_vector[25] = rb->pParity2[p][np2];
                        
       err = DecodePQ(rb->rt, p_vector, P_PADDING, ignore, 0);
       if(err < refError && err >= 0)
       {
         SetPVector(rb->recovered, p_vector, p);                        
         return TRUE;
       }
     }
   }
   
   return FALSE;
}

static int find_better_q(RawBuffer *rb, int q, int refError)
{  unsigned char q_vector[Q_VECTOR_SIZE];
   int nq1, nq2;
   int ignore[2];
   int err;
  
   /* Try all possible Qs and see whether we get a better load. */

   for(nq1 = 0; nq1 < rb->qParityN[q][0]; nq1++)
   {
     for(nq2 = 0; nq2 < rb->qParityN[q][1]; nq2++)
     {
       GetQVector(rb->recovered, q_vector, q);
       q_vector[43] = rb->qParity1[q][nq1];
       q_vector[44] = rb->qParity2[q][nq2];
                        
       err = DecodePQ(rb->rt, q_vector, Q_PADDING, ignore, 0);
       if(err < refError && err >= 0)
       {
         SetQVector(rb->recovered, q_vector, q);                        
         return TRUE;
       }
     }
   }
   
   return FALSE;
}

static void initialize_frame(RawBuffer *rb)
{  int best_sector = 0;
   int max_load = 2 * (N_P_VECTORS + N_Q_VECTORS);
   int i;
        
   /* Initialize sector header. */

   InitializeCDFrame(rb->recovered, rb->lba, rb->xaMode, 0);

   /* Search for block with least P and Q load and load into the frame. */

   for(i = 0; i < rb->samplesRead; i++) 
   {
     if(rb->qLoad[i] + rb->pLoad[i] < max_load)
     {
       max_load = rb->pLoad[i] + rb->qLoad[i];
       best_sector = i;
     }
   }
   
   memcpy(&(rb->recovered[16]), &(rb->rawBuf[best_sector][16]), rb->sampleSize - 16);
   if(!rb->xaMode)
      memset(&(rb->recovered[2068]), 0, 8); /* 8 zero fill bytes */
}

//#define DEBUG_SEARCH_PLAUSIBLE

int SearchPlausibleSector(RawBuffer *rb, int noCreateBuffer)
{
   unsigned char p_vector[26];
   unsigned char q_vector[45];
   unsigned char cp_vector[26];
   unsigned char cq_vector[45];
   int decimated_erasures[2];
   int ignore[2];
   int p_failures, q_failures;
   int p_corrected, q_corrected;
   int p,q;
   int iteration=1;
   int p_err, q_err;
   int p_decimated, q_decimated;
   int last_p_err = N_P_VECTORS;
   int last_q_err = N_Q_VECTORS;
   int last_p_failures = N_P_VECTORS;
   int last_q_failures = N_Q_VECTORS;
   int err;
   
   /* Initialize sector */     

   if(!noCreateBuffer) initialize_frame(rb);
   else InitializeCDFrame(rb->recovered, rb->lba, rb->xaMode, 1);
	
   for(; ;) /* iterate over P- and Q-Parity until failures converge */
   {   
      p_failures = q_failures = 0;
      p_corrected = q_corrected = 0;
      p_err = q_err = 0;
      p_decimated = q_decimated = 0;

      /* Perform Q-Parity error correction */

      for(q=0; q<N_Q_VECTORS; q++)
      {  
      	/* Check whether Q is correct. */
      	GetQVector(rb->recovered, q_vector, q);
	err = DecodePQ(rb->rt, q_vector, Q_PADDING, ignore, 0);
         
	/* If it is not correct. */
	if(err == 1 || err < 0)
        {	        
	  /* If Q is correctable. */
	  if(err == 1)
	  {
	    /* Check correction for plausibility: if corrected byte was read in one of the read attempts. */
	    if(check_q_plausibility(rb, q_vector, q, -1, -1))
	    {
	      /* Store back corrected vector */ 
	      SetQVector(rb->recovered, q_vector, q);
	      q_corrected++;
	      q_err++;
	    }
	    else 
	    {
	      /* See whether we can find some Q parity bytes accepting the original Q vector. */
	      if(find_better_q(rb, q, 1)) 
	      { 
		q_corrected++; 
		q_err++; 
	      }
	      else err = -1;
	    }
	  }
	         
	  /* If correction is not plausible or possible then try 2 error-decimation. */
	  if(err < 0)
	  {
	    /* Try error correction with decimated erasures.
	       Note that no erasure information for the parity bytes is available */
	    int a, b; 
	    int solFound = FALSE;     

	    GetQVector(rb->byteCount, cq_vector, q);

	    for(a = 0; a < Q_VECTOR_SIZE - 2; a++)
	    {
	      if(cq_vector[a] == 1) continue; /* no alternatives */
	      for(b = a + 1; b < Q_VECTOR_SIZE - 2; b++)
	      {
		if(cq_vector[b] == 1) continue; /* again, no alternatives present */

		decimated_erasures[0] = a;
		decimated_erasures[1] = b;
		GetQVector(rb->recovered, q_vector, q);
		err = DecodePQ(rb->rt, q_vector, Q_PADDING, decimated_erasures, 2);
		if(err == 2) 
		{ 
		  if(check_q_plausibility(rb, q_vector, q, a, b)) { solFound = TRUE; break;	}
		}
	      }
	      if(solFound) break;
	    }
	    if(solFound)
	    {
	      /* Store back corrected vector */ 
	      SetQVector(rb->recovered, q_vector, q);
	      q_err += 2;
	      q_corrected++;
	      q_decimated++;
	    }
	    else 
	    {
	      /* See whether we can find some Q parity bytes accepting the original Q vector. */
	      if(find_better_q(rb, q, 2)) 
	      { 
		q_corrected++; 
		q_err++; 
	      }
	      else q_failures++; /* If Q failed */
	    }
	  }
	}
      }

      /* Perform P-Parity error correction */

      for(p=0; p<N_P_VECTORS; p++)
      {  
      	/* Check whether P is correct. */
      	GetPVector(rb->recovered, p_vector, p);
	err = DecodePQ(rb->rt, p_vector, P_PADDING, ignore, 0);
         
	/* If it is not correct. */
	if(err == 1 || err < 0)
        {	        
	  /* If Q is correctable. */
	  if(err == 1)
	  {
	    /* Check correction for plausibility: if corrected byte was read in one of the read attempts. */
	    if(check_p_plausibility(rb, p_vector, p, -1, -1))
	    {
	      /* Store back corrected vector */ 
	      SetPVector(rb->recovered, p_vector, p);
	      p_corrected++;
	      p_err++;
	    }
	    else 
	    {
	      /* See whether we can find some P parity bytes accepting the original P vector. */
	      if(find_better_p(rb, p, 1)) 
	      { 
		p_corrected++; 
		p_err++; 
	      }
	      else err = -1;
	    }
	  }
	  
	  /* If correction is not plausible or possible then try 2 error-decimation. */
	  if(err < 0)
	  {
	    /* Try error correction with decimated erasures */
	    int a, b; 
	    int solFound = FALSE;     
	    GetPVector(rb->byteCount, cp_vector, p);
	    for(a = 0; a < P_VECTOR_SIZE-2; a++)         /* fixme: why -2? */
	    { if(cp_vector[a] == 1) continue;

	      for(b = a + 1; b < P_VECTOR_SIZE-2; b++)   /* fixme: why -2? */
	      { if(cp_vector[b] == 1) continue;

		decimated_erasures[0] = a;
		decimated_erasures[1] = b;
		GetPVector(rb->recovered, p_vector, p);
		err = DecodePQ(rb->rt, p_vector, P_PADDING, decimated_erasures, 2);
		if(err == 2) 
		{ 
		  if(check_p_plausibility(rb, p_vector, p, a, b)) { solFound = TRUE; break; }
		}
	      }
	      if(solFound) break;
	    }
	    if(solFound)
	    {
	      /* Store back corrected vector */ 
	      SetPVector(rb->recovered, p_vector, p);
	      p_err += 2;
	      p_corrected++;
	      p_decimated++;
	    }
	    else 
	    {
	      /* See whether we can find some P parity bytes accepting the original P vector. */
	      if(find_better_p(rb, p, 2)) 
	      { 
		p_corrected++; 
		p_err++; 
	      }
	      else p_failures++; /* If P failed */
	    }
	  }
	}
      }

      /* See if there was an improvement */

#ifdef DEBUG_SEARCH_PLAUSIBLE
      Verbose("SPS L-EC: iteration %d\n", iteration); 
      Verbose("      Q-f/c/e + d: %2d/%2d/%2d + %2d\n", q_failures, q_corrected, q_err, q_decimated);
      Verbose("      P-f/c/e + d: %2d/%2d/%2d + %2d\n", p_failures, p_corrected, p_err, p_decimated);
#endif
      
      if(p_failures + p_err + q_failures + q_err == 0) break;
      if(last_p_err <= p_err && last_q_err <= q_err && last_p_failures <= p_failures && last_q_failures <= q_failures) break;
      
      if(iteration > N_P_VECTORS + N_Q_VECTORS) break;
      if(p_failures == 0)
      {
	if(CheckEDC(rb->recovered, rb->xaMode)) break;
      }
      
      last_p_err = p_err;
      last_q_err = q_err;
      last_p_failures = p_failures;
      last_q_failures = q_failures;
      iteration++;
   }	

   return TRUE;
}

/***
 *** Correction by mutual Acknowledgement
 *** Andrei Grecu, 2007
 */

/* checks wether P acknowledges a Q byte */
/* the value to be used as the final Q byte is returned in *val */
int does_P_ack(RawBuffer *rb, int q, int qpos, unsigned char *val, unsigned char *q_status)
{
  unsigned char  p_vector[26];
  unsigned char  q_vector[45];
  unsigned char tp_vector[26];
  unsigned char tq_vector[45];
  int decimated_erasures[2];
  int ignore[2];
  int err;
  int p, ppos;
  int idx;

  int tryAllErasures = 0;

  /* initialize q vectors */
  GetPVector(rb->recovered,  q_vector, q);
  GetPVector(rb->recovered, tq_vector, q);
  q_vector[qpos] = *val;

  /* check corresponding p for errors */
  idx = QToByteIndex(q, qpos);
  ByteIndexToP(idx, &p, &ppos);

  GetPVector(rb->recovered, p_vector, p);
  err = DecodePQ(rb->rt, p_vector, P_PADDING, ignore, 0);

  /* well we do have a problem here: p shows no error, but q does */
  if(err == 0)
  {
     /* this is a special case of p and q want to correct the same byte */
     /* provisoric heuristic solution: usually we should leave the byte untouched */
     /*   but if q wants to correct this byte to something which was already read */
     /*   then we will take the corrected byte */
     if(!check_q_plausibility(rb, tq_vector, q, qpos, -1) && check_q_plausibility(rb, q_vector,  q, qpos, -1)) return 1;
  }
  
  /* p and q show an error */
  if(err == 1)
  {
     /* get position of corrected byte */
     int tpos = -1, i;
     GetPVector(rb->recovered, tp_vector, p);
     for(i = 0; i < 26; i++) 
     {
	if(p_vector[i] != tp_vector[i]) { tpos = i; break; }
     }

     /* looks good, p wants to correct this byte too */
     if(tpos == ppos)
     {
	/* fantastic, p and q want to correct this byte to the same value */
	if(q_vector[qpos] == p_vector[ppos]) return 1;
	/* not good, p and q want to correct this byte to different values */
	/* possibility 1: q has more than one faulty byte */
	/* possibility 2: p has more than one faulty byte */
	/* possibility 3: q, p have more than one faulty byte */
	/* provisoric heuristic solution: take the version which was already read, if it was already read */
	else
	{
	   if(check_q_plausibility(rb, q_vector, q, qpos, -1)) {					     return 1; }
	   if(check_p_plausibility(rb, p_vector, p, ppos, -1)) { *val = p_vector[ppos]; return 1; }
	}
     }
     /* looks bad, p wants to correct another byte */
     /* possibility 1: q has more than one faulty byte */
     /* possibility 2: p has more than one faulty byte */
     /* possibility 3: q, p have more than one faulty byte */
     /* provisoric heuristic solution: consider p has failed */
     else tryAllErasures = 1;
  }

  /* p failure... we have to try all 2-erasure combinations containing this position */
  if(err < 0) tryAllErasures = 1;
  
  /* try all p 2-erasures for this position */
  if(tryAllErasures)
  {
     int a; 
     for(a = 0; a < P_VECTOR_SIZE; a++)
     {
	int qi, qposi;
	
	if(a == ppos) continue;
	
	if(q_status != NULL)
	{
	   /* if the corresponding q does not show an error we should not try to correct it */
	   idx = PToByteIndex(p, a); ByteIndexToQ(idx, &qi, &qposi);
	   if(q_status[qi] == 0) continue;
	}
	
	decimated_erasures[0] = ppos < a ? ppos : a;
	decimated_erasures[1] = ppos < a ? a : ppos;
	GetPVector(rb->recovered, p_vector, p);
	err = DecodePQ(rb->rt, p_vector, P_PADDING, decimated_erasures, 2);
	if(err == 2) 
	{ 
	   /* please note: there can be more matching versions */
	   /* provisoric heuristic solution: ignore the others */
	   if(p_vector[ppos] == q_vector[qpos]) 
	   { 
	      return 1;
	   }
	}
     }
  }
  
  return 0;
}

/* checks wether Q acknowledges a P byte */
/* the value to be used as the final P byte is returned in *val */
int does_Q_ack(RawBuffer *rb, int p, int ppos, unsigned char *val, unsigned char *p_status)
{
   unsigned char  p_vector[26];
   unsigned char  q_vector[45];
   unsigned char tp_vector[26];
   unsigned char tq_vector[45];
   int decimated_erasures[2];
   int ignore[2];
   int err;
   int q, qpos;
   int idx;
   
   int tryAllErasures = 0;
   
   /* initialize p vectors */
   GetPVector(rb->recovered,  p_vector, p);
   GetPVector(rb->recovered, tp_vector, p);
   p_vector[ppos] = *val;
   
   /* check corresponding q for errors */
   idx = PToByteIndex(p, ppos);
   ByteIndexToQ(idx, &q, &qpos);
   
   GetQVector(rb->recovered, q_vector, q);
   err = DecodePQ(rb->rt, q_vector, Q_PADDING, ignore, 0);
   
   /* well we do have a problem here: q shows no error, but p does */
   if(err == 0)
   {
      /* this is a special case of p and q want to correct the same byte */
      /* provisoric heuristic solution: usually we should leave the byte untouched */
      /*   but if q wants to correct this byte to something which was already read */
      /*   then we will take the corrected byte */
      if(!check_p_plausibility(rb, tp_vector, p, ppos, -1) && check_p_plausibility(rb, p_vector,  p, ppos, -1)) return 1;
   }
   
   /* p and q show an error */
   if(err == 1)
   {
      /* get position of corrected byte */
      int tpos = -1, i;
      GetQVector(rb->recovered, tq_vector, q);
      for(i = 0; i < 45; i++) 
      {
	 if(q_vector[i] != tq_vector[i]) { tpos = i; break; }
      }
      
      /* looks good, q wants to correct this byte too */
      if(tpos == ppos)
      {
	 /* fantastic, p and q want to correct this byte to the same value */
	 if(p_vector[ppos] == q_vector[qpos]) return 1;
	 /* not good, p and q want to correct this byte to different values */
	 /* possibility 1: q has more than one faulty byte */
	 /* possibility 2: p has more than one faulty byte */
	 /* possibility 3: q, p have more than one faulty byte */
	 /* provisoric heuristic solution: take the version which was already read, if it was already read */
	 else
	 {
	    if(check_p_plausibility(rb, p_vector, p, qpos, -1)) {					     return 1; }
	    if(check_q_plausibility(rb, q_vector, q, ppos, -1)) { *val = q_vector[qpos]; return 1; }
	 }
      }
      /* looks bad, q wants to correct another byte */
      /* possibility 1: q has more than one faulty byte */
      /* possibility 2: p has more than one faulty byte */
      /* possibility 3: q, p have more than one faulty byte */
      /* provisoric heuristic solution: consider p has failed */
      else tryAllErasures = 1;
   }
   
   /* q failure... we have to try all 2-erasure combinations containing this position */
   if(err < 0) tryAllErasures = 1;
   
   /* try all q 2-erasures for this position */
   if(tryAllErasures)
   {
      int a; 
      for(a = 0; a < Q_VECTOR_SIZE; a++)
      {
	 int pi, pposi;
	 
	 if(a == qpos) continue;
	 
	 if(p_status != NULL)
	 {
	    /* if the corresponding p does not show an error we should not try to correct it */
	    idx = QToByteIndex(q, a); ByteIndexToP(idx, &pi, &pposi);
	    if(p_status[pi] == 0) continue;
	 }
	 
	 decimated_erasures[0] = qpos < a ? qpos : a;
	 decimated_erasures[1] = qpos < a ? a : qpos;
	 GetQVector(rb->recovered, q_vector, q);
	 err = DecodePQ(rb->rt, q_vector, Q_PADDING, decimated_erasures, 2);
	 if(err == 2) 
	 { 
	    /* please note: there can be more matching versions */
	    /* provisoric heuristic solution: ignore the others */
	    if(q_vector[qpos] == p_vector[ppos]) 
	    { 
	       return 1;
	    }
	 }
      }
   }
   
   return 0;
}

int byteBadProbACK[2][4][4][2];

int AckHeuristic(RawBuffer *rb)
{
   unsigned char p_vector[26];
   unsigned char q_vector[45];
   unsigned char tp_vector[26];
   unsigned char tq_vector[45];
   unsigned char qp_sector[45][26 * 2];
   unsigned char pq_sector[26][45 * 2];
   unsigned char qp_sector_valid[45][26];
   unsigned char pq_sector_valid[26][45];
   unsigned char qp_sector_exist[45];
   unsigned char pq_sector_exist[26];
   unsigned char p_status[N_P_VECTORS];
   unsigned char q_status[N_Q_VECTORS];
   int decimated_erasures[2];
   int ignore[2];
   int p_failures, q_failures;
   int p_corrected, q_corrected;
   int p, q;
   int i, j;
   int iteration=1;
   int p_err, q_err;
   int last_p_err = N_P_VECTORS;
   int last_q_err = N_Q_VECTORS;
   int last_p_failures = N_P_VECTORS;
   int last_q_failures = N_Q_VECTORS;
   int err;
   int qpos, ppos, tpos, idx;
 
   /* Re-Initialize sector */     
   InitializeCDFrame(rb->recovered, rb->lba, rb->xaMode, 1);
   
   /* initialize counters */
   memset(qp_sector_exist, 0, 45);
   memset(pq_sector_exist, 0, 26);

   for(; ;) /* iterate over P- and Q-Parity until failures converge */
   {   
      p_failures = q_failures = 0;
      p_corrected = q_corrected = 0;
      p_err = q_err = 0;
      
      /* Get the entire Q status */
      for(q = 0; q < N_Q_VECTORS; q++)
      {  
	 GetQVector(rb->recovered, q_vector, q);
	 err = DecodePQ(rb->rt, q_vector, Q_PADDING, ignore, 0);
	 if(err <  0) q_status[q] = 2;
	 if(err == 1) q_status[q] = 1;
	 if(!err)	 q_status[q] = 0;
      }
      
      /* Get the entire P status */
      for(p = 0; p < N_P_VECTORS; p++)
      {  
	 GetPVector(rb->recovered, p_vector, p);
	 err = DecodePQ(rb->rt, p_vector, P_PADDING, ignore, 0);
	 if(err <  0) p_status[p] = 2;
	 if(err == 1) p_status[p] = 1;
	 if(!err)	 p_status[p] = 0;
      }
      
      /* Perform Q-Parity error correction */
      for(q = 0; q < N_Q_VECTORS; q++)
      {  
	 /* Check whether Q is correct. */
	 GetQVector(rb->recovered, q_vector, q);
	 err = DecodePQ(rb->rt, q_vector, Q_PADDING, ignore, 0);
	 
	 /* if Q is correctable */
	 if(err == 1)
	 {
	    q_err++;
	    
	    /* get position of corrected byte */
	    qpos = -1;
	    GetQVector(rb->recovered, tq_vector, q);
	    for(i = 0; i < 45; i++) if(q_vector[i] != tq_vector[i]) { qpos = i; break; }
	    
	    /* Q wants to correct itself */
	    if(qpos == 43 || qpos == 44)
	    {
	       /* we only allow this if the byte was already read */
	       if(check_q_plausibility(rb, q_vector, q, qpos, -1)) { q_corrected++; SetQVector(rb->recovered, q_vector, q); }
	    }
	    else {
	       if(does_P_ack(rb, q, qpos, &q_vector[qpos], q_status)) { q_corrected++; SetQVector(rb->recovered, q_vector, q); }
	    }
	 }
	 
	 /* q failure... we have to try all 2-erasure combinations of q and check p (try all 2-erasure combinations of p if necessary) */
	 if(err < 0)
	 {
	    int a, b;
	    int manySols = 0;
	    int undecidable = 0;
	    int bestCompFound = 0;
	    int bestA = 0;
	    int bestB = 0;
	    int bestPSol1 = 0;
	    int bestPSol2 = 0;
	    int bestP1 = 0;
	    int bestP2 = 0;
	    int bestP1_ACK = 0;
	    int bestP2_ACK = 0;

	    q_failures++; q_err += 2;
	    
	    /* try 2-erasure combinations of p */
	    for(i = 0; i < 45 - 2; i++)
	    {
	       int tryAllErasures = 0;
	       /* check corresponding p for errors */
	       idx = QToByteIndex(q, i);
	       ByteIndexToP(idx, &p, &ppos);
	       
	       GetPVector(rb->recovered, p_vector, p);
	       err = DecodePQ(rb->rt, p_vector, P_PADDING, ignore, 0);
	       
	       /* if p shows no error, then fill vector with already existing bytes */
	       if(err == 0) 
	       {
		  qp_sector_exist[i] = 0;
	       }

	       if(err == 1)
	       {
		  /* get position of corrected byte */
		  tpos = -1;
		  GetPVector(rb->recovered, tp_vector, p);
		  for(j = 0; j < 26; j++) if(p_vector[j] != tp_vector[j]) { tpos = j; break; }
		  
		  /* looks good, p wants to correct this byte too */
		  if(tpos == ppos)
		  {
		     for(j = 0; j < 26; j++)
		     {
			qp_sector[i][j * 2    ] = p_vector[ppos];
			qp_sector[i][j * 2 + 1] = p_vector[j];
			qp_sector_valid[i][j] = 1;
		     }
		     qp_sector_exist[i] = 2;
		  }
		  /* p wants to correct another byte */
		  /* provisoric heuristic solution: consider p has failed */
		  else tryAllErasures = 1;
	       }
	       
	       /* p failure... we have to try all 2-erasure combinations containing this position */
	       if(err < 0) tryAllErasures = 1;
	       
	       if(tryAllErasures)
	       {
		  for(a = 0; a < P_VECTOR_SIZE; a++)
		  {
		     int qi, qposi;
		     
		     if(a == ppos) continue;

		     idx = PToByteIndex(p, a); ByteIndexToQ(idx, &qi, &qposi);
		     
		     /* if this q does not show an error we should not try to correct it */
		     if(q_status[qi] == 0) 
		     {
			qp_sector_valid[i][a] = 0;
			continue;
		     }
		     
		     decimated_erasures[0] = ppos < a ? ppos : a;
		     decimated_erasures[1] = ppos < a ? a : ppos;
		     GetPVector(rb->recovered, p_vector, p);
		     err = DecodePQ(rb->rt, p_vector, P_PADDING, decimated_erasures, 2);
		     if(err == 2) 
		     { 
			qp_sector[i][a * 2    ] = p_vector[ppos];
			qp_sector[i][a * 2 + 1] = p_vector[a];
			qp_sector_valid[i][a] = 1;
		     }
		     else qp_sector_valid[i][a] = 0;
		  }
		  
		  qp_sector_exist[i] = 1;
	       }
	    }
	    
	    /* try 2-erasure combinations of q */
	    for(a = 0; a < 45 - 1; a++)
	    {
	       if(a < 45 - 2 && !qp_sector_exist[a]) continue; /* if p reported no problems for first byte then we cannot do much here */
	       
	       for(b = a + 1; b < 45; b++)
	       {
		  int compFound = 0;
		  int pSol1 = -1, pSol2 = -1;
		  int aMul = 0, bMul = 0;

		  if(b < 45 - 2 && !qp_sector_exist[b]) continue; /* if p reported no problems for second byte then we cannot do much here */

		  decimated_erasures[0] = a;
		  decimated_erasures[1] = b;
		  GetQVector(rb->recovered, q_vector, q);
		  err = DecodePQ(rb->rt, q_vector, Q_PADDING, decimated_erasures, 2);
		  if(err == 2) 
		  { 
		     int ppos1, ppos2;
		     int p1, p2;
		     idx = QToByteIndex(q, a); ByteIndexToP(idx, &p1, &ppos1);
		     idx = QToByteIndex(q, b); ByteIndexToP(idx, &p2, &ppos2);
		     
		     /* check first q corrected byte with all its p counterparts */ 
		     if(a < 45 - 2 && qp_sector_exist[a])
		     {
			for(i = 0; i < P_VECTOR_SIZE; i++)
			{
			   if(i == ppos1) continue;
			   if(qp_sector_valid[a][i] == 0) continue;
			   
			   if(qp_sector[a][i * 2] == q_vector[a])
			   {
			      aMul++;
			      
			      /* we found matching q and p corrected bytes */
			      compFound |= 0x01;
			      /* if p also wants to correct this byte with same value */
			      if(qp_sector_exist[a] == 2) 
			      {
				 compFound |= 0x04;
				 break;
			      }
			      
			      pSol1 = i;
			   }
			}
		     }
		     
		     /* check second q corrected byte with all its p counterparts */ 
		     if(b < 45 - 2 && qp_sector_exist[b])
		     {
			for(i = 0; i < P_VECTOR_SIZE; i++)
			{
			   if(i == ppos2) continue;
			   if(qp_sector_valid[b][i] == 0) continue;
			   
			   if(qp_sector[b][i * 2] == q_vector[b])
			   {
			      bMul++;
			      
			      /* we found matching q and p corrected bytes */
			      compFound |= 0x02;
			      /* if p also wants to correct this byte with same value */
			      if(qp_sector_exist[b] == 2) 
			      {
				 compFound |= 0x08;
				 break;
			      }
			      
			      pSol2 = i;
			   }
			}
		     }
		     
		     /* both bytes were acknowledged by the respective ps so save the change into temporary parameters */
		     if(compFound & 0x01 || compFound & 0x02 || compFound & 0x04 || compFound & 0x08)
		     {
			if(aMul <= 1 && bMul <= 1)
			{
			   /* FIXME: maybe error: did original code really want "+" precede "&"? */
#if 0  /* original code */
			   int     quality =     compFound & 0x03 + (    compFound & 0x04) * 2 + (    compFound & 0x08) * 2;
			   int bestQuality = bestCompFound & 0x03 + (bestCompFound & 0x04) * 2 + (bestCompFound & 0x08) * 2;
#endif
			   int     quality =     (compFound & 0x03) + (    compFound & 0x04) * 2 + (    compFound & 0x08) * 2;
			   int bestQuality = (bestCompFound & 0x03) + (bestCompFound & 0x04) * 2 + (bestCompFound & 0x08) * 2;
			   int P1_ACK = 0, P2_ACK = 0;
			   
			   unsigned char c;
			   
			   if(compFound & 0x01 && !(compFound & 0x04))
			   {
			      c = qp_sector[a][pSol1 * 2 + 1];
			      if(does_Q_ack(rb, p1, pSol1, &c, NULL)) P1_ACK = 1;
			      
			      p_vector[pSol1] = qp_sector[a][pSol1 * 2 + 1];
			      if(check_p_plausibility(rb, p_vector, p1, pSol1, -1)) P1_ACK = 1;
			      
			      if(check_q_plausibility(rb, q_vector, q, a, -1)) P1_ACK = 1;
			   }
			   
			   if(compFound & 0x02 && !(compFound & 0x08))
			   {
			      c = qp_sector[b][pSol2 * 2 + 1];
			      if(does_Q_ack(rb, p2, pSol2, &c, NULL)) P2_ACK = 1;
			      
			      p_vector[pSol2] = qp_sector[b][pSol2 * 2 + 1];
			      if(check_p_plausibility(rb, p_vector, p2, pSol2, -1)) P2_ACK = 1;
			      
			      if(check_q_plausibility(rb, q_vector, q, b, -1)) P2_ACK = 1;
			   }
			   
			   if(!P1_ACK && !P2_ACK && !(compFound & 0x04) && !(compFound & 0x08)) quality = 0;
			   
			   if(quality > bestQuality)
			   {
			      bestA = a;
			      bestB = b;
			      bestPSol1 = pSol1;
			      bestPSol2 = pSol2;
			      bestP1 = p1;
			      bestP2 = p2;
			      bestP1_ACK = P1_ACK;
			      bestP2_ACK = P2_ACK;
			      
			      bestCompFound = compFound;
			      
			      manySols++;
			   }
			   else if(quality == bestQuality) undecidable = 1;
			   
			}
			
			if(aMul > 1 || bMul > 1)
			{
			   aMul++;
			   bMul++;
			}
		     }
		     
		     /* if both parity bytes should be corrected */
		     /* take them only if they were already read */
		     if(a == 45 - 2)
		     {
			if(check_q_plausibility(rb, q_vector, q, a, b))
			{
			   /* this solution has low priority, because here we have no acknowledging p */
			   if(manySols == 0)
			   {
			      bestA = a;
			      bestB = b;
			      bestCompFound = 0x10;
			      
			      manySols++;
			   }
			}
		     }
		  }
	       }
	    }
	    
	    /* after the best heuristic solution was found */
	    if(!undecidable && manySols > 0)
	    {
	       int qquality = 0;
	       
	       decimated_erasures[0] = bestA;
	       decimated_erasures[1] = bestB;
	       GetQVector(rb->recovered, q_vector, q);
	       DecodePQ(rb->rt, q_vector, Q_PADDING, decimated_erasures, 2);
	       
	       /* take both q corrections */
	       SetQVector(rb->recovered, q_vector, q);
	       
	       idx = QToByteIndex(q, bestA); qquality += rb->recovered[idx] == rb->reference[idx];
	       idx = QToByteIndex(q, bestB); qquality += rb->recovered[idx] == rb->reference[idx];
	       
	       byteBadProbACK[1][bestCompFound & 0x03][((bestCompFound & 0x04) >> 2) + ((bestCompFound & 0x08) >> 2)][!(qquality == 2)]++;
	       
	       if(bestCompFound & 0x01 && !(bestCompFound & 0x04) && bestP1_ACK)
	       {
		  idx = PToByteIndex(bestP1, bestPSol1);
		  rb->recovered[idx] = qp_sector[bestA][bestPSol1 * 2 + 1];
		  
		  p_failures++; p_err += 2; p_corrected = 2;
	       }
	       
	       /* take second p correction if it was a two erasure correction */
	       if(bestCompFound & 0x02 && !(bestCompFound & 0x08) && bestP2_ACK)
	       {
		  idx = PToByteIndex(bestP2, bestPSol2);
		  rb->recovered[idx] = qp_sector[bestB][bestPSol2 * 2 + 1];
		  
		  p_failures++; p_err += 2; p_corrected = 2;
	       }
	       
	       q_corrected += 2;
	    }
	 }
      }
      
      if(CheckEDC(rb->recovered, rb->xaMode)) break;
      
      /* Get the entire Q status */
      for(q = 0; q < N_Q_VECTORS; q++)
      {  
	 GetQVector(rb->recovered, q_vector, q);
	 err = DecodePQ(rb->rt, q_vector, Q_PADDING, ignore, 0);
	 if(err <  0) q_status[q] = 2;
	 if(err == 1) q_status[q] = 1;
	 if(!err)	 q_status[q] = 0;
      }

      /* Get the entire P status */
      for(p = 0; p < N_P_VECTORS; p++)
      {  
	 GetPVector(rb->recovered, p_vector, p);
	 err = DecodePQ(rb->rt, p_vector, P_PADDING, ignore, 0);
	 if(err <  0) p_status[p] = 2;
	 if(err == 1) p_status[p] = 1;
	 if(!err)	 p_status[p] = 0;
      }
      
      /* Perform P-Parity error correction */
      for(p = 0; p < N_P_VECTORS; p++)
      {  
	 /* Check whether P is correct. */
	 GetPVector(rb->recovered, p_vector, p);
	 err = DecodePQ(rb->rt, p_vector, P_PADDING, ignore, 0);
	 
	 /* if P is correctable */
	 if(err == 1)
	 {
	    p_err++;
	    
	    /* get position of corrected byte */
	    ppos = -1;
	    GetPVector(rb->recovered, tp_vector, p);
	    for(i = 0; i < 26; i++) if(p_vector[i] != tp_vector[i]) { ppos = i; break; }
	    
	    if(does_Q_ack(rb, p, ppos, &p_vector[ppos], p_status)) { p_corrected++; SetPVector(rb->recovered, p_vector, p); }
	 }
	 
	 /* p failure... we have to try all 2-erasure combinations of q and check p (try all 2-erasure combinations of q if necessary) */
	 if(err < 0)
	 {
	    int a, b;
	    int manySols = 0;
	    int undecidable = 0;
	    int bestCompFound = 0;
	    int bestA = 0;
	    int bestB = 0;
	    int bestQSol1 = 0;
	    int bestQSol2 = 0;
	    int bestQ1 = 0;
	    int bestQ2 = 0;
	    int bestQ1_ACK = 0;
	    int bestQ2_ACK = 0;

	    p_failures++; p_err += 2;
	    
	    /* try 2-erasure combinations of p */
	    for(i = 0; i < 26; i++)
	    {
	       int tryAllErasures = 0;
	       /* check corresponding q for errors */
	       idx = PToByteIndex(p, i);
	       ByteIndexToQ(idx, &q, &qpos);
	       
	       GetQVector(rb->recovered, q_vector, q);
	       err = DecodePQ(rb->rt, q_vector, Q_PADDING, ignore, 0);
	       
	       /* if q shows no error, then fill vector with already existing bytes */
	       if(err == 0) 
	       {
		  pq_sector_exist[i] = 0;
	       }
	       
	       if(err == 1)
	       {
		  /* get position of corrected byte */
		  tpos = -1;
		  GetQVector(rb->recovered, tq_vector, q);
		  for(j = 0; j < 45; j++) if(q_vector[j] != tq_vector[j]) { tpos = j; break; }
		  
		  /* looks good, q wants to correct this byte too */
		  if(tpos == qpos)
		  {
		     for(j = 0; j < 45; j++)
		     {
			pq_sector[i][j * 2    ] = q_vector[qpos];
			pq_sector[i][j * 2 + 1] = q_vector[j];
			pq_sector_valid[i][j] = 1;
		     }
		     pq_sector_exist[i] = 2;
		  }
		  /* q wants to correct another byte */
		  /* provisoric heuristic solution: consider q has failed */
		  else if(tpos != 43 && tpos != 44) tryAllErasures = 1;
	       }
	       
	       /* q failure... we have to try all 2-erasure combinations containing this position */
	       if(err < 0) tryAllErasures = 1;
	       
	       if(tryAllErasures)
	       {
		  for(a = 0; a < Q_VECTOR_SIZE; a++)
		  {
		     int pi, pposi;
		     
		     if(a == qpos) continue;
		     
		     idx = QToByteIndex(q, a); ByteIndexToP(idx, &pi, &pposi);
		     
		     /* if this q does not show an error we should not try to correct it */
		     if(p_status[pi] == 0) 
		     {
			pq_sector_valid[i][a] = 0;
			continue;
		     }
		     
		     decimated_erasures[0] = qpos < a ? qpos : a;
		     decimated_erasures[1] = qpos < a ? a : qpos;
		     GetQVector(rb->recovered, q_vector, q);
		     err = DecodePQ(rb->rt, q_vector, Q_PADDING, decimated_erasures, 2);
		     if(err == 2) 
		     { 
			pq_sector[i][a * 2    ] = q_vector[qpos];
			pq_sector[i][a * 2 + 1] = q_vector[a];
			pq_sector_valid[i][a] = 1;
		     }
		     else pq_sector_valid[i][a] = 0;
		  }
		  
		  pq_sector_exist[i] = 1;
	       }
	    }
	    
	    /* try 2-erasure combinations of q */
	    for(a = 0; a < 26 - 1; a++)
	    {
	       if(!pq_sector_exist[a]) continue; /* if q reported no problems for first byte then we cannot do much here */
	       
	       for(b = a + 1; b < 26; b++)
	       {
		  int compFound = 0;
		  int qSol1 = -1, qSol2 = -1;
		  int aMul = 0, bMul = 0;
		  
		  if(!pq_sector_exist[b]) continue; /* if q reported no problems for second byte then we cannot do much here */
		  
		  decimated_erasures[0] = a;
		  decimated_erasures[1] = b;
		  GetPVector(rb->recovered, p_vector, p);
		  err = DecodePQ(rb->rt, p_vector, P_PADDING, decimated_erasures, 2);
		  if(err == 2) 
		  { 
		     int qpos1, qpos2;
		     int q1, q2;
		     idx = PToByteIndex(p, a); ByteIndexToQ(idx, &q1, &qpos1);
		     idx = PToByteIndex(p, b); ByteIndexToQ(idx, &q2, &qpos2);
		     
		     /* check first q corrected byte with all its p counterparts */ 
		     if(pq_sector_exist[a])
		     {
			for(i = 0; i < Q_VECTOR_SIZE; i++)
			{
			   if(i == qpos1) continue;
			   if(pq_sector_valid[a][i] == 0) continue;
			   
			   if(pq_sector[a][i * 2] == p_vector[a])
			   {
			      aMul++;
			      
			      /* we found matching q and p corrected bytes */
			      compFound |= 0x01;
			      /* if p also wants to correct this byte with same value */
			      if(pq_sector_exist[a] == 2) 
			      {
				 compFound |= 0x04;
				 break;
			      }
			      
			      qSol1 = i;
			   }
			}
		     }
		     
		     /* check second q corrected byte with all its p counterparts */ 
		     if(pq_sector_exist[b])
		     {
			for(i = 0; i < Q_VECTOR_SIZE; i++)
			{
			   if(i == qpos2) continue;
			   if(pq_sector_valid[b][i] == 0) continue;
			   
			   if(pq_sector[b][i * 2] == p_vector[b])
			   {
			      bMul++;
			      
			      /* we found matching q and p corrected bytes */
			      compFound |= 0x02;
			      /* if p also wants to correct this byte with same value */
			      if(pq_sector_exist[b] == 2) 
			      {
				 compFound |= 0x08;
				 break;
			      }
			      
			      qSol2 = i;
			   }
			}
		     }
		     
		     /* both bytes were acknowledged by the respective ps so save the change into temporary parameters */
		     if(compFound & 0x01 || compFound & 0x02 || compFound & 0x04 || compFound & 0x08)
		     {
			if(aMul <= 1 && bMul <= 1)
			{
			   /* FIXME: same probs as above */
#if 0 
			   int     quality =     compFound & 0x03 + (    compFound & 0x04) * 2 + (    compFound & 0x08) * 2;
			   int bestQuality = bestCompFound & 0x03 + (bestCompFound & 0x04) * 2 + (bestCompFound & 0x08) * 2;
#endif
			   int     quality =     (compFound & 0x03) + (    compFound & 0x04) * 2 + (    compFound & 0x08) * 2;
			   int bestQuality = (bestCompFound & 0x03) + (bestCompFound & 0x04) * 2 + (bestCompFound & 0x08) * 2;
			   int Q1_ACK = 0, Q2_ACK = 0;
			   
			   unsigned char c;
			   
			   if(compFound & 0x01 && !(compFound & 0x04))
			   {
			      if(qSol1 < 43)
			      {
				 c = pq_sector[a][qSol1 * 2 + 1];
				 if(does_P_ack(rb, q1, qSol1, &c, NULL))	Q1_ACK = 1;
			      }
			      
			      q_vector[qSol1] = pq_sector[a][qSol1 * 2 + 1];
			      if(check_q_plausibility(rb, q_vector, q1, qSol1, -1)) Q1_ACK = 1;
			      
			      if(check_p_plausibility(rb, p_vector, p, a, -1)) Q1_ACK = 1;
			   }
			   
			   if(compFound & 0x02 && !(compFound & 0x08))
			   {
			      if(qSol2 < 43)
			      {
				 c = pq_sector[b][qSol2 * 2 + 1];
				 if(does_P_ack(rb, q2, qSol2, &c, NULL))	Q2_ACK = 1;
			      }
			      
			      q_vector[qSol2] = pq_sector[b][qSol2 * 2 + 1];
			      if(check_q_plausibility(rb, q_vector, q2, qSol2, -1)) Q2_ACK = 1;
			      
			      if(check_p_plausibility(rb, p_vector, p, b, -1)) Q2_ACK = 1;
			   }
			   
			   if(!Q1_ACK && !Q2_ACK && !(compFound & 0x04) && !(compFound & 0x08)) quality = 0;
			   
			   if(quality > bestQuality)
			   {
			      bestA = a;
			      bestB = b;
			      bestQSol1 = qSol1;
			      bestQSol2 = qSol2;
			      bestQ1 = q1;
			      bestQ2 = q2;
			      bestQ1_ACK = Q1_ACK;
			      bestQ2_ACK = Q2_ACK;
			      
			      bestCompFound = compFound;
			      
			      manySols++;
			   }
			   else if(quality == bestQuality) undecidable = 1;
			   
			}
			
			if(aMul > 1 || bMul > 1)
			{
			   aMul++;
			   bMul++;
			}
		     }
		  }
	       }
	    }
	    
	    /* after the best heuristic solution was found */
	    if(!undecidable && manySols > 0)
	    {
	       int pquality = 0;
	       
	       decimated_erasures[0] = bestA;
	       decimated_erasures[1] = bestB;
	       GetPVector(rb->recovered, p_vector, p);
	       DecodePQ(rb->rt, p_vector, P_PADDING, decimated_erasures, 2);
	       
	       /* take both p corrections */
	       SetPVector(rb->recovered, p_vector, p);
	       
	       idx = PToByteIndex(p, bestA); pquality += rb->recovered[idx] == rb->reference[idx];
	       idx = PToByteIndex(p, bestB); pquality += rb->recovered[idx] == rb->reference[idx];
	       
	       byteBadProbACK[1][bestCompFound & 0x03][((bestCompFound & 0x04) >> 2) + ((bestCompFound & 0x08) >> 2)][!(pquality == 2)]++;
	       
	       if(bestCompFound & 0x01 && !(bestCompFound & 0x04) && bestQ1_ACK)
	       {
		  idx = QToByteIndex(bestQ1, bestQSol1);
		  rb->recovered[idx] = pq_sector[bestA][bestQSol1 * 2 + 1];
		  
		  q_failures++; q_err += 2; q_corrected = 2;
	       }
	       
	       /* take second p correction if it was a two erasure correction */
	       if(bestCompFound & 0x02 && !(bestCompFound & 0x08) && bestQ2_ACK)
	       {
		  idx = QToByteIndex(bestQ2, bestQSol2);
		  rb->recovered[idx] = pq_sector[bestB][bestQSol2 * 2 + 1];
		  
		  q_failures++; q_err += 2; q_corrected = 2;
	       }
	       
	       p_corrected += 2;
	    }
	 }
      }
      
      /* See if there was an improvement */
      
#ifdef DEBUG_ACK_HEURISTIC
      printf("AH L-EC: iteration %d\n", iteration); 
      printf("      Q-f/c/e + d: %2d/%2d/%2d\n", q_failures, q_corrected, q_err);
      printf("      P-f/c/e + d: %2d/%2d/%2d\n", p_failures, p_corrected, p_err);
#endif
      
      if(p_failures + p_err + q_failures + q_err == 0) break;
      if(last_p_err <= p_err && last_q_err <= q_err && last_p_failures <= p_failures && last_q_failures <= q_failures) break;
      
      if(iteration > N_P_VECTORS + N_Q_VECTORS) break;
      if(p_failures == 0)
      {
	 if(CheckEDC(rb->recovered, rb->xaMode)) break;
      }
      
      last_p_err = p_err;
      last_q_err = q_err;
      last_p_failures = p_failures;
      last_q_failures = q_failures;
      iteration++;
   }

   return 1;
}

int BruteForceSearchPlausibleSector(RawBuffer *rb)
{
   unsigned char p_vector[26];
   unsigned char q_vector[45];
   unsigned char cp_vector[26];
   unsigned char cq_vector[45];
   int ignore[2];
   int p_failures, q_failures;
   int p_corrected, q_corrected;
   int p,q;
   int iteration=1;
   int p_err, q_err;
   int last_p_err = N_P_VECTORS;
   int last_q_err = N_Q_VECTORS;
   int last_p_failures = N_P_VECTORS;
   int last_q_failures = N_Q_VECTORS;
   int err;
   
   unsigned char  zList[45][256]; /* stores different bytes which were read for each position in a sector */   
   unsigned char czList[45];	   /* counts different bytes which were read for each position in a sector */
   int zStack[45], pzStack;

   /* Re-Initialize sector */     
   InitializeCDFrame(rb->recovered, rb->lba, rb->xaMode, 1);

   for(; ;) /* iterate over P- and Q-Parity until failures converge */
   {   
      p_failures = q_failures = 0;
      p_corrected = q_corrected = 0;
      p_err = q_err = 0;

      /* Perform Q-Parity error correction */
      for(q = 0; q < N_Q_VECTORS; q++)
      {  
	 int referr;
	 /* Check whether Q is correct. */
	 GetQVector(rb->recovered, q_vector, q);
	 referr = DecodePQ(rb->rt, q_vector, Q_PADDING, ignore, 0);

	 /* If it is not correct. */
	 if(referr == 1 || referr < 0)
	 {	        
	    int a, b, c, complexity = 1;
	    
	    if(referr  < 0) { q_failures++; q_err += 2; }
	    if(referr == 1) {				q_err++;    }

	    /* generate zList */
	    for(a = 0; a < 45; a++)
	    {			
	       czList[a] = 0;					
	       
	       if(a < 45 - 2)
	       {
		  int p, ppos;
		  int idx = QToByteIndex(q, a); 
		  ByteIndexToP(idx, &p, &ppos);
		  
		  GetPVector(rb->recovered, p_vector, p);
		  err = DecodePQ(rb->rt, p_vector, P_PADDING, ignore, 0);
		  if(!err) 
		  {
		     zList[a][czList[a]] = p_vector[ppos];
		     czList[a]++;
		     continue;
		  }
	       }
	       
	       for(b = 0; b < rb->samplesRead; b++)
	       {
		  int found = 0;
		  int idx = QToByteIndex(q, a); 
		  
		  for(c = 0; c < czList[a]; c++)
		  {
		     if(rb->rawBuf[b][idx] == zList[a][c]) { found = 1; break; }
		  }
		  
		  if(!found)
		  {
		     zList[a][czList[a]] = rb->rawBuf[b][idx];
		     czList[a]++;
		  }
	       }
	       
	       complexity *= czList[a];
	       if(complexity > 65536) break;
	    }

	    /* do not let the enumeration get too complex */
	    if(complexity > 65536) continue;
	    /* no degrees of freedom */
	    if(complexity == 1) continue; 

	    memset(zStack, 0, 45 * sizeof(int));
	    while(1)
	    {
	       for(a = 0; a < 45; a++)
	       {
		  cq_vector[a] = zList[a][zStack[a]];
	       }					
	       
	       err = DecodePQ(rb->rt, cq_vector, Q_PADDING, ignore, 0);
	       if((referr < 0 && err >= 0) || (referr == 1 && err == 0)) 
	       {
		  SetQVector(rb->recovered, cq_vector, q);
		  q_corrected++;

		  referr = err;
		  if(referr == 0) break;
	       }
	       
	       pzStack = 0;
	       while(pzStack < 45)
	       {
		  zStack[pzStack]++;
		  if(zStack[pzStack] == czList[pzStack]) 
		  {
		     zStack[pzStack] = 0;
		     pzStack++;
		  }
		  else break;
	       }
	       
	       if(pzStack == 45) break;
	    }
	 }
      }

      /* Perform P-Parity error correction */
      for(p = 0; p < N_P_VECTORS; p++)
      {  
	 int referr;
	 /* Check whether Q is correct. */
	 GetPVector(rb->recovered, p_vector, p);
	 referr = DecodePQ(rb->rt, p_vector, P_PADDING, ignore, 0);

	 /* If it is not correct. */
	 if(referr == 1 || referr < 0)
	 {	        
	    int a, b, c, complexity = 1;
	    
	    if(referr  < 0) { p_failures++; p_err += 2; }
	    if(referr == 1) { p_err++;    }
	    
	    /* generate zList */
	    for(a = 0; a < 26; a++)
	    {			
	       czList[a] = 0;					
	       
	       {
		  int q, qpos;
		  int idx = PToByteIndex(p, a); 
		  ByteIndexToQ(idx, &q, &qpos);
		  
		  GetQVector(rb->recovered, q_vector, q);
		  err = DecodePQ(rb->rt, q_vector, Q_PADDING, ignore, 0);
		  if(!err) 
		  {
		     zList[a][czList[a]] = q_vector[qpos];
		     czList[a]++;
		     continue;
		  }
	       }

	       for(b = 0; b < rb->samplesRead; b++)
	       {
		  int found = 0;
		  int idx = PToByteIndex(p, a); 
		  
		  for(c = 0; c < czList[a]; c++)
		  {
		     if(rb->rawBuf[b][idx] == zList[a][c]) { found = 1; break; }
		  }
		  
		  if(!found)
		  {
		     zList[a][czList[a]] = rb->rawBuf[b][idx];
		     czList[a]++;
		  }
	       }
	       
	       complexity *= czList[a];
	       if(complexity > 65536) break;
	    }
	    
	    /* do not let the enumeration get too complex */
	    if(complexity > 65536) continue;
	    /* no degrees of freedom */
	    if(complexity == 1) continue; 
	    
	    memset(zStack, 0, 26 * sizeof(int));
	    while(1)
	    {
	       for(a = 0; a < 26; a++)
	       {
		  cp_vector[a] = zList[a][zStack[a]];
	       }					

	       err = DecodePQ(rb->rt, cp_vector, P_PADDING, ignore, 0);
	       if((referr < 0 && err >= 0) || (referr == 1 && err == 0)) 
	       {
		  SetPVector(rb->recovered, cp_vector, p);
		  p_corrected++;
		  
		  referr = err;
		  if(referr == 0) break;
	       }
	       
	       pzStack = 0;
	       while(pzStack < 26)
	       {
		  zStack[pzStack]++;
		  if(zStack[pzStack] == czList[pzStack]) 
		  {
		     zStack[pzStack] = 0;
		     pzStack++;
		  }
		  else break;
	       }
	       
	       if(pzStack == 26) break;
	    }
	 }
      }

      /* See if there was an improvement */

#ifdef DEBUG_SEARCH_PLAUSIBLE
      Verbose("SPS L-EC: iteration %d\n", iteration); 
      Verbose("      Q-f/c/e + d: %2d/%2d/%2d\n", q_failures, q_corrected, q_err);
      Verbose("      P-f/c/e + d: %2d/%2d/%2d\n", p_failures, p_corrected, p_err);
#endif

      if(p_failures + p_err + q_failures + q_err == 0) break;
      if(last_p_err <= p_err && last_q_err <= q_err && last_p_failures <= p_failures && last_q_failures <= q_failures) break;

      if(iteration > N_P_VECTORS + N_Q_VECTORS) break;
      if(p_failures == 0)
      {
	 if(CheckEDC(rb->recovered, rb->xaMode)) break;
      }

      last_p_err = p_err;
      last_q_err = q_err;
      last_p_failures = p_failures;
      last_q_failures = q_failures;
      iteration++;
   }	
   
   return 1;
}
