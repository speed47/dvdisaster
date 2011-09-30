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
require("../include/footnote.php");
begin_page();
howto_headline("Augmenting images with error correction data", "Overview", "images/create-icon.png");
?>

<!--- Insert actual page content below --->

<h3>Putting error correction data directly onto the medium</h3>

<table width="100%" cellspacing="5">
<tr valign="top">
<td width="20%"><b>Task</b></td>
<td>
Error correction data is stored along with the user data on the same medium.
</td>
</tr>
<tr><td> <pre> </pre> </td></tr>

<tr valign="top">
<td></td>
<td>Note: This page describes how an ISO image is augmented with error
correction data prior to writing it onto a medium.
There is also a method for creating and placing error correction data into
a separate file. 
<a href="howtos21.php">Would you like help on deciding between these two methods?</a></td>
<tr><td> <pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>Required:</b><p></td>
</tr>

<tr>
<td><img src="../images/good-image.png"></td>
<td>
<ul>
<li>an authoring ("burning") software capable of creating ISO images</li>
<li>the medium which is to be augmented with error correction data
 has not yet been written <a href="#footnote"><sup>*)</sup></a></li>
<li>at least 20% of free space on the medium which is to be created</li>
</ul>
</tr>
<tr><td> <pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>What to do:<p></b></td>
</tr>

<tr>
<td></td>
<td>
1. <a href="howtos32.php">Configure basic settings</a><p>
2a. <a href="howtos33.php#a">Create an ISO image,</a><br>
2b. <a href="howtos33.php#b">augment it with error correction data,</a><br>
2c. <a href="howtos33.php#c">and write it to a medium.</a>
</td>
</tr>
</table><p>

<a href="howtos32.php">Configuring basic settings...</a>

<pre>


</pre>

<!--- do not change below --->

<?php
footnote("*","footnote","An already written medium can not be augmented with error correction data.");

# end_page() adds the footer line and closes the HTML properly.
end_page();
?>
