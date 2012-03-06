/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2004-2011 Carsten Gnoerlich.
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

#ifdef SYS_MINGW
 #include <windows.h>
 #include <tlhelp32.h>
 #include <psapi.h>
 #include <shlobj.h>

 /* safety margin in case we're getting UTF path names */
 #define WIN_MAX_PATH (4*MAX_PATH)
#endif

#if 0 
 #define Verbose g_printf
#else
 #define Verbose(format, ...)
#endif

/***
 *** Locate the binary and documentation directory
 ***/

/* 
 * Find location of special windows directories.
 * Copied from glib sources since they have declared it static.
 * Windows only.
 * CHECKME: Is it okay to return UTF8?
 */ 

#ifdef SYS_MINGW
static gchar *get_special_folder(int csidl)
{
  union 
  { char c[WIN_MAX_PATH+1];
    wchar_t wc[WIN_MAX_PATH+1];
  } path;

  HRESULT hr;
  LPITEMIDLIST pidl = NULL;
  BOOL b;
  gchar *retval = NULL;

  hr = SHGetSpecialFolderLocation(NULL, csidl, &pidl);
  if(hr == S_OK)
  {  if (G_WIN32_HAVE_WIDECHAR_API())
     {  b = SHGetPathFromIDListW(pidl, path.wc);
	if(b)
	  retval = g_utf16_to_utf8(path.wc, -1, NULL, NULL, NULL);
     }
     else
     {  b = SHGetPathFromIDListA(pidl, path.c);
        if(b)
	  retval = g_locale_to_utf8(path.c, -1, NULL, NULL, NULL);
     }
     CoTaskMemFree(pidl);
  }
  return retval;
}
#endif

/*
 * Find the place of our executable
 * (Windows only)
 */

#ifdef SYS_MINGW
static char* get_exe_path()
{  char path[WIN_MAX_PATH];
   int n = GetModuleFileNameA(NULL, path, WIN_MAX_PATH);

   if(n>0 && n<WIN_MAX_PATH-1)
   {  char *backslash = strrchr(path, '\\');

      if(backslash) *backslash=0;
      
      return g_strdup(path);
   }
 
   return NULL;
}
#endif

/*
 * Create / compare signatures for our installation directory
 */

#ifdef SYS_MINGW
static unsigned char *create_signature()
{  struct MD5Context md5ctxt;
   unsigned char *digest = g_malloc(20);
   char buf[80];

   sprintf(buf,"dvdisaster %s signature string", Closure->cookedVersion);

   MD5Init(&md5ctxt);
   MD5Update(&md5ctxt, buf, strlen(buf));
   MD5Final(digest+2, &md5ctxt);
   digest[ 0] = digest[ 7]+19;
   digest[ 1] = digest[ 9]+58;
   digest[18] = digest[12]+31;
   digest[19] = digest[15]+5;

   return digest;
}

void WriteSignature()
{  char loc[strlen(Closure->binDir) + strlen("\\signature")+ 10];
   unsigned char *sig = create_signature();
   FILE *file;

   /* processing of error conditions not necessary */

   sprintf(loc, "%s\\signature", Closure->binDir);
   if(!(file = portable_fopen(loc, "wb")))
     return;
   fwrite(sig, 20, 1, file);
   fclose(file);
   g_free(sig);
}

int VerifySignature()
{  char loc[strlen(Closure->binDir) + strlen("\\signature")+ 10];
   unsigned char *sig = create_signature();
   char buf[20];
   FILE *file;
   int result;

   sprintf(loc, "%s\\signature", Closure->binDir);
   if(!(file = portable_fopen(loc, "rb")))
     return FALSE;

   fread(buf, 20, 1, file);
   fclose(file);

   result = !memcmp(sig, buf, 20);

   return result;
}

#endif


