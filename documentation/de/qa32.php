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

<h3 class="top">Datenrekonstruktion auf Abbild-Ebene</h3>

Eine Datenträger-Rekonstruktion mit fehlerkorrigierenden Kodes findet in
zwei Schritten statt:

<ol>
<li>Zuerst wird versucht, möglichst viele Daten von dem Datenträger zu lesen.<p></li>
<li>Dann werden die noch fehlenden Daten durch den Fehlerkorrektur-Kode rekonstruiert.</li>
</ol>

Die Ausbeute an noch lesbaren Daten (Schritt 1) hängt nicht nur von dem
verwendeten Laufwerk ab, sondern auch davon, auf welcher logischen Ebene 
auf den Datenträger zugegriffen wird. Diese Seite erklärt die logischen Ebenen
und warum dvdisaster auf der Ebene von Abbildern arbeitet:<p>

<b>Logische Ebenen eines Datenträgers</b><p>

Optische Datenträger sind in <i>Daten-Sektoren</i> von jeweils 2048 Bytes aufgeteilt.
Liest man diese Sektoren nacheinander aus und speichert sie ab, 
so erhält man ein <i>Abbild</i> des Datenträgers.<p>

Das Arbeiten mit einzelnen Sektoren ist aus Benutzersicht sehr unhandlich. 
Deshalb werden Datenträger
mit einem <i>Dateisystem</i> versehen, das Daten-Sektoren zu <i>Dateien</i> 
zusammenfaßt. Dies erfordert eine genaue Buchführung darüber, aus welchen
Daten-Sektoren die Dateien bestehen (sowie weitere Merkmale wie 
Dateinamen und Zugriffsrechte). Für diese Buchführung
wird ein Teil der Daten-Sektoren reserviert und mit entsprechenden Datenstrukturen
gefüllt.<p>

Datenträger lassen sich somit in verschiedene <i>logische Ebenen</i> einteilen: 
Betrachtet man den Inhalt eines Datenträgers als eine Folge von Daten-Sektoren,
so arbeitet man auf der <i>Abbild-Ebene</i>. Stellt man ihn sich hingegen als eine
Menge von Dateien vor, so befindet man sich auf der <i>Dateisystem-Ebene</i>.<p>

In Hinsicht auf die Daten-Rekonstruktion haben die beiden Ebenen unterschiedliche 
Eigenschaften:<p>


<a name="file"> </a>
<b>Probleme beim Lesen auf Dateisystem-Ebene</b><p>

Beim Lesen auf Dateisystem-Ebene wird versucht, die auf einem
defekten Datenträger enthaltenen Dateien einzeln soweit wie möglich auszulesen.<p>

Dabei entsteht ein Problem, wenn Daten-Sektoren beschädigt sind, 
die zur Buchführung im Dateisystem dienen. Dies kann bewirken, daß die Liste
aller Dateien auf dem Datenträger unvollständig ist. Oder die Zuordnung
von Daten-Sektoren zu Dateien ist nicht mehr vorhanden.
Dadurch gehen Dateien oder Teile davon verloren, 
selbst wenn die zugehörigen Daten-Sektoren 
noch technisch lesbar sind. Das ist schlecht, denn auch die noch lesbaren Anteile
von beschädigten Dateien sind für den Fehlerkorrektur-Kode wichtig.<p>

Ein besonders schlechter Fall entsteht, wenn die Fehlerkorrektur-Daten auch 
in Dateien abgelegt sind. Dann werden die Fehlerkorrektur-Daten gebraucht, 
um das zugehörige Dateisystem zu reparieren, aber aufgrund des defekten 
Dateisystems ist kein Zugriff auf die Fehlerkorrektur-Daten möglich.
Das führt zum vollständigen Datenverlust und hat auch Konsequenzen für das 
<a href="#eccfile"> Aufheben von Fehlerkorrektur-Dateien</a> - dazu gleich mehr.
<p>
Das gleiche Problem entsteht auch, wenn man Dateien mit dem PAR2-Format 
um Fehlerkorrektur-Daten ergänzt und dann alle diese Dateien auf 
einem Datenträger abspeichert. Wenn jetzt die Buchführung des Dateisystems
beschädigt wird, kommt man an die Dateien und damit auch an die
Fehlerkorrektur-Daten nicht mehr heran.<p>


Mit einem Abbild-basierten Ansatz sieht die Situation hingegen besser aus:<p>

<a name="image"> </a>
<b>Vorteile beim Lesen auf Abbild-Ebene</b><p>

Beim Einlesen auf der Abbild-Ebene wird auf die Daten-Sektoren durch direkte Kommunikation
mit der Laufwerks-Hardware zugegriffen.<p>

