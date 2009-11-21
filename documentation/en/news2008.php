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

news_headline("News archive from 2008");

news_item("30.03.2008", "dvdisaster 0.71.26 released", "
  This version contains a new dialog providing information about inserted 
  media (menu Tools/Medium info);
  during this course detection of media types (CD/DVD/BD) was changed. 
  Please report if this breaks compatibility with formerly working drives.
  Improved support for BD media.
");

news_item("05.03.2008", "Problem with previous Windows release fixed (0.70.6 / 0.71.25)", "
  Rolled back support for localized file names in version 0.70.6
  as it broke large file support under Windows. A new handler
  for localized file names will now be tested in the experimental version
  0.71.25 first.
");

news_item("03.03.2008", "Oops - images &gt;2GB fail in 0.70.5 and 0.71.24 under Windows", "
   The fix for localized file names caused problems when processing
   images &gt; 2GB under Windows in the just released versions 0.70.5 and 0.71.24.
   Please stay tuned for fixed versions.
");

news_item("24.02.2008", "dvdisaster 0.70.5 / 0.71.24 fix problems with newer Linux versions", "
   A problem with newer Linux kernels was fixed which would lead
   to a frozen system under some circumstances. Please upgrade on systems 
   running kernels 2.6.17 and above; maybe earlier kernels are also affected.<p> 

   The release of dvdisaster 0.71.24 also marks the start of
   an online documentation rewrite, including a Russian translation made
   by Igor Gorbounov.

   <i>Currently, the english documentation is far from being complete.
   Please bear with us; we'll catch up soon.</i>

"); # end of news_item

if($news_flash == 0) 
   end_page();
?>
