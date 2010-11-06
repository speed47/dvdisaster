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

<h3>Advantages of using dvdisaster:</h3>

<ul>
<li><b>Protects</b> against aging and accidental medium damage (within certain limits).<p></li>
<li><a href="howtos10.php">Read error tests</a> run <b>faster</b> than quality scans;
up to full reading speed depending on the drive.<p></li>
<li><b>Cost-effective:</b> Media must be replaced with a new copy 
only when they are really defective.</li>
</ul>

<h3>Limitations of using dvdisaster:</h3>

You need a backup strategy and at least 15% of additional storage.

<ul>
<li>Error correction data <b>must be created before the medium fails</b>, 
preferably at the same time the medium is written.<p></li>
<li>Error correction data requires <b>additional storage space</b> either on the protected 
medium or by using additional media. 
Using the standard settings the additional
storage space amounts to 15% of the original data size
(approx. 700MB for a full 4.7GB DVD).<p></li>
<li>no guaranteed protection against data loss.</li>
</ul>

See also the collection of <a href="http://dvdisaster.net/legacy/en/background.html">background information</a>  in the old documentation
to learn more about the functioning of dvdisaster.


<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
