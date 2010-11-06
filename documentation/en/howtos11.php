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

howto_headline("Scanning media for errors", "Basic settings", "images/scan-icon.png");
?>

<!--- Insert actual page content below --->

<?php begin_screen_shot("Opening the configuration dialog.","global-prefs-invoke.png"); ?>
<table><tr><td valign="top"><img src="../images/prefs-icon.png" valign="bottom"></td>
<td>The following tabs are found in the configuration dialog.
Open the dialog by selecting the symbol marked green in the screen shot
(click the image to expand it). The symbol may look different
due to the symbol theme you are using.</td>
</tr></table>
<?php end_screen_shot(); ?>

<?php begin_screen_shot("\"Image\" tab.","scan-prefs-image.png"); ?>
<b>"Image" tab.</b> Selecting the proper method for determining the image size is important.
The setting "ISO/UDF" (marked green) usually works in any situation. 
Pick the selection "ECC/RS02" (marked red) only if you are sure that the medium contains 
RS02 error correction data. Using the error correction (ecc) data 
will improve scanning results,
but searching for non existing ecc data will cost several minutes. <p>
Adjust the remaining settings as shown in the screen shot.<p>
<?php end_screen_shot(); ?>

<pre> </pre>

<?php begin_screen_shot("\"Drive\" tab.","scan-prefs-drive.png"); ?>
<b>"Drive" tab.</b> Reading data from the drive while it is spinning up
can generate spurious error reports. Adjust the spin up time for your drive
(typically 5-10 seconds) in the field marked green to make dvdisaster wait
for the appropriate time.<p>
Leave the other settings at the values shown; you can <a href="howtos14.php">optimize</a> them later.<p>
<?php end_screen_shot(); ?>

<pre> </pre>

<?php begin_screen_shot("\"Read attempts\" tab.","scan-prefs-read-attempts.png"); ?>
<b>"Read attempts" tab.</b> Adjust the reading attempts settings as shown here.
Using larger values causes unnecessary reading activity but will not improve
the scan.
The option "Read and analyse raw sectors" (first green marking) uses C2 analysis and possibly
more raw data reported by the drive for a better assessment of CD media quality. This setting does
nothing for DVD and BD media, but it is safe to remain activated unless it causes problems
with your drive reading CDs.
After a read error no less than 16 sectors should be skipped (second green marking);
when scanning badly damaged media this setting can be 
<a href="howtos14.php">optimized using larger values</a>.<br>
Performing multiple read attempts is not recommended during a scan; set the number of retries
to 1 in the three places marked in orange. Collecting raw sectors should also be off
during the scan.<p>
<?php end_screen_shot(); ?>

<pre> </pre>

<?php begin_screen_shot("\"Misc\" tab.","general-prefs-misc.png"); ?>
<b>"Misc" tab.</b> Currently this tab only has functions for creating
log files. This is helpful for sending in <a href="feedback.php">bug reports</a>
but should be left off during normal operation.
<?php end_screen_shot(); ?>

<pre> </pre>

<b>Not used tabs</b><p>

The "Error correction" and "Files" tabs have no influence on scanning media.
The "Appearance" tab allows you to adapt the output colors to your taste, 
but these have no further effects on the scanning process.

<pre> </pre>


<a href="howtos12.php">Performing the scan...</a>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
