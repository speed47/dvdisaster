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

news_item("01.10.2009", "Assessment of potential vulnerabilities in the windows version
of the GTK library", "
The Windows versions of dvdisaster 0.70.x/0.72.x are shipped with an outdated
GTK library containing vulnerabilities in its image processing routines.
To exploit the vulnerability, manipulated images need to be loaded from
an external source. Since dvdisaster does not contain/use such functions,
these vulnerabilities are not considered to be a threat.<p>

It is not recommended to replace GTK in the 0.70.x/0.72.x versions of
dvdisaster as some interfaces have been changed in newer GTK versions.
Replacing GTK will likely cause severe malfunction.<p>

The windows version of dvdisaster 0.73.1 will have updated interfaces
and will be shipped with a current version of GTK.<p>

Many thanks to all users who brought this issue to my attention.
", 13, "2009-10-01T00:00:00Z", "2009-10-01T00:00:00Z");

news_item("10.08.2009", "Project represention on SourceForge may be outdated", "
Due to the recent functionality changes on SourceForge we are unable
to upload the News, Downloads and CVS contents in time. Please refer
to the respective sections on this site (http://dvdisaster.net) instead.
", 12, "2009-08-10T00:00:00Z", "2009-08-10T00:00:00Z");

news_item("08.08.2009", "dvdisaster 0.72.1 released", "
Pablo Almeida provided a Portuguese translation for the screen text.
Added workaround to avoid Win XP freezing on certain CD-RW/drive pairs.<p>
<i>Update: The workaround is not complete. If the problem persists please
try version 0.79.x.</i>
", 11, "2009-08-08T00:00:00Z", "2010-02-06T00:00:00Z");

news_item("04.07.2009", "dvdisaster 0.72 released", "
This is the first stable version of the 0.72 branch.
Igor Gorbounov has completed the russian online documentation.
Some minor bugs from the first release candidate have been removed.<p>
", 10, "2009-07-04T00:00:00Z", "2009-07-04T00:00:00Z");

news_finalize();

if($news_flash == 0) 
   end_page();
?>
