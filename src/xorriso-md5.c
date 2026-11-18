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
 *
 * The check does not read the image itself. The verify functions of the
 * codecs feed it every image sector in ascending order while they read
 * the image anyway (XorrisoCheckSector()), so the image is read only once.
 * The last 32 sectors are kept, which covers the range of a superblock
 * tag; a session is therefore recognized by its superblock tag wherever
 * it starts, without guessing the session start.
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

#define RING 32           /* a superblock tag covers less than 32 sectors */

enum { RES_GOOD, RES_MISMATCH, RES_DAMAGED, RES_UNREADABLE, RES_MISSING_NEXT };

typedef struct
{  int type, result;
   gint64 pos, start;    /* RES_MISSING_NEXT: pos is the expected sector */
} TagResult;

struct _XorrisoCheck
{  gint64 nextLba;                    /* next sector expected from the caller */
   int outOfOrder;                    /* caller skipped sectors; results void */

   unsigned char ring[RING][2048];    /* the last RING sectors */
   int ringPresent[RING];

   int inChain;                       /* inside a session, after its superblock tag */
   gint64 sessionStart, expectedTag;
   int sessionMissing;                /* missing sectors in sessionStart..current */
   struct MD5Context ctx;             /* md5 of sessionStart..current */

   GArray *results;
};

XorrisoCheck* XorrisoCheckNew(void)
{  XorrisoCheck *xc = g_malloc0(sizeof(XorrisoCheck));

   xc->results = g_array_new(FALSE, FALSE, sizeof(TagResult));
   return xc;
}

void XorrisoCheckFree(XorrisoCheck *xc)
{  if(!xc) return;
   g_array_free(xc->results, TRUE);
   g_free(xc);
}

static void add_result(XorrisoCheck *xc, int type, int result, gint64 pos, gint64 start)
{  TagResult r;

   r.type = type; r.result = result; r.pos = pos; r.start = start;
   g_array_append_val(xc->results, r);
}

/* Compare a tag against the md5 of its range */

static void check_tag(XorrisoCheck *xc, XorrisoTag *tag, gint64 start,
		      struct MD5Context *range_ctx, int missing)
{  struct MD5Context copy = *range_ctx;
   unsigned char digest[16];
   char ascii[33];
   int result;

   MD5Final(digest, &copy);
   AsciiDigest(ascii, digest);

   if(!tag->selfOk || tag->rangeStart != start || tag->rangeSize != tag->pos - start)
        result = RES_DAMAGED;
   else if(missing)
        result = RES_UNREADABLE;
   else if(strcmp(ascii, tag->md5))
        result = RES_MISMATCH;
   else result = RES_GOOD;

   add_result(xc, tag->type, result, tag->pos, start);
}

/* Outside of a session: look for a (relocated) superblock tag.
   Its range lies within the last RING sectors. */

static void look_for_superblock(XorrisoCheck *xc, gint64 lba, unsigned char *buf)
{  struct MD5Context ctx;
   XorrisoTag tag;
   gint64 s, start;
   int type, missing = 0;

   if(strncmp((char*)buf, "libisofs_", 9))   /* fast path */
      return;

   type = parse_tag(buf, &tag);
   if(type != SB_TAG && type != RLSB_TAG)
      return;
   if(tag.pos != lba || tag.rangeStart < 0
      || lba - tag.rangeStart <= 0 || lba - tag.rangeStart >= RING)
      return;
   if(type == RLSB_TAG && tag.rangeStart != 0)
      return;

   start = tag.rangeStart;
   MD5Init(&ctx);
   for(s=start; s<lba; s++)
   {  MD5Update(&ctx, xc->ring[s%RING], 2048);
      if(!xc->ringPresent[s%RING]) missing = 1;
   }
   check_tag(xc, &tag, start, &ctx, missing);

   if(type == RLSB_TAG)  /* stands alone; the first session follows at 32 */
      return;

   /* Superblock tag: the tree and session tags cover the session
      from its start, including the superblock tag itself. */

   xc->inChain = TRUE;
   xc->sessionStart = start;
   xc->sessionMissing = missing;
   xc->ctx = ctx;
   MD5Update(&xc->ctx, buf, 2048);
   xc->expectedTag = tag.next;
   if(tag.next <= lba)
   {  add_result(xc, 0, RES_MISSING_NEXT, tag.next, start);
      xc->inChain = FALSE;
   }
}

