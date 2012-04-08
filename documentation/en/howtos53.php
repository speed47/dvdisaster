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

<h3 class="top">Results for images augmented with error correction data</h3>

<?php begin_howto_shot("Image with error correction data.","compat-okay-rs02.png", ""); ?>
When verifying an image against its embedded error correction data the
information will be given with respect to: 
<ul>
<li>the whole (augmented) image</li>
<li>the error correction data part:</li>
</ul>
<?php end_howto_shot(); ?>

<table>
<tr><td colspan="2">Output field <b>"Image file summary":</b><br><hr></td><td></td></tr>
<tr>
<td class="valignt">Medium sectors:</td>
<td>The number of sectors in the augmented image (including the sectors added
by dvdisaster; one sector = 2KB).</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td class="valignt">Data checksum:</td>
<td>The MD5 checksum of the original image (prior to augmenting it with error
correction data).</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td>
Ecc headers:<br>
Data section:<br>
Crc section:<br>
Ecc section:
</td>
<td class="valignt">The augmented image consists of three sections plus
some ecc header sectors embedded into them. These values describe how many
sectors are unreadable in the respective sections.
</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr><td colspan="2">
If all values in this output field are okay the message
"<span class="green">Good image.</span>" appears.
Otherwise the most important error will be explained there.
</td>
</tr>

<tr><td>&nbsp;</td><td></td></tr>
<tr><td colspan="2">Output field <b>"Error correction data":</b><br><hr></td><td></td></tr>
<tr>
<td class="valignt">Created by:</td>
<td>Prints the dvdisaster version which was used for creating the
error correction data. Alpha/developer versions are highlighted in red.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td class="valignt">Method:</td>
<td>The method and redundancy used for creating the error correction data.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td class="valignt">Requires:</td>
<td>Processing the error correction data requires at least the shown
version of dvdisaster.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td class="valignt">Medium sectors:</td>
<td>The first value is the number of sectors in the augmented image;
the second one describes the number of sectors the image had before it
was processed with dvdisaster. Since the error correction data is placed
behind the user data, the checksum of the original image can be
obtained as follows (using the command line of GNU/Linux):<br>
<tt>head -c $((2048*121353)) medium.iso | md5sum</tt><br>
The first parameter for <i>head</i> is the sector size (2048) 
multiplied with the original image length (121353). This property
of augmented images can also be used to cut off the error correction data:<br>
<tt>head -c $((2048*121353)) medium.iso >stripped_image.iso</tt>
</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td class="valignt">Data checksum:</td>
<td>The MD5 checksum of the original image (see previous explanations).</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td class="valignt">
CRC checksum:<br>
ECC checksum:</td>
<td>MD5 checksums of the CRC and ECC sections of the augmented image.
These two can not be easily reproduced outside of dvdisaster.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr><td colspan="2">
If all values in this output field are okay the message
"<span class="green">Good error correction data.</span>" appears.
Otherwise the most important error will be explained there.
</td>
</tr>

</table>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
