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

<h3>dvdisaster as a complement to quality scans</h3>

<a href="qa.php#pipo">Quality scans</a>, e.g. C2 error or PI/PO scans are a valuable 
tool for testing the results of the media writing process.<p>

But quality scans are <b>not</b> a reliable means of <b>predicting 
the lifetime</b> of optical media. <br>
Consider we are looking for the right time
to copy a worn-out medium onto a new one:

<ul>
<li>Too early: Copying media because of a bad quality scan is cost-ineffective.
Sometimes such media remain readable much longer than expected.<p></li>
<li>Too late: When the quality scan reveals unreadable sectors some
data has already been lost.<p></li>
<li>Right before the medium fails: The ideal case, but how to tell?
</ul>

However, we could do it the dvdisaster way:

<ul>
<li><a href="howtos20.php">Create error correction data</a> for the medium.<p></li>
<li><a href="howtos10.php">Scan the medium</a> regularly. Use it until the first read errors occur.<p></li>
<li><a href="howtos40.php">Recover</a> the read errors <a href="howtos40.php">using the error correction data</a>.
Write the recovered image to a new medium.</li>
</ul>

<p>
<a href="index30.php">Pro and con of dvdisaster at a glance...</a>


<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
