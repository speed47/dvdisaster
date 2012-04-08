<?php
# dvdisaster: German homepage translation
# Copyright (C) 2004-2012 Carsten Gnörlich
#
# UTF-8 trigger: äöüß 
#
# Include our PHP sub routines, then call begin_page()
# to start the HTML page, insert the header, 
# navigation and news if appropriate.

require("../include/dvdisaster.php");
require("../include/footnote.php");
begin_page();
howto_headline("Informationen zu Abbildern/Fehlerkorrektur-Daten anzeigen", "Übersicht", "images/compare-icon.png");
?>

<!-- Insert actual page content below -->

<table width="100%" cellspacing="5">
<tr valign="top">
<td class="w20p"><b>Aufgabe</b></td>
<td>
Zeigt Informationen über die Art und den Zustand von Abbildern 
und Fehlerkorrektur-Dateien.
</td>
</tr>
<tr><td> <pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>Benötigt werden:</b><p></td>
</tr>
<tr>
 <td class="w150x" align="right">
   <img src="../images/good-image.png" alt="Symbol: vollständiges Abbild" class="valignt">
   <img src="../images/ecc.png" alt="Symbo: Eigenständige Fehlerkorrektur-Datei">
 </td>
<td>
Ein Datenträger-Abbild und gegebenenfalls die zugehörige
Fehlerkorrektur-Datei. 
</td>
<tr><td> <pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>Dies ist zu tun:</b><p></td>
</tr>

<tr>
<td></td>
<td>
1. <a href="howtos51.php">Informationen anzeigen</a><p>
2. <a href="howtos51.php#examine">Ergebnisse auswerten</a>
</td>
</tr>
</table><p>

<pre>


</pre>

<a href="howtos51.php">Informationen anzeigen...</a>

<!-- do not change below -->
<?php
# end_page() adds the footer line and closes the HTML properly.
end_page();
?>
