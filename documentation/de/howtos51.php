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
require("../include/screenshot.php");

begin_page();

howto_headline("Informationen zu Abbildern/Fehlerkorrektur-Daten anzeigen", "Anzeigen", "images/compare-icon.png");
?>

<!--- Insert actual page content below --->

Für diese Funktion werden keine Grundeinstellungen benötigt; allerdings müssen
Sie bereits eine Abbild-Datei und gegebenenfalls die zugehörige 
<a href="howtos20.php">Fehlerkorrektur-Datei</a> vorliegen haben.

<hr>

<table>
<tr>
<td width="200px" align="center">
<a href="howtosa2.php">
<img src="../images/select-image.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Geben Sie den Dateinamen des ISO-Abbilds</b> an, 
über das Sie Informationen erhalten möchten. Das Abbild muß sich bereits
auf der Festplatte befinden; verwenden Sie bei Bedarf die "Lesen"-Funktion,
um es von einem Datenträger zu lesen.
</td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center">
<a href="howtosa3.php">
<img src="../images/select-ecc.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top">
<b>Geben Sie den Namen der zugehörigen Fehlerkorrektur-Datei</b> an.
Lassen Sie dieses Feld frei falls der Datenträger
<a href="howtos30.php">mit Fehlerkorrektur-Daten erweitert</a> wurde.
</td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center">
<a href="howtosa4.php">
<img src="images/compare-icon.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Beginnen Sie die Auswertung</b> durch
Klicken auf den "Vergleichen"-Knopf.</td>
</tr>
</table>

<?php begin_howto_shot("Informationen anzeigen.","compat-okay-rs01.png", ""); ?>
<b>Beobachten Sie den Fortschritt des Vorgangs.</b>
Zum Anzeigen aller Informationen müssen das Abbild und die
Fehlerkorrektur-Datei vollständig gelesen werden. 
<?php end_howto_shot(); ?>

<hr>

<a name="examine">Weiterführende Informationen zum Auswerten der Ergebnisse:</a><p>

<ul>
<li><a href="howtos52.php">Erklärung der Ausgaben für Fehlerkorrektur-Dateien</a><p></li>
<li><a href="howtos53.php">Erklärung der Ausgaben für Abbilder mit Fehlerkorrektur-Daten</a><p></li>
<li><a href="howtos59.php">Beispiele</a><p></li>
</ul>


<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
