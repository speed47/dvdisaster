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

$way=$_GET["way"];
switch($way)
{  case 1: $action="from a medium"; break;
   case 2: $action="from an ISO image"; break;
   default: $action="Walkthrough"; break;
}

howto_headline("Creating error correction files", $action, "images/create-icon.png");
?>

<!--- Insert actual page content below --->

Please make sure that dvdisaster has been configured as described in the
<a href="howtos22.php">basic settings</a> section as some selections might
create sub optimal error correction data.
<p>

The next steps depend on the source for the error correction data. Select between
these two ways:<p>

<table width="100%" cellspacing="5">
<tr>

<?php
$expand=$_GET["expand"];
if($expand=="") $expand=0;
echo "<td><a href=\"howtos23.php?way=1&expand=$expand\"><img src=\"../images/good-cd.png\" border=\"0\"></a></td>\n";
echo "<td><a href=\"howtos23.php?way=1&expand=$expand\">Create error correction file from a CD/DVD/BD medium</a></td>\n";
echo "<td><a href=\"howtos23.php?way=2&expand=$expand\"><img src=\"../images/good-image.png\" border=\"0\"></a></td>\n";
echo "<td><a href=\"howtos23.php?way=2&expand=$expand\">Create error correction file from an ISO image</a></td>\n";
?>

</tr>
</table>

<?php
if($way==1){
?>
<hr><p>

<table>
<tr>
<td width="200px" align="center"><img src="../images/slot-in.png">
<br><img src="../images/down-arrow.png"></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Insert the medium you want to read into a drive</b>
which is directly connected to your computer. 
You can not use network drives, software drives and drives inside virtual machines.
</td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center"><img src="../images/winbrowser.png">
<br><img src="../images/down-arrow.png"></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Close any windows</b> which may be opened by your operating system 
for viewing or performing the medium contents. 
Wait until the drive has recognized the medium and the medium has spun down.
</td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center"><a href="howtosa1.php">
<img src="../images/select-drive.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Select the drive containing the medium</b>
in dvdisasters drop down menu.
</tr>
</table>

<table>
<tr>
<td width="200px" align="center">
<a href="howtosa2.php">
<img src="../images/select-image.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Select a directory and file name</b> 
for storing the ISO image.</td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center">
<a href="howtosa4.php">
<img src="images/read-icon.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Create the ISO image</b> from the medium by clicking
the "Read" button.</td>
</tr>
</table>

<?php begin_howto_shot("Reading the image.","watch-read1.png", "down-arrow.png"); ?>
<b>Watch the reading progress.</b>
Wait until the medium has been completely read. If the medium turns out
to contain defective sectors it will not be possible to create error correction data.
<?php end_howto_shot(); 
 }  /* end of if($way == 1) */

if($way == 2) {
?>
<hr><p>

<table>
<tr>
<td width="200px" align="center">
<a href="howtosa2.php">
<img src="../images/select-image.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Select the directory and name of the ISO image</b>
for which you want to create the error correction data.
(It is assumed that the ISO image has been created by some other means,
e.g. by using your CD/DVD/BD authoring software.)</td>
</tr>
</table>
<?php
}

if($way != 0) {
?>
<table>
<tr>
<td width="200px" align="center">
<a href="howtosa3.php">
<img src="../images/select-ecc.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Select a directory and name</b> 
for storing the error correction file.</td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center">
<a href="howtosa4.php">
<img src="images/create-icon.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Create the error correction file</b>
by clicking the "Create" button.</td>
</tr>
</table>

<?php begin_howto_shot("Creating the error correction file.","watch-create.png", "down-fork-arrow.png"); ?>
<b>Wait until the creation process finishes.</b>
This may take a while depending on the image size and the selected redundancy.
For example creating an error correction file using the "normal" redundancy setting
will take about 5 minutes for a 4GB sized DVD image and using standard hardware.
<?php end_howto_shot(); ?>

<table>
<tr>
<td width="200px"align="center">
<img src="../images/old-image.png" border="0" align="center">
&nbsp;&nbsp;&nbsp;
<img src="../images/ecc.png" border="0" align="center"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Wrapping up.</b> You can delete the image file now. However you 
must keep the error correction file and, even more important, protect it from being
damaged. Refer to the next page for some suggestions about
<a href="howtos24.php">error correction file archival</a>.
</td>
</tr>
</table>

<p>
<a href="howtos24.php">Error correction file archival...</a>
<?php
} /* end of if($way != 0) */
?>
<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
