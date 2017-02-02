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
   MODE_MARKED_IMAGE,
   MODE_MEDIUM_INFO,
   MODE_MERGE_IMAGES,
   MODE_RANDOM_ERR, 
   MODE_RANDOM_IMAGE,
   MODE_RAW_SECTOR,
   MODE_READ_SECTOR,
   MODE_SEND_CDB,
   MODE_SHOW_HEADER,
   MODE_SHOW_SECTOR, 
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
   MODIFIER_ENCODING_ALGORITHM,
   MODIFIER_ENCODING_IO_STRATEGY,
   MODIFIER_FILL_UNREADABLE,
   MODIFIER_FIXED_SPEED_VALUES,
   MODIFIER_IGNORE_FATAL_SENSE,
   MODIFIER_IGNORE_ISO_SIZE,
   MODIFIER_INTERNAL_REREADS,
   MODIFIER_OLD_DS_MARKER,
   MODIFIER_PREFETCH_SECTORS,
   MODIFIER_RANDOM_SEED,
   MODIFIER_RAW_MODE,
   MODIFIER_READ_ATTEMPTS,
   MODIFIER_READ_MEDIUM,
   MODIFIER_READ_RAW,
   MODIFIER_RESOURCE_FILE,
   MODIFIER_SCREEN_SHOT,
   MODIFIER_SET_VERSION,
   MODIFIER_SIMULATE_CD,
   MODIFIER_SIMULATE_DEFECTS,
   MODIFIER_SPEED_WARNING, 
   MODIFIER_SPINUP_DELAY, 
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
   char src_locale_path[strlen(SRCDIR)+10];
 #endif /* WITH_EMBEDDED_SRC_PATH_YES */
#endif
   int debug_mode_required=FALSE;

