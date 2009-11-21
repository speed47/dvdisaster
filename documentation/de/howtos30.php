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
howto_headline("Fehlerkorrektur-Daten erstellen", "Übersicht", "images/create-icon.png");
?>

<!--- Insert actual page content below --->

<h3>Fehlerkorrektur-Daten auf dem Datenträger selbst unterbringen</h3>

<table width="100%" cellspacing="5">
<tr valign="top">
<td width="20%"><b>Aufgabe</b></td>
<td>
Fehlerkorrektur-Daten werden zusammen mit den Nutzdaten auf dem gleichen Datenträger
abgelegt.
</td>
</tr>
<tr><td> <pre> </pre> </td></tr>

<tr valign="top">
<td></td>
<td>Hinweis: Hier wird beschrieben, wie Fehlerkorrektur-Daten auf dem Datenträger selbst
untergebracht werden. 
Es gibt auch eine Möglichkeit, Fehlerkorrektur-Daten in Form einer eigenständigen Datei
zu erzeugen.
<a href="howtos21.php">Möchten Sie eine Entscheidungshilfe?</a></td>
<tr><td> <pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>Voraussetzungen:</b><p></td>
</tr>

<tr>
<td><img src="../images/good-image.png"></td>
<td>
<ul>
<li>eine (Brenn-)Software, die ISO-Abbilder erzeugen kann</li>
<li>der zu schützende Datenträger soll erst noch gebrannt werden
<a href="#footnote"><sup>*)</sup></a></li>
<li>mindestens 20% sind noch auf dem zu erzeugenden Datenträger frei</li>
</ul>
</tr>
<tr><td> <pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>Durchzuführende Schritte:<p></b></td>
</tr>

<tr>
<td></td>
<td>
1. <a href="howtos32.php">Grundeinstellungen für die Fehlerkorrektur-Daten vornehmen</a><p>
2a. <a href="howtos33.php#a">Ein ISO-Abbild erstellen,</a><br>
2b. <a href="howtos33.php#b">mit Fehlerkorrektur-Daten erweitern</a><br>
2c. <a href="howtos33.php#c">und auf einen Datenträger schreiben.</a>
</td>
</tr>
</table><p>

<a href="howtos32.php">ISO-Abbild erzeugen...</a>

<pre>


</pre>

<!--- do not change below --->

<?php
footnote("*","footnote","Ein bereits geschriebener Datenträger kann nicht mehr nachträglich um
Fehlerkorrektur-Daten erweitert werden.");

# end_page() adds the footer line and closes the HTML properly.
end_page();
?>
