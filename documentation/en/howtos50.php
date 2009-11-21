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
require("../include/footnote.php");
begin_page();
howto_headline("Getting information on images and error correction data", "Overview", "images/compare-icon.png");
?>

<!--- Insert actual page content below --->

<table width="100%" cellspacing="5">
<tr valign="top">
<td width="20%"><b>Task</b></td>
<td>
Shows information on types and states of images and error correction files.
</td>
</tr>
<tr><td> <pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>Required:</b><p></td>
</tr>
<tr>
 <td width="150px" align="right">
   <img src="../images/good-image.png" align="top">
   <img src="../images/ecc.png">
 </td>
<td>
An image file and optionally the error correction file for it.
</td>
<tr><td> <pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>What to do:<p></b></td>
</tr>

<tr>
<td></td>
<td>
1. <a href="howtos51.php">Show the information</a><p>
2. <a href="howtos51.php#examine">Interpreting the results</a>
</td>
</tr>
</table><p>

<pre>


</pre>

<a href="howtos51.php">Show the information...</a>

<!--- do not change below --->
<?php
# end_page() adds the footer line and closes the HTML properly.
end_page();
?>
