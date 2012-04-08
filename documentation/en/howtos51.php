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

howto_headline("Getting information on images and error correction data", "Showing it", "images/compare-icon.png");
?>

<!-- Insert actual page content below -->

The are no settings for this function; however you need an image file
and optionally the
<a href="howtos20.php">error correction file</a> belonging to it.

<hr>

<table>
<tr>
<td class="w200x" align="center">
<a href="howtosa2.php">
<img src="../images/select-image.png" alt="dvdisaster UI: Image file selection (input field and button)" class="noborder">
<br><img src="../images/down-arrow.png" alt="Icon: Arrow down" class="noborder"></a></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Enter the file name of the ISO image</b>
for which you want to get information. The image must already be present
on hard disk; otherwise use the "Read" function to get it from a medium. 
</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center">
<a href="howtosa3.php">
<img src="../images/select-ecc.png" alt="dvdisaster UI: Error correction file selection (input field and button)" class="noborder">
<br><img src="../images/down-arrow.png" alt="Icon: Arrow down" class="noborder"></a></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt">
<b>Enter the name of the error correction file</b>
which belongs to this medium. Leave this entry blank when the image
has been
<a href="howtos30.php">augmented with error correction data</a>.
</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center">
<a href="howtosa4.php">
<img src="images/compare-icon.png" alt="dvdisaster UI: Verify (button)" class="noborder">
<br><img src="../images/down-arrow.png" alt="Icon: Arrow down" class="noborder"></a></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Start the evaluation</b> by clicking on the
"Verify" button.</td>
</tr>
</table>

<?php begin_howto_shot("Showing information.","compat-okay-rs01.png", ""); ?>
<b>Watch the verification progress.</b>
In order to display all information the image and error correction files
must be fully read. 
<?php end_howto_shot(); ?>

<hr>

<a name="examine">Further information on interpreting the results:</a><p>

<ul>
<li><a href="howtos52.php">Results for error correction files explained</a><p></li>
<li><a href="howtos53.php">Results for augmented images explained</a><p></li>
<li><a href="howtos59.php">Examples</a><p></li>
</ul>


<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
