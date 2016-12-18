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
   
   if(!image->eccFile)
   {  if(new_image) 
      {  g_free(image);
         return NULL;
      }
      image->eccFileState = ECCFILE_MISSING;
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
      }
   }

   /* bail out (unrecognized ecc file) */

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
  /* No image file */

  if(!image || image->type == IMAGE_NONE)
  {  if(image) CloseImage(image);
     if(Closure->guiMode)
     {     CreateMessage(_("Image file %s not present or permission denied.\n"), GTK_MESSAGE_ERROR, Closure->imageName);
	   return TRUE;
     }
     else
     {     Stop(_("Image file %s not present or permission denied.\n"), Closure->imageName);
     }
  }

  /* Image file but unknown ecc */

  if(image->eccFile && !image->eccFileMethod)
  {  CloseImage(image);
     if(Closure->guiMode)
     {   CreateMessage(_("\nError correction file type unknown.\n"), GTK_MESSAGE_ERROR);
	 return TRUE;
     }
     else
     {  Stop(_("\nError correction file type unknown.\n"));
     }
  }
  
  /* Augmented image but unknown ecc method */

  if(!image->eccFile && !image->eccMethod)
  {  CloseImage(image);
     if(Closure->guiMode)
     {    CreateMessage(_("\nNo error correction file present.\n"
			  "No error correction data recognized in image.\n"), GTK_MESSAGE_ERROR);
	  return TRUE;
     }
     else
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
	    Verbose("GetImageFingerprint(%lld): cached unreadable\n", sector);
	    return FALSE;
	 case 2:    /* already cached */
	    if(fp_out)
	      memcpy(fp_out, image->imageFP, 16);
	    Verbose("GetImageFingerprint(%lld): cached\n", sector);
	    return TRUE;
      }

     ab = CreateAlignedBuffer(2048);
     status = ImageReadSectors(image, ab->buf, sector, 1);
   
     image->fpSector = sector;
     if(status != 1)  /* read error */
     {  image->fpState = 1;
        Verbose("GetImageFingerprint(%lld): not readable\n", sector);
     }
     else
     {  struct MD5Context md5ctxt;
   
	image->fpState = 2;

	MD5Init(&md5ctxt);
	MD5Update(&md5ctxt, ab->buf, 2048);
	MD5Final(image->imageFP, &md5ctxt);
	if(fp_out)
	  memcpy(fp_out, image->imageFP, 16);
	Verbose("GetImageFingerprint(%lld): read & cached\n", sector);
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
