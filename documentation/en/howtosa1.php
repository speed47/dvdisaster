<?php
# dvdisaster: English homepage translation
# Copyright (C) 2004-2012 Carsten Gnörlich
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

<!-- Insert actual page content below -->

<h3 class="top">Drive selection</h3>

<?php begin_screen_shot("Drive selection","dialog-drive-full.png"); ?>
The drive selection menu is located in the upper left corner of the
tool bar (see green marking). Click into the field to the right of the CD symbol
to drop down the drive selection. Then select the drive which contains the medium
you want to process with dvdisaster.<p>

To simplify identification of the drives the following information
is given in the menu entries:
<ul>
<li>The device identification which is typically comprised of the vendor name
and the drive model number. These values have been programmed into the drive
by the vendor. Since dvdisaster is displaying them without further processing
you will see here whatever the drive vendor deemed appropriate. Sometimes
this identification is not very meaningful.<p></li>
<li>The handle under which the drive is managed by the operating system 
(e.g. /dev/hda using GNU/Linux or F: using Windows)</li>
</ul>
<?php end_screen_shot(); ?>


<p>
<b>Examples:</b>
<table width="100%">
<tr>
<td class="w50p" align="center"><img src="images/select-drive-linux.png" alt="dvdisaster GUI: drive selection in Linux"><br>
Unfolded selection using GNU/Linux</td>
<td class="w50p" align="center"><img src="images/select-drive-win.png" alt="dvdisaster GUI: drive selection in Windows"><br>
Unfolded selection using Windows</td>
</tr>
</table><p>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
