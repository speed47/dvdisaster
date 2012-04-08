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

howto_headline("Datenträger überprüfen", "fortgeschrittene Einstellungen", "images/scan-icon.png");
?>

<!-- Insert actual page content below -->

<?php begin_screen_shot("Reiterkarte \"Laufwerk\".","scan-prefs-drive-adv.png"); ?>
<b>Schwerwiegende Fehler nicht beachten.</b>
Normalerweise beendet dvdisaster die Überprüfung, wenn das Laufwerk einen 
schwerwiegenden Fehler wie z.B. ein Problem mit der Laufwerksmechanik meldet.
Dies soll eine Beschädigung des Laufwerks verhindern.
Einige Laufwerke melden schwere Fehler aber irrtümlich, wenn sie durch einen
beschädigten Datenträger verwirrt werden. Wenn Sie so ein Laufwerk haben,
aktivieren Sie diese Option, um den Datenträger bis zum Ende überprüfen zu können.<p>
<b>Datenträger nach erfolgreichem Lesen auswerfen.</b>
Ist diese Option aktiviert, so versucht dvdisaster den Datenträger nach einer
erfolgreichen Überprüfung automatisch auszuwerfen, sofern das Betriebssystem dies
erlaubt. Wenn sich
nach dem Einlegen des Datenträgers automatisch ein Fenster zum Abspielen oder Ansehen
des Inhalts öffnet, ist das automatische Auswerfen typischerweise nicht möglich.
<p>
<?php end_screen_shot(); ?>

<pre> </pre>

<?php begin_screen_shot("Reiterkarte \"Leseversuche\".","scan-prefs-read-attempts-adv.png"); ?>
<b>Sektoren nach einem Lesefehler überspringen.</b> 
Jeder Versuch, einen fehlerhaften Sektor zu lesen
kostet verhältnismäßig viel Zeit. Da die Wahrscheinlichkeit hoch ist, 
daß nach einem Fehler noch weitere unmittelbar folgen, spart das
Überspringen einer Anzahl von Sektoren nach einem Lesefehler Zeit.
Wenn Sie einen schnellen Überblick über den Grad der Beschädigung des Datenträgers
möchten, sollten Sie daher diesen Wert auf z.B. 1024 hoch setzen. 
Allerdings werden alle übersprungenen Sektoren als fehlerhaft betrachtet, d.h.
die Anzahl der gemeldeten Fehler wird höher und ungenauer.
<p>
<?php end_screen_shot(); ?>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
