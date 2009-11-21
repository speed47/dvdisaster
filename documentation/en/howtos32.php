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

howto_headline("Augmenting images with error correction data", "Basic settings", "images/create-icon.png");
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

<pre> </pre>

<?php begin_screen_shot("\"Error correction\" tab.","create-prefs-ecc2.png"); ?>
<b>"Error correction" tab.</b> Choose "Augmented Image (RS02)" as storage
method (green menu). Select "Use smallest possible size from 
following table" if you are working with standard media sizes.
dvdisaster will then choose the smallest possible medium type which can
be used for storing the image. The remaining free space on the medium
will be used for error correction data and the image will be prepared
accordingly.
<?php end_screen_shot(); ?>


<pre> </pre>

<b>Not used tabs</b><p>

The "Misc" tab currently has only functions for creating
log files. This is helpful for sending in <a href="feedback.php">bug reports</a>
but should be left off during normal operation.
The "Appearance" tab allows you to adapt the output colors to your taste, 
but these have no further effects on the error correction data creation.

<pre> </pre>

<a href="howtos33.php">Augmenting the image with error correction data...</a>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
