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

howto_headline("Fehlerkorrektur-Daten auf dem Datenträger ablegen", "fortgeschrittene Einstellungen", "images/create-icon.png");
?>

<?php begin_screen_shot("Reiterkarte \"Fehlerkorrektur\".","create-prefs-ecc2-adv.png"); ?>
<b>Datenträger-Größen anpassen</b>. In der Tabelle sind für CD, DVD und BD 
standardisierte Mindestgrößen voreingestellt. Diese werden hoffentlich 
von allen Datenträgern erfüllt. 
Einige Hersteller produzieren jedoch Datenträger mit leicht höheren
Kapazitäten. Wenn Sie einen solchen Datenträger haben, legen Sie Ihn in das momentan
in dvdisaster ausgewählte Laufwerk ein und klicken Sie auf den 
grün markierten "Datenträger abfragen"-Knopf. dvdisaster wird dann die 
Größe des Datenträgers ermitteln und in die Tabelle aufnehmen.<p>
<b>Hinweis:</b> Die Datenträger-Größe kann typischerweise nur von Laufwerken
ermittelt werden, die den entsprechenden Datenträger-Typ auch schreiben können.
<pre> </pre>
<b>Freie Wahl der Datenträger-Größe.</b> Sie können auch die Größe frei vorgeben,
die das Abbild nach der Erweiterung um Fehlerkorrektur-Daten haben darf.
Aktivieren Sie in diesem Fall die gelb markierte Funktion "Verwende höchstens ... Sektoren" und tragen Sie in das gelb markierte Feld die Anzahl der Sektoren ein (1 Sektor = 2KB), die das Abbild höchstens erreichen darf.

<?php end_screen_shot(); ?>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
