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

howto_headline("Datenträger-Abbild rekonstruieren", "Durchführen", "images/fix-icon.png");
?>

<!-- Insert actual page content below -->

Vergewissern Sie sich zunächst, daß dvdisaster wie in den 
<a href="howtos41.php">Grundeinstellungen</a> beschrieben konfiguriert ist.
Führen Sie dann die folgenden Schritte aus:<p>

<hr>

<a name="a"></a>
<table>
<tr>
<td class="w200x" align="center"><img src="../images/slot-in.png" alt="Symbol: Datenträger in Laufwerk einlegen">
<br><img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten"></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Legen Sie den defekten Datenträger in ein Laufwerk</b>, 
das direkt mit 
Ihrem Rechner verbunden ist. Sie können keine Netzwerklaufwerke, keine Softwarelaufwerke und keine 
Laufwerke innerhalb von virtuellen Maschinen verwenden.</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center"><img src="../images/winbrowser.png" alt="Symbol: Automatisch geöffnete Fenster schließen">
<br><img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten"></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Schließen Sie alle Fenster,</b> die Ihr Betriebssystem
möglicherweise öffnet, um den Inhalt des Datenträgers anzuzeigen oder abzuspielen.
Warten Sie, bis das Laufwerk den Datenträger erkannt hat und zur
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

<a name="select_eccfile"></a>
<table>
<tr>
<td class="w200x"align="center">
<img src="../images/select-ecc.png" alt="Bedienelement: Fehlerkorrektur-Datei auswählen (Eingabefeld und Knopf für Dialog)" class="nobordervalignm"></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt">
Wenn Sie mit <a href="howtos20.php">Fehlerkorrektur-Dateien</a> arbeiten, geben
Sie jetzt den Dateinamen an. Lassen Sie das Eingabefeld leer falls der Datenträger 
<a href="howtos30.php">mit Fehlerkorrektur-Daten erweitert</a> wurde.<br>
</td>
</tr>
<tr>
<td class="w200x" align="center"><a href="howtosa1.php">
<img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten" class="noborder"></a></td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center">
<a href="howtosa4.php">
<img src="images/read-icon.png" alt="Bedienelement: Lesen (Auswahlknopf)" class="noborder">
<br><img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten" class="noborder"></a></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Beginnen Sie den Lesevorgang</b> durch Klicken 
auf den "Lesen"-Knopf.</td>
</tr>
</table>

<?php begin_howto_shot("Abbild einlesen.","adaptive-progress.png", ""); ?>
<b>Beobachten Sie den Fortschritt des Lesevorgangs.</b>
Durch das angepaßte Leseverfahren entstehen während des Einlesens Lücken, die auf der Suche nach lesbaren
Bereichen systematisch geschlossen werden. Bei den meisten Datenträgern
ist der Effekt aber nicht so deutlich sichtbar wie auf dem Bildschirmfoto.
Wenn sich die defekten Bereiche auf das Datenträger-Ende konzentrieren, kann
es sogar sein daß der Lesevorgang endet bevor die defekten Sektoren erreicht werden.
<?php end_howto_shot(); ?>
<p>

<table>
<tr>
<td class="w200x" align="center">
<img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten" class="noborder"></td>
</tr>
</table>

<?php begin_howto_shot("Lesevorgang erfolgreich.","adaptive-success.png", ""); ?>
<b>Das weitere Vorgehen hängt vom Ergebnis des Lesevorgangs ab.</b> 
Das Leseverfahren beendet sich automatisch, wenn genügend Daten für eine
erfolgreiche Wiederherstellung gesammelt worden sind (siehe auch die grün
hervorgehobenenen Ausgaben). Klicken Sie wie unten beschrieben auf den
"Reparieren"-Knopf um die Wiederherstellung zu beginnen.
<?php end_howto_shot(); ?>

