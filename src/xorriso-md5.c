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

/* Tag type from the tag id at the beginning of buf alone, or NO_TAG */

static int tag_id_type(unsigned char *buf)
{  int type;

   for(type=RLSB_TAG; type<=SESSION_TAG; type++)
   {  int len = strlen(tag_ids[type]);

      if(!strncmp((char*)buf, tag_ids[type], len) && !strncmp((char*)buf+len, " pos=", 5))
	 return type;
   }

   return NO_TAG;
}

/* The pos= of a tag id at the beginning of buf, even if the tag is
   damaged otherwise; -1 if unknown */

static gint64 tag_id_pos(unsigned char *buf)
{  char line[256];

   memcpy(line, buf, 255);
   line[255] = 0;
   return get_num(line, " pos=");
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

   type = tag_id_type((unsigned char*)line);
   if(type == NO_TAG)
      return NO_TAG;

   md5 = strstr(line, " md5=");
   if(!md5 || !is_md5(md5+5) || strncmp(md5+37, " self=", 6) || !is_md5(md5+43))
      return NO_TAG;

   tag->type       = type;
   tag->pos        = get_num(line, " pos=");
   tag->rangeStart = get_num(line, " range_start=");
   tag->rangeSize  = get_num(line, " range_size=");
   tag->next       = get_num(line, " next=");
   if((type == SB_TAG || type == TREE_TAG) && tag->next < 0)
      return NO_TAG;              /* no pointer to the next tag */
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

enum { RES_GOOD, RES_MISMATCH, RES_DAMAGED, RES_UNREADABLE, RES_MISSING_NEXT,
       RES_UNCHECKED };

typedef struct
{  int type, result;
   gint64 pos, start;    /* RES_MISSING_NEXT: pos is the expected sector;
			    start < 0: range unknown */
} TagResult;

struct _XorrisoCheck
{  gint64 nextLba;                    /* next sector expected from the caller */

   unsigned char ring[RING][2048];    /* the last RING sectors */
   int ringPresent[RING];

   int inChain;                       /* inside a session, after its superblock tag */
   gint64 sessionStart, expectedTag;
   int sessionMissing;                /* missing sectors in sessionStart..current */
   struct MD5Context ctx;             /* md5 of sessionStart..current */
   int sawRlsb;                       /* relocated superblock tag seen */
   gint64 brokenAt;                   /* >= 0: sector fed out of order */

   GArray *results;
};

XorrisoCheck* XorrisoCheckNew(void)
{  XorrisoCheck *xc = g_malloc0(sizeof(XorrisoCheck));

   xc->results = g_array_new(FALSE, FALSE, sizeof(TagResult));
   xc->brokenAt = -1;
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

/* Outside of a session: look for a (relocated) superblock tag, which
   starts the check of a session. Its range lies within the last RING
   sectors. Other sectors carrying a tag id are reported, so that a
   damaged superblock tag does not hide its whole session:
   - a tag failing to parse or its self check is a damaged tag, unless
     its pos= is not its own sector (then it is not ours, see below),
   - a good tree or session tag here lacks its superblock tag (damaged
     beyond recognition or unreadable) and can not be checked.
   A good tag whose pos is not its own sector belongs to something else,
   e.g. a tagged .iso stored as a file in the image, or a session image
   written for appending at an offset (-C), and is ignored.
   Also ignored: tree and session tags covering a range from sector 0 in
   an image with a relocated superblock tag. They belong to a first
   session written at sector 0 (e.g. by xorriso -as mkisofs --md5) to
   which sessions were appended later; growing the image overwrote
   sectors 0-31 with the relocated superblock, so these tags can not
   match any more and their superblock tag is gone. */

static void look_for_tag(XorrisoCheck *xc, gint64 lba, unsigned char *buf)
{  struct MD5Context ctx;
   XorrisoTag tag;
   gint64 s, start;
   int type, id_type, missing = 0;

   if(strncmp((char*)buf, "libisofs_", 9))   /* fast path */
      return;

   id_type = tag_id_type(buf);
   if(id_type == NO_TAG)
      return;

   type = parse_tag(buf, &tag);
   if(type == NO_TAG || !tag.selfOk)
   {  if(tag_id_pos(buf) != lba)   /* foreign, or data looking like a tag */
	 return;
      add_result(xc, id_type, RES_DAMAGED, lba, -1);
      return;
   }

   if(tag.pos != lba)   /* foreign tag */
      return;

   if(type == RLSB_TAG)
      xc->sawRlsb = TRUE;

   if(type == TREE_TAG || type == SESSION_TAG)
   {  if(xc->sawRlsb && tag.rangeStart == 0)   /* superseded first session */
	 return;
      add_result(xc, type, RES_UNCHECKED, lba, -1);
      return;
   }

   if(tag.rangeStart < 0 || lba - tag.rangeStart <= 0 || lba - tag.rangeStart >= RING
      || (type == RLSB_TAG && tag.rangeStart != 0))
   {  add_result(xc, type, RES_DAMAGED, lba, -1);
      return;
   }

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
   starting at sector 0, without gaps; present is FALSE for missing
   (unreadable) sectors. Anything else is a bug in the calling codec;
   the check then ends and the report shows an internal error (no Stop()
   here: it would bypass the caller's cleanup). */

void XorrisoCheckSector(XorrisoCheck *xc, gint64 lba, unsigned char *buf, int present)
{  if(!xc || xc->brokenAt >= 0)
      return;
   if(lba != xc->nextLba)
   {  xc->brokenAt = lba;
      xc->inChain = FALSE;
      return;
   }
   xc->nextLba++;

   /* The ring is only read by look_for_tag(), i.e. outside of a chain.
      Inside a chain, keep only the sectors right before the next expected
      tag, where the chain may end or break. */

   if(!xc->inChain || lba >= xc->expectedTag - RING)
   {  memcpy(xc->ring[lba%RING], buf, 2048);
      xc->ringPresent[lba%RING] = present;
   }

   if(xc->inChain)
   {  if(lba == xc->expectedTag)
      {  XorrisoTag tag;
	 int type = present ? parse_tag(buf, &tag) : NO_TAG;
	 int id_type = present ? tag_id_type(buf) : NO_TAG;

	 if((type == TREE_TAG || type == SESSION_TAG) && tag.pos == lba)
	 {  check_tag(xc, &tag, xc->sessionStart, &xc->ctx, xc->sessionMissing);

	    /* A damaged tag's next= can not be trusted; later tags of the
	       session are then reported as not checked. */

	    if(type == SESSION_TAG || !tag.selfOk)
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

	 xc->inChain = FALSE;

	 if(!present)
	 {  add_result(xc, 0, RES_UNREADABLE, lba, xc->sessionStart);
	    return;
	 }

	 /* The expected tag is there but damaged: report it once */

	 if(id_type != NO_TAG && (type == NO_TAG || !tag.selfOk))
	 {  add_result(xc, id_type, RES_DAMAGED, lba, -1);
	    return;
	 }

	 /* No tag of this chain here; the sector might start something new */

	 add_result(xc, 0, RES_MISSING_NEXT, lba, xc->sessionStart);
      }
      else
      {  MD5Update(&xc->ctx, buf, 2048);
	 if(!present) xc->sessionMissing = TRUE;
	 return;
      }
   }

   if(present)
      look_for_tag(xc, lba, buf);
}

static char* tag_name(int type)
{  switch(type)
   {  case RLSB_TAG:    return _("relocated sb tag");
      case SB_TAG:      return _("superblock tag");
      case TREE_TAG:    return _("tree tag");
      case SESSION_TAG: return _("session tag");
      default:          return _("next tag");
   }
}

/* End of the image: a chain still open lacks its next tag */

static void close_chain(XorrisoCheck *xc)
{  if(xc->inChain)
   {  add_result(xc, 0, RES_MISSING_NEXT, xc->expectedTag, xc->sessionStart);
      xc->inChain = FALSE;
   }
}

/* Number of bad tags, to be called after all sectors have been fed;
   for the codecs' verdict on the image. */

int XorrisoCheckBad(XorrisoCheck *xc)
{  int i, bad = 0;

   if(!xc)
      return 0;

   close_chain(xc);
   for(i=0; i<xc->results->len; i++)
      if(g_array_index(xc->results, TagResult, i).result != RES_GOOD)
	 bad++;

   return bad + (xc->brokenAt >= 0);
}

/* Append a result line to the log text and, if markup is not NULL,
   to the GUI text (red if bad). */

static void add_line(GString *out, GString *markup, int bad, char *format, ...) PRINTF_FORMAT(4);

static void add_line(GString *out, GString *markup, int bad, char *format, ...)
{  va_list args;
   char *line;

   va_start(args, format);
   line = g_strdup_vprintf(format, args);
   va_end(args);

   g_string_append(out, line);
   if(markup)
   {  if(bad)
	 g_string_append_printf(markup, "<span %s>%s</span>", Closure->redMarkup, line);
      else g_string_append(markup, line);
   }
   g_free(line);
}

/* Print the results after the image has been read completely.
   Sets the exit code and the JSON report. Returns NULL if the image
   has no tags, otherwise the text for the GUI (with markup if wanted;
   to be freed by the caller). */

static char* finish(XorrisoCheck *xc, int repairable, int want_markup)
{  GString *out, *markup;
   char *text;
   int i, good = 0, bad = 0;

   if(!xc)
      return NULL;

   close_chain(xc);

   if(!xc->results->len && xc->brokenAt < 0)
      return NULL;

   out = g_string_new(NULL);
   markup = want_markup ? g_string_new(NULL) : NULL;
   add_line(out, markup, FALSE, "%s", _("\nxorriso MD5 checksum tags:\n"));
   if(markup)   /* monospace, so that the ':' of the result lines align */
      g_string_append(markup, "<tt>");

   for(i=0; i<xc->results->len; i++)
   {  TagResult *r = &g_array_index(xc->results, TagResult, i);
      char *result;

      if(r->result == RES_MISSING_NEXT)
      {  bad++;
	 add_line(out, markup, TRUE, _("- %-17s: missing (expected at sector %" PRId64 ")\n"),
		  tag_name(0), r->pos);
	 continue;
      }

      switch(r->result)
      {  case RES_GOOD:       result = _("good"); good++; break;
         case RES_MISMATCH:   result = _("MD5 MISMATCH"); bad++; break;
         case RES_DAMAGED:    result = _("damaged tag"); bad++; break;
         case RES_UNCHECKED:  result = _("not checked, previous tag missing or damaged"); bad++; break;
         default:             result = _("unreadable sectors"); bad++; break;
      }

      if(r->type && r->start >= 0)
	 add_line(out, markup, r->result != RES_GOOD,
		  _("- %-17s: %s (sector %" PRId64 ", covering sectors %" PRId64 "-%" PRId64 ")\n"),
		  tag_name(r->type), result, r->pos, r->start, r->pos-1);
      else  /* range unknown, or next tag of a chain itself unreadable */
	 add_line(out, markup, r->result != RES_GOOD,
		  _("- %-17s: %s (sector %" PRId64 ")\n"),
		  tag_name(r->type), result, r->pos);
   }

   if(xc->brokenAt >= 0)
   {  bad++;
      add_line(out, markup, TRUE,
	       _("* tag check aborted: internal error, got sector %" PRId64 " instead of %" PRId64 "\n"),
	       xc->brokenAt, xc->nextLba);
   }

   if(!bad)
      add_line(out, markup, FALSE, "%s", _("- all xorriso checksums are good.\n"));

   PrintLog("%s", out->str);

   ReportNumber("xorriso_tags", "good", good);
   ReportNumber("xorriso_tags", "bad", bad);

   /* Bad tags alone can not be repaired: when the ecc data finds the image
      intact, it was created from the defective image (or the tags themselves
      are damaged). Recovery can only be attempted if the ecc data finds
      defects too. */

   if(bad)
      SetExitCode(EXIT_FAILURE, repairable ? EXIT_CODE_DEFECTS : EXIT_CODE_INCOMPLETE);

   if(markup)
   {  if(markup->str[markup->len-1] == '\n')
	 g_string_truncate(markup, markup->len-1);
      g_string_append(markup, "</tt>");
   }

   text = g_strdup(g_strstrip(markup ? markup->str : out->str));
   g_string_free(out, TRUE);
   if(markup) g_string_free(markup, TRUE);

   return text;
}

/* Called by the codecs at the end of their verify function, also after
   errors or interruption (xc may be NULL then). Reports the results and
   shows them in label (may be NULL). xc remains owned by the caller.
   repairable: the codec has found defects in the image which its ecc
   data can attempt to repair; otherwise bad tags can not be repaired. */

void XorrisoCheckReport(XorrisoCheck *xc, int repairable, GtkWidget *label)
{  char *text;

   if(!xc || Closure->stopActions)   /* image not read completely */
      return;

   text = finish(xc, repairable, label && Closure->guiMode);
   GuiSetLabelText(label, "%s", text ? text : "");
   g_free(text);
}
