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

<h3 class="top">Digital signature</h3>

The downloadable dvdisaster packages have been digitally signed using
<a href="http://www.gnupg.org/gnupg.html">GnuPG</a> so that you can verify
that the software is in its original state.<p>


The signature has been made with the following <a href="../pubkey.asc">public key</a>:

<pre>
pub   1024D/F5F6C46C 2003-08-22
      Key fingerprint = 12B3 1535 AF90 3ADE 9E73  BA7E 5A59 0EFE F5F6 C46C
uid                  dvdisaster (pkg signing key #1)
sub   1024g/091AD320 2003-08-22
</pre>

Feel free to send an email to <img src="../images/email.png" alt="email address display as graphics image" class="valigntt"> to obtain 
the fingerprint directly from the developers. 
Please include "GPG finger print" in the subject line.

<h3>MD5 checksum</h3>

Contrary to the digital signature, MD5 checksums are cryptographically weak:
It is possible to create a manipulated package which still has the same
checksum as the original. However MD5 checksums are sufficient for a quick
check whether the download has finished completely and without transmission
errors.

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
