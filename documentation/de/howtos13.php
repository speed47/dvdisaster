<?php
# dvdisaster: German homepage translation
# Copyright (C) 2004-2010 Carsten Gnörlich
#
# UTF-8 trigger: äöüß 
#
# Include our PHP sub routines, then call begin_page()../images/
# to start the HTML page, insert the header, 
# navigation and news if appropriate.

require("../include/dvdisaster.php");
require("../include/screenshot.php");

begin_page();

howto_headline("Datenträger überprüfen", "Ergebnisse bewerten", "images/scan-icon.png");
?>

<!--- Insert actual page content below --->

<?php begin_screen_shot("Übersicht","defective-cd.png"); ?>
<b>Übersicht.</b> dvdisaster stellt die Ergebnisse der Überprüfung in mehreren 
Ansichten dar:
<ul>
<li>Die Spirale unter "<b>Datenträger-Zustand</b>" (rechts).<p>
Die Spirale ist die entscheidende Anzeige für die Lesbarkeit des Datenträgers. 
Nur wenn alle Blöcke darin grün gefärbt sind, ist der Datenträger in Ordnung. 
Gelbe oder rote Blöcke markieren hingegen Stellen, 
an denen Daten nicht korrekt lesbar sind. Die genaue Anzahl der fehlerhaften Stellen
wird am unteren Fensterrand nach <i>"Überprüfung beendet:</i> ausgegeben.<p>
</li>
<li>"<b>Geschwindigkeit</b>" - Die Kurve der Lesegeschwindigkeit (links oben).<p>
Die Lesegeschwindigkeit ist kein absolutes Kriterium für den Zustand eines
Datenträgers, aber im großen und ganzen gilt: Je gleichmäßiger diese Kurve
verläuft, desto besser ist der Datenträger. Auf den Bildschirmfotos weiter unten
finden Sie Beispiele für gute und schlechte Lesekurven.<p></li>
<li>"<b>C2-Fehler</b>" - Eine Zustandseinschätzung durch das Laufwerk (links unten).<p>
Diese Analyse ist momentan <a href="qa.php?pipo">nur für CD-Datenträger verfügbar</a>.
CD-Laufwerke haben eine eingebaute Fehlerkorrektur, die kleinere Beschädigungen des
Datenträgers aus den Daten "herausrechnen" kann. Die Anzahl der C2-Fehler gibt an,
wie oft das Laufwerk beim Lesen auf diesen Mechanismus zurückgreifen mußte - bei
guten Datenträgern wird hier nichts angezeigt.</li>
</ul>
<?php end_screen_shot(); ?>

<b>Beispiele für gute Datenträger</b><p>

<?php begin_screen_shot("Gute CD","good-cd.png"); ?>
<b>Gute CD</b>: Auf diesem Bildschirmfoto sehen Sie ein Beispiel für eine gute CD:
Alle Blöcke unter "Datenträger-Zustand"  sind grün, es werden keine C2-Fehler
angezeigt und die Lesekurve verläuft gleichmäßig. Bei den meisten Datenträgern ist es
normal, daß die Lesegeschwindigkeit von vorne nach hinten ansteigt (Gegenbeispiel
siehe nächstes Bildschirmfoto). Die kleinen Zacken nach unten am Anfang und Ende
der Kurve sind normal; ebenfalls sind kleine Einbrüche wie der bei ca. 250M kein
Grund zur Beunruhigung.
<?php end_screen_shot(); ?>

<!--- do not change below --->
<?php begin_screen_shot("Gute zweischichtige DVD","good-dvd9.png"); ?>
<b>Die Lesekurve muß nicht immer gerade ansteigen</b>: Bei mehrschichtigen
Datenträgern kann die Lesekurve auch symmetrisch ansteigen und wieder abfallen.
Nicht gezeigt, aber auch möglich sind waagerechte Lesekurven, bei denen sich
die Lesegeschwindigkeit gar nicht ändert (typischerweise bei DVD-RAM).
<?php end_screen_shot(); ?><p>

<b>Ein Beispiel für einen schwachen Datenträger</b><p>

