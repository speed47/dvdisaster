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
 *** Search a directory for the .ecc file matching the image
 ***
 * Only *.ecc files directly inside the directory are examined.
 * A file matches if its header fingerprint equals the image fingerprint;
 * copies with identical headers count as one match.
 * Returns the newly allocated path of the single match, or NULL with
 * *state telling why: ECCFILE_MISSING (no match, or the fingerprint
 * could not be read; the caller then proceeds as without an ecc file)
 * or ECCFILE_CANCELLED; except when cancelled, the exit code is set
 * to failure since the ecc file was asked for explicitly.
 * Stops in CLI mode if the directory can not be read or more than
 * one file matches; the GUI calls this from its main thread where
 * Stop() is not usable, so it shows the error and proceeds as
 * without an ecc file instead.
 */

static gint compare_names(gconstpointer a, gconstpointer b)
{  return strcmp(*(char**)a, *(char**)b);
}

static void find_ecc_error(char *format, ...) PRINTF_FORMAT(1);

static void find_ecc_error(char *format, ...)
{  va_list argp;
   char *msg;

   va_start(argp, format);
   msg = g_strdup_vprintf(format, argp);
   va_end(argp);

   if(!Closure->guiMode)
     Stop("%s", msg);

   LogWarning("%s", msg);
#ifdef WITH_GUI_YES
   {  char *utf_msg = g_locale_to_utf8(msg, -1, NULL, NULL, NULL);
      char *markup = g_markup_escape_text(utf_msg ? utf_msg : msg, -1);

      GuiShowMessage(Closure->window, markup, GTK_MESSAGE_ERROR);
      g_free(markup);
      g_free(utf_msg);
   }
#endif
   g_free(msg);
}

static char *find_ecc_file(Image *image, char *dir, int *state)
{  GError *err = NULL;
   GDir *gdir = g_dir_open(dir, 0, &err);
   GPtrArray *names;
   const char *name;
   char *found = NULL;
   EccHeader *found_eh = NULL;
   int fp_unreadable = FALSE;
   int ambiguous = FALSE;
   int n;

   *state = ECCFILE_MISSING;

   if(!gdir)
   {  find_ecc_error(_("Can't open %s:\n%s\n"), dir, err->message);
      g_error_free(err);
      return NULL;
   }

   names = g_ptr_array_new_with_free_func(g_free);
   while((name = g_dir_read_name(gdir)))
     if(strlen(name) >= 4 && !g_ascii_strcasecmp(name+strlen(name)-4, ".ecc"))
       g_ptr_array_add(names, g_build_path("/", dir, name, NULL));
   g_dir_close(gdir);

   /* sorted for reproducible results */

   g_ptr_array_sort(names, (GCompareFunc)compare_names);

   for(n=0; n<names->len && *state != ECCFILE_CANCELLED; n++)
   {  char *path = g_ptr_array_index(names, n);
      LargeFile *file = LargeOpen(path, O_RDONLY, IMG_PERMS);
      EccHeader *eh = NULL;
      int i, match = FALSE;

      if(!file)
	continue;

      for(i=0; i<Closure->methodList->len; i++)
      {  Method *method = g_ptr_array_index(Closure->methodList, i);
	 int result;

	 if(!method->recognizeEccFile)
	   continue;

	 result = method->recognizeEccFile(file, &eh);
	 if(result == ECCFILE_CANCELLED)
	 {  *state = ECCFILE_CANCELLED;
	    break;
	 }
	 if(result == ECCFILE_PRESENT)
	 {  guint8 fp[16];

	    if(GetImageFingerprint(image, fp, eh->fpSector))
	         match = !memcmp(fp, eh->mediumFP, 16);
	    else fp_unreadable = TRUE;
	    break;
	 }
      }
      LargeClose(file);

      Verbose("find_ecc_file(): %s %s\n", path, match ? "matches" : "does not match");
      if(!match)
      {  g_free(eh);
	 continue;
      }

      /* A copy of the file already found is not ambiguous */

      if(found && !memcmp(eh, found_eh, sizeof(EccHeader)))
      {  Verbose("find_ecc_file(): %s has the same header as %s, ignored\n", path, found);
	 g_free(eh);
	 continue;
      }

      if(found)
      {  find_ecc_error(_("Both %s and %s match the image; please select one with -e.\n"), found, path);
	 g_free(eh);
	 g_free(found);
	 found = NULL;
	 ambiguous = TRUE;
	 break;
      }

      found = g_strdup(path);
      found_eh = eh;
   }
   g_ptr_array_free(names, TRUE);
   g_free(found_eh);

   if(*state == ECCFILE_CANCELLED)
   {  g_free(found);
      return NULL;
   }

   if(found)
   {  PrintLog(_("Using ecc file %s\n"), found);
      *state = ECCFILE_PRESENT;
   }
   else
   {  /* The ecc file was asked for explicitly: do not report success
	 although the action went on without it. */

      SetExitCode(EXIT_FAILURE, EXIT_FAILURE);
      if(fp_unreadable)
	LogWarning(_("The fingerprint sector of the image is unreadable, so no ecc file in %s\n"
		  "can be matched to it. Continuing without an ecc file;\n"
		  "select the ecc file with -e to use it.\n"), dir);
      else if(!ambiguous)
	LogWarning(_("No ecc file in %s matches the image; continuing without an ecc file.\n"), dir);
   }

   return found;
}

