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
?>

<!--- Insert actual page content below --->

<h3>Examples of the error correction</h3>

<?php begin_screen_shot("Reading the defective medium.", "recover-linear.png"); ?>
   <b>Recovery of aged media.</b> The medium processed here has become
   discolored and partly unreadable in its outer region. A surface scan
   yields about 23.000 unreadable sectors of 342.000 sectors total; resulting
   in about 7,2% defective sectors.
<?php end_screen_shot(); ?>

<?php begin_screen_shot("Repairing the defective image.", "fix-image.png"); ?>
   <b>Repairing the defective image.</b> The resulting image is still incomplete
   since about 23.000 sectors could not be read. These sectors are now 
   reconstructed using 
   the error correction data created with dvdisaster.
   During the recovery a maximum of 20 errors
   per error correction block is encountered. This results in a peak
   error correction load of 63%, meaning that this degree of damage is handled well
   by error correction data created with default settings.
<?php end_screen_shot(); ?>

<b>Recovery needs error correction data:</b> The recovery process described above uses
error correction ("ecc") data. 
Think of this data as a special
form of backup data (it needs less space than a normal backup, though).
Like an ordinary backup, the ecc data needs to be created
<i>before</i> the medium goes defective.<p>

So if you have a defective medium but never created ecc data for it - sorry, 
your data is probably lost.<p>


<a href="index20.php">Why quality scans won't suffice...</a>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
