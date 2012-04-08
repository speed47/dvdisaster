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

howto_headline("Scanning media for errors", "Overview", "images/scan-icon.png");
?>

<!-- Insert actual page content below -->

<table width="100%" cellspacing="5">
<tr>
 <td><b>Task</b></td>
 <td>
   The medium is scanned for unreadable sectors.
  </td>
</tr>
<tr><td> <pre> </pre> </td></tr>

<tr>
 <td colspan="2"><b>Required:</b></td>
</tr>
<tr>
 <td class="w150x"><img src="../images/good-cd.png" alt="Icon: Good medium (without read errors)" class="valignt">
   &nbsp; <img src="../images/bad-cd.png" alt="Icon: Damaged medium (partially unreadable)" class="valignt"></td>
<td>
   A medium in any state (good or containing read errors).
</td>
</tr>

<tr>
 <td><img src="../images/ecc.png" alt="Icon: Separate file with error correction data"></td>
 <td>If error correction data is available additional tests are carried out.
However scanning will also work without error correction data.</td>
</tr>

<tr><td> <pre> </pre> </td></tr>

<tr valign="top">
 <td><b>What to do:</b></td>
 <td>
  1. <a href="howtos11.php">Configure basic settings</a><br>
  2. <a href="howtos12.php">Scan the medium</a><br>
  3. <a href="howtos13.php">Interpret the results</a><br>
 </td>
</tr>

<tr><td> <pre> </pre> </td></tr>

<tr valign="top">
 <td><b>Related functions:</b></td>
 <td><a href="howtos42.php#a">Reading of damaged media</a> and<br>
     <a href="howtos40.php">Recovering images</a>.</td>
 </tr>
</table><p>

<pre> </pre>

<a href="howtos11.php">Configuring basic settings...</a>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
