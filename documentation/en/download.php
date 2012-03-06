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
require("../include/footnote.php");
begin_page();
$show_all=$_GET["showall"];
?>

<!--- Insert actual page content below --->

<h3>Download dvdisaster</h3>

dvdisaster is available for <a href="download10.php">recent versions</a>
of the FreeBSD, GNU/Linux, Mac OS X(Darwin), NetBSD 
and Windows operating systems. It is provided
as <a href="http://www.germany.fsfeurope.org/documents/freesoftware.en.html">free Software</a> 
under the <a href="http://dvdisaster.cvs.sourceforge.net/dvdisaster/dvdisaster/COPYING?view=markup">GNU General Public License v2</a><a href="#gpl3"><sup>*)</sup></a>.<p>

Please download either the source code or a binary version from the list below.
A <a href="download20.php">digital signature</a> is provided for verification that the
packets are in their original state.<p>

<ul>
<li>The source code version contains a file <tt>INSTALL</tt> with further instructions for compiling it.</li>
<li>For Mac OS X a ZIP archive is provided which is installed by unpacking it
into an arbitrary place. Please note the 
<a href="download30.php#mac">special hints for Mac OS X</a>.</li>
<li>To <a href="download30.php#win">install</a> the binary version for Windows
please execute the downloaded program and follow the dialog.</li>
</ul>

<!---
<b>Alpha (unstable) versions</b> - new and experimental for experienced users!<p>

Testers are welcome for the upcoming dvdisaster release, but should be aware of remaining
bugs and incompatibilities.
The current unstable release is 
<a href="download40.php"><?php echo $cooked_version?></a>.
-->

<pre> </pre>

<b>Stable version</b> - recommended for getting started.<p>

<table class="download" cellpadding="0" cellspacing="5">
<tr><td><b>dvdisaster-0.72</b></td><td align="right">05-Oct-2011</td></tr>
<tr bgcolor="#000000"><td colspan="2"><img width=1 height=1 alt=""></td></tr>
<tr><td colspan="2">
  <table>
    <tr><td align="right">&nbsp;&nbsp;Source code for all operating systems:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.3.tar.bz2">dvdisaster-0.72.3.tar.bz2</a></td></tr>
    <tr><td align="right">Digital signature:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.3.tar.bz2.gpg">dvdisaster-0.72.3.tar.bz2.gpg</a></td></tr>
    <tr><td align="right">Binary for Mac OS X 10.5 / x86:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.3.app.zip">dvdisaster-0.72.3.app.zip</a>&nbsp;--&nbsp;please read these <a href="download30.php#mac">hints</a> first</td></tr>
    <tr><td align="right">Digital signature:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.3.app.zip.gpg">dvdisaster-0.72.3.app.zip.gpg</a></td></tr>
    <tr><td align="right">Binary for Windows:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.3-setup.exe">dvdisaster-0.72.3-setup.exe</a></td></tr>
    <tr><td align="right">Digital signature:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.3-setup.exe.gpg">dvdisaster-0.72.3-setup.exe.gpg</a></td></tr>
    <tr><td colspan="2"> </td></tr>
