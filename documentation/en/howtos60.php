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

<h3 class="top">The big picture - a comparison of dvdisaster with conventional backup</h3>

dvdisaster stores data on CD/DVD/BD in a way that the data is fully recoverable even after
the medium has developed some read errors. The method employed in dvdisaster uses
less storage space (or additional media) than a full backup would do.
Before using dvdisaster it is important to understand the similarities and 
differences between dvdisaster and a conventional (full) backup:<p> 

Let's first consider how a conventional backup scheme works:<p>

<table width="100%">
<tr>
<td class="w65x"><img src="../images/backup1.png" alt="Icon: Original medium"></td>
<td class="w65x">Copy<br><img src="../images/right-arrow.png" alt="Icon: Arrow right"></td>
<td class="w65x"><img src="../images/backup2.png" alt="Icon: Backup medium"></td>
<td> &nbsp; </td>
<td>An existing medium (1) is copied onto a backup medium (2).</td>
</tr>

<tr>
<td align="center"><img src="../images/down-arrow.png" alt="Icon: Arrow down">&nbsp;&nbsp;</td>
<td></td>
<td align="center"><img src="../images/down-arrow.png" alt="Icon: Arrow down">&nbsp;&nbsp;</td>
<td> </td>
</tr>

<tr>
<td class="w65x"><img src="../images/bad-cd1.png" alt="Icon: Damaged medium"></td>
<td class="w65x"> </td>
<td class="w65x"><img src="../images/backup2.png" alt="Icon: Backup medium"></td>
<td></td>
<td>If any one of the two media is damaged afterwards, we still have
an intact medium left.</td>
</tr>
</table><p>

There are actually some cases where it is important to keep a second copy of
a CD/DVD/BD: One medium might get lost, burst while spinning in the drive,
or it may be destroyed due to mishandling. However such cases of complete 
data loss are rare as long as media are handled properly.<p>

It is more likely that the medium starts to gradually lose data 
after a few years - a nearly unavoidable aging process.
When the medium is regularly used (or scanned for defects) the data loss
will typically be noticed after 5% to 10% of the medium have already become
unreadable. At this point the medium is unusable as a whole, 
but maybe 90% of it is still readable. <i>On the other hand a full backup copy of the 
medium is not required; we simply need a method for recovering the
missing 10% of data.</i><p>

This is where dvdisaster comes into play. Consider this:<p>

<table width="100%">
<tr>
<td class="w65x"><img src="../images/good-cd.png" alt="Icon: Good medium (without read errors)"></td>
<td class="w65x">Create<br><img src="../images/right-arrow.png" alt="Icon: Arrow right"><br>ECC</td>
<td class="w65x"><img src="../images/ecc.png" alt="Icon: Separate file with error correction data"></td>
<td> &nbsp; </td>
<td>
This time we do not make a full backup. dvdisaster is used to create error correction data
("ECC") which can recover up to 20% of a degraded medium.
The value of 20% was chosen to have a safety margin over the expected data loss of 5-10%.  
</td>
</tr>

<tr>
<td align="center"><img src="../images/down-arrow.png" alt="Icon: Arrow down">&nbsp;&nbsp;</td>
<td></td>
<td align="center"><img src="../images/down-arrow.png" alt="Icon: Arrow down">&nbsp;&nbsp;</td>
<td> </td>
</tr>

<tr>
<td><img src="../images/bad-cd.png" alt="Icon: Damaged medium (partially unreadable)"></td>
<td> </td>
<td><img src="../images/ecc.png" alt="Icon: Separate file with error correction data"></td>
<td> &nbsp; </td>
<td>
Wenn the medium fails at a later time,
its contents are recovered from its still readable parts and from the
error correction data.
</td>
</tr>

<tr>
<td align="right" class="w65x">80%<img src="../images/rdiag-arrow.png" alt="Icon: Diagonal arrow right"></td>
<td> </td>
<td align="left" class="w65x"><img src="../images/ldiag-arrow.png" alt="Icon: Diagonal arrow left">20%</td>
<td> </td>
<td>
For a successful recovery at least 80% of the data must still be readable from the medium,
and the remaining 20% are recalculated from the error correction data.</td>
</tr>

<tr>
<td> </td>
<td> <img src="../images/good-image.png" alt="Icon: Complete image"></td>
<td> </td>
<td> </td>
<td>The completely recovered data is now available as an ISO image on the hard drive
(the medium remains defective as physical data loss is irrevocable). 
</td>
</tr>

<tr>
<td> </td>
<td align="center"><img src="../images/down-arrow.png" alt="Icon: Arrow down"></td>
<td> </td>
<td> </td>
<td>Write the image to a blank medium using your favourite CD/DVD/BD authoring software.</td>
</tr>

<tr>
<td> </td>
<td align="center"><img src="../images/good-cd.png" alt="Icon: Good medium (without read errors)"></td>
<td> </td>
<td> </td>
<td>You now have a new error-free medium.</td>
</tr>
</table><p>

As you have seen the data recovery took more steps then doing a conventional backup.
So let's summarize the pros and cons of dvdisaster compared with conventional backup:<p>

<table>
<tr valign="top"><td>Advantages</td>
<td><ul>
<li>dvdisaster uses less storage. When using error correction data with a 20%
recovery capability, protecting 5 media requires only one additional medium for
the ECC data.</li>
<li>Since all media will eventually age and start losing data in similar places
(typically in the outermost region), doing a 1:1 copy might not help at all.
Both copies may turn out defective in the same places after a few years.</li>
</ul></td></tr>
<tr valign="top"><td>Similarities</td>
<td><ul><li>Both backup copies and error correction data must be created
before the master disc fails. You can't create them from an already defective
medium.</li></ul></td></tr>
<tr valign="top"><td>Disadvantages</td>
<td><ul><li>If the recovery capability of the error correction data is exceeded
(or the medium gets lost), no data can be recovered!
Especially take note that error correction data with a repair rate of 20% together
with a 75% readable the medium does not result in 95% recovery! In that case,
nothing beyond the 75% readable data from the medium can be recovered!</li></ul></td></tr>
</table> 

The next three pages provide more related information:<p>

<ul>
<li>The general idea of the <a href="howtos61.php">error correction</a> is explained.<p></li>
<li>Jane demonstrates the
<a href="howtos62.php">proper usage of dvdisaster</a>. She will create error correction
data in advance and is therefore able to recover all data when her media become defective.<p></li>
<li>However you should <a href="howtos63.php">not follow the way</a> of Joe. 
He does not use error correction data and finds out that his defective
media are not recoverable even after multiple reading passes. As a consequence he loses
data from a defective medium.<p></li>
</ul>

Of course these stories are purely fictional and any similarities with existing
persons or situations are purely conincidental.


<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