static void get_base_dirs()
{  
#ifdef SYS_MINGW
   char *appdata;
   char *homedir;
#endif
#ifdef SYS_DARWIN
   char *resdir;
#endif

   /*** Unless completely disabled through a configure option, the
	source directory is supposed to hold the most recent files,
	so try this first. Not necessary under Windows as it will always
	use the directory the binary has been called from. */

#ifdef WITH_EMBEDDED_SRC_PATH_YES

#ifndef SYS_MINGW
   if(DirStat(SRCDIR))
   {  Closure->binDir = g_strdup(SRCDIR);
      Closure->docDir = g_strdup_printf("%s/documentation",SRCDIR);
      Verbose("Using paths from SRCDIR = %s\n", SRCDIR);
      goto find_dotfile;
   } 
#endif

#endif /* WITH_EMBEDDED_SRC_PATH_YES */

   /*** Otherwise try the installation directory. 
	On Unices this is a hardcoded directory.
	Windows has binary distributions with no prior known installation place,
	but luckily it provides a way for figuring out that location. 
        Darwin is like Unix when installed via make install;
        but tricky when app bundles are used. In that case we
        use GTK_PATH to find out our location. */

#if defined(SYS_LINUX) || defined(SYS_FREEBSD) || defined(SYS_NETBSD) || defined(SYS_UNKNOWN) || defined(SYS_SOLARIS)
   if(DirStat(BINDIR))
     Closure->binDir = g_strdup(BINDIR);

   if(DirStat(DOCDIR))
     Closure->docDir = g_strdup(DOCDIR);
   Verbose("Using hardcoded BINDIR = %s, DOCDIR = %s\n", BINDIR, DOCDIR);
#endif

#ifdef SYS_DARWIN
   resdir = (char*)g_getenv("GTK_PATH");
   if(resdir)  /* looks like we are inside an app bundle */
   {  if(DirStat(resdir))
         Closure->binDir = g_strdup(resdir);

      Closure->docDir = g_strdup_printf("%s/doc", resdir);
      if(!DirStat(Closure->docDir))
      {  g_free(Closure->docDir);
	 Closure->docDir = NULL;
      }
   }
   else  /* maybe a normal install */
   {
      if(DirStat(BINDIR))
         Closure->binDir = g_strdup(BINDIR);

      if(DirStat(DOCDIR))
         Closure->docDir = g_strdup(DOCDIR);
   }
   Verbose("Using BINDIR = %s, DOCDIR = %s\n", BINDIR, DOCDIR);
#endif

#ifdef SYS_MINGW
   Closure->binDir = get_exe_path();

   if(Closure->binDir)
      Closure->docDir = g_strdup_printf("%s\\documentation", Closure->binDir);
   Verbose("Using path from get_exe_path() = %s\n", Closure->binDir);
#endif

   /*** The location of the dotfile depends on the operating system. 
	Under Unix the users home directory is used. */

#ifndef SYS_MINGW
 #ifdef WITH_EMBEDDED_SRC_PATH_YES
find_dotfile:
 #endif /* WITH_EMBEDDED_SRC_PATH_YES */
   
   Closure->homeDir = g_strdup(g_getenv("HOME"));
   Closure->dotFile = g_strdup_printf("%s/.dvdisaster", Closure->homeDir);
#endif

#ifdef SYS_MINGW
   /* For Windows the user's application directory in the roaming
      profile is preferred for the dotfile; 
      if it does not exist we use the installation directory.  */

   appdata = get_special_folder(CSIDL_APPDATA);
   homedir = get_special_folder(CSIDL_PERSONAL);
   Verbose("Windows specific paths:\n"
	   "- CSIDL_APPDATA: %s\n"
	   "- CSIDL_PERSONAL: %s\n",
	   appdata ? appdata : "NULL",
	   homedir ? homedir : "NULL");

   if(homedir && DirStat(homedir))
   {
      Closure->homeDir = g_strdup(homedir);
      Verbose("- homedir path : %s\n", Closure->homeDir);

      g_free(homedir);
   }

   if(appdata)
   {  Closure->appData  = g_strdup_printf("%s\\dvdisaster", appdata);

      if(DirStat(appdata)) /* CSIDL_APPDATA present? */
      { 
	 Verbose("- dotfile path : %s\n", Closure->appData);

	 if(DirStat(Closure->appData))
	 {  Closure->dotFile = g_strdup_printf("%s\\.dvdisaster", Closure->appData);
	    Verbose("- dotfile path : present\n");
	 }
	 else if(!portable_mkdir(Closure->appData)) /* Note: Windows! */
	 {  Closure->dotFile = g_strdup_printf("%s\\.dvdisaster", Closure->appData);
	    Verbose("- dotfile path : - created -\n");
	 }
	 else 
	 {  g_free(Closure->appData);
	    Closure->appData = NULL;
	 }
      }
      else 
      {	 Verbose("- dotfile path : *can not be used*\n");
	 g_free(Closure->appData);
	 Closure->appData = NULL;
      }

      g_free(appdata);
   }

   /* Fallbacks: Expect .dvdisaster file in binDir;
                 propose C:\Windows\Temp as working dir */

   if(!Closure->dotFile)
      Closure->dotFile = g_strdup_printf("%s\\.dvdisaster", Closure->binDir);
   if(!Closure->homeDir)
      Closure->homeDir = g_strdup("C:\\Windows\\Temp");
#endif

   Verbose("\nUsing file locations:\n"
	   "- Homedir: %s\n"
	   "- Bin dir: %s\n"
	   "- Doc dir: %s\n"
	   "- AppData: %s\n"
	   "- dotfile: %s\n\n",
	   Closure->homeDir,
	   Closure->binDir,
	   Closure->docDir,
	   Closure->appData ? Closure->appData : "Null",
	   Closure->dotFile);   
}

