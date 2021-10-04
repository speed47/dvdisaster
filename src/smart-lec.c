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

/*** src type: no GUI code ***/

#include "dvdisaster.h"

//#define LOCAL_ONLY 1
//#define PRINT_STEPS 1

#define VERBOSE 1
#ifdef VERBOSE
  #define verbose(format,...) printf(format, __VA_ARGS__)
#else
  #define verbose(format,...)
#endif

/***
 *** Weights for the evaluation heuristics
 ***/

/* Weights for self-correcting vectors */

/* Vector corrected itself */
#define BONUS_SELF_CORRECT     50

/* Vector corrected itself and improved another one */
#define BONUS_CROSSED_CORRECT 100

/* Vector corrected itself but destroyed another one */
#define MALUS_CROSSED_DESTROY 100

/* Weights for swapping vector alternatives */

/* Vector was swapped with an accepting one */
#define BONUS_SWAPPED_WITH_BETTER_VECTOR 50

/* Swapped vector improved another vector */
#define BONUS_SWAP_IMPROVED_CROSSING 100

/* Swapped vector destroyed another vector */
#define MALUS_SWAP_DESTROYED_CROSSING 100

/* Swapped for improvement bonus */
#define SWAPPED_WITH_IMPROVEMENT_BONUS 95 

/* Swapped for improvement change crossing vector for worse */
#define SWAPPED_WITH_IMPROVEMENT_MALUS 50 

/* Indirect improvement bonus for initial P */
#define INDIRECT_IMPROVEMENT_BONUS_FOR_P 90
#define INDIRECT_IMPROVEMENT_MALUS_FOR_P 40

/* Indirect improvement bonus for inner P2 and Q*/
#define INDIRECT_IMPROVEMENT_BONUS_FOR_P2 50
#define INDIRECT_IMPROVEMENT_BONUS_FOR_Q 90

/* Penalty for reaching an already existing solution */
#define CYCLE_PENALTY 20

/***
 *** Local data struct
 ***/

enum
{  ITERATION_AUTORUN,  
   ITERATION_PICK_BEST_SECTOR,
   ITERATION_RUN_HEURISTICS
};

typedef struct _sh_context
{  RawBuffer *rb;
   unsigned char *visited;        /* md5sums of already tried solutions */
   int *penalty;                  /* penalty for allready tried solution */
   int visitedMax;
   int visitedCnt;
   int iteration;                 /* for iterative running within the editor */
   char msg[SMART_LEC_MESSAGE_SIZE]; /* diagnostic output */

   unsigned char bestFrame[MAX_RAW_TRANSFER_SIZE];
   int bestBonus;
   int bestMalus;

   int pState[N_P_VECTORS];
   int pPosition[N_P_VECTORS];    /* position of erroneous byte in P vector */
   int pValue[N_P_VECTORS];       /* value of corrected byte */
   int crossedQ[N_P_VECTORS];     /* q vector affected by corrected byte */
   int crossedQIdx[N_P_VECTORS];  /* q vector affected by corrected byte */
   int hitByQ[N_P_VECTORS];       /* hit by how many q vectors? */

   int qState[N_Q_VECTORS];
   int qPosition[N_Q_VECTORS];    /* position of erroneous byte in Q vector */
   int qValue[N_Q_VECTORS];       /* value of corrected byte */
   int crossedP[N_Q_VECTORS];     /* p vector affected by corrected byte */
   int crossedPIdx[N_Q_VECTORS];  /* p vector affected by corrected byte */
   int hitByP[N_Q_VECTORS];       /* hit by how many p vectors? */

} sh_context;

static sh_context* create_sh_context(RawBuffer *rb)
{  sh_context *shc = g_malloc0(sizeof(sh_context));

   shc->rb = rb;
   shc->visited = g_malloc(16*4);
   shc->penalty = g_malloc(sizeof(int)*4);
   shc->visitedMax = 4;
   shc->visitedCnt = 0;

   return shc;
}

static void free_sh_context(sh_context *shc)
{  
   g_free(shc->visited);
   g_free(shc->penalty);
   g_free(shc);
}

/*
 * Predicate for recognizing a better solution
 */

int found_better_solution(sh_context *shc, int bonus, int malus)
{
   if(bonus < 0 || malus > shc->bestMalus)
      return FALSE;

   if(   (malus < shc->bestMalus  && bonus > 0)
      || (malus == shc->bestMalus && bonus > shc->bestBonus))
   {  shc->bestBonus = bonus;
      shc->bestMalus = malus;
      verbose("pick %d/%d\n",bonus,malus);
      return TRUE;
   }

   return FALSE;
}

/*
 * Add a frame to the visited list
 */

static void push_frame(sh_context *shc, unsigned char *frame)
{  MD5Context ctxt;

   if(shc->visitedCnt >= shc->visitedMax)
   {  shc->visitedMax *= 2;
      shc->visited = g_realloc(shc->visited, 16*shc->visitedMax);
      shc->penalty = g_realloc(shc->penalty, sizeof(int)*shc->visitedMax);
   }

   MD5Init(&ctxt);
   MD5Update(&ctxt, frame, shc->rb->sampleSize);
   MD5Final(&shc->visited[16*shc->visitedCnt], &ctxt);
   shc->penalty[shc->visitedCnt] = 0;
   shc->visitedCnt++;
   printf("pushed\n");
}

/*
 * Check whether the solution was found before
 */

static int frame_visited(sh_context *shc, unsigned char *frame)
{  MD5Context ctxt;
   unsigned char md5sum[16];
   int i,n;

   MD5Init(&ctxt);
   MD5Update(&ctxt, frame, shc->rb->sampleSize);
   MD5Final(md5sum, &ctxt);

   n = shc->visitedCnt*16;
   for(i=0; i<n; i+=16)
      if(!memcmp(md5sum, &shc->visited[i], 16))
	 return i/16+1;

   return 0;
}

/*
 * Calculate a penalty for already seen solution
 */

static int cycle_penalty(sh_context *shc, unsigned char *frame)
{  int idx = frame_visited(shc, frame);

   if(!idx) return 0;

   idx--;
   shc->penalty[idx] += CYCLE_PENALTY;

   return shc->penalty[idx];
}

/*
 * Check whether the frame has been corrected
 */

static int frame_corrected(RawBuffer *rb)
{
   if(   CheckEDC(rb->recovered, rb->xaMode)
      && CheckMSF(rb->recovered, rb->lba, STRICT_MSF_CHECK))
      return TRUE;

   return FALSE;
}

/*
 * Try lec with given p/q vectors without erasures.
 * Returns position of error if err==1
 */

static int decode_p(RawBuffer *rb, unsigned char* p_vector, int *pos)
{  int eras[2], err;
   unsigned char backup[P_VECTOR_SIZE];

   memcpy(backup, p_vector, P_VECTOR_SIZE);

   err = DecodePQ(rb->rt, p_vector, P_PADDING, eras, 0);

   if(err == 0)
      return err;

   if(err == 1)
   {  int i;

      for(i=0; i<P_VECTOR_SIZE; i++)
	 if(backup[i] != p_vector[i] && i != eras[0])
	 {  printf("POSITION FAILURE\n");
	    exit(0);
	 }
      *pos = eras[0];
      return err;
   }

   return 2;
}

