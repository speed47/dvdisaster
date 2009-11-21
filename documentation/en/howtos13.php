<?php
# dvdisaster: English homepage translation
# Copyright (C) 2004-2009 Carsten Gnörlich
#
# UTF-8 trigger: äöüß 
#
# Include our PHP sub routines, then call begin_page()../images/
# to start the HTML page, insert the header, 
# navigation and news if appropriate.

require("../include/dvdisaster.php");
require("../include/screenshot.php");

begin_page();

howto_headline("Scanning media for errors", "Interpreting results", "images/scan-icon.png");
?>

<!--- Insert actual page content below --->

<?php begin_screen_shot("Overview","defective-cd.png"); ?>
<b>Overview.</b> dvdisaster provides several information about the scanning results:
<ul>
<li>The spiral under "<b>Medium State</b>" (to the right).<p>
The spiral provides information about the medium readability. The medium is
fully readable when all segments of the spiral are colored green. Yellow or red
blocks mark places where data could not be correctly read from the medium.
The total number of unreadable sectors is printed
in the <i>"Scanning finished:"</i> message 
at the window bottom.<p>
</li>
<li>"<b>Speed</b>" - The reading speed curve (upper left).<p>
The reading speed is not an absolute gauge of the medium health,
but it is usable as a rule of thumb: 
The more regular the curve, the better the medium.
You will find examples of good and bad reading speed curves further down this
page.<p></li>
<li>"<b>C2 errors</b>" - A medium state gauge provided by the drive (down left).<p>
This kind of analysis 
is <a href="qa.php?pipo">currently only available for CD media</a>.
CD drives have a built-in error correction which can eliminate small data losses
caused by minor defects on the medium. The number of C2 errors is a measurement
of how often the drive needed to employ its  internal error correction during
the read - this value should be zero on good media.</li>
</ul>
<?php end_screen_shot(); ?>

<b>Examples for good media</b><p>

<?php begin_screen_shot("Good CD","good-cd.png"); ?>
<b>Good CD</b>: This screen shot shows a perfect CD:
All blocks under "Medium state" are green, no C2 errors have been reported
and the reading curve runs smoothly. A rising reading speed is normal for most
media (see the next screen shot for a counter example). The small spikes
at the beginning and at the end of the curve are normal; minor glitches like
the one shown at 250M are also harmless.
<?php end_screen_shot(); ?>


<?php begin_screen_shot("Good two layered DVD","good-dvd9.png"); ?>
<b>Sometimes the reading curve won't rise steadily</b>: Multi-layered media
might yield reading curves which are rising and dropping in a symmetric pattern.
Not shown but also possible are flat curves without any change in reading speed
(most typically seen with DVD-RAM).
<?php end_screen_shot(); ?><p>

<b>An example for a weak medium</b><p>

<?php begin_screen_shot("Weak CD","weak-cd.png"); ?>
This medium is still readable as indicated by the green spiral shown under
"Medium state". However there are clear signs of serious trouble ahead:
The drive must slow down significantly towards the end of the medium in order
to read from it. Note the steep fall of reading speed after the 600M mark.
This comes along with C2 error rates rising to the 100 mark; this is another
warning that the medium is decaying in the outer region.
If you have not created <a href="howtos20.php">error correction</a>  data
this is probably the last opportunity to do so as the medium will develop
the first read errors soon.
<?php end_screen_shot(); ?><p>

<b>Examples of defective media</b><p>

<?php begin_screen_shot("Defective CD","defective-cd.png"); ?>
<b>Defective CD.</b> 
The red sectors in the spiral visualize large unreadable sections
in the outer region of the medium. At the bottom of the window you will find
the information that the medium contains 28752 unreadable sectors.
This sums up to about 8.2% defective sectors (of 352486 sectors total) and
is well within the 
<a href="howtos40.php">recovery</a> bounds by 
<a href="howtos20.php">error correction (ecc) data</a> made 
with default settings - if you have made the ecc data in time! 
Otherwise the contents of the red
sectors are lost since ecc data cannot be created from already defective media.
<?php end_screen_shot(); ?><p>

<a name="crc"></a>
<?php begin_screen_shot("Checksum errors","crc-cd.png"); ?>
<b>Checksum errors.</b> Yellow spots in the spiral depict places
where the medium was fully readable, but the data read did not match
checksums in the error correction data. There are two main causes:
<p>

<ul><li>
<b>The image has been manipulated</b> after the creation 
of error correction data and before writing it to the medium.
This can happen on Unix systems when the image is mounted with write access
after ecc data has been created. Typical signs are CRC errors in sector 64
and in sectors 200 to 400 as the system updates the file access times there.
Performing a data recovery using dvdisaster is typically harmless in this
situation.<p>

However if you have modified files in the image after creating the ecc data,
the error correction data will be both worthless and dangerous.
Applying a recovery to the medium will restore the image state
at the time the ecc data has been created, and this will obviously not 
represent the most recent contents of the medium.<p></li>

<li><b>There are technical problems with the computer system,</b>
especially in mass storage communication. Perform the scan again
and observe the CRC error locations.
If CRC errors disappear or surface at different locations your system
might have defective RAM, bad drive cabling/controllers or incorrect clock
speeds.</li></ul>
<?php end_screen_shot(); ?><p>

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