/***
 *** Open a ECC file for an existing image
 ***/

Image* OpenEccFileForImage(Image *image, char *filename, int flags, mode_t mode)
{  int new_image = 0;
   int i;

   /* A directory was given: pick the matching ecc file from it.
      All callers pass Closure->eccName, which is updated so that
      later messages and reopens refer to the selected file. */

   if(image && filename && g_file_test(filename, G_FILE_TEST_IS_DIR))
   {  int state;
      char *found = find_ecc_file(image, filename, &state);

      if(!found)  /* proceed as if no ecc file had been given */
      {  image->eccFile = NULL;
	 image->eccFileState = state;
	 return image;
      }
      g_free(Closure->eccName);
      filename = Closure->eccName = found;
   }

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
 *      by adaptive reading)
 *
 * An existing mapfile (from an earlier dvdisaster or ddrescue run on
 * the same image) is updated like ddrescue does: sectors which are
 * still missing and were not tried in this session keep their failed
 * status (-, / or *) from the earlier mapfile.
 *
 * The bitmaps are only allocated when --mapfile was given, so the
 * recording functions are no-ops otherwise. In linear reading the
 * reader and worker threads update them under the reader mutex.
 */

typedef struct
{  guint64 pos, size;
   char status;
} map_run;

static void free_bitmaps(void)
{  if(Closure->mapBad) FreeBitmap(Closure->mapBad);
   if(Closure->mapFailedBlock) FreeBitmap(Closure->mapFailedBlock);
   Closure->mapBad = Closure->mapFailedBlock = NULL;
   Closure->mapSectors = 0;
}

void MapfileInit(gint64 sectors)
{  if(!Closure->mapFile || sectors <= 0)
     return;

   free_bitmaps();
   Closure->mapBad = CreateBitmap0(sectors);
   Closure->mapFailedBlock = CreateBitmap0(sectors);
   Closure->mapSectors = sectors;
}

/* The medium ends earlier than reported, e.g. at the two run-out
   sectors of a TAO CD which are truncated from the image: the mapfile
   does not list the sectors beyond. */

void MapfileTruncate(gint64 sectors)
{  if(sectors >= 0 && sectors < Closure->mapSectors)
      Closure->mapSectors = sectors;
}