static int decode_q(RawBuffer *rb, unsigned char* q_vector, int *pos)
{  int eras[2], err;
   unsigned char backup[Q_VECTOR_SIZE];

   memcpy(backup, q_vector, Q_VECTOR_SIZE);

   err = DecodePQ(rb->rt, q_vector, Q_PADDING, eras, 0);

   if(err == 0)
     return err;

   if(err == 1)
   {  int i;

      for(i=0; i<Q_VECTOR_SIZE; i++)
	 if(backup[i] != q_vector[i] && i != eras[0])
	 {  printf("POSITION FAILURE\n");
	    exit(0);
	 }
      *pos = eras[0];
      return err;
   }

   return 2;
}

/***
 *** Pick the best P/Q vectors.
 ***
 * Find all different versions of P/Q vectors in the samples
 * which evaluate to 0 or 1 in the error correction.
 * In the latter case, the corrected version is stored.
 */

void CollectGoodVectors(RawBuffer *rb)
{  unsigned char vector[Q_VECTOR_SIZE];
   int eras[2];
   int sample_idx;
   int err;
   int i,p,q;

   if(!rb->samplesRead)  /* We need at least one sample */
      return;

   sample_idx = rb->samplesRead-1;

   /* Find all P vectors which are accepted by the error correction */

   for(p=0; p<N_P_VECTORS; p++)
   {  int found = FALSE;
      int last_p = rb->pn[p];

      GetPVector(rb->rawBuf[sample_idx], vector, p);
      err = DecodePQ(rb->rt, vector, P_PADDING, eras, 0);

      if(err != 0 && err != 1) 
	 continue;

      for(i=0; i<last_p; i++)
	 if(!memcmp(rb->pList[p][i], vector, P_VECTOR_SIZE))
	 {  found = TRUE;
	    break;
	 }

      if(!found)
      {  memcpy(rb->pList[p][last_p], vector, P_VECTOR_SIZE);
	 rb->pn[p]++;
      }
   }

   /* Find all Q vectors which are accepted by the error correction */

   for(q=0; q<N_Q_VECTORS; q++)
   {  int found = FALSE;
      int last_q = rb->qn[q];

      GetQVector(rb->rawBuf[sample_idx], vector, q);
      err = DecodePQ(rb->rt, vector, Q_PADDING, eras, 0);

      if(err != 0 && err != 1) 
	 continue;

      for(i=0; i<last_q; i++)
	 if(!memcmp(rb->qList[q][i], vector, Q_VECTOR_SIZE))
	 {  found = TRUE;
	    break;
	 }

      if(!found)
      {  memcpy(rb->qList[q][last_q], vector, Q_VECTOR_SIZE);
	 rb->qn[q]++;
      }
   }
}

/*
 * Debugging function
 */

void PrintPQStats(RawBuffer *rb)
{  int i;

   PrintLog("PQ vector good variants from read samples:\n");

   for(i=0; i<N_P_VECTORS; i++)
      PrintLog("P%02d: %02d\n", i, rb->pn[i]);

   for(i=0; i<N_Q_VECTORS; i++)
      PrintLog("Q%02d: %02d\n", i, rb->qn[i]);
}

/***
 *** Find correctable vectors and their interrelationship 
 ***/

static void update_pq_state(sh_context *shc)
{  RawBuffer *rb = shc->rb;
   unsigned char vector[Q_VECTOR_SIZE];
   int eras[2],err,i;
   int crossed,crossed_idx;

   memset(shc->crossedP, 0, sizeof(shc->crossedP));
   memset(shc->crossedQ, 0, sizeof(shc->crossedQ));
   memset(shc->crossedPIdx, 0, sizeof(shc->crossedPIdx));
   memset(shc->crossedQIdx, 0, sizeof(shc->crossedQIdx));
   memset(shc->hitByP, 0, sizeof(shc->hitByP));
   memset(shc->hitByQ, 0, sizeof(shc->hitByQ));

   for(i=0; i<N_P_VECTORS; i++)
   {  GetPVector(rb->recovered, vector, i);
      err = DecodePQ(rb->rt, vector, P_PADDING, eras, 0);

      switch(err)
      {  case 0: 
	    shc->pState[i] = 0; 
	    break;
	 case 1:
	    shc->pState[i]    = 1;
	    shc->pPosition[i] = eras[0];
	    shc->pValue[i]    = vector[eras[0]];
	    ByteIndexToQ(PToByteIndex(i, eras[0]), &crossed, &crossed_idx);
	    shc->crossedQ[i]  = crossed;
	    shc->crossedQIdx[i]  = crossed_idx;
	    shc->hitByP[crossed]++;
	    break;
	 default:
	    shc->pState[i] = 2;
	    break;
      }
   }

   for(i=0; i<N_Q_VECTORS; i++)
   {  GetQVector(rb->recovered, vector, i);
      err = DecodePQ(rb->rt, vector, Q_PADDING, eras, 0);

      switch(err)
      {  case 0: 
	    shc->qState[i] = 0; 
	    break;
	 case 1:
	    shc->qState[i]    = 1;
	    shc->qPosition[i] = eras[0];
	    shc->qValue[i]    = vector[eras[0]];
	    ByteIndexToP(QToByteIndex(i, eras[0]), &crossed, &crossed_idx);
	    shc->crossedP[i]  = crossed;
	    shc->crossedPIdx[i]  = crossed_idx;
	    shc->hitByQ[crossed]++;
	    break;
	 default:
	    shc->qState[i] = 2;
	    break;
      }
   }
}

static void print_pq_state(sh_context *shc)
{  int i;

   verbose("%s", "PQ states: \n");

   for(i=0; i<N_P_VECTORS; i++)
   {  if(shc->pState[i] == 1)
	 verbose("P %02d: correctable, crosses Q %02d\n", i, shc->crossedQ[i]);

      if(shc->pState[i] == 2)
	 verbose("P %02d: failure\n", i);
   }

   for(i=0; i<N_Q_VECTORS; i++)
   {
      if(shc->qState[i] == 1)
	 verbose("Q %02d: correctable, crosses P %02d\n", i, shc->crossedP[i]);

      if(shc->qState[i] == 2)
	 verbose("Q %02d: failure\n", i);
   }
}

/***
 *** Track changes of one byte recursively
 ***/

static void recursive_q_correction(RawBuffer*, unsigned char*, int, int, int*, int*); 

static void recursive_p_correction(RawBuffer *rb, unsigned char *frame, 
				   int byte_idx, int new_byte, 
				   int *better, int *worse)
{  unsigned char p_vector[P_VECTOR_SIZE];
   int old_err, new_err, pos;
   int p,p_idx;

   ByteIndexToP(byte_idx, &p, &p_idx); 

   GetPVector(frame, p_vector, p);
   old_err = decode_p(rb, p_vector, &pos);

   GetPVector(frame, p_vector, p);
   p_vector[p_idx] = new_byte;
   new_err = decode_p(rb, p_vector, &pos);


   if(new_err > old_err)
   {  *worse += (new_err - old_err);
      verbose("rec_p_corr(%d): %d > %d, stopping\n", p, new_err, old_err);
      return;
   }

   /* Return if no improvent occurred.
      Exception: If P goes from 1->1 it might have made up its mind on
      which byte to correct. */

   if(new_err == old_err && new_err != 1)
   {  verbose("rec_p_corr(%d): %d == %d, no improvement\n", p, new_err, old_err);
      return;
   }

   /* P corrected another byte; 
      update frame and continue with crossed Q */
   
   frame[byte_idx] = new_byte;
   (*better)++;
   verbose("rec_p_corr(%d): %d < %d, continue\n", p, new_err, old_err);

   recursive_q_correction(rb, frame, PToByteIndex(p, pos), p_vector[pos], better, worse);
}

