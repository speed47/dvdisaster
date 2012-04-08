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

news_headline("Archiv novinek za rok 2007");

news_item("28.10.2007", "Začala práce na nové dokumentaci", "Začala práce na přepsání dokumentace pro nadcházející verzi 0.72. Buďte prosím trpěliví, nová dokumentace bude doufám užitečnější než ta současná, ale její vytvoření bude nějaký ten týden trvat.", 1, "2007-10-28T00:00:00Z", "2007-10-28T00:00:00Z");

if($news_flash == 0) 
   end_page();
?>
