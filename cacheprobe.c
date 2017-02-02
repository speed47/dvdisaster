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

#ifdef SYS_LINUX
int ProbeCacheLineSize()
{  int cl_size = 0;

   cl_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);

   if(!cl_size)
     cl_size = sysconf(_SC_LEVEL2_CACHE_LINESIZE);

   if(cl_size < 16)
     cl_size = 64;

   return cl_size;
}
#endif

#ifdef SYS_FREEBSD
#include <sys/param.h>

int ProbeCacheLineSize()
{  int cl_size = CACHE_LINE_SIZE;

  /* Doing this at compile time may backfire,
     but let's just hope for the best. */

   if(cl_size < 16)
     cl_size = 64;

   return cl_size;
}
#endif

#ifdef SYS_NETBSD
#include <sys/param.h>

int ProbeCacheLineSize()
{  int cl_size = CACHE_LINE_SIZE;

  /* Doing this at compile time may backfire,
     but let's just hope for the best. */

   if(cl_size < 16)
     cl_size = 64;

   return cl_size;
}
#endif

#ifdef SYS_UNKNOWN
int ProbeCacheLineSize()
{
  return 64;
}
#endif


