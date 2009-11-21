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
howto_headline("Fehlerkorrektur-Daten als eigenständige Datei erzeugen", "Übersicht", "images/create-icon.png");?>

<!--- Insert actual page content below --->

<table width="100%" cellspacing="5">
<tr valign="top">
<td width="20%"><b>Aufgabe</b></td>
<td>
Zu einem Datenträger wird eine Fehlerkorrektur-Datei erzeugt.
</td>
</tr>
<tr><td> <pre> </pre> </td></tr>

<tr valign="top">
<td></td>
<td>Hinweis: Hier wird beschrieben, wie Fehlerkorrektur-Daten in einer eigenständigen Datei abgelegt werden.
Es gibt auch eine Möglichkeit, Fehlerkorrektur-Daten auf dem Datenträger selbst unterzubringen. 
<a href="howtos21.php">Möchten Sie eine Entscheidungshilfe?</a></td>
<tr><td> <pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>Benötigt werden:</b><p></td>
</tr>

<tr>
<td><img src="../images/good-cd.png"></td>
<td>
Ein guter, fehlerfreier<a href="#footnote"><sup>*)</sup></a> Datenträger,</td>
</tr>

<tr><td></td><td>oder</td></tr>


<tr>
<td><img src="../images/good-image.png"></td>
<td>ein bereits vorhandenes und vollständiges<a href="#footnote"><sup>*)</sup></a> ISO-Abbild des Datenträgers (zum Beispiel vom Brennvorgang).
</td>
</tr>
<tr><td> <pre> </pre> </td></tr>


<tr>
<td colspan="2"><b>Durchzuführende Schritte:<p></b></td>
</tr>

<tr>
<td></td>
<td>
1. <a href="howtos22.php">Grundeinstellungen vornehmen</a><br>
2. <a href="howtos23.php">Fehlerkorrektur-Datei erstellen</a><br>
3. <a href="howtos24.php">Fehlerkorrektur-Dateien archivieren</a>
</td>
</tr>
</table><p>

<a href="howtos22.php">Grundeinstellungen vornehmen...</a>

<pre>


</pre>

<?php
footnote("*","footnote","Die Fehlerkorrektur-Daten müssen vor dem Eintreten des
Datenverlustes erzeugt werden: Von einem bereits defekten Datenträger können keine Fehlerkorrektur-Dateien 
mehr erstellt werden.");
?>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
