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

<h3>Image file selection</h3>

The image file contains the data from all medium sectors, including the information
whether a sector was readable. dvdisaster works on image files because they are
stored on hard disk which makes certain random access patterns much faster.
Applying this kind of random access to CD/DVD/BD drives would slow them down
significantly and eventually wear them out (the image files are created using
sequential access which unlike random access is handled efficiently by the drives).
The default file suffix for images is ".iso".<p>

<?php begin_screen_shot("Image file selection","dialog-iso-full.png"); ?>
There are two ways of choosing the image file:
<ul>
<li>using a <a href="#filechooser">file chooser dialog</a> (button marked green), or</li>
<li>by directly entering the file location (text entry field marked blue).</li><p>
</ul>
The direct entry is helpful when you are processing several files
in the same directory. In that case simply change the file name in the text field.
<p>
<?php end_screen_shot(); ?>

<? require("howtos_winfile.php"); ?>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
