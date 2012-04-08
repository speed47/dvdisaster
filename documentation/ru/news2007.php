<?php
# dvdisaster: Russian homepage translation
# Copyright (C) 2007-2012 Igor Gorbounov
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

news_item("28.10.2007", "Начата новая документация", "
   В настоящее время документация на dvdisaster перерабатывается для предстоящего
   выхода V0.72. Наберитесь терпения; возможно, новая документация будет более
   полезной, чем старая, но нам требуется несколько недель, чтобы все охватить.
", 1, "2007-10-28T00:00:00Z", "2007-10-28T00:00:00Z");

if($news_flash == 0) 
   end_page();
?>
