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

howto_headline("Creating error correction files", "Basic settings", "images/create-icon.png");
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

<hr>

<a name="read"><b>Settings for reading the image from the medium</b></a><p>

<table width="100%" cellspacing="5">
<tr>
<td><img src="../images/good-image.png"></td>
<td>If you already have an ISO image available you can skip
the next two tabs and proceed with the <a href="#ecc">error correction settings</a>.
But make sure that you really have an ISO type image; other formats like ".nrg"
do not produce usable error correction data.
</td>
</tr>
</table><p>

<?php begin_screen_shot("\"Image\" tab.","create-prefs-image.png"); ?>
<b>"Image" tab.</b> Make sure that the image size is determined
using the "ISO/UDF" setting and that the linear reading strategy is selected.
The required selections are marked green. Leave the other settings at their defaults.
<p>
<?php end_screen_shot(); ?>

<pre> </pre>

<?php begin_screen_shot("\"Drive\" tab.","create-prefs-drive.png"); ?>
<b>"Drive" tab.</b> Reading data from the drive while it is spinning up
can generate spurious error reports. Adjust the spin up time for your drive
(typically 5-10 seconds) in the field marked green to make dvdisaster wait
for the appropriate time.<p>
Leave the other settings at the shown values.<p>
<?php end_screen_shot(); ?>

<?php begin_screen_shot("\"Read attempts\".","create-prefs-read-attempts.png"); ?>
<b>"Read attempts".</b> The option "Read and analyse raw sectors" (marked green)
uses additional information provided by the drive to check the integrity of read
data. This is recommended as we are interested in creating 
error correction data from a properly read image.
On the other hand since error correction data can only be created from fully
readable media we do not need multiple reading attempts and caching of raw sectors
as shown in the screen shot.
<?php end_screen_shot(); ?>

<hr>

<a name="ecc"><b>Error correction settings</b></a><p>

<?php begin_screen_shot("\"Error correction\" tab.","create-prefs-ecc.png"); ?>
<b>"Error correction" tab.</b> First choose "Error correction file (RS01)" in
the "Storage method" list (green marking). By picking the redundancy you will
determine the maximum error correction capability: An error correction file
with x% redundancy can correct up to x% of read errors under optimal circumstances.
Since the best case is usually not encountered you should add some safety margin
to the redundancy by picking one of the following choices (see yellow markings);

<ul>
<li>The "normal" and "high" presets provide a redundancy of 14.3% and 33.5%
respectively. These two settings are very fast in creating error correction
files due to optimized program code.</li>
<li>You can freely choose the redundancy by activating the "other" item and dragging
the slider.</li>
<li>By activating the "Use at most" button you can specify the error correction
file size in MB. dvdisaster will choose a suitable redundancy so that the
error correction file will be close to but not larger than the specified size.
</li>
</ul>

The redundancy will also determine the size of the error correction file;
using x% redundancy will create an error correction file of about x% the size
of the image. Using redundancies lower than the "normal" setting (14.3%) is not
recommended as the error correction might be overloaded too quickly.
<?php end_screen_shot(); ?>

<?php begin_screen_shot("\"Files\" tab.","create-prefs-file.png"); ?>
<b>"Files" tab.</b> In this tab, leave the settings off for the moment; 
suggestions for further 
<a href="howtos25.php">optimization</a> follow later.
<?php end_screen_shot(); ?>

<pre> </pre>

<b>Not used tabs</b><p>

The "Misc" tab currently has only functions for creating
log files. This is helpful for sending in <a href="feedback.php">bug reports</a>
but should be left off during normal operation.
The "Appearance" tab allows you to adapt the output colors to your taste, 
but these have no further effects on the error correction data creation.

<pre> </pre>

<a href="howtos23.php">Creating the error correction data...</a>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
