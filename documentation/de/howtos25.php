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

howto_headline("Fehlerkorrektur-Datei erstellen", "fortgeschrittene Einstellungen", "images/create-icon.png");
?>

<?php begin_screen_shot("Reiterkarte \"Laufwerk\".","create-prefs-drive-adv.png"); ?>
<b>Datenträger nach erfolgreichem Lesen auswerfen</b>. Diese Option ist hilfreich, wenn
Sie mehrere Datenträger bearbeiten möchten. dvdisaster wird versuchen, den Datenträger
nach dem erfolgreichen Einlesen auszuwerfen, sofern das Betriebssystem dies erlaubt.
Wenn sich nach dem Einlegen des Datenträgers automatisch ein Fenster zum Abspielen oder Ansehen des Inhalts öffnet, ist das automatische Auswerfen typischerweise nicht möglich.
<?php end_screen_shot(); ?>

<?php begin_screen_shot("Reiterkarte \"Dateien\".","create-prefs-file-adv.png"); ?>
<b>Automatisches Erstellen und Löschen von Dateien.</b> Wenn Sie Fehlerkorrektur-Dateien
zu mehreren Datenträgern hintereinander erstellen, können Sie den Vorgang mit Hilfe
dieser beiden Einstellungen automatisieren. Die erste Einstellung bewirkt, daß sofort nach dem
Einlesen eines Datenträgers mit dem Erstellen der Fehlerkorrektur-Datei begonnen wird.
Die zweite Option löscht das eingelesene Abbild, sobald die Fehlerkorrektur-Datei erstellt 
wurde.<p>
<b>Hinweis:</b> Denken Sie daran, nach dem Einlegen eines neuen Datenträgers einen neuen
Namen für die Fehlerkorrektur-Datei anzugeben, da sonst die vorherige Datei überschrieben wird.
<?php end_screen_shot(); ?>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
