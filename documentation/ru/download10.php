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

<h3 class="top">Hardware requirements</h3>

<ul>
 <li>x86, PowerPC or Sparc processor;</li>
 <li>an up-to-date CD/DVD/BD drive with ATAPI, SATA or SCSI interface;</li>
 <li>enough hard disk space for creating .iso images from processed media.
</ul>
<p>

<h3>Supported operating systems</h3>
The following table gives an overview of the supported operating
systems. The specified releases have been used for developing and
testing the current dvdisaster version. Typically, slightly older
and newer OS versions will also work.<p>

The dvdisaster project recommends GNU/Linux.<p>

<table border="1">
<tr>
<td>Operating System</td>
<td>Release</td>
<td>32bit support</td>
<td>64bit support</td>
</tr>
<tr>
<td>GNU/Linux</td>
<td>Debian Squeeze (6.0.4)<br>Kernel 2.6.32</td>
<td align="center">yes</td>
<td align="center">yes</td>
</tr>
<tr>
<td>FreeBSD<sup>1)</sup></td>
<td>9.0</td>
<td align="center">yes</td>
<td align="center">yes</td>
</tr>
<tr>
<td>NetBSD</td>
<td>5.1.2</td>
<td align="center">yes</td>
<td align="center">yes</td>
</tr>
<tr>
<td>Mac OS X</td>
<td>10.6 (Snow Leopard)</td>
<td align="center">yes</td>
<td align="center">no<sup>2)</sup></td>
</tr>
<tr>
<td>Windows<sup>4)</sup></td>
<td>Windows 2000 SP4<sup>3)</sup></td>
<td align="center">yes</td>
<td align="center">no<sup>2)</sup></td>
</tr>
</table><p>

<sup>1)</sup>FreeBSD: using ATAPI drives requires loading the <i>atapicam</i> kernel module -- see INSTALL doc<br>
<sup>2)</sup>64bit Support not planned.<br>
<sup>3)</sup>Later Versions up to Windows 7 have been reported to work.
Windows 2000 SP3 and earlier versions are not supported.<br>
<sup>4)</sup>Support for multicore processors varies. On some editions using additional
cores for dvdisaster does not result in more performance.
<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
