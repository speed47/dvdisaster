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
$way=$_GET["way"];
if($way & 1) $make_iso_action=$way&2;
else         $make_iso_action=$way|1;
if($way & 2) $write_iso_action=$way&1;
else	     $write_iso_action=$way|2;
?>

<!--- Insert actual page content below --->

<?php
howto_headline("Augmenting images with error correction data", "Walkthrough", "images/create-icon.png");
?>

dvdisaster is specialized in working with error correction data and reading
of defective media. Creating ISO or UDF images and writing them to a medium
is a totally different business, and also bears high complexity. 
We do not want to re-invent medium writing in dvdisaster. Instead we 
continue using the CD/DVD/BD writing software which you have got when
purchasing your drive.<p>

<pre> </pre>

<a name="a"></a>
<table>
<tr>
<td width="200px" align="center">
<?php
echo "<a href=\"howtos33.php?way=$make_iso_action\">\n";
?>
<img src="thumbnails/make-iso1.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>First create an ISO image</b> using your CD/DVD/BD
writing software. Select the files you want to write to medium,
but do not start the writing process yet. Instead, create an ISO
image on your hard disk. Click on the image to the left to see a 
<?php
echo "<a href=\"howtos33.php?way=$make_iso_action\">\n";
echo "detailed walk-thorugh</a>.\n";
?>
</td>
</tr>
</table>

<?php
if($way&1)
{
?>

<hr>

<b>Detailed example: Creating an ISO image on hard disk</b>. 
Since there are many different media writing programs available we
are demonstrating the required steps by using
the popular GNU/Linux software <i>K3b</i> as an example.
If you are using a different software you should be able to figure
out the required actions from the descriptions below.
<p>

<hr>

<?php begin_screen_shot("Start new project","make-iso1.png"); ?>
<b>Begin a new project.</b>
First open your media writing application. Many programs expect you to
start a new project. Within the project you will then make
the selections for the new medium. <p>
Using K3b: <i>Begin a new project by clicking into the highlighted field
("New Data CD project") in the main window.</i>
<?php end_screen_shot(); ?>

<hr>

<?php begin_screen_shot("File selection","make-iso2.png"); ?>
<b>Select the files to be written on the medium.</b>
Typically there is a file selection dialog from which you can select files
or drag them into the project.<p>

Using K3b: <i>Choose the required files in the upper half of the window.
In the example the files <i>backup.tar.gz</i>,
<i>win.zip</i> and <i>work.tar.gz</i> have been selected for writing onto CD.
The currently selected files are shown in the lower window half.</i><p>

<b>Important:</b> Do not completely fill the medium. Make sure to keep
at least 20% of the medium space for the error correction data.<p>

Using K3b: <i> The currently used medium space is shown in the green bar
at the bottom of the window (558,9 MB).</i>
<?php end_screen_shot(); ?>

<hr>

<?php begin_screen_shot("Configuring the writing software","make-iso2.png"); ?>
<b>Configuring the writing software.</b> The software will let you
choose the writing target before the actual writing process is invoked.
Do <b>not</b> select the CD/DVD/BD writing drive here, but configure
the creation of an ISO/UDF image on hard disk as described below.<p>

<b>Hint:</b> Remove all media from the drives before proceeding to make sure
that you do not inadvertently start the writing process.<p>

Using K3b: <i>Open the writing dialog tab by clicking on the "Burn" button 
near to the left edge of the window.</i>
<?php end_screen_shot(); ?>

<?php begin_screen_shot("Selecting image creation","make-iso3.png"); ?>
<b>Selecting image writing.</b> Most writing programs will simply let
you click an option for creating the ISO image on hard disk.
If your program seems to be missing this option you might have to select
an "image recorder" instead of the actual CD burner.<p>

Using K3b: <i>Select the "Writing" tab". 
Activate the "Only create image" option (marked in green).</i>
<?php end_screen_shot(); ?>

<?php begin_screen_shot("Image file selection","make-iso4.png"); ?>
<b>Select image file and type.</b>
Select the target directory, name and type for the image file.
Use image files of type ".iso" or ".udf" only! Other image formats like
".nrg" are not supported by dvdisaster; processing such image files with
dvdisaster will render them unusable without further notice or error
messages.<p>

Using K3b: <i>Choose the "Image" tab. 
Enter the target directory for the file 
(the example file "medium.iso" will be put into the sub 
directory "/var/tmp/cg"). K3b will always create .iso images so there
are no choices to be made for the image type.</i>
<?php end_screen_shot(); ?>

<hr>

<table>
<tr>
<td width="200px" align="center">
<img src="../images/down-arrow.png" border="0"></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"></td>
</tr>
</table>

<?php
}  /* end from if($way&1) above */
?>

