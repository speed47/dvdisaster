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

howto_headline("Scanning media for errors", "Walkthrough", "images/scan-icon.png");
?>

<!--- Insert actual page content below --->

Please make sure that dvdisaster has been configured as described in the 
<a href="howtos11.php">basic settings </a>section as some settings
might negatively affect the scanning results. Then perform the following steps:
<p>

<hr>

<table>
<tr>
<td width="200px" align="center"><img src="../images/slot-in.png">
<br><img src="../images/down-arrow.png"></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Insert the medium you want to scan into a drive</b>
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
</td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center">
<a href="howtosa3.php">
<img src="../images/select-ecc.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Select the error correction file for this medium</b>
if you have one available. Ecc data from RS02 augmented media is used
automatically.</td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center">
<a href="howtosa4.php">
<img src="images/scan-icon.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top">Start the scan by <b>clicking the "Scan" button</b>.</td>
</tr>
</table>

<?php begin_howto_shot("Scanning the medium.","good-cd.png", ""); ?>
<b>Watch the scanning progress.</b>
Do not perform any other actions on your computer while the scan is running.
Opening or working with other programs as well as moving other windows around
might affect the scanning results.
<?php end_howto_shot(); ?>
<p>

<hr>

<a href="howtos13.php">Interpreting the results...</a>


<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
