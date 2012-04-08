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
require("../include/footnote.php");
begin_page();
howto_headline("Creating error correction data as a separate file", "Overview", "images/create-icon.png");?>

<!-- Insert actual page content below -->

<table width="100%" cellspacing="5">
<tr valign="top">
<td class="w20p"><b>Task</b></td>
<td>
An error correction file is created for a CD/DVD/BD medium.
</td>
</tr>
<tr><td> <pre> </pre> </td></tr>

<tr valign="top">
<td></td>
<td>Note: This page describes how error correction data is created and placed
into a separate file. 
There is also a method for placing the error correction data
directly onto the medium.
<a href="howtos21.php">Would you like help on deciding between these two methods?</a></td>
<tr><td> <pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>Required:</b><p></td>
</tr>

<tr>
<td><img src="../images/good-cd.png" alt="Icon: Good medium (without read errors)"></td>
<td>
A good, error free<a href="#footnote"><sup>*)</sup></a> medium,</td>
</tr>

<tr><td></td><td>or</td></tr>


<tr>
<td><img src="../images/good-image.png" alt="Icon: Complete image"></td>
<td>an already existing and complete<a href="#footnote"><sup>*)</sup></a> 
ISO image of the medium (e.g. the image used for writing the medium).
</td>
</tr>
<tr><td> <pre> </pre> </td></tr>


<tr>
<td colspan="2"><b>What to do:</b><p></td>
</tr>

<tr>
<td></td>
<td>
1. <a href="howtos22.php">Configure basic settings</a><br>
2. <a href="howtos23.php">Create the error correction file</a><br>
3. <a href="howtos24.php">Archive the error correction file</a>
</td>
</tr>
</table><p>

<a href="howtos22.php">Configuring basic settings...</a>

<pre>


</pre>

<?php
footnote("*","footnote","Error correction data must be created before any
data loss occurs: It is not possible to create error correction files
from an already defective medium.");
?>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
