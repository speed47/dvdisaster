/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * This is derived from the Berkeley source:
 *	@(#)random.c	5.5 (Berkeley) 7/6/88
 * It was reworked for the GNU C Library by Roland McGrath.
 * Rewritten to use reentrant functions by Ulrich Drepper, 1995.
 */

/*
 * This is derived from glibc-2.1:
 * 	glibc2.1/stdlib/random.c
 * 	glibc2.1/stdlib/random_r.c
 * Code was reworked and cut down to be a work-alike of the default 
 * settings of srandom() and random() by Carsten Gnörlich, 2004.
 *
 * Note that the original code is much more sophisticated than this one,
 * so you will probably want to review the real thing if you are 
 * interested in the inner workings of the original glibc functions.
 * 
 * There's nothing special about this code; it's simply here to
 * provide consistency among systems with different random() 
 * implementations. E.g. it makes sure that dvdisaster produces
 * the same "random" images on all supported platforms.
 *
 * Note that unlike in the real thing, you must call SRandom()
 * before using Random() the first time. 
 */

#include "dvdisaster.h"

/* Some hardcoded values from glibc's default setting. */

#define	MY_DEG		31
#define	MY_SEP		3

/*
 * State information for the random number generator.
 */

static gint32 *fptr;		/* Front pointer.  */
static gint32 *rptr;		/* Rear pointer.  */
static gint32 state[MY_DEG];	/* Array of state values.  */
static gint32 *end_ptr;		/* Pointer behind state table.  */

/* Initialize the random number generator based on the given seed
 * via a linear congruential generator.  
 * Then, the pointers are set to known locations that are exactly 
 * MY_SEP places apart.  
 * Lastly, it cycles the state information a given number of times 
 * to get rid of any initial dependencies introduced by the L.C.R.N.G.
 */

void SRandom(gint32 seed)
{ gint32 i;
  gint32 word;

  /* We must make sure the seed is not 0.  
   * Take arbitrarily 1 in this case.  */

  if (!seed) seed = 1;
 
  /* This does: state[i] = (16807 * state[i - 1]) % 2147483647;
     but avoids overflowing 31 bits.  */

  state[0] = word = seed;
  for (i=1; i < MY_DEG; i++)
  {  gint32 hi = word / 127773;
     gint32 lo = word % 127773;

     word = 16807 * lo - 2836 * hi;
     if (word < 0) word += 2147483647;
 
     state[i] = word;
  }

  /* Now prepare the pointers and cycle the state info 10 times around */

  fptr    = state + MY_SEP;
  rptr    = state;
  end_ptr = state + MY_DEG;

  for(i=10*MY_DEG; i; i--)
     Random();
}

/* Deliver the next pseudo-random number in the current series.
 * This uses only the trinomial branch of the original code, 
 * which is supposed to give the best results.
 * The basic operation is to add the number at the rear pointer into
 * the one at the front pointer.  Then both pointers are advanced to the next
 * location cyclically in the table.  The value returned is the sum generated,
 * reduced to 31 bits by throwing away the "least random" low bit.
 * Note: The code takes advantage of the fact that both the front and
 * rear pointers can't wrap on the same call by not testing the rear
 * pointer if the front one has wrapped.  Returns a 31-bit random number.  
 */

gint32 Random(void)
{  gint32 val,result;

   val = *fptr += *rptr;

   /* Chucking least random bit.  */
   result = (val >> 1) & 0x7fffffff;

   ++fptr;
   if(fptr >= end_ptr)
   {  fptr = state;
      ++rptr;
   }
   else
   {  ++rptr;
      if(rptr >= end_ptr)
	 rptr = state;
   }

   return result;
}

/* Create a 32-bit random value from two sequential 31-bit values.
 * Note that this is all simple stuff to produce a sequence of "different"
 * numbers; it's not meant to deliver cryptographically strong random
 * sequences.
 */

guint32 Random32(void)
{  guint32 value;

   value = (Random() & 0xffff);
   value <<= 16;
   value |= (Random() & 0xffff);

   return value;
}
