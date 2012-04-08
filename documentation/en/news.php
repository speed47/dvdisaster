<?php
# dvdisaster: English homepage translation
# Copyright (C) 2004-2012 Carsten Gnörlich
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

news_item("07.04.2012", "dvdisaster 0.72.4 released", "
Updated to work with recent versions of GNU/Linux, FreeBSD and NetBSD.
", 21, "2012-04-07T01:00:00Z", "2012-04-07T01:00:00Z");

news_item("05.10.2011", "dvdisaster 0.72.3 released", "
The \"Verify\" function hangs when working on RS01 error
correction files which are larger than 2GB (the error correction
files are correctly generated though).
Volodymyr Bychkoviak discovered the problem and sent in
a bug fix.", 20, "2011-10-05T00:00:00Z", "2011-10-05T00:00:00Z");

news_item("28.07.2011", "Documentation added", "
The temporarily lost <a href=\"qa30.php\">background information</a> 
from the 0.70 documentation has been updated and made online again.
", 19, "2011-07-28T00:00:00Z", "2011-07-28T00:00:00Z");

news_item("21.11.2010", "dvdisaster 0.79.3 released", "
For GNU/Linux, the SG_IO driver is used by default
for accessing optical drives. This resolves the problem
with system freezes on parallel SCSI adapters which were
caused by the old CDROM_SEND_PACKET driver.
The RS03 codec now contains Altivec optimizations
on PowerPC platforms.
", 18, "2010-11-21T00:00:00Z", "2010-11-21T00:00:00Z");

news_item("07.11.2010", "Added Windows and Mac OS X versions for 0.72.2", "
Windows and Mac OS X versions of 0.72.2 are now available.
They were built using the updated development environment of 0.79.x.
This results in shipping with newer versions of the GTK+ libraries
and might yield slightly different visuals and behaviour.
", 17, "2010-11-07T00:00:00Z", "2010-11-07T00:00:00Z");

news_item("31.10.2010", "dvdisaster 0.72.2 released", "
This version introduces a workaround which prevents parallel SCSI
adapters from freezing under Linux. 
Improved upward compatibility with dvdisaster 0.79.x.
", 16, "2010-10-31T00:00:00Z", "2010-10-31T00:00:00Z");

news_item("28.02.2010", "dvdisaster 0.79.2 released", "
 Mac OS X binaries are available again as the Mac OS X development environment 
 has been updated for this version.
 RS03 codec development progresses,
but is still far from being finished.
", 15, "2010-02-28T00:00:00Z", "2010-02-28T00:00:00Z");

news_item("07.02.2010", "Started new development branch 0.79", "
The first version of the new development branch (0.79.1) has just
been released. This release is meant for testing new functions;
it is not recommended for doing productive work. See the
<a href=\"download40.php\">download page</a> for information on
what has been changed and how you can participate in testing.
", 14, "2010-02-07T00:00:00Z", "2010-02-07T00:00:00Z");

news_finalize();

if($news_flash == 0) 
   end_page();
?>
