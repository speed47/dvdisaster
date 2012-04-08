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
begin_page();

?>

<!-- Insert actual page content below -->

<h3 class="top">Bedienelemente</h3>

Dieses Kapitel erklärt einige der häufig verwendeten Bedienelemente:

<pre> </pre>

<table width="100%">
<tr>
<td align="center"><a href="howtosa1.php"><img src="../images/good-cd.png" alt="Bedienelement: Laufwerk auswählen (Ausklappbares Menü)" class="noborder"></a></td>
<td>Das <a href="howtosa1.php">Menü zum Auswählen von Laufwerken</a>.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>
<tr>
<td align="center"><a href="howtosa2.php"><img src="../images/good-image.png" alt="Symbol: Gute Abbild (ohne Lesefehler)" class="noborder"></a></td>
<td>Das <a href="howtosa2.php">Fenster zum Auswählen von Abbild-Dateien</a>.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>
<tr>
<td align="center"><a href="howtosa3.php"><img src="../images/ecc.png" alt="Symbol: Fehlerkorrektur-Datei" class="noborder"></a></td>
<td>Das <a href="howtosa3.php">Fenster zum Auswählen von Fehlerkorrektur-Dateien</a>.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>
<tr>
<td align="center"><a href="howtosa4.php">
  <img src="images/read-icon.png" alt="Bedienelement: Lesen (Auswahlknopf)" class="noborder">
  <img src="images/create-icon.png" alt="Bedienelement: Erzeugen (Auswahlknopf)" class="noborder"><br>
  <img src="images/scan-icon.png" alt="Bedienelement: Prüfen (Auswahlknopf)" class="noborder">
  <img src="images/fix-icon.png" alt="Bedienelement: Reparieren (Auswahlknopf)" class="noborder"><br>
  <img src="images/compare-icon.png" alt="Bedienelement: Vergleichen (Auswahlknopf)" class="noborder">
  <img src="images/stop-icon.png" alt="Bedienelement: Abbrechen (Auswahlknopf)" class="noborder">
</a></td>
<td>Die <a href="howtosa4.php">Piktogramme zum Auswählen von Aktionen</a>.</td>
</tr>
</table>



<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
