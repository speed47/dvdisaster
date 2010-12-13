<?php
# dvdisaster: German homepage translation
# Copyright (C) 2004-2010 Carsten Gnörlich
#
# UTF-8 trigger: äöüß 
#
# Include our PHP sub routines, then call begin_page()
# to start the HTML page, insert the header, 
# navigation and news if appropriate.

require("../include/dvdisaster.php");
require("../include/download.php");
begin_page();
$show_all=$_GET["showall"];
?>

<!--- Insert actual page content below --->

<h3>Alpha-/Entwicklerversionen</h3>

<b>Helfen Sie beim Testen!</b> Hier finden Sie experimentelle 
dvdisaster-Versionen, die auf dem Weg zur nächsten "stabilen" Version
entstehen.<p>

<b>Ein Wort der Vorsicht:</b> Diese Version befindet sich noch im 
Entwicklungsvorgang und einige Teile sind noch nicht fertig. Sie kann
Programmfehler enthalten und nicht offensichtliche Fehlfunktionen haben,
auch in Teilen die in älteren Versionen bereits funktionierten.
Bearbeiten Sie mit dieser Version keine wichtigen Daten und
verwenden Sie die erzeugten Abbilder und Fehlerkorrektur-Daten
nicht für Archivierungszwecke; dafür
ist die <a href="download.php">stabile Version 0.72</a> gedacht.

<hr>

<h3>Geplante Änderungen in der neuen Version</h3>

Allgemein:

<ul>
<li> Einbauen mehrerer kleinerer Erweiterungen, die während des langen
0.72er-Entwicklungszyklus liegengeblieben sind. <i>[noch nicht angefangen]</i></li>
<li> Entfernung nicht mehr benötigter Funktionen. <i>[fertig]</i></li>
<li> Aufräumen der Kodebasis und Vorbereitung für Multithreading und
Mehrkernprozessoren. <i>[in Bearbeitung]</i></li>
<li> Entwicklung des Multithreading-fähigen RS03-Kodierers <i>[in Bearbeitung]</i></li>
<li> Dokumentation von RS03. <i>[noch nicht angefangen]</i></li>
</ul>

Windows:

<ul>
<li> Aktualisierung des GTK+-Toolkits und der übrigen Entwicklungsumgebung. <i>[fertig]</i></li>
<li> Erhöhen der Systemvoraussetzungen auf Windows 2000 oder neuer (ältere
Windows-Versionen werden von der Entwicklungsumgebung nicht mehr unterstützt). 
Damit wird auch die Unterstützung von ASPI-Treibern und das Aufteilen von 
Dateien in 2G-Segmente überflüssig. <i>[fertig]</i></li>
</ul>

MacOS:

<ul>
<li> Aktualisierung des GTK+-Toolkits und weitere Workarounds für die
Benutzeroberfläche. <i>[in Bearbeitung]</i></li>
</ul>

<hr>

<h3>Herunterladen</h3>
<a name="download"></a>

Die Alpha-Versionen verwenden das gleiche Paketformat wie die normalen
Versionen.<p>

<table class="download" cellpadding="0" cellspacing="5">
<tr><td><b>dvdisaster-0.79</b></td><td align="right">21-Nov-2010</td></tr>
<tr bgcolor="#000000"><td colspan="2"><img width=1 height=1 alt=""></td></tr>
<tr><td colspan="2">
  <table>
<?php    
    download_version("0.79.3", 0, "hidden", "hidden", "hidden");

    if($show_all == 0)
    {  echo "    <tr><td colspan=\"2\"><a href=\"download40.php?showall=1#download\">Ältere Veröffentlichungen des 0.79er-Versionszweiges anzeigen</a></td></tr>\n";
    }
    else 
    {  echo "    <tr><td colspan=\"2\"><a href=\"download40.php?showall=0#download\">Ältere Veröffentlichungen des 0.79er-Versionszweiges verbergen</a></td></tr>\n";
       echo "    <tr><td colspan=\"2\"> </td></tr>\n";

       download_version("0.79.2", 1, "378ed135c2faf0eaf643125d1f7726c6", "f673e41b5ddc31a6ecb48a5f053de885", "0b4c0b46e827c7f796416473511ab036");

       download_version("0.79.1", 1, "ba6d0178dc03119080e07ef0a2967c38", "none", "b4c62833a2447097950b563e4a7b2065");
    }
?>
  </table>
</td></tr>
<tr bgcolor="#000000"><td colspan="2"><img width=1 height=1 alt=""></td></tr>
<tr><td colspan="2">

<b>Alle Plattformen:</b> Diese Versionen enthalten umfangreiche Änderungen an
den inneren Strukturen im Vergleich zu Version 0.72.x. Bitte verwenden Sie sie
mit Vorsicht.<p>

<b>0.79.3</b> (21-Nov-2010)<br>
<ul>
<li>Unter Linux wird ab dieser Version als Voreinstellung 
der SG_IO-Treiber zum Zugriff auf die optischen Laufwerke verwendet; der
bisher benutzte CDROM_SEND_PACKET-Treiber kann optional ausgewählt werden.
In den vorherigen dvdisaster-Versionen war es genau anders herum; in 
den  gegenwärtigen Linux-Kerneln hat der SG_IO-Treiber aber die 
bessere Kompatibilität.</li>
<li>Michael Klein hat eine Altivec-Optimierung für den RS03-Kodierer
beigesteuert.</li>
</ul>

<b>0.79.2</b> (28-Feb-2010)<br>
<ul>
<li>Für Mac OS X ist wieder ein Binärpaket verfügbar. Die Entwicklungsumgebung
wurde auf einen aktuellen Stand gebracht; dies hat einige Probleme mit der
graphischen Darstellung behoben.
</li>
<li>
Die Entwicklung von RS03 schreitet weiter voran,
ist aber noch längst nicht abgeschlossen. 
</li>
</ul>

<b>0.79.1</b> (07-Feb-2010)<br>
<ul>
<li>Die SCSI-Schicht enthält eine Umgehungslösung für fehlerhafte Chipsätze,
die in neueren Laufwerken verbaut sein können. Das Ausführen einer Lese- oder
"Prüfen"-Operation kann bei diesen Laufwerken dazu führen daß das System
einfriert. Das Problem ist besonders ausgeprägt bei Windows XP, kann aber
auch bei anderen Betriebssystemen auftreten. Bitte testen Sie ob die
betroffenen Laufwerke nun korrekt arbeiten, und ob keine Fehlfunktionen
bei Laufwerken auftreten, deren Verhalten vorher in Ordnung war.</li> 
<li>Eine Referenz-Implementierung
des RS03-Kodierers ist enthalten.
Diese Version dient hauptsächlich dazu, daß interessierte Personen den
Kodierer mit seiner <a href="download50.php">Spezifikation</a> vergleichen
können. Seien Sie vorsichtig und verwenden Sie ihn nicht für produktives
Arbeiten. Die endgültige Version wird mit dvdisaster 0.80 veröffentlicht.</li>
</ul>
<b>Windows:</b> Die gesamte Entwicklungsumgebung einschließlich der mitgelieferten
Programmbibliotheken wurde erneuert. Bitte probieren Sie aus ob die
graphische Benutzeroberfläche und die Übersetzungen der Bildschirmtexte
wie erwartet angezeigt werden.
</td></tr></table><p>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
