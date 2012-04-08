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
require("../include/screenshot.php");
begin_page();
$way=$_GET["way"];
if($way & 1) $make_iso_action=$way&2;
else         $make_iso_action=$way|1;
if($way & 2) $write_iso_action=$way&1;
else	     $write_iso_action=$way|2;
?>

<!-- Insert actual page content below -->

<?php
howto_headline("Fehlerkorrektur-Daten auf dem Datenträger ablegen", "Durchführen", "images/create-icon.png");
?>

dvdisaster ist ein Spezialist für das Arbeiten mit Fehlerkorrektur-Daten und das
Lesen von defekten Datenträgern. Das Brennen von Dateien auf einen Datenträger ist,
einschließlich der Erzeugung des zugehörigen Abbildes, eine ganz andere und ebenso
komplexe Angelegenheit. Für diese Aufgabe wollen wir in dvdisaster nicht das Rad
neu erfinden, sondern die vorhandene Brennsoftware weiterverwenden, die Sie mit Ihrem
Brenner erhalten oder dazugekauft haben. <p>

<pre> </pre>

<a name="a"></a>
<table>
<tr>
<td class="w200x" align="center">
<?php
echo "<a href=\"howtos33.php?way=$make_iso_action\">\n";
?>
<img src="thumbnails/make-iso1.png" alt="Abb.: Abbild-Datei erzeugen" class="noborder">
<br><img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten" class="noborder"></a></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Erstellen Sie ein ISO-Abbild</b> mit Ihrer Brennsoftware.
Wählen Sie in der Brennsoftware die Dateien aus, 
die Sie auf den Datenträger brennen möchten. 
Starten Sie den Brennvorgang jedoch noch nicht, sondern erzeugen
Sie zunächst ein ISO-Abbild auf der Festplatte. 
Klicken Sie auf das Bild zur linken Seite um ein
<?php
echo "<a href=\"howtos33.php?way=$make_iso_action\">\n";
echo "ausführliches Beispiel einzublenden</a>.\n";
?>
</td>
</tr>
</table>

