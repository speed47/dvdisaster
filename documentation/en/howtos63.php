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
begin_page();
?>

<!--- Insert actual page content below --->

<h3>Using dvdisaster the wrong way</h3>

Joe bets on his media keeping their content
without additional protection.<p>

<table width="100%">
<tr>
<td width="15%">10. Feb. 2004</td>
<td width="60px"><img src="../images/good-cd.png"></td>
<td width="60px"><img src="../images/good-cd.png"></td>
<td>Joe creates two CDs containing important data. But he does not
make any provisions against data loss on those media.</td>
</tr>
<tr><td colspan="4"> <hr> </td></tr>
<tr>
<td>14. May 2005</td>
<td><img align="top" src="../images/good-cd.png"></td>
<td><img align="top" src="../images/good-cd.png"></td>
<td>Joe uses his CDs regularly. After one year they are still perfectly 
readable.</td>
</tr>
<tr><td colspan="4"> <hr> </td></tr>
<tr>
<td>19. Aug 2007</td>
<td><img align="top" src="../images/bad-cd.png"></td>
<td><img align="top" src="../images/good-cd.png"></td>
<td>After two more years Joe notices that some data on one CD is no
longer readable.</td> 
</tr>
<tr>
 <td align="right"><a href="howtos10.php">scanning</a></td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td></td>
</tr>
<tr>
<td>20. Aug 2007</td>
<td><img align="top" src="../images/bad-cd.png"></td>
<td><img align="top" src="../images/bad-cd.png"></td>
<td>Joe downloads dvdisaster and performs
<a href="howtos10.php">a read error scan</a>. 
He finds out that the CD contains 25000 unreadable sectors. A scan
of the second CD reveals that it has developed 1500 unreadable sectors
gone unnoticed so far. </td>
</tr>
<tr>
 <td align="right">reading</td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td></td>
</tr>
<tr><td colspan="4"> <p> </td></tr>
<tr>
<td>21. Aug 2007</td>
<td><img align="top" src="../images/bad-image.png"></td>
<td><img align="top" src="../images/bad-image.png"></td>
<td>Joe uses dvdisaster to 
read as much sectors as possible from the 
defective media. But since he does not have error correction data
there is no way of recalculating the unreadable sectors.</td>
</tr>
<tr>
 <td align="right">many reading<br>attempts</td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td></td>
</tr>
<tr><td colspan="4"> <p> </td></tr>
<tr>
<td>05. Sep 2007</td>
<td><img align="top" src="../images/bad-image.png"></td>
<td><img align="top" src="../images/good-image.png"></td>
<td>Joe takes advantage of dvdisaster's feature to complete images 
through multiple reading passes. He moves the defective images to
several computers to perform reading attempts with different drives.
After two weeks of trying at least all missing sectors from the second
CD have been read. However on the first CD still 21000 sectors
remain unreadable by any drive he tried.</td>
</tr>
<tr>
 <td align="right">only one CD<br>recovered</td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td></td>
</tr>
<tr><td colspan="4"> <p> </td></tr>
<tr>
<td>06. Sep 2007</td>
<td><img align="top" src="../images/bad-cd.png"></td>
<td><img align="top" src="../images/good-cd.png"></td>
<td>Joe dismisses the first CD as unrecoverable and considers himself lucky
to have a complete image from the second CD again.
However if he had created error correction data in time, he'd probably <sup>1)</sup>
saved two weeks of reading attempts and recovered the contents from both CDs.</td></tr>
</table>
<hr>
<sup>1)</sup>The error correction assumes a typical aging process.
If the CD gets severely damaged it becomes unrecoverable even with
error correction data. Do not rely on dvdisaster alone for protecting
important data; instead employ additional measures like creating 
additional copies on different types of media.

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
