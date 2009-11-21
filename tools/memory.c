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

#if !defined(SYS_FREEBSD) && !defined(SYS_DARWIN)   /* FreeBSD declares malloc() in stdlib.h */
 #include <malloc.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Endian fiddling
 */

unsigned int SwapBytes32(unsigned int in)
{
  return
        ((in & 0xff000000) >> 24) 
      | ((in & 0x00ff0000) >>  8) 
      | ((in & 0x0000ff00) <<  8) 
      | ((in & 0x000000ff) << 24);
}

/*
 * Tell user that current action was aborted due to a serious error.
 */

void Stop(char *format, ...)
{  va_list argp;

   /*** Show message depending on commandline / GUI mode  */ 

   fprintf(stdout, "*\n* pngpack - can not continue:\n*\n");
   va_start(argp, format);
   vfprintf(stdout, format, argp);
   va_end(argp);
   fprintf(stdout, "\n\n");
   fflush(stdout);

   exit(EXIT_FAILURE);
}

/***
 *** Keeping track of allocated pointers. 
 ***/

/*
 * A structure containing info about each pointer which was
 * produced through our memory allocation routines.
 */

typedef struct _memchunk
{  void *ptr;	/* allocated memory chunk */
   int size;	/* size of chunk */
   char *file;	/* source file this was allocated in */
   int  line;	/* line number of source file */
} memchunk;

static struct _memchunk **ptrhash[64];	/* 64 buckets of memory chunks */
static int phCnt[64];
static int phMax[64];
static int currentAllocation;		/* current memory allocation */
static int peakAllocation;		/* maximum allocation */

/*
 * Remember an allocated pointer. 
 */

void remember(void *ptr, int size, char *file, int line)
{  memchunk *mc;
   int hash_idx;

   hash_idx = (((long)ptr)>>3)&63;
   if(phCnt[hash_idx] >= phMax[hash_idx])
   {  if(!phMax[hash_idx]) phMax[hash_idx] = 16;
      else                 phMax[hash_idx] *= 2;
      if(!(ptrhash[hash_idx] = realloc(ptrhash[hash_idx], sizeof(memchunk*)*phMax[hash_idx])))
	 Stop("can't realloc memchunk hashtable");
   }

   if(!(mc=malloc(sizeof(memchunk))))
      Stop("can't alloc memchunk");

   ptrhash[hash_idx][phCnt[hash_idx]++] = mc;

   mc->ptr   = ptr;
   mc->size  = size;
   mc->file  = file;
   mc->line  = line;

   currentAllocation += size;
   if(currentAllocation > peakAllocation)
      peakAllocation = currentAllocation;
} 

/*
 * Remove a remembered pointer from the hash bucket.
 */

int forget(void *ptr)
{  memchunk **ptrlist;
   int hash_idx;
   int i;

   hash_idx = (((long)ptr)>>3)&63;
   ptrlist = ptrhash[hash_idx];

   for(i=0; i<phCnt[hash_idx]; i++)
      if(ptrlist[i]->ptr==ptr)
      {  currentAllocation -= ptrlist[i]->size;
	 free(ptrlist[i]);
         phCnt[hash_idx]--;
	 if(phCnt[hash_idx] > 0)
	    ptrlist[i] = ptrlist[phCnt[hash_idx]];

         return 0;
      }

   return 1;
}

/*
 * Print the contents of the ptrlist.
 */

static void print_ptr(memchunk *mc, int size)
{  char strbuf[16];
   char *ptr = (char*)mc->ptr; 
   int j,maxlen;

   if(mc->size < size) maxlen = mc->size; else maxlen = size;
   for(j=0; j<15; j++)
   {  if(ptr[j]<32) break;
      strbuf[j] = ptr[j];
   } 

   if(j) 
   {  strbuf[j]=0;
      fprintf(stdout, "Address 0x%lx (\"%s\"), %d bytes, from %s, line %d\n",
	       (unsigned long)mc->ptr,strbuf,mc->size,mc->file,mc->line);
   }
   else 
      fprintf(stdout, "Address 0x%lx (binary data), %d bytes, from %s, line %d\n",
	      (unsigned long)mc->ptr,mc->size,mc->file,mc->line);
}

static void print_ptrs(char *msg)
{  int bucket,i,n=0;

   fprintf(stdout, msg);

   for(bucket=0; bucket<64; bucket++)
      for(i=0; i<phCnt[bucket]; i++)
      {  
	 print_ptr(ptrhash[bucket][i], 15);
	 n++;
      }

   fprintf(stdout, "%d memory chunks total.\n",n);

}

/***
 *** Replacements for the libc memory allocators.
 ***/

/* 
 * Protected malloc().
 */

void *malloc_ext(int size, char* file, int line)
{  void *ptr;
#if 0
   printf("allocating %d bytes from file %s, line %d\n", size, file, line); 
#endif
   if(!(ptr = calloc(1,size)))
      Stop("out of memory while allocating %d bytes",size);

   remember(ptr,size,file,line);

   return ptr;
}

/* 
 * Protected calloc().
 */

void *calloc_ext(int n, int size, char* file, int line)
{  void *ptr;

   if(!(ptr = calloc(n,size)))
      Stop("out of memory while allocating %d bytes",size);

   remember(ptr,size,file,line);

   return ptr;
}

/* 
 * Protected realloc(). 
 */

void *realloc_ext(void *ptr, int size, char *file, int line)
{  void *ret;

   if(ptr && forget(ptr))
   {  fprintf(stdout, "trying to realloc undefined pointer 0x%lx\n"
	       "file: %s, line: %d",(long)ptr,file,line);
      exit(EXIT_FAILURE);
   }

   if(!(ret=realloc(ptr,size)))
   {  fprintf(stdout, "out of memory for ptr 0x%lx, %d bytes\n",(long)ptr,size);
      exit(EXIT_FAILURE);
   }

   remember(ret,size,file,line);

   return ret;
}

/*
 * String duplication.
 */

char *strdup_ext(const char *string, char *file, int line)
{  int length = strlen(string)+1;
   char *copy;

   if(!(copy = calloc(1,length)))
      Stop("out of memory while allocating %d bytes",length);

   strcpy(copy,string);
   remember(copy,length,file,line);
   return copy;
}

/* 
 * Free and forget a pointer
 */

void free_ext(void *ptr, char *file, int line)
{ 
   if(forget(ptr))
   {  fprintf(stdout, "trying to free undefined pointer 0x%lx\n"
	       "file: %s, line: %d",(long)ptr,file,line);
      exit(EXIT_FAILURE);
   }

   free(ptr);
}

/***
 *** Checking for memory leaks.
 ***/

void CheckMemleaks(void)
{  int i,memleak = 0;

   /*** See if some memory chunks have been left over */
 
   for(i=0; i<64; i++)
      if(phCnt[i])
	 memleak = 1;

   if(memleak)
   {  char msg[80];

      sprintf(msg,"\npngpack:\nMemory leak warning,"
	      " non-freed memory chunks detected.\n\n");
      print_ptrs(msg);
   }
   else fprintf(stdout, "\npngpack: No memory leaks found.\n");
}

