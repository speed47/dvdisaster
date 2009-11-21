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

howto_headline("Creating error correction files", "Advanced settings", "images/create-icon.png");
?>

<?php begin_screen_shot("\"Drive\" tab.","create-prefs-drive-adv.png"); ?> 
<b>Eject medium after successful read</b>. This feature is helpful 
when you are processing a batch of media. Use it together with 
the options shown in the second screen shot below.<p> 
dvdisaster will try to eject the medium after the
image has been read.  However ejecting the medium might be prohibited
by the operating system so this is not guaranteed to work. For example
if upon media insertion a window is opened for performing the contents
it may not be possible to automatically eject the medium.
<?php end_screen_shot(); ?>

<?php begin_screen_shot("\"Files\" tab.","create-prefs-file-adv.png"); ?>
<b>Automatic file creation and deletion.</b> 
You can automate the process of creating error correction files using these
options. The first option lets dvdisaster create the error correction file
immediately after the medium has been (completely) read. The second option
deletes the image when the error correction file has been created.<p>

<b>Please note:</b> Remember to choose a different name for the error 
correction file after inserting a new medium. Otherwise 
the previous error correction file will be overwritten.
<?php end_screen_shot(); ?>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