/***
 *** Set/get color values
 ***/

/*
 * Update color string for the <span color="#f00baa">...</span> string
 */

void UpdateMarkup(char **string, GdkColor *color)
{  int hexval;
 
   hexval  = (color->red  << 8) & 0xff0000;
   hexval |=  color->green      & 0xff00;
   hexval |= (color->blue >> 8) & 0xff;

   if(*string) g_free(*string);
   *string = g_strdup_printf("color=\"#%06x\"", hexval);
}

/*
 * Default color values
 */

void DefaultColors()
{
   Closure->redText->red        = 0xffff;
   Closure->redText->green      = 0;
   Closure->redText->blue       = 0;

   Closure->greenText->red      = 0;
   Closure->greenText->green    = 0x8000;
   Closure->greenText->blue     = 0;

   Closure->barColor->red       = 0xffff;
   Closure->barColor->green     = 0;
   Closure->barColor->blue      = 0;

   Closure->logColor->red       = 0xffff;
   Closure->logColor->green     = 0;
   Closure->logColor->blue      = 0xffff;

   Closure->curveColor->red     = 0;
   Closure->curveColor->green   = 0;
   Closure->curveColor->blue    = 0xffff;

   Closure->redSector->red      = 0xffff;
   Closure->redSector->green    = 0;
   Closure->redSector->blue     = 0;

   Closure->yellowSector->red   = 0xffff;
   Closure->yellowSector->green = 0xc000;
   Closure->yellowSector->blue  = 0;
      
   Closure->greenSector->red    = 0;
   Closure->greenSector->green  = 0xdb00;
   Closure->greenSector->blue   = 0;

   Closure->darkSector->red     = 0;
   Closure->darkSector->green   = 0x8000;
   Closure->darkSector->blue    = 0;

   Closure->blueSector->red     = 0;
   Closure->blueSector->green   = 0;
   Closure->blueSector->blue    = 0xffff;

   Closure->whiteSector->red    = 0xffff;
   Closure->whiteSector->green  = 0xffff;
   Closure->whiteSector->blue   = 0xffff;

   UpdateMarkup(&Closure->redMarkup, Closure->redText);
   UpdateMarkup(&Closure->greenMarkup, Closure->greenText);
}

static void save_colors(FILE *dotfile, char *symbol, GdkColor *color)
{  char *blanks="                    ";
   char *pad;
   int len=strlen(symbol);

   if(len>19) pad=blanks+19;
   else       pad=blanks+len;

   fprintf(dotfile, "%s:%s%02x%02x%02x\n", symbol, pad,
	   color->red>>8, color->green>>8, color->blue>>8);
}

static void get_color(GdkColor *color, char *value)
{  unsigned int hex = strtol(value, NULL, 16);
   
   color->red   = (hex>>8)&0xff00;
   color->green = hex&0xff00;
   color->blue  = (hex<<8)&0xff00;
}

/***
 *** Save and restore user settings to/from the .dvdisaster file
 ***/

#define MAX_LINE_LEN 512