static void recursive_q_correction(RawBuffer *rb, unsigned char *frame, 
				   int byte_idx, int new_byte, 
				   int *better, int *worse)
{  unsigned char q_vector[Q_VECTOR_SIZE];
   int old_err, new_err, pos;
   int q,q_idx;

   ByteIndexToQ(byte_idx, &q, &q_idx); 

   GetQVector(frame, q_vector, q);
   old_err = decode_q(rb, q_vector, &pos);

   GetQVector(frame, q_vector, q);
   q_vector[q_idx] = new_byte;
   new_err = decode_q(rb, q_vector, &pos);

   if(new_err > old_err)
   {  *worse += (new_err - old_err);
      verbose("rec_q_corr(%d): %d > %d, stopping\n", q, new_err, old_err);
      return;
   }

   /* Return if no improvent occurred.
      Exception: If Q goes from 1->1 it might have made up its mind on
      which byte to correct. */

   if(new_err == old_err && new_err != 1)
   {  verbose("rec_q_corr(%d): %d == %d, no improvement\n", q, new_err, old_err);
      return;
   }

   /* Q corrected another byte; 
      update frame and continue with crossed P */
   
   frame[byte_idx] = new_byte;
   (*better)++;
   verbose("rec_q_corr(%d): %d < %d, continue\n", q, new_err, old_err);

   recursive_p_correction(rb, frame, QToByteIndex(q, pos), q_vector[pos], better, worse);
}

/***
 *** See if multiple vectors can correct a crossed vector
 ***
 * Strategy note: If the crossed vector becomes correctable,
 * e.g. it evaluates to err=1, we could try to propagate
 * the one-byte correction further (and possible gain a good
 * scoring). This might have the disadvantage of masking off
 * another crossing correction, though
 * Example: Q is crossed by 4 P vectors; if all 4 P are corrected,
 * Q will evaluate to zero errors, also. That implies that applying
 * all combinations of 3 P will leave Q with err=1, and the recursive
 * propagation might give it a higher score than the solution with
 * 4 corrected P. Therefore we might miss picking the better case
 * using all 4 corrected P. So we avoid doing the recursive
 * propagation here in order to max out the crossing vectors.
 */

/*
 * Try to correct a q vector from crossing p vectors 
 */

static void many_p_correct_one_q(sh_context *shc)
{  RawBuffer *rb = shc->rb;
   int crossing_p[N_P_VECTORS];
   int selection[N_P_VECTORS];
   int i,p,q;
   int n_p=0;

   /* Determine all P vectors which cross the Q */

   for(q=0; q<N_Q_VECTORS; q++)
   {  int n_iterations=1;

      if(shc->hitByP[q] < 2)
	 continue;

      verbose("Q%02d is crossed by: ",q);
      n_p = 0;
      for(p=0; p<N_P_VECTORS; p++)
	 if(shc->crossedQ[p] == q)
	 {  crossing_p[n_p] = p;
	    selection[n_p] = 1;
	    n_p++;
	    n_iterations *= 2;
	    verbose("P%02d ", p);
	 }

      n_iterations--;  /* number of combinations to test */
      verbose(" (%d combinations)\n", n_iterations);
      if(n_iterations <= 0)
	 continue;

      /* Enumerate all combinations by using <selection> as a binary counter */

      verbose("Trying all combinations for Q%02d\n", q);

      while(n_iterations--)
      {  unsigned char scratch[MAX_RAW_TRANSFER_SIZE];
	 unsigned char vector[Q_VECTOR_SIZE];
	 int q_before, q_after, pos; 
	 int bonus = 0;
	 int malus = 0;
	 int index = 0;
	 int better = 0;
	 int worse = 0;

	 for(p=n_p-1; p>=0; p--)
	    verbose("%d", selection[p]);
      
	 /* evaluate the vector combination */

	 GetQVector(rb->recovered, vector, q);
	 q_before = decode_q(rb, vector, &pos);

	 GetQVector(rb->recovered, vector, q);
	 for(i=0; i<n_p; i++)
	 {  if(selection[i])
	    {  p = crossing_p[i];
	       bonus += BONUS_SELF_CORRECT;
	       vector[shc->crossedQIdx[p]] = shc->pValue[p];
	    }
	 }

	 q_after = decode_q(rb, vector, &pos);

	 if(q_after <= q_before)
	      bonus += (q_before-q_after)*BONUS_CROSSED_CORRECT;
	 else malus += (q_after-q_before)*MALUS_CROSSED_DESTROY;

	 verbose(" Q:%d->%d; bonus: %3d malus: %3d\n",
		q_before, q_after, bonus, malus);

	 /* If q becomes correctable, continue correction. */

	 if(q_after == 1)
	 {  unsigned char byte_before_corr;
	    int byte_idx;

	    byte_idx = QToByteIndex(q, pos);
	    byte_before_corr = rb->recovered[byte_idx];

	    /* Copy rb->recovered into scratch, including all bytes changed
	       in Q (there may be many, up to n_p.
	       However the byte which Q currently wants to correct is NOT
	       included in scratch so that recursive_p_correction() can
	       judge for itself. */

	    memcpy(scratch, rb->recovered, rb->sampleSize);
	    SetQVector(scratch, vector, q);
	    scratch[byte_idx] = byte_before_corr;

	    recursive_p_correction(rb, scratch, byte_idx, vector[pos], &better, &worse);
	    verbose("  recursive gain: better/worse: %d / %d\n", better, worse);

	    bonus += BONUS_CROSSED_CORRECT * better;
	    //	    malus -= MALUS_CROSSED_DESTROY; /* P went back from 1->0 */
	    malus += MALUS_CROSSED_DESTROY * worse;
	 }

	 /* Better solution found? */

	 bonus -= cycle_penalty(shc, scratch);

	 if(q_after < 2 && found_better_solution(shc, bonus, malus))
	 {  if(q_after != 1)
	    {  memcpy(shc->bestFrame, rb->recovered, rb->sampleSize);
	       SetQVector(shc->bestFrame, vector, q);
	    }
	    else memcpy(shc->bestFrame, scratch, rb->sampleSize);

	    snprintf(shc->msg, SMART_LEC_MESSAGE_SIZE,
		     "Q%02d improved from %d->%d by crossing P vectors (bonus %d/ malus %d).",
		     q,q_before, q_after, bonus, malus);
	 }

	 /* binary decrement on selection[] */

	 while(selection[index] != 1 && index < n_p)
	 {  index++;
	    selection[index-1] = 1;
	 }
	 if(index >= n_p) break;
	 selection[index]=0;
      }
   }
}

