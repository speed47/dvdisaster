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
require("../include/footnote.php");
begin_page();
howto_headline("Datenträger-Abbild rekonstruieren", "Übersicht", "images/fix-icon.png");
?>

<!--- Insert actual page content below --->

<h3>Datenträger-Abbilder rekonstruieren</h3>

<table width="100%" cellspacing="5">
<tr valign="top">
<td width="20%"><b>Aufgabe</b></td>
<td>
Wiederherstellen des Inhalts eines defekten Datenträgers.
</td>
</tr>
<tr><td> <pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>Benötigt werden:</b><p></td>
</tr>
<tr>
 <td width="150px" align="right">
   <img src="../images/bad-cd-ecc.png" align="top">
 </td>
<td>
Ein defekter Datenträger, der <a href="howtos30.php">Fehlerkorrektur-Daten enthält</a>,
</td>
</tr>
<tr><td></td><td>oder</td></tr>
<tr>
 <td width="150px" align="right">
   <img src="../images/bad-cd.png">
   <img src="../images/ecc.png">
 </td>
<td>
ein defekter Datenträger und eine zugehörige <a href="howtos20.php">Fehlerkorrektur-Datei</a><a href="#footnote"><sup>*)</sup></a>.
</td>
</tr>
<tr><td> <pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>Dies ist zu tun:<p></b></td>
</tr>

<tr>
<td></td>
<td>
1. <a href="howtos41.php">Grundeinstellungen zum Lesen vornehmen,</a><br>
2a. <a href="howtos42.php#a">ein ISO-Abbild von dem defekten Datenträger erstellen,</a><br>
2b. <a href="howtos42.php#b">das Abbild rekonstruieren und erneut brennen.</a>
</td>
</tr>
</table><p>

<a href="howtos42.php">ISO-Abbild erstellen...</a>

<pre>


</pre>

<!--- do not change below --->
<?php
footnote("*","footnote",
"Die Fehlerkorrektur-Datei muß erstellt worden sein als der Datenträger noch in Ordnung war: Von einem bereits defekten Datenträger können keine Fehlerkorrektur-Dateien mehr erstellt werden.");
# end_page() adds the footer line and closes the HTML properly.
end_page();
?>
