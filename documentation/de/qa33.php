<?php
# dvdisaster: English homepage translation
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

<h3 class="top">Übersicht über die RS01-, RS02 und RS03-Fehlerkorrektur-Verfahren</h3>

dvdisaster verfügt über drei Fehlerkorrektur-Verfahren. RS01 und RS02
sind die bisherigen und erprobten Verfahren, während sich RS03 
gerade in Entwicklung befindet.<p>

<b>Vergleich der Verfahren.</b>

Alle Verfahren setzen die gleiche 
<a href="qa31.php">Reed-Solomon</a>-Fehlerkorrektur ein.
Sie berechnen Fehlerkorrektur-Informationen zu ISO-Abbildern mit dem Ziel,
später unlesbar gewordene Abbild-Sektoren aus diesen Informationen 
wiederherzustellen.<p>

Die Unterschiede liegen in der Art, wie die Fehlerkorrektur-Informationen abgelegt werden:<p>

<ul>
<li>
<a name="file"> </a>
RS01 erzeugt <b>Fehlerkorrektur-Dateien,</b> die unabhängig von dem
zugehörigen Abbild aufbewahrt werden. Da ein Schutz von Daten auf
<a href="qa32.php#file">Datei-Ebene</a> schwierig ist, 
müssen Fehlerkorrektur-Dateien auf Datenträgern gespeichert werden,
die selbst mit dvdisaster gegen Datenverlust geschützt sind.<p></li>

<li>
<a name="image"> </a>
Beim RS02-Verfahren wird zunächst ein Abbild der zu sichernden Daten 
auf der Festplatte mit Hilfe einer Brennsoftware erzeugt. Vor dem Schreiben auf
den Datenträger wird dieses <b>Abbild</b> jedoch mit dvdisaster um 
Fehlerkorrektur-Daten <b>erweitert</b>.
Dadurch befinden sich die zu schützenden Daten zusammen mit den 
Fehlerkorrektur-Informationen auf dem selben Datenträger. 
Defekte Sektoren in den Fehlerkorrektur-Informationen verringern
die Kapazität der Fehlerkorrektur, machen diese aber nicht unmöglich - ein
zweiter Datenträger zum Aufbewahren oder Schützen 
der Fehlerkorrektur-Informationen wird nicht benötigt.<p></li>
</ul>

RS03 ist eine Weiterentwicklung von RS01 und RS02. Es kann
sowohl Fehlerkorrektur-Dateien erzeugen als auch Abbilder mit
Fehlerkorrektur-Daten erweitern:
<ul>
<li>RS03 kann seine Arbeit auf mehrere Prozessorkerne verteilen
und ist damit auf aktuellen Computern wesentlich schneller als RS01/RS02.</li>
<li>RS03-<b>Fehlerkorrektur-Dateien</b> sind im Gegensatz zu RS01 robust gegen 
Beschädigungen. Dies darf allerdings kein Freibrief sein um mit den Fehlerkorrektur-Dateien sorglos umzugehen, denn die Nachteile des <a href="qa32.php#file">Lesens
 auf Dateisystem-Ebene</a> gelten nach wie vor.</li>
<li>Mit RS03 <b>erweiterte Abbilder</b> enthalten im Gegensatz zu RS02 keine
Masterblöcke mehr, an denen wichtige Informationen abgelegt sind. Dadurch wird
RS03 noch etwas robuster, muß nun allerdings immer den gesamten Rest des
Datenträgers ausfüllen - die Abbildgröße läßt sich nicht mehr von Hand einstellen.</li>
</ul>
Durch die Änderungen für parallele Berechnung und höhere Robustheit ist RS03
etwas weniger platzeffizient, d.h. bei gleicher Größe haben RS03-Daten geringfügig weniger Fehlerkorrektur-Kapazität als ihre RS01- und RS02-Gegenstücke.<p>

<a name="table"> </a>
<b>Vergleich zum Ablegen der Fehlerkorrektur-Informationen</b><p>

Die folgende Tabelle faßt die Unterschiede zwischen dem Erzeugen
von Fehlerkorrektur-Dateien (RS01, RS03) und dem Erweitern von Abbildern
mit Fehlerkorrektur-Daten (RS02, RS03) zusammen:<p>

<table width="100%" border="1" cellspacing="0" cellpadding="5">
<tr>
<td class="w50p"><i>Fehlerkorrektur-Dateien</i></td>
<td class="w50p"><i>Fehlerkorrektur-Daten im Abbild</i></td>
</tr>
<tr valign="top">
<td> Redundanz kann beliebig groß gewählt werden</td>
<td> Redundanz ist durch freien Platz auf dem Datenträger beschränkt<br>
(= Kapazität des Datenträgers - Größe des ursprünglichen Abbildes)</td>
</tr>

<tr valign="top">
<td>bereits wirksam ab 15% Redundanz, weil die Fehlerkorrektur-Daten
nach Voraussetzung unbeschädigt vorliegen</td>
<td>benötigt mehr Redundanz (empfohlen: 20-30%), 
um Verluste von Fehlerkorrektur-Daten auszugleichen</td> 
</tr>

<tr valign="top">
<td>der Datenträger kann beliebig voll sein</td>
<td>die nutzbare Datenträger-Kapazität sinkt entsprechend der erzielten Redundanz</td>
</tr>

<tr valign="top">
<td> können nachträglich für bereits existierende Datenträger
erzeugt werden</td>
<td> nur beim Brennen neuer Datenträger anwendbar, weil das Abbild vorher 
um Fehlerkorrektur-Daten erweitert werden muß</td>
</tr>

<tr valign="top">
<td> unabhängige Speicherung von den zu schützenden Daten erhöht Datensicherheit</td>
<td> gemeinsame Aufbewahrung von Nutzdaten und Fehlerkorrektur-Daten auf dem gleichen Datenträger vermindert die Fehlerkorrektur-Kapazität</td>
</tr>

<tr valign="top">
<td>Zuordnung von Fehlerkorrektur-Dateien zu Datenträgern
muß geeignet realisiert werden. Fehlerkorrektur-Dateien müssen
vor Beschädigung geschützt werden</td>
<td>Einfache Lösung mit einem Datenträger; Fehlerkorrektur-Informationen müssen nicht
katalogisiert oder geschützt werden</td></tr>

<tr valign="top">
<td> keine Kompatibilitätsprobleme beim Abspielen </td>
<td> um Fehlerkorrektur-Daten erweiterte Abbilder
 sind möglicherweise nicht überall abspielbar</td>
</tr>
</table><p>


<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