/*
 * Try to correct a p vector from crossing q vectors 
 */

static void many_q_correct_one_p(sh_context *shc)
{  RawBuffer *rb = shc->rb;
   int crossing_q[N_Q_VECTORS];
   int selection[N_Q_VECTORS];
   int i,p,q;
   int n_q=0;

   /* Determine all Q vectors which cross the P */

   for(p=0; p<N_P_VECTORS; p++)
   {  int n_iterations=1;

      if(shc->hitByQ[p] < 1)
	 continue;

      verbose("P%02d is crossed by: ",p);
      n_q = 0;
      for(q=0; q<N_Q_VECTORS; q++)
	 if(shc->crossedP[q] == p)
	 {  crossing_q[n_q] = q;
	    selection[n_q] = 1;
	    n_q++;
	    n_iterations *= 2;
	    verbose("Q%02d ", q);
	 }

      n_iterations--;  /* number of combinations to test */
      verbose("(%d combinations)\n", n_iterations);
      if(n_iterations <= 0)
	 continue;

      /* Enumerate all combinations by using <selection> as a binary counter */

      verbose("Trying all combinations for P%02d\n", p);

      while(n_iterations--)
      {  unsigned char scratch[MAX_RAW_TRANSFER_SIZE];
	 unsigned char vector[Q_VECTOR_SIZE];
	 int p_before, p_after, pos; 
	 int bonus = 0;
	 int malus = 0;
	 int index = 0;
	 int better = 0;
	 int worse = 0;

	 for(q=n_q-1; q>=0; q--)
	    verbose("%d", selection[q]);
      
	 /* evaluate the vector combination */

	 GetPVector(rb->recovered, vector, p);
	 p_before = decode_p(rb, vector, &pos);

	 GetPVector(rb->recovered, vector, p);
	 for(i=0; i<n_q; i++)
	 {  if(selection[i])
	    {  q = crossing_q[i];
	       bonus += BONUS_SELF_CORRECT;
	       vector[shc->crossedPIdx[q]] = shc->qValue[q];
	    }
	 }

	 p_after = decode_p(rb, vector, &pos);

	 if(p_after <= p_before)
	      bonus += (p_before-p_after)*BONUS_CROSSED_CORRECT;
	 else malus += (p_after-p_before)*MALUS_CROSSED_DESTROY;

	 verbose(" P:%d->%d; bonus: %3d malus: %3d\n",
		p_before, p_after, bonus, malus);

	 /* If p becomes correctable, continue correction. */

	 if(p_after == 1)
	 {  unsigned char byte_before_corr;
	    int byte_idx;
	    
	    byte_idx = PToByteIndex(p, pos);
	    byte_before_corr = rb->recovered[byte_idx];

	    /* Copy rb->recovered into scratch, including all bytes changed
	       in P (there may be many, up to n_q.
	       However the byte which P currently wants to correct is NOT
	       included in scratch so that recursive_q_correction() can
	       judge for itself. */

	    memcpy(scratch, rb->recovered, rb->sampleSize);
	    SetPVector(scratch, vector, p);
	    scratch[byte_idx] = byte_before_corr;

	    recursive_q_correction(rb, scratch, byte_idx, vector[pos], &better, &worse);
	    verbose("  recursive gain: better/worse: %d / %d\n", better, worse);

	    bonus += BONUS_CROSSED_CORRECT * better;
	    //	    malus -= MALUS_CROSSED_DESTROY; /* P went back from 1->0 */
	    malus += MALUS_CROSSED_DESTROY * worse;
	 }

	 /* Better solution found? */

	 bonus -= cycle_penalty(shc, scratch);

	 if(p_after < 2 && found_better_solution(shc, bonus, malus))
	 {  if(p_after != 1) 
	    {    memcpy(shc->bestFrame, rb->recovered, rb->sampleSize);
	         SetPVector(shc->bestFrame, vector, p);
	    }
	    else memcpy(shc->bestFrame, scratch, rb->sampleSize);

	    snprintf(shc->msg, SMART_LEC_MESSAGE_SIZE,
		     "P%02d improved from %d->%d by crossing Q vectors (bonus %d/ malus %d).",
		     p,p_before, p_after, bonus, malus);
	 }
	 verbose("%d\n", rb->sampleSize);

	 /* binary decrement on selection[] */

	 while(selection[index] != 1 && index < n_q)
	 {  selection[index] = 1;
	    index++;
	 }
	 selection[index]=0;
      }
   }
}


/***
 *** See if replacing a vector with another accepting version improves anything
 ***/

static void evaluate_new_frame(sh_context *shc, unsigned char *new,
			       int *better, int *worse)
{  RawBuffer *rb = shc->rb;
   unsigned char vector[Q_VECTOR_SIZE];
   int p,q;
   int err, pos;

   for(p=0; p<N_P_VECTORS; p++)
   {  GetPVector(new, vector, p);
      err = decode_p(rb, vector, &pos);

      if(err > shc->pState[p]) *worse  += err - shc->pState[p];
      else                     *better += shc->pState[p] - err;
   }

   for(q=0; q<N_Q_VECTORS; q++)
   {  GetQVector(new, vector, q);
      err = decode_q(rb, vector, &pos);

      if(err > shc->qState[q]) *worse  += err - shc->qState[q];
      else                     *better += shc->qState[q] - err;
   }
}

static void try_alternative_vectors(sh_context *shc)
{  RawBuffer *rb = shc->rb;
   unsigned char scratch[MAX_RAW_TRANSFER_SIZE];
   int better, worse;
   int i,p,q;

   for(p=0; p<N_P_VECTORS; p++)
   {  if(shc->pState[p] == 0)
	 continue;

      for(i=0; i<rb->pn[p]; i++)
      {	 int bonus;
	 int malus;

         memcpy(scratch, rb->recovered, rb->sampleSize);
	 SetPVector(scratch, rb->pList[p][i], p);
	
	 better = -shc->pState[p];  /* We replaced damaged P with an accepting P */
	 worse  = 0;
	 evaluate_new_frame(shc, scratch, &better, &worse);
	 bonus =   BONUS_SWAPPED_WITH_BETTER_VECTOR*2
	         + BONUS_SWAP_IMPROVED_CROSSING*better;
	 malus = MALUS_SWAP_DESTROYED_CROSSING*worse;

	 bonus -= cycle_penalty(shc, scratch);

	 verbose("Trying P%02d, variant %d: %d better, %d worse (%d/%d)\n", 
		p, i, better, worse, bonus, malus);

	 /* This is maybe too restrictive? */

	 if(malus > 0)
	    continue;

	 if(found_better_solution(shc, bonus, malus))
	 {  memcpy(shc->bestFrame, scratch, rb->sampleSize);

	    snprintf(shc->msg, SMART_LEC_MESSAGE_SIZE,
		     "Found alternative vector for P%02d (bonus %d/ malus %d).",
		     p, bonus, malus);
	 }
      }
   }

   for(q=0; q<N_Q_VECTORS; q++)
   {  if(shc->qState[q] == 0)
	 continue;

      for(i=0; i<rb->qn[q]; i++)
      {	 int bonus;
	 int malus;

         memcpy(scratch, rb->recovered, rb->sampleSize);
	 SetQVector(scratch, rb->qList[q][i], q);
	
	 better = -shc->qState[q];  /* We replaced damaged Q with an accepting Q */
	 worse  = 0;
	 evaluate_new_frame(shc, scratch, &better, &worse);
	 bonus =   BONUS_SWAPPED_WITH_BETTER_VECTOR*2
	         + BONUS_SWAP_IMPROVED_CROSSING*better;
	 malus = MALUS_SWAP_DESTROYED_CROSSING*worse;

	 bonus -= cycle_penalty(shc, scratch);

	 verbose("Trying Q%02d, variant %d: %d better, %d worse (%d/%d)\n", 
		q, i, better, worse, bonus, malus);

	 /* This is maybe too restrictive? */

	 if(malus > 0)
	    continue;

	 if(found_better_solution(shc, bonus, malus))
	 {  memcpy(shc->bestFrame, scratch, rb->sampleSize);
	    snprintf(shc->msg, SMART_LEC_MESSAGE_SIZE,
		     "Found alternative vector for Q%02d (bonus %d/ malus %d).",
		     q, bonus, malus);
	 }
      }
   }
}
   
