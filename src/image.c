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

/*** src type: some GUI code ***/

#include "dvdisaster.h"
#include "scsi-layer.h"
#include "udf.h"

/***
 *** Open image from a given device or file.
 ***
 * Note that OpenImageFromDevice() is located in scsi-layer.c
 * for convenience reasons.
 */

Image *OpenImageFromFile(char *filename, int flags, mode_t mode)
{  LargeFile *file;

   file = LargeOpen(filename, flags, mode);

   if(file)
   {  Image *image = g_malloc0(sizeof(Image));

      image->fpSector = -1;
      image->type = IMAGE_FILE;
      image->file = file;

      CalcSectors(file->size, &image->sectorSize, &image->inLast);

      ExamineUDF(image);
      ExamineECC(image);

      GetImageFingerprint(image, NULL, FINGERPRINT_SECTOR);

      return image;
   }

   return NULL;
}

/***
 *** Find out whether the image contains ECC information.
 ***/

void ExamineECC(Image *image)
{  int i;

   Verbose("ExamineECC() started\n");

   for(i=0; i<Closure->methodList->len; i++)  
   {  Method *method = g_ptr_array_index(Closure->methodList, i);
      char buf[5];
      strncpy(buf,method->name,4);
      buf[4]=0;

      Verbose("...trying %s\n", buf);

      if(   method->recognizeEccImage
	 && method->recognizeEccImage(image))
      {  Verbose("...augmented image found\n");
	 image->eccMethod = method;
	 image->expectedSectors = method->expectedImageSize(image);
	 return;
      }
   }

   Verbose("...no augmented image detected.\n");
} 

/***
 *** Warn if the image extends beyond its ISO file system.
 ***
 * Trailing sectors may be mastering padding or intentional additional data.
 * Explain that they will be protected, without blocking image creation.
 */

/* Match the padding pattern, not the identity of the mastering software.
   A failed probe simply leaves us with the generic trailing-data notice. */

static int has_xorriso_padding(Image *image)
{  static const unsigned char zeros[2048] = {0};
   unsigned char buf[2048];
   guint64 offset;
   int sector, matches = FALSE;

   if(image->type != IMAGE_FILE
      || image->sectorSize - image->isoInfo->volumeSize != 150)
     return FALSE;

   offset = image->file->offset;
   if(LargeSeek(image->file, (guint64)image->isoInfo->volumeSize * 2048))
   {  for(sector=0; sector<150; sector++)
        if(LargeRead(image->file, buf, sizeof(buf)) != sizeof(buf)
           || memcmp(buf, zeros, sizeof(buf)))
          break;
      matches = sector == 150;
   }

   if(!LargeSeek(image->file, offset))
     matches = FALSE;

   return matches;
}

void WarnAboutImagePadding(Image *image)
{  char *message;
   const char *details;

   /* Limit this notice to images with an ISO file system and whole sectors. */

   if(!image->isoInfo || image->inLast != 2048
      || image->sectorSize <= image->isoInfo->volumeSize)
     return;

   if(has_xorriso_padding(image))
     details = _("These sectors are all zero, consistent with xorriso's default 300 KiB padding.\n"
                 "If you use xorriso, consider \"-padding 0\" when creating future images\n"
                 "to omit this padding and potentially leave more space for error correction.\n"
                 "These sectors will be preserved and protected by error correction.\n");
   else
     details = _("These sectors may contain padding or additional data. They will be\n"
                 "preserved and protected by error correction.\n"
                 "If your mastering software added unnecessary padding, disabling it\n"
                 "when creating future images may leave more space for error correction.\n");

   message = g_strdup_printf(
     _("The image contains %" PRId64 " sectors beyond the size recorded in its ISO file system (%u sectors).\n"
       "%s"
       "Creation will continue.\n"),
     image->sectorSize - image->isoInfo->volumeSize,
     image->isoInfo->volumeSize, details);

   LogWarning("%s", message);

#ifdef WITH_GUI_YES
   if(Closure->guiMode)
   {  char *utf_message = g_locale_to_utf8(message, -1, NULL, NULL, NULL);
      char *markup = g_markup_escape_text(utf_message, -1);

      GuiShowMessage(Closure->window, markup, GTK_MESSAGE_WARNING);
      g_free(markup);
      g_free(utf_message);
   }
#endif
   g_free(message);
}

/***
 *** Open a ECC file for an existing image
 ***/