<?php begin_screen_shot("Schwache CD","weak-cd.png"); ?>
Dieser Datenträger ist noch vollständig lesbar, wie die grüne Spirale unter
"Datenträger-Zustand" anzeigt. Es kündigen sich allerdings bereits ernsthafte
Probleme an: Gegen Ende der CD muß das Laufwerk deutlich abbremsen um die Daten noch lesen zu können. Dies erkennt man
an dem Einbruch der Lesegeschwindigkeit ab 600M. Außerdem steigen die 
C2-Fehlerraten in diesem Bereich schon knapp unter die 100er-Marke an; dies ist ein
weiterer Hinweis darauf, daß dieser Datenträger beginnt, im Außenbereich zu zerfallen.<p>
Wenn für diese CD noch keine <a href="howtos20.php">Fehlerkorrektur-Daten</a>  erzeugt
worden sind, dann ist jetzt wahrscheinlich die letzte Gelegenheit dazu, da es nicht
mehr lange dauern wird bis die ersten Bereiche unlesbar werden.
<?php end_screen_shot(); ?><p>

<b>Beispiele für defekte Datenträger</b><p>

<?php begin_screen_shot("Defekte CD","defective-cd.png"); ?>
<b>Defekte CD.</b> Die roten Sektoren
in der Spirale verdeutlichen, daß der Datenträger im Außenbereich große unlesbare
Abschnitte enthält. Der Angabe am unteren Fensterrand können Sie entnehmen, daß
15808 Sektoren auf dem Datenträger nicht mehr lesbar sind. Da dies gerade mal 
4.6 Prozent von 343024 Sektoren insgesamt sind, wird dvdisaster den Inhalt der
fehlenden Sektoren problemlos <a href="howtos40.php">wiederherstellen</a> können -
wenn Sie die zugehörigen <a href="howtos20.php">Fehlerkorrektur-Daten</a> haben! 
Anderenfalls ist der Inhalt der roten Sektoren verloren, da Fehlerkorrektur-Daten
nicht nachträglich von defekten Datenträgern erstellt werden können.
<?php end_screen_shot(); ?><p>

<a name="crc"></a>
<?php begin_screen_shot("Prüfsummenfehler","crc-cd.png"); ?>
<b>Prüfsummenfehler.</b> Die gelb markierten Stellen in der Spirale zeigen an,
daß die betreffenden Sektoren des Datenträgers zwar perfekt lesbar waren,
aber ihr Inhalt stimmt nicht mit den Prüfsummen in den Fehlerkorrektur-Daten überein.
Dafür gibt es zwei Hauptursachen:<p>

<ul><li>
<b>Das Abbild ist nach dem Erzeugen der Fehlerkorrektur-Daten noch verändert worden</b>, bevor
es auf den Datenträger gebrannt wurde. Das passiert zum Beispiel auf Unix-Systemen, 
wenn das Datenträger-Abbild nach dem Erstellen der Fehlerkorrektur-Daten mit Schreibrechten in das System eingebunden wurde. Ein typisches Indiz für diesem Fall sind
CRC-Fehler in Sektor 64 und in den Sektoren 200 bis 400; das Durchführen einer
Wiederherstellung durch dvdisaster ist in dieser Situation typischerweise ungefährlich.<p>

Falls Sie nach dem Erstellen der Fehlerkorrektur-Daten allerdings Dateien in dem Abbild 
verändert haben, so sind die Fehlerkorrektur-Daten wertlos und gegebenfalls
sogar schädlich. Die Rekonstruktion des Datenträgers wird den Zustand wieder herstellen,
den das Abbild beim Erstellen der Fehlerkorrektur-Daten hatte, und dies entspricht ja
offensichtlich nicht mehr dem Inhalt der Daten auf dem Datenträger.<p></li>

<li><b>Es gibt technische Probleme mit dem verwendeten Computersystem,</b> 
insbesondere bei der Kommunikation mit den Massenspeichern. Merken Sie
sich an welchen Stellen die Fehler auftraten und führen Sie die
Überprüfung erneut durch. Wenn die Fehler verschwinden oder an einer 
anderen Stelle auftreten, hat Ihr Rechner möglicherweise ein Problem mit 
defektem Hauptspeicher, fehlerhaften Laufwerks-Verkabelungen/-Kontrollern
oder falsch eingestellte Taktfrequenzen.</li></ul>
<?php end_screen_shot(); ?><p>

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
