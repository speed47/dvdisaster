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

/***
 *** Verify the MD5 checksum tags written by xorriso / libisofs ("-md5 on").
 ***
 * Each tag is a line of text at the beginning of a 2048 byte block
 * (see doc/checksums.txt in the libisofs sources):
 *
 * Tag_id pos=# range_start=# range_size=# [session_start|next=#] md5=# self=#\n
 *
 * md5 covers range_size sectors starting at range_start (= session start)
 * and ending right before the tag; self is the md5 of the tag line up to
 * and including the last digit of md5. In each session the superblock tag
 * follows the volume descriptors (sectors 16-31 of the session) and points
 * via next= to the tree tag, which points to the session tag.
 * Images written for overwritable media begin with a relocated superblock
 * (tag at sector 16-31) and have their first session at sector 32.
 * Further sessions start at the next multiple of 32 after a session tag.
 */

enum { NO_TAG, RLSB_TAG, SB_TAG, TREE_TAG, SESSION_TAG };

static char *tag_ids[] = { NULL, "libisofs_rlsb32_checksum_tag_v1", "libisofs_sb_checksum_tag_v1",
			   "libisofs_tree_checksum_tag_v1", "libisofs_checksum_tag_v1" };

typedef struct
{  int type;
   gint64 pos, rangeStart, rangeSize, next;
   char md5[33];
   int selfOk;
} XorrisoTag;

#define CHUNK 64

static gint64 get_num(char *line, char *key)
{  char *p = strstr(line, key);

   return p ? (gint64)g_ascii_strtoull(p+strlen(key), NULL, 10) : -1;
}

static int is_md5(char *p)
{  return strspn(p, "0123456789abcdef") == 32;
}

/* Parse the tag at the beginning of buf. Returns the tag type or NO_TAG. */

static int parse_tag(unsigned char *buf, XorrisoTag *tag)
{  struct MD5Context ctx;
   unsigned char digest[16];
   char line[256], self[33], *p, *md5;
   int type;

   memcpy(line, buf, 255);
   line[255] = 0;
   if(!(p = strchr(line, '\n')))
      return NO_TAG;
   *p = 0;

   for(type=RLSB_TAG; type<=SESSION_TAG; type++)
   {  int len = strlen(tag_ids[type]);

      if(!strncmp(line, tag_ids[type], len) && !strncmp(line+len, " pos=", 5))
	 break;
   }
   if(type > SESSION_TAG)
      return NO_TAG;

   md5 = strstr(line, " md5=");
   if(!md5 || !is_md5(md5+5) || strncmp(md5+37, " self=", 6) || !is_md5(md5+43))
      return NO_TAG;

   tag->type       = type;
   tag->pos        = get_num(line, " pos=");
   tag->rangeStart = get_num(line, " range_start=");
   tag->rangeSize  = get_num(line, " range_size=");
   tag->next       = get_num(line, " next=");
   memcpy(tag->md5, md5+5, 32);
   tag->md5[32] = 0;

   MD5Init(&ctx);
   MD5Update(&ctx, (unsigned char*)line, md5+37-line);
   MD5Final(digest, &ctx);
   AsciiDigest(self, digest);
   tag->selfOk = !strncmp(self, md5+43, 32);

   return type;
}

/* Add sectors *lba ... end-1 to the md5 sum */

static int md5_sectors(Image *image, struct MD5Context *ctx, gint64 *lba, gint64 end, unsigned char *buf)
{  while(*lba < end)
   {  int n = end - *lba > CHUNK ? CHUNK : end - *lba;

      if(ImageReadSectors(image, buf, *lba, n) != n)
	 return FALSE;
      MD5Update(ctx, buf, n*2048);
      *lba += n;
   }

   return TRUE;
}

static void print_tag(XorrisoTag *tag, gint64 start, char *result)
{  static char *names[] = { NULL, "relocated sb tag", "superblock tag",
			    "tree tag", "session tag" };

   PrintLog("- %-17s: %s (sector %" PRId64 ", covering sectors %" PRId64 "-%" PRId64 ")\n",
	    names[tag->type], result, tag->pos, start, tag->pos-1);
}

/* Verify the tag chain of the session starting at sector start.
   Returns the sector of the final tag, or -1 if the chain is broken. */

static gint64 check_session(Image *image, gint64 start, XorrisoTag *tag, unsigned char *buf, int *bad)
{  struct MD5Context ctx;
   gint64 lba = start;

   MD5Init(&ctx);

   for(;;)
   {  struct MD5Context copy;
      unsigned char digest[16];
      char ascii[33];
      gint64 next;

      if(!md5_sectors(image, &ctx, &lba, tag->pos, buf))
      {  print_tag(tag, start, _("unreadable sectors"));
	 (*bad)++;
	 return -1;
      }

      copy = ctx;
      MD5Final(digest, &copy);
      AsciiDigest(ascii, digest);

      if(!tag->selfOk || tag->rangeStart != start || tag->rangeSize != tag->pos - start)
      {  print_tag(tag, start, _("damaged tag"));
	 (*bad)++;
      }
      else if(strcmp(ascii, tag->md5))
      {  print_tag(tag, start, _("MD5 MISMATCH"));
	 (*bad)++;
      }
      else print_tag(tag, start, _("good"));

      if(tag->type == RLSB_TAG || tag->type == SESSION_TAG)
	 return tag->pos;

      /* Superblock and tree tags point to the next tag of the chain */

      next = tag->next;
      if(next <= tag->pos
	 || ImageReadSectors(image, buf, next, 1) != 1
	 || parse_tag(buf, tag) <= SB_TAG
	 || tag->pos != next)
      {  PrintLog(_("- %-17s: missing (expected at sector %" PRId64 ")\n"), "next tag", next);
	 (*bad)++;
	 return -1;
      }
   }
}

/* Look for xorriso checksum tags in the image file and verify them.
   Prints nothing if the image has no such tags. */

void CheckXorrisoTags(char *image_name)
{  Image *image = g_malloc0(sizeof(Image));
   unsigned char *buf;
   gint64 start = 0;
   int found = 0, bad = 0;

   /* Plain file access; OpenImageFromFile() would examine the image again */

   image->type = IMAGE_FILE;
   image->file = LargeOpen(image_name, O_RDONLY, IMG_PERMS);
   if(!image->file)
   {  g_free(image);
      return;
   }
   CalcSectors(image->file->size, &image->sectorSize, &image->inLast);

   buf = g_malloc(CHUNK*2048);

   while(start+32 <= (gint64)image->sectorSize)
   {  XorrisoTag tag;
      gint64 lba, end;

      /* Superblock tag follows the volume descriptors */

      for(lba=start+16; lba<start+32; lba++)
	 if(ImageReadSectors(image, buf, lba, 1) == 1
	    && parse_tag(buf, &tag) != NO_TAG
	    && tag.pos == lba
	    && (tag.type == SB_TAG || (tag.type == RLSB_TAG && !start)))
	    break;
      if(lba == start+32)
	 break;

      if(!found++)
	 PrintLog(_("\nxorriso MD5 checksum tags:\n"));

      end = check_session(image, start, &tag, buf, &bad);
      if(end < 0)
	 break;
      start = (end/32+1)*32;
   }

   if(found && !bad)
      PrintLog(_("- all xorriso checksums are good.\n"));

   g_free(buf);
   LargeClose(image->file);
   g_free(image);
}
