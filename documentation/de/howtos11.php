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
require("../include/screenshot.php");
begin_page();

howto_headline("Datenträger überprüfen", "Grundeinstellungen", "images/scan-icon.png");
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

<?php begin_screen_shot("Reiterkarte \"Abbild\".","scan-prefs-image.png"); ?>
<b>Reiterkarte "Abbild".</b> Es ist wichtig, die Methode zum Ermitteln der
Abbild-Größe korrekt einzustellen. Die Einstellung "ISO/UDF" (grüne Markierung) ist
eine gute Wahl, da sie in so gut wie allen Situationen funktioniert. 
Wählen Sie die Einstellung "ECC/RS02" (rote Markierung)
nur, wenn der Datenträger Fehlerkorrektur-Daten enthält. Die Fehlerkorrektur-Daten verbessern
die Prüfergebnisse, aber die vergebliche Suche nach diesen Daten kostet mehrere Minuten.<p>
Nehmen Sie die übrigen Einstellung wie in dem Bildschirmfoto angegeben vor.<p>
<?php end_screen_shot(); ?>

<pre> </pre>

<?php begin_screen_shot("Reiterkarte \"Laufwerk\".","scan-prefs-drive.png"); ?>
<b>Reiterkarte "Laufwerk".</b> Stellen Sie unter "Laufwerk vorbereiten" die
Zeit ein, die Ihr Laufwerk zum Hochdrehen aus dem Stillstand benötigt 
(typischerweise 5-10 Sekunden; grüne Markierung). 
Dies verhindert, daß Sektoren während des 
Hochdrehens gelesen und möglicherweise als schlecht erkannt werden.<p>
Lassen Sie die anderen Einstellungen zunächst auf den gezeigten Werten;
diese können Sie später noch <a href="howtos14.php">optimieren</a>.<p>
<?php end_screen_shot(); ?>

<pre> </pre>

<?php begin_screen_shot("Reiterkarte \"Leseversuche\".","scan-prefs-read-attempts.png"); ?>
<b>Reiterkarte "Leseversuche".</b> Es ist wichtig, daß Sie den Vorgaben für
diese Reiterkarte folgen, da Sie sonst unnötig Zeit verschwenden, ohne die
Qualität der Überprüfung zu verbessern. Die Option "Sektoren raw lesen" (erste
grüne Markierung) nutzt C2-Analysen und gegebenenfalls weitere vom Laufwerk
bereitgestellte Informationen, um CD-Datenträger noch genauer zu überprüfen.
Bei allen anderen Datenträger-Typen bewirkt diese Einstellung nichts, aber Sie 
können sie immer aktiviert lassen, solange Ihr Laufwerk mit dieser Einstellung bei CDs
keine Schwierigkeiten macht. 
Nach einem Lesefehler sollten nicht weniger als 16 Sektoren übersprungen werden
(zweite grüne Markierung); bei stark beschädigten Datenträgern lohnt sich
eine <a href="howtos14.php">Optimierung mit größeren Werten</a>.<br>
Beim Überprüfen sind mehrere Leseversuche nicht sinnvoll; stellen Sie daher die
Anzahl der Leseversuche in den drei orange markierten Stellen auf 1.
Das Sammeln von Roh-Sektoren ist während der Überprüfung ebenfalls 
nicht empfehlenswert.<p>
<?php end_screen_shot(); ?>

<pre> </pre>

<?php begin_screen_shot("Reiterkarte \"Sonstiges\".","general-prefs-misc.png"); ?>
<b>Reiterkarte "Sonstiges".</b> In dieser Reiterkarte befinden sich momentan nur
Funktionen zum Erstellen von Protokolldateien, die zum Einsenden 
von <a href="feedback.php">Fehlerberichten</a> benötigt werden.
Im Normalbetrieb sollten sie wie gezeigt ausgeschaltet sein.
<?php end_screen_shot(); ?>

<pre> </pre>

<b>Nicht verwendete Reiterkarten</b><p>

Die Reiterkarten "Fehlerkorrektur" und "Dateien" haben auf die Überprüfung 
keinen Einfluß. In der Reiterkarte "Darstellung" können Sie die Anzeige
von dvdisaster nach Ihrem Geschmack farblich verändern; dies hat aber ebenfalls
keine Auswirkungen auf die Überprüfung von Datenträgern.

<pre> </pre>


<a href="howtos12.php">Überprüfung durchführen...</a>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
