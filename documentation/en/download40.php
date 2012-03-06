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

<h3>Alpha (developer) versions</h3>

<b>Help us testing!</b> This page contains experimental dvdisaster versions
which are created on the way to the next stable release.<p>

<b>A word of caution:</b> Alpha versions are not thoroughly tested. They
may contain more errors than a stable version and should not be used
to process important data.<p>

If in doubt please continue using the <a href="download.php">stable version 0.72</a>
and wait for the release of version 0.74.

<hr>

<h3>Downloads</h3>

Please visit the <a href="http://dvdisaster.net/en/download40.php">online version of these pages</a> for currently available alpha versions. 

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
