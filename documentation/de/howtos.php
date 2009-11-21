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

<h3>Typische Anwendungen</h3>

dvdisaster ist ein komplexes Werkzeug, zu dessen vollständiger Beschreibung
ein kleines Buch benötigt würde. Da uns dazu momentan die
nötigen Ressourcen fehlen, zeigen wir Ihnen zunächst, wie 
die <a href="howtos60.php">einzelnen Funktionen von dvdisaster zusammenspielen</a>.
Danach folgt eine Beschreibung der typischen Anwendungen in der Form von 
"Kochrezepten" mit typischen Schritten und Einstellungen.
Die Kochrezepte sind so aufgebaut, daß Sie damit in den meisten Fällen
bereits das gewünschte Ergebnis erzielen. Am Ende jedes Kochrezepts 
finden fortgeschrittene Benutzer eine Diskussion weiterführender Einstellungen,
um dvdisaster an spezielle Bedingungen anzupassen.<p>

<h3>Verwendete Symbole</h3>

Zur Arbeit mit dvdisaster werden Datenträger, Datenträger-Abbilder und
Fehlerkorrektur-Daten benötigt. 
Zu Anfang jedes Kochrezeptes zeigen Ihnen die folgenden Symbole, 
welche Kombinationen davon benötigt werden:<p>

<b>Datenträger</b> (zum Beispiel eine CD)

<table cellspacing="10">
<tr>
<td align="center" width="15%"><img src="../images/good-cd.png"></td>
<td align="center" width="15%"><img src="../images/bad-cd.png"></td>
<td width="55%">Dieses Symbol gibt an, ob ein Datenträger für die beschriebene 
Aufgabe benötigt wird, und ob dieser noch vollständig lesbar oder bereits 
beschädigt sein darf.
</td>
</tr>
<tr  valign="top">
<td>guter Datenträger (<b>ohne</b> Lesefehler)</td>
<td>beschädigter Datenträger (<b>mit</b> Lesefehlern)</td>
<td></td>
</tr>
</table><p>

<b>Datenträger-Abbilder</b> (ISO-Abbild eines Datenträgers auf der Festplatte)

<table cellspacing="10">
<tr>
<td align="center" width="15%"><img src="../images/good-image.png"></td>
<td align="center" width="15%"><img src="../images/bad-image.png"></td>
<td width="55%">Einige Funktionen arbeiten nicht direkt mit einem Datenträger, sondern
auf einem ISO-Abbild davon auf der Festplatte. Entsprechend des Zustands des 
Datenträgers sind die Abbilder vollständig oder unvollständig.</td>
</tr>
<tr valign="top">
<td>vollständiges Abbild (von gutem Datenträger)</td>
<td>unvollständiges Abbild (von beschädigtem Datenträger)</td>
</tr>
</table><p>

<b>Fehlerkorrektur-Daten</b>

<table cellspacing="10">
<tr>
<td align="center" width="15%"><img src="../images/good-cd-ecc.png"></td>
<td align="center" width="15%"><img src="../images/ecc.png"></td>
<td width="55%">Die zentrale Anwendung von dvdisaster ist die Wiederherstellung
von Abbildern durch Fehlerkorrektur-Daten. An diesem Symbol erkennen Sie, ob
Fehlerkorrektur-Daten benötigt werden.
</td>
</tr>
<tr  valign="top">
<td>Datenträger mit Fehlerkorrektur- Daten</td>
<td>separate Fehlerkorrektur- Datei</td>
<td></td>
</tr>
</table><p>

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
