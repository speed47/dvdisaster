<?php
# dvdisaster: English homepage translation
# Copyright (C) 2004-2009 Carsten Gnörlich
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

<h3>Alpha (developer) versions</h3>

<b>Help us testing!</b> This page contains experimental dvdisaster versions
which are created on the way to the next stable release.<p>

<b>A word of caution:</b> Alpha versions are not thoroughly tested. They
may contain more errors than a stable version and should not be used
to process important data.<p>

If in doubt please continue using the <a href="download.php">stable version 0.72</a>
and wait for the release of version 0.74.

<hr>

<h3>Downloads</h3>

The alpha versions use the same package format as the regular releases.<p>

<table class="download" cellpadding="0" cellspacing="5">
<tr><td><b>dvdisaster-0.73 (devel1)</b></td><td align="right">xx-XXX-2009</td></tr>
<tr bgcolor="#000000"><td colspan="2"><img width=1 height=1 alt=""></td></tr>
<tr><td colspan="2">
  <table>
    <tr><td align="right">&nbsp;&nbsp;Source code for all operating systems:&nbsp;</td>
        <td><a href="http://prdownloads.sourceforge.net/dvdisaster/dvdisaster-0.73.1.tar.bz2?download">dvdisaster-0.73.1.tar.bz2</a></td></tr>
    <tr><td align="right">Digital signature:&nbsp;</td>
        <td><a href="http://prdownloads.sourceforge.net/dvdisaster/dvdisaster-0.73.1.tar.bz2.gpg?download">dvdisaster-0.73.1.tar.bz2.gpg</a></td></tr>
    <tr><td align="right">Binary for Windows:&nbsp;</td>
        <td><a href="http://prdownloads.sourceforge.net/dvdisaster/dvdisaster-0.73.1-setup.exe?download">dvdisaster-0.73.1-setup.exe</a></td></tr>
    <tr><td align="right">Digital signature:&nbsp;</td>
        <td><a href="http://prdownloads.sourceforge.net/dvdisaster/dvdisaster-0.73.1-setup.exe.gpg?download">dvdisaster-0.73.1-setup.exe.gpg</a></td></tr>
  </table>
</td></tr>
<tr bgcolor="#000000"><td colspan="2"><img width=1 height=1 alt=""></td></tr>
<tr><td colspan="2">
Not yet released.
</td></tr></table><p>


<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
