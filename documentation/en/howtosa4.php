<?php
# dvdisaster: English homepage translation
# Copyright (C) 2004-2012 Carsten Gnörlich
#
# UTF-8 trigger: äöüß 
#
# Include our PHP sub routines, then call begin_page()
# to start the HTML page, insert the header, 
# navigation and news if appropriate.

require("../include/dvdisaster.php");
require("../include/screenshot.php");
begin_page();

?>

<!-- Insert actual page content below -->

<h3 class="top">Starting actions</h3>

<?php begin_screen_shot("Starting actions","action-buttons.png"); ?>
To start an action in dvdisaster, click on one of the buttons marked green:<p>

<table>
<tr>
<td class="valignt"><img src="images/read-icon.png" alt="dvdisaster UI: Read (button)"> &nbsp;</td>
<td><b>Reading medium contents into an image file</b> to:
<ul>
<li>read in <a href="howtos42.php#a">a defective medium</a> for a subsequent recovery.
<li>read in <a href="howtos23.php?way=1&expand=0">an error-free medium</a> for creating an error
correction file.</ul></td>
</tr>

<tr>
<td class="valignt"><img src="images/create-icon.png" alt="dvdisaster UI: Create (button)"> &nbsp;</td>
<td><b><a href="howtos20.php">Creating an error correction file</a></b><br>
(only possible from defect-free media!)</td>
</tr>

<tr>
<td class="valignt"><img src="images/scan-icon.png" alt="dvdisaster UI: Scan (button)"> &nbsp;</td>
<td><b><a href="howtos10.php">Scanning a medium for read errors.</a></b>
</td>
</tr>

<tr>
<td class="valignt"><img src="images/fix-icon.png" alt="dvdisaster UI: Fix (button)"> &nbsp;</td>
<td><b><a href="howtos40.php">Recover the image of a defective medium</a></b><br>
provided that 
 <a href="howtos20.php">error correction data</a> is available.
</td>
</tr>

<tr>
<td class="valignt"><img src="images/compare-icon.png" alt="dvdisaster UI: Verify (button)"> &nbsp;</td>
<td>Display <a href="howtos50.php">information on images and error correction data</a>.
</td>
</tr>
</table><p>

<b>Other buttons related to the above actions:</b>

<table>
<tr>
<td class="valignt"><img src="images/log-icon.png" alt="dvdisaster UI: View log (button)"> &nbsp;</td>
<td><b>View log file of running action</b> (marked yellow).<br>
See also: <a href="feedback.php#log">Log file creation</a>.
</td>
</tr>

<tr>
<td class="valignt"><img src="images/stop-icon.png" alt="dvdisaster UI: Stop (button)"> &nbsp;</td>
<td><b>Aborting the running action</b> (marked red).<br>
Some actions may take some time to abort; especially when this button
is hit while reading a defective sector.
</td>
</tr>
</table>

<?php end_screen_shot(); ?>


<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
