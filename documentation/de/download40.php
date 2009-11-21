<?php
# dvdisaster: German homepage translation
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

<h3>Alpha-/Entwicklerversionen</h3>

<b>Helfen Sie beim Testen!</b> Hier finden Sie experimentelle 
dvdisaster-Versionen, die auf dem Weg zur nächsten "stabilen" Version
entstehen.<p>

<b>Ein Wort der Vorsicht:</b> Alpha-Versionen sind nicht so intensiv getestet
wie die stabilen Versionen. Sie können mehr Fehler enthalten und 
sollten nicht zum Bearbeiten von wichtigen Daten
verwendet werden.<p>

Verwenden Sie im Zweifelsfall die <a href="download.php">stabile Version 0.72</a>
und warten Sie auf die Veröffentlichung von Version 0.74.

<hr>

<h3>Herunterladen</h3>

Die Alpha-Versionen verwenden das gleiche Paketformat wie die normalen
Versionen.<p>

<table class="download" cellpadding="0" cellspacing="5">
<tr><td><b>dvdisaster-0.73 (devel1)</b></td><td align="right">xx-XXX-2009</td></tr>
<tr bgcolor="#000000"><td colspan="2"><img width=1 height=1 alt=""></td></tr>
<tr><td colspan="2">
  <table>
    <tr><td align="right">&nbsp;&nbsp;Quellkode für alle Betriebssysteme:&nbsp;</td>
        <td><a href="http://prdownloads.sourceforge.net/dvdisaster/dvdisaster-0.73.1.tar.bz2?download">dvdisaster-0.73.1.tar.bz2</a></td></tr>
    <tr><td align="right">Digitale Unterschrift:&nbsp;</td>
        <td><a href="http://prdownloads.sourceforge.net/dvdisaster/dvdisaster-0.73.1.tar.bz2.gpg?download">dvdisaster-0.73.1.tar.bz2.gpg</a></td></tr>
    <tr><td align="right">Binärversion für Windows:&nbsp;</td>
        <td><a href="http://prdownloads.sourceforge.net/dvdisaster/dvdisaster-0.73.1-setup.exe?download">dvdisaster-0.73.1-setup.exe</a></td></tr>
    <tr><td align="right">Digitale Unterschrift:&nbsp;</td>
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
