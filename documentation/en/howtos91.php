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

<h3 class="top">Testing compatibility between error correction files and ISO images</h3>


<b>Motivation:</b> You want to write data to a medium and create an
error correction file for it. In order to save time you do the following:

<ol>
<li>You create an ISO image using your CD/DVD/BD writing software.</li>
<li>You write the image to a medium.</li>
<li>You create the error correction file from the same image.</li>
</ol>

<b>Possible incompatibility:</b> The writing software creates a medium
which does not exactly match the image. This might prevent the error
correction from recovering the medium contents when it becomes defective. 
<p>

<b>How to test compatibility:</b><p>

Please note that some steps are only sketched out here;
follow the links to the respective sections to find detailed instructions
and examples.<p>

<table>
<tr>
<td class="w200x" align="center"><img src="../images/good-image.png" alt="Icon: Complete image">
<p><img src="../images/down-fork-arrow.png" alt="Icon: Forked arrow"></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Create an ISO image of the data</b> you want
to write on the medium. If you need help on creating ISO images
please refer to the
<a href="howtos33.php?way=1">example of creating ISO images</a>.
</td>
</tr>
</table>

<table>
<tr>
<td class="w100x" align="center">
<img src="../images/good-cd.png" alt="Icon: Good medium (without read errors)" class="nobordervalignm"><p>
<img src="../images/down-arrow.png" alt="Icon: Arrow down" class="noborder">
</td>
<td class="w100x" align="center" valign="top">
<img src="../images/ecc.png" alt="Icon: Separate file with error correction data" class="nobordervalignm">
</td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Write the medium and create the error correction file.</b>
Use the just created image to 
<a href="howtos33.php?way=3#c">write the medium</a>. 
Then perform these <a href="howtos22.php#ecc">basic settings</a> and
<a href="howtos23.php?way=2">create an error correction file</a>
from the image.
</td>
</tr>
</table>

<table>
<tr>
<td class="w100x" align="center">
<img src="../images/good-image2.png" alt="Icon: Complete image from the previously written medium" class="noborder"><p>
<img src="../images/down-arrow.png" alt="Icon: Arrow down" class="noborder">
</td>
<td class="w100x" align="center"> </td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Create a <i>second</i> image from the <i>written</i> 
medium.
</b> Use these <a href="howtos22.php#read">settings</a>
and read the medium as described 
in <a href="howtos23.php?way=1">creating an image</a> 
for making an error correction file. However you can stop the walk-through
when the reading is finished as we do not need to create the error 
correction file again.
</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center">
<a href="howtosa2.php">
<img src="../images/select-image.png" alt="dvdisaster UI: Image file selection (input field and button)" class="noborder">
<br><img src="../images/down-arrow.png" alt="Icon: Arrow down" class="noborder"></a></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Enter the name of the <i>second</i> ISO image</b>
which you have just read from the medium. Please note that the following
test is useless when working with the image which was initially created
using the CD/DVD/BD authoring software.
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
in case it is not already present from the previous actions.
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
<td class="valignt"><b>Start the evaluation</b> by clicking
on the "Verify" button.</td>
</tr>
</table>

<?php begin_howto_shot("Show information.","compat-okay-rs01.png", ""); ?>
<b>Look at the verification results.</b>
If you get the green messages "Good image." and "Good error correction file."
your authoring software and dvdisaster are compatible. You can continue
creating the error correction files directly from the ISO images produced
by the authoring software.
<?php end_howto_shot(); ?>

<hr>

<a name="err"> </a>
<b>Possible error causes and remedy:</b><p>

<?php begin_howto_shot("Wrong image size.","compat-150-rs01.png", "down-arrow.png"); ?>
<b>Typical problem: wrong image size.</b>
The verification may find out that the image is larger as expected.
Typically the difference is 150 or 300 sectors for CD media and
1-15 sectors for DVD/BD media. These might simply be zero padding sectors
appended by the writing software. To find out if this really is the case
do the following:
<?php end_howto_shot(); ?>

<table>
<tr>
<td class="w200x" align="center">
<img src="images/fix-icon.png" alt="dvdisaster UI: Fix (button)" class="noborder">
<p><img src="../images/down-arrow.png" alt="Icon: Arrow down" class="noborder"></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Start a recovery process.</b>
</td>
</tr>
</table>

<?php begin_howto_shot("Truncate the image.","compat-dialog-rs01.png", "down-arrow.png"); ?>
<b>Confirm the dialog.</b>
A dialog will appear asking you if it is okay to remove the superflous
sectors from the image. Answer "OK".
<?php end_howto_shot(); ?>


<table>
<tr>
<td class="w200x" align="center">
<img src="images/stop-icon.png" alt="dvdisaster UI: Stop (button)" class="noborder">
<p><img src="../images/down-arrow.png" alt="Icon: Arrow down" class="noborder"></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Stop the recovery process,</b>
as after truncating the image there is nothing more to do.
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
<td class="valignt"><b>Start the verification again</b>
by clicking on the "Verify" button.</td>
</tr>
</table>

<?php begin_howto_shot("Show information.","compat-okay-rs01.png", ""); ?>
<b>Consider the new results.</b>
If you now get the green messages "Good image." and "Good error correction 
file." your problem is purely cosmetic: The writing software has indeed
added zero padding sectors while writing the medium.
<?php end_howto_shot(); ?>

<span class="red">If the problem persists after carrying out the above
steps do <i>not</i> assume that dvdisaster and the writing software
are compatible. The created error correction files will probably be
unusable.</span> <p> 
Use the following method for creating the error correction files instead:

<hr>

<pre> </pre>

<b>Alternative method avoiding incompatibilities:</b>

<ol>
<li>First write the data to the medium.</li>
<li>Use dvdisaster to create an ISO image from the written medium.</li>
<li>Use this image to create the error correction file.</li>
</ol>
This method takes more time due to the additional reading process,
but it also has the advantage of testing the newly created
medium for readability. 


<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
