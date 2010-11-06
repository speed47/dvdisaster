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

?>

<!--- Insert actual page content below --->

<h3>Fehlerkorrektur-Datei auswählen</h3>

Die Fehlerkorrektur-Datei enthält Informationen, um unlesbare Sektoren aus einem
beschädigten Datenträger wiederherzustellen. Sie ist auch hilfreich, um einen
Datenträger auf Beschädigungen zu überprüfen. Die voreingestellte 
Dateinamen-Erweiterung ist ".ecc".<p>

<?php begin_screen_shot("Fehlerkorrektur-Datei auswählen","dialog-ecc-full.png"); ?>
Sie können die Fehlerkorrektur-Datei auf zwei Arten auswählen:
<ul>
<li>im normalen <a href="#filechooser">Dateiauswahl-Dialog</a> (grün markierter Knopf), oder</li>
<li>durch Direkteingabe in das Textfeld (blau markiertes Textfeld).</li><p>
</ul>
Die Direkteingabe ist hilfreich, 
wenn Sie mehrere Fehlerkorrektur-Dateien nacheinander im gleichen
Unterverzeichnis erzeugen. Ändern Sie dazu einfach den Namen in dem Textfeld.<p>
<?php end_screen_shot(); ?>

<? require("howtos_winfile.php"); ?>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
