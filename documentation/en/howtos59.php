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

<h3 class="top">Examples</h3>

You  have already seen examples of
 <a href="howtos52.php?expand=1">good images and error correction files</a>
and <a href="howtos53.php?expand=1">good images augmented with
error correction data</a> on the previous pages.
In the following some typical examples of error situations are presented:<p>

<hr>

Here are two typical cases of showing information for images
which have not yet been fully recovered:<p>

<?php begin_screen_shot("Image with unreadable sectors, RS01","compare-bad-rs01.png"); ?>
<b>Image with unreadable sectors and error correction file</b>.
The image shown here contains 6245 unreadable sectors; error correction
data is present by means of an error correction file.
<p>
<?php end_screen_shot(); ?>

<?php begin_screen_shot("Image with unreadable sectors, RS02","compare-bad-rs02.png"); ?>
<b>Image augmented with error correction data, containing unreadable sectors</b>.
This image contains unreadable sectors towards its end. Especially the
ECC section is affected since the error correction data is located
at the end of the image. Please note that this does not weaken the
error correction since its corrective power is independent from the
error location: 10000 errors at the beginning of the medium are just as easy
to correct as 10000 errors towards its end.<br>
The RS02 encoder which was used for creating the error correction
data is capable of predicting the odds of a successful image recovery.
These are shown at the end of the error correction data output area.
<?php end_screen_shot(); ?>

<hr>  

<?php begin_screen_shot("Aborted read","compare-trunc-rs01.png"); ?>
<b>Image from aborted reading process</b>.
This image is shorter than expected; this usually happens when
the reading process is stopped prematurely.
<p>
<?php end_screen_shot(); ?>

<?php begin_screen_shot("Image too large","compat-150-rs01.png"); ?>
<b>Image is larger than expected</b>.
This image is larger than expected; possible causes are discussed
in the section about <a href="howtos90.php">image  compatibility</a>.
It may be possible to recover from this problem; see hints related to
<a href="howtos91.php#err">using error correction files</a> and
<a href="howtos92.php#err">using augmented images</a>.
<?php end_screen_shot(); ?>

<?php begin_screen_shot("Wrong error correction file","compare-mismatch-rs01.png"); ?>
<b>Wrong error correction file</b>.
The error correction file was created for a different image.
This causes lots of CRC errors since the sectors have different contents.
However the most important hint is:<p>
Fingerprint: <span class="red">mismatch</span><p>
This tells you that the error correction file does not belong to the image.
<?php end_screen_shot(); ?>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
