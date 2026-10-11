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

/*** src type: no GUI code ***/

#include "dvdisaster.h"

#include "scsi-layer.h"

#include <time.h>

/***
 *** Machine readable summary of a command line run (--json).
 ***
 * Results are collected as (section, key, value) triples while the
 * actions run and written as one JSON object when the program exits.
 * A NULL section means a top level key. Setting a key again replaces
 * its value. Codecs may add their own keys (e.g. to the "ecc" section)
 * by calling ReportString() and ReportNumber().
 */

typedef struct
{  char *section;    /* string constants only */
   char *key;
   GString *value;   /* JSON encoded value */
} report_item;

static GPtrArray *items;

/* Returns the value of the given key (emptied unless keep is set) */

static GString* item_value(char *section, char *key, int keep)
{  report_item *item;
   int i, last = -1;

   if(!Closure->jsonFile || Closure->guiMode)
     return NULL;

   if(!items)
     items = g_ptr_array_new();

   for(i=0; i<items->len; i++)
   {  item = g_ptr_array_index(items, i);
      if(g_strcmp0(item->section, section))
	continue;
      if(!strcmp(item->key, key))
      {  if(!keep) g_string_truncate(item->value, 0);
	 return item->value;
      }
      last = i;
   }

   item = g_malloc(sizeof(report_item));
   item->section = section;
   item->key = key;
   item->value = g_string_new(NULL);

   /* keep the keys of a section together */

   if(last >= 0) g_ptr_array_insert(items, last+1, item);
   else          g_ptr_array_add(items, item);

   return item->value;
}

static void append_json_string(GString *out, const char *s)
{  g_string_append_c(out, '"');
   for(; *s; s++)
   {  unsigned char c = *s;

      switch(c)
      {  case '"':  g_string_append(out, "\\\""); break;
	 case '\\': g_string_append(out, "\\\\"); break;
	 case '\n': g_string_append(out, "\\n"); break;
	 case '\t': g_string_append(out, "\\t"); break;
	 default:
	   if(c < 0x20) g_string_append_printf(out, "\\u%04x", c);
	   else         g_string_append_c(out, c);
	   break;
      }
   }
   g_string_append_c(out, '"');
}

void ReportString(char *section, char *key, char *value)
{  GString *v = item_value(section, key, FALSE);

   if(!v) return;
   if(value) append_json_string(v, value);
   else      g_string_append(v, "null");
}

void ReportNumber(char *section, char *key, gint64 value)
{  GString *v = item_value(section, key, FALSE);

   if(v) g_string_append_printf(v, "%" PRId64, value);
}

void ReportMd5(char *section, char *key, unsigned char *digest)
{  char ascii[33];

   AsciiDigest(ascii, digest);
   ReportString(section, key, ascii);
}

/* "actions" is an array of the actions in the order they were started */

void ReportAction(char *action)
{  GString *v = item_value(NULL, "actions", TRUE);

   if(!v) return;

   if(v->len) g_string_overwrite(v, v->len-1, ", ");  /* replace "]" */
   else       g_string_append_c(v, '[');
   append_json_string(v, action);
   g_string_append_c(v, ']');
}

/* Information about the drive and medium */

void ReportDevice(DeviceHandle *dh)
{  ReportString(NULL, "device", dh->device);
   ReportString("drive", "info", dh->devinfo);
   ReportString("medium", "type", dh->typeDescr);
   ReportString("medium", "manufacturer", dh->manuID);
   ReportNumber("medium", "sectors", dh->sectors);
}

/* Codec independent information from the ecc header */

void ReportEccHeader(EccHeader *eh)
{  static unsigned char zero[16];
   GString *v;
   char method[5];

   if(!eh) return;

   memcpy(method, eh->method, 4);
   method[4] = 0;

   ReportString("ecc", "codec", method);
   ReportNumber("ecc", "data_bytes", eh->dataBytes);
   ReportNumber("ecc", "ecc_bytes", eh->eccBytes);
   v = item_value("ecc", "redundancy", FALSE);  /* percent; locale independent */
   if(v)
   {  char buf[G_ASCII_DTOSTR_BUF_SIZE];

      g_string_append(v, g_ascii_formatd(buf, sizeof(buf), "%.1f",
					 ((double)eh->eccBytes*100.0)/(double)eh->dataBytes));
   }
   ReportNumber("ecc", "medium_sectors", uchar_to_gint64(eh->sectors));
   if(memcmp(eh->mediumSum, zero, 16))  /* not filled in by all codecs */
      ReportMd5("ecc", "medium_md5", eh->mediumSum);
}

/*
 * Write the report. Called once at exit, with the error message
 * if we are terminated by Stop().
 */

static char *verdicts[] = { "ok", "error", "defects", "data_loss" };

void WriteReport(char *error_msg)
{  GString *out;
   char *section = "";
   char timestamp[32];
   time_t now;
   FILE *file;
   int i;

   if(!Closure->jsonFile || Closure->guiMode)
     return;

   if(error_msg)
     ReportString(NULL, "error", error_msg);
   ReportNumber(NULL, "exit_code", exitCode);
   ReportString(NULL, "verdict",
		exitCode >= 0 && exitCode <= EXIT_CODE_DATA_LOSS ? verdicts[exitCode] : "error");

   /* Top level keys first, then the sections in order of appearance */

   now = time(NULL);
   strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
   out = g_string_new("{\n  \"program\": \"dvdisaster\",\n  \"version\": ");
   append_json_string(out, Closure->cookedVersion);
   g_string_append_printf(out, ",\n  \"time\": \"%s\"", timestamp);

   for(i=0; i<items->len; i++)
   {  report_item *item = g_ptr_array_index(items, i);

      if(!item->section)
	g_string_append_printf(out, ",\n  \"%s\": %s", item->key, item->value->str);
   }

   for(i=0; i<items->len; i++)
   {  report_item *item = g_ptr_array_index(items, i);

      if(!item->section)
	continue;

      if(strcmp(item->section, section))  /* keys of a section are adjacent */
      {  g_string_append_printf(out, "%s,\n  \"%s\": {\n", *section ? "\n  }" : "", item->section);
	 section = item->section;
      }
      else g_string_append(out, ",\n");
      g_string_append_printf(out, "    \"%s\": %s", item->key, item->value->str);
   }
   g_string_append_printf(out, "%s\n}\n", *section ? "\n  }" : "");

   /* Write it */

   if(!strcmp(Closure->jsonFile, "-"))
      file = stdout;
   else file = fopen(Closure->jsonFile, "w");

   if(!file)
   {  g_fprintf(stderr, _("Could not open %s: %s"), Closure->jsonFile, strerror(errno));
      g_fprintf(stderr, "\n");
   }
   else
   {  fputs(out->str, file);
      if(file == stdout) fflush(stdout);
      else               fclose(file);
   }

   /* Clean up */

   g_string_free(out, TRUE);
   for(i=0; i<items->len; i++)
   {  report_item *item = g_ptr_array_index(items, i);

      g_string_free(item->value, TRUE);
      g_free(item);
   }
   g_ptr_array_free(items, TRUE);
   items = NULL;
   g_free(Closure->jsonFile);
   Closure->jsonFile = NULL;   /* prevent writing twice */
}
