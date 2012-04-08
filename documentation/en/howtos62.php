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

<h3 class="top">Using dvdisaster the right way</h3>

Let's demonstrate how Jane uses dvdisaster. <p>

<table width="100%">
<tr>
<td class="w15p">10. Feb. 2004</td>
<td class="w65x"><img src="../images/good-cd.png" alt="Icon: Good medium (without read errors)"></td>
<td class="w65x"></td>
<td>Jane creates a new CD with important data.</td>
</tr>
<tr><td colspan="4"> <p> </td></tr>
<tr>
<td> </td>
<td><img class="valignt" src="../images/good-cd.png" alt="Icon: Good medium (without read errors)"></td>
<td><img class="valignt" src="../images/ecc.png" alt="Icon: Separate file with error correction data"></td>
<td>To protect the CD from data loss
    <a href="howtos20.php">she creates error correction data with dvdisaster</a>.
    She keeps both kinds of data for later usage.</td>
</tr>
<tr><td colspan="4"> <hr> </td></tr>
<tr>
<td>14. May 2005</td>
<td><img class="valignt" src="../images/good-cd.png" alt="Icon: Good medium (without read errors)"></td>
<td><img class="valignt" src="../images/ecc.png" alt="Icon: Separate file with error correction data"></td>
<td> Jane knows that during daily use not all data of her CD might be 
accessed. So after a year has passed 
she <a href="howtos10.php">scans the CD for read errors</a> to make sure
that it has not developed any defects in seldom used data regions. However
after one year the CD is still perfectly readable.</td>
</tr>
<tr><td colspan="4"> <hr> </td></tr>
<tr>
<td>19. Aug 2007</td>
<td><img class="valignt" src="../images/bad-cd.png" alt="Icon: Damaged medium (partially unreadable)"></td>
<td><img class="valignt" src="../images/ecc.png" alt="Icon: Separate file with error correction data"></td>
<td>Two more years have passed and Jane notices that some data on the 
CD is no longer readable. A <a href="howtos10.php">scan for read errors</a> 
confirms that the CD has become defective due to aging.</td>
</tr>
<tr>
 <td align="right"><a href="howtos42.php#a">read</a></td>
 <td align="center"><img class="valignt" src="../images/down-arrow.png" alt="Icon: Arrow down"></td>
 <td></td><td></td>
</tr>
<tr><td colspan="4"> <p> </td></tr>
<tr>
<td> </td>
<td><img class="valignt" src="../images/bad-image.png" alt="Icon: Partial image"></td>
<td><img class="valignt" src="../images/ecc.png" alt="Icon: Separate file with error correction data"></td>
<td>Jane uses dvdisaster to <a href="howtos42.php#a">read as much sectors
as possible</a> from the defective CD into an ISO image.</td>
<tr>
 <td align="right"><a href="howtos42.php#b">reconstruct</a></td>
 <td align="center" colspan="2"><img class="valignt" src="../images/dbl-arrow-left.png" alt="Icon: Double arrow left"></td>
 <td></td>
</tr>
<tr><td colspan="4"> <p> </td></tr>
<tr>
<td> </td>
<td><img class="valignt" src="../images/good-image.png" alt="Icon: Complete image"></td>
<td><img class="valignt" src="../images/ecc.png" alt="Icon: Separate file with error correction data"></td>
<td>By using the error correction data Jane
    <a href="howtos42.php#b">recovers the missing parts in the ISO image</a>.
<tr>
 <td align="right">Write new CD</td>
 <td align="center"><img class="valignt" src="../images/down-arrow.png" alt="Icon: Arrow down"></td>
 <td></td><td></td>
</tr>
<tr><td colspan="4"> <p> </td></tr>
<tr>
<td> </td>
<td><img class="valignt" src="../images/good-cd.png" alt="Icon: Good medium (without read errors)"></td>
<td><img class="valignt" src="../images/ecc.png" alt="Icon: Separate file with error correction data"></td>
<td>Jane writes a new CD from the recovered ISO image. She keeps the
error correction data for the new CD as it may also become defective
in the future.</td>
</table>


<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
