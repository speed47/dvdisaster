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

<h3 class="top">Technische Eigenschaften der Fehlerkorrektur</h3>

Diese Seite skizziert die Grundideen hinter dvdisaster, damit Sie abschätzen können,
ob es Ihren Anforderungen zur Datensicherung genügt. Im Zweifelsfall sollten Sie
dvdisaster nicht einsetzen oder zusätzliche Sicherungsstrategien anwenden.<p>

<b>Fehlerkorrektur-Verfahren.</b> &nbsp; dvdisaster verwendet einen 
<a href="http://de.wikipedia.org/wiki/Reed-Solomon-Code">Reed-Solomon</a>-Kode
mit einem auf die Behandlung von Auslöschungen optimierten Fehlerkorrektur-Algorithmus.
Die Implementierung bezieht eine Menge Inspiration und Programmcode aus der
hervorragenden <a href="http://www.ka9q.net/code/fec/">Reed-Solomon-Bibliothek</a>
von <a href="http://www.ka9q.net/">Phil Karn</a>.

<p>

Bei der Verwendung von <a href="howtos20.php">Fehlerkorrektur-Dateien</a>
bilden in der <a href="howtos22.php#ecc">Grundeinstellung</a>
jeweils 223 Datenträger-Sektoren einen Fehlerkorrektur ("ECC") - Bereich.
Auf dem Datenträger auftretende Lesefehler werden als Auslöschungen betrachtet,
so daß pro ECC-Bereich bis zu 
32 defekte Sektoren<sup><a href="#footnote1">*)</a></sup> rekonstruierbar sind.<p>

Die 223 Sektoren werden so ausgewählt, daß sie sich gleichmäßig über die gesamte
Datenträger-Oberfläche verteilen. Dadurch können große zusammenhängende Bereiche von defekten Sektoren 
korrigiert werden, bevor die kritische Anzahl von 
32 Defekten<sup><a href="#footnote1">*)</a></sup> pro ECC-Bereich erreicht wird. 
Dieses Fehlermuster ist besonders typisch für alternde Datenträger, bei denen im Außenbereich gehäuft 
Fehler auftreten, und für Kratzer entlang der Datenspirale. <p>

Radiale oder diagonale Kratzer werden hingegen in der Regel schon im Laufwerk selbst korrigiert.
Für diese Fälle ist die verwendete Fehlerkorrektur weder besonders gut noch besonders schlecht geeignet.<p>

<b>Grenzen der Fehlerkorrektur.</b> &nbsp; Im schlechtesten Fall reichen schon 
33 defekte Sektoren<sup><a href="#footnote1">*)</a></sup> auf dem
Datenträger, um seine Wiederherstellung zu verhindern. Damit diese Wirkung eintritt, müssen die Fehler
wie ein Schrotschuß über den Datenträger verteilt sein und alle im gleichen ECC-Bereich liegen -
das ist eher unwahrscheinlich. <br>
Erfahrungstests haben ergeben, daß bei normaler Alterung ca. 10% an Sektoren ausfallen können,
bevor die kritsche Anzahl von 33 Defekten pro ECC-Bereich<sup><a href="#footnote1">*)</a></sup>
erreicht wird.<br>
Bei Kratzern wird die Ausfallschwelle früher erreicht; deshalb empfiehlt sich eine ständige Sichtkontrolle
und ein sofortiges Umkopieren von Datenträgern, die durch Kratzer verursachte Lesefehler aufweisen. <p>

<b>Technische Einschränkungen.</b> &nbsp; Viele Laufwerke erkennen den Datenträger nicht mehr, 
wenn die Einführungszone ("Lead in") vor dem ersten Sektor (nahe dem Innenloch) beschädigt ist.
In diesem Fall können Sie dvdisaster nicht mehr anwenden, um den Inhalt des Datenträgers zu retten. <p>

Es ist <i>nicht möglich</i>, die Qualität <b>minderwertiger Datenträger</b> durch dvdisaster aufzuwerten.
Billige Rohlinge können bereits nach wenigen Tagen so stark verfallen, daß die Kapazität des
Fehlerkorrekturkodes überschritten wird. 

<pre> </pre>
<table width="50%"><tr><td><hr></td></tr></table>

<span class="fs">
<a name="footnote1"><sup>*)</sup></a> Die angegebene Grenze 
von 32 korrigierbaren Defekten pro ECC-Bereich ist die Grundeinstellung.
Sie können hier andere Werte <a href="howtos22.php#ecc">einstellen</a>
und so die Leistungsfähigkeit der Fehlerkorrektur anpassen.
Wenn die <a href="howtos30.php">Fehlerkorrektur-Daten auf dem Datenträger selbst untergebracht</a>
werden, hängt die Anzahl der korrigierbaren Fehler von der Restkapazität des
Datenträgers ab.
</span>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
