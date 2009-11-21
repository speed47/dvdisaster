/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2004-2009 Carsten Gnoerlich.
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
 #include <winnls.h>
#endif

/*
 * Create the error correction file
 */

void CreateEcc(void)
{  Method *method = FindMethod(Closure->methodName); 

   /*** GUI mode does its own FindMethod() before calling us
	so the following Stop() will never execute in GUI mode */

   if(!method) Stop(_("\nMethod %s not available.\n"
		      "Use -m without parameters for a method list.\n"), 
	            Closure->methodName);

   method->create(method);
}

/*
 * Fix the medium with ecc information
 */

void FixEcc(void)
{  Method *method; 
   
   /* Error handling is done within EccFileMethod() */

   method = EccFileMethod(TRUE);
 
   /* Dispatch to the proper method */
  
   method->fix(method);
}

/*
 * Verfiy the image against ecc data 
 */

void Verify(void)
{  Method *method; 
   
  /* If something is wrong with the .iso or .ecc files
     we fall back to the RS01 method for comparing 
     since it is robust against missing files. */

  if(!(method = EccFileMethod(FALSE)))
    if(!(method = FindMethod("RS01")))
      Stop(_("RS01 method not available for comparing files."));
 
   /* Dispatch to the proper method */

   method->verify(method);
}

/*
 * The all-famous main() loop 
 */

typedef enum
{  MODE_NONE, 

   MODE_CREATE,
   MODE_HELP,
   MODE_FIX,
   MODE_VERIFY, 
   MODE_READ, 
   MODE_SCAN,
   MODE_SEQUENCE, 

   MODE_BYTESET, 
   MODE_COPY_SECTOR,
   MODE_CMP_IMAGES,
   MODE_DEBUG_MAINT1,
   MODE_ERASE, 
   MODE_LIST_ASPI,
   MODE_MARKED_IMAGE,
   MODE_MERGE_IMAGES,
   MODE_RANDOM_ERR, 
   MODE_RANDOM_IMAGE,
   MODE_RAW_SECTOR,
   MODE_READ_SECTOR,
   MODE_SEND_CDB,
   MODE_SHOW_SECTOR, 
   MODE_SIGN,
   MODE_TRUNCATE,
   MODE_ZERO_UNREADABLE,

   MODIFIER_ADAPTIVE_READ, 
   MODIFIER_AUTO_SUFFIX,
   MODIFIER_CACHE_SIZE, 
   MODIFIER_CLV_SPEED,    /* unused */ 
   MODIFIER_CAV_SPEED,    /* unused */
   MODIFIER_CDUMP, 
   MODIFIER_DAO, 
   MODIFIER_DEBUG,
   MODIFIER_DEFECTIVE_DUMP,
   MODIFIER_DRIVER,
   MODIFIER_EJECT,
   MODIFIER_FILL_UNREADABLE,
   MODIFIER_IGNORE_FATAL_SENSE,
   MODIFIER_INTERNAL_REREADS,
   MODIFIER_QUERY_SIZE,
   MODIFIER_NEW_DS_MARKER,
   MODIFIER_RANDOM_SEED,
   MODIFIER_READ_ATTEMPTS,
   MODIFIER_READ_MEDIUM,
   MODIFIER_READ_RAW,
   MODIFIER_RAW_MODE,
   MODIFIER_SCREEN_SHOT,
   MODIFIER_SIMULATE_DEFECTS,
   MODIFIER_SPEED_WARNING, 
   MODIFIER_SPINUP_DELAY, 
   MODIFIER_SPLIT_FILES,
   MODIFIER_TRUNCATE,
   MODIFIER_VERSION,
} run_mode;

