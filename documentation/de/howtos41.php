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
require("../include/screenshot.php");
begin_page();

howto_headline("Datenträger-Abbild rekonstruieren", "Grundeinstellungen", "images/fix-icon.png");
?>

<!--- Insert actual page content below --->

<?php begin_screen_shot("Einstellungsdialog aufrufen.","global-prefs-invoke.png"); ?>
<table><tr><td valign="top"><img src="../images/prefs-icon.png" valign="bottom"></td>
<td>Die nachfolgend besprochenen Reiterkarten finden Sie
im Einstellungsdialog. Das zum Aufruf verwendete Symbol ist
in dem Bildschirmfoto grün markiert (Anklicken vergrößert das Bild).
Das Symbol kann je nach verwendetem Symbol-Thema anders aussehen.</td>
</tr></table>
<?php end_screen_shot(); ?>

Mit den hier gezeigten Einstellungen wird dvdisaster für das Einlesen von beschädigten
Datenträgern konfiguriert. Das Rekonstruieren des Abbilds aus den Fehlerkorrektur-Daten benötigt
keine speziellen Einstellungen.
<pre> </pre>

<?php begin_screen_shot("Reiterkarte \"Abbild\".","fix-prefs-image.png"); ?>
<b>Reiterkarte "Abbild".</b> Wählen Sie zunächst die Art der Fehlerkorrektur-Daten aus. 
Nehmen Sie die Einstellung "ISO/UDF" (grüne Markierung) wenn Sie eine Fehlerkorrektur-Datei haben.
Aktivieren Sie hingegen die Einstellung "ECC/RS02" (blaue Markierung) um einen Datenträger zu
bearbeiten, der direkt mit Fehlerkorrektur-Daten erweitert wurde.<p>
Das angepaßte Leseverfahren nutzt die Informationen aus den Fehlerkorrektur-Daten, um das Einlesen
effizient zu steuern. Aktivieren Sie es mit dem gelb markierten Knopf.<p>
Nehmen Sie die übrigen Einstellungen wie in dem Bildschirmfoto angegeben vor.<p>
<?php end_screen_shot(); ?>

<?php begin_screen_shot("Reiterkarte \"Laufwerk\".","fix-prefs-drive.png"); ?>
<b>Reiterkarte "Laufwerk".</b> Arbeiten Sie in dieser Reiterkarte zunächst 
mit den gezeigten Grundeinstellungen. Bei einigen Laufwerken ergibt die
Einstellung "21h" bei "Raw-Lese-Verfahren" bessere Ergebnisse. Mehr Informationen
dazu gibt es in den <a href="howtos43.php#21h">fortgeschrittenen Einstellungen</a>.
<p>
<?php end_screen_shot(); ?>

<?php begin_screen_shot("Reiterkarte \"Leseversuche\".","fix-prefs-read-attempts.png"); ?>
<b>Reiterkarte "Leseversuche".</b> Die Stärke des angepaßten Leseverfahrens besteht darin,
die noch lesbaren Sektoren auf dem Datenträger zu finden und sich nicht mit dem Einlesen
von defekten Sektoren zu verzetteln. Verwenden Sie daher "raw" lesen (grüne Markierung, kostet keinen
zusätzlichen Aufwand), aber reduzieren Sie zunächst die Anzahl der Leseversuche 
auf ein Minimum (gelbe Markierungen). Stellen Sie für den ersten Leseversuch ein
moderates Abbruchkriterium von 128 unlesbaren Bereichen 
(blaue Markierung) ein und lassen
Sie das Aufbewahren von Roh-Sektoren zunächst abgewählt.
Erst wenn diese Einstellungen nicht genügend Daten liefern können Sie weitere 
<a href="howtos43.php">Anpassungen</a> vornehmen.
<p>
<?php end_screen_shot(); ?>

<pre> </pre>

<b>Nicht verwendete Reiterkarten</b><p>

Die Reiterkarten "Fehlerkorrektur" und "Sonstiges" haben auf das Einlesen
keinen Einfluß. In der Reiterkarte "Darstellung" können Sie die Anzeige
von dvdisaster nach Ihrem Geschmack farblich verändern; dies hat aber ebenfalls
keine Auswirkungen auf das Einlesen von Datenträgern.

<pre> </pre>


<a href="howtos42.php">Abbild einlesen und Daten wiederherstellen...</a>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