/***
 *** See if swapping some bytes from alternative vectors improves something
 ***/

static void try_alternative_crossing_bytes(sh_context *shc)
{  RawBuffer *rb = shc->rb;
   unsigned char scratch[MAX_RAW_TRANSFER_SIZE];
   int better, worse;
   int i,j,p,q;

   for(p=0; p<N_P_VECTORS; p++)
   {  unsigned char current_p[P_VECTOR_SIZE];

      if(shc->pState[p] == 0)
	 continue;

      GetPVector(rb->recovered, current_p, p);

      for(i=0; i<rb->pn[p]; i++)
      {  unsigned char *alternative_p = rb->pList[p][i];

	 /* If the alternative vector differs in byte j,
	    see if swapping the current byte with byte j improves
	    the crossed vector. */

	 for(j=0; j<P_VECTOR_SIZE; j++)
	 {  if(current_p[j] != alternative_p[j])
	    {  int byte_idx, crossed, crossed_idx;

	       byte_idx = PToByteIndex(p, j);
	       ByteIndexToQ(byte_idx, &crossed, &crossed_idx);
	       if(shc->qState[crossed] > 0)
	       {  better = worse = 0;
		  memcpy(scratch, rb->recovered, rb->sampleSize);
		  recursive_q_correction(rb, scratch, byte_idx, alternative_p[j],
					 &better, &worse);

		  if(better > 0)
		  {  int bonus, malus;

		     verbose("Q%02d improved by swapping %d in P%02d, %d/%d\n", 
			    crossed, j, p, better, worse);

		     bonus =   better*BONUS_CROSSED_CORRECT
		             - cycle_penalty(shc, scratch);
		     malus = worse*MALUS_CROSSED_DESTROY;

		     if(found_better_solution(shc, bonus, malus))
		     {  memcpy(shc->bestFrame, scratch, rb->sampleSize);

			snprintf(shc->msg, SMART_LEC_MESSAGE_SIZE,
				 "Q%02d improved by swapping %d in P%02d, %d/%d (bonus %d/ malus %d).", 
				 crossed, j, p, better, worse, bonus, malus);
		     }
		  }
	       }
	    }
	 }
      }
   }

   for(q=0; q<N_Q_VECTORS; q++)
   {  unsigned char current_q[Q_VECTOR_SIZE];

      if(shc->qState[q] == 0)
	 continue;

      GetQVector(rb->recovered, current_q, q);

      for(i=0; i<rb->qn[q]; i++)
      {  unsigned char *alternative_q = rb->qList[q][i];

	 /* If the alternative vector differs in byte j,
	    see if swapping the current byte with byte j improves
	    the crossed vector. */

	 for(j=0; j<Q_VECTOR_SIZE; j++)
	 {  if(current_q[j] != alternative_q[j])
	    {  int byte_idx, crossed, crossed_idx;

	       byte_idx = QToByteIndex(q, j);
	       ByteIndexToP(byte_idx, &crossed, &crossed_idx);
	       if(shc->pState[crossed] > 0)
	       {  better = worse = 0;
		  memcpy(scratch, rb->recovered, rb->sampleSize);
		  recursive_p_correction(rb, scratch, byte_idx, alternative_q[j],
					 &better, &worse);

		  if(better > 0)
		  {  int bonus, malus;

		     verbose("P%02d improved by swapping %d in Q%02d, %d/%d\n", 
			    crossed, j, q, better, worse);

		     bonus =   better*BONUS_CROSSED_CORRECT
			     - cycle_penalty(shc, scratch);
		     malus = worse*MALUS_CROSSED_DESTROY;

		     if(found_better_solution(shc, bonus, malus))
		     {  memcpy(shc->bestFrame, scratch, rb->sampleSize);

			snprintf(shc->msg, SMART_LEC_MESSAGE_SIZE,
				 "P%02d improved by swapping %d in Q%02d, %d/%d (bonus %d/ malus %d).", 
				 crossed, j, q, better, worse, bonus, malus);
		     }
		  }
	       }
	    }
	 }
      }
   }
}	       

/***
 *** See if a p can improve two crossing q vectors
 ***/

static void recursive_double_q_correction(RawBuffer*, unsigned char*, 
					  int, int, int, int, int*, int*);

static void recursive_double_p_correction(RawBuffer *rb, unsigned char *frame, 
				   int byte_idx1, int new_byte1, 
				   int byte_idx2, int new_byte2, 
				   int *better, int *worse)
{  unsigned char p_vector1[P_VECTOR_SIZE];
   unsigned char p_vector2[P_VECTOR_SIZE];
   int old_err1, new_err1, pos1;
   int old_err2, new_err2, pos2;
   int p1,p1_idx,p2,p2_idx;

   ByteIndexToP(byte_idx1, &p1, &p1_idx); 
   GetPVector(frame, p_vector1, p1);
   old_err1 = decode_p(rb, p_vector1, &pos1);

   GetPVector(frame, p_vector1, p1);
   p_vector1[p1_idx] = new_byte1;
   new_err1 = decode_p(rb, p_vector1, &pos1);

   ByteIndexToP(byte_idx2, &p2, &p2_idx); 
   GetPVector(frame, p_vector2, p2);
   old_err2 = decode_p(rb, p_vector2, &pos2);

   if(p1 != p2)
   {  GetPVector(frame, p_vector2, p2);
      p_vector2[p2_idx] = new_byte2;
      new_err2 = decode_p(rb, p_vector2, &pos2);
   }
   else new_err2 = old_err2 = new_err1;

   verbose("rec 2p: old %d %d, new %d %d - %d %d\n", 
	  old_err1, old_err2, new_err1, new_err2, p1, p2);

   /* Best case: both P improved */

   if(new_err1 < old_err1 && new_err2 < old_err2)
   {  frame[byte_idx1] = new_byte1;
      frame[byte_idx2] = new_byte2;
      (*better) +=(old_err1-new_err1)+(old_err2-new_err2);

      recursive_double_q_correction(rb, frame, 
				    PToByteIndex(p1, pos1), p_vector1[pos1],
				    PToByteIndex(p2, pos2), p_vector2[pos2],
				    better, worse);
      return;
   }

   /* Nothing improved at all */

   if(new_err1 >= old_err1 && new_err2 >= old_err2)
   {  (*worse)+=(new_err1-old_err1)+(new_err2-old_err2);
      return;
   }

   /* One P improved */

   if(new_err1 < old_err1)  /* first P improved */
   {  (*better) +=  old_err1 - new_err1;
      (*worse)  +=  new_err2 - old_err2;
      frame[byte_idx1] = new_byte1;
      recursive_q_correction(rb, frame, PToByteIndex(p1, pos1), p_vector1[pos1], 
			     better, worse);
      
      return;
   }
   else                     /* second P improved */
   {  (*better) +=  old_err2 - new_err2;
      (*worse)  +=  new_err1 - old_err1;
      frame[byte_idx2] = new_byte2;
      recursive_q_correction(rb, frame, PToByteIndex(p2, pos2), p_vector2[pos2], 
			     better, worse);
      
      return;
   }
}