<?php
  if($show_all == 0) {
?>
    <tr><td colspan="2"><a href="download.php?showall=1">Show older releases in the 0.72 version branch</a></td></tr>
<?php
  }
  else {
?> 
   <tr><td colspan="2"><a href="download.php?showall=0">Hide older releases in the 0.72 version branch</a></td></tr>

    <tr><td align="right">&nbsp;&nbsp;Source code for all operating systems:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.2.tar.bz2">dvdisaster-0.72.2.tar.bz2</a></td></tr>
    <tr><td align="right">Digital signature:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.2.tar.bz2.gpg">dvdisaster-0.72.2.tar.bz2.gpg</a></td></tr>
    <tr><td align="right">Binary for Mac OS X 10.5 / x86:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.2.app.zip">dvdisaster-0.72.2.app.zip</a>&nbsp;--&nbsp;please read these <a href="download30.php#mac">hints</a> first</td></tr>
    <tr><td align="right">Digital signature:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.2.app.zip.gpg">dvdisaster-0.72.2.app.zip.gpg</a></td></tr>
    <tr><td align="right">Binary for Windows:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.2-setup.exe">dvdisaster-0.72.2-setup.exe</a></td></tr>
    <tr><td align="right">Digital signature:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.2-setup.exe.gpg">dvdisaster-0.72.2-setup.exe.gpg</a></td></tr>
    <tr><td colspan="2"> </td></tr>
    <tr><td align="right">&nbsp;&nbsp;Source code for all operating systems:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.1.tar.bz2">dvdisaster-0.72.1.tar.bz2</a></td></tr>
    <tr><td align="right">Digital signature:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.1.tar.bz2.gpg">dvdisaster-0.72.1.tar.bz2.gpg</a></td></tr>
    <tr><td align="right">Binary for Mac OS X 10.5 / x86:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.1.app.zip">dvdisaster-0.72.1.app.zip</a>&nbsp;--&nbsp;please read these <a href="download30.php#mac">hints</a> first</td></tr>
    <tr><td align="right">Digital signature:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.1.app.zip.gpg">dvdisaster-0.72.1.app.zip.gpg</a></td></tr>
    <tr><td align="right">Binary for Windows:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.1-setup.exe">dvdisaster-0.72.1-setup.exe</a></td></tr>
    <tr><td align="right">Digital signature:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.1-setup.exe.gpg">dvdisaster-0.72.1-setup.exe.gpg</a></td></tr>
    <tr><td colspan="2"> </td></tr>
    <tr><td align="right">&nbsp;&nbsp;Source code for all operating systems:&nbsp;</td>
        <td><a href="http://prdownloads.sourceforge.net/dvdisaster/dvdisaster-0.72.tar.bz2?download">dvdisaster-0.72.tar.bz2</a></td></tr>
    <tr><td align="right">Digital signature:&nbsp;</td>
        <td><a href="http://prdownloads.sourceforge.net/dvdisaster/dvdisaster-0.72.tar.bz2.gpg?download">dvdisaster-0.72.tar.bz2.gpg</a></td></tr>
    <tr><td align="right">Binary for Mac OS X 10.5 / x86:&nbsp;</td>
        <td><a href="http://prdownloads.sourceforge.net/dvdisaster/dvdisaster-0.72.app.zip?download">dvdisaster-0.72.app.zip</a>&nbsp;--&nbsp;please read these <a href="download30.php#mac">hints</a> first</td></tr>
    <tr><td align="right">Digital signature:&nbsp;</td>
        <td><a href="http://prdownloads.sourceforge.net/dvdisaster/dvdisaster-0.72.app.zip.gpg?download">dvdisaster-0.72.app.zip.gpg</a></td></tr>
    <tr><td align="right">Binary for Windows:&nbsp;</td>
        <td><a href="http://prdownloads.sourceforge.net/dvdisaster/dvdisaster-0.72-setup.exe?download">dvdisaster-0.72-setup.exe</a></td></tr>
    <tr><td align="right">Digital signature:&nbsp;</td>
        <td><a href="http://prdownloads.sourceforge.net/dvdisaster/dvdisaster-0.72-setup.exe.gpg?download">dvdisaster-0.72-setup.exe.gpg</a></td></tr>
<?php
  }
?>
  </table>
</td></tr>
<tr bgcolor="#000000"><td colspan="2"><img width=1 height=1 alt=""></td></tr>
<tr><td colspan="2">
Most important changes in this version:<p>
<ul>
<li>Support for <a href="qa10.php#media">Blu-Ray media</a></li>
<li>"Raw" reading and C2 scans for CD media</li>
<li>Selectable number of reading attempts</li>
<li>First native Mac OS X application bundle</li>
<li>NetBSD-Port by Sergey Svishchev</li>
<li>Improved media type recognition</li>
<li>Info window describing inserted medium</li>
<li>Improved and enhanced preferences dialog</li>
<li>Reworked and enhanced documentation</li>
<li>Russian translations by Igor Gorbounov</li>
<li>... and many more small changes and fixes.</li>
</ul>

<b>Patches</b> (small changes after version 0.72; files above have been updated):<p>

<b>0.72 pl3</b> Fixed a problem with the "Verify" function when
 working with RS01 error correction files larger than 2GB.
Thanks to Volodymyr Bychkoviak for reporting the problem and
sending a fix. (05-Oct-2011)<p>

<b>0.72 pl2</b> 
This version introduces a workaround which prevents parallel SCSI
adapters from freezing under Linux. 
Improved compatibility with the new version branch 0.79.x. (31-Oct-2010)<p>

<b>0.72 pl1</b> Pablo Almeida provided Portuguese translations of the screen texts.
Added workaround to avoid Win XP freezing on certain CD-RW/drive pairs.(08-Aug-2009)<p>

<b>0.72</b> This is the first stable version of the 0.72 branch.
Igor Gorbounov has completed the russian online documentation.
Some minor bugs from the first release candidate have been removed.<p>
Newer Windows versions may not display the proper screen language
under certain language settings. This problem is rather complex
and will be addressed in the upcoming 0.73.x releases. (04-Jul-2009)
<p>
 
