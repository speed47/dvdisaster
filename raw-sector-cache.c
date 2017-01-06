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
 * Write the raw dump header
 */

static void init_defective_sector_file(char *path, RawBuffer *rb, LargeFile **file, DefectiveSectorHeader *dsh)
{   int n;

    *file = LargeOpen(path, O_RDWR | O_CREAT, IMG_PERMS);

    if(!*file)
       Stop(_("Could not open %s: %s"), path, strerror(errno));

    memset(dsh, 0, sizeof(DefectiveSectorHeader));
    dsh->lba        = rb->lba;
    dsh->sectorSize = CD_RAW_DUMP_SIZE;

    if(rb->xaMode)
       dsh->properties |= DSH_XA_MODE;

    if(rb->validFP)
    {  memcpy(dsh->mediumFP, rb->mediumFP, 16);
       dsh->properties |= DSH_HAS_FINGERPRINT;
    }

#ifdef HAVE_BIG_ENDIAN
    SwapDefectiveHeaderBytes(dsh);
#endif

    n = LargeWrite(*file, dsh, sizeof(DefectiveSectorHeader));

#ifdef HAVE_BIG_ENDIAN
    SwapDefectiveHeaderBytes(dsh);
#endif

    if(n != sizeof(DefectiveSectorHeader))
       Stop(_("Failed writing to defective sector file: %s"), strerror(errno));
}

/*
 * Open raw dump, read the header
 */

static void open_defective_sector_file(RawBuffer *rb, char *path, LargeFile **file, 
				       DefectiveSectorHeader *dsh)
{   guint64 length;
    int n;

    *file = LargeOpen(path, O_RDWR, IMG_PERMS);

    if(!*file) return;

    LargeStat(path, &length);

    n = LargeRead(*file, dsh, sizeof(DefectiveSectorHeader));
    if(n != sizeof(DefectiveSectorHeader))
       Stop(_("Failed reading from defective sector file: %s"), strerror(errno));

#ifdef HAVE_BIG_ENDIAN
    SwapDefectiveHeaderBytes(dsh);
#endif

    dsh->nSectors = (length-sizeof(DefectiveSectorHeader))/dsh->sectorSize;
    if(dsh->nSectors*dsh->sectorSize+sizeof(DefectiveSectorHeader) != length)
       Stop(_("Defective sector file is truncated"));

    /* Expand the old non-C2 raw dumps to new size */

    if(dsh->sectorSize == 2352) /* old non C2-style raw dump? */
    {  unsigned char *buf,*ptr;
       unsigned char zero[296];
       int i,n;

       PrintCLI(" * Expanding raw dump for sector %lld from 2352 to %d bytes *\n",
		(long long)dsh->lba,  MAX_RAW_TRANSFER_SIZE);

       buf = g_malloc(dsh->sectorSize*dsh->nSectors);
       for(i=0, ptr=buf; i<dsh->nSectors; i++, ptr+=2352)
       {  int n=LargeRead(*file, ptr, dsh->sectorSize);
	 
	  if(n != dsh->sectorSize)
	     Stop(_("Failed reading from defective sector file: %s"), strerror(errno));
       }

       memset(zero, 0, 296);
       dsh->sectorSize = MAX_RAW_TRANSFER_SIZE;

       if(!LargeSeek(*file, 0))
	  Stop(_("Failed seeking in defective sector file: %s"), strerror(errno));

#ifdef HAVE_BIG_ENDIAN
    SwapDefectiveHeaderBytes(dsh);
#endif
       n = LargeWrite(*file, dsh, sizeof(DefectiveSectorHeader));
       
#ifdef HAVE_BIG_ENDIAN
    SwapDefectiveHeaderBytes(dsh);
#endif

       if(n != sizeof(DefectiveSectorHeader))
	  Stop(_("Failed writing to defective sector file: %s"), strerror(errno));

       for(i=0, ptr=buf; i<dsh->nSectors; i++, ptr+=2352)
       {  n=LargeWrite(*file, ptr, 2352);
	 
	  if(n != 2352)
	     Stop(_("Failed writing to defective sector file: %s"), strerror(errno));

	  n=LargeWrite(*file, zero, 296);
	  if(n != 296)
	     Stop(_("Failed writing to defective sector file: %s"), strerror(errno));
       }

       if(!LargeSeek(*file, sizeof(DefectiveSectorHeader)))
	  Stop(_("Failed seeking in defective sector file: %s"), strerror(errno));
       
       g_free(buf);
    }

    /* If the cache file has no fingerprint, add it now */

    if(!(dsh->properties & DSH_HAS_FINGERPRINT) && rb->validFP)
    {  memcpy(dsh->mediumFP, rb->mediumFP, 16);
       dsh->properties |= DSH_HAS_FINGERPRINT;

       if(!LargeSeek(*file, 0))
	  Stop(_("Failed seeking in defective sector file: %s"), strerror(errno));

#ifdef HAVE_BIG_ENDIAN
    SwapDefectiveHeaderBytes(dsh);
#endif
       n = LargeWrite(*file, dsh, sizeof(DefectiveSectorHeader));

#ifdef HAVE_BIG_ENDIAN
    SwapDefectiveHeaderBytes(dsh);
#endif
       
       if(n != sizeof(DefectiveSectorHeader))
	  Stop(_("Failed writing to defective sector file: %s"), strerror(errno));
    }

    /* Verify cache and medium fingerprint */

    if((dsh->properties & DSH_HAS_FINGERPRINT) && rb->validFP)
    {  if(memcmp(dsh->mediumFP, rb->mediumFP, 16))
	  Stop(_("Fingerprints of medium and defective sector cache do not match!"));
    }
}

