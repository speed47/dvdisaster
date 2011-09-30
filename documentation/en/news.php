<?php
# dvdisaster: English homepage translation
# Copyright (C) 2004-2010 Carsten Gnörlich
#
# UTF-8 trigger: äöüß 
#
# Include our PHP sub routines, then call begin_page()
# to start the HTML page, insert the header, 
# navigation and news if appropriate.

if($news_flash == 0) 
{  require("../include/dvdisaster.php");
   begin_page();
}

$news_counter = 0;

# The news page is different from the other pages;
# you must only use the news_*() functions below to create content.
# Do not insert plain HTML!

news_headline("dvdisaster News");

news_item("31.10.2010", "dvdisaster 0.72.2 released", "
This version introduces a workaround which prevents parallel SCSI
adapters from freezing under Linux. RS03 images from 0.79.x
versions will no longer cause spurious error messages.");

news_item("10.08.2009", "Project represention on SourceForge may be outdated", "
Due to the recent functionality changes on SourceForge we are unable
to upload the News, Downloads and CVS contents in time. Please refer
to the respective sections on this site (http://dvdisaster.net) instead.
");

news_item("08.08.2009", "dvdisaster 0.72.1 released", "
Pablo Almeida provided a Portuguese translation for the screen text.
Added workaround to avoid Win XP freezing on certain CD-RW/drive pairs.<p>
");

news_item("04.07.2009", "dvdisaster 0.72 released", "
This is the first stable version of the 0.72 branch.
Igor Gorbounov has completed the russian online documentation.
Some minor bugs from the first release candidate have been removed.<p>
");

news_item("14.04.2009", "Added: dvdisaster 0.72.rc1 for Mac OS X", "
  The native version for Mac OS X is still a bit <a href=\"download30.php#mac\">rough 
  around the edges</a> in the user interface: The underlying GTK+ library port is in an early
  development stage. But with todays update, dvdisaster is becoming usable on the Mac...
");

news_item("11.04.2009", "dvdisaster 0.72.rc1 released", "
  The first release candidate for the stable 0.72 branch is available now.
  Among the new features are Blu-Ray media support, raw reading and C2 scans
  for CD.
  See the <a href=\"download.php\">download page</a> for more information.
");

news_item("08.03.2009", "dvdisaster 0.71.28 released", "
  This version fixes \"illegal instruction\" crashes on x86 machines
  which do not support SSE2. More minor fixes towards the release
  candidate.
");

news_item("18.01.2009", "dvdisaster 0.71.27 released", "
  The documentation is finally complete. Some crashes and incompatibilities
  with uncommon media and drives have been fixed. This version includes
  the last major internal overhaul planned for the 0.71.x series, so if
  all goes well the next version will be a stable release candidate.
");

if($news_flash == 0) 
   end_page();
?>
