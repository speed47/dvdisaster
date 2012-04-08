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
begin_page();

?>

<!-- Insert actual page content below -->

<h3 class="top">Dialogs and Buttons</h3>

This section explains commonly used dialogs and buttons:

<pre> </pre>

<table width="100%">
<tr>
<td align="center"><a href="howtosa1.php"><img src="../images/good-cd.png" alt="dvdisaster UI: Drive selection (dropdown menu)" class="noborder"></a></td>
<td>The <a href="howtosa1.php">drive selection menu</a>.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>
<tr>
<td align="center"><a href="howtosa2.php"><img src="../images/good-image.png" alt="dvdisaster UI: Image file selection (button)" class="noborder"></a></td>
<td>The <a href="howtosa2.php">image file chooser window</a>.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>
<tr>
<td align="center"><a href="howtosa3.php"><img src="../images/ecc.png" alt="dvdisaster UI: Error correction file selection (button)" class="noborder"></a></td>
<td>The <a href="howtosa3.php">error correction file chooser window</a>.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>
<tr>
<td align="center"><a href="howtosa4.php">
  <img src="images/read-icon.png" alt="dvdisaster UI: Read (button)" class="noborder">
  <img src="images/create-icon.png" alt="dvdisaster UI: Create (button)" class="noborder"><br>
  <img src="images/scan-icon.png" alt="dvdisaster UI: Scan (button)" class="noborder">
  <img src="images/fix-icon.png" alt="dvdisaster UI: Fix (button)" class="noborder"><br>
  <img src="images/compare-icon.png" alt="dvdisaster UI: Verify (button)" class="noborder">
  <img src="images/stop-icon.png" alt="dvdisaster UI: Stop (button)" class="noborder">
</a></td>
<td>The <a href="howtosa4.php">buttons for starting actions</a>.</td>
</tr>
</table>



<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
