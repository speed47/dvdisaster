<?php
# dvdisaster: English homepage translation
# Copyright (C) 2004-2009 Carsten Gnörlich
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

<h3>Dialogs and Buttons</h3>

This section explains commonly used dialogs and buttons:

<pre> </pre>

<table width="100%">
<tr>
<td align="center"><a href="howtosa1.php"><img src="../images/good-cd.png" border="0"></a></td>
<td>The <a href="howtosa1.php">drive selection menu</a>.</td>
</tr>
<tr>
<tr><td>&nbsp;</td><td></td></tr>
<td align="center"><a href="howtosa2.php"><img src="../images/good-image.png" border="0"></a></td>
<td>The <a href="howtosa2.php">image file chooser window</a>.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>
<tr>
<td align="center"><a href="howtosa3.php"><img src="../images/ecc.png" border="0"></a></td>
<td>The <a href="howtosa3.php">error correction file chooser window</a>.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>
<tr>
<td align="center"><a href="howtosa4.php">
  <img src="images/read-icon.png" border="0">
  <img src="images/create-icon.png" border="0"><br>
  <img src="images/scan-icon.png" border="0">
  <img src="images/fix-icon.png" border="0"><br>
  <img src="images/compare-icon.png" border="0">
  <img src="images/stop-icon.png" border="0">
</a></td>
<td>The <a href="howtosa4.php">buttons for starting actions</a>.</td>
</tr>
</table>



<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
