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

<h3>Typical applications</h3>

dvdisaster is a complex tool which would require a whole book to cover
all of its features. Since we are currently lacking the resources for
doing a book (and you might be short on reading time also) we will take a
different approach here.
First we will demonstrate how the <a href="howtos60.php">different
functions of dvdisaster work together</a>.
Then we will describe common tasks and provide step by step instructions 
for solving them. In most cases following these 
steps will be all you need to do. At the end of
each instruction set a discussion of
further configuration options is included for advanced users.<p>

<h3>Symbols used in this document</h3>

Working with dvdisaster requires certain combinations of optical media,
media images and error correction data. Check out the following symbols
to find out what you will need for the respective tasks:<p>

<b>Medium</b> (a CD for example)

<table cellspacing="10">
<tr>
<td align="center" width="15%"><img src="../images/good-cd.png"></td>
<td align="center" width="15%"><img src="../images/bad-cd.png"></td>
<td width="55%">These symbols indicate whether processing a medium
is part of the respective task, and if the medium
needs be completely error free or may already be damaged.
</td>
</tr>
<tr  valign="top">
<td>good medium (<b>no</b> read errors)</td>
<td>bad medium (<b>with</b> read errors)</td>
<td></td>
</tr>
</table><p>

<b>Medium image</b> (ISO image of a medium stored on the hard disk)

<table cellspacing="10">
<tr>
<td align="center" width="15%"><img src="../images/good-image.png"></td>
<td align="center" width="15%"><img src="../images/bad-image.png"></td>
<td width="55%">Some functions do not work directly with the medium, but
with an ISO image on hard disk instead. Depending on the condition of the
respective medium the image may be complete or incomplete.</td>
</tr>
<tr valign="top">
<td>complete image (made from good medium)</td>
<td>incomplete image (made from bad medium)</td>
</tr>
</table><p>

<b>Error correction data</b>

<table cellspacing="10">
<tr>
<td align="center" width="15%"><img src="../images/good-cd-ecc.png"></td>
<td align="center" width="15%"><img src="../images/ecc.png"></td>
<td width="55%">Recovering media images by using error correction data
is the key feature of dvdisaster. This symbol shows whether error correction
data is required.
</td>
</tr>
<tr  valign="top">
<td>Medium containing error correction data</td>
<td>Separate error correction file</td>
<td></td>
</tr>
</table><p>

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
