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
begin_page();
?>

<!-- Insert actual page content below -->

<h3 class="top">Remarks on read errors</h3>

Optical media have their own error correction code which protects the data
against small manufacturing errors and inaccuracies during writing.
If the writer and medium are compatible and of high quality, the
error correction built into the medium will at first be mainly idle.
This leaves enough reserves to compensate normal wear and aging effects 
during many years of the medium usage.
<p>

When the capacity of the built-in error correction is finally exhausted,
read errors will start to appear on the medium. These will be reported by
the <a href="howtos10.php">"Scan"</a>-operation of dvdisaster.
Depending on the time of first occurrence, 
two types of read errors are of particular interest:<p>

<b>Read errors appearing right after writing the medium.</b>
This is a sign of:

<ul>
<li>media from a faulty production run, or</li>
<li>media which are not compatible with the writer.</li>
</ul>

A prudential choice is to dispose of the faulty media and
to write the data on error-free media, possibly switching to a 
different producer.<p>

Please withstand the temptation of trying to preserve 
the faulty media by means of an error correction file - this
will most likely end with data loss.<p>

<b>Read errors after a few months/years.</b> 
The built-in error correction of the medium will be increasingly loaded
during its life time until it finally fails and read errors show up.
This happens for mechanical reasons (scratches, warping of the plastic material)
as well as for chemical causes (decaying dye and/or reflective layer).<p>

These effects typically occur while the medium is stored away for a few months,
and it may not be possible to read in all sectors afterwards.<p>

Therefore it is crucial to create the 
<a href="howtos21.php">error correction data</a> in time. 
The ecc data contains information for recalculating the contents of
missing sectors
<a href="qa31.php">(within certain limits)</a>.
Therefore with the help of the ecc data
dvdisaster can recover images even if not all sectors
could actually be read by the drive.<p>

Since the error correction can reconstruct missing sectors up to a certain number,
it is not necessary to squeeze out a defective medium for every readable sector.
The <a href="qa35.php">adaptive reading strategy</a> checks during
reading whether enough data for error correction has been collected.
As soon as this is the case, reading stops and still unread sectors
will be recovered using the ecc data.<p>

<a name="reading-tips"><b>
Some hints for effectively reading damaged media</b></a><p>

The outcome from reading damaged media depends on several factors:

<ul>
<li><b>Not all drives are built the same.</b><br>
Different drives have different reading capabilities.
Take advantage of dvdisaster's function for completing an image 
with several reading passes and use different drives for each pass.
Transfer the image file between computers using a network or rewritable media
in order to use drives installed in different machines.
<p></li>
<li><b>Eject and insert the medium again.</b><br>
Sometimes it makes a difference to eject the medium, turn it about a quarter,
and then load it again for another reading pass.
<p></li>
<li><b>Some drives read better while being cold.</b><br>
Turn off the computer over night and perform another reading attempt in the next
morning.<p>
But note: "Cold" refers to normal living room conditions - putting hardware
or media into the fridge can be very unhealthy for them.<p></li>
</ul>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