static void recursive_double_q_correction(RawBuffer *rb, unsigned char *frame, 
				   int byte_idx1, int new_byte1, 
				   int byte_idx2, int new_byte2, 
				   int *better, int *worse)
{  unsigned char q_vector1[Q_VECTOR_SIZE];
   unsigned char q_vector2[Q_VECTOR_SIZE];
   int old_err1, new_err1, pos1;
   int old_err2, new_err2, pos2;
   int q1,q1_idx,q2,q2_idx;

   ByteIndexToQ(byte_idx1, &q1, &q1_idx); 
   GetQVector(frame, q_vector1, q1);
   old_err1 = decode_q(rb, q_vector1, &pos1);

   GetQVector(frame, q_vector1, q1);
   q_vector1[q1_idx] = new_byte1;
   new_err1 = decode_q(rb, q_vector1, &pos1);

   ByteIndexToQ(byte_idx2, &q2, &q2_idx); 
   GetQVector(frame, q_vector2, q2);
   old_err2 = decode_q(rb, q_vector2, &pos2);

   if(q1 != q2)
   {  GetQVector(frame, q_vector2, q2);
      q_vector2[q2_idx] = new_byte2;
      new_err2 = decode_q(rb, q_vector2, &pos2);
   }
   else new_err2 = old_err2 = new_err1;

   verbose("rec 2q: old %d %d, new %d %d - %d %d\n", 
	  old_err1, old_err2, new_err1, new_err2, q1, q2);

   /* Best case: both Q improved */

   if(new_err1 < old_err1 && new_err2 < old_err2)
   {  frame[byte_idx1] = new_byte1;
      frame[byte_idx2] = new_byte2;
      (*better) +=(old_err1-new_err1)+(old_err2-new_err2);

      recursive_double_p_correction(rb, frame, 
				    QToByteIndex(q1, pos1), q_vector1[pos1],
				    QToByteIndex(q2, pos2), q_vector2[pos2],
				    better, worse);
      return;
   }

   /* Nothing improved at all */

   if(new_err1 >= old_err1 && new_err2 >= old_err2)
   {  (*worse)+=(new_err1-old_err1)+(new_err2-old_err2);
      return;
   }

   /* One Q improved */

   if(new_err1 < old_err1)  /* first Q improved */
   {  (*better) +=  old_err1 - new_err1;
      (*worse)  +=  new_err2 - old_err2;
      frame[byte_idx1] = new_byte1;
      recursive_p_correction(rb, frame, QToByteIndex(q1, pos1), q_vector1[pos1], 
			     better, worse);
      
      return;
   }
   else                     /* second Q improved */
   {  (*better) +=  old_err2 - new_err2;
      (*worse)  +=  new_err1 - old_err1;
      frame[byte_idx2] = new_byte2;
      recursive_p_correction(rb, frame, QToByteIndex(q2, pos2), q_vector2[pos2], 
			     better, worse);
      
      return;
   }
}

static void find_p_with_two_erasures(sh_context *shc)
{  RawBuffer *rb = shc->rb;
   int erasure_pos[P_VECTOR_SIZE];
   int crossed_q[P_VECTOR_SIZE];
   int crossed_q_idx[P_VECTOR_SIZE];
   int n_q = 0;
   int p,q,i;
   int e1, e2;

   for(p=0; p<=N_P_VECTORS; p++)
   {  if(shc->pState[p] != 2)
	 continue;
      
      for(i=0; i<P_VECTOR_SIZE; i++)
      {  int q_idx;

	 ByteIndexToQ(PToByteIndex(p, i), &q, &q_idx);
	 if(shc->qState[q] > 0)
	 {  erasure_pos[n_q] = i;
	    crossed_q[n_q] = q;
	    crossed_q_idx[n_q] = q_idx;
	    n_q++;
	 }
      }

      if(n_q < 2) continue;
      if(n_q > 7) return;  /* too much damage */

      /* Enumerate all erasure combinations */

      for(e1=0; e1<n_q-1; e1++)
	 for(e2=e1+1; e2<n_q; e2++)
	 {  unsigned char p_vector[P_VECTOR_SIZE];
	    unsigned char scratch[MAX_RAW_TRANSFER_SIZE];
	    int better, worse;
	    int eras[2], err;

            GetPVector(rb->recovered, p_vector, p);
            eras[0] = erasure_pos[e1];
	    eras[1] = erasure_pos[e2];

	    err = DecodePQ(rb->rt, p_vector, P_PADDING, eras, 2);
	    if(err != 2) continue;

	    better = worse = 0;
	    memcpy(scratch, rb->recovered, rb->sampleSize);
	    recursive_double_q_correction(rb, scratch, 
	      QToByteIndex(crossed_q[e1],crossed_q_idx[e1]), p_vector[erasure_pos[e1]],
	      QToByteIndex(crossed_q[e2],crossed_q_idx[e2]), p_vector[erasure_pos[e2]],
					  &better, &worse);

	    if(better > 0)
	    {  int bonus, malus;

	       verbose("P%02d with erasure pos %02d/%02d: %d better, %d worse\n",
		      p,erasure_pos[e1],erasure_pos[e2],better,worse);

	       bonus =   better*BONUS_CROSSED_CORRECT
		       - cycle_penalty(shc, scratch);
	       malus = worse*MALUS_CROSSED_DESTROY;

	       if(found_better_solution(shc, bonus, malus))
	       {  memcpy(shc->bestFrame, scratch, rb->sampleSize);

		  snprintf(shc->msg, SMART_LEC_MESSAGE_SIZE,
			   "P%02d with erasures %02d/%02d: %d better, %d worse (bonus %d/ malus %d).",
			   p,erasure_pos[e1],erasure_pos[e2],better,worse,bonus,malus);
	       }
	    }
	 }
   }
}

/***
 *** Swap P with an alternative vector,
 *** then selectively correct the crossing vectors
 * to see if a new combination arises which makes P
 * correct a new vector.
 */