int main(int argc, char *argv[])
{  int mode = MODE_NONE; 
   int sequence = MODE_NONE;
   int devices_queried = FALSE;
   char *debug_arg = NULL;
   char *read_range = NULL;
#ifdef WITH_NLS_YES
   char *locale_test;
 #ifdef WITH_EMBEDDED_SRC_PATH_YES
  #ifndef SYS_MINGW
   char src_locale_path[strlen(SRCDIR)+10];
  #endif
 #endif /* WITH_EMBEDDED_SRC_PATH_YES */
#if defined(SYS_MINGW) || defined(SYS_DARWIN)
   char *bin_locale_path = NULL;
 #endif
#endif

#ifdef WITH_MEMDEBUG_YES
    atexit(check_memleaks);
#endif

    /*** Do this early so that we can use mutexes */

    g_thread_init(NULL);

#ifdef SYS_MINGW
    /*** Create a named mutex so that the installer can detect
	 that we are running. A malicious user may cause this 
	 to fail by reserving the mutex for himself, 
	 so we do not care if the mutex can not be created. */

    CreateMutex(NULL, FALSE, "dvdisaster");
#endif

    /*** Setup the global closure. */

    InitClosure();

    /*** Setup for multithreading */

    Closure->mainThread = g_thread_self();

    /*** Setup the locale.
         Try the local source directory first (it may be more recent),
         then fall back to the global installation directory.
         The test phrase is supposed to be translated with "yes",
         _independent_ of the actual locale! */

#ifdef WITH_NLS_YES
#ifdef SYS_MINGW
    if(!g_getenv("LANG"))  /* Unix style setting has precedence */
    {  LANGID lang_id;

       /* Try to get locale from Windows
	  and set the respective environment variables. */

       lang_id = GetUserDefaultLangID(); 
       switch(PRIMARYLANGID(lang_id))
       {  case LANG_CZECH:
	     g_setenv("LANG", "cs_CZ", 1);
#ifdef WIN_CONSOLE
	     g_setenv("OUTPUT_CHARSET", "CP852", 1);
#else
	     g_setenv("OUTPUT_CHARSET", "CP1250", 1);
#endif
	     break;

          case LANG_GERMAN:
	     g_setenv("LANG", "de_DE", 1);
#ifdef WIN_CONSOLE
	     g_setenv("OUTPUT_CHARSET", "CP850", 1);
#else
	     g_setenv("OUTPUT_CHARSET", "CP1252", 1);
#endif
	     break;

          case LANG_ITALIAN:
	     g_setenv("LANG", "it_IT", 1);
#ifdef WIN_CONSOLE
	     g_setenv("OUTPUT_CHARSET", "CP850", 1);
#else
	     g_setenv("OUTPUT_CHARSET", "CP1252", 1);
#endif
	     break;

          case LANG_RUSSIAN:
	     g_setenv("LANG", "ru_RU", 1);
#ifdef WIN_CONSOLE
	     g_setenv("OUTPUT_CHARSET", "CP855", 1);
#else
	     g_setenv("OUTPUT_CHARSET", "CP1251", 1);
#endif
	     break;

          case LANG_SWEDISH:
	     g_setenv("LANG", "sv_SV", 1);
#ifdef WIN_CONSOLE
	     g_setenv("OUTPUT_CHARSET", "CP850", 1);
#else
	     g_setenv("OUTPUT_CHARSET", "CP1252", 1);
#endif
	     break;
       }
    }
#endif /* SYS_MINGW */

    setlocale(LC_CTYPE, "");
    setlocale(LC_MESSAGES, "");
    textdomain("dvdisaster");

#ifndef SYS_MINGW
    /* Try local source directory first */

#ifdef WITH_EMBEDDED_SRC_PATH_YES
    g_sprintf(src_locale_path,"%s/locale",SRCDIR);
    bindtextdomain("dvdisaster", src_locale_path);
#endif /* WITH_EMBEDDED_SRC_PATH_YES */

    /* TRANSLATORS: 
       This is a dummy entry which is supposed to translate into "ok".
       Please do not return anything else here. */
    locale_test = gettext("test phrase for verifying the locale installation");

    /* Try current directory. Might work if we are starting
       from the root dir of a binary distribution. */

    if(strcmp(locale_test, "ok"))
    {  char buf[256];

       if(getcwd(buf, 256))
       {  char locale_path[strlen(buf)+20];
          g_sprintf(locale_path,"%s/locale", buf);
          bindtextdomain("dvdisaster", locale_path);
	  locale_test = gettext("test phrase for verifying the locale installation");
       }
    }
#endif

#if defined(SYS_MINGW) || defined(SYS_DARWIN)
    /* Try the directory where our executable comes from.
       This is only possible under Windows and Mac OS, 
       and should cover all cases. */

#ifdef SYS_MINGW
    bin_locale_path = g_strdup_printf("%s\\locale", Closure->binDir);
#else
    bin_locale_path = g_strdup_printf("%s/locale", Closure->binDir);
#endif
    bindtextdomain("dvdisaster", bin_locale_path);
    locale_test = gettext("test phrase for verifying the locale installation");
    g_free(bin_locale_path);
#endif

    /* Last resort: fall back to global locale */

    if(strcmp(locale_test, "ok"))
      bindtextdomain("dvdisaster", LOCALEDIR);  
#endif /* WITH_NLS_YES */

   /*** Create some localized file name presets */

   LocalizedFileDefaults();   

   /*** Collect the Method list */

   CollectMethods();

   /*** The following test may go wrong if the compiler chooses the
	wrong packing. */

   if(sizeof(EccHeader) != 4096)
     Stop("sizeof(EccHeader) is %d, but must be 4096.\n", sizeof(EccHeader));

   /*** Parse the options */
   
   for(;;)
   {  int option_index,c;
      static struct option long_options[] =
      { {"adaptive-read", 0, 0, MODIFIER_ADAPTIVE_READ},
	{"auto-suffix", 0, 0,  MODIFIER_AUTO_SUFFIX},
	{"byteset", 1, 0, MODE_BYTESET },
	{"copy-sector", 1, 0, MODE_COPY_SECTOR },
	{"compare-images", 1, 0, MODE_CMP_IMAGES },
	{"cache-size", 1, 0, MODIFIER_CACHE_SIZE },
	{"cav", 1, 0, MODIFIER_CAV_SPEED },
	{"cdump", 0, 0, MODIFIER_CDUMP },
	{"clv", 1, 0, MODIFIER_CLV_SPEED },
	{"create", 0, 0, 'c'},
	{"dao", 0, 0, MODIFIER_DAO },
	{"debug", 0, 0, MODIFIER_DEBUG },
	{"debug1", 1, 0, MODE_DEBUG_MAINT1 },
	{"defective-dump", 1, 0, MODIFIER_DEFECTIVE_DUMP },
	{"device", 0, 0, 'd'},
	{"driver", 1, 0, MODIFIER_DRIVER },
        {"ecc", 1, 0, 'e'},
	{"eject", 0, 0, MODIFIER_EJECT },
	{"erase", 1, 0, MODE_ERASE },
	{"fill-unreadable", 1, 0, MODIFIER_FILL_UNREADABLE },
	{"fix", 0, 0, 'f'},
	{"help", 0, 0, 'h'},
	{"ignore-fatal-sense", 0, 0, MODIFIER_IGNORE_FATAL_SENSE },
	{"internal-rereads", 1, 0, MODIFIER_INTERNAL_REREADS },
        {"image", 1, 0, 'i'},
	{"jump", 1, 0, 'j'},
#ifdef SYS_MINGW
	{"list", 0, 0, 'l' },
#endif
	{"marked-image", 1, 0, MODE_MARKED_IMAGE },
	{"merge-images", 1, 0, MODE_MERGE_IMAGES },
	{"method", 2, 0, 'm' },
	{"new-ds-marker", 0, 0, MODIFIER_NEW_DS_MARKER },
        {"prefix", 1, 0, 'p'},
	{"query-size", 1, 0, MODIFIER_QUERY_SIZE },
	{"random-errors", 1, 0, MODE_RANDOM_ERR },
	{"random-image", 1, 0, MODE_RANDOM_IMAGE },
	{"random-seed", 1, 0, MODIFIER_RANDOM_SEED },
	{"raw-mode", 1, 0, MODIFIER_RAW_MODE },
	{"raw-sector", 1, 0, MODE_RAW_SECTOR},
	{"read", 2, 0,'r'},
	{"read-attempts", 1, 0, MODIFIER_READ_ATTEMPTS },
	{"read-medium", 1, 0, MODIFIER_READ_MEDIUM },
	{"read-sector", 1, 0, MODE_READ_SECTOR},
	{"read-raw", 0, 0, MODIFIER_READ_RAW},
	{"redundancy", 1, 0, 'n'},
	{"scan", 2, 0,'s'},
	{"screen-shot", 0, 0, MODIFIER_SCREEN_SHOT },
	{"send-cdb", 1, 0, MODE_SEND_CDB},
	{"show-sector", 1, 0, MODE_SHOW_SECTOR},
	{"sign", 0, 0, MODE_SIGN},
	{"sim-defects", 1, 0, MODIFIER_SIMULATE_DEFECTS},
	{"speed-warning", 2, 0, MODIFIER_SPEED_WARNING},
	{"spinup-delay", 1, 0, MODIFIER_SPINUP_DELAY},
	{"split-files", 0, 0, MODIFIER_SPLIT_FILES},
	{"test", 0, 0, 't'},
        {"threads", 1, 0, 'x'},
	{"truncate", 2, 0, MODIFIER_TRUNCATE},
	{"unlink", 0, 0, 'u'},
       	{"verbose", 0, 0, 'v'},
	{"version", 0, 0, MODIFIER_VERSION},
	{"zero-unreadable", 0, 0, MODE_ZERO_UNREADABLE},
        {0, 0, 0, 0}
      };

      c = getopt_long(argc, argv, 
		      "cd:e:fhi:j:lm::n:p:r::s::tuvx:",
		      long_options, &option_index);

      if(c == -1) break;

      switch(c)
      {  case 'r': mode = MODE_SEQUENCE; sequence |= 1<<MODE_READ; 
	           if(optarg) read_range = g_strdup(optarg);
		   break;

         case 's': mode = MODE_SEQUENCE; sequence |= 1<<MODE_SCAN; 
	           if(optarg) read_range = g_strdup(optarg); 
		   break;
         case 'c': mode = MODE_SEQUENCE; sequence |= 1<<MODE_CREATE; break;
         case 'f': mode = MODE_SEQUENCE; sequence |= 1<<MODE_FIX; break;
         case 't': mode = MODE_SEQUENCE; sequence |= 1<<MODE_VERIFY; break;
         case 'u': Closure->unlinkImage = TRUE; break;
         case 'h': mode = MODE_HELP; break;
         case 'i': if(optarg) 
	           {  g_free(Closure->imageName);
		      Closure->imageName = g_strdup(optarg); 
		   }
	           break;
         case 'j': if(optarg) Closure->sectorSkip = atoi(optarg) & ~0xf;
	           if(Closure->sectorSkip<0) Closure->sectorSkip = 0;
		   break;
	 case 'l': mode = MODE_LIST_ASPI; break;
         case 'm': if(optarg && strlen(optarg) == 4) 
	           {  g_free(Closure->methodName);
	              Closure->methodName = g_strdup(optarg); 
                   }
	           else {  ListMethods(); FreeClosure(); exit(EXIT_SUCCESS); }
	           break;
         case 'n': if(optarg) 
		   {  Closure->redundancy = g_strdup(optarg); 
		      if(!strcmp(optarg, "CD") || !strcmp(optarg, "cd"))
			   Closure->mediumSize = CDR_SIZE;
		      else if(!strcmp(optarg, "DVD") || !strcmp(optarg, "dvd"))
			   Closure->mediumSize = DVD_SL_SIZE;
		      else if(!strcmp(optarg, "DVD9") || !strcmp(optarg, "dvd9"))
			   Closure->mediumSize = DVD_DL_SIZE;
		      else Closure->mediumSize = (gint64)atoll(optarg);
		      break;
		   }
         case 'e': if(optarg) 
	           {  g_free(Closure->eccName);
		      Closure->eccName = g_strdup(optarg);
		   }
	           break;
         case 'p': if(optarg) 
		   {  g_free(Closure->imageName);
		      g_free(Closure->eccName);
		      Closure->eccName = g_malloc(strlen(optarg)+5);
		      Closure->imageName = g_malloc(strlen(optarg)+5);
		      g_sprintf(Closure->eccName,"%s.ecc",optarg);
		      g_sprintf(Closure->imageName,"%s.iso",optarg);
		   }
	           break;
         case 'd': if(optarg) 
	           {  g_free(Closure->device);
	              Closure->device = g_strdup(optarg); 
	              break;
                   }
         case 'v': Closure->verbose = TRUE;
	           break;

         case 'x': Closure->codecThreads = atoi(optarg);
                   if(Closure->codecThreads < 1 || Closure->codecThreads > MAX_CODEC_THREADS)
                     Stop(_("--threads must be 1..%d\n"), MAX_CODEC_THREADS);
                   break;

         case  0 : break; /* flag argument */

         case MODIFIER_ADAPTIVE_READ:
	   Closure->adaptiveRead = TRUE;
	   break;
         case MODIFIER_AUTO_SUFFIX:
	   Closure->autoSuffix = TRUE;
	   break;
         case MODIFIER_CACHE_SIZE:
	   Closure->cacheMB = atoi(optarg);
	   if(Closure->cacheMB <   8) 
	     Stop(_("--cache-size must at least be 8MB; 16MB or higher is recommended.")); 
	   if(Closure->cacheMB > 8192) 
	     Stop(_("--cache-size maximum is 8192MB.")); 
	   break;
         case MODIFIER_CDUMP:
	   Closure->debugCDump = TRUE;
	   break;
         case MODIFIER_DAO: 
	   Closure->noTruncate = 1; 
	   break;
         case MODIFIER_EJECT: 
	   Closure->eject = 1; 
	   break;
	 case MODIFIER_DRIVER: /* currently undocumented feature */
#if defined(SYS_LINUX)
	   if(optarg && !strcmp(optarg,"sg"))
	      Closure->useSGioctl = TRUE;
	   else
	      Stop(_("Valid args for --driver: sg"));
#else
	   Stop(_("--driver is only supported on GNU/Linux"));
#endif
	   break;
         case MODIFIER_FILL_UNREADABLE:
	   if(optarg) Closure->fillUnreadable = strtol(optarg, NULL, 0);
	   break;
         case MODIFIER_IGNORE_FATAL_SENSE:
	   Closure->ignoreFatalSense = TRUE;
	   break;
	 case MODIFIER_INTERNAL_REREADS:
	    if(optarg)
	       Closure->internalAttempts = atoi(optarg);
	    if(Closure->internalAttempts < 0) 
	       Closure->internalAttempts = -1;
	    if(Closure->internalAttempts > 10) 
	       Closure->internalAttempts = 10;
	    break;
         case MODIFIER_TRUNCATE: 
	   if(optarg)                  /* debugging truncate mode */
	   {  mode = MODE_TRUNCATE;
	      debug_arg = g_strdup(optarg);
	   }
	   else Closure->truncate = 1; /* truncate confirmation for fix mode */
	   break;
         case MODIFIER_DEBUG:
	   Closure->debugMode = TRUE;
	   break;
         case MODIFIER_DEFECTIVE_DUMP:
	 {  char *c;
	    Closure->defectiveDump = TRUE;
	    g_free(Closure->dDumpDir);
	    Closure->dDumpDir = g_strdup(optarg);
	    c = strrchr(Closure->dDumpDir, '/');
	    if(c)
	    {  *c = 0;
	       g_free(Closure->dDumpPrefix);
	       Closure->dDumpPrefix = g_strdup(c+1);
	    }
	 }
	   break;
	 case MODIFIER_NEW_DS_MARKER:
	    Closure->dsmVersion = 1;
	    break;
         case MODIFIER_QUERY_SIZE:
	        if(!strcmp(optarg, "drive")) Closure->querySize = 0;
	   else if(!strcmp(optarg, "udf"))   Closure->querySize = 1;
	   else if(!strcmp(optarg, "ecc"))   Closure->querySize = 2;
           else Stop("--query-size requires one of these arguments: drive udf ecc\n");
	   break;
         case MODIFIER_RANDOM_SEED:
	   if(optarg) Closure->randomSeed = atoi(optarg);
	   break;
         case MODIFIER_RAW_MODE:
	    if(optarg) Closure->rawMode = strtol(optarg,NULL,16);
	   break;
         case MODIFIER_READ_ATTEMPTS:
	   if(optarg) 
	   {  char copy[strlen(optarg)+1];
	      char *cpos;

	      strcpy(copy, optarg);
	      cpos = strchr(copy,'-');

	      if(!cpos)
	      {  Closure->minReadAttempts = Closure->maxReadAttempts = atoi(optarg); 
	      }
	      else
	      {  *cpos = 0;
		 Closure->minReadAttempts = atoi(copy);
		 Closure->maxReadAttempts = atoi(cpos+1);
	      }
	      
	      if(Closure->minReadAttempts < 1)
		Closure->minReadAttempts = 1;
	      if(Closure->maxReadAttempts < Closure->minReadAttempts)
		Closure->maxReadAttempts = Closure->minReadAttempts;
	   }
	   break;
         case MODIFIER_READ_MEDIUM:
	   Closure->readingPasses = atoi(optarg);
	   break;
         case MODIFIER_READ_RAW:
	   Closure->readRaw = TRUE;
	   break;
         case MODIFIER_SCREEN_SHOT:
	   Closure->screenShotMode = TRUE;
	   break;
         case MODIFIER_SIMULATE_DEFECTS:
	   if(optarg) Closure->simulateDefects = atoi(optarg);
	   else Closure->simulateDefects = 10;
	   break;
         case MODIFIER_SPINUP_DELAY:
	   if(optarg) Closure->spinupDelay = atoi(optarg);
	   break;
         case MODIFIER_SPEED_WARNING:
	   if(optarg) Closure->speedWarning = atoi(optarg);
	   else Closure->speedWarning=10;
	   break;
         case MODIFIER_SPLIT_FILES:
	   Closure->splitFiles = 1;
	   break;
         case MODIFIER_CLV_SPEED:
	   Closure->driveSpeed = atoi(optarg);
	   break;
         case MODIFIER_CAV_SPEED:
	   Closure->driveSpeed = -atoi(optarg);
	   break;
         case MODIFIER_VERSION:
	   PrintCLI(_("\ndvdisaster version %s build %d\n\n"), 
		    Closure->cookedVersion, buildCount);
	   FreeClosure();
	   exit(EXIT_SUCCESS); 
	   break;
         case MODE_BYTESET:
	   mode = MODE_BYTESET;
	   debug_arg = g_strdup(optarg);
	   break;
	 case MODE_CMP_IMAGES:
	   mode = MODE_CMP_IMAGES;
	   debug_arg = g_strdup(optarg);
	   break;
	 case MODE_COPY_SECTOR:
	   mode = MODE_COPY_SECTOR;
	   debug_arg = g_strdup(optarg);
	   break;
         case MODE_DEBUG_MAINT1:
	   mode = MODE_DEBUG_MAINT1;
	   debug_arg = g_strdup(optarg);
	   break;
         case MODE_ERASE: 
	   mode = MODE_ERASE;
	   debug_arg = g_strdup(optarg);
	   break;
         case MODE_MARKED_IMAGE:
	   mode = MODE_MARKED_IMAGE;
	   debug_arg = g_strdup(optarg);
	   break;
         case MODE_MERGE_IMAGES:
	   mode = MODE_MERGE_IMAGES;
	   debug_arg = g_strdup(optarg);
	   break;
         case MODE_RANDOM_ERR:
	   mode = MODE_RANDOM_ERR;
	   debug_arg = g_strdup(optarg);
	   break;
         case MODE_RANDOM_IMAGE:
	   mode = MODE_RANDOM_IMAGE;
	   debug_arg = g_strdup(optarg);
	   break;
         case MODE_RAW_SECTOR:
	   mode = MODE_RAW_SECTOR;
	   debug_arg = g_strdup(optarg);
	   break;
         case MODE_READ_SECTOR:
	   mode = MODE_READ_SECTOR;
	   debug_arg = g_strdup(optarg);
	   break;
         case MODE_SEND_CDB: 
	   mode = MODE_SEND_CDB;
	   debug_arg = g_strdup(optarg);
	   break;
         case MODE_SIGN:
	   mode = MODE_SIGN;
	   break;
         case MODE_SHOW_SECTOR:
	   mode = MODE_SHOW_SECTOR;
	   debug_arg = g_strdup(optarg);
	   break;
         case MODE_ZERO_UNREADABLE:
	   mode = MODE_ZERO_UNREADABLE;
	   break;
         case '?': mode = MODE_HELP; break;
         default: PrintCLI(_("?? illegal getopt return value %d\n"),c); break;
      }
   }

   /*** Don't allow debugging option if --debug wasn't given */
   
   if(!Closure->debugMode)
     switch(mode)
     {  case MODE_BYTESET:
	case MODE_COPY_SECTOR:
	case MODE_CMP_IMAGES:
        case MODE_ERASE:
        case MODE_RANDOM_ERR:
        case MODE_RANDOM_IMAGE:
        case MODE_READ_SECTOR:
        case MODE_RAW_SECTOR:
        case MODE_SEND_CDB:
        case MODE_SHOW_SECTOR:
        case MODE_MARKED_IMAGE:
        case MODE_MERGE_IMAGES:
        case MODE_SIGN:
        case MODE_TRUNCATE:
        case MODE_ZERO_UNREADABLE:
	  mode = MODE_HELP;
          break;
     }
	  
#ifdef WIN_CONSOLE
   if(mode != MODE_SIGN && !VerifySignature())
   {  char version[80];

      if(Closure->version % 100)
           sprintf(version, "dvdisaster-%s.%d-setup.exe", VERSION, Closure->version%100);
      else sprintf(version, "dvdisaster-%s-setup.exe", VERSION);
      Stop(_("dvdisaster is not properly installed.\n"
	     "Please execute the installer program (%s) again.\n"), version);
   }
#endif

   /*** CPU type detection. */

   Closure->useSSE2 = ProbeSSE2();

   /*** Parse the sector ranges for --read and --scan */

   if(read_range)
   {  char *dashpos = strchr(read_range,'-');

      if(dashpos)
      {  *dashpos = 0;

         Closure->readStart = atoi(read_range);
	 if(strlen(dashpos+1) == 3 && !strncmp(dashpos+1, "end", 3))
	   Closure->readEnd = -1;
	 else Closure->readEnd = atoi(dashpos+1);
      }
      else Closure->readStart = Closure->readEnd = atoi(read_range);
      g_free(read_range);
   }

   /*** Apply auto suffixes 
        (--read etc. may be processed before --auto-suffix,
         so we must defer autosuffixing until all args have been read) */

   if(Closure->autoSuffix)
   {  Closure->eccName   = ApplyAutoSuffix(Closure->eccName, "ecc");
      Closure->imageName = ApplyAutoSuffix(Closure->imageName, "iso");
   }

   /*** Determine the default device (OS dependent!) if none
        has been specified on the command line. */

   if(!Closure->device)
   {  Closure->device = DefaultDevice();
      devices_queried = TRUE;
   }

   /*** Dispatch action depending on mode.
        The major modes can be executed in sequence, 
	but not all combinations may be really useful.

	The GUI version for Windows does not have an open console,
	so no command line actions can be carried out. 
	Force opening the GUI by clearing the mode variable. */

#if defined(SYS_MINGW) && !defined(WIN_CONSOLE)
   mode = MODE_NONE;
#endif

   switch(mode)
   {  case MODE_SEQUENCE:
	if(sequence & 1<<MODE_SCAN)
	  ReadMediumLinear((gpointer)1);

	if(sequence & 1<<MODE_READ)
	{  if(sequence & 1<<MODE_CREATE) 
	      Closure->readAndCreate = TRUE;
	   if(Closure->adaptiveRead) 
	        ReadMediumAdaptive((gpointer)0);
	   else ReadMediumLinear((gpointer)0);
	}

	if(sequence & 1<<MODE_CREATE)
	  CreateEcc();

	if(sequence & 1<<MODE_FIX)
	  FixEcc();

	if(sequence & 1<<MODE_VERIFY)
	  Verify();
	break;

      case MODE_BYTESET:
         Byteset(debug_arg);
	 break;

      case MODE_CMP_IMAGES:
	 MergeImages(debug_arg, FALSE);
	 break;

      case MODE_COPY_SECTOR:
	 CopySector(debug_arg);
	 break;

      case MODE_DEBUG_MAINT1:
 	 Maintenance1(debug_arg);
	 break;

      case MODE_ERASE:
         Erase(debug_arg);
	 break;

      case MODE_SEND_CDB:
	 SendCDB(debug_arg);
	 break;

      case MODE_RAW_SECTOR:
	 RawSector(debug_arg);
	 break;

      case MODE_READ_SECTOR:
	 ReadSector(debug_arg);
	 break;

      case MODE_SHOW_SECTOR:
	 ShowSector(debug_arg);
	 break;

      case MODE_RANDOM_ERR:
	 RandomError(Closure->imageName, debug_arg);
	 break;

      case MODE_MARKED_IMAGE:
	 RandomImage(Closure->imageName, debug_arg, 1);
	 break;

      case MODE_MERGE_IMAGES:
	 MergeImages(debug_arg, TRUE);
	 break;

      case MODE_RANDOM_IMAGE:
	RandomImage(Closure->imageName, debug_arg, 0);
	 break;

      case MODE_TRUNCATE:
	 TruncateImage(debug_arg);
	 break;

      case MODE_ZERO_UNREADABLE:
	 ZeroUnreadable();
	 break;

#ifdef SYS_MINGW
      case MODE_LIST_ASPI:
	 ListAspiDrives();
	 break;

      case MODE_SIGN:
	 WriteSignature();
	 exit(0);
	 break;
#endif
      default:
	break;
   }

   if(debug_arg) g_free(debug_arg);

   /*** If no mode was selected, print the help screen. */

#ifdef WIN_CONSOLE
   if(mode == MODE_HELP || mode == MODE_NONE)
#else
   if(mode == MODE_HELP)
#endif
   {  
     /* TRANSLATORS: Program options like -r and --read are not to be translated
	to avoid confusion when discussing the program in international forums. */
      PrintCLI(_("\nCommon usage examples:\n"
	     "  dvdisaster -r,--read   # Read the medium image to hard disc.\n"
	     "                         # Use -rn-m to read a certain sector range, e.g. -r100-200\n"
	     "  dvdisaster -c,--create # Create .ecc information for the medium image.\n"
	     "  dvdisaster -f,--fix    # Try to fix medium image using .ecc information.\n"
	     "  dvdisaster -s,--scan   # Scan the medium for read errors.\n"
	     "  dvdisaster -t,--test   # Test integrity of the .iso and .ecc files.\n"
	     "  dvdisaster -u,--unlink # Delete .iso files (when other actions complete)\n\n"));

      PrintCLI(_("Drive and file specification:\n"
	     "  -d,--device device     - read from given device   (default: %s)\n"
	     "  -p,--prefix prefix     - prefix of .iso/.ecc file (default: medium.*  )\n"
	     "  -i,--image  imagefile  - name of image file       (default: medium.iso)\n"
	     "  -e,--ecc    eccfile    - name of parity file      (default: medium.ecc)\n"),
	     Closure->device);

#ifdef SYS_MINGW
      PrintCLI(_("  -l,--list              - list drives available under ASPI manager\n\n"));
#else
      PrintCLI("\n");
#endif

      PrintCLI(_("Tweaking options (see manual before using!)\n"
	     "  -j,--jump n            - jump n sectors forward after a read error (default: 16)\n"
	     "  -m n                   - list/select error correction methods (default: RS01)\n" 
	     "  -n,--redundancy n%%     - error correction file redundancy (in percent), or\n"
	     "                           maximum error correction image size (in sectors)\n"
	     "  -v,--verbose           - more diagnostic messages\n"
//             "  -x, --threads n        - use n threads for en-/decoding (if supported by codec)\n"
	     "  --adaptive-read        - use optimized strategy for reading damaged media\n"
	     "  --auto-suffix          - automatically add .iso and .ecc file suffixes\n"
	     "  --cache-size n         - image cache size in MB during -c mode (default: 32MB)\n"
	     "  --dao                  - assume DAO disc; do not trim image end\n"
	     "  --defective-dump d     - directory for saving incomplete raw sectors\n"
	     "  --eject                - eject medium after successful read\n"
	     "  --fill-unreadable n    - fill unreadable sectors with byte n\n"
	     "  --ignore-fatal-sense   - continue reading after potentially fatal error conditon\n"
	     "  --internal-rereads n   - drive may attempt n rereads before reporting an error\n"
      	     "  --query-size n         - query drive/udf/ecc for image size (default: ecc)\n"   
	     "  --raw-mode n           - mode for raw reading CD media (20 or 21)\n"
	     "  --read-attempts n-m    - attempts n upto m reads of a defective sector\n"
	     "  --read-medium n        - read the whole medium up to n times\n"
	     "  --read-raw             - performs read in raw mode if possible\n"
	     "  --speed-warning n      - print warning if speed changes by more than n percent\n"
	     "  --spinup-delay n       - wait n seconds for drive to spin up\n"
	     "  --split-files          - split image into files <= 2GB\n\n"));

      if(Closure->debugMode)
      { PrintCLI(_("Debugging options (purposefully undocumented and possibly harmful)\n"
	     "  --debug           - enables the following options\n"
	     "  --byteset s,i,b   - set byte i in sector s to b\n"
             "  --cdump           - creates C #include file dumps instead of hexdumps\n" 
	     "  --compare-images a,b  - compare sectors in images a and b\n"
	     "  --copy-sector a,n,b,m - copy sector n from image a to sector m in image b\n"
	     "  --erase sector    - erase the given sector\n"
	     "  --erase n-m       - erase sectors n - m, inclusively\n"
	     "  --marked-image n  - create image with n marked random sectors\n"
	     "  --merge-images a,b  merge image a with b (a receives sectors from b)\n"
	     "  --random-errors r,e seed image with (correctable) random errors\n"
	     "  --random-image n  - create image with n sectors of random numbers\n"
	     "  --random-seed n   - random seed for built-in random number generator\n"
	     "  --raw-sector n    - shows hexdump of the given raw sector from medium in drive\n"
	     "  --read-sector n   - shows hexdump of the given sector from medium in drive\n"
	     "  --screen-shot     - useful for generating screen shots\n"
	     "  --send-cdb arg    - executes given cdb at drive; kills system if used wrong\n"
	     "  --show-sector n   - shows hexdump of the given sector in an image file\n"
	     "  --sim-defects n   - simulate n%% defective sectors on medium\n"
	     "  --truncate n      - truncates image to n sectors\n"
	     "  --zero-unreadable - replace the \"unreadable sector\" markers with zeros\n\n"));
      }

#ifdef WIN_CONSOLE
      PrintCLI(_("NOTE: This is the Windows console version of dvdisaster.\n"
		 "The version providing a graphical user interface is called\n"
		 "dvdisaster-win.exe (also contained in this installation).\n\n"));
#endif
      FreeClosure();
      exit(EXIT_FAILURE);
   }

   /* If no mode was selected at the command line, 
      start the graphical user interface. 
      Unless we are in Windows console mode where starting the GUI
      won't work. */

   if(mode == MODE_NONE)
   {  
      if(Closure->screenShotMode)
      {  GPtrArray *a=Closure->deviceNames;
	 int i;
	 for(i=0; i<a->len; i++)
	 {  char *p = g_ptr_array_index(a,i);
	    g_free(p);
	    a->pdata[i] = g_strdup(_("Optical drive 52X FW 1.02"));
	 }
      }


      /* We need to query devices in order to build
	 the drop-down menu.*/

      if(!devices_queried)
      {  if(Closure->device)
	    g_free(Closure->device);
	 Closure->device = DefaultDevice();
      }

      Closure->guiMode = TRUE;
      ReadDotfile();
      CreateMainWindow(&argc, &argv);
   }

   FreeClosure();

   exit(exitCode);
}
