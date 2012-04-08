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

howto_headline("Fehlerkorrektur-Daten auf dem Datenträger ablegen", "Grundeinstellungen", "images/create-icon.png");
?>

<!-- Insert actual page content below -->

<?php begin_screen_shot("Einstellungsdialog aufrufen.","global-prefs-invoke.png"); ?>
<table><tr><td class="valignt"><img src="../images/prefs-icon.png" alt="Bedienelement: Einstellungen (Auswahlknopf)" class="valignb"></td>
<td>Die nachfolgend besprochenen Reiterkarten finden Sie
im Einstellungsdialog. Das zum Aufruf verwendete Symbol ist
in dem Bildschirmfoto grün markiert (Anklicken vergrößert das Bild).
Das Symbol kann je nach verwendetem Symbol-Thema anders aussehen.</td>
</tr></table>
<?php end_screen_shot(); ?>

<pre> </pre>

<?php begin_screen_shot("Reiterkarte \"Fehlerkorrektur\".","create-prefs-ecc2.png"); ?>
<b>Reiterkarte "Fehlerkorrektur".</b> Wählen Sie
als Fehlerkorrektur-Verfahren "Erweitertes Abbild (RS02)" (grünes Auswahlmenü, oben).
Überlassen Sie dvdisaster die Auswahl der Abbild-Größe, indem Sie die
Funktion "Verwende kleinsten möglichen Wert aus folgender Tabelle" (ebenfalls
grün markiert) aktivieren. Dies bewirkt, daß dvdisaster den kleinsten
Datenträger-Typ auswählt, auf den das erweiterte Abbild gespeichert werden kann.<p>
<?php end_screen_shot(); ?>


<pre> </pre>

<b>Nicht verwendete Reiterkarten</b><p>

Die übrigen Reiterkarten haben keinen Einfluß auf die Erstellung der 
Fehlerkorrektur-Daten. Mit der Reiterkarte "Darstellung" können Sie die Anzeige
von dvdisaster nach Ihrem Geschmack farblich verändern; dies hat aber
keine Auswirkungen auf die erstellten Fehlerkorrektur-Daten.

<pre> </pre>

<a href="howtos33.php">Fehlerkorrektur-Daten erzeugen...</a>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