void ReadDotfile()
{  FILE *dotfile;
   char line[MAX_LINE_LEN];

   dotfile = portable_fopen(Closure->dotFile, "rb");
   if(!dotfile)
      return;

   while(TRUE)
   {  int n;
      char symbol[41];
      char *value, *ignore;

      /* Get first MAX_LINE_LEN bytes of line, discard the rest */
     
      line[MAX_LINE_LEN-1] = 1;
      ignore = fgets(line, MAX_LINE_LEN, dotfile);
      if(!line[MAX_LINE_LEN-1])  /* line longer than buffer */
	while(!feof(dotfile) && fgetc(dotfile) != '\n')
	  ;

      /* Trivially reject the line */

      if(feof(dotfile)) break;
      if(*line == '#') continue;
      if(!sscanf(line, "%40[0-9a-zA-Z-]%n", symbol, &n)) continue;
      if(line[n] != ':') continue;

      /* Separate line contents into symbol: value pair */

      value = line+n+1;
      while(*value && *value == ' ')
	value++;
      if(!*value) continue;
      n = strlen(value);
      if(value[n-1] == '\n')
	value[n-1] = 0;

      /* Parse the symbols which are recognized in this version */

      if(!strcmp(symbol, "last-device"))     { if(Closure->device) g_free(Closure->device);
	                                       Closure->device      = g_strdup(value); continue; }
      if(!strcmp(symbol, "last-image"))      { g_free(Closure->imageName);
	                                       if(!strcmp(value, "none"))
						    Closure->imageName = g_strdup("");
					       else Closure->imageName = g_strdup(value); continue; 
                                             }
      if(!strcmp(symbol, "last-ecc"))        { g_free(Closure->eccName);
	                                       if(!strcmp(value, "none"))
						    Closure->eccName = g_strdup("");
                                               else Closure->eccName = g_strdup(value); continue; 
                                             }
      if(!strcmp(symbol, "browser"))         { g_free(Closure->browser);
                                               Closure->browser     = g_strdup(value); continue; }

      if(!strcmp(symbol, "adaptive-read"))   { Closure->adaptiveRead   = atoi(value); continue; }
      if(!strcmp(symbol, "auto-suffix"))     { Closure->autoSuffix  = atoi(value); continue; }
      if(!strcmp(symbol, "bd-size1"))        { Closure->bdSize1 = Closure->savedBDSize1 = atoll(value); continue; }
      if(!strcmp(symbol, "bd-size2"))        { Closure->bdSize2 = Closure->savedBDSize2 = atoll(value); continue; }
      if(!strcmp(symbol, "cache-size"))      { Closure->cacheMB     = atoi(value); continue; }
      if(!strcmp(symbol, "cd-size"))         { Closure->cdSize = Closure->savedCDSize = atoll(value); continue; }
      if(!strcmp(symbol, "codec-threads"))   { Closure->codecThreads = atoi(value); continue; }
      if(!strcmp(symbol, "dao"))             { Closure->noTruncate  = atoi(value); continue; }
      if(!strcmp(symbol, "defective-dump"))  { Closure->defectiveDump = atoi(value); continue; }
      if(!strcmp(symbol, "defective-dir"))   { if(Closure->dDumpDir) g_free(Closure->dDumpDir);
	                                       Closure->dDumpDir = g_strdup(value); continue; }
      if(!strcmp(symbol, "defective-prefix")){ if(Closure->dDumpPrefix) g_free(Closure->dDumpPrefix); 
	                                       Closure->dDumpPrefix = g_strdup(value); continue; }
      if(!strcmp(symbol, "dotfile-version")) { Closure->dotFileVersion = atoi(value); continue; }
      if(!strcmp(symbol, "dvd-size1"))       { Closure->dvdSize1 = Closure->savedDVDSize1 = atoll(value); continue; }
      if(!strcmp(symbol, "dvd-size2"))       { Closure->dvdSize2 = Closure->savedDVDSize2 = atoll(value); continue; }
      if(!strcmp(symbol, "eject"))           { Closure->eject  = atoi(value); continue; }
      if(!strcmp(symbol, "fill-unreadable")) { Closure->fillUnreadable = atoi(value); continue; }
      if(!strcmp(symbol, "ignore-fatal-sense")) { Closure->ignoreFatalSense  = atoi(value); continue; }
      if(!strcmp(symbol, "internal-attempts"))  { Closure->internalAttempts = atoi(value); continue; }
      if(!strcmp(symbol, "jump"))            { Closure->sectorSkip  = atoi(value); continue; }
      if(!strcmp(symbol, "log-file-enabled")){ Closure->logFileEnabled = atoi(value); continue; }
      if(!strcmp(symbol, "log-file"))        { if(Closure->logFile) g_free(Closure->logFile);
	                                       Closure->logFile  = g_strdup(value); continue; }
      if(!strcmp(symbol, "medium-size"))     { Closure->mediumSize  = atoll(value); continue; }
      if(!strcmp(symbol, "method-name"))     { if(Closure->methodName) g_free(Closure->methodName);
	                                       Closure->methodName = g_strdup(value); continue; }
      if(!strcmp(symbol, "max-read-attempts"))   { Closure->maxReadAttempts = atoi(value); continue; }
      if(!strcmp(symbol, "min-read-attempts"))   { Closure->minReadAttempts = atoi(value); continue; }
      if(!strcmp(symbol, "missing-sector-marker"))  { Closure->dsmVersion  = atoi(value); continue; }
      if(!strcmp(symbol, "query-size"))      { Closure->querySize  = atoi(value); continue; }
      if(!strcmp(symbol, "raw-mode"))        { Closure->rawMode = atoi(value); continue; }
      if(!strcmp(symbol, "read-and-create")) { Closure->readAndCreate = atoi(value); continue; }
      if(!strcmp(symbol, "read-medium"))     { Closure->readingPasses = atoi(value); continue; }
      if(!strcmp(symbol, "read-raw"))        { Closure->readRaw = atoi(value); continue; }
      if(!strcmp(symbol, "redundancy"))      { if(Closure->redundancy) g_free(Closure->redundancy);
                                               Closure->redundancy  = g_strdup(value); continue; }
      if(!strcmp(symbol, "reverse-cancel-ok")) { Closure->reverseCancelOK = atoi(value); continue; }
      if(!strcmp(symbol, "spinup-delay"))    { Closure->spinupDelay = atoi(value); continue; }
      if(!strcmp(symbol, "split-files"))     { Closure->splitFiles  = atoi(value); continue; }
      if(!strcmp(symbol, "unlink"))          { Closure->unlinkImage = atoi(value); continue; }
      if(!strcmp(symbol, "verbose"))         { Closure->verbose = atoi(value); continue; }
      if(!strcmp(symbol, "welcome-msg"))     { Closure->welcomeMessage = atoi(value); continue; }

      if(!strcmp(symbol, "positive-text"))   { get_color(Closure->greenText, value); 
	                                       UpdateMarkup(&Closure->greenMarkup, Closure->greenText);
	                                       continue; 
                                             }
      if(!strcmp(symbol, "negative-text"))   { get_color(Closure->redText, value);
	                                       UpdateMarkup(&Closure->redMarkup, Closure->redText); 
					       continue; 
                                             }
      if(!strcmp(symbol, "bar-color"))       { get_color(Closure->barColor, value); continue; }
      if(!strcmp(symbol, "log-color"))       { get_color(Closure->logColor, value); continue; }
      if(!strcmp(symbol, "curve-color"))     { get_color(Closure->curveColor, value); continue; }
      if(!strcmp(symbol, "defective-sector")){ get_color(Closure->redSector, value); continue; }
      if(!strcmp(symbol, "bad-checksum-sector")){ get_color(Closure->yellowSector, value); continue; }
      if(!strcmp(symbol, "good-sector"))     { get_color(Closure->greenSector, value); continue; }
      if(!strcmp(symbol, "ignored-sector"))  { get_color(Closure->blueSector, value); continue; }
      if(!strcmp(symbol, "highlit-sector"))  { get_color(Closure->whiteSector, value); continue; }
      if(!strcmp(symbol, "present-sector"))  { get_color(Closure->darkSector, value); continue; }
   }

   if(fclose(dotfile))
     g_fprintf(stderr, "Error closing configuration file %s: %s\n", 
	       Closure->dotFile, strerror(errno));
}

