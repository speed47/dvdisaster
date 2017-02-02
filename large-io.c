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

/***
 *** Wrappers around the standard low level file system interface.
 ***
 * This is pointless for GNU/Linux, but gives us the possibility to 
 * hide differences in OS-specific semantics.
 *
 * Note the different return value semantics from standard functions:
 * - LargeOpen() returns a LargeFile pointer on success and NULL otherwise;
 * - LargeRead() and LargeWrite() return the number of bytes read/written;
 * - the remaining functions return True on success or False on failure.
 *
 * Also, individual behaviour may deviate from standard functions. 
 */

/*
 * convert special chars in file names to correct OS encoding
 */

static gchar* os_path(char *path_in)
{  gchar *cp_path = g_filename_from_utf8(path_in, -1, NULL, NULL, NULL);

   if(cp_path == NULL)
   {  errno = EINVAL;
      return NULL;
   }
   
   REMEMBER(cp_path);
   return cp_path;
}

/*
 * Large stat replacement (queries only file size)
 */

int LargeStat(char *path, guint64 *length_return)
{  struct stat mystat;
   gchar *cp_path = os_path(path);

   if(!cp_path) return FALSE;

   if(stat(cp_path, &mystat) == -1)
   {  g_free(cp_path);
      return FALSE;
   }
   g_free(cp_path);

   if(!S_ISREG(mystat.st_mode))
      return FALSE;

   *length_return = mystat.st_size;
   return TRUE;
}

/*
 * Stat() variant for testing directories
 */

int DirStat(char *path)
{  struct stat mystat;
   gchar *cp_path = os_path(path);

   if(!cp_path) return FALSE;

   if(stat(cp_path, &mystat) == -1)
   {  g_free(cp_path);
      return FALSE;
   }
   g_free(cp_path);

   if(!S_ISDIR(mystat.st_mode))
     return FALSE;

   return TRUE;
}

/*
 * Open a file
 */

LargeFile* LargeOpen(char *name, int flags, mode_t mode)
{  LargeFile *lf = g_malloc0(sizeof(LargeFile));
   struct stat mystat;
   gchar *cp_path;

#ifdef HAVE_O_LARGEFILE
   flags |= O_LARGEFILE;
#endif

   cp_path = os_path(name);
   if(!cp_path) 
   {  g_free(lf); return FALSE;
   }

   /* Do not try to open directories etc. */

   if(    (stat(cp_path, &mystat) == 0)
       && !S_ISREG(mystat.st_mode))
   {  g_free(cp_path), g_free(lf); return NULL;
   }

   lf->fileHandle = open(cp_path, flags, mode);
   g_free(cp_path);

   if(lf->fileHandle == -1)
   {  g_free(lf); return NULL;
   }

   lf->path = g_strdup(name);
   LargeStat(name, &lf->size);  /* Do NOT use cp_path! */

   lf->flags = flags;
   return lf;
}

/*
 * Seeking in large files.
 * Note: Seeking beyond the end of a split file is undefined.
 */

int LargeSeek(LargeFile *lf, off_t pos)
{  
   lf->offset = pos;
   if(lseek(lf->fileHandle, pos, SEEK_SET) != pos)
      return FALSE;

   return TRUE;
}

/*
 * EOF predicate for large files.
 *
 * Note: Works only correctly for read only files!
 */

int LargeEOF(LargeFile *lf)
{  
   return lf->offset >= lf->size;
}

/*
 * Reading large files
 */

ssize_t LargeRead(LargeFile *lf, void *buf, size_t count)
{  ssize_t n;

   n = read(lf->fileHandle, buf, count);
   lf->offset += n;

   return n;
}

/*
 * Writing large files
 */

static void insert_buttons(GtkDialog *dialog)
{  
  gtk_dialog_add_buttons(dialog, 
			 GTK_STOCK_REDO , 1,
			 GTK_STOCK_CANCEL, 0, NULL);
} 

static ssize_t xwrite(int fdes, void *buf_base, size_t count)
{  unsigned char *buf = (unsigned char*)buf_base;
   ssize_t total = 0;

   /* Simply fail when going out of space in command line mode */

   if(!Closure->guiMode)
   {  while(count)
      {  ssize_t n = write(fdes, buf, count);
      
	 if(n<=0) return total;  /* error occurred */

	 if(n>0)  /* write at least partially successful */
	 {  total += n;
	    count -= n;
	    buf   += n;
	 }
      }
      return total;
   }

   /* Give the user a chance to free more space in GUI mode.
      When running out of space, the last write() may complete
      with n<count but no error condition, so we try writing
      until a real error hits (n = -1). */

   while(count)
   {  ssize_t n = write(fdes, buf, count);

      if(n <= 0) /* error occurred */
      {  int answer; 

	 if(errno != ENOSPC) return total;

	 answer = ModalDialog(GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, insert_buttons,
			      _("Error while writing the file:\n\n%s\n\n"
				"You can redo this operation after freeing some space."),
			      strerror(errno),n);

	 if(!answer) return total; 
      }

      if(n>0)  /* write at least partially successful */
      {  total += n;
	 count -= n;
	 buf   += n;
      }
   }

   return total;
}

ssize_t LargeWrite(LargeFile *lf, void *buf, size_t count)
{  ssize_t n;

   n = xwrite(lf->fileHandle, buf, count);
   lf->offset += n;

   return n;
}

/*
 * Large file closing
 */

int LargeClose(LargeFile *lf)
{  int result = TRUE;

   result = (close(lf->fileHandle) == 0);

   /* Free the LargeFile struct and return results */

   if(lf->path) g_free(lf->path);
   g_free(lf);

   return result;
}

/*
 * Large file truncation
 */

int LargeTruncate(LargeFile *lf, off_t length)
{  int result;

   result = (ftruncate(lf->fileHandle, length) == 0);

   if(result)
     lf->size = length;

   return result;
}

/*
 * Large file unlinking
 */

int LargeUnlink(char *path)
{  gchar *cp_path;
   int result;

   cp_path = os_path(path);
   if(!cp_path) return FALSE;

   result = unlink(cp_path);
   g_free(cp_path);

   return result == 0;
}

/***
 *** Wrappers around other IO
 ***/

FILE *portable_fopen(char *path, char *modes)
{  FILE *file;
   char *cp_path;

   cp_path = os_path(path);
   file = fopen(cp_path, modes);
   g_free(cp_path);

   return file;
}

/***
 *** Convenience functions
 ***/

/*
 * Append the given file suffix if none is already present
 */

char *ApplyAutoSuffix(char *filename, char *suffix)
{  char *out;

   if(!filename || !*filename || strrchr(filename, '.')) 
     return filename;

   out = g_strdup_printf("%s.%s",filename,suffix);
   g_free(filename);
   
   return out;
}
