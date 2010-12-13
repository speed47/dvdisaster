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
require("../include/download.php");
begin_page();
$show_all=$_GET["showall"];
?>

<!--- Insert actual page content below --->

<h3>Alpha (developer) versions</h3>

<b>Help us testing!</b> This page contains experimental dvdisaster versions
which are created on the way to the next stable release.<p>

<b>A word of caution:</b> This version is still evolving and some parts
are not yet implemented. It may contain severe bugs and fail in non-obvious
ways, even in functions which worked in previous versions. 
Do not process important data with this version and do not keep images and
error correction data for archival purposes;
that's what the <a href="download.php">stable version 0.72</a>
is for.

<hr>

<h3>Planned changes in the new version</h3>

All platforms:

<ul>
<li> Implement some small additions which have been put on hold
during the 0.72 development cycle. <i>[not yet started]</i></li>
<li> Remove obsolete functionality. <i>[completed]</i></li>
<li> Clean up source code and prepare for multithreading and multi core 
processors. <i>[in progress]</i></li>
<li> Implement the multithreaded RS03 codec. <i>[in progress]</i></li>
<li> Document RS03 usage. <i>[not yet started]</i></li>
</ul>

Windows:

<ul>
<li> Update the GTK+ toolkit and development system. <i>[completed]</i></li>
<li> Raise system requirements to Windows 2000 or newer (older
Windows releases are no longer supported by the development tools).
This makes support for ASPI drivers and splitting files into 2G
segments obsolete. <i>[completed]</i></li>
</ul>

MacOS:

<ul>
<li> Update the GTK+ toolkit and provide more workarounds
for the graphical user interface. <i>[in progress]</i></li>
</ul>

<hr>

<h3>Downloads</h3>
<a name="download">

The alpha versions use the same package format as the regular releases.<p>

<table class="download" cellpadding="0" cellspacing="5">
<tr><td><b>dvdisaster-0.79</b></td><td align="right">21-Nov-2010</td></tr>
<tr bgcolor="#000000"><td colspan="2"><img width=1 height=1 alt=""></td></tr>
<tr><td colspan="2">
  <table>
<?php
    download_version("0.79.3", 0, "hidden", "hidden", "hidden");

    if($show_all == 0) 
    {  echo "    <tr><td colspan=\"2\"><a href=\"download40.php?showall=1#download\">Show older releases in the 0.79 version branch</a></td></tr>\n";
    }
    else 
    {  echo "   <tr><td colspan=\"2\"><a href=\"download40.php?showall=0#download\">Hide older releases in the 0.79 version branch</a></td></tr>\n";
       echo "    <tr><td colspan=\"2\"> </td></tr>\n";

       download_version("0.79.2", 1, "378ed135c2faf0eaf643125d1f7726c6", "f673e41b5ddc31a6ecb48a5f053de885", "0b4c0b46e827c7f796416473511ab036");

       download_version("0.79.1", 1, "ba6d0178dc03119080e07ef0a2967c38", "none", "b4c62833a2447097950b563e4a7b2065");
   }
?>
  </table>
</td></tr>
<tr bgcolor="#000000"><td colspan="2"><img width=1 height=1 alt=""></td></tr>
<tr><td colspan="2">

<b>All platforms:</b> These releases contain major internal changes compared
to 0.72.x. Please use them carefully.<p>

<b>0.79.3</b> (21-Nov-2010)<br>
<ul>
<li>GNU/Linux: Starting with this version the SG_IO driver is used by default
for accessing optical drives; the previously used
CDROM_SEND_PACKET driver can be selected optionally. 
Driver defaults were the other way around in previous versions;
but in recent Linux kernels the SG_IO driver provides better 
compatibility.</li>
<li>Michael Klein provided Altivec optimization for the RS03 codec.
</li>
</ul>

<b>0.79.2</b> (28-Feb-2010)<br>
<ul>
<li>A binary package for Mac OS X is available now. The Mac OS X
development environment has been updated; this removed some glitches
in the graphical user interface.
</li>
<li>
Development of the RS03 codec makes progress, but is far from being
finished yet.
</li>
</ul>

<b>0.79.1</b> (07-Feb-2010)<br>
<ul>
<li>The SCSI layer contains a workaround for buggy chipsets found in recent
drives. Starting a read or scan operation would case a system freeze with
such drives. The problem seems to be especially visible under Windows XP, 
but other OS might expose similar failures. 
Please test if these drives are working
now, and also report if some drives stopped working which were okay previously.</li> 
<li>A reference implementation of the RS03 codec is
included. This version is only supplied so that interested people can
compare it against its <a href="download50.php">specification</a>.
Take care and do not use it for productive work. The final version will
be released with version 0.80.</li>
</ul>
<b>Windows:</b> All components of the development environment and the
supplied libraries have been updated. Please test whether the graphical
user interface and localization still work as expected.
</td></tr></table><p>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