/*
 * Append RawBuffer contents to defective sector dump
 */

int SaveDefectiveSector(RawBuffer *rb, int can_c2_scan)
{  LargeFile *file;
   DefectiveSectorHeader *dsh = alloca(sizeof(DefectiveSectorHeader));
   unsigned char *cache_sectors = NULL;
   char *filename;
   guint64 length,offset;
   int count=0;
   int i,j,idx;

   if(!rb->samplesRead) 
     return 0;  /* Nothing to be done */

   /* Open cache file */

   filename = g_strdup_printf("%s/%s%lld.raw", 
			      Closure->dDumpDir, Closure->dDumpPrefix, 
			      (long long)rb->lba);

   if(!LargeStat(filename, &length))
   {  PrintCLIorLabel(Closure->status,_(" [Creating new cache file %s]\n"), filename);
      init_defective_sector_file(filename, rb, &file, dsh);
   }
   else 
   {  open_defective_sector_file(rb, filename, &file, dsh);
      if(!file)
	 Stop(_("Could not open %s: %s"), filename, strerror(errno));
   }

   /* Read already cached sectors */

   if(dsh->nSectors > 0)
   {  if(!LargeSeek(file, sizeof(DefectiveSectorHeader)))
	 Stop(_("Failed seeking in defective sector file: %s"), strerror(errno));

      cache_sectors = g_malloc(dsh->sectorSize*dsh->nSectors);
      for(i=0, idx=0; i<dsh->nSectors; i++, idx+=dsh->sectorSize)
      {  int n=LargeRead(file, cache_sectors+idx, dsh->sectorSize);
	 
	 if(n != dsh->sectorSize)
	    Stop(_("Failed reading from defective sector file: %s"), strerror(errno));
      }
   }

   /* Store sectors which are not already cached */
   
   offset = sizeof(DefectiveSectorHeader) + dsh->sectorSize*dsh->nSectors;
   if(!LargeSeek(file, offset))
      Stop(_("Failed seeking in defective sector file: %s"), strerror(errno));

   for(i=0; i<rb->samplesRead; i++)
   {  int new_sector = TRUE;

      /* See comment below on C2 mask field to understand rb->sampleSize-1 */

      if(cache_sectors)  /* Sector already in cache? */
      {  
	 for(j=0, idx=0; j<dsh->nSectors; j++, idx+=dsh->sectorSize) 
	 {  if(!memcmp(rb->rawBuf[i], cache_sectors+idx, rb->sampleSize-1))
	    {  new_sector = FALSE;
	       break;
	    }
	 }
      }

      for(j=0; j<i; j++) /* Some drives return cached data after first read */
      {  if(!memcmp(rb->rawBuf[i], rb->rawBuf[j], rb->sampleSize-1))
	 {  new_sector = FALSE;
	    break;
	 }
      }

      if(new_sector)   /* same sector already in cache */
      {  int n;

        /* The C2 mask field is not used; so we put a flag into it
	   to mark raw sectors containing C2 error information. */

	 if(can_c2_scan)
	    rb->rawBuf[i][CD_RAW_DUMP_SIZE-1] = 1;

	 n=LargeWrite(file, rb->rawBuf[i], dsh->sectorSize);
	 
	 if(n != dsh->sectorSize)
	    Stop(_("Failed writing to defective sector file: %s"), strerror(errno));
	 count++;
      }
   }

   LargeClose(file);

   PrintCLIorLabel(Closure->status,
		   _(" [Appended %d/%d sectors to cache file %s; LBA=%lld, ssize=%d, %d sectors]\n"), 
		   count, rb->samplesRead, filename, dsh->lba, dsh->sectorSize, dsh->nSectors);

   g_free(filename);
   if(cache_sectors)
      g_free(cache_sectors);

   return count;
}