/* Feed the next image sector. Sectors must be fed in ascending order
   starting at sector 0; present is FALSE for missing (unreadable) sectors. */

void XorrisoCheckSector(XorrisoCheck *xc, gint64 lba, unsigned char *buf, int present)
{  if(!xc || xc->outOfOrder)
      return;
   if(lba != xc->nextLba)
   {  xc->outOfOrder = TRUE;
      return;
   }
   xc->nextLba++;

   memcpy(xc->ring[lba%RING], buf, 2048);
   xc->ringPresent[lba%RING] = present;

   if(xc->inChain)
   {  if(lba == xc->expectedTag)
      {  XorrisoTag tag;
	 int type = present ? parse_tag(buf, &tag) : NO_TAG;

	 if((type == TREE_TAG || type == SESSION_TAG) && tag.pos == lba)
	 {  check_tag(xc, &tag, xc->sessionStart, &xc->ctx, xc->sessionMissing);

	    if(type == SESSION_TAG)
	    {  xc->inChain = FALSE;
	       return;
	    }

	    MD5Update(&xc->ctx, buf, 2048);
	    xc->expectedTag = tag.next;
	    if(tag.next <= lba)
	    {  add_result(xc, 0, RES_MISSING_NEXT, tag.next, xc->sessionStart);
	       xc->inChain = FALSE;
	    }
	    return;
	 }

	 /* Chain is broken; this sector might start something new */

	 add_result(xc, 0, present ? RES_MISSING_NEXT : RES_UNREADABLE, lba, xc->sessionStart);
	 xc->inChain = FALSE;
      }
      else
      {  MD5Update(&xc->ctx, buf, 2048);
	 if(!present) xc->sessionMissing = TRUE;
	 return;
      }
   }

   if(present)
      look_for_superblock(xc, lba, buf);
}

/* Print the results after the image has been read completely.
   Sets the exit code and the JSON report. Returns NULL if the image
   has no tags, otherwise the printed text for the GUI (to be freed by
   the caller). Frees xc. */

char* XorrisoCheckFinish(XorrisoCheck *xc, int have_ecc)
{  static char *names[] = { "next tag", "relocated sb tag", "superblock tag",
			    "tree tag", "session tag" };
   GString *out;
   char *text;
   int i, good = 0, bad = 0;

   if(!xc)
      return NULL;

   if(xc->inChain)   /* image ends before the next tag of the chain */
      add_result(xc, 0, RES_MISSING_NEXT, xc->expectedTag, xc->sessionStart);

   if(xc->outOfOrder || !xc->results->len)
   {  XorrisoCheckFree(xc);
      return NULL;
   }

   out = g_string_new(NULL);
   g_string_append(out, _("\nxorriso MD5 checksum tags:\n"));

   for(i=0; i<xc->results->len; i++)
   {  TagResult *r = &g_array_index(xc->results, TagResult, i);
      char *result;

      if(r->result == RES_MISSING_NEXT)
      {  bad++;
	 g_string_append_printf(out, _("- %-17s: missing (expected at sector %" PRId64 ")\n"),
				names[0], r->pos);
	 continue;
      }

      switch(r->result)
      {  case RES_GOOD:       result = _("good"); good++; break;
         case RES_MISMATCH:   result = _("MD5 MISMATCH"); bad++; break;
         case RES_DAMAGED:    result = _("damaged tag"); bad++; break;
         default:             result = _("unreadable sectors"); bad++; break;
      }

      if(r->type)
	 g_string_append_printf(out, "- %-17s: %s (sector %" PRId64 ", covering sectors %" PRId64 "-%" PRId64 ")\n",
				names[r->type], result, r->pos, r->start, r->pos-1);
      else  /* next tag of a chain itself unreadable */
	 g_string_append_printf(out, "- %-17s: %s (sector %" PRId64 ")\n",
				names[0], result, r->pos);
   }

   if(!bad)
      g_string_append(out, _("- all xorriso checksums are good.\n"));

   PrintLog("%s", out->str);

   ReportNumber("xorriso_tags", "good", good);
   ReportNumber("xorriso_tags", "bad", bad);

   /* Like missing sectors or CRC errors: repairable only with ecc data */

   if(bad)
      SetExitCode(EXIT_FAILURE, have_ecc ? EXIT_CODE_DEFECTS : EXIT_CODE_INCOMPLETE);

   text = g_strdup(g_strstrip(out->str));
   g_string_free(out, TRUE);
   XorrisoCheckFree(xc);

   return text;
}
