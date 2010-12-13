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

<h3>Make sure you're not getting ripped off: The small print (and other things).</h3>

The dvdisaster project provides this software 
as <a href="http://www.gnu.org/philosophy/free-sw.html">free software</a> 
to you using the
<a href="http://dvdisaster.cvs.sourceforge.net/dvdisaster/dvdisaster/COPYING?view=markup">GNU General Public License v2</a>. <p>

The dvdisaster project also wants to make sure that you know
you can download the software <a href="download.php">from here</a> at no cost
and keeping your full privacy.<p>

To make it clear how we distribute dvdisaster, what we do and what we won't do,
we have compiled the following list:<p>

<b>Internet and download sites</b><p>

The dvdisaster project uses the following internet domains
for publishing its web sites and supplying software downloads:<p>

dvdisaster.com<br>
dvdisaster.de<br>
dvdisaster.net<br>
dvdisaster.org<p>

All domains are forwarded to the same site at dvdisaster.net.<br> 
In addition, the dvdisaster project is using the hosting facilities of 
<a href="http://sourceforge.net/projects/dvdisaster">SourceForge.net</a>.<p>

No other internet or download sites are run by the dvdisaster project.<p>

<b>No money or personal data required</b><p>

There is <b>no registration</b> process for using this software. <br>
The dvdisaster project <b>never</b> asks you to enter personal data, 
to pay a fee or to donate money for:

<ul>
<li>using this web site,</li>
<li>downloading the software, and</li>
<li>running the software.</li>
</ul><p>

<b>Cryptographic signature and checksums</b><p>

dvdisaster releases are always published with 
<a href="download20.php">cryptographic signatures</a> and md5 checksums. 
See the <a href="download.php#download">download page</a> for examples. <p>

Be very cautious if signatures and checksums are missing, invalid or
not matching those published at the sites mentioned above.<p>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
