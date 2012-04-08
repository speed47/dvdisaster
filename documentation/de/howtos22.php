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

howto_headline("Fehlerkorrektur-Datei erstellen", "Grundeinstellungen", "images/create-icon.png");
?>

<!-- Insert actual page content below -->

<?php begin_screen_shot("Einstellungsdialog aufrufen.","global-prefs-invoke.png"); ?>
<table><tr><td class="valignt"><img src="../images/prefs-icon.png" alt="Bedienelement: Einstellungen (Aufruf-Knopf)" class="valignb"></td>
<td>Die nachfolgend besprochenen Reiterkarten finden Sie
im Einstellungsdialog. Das zum Aufruf verwendete Symbol ist
in dem Bildschirmfoto grün markiert (Anklicken vergrößert das Bild).
Das Symbol kann je nach verwendetem Symbol-Thema anders aussehen.</td>
</tr></table>
<?php end_screen_shot(); ?>

<hr>

<a name="read"><b>Einstellungen zum Einlesen des Abbilds</b></a><p>

<table width="100%" cellspacing="5">
<tr>
<td><img src="../images/good-image.png" alt="Symbol: Vollständiges Abbild"></td>
<td>Wenn Sie bereits ein ISO-Abbild des Datenträgers vorliegen haben, 
dann können sie die nächsten beiden Reiterkarten überspringen und gleich 
mit den <a href="#ecc">Einstellungen zur Fehlerkorrektur-Datei beginnen</a>.
Sie brauchen allerdings ein richtiges "ISO"-Abbild; andere Abbild-Formate wie zum Beispiel .nrg 
erzeugen keine brauchbaren Fehlerkorrektur-Dateien.
</td>
</tr>
</table><p>

<?php begin_screen_shot("Reiterkarte \"Abbild\".","create-prefs-image.png"); ?>
<b>Reiterkarte "Abbild".</b> Achten Sie unbedingt darauf,
daß "ISO/UDF" zur Ermittlung der Abbild-Größe eingestellt ist und das lineare
Leseverfahren verwendet wird. Diese beiden Einstellungen sind grün markiert.
Lassen Sie die übrigen Einstellungen ausgeschaltet.<p>
<?php end_screen_shot(); ?>

<pre> </pre>

<?php begin_screen_shot("Reiterkarte \"Laufwerk\".","create-prefs-drive.png"); ?>
<b>Reiterkarte "Laufwerk".</b> Stellen Sie unter "Laufwerk vorbereiten" die
Zeit ein, die Ihr Laufwerk zum Hochdrehen aus dem Stillstand benötigt 
(typischerweise 5-10 Sekunden; grüne Markierung). 
Dies verhindert, daß Sektoren während des 
Hochdrehens gelesen und möglicherweise als schlecht erkannt werden.
Lassen Sie die anderen Einstellungen auf den gezeigten Werten stehen.<p>
<?php end_screen_shot(); ?>

<?php begin_screen_shot("Reiterkarte \"Leseversuche\".","create-prefs-read-attempts.png"); ?>
<b>Reiterkarte "Leseversuche".</b> Die grün markierte Option "Sektoren raw lesen" nutzt vom Laufwerk 
bereitgestellte Zusatzinformationen, um zu überprüfen daß die Daten richtig gelesen wurden.
Dies ist hilfreich, da wir natürlich ein Interesse daran haben, Fehlerkorrektur-Dateien von einem
korrekten Abbild zu erstellen. Andererseits sind Fehlerkorrektur-Dateien ohnehin nur für 
gute, vollständig lesbare Datenträger erzeugbar. Daher können wir auf mehrere Leseversuche
und das Abspeichern von Roh-Sektoren verzichten. 
<?php end_screen_shot(); ?>

<hr>

<a name="ecc"><b>Einstellungen zur Fehlerkorrektur</b></a><p>

<?php begin_screen_shot("Reiterkarte \"Fehlerkorrektur\".","create-prefs-ecc.png"); ?>
<b>Reiterkarte "Fehlerkorrektur".</b> Wählen Sie zunächst in der Liste neben "Abspeichern in:" den Eintrag
"Fehlerkorrektur-Datei (RS01)" (grüne Markierung). Mit der Auswahl der Redundanz treffen Sie eine grundlegende Entscheidung:
Eine Fehlerkorrektur-Datei mit x% Redundanz kann später im günstigsten Fall bis zu x% an Lesefehlern korrigieren.
Weil der Idealfall natürlich selten eintritt, sollten Sie die Redundanz großzügig 
mit einer der gelb markierten Möglichkeiten auswählen:

<ul>
<li>Die Voreinstellungen "normal" und "hoch" liefern eine Redundanz von 14.3% bzw. 33.5%. Mit diesen beiden Einstellungen werden Fehlerkorrektur-Dateien durch optimierten Programmkode besonders schnell erzeugt.</li>
<li>Nach Aktivieren des Punktes "Andere" können Sie die gewünschte Redundanz direkt in Prozent eingeben.</li>
<li>Durch Aktivieren des "Verwende höchstens"-Punktes können Sie die Größe der Fehlerkorrektur-Datei in MB vorgeben. In diesem Fall wählt dvdisaster eine geeignete Redundanz, damit die Fehlerkorrektur-Datei nicht größer als angegeben wird.</li>
</ul>

Die Redundanz bestimmt gleichzeitig auch den Speicherplatzverbrauch der Fehlerkorrektur-Datei; die Datei wird bei
x% Redundanz ungefähr die Größe von x% des Abbilds erreichen.
Redundanzen unterhalb der "normal"-Einstellung (14.3%) sind nicht empfehlenswert, da die Fehlerkorrektur sonst zu
schnell überlastet werden könnte.
<?php end_screen_shot(); ?>

<?php begin_screen_shot("Reiterkarte \"Dateien\".","create-prefs-file.png"); ?>
<b>Reiterkarte "Dateien".</b> Lassen Sie die Optionen auf dieser Reiterkarte
zunächst ausgeschaltet; Hinweise auf <a href="howtos25.php">Optimierungen</a>
mit Hilfe dieser Werte folgen später.
<?php end_screen_shot(); ?>

<pre> </pre>

<b>Nicht verwendete Reiterkarten</b><p>

Die Reiterkarte "Sonstiges" enthält momentan nur Einstellungen zum Erzeugen von Protokolldateien,
die zum Einsenden von <a href="feedback.php">Fehlerberichten</a> benötigt werden. Zum Erstellen
von Fehlerkorrektur-Dateien werden diese Funktionen nicht benötigt.
In der Reiterkarte "Darstellung" können Sie die Anzeige
von dvdisaster nach Ihrem Geschmack farblich verändern; dies hat aber
keine Auswirkungen auf die erstellten Fehlerkorrektur-Dateien.

<pre> </pre>

<a href="howtos23.php">Fehlerkorrektur-Datei erzeugen...</a>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
