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
require("../include/screenshot.php");

begin_page();

howto_headline("Datenträger überprüfen", "Durchführen", "images/scan-icon.png");
?>

<!-- Insert actual page content below -->

Vergewissern Sie sich zunächst, daß dvdisaster wie in den 
<a href="howtos11.php">Grundeinstellungen</a> beschrieben konfiguriert ist.
Ungünstige Einstellungen können anderenfalls das Ergebnis der Überprüfung negativ
beeinflussen. Führen Sie dann die folgenden Schritte aus:<p>

<hr>

<table>
<tr>
<td class="w200x" align="center"><img src="../images/slot-in.png" alt="Symbol: Datenträger in Laufwerk einlegen">
<br><img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten"></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Legen Sie den zu prüfenden Datenträger in ein Laufwerk</b>, 
das direkt mit 
Ihrem Rechner verbunden ist. Sie können keine Netzwerklaufwerke und keine
Laufwerke innerhalb von virtuellen Maschinen verwenden.</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center"><img src="../images/winbrowser.png" alt="Symbol: Alle automatisch geöffneten Fenster schließen">
<br><img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten"></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Schließen Sie alle Fenster,</b> die Ihr Betriebssystem
möglicherweise öffnet, um den Inhalt des Datenträgers anzuzeigen oder abzuspielen.
Warten Sie mit dem Test, bis das Laufwerk den Datenträger erkannt hat und zur
Ruhe gekommen ist, also z.B. den Datenträger nicht mehr dreht.</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center"><a href="howtosa1.php">
<img src="../images/select-drive.png" alt="Bedienelement: Laufwerksauswahl (Ausklappbares Menü)" class="noborder">
<br><img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten" class="noborder"></a></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Wählen Sie in dvdisaster das Laufwerk aus,</b>
in das Sie den Datenträger eingelegt haben.</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center">
<a href="howtosa3.php">
<img src="../images/select-ecc.png" alt="Bedienelement: Fehlerkorrektur-Datei auswählen (Eingabefeld und Knopf für Dialog)" class="noborder">
<br><img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten" class="noborder"></a></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Wählen Sie die Fehlerkorrektur-Datei zu dem Datenträger aus,</b>
sofern Sie eine haben.</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center">
<a href="howtosa4.php">
<img src="images/scan-icon.png" alt="Bedienelement: Prüfen (Aufruf-Knopf)" class="noborder">
<br><img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten" class="noborder"></a></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Führen Sie die Überprüfung durch Klicken 
auf den "Prüfen"-Knopf durch.</b></td>
</tr>
</table>

<?php begin_howto_shot("Datenträger prüfen.","good-cd.png", ""); ?>
<b>Beobachten Sie den Fortschritt der Überprüfung.</b>
Führen Sie während dieser Zeit keine anderen Aktionen auf dem Computer durch.
Das Aufrufen oder Bedienen von anderen Programmen oder auch das 
Verschieben von Fenstern kann die Überprüfung beeinflussen.
<?php end_howto_shot(); ?>
<p>

<hr>

<a href="howtos13.php">Ergebnisse bewerten...</a>


<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
