
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

?>

<!--- Insert actual page content below --->

<h3>Aktionen beginnen</h3>

<?php begin_screen_shot("Aktionen beginnen","action-buttons.png"); ?>
Um in dvdisaster eine Aktion zu beginnen, klicken Sie auf einen
der grün markierten Knöpfe:<p>

<table>
<tr>
<td valign="top"><img src="images/read-icon.png"> &nbsp;</td>
<td><b>Einlesen eines Datenträgers in eine Abbild-Datei</b>, um:
<ul>
<li><a href="howtos42.php#a">einen beschädigten Datenträger</a> für eine nachfolgende
Rekonstruktion einzulesen. 
<li><a href="howtos23.php?way=1&expand=0">einen fehlerfreien Datenträger</a> zur Erstellung
einer Fehlerkorrektur-Datei einzulesen.</td>
</tr>

<tr>
<td valign="top"><img src="images/create-icon.png"> &nbsp;</td>
<td><b><a href="howtos20.php">Erstellen einer Fehlerkorrektur-Datei</a></b><br>
(geht nur aus einem fehlerfreien Datenträger!)</td>
</tr>

<tr>
<td valign="top"><img src="images/scan-icon.png"> &nbsp;</td>
<td><b><a href="howtos10.php">Überprüfung eines Datenträgers auf Lesefehler.</a></b>
</td>
</tr>

<tr>
<td valign="top"><img src="images/fix-icon.png"> &nbsp;</td>
<td><b><a href="howtos40.php">Rekonstruieren eines beschädigten Datenträgers</a>,</b><br>
zu dem eine <a href="howtos42.php#a">Abbild-Datei</a> und
 <a href="howtos20.php">Fehlerkorrektur-Daten</a> bereits vorliegen.
</td>
</tr>

<tr>
<td valign="top"><img src="images/compare-icon.png"> &nbsp;</td>
<td><b><a href="howtos50.php">Informationen über Abbilder und Fehlerkorrektur-Daten</a></b>
anzeigen.
</td>
</tr>
</table><p>

<b>Weitere Bedienelemente im Zusammenhang mit Aktionen:</b>

<table>
<tr>
<td valign="top"><img src="images/log-icon.png"> &nbsp;</td>
<td><b>Protokolldaten der laufenden Aktion ansehen</b> (gelb markiert).<br>
Siehe auch <a href="feedback.php#log">Erzeugen von Protokolldateien</a>.
</td>
</tr>

<tr>
<td valign="top"><img src="images/stop-icon.png"> &nbsp;</td>
<td><b>Abbrechen der laufenden Aktion</b> (rot markiert).<br>
Das Abbrechen der laufenden Aktion kann einen Moment dauern; insbesondere dann,
wenn dvdisaster gerade mit dem Lesen eines beschädigten Sektors beschäftigt ist.
</td>
</tr>
</table>

<?php end_screen_shot(); ?>


<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
