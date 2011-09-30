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

#ifdef HAVE_SSE2
  #include <emmintrin.h>

#ifdef HAVE_CPUID
  #include <cpuid.h>
#else
  #include "compat/cpuid.h"
#endif
#endif

/***
 *** Reed-Solomon encoding using SSE2 intrinsics
 ***/

/* SSE 2 version */

#ifdef HAVE_SSE2 
int ProbeSSE2(void)
{  unsigned int eax, ebx, ecx, edx;

   if(!__get_cpuid(1, &eax, &ebx, &ecx, &edx))
   {  Verbose("[ProbeSSE2: get_cpuid() failed]\n");
      return 0;
   }

   if(edx & bit_SSE2)
   {  Verbose("[ProbeSSE2: SSE2 available]\n");
      return 1;
   }
   else
   {  Verbose("[ProbeSSE2: no SSE2]\n");
      return 0;
   }
}

#else /* don't have SSE2 */
/* Stub functions to keep the linker happy.
 * Should never be executed.
 */

int ProbeSSE2()
{  return 0;
}
#endif /* HAVE_SSE2 */