<?php begin_howto_shot("Lesevorgang fehlgeschlagen.","adaptive-failure.png", ""); ?>
Das Leseverfahren wird auch abbrechen, wenn es nicht genügend lesbare
Datensektoren finden konnte (siehe die rot hervorgehobenen Ausgaben). 
In diesem Zustand kann das Datenträger-Abbild
<b>nicht</b> wiederhergestellt werden. Versuchen Sie mit Hilfe der Tips unter den
<a href="howtos43.php">fortgeschrittenen Einstellungen</a> genügend Daten für
die Fehlerkorrektur einzulesen.
<?php end_howto_shot(); ?>

<table>
<tr>
<td class="w200x" align="center"><img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten" class="noborder"></td>
<td></td><td></td>
</tr>

<tr>
<td class="w200x" align="center"><a name="b"></a><a href="howtosa4.php">
<img src="images/fix-icon.png" alt="Bedienelement: Reparieren (Auswahlknopf)" class="noborder"></a>
</td>
<td>&nbsp;&nbsp;</td>
<td class="valignt">Klicken Sie auf den "Reparieren"-Knopf, um die
<b>Wiederherstellung des Abbilds</b> zu beginnen (<b>nur</b> wenn der Lesevorgang erfolgreich war!).</td>
</tr>

<tr>
<td class="w200x" align="center"><img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten" class="noborder"></td>
<td></td><td></td>
</tr>
</table>

<?php begin_howto_shot("Wiederstellung verfolgen.","fix-success.png", ""); ?>
<b>Verfolgen Sie den Fortschritt der Wiederherstellung.</b> Weil das angepaßte Lesen
nur so viele Daten sammelt wie für die Fehlerkorrektur notwendig sind, wird
die Fehlerkorrektur vollständig ausgelastet. Dadurch wird im Fenster unter
"Fehler/Ecc-Bereich" ein massiver roter Balken ausgegeben.
Die Wiederherstellung kann je nach Datenträgergröße und Systemleistung
mehrere Minuten bis Stunden dauern.
<?php end_howto_shot(); ?>

<table>
<tr>
<td class="w200x"align="center">
<img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten" class="noborder">
</td>
</tr>
</table>

<table>
<tr>
<td class="w200x"align="center">
<img src="../images/good-image.png" alt="Symbol: Vollständiges Abbild" class="nobordervalignm"></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt">Nach dem Abschluß der Wiederherstellung
haben Sie wieder ein vollständiges ISO-Abbild.
</td>
</tr>
</table>

<table>
<tr>
<td class="w200x"align="center">
<img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten" class="noborder">
</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center">
<a href="howtos33.php?way=2#c">
<img src="thumbnails/write-iso1.png" alt="Abb.: Abbild auf einen Datenträger schreiben" class="noborder"></a></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Schreiben Sie das wiederhergestellte ISO-Abbild</b> 
auf einen neuen Datenträger. Gehen Sie dabei genau so vor wie es im
Abschnitt zum <a href="howtos33.php?way=2#c">Brennen von Datenträgern</a> beschrieben
ist, die <a href="howtos33.php">mit Fehlerkorrektur-Daten erweitert wurden</a>.
</td>
</tr>
</table>

<table>
<tr>
<td class="w200x"align="center">
<img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten" class="noborder">
</td>
</tr>
</table>

<table>
<tr>
<td class="w200x"align="center">
<img src="../images/old-cd.png" alt="Symbol: Alter (beschädigter) Datenträger" class="nobordervalignm">
<img src="../images/old-image.png" alt="Symbol: Alte Abbild-Datei" class="nobordervalignm">
<img src="../images/good-cd.png" alt="Symbol: Guter Datenträger (ohne Lesefehler)" class="nobordervalignm"></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt">Sie haben jetzt einen neuen Datenträger erzeugt, der wieder
alle Daten enthält. <a href="howtos10.php">Überprüfen Sie ihn zur Sicherheit auf 
Lesefehler</a>. Danach können Sie den alten defekten Datenträger wegwerfen 
sowie das ISO-Abbild löschen. Eine für den alten Datenträger erstellte
Fehlerkorrektur-Datei können Sie mit dem neuen Datenträger weiter verwenden.
</td>
</tr>
</table>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
