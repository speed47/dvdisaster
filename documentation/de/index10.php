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
?>

<!-- Insert actual page content below -->

<h3 class="top">Beispiele für die Fehlerkorrektur</h3>

<?php begin_screen_shot("Einlesen eines beschädigten Datenträgers.","recover-linear.png"); ?>
   <b>Einlesen beschädigter Datenträger.</b> Der hier bearbeitete Datenträger
   ist in den äußeren Bereichen verfärbt und teilweise unlesbar geworden. Eine
   Oberflächenanalyse ergibt rund 23.000 unlesbare Sektoren (von insgesamt ca. 342.000
   Sektoren, also  7,2% defekte Sektoren).
<?php end_screen_shot(); ?>

<?php begin_screen_shot("Wiederherstellung des Abbilds.","fix-image.png"); ?>
   <b>Wiederherstellung des Abbilds.</b> Das eingelesene Abbild ist unvollständig,
   da rund 23.000 Sektoren nicht gelesen werden konnten. Diese Sektoren werden nun
   mit Hilfe der Fehlerkorrektur-Daten
   von dvdisaster wiederhergestellt.
   Bei der Wiederherstellung enthielt der schlechteste
   Fehlerkorrektur-Bereich 20 Fehler. Dies entspricht
   einer maximal 63%-igen Auslastung der Fehlerkorrektur und bedeutet, daß der
   vorliegende Grad an Beschädigung ohne Schwierigkeiten durch die Fehlerkorrektur
   mit den gewählten Grundeinstellungen bewältigt werden kann. 
<?php end_screen_shot(); ?>

<b>Die Wiederherstellung benötigt Fehlerkorrektur-Daten:</b> 
Der gerade beschriebene Wiederherstellungsprozeß benötigte
Fehlerkorrektur-Daten. Stellen Sie sich diese
Daten als eine spezielle Form des Anlegens einer Sicherungskopie vor (der Speicherbedarf 
ist allerdings geringer als bei einer vollständigen Sicherung).
Wie bei einer normalen Datensicherung müssen die Fehlerkorrektur-Daten angelegt werden,
<i>bevor</i> der Datenträger beschädigt wird.<p>

Oder anderes herum gesagt, wenn Sie einen beschädigten Datenträger besitzen, aber keine
Fehlerkorrektur-Daten dafür angelegt haben, dann sind Ihre Daten sehr wahrscheinlich
verloren.<p>


<a href="index20.php">Warum Qualitäts-Analysen nicht ausreichen...</a>


<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
