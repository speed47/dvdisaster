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
require("../include/screenshot.php");
begin_page();

howto_headline("Recovering media images", "Advanced settings", "images/create-icon.png");
?>

The first attempt of <a href="howtos42.php">reading the defective medium</a> 
will usually provide enough data for the error correction. If it did not,
try the following:<p>

<?php begin_screen_shot("Estimating the chance of recovery","adaptive-failure.png"); ?>
<b>Estimating the chance of recovery</b><p>
Examine the output of the reading process. Under the "Sectors processed" section
you will find the actual percentage of readable sectors and how many percent will
be needed for a full recovery. Using the difference between the two values
(85.6% - 81.3% = 4.3% in the example) you can estimate the likelyhood of being able
to collect enough sectors for a successful recovery:<p>
<?php end_screen_shot(); ?>

<table cellspacing="0" cellpadding="10px">
<tr bgcolor="#c0ffc0">
<td width="10%" align="center" valign="top">&lt; 5%</td>
<td>Chances are good that you will get enough data using more reading attempts.
</td></tr>
<tr bgcolor="#ffffc0">
<td width="10%" align="center" valign="top">5%-10%</td>
<td> If you have several drives with different reading characteristics 
you may get the required data by being persistent and patient.
</td></tr>
<tr bgcolor="#ffe0c0">
<td width="10%" align="center" valign="top">10%-20%</td>
<td> You are in trouble. If the missing sectors do not drop significantly
below 10% during the next 2-3 reading attempts the medium is probably unrecoverable. 
</td></tr>
<tr bgcolor="#ffc0c0">
<td width="10%" align="center" valign="top">&gt; 20%</td>
<td>Too much data loss; you can write this medium off as unrecoverable.
To prevent this from happening again, use error correction data with
higher redundancies and shorten the intervals for defect scanning.
</td></tr>
</table><p>

Try the following settings one by one in further read attempts.
Please perform a complete reading pass for each setting so that you 
learn how it affects the outcome (sometimes the results also differ
depending on the drive used for reading). When you have gone through the list
you may combine them into more powerful configurations.

<hr>

<?php begin_screen_shot("Perform another reading attempt","fix-prefs-read-attempts1.png"); ?>
<b>Perform another reading attempt</b><p>
Do not alter any values except for setting a smaller value for terminating the
reading process. Recommended values are: 32 for BD, 16 for DVD and 0 for CD (use
the slider marked green). Perform another reading attempt using this setting.
You can repeatedly read the medium as long as any pass
provides a significant number of new sectors.<p>
<b>Hint:</b> Let the drive cool down between the reading passes. Eject and load
the medium before each pass; sometimes the medium comes to rest in a better
position and the number of readable sectors improves.
<p>
<?php end_screen_shot(); ?>

<hr>

<b>Complete the image using different drives</b><p>
Perform additional reading attempts using different drives. Transfer the
image to other computers to see if their drives can contribute more readable
sectors.</b><p>

<hr>

<?php begin_screen_shot("Increase the number of reading attempts","fix-prefs-read-attempts2.png"); ?>
<b>Increase the number of reading attempts</b><p>
<i>For all media types (CD, DVD, BD):</i><p>
Set the number of reading attempts per sector to a minimum of 5 and a maximum of 9
(green markings).<p>
<i>Only for CD media:</i><p>
Some drives are capable of partially reading defective sectors on CD media.
Activate the "Raw sector caching" option and specify a directory where
fragments of defective sectors should be stored (yellow markings).
If enough fragments of a defective sector have been collected it may be possible
to fully reconstruct it from that information.
<?php end_screen_shot(); ?>


<?php begin_screen_shot("Multiple reading attempts","fix-reread-dvd.png"); ?>

<i>Examining results of multiple reading attempts (CD, DVD, BD):</i><br>
Not all drives show an improvement after increasing the number of reading attempts.
Watch for messages of the form "Sector ..., try x: success" (highlighted in yellow).
These indicate that
the drive could read a sector after several reading attempts. If you never see such
messages, increasing the number of reading attempts does not pay off for the
respective drive.
<?php end_screen_shot(); ?>

<a name="21h"></a>
<i>Examining partial reading of defective CD sectors:</i><br>
When the whole medium has been processed, look into the directory you entered
above (/var/tmp/raw in the example). If no raw files have been created
the drive may not support the required reading mode.
However if you have several drives which do create raw files, then let them all
work in the same raw file directory. Collecting raw sector fragments from
different drives hightens the chance of reconstructing the defective sectors.
<p>

<?php begin_screen_shot("Use different raw reading mode","fix-prefs-drive2.png"); ?>
<i>Use a different raw reading mode for CD media:</i><br>
Using the preset "20h" raw reading mode might not work on some drives.
Perform another reading attempt using raw reading mode "21h" (see the screenshot).
Check again whether some raw files have been created.
<?php end_screen_shot(); ?>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
