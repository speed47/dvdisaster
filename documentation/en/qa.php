<?php
# dvdisaster: English homepage translation
# Copyright (C) 2004-2010 Carsten Gnörlich
#
# UTF-8 trigger: äöüß 
#
# Include our PHP sub routines, then call begin_page()
# to start the HTML page, insert the header, 
# navigation and news if appropriate.

require("../include/dvdisaster.php");
begin_page();
?>

<!--- Insert actual page content below --->

<h3><a name="top">General questions and answers</a></h3>

<a href="#pronounce">1.1 How is "dvdisaster" pronounced?</a><p>
<a href="#pipo">1.2 What are quality scans and why don't you support more?</a><p>
<a href="#compat">1.3 Is dvdisaster compatible with future releases?</a><p>

<hr><p>

<b><a name="pronounce">1.1 How is "dvdisaster" pronounced?</a></b><p>
Since the word stems from the english language, simply spell "dv" before
saying "disaster". Perhaps "dee-vee-disaster" is a suitable
phonetic circumscription.
<div align=right><a href="#top">&uarr;</a></div>

<b><a name="pipo">1.2 What are quality scans and why don't you support more?</a></b><p>
Optical media have a built-in error correction which is similar to 
the method used in dvdisaster. 
Some drives can report the number of errors corrected
while reading a medium. This provides a rough estimate 
of the writing and media qualities.<p>

Since dvdisaster is free software, it can only include code and information
which can be redistributed freely. This is currently true 
for C2 <a href="howtos10.php">scanning</a> of CD media, 
which is officially standardized and has free documentation available.<p>

On the other hand, DVD quality scans ("PI/PO scans") are not standardized. 
Those drive vendors who support it are using proprietary programming
interfaces. The respective specifications seem not to be available 
for use in free software. So we must patiently wait until manufacturers 
understand that having free software available for a drive 
will sell more drives. <p>

<div align=right><a href="#top">&uarr;</a></div>

<b><a name="compat">1.3 Is dvdisaster compatible with future releases?</a></b><p>
Yes, dvdisaster files are intended for an archival time of many years.
When upgrading to a newer version of dvdisaster you can continue using
images and error correction data created from previous versions.
There is <i>no</i> need to recreate them again.
<div align=right><a href="#top">&uarr;</a></div><p>


<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
