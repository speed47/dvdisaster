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

howto_headline("Recovering media images", "Basic settings", "images/fix-icon.png");
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

The settings shown here configure dvdisaster for reading the defective medium.
There are no dedicated settings for reconstructing the image from the error
correction data.
<pre> </pre>

<?php begin_screen_shot("\"Image\" tab.","fix-prefs-image.png"); ?>
<b>"Image" tab.</b> First choose the type of error correction data.
Pick the setting "ISO/UDF" (marked green) if you have an error correction file.
Otherwise choose "ECC/RS02" (marked blue) to process a medium which has been
augmented with error correction data.<p>
The adaptive reading strategy uses information from the error correction data to
make the reading process as efficient as possible. Activate it using the button
marked yellow.<p>
Leave the remaining settings at the values shown in the screen shot.<p>
<?php end_screen_shot(); ?>

<?php begin_screen_shot("\"Drive\" tab.","fix-prefs-drive.png"); ?>
<b>"Drive" tab.</b> Leave this tab at the shown default settings for the moment.
Some drives might work better using the raw reading mode "21h". See the
<a href="howtos43.php#21h">advanced settings</a> for more information.<p>
<?php end_screen_shot(); ?>

<?php begin_screen_shot("\"Reading attempts\" tab.","fix-prefs-read-attempts.png"); ?>
<b>"Reading attempts" tab.</b> The strength of the adaptive reading strategy lies in
finding the still readable sectors and avoiding the lengthy process of trying to read
defective sectors. Therefore select "raw" reading (marked green) as it will not cost additional
processing time, but reduce the number of reading attempts to the minimum values
(marked yellow). Use a moderate termination criterium of 128 unreadable sectors
(marked blue) for the first reading attempt. Do not activate raw sector caching yet.
If it turns out that these settings do not provide enough data for a successful
recovery they can be 
<a href="howtos43.php">optimized</a> later.
<p>
<?php end_screen_shot(); ?>

<?php begin_screen_shot("\"Files\" tab.","create-prefs-file.png"); ?>
<b>"Files" tab.</b> 
If your operating system can not create files larger than 2GB then you
must choose the "Split files " setting (marked green). In that case
dvdisaster will create up to 100 segments called "medium00.iso",
"medium01.iso" etc. instead of a single "medium.iso" file. Using this
option results in a small performance hit. This option is mostly
useful under Windows if the old FAT32 file system is still used. Leave
the other settings off as shown in the screen shot.
<?php end_screen_shot(); ?>

<pre> </pre>

<b>Not used tabs</b><p>

The "Error correction" tab has no influence on the reading process.
The "Misc" tab currently has only functions for creating
log files. This is helpful for sending in <a href="feedback.php">bug reports</a>
but should be left off during normal operation.
The "Appearance" tab allows you to adapt the output colors to your taste, 
but these have no further effects on the reading process.

<pre> </pre>


<a href="howtos42.php">Reading the medium and recovering its contents...</a>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
