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
begin_page();
?>

<!--- Insert actual page content below --->

<h3>Testing image compatibility</h3>

<b>Why dvdisaster uses ISO images.</b> 
Some dvdisaster functions work on image files stored on hard disk.
CD/DVD/BD drives are too slow for carrying out the required access
patterns, and they would wear out quickly. However hard disks are designed
for this type of access, and they do them quickly and without wear.<p>

<b>Testing image compatibility is important.</b> 
During the work with dvdisaster you can (and sometimes must) use ISO images
which have been created by third-party software. Unfortunately ISO images
are only informally standardized. Typically all programs will produce
the same images when advised to use the ".iso" file format, but it is
better to make sure that a usable ISO image has been created:
Processing a non-iso image with dvdisaster will result in
unusable error correction data. Especially, formats like 
<b>.nrg are not suitable</b> for processing with dvdisaster.
<p>

<b>Possible scenarios.</b> The following situations require exchanging
ISO images between dvdisaster and a third party software:<p>

<b>a) Creating error correction files from ISO images made by a CD authoring software</b><p>

A CD/DVD/BD authoring software is used to create an ISO image.
This image is used for writing the medium and for creating the
error correction file. When using the authoring software for the first time
with dvdisaster, make sure that 
the <a href="howtos91.php">image was written to the medium without 
modifications</a>.<p>


<b>b) Augmenting ISO images with error correction data</b><p>

dvdisaster adds "invisible" error correction data to the medium in order to
minimize interference with other applications.
Therefore it is possible that some CD/DVD/BD writing software will not properly 
write the error correction data to the medium. Make sure that your writing
software does <a href="howtos92.php">correctly transfer the error correction data</a> when using it with augmented images for the first time.

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