Image* OpenEccFileForImage(Image *image, char *filename, int flags, mode_t mode)
{  int new_image = 0;
   int i;

   if(!image)
   {  new_image = 1;
      image = g_malloc0(sizeof(Image));
   }

   image->eccFile = LargeOpen(filename, flags, mode);
   if(errno == EACCES)
        image->eccFileState = ECCFILE_NOPERM;
   else image->eccFileState = ECCFILE_MISSING;
   
   if(!image->eccFile)
   {  if(new_image) 
      {  g_free(image);
	 return NULL;
      }
      return image;
   }

   /* Determine codec for ecc file */
   
   for(i=0; i<Closure->methodList->len; i++)  
   {  Method *method = g_ptr_array_index(Closure->methodList, i);

      if(method->recognizeEccFile)
      {  image->eccFileState = method->recognizeEccFile(image->eccFile, &image->eccFileHeader);
	 if(image->eccFileState == ECCFILE_PRESENT)
	 {  image->eccFileMethod = method; 
	    image->expectedSectors = method->expectedImageSize(image);
	    GetImageFingerprint(image, NULL, image->eccFileHeader->fpSector);
	    return image;
	 }
	 if(image->eccFileState == ECCFILE_CANCELLED)
	    break;  /* user stopped the search; do not try other codecs */
      }
   }

   /* bail out (unrecognized ecc file or cancelled search) */

   LargeClose(image->eccFile);
   image->eccFile = NULL;
   if(new_image) 
   {  g_free(image);
      return NULL;
   }
   return image;
}

/*
 * Report inconsistencies found in image and ecc files.
 */

int ReportImageEccInconsistencies(Image *image)
{
  /* The user stopped the action while the ecc data was being searched for.
     Whatever is missing below is a consequence of that, so abort quietly. */

  if(Closure->stopActions)
  {  if(image) CloseImage(image);
     return TRUE;
  }

  /* No image file */

  if(!image || image->type == IMAGE_NONE)
  {  if(image) CloseImage(image);

#ifdef WITH_GUI_YES
     if(Closure->guiMode)
     {     GuiCreateMessage(_("Image file %s not present or permission denied.\n"), GTK_MESSAGE_ERROR, Closure->imageName);
	   return TRUE;
     }
     else
#endif
     {     Stop(_("Image file %s not present or permission denied.\n"), Closure->imageName);
     }
  }

  /* Image file but unknown ecc */

  if(image->eccFile && !image->eccFileMethod)
  {  CloseImage(image);

#ifdef WITH_GUI_YES
     if(Closure->guiMode)
     {   GuiCreateMessage(_("\nError correction file type unknown.\n"), GTK_MESSAGE_ERROR);
	 return TRUE;
     }
     else
#endif       
     {  Stop(_("\nError correction file type unknown.\n"));
     }
  }

  /* Permission denied for ecc file */

  if(!image->eccFile && image->eccFileState == ECCFILE_NOPERM)
  {  CloseImage(image);

#ifdef WITH_GUI_YES
     if(Closure->guiMode)
     {    GuiCreateMessage(_("\nPermission denied on ecc file (perhaps not writeable?).\n"),
			  GTK_MESSAGE_ERROR);
	  return TRUE;
     }
     else
#endif
     {  Stop(_("\nPermission denied on ecc file (perhaps not writeable?).\n"));
     }
  }

  
  /* Augmented image but unknown ecc method */

  if(!image->eccFile && !image->eccMethod)
  {  CloseImage(image);

#ifdef WITH_GUI_YES
     if(Closure->guiMode)
     {    GuiCreateMessage(_("\nNo error correction file present.\n"
			     "No error correction data recognized in image.\n"), GTK_MESSAGE_ERROR);
	  return TRUE;
     }
     else
#endif
     {  Stop(_("\nNo error correction file present.\n"
	       "No error correction data recognized in image.\n"));
     }
  }

  return FALSE;
}

/***
 *** Image access functions
 ***
 * Once the image has been opened, the following functions
 * provide transparent access to it (e.g. it does not matter
 * whether the image resides in a file or on a optical drive.
 * However these functions are not optimized for recovering
 * data from a defective optical medium - go through the
 * DeviceHandle related functions in scsi-layer.c in that case.
 */

// fixme: Demand AlignedBuffer instead of void *buf 
int ImageReadSectors(Image* image, void *buf, guint64 first, int n)
{  guint64 first_defect;

   switch(image->type)
   {  case IMAGE_FILE:
        if(!LargeSeek(image->file, first*2048))
	  Stop("ImageReadSectors(): seek failed");
        if(LargeRead(image->file, buf, n*2048) == n*2048)
	{  if(CheckForMissingSectors(buf, first, NULL, 0, n, &first_defect) == SECTOR_PRESENT)
	        return n;
	   else return 0;
	}
	else return 0;
	break;
      case IMAGE_MEDIUM:
	if(!ReadSectorsFast(image->dh, buf, first, n))
	     return n;
	else return 0;
	break;
   }

   return 0;
}