<?php
if($way&1)
{
?>

<hr>

<b>Ausführliches Beispiel: ISO-Abbild auf der Festplatte erzeugen</b>. 
Da es viele verschiedene Brennprogramme gibt, stellen wir Ihnen die benötigten
Schritte anhand des GNU/Linux-Programmes <i>K3b</i> stellvertretend für alle 
anderen vor. Es dürfte für Sie ein Leichtes sein, 
die entsprechenden Aktionen mit dem von Ihnen verwendeten 
Programm nachzuvollziehen.<p>

<hr>

<?php begin_screen_shot("Neues Projekt beginnen","make-iso1.png"); ?>
<b>Beginnen Sie ein neues Datenträger-Projekt.</b>
Rufen sie zunächst Ihr Brennprogramm auf.
Viele Brennprogramme erwarten, daß Sie ein neues Projekt beginnen,
in dem die Auswahlen zum Brennen eines neuen Datenträgers getätigt werden.<p>
In K3b: <i>Beginnen Sie ein neues Projekt durch Klicken auf die weiß hervorgehobene
Schaltfläche ("Neue Daten-CD") im unteren Bereich des Fensters, das sich nach
dem Programmstart öffnet.</i>
<?php end_screen_shot(); ?>

<hr>

<?php begin_screen_shot("Dateien auswählen","make-iso2.png"); ?>
<b>Wählen Sie die Dateien zum Brennen aus.</b>
Typischerweise gibt es einen Auswahldialog aus dem Sie die Dateien wählen
oder in das Projekt ziehen können. <p>

In K3b: <i>Wählen Sie die gewünschten Dateien in der oberen Fensterhälfte aus.  
In dem Beispiel wurden die Dateien <i>backup.tar.gz</i>,
<i>win.zip</i> und <i>work.tar.gz</i> zum Brennen auf CD ausgewählt. Die momentane
Auswahl wird in der unteren Fensterhälfte angezeigt.</i><p> 

<b>Wichtig:</b> Achten Sie darauf, den Datenträger nicht vollständig zu füllen. 
Es müssen mindestens 20% Platz für die Fehlerkorrektur-Daten übrig bleiben.<p>

In K3b: <i> In der grünen Leiste wird unten der Platzbedarf auf dem 
Datenträger angezeigt (558,9 MB).</i>
<?php end_screen_shot(); ?>

<hr>

<?php begin_screen_shot("Brennsoftware konfigurieren","make-iso2.png"); ?>
<b>Brennsoftware konfigurieren.</b> Das Brennprogramm wird Sie vor dem
Schreiben das Ziel des Brennvorgangs
auswählen lassen. Wählen Sie hier <b>nicht</b> den Brenner, sondern wie
nachfolgend beschrieben das Erstellen eines ISO-Abbildes auf der Festplatte.<p>

<b>Tip:</b> Nehmen Sie zur Sicherheit alle Datenträger aus den Laufwerken
um zu verhindern daß Sie den Brennvorgang versehentlich jetzt schon auslösen.<p>

In K3b: <i>Öffnen Sie den Einstellungs-Dialog mit Hilfe des "Brennen"-Knopfes an der linken Seite des Fensters.</i>
<?php end_screen_shot(); ?>

<?php begin_screen_shot("Abbild-Schreiben auswählen","make-iso3.png"); ?>
<b>Schreiben des Abbildes auswählen.</b> Bei vielen Brennprogrammen können Sie einfach
anklicken, daß Sie ein ISO-Abbild auf der Festplatte erzeugen möchten.
Wenn Ihr Programm diese Auswahl nicht bietet können Sie möglicherweise 
anstelle des Brenners einen "Abbild-Aufzeichner"
("Image-Recorder", oder ähnlich bezeichnet) auswählen, der den Brennvorgang in
ein ISO-Abbild auf der Festplatte umleitet.<p>

In K3b: <i>Wählen Sie die Reiterkarte "Brennen". Aktivieren Sie dort die grün markierte Option
"Nur Abbild-Datei erzeugen".</i>
<?php end_screen_shot(); ?>

<?php begin_screen_shot("Abbild-Datei auswählen","make-iso4.png"); ?>
<b>Abbild-Datei und-Typ auswählen.</b>
Wählen Sie den Speicherort, Namen und Typ für die Abbild-Datei aus. 
Verwenden Sie nur Abbilder vom Typ ".iso" oder ".udf"! Andere Dateiformate 
wie z.B. ".nrg" werden von dvdisaster nicht unterstützt; typischerweise werden diese
Abbilder durch die Bearbeitung mit dvdisaster unbrauchbar ohne daß es eine
Fehlermeldung gibt.<p>

In K3b: <i>Wählen Sie die Reiterkarte "Abbild". Geben Sie dort den Speicherort
für die Abbild-Datei an (im Beispiel "medium.iso" im Unterverzeichnis "/var/tmp/cg").
K3b erzeugt immer .iso-Abbilder, d.h. zum Typ ist nichts weiter einzustellen.</i>
<?php end_screen_shot(); ?>

<hr>

<table>
<tr>
<td class="w200x" align="center">
<img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten" class="noborder"></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"></td>
</tr>
</table>

<?php
}  /* end from if($way&1) above */
?>

<a name="b"></a>
<table>
<tr>
<td class="w200x" align="center">
<img src="../images/good-image.png" alt="Symbol: Vollständiges Abbild (von einem unbeschädigten Datenträger)" class="noborder">
<br><img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten" class="noborder"></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Arbeiten Sie mit dvdisaster</b> weiter, sobald
Sie das Abbild haben.
Vergewissern Sie sich zunächst, daß dvdisaster wie in den 
<a href="howtos32.php">Grundeinstellungen</a> beschrieben konfiguriert ist.
</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center">
<a href="howtosa2.php">
<img src="../images/select-image2.png" alt="Bedienelement: Abbild-Datei auswählen (Eingabefeld und Knopf)" class="noborder">
<br><img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten" class="noborder"></a></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Wählen Sie in dvdisaster die Abbild-Datei aus,</b> 
die Sie eben mit Ihrer Brennsoftware erzeugt haben.</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center">
<a href="howtosa4.php">
<img src="images/create-icon.png" alt="Bedienelement: Erzeugen (Auswahlknopf)" class="noborder">
<br><img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten" class="noborder"></a></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Erweitern Sie das Abbild mit Fehlerkorrektur-Daten</b> durch
Klicken auf den "Erzeugen"-Knopf.</td>
</tr>
</table>

<?php begin_howto_shot("Fehlerkorrektur-Daten erzeugen.","make-ecc1.png", "down-arrow.png"); ?>
<b>Beobachten Sie den Fortschritt des Vorgangs.</b>
Je nach der Größe des Abbild und dem noch verfügbaren freien Platz kann
die Erstellung der Fehlerkorrektur-Daten eine Weile dauern. Für die
Bearbeitung eines DVD-Abbildes mit 20-30% freiem Platz müssen Sie auf einem
aktuellen Rechner ca. 5-10 Minuten einplanen.
<?php end_howto_shot(); ?>

