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

<h3 class="top">Checking compatibility for images augmented with error correction data</h3>

<b>Motivation:</b> dvdisaster can put error correction data
<a href="howtos30.php">together with the user data on the medium</a>.
The error correction data is appended to the ISO image in a way invisible
to most applications in order to not interfere with them.<p>

<b>Possible incompatibility:</b> The CD/DVD/BD writing software may also
be unable to see the error correction data. While being unlikely it is
possible that the writing software will truncate or damage the error 
correction data while creating the medium. In that case the error correction
will not work.<p>


<b>How to test compatibility:</b><p>

Please note that some steps are only sketched out here;
follow the links to the respective sections to find detailed instructions
and examples.<p>

<table>
<tr>
<td class="w200x" align="center"><img src="../images/good-cd-ecc.png" alt="Icon: Medium containing error correction data">
<p><img src="../images/down-arrow.png" alt="Icon: Arrow down"></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>First create a medium which was augmented with
error correction data</b>. Do not forget
to use the proper <a href="howtos32.php">settings</a> and follow the 
<a href="howtos33.php">step by step</a> instructions. <br>
Do not use rewriteable DVD or BD media as they may influence the test
under some circumstances (see <a href="qa20.php#rw">item 3.4 in the
questions and answers</a>).
</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center">
<img src="../images/good-image2.png" alt="Icon: Complete image from the previously written medium" class="noborder"><p>
<img src="../images/down-arrow.png" alt="Icon: Arrow down" class="noborder">
</td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Create a <i>second</i> image from the <i>written</i> 
medium</b>.
Use the same <a href="howtos22.php">settings</a> and steps as in
<a href="howtos23.php?way=1">reading a medium</a> for creating an error
correction file; however you can stop after the reading has finished as
we do not need the error correction file.
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
using the CD/DVD/BD authoring software and augmented with dvdisaster.
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

<?php begin_howto_shot("Show information.","compat-okay-rs02.png", ""); ?>
<b>Look at the verification results.</b>
If you get the green messages "Good image." and "Good error correction data."
your authoring software and dvdisaster are compatible with respect to
the augmented images.
<?php end_howto_shot(); ?>

<hr>

<a name="err"> </a>
<b>Possible error causes and remedy:</b><p>

<?php begin_howto_shot("Wrong image size.","compat-150-rs02.png", "down-arrow.png"); ?>
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

<?php begin_howto_shot("Truncating the image.","compat-dialog-rs02.png", "down-arrow.png"); ?>
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

<?php begin_howto_shot("Show Information.","compat-okay-rs02.png", ""); ?>
<b>Consider the new results.</b>
If you now get the green messages "Good image." and "Good error correction 
data." your problem is purely cosmetic: The writing software has indeed
added zero padding sectors while writing the medium.
<?php end_howto_shot(); ?>

<span class="red">If the problem persists after carrying out the above
steps you can <i>not</i> use the CD/DVD/BD writing software
for creating media from augmented images.
Perform the test again using a software from a different vendor.
</span> <p> 

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