#ifdef WITH_MEMDEBUG_YES
    atexit(check_memleaks);
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

    /* This is necessary, but feels broken */
    setlocale(LC_CTYPE, "");
    setlocale(LC_MESSAGES, "");
    textdomain("dvdisaster");

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

   /*** If we have too much command line options fail here */

   if(MODIFIER_VERSION >= 'a')
     Stop("Too many command line options\n");

   /*** CPU type detection. Must be done before parsing the options
        as some may be CPU-related. */

   Closure->useSSE2 = ProbeSSE2();
   Closure->useAltiVec = ProbeAltiVec();
   Closure->clSize = ProbeCacheLineSize();

   /*** Parse the options */
   
   for(;;)
   {  int option_index,c;
      static struct option long_options[] =
      { {"adaptive-read", 0, 0, MODIFIER_ADAPTIVE_READ},
	{"auto-suffix", 0, 0,  MODIFIER_AUTO_SUFFIX},
	{"assume", 1, 0, 'a'},
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
	{"ecc-target", 1, 0, 'o'},
	{"eject", 0, 0, MODIFIER_EJECT },
	{"encoding-algorithm", 1, 0, MODIFIER_ENCODING_ALGORITHM },
	{"encoding-io-strategy", 1, 0, MODIFIER_ENCODING_IO_STRATEGY },
	{"erase", 1, 0, MODE_ERASE },
	{"fill-unreadable", 1, 0, MODIFIER_FILL_UNREADABLE },
	{"fix", 0, 0, 'f'},
	{"fixed-speed-values", 0, 0, MODIFIER_FIXED_SPEED_VALUES },
	{"help", 0, 0, 'h'},
	{"ignore-fatal-sense", 0, 0, MODIFIER_IGNORE_FATAL_SENSE },
	{"ignore-iso-size", 0, 0, MODIFIER_IGNORE_ISO_SIZE },
	{"internal-rereads", 1, 0, MODIFIER_INTERNAL_REREADS },
        {"image", 1, 0, 'i'},
	{"jump", 1, 0, 'j'},
	{"marked-image", 1, 0, MODE_MARKED_IMAGE },
	{"medium-info", 0, 0, MODE_MEDIUM_INFO },
	{"merge-images", 1, 0, MODE_MERGE_IMAGES },
	{"method", 2, 0, 'm' },
	{"old-ds-marker", 0, 0, MODIFIER_OLD_DS_MARKER },
	{"prefetch-sectors", 1, 0, MODIFIER_PREFETCH_SECTORS },
        {"prefix", 1, 0, 'p'},
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
	{"resource-file", 1, 0, MODIFIER_RESOURCE_FILE},
	{"scan", 2, 0,'s'},
	{"screen-shot", 0, 0, MODIFIER_SCREEN_SHOT },
	{"set-version", 1, 0, MODIFIER_SET_VERSION},
	{"send-cdb", 1, 0, MODE_SEND_CDB},
	{"show-header", 1, 0, MODE_SHOW_HEADER},
	{"show-sector", 1, 0, MODE_SHOW_SECTOR},
	{"sim-cd", 2, 0, MODIFIER_SIMULATE_CD},
	{"sim-defects", 1, 0, MODIFIER_SIMULATE_DEFECTS},
	{"speed-warning", 2, 0, MODIFIER_SPEED_WARNING},
	{"spinup-delay", 1, 0, MODIFIER_SPINUP_DELAY},
	{"test", 2, 0, 't'},
        {"threads", 1, 0, 'x'},
	{"truncate", 2, 0, MODIFIER_TRUNCATE},
	{"unlink", 0, 0, 'u'},
       	{"verbose", 0, 0, 'v'},
	{"version", 0, 0, MODIFIER_VERSION},
	{"zero-unreadable", 0, 0, MODE_ZERO_UNREADABLE},
        {0, 0, 0, 0}
      };

      c = getopt_long(argc, argv, 
		      "a:cd:e:fhi:j:lm::n:o:p:r::s::t::uvx:",
		      long_options, &option_index);

      if(c == -1) break;

      switch(c)
      {            
	 case 'a': if(strstr(optarg, "rs02") || strstr(optarg, "RS02"))
		      Closure->examineRS02 = TRUE; 
	           if(strstr(optarg, "rs03") || strstr(optarg, "RS03"))
		      Closure->examineRS03 = TRUE; 
		   break;	       
         case 'c': mode = MODE_SEQUENCE; sequence |= 1<<MODE_CREATE; break;
         case 'd': if(optarg) 
		   {  if(Closure->device)
			 g_free(Closure->device);
	              Closure->device = g_strdup(optarg); 
	              break;
                   }
         case 'e': if(optarg) 
		   {  if(Closure->eccName)
		         g_free(Closure->eccName);
		      Closure->eccName = g_strdup(optarg);
		   }
	           break;
         case 'f': mode = MODE_SEQUENCE; sequence |= 1<<MODE_FIX; break;
         case 'h': mode = MODE_HELP; break;
         case 'i': if(optarg) 
	           {  g_free(Closure->imageName);
		      Closure->imageName = g_strdup(optarg); 
		   }
	           break;
         case 'j': if(optarg) Closure->sectorSkip = atoi(optarg) & ~0xf;
	           if(Closure->sectorSkip<0) Closure->sectorSkip = 0;
		   break;
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
		      else if(!strcmp(optarg, "BD") || !strcmp(optarg, "bd"))
			   Closure->mediumSize = BD_SL_SIZE;
		      else if(!strcmp(optarg, "BD2") || !strcmp(optarg, "bd2"))
			   Closure->mediumSize = BD_DL_SIZE;
		      else 
		      {  int len = strlen(optarg);
			 if(strchr("0123456789", optarg[len-1]))
			    Closure->mediumSize = (gint64)atoll(optarg);
		      }
		      break;
		   }
         case 'o': if(!strcmp(optarg, "file"))
		     Closure->eccTarget = ECC_FILE;
	           else if(!strcmp(optarg, "image"))
		     Closure->eccTarget = ECC_IMAGE;
	           else Stop(_("-o/--ecc-target expects 'file' or 'image'"));
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
         case 'r': mode = MODE_SEQUENCE; sequence |= 1<<MODE_READ; 
	           if(optarg) read_range = g_strdup(optarg);
		   break;

         case 's': mode = MODE_SEQUENCE; sequence |= 1<<MODE_SCAN; 
	           if(optarg) read_range = g_strdup(optarg); 
		   break;
                  case 'v': Closure->verbose = TRUE;
	           break;
         case 't': mode = MODE_SEQUENCE; sequence |= 1<<MODE_VERIFY; 
	           if(optarg) Closure->quickVerify = TRUE;
	           break;
         case 'u': Closure->unlinkImage = TRUE; break;
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
	   Closure->cacheMiB = atoi(optarg);
	   if(Closure->cacheMiB <   8) 
	     Stop(_("--cache-size must at least be 8MiB; 16MiB or higher is recommended.")); 
	   if(Closure->cacheMiB > MAX_OLD_CACHE_SIZE) 
	      Stop(_("--cache-size maximum is %dMiB."), MAX_OLD_CACHE_SIZE); 
	   break;
         case MODIFIER_CDUMP:
	   Closure->debugCDump = TRUE;
	   debug_mode_required = TRUE;
	   break;
         case MODIFIER_DAO: 
	   Closure->noTruncate = 1; 
	   break;
         case MODIFIER_EJECT: 
	   Closure->eject = 1; 
	   break;
         case MODIFIER_ENCODING_ALGORITHM:
	   Closure->encodingAlgorithm = ENCODING_ALG_INVALID;
	   if(!strcmp(optarg, "32bit"))
	     Closure->encodingAlgorithm = ENCODING_ALG_32BIT;
	   if(!strcmp(optarg, "64bit"))
	     Closure->encodingAlgorithm = ENCODING_ALG_64BIT;
#ifdef HAVE_SSE2
	   if(!strcmp(optarg, "SSE2"))
	   {  Closure->encodingAlgorithm = ENCODING_ALG_SSE2;

	     if(!Closure->useSSE2)
	       Stop(_("--encoding-algorithm: SSE2 not supported on this processor!"));
	   }

	   if(Closure->encodingAlgorithm == ENCODING_ALG_INVALID)
	     Stop(_("--encoding-algorithm: valid types are 32bit, 64bit, SSE2"));
#endif
#ifdef HAVE_ALTIVEC
	   if(!strcmp(optarg, "AltiVec"))
	   {  Closure->encodingAlgorithm = ENCODING_ALG_ALTIVEC;

	     if(!Closure->useAltiVec)
	       Stop(_("--encoding-algorithm: AltiVec not supported on this processor!"));
	   }

	   if(Closure->encodingAlgorithm == ENCODING_ALG_INVALID)
	     Stop(_("--encoding-algorithm: valid types are 32bit, 64bit, AltiVec"));
#endif
	   if(Closure->encodingAlgorithm == ENCODING_ALG_INVALID)
	     Stop(_("--encoding-algorithm: valid types are 32bit, 64bit"));
	   break;
	 case MODIFIER_ENCODING_IO_STRATEGY:
	   if(!strcmp(optarg, "readwrite"))
	   {  Closure->encodingIOStrategy = IO_STRATEGY_READWRITE;
	   }
	   else if(!strcmp(optarg, "mmap"))
	   {  Closure->encodingIOStrategy = IO_STRATEGY_MMAP;
#ifndef HAVE_MMAP
	      Stop(_("--encoding-io-strategy: mmap not supported on this OS"));
#endif
	   }
	   else
	      Stop(_("--encoding-io-strategy: valid types are readwrite and mmap"));
	   break;
	 case MODIFIER_DRIVER:
#if defined(SYS_LINUX)
	   if(optarg && !strcmp(optarg,"sg"))
	      Closure->useSCSIDriver = DRIVER_SG;
	   else 
	   if(optarg && !strcmp(optarg,"cdrom"))
	      Closure->useSCSIDriver = DRIVER_CDROM;
	   else
	      Stop(_("Valid args for --driver: sg,cdrom"));
#else
	   Stop(_("--driver is only supported on GNU/Linux"));
#endif
	   break;
         case MODIFIER_FILL_UNREADABLE:
	   if(optarg) Closure->fillUnreadable = strtol(optarg, NULL, 0);
	   break;
         case MODIFIER_FIXED_SPEED_VALUES:
	    if(!Closure->debugMode)
	      Stop(_("--fixed-speed-values is only allowed in debug mode"));
	    Closure->fixedSpeedValues=TRUE;
 	    debug_mode_required = TRUE;
	    break;
         case MODIFIER_IGNORE_FATAL_SENSE:
	   Closure->ignoreFatalSense = TRUE;
	   break;
         case MODIFIER_IGNORE_ISO_SIZE:
	   Closure->ignoreIsoSize = TRUE;
	   break;
	 case MODIFIER_INTERNAL_REREADS:
	    if(optarg)
	       Closure->internalAttempts = atoi(optarg);
	    if(Closure->internalAttempts < 0) 
	       Closure->internalAttempts = -1;
	    if(Closure->internalAttempts > 10) 
	       Closure->internalAttempts = 10;
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
	 case MODIFIER_OLD_DS_MARKER:
	    Closure->dsmVersion = 0;
	    break;
         case MODIFIER_PREFETCH_SECTORS:
 	    Closure->prefetchSectors = atoi(optarg);
	    if(   Closure->prefetchSectors < 32
	       || Closure->prefetchSectors > MAX_PREFETCH_CACHE_SIZE)
	      Stop(_("--prefetch-sectors must be in range 32...%s"),
		    MAX_PREFETCH_CACHE_SIZE);
	    break;
         case MODIFIER_RANDOM_SEED:
	   if(optarg) Closure->randomSeed = atoi(optarg);
	   debug_mode_required = TRUE;
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
         case MODIFIER_RESOURCE_FILE:
	   if(Closure->dotFile)
	     g_free(Closure->dotFile);
	   Closure->dotFile = g_strdup(optarg);
	   break;
         case MODIFIER_SCREEN_SHOT:
	   Closure->screenShotMode = TRUE;
	   debug_mode_required = TRUE;
	   break;
         case MODIFIER_SET_VERSION:
	 {  int v1,v2;
	    if(!Closure->debugMode)
	      Stop(_("--set-version is only allowed in debug mode"));
	    g_free(Closure->cookedVersion);
	    Closure->cookedVersion = g_strdup(optarg);
	    sscanf(Closure->cookedVersion,"%d.%d",&v1,&v2);
	    Closure->version=10000*v1+100*v2;
 	    debug_mode_required = TRUE;
	 } 
	    break;
        case MODIFIER_SIMULATE_CD:
	   if(optarg) Closure->simulateCD = g_strdup(optarg);
	   debug_mode_required = TRUE;
	   break;
        case MODIFIER_SIMULATE_DEFECTS:
	   if(optarg) Closure->simulateDefects = atoi(optarg);
	   else Closure->simulateDefects = 10;
	   debug_mode_required = TRUE;
	   break;
         case MODIFIER_SPINUP_DELAY:
	   if(optarg) Closure->spinupDelay = atoi(optarg);
	   break;
         case MODIFIER_SPEED_WARNING:
	   if(optarg) Closure->speedWarning = atoi(optarg);
	   else Closure->speedWarning=10;
	   break;
         case MODIFIER_TRUNCATE: 
	   if(optarg)                  /* debugging truncate mode */
	   {  mode = MODE_TRUNCATE;
	      debug_arg = g_strdup(optarg);
	      debug_mode_required = TRUE;
	   }
	   else Closure->truncate = 1; /* truncate confirmation for fix mode */
	   break;
         case MODIFIER_CLV_SPEED:
	   Closure->driveSpeed = atoi(optarg);
	   break;
         case MODIFIER_CAV_SPEED:
	   Closure->driveSpeed = -atoi(optarg);
	   break;
         case MODIFIER_VERSION:
	    PrintCLI("\n%s\n\n", Closure->versionString);
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
         case MODE_MEDIUM_INFO:
	   mode = MODE_MEDIUM_INFO;
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
         case MODE_SHOW_HEADER:
	   mode = MODE_SHOW_HEADER;
	   debug_arg = g_strdup(optarg);
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
   { if(debug_mode_required)
	 mode=MODE_HELP;

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
        case MODE_SHOW_HEADER:
        case MODE_SHOW_SECTOR:
        case MODE_MARKED_IMAGE:
        case MODE_MERGE_IMAGES:
        case MODE_TRUNCATE:
        case MODE_ZERO_UNREADABLE:
	  mode = MODE_HELP;
          break;
     }
   }
	  
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

   /*** Determine the default device (OS dependent!) if 
	- none has been specified on the command line
        - and one if actually required in command line mode.

	GUI mode will unconditionally query devices later anyways
	in order to build the menu so we don't have to care about
	that now. */

   if(!Closure->device && mode == MODE_SEQUENCE 
      && (sequence & (1<<MODE_READ | 1<<MODE_SCAN))) 
   {  Closure->device = DefaultDevice();
      devices_queried = TRUE;
   }

   /*** Dispatch action depending on mode.
        The major modes can be executed in sequence, 
	but not all combinations may be really useful. */

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
	{  Method *method = FindMethod(Closure->methodName); 

	   if(!method) Stop(_("\nMethod %s not available.\n"
			      "Use -m without parameters for a method list.\n"), 
			    Closure->methodName);

	   method->create();
	}

	if(sequence & 1<<MODE_FIX)
	{  Method *method = NULL;
	   Image *image;

	   PrintLog(_("\nOpening %s"), Closure->imageName);
	   image = OpenImageFromFile(Closure->imageName, O_RDWR, IMG_PERMS);
	   if(!image)
	   {  PrintLog(": %s.\n", strerror(errno));
	   }
	   else 
	   {  if(image->inLast == 2048)
	           PrintLog(_(": %lld medium sectors.\n"), image->sectorSize);
	      else PrintLog(_(": %lld medium sectors and %d bytes.\n"), 
		   image->sectorSize-1, image->inLast);
	   }
	   image = OpenEccFileForImage(image, Closure->eccName, O_RDWR, IMG_PERMS);
	   ReportImageEccInconsistencies(image);

	   /* Determine method. Ecc files win over augmented ecc. */

	   if(image && image->eccFileMethod) method = image->eccFileMethod;
	   else if(image && image->eccMethod) method = image->eccMethod;
	   else Stop("Internal error: No suitable method for repairing image.");

	   method->fix(image);
	}

	if(sequence & 1<<MODE_VERIFY)
	{  Method *method;
	   Image *image;

	   image = OpenImageFromFile(Closure->imageName, O_RDONLY, IMG_PERMS);
	   image = OpenEccFileForImage(image, Closure->eccName, O_RDONLY, IMG_PERMS);

	   /* Determine method. Ecc files win over augmented ecc. */

	   if(image && image->eccFileMethod) method = image->eccFileMethod;
	   else if(image && image->eccMethod) method = image->eccMethod;
	   else if(!(method = FindMethod("RS01")))
	           Stop(_("RS01 method not available for comparing files."));
	     
	   method->verify(image);
	}
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
         if(!Closure->device) Closure->device = DefaultDevice();
	 SendCDB(debug_arg);
	 break;

      case MODE_RAW_SECTOR:
         if(!Closure->device) Closure->device = DefaultDevice();
	 RawSector(debug_arg);
	 break;

      case MODE_READ_SECTOR:
         if(!Closure->device) Closure->device = DefaultDevice();
	 ReadSector(debug_arg);
	 break;

      case MODE_SHOW_HEADER:
	 ShowHeader(debug_arg);
	 break;

      case MODE_SHOW_SECTOR:
	 ShowSector(debug_arg);
	 break;

      case MODE_RANDOM_ERR:
	 RandomError(debug_arg);
	 break;

      case MODE_MARKED_IMAGE:
	 RandomImage(Closure->imageName, debug_arg, 1);
	 break;

      case MODE_MEDIUM_INFO:
         if(!Closure->device) Closure->device = DefaultDevice();
	 PrintMediumInfo(NULL);
	 break;

      case MODE_MERGE_IMAGES:
	 MergeImages(debug_arg, TRUE);
	 break;

      case MODE_RANDOM_IMAGE:
	RandomImage(Closure->imageName, debug_arg, 0);
	 break;

      case MODE_TRUNCATE:
	 TruncateImageFile(debug_arg);
	 break;

      case MODE_ZERO_UNREADABLE:
	 ZeroUnreadable();
	 break;

      default:
	break;
   }

   if(debug_arg) g_free(debug_arg);

   /*** If no mode was selected, print the help screen. */

   if(mode == MODE_HELP)
   {  
     /* TRANSLATORS: Program options like -r and --read are not to be translated
	to avoid confusion when discussing the program in international forums. */
      PrintCLI(_("\nCommon usage examples:\n"
	     "  dvdisaster -r,--read   # Read the medium image to hard disk.\n"
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
	     "  -e,--ecc    eccfile    - name of parity file      (default: medium.ecc)\n"
	     "  -o,--ecc-target [file image] - where to put ecc data in RS03\n"),
	       Closure->device);

      PrintCLI("\n");

      PrintCLI(_("Tweaking options (see manual before using!)\n"));
      PrintCLI(_("  -a,--assume x,y,...    - assume image is augmented with codec(s) x,y,...\n"));
      PrintCLI(_("  -j,--jump n            - jump n sectors forward after a read error (default: 16)\n"));
      PrintCLI(_("  -m n                   - list/select error correction methods (default: RS01)\n"));
      PrintCLI(_("  -n,--redundancy n%%     - error correction data redundancy\n"
		 "                           allowed values depend on codec (see manual)\n"));
      PrintCLI(_("  -v,--verbose           - more diagnostic messages\n"));
      PrintCLI(_("  -x,--threads n         - use n threads for en-/decoding (if supported by codec)\n"));
      PrintCLI(_("  --adaptive-read        - use optimized strategy for reading damaged media\n"));
      PrintCLI(_("  --auto-suffix          - automatically add .iso and .ecc file suffixes\n"));
      PrintCLI(_("  --cache-size n         - image cache size in MiB during -c mode (default: 32MiB)\n"));
      PrintCLI(_("  --dao                  - assume DAO disc; do not trim image end\n"));
      PrintCLI(_("  --defective-dump d     - directory for saving incomplete raw sectors\n"));
#ifdef SYS_LINUX
      PrintCLI(_("  --driver=sg/cdrom      - use sg(default) or alternative cdrom driver (see man page!)\n"));
#endif
      PrintCLI(_("  --eject                - eject medium after successful read\n"));
      PrintCLI(_("  --encoding-algorithm n - possible values: 32bit,64bit,SSE2,AltiVec\n"));
      PrintCLI(_("  --encoding-io-strategy n - possible values: readwrite, mmap\n"));
      PrintCLI(_("  --fill-unreadable n    - fill unreadable sectors with byte n\n"));
      PrintCLI(_("  --ignore-fatal-sense   - continue reading after potentially fatal error conditon\n"));
      PrintCLI(_("  --ignore-iso-size      - ignore image size from ISO/UDF data (dangerous - see man page!)\n"));
      PrintCLI(_("  --internal-rereads n   - drive may attempt n rereads before reporting an error\n"));
      PrintCLI(_("  --medium-info          - print info about medium in drive\n"));
      PrintCLI(_("  --old-ds-marker        - mark missing sectors compatible with dvdisaster <= 0.70\n"));
      PrintCLI(_("  --prefetch-sectors n   - prefetch n sectors for RS03 encoding (uses ~nMiB)\n"));
      PrintCLI(_("  --raw-mode n           - mode for raw reading CD media (20 or 21)\n"));
      PrintCLI(_("  --read-attempts n-m    - attempts n upto m reads of a defective sector\n"));
      PrintCLI(_("  --read-medium n        - read the whole medium up to n times\n"));
      PrintCLI(_("  --read-raw             - performs read in raw mode if possible\n"));
      PrintCLI(_("  --resource-file p      - get resource file from given path\n"));
      PrintCLI(_("  --speed-warning n      - print warning if speed changes by more than n percent\n"));
      PrintCLI(_("  --spinup-delay n       - wait n seconds for drive to spin up\n"));

      if(Closure->debugMode)
      { PrintCLI("\n");
	PrintCLI(_("Debugging options (purposefully undocumented and possibly harmful)\n"));
	PrintCLI(_("  --debug           - enables the following options\n"));
	PrintCLI(_("  --byteset s,i,b   - set byte i in sector s to b\n"));
	PrintCLI(_("  --cdump           - creates C #include file dumps instead of hexdumps\n")); 
	PrintCLI(_("  --compare-images a,b  - compare sectors in images a and b\n"));
	PrintCLI(_("  --copy-sector a,n,b,m - copy sector n from image a to sector m in image b\n"));
	PrintCLI(_("  --erase sector    - erase the given sector\n"));
	PrintCLI(_("  --erase n-m       - erase sectors n - m, inclusively\n"));
	PrintCLI(_("  --fixed-speed-values - output fixed speed values for better output diffing\n"));
	PrintCLI(_("  --marked-image n  - create image with n marked random sectors\n"));
	PrintCLI(_("  --merge-images a,b  merge image a with b (a receives sectors from b)\n"));
	PrintCLI(_("  --random-errors e - seed image with (correctable) random errors\n"));
	PrintCLI(_("  --random-image n  - create image with n sectors of random numbers\n"));
	PrintCLI(_("  --random-seed n   - random seed for built-in random number generator\n"));
	PrintCLI(_("  --raw-sector n    - shows hexdump of the given raw sector from medium in drive\n"));
	PrintCLI(_("  --read-sector n   - shows hexdump of the given sector from medium in drive\n"));
	PrintCLI(_("  --screen-shot     - useful for generating screen shots\n"));
	PrintCLI(_("  --send-cdb arg    - executes given cdb at drive; kills system if used wrong\n"));
	PrintCLI(_("  --set-version     - set program version for debugging purposes (dangerous!)\n"));
	PrintCLI(_("  --show-header n   - assumes given sector is a ecc header and prints it\n"));
	PrintCLI(_("  --show-sector n   - shows hexdump of the given sector in an image file\n"));
	PrintCLI(_("  --sim-cd image    - simulate a SCSI-Level CD with contents supplied by the ISO image\n"));
	PrintCLI(_("  --sim-defects n   - simulate n%% defective sectors on medium\n"));
	PrintCLI(_("  --truncate n      - truncates image to n sectors\n")); 
	PrintCLI(_("  --zero-unreadable - replace the \"unreadable sector\" markers with zeros\n\n"));
      }

      FreeClosure();
      exit(EXIT_FAILURE);
   }

   /* If no mode was selected at the command line, 
      start the graphical user interface. */

   if(mode == MODE_NONE)
   {  
      /* We need to query devices in order to build
	 the drop-down menu.*/

      if(!devices_queried)
      {  if(Closure->device)
	    g_free(Closure->device);
	 Closure->device = DefaultDevice();
      }

      /* Insert generic drive name for screen shot mode */

      if(Closure->screenShotMode)
      {  GPtrArray *a=Closure->deviceNames;
	 int i;
	 for(i=0; i<a->len; i++)
	 {  char *p = g_ptr_array_index(a,i);
	    g_free(p);
	    a->pdata[i] = g_strdup(_("Optical drive 52X FW 1.02"));
	 }
      }

      Closure->guiMode = TRUE;
      ReadDotfile();
      CreateMainWindow(&argc, &argv);
   }

   FreeClosure();
   exit(exitCode);
}