/*
 * Read sectors from the defective sector dump,
 * feed them into the raw buffer one by one
 * and retry recovery.
 */

int TryDefectiveSectorCache(RawBuffer *rb, unsigned char *outbuf)
{  DefectiveSectorHeader dsh;
   LargeFile *file;
   char *path;
   int status;
   int last_sector;
   int i;

   path = g_strdup_printf("%s/%s%lld.raw", 
			  Closure->dDumpDir, Closure->dDumpPrefix, 
			  (long long)rb->lba);
   open_defective_sector_file(rb, path, &file, &dsh);
   g_free(path);

   if(!file)   /* No cache file */
      return -1;

   /* skip sectors added in current pass */

   last_sector = dsh.nSectors - rb->samplesRead; 

   ReallocRawBuffer(rb, dsh.nSectors);

   for(i=0; i<last_sector; i++)
   {  int n;
  
      n = LargeRead(file, rb->workBuf->buf, dsh.sectorSize);
      if(n != dsh.sectorSize)
	 Stop(_("Failed reading from defective sector file: %s"), strerror(errno));

      status = TryCDFrameRecovery(rb, outbuf);
      if(!status) 
      {  PrintCLIorLabel(Closure->status,
			 " [Success after processing cached sector %d]\n", i+1);
	 return status; 
      }
   }

   LargeClose(file);
   
   return -1;
}

/*
 * Read sectors from the defective sector dump
 */

void ReadDefectiveSectorFile(DefectiveSectorHeader *dsh, RawBuffer *rb, char *path)
{  LargeFile *file;

   open_defective_sector_file(rb, path, &file, dsh);
   if(!file)
   {  Stop(_("Could not open %s: %s"), path, strerror(errno));
      return;
   }

   rb->lba = dsh->lba;

   if(dsh->properties & DSH_XA_MODE)
        rb->dataOffset = 24;
   else rb->dataOffset = 16;

   ReallocRawBuffer(rb, dsh->nSectors);

   for(rb->samplesRead=0; rb->samplesRead<dsh->nSectors; )
   {  int n=LargeRead(file, rb->rawBuf[rb->samplesRead], dsh->sectorSize);

      if(n != dsh->sectorSize)
      {  Stop(_("Failed reading from defective sector file: %s"), strerror(errno));
	 return;
      }

      rb->samplesRead++;
      UpdateFrameStats(rb);
      CollectGoodVectors(rb);
   }

   LargeClose(file);
}