static void update_dotfile()
{  const char *no_dot_files;
   FILE *dotfile;

   /*** If the environment $NO_DOT_FILES is set,
        do not alter the dotfile. */

   no_dot_files = g_getenv("NO_DOT_FILES");

   if(no_dot_files && atoi(no_dot_files))
      return;

   /*** Otherwise, save our session */

   dotfile = portable_fopen(Closure->dotFile, "wb");
   if(!dotfile)
   {  g_fprintf(stderr, "Could not open configuration file %s: %s\n", 
		Closure->dotFile, strerror(errno));
      return;
   }

   g_fprintf(dotfile, 
	     _("# dvdisaster-%s configuration file\n"
	       "# This is an automatically generated file\n"
	       "# which will be overwritten each time dvdisaster is run.\n\n"),
	     VERSION);

   g_fprintf(dotfile, "last-device:       %s\n", Closure->device);
   g_fprintf(dotfile, "last-image:        %s\n", Closure->imageName);
   g_fprintf(dotfile, "last-ecc:          %s\n", Closure->eccName);
   g_fprintf(dotfile, "browser:           %s\n\n", Closure->browser);

   g_fprintf(dotfile, "adaptive-read:     %d\n", Closure->adaptiveRead);
   g_fprintf(dotfile, "auto-suffix:       %d\n", Closure->autoSuffix);
   g_fprintf(dotfile, "bd-size1:          %lld\n", (long long int)Closure->bdSize1);
   g_fprintf(dotfile, "bd-size2:          %lld\n", (long long int)Closure->bdSize2);
   g_fprintf(dotfile, "cache-size:        %d\n", Closure->cacheMB);
   g_fprintf(dotfile, "cd-size:           %lld\n", (long long int)Closure->cdSize);
   g_fprintf(dotfile, "codec-threads:     %d\n", Closure->codecThreads);
   g_fprintf(dotfile, "dao:               %d\n", Closure->noTruncate);
   g_fprintf(dotfile, "defective-dump:    %d\n", Closure->defectiveDump);
   g_fprintf(dotfile, "defective-dir:     %s\n", Closure->dDumpDir);
   g_fprintf(dotfile, "defective-prefix:  %s\n", Closure->dDumpPrefix);
   g_fprintf(dotfile, "dotfile-version:   %d\n", Closure->dotFileVersion);
   g_fprintf(dotfile, "dvd-size1:         %lld\n", (long long int)Closure->dvdSize1);
   g_fprintf(dotfile, "dvd-size2:         %lld\n", (long long int)Closure->dvdSize2);
   g_fprintf(dotfile, "eject:             %d\n", Closure->eject);
   g_fprintf(dotfile, "fill-unreadable:   %d\n", Closure->fillUnreadable);
   g_fprintf(dotfile, "ignore-fatal-sense: %d\n", Closure->ignoreFatalSense);
   g_fprintf(dotfile, "internal-attempts: %d\n", Closure->internalAttempts);
   g_fprintf(dotfile, "jump:              %d\n", Closure->sectorSkip);
   g_fprintf(dotfile, "log-file-enabled:  %d\n", Closure->logFileEnabled);
   g_fprintf(dotfile, "log-file:          %s\n", Closure->logFile);
   g_fprintf(dotfile, "medium-size:       %lld\n", (long long int)Closure->mediumSize);
   g_fprintf(dotfile, "method-name:       %s\n", Closure->methodName);
   g_fprintf(dotfile, "max-read-attempts: %d\n", Closure->maxReadAttempts);
   g_fprintf(dotfile, "min-read-attempts: %d\n", Closure->minReadAttempts);
   g_fprintf(dotfile, "missing-sector-marker: %d\n", Closure->dsmVersion);
   g_fprintf(dotfile, "query-size:        %d\n", Closure->querySize);
   g_fprintf(dotfile, "raw-mode:          %d\n", Closure->rawMode);
   g_fprintf(dotfile, "read-and-create:   %d\n", Closure->readAndCreate);
   g_fprintf(dotfile, "read-medium:       %d\n", Closure->readingPasses);
   g_fprintf(dotfile, "read-raw:          %d\n", Closure->readRaw);
   if(Closure->redundancy)
     g_fprintf(dotfile, "redundancy:        %s\n", Closure->redundancy);
   g_fprintf(dotfile, "reverse-cancel-ok: %d\n", Closure->reverseCancelOK);
   g_fprintf(dotfile, "spinup-delay:      %d\n", Closure->spinupDelay);
   g_fprintf(dotfile, "split-files:       %d\n", Closure->splitFiles);
   g_fprintf(dotfile, "unlink:            %d\n", Closure->unlinkImage);
   g_fprintf(dotfile, "verbose:           %d\n", Closure->verbose);
   g_fprintf(dotfile, "welcome-msg:       %d\n\n", Closure->welcomeMessage);

   save_colors(dotfile, "positive-text",      Closure->greenText);
   save_colors(dotfile, "negative-text",      Closure->redText);
   save_colors(dotfile, "bar-color",          Closure->barColor);
   save_colors(dotfile, "log-color",          Closure->logColor);
   save_colors(dotfile, "curve-color",        Closure->curveColor);
   save_colors(dotfile, "defective-sector",   Closure->redSector);
   save_colors(dotfile, "bad-checksum-sector",Closure->yellowSector);
   save_colors(dotfile, "good-sector",        Closure->greenSector);
   save_colors(dotfile, "ignored-sector",     Closure->blueSector);
   save_colors(dotfile, "highlit-sector",     Closure->whiteSector);
   save_colors(dotfile, "present-sector",     Closure->darkSector);

   if(fclose(dotfile))
     g_fprintf(stderr, "Error closing configuration file %s: %s\n", 
	       Closure->dotFile, strerror(errno));
}

