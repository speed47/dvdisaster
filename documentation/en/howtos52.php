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

<h3>Results for error correction files</h3>

<?php begin_howto_shot("Image and error correction file.","compat-okay-rs01.png", ""); ?>

Comparing an image against its error correction file produces information
in two output fields; each related to one of the files:
<?php end_howto_shot(); ?>

<table>
<tr><td colspan="2">Output field <b>"Image file summary":</b><br><hr></td><td></td></tr>
<tr>
<td valign="top">Medium sectors:</td>
<td>The number of sectors in the ISO image (one sector = 2KB).</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<td valign="top">Checksum errors:</td>
<td>The error correction file contains CRC32 checksums for each image
sector. If this value is greater than zero some sectors were readable
but their contents do not match the checksum. The error correction will
try to recalculate the contents of these sectors.
</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td valign="top">Missing sectors:</td>
<td>This is the number of sectors which could not be read from the medium.
The error correction will try to recover the contents of these sectors.
</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td valign="top">Image checksum:</td>
<td>A MD5 checksum is calculated for the complete ISO image.
You can reproduce this value using the command line of GNU/Linux:<br>
<tt>md5sum medium2.iso</tt></td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr><td colspan="2">
If all values in this output field are okay the message
"<font color="#008000">Good image.</font>" appears.
Otherwise the most important error will be explained there.
</td>
</tr>

<tr><td>&nbsp;</td><td></td></tr>
<tr>
<td colspan="2">Output field <b>"Error correction file summary"</b>:<br><hr></td><td></td>
</tr>
<tr>
<td valign="top">Created by:</td>
<td>Prints the dvdisaster version which was used for creating the
error correction data. Alpha/developer versions are highlighted in red.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td valign="top">Method:</td>
<td>The method and redundancy used for creating the error correction file.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td valign="top">Requires:</td>
<td>Processing the error correction data requires at least the shown
version of dvdisaster.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td valign="top">Medium sectors:</td>
<td>The expected number of sectors in the image file.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td valign="top">Image checksum:</td>
<td>The expected MD5 sum of the image file.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td valign="top">Fingerprint:</td>
<td>dvdisaster uses the checksum of a special sector to determine whether
the error correction file was made for a given image.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td valign="top">Ecc blocks:</td>
<td>The error correction divides the image into small blocks which can
be processed independently. This information is mostly useless as long
as the number of ecc blocks is correct ;-) 
</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td valign="top">Ecc checksum:</td>
<td>A MD5 checkum is calculated over the error correction file,
not taking into account the first 4KB. You can reproduce this value
using the command line of GNU/Linux:<br>
<tt>tail -c +4097 medium.ecc | md5sum</tt>
</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr><td colspan="2">
If all values in this output field are okay the message
"<font color="#008000">Good error correction file.</font>" appears.
Otherwise the most important error will be explained there.
</td>
</tr>
</table>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