<a name="b"></a>
<table>
<tr>
<td width="200px" align="center">
<img src="../images/good-image.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top">When you have prepared the image
<b>switch over to dvdisaster</b>. Make sure that it
has been configured as described in 
the <a href="howtos32.php">basic settings</a>.
</td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center">
<a href="howtosa2.php">
<img src="../images/select-image2.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Select the directory and file name</b> 
of the ISO image which you have just created.</td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center">
<a href="howtosa4.php">
<img src="images/create-icon.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Augment the image with error correction data</b> by
clicking on the "Create" button.</td>
</tr>
</table>

<?php begin_howto_shot("Creating error correction data.","make-ecc1.png", "down-arrow.png"); ?>
<b>Please wait while the error correction data is created.</b>
This may take a while depending on the image size and the 
available free  space on the medium. Processing a DVD image with about
20-30% free space should take about 5-10 minutes on recent hardware.
<?php end_howto_shot(); ?>

<?php begin_howto_shot("Comparing image sizes.","make-ecc2.png", "down-arrow.png"); ?>
<b>Please note:</b> dvdisaster does not create a new image, but will rather
augment the existing one. Look at the image in the file manager before
and after processing it with dvdisaster and note how its size increases.
<?php end_howto_shot(); ?>

<a name="c"></a>
<table>
<tr>
<td width="200px" align="center">
<?php
echo "<a href=\"howtos33.php?way=$write_iso_action\">\n";
?>
<img src="thumbnails/write-iso1.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Write the augmented ISO image</b> on the medium.
Select the augmented image in your writing software and
start the writing process. Click on the screen shot to the left for
<?php
echo "<a href=\"howtos33.php?way=$write_iso_action\">\n";
echo "a detailed example</a>.\n";
?>
</td>
</tr>
</table>

<?php
if($way&2)
{
?>
<table>
<tr>
<td width="200px" align="center">
<img src="../images/down-arrow.png" border="0"></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"></td>
</tr>
</table>

<hr>

<b>Detailed example: Writing the ISO image on the medium</b>.<p>

<?php begin_screen_shot("Select image writing","write-iso1.png"); ?>
<b>Select writing of the image.</b>
Now open your media writing software again. Invoke the mode for writing
pre-existing .iso images on the medium.<p>

Using K3b: <i>Click on the highlighted field
("Burn CD image") in the main window.</i>
<?php end_screen_shot(); ?>

<hr>

<?php begin_screen_shot("Image selection","write-iso2.png"); ?>
<b>Image selection.</b>
Select the image you have just created and augmented with dvdisaster.
<p>

Using K3b: <i>Use the field marked in green to select the image file or
enter its name directly.</i>
<?php end_screen_shot(); ?>

<?php begin_screen_shot("More settings","write-iso2.png"); ?>
<b>More settings.</b>
Select the "DAO" ("disk at once") writing mode if supported by your drive.
This improves the compatibility between the medium and the error correction.
In addition this prevents you from accidentally adding more sessions 
to the disc which would destroy the error correction data.
<p>

Using K3b: <i>Choose "DAO" in the field marked yellow.</i>
<?php end_screen_shot(); ?>

<?php begin_screen_shot("Writing the medium","write-iso3.png"); ?>
<b>Writing the medium.</b>
Now start the writing process.
<p>

Using K3b: <i>Click on the "Start" button in the window from the previous
screen shot.</i>
<?php end_screen_shot(); ?>

<hr>

<?php
}  /* end from if($way$2) above */
?>

<table>
<tr>
<td width="200px" align="center">
<img src="../images/down-arrow.png" border="0"></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"></td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center">
<img src="../images/good-cd-ecc.png" border="0"></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Finished!</b> You have now created a CD which is
protected by error correction code.</td>
</tr>
</table>

<pre> </pre>

<b>Related information</b>

<ul>
<li><a href="howtos90.php">Check whether the writing process has affected 
the error correction data.</a><p>
It is recommended to perform this test once every time you change to a new
version (or vendor) of your media writing software to make sure that it
interoperates well with dvdisaster.
</li>
</ul>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