static void swap_p_for_new_improvement(sh_context *shc)
{  RawBuffer *rb = shc->rb;
   unsigned char old_vector[Q_VECTOR_SIZE];
   unsigned char new_vector[Q_VECTOR_SIZE];
   unsigned char p_vector[P_VECTOR_SIZE];
   unsigned char q_vector[Q_VECTOR_SIZE];
   unsigned char scratch[MAX_RAW_TRANSFER_SIZE];
   int err_pos;
   int p,q;

   for(p=0; p<N_P_VECTORS; p++)
      //for(p=45; p<46; p++)
   {  int crossed, crossed_idx;
      int i,j;

      if(shc->pState[p] == 0)
	 continue;

      GetPVector(rb->recovered, old_vector, p);

      for(i=0; i<rb->pn[p]; i++)  /* try all alternative p vectors */
      {	 int selection[N_Q_VECTORS];
	 int crossing_q[N_Q_VECTORS];
	 int crossing_err[N_Q_VECTORS];
	 int err;
	 int n_q = 0;
	 int n_iterations = 1;

	 verbose("swap p for new improvement: alternative %d for P%02d\n",i, p);

         memcpy(scratch, rb->recovered, rb->sampleSize);
	 SetPVector(scratch, rb->pList[p][i], p);
	 memcpy(new_vector, rb->pList[p][i], P_VECTOR_SIZE);

	 for(j=0; j<P_VECTOR_SIZE; j++)
	 {  if(old_vector[j] != new_vector[j])
	    {   
	        ByteIndexToQ(PToByteIndex(p, j), &crossed, &crossed_idx);
		GetQVector(scratch, q_vector, crossed);
		err = decode_q(rb, q_vector, &err_pos);
		verbose("diff at %2d: Q%2d from %d->%d\n", 
		       j, crossed, shc->qState[crossed], err);
		crossing_q[n_q] = crossed;
		crossing_err[n_q] = err;
		selection[n_q] = 1;
		n_q++;
		n_iterations *= 2;
	    }
	 }

	 if(n_q < 2)      /* We need at least 2 crossing vectors */
	    continue;
	 n_iterations--;  /* number of combinations to test */

	 /* Enumerate all combinations by using <selection> as a binary counter */

	 verbose("Trying all combinations for P%02d\n", p);

	 while(n_iterations--)
	 {  int index = 0;
	    int count = 0;
	    int bonus = 0;
	    int malus = 0;

	    for(q=n_q-1; q>=0; q--)
	       verbose("%d", selection[q]);

	    /* Count number of changed crossed vectors.
	       Less than 2 do not produce an interesting new P vector. */

	    for(j=0; j<n_q; j++)
	       if(selection[j])
		  count++;

	    if(count < 2)
	    {  verbose("%s", " pruned");
	       goto decrement;
	    }

	    /* See if we get an interesting P vector from
	       correcting some of the crossing Qs */

	    memcpy(scratch, rb->recovered, rb->sampleSize);
	    SetPVector(scratch, new_vector, p);

	    for(j=0; j<n_q; j++)
	    {  if(!selection[j]) 
	       {  malus += crossing_err[j] * SWAPPED_WITH_IMPROVEMENT_MALUS;  
		  continue;
	       }
	       GetQVector(scratch, q_vector, crossing_q[j]);
	       err = decode_q(rb, q_vector, &err_pos);
	       if(err == 1) SetQVector(scratch, q_vector, crossing_q[j]);
	    }

	    GetPVector(scratch, p_vector, p);
	    err = decode_p(rb, p_vector, &err_pos);

	    verbose(" %3d", err);

	    if(err==1)
	    {  int crossed_q;
	       int prev_state, new_state;

	       ByteIndexToQ(PToByteIndex(p, err_pos), &crossed_q, &crossed_idx);
	       verbose(", affecting Q %d:", crossed_q);

	       GetQVector(scratch, q_vector, crossed_q);
	       prev_state = decode_q(rb, q_vector, &err_pos);

	       SetPVector(scratch, p_vector, p);
	       GetQVector(scratch, q_vector, crossed_q);
	       new_state  = decode_q(rb, q_vector, &err_pos);

	       verbose("%d -> %d", prev_state, new_state);

	       /* Better solution found? */

	       if(prev_state > new_state)
	       {  bonus = SWAPPED_WITH_IMPROVEMENT_BONUS - cycle_penalty(shc, scratch);
		  
		  if(found_better_solution(shc, bonus, malus))
		  {  memcpy(shc->bestFrame, scratch, rb->sampleSize);
		     snprintf(shc->msg, SMART_LEC_MESSAGE_SIZE,
			      "P%02d swapped; Q%02d improved (%d/%d)",p,crossed_q,bonus,malus);
		  }
	       }
	    }

	    /* binary decrement on selection[] */
decrement:
	    while(selection[index] != 1 && index < n_q)
	    {  index++;
	       selection[index-1] = 1;
	    }
	    if(index >= n_q) break;
	    selection[index]=0;

	    verbose("%s", "\n");
	 }
      }
   }
}

/***
 *** Try to get out of a local maximum.
 ***
 * See if swapping a byte in P produces a Q which can be changed
 * by another crossing P. At worst, we get rid of the crossing P
 * and the changed Q might correct something else in a later stage.
 */

static void try_indirect_improvement(sh_context *shc)
{  RawBuffer *rb = shc->rb;
   unsigned char scratch[MAX_RAW_TRANSFER_SIZE];
   unsigned char vector[Q_VECTOR_SIZE];
   int i,j,p,q;

   /* Find uncorrected P vectors */

   for(p=0; p<N_P_VECTORS; p++)
   {  unsigned char current_p[P_VECTOR_SIZE];

      if(shc->pState[p] == 0)
	 continue;  /* already good */

      GetPVector(rb->recovered, current_p, p);

      for(i=0; i<rb->pn[p]; i++)
      {  unsigned char *alternative_p = rb->pList[p][i];

	 /* If the alternative vector differs in byte j,
	    see what happens with the Q crossing byte j. */

	 for(j=0; j<P_VECTOR_SIZE; j++)
	 {  if(current_p[j] != alternative_p[j])
	    {  unsigned char crossed_q[Q_VECTOR_SIZE];
	       int byte_idx, crossed_idx, ignore, err;
	       int bonus=0, malus=0;

	       /* Determine crossed Q and modify it */

	       byte_idx = PToByteIndex(p, j);
	       ByteIndexToQ(byte_idx, &q, &crossed_idx);
	       GetQVector(rb->recovered, crossed_q, q);
	       crossed_q[crossed_idx] = alternative_p[j];

	       /* Calculated state of modified Q.
		  We want a Q which either improves or stays within the
	          correctable state (maybe it corrects the right byte now) */

	       memcpy(vector, crossed_q, Q_VECTOR_SIZE);
	       err = decode_q(rb, vector, &ignore);

	       if(err == 1)
	       {  int p2,p2_index,k;
		  verbose("indirect for P%02d: interesting Q%02d %d->%d\n", p, q, shc->qState[q], err);

		  /* Now see if we can find another crossing P which makes Q go from
		     1->1 or 0 */

		  memcpy(scratch, rb->recovered, rb->sampleSize);
		  scratch[byte_idx] = alternative_p[j];

		  for(k=0; k<Q_VECTOR_SIZE; k++)
		  {  unsigned char p_vector[P_VECTOR_SIZE];

		     byte_idx = QToByteIndex(q,k);
		     ByteIndexToP(byte_idx, &p2, &p2_index);
		     if(   (p2 == p)
                        || (shc->pState[p2] != 1)
			|| (shc->pPosition[p2] != p2_index))
			continue;  /* trivially reject; crossing P won't change Q */

		     /* Crossed P changed something in Q. Re-evaluate Q. */

		     memcpy(vector, crossed_q, Q_VECTOR_SIZE);
		     vector[k] = shc->pValue[p2];
		     err = decode_q(rb, vector, &ignore);

		     if(err>1)  /* Q got uncorrectable */ 
			continue;

		     scratch[byte_idx] = shc->pValue[p2];

		     /* Add bonus for improved Q (err of Q was 1 before) */
		     
		     if(!err) bonus += INDIRECT_IMPROVEMENT_BONUS_FOR_Q;

		     /* Set score for the outer P */

		     memcpy(p_vector, current_p, P_VECTOR_SIZE);
		     current_p[j] = alternative_p[j];
		     err = decode_p(rb, p_vector, &ignore);
		     if(err < shc->pState[p]) bonus += INDIRECT_IMPROVEMENT_BONUS_FOR_P;
		     if(err > shc->pState[p]) malus += INDIRECT_IMPROVEMENT_MALUS_FOR_P;

		     /* Add some for the inner P which changed from 1->0 */

		     bonus += INDIRECT_IMPROVEMENT_BONUS_FOR_P;

		     verbose("crossed P%02d does something (%d/%d)\n", p2,bonus,malus);

		     if(found_better_solution(shc, bonus, malus))
		     {  memcpy(shc->bestFrame, scratch, rb->sampleSize);
			snprintf(shc->msg, SMART_LEC_MESSAGE_SIZE,
				 "Q%02d improved indirectly by P%02d, P%02d (%d/%d)",q,p,p2,bonus,malus);
		  }

		  }
	       }
	    }
	 }
      }
   }
}