<b>0.72-rc1</b> First release candidate. (11-Apr-2009)
</td></tr></table><p>

<b>Previous release</b> - upgrading to version 0.72 is recommended.<p>

<table class="download" cellpadding="0" cellspacing="5">
<tr><td><b>dvdisaster-0.70</b></td><td align="right">04-Mar-2008</td></tr>
<tr bgcolor="#000000"><td colspan="2"><img width=1 height=1 alt=""></td></tr>
<tr><td colspan="2">
  <table>
    <tr><td align="right">&nbsp;&nbsp;Source code for all operating systems:&nbsp;</td>
        <td><a href="http://prdownloads.sourceforge.net/dvdisaster/dvdisaster-0.70.6.tar.bz2?download">dvdisaster-0.70.6.tar.bz2</a></td></tr>
    <tr><td align="right">Digital signature:&nbsp;</td>
        <td><a href="http://prdownloads.sourceforge.net/dvdisaster/dvdisaster-0.70.6.tar.bz2.gpg?download">dvdisaster-0.70.6.tar.bz2.gpg</a></td></tr>
    <tr><td align="right">Binary for Windows:&nbsp;</td>
        <td><a href="http://prdownloads.sourceforge.net/dvdisaster/dvdisaster-0.70.6-setup.exe?download">dvdisaster-0.70.6-setup.exe</a></td></tr>
    <tr><td align="right">Digital signature:&nbsp;</td>
        <td><a href="http://prdownloads.sourceforge.net/dvdisaster/dvdisaster-0.70.6-setup.exe.gpg?download">dvdisaster-0.70.6-setup.exe.gpg</a></td></tr>
  </table>
</td></tr>
<tr bgcolor="#000000"><td colspan="2"><img width=1 height=1 alt=""></td></tr>
<tr><td colspan="2">
The RS02 error correction method
is fully supported in the graphical user interface. Images created
with RS02 can be used 
with the adaptive reading strategy.<p>

Julian Einwag started porting dvdisaster 
to Mac OS X / Darwin.<p>

Daniel Nylander provided Swedish translations of the screen texts.<p>

<b>Patches</b> (small changes after version 0.70; files above have been updated):<p>

<b>pl6</b> Rolled back support for localized file names
as it broke large file support under Windows. A new handler
for localized file names will first be tested in the experimental version
0.71.25. <i>(04-Mar-2008)</i><p>

<b>pl5</b> 
Fixes a problem with newer Linux kernels which may lead to a frozen system under
some circumstances. Treatment of file names containing localized characters has
been improved. 
Contains backports of major bug fixes from 0.71.24. <i>(24-Feb-2008)</i>.<p>

<b>pl4</b> provides better compatibility with two layered DVDs 
(DVD-R DL and DVD+R DL).<br> 
Some minor bugs have been fixed. <i>(20-Jan-2007)</i>.<p>

<b>pl3</b> fixes incorrect recognization of unsupported CD formats which
produced a Windows blue screen under rare circumstances. Added cancel option
during RS02 initialization on DVD RW media.
<i>(10-Dec-2006)</i>.<p>

<b>pl2</b> fixes incorrect memory freeing when the program window is closed.
Unpacking of documentation screen shots is fixed for PPC platforms.
Only the source code archives have been updated.
<i>(03-Oct-2006)</i>.<p>

<b>pl1</b> fixes a bug in adaptive reading for RS02 which would sometimes result in
reading insufficient data for a successful recovery. Adds some small
improvements of documentation and usability. <i>(30-Jul-2006)</i>
</td></tr></table><p>


The dvdisaster source code is also
<a href="http://sourceforge.net/cvs/?group_id=157550">available via CVS</a>.
Some file of particular interest are:
<ul>
<li><a href="http://dvdisaster.cvs.sourceforge.net/dvdisaster/dvdisaster/CHANGELOG?view=markup">CHANGELOG</a>- changes from the previous versions;</li>
<li><a href="http://dvdisaster.cvs.sourceforge.net/dvdisaster/dvdisaster/CREDITS.en?view=markup">CREDITS.en</a>- people involved in the project;</li>
<li><a href="http://dvdisaster.cvs.sourceforge.net/dvdisaster/dvdisaster/INSTALL?view=markup">INSTALL</a> - additional hints for installation;</li>
<li><a href="http://dvdisaster.cvs.sourceforge.net/dvdisaster/dvdisaster/README?view=markup">README</a> - a road map of the source code archive.</li>
</ul>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