/***
 *** Allocate and initialize our global variables
 ***/

GlobalClosure *Closure;
int exitCode = EXIT_SUCCESS;

void InitClosure()
{  int v1,v2,v3,dots=0;
   char *v,version[strlen(VERSION)+1];

   Closure = g_malloc0(sizeof(GlobalClosure));

   /* Give versions with patch levels a nicer formatting */

   if(!strcmp(RELEASE_STATUS, "patch"))
     Closure->cookedVersion = g_strdup_printf("%s (pl%s)",VERSION,RELEASE_MICRO);
   else if(!strcmp(RELEASE_STATUS, "devel"))
   { Closure->releaseFlags = MFLAG_DEVEL;
     Closure->cookedVersion = g_strdup_printf("%s (devel-%s)",VERSION,RELEASE_MICRO);
   }
   else if(!strcmp(RELEASE_STATUS, "rc"))
   { Closure->releaseFlags = MFLAG_RC;
     Closure->cookedVersion = g_strdup_printf("%s (rc-%s)",VERSION,RELEASE_MICRO);
   }
   else Closure->cookedVersion = g_strdup(VERSION);

   /* Replace the dot with a locale-resistant separator */

   strcpy(version,VERSION);
   for(v=version; *v; v++)
     if(*v=='.') 
     {  *v='x';
        dots++;
     }

   if(dots == 1) 
   {  v1 = v2 = v3 = 0;
      sscanf(version,"%dx%d",&v1,&v2);
   }
   else 
   {  g_printf("Error: malformed version number %s\n",VERSION);
      exit(EXIT_FAILURE);
   }

   v3 = atoi(RELEASE_MICRO);
   Closure->version = 10000*v1 + 100*v2 + v3;

#if 0
   printf("Version %s; %d; Flags %d\n", Closure->cookedVersion, Closure->version, Closure->releaseFlags);
#endif

   /* Get home and system directories */

   get_base_dirs();

   /* Fill in other closure defaults */

   Closure->deviceNames = g_ptr_array_new();
   Closure->deviceNodes = g_ptr_array_new();
   Closure->browser     = g_strdup("xdg-open");
   Closure->methodList  = g_ptr_array_new();
   Closure->methodName  = g_strdup("RS01");
   Closure->dDumpDir    = g_strdup(Closure->homeDir);
   Closure->cacheMB     = 32;
   Closure->codecThreads = 1;
   Closure->minReadAttempts = 1;
   Closure->maxReadAttempts = 1;
   Closure->rawMode     = 0x20;
   Closure->internalAttempts = -1;
   Closure->sectorSkip  = 16;
   Closure->spinupDelay = 5;
   Closure->querySize   = 2;
   Closure->fillUnreadable = -1;
   Closure->welcomeMessage = 1;
   Closure->useSCSIDriver = DRIVER_CDROM_DEFAULT;

   /* default sizes for typical CD and DVD media */

   Closure->cdSize   = Closure->savedCDSize   = CDR_SIZE;
   Closure->dvdSize1 = Closure->savedDVDSize1 = DVD_SL_SIZE;
   Closure->dvdSize2 = Closure->savedDVDSize2 = DVD_DL_SIZE;
   Closure->bdSize1  = Closure->savedBDSize1  = BD_SL_SIZE;
   Closure->bdSize2  = Closure->savedBDSize2  = BD_DL_SIZE;

   Closure->logString = g_string_sized_new(1024);
   Closure->logLock   = g_mutex_new();

   Closure->background = g_malloc0(sizeof(GdkColor));
   Closure->foreground = g_malloc0(sizeof(GdkColor));
   Closure->grid       = g_malloc0(sizeof(GdkColor));

   Closure->redText     = g_malloc0(sizeof(GdkColor));
   Closure->greenText   = g_malloc0(sizeof(GdkColor));
   Closure->barColor    = g_malloc0(sizeof(GdkColor));
   Closure->logColor    = g_malloc0(sizeof(GdkColor));
   Closure->curveColor  = g_malloc0(sizeof(GdkColor));
   Closure->redSector   = g_malloc0(sizeof(GdkColor));
   Closure->yellowSector= g_malloc0(sizeof(GdkColor));
   Closure->greenSector = g_malloc0(sizeof(GdkColor));
   Closure->blueSector  = g_malloc0(sizeof(GdkColor));
   Closure->whiteSector = g_malloc0(sizeof(GdkColor));
   Closure->darkSector  = g_malloc0(sizeof(GdkColor));

   DefaultColors();

   memset(Closure->bs, '\b', 255);

   DefaultLogFile();

#ifdef SYS_MINGW
   OpenAspi();
#endif
}