/***
 *** Try all bytes at positions where uncorrectable p/q overlap
 ***/

#if 0
static void guess_crossing_byte(sh_context *shc)
{  RawBuffer *rb = shc->rb;
   int p,q,i,j,g;

   for(p=0; p<N_P_VECTORS; p++)
   {  int crossed_idx;

      if(shc->pState[p] != 2)
	 continue;

      for(i=0; i<P_VECTOR_SIZE; i++)
      {  ByteIndexToQ(PToByteIndex(p, i), &q, &crossed_idx);

	 if(shc->qState[q] != 2)
	    continue;

	 verbose("Guess candidate P%02d.%02d / Q%02d.%02d\n", p,i,q,crossed_idx);

	 for(g=0; g<256; g++)
	 {  unsigned char p_vector[P_VECTOR_SIZE];
	    unsigned char q_vector[Q_VECTOR_SIZE];
	    int ignore;
	    int idx = PToByteIndex(p, i);

	    for(j=0; j<rb->samplesRead; j++)
	       if(rb->rawBuf[j][idx] == g)
		  break;

	    if(j>=256) continue;

	    GetPVector(rb->recovered, p_vector, p);
	    p_vector[i] = g;

	    if(decode_p(rb, p_vector, &ignore) == 1)
	    {  GetQVector(rb->recovered, q_vector, q);
	       q_vector[crossed_idx] = g;

	       if(decode_q(rb, q_vector, &ignore) == 1)
		  verbose("... guess %d solves P and Q\n", g);
	    }
	 }
      }
   }
}
#endif

/***
 *** The smart lec wrapper.
 ***/

static void pick_best_frame(sh_context *shc, char *message)
{  RawBuffer *rb = shc->rb;

   printf("Best P: %d/%d, %d/%d\n", rb->bestP2, rb->bestP1, rb->bestQ2, rb->bestQ1);

   memcpy(rb->recovered, rb->rawBuf[rb->bestFrame], rb->sampleSize);
   if(message)
      snprintf(message, SMART_LEC_MESSAGE_SIZE, 
	       "selected best sector frame %d with %d/%d, %d/%d defective P/Q vectors.",
		  rb->bestFrame, rb->bestP2, rb->bestP1, rb->bestQ2, rb->bestQ1);
}

static int smart_lec_iteration(sh_context *shc, char *message)
{  RawBuffer *rb = shc->rb;
  
   shc->bestBonus = 0;
   shc->bestMalus = 100000;
   memcpy(shc->bestFrame, rb->recovered, rb->sampleSize);
   sprintf(shc->msg, "smart_lec: no further improvement");

   update_pq_state(shc);
   print_pq_state(shc);

   many_p_correct_one_q(shc);
   many_q_correct_one_p(shc);
#ifndef LOCAL_ONLY
   try_alternative_vectors(shc);

   try_alternative_crossing_bytes(shc);
#endif

   find_p_with_two_erasures(shc);

   swap_p_for_new_improvement(shc);

   try_indirect_improvement(shc);

   if(frame_visited(shc, shc->bestFrame))
      printf("pruning!\n");
   memcpy(rb->recovered, shc->bestFrame, rb->sampleSize);
   push_frame(shc, shc->bestFrame);

   if(message)
      memcpy(message, shc->msg, SMART_LEC_MESSAGE_SIZE);

   return TRUE;
}

int SmartLEC(RawBuffer *rb)
{  sh_context *shc = create_sh_context(rb);
   unsigned char prev_state[rb->sampleSize];

   pick_best_frame(shc, NULL);
#ifdef PRINT_STEPS
   printf("%s\n", shc->msg);
#endif

   CreateMissingSector(prev_state, rb->lba, NULL, 0,
		       "SmartLEC() dummy sector");
   while(TRUE)
   {  smart_lec_iteration(shc, NULL);
#ifdef PRINT_STEPS
      printf("%s\n", shc->msg);
#endif
      if(frame_corrected(shc->rb))
      {  free_sh_context(shc);
	 return TRUE;
      }

      if(!memcmp(prev_state, rb->recovered, rb->sampleSize))
      {  free_sh_context(shc);
	 return FALSE;
      }

      memcpy(prev_state, rb->recovered, rb->sampleSize);
   }

   free_sh_context(shc);
   return FALSE;
}

/*
 * Special actions for iteratively running the smart lec
 * from the raw editor
 */

void *PrepareIterativeSmartLEC(RawBuffer *rb)
{  sh_context *shc = create_sh_context(rb);

   shc->iteration = ITERATION_PICK_BEST_SECTOR;
   return shc;
}

void SmartLECIteration(void *shc_handle, char *message)
{  sh_context *shc = (sh_context*)shc_handle;

   switch(shc->iteration)
   {  case ITERATION_PICK_BEST_SECTOR:
#ifndef LOCAL_ONLY
	 pick_best_frame(shc, message);
#endif
	 shc->iteration++;
	 break;

      default:
	 smart_lec_iteration(shc, message);
	 break;
   }

#ifdef PRINT_STEPS
   printf("%s\n", shc->msg);
#endif
}
   

void EndIterativeSmartLEC(void *shc)
{  free_sh_context((sh_context*)shc);
}