Der Erfolg beim Einlesen von Daten-Sektoren hängt nur von den Lesefähigkeiten des
Laufwerks ab, nicht aber vom Zustand des Dateisystems. Lesefehler in einem Sektor
verhindern nicht den Zugriff auf andere Sektoren. Weil <i>alle</i> noch technisch 
lesbaren Daten gerettet werden können, liefert das Verfahren die günstigste
Ausgangsbasis für die Fehlerkorrektur.<p>

Das Abbild enthält alle Daten-Sektoren des Datenträgers. Deshalb ist nach der 
Wiederherstellung des Abbilds auch das darauf gespeicherte Dateisystem 
wieder vollständig. Ein Schutz des Datenträgers auf der Abbild-Ebene ist damit
umfassender als eine Fehlerkorrektur auf der Datei-Ebene.<p>

dvdisaster arbeitet ausschließlich auf der Abbild-Ebene, um von diesen Vorteilen
zu profitieren. Mit dem <a href="qa33.php">RS02-Verfahren</a>
ist sogar ein Ablegen der Fehlerkorrektur-Daten auf dem selben Datenträger 
möglich, weil das Auslesen der Fehlerkorrektur-Daten auf der Abbild-Ebene
nicht durch Fehler an anderen Stellen verhindert wird (beschädigte 
Sektoren mit Fehlerkorrektur-Daten verringern natürlich die Leistung der 
Fehlerkorrektur, machen sie aber nicht unmöglich).<p>

Das <a href="qa33.php">RS01-Verfahren</a> schützt Datenträger ebenfalls
auf der Abbild-Ebene, legt die Fehlerkorrektur-Informationen aber in Dateien ab.
Auf mögliche Fallgruben in diesem Zusammenhang weist der nächste Abschnitt hin.<p>

<a name="eccfile"> </a>
<b>Konsequenzen für das Aufbewahren von Fehlerkorrektur-Dateien</b><p>

Datenträger sind durch die mit dvdisaster erzeugten Fehlerkorrektur-Daten
auf Abbild-Ebene geschützt. Aber was ist mit den Fehlerkorrektur-Dateien selbst?<p>

Da Fehlerkorrektur-Dateien auf Dateisystem-Ebene gelesen werden, unterliegen sie
den entsprechenden Einschränkungen. Wenn der Datenträger mit den
Fehlerkorrektur-Dateien schadhaft wird, ist nicht mehr
sichergestellt, daß sich die Fehlerkorrektur-Dateien noch vollständig lesen
lassen.<p>

<table width="100%"><tr><td class="vsep"></td><td>
Deshalb ist es unverzichtbar, auch Fehlerkorrektur-Dateien auf der
Abbild-Ebene zu schützen: Die 
<a href="qa37.php">Datenträger mit Fehlerkorrektur-Dateien</a> 
müssen ebenfalls mit dvdisaster gesichert werden.
</td></tr></table><p>


Weil dies vorausgesetzt wird, enthalten RS01- Fehlerkorrektur-Dateien <i>keinen</i> 
eigenen Schutz gegen Beschädigung! Dies würde auch nicht viel nutzen:
Natürlich könnten die Fehlerkorrektur-Dateien so aufgebaut werden, 
daß sie auch im beschädigten Zustand 
noch eine verminderte Fehlerkorrektur leisten<sup><a href="#footnote1">*)</a></sup>. 
Aber egal wie ausgeklügelt  der innere Schutz-Mechanismus auch wäre, 
es bliebe ein Schutz auf Dateisystem-Ebene mit den oben beschriebenen Nachteilen!<p>

Hinzu kommt, daß die dafür benötigte Rechenzeit und Redundanz besser 
auf der Abbild-Ebene investiert sind: Die Reed-Solomon-Fehlerkorrektur profitiert davon,
wenn Fehlerkorrektur-Informationen über große Datenmengen verteilt werden. 
Das Abbild als Ganzes läßt sich besser schützen als die einzelnen 
Fehlerkorrektur-Dateien darin.

<pre> </pre>
<table width="50%"><tr><td><hr></td></tr></table>

<span class="fs">
<a name="footnote1"><sup>*)</sup></a> Mit dem neuen RS03-Kodierer
erzeugte Fehlerkorrektur-Dateien haben genau diese Eigenschaften,
d.h. sie sind robust gegen Beschädigungen soweit wie die
Fehlerkorrektur-Kapazität reicht. Dies ist möglich da mittlerweile
genügend Rechenleistung und schnelle Festplattenzugriffe zur Verfügung
stehen um die Dateien entsprechend aufzubauen. Zur Zeit der Entwicklung
von RS01 war dies noch nicht der Fall. Aber auch für RS03-Fehlerkorrektur-Dateien
gelten die oben erwähnten Nachteile des Lesens auf Dateisystem-Ebene!
</span>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