/*
 * Get the md5sum from the specified sector. 
 * Results are cached in the Image as multiple queries may occur.
 */

int GetImageFingerprint(Image *image, guint8 *fp_out, guint64 sector)
{  AlignedBuffer *ab;
   int status;

   /* Sector already cached? */

   if(image->fpSector == sector)
      switch(image->fpState)
      {  case 0:    /* not read */
	    break;
	 case 1:    /* unreadable */
	    if(fp_out) 
	      memset(fp_out, 0, 16);
	    Verbose("GetImageFingerprint(%" PRId64 "): cached unreadable\n", sector);
	    return FALSE;
	 case 2:    /* already cached */
	    if(fp_out)
	      memcpy(fp_out, image->imageFP, 16);
	    Verbose("GetImageFingerprint(%" PRId64 "): cached\n", sector);
	    return TRUE;
      }

     ab = CreateAlignedBuffer(2048);
     status = ImageReadSectors(image, ab->buf, sector, 1);
   
     image->fpSector = sector;
     if(status != 1)  /* read error */
     {  image->fpState = 1;
        Verbose("GetImageFingerprint(%" PRId64 "): not readable\n", sector);
     }
     else
     {  struct MD5Context md5ctxt;
   
	image->fpState = 2;

	MD5Init(&md5ctxt);
	MD5Update(&md5ctxt, ab->buf, 2048);
	MD5Final(image->imageFP, &md5ctxt);
	if(fp_out)
	  memcpy(fp_out, image->imageFP, 16);
	Verbose("GetImageFingerprint(%" PRId64 "): read & cached\n", sector);
     }

     FreeAlignedBuffer(ab);

     return image->fpState == 2;
}

/*
 * Truncate the image, update internal structures
 */

int TruncateImage(Image *image, guint64 new_size)
{  int result;
  
   if(image->type != IMAGE_FILE)
     Stop("TruncateImage: called for non-file type image!\n");

   result = LargeTruncate(image->file, (gint64)new_size);

   CalcSectors(image->file->size, &image->sectorSize, &image->inLast);

   return result;
}

/*
 * Close the image
 */

void CloseImage(Image *image)
{
  switch(image->type)
  {  case IMAGE_FILE:
       LargeClose(image->file);
       break;

     case IMAGE_MEDIUM:
       CloseDevice(image->dh);
       break;
  }

  if(image->eccHeader)
     g_free(image->eccHeader);
  if(image->isoInfo)
     FreeIsoInfo(image->isoInfo);
  if(image->eccFile)
    LargeClose(image->eccFile);
  if(image->eccFileHeader)
    g_free(image->eccFileHeader);
  if(image->cachedLayout)
    g_free(image->cachedLayout);

  g_free(image);
}

/*
 * GNU ddrescue compatible mapfile (--mapfile, only together with -r).
 *
 * The reading code records the outcome of its read attempts here;
 * the image contents alone can not tell apart sectors which have been
 * tried and failed from sectors which have been skipped or never
 * been tried. Status of each sector in the mapfile:
 *
 *   -  read individually and failed, or read with a CRC error
 *   +  present in the image
 *   *  part of a failed multi-sector read which was not split up
 *      ("non-trimmed": ddrescue will trim/scrape it by default)
 *   ?  not tried (skipped, outside the reading range, not needed
 *      by adaptive reading, missing from an earlier session)
 *
 * The bitmaps are only allocated when --mapfile was given, so the
 * recording functions are no-ops otherwise. In linear reading the
 * reader and worker threads update them under the reader mutex.
 */

void MapfileInit(gint64 sectors)
{  if(!Closure->mapFile || sectors <= 0)
     return;

   MapfileFree();
   Closure->mapBad = CreateBitmap0(sectors);
   Closure->mapFailedBlock = CreateBitmap0(sectors);
}

void MapfileFree(void)
{  if(Closure->mapBad) FreeBitmap(Closure->mapBad);
   if(Closure->mapFailedBlock) FreeBitmap(Closure->mapFailedBlock);
   Closure->mapBad = Closure->mapFailedBlock = NULL;
}

