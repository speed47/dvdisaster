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

howto_headline("Fehlerkorrektur-Datei erstellen", "Archivieren", "images/create-icon.png");
?>

<!--- Insert actual page content below --->
<h3>Tips zum Aufbewahren der Fehlerkorrektur-Datei</h3>

Zur Zeit gibt es kaum Wechselspeichersysteme, 
die eine wirtschaftliche Alternative zu CD/DVD/BD-Formaten darstellen. 
Vermutlich werden Sie daher Ihre Fehlerkorrektur-Dateien 
auch auf diesen Formaten speichern.<p>

Dagegen ist nichts einzuwenden, aber Sie müssen sich dabei bewußt sein, daß sich Ihre Nutzdaten und die Fehlerkorrektur-Dateien auf Speichermedien mit ähnlicher Verläßlichkeit befinden. Wenn Lesefehler auf einem zu rekonstruierenden Datenträger auftreten, so müssen Sie damit rechnen, daß die zur gleichen Zeit erstellte Scheibe mit den Fehlerkorrektur-Dateien ebenfalls nicht mehr vollständig lesbar ist.<p>

<table width=100%><tr><td bgcolor=#000000 width=2><img width=1 height=1 alt=""></td>
<td>&nbsp;</td>
<td>Es mag überraschend klingen, aber es kann nicht sichergestellt werden,
daß eine Fehlerkorrektur-Datei funktionsfähig bleibt, wenn sie auf einem beschädigten
Datenträger gespeichert ist.
Die <a href="http://dvdisaster.net/legacy/de/background20.html">technischen Hintergründe</a>
sind in der alten Dokumentation zu dvdisaster erklärt.
</td></tr></table><p>

Deshalb ist es wichtig, die Fehlerkorrektur-Dateien genauso wie die übrigen Daten zu schützen. 
Das heißt Sie müssen für den Datenträger, auf dem die Fehlerkorrektur-Dateien liegen,
ebenfalls Fehlerkorrektur-Daten erzeugen. Dazu zwei Anregungen: 

<ol>
<li>Fehlerkorrektur-Dateien auf eigenen Datenträgern sammeln:<p>

Verwenden Sie zusätzliche Datenträger, auf denen Sie ausschließlich Fehlerkorrektur-Dateien sammeln.
Wenn Sie höchstens 80% des Datenträgers für die Fehlerkorrektur-Dateien benutzen,
können Sie den Datenträger anschließend <a href="howtos30.php"> mit Fehlerkorrektur-Daten</a>
erweitern, bevor Sie ihn brennen. So können Sie den Inhalt des Datenträgers wiederherstellen, 
falls Sie später die Fehlerkorrektur-Dateien brauchen und es Probleme mit dem Datenträger gibt.<p></li>

<li>Fehlerkorrektur-Dateien jeweils auf dem nächsten Datenträger speichern:<p>

Nehmen wir an, Sie nutzen ihre Datenträger für eine fortlaufende Datensicherung. Sie sammeln
also so lange Dateien, bis Sie einen kompletten Datenträger davon brennen können.
Brennen Sie dann den ersten Datenträger wie gewohnt und erzeugen Sie eine Fehlerkorrektur-Datei
davon. Nehmen Sie die Fehlerkorrektur-Datei in den Satz von Dateien auf, die Sie für den zweiten
Datenträger sammeln, und schreiben Sie sie mit auf den zweiten Datenträger. Vom zweiten Datenträger
erzeugen Sie dann wiederum eine Fehlerkorrektur-Datei, die Sie zusammen mit anderen Dateien
auf den dritten Datenträger brennen. Wenn Sie so weitermachen, sind stets alle Fehlerkorrekur-Dateien bis auf die vom letzten Datenträger mit dvdisaster gesichert.<p>

Nach Murphys Law können Sie natürlich Pech haben, daß sich alle Datenträger der Kette
als beschädigt erweisen. In diesem Fall müssen Sie alle Datenträger von hinten nach vorne
rekonstruieren, um an die Daten des ersten wieder heranzukommen ;-)
</li>
</ol>

<!--- do not change below --->

<?php

# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
