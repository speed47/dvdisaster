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

?>

<!--- Insert actual page content below --->

<h3>Error correction file selection</h3>

The error correction file contains information for reconstructing unreadable
sectors from a defective medium. It can also be used to check a medium for
damaged or altered sectors. The default file extension is ".ecc".<p>

<?php begin_screen_shot("Error correction file selection","dialog-ecc-full.png"); ?>
There are two ways of choosing the error correction file:
<ul>
<li>using a <a href="#filechooser">file chooser dialog</a> (button marked green), or</li>
<li>by directly entering the error correction file location (text entry field marked blue).</li><p>
</ul>
The direct entry is helpful when you are going to create several error correction
files in the same directory. In that case simply change the file name
in the text field.<p>
<?php end_screen_shot(); ?>

<? require("howtos_winfile.php"); ?>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
