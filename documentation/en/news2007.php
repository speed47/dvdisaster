<?php
# dvdisaster: English homepage translation
# Copyright (C) 2004-2009 Carsten Gnörlich
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

news_headline("News archive from 2007");

news_item("28.10.2007", "New documentation started", "
   The dvdisaster documentation is currently being reworked for the upcoming
   V0.72 release. Please be patient; the new documentation will hopefully be more
   useful than the old one, but we will need a few weeks to fill in all parts.
"); # end of news_item

if($news_flash == 0) 
   end_page();
?>
