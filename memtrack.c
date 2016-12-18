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

#define _GNU_SOURCE

#if !defined(SYS_FREEBSD)   /* FreeBSD declares malloc() in stdlib.h */
 #include <malloc.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <glib/gprintf.h>

/*
 * We're not pulling in dvdisaster.h on purpose...
 */

void Stop(char*, ...);

/***
 *** Special routines for keeping track of memory allocation.
 ***
 * memtrack.c currently uses malloc() and friends.
 * This is probably a bad idea for those operating systems where g_malloc()
 * and malloc() are different and not compatible. 
 * We must keep an eye on that.
 */

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

/*
 * Since we're compiled in optionally,
 * we do not use the common Closure struct.
 */

static struct _memchunk **ptrhash[64];	/* 64 buckets of memory chunks */
static int phCnt[64];
static int phMax[64];
static int currentAllocation;		/* current memory allocation */
static int peakAllocation;		/* maximum allocation */
static GMutex phMutex;

/*
 * Remember an allocated pointer. 
 */

void remember(void *ptr, int size, char *file, int line)
{  memchunk *mc;
   int hash_idx;

   g_mutex_lock(&phMutex);

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

   g_mutex_unlock(&phMutex);
} 

/*
 * Remove a remembered pointer from the hash bucket.
 */

int forget(void *ptr)
{  memchunk **ptrlist;
   int hash_idx;
   int i;

   g_mutex_lock(&phMutex);

   hash_idx = (((long)ptr)>>3)&63;
   ptrlist = ptrhash[hash_idx];

   for(i=0; i<phCnt[hash_idx]; i++)
      if(ptrlist[i]->ptr==ptr)
      {  currentAllocation -= ptrlist[i]->size;
	 free(ptrlist[i]);
         phCnt[hash_idx]--;
	 if(phCnt[hash_idx] > 0)
	    ptrlist[i] = ptrlist[phCnt[hash_idx]];

	 g_mutex_unlock(&phMutex);
         return 0;
      }

   g_mutex_unlock(&phMutex);
   return 1;
}

/*
 * Print the contents of the ptrlist.
 */

static void print_ptr(memchunk *mc, int size)
{  char strbuf[16];
   char *ptr = (char*)mc->ptr; 
   int j;

   /* print the pointer */

   for(j=0; j<15; j++)
   {  if(ptr[j]<32) break;
      strbuf[j] = ptr[j];
   } 

#ifdef HAVE_64BIT
   if(j) 
   {  strbuf[j]=0;
      g_printf("Address 0x%llx (\"%s\"), %d bytes, from %s, line %d\n",
	       (unsigned long long)mc->ptr,strbuf,mc->size,mc->file,mc->line);
   }
   else 
     g_printf("Address 0x%llx (binary data), %d bytes, from %s, line %d\n",
	      (unsigned long long)mc->ptr,mc->size,mc->file,mc->line);
#else /* hopefully 32BIT */
   if(j) 
   {  strbuf[j]=0;
      g_printf("Address 0x%lx (\"%s\"), %d bytes, from %s, line %d\n",
	       (unsigned long)mc->ptr,strbuf,mc->size,mc->file,mc->line);
   }
   else 
     g_printf("Address 0x%lx (binary data), %d bytes, from %s, line %d\n",
	      (unsigned long)mc->ptr,mc->size,mc->file,mc->line);
#endif
}

static void print_ptrs(char *msg)
{  int bucket,i,n=0;

   g_printf("%s", msg);

   for(bucket=0; bucket<64; bucket++)
      for(i=0; i<phCnt[bucket]; i++)
      {  
	 print_ptr(ptrhash[bucket][i], 15);
	 n++;
      }

   g_printf("%d memory chunks total.\n",n);

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
 * Protected try_malloc().
 */

void *try_malloc_ext(int size, char* file, int line)
{  void *ptr;

   if((ptr = calloc(1,size)))
     remember(ptr,size,file,line);

   return ptr;
}

/* 
 * Protected realloc(). 
 */

void *realloc_ext(void *ptr, int size, char *file, int line)
{  void *ret;

   if(ptr && forget(ptr))
   {  g_printf("trying to realloc undefined pointer 0x%lx\n"
	       "file: %s, line: %d",(long)ptr,file,line);
      exit(EXIT_FAILURE);
   }

   if(!(ret=realloc(ptr,size)))
   {  g_printf("out of memory for ptr 0x%lx, %d bytes\n",(long)ptr,size);
      exit(EXIT_FAILURE);
   }

   remember(ret,size,file,line);

   return ret;
}

/* 
 * Free and forget a pointer
 */

void free_ext(void *ptr, char *file, int line)
{ 
   if(forget(ptr))
   {  g_printf("trying to free undefined pointer 0x%lx\n"
	       "file: %s, line: %d",(long)ptr,file,line);
      exit(EXIT_FAILURE);
   }

   free(ptr);
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
 * The allocating printf()s 
 */

char* strdup_printf_ext(char *format, char *file, int line, ...)
{  va_list argp;
   char *ret;

   va_start(argp,line);
   ret = g_strdup_vprintf(format, argp);
   remember(ret,strlen(ret),file,line);
   va_end(argp);

   return ret;
}

char* strdup_vprintf_ext(char *format, va_list ap, char *file, int line)
{  char *ret; 

   ret = g_strdup_vprintf(format, ap);
   remember(ret,strlen(ret),file,line);

   return ret;
}

/*
 * The utf8 converter
 */

gchar* g_locale_to_utf8_ext(const gchar *str, gssize len,
			    gsize *bytes_read, gsize *bytes_written_out, GError **error,
			    char *file, int line)
{  char *ret;
   gsize bytes_written; 

   ret = g_locale_to_utf8(str, len, bytes_read, &bytes_written, error);
   remember(ret, bytes_written, file, line);

   if(bytes_written_out)
     *bytes_written_out = bytes_written;

   return ret;
}

/***
 *** Checking for memory leaks.
 ***/

void check_memleaks(void)
{  int i,memleak = 0;

   /*** See if some memory chunks have been left over */
 
   for(i=0; i<64; i++)
      if(phCnt[i])
	 memleak = 1;

   if(memleak)
   {  char msg[80];

      sprintf(msg,"\ndvdisaster:\nMemory leak warning,"
	      " non-freed memory chunks detected.\n\n");
      print_ptrs(msg);
   }
   else g_printf("dvdisaster: No memory leaks found.\n");
}