<?php begin_howto_shot("Abbild-Größen vergleichen.","make-ecc2.png", "down-arrow.png"); ?>
<b>Hinweis:</b> dvdisaster erzeugt kein neues Abbild, sondern das vorhandene wird
durch die Fehlerkorrektur-Daten vergrößert. Wenn Sie das Abbild vorher und nachher
im Dateimanager betrachten, sehen Sie wie sich die Größe des Abbild verändert.
<?php end_howto_shot(); ?>

<a name="c"></a>
<table>
<tr>
<td class="w200x" align="center">
<?php
echo "<a href=\"howtos33.php?way=$write_iso_action\">\n";
?>
<img src="thumbnails/write-iso1.png" alt="Abb.: Abbild auf einen Datenträger schreiben" class="noborder"></a></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Schreiben Sie das erweiterte ISO-Abbild</b> auf den Datenträger.
Wählen Sie in der Brennsoftware das eben erweiterte Abbild aus und führen Sie
den Brennvorgang durch. Klicken Sie auf das Bild zur linken Seite um ein
<?php
echo "<a href=\"howtos33.php?way=$write_iso_action\">\n";
echo "ausführliches Beispiel einzublenden</a>.\n";
?>
</td>
</tr>
</table>

<?php
if($way&2)
{
?>
<table>
<tr>
<td class="w200x" align="center">
<img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten" class="noborder"></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"></td>
</tr>
</table>

<hr>

<b>Ausführliches Beispiel: ISO-Abbild auf den Datenträger schreiben</b>.<p>

<?php begin_screen_shot("Schreiben des Abbilds auswählen","write-iso1.png"); ?>
<b>Schreiben des Abbilds auswählen.</b>
Rufen sie zunächst Ihr Brennprogramm wieder auf. Wechseln Sie in die Betriebsart
des Programms, mit der Sie ein bereits auf der Festplatte vorliegendes Abbild 
brennen können.<p>

In K3b: <i>Klicken Sie auf die weiß hervorgehobene
Schaltfläche ("ISO-Abbild auf CD brennen...") im unteren Bereich des Fensters, das sich nach
dem Programmstart öffnet.</i>
<?php end_screen_shot(); ?>

<hr>

<?php begin_screen_shot("Abbild-Datei auswählen","write-iso2.png"); ?>
<b>Abbild-Datei auswählen.</b>
Wählen Sie die Abbild-Datei aus, die Sie eben mit dvdisaster um Fehlerkorrektur-Daten erweitert
haben. 
<p>

In K3b: <i>Verwenden Sie das grün markierte Bedienfeld um den Namen der Abbild-Datei auszuwählen
oder direkt einzugeben.</i>
<?php end_screen_shot(); ?>

<?php begin_screen_shot("Weitere Einstellungen","write-iso2.png"); ?>
<b>Weitere Einstellungen.</b>
Stellen Sie als Brennmodus "DAO" ("disk at once", Datenträger in einem Durchgang beschreiben) ein,
sofern Ihr Brenner dies unterstützt. Dies verbessert die Kompatibilität des Datenträgers mit
der Fehlerkorrektur. Dadurch wird außerdem verhindert, daß später noch weitere Sitzungen an den
Datenträger angehängt werden, wodurch die Fehlerkorrekur-Daten zerstört würden.
<p>

In K3b: <i>Wählen Sie "DAO" in dem gelb markierten Bedienfeld.</i>
<?php end_screen_shot(); ?>

<?php begin_screen_shot("Datenträger schreiben","write-iso3.png"); ?>
<b>Datenträger schreiben.</b>
Beginnen Sie nun den Schreibvorgang.
<p>

In K3b: <i>Klicken Sie auf den "Start"-Knopf in dem Fenster aus dem vorherigen Bildschirmfoto.</i>
<?php end_screen_shot(); ?>

<hr>

<?php
}  /* end from if($way$2) above */
?>

<table>
<tr>
<td class="w200x" align="center">
<img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten" class="noborder"></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"></td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center">
<img src="../images/good-cd-ecc.png" alt="Symbol: Abgesicherter Datenträger mit Fehlerkorrektur-Daten" class="noborder"></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Fertig!</b> Sie haben jetzt eine mit Fehlerkorrektur-Daten abgesicherte CD erstellt.</td>
</tr>
</table>

<pre> </pre>

<b>Verwandte Informationen</b>

<ul>
<li><a href="howtos90.php">Überprüfen Sie, ob der Brennvorgang die Fehlerkorrektur-Daten verändert hat.</a><p>
Dies ist empfohlen, wenn Sie zum ersten Mal mit Ihrer Brennsoftware ein erweitertes Abbild brennen.
</li>
</ul>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
