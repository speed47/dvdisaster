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

<!-- Insert actual page content here -->

<h3 class="top">The dvdisaster project:</h3>

Optical media (CD,DVD,BD) keep their data only for a 
finite time (typically for many years).
After that time, data loss develops slowly with read errors 
growing from the outer media region towards the inside.<p>

<b>Archival with data loss protection</b><p>

dvdisaster stores data on CD/DVD/BD (<a href="qa10.php#media">supported media</a>)
in a way that it is fully recoverable even after some read errors have developed. 
This enables you to rescue the complete data to a new medium.<p>

Data loss is prevented by 
using error correcting codes.
Error correction data is either added to the medium or kept in separate
error correction files. dvdisaster works at 
the image level so that the recovery 
does not depend on the file system of the medium.
The maximum error correction capacity is user-selectable.<p>

<b>Common misunderstandings about dvdisaster:</b>

<ul>
<li>dvdisaster can not make defective media readable again.
Contents of a defective medium can <i>not</i> be recovered without the
error correction data.<p></li> 
<li><img src="images/exclude_from_search_terms.png" alt="" class="valignm"><br>
Such functions are outside the scope of dvdisaster's internal design and goals.</li>
</ul>

<p>
<a href="index10.php">Examples of the error correction...</a>

<?php
# Adds the footer line and closes the HTML properly.

end_page();
?>
