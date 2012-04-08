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

<h3 class="top">Tips zum Aufbewahren der Fehlerkorrektur-Datei</h3>

Zur Zeit gibt es kaum Wechselspeichersysteme, 
die eine wirtschaftliche Alternative zu CD/DVD/BD-Formaten darstellen.
Vermutlich werden Sie daher Ihre Fehlerkorrektur-Dateien auch auf CD,DVD
oder BD speichern. <p>

Dagegen ist nichts einzuwenden, aber Sie müssen sich dabei bewußt sein,
daß sich Ihre Nutzdaten und die Fehlerkorrektur-Dateien auf 
Speichermedien mit ähnlicher Verläßlichkeit befinden.
Wenn Lesefehler
auf einem zu rekonstruierenden Datenträger auftreten, so müssen Sie damit rechnen,
daß die zur gleichen Zeit erstellte Scheibe mit den Fehlerkorrektur-Daten
ebenfalls nicht mehr vollständig lesbar ist.<p>

Deshalb ist es wichtig, Fehlerkorrektur-Dateien
genauso wie die übrigen Daten zu schützen<sup><a href="#footnote1">*)</a></sup>. 
Am einfachsten geht dies, wenn Sie die
Fehlerkorrektur-Dateien in Ihre normale Datensicherung mit einbeziehen. 
Dazu zwei Anregungen:<p>

<b>1. Fehlerkorrektur-Dateien auf eigenen Datenträgern sammeln:</b><p>

Wenn Sie Fehlerkorrektur-Dateien auf extra dafür vorgesehenen Datenträgern speichern,
ist es <a href="qa32.php#eccfile">wichtig</a>, diese Datenträger ebenfalls
mit dvdisaster zu schützen. Um zu verhindern, daß man eine endlose Kette 
(Fehlerkorrektur-Dateien über Fehlerkorrektur-Dateien über ...) erhält, 
hilft folgender Kniff:<p>

Angenommen, Sie können jeweils 5 Fehlerkorrektur-Dateien pro Datenträger speichern.
Legen Sie die ersten fünf Fehlerkorrektur-Dateien auf dem ersten Datenträger ab
und erzeugen Sie dann eine weitere Fehlerkorrektur-Datei für diesen Datenträger.
Speichern Sie die neu erzeugte Fehlerkorrektur-Datei zusammen mit vier weiteren auf dem
zweiten Datenträger. Wenn Sie so weitermachen, sind stets alle Fehlerkorrekur-Dateien
bis auf diejenigen vom letzten Datenträger mit dvdisaster gesichert.<p>

<b>2. Fehlerkorrektur-Dateien jeweils auf dem nächsten Datenträger speichern:</b><p>

Wenn Sie Ihre Datenträger nicht randvoll mit Nutzdaten 
(also z.B. mit weniger als 4GB für eine einlagige DVD) beschreiben,
können Sie die Fehlerkorrektur-Dateien innerhalb einer Serie von Datenträgern
jeweils auf dem nächsten Datenträger ablegen.<p>

<pre> </pre>
<table width="50%"><tr><td><hr></td></tr></table>

<span class="fs">
<a name="footnote1"><sup>*)</sup></a> 
Oder Sie verwenden anstelle von Fehlerkorrektur-Dateien ein mit RS02 oder
RS03 <a href="howtos30.php"> erweitertes Abbild.</a>
</span>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
