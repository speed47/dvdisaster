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

howto_headline("Augmenting images with error correction data", "Advanced settings", "images/create-icon.png");
?>

<?php begin_screen_shot("\"Error correction\" tab.","create-prefs-ecc2-adv.png"); ?>
<b>Choosing the image size</b>. dvdisaster has a table of standard sizes
for CD, DVD and BD media. Any media should meet these size requirements.
Some vendors produce slightly higher capacity media. If you have
such media, insert a blank one into the currently selected drive and click the
"query medium" button (marked green) to the right of the proper medium type.
dvdisaster will determine the medium size and update the table accordingly.<p>
<b>Note:</b> The medium size can only be determined in drives which
are capable of writing the respective media type.
<pre> </pre>
<b>Arbitrary image sizes.</b> You can set a specific image size which
will not be exceeded after augmenting it with error correction data.
To do so activate the button beneath "Use at most ... sectors" and enter
the maximum image size in units of sectors (1 sector = 2KB).
<?php end_screen_shot(); ?>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