/*
 * Add some localized file name defaults.
 * Can't do this in InitClosure() as the locale has not been
 * initialized when it is being called. 
 */

void LocalizedFileDefaults()
{  
#ifdef SYS_DARWIN
   /* Darwin uses / as the current working directory when invoked
      from an app bundle. Not good. */
   char *buf = getcwd(NULL,0);
   
   Closure->imageName = NULL;

   if(buf && !strcmp(buf,"/"))
   {  free(buf);
      buf = getenv("HOME");
      if(buf)
      {
         Closure->imageName   = g_strdup_printf("%s/%s",buf,_("medium.iso"));
         Closure->eccName     = g_strdup_printf("%s/%s",buf,_("medium.ecc"));
         Closure->dDumpPrefix = g_strdup_printf("%s/%s",buf,_("sector-"));
      }
   }

   if(!Closure->imageName)
   {
     Closure->imageName   = g_strdup(_("medium.iso"));
     Closure->eccName     = g_strdup(_("medium.ecc"));
     Closure->dDumpPrefix = g_strdup(_("sector-"));
   }

#else
   /* On all other OS, storing the files in the cwd is a sane default. */
   Closure->imageName   = g_strdup(_("medium.iso"));
   Closure->eccName     = g_strdup(_("medium.ecc"));
   Closure->dDumpPrefix = g_strdup(_("sector-"));
#endif
}