void MapfileReadOK(gint64 sector, gint64 n)
{  Bitmap *bad = Closure->mapBad, *block = Closure->mapFailedBlock;

   if(!bad) return;
   for(; n>0 && sector >= 0 && sector < bad->size; sector++, n--)
   {  ClearBit(bad, sector);
      ClearBit(block, sector);
   }
}

void MapfileReadBad(gint64 sector, gint64 n)
{  Bitmap *bad = Closure->mapBad, *block = Closure->mapFailedBlock;

   if(!bad) return;
   for(; n>0 && sector >= 0 && sector < bad->size; sector++, n--)
   {  SetBit(bad, sector);
      ClearBit(block, sector);
   }
}

void MapfileBlockFailed(gint64 sector, gint64 n)
{  Bitmap *block = Closure->mapFailedBlock;

   if(!block) return;
   for(; n>0 && sector >= 0 && sector < block->size; sector++, n--)
      SetBit(block, sector);
}

static int map_status(gint64 sector, unsigned char *buf)
{  Bitmap *bad = Closure->mapBad, *block = Closure->mapFailedBlock;
   int known = bad && sector < bad->size;

   if(known && GetBit(bad, sector))
     return '-';
   if(CheckForMissingSector(buf, sector, NULL, 0) == SECTOR_PRESENT)
     return '+';
   if(known && GetBit(block, sector))
     return '*';
   return '?';
}

static void write_map_run(FILE *map, guint64 pos, guint64 size, int status)
{  fprintf(map, "0x%08" PRIx64 "  0x%08" PRIx64 "  %c\n", pos, size, status);
}

/*
 * Returns FALSE (after printing the reason) if the mapfile could not
 * be written. Never overwrites an existing file; a partially written
 * mapfile is removed.
 */

gboolean WriteMapfile(char *image_name, char *map_name)
{  LargeFile *image;
   FILE *map;
   unsigned char *buf;
   guint64 size, sectors, s, run_start = 0;
   int run_status = 0;
   int err = 0;
   char *what = NULL;

   if(!LargeStat(image_name, &size))
   {  PrintError(_("Could not write mapfile %s: image file %s not present.\n"),
		 map_name, image_name);
      return FALSE;
   }

   image = LargeOpen(image_name, O_RDONLY, IMG_PERMS);
   if(!image)
   {  PrintError(_("Could not write mapfile %s: can't open %s: %s\n"),
		 map_name, image_name, strerror(errno));
      return FALSE;
   }

   map = OpenNewFile(map_name);
   if(!map)
   {  PrintError(_("Could not write mapfile %s: %s\n"), map_name, strerror(errno));
      LargeClose(image);
      return FALSE;
   }

   fprintf(map, "# Mapfile. Created by dvdisaster\n"
	        "# current_pos  current_status  current_pass\n"
	        "0x00000000     +               1\n"
	        "#      pos        size  status\n");

   buf = g_malloc(64*2048);
   sectors = (size+2047)/2048;

   for(s=0; s<sectors; s+=64)
   {  int i, n = MIN(64, sectors-s);
      gint64 expected = MIN((guint64)n*2048, size-s*2048);
      gint64 bytes = LargeRead(image, buf, expected);

      if(bytes != expected)
      {  err = bytes < 0 && errno ? errno : EIO;
	 what = image_name;
	 break;
      }
      if(bytes < n*2048)   /* incomplete last sector */
	 memset(buf+bytes, 0, n*2048-bytes);

      for(i=0; i<n; i++)
      {  int status = map_status(s+i, buf+2048*i);

	 if(status != run_status)
	 {  if(run_status)
	       write_map_run(map, run_start*2048, (s+i-run_start)*2048, run_status);
	    run_status = status;
	    run_start = s+i;
	 }
      }
      if(ferror(map))
	 break;
   }

   if(!err && run_status)
      write_map_run(map, run_start*2048, size-run_start*2048, run_status);

   if(!err && ferror(map))
   {  err = errno ? errno : EIO;
      what = map_name;
   }
   if(fclose(map) == EOF && !err)
   {  err = errno ? errno : EIO;
      what = map_name;
   }

   g_free(buf);
   LargeClose(image);

   if(err)
   {  if(what == image_name)
	   PrintError(_("Could not write mapfile %s: error reading %s: %s\n"),
		      map_name, image_name, strerror(err));
      else PrintError(_("Could not write mapfile %s: %s\n"), map_name, strerror(err));
      g_unlink(map_name);  /* created exclusively by us above */
      return FALSE;
   }

   PrintLog(_("Mapfile written to %s.\n"), map_name);
   return TRUE;
}
