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
?>

<!--- Insert actual page content below --->

<h3>Recovering media images</h3>

<table width="100%" cellspacing="5">
<tr valign="top">
<td width="20%"><b>Task</b></td>
<td>
Recover the contents of a defective medium.
</td>
</tr>
<tr><td> <pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>Required:</b><p></td>
</tr>
<tr>
 <td width="150px" align="right">
   <img src="../images/bad-cd-ecc.png" align="top">
 </td>
<td>
A defective medium containing <a href="howtos30.php">error correction data</a>,
</td>
</tr>
<tr><td></td><td>or</td></tr>
<tr>
 <td width="150px" align="right">
   <img src="../images/bad-cd.png">
   <img src="../images/ecc.png">
 </td>
<td>
a defective medium with an appropriate <a href="howtos20.php">error correction file</a><a href="#footnote"><sup>*)</sup></a>.
</td>
</tr>
<tr><td> <pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>What to do:<p></b></td>
</tr>

<tr>
<td></td>
<td>
1. <a href="howtos41.php">Configure basic settings for reading,</a><br>
2a. <a href="howtos42.php#a">create an ISO image from the defective medium,</a><br>
2b. <a href="howtos42.php#b">recover the image and write it to new medium.</a>
</td>
</tr>
</table><p>

<a href="howtos42.php">Create and recover the ISO image...</a>

<pre>


</pre>

<!--- do not change below --->
<?php
footnote("*","footnote",
"The error correction file must have been created at a time the medium was still intact: It is not possible to create error correction data from an already defective medium.");
# end_page() adds the footer line and closes the HTML properly.
end_page();
?>
