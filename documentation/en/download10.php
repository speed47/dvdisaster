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

<h3>System requirements</h3>

<ul>
 <li>Processors: x86, PowerPC or Sparc;<p></li>
 <li>with processing speed equal or better than a P4 at 2Ghz;<p></li>
 <li>an up-to-date CD/DVD/BD drive with ATAPI or SCSI interface;<p></li>
 <li>enough hard disk space for creating .iso images from processed media.<p>
</ul>

<h3>Operating systems</h3>
<ul>
 <li><a name="#freebsd"></a><b>FreeBSD</b> version <b>6.0</b> or later<br>
     (using ATAPI drives requires loading the <i>atapicam</i> kernel module -- see INSTALL doc)<p>
 </li>
 <li><b>GNU/Linux</b> with kernel <b>2.6.7</b> or later.<p>
 </li>
 <li><b>Mac OS X</b> version 10.6 or later,<br> 
      on x86 and PowerPC hardware.<p>
 <li><b>NetBSD</b> version 3.1 or later.<p></li> 
 <li><b>Windows 2000</b>, <b>Windows XP</b> oder <b>Windows Vista (R).</b></li>
 </ul>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