/*
 * Clear the CRC cache
 */

void ClearCrcCache(void)
{  if(Closure->crcCache)
      g_free(Closure->crcCache);
   if(Closure->crcImageName)
      g_free(Closure->crcImageName);

   Closure->crcCache = NULL;
   Closure->crcImageName = NULL;
   memset(Closure->md5Cache, 0, 16);
}

/*
 * Clean up properly 
 */

#define cond_free(x) if(x) g_free(x)

/* Doing a simple g_ptr_array_free(a, TRUE)
   would confuse our memory leak checker */

void cond_free_ptr_array(GPtrArray *a) 
{  unsigned int i;

   if(!a) return;

   for(i=0; i<a->len; i++)
     g_free(g_ptr_array_index(a,i));

   g_ptr_array_free(a, FALSE);
}
    
void FreeClosure()
{
   if(Closure->guiMode)
     update_dotfile();

   ClearCrcCache();

   cond_free(Closure->cookedVersion);
   cond_free(Closure->device);
   cond_free_ptr_array(Closure->deviceNames);
   cond_free_ptr_array(Closure->deviceNodes);
   cond_free(Closure->imageName);
   cond_free(Closure->eccName);
   cond_free(Closure->redundancy);

   CallMethodDestructors();
   cond_free_ptr_array(Closure->methodList);

   cond_free(Closure->methodName);
   cond_free(Closure->homeDir);
   cond_free(Closure->dotFile);
   cond_free(Closure->logFile);
   cond_free(Closure->binDir);
   cond_free(Closure->docDir);
   cond_free(Closure->appData);
   cond_free(Closure->browser);
   cond_free(Closure->errorTitle);
   cond_free(Closure->dDumpDir);
   cond_free(Closure->dDumpPrefix);

   if(Closure->prefsContext)
     FreePreferences(Closure->prefsContext);

   if(Closure->rawEditorContext)
      FreeRawEditorContext(Closure->rawEditorContext);

   if(Closure->logString)
      g_string_free(Closure->logString, TRUE);

   if(Closure->logLock)
      g_mutex_free(Closure->logLock);

   if(Closure->drawGC)
     g_object_unref(Closure->drawGC);

   cond_free(Closure->background);
   cond_free(Closure->foreground);
   cond_free(Closure->grid);
   cond_free(Closure->redText);
   cond_free(Closure->greenText);
   cond_free(Closure->barColor);
   cond_free(Closure->logColor);
   cond_free(Closure->curveColor);
   cond_free(Closure->redSector);
   cond_free(Closure->yellowSector);
   cond_free(Closure->greenSector);
   cond_free(Closure->blueSector);
   cond_free(Closure->whiteSector);
   cond_free(Closure->darkSector);

   cond_free(Closure->redMarkup);
   cond_free(Closure->greenMarkup);
   cond_free(Closure->invisibleDash);

   if(Closure->readLinearCurve)
     FreeCurve(Closure->readLinearCurve);

   if(Closure->readLinearSpiral)
     FreeSpiral(Closure->readLinearSpiral);

   if(Closure->readAdaptiveSpiral)
     FreeSpiral(Closure->readAdaptiveSpiral);

   if(Closure->readAdaptiveSubtitle)
     g_free(Closure->readAdaptiveSubtitle);

   if(Closure->readAdaptiveErrorMsg)
     g_free(Closure->readAdaptiveErrorMsg);

   g_free(Closure);

#ifdef SYS_MINGW
   CloseAspi();
#endif
}
