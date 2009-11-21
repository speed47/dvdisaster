/*  pngpack: lossless image compression for a series of screen shots
 *  Copyright (C) 2005-2009 Carsten Gnoerlich.
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

#ifndef MEMORY_H
#define MEMORY_H

unsigned int SwapBytes32(unsigned int);
void Stop(char*, ...);

#ifdef WITH_MEMDEBUG_YES

#undef strdup
#define malloc(size) malloc_ext(size,__FILE__,__LINE__)
#define calloc(n,size) calloc_ext(n,size,__FILE__,__LINE__)
#define realloc(ptr,size) realloc_ext(ptr,size,__FILE__,__LINE__)
#define strdup(str) strdup_ext(str,__FILE__,__LINE__)
#define free(size) free_ext(size,__FILE__,__LINE__)

void*	malloc_ext(int,char*,int);
void*	calloc_ext(int,int,char*,int);
void*	realloc_ext(void*, int, char*, int);
char*	strdup_ext(const char*,char*,int);
void	free_ext(void*,char*,int);

void    CheckMemleaks(void);
#endif /* WITH_MEMDEBUG_YES */

#endif /* MEMORY_H */