void MapfileFree(void)
{  free_bitmaps();
   if(Closure->mapPrevious)
     g_array_free(Closure->mapPrevious, TRUE);
   Closure->mapPrevious = NULL;
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

/*
 * Parse an existing ddrescue mapfile. Returns FALSE and sets *error
 * (to be g_free()d) if the file is not a valid mapfile.
 * On success, the blocks are stored for merging them into the
 * mapfile written after reading.
 */

#define MAX_MAPFILE_SIZE (64*1024*1024)

static gboolean parse_number(char *token, guint64 *value)
{  char *end;

   if(!g_ascii_isdigit(*token))
     return FALSE;
   errno = 0;
   *value = g_ascii_strtoull(token, &end, 0);
   return !errno && !*end;
}

gboolean ReadPreviousMapfile(char *path, char **error)
{  GArray *runs;
   char *contents, **lines;
   gsize length;
   GError *gerr = NULL;
   gboolean have_status_line = FALSE;
   guint64 end = 0;
   int i;

   *error = NULL;
   if(!g_file_get_contents(path, &contents, &length, &gerr))
   {  *error = g_strdup(gerr->message);
      g_error_free(gerr);
      return FALSE;
   }
   if(length > MAX_MAPFILE_SIZE || memchr(contents, 0, length))
   {  g_free(contents);
      *error = g_strdup(_("not a ddrescue mapfile"));
      return FALSE;
   }

   runs = g_array_new(FALSE, FALSE, sizeof(map_run));
   lines = g_strsplit(contents, "\n", -1);
   g_free(contents);

   for(i=0; lines[i] && !*error; i++)
   {  char **tok;
      int n, j;

      g_strstrip(lines[i]);
      if(!*lines[i] || *lines[i] == '#')
	continue;

      tok = g_strsplit_set(lines[i], " \t", -1);
      n = 0;
      for(j=0; tok[j]; j++)   /* compact away empty tokens */
	if(*tok[j]) tok[n++] = tok[j];
	else g_free(tok[j]);
      tok[n] = NULL;

      if(!have_status_line)   /* current_pos current_status [current_pass] */
      {  guint64 pos;

	 if(n < 2 || n > 3 || !parse_number(tok[0], &pos) || strlen(tok[1]) != 1)
	    *error = g_strdup_printf(_("invalid status line %d"), i+1);
	 have_status_line = TRUE;
      }
      else   /* pos size status */
      {  map_run run;

	 if(   n != 3 || !parse_number(tok[0], &run.pos)
	    || !parse_number(tok[1], &run.size) || run.size == 0
	    || strlen(tok[2]) != 1 || !strchr("?*/-+", tok[2][0]))
	    *error = g_strdup_printf(_("invalid block in line %d"), i+1);
	 else if(run.pos < end || run.pos + run.size < run.pos)
	    *error = g_strdup_printf(_("overlapping or unsorted block in line %d"), i+1);
	 else
	 {  run.status = tok[2][0];
	    g_array_append_val(runs, run);
	    end = run.pos + run.size;
	 }
      }
      g_strfreev(tok);
   }
   g_strfreev(lines);

   if(!*error && !have_status_line)
      *error = g_strdup(_("not a ddrescue mapfile"));
   if(*error)
   {  g_array_free(runs, TRUE);
      return FALSE;
   }

   if(Closure->mapPrevious)
     g_array_free(Closure->mapPrevious, TRUE);
   Closure->mapPrevious = runs;
   return TRUE;
}

/* Ranking of the failed states; a later state wins when merging */

static int failed_rank(int status)
{  switch(status)
   {  case '*': return 1;   /* non-trimmed */
      case '/': return 2;   /* non-scraped */
      case '-': return 3;   /* bad sector */
      default:  return 0;
   }
}

/* Worst failed status of the previous mapfile in [from, to),
   or '?' if there is none. *cursor speeds up ascending lookups. */

static int previous_status(guint64 from, guint64 to, guint *cursor)
{  GArray *runs = Closure->mapPrevious;
   int status = '?';
   guint i;

   if(!runs) return '?';

   while(*cursor < runs->len)
   {  map_run *r = &g_array_index(runs, map_run, *cursor);

      if(r->pos + r->size > from) break;
      (*cursor)++;
   }

   for(i=*cursor; i<runs->len; i++)
   {  map_run *r = &g_array_index(runs, map_run, i);

      if(r->pos >= to) break;
      if(failed_rank(r->status) > failed_rank(status))
	 status = r->status;
   }
   return status;
}

static int map_status(gint64 sector, unsigned char *buf, guint *cursor)
{  Bitmap *bad = Closure->mapBad, *block = Closure->mapFailedBlock;
   int known = bad && sector < bad->size;
   int status, previous;

   if(known && GetBit(bad, sector))
     return '-';
   if(buf && CheckForMissingSector(buf, sector, NULL, 0) == SECTOR_PRESENT)
     return '+';   /* buf is NULL for medium sectors beyond the image end */

   status = known && GetBit(block, sector) ? '*' : '?';
   previous = previous_status(2048*sector, 2048*sector+2048, cursor);
   return failed_rank(previous) > failed_rank(status) ? previous : status;
}

/* Collects adjacent areas with the same status into one block line */

typedef struct
{  FILE *file;
   guint64 pos, size;
   int status;
} map_writer;

static void map_emit(map_writer *w, guint64 pos, guint64 size, int status)
{  if(w->size && w->status == status && w->pos + w->size == pos)
   {  w->size += size;
      return;
   }
   if(w->size)
      fprintf(w->file, "0x%08" PRIx64 "  0x%08" PRIx64 "  %c\n", w->pos, w->size, w->status);
   w->pos = pos; w->size = size; w->status = status;
}

/*
 * Returns FALSE (after printing the reason) if the mapfile could not
 * be written. A new mapfile is created exclusively; an existing one
 * (validated by ReadPreviousMapfile() before reading) is replaced
 * through a temporary file, so it is never lost on errors.
 */

gboolean WriteMapfile(char *image_name, char *map_name)
{  LargeFile *image;
   map_writer w = { NULL, 0, 0, 0 };
   unsigned char *buf;
   guint64 size, sectors, s, covered;
   guint cursor = 0;
   gboolean update = Closure->mapPrevious != NULL;
   char *out_name = update ? g_strdup_printf("%s.tmp", map_name) : g_strdup(map_name);
   int err = 0;
   gboolean read_error = FALSE;

   if(!LargeStat(image_name, &size))
   {  PrintError(_("Could not write mapfile %s: image file %s not present.\n"),
		 map_name, image_name);
      g_free(out_name);
      return FALSE;
   }

   image = LargeOpen(image_name, O_RDONLY, IMG_PERMS);
   if(!image)
   {  PrintError(_("Could not write mapfile %s: can't open %s: %s\n"),
		 map_name, image_name, strerror(errno));
      g_free(out_name);
      return FALSE;
   }

   w.file = OpenNewFile(out_name);
   if(!w.file)
   {  PrintError(_("Could not write mapfile %s: %s\n"), out_name, strerror(errno));
      LargeClose(image);
      g_free(out_name);
      return FALSE;
   }

   /* A read stopped by --time-limit/--abort-file is still in its
      first (copying non-tried) phase at the image end. */

   fprintf(w.file, "# Mapfile. Created by dvdisaster\n"
	           "# current_pos  current_status  current_pass\n");
   if(Closure->stopReason)
        fprintf(w.file, "0x%08" PRIx64 "     ?               1\n", size);
   else fprintf(w.file, "0x00000000     +               1\n");
   fprintf(w.file, "#      pos        size  status\n");

   buf = g_malloc(64*2048);
   sectors = (size+2047)/2048;

   for(s=0; s<sectors && !ferror(w.file); s+=64)
   {  int i, n = MIN(64, sectors-s);
      gint64 expected = MIN((guint64)n*2048, size-s*2048);
      gint64 bytes = LargeRead(image, buf, expected);

      if(bytes != expected)
      {  err = bytes < 0 && errno ? errno : EIO;
	 read_error = TRUE;
	 break;
      }
      if(bytes < n*2048)   /* incomplete last sector */
	 memset(buf+bytes, 0, n*2048-bytes);

      for(i=0; i<n; i++)
      {  guint64 pos = (s+i)*2048;

	 map_emit(&w, pos, MIN(2048, size-pos), map_status(s+i, buf+2048*i, &cursor));
      }
   }

   /* Medium sectors beyond the image end, e.g. after a stopped read:
      bad (-) or failed (*) as recorded during reading, otherwise
      non-tried (?). Only for an image of whole sectors, so that the
      blocks stay contiguous. */

   covered = size;
   if(!err && Closure->mapBad && size % 2048 == 0)
   {  for(s=sectors; s<(guint64)Closure->mapSectors; s++)
	 map_emit(&w, s*2048, 2048, map_status(s, NULL, &cursor));
      covered = MAX(size, 2048*(guint64)Closure->mapSectors);
   }

   /* Keep the previous mapfile's areas beyond that;
      their data is not in the image (any more), though. */

   if(!err && update)
   {  guint i;

      for(i=0; i<Closure->mapPrevious->len; i++)
      {  map_run *r = &g_array_index(Closure->mapPrevious, map_run, i);
	 guint64 start = MAX(r->pos, covered);

	 if(r->pos + r->size > start)
	    map_emit(&w, start, r->pos + r->size - start,
		     r->status == '+' ? '?' : r->status);
      }
   }
   if(!err)
      map_emit(&w, 0, 0, 0);   /* flush the last block */

   if(!err && ferror(w.file))
      err = errno ? errno : EIO;
   if(fclose(w.file) == EOF && !err)
      err = errno ? errno : EIO;

   if(!err && update && g_rename(out_name, map_name))
      err = errno ? errno : EIO;

   g_free(buf);
   LargeClose(image);

   if(err)
   {  if(read_error)
	   PrintError(_("Could not write mapfile %s: error reading %s: %s\n"),
		      map_name, image_name, strerror(err));
      else PrintError(_("Could not write mapfile %s: %s\n"), map_name, strerror(err));
      g_unlink(out_name);  /* created exclusively by us above */
      g_free(out_name);
      return FALSE;
   }

   if(update)
        PrintLog(_("Mapfile %s updated.\n"), map_name);
   else PrintLog(_("Mapfile written to %s.\n"), map_name);
   g_free(out_name);
   return TRUE;
}
