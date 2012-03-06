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
require("../include/screenshot.php");

begin_page();

howto_headline("Scanning media for errors", "Advanced settings", "images/scan-icon.png");
?>

<!--- Insert actual page content below --->

<?php begin_screen_shot("\"Drive\" tab.","scan-prefs-drive-adv.png"); ?>
<b>Ignoring fatal errors.</b>
dvdisaster will normally abort the scan when the drive reports
a fatal internal error like mechanical problems. The intention is
to avoid damaging the drive. However some drives will erroneously report
such problems when they get confused by damaged media. If you have such
a drive you can use this option to force the scan to continue.<p>
<b>Eject medium after sucessful read.</b>
dvdisaster tries to eject the medium after a successful scan if this
option is activated. However ejecting the medium might be prohibited by
the operating system so this is not guaranteed to work. For example if
upon media insertion a window is opened for performing the contents it
may not be possible to automatically eject the medium.
<p>
<?php end_screen_shot(); ?>

<pre> </pre>

<?php begin_screen_shot("\"Read attempts\" tab.","scan-prefs-read-attempts-adv.png"); ?>
<b>Skipping sectors after a read error.</b>
Attempts for reading defective sectors cost a lot of time.
Since it is likely to encounter another defective sector after hitting a 
read error, skipping a few sectors after a read error saves time
and reduces wear on the drive.
If you only want a quick overview of a damaged medium setting this value
to 1024 might help. But keep in mind that all skipped sectors are treated
as being defective so the number of reported errors becomes higher and less
accurate.<p>
<?php end_screen_shot(); ?>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
